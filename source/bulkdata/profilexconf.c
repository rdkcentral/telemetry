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
	if(singleProfile->topMarkerList)
        {
            Vector_Destroy(singleProfile->topMarkerList, freeGMarker);
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

static void* CollectAndReportXconf(void* data)
{
    (void) data;// To fix compiler warning
    pthread_mutex_lock(&plMutex);
    ProfileXConf* profile = singleProfile;
    if(profile == NULL)
    {
        T2Error("profile is NULL\n");
        pthread_mutex_unlock(&plMutex);
        return NULL;
    }
    pthread_cond_init(&reuseThread, NULL);
    reportThreadExits = true;
    do
    {
        T2Info("%s while Loop -- START \n", __FUNCTION__);
        profile = singleProfile;
        Vector *profileParamVals = NULL;
        Vector *grepResultList = NULL;
        cJSON *valArray = NULL;
        char* jsonReport = NULL;
        char* customLogPath = NULL;
        bool checkRotated = true;

        struct timespec startTime;
        struct timespec endTime;
        struct timespec elapsedTime;

        T2ERROR ret = T2ERROR_FAILURE;
        if(profile->name != NULL)
        {
            T2Info("%s ++in profileName : %s\n", __FUNCTION__, profile->name);
        }


        clock_gettime(CLOCK_REALTIME, &startTime);
        if(profile->encodingType != NULL && !strcmp(profile->encodingType, "JSON"))
        {
            if(T2ERROR_SUCCESS != initJSONReportXconf(&profile->jsonReportObj, &valArray))
            {
                T2Error("Failed to initialize JSON Report\n");
                profile->reportInProgress = false;
                //pthread_mutex_unlock(&plMutex);
                //return NULL;
                goto reportXconfThreadEnd;
            }

#ifdef PERSIST_LOG_MON_REF
            if(profile->checkPreviousSeek)
            {
                cJSON *arrayItem = NULL;
                arrayItem = cJSON_CreateObject();
                cJSON_AddStringToObject(arrayItem, PREVIOUS_LOG, PREVIOUS_LOGS_VAL);
                cJSON_AddItemToArray(valArray, arrayItem);
                customLogPath = PREVIOUS_LOGS_PATH;
                profile->bClearSeekMap = true;
                checkRotated = false;
                T2Debug("Adding Previous Logs Header to JSON report\n");
            }
#endif

            if(profile->paramList != NULL && Vector_Size(profile->paramList) > 0)
            {
                profileParamVals = getProfileParameterValues(profile->paramList);
                T2Info("Fetch complete for TR-181 Object/Parameter Values for parameters \n");
                if(profileParamVals != NULL)
                {
                    encodeParamResultInJSON(valArray, profile->paramList, profileParamVals);
                }
                Vector_Destroy(profileParamVals, freeProfileValues);
            }
	    if(profile->topMarkerList != NULL && Vector_Size(profile->topMarkerList) > 0)
            {
                Vector *topMarkerResultList = NULL;
                Vector_Create(&topMarkerResultList);
                processTopPattern(profile->name, profile->topMarkerList, topMarkerResultList);
                long int reportSize = Vector_Size(topMarkerResultList);
                if(reportSize != 0)
                {
                    T2Info("Top markers report is compleated report size %ld\n", (unsigned long)reportSize);
                    encodeGrepResultInJSON(valArray, topMarkerResultList);
                }
                else
                {
                    T2Debug("Top markers report generated but is empty possabliy the memory value is changed");
                }
                Vector_Destroy(topMarkerResultList, freeGResult);
            }
            if(profile->gMarkerList != NULL && Vector_Size(profile->gMarkerList) > 0)
            {
                getGrepResults(profile->name, profile->gMarkerList, &grepResultList, profile->bClearSeekMap, checkRotated, customLogPath); // Passing 5th argument as true to check rotated logs only in case of single profile
                T2Info("Grep complete for %lu markers \n", (unsigned long)Vector_Size(profile->gMarkerList));
                encodeGrepResultInJSON(valArray, grepResultList);
                Vector_Destroy(grepResultList, freeGResult);
            }

            dcaFlagReportCompleation();

            if(profile->eMarkerList != NULL && Vector_Size(profile->eMarkerList) > 0)
            {
                encodeEventMarkersInJSON(valArray, profile->eMarkerList);
            }
            ret = prepareJSONReport(profile->jsonReportObj, &jsonReport);
            destroyJSONReport(profile->jsonReportObj);
            profile->jsonReportObj = NULL;

            if(ret != T2ERROR_SUCCESS)
            {
                T2Error("Unable to generate report for : %s\n", profile->name);
                profile->reportInProgress = false;
                //pthread_mutex_unlock(&plMutex);
                //return NULL;
                goto reportXconfThreadEnd;
            }
            long size = strlen(jsonReport);
            T2Info("cJSON Report = %s\n", jsonReport);
            T2Info("Report Size = %ld\n", size);
            if(profile->isUpdated)
            {
                T2Info("Profile is udpated, report is cached to send with updated Profile TIMEOUT\n");
                T2Debug("Vector list size = %lu\n",  (unsigned long) Vector_Size(profile->cachedReportList));
                if(profile->cachedReportList != NULL && Vector_Size(profile->cachedReportList) >= MAX_CACHED_REPORTS)
                {
                    while(Vector_Size(profile->cachedReportList) > MAX_CACHED_REPORTS)
                    {
                        int pos = Vector_Size(profile->cachedReportList);
                        T2Info("Max Cached Reports Limit Exceeded, Removing the extra reports\n");
                        char *extraCachedreport =  (char*) Vector_At(profile->cachedReportList, (pos - 1));
                        Vector_RemoveItem(profile->cachedReportList, (void*) extraCachedreport, NULL);
                        free(extraCachedreport);
                    }
                    T2Info("Max Cached Reports Limit Reached, Overwriting third recent report\n");
                    char *thirdCachedReport = (char*) Vector_At(profile->cachedReportList, MAX_CACHED_REPORTS - 3);
                    Vector_RemoveItem(profile->cachedReportList, (void*) thirdCachedReport, NULL);
                    free(thirdCachedReport);
                }
                Vector_PushBack(profile->cachedReportList, strdup(jsonReport));
                profile->reportInProgress = false;
                /* CID 187010: Dereference before null check */
                free(jsonReport);
                jsonReport = NULL;
                T2Debug("%s --out\n", __FUNCTION__);
                //pthread_mutex_unlock(&plMutex);
                //return NULL;
                goto reportXconfThreadEnd;
            }
            if(size > DEFAULT_MAX_REPORT_SIZE)
            {
                T2Warning("Report size is exceeding the max limit : %d\n", DEFAULT_MAX_REPORT_SIZE);
            }
#ifdef PERSIST_LOG_MON_REF
            if(profile->checkPreviousSeek)
            {
                T2Info("This is a Previous Logs Report sleep randomly for 1-100 sec\n");
                srand(size + 1);
                int random_sleep = (int) rand() % 99;
                sleep(random_sleep);
            }
#endif
            if(profile->protocol != NULL && strcmp(profile->protocol, "HTTP") == 0)
            {
                // If a terminate is initiated, do not attempt to upload report
                if(isAbortTriggered)
                {
                    T2Info("On-demand report upload has been aborted. Skip report upload \n");
                    ret = T2ERROR_FAILURE ;
                }
                else
                {
                    T2Debug("Abort upload is not yet set.\n");
                    ret = sendReportOverHTTP(profile->t2HTTPDest->URL, jsonReport, &xconfReportPid);
                }

#ifdef PERSIST_LOG_MON_REF
                if(profile->saveSeekConfig)
                {
                    saveSeekConfigtoFile(profile->name);
                }
                if(profile->checkPreviousSeek)
                {
                    T2Info("Previous Logs report is sent clear the previousSeek flag\n");
                    profile->checkPreviousSeek = false;
                    customLogPath = NULL;
                    profile->bClearSeekMap = false;
                }
#endif

                xconfReportPid = -1 ;
                if(ret == T2ERROR_FAILURE)
                {
                    if(profile->cachedReportList != NULL && Vector_Size(profile->cachedReportList) >= MAX_CACHED_REPORTS)
                    {
                        while(Vector_Size(profile->cachedReportList) > MAX_CACHED_REPORTS)
                        {
                            int pos = Vector_Size(profile->cachedReportList);
                            T2Info("Max Cached Reports Limit Exceeded, Removing the extra reports\n");
                            char *extraCachedreport =  (char*) Vector_At(profile->cachedReportList, (pos - 1));
                            Vector_RemoveItem(profile->cachedReportList, (void*) extraCachedreport, NULL);
                            free(extraCachedreport);
                        }
                        T2Info("Max Cached Reports Limit Reached, Overwriting third recent report\n");
                        char *thirdCachedReport = (char*) Vector_At(profile->cachedReportList, MAX_CACHED_REPORTS - 3);
                        Vector_RemoveItem(profile->cachedReportList, (void*) thirdCachedReport, NULL);
                        free(thirdCachedReport);
                    }
                    Vector_PushBack(profile->cachedReportList, strdup(jsonReport));

                    T2Info("Report Cached, No. of reportes cached = %lu\n", (unsigned long)Vector_Size(profile->cachedReportList));
                    // Save messages from cache to a file in persistent location.
                    saveCachedReportToPersistenceFolder(profile->name, profile->cachedReportList);
                }
                else if(profile->cachedReportList != NULL && Vector_Size(profile->cachedReportList) > 0)
                {
                    T2Info("Trying to send  %lu cached reports\n", (unsigned long)Vector_Size(profile->cachedReportList));
                    ret = sendCachedReportsOverHTTP(profile->t2HTTPDest->URL, profile->cachedReportList);
                    if(ret == T2ERROR_SUCCESS)
                    {
                        // Do not get misleaded by function name. Call is to delete the directory for storing cached reports
                        removeProfileFromDisk(CACHED_MESSAGE_PATH, profile->name);
                    }
                }
            }
            else
            {
                T2Error("Unsupported report send protocol : %s\n", profile->protocol);
            }
        }
        else
        {
            T2Error("Unsupported encoding format : %s\n", profile->encodingType);
        }

# ifdef PERSIST_LOG_MON_REF
        if(T2ERROR_SUCCESS == saveSeekConfigtoFile(profile->name))
        {
            T2Info("Successfully saved grep config to file for profile: %s\n", profile->name);
        }
        else
        {
            T2Warning("Failed to save grep config to file for profile: %s\n", profile->name);
        }
#endif

        clock_gettime(CLOCK_REALTIME, &endTime);
        getLapsedTime(&elapsedTime, &endTime, &startTime);
        T2Info("Elapsed Time for : %s = %lu.%lu (Sec.NanoSec)\n", profile->name, (unsigned long)elapsedTime.tv_sec, elapsedTime.tv_nsec);
        if(jsonReport)
        {
            free(jsonReport);
            jsonReport = NULL;
        }

        // Notify status of upload in case of on demand report upload.
        if(isOnDemandReport)
        {
            if(ret == T2ERROR_FAILURE)
            {
                if(isAbortTriggered)
                {
                    publishReportUploadStatus("ABORTED");
                }
                else
                {
                    publishReportUploadStatus("FAILURE");
                }
            }
            else
            {
                publishReportUploadStatus("SUCCESS");
            }
        }

        // Reset the abort trigger flags
        if(isAbortTriggered == true)
        {
            isAbortTriggered = false ;
        }

        profile->reportInProgress = false;
        //pthread_mutex_unlock(&plMutex);
reportXconfThreadEnd :
        T2Info("%s while Loop -- END \n", __FUNCTION__);
        T2Info("%s --out\n", __FUNCTION__);
        pthread_cond_wait(&reuseThread, &plMutex);
    }
    while(initialized);
    reportThreadExits = false;
    pthread_mutex_unlock(&plMutex);
    pthread_cond_destroy(&reuseThread);
    T2Debug("%s --out exiting the CollectAndReportXconf thread \n", __FUNCTION__);
    return NULL;
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
        if(singleProfile && (singleProfile->name != NULL) && (profileName != NULL) && !strcmp(singleProfile->name, profileName)) //Adding NULL check to avoid strcmp crash
        {
            T2Info("singleProfile->name = %s and profileName = %s\n", singleProfile->name, profileName);
            isName = true;
        }
    }
    pthread_mutex_unlock(&plMutex);
    return isName;
}

T2ERROR ProfileXConf_delete(ProfileXConf *profile)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if(!initialized)
    {
        T2Error("profile list is not initialized yet, ignoring\n");
        return T2ERROR_FAILURE;
    }

    bool isNameEqual = ProfileXConf_isNameEqual(profile->name);

    pthread_mutex_lock(&plMutex);
    if(!singleProfile)
    {
        T2Error("Profile not found in %s\n", __FUNCTION__);
        pthread_mutex_unlock(&plMutex);
        return T2ERROR_FAILURE;
    }

    pthread_mutex_unlock(&plMutex);
    if(isNameEqual)
    {
        T2Info("Profile exists already, updating the config in file system\n");
        if(profile->cachedReportList != NULL)
        {
            singleProfile->isUpdated = true;
        }
    }
    else
    {
        if(T2ERROR_SUCCESS != unregisterProfileFromScheduler(singleProfile->name))
        {
            T2Error("Profile : %s failed to  unregister from scheduler\n", singleProfile->name);
        }
    }

    if(singleProfile->reportInProgress)
    {
        T2Info("Waiting for CollectAndReport to be complete : %s\n", singleProfile->name);
        pthread_mutex_lock(&plMutex);
        initialized = false;
        T2Info("Sending signal to reuse Thread in CollectAndReportXconf\n");
        pthread_cond_signal(&reuseThread);
        pthread_mutex_unlock(&plMutex);
        pthread_join(singleProfile->reportThread, NULL);
        T2Info("reportThread exits and initialising the profile list\n");
        reportThreadExits = false;
        initialized = true;
        singleProfile->reportInProgress = false ;
    }

    pthread_mutex_lock(&plMutex);

    size_t count = Vector_Size(singleProfile->cachedReportList);
    // Copy any cached message present in previous single profile to new profile
    if(isNameEqual)
    {
        profile->bClearSeekMap = singleProfile->bClearSeekMap ;
        profile->reportInProgress = false ;
        if(count > 0 && profile->cachedReportList != NULL)
        {
            T2Info("There are %zu cached reports in the profile \n", count);
            size_t index = 0;
            while(index < count)
            {
                Vector_PushBack(profile->cachedReportList, (void *) Vector_At(singleProfile->cachedReportList, 0));
                Vector_RemoveItem(singleProfile->cachedReportList, (void *) Vector_At(singleProfile->cachedReportList, 0), NULL);/*TODO why this instead of Vector_destroy*/
                index++;
            }
        }
    }
    else
    {
        if(count > 0)  //Destroy the cachedReportList vector when the profile name is not equal
        {
            Vector_Destroy(singleProfile->cachedReportList, free);
            singleProfile->cachedReportList = NULL;
        }
    }
    // copy max events irrespective of the profile name
    // Copy the event's recived till now and pass it on to the new profile
    if((Vector_Size(singleProfile->eMarkerList) > 0) && (Vector_Size(profile->eMarkerList) > 0))
    {
        size_t i, j;
        T2Info("check the events from the old profile and forward to the new profile \n");
        for (j = 0; j < Vector_Size(profile->eMarkerList); j++)
        {
            EventMarker *eMarkerNew = NULL;
            eMarkerNew = (EventMarker *)Vector_At(profile->eMarkerList, j);
            T2Debug("Check the New Event : %s index : %zu \n", eMarkerNew->markerName, j);
            for (i = 0; i < Vector_Size(singleProfile->eMarkerList); i++)
            {
                EventMarker *eMarkerCurrent = NULL;
                eMarkerCurrent = (EventMarker *)Vector_At(singleProfile->eMarkerList, i);
                T2Debug("Check the Old Event : %s index : %zu \n", eMarkerCurrent->markerName, i);
                if((strcmp(eMarkerNew->markerName, eMarkerCurrent->markerName) == 0) && eMarkerNew->mType == eMarkerCurrent->mType)
                {
                    T2Debug("Event marker with name : %s type %d is in both profiles copy the value \n", eMarkerCurrent->markerName, eMarkerCurrent->mType);
                    switch(eMarkerNew->mType)
                    {
                    case MTYPE_XCONF_COUNTER:
                        T2Debug("Marker type MTYPE_XCONF_COUNTER and value : %d \n", eMarkerCurrent->u.count);
                        eMarkerNew->u.count = eMarkerCurrent->u.count;
                        break;
                    case MTYPE_XCONF_ABSOLUTE:
                        T2Debug("Marker type MTYPE_XCONF_ABSOLUTE and value : %s \n", eMarkerCurrent->u.markerValue);
                        if (eMarkerCurrent->u.markerValue != NULL)
                        {
                            eMarkerNew->u.markerValue = strdup(eMarkerCurrent->u.markerValue);
                        }
                        break;
                    case MTYPE_XCONF_ACCUMULATE:
                        count =  Vector_Size(eMarkerCurrent->u.accumulatedValues);
                        T2Debug("Marker type MTYPE_XCONF_ACCUMULATE and count : %zu \n", count);
                        if(eMarkerCurrent->u.accumulatedValues != NULL && count > 0)
                        {
                            size_t index = 0;
                            while(index < count)
                            {
                                Vector_PushBack(eMarkerNew->u.accumulatedValues, (void *) Vector_At(eMarkerCurrent->u.accumulatedValues, 0));
                                Vector_RemoveItem(eMarkerCurrent->u.accumulatedValues, (void *) Vector_At(eMarkerCurrent->u.accumulatedValues, 0), NULL);
                                index++;
                            }
                        }
                        break;
                    default:
                        T2Error("The Marker Type with enum %d is not supported \n", eMarkerNew->mType);
                        break;
                    }
                    Vector_RemoveItem(singleProfile->eMarkerList, (void *) eMarkerCurrent, freeEMarker);
                    break;
                }
            }
        }
    }


    if (Vector_Size(singleProfile->gMarkerList) > 0 )
    {
        bool clearSeekMap = true;
        if(isNameEqual)
        {
            clearSeekMap = false;
        }
#ifdef PERSIST_LOG_MON_REF
        else
        {
            removeProfileFromDisk(SEEKFOLDER, singleProfile->name);
        }
#endif
        removeGrepConfig(singleProfile->name, clearSeekMap, true);
    }


    T2Info("removing profile : %s\n", singleProfile->name);
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

