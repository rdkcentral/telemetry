/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2019 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#include <stdbool.h>
#include <malloc.h>
#include <string.h>
#include <signal.h>
#include <cjson/cJSON.h>

#include "profilexconf.h"
#include "reportprofiles.h"
#include "t2eventreceiver.h"
#include "t2markers.h"
#include "t2log_wrapper.h"
#include "busInterface.h"
#include "curlinterface.h"
#include "scheduler.h"
#include "persistence.h"
#include "vector.h"
#include "dcautil.h"
#include "t2parserxconf.h"

#define T2REPORT_HEADER "T2"
#define T2REPORT_HEADERVAL  "1.0"
#define DEFAULT_FIRST_REPORT_INT 0

#define MAX_TIME_INFO_LEN  35

static bool initialized = false;

// This is profileXconf object which is used to store the profile information
static ProfileXConf *singleProfile = NULL;
static pthread_mutex_t plMutex; /* TODO - we can remove plMutex most likely but firseck that CollectAndReport doesn't cause issue */
static pthread_cond_t reuseThread;

static bool reportThreadExits = false;

static pid_t xconfReportPid;
static bool isAbortTriggered = false ;
static bool isOnDemandReport = false ;

static char *getTimeStamp (void)
{
    char *timeStamp;
    timeStamp = malloc(MAX_TIME_INFO_LEN);
    if (timeStamp)
    {
        time_t timeObj = time(NULL);
        struct tm *tmInfo = gmtime(&timeObj);
        // Format -  yyyy-mm-dd hh:mm:ss
        if (strftime(timeStamp, MAX_TIME_INFO_LEN, "%F %H:%M:%S", tmInfo) == 0)
        {
            free(timeStamp);
            timeStamp = NULL;
        }
    }

    return timeStamp;
}

static void freeConfig(void *data)
{
    if(data != NULL)
    {
        Config *config = (Config *)data;

        if(config->name)
        {
            free(config->name);
        }
        if(config->configData)
        {
            free(config->configData);
        }

        free(config);
    }
}

static void freeProfileXConf()
{
    if(singleProfile != NULL)
    {
        if(singleProfile->name)
        {
            free(singleProfile->name);
        }
        if(singleProfile->protocol)
        {
            free(singleProfile->protocol);
        }
        if(singleProfile->encodingType)
        {
            free(singleProfile->encodingType);
        }
        if(singleProfile->t2HTTPDest)
        {
            free(singleProfile->t2HTTPDest->URL);
            free(singleProfile->t2HTTPDest);
        }
        if(singleProfile->eMarkerList)
        {
            Vector_Destroy(singleProfile->eMarkerList, freeEMarker);
        }
        if(singleProfile->gMarkerList)
        {
            Vector_Destroy(singleProfile->gMarkerList, freeGMarker);
        }
        if(singleProfile->paramList)
        {
            Vector_Destroy(singleProfile->paramList, freeParam);
        }

        if(singleProfile->jsonReportObj)
        {
            cJSON_Delete(singleProfile->jsonReportObj);
            singleProfile->jsonReportObj = NULL;
        }

        // Data elements from this list is copied in new profile. So do not destroy the vector
        if(singleProfile->cachedReportList)
        {
            free(singleProfile->cachedReportList);
            singleProfile->cachedReportList = NULL;
        }
        free(singleProfile);
        singleProfile = NULL;
    }
}

/* This function is used to initialize the JSON report object and add the header, profile name and timestamp to it.
 * @param jsonObj Pointer to the cJSON object to be initialized.
 * @param valArray Pointer to the cJSON array to be created and added to the JSON object.
 * @return T2ERROR_SUCCESS on success, T2ERROR_FAILURE on failure.
 */
static T2ERROR initJSONReportXconf(cJSON** jsonObj, cJSON **valArray)
{
    *jsonObj = cJSON_CreateObject();
    if(*jsonObj == NULL)
    {
        T2Error("Failed to create cJSON object\n");
        return T2ERROR_FAILURE;
    }

    cJSON_AddItemToObject(*jsonObj, "searchResult", *valArray = cJSON_CreateArray());

    cJSON *arrayItem = NULL;
    char *currenTime ;

    arrayItem = cJSON_CreateObject();
    cJSON_AddStringToObject(arrayItem, T2REPORT_HEADER, T2REPORT_HEADERVAL);
    cJSON_AddItemToArray(*valArray, arrayItem);

    arrayItem = cJSON_CreateObject();
    // For Comcast platforms, requirement from field triage to be a fixed string instead of actual profile name .
#if defined(_LG_OFW_)
    cJSON_AddStringToObject(arrayItem, "Profile", singleProfile->name);
#elif defined(ENABLE_RDKB_SUPPORT)
    cJSON_AddStringToObject(arrayItem, "Profile", "RDKB");
#elif defined(ENABLE_RDKC_SUPPORT)
    cJSON_AddStringToObject(arrayItem, "Profile", "RDKC");
#else
    cJSON_AddStringToObject(arrayItem, "Profile", "RDKV");
#endif
    cJSON_AddItemToArray(*valArray, arrayItem);

    currenTime = getTimeStamp();
    arrayItem = cJSON_CreateObject( );
    if (NULL != currenTime)
    {
        cJSON_AddStringToObject(arrayItem, "Time", currenTime);
        free(currenTime);
        currenTime = NULL;
    }
    else
    {
        cJSON_AddStringToObject(arrayItem, "Time", "Unknown");
    }
    cJSON_AddItemToArray(*valArray, arrayItem);

    return T2ERROR_SUCCESS;
}


/* This function is used to collect and report the Xconf data.
 * It runs in a separate thread and performs the following steps:
 * 1. Initializes the JSON report object.
 * 2. Collects parameter values and grep results.
 * 3. Prepares the JSON report.
 * 4. Sends the report over HTTP if the protocol is HTTP.
 * 5. Cleans up resources and waits for the next iteration.
 */
static void* CollectAndReportXconf(void* data)
{
    (void)data; // To fix compiler warning
    pthread_mutex_lock(&plMutex);
    ProfileXConf* profile = singleProfile;
    if (!profile)
    {
        T2Error("profile is NULL\n");
        pthread_mutex_unlock(&plMutex);
        return NULL;
    }

    pthread_cond_init(&reuseThread, NULL);
    reportThreadExits = true;

    while (initialized)
    {
        T2Debug("%s while Loop -- START \n", __FUNCTION__);
        profile = singleProfile;

        if (!profile || !profile->name)
        {
            T2Error("Invalid profile or profile name\n");
            goto reportXconfThreadEnd;
        }

        T2Info("%s ++in profileName : %s\n", __FUNCTION__, profile->name);

        if (prepareAndSendReport(profile) != T2ERROR_SUCCESS)
        {
            T2Error("Failed to prepare and send report for profile: %s\n", profile->name);
        }

        profile->reportInProgress = false;

reportXconfThreadEnd:
        T2Debug("%s while Loop -- END \n", __FUNCTION__);
        pthread_cond_wait(&reuseThread, &plMutex);
    }

    reportThreadExits = false;
    pthread_mutex_unlock(&plMutex);
    pthread_cond_destroy(&reuseThread);
    T2Debug("%s --out exiting the CollectAndReportXconf thread \n", __FUNCTION__);
    return NULL;
}

static T2ERROR prepareAndSendReport(ProfileXConf* profile)
{
    Vector *profileParamVals = NULL, *grepResultList = NULL;
    cJSON *valArray = NULL;
    char* jsonReport = NULL;
    char* customLogPath = NULL;
    bool checkRotated = true;
    T2ERROR ret = T2ERROR_FAILURE;

    struct timespec startTime, endTime, elapsedTime;
    clock_gettime(CLOCK_REALTIME, &startTime);

    if (profile->encodingType && strcmp(profile->encodingType, "JSON") == 0)
    {
        if (initJSONReportXconf(&profile->jsonReportObj, &valArray) != T2ERROR_SUCCESS)
        {
            T2Error("Failed to initialize JSON Report\n");
            goto cleanup;
        }

#ifdef PERSIST_LOG_MON_REF
        if (profile->checkPreviousSeek)
        {
            addPreviousLogsHeader(valArray);
            customLogPath = PREVIOUS_LOGS_PATH;
            profile->bClearSeekMap = true;
            checkRotated = false;
        }
#endif

        collectProfileData(profile, valArray, &profileParamVals, &grepResultList, customLogPath, checkRotated);

        if (prepareJSONReport(profile->jsonReportObj, &jsonReport) != T2ERROR_SUCCESS)
        {
            T2Error("Unable to generate report for : %s\n", profile->name);
            goto cleanup;
        }

        ret = handleReportUpload(profile, jsonReport);
    }
    else
    {
        T2Error("Unsupported encoding format : %s\n", profile->encodingType);
    }

cleanup:
    clock_gettime(CLOCK_REALTIME, &endTime);
    getLapsedTime(&elapsedTime, &endTime, &startTime);
    T2Info("Elapsed Time for : %s = %lu.%lu (Sec.NanoSec)\n", profile->name, (unsigned long)elapsedTime.tv_sec, elapsedTime.tv_nsec);

    if (jsonReport)
    {
        free(jsonReport);
    }

    return ret;
}

/* This function adds a header to the JSON report indicating that it contains previous logs.
 * @param valArray Pointer to the cJSON array to which the header will be added.
 */
static void addPreviousLogsHeader(cJSON* valArray)
{
    cJSON *arrayItem = cJSON_CreateObject();
    cJSON_AddStringToObject(arrayItem, PREVIOUS_LOG, PREVIOUS_LOGS_VAL);
    cJSON_AddItemToArray(valArray, arrayItem);
    T2Debug("Adding Previous Logs Header to JSON report\n");
}

static void collectProfileData(ProfileXConf* profile, cJSON* valArray, Vector** profileParamVals, Vector** grepResultList, char* customLogPath, bool checkRotated)
{
    if (profile->paramList && Vector_Size(profile->paramList) > 0)
    {
        *profileParamVals = getProfileParameterValues(profile->paramList);
        if (*profileParamVals)
        {
            encodeParamResultInJSON(valArray, profile->paramList, *profileParamVals);
            Vector_Destroy(*profileParamVals, freeProfileValues);
        }
    }

    if (profile->gMarkerList && Vector_Size(profile->gMarkerList) > 0)
    {
        getGrepResults(profile->name, profile->gMarkerList, grepResultList, profile->bClearSeekMap, checkRotated, customLogPath);
        if (*grepResultList)
        {
            encodeGrepResultInJSON(valArray, *grepResultList);
            Vector_Destroy(*grepResultList, freeGResult);
        }
    }

    if (profile->eMarkerList && Vector_Size(profile->eMarkerList) > 0)
    {
        encodeEventMarkersInJSON(valArray, profile->eMarkerList);
    }
}

static T2ERROR handleReportUpload(ProfileXConf* profile, char* jsonReport)
{
    T2ERROR ret = T2ERROR_FAILURE;

    if (profile->protocol && strcmp(profile->protocol, "HTTP") == 0)
    {
        if (isAbortTriggered)
        {
            T2Info("On-demand report upload has been aborted. Skip report upload \n");
        }
        else
        {
            ret = sendReportOverHTTP(profile->t2HTTPDest->URL, jsonReport, &xconfReportPid);
        }

#ifdef PERSIST_LOG_MON_REF
        handleSeekConfig(profile);
#endif

        if (ret == T2ERROR_FAILURE)
        {
            cacheFailedReport(profile, jsonReport);
        }
        else if (profile->cachedReportList && Vector_Size(profile->cachedReportList) > 0)
        {
            ret = sendCachedReportsOverHTTP(profile->t2HTTPDest->URL, profile->cachedReportList);
            if (ret == T2ERROR_SUCCESS)
            {
                removeProfileFromDisk(CACHED_MESSAGE_PATH, profile->name);
            }
        }
    }
    else
    {
        T2Error("Unsupported report send protocol : %s\n", profile->protocol);
    }

    return ret;
}

#ifdef PERSIST_LOG_MON_REF
static void handleSeekConfig(ProfileXConf* profile)
{
    if (profile->saveSeekConfig)
    {
        saveSeekConfigtoFile(profile->name);
    }

    if (profile->checkPreviousSeek)
    {
        profile->checkPreviousSeek = false;
        profile->bClearSeekMap = false;
    }
}
#endif

static void cacheFailedReport(ProfileXConf* profile, char* jsonReport)
{
    if (profile->cachedReportList && Vector_Size(profile->cachedReportList) >= MAX_CACHED_REPORTS)
    {
        while (Vector_Size(profile->cachedReportList) > MAX_CACHED_REPORTS)
        {
            char* extraCachedReport = (char*)Vector_At(profile->cachedReportList, Vector_Size(profile->cachedReportList) - 1);
            Vector_RemoveItem(profile->cachedReportList, extraCachedReport, NULL);
            free(extraCachedReport);
        }
    }

    Vector_PushBack(profile->cachedReportList, strdup(jsonReport));
    saveCachedReportToPersistenceFolder(profile->name, profile->cachedReportList);
}

T2ERROR ProfileXConf_init(bool checkPreviousSeek)
{
    (void) checkPreviousSeek; // To fix compiler warning
    T2Debug("%s ++in\n", __FUNCTION__);
    if(!initialized)
    {
        Vector *configList = NULL;
        Config *config = NULL;

        initialized = true;
        if(pthread_mutex_init(&plMutex, NULL) != 0)
        {
            T2Error("%s Mutex init has failed\n", __FUNCTION__);
            return T2ERROR_FAILURE;
        }
        Vector_Create(&configList);
        fetchLocalConfigs(XCONFPROFILE_PERSISTENCE_PATH, configList);

        if(Vector_Size(configList) > 1) // This is to address corner cases where if multiple configs are saved, we ignore them and wait for new config.
        {
            T2Info("Found multiple saved profiles, removing all from the disk and waiting for updated configuration\n");
            clearPersistenceFolder(XCONFPROFILE_PERSISTENCE_PATH);
        }
        else if(Vector_Size(configList) == 1)
        {
            config = Vector_At(configList, 0);
            ProfileXConf *profile = 0;
            T2Debug("Processing config with name : %s\n", config->name);
            T2Debug("Config Size = %lu\n", (unsigned long)strlen(config->configData));
            if(T2ERROR_SUCCESS == processConfigurationXConf(config->configData, &profile))
            {
#ifdef PERSIST_LOG_MON_REF
                if(checkPreviousSeek && loadSavedSeekConfig(profile->name) == T2ERROR_SUCCESS && firstBootStatus())
                {
                    profile->checkPreviousSeek = true;
                }
                else
                {
                    profile->checkPreviousSeek = false;
                }
#else
                profile->checkPreviousSeek = false;
#endif

                if(T2ERROR_SUCCESS == ProfileXConf_set(profile))
                {
                    T2Info("Successfully set new profile: %s\n", profile->name);

#ifdef PERSIST_LOG_MON_REF
                    if(profile->checkPreviousSeek)
                    {
                        T2Info("Previous Seek is enabled so generate the Xconf report \n");
                        // Trigger a xconf report if previous seek is enabled and valid.
                        ProfileXConf_notifyTimeout(true, true);
                    }
#endif
                    populateCachedReportList(profile->name, profile->cachedReportList);
                }
                else
                {
                    T2Error("Failed to set new profile: %s\n", profile->name);
                }
            }
        }
        Vector_Destroy(configList, freeConfig);
    }
    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

T2ERROR ProfileXConf_uninit()
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if(!initialized || !singleProfile)
    {
        T2Info("profile list is not initialized yet, ignoring\n");
        return T2ERROR_SUCCESS;
    }
    initialized = false;

    if(singleProfile->reportInProgress)
    {
        T2Debug("Waiting for final report before uninit\n");
        pthread_mutex_lock(&plMutex);
        pthread_cond_signal(&reuseThread);
        pthread_mutex_unlock(&plMutex);
        pthread_join(singleProfile->reportThread, NULL);
        reportThreadExits = false;
        singleProfile->reportInProgress = false ;
        T2Info("Final report is completed, releasing profile memory\n");
    }
    pthread_mutex_lock(&plMutex);
    freeProfileXConf();
    pthread_mutex_unlock(&plMutex);

    pthread_mutex_destroy(&plMutex);
    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

T2ERROR ProfileXConf_set(ProfileXConf *profile)
{
    T2Debug("%s ++in\n", __FUNCTION__);

    T2ERROR ret = T2ERROR_FAILURE;

    pthread_mutex_lock(&plMutex);

    if(!singleProfile)
    {
        singleProfile = profile;
        singleProfile->reportInProgress = false ;
        size_t emIndex = 0;
        EventMarker *eMarker = NULL;
        for(; emIndex < Vector_Size(singleProfile->eMarkerList); emIndex++)
        {
            eMarker = (EventMarker *)Vector_At(singleProfile->eMarkerList, emIndex);
            addT2EventMarker(eMarker->markerName, eMarker->compName, singleProfile->name, eMarker->skipFreq);
        }

        if(registerProfileWithScheduler(singleProfile->name, singleProfile->reportingInterval, INFINITE_TIMEOUT, false, true, false, DEFAULT_FIRST_REPORT_INT, NULL) == T2ERROR_SUCCESS)
        {
            T2Info("Successfully set profile : %s\n", singleProfile->name);
            ret = T2ERROR_SUCCESS;
        }
        else
        {
            T2Error("Unable to register profile : %s with Scheduler\n", singleProfile->name);
        }
    }
    else
    {
        T2Error("XConf profile already added, can't have more then 1 profile\n");
    }

    pthread_mutex_unlock(&plMutex);

    T2Debug("%s --out\n", __FUNCTION__);
    return ret;
}

void ProfileXConf_updateMarkerComponentMap()
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if(!initialized)
    {
        T2Error("profile list is not initialized yet, ignoring\n");
        return ;
    }
    size_t emIndex = 0;
    EventMarker *eMarker = NULL;
    pthread_mutex_lock(&plMutex);
    if(singleProfile)
    {
        for(; emIndex < Vector_Size(singleProfile->eMarkerList); emIndex++)
        {
            eMarker = (EventMarker *)Vector_At(singleProfile->eMarkerList, emIndex);
            addT2EventMarker(eMarker->markerName, eMarker->compName, singleProfile->name, eMarker->skipFreq);
        }
    }
    else
    {
        T2Error("Profile not found in %s\n", __FUNCTION__);
    }
    pthread_mutex_unlock(&plMutex);
    T2Debug("%s --out\n", __FUNCTION__);
}

bool ProfileXConf_isNameEqual(char* profileName)
{
    bool isName = false;
    pthread_mutex_lock(&plMutex);
    if(initialized)
    {
        if(singleProfile && !strcmp(singleProfile->name, profileName))
        {
            isName = true;
        }
    }
    pthread_mutex_unlock(&plMutex);
    return isName;
}

T2ERROR ProfileXConf_delete(ProfileXConf *profile)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if (!initialized)
    {
        T2Error("Profile list is not initialized yet, ignoring\n");
        return T2ERROR_FAILURE;
    }

    bool isNameEqual = ProfileXConf_isNameEqual(profile->name);

    pthread_mutex_lock(&plMutex);
    if (!singleProfile)
    {
        T2Error("Profile not found in %s\n", __FUNCTION__);
        pthread_mutex_unlock(&plMutex);
        return T2ERROR_FAILURE;
    }

    if (singleProfile->reportInProgress)
    {
        T2Info("Waiting for CollectAndReport to complete: %s\n", singleProfile->name);
        initialized = false;
        pthread_cond_signal(&reuseThread);
        pthread_mutex_unlock(&plMutex);
        pthread_join(singleProfile->reportThread, NULL);
        reportThreadExits = false;
        initialized = true;
        singleProfile->reportInProgress = false;
    }
    else
    {
        pthread_mutex_unlock(&plMutex);
    }

    pthread_mutex_lock(&plMutex);

    if (isNameEqual)
    {
        profile->bClearSeekMap = singleProfile->bClearSeekMap;
        profile->reportInProgress = false;

        if (singleProfile->cachedReportList && Vector_Size(singleProfile->cachedReportList) > 0)
        {
            size_t count = Vector_Size(singleProfile->cachedReportList);
            for (size_t i = 0; i < count; i++)
            {
                Vector_PushBack(profile->cachedReportList, Vector_At(singleProfile->cachedReportList, 0));
                Vector_RemoveItem(singleProfile->cachedReportList, Vector_At(singleProfile->cachedReportList, 0), NULL);
            }
        }
    }
    else
    {
        if (singleProfile->cachedReportList && Vector_Size(singleProfile->cachedReportList) > 0)
        {
            Vector_Destroy(singleProfile->cachedReportList, free);
            singleProfile->cachedReportList = NULL;
        }
    }

    if (Vector_Size(singleProfile->eMarkerList) > 0 && Vector_Size(profile->eMarkerList) > 0)
    {
        for (size_t j = 0; j < Vector_Size(profile->eMarkerList); j++)
        {
            EventMarker *eMarkerNew = Vector_At(profile->eMarkerList, j);
            EventMarker *eMarkerCurrent = NULL;

            for (size_t i = 0; i < Vector_Size(singleProfile->eMarkerList); i++)
            {
                eMarkerCurrent = Vector_At(singleProfile->eMarkerList, i);
                if (strcmp(eMarkerNew->markerName, eMarkerCurrent->markerName) == 0 && eMarkerNew->mType == eMarkerCurrent->mType)
                {
                    break;
                }
                eMarkerCurrent = NULL;
            }

            if (eMarkerCurrent)
            {
                switch (eMarkerNew->mType)
                {
                    case MTYPE_XCONF_COUNTER:
                        eMarkerNew->u.count = eMarkerCurrent->u.count;
                        break;
                    case MTYPE_XCONF_ABSOLUTE:
                        if (eMarkerCurrent->u.markerValue)
                        {
                            eMarkerNew->u.markerValue = strdup(eMarkerCurrent->u.markerValue);
                        }
                        break;
                    case MTYPE_XCONF_ACCUMULATE:
                        if (eMarkerCurrent->u.accumulatedValues)
                        {
                            size_t count = Vector_Size(eMarkerCurrent->u.accumulatedValues);
                            for (size_t k = 0; k < count; k++)
                            {
                                Vector_PushBack(eMarkerNew->u.accumulatedValues, Vector_At(eMarkerCurrent->u.accumulatedValues, k));
                            }
                            Vector_Clear(eMarkerCurrent->u.accumulatedValues, free);
                        }
                        break;
                    default:
                        T2Error("Unsupported marker type: %d\n", eMarkerNew->mType);
                        break;
                }
                Vector_RemoveItem(singleProfile->eMarkerList, eMarkerCurrent, freeEMarker);
            }
        }
    }

    if (Vector_Size(singleProfile->gMarkerList) > 0)
    {
        bool clearSeekMap = !isNameEqual;
#ifdef PERSIST_LOG_MON_REF
        if (!isNameEqual)
        {
            removeProfileFromDisk(SEEKFOLDER, singleProfile->name);
        }
#endif
        removeGrepConfig(singleProfile->name, clearSeekMap, true);
    }

    T2Info("Removing profile: %s\n", singleProfile->name);
    freeProfileXConf();

    pthread_mutex_unlock(&plMutex);
    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

bool ProfileXConf_isSet()
{
    bool isSet = false;
    pthread_mutex_lock(&plMutex);

    if(singleProfile != NULL)
    {
        T2Debug("ProfileXConf is set\n");
        isSet = true;
    }

    pthread_mutex_unlock(&plMutex);
    return isSet;
}

char* ProfileXconf_getName()
{
    char* profileName = NULL ;
    pthread_mutex_lock(&plMutex);
    if(initialized)
    {
        if(singleProfile)
        {
            profileName = strdup(singleProfile->name);
        }
    }
    pthread_mutex_unlock(&plMutex);
    return profileName;
}

void ProfileXConf_notifyTimeout(bool isClearSeekMap, bool isOnDemand)
{
    T2Debug("%s ++in\n", __FUNCTION__);

    int reportThreadStatus = 0 ;
    pthread_mutex_lock(&plMutex);
    if(!singleProfile)
    {
        T2Error("Profile not found in %s\n", __FUNCTION__);
        pthread_mutex_unlock(&plMutex);
        return ;
    }
    isOnDemandReport = isOnDemand;
    if(!singleProfile->reportInProgress)
    {
        singleProfile->bClearSeekMap = isClearSeekMap;
        singleProfile->reportInProgress = true;

        if (reportThreadExits)
        {
            pthread_cond_signal(&reuseThread);
        }
        else
        {
            reportThreadStatus = pthread_create(&singleProfile->reportThread, NULL, CollectAndReportXconf, NULL);
            if ( reportThreadStatus != 0 )
            {
                T2Error("Failed to create report thread with error code = %d !!! \n", reportThreadStatus);
            }
        }
    }
    else
    {
        T2Warning("Received profileTimeoutCb while previous callback is still in progress - ignoring the request\n");
    }

    pthread_mutex_unlock(&plMutex);

    T2Debug("%s --out\n", __FUNCTION__);
}


T2ERROR ProfileXConf_storeMarkerEvent(T2Event *eventInfo)
{
    T2Debug("%s ++in\n", __FUNCTION__);

    pthread_mutex_lock(&plMutex);
    if(!singleProfile)
    {
        T2Error("Profile not found in %s\n", __FUNCTION__);
        pthread_mutex_unlock(&plMutex);
        return T2ERROR_FAILURE;
    }

    size_t eventIndex = 0;
    EventMarker *lookupEvent = NULL;
    for(; eventIndex < Vector_Size(singleProfile->eMarkerList); eventIndex++)
    {
        EventMarker *tempEventMarker = (EventMarker *)Vector_At(singleProfile->eMarkerList, eventIndex);
        if(!strcmp(tempEventMarker->markerName, eventInfo->name))
        {
            lookupEvent = tempEventMarker;
            break;
        }
    }
    int arraySize = 0;
    if(lookupEvent != NULL)
    {
        switch(lookupEvent->mType)
        {
        case MTYPE_XCONF_COUNTER:
            lookupEvent->u.count++;
            T2Debug("Increment marker count to : %d\n", lookupEvent->u.count);
            break;

        case MTYPE_XCONF_ACCUMULATE:
            T2Debug("Marker type is ACCUMULATE Event Value : %s\n", eventInfo->value);
            arraySize = Vector_Size(lookupEvent->u.accumulatedValues);
            T2Debug("Current array size : %d \n", arraySize);
            if( arraySize < MAX_ACCUMULATE)
            {
                Vector_PushBack(lookupEvent->u.accumulatedValues, strdup(eventInfo->value));
                T2Debug("Sucessfully added value into vector New Size : %d\n", ++arraySize);
            }
            else if ( arraySize == MAX_ACCUMULATE )
            {
                T2Warning("Max size of the array has been reached appending warning message : %s\n", MAX_ACCUMULATE_MSG);
                Vector_PushBack(lookupEvent->u.accumulatedValues, strdup(MAX_ACCUMULATE_MSG));
                T2Debug("Sucessfully added warning message into vector New Size : %d\n", ++arraySize);
            }
            else
            {
                T2Warning("Max size of the array has been reached Ignore New Value\n");
            }
            break;

        case MTYPE_XCONF_ABSOLUTE:
        default:
            if(lookupEvent->u.markerValue)
            {
                free(lookupEvent->u.markerValue);
            }
            lookupEvent->u.markerValue = strdup(eventInfo->value);
            T2Debug("New marker value saved : %s\n", lookupEvent->u.markerValue);
            break;
        }
    }
    else
    {
        T2Error("Event name : %s value : %s\n", eventInfo->name, eventInfo->value);
        T2Error("Event doens't match any marker information, shouldn't come here\n");
        pthread_mutex_unlock(&plMutex);
        return T2ERROR_FAILURE;
    }

    pthread_mutex_unlock(&plMutex);

    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}


T2ERROR ProfileXConf_terminateReport()
{

    T2ERROR ret = T2ERROR_FAILURE;

    if(!singleProfile)
    {
        T2Error("Xconf profile is not set.\n");
        return ret;
    }

    // Check whether any XconfReport is in progress
    if(singleProfile->reportInProgress)
    {
        isAbortTriggered = true;
        // Check if a child pid is still alive
        if((xconfReportPid > 0) && !kill(xconfReportPid, 0))
        {
            T2Info("Report upload in progress, terminating the forked reporting child : %d \n", xconfReportPid);
            if(!kill(xconfReportPid, SIGKILL))
            {
                ret = T2ERROR_SUCCESS;
            }
        }
        else
        {
            T2Info(" Report upload has net yet started, set the abort flag \n");
            ret = T2ERROR_SUCCESS;
        }
    }
    else
    {
        T2Info("No report generation in progress. No further action required for abort.\n");
    }

    return ret;

}

