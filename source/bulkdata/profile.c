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
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>

#include "profile.h"
#include "reportprofiles.h"
#include "t2eventreceiver.h"
#include "t2markers.h"
#include "t2log_wrapper.h"
#include "busInterface.h"
#include "curlinterface.h"
#include "rbusmethodinterface.h"
#include "scheduler.h"
#include "persistence.h"
#include "vector.h"
#include "dcautil.h"
#include "t2parser.h"
#include "rbusInterface.h"
#include "legacyutils.h"

#if defined(PRIVACYMODES_CONTROL)
#include "rdkservices_privacyutils.h"
#endif

/*
 * =============================================================================
 * DEADLOCK PREVENTION GUIDELINES
 * =============================================================================
 *
 * Lock Hierarchy (strict ordering to prevent deadlocks):
 * 1. Global locks: plRwLock (profile list), reportLock, triggerConditionQueMutex
 * 2. Per-profile locks: reuseThreadMutex, reportInProgressMutex, eventMutex, triggerCondMutex
 *
 * CRITICAL RULES:
 * - Always acquire locks in hierarchy order (global first, then per-profile)
 * - Never call external functions while holding global locks
 * - Release global locks before acquiring per-profile locks when possible
 * - Use timeouts for lock operations to detect potential deadlocks
 * - Copy data needed for external calls before releasing global locks
 *
 * Per-Profile Lock Ordering:
 * 1. reuseThreadMutex (thread lifecycle)
 * 2. reportInProgressMutex (report state)
 * 3. eventMutex (event processing)
 * 4. triggerCondMutex (trigger conditions)
 *
 * Safe Patterns:
 * ✅ pthread_rwlock_rdlock(&plRwLock) → getProfile() → pthread_rwlock_unlock(&plRwLock) → profile locks
 * ✅ Copy data while holding global lock → Release global lock → External calls
 * ✅ Per-profile locks in defined order only
 *
 * Dangerous Patterns:
 * ❌ External function calls while holding global locks
 * ❌ Acquiring global lock while holding per-profile locks
 * ❌ Per-profile locks acquired in different orders
 * ❌ Long-running operations while holding multiple locks
 *
 * =============================================================================
 */

#define MAX_LEN 256

#ifdef GTEST_ENABLE
#define sendReportOverHTTP __wrap_sendReportOverHTTP
#define sendCachedReportsOverHTTP __wrap_sendCachedReportsOverHTTP
#endif

static bool initialized = false;
static Vector *profileList;
static pthread_rwlock_t plRwLock;
static pthread_mutex_t reportLock;

static pthread_mutex_t triggerConditionQueMutex = PTHREAD_MUTEX_INITIALIZER;
static queue_t *triggerConditionQueue = NULL;

/* DEADLOCK DETECTION: Timeout values for lock operations */
#define GLOBAL_LOCK_TIMEOUT_SEC 5    /* Timeout for global locks */
#define PROFILE_LOCK_TIMEOUT_SEC 3   /* Timeout for per-profile locks */

/**
 * Safe global lock acquisition with timeout for deadlock detection
 * @param lock_type: "read" or "write"  
 * @param func_name: Name of calling function for logging
 * @return: 0 on success, -1 on timeout (potential deadlock)
 */
static int safe_acquire_global_lock(const char* lock_type, const char* func_name)\n{
    struct timespec timeout;
    clock_gettime(CLOCK_REALTIME, &timeout);
    timeout.tv_sec += GLOBAL_LOCK_TIMEOUT_SEC;

    int result;
    if (strcmp(lock_type, "read") == 0)
    {
        result = pthread_rwlock_timedrdlock(&plRwLock, &timeout);
    }
    else
    {
        result = pthread_rwlock_timedwrlock(&plRwLock, &timeout);
    }

    if (result == ETIMEDOUT)
    {
        T2Error("DEADLOCK DETECTED: Global %s lock timeout in %s after %d seconds\\n", 
                lock_type, func_name, GLOBAL_LOCK_TIMEOUT_SEC);
        return -1;
    }
    else if (result != 0)
    {
        T2Error("Global %s lock failed in %s: %s\\n", lock_type, func_name, strerror(result));
        return -1;
    }
    
    return 0;
}

/**
 * Safe per-profile lock acquisition with timeout
 * @param mutex: Profile mutex to acquire
 * @param mutex_name: Name for logging
 * @param func_name: Name of calling function
 * @return: 0 on success, -1 on timeout
 */
static int safe_acquire_profile_lock(pthread_mutex_t* mutex, const char* mutex_name, const char* func_name)
{
    struct timespec timeout;
    clock_gettime(CLOCK_REALTIME, &timeout);
    timeout.tv_sec += PROFILE_LOCK_TIMEOUT_SEC;

    int result = pthread_mutex_timedlock(mutex, &timeout);
    if (result == ETIMEDOUT)
    {
        T2Error("DEADLOCK DETECTED: Profile %s lock timeout in %s after %d seconds\\n", 
                mutex_name, func_name, PROFILE_LOCK_TIMEOUT_SEC);
        return -1;
    }
    else if (result != 0)
    {
        T2Error("Profile %s lock failed in %s: %s\\n", mutex_name, func_name, strerror(result));
        return -1;
    }
    
    return 0;
}

typedef struct __triggerConditionObj__
{
    char referenceName[MAX_LEN];
    char referenceValue[MAX_LEN];
} triggerConditionObj ;

static void freeRequestURIparam(void *data)
{
    if(data != NULL)
    {
        HTTPReqParam *hparam = (HTTPReqParam *)data;
        if(hparam->HttpName)
        {
            free(hparam->HttpName);
        }
        if(hparam->HttpRef)
        {
            free(hparam->HttpRef);
        }
        if(hparam->HttpValue)
        {
            free(hparam->HttpValue);
        }
        free(hparam);
    }
}

static void freeReportProfileConfig(void *data)
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

static void freeProfile(void *data)
{
    T2Debug("%s ++in \n", __FUNCTION__);
    if(data != NULL)
    {
        Profile *profile = (Profile *)data;
        if(profile->name)
        {
            free(profile->name);
        }
        if(profile->hash)
        {
            free(profile->hash);
        }
        if(profile->protocol)
        {
            free(profile->protocol);
        }
        if(profile->encodingType)
        {
            free(profile->encodingType);
        }
        if(profile->RootName)
        {
            free(profile->RootName);
        }
        if(profile->Description)
        {
            free(profile->Description);
        }
        if(profile->version)
        {
            free(profile->version);
        }
        if(profile->jsonEncoding)
        {
            free(profile->jsonEncoding);
        }
        if(profile->timeRef)
        {
            free(profile->timeRef);
        }
        if(profile->t2HTTPDest)
        {
            free(profile->t2HTTPDest->URL);
            if(profile->t2HTTPDest->RequestURIparamList)
            {
                Vector_Destroy(profile->t2HTTPDest->RequestURIparamList, freeRequestURIparam);
            }
            free(profile->t2HTTPDest);
        }
        if(profile->t2RBUSDest)
        {
            if(profile->t2RBUSDest->rbusMethodName)
            {
                memset(profile->t2RBUSDest->rbusMethodName, 0, strlen(profile->t2RBUSDest->rbusMethodName));
                free(profile->t2RBUSDest->rbusMethodName);
                profile->t2RBUSDest->rbusMethodName = NULL ;
            }
            if(profile->t2RBUSDest->rbusMethodParamList)
            {
                // TBD determine whether data is simple string before passing free as cleanup function
                Vector_Destroy(profile->t2RBUSDest->rbusMethodParamList, free);
            }
            free(profile->t2RBUSDest);
        }
        if(profile->eMarkerList)
        {
            Vector_Destroy(profile->eMarkerList, freeEMarker);
        }
        if(profile->gMarkerList)
        {
            Vector_Destroy(profile->gMarkerList, freeGMarker);
        }
        if(profile->topMarkerList)
        {
            Vector_Destroy(profile->topMarkerList, freeGMarker);
        }
        if(profile->paramList)
        {
            Vector_Destroy(profile->paramList, freeParam);
        }
        if (profile->staticParamList)
        {
            Vector_Destroy(profile->staticParamList, freeStaticParam);
        }
        if(profile->triggerConditionList)
        {
            Vector_Destroy(profile->triggerConditionList, freeTriggerCondition);
        }

        if(profile->cachedReportList)
        {
            Vector_Destroy(profile->cachedReportList, free);
            profile->cachedReportList = NULL;
        }
        if(profile->jsonReportObj)
        {
            cJSON_Delete(profile->jsonReportObj);
            profile->jsonReportObj = NULL;
        }
        free(profile);
    }
    T2Debug("%s ++out \n", __FUNCTION__);
}

static T2ERROR getProfile(const char *profileName, Profile **profile)
{
    size_t profileIndex = 0;
    Profile *tempProfile = NULL;
    T2Debug("%s ++in\n", __FUNCTION__);
    if(profileName == NULL)
    {
        T2Error("profileName is null\n");
        return T2ERROR_FAILURE;
    }
    for(; profileIndex < Vector_Size(profileList); profileIndex++)
    {
        tempProfile = (Profile *)Vector_At(profileList, profileIndex);
        if(strcmp(tempProfile->name, profileName) == 0)
        {
            *profile = tempProfile;
            T2Debug("%s --out\n", __FUNCTION__);
            return T2ERROR_SUCCESS;
        }
    }
    T2Error("Profile with Name : %s not found\n", profileName);
    return T2ERROR_PROFILE_NOT_FOUND;
}

static T2ERROR initJSONReportProfile(cJSON** jsonObj, cJSON **valArray, char *rootname)
{
    *jsonObj = cJSON_CreateObject();
    if(*jsonObj == NULL)
    {
        T2Error("Failed to create cJSON object\n");
        return T2ERROR_FAILURE;
    }
    cJSON_AddItemToObject(*jsonObj, rootname, *valArray = cJSON_CreateArray());


    return T2ERROR_SUCCESS;
}

static void calculateMaxUploadLatency(Profile *profile, time_t *maxuploadinmilliSec, time_t *maxuploadinSec)
{
    unsigned int random_value = 0;
    // Ensure maxUploadLatency > 1 to avoid modulo by zero
    if(profile->maxUploadLatency > 1)
    {
        int urandom = -1;
        urandom = open("/dev/urandom", O_RDONLY);
        if(urandom != -1 && read(urandom, &random_value, sizeof(random_value)) == sizeof(random_value))
        {
            *maxuploadinmilliSec = random_value % (profile->maxUploadLatency - 1);
            if(close(urandom) == -1)
            {
                T2Error("Failed to close /dev/urandom: %s\n", strerror(errno));
            }
        }
        else
        {
            if(urandom != -1)
            {
                if(close(urandom) == -1)
                {
                    T2Error("Failed to close /dev/urandom: %s\n", strerror(errno));
                }
            }
            *maxuploadinmilliSec = (unsigned int)(time(0) % (profile->maxUploadLatency - 1));
        }
    }
    else
    {
        // If maxUploadLatency is 1, set maxuploadinmilliSec to 0
        *maxuploadinmilliSec = 0;
    }
    *maxuploadinSec = (*maxuploadinmilliSec + 1) / 1000;
}

T2ERROR profileWithNameExists(const char *profileName, bool *bProfileExists)
{
    size_t profileIndex = 0;
    Profile *tempProfile = NULL;
    T2Debug("%s ++in\n", __FUNCTION__);
    if(!initialized)
    {
        T2Error("profile list is not initialized yet, ignoring\n");
        return T2ERROR_FAILURE;
    }
    if(profileName == NULL)
    {
        T2Error("profileName is null\n");
        *bProfileExists = false;
        return T2ERROR_FAILURE;
    }
    pthread_rwlock_rdlock(&plRwLock);
    for(; profileIndex < Vector_Size(profileList); profileIndex++)
    {
        tempProfile = (Profile *)Vector_At(profileList, profileIndex);
        if(strcmp(tempProfile->name, profileName) == 0)
        {
            *bProfileExists = true;
            pthread_rwlock_unlock(&plRwLock);
            return T2ERROR_SUCCESS;
        }
    }
    *bProfileExists = false;
    pthread_rwlock_unlock(&plRwLock);
    T2Error("Profile with Name : %s not found\n", profileName);
    return T2ERROR_PROFILE_NOT_FOUND;
}

void getMarkerCompRbusSub(bool subscription)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    Vector* eventMarkerListForComponent = NULL;
    getComponentMarkerList(T2REPORTCOMPONENT, (void**)&eventMarkerListForComponent);
    int length = Vector_Size(eventMarkerListForComponent);
    int i;
    if(length > 0)
    {
        for(i = 0; i < length; ++i )
        {
            char* markerName = (char *) Vector_At(eventMarkerListForComponent, i);
            if(markerName)
            {
                int ret = T2RbusReportEventConsumer(markerName, subscription);
                T2Debug("%d T2RbusEventReg with name = %s: subscription = %s ret %d \n", i, markerName, (subscription ? "Subscribe" : "Un-Subscribe"), ret);
            }
            else
            {
                T2Error("Error while retrieving Marker Name at index : %d \n", i);
            }
        }
        if(eventMarkerListForComponent != NULL)
        {
            Vector_Destroy(eventMarkerListForComponent, free);
        }
    }
    //CID 255490: Resource leak (RESOURCE_LEAK)
    else
    {
        if(eventMarkerListForComponent != NULL)
        {
            Vector_Destroy(eventMarkerListForComponent, free);
        }
    }
    T2Debug("%s --out\n", __FUNCTION__);
}

static void* CollectAndReport(void* data)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if(data == NULL)
    {
        T2Error("data passed is NULL can't identify the profile, existing from CollectAndReport\n");
        return NULL;
    }
    Profile* profile = (Profile *)data;
    pthread_mutex_init(&profile->reuseThreadMutex, NULL);
    pthread_cond_init(&profile->reuseThread, NULL);
    pthread_mutex_lock(&profile->reuseThreadMutex);
    profile->restartRequested = false;
    profile->threadExists = true;
    //GrepSeekProfile *GPF = profile->GrepSeekProfle;
    do
    {
        T2Info("%s while Loop -- START \n", __FUNCTION__);
        pthread_mutex_lock(&profile->reportInProgressMutex);
        profile->reportInProgress = true;
        pthread_cond_signal(&profile->reportInProgressCond);
        pthread_mutex_unlock(&profile->reportInProgressMutex);
        pthread_mutex_unlock(&profile->reuseThreadMutex);

        int count = profile->grepSeekProfile->execCounter;

        Vector *profileParamVals = NULL;
        cJSON *valArray = NULL;
        char* jsonReport = NULL;
        cJSON *triggercondition = NULL;
        time_t maxuploadinSec = 0;
        time_t maxuploadinmilliSec = 0;
        int n = 0;
        struct timespec startTime;
        struct timespec endTime;
        struct timespec elapsedTime;
        char* customLogPath = NULL;
        int clockReturn = 0;

        T2ERROR ret = T2ERROR_FAILURE;
        if( profile->name == NULL || profile->encodingType == NULL || profile->protocol == NULL )
        {
            T2Error("Incomplete profile parameters\n");
            if(profile->triggerReportOnCondition)
            {
                T2Info(" Unlock trigger condition mutex and set report on condition to false \n");
                profile->triggerReportOnCondition = false ;
                pthread_mutex_unlock(&profile->triggerCondMutex);
            }
            else
            {
                T2Debug(" profile->triggerReportOnCondition is not set \n");
            }
            pthread_mutex_lock(&profile->reuseThreadMutex);
            pthread_mutex_lock(&profile->reportInProgressMutex);
            profile->reportInProgress = false;
            pthread_cond_signal(&profile->reportInProgressCond);
            pthread_mutex_unlock(&profile->reportInProgressMutex);
            pthread_mutex_unlock(&profile->reuseThreadMutex);
            //return NULL;
            goto reportThreadEnd;
        }

        T2Info("%s ++in profileName : %s\n", __FUNCTION__, profile->name);


        clockReturn = clock_gettime(CLOCK_MONOTONIC, &startTime);
        if( !strcmp(profile->encodingType, "JSON") || !strcmp(profile->encodingType, "MessagePack"))
        {
            JSONEncoding *jsonEncoding = profile->jsonEncoding;
            if (jsonEncoding->reportFormat != JSONRF_KEYVALUEPAIR)
            {
                //TODO: Support 'ObjectHierarchy' format in RDKB-26154.
                T2Error("Only JSON name-value pair format is supported \n");
                if(profile->triggerReportOnCondition)
                {
                    T2Info(" Unlock trigger condition mutex and set report on condition to false \n");
                    profile->triggerReportOnCondition = false ;
                    pthread_mutex_unlock(&profile->triggerCondMutex);
                }
                else
                {
                    T2Debug(" profile->triggerReportOnCondition is not set \n");
                }
                pthread_mutex_lock(&profile->reuseThreadMutex);
                pthread_mutex_lock(&profile->reportInProgressMutex);
                profile->reportInProgress = false;
                pthread_cond_signal(&profile->reportInProgressCond);
                pthread_mutex_unlock(&profile->reportInProgressMutex);
                pthread_mutex_unlock(&profile->reuseThreadMutex);
                //return NULL;
                goto reportThreadEnd;
            }
            // pthread_mutex_lock(&profile->triggerCondMutex);
            if(profile->triggerReportOnCondition && (profile->jsonReportObj != NULL))
            {
                triggercondition = profile->jsonReportObj;
                profile->jsonReportObj = NULL;
            }
            if(T2ERROR_SUCCESS != initJSONReportProfile(&profile->jsonReportObj, &valArray, profile->RootName))
            {
                T2Error("Failed to initialize JSON Report\n");
                pthread_mutex_lock(&profile->reuseThreadMutex);
                pthread_mutex_lock(&profile->reportInProgressMutex);
                profile->reportInProgress = false;
                pthread_cond_signal(&profile->reportInProgressCond);
                pthread_mutex_unlock(&profile->reportInProgressMutex);
                pthread_mutex_unlock(&profile->reuseThreadMutex);
                //pthread_mutex_unlock(&profile->triggerCondMutex);
                if(profile->triggerReportOnCondition)
                {
                    T2Info(" Unlock trigger condition mutex and set report on condition to false \n");
                    profile->triggerReportOnCondition = false;
                    pthread_mutex_unlock(&profile->triggerCondMutex);
                    if(profile->callBackOnReportGenerationComplete)
                    {
                        profile->callBackOnReportGenerationComplete(profile->name);
                    }
                }
                //return NULL;
                goto reportThreadEnd;
            }
            else
            {
#ifdef PERSIST_LOG_MON_REF
                if(profile->checkPreviousSeek)
                {
                    cJSON *arrayItem = NULL;
                    arrayItem = cJSON_CreateObject();
                    cJSON_AddStringToObject(arrayItem, PREVIOUS_LOG, PREVIOUS_LOGS_VAL);
                    cJSON_AddItemToArray(valArray, arrayItem);
                    customLogPath = PREVIOUS_LOGS_PATH;
                    profile->bClearSeekMap = true;
                    T2Debug("Adding Previous Logs Header to JSON report\n");
                }
#endif
                if(profile->staticParamList != NULL && Vector_Size(profile->staticParamList) > 0)
                {
                    T2Debug(" Adding static Parameter Values to Json report\n");
                    encodeStaticParamsInJSON(valArray, profile->staticParamList);
                }
                if(profile->paramList != NULL && Vector_Size(profile->paramList) > 0)
                {
                    T2Debug("Fetching TR-181 Object/Parameter Values\n");
                    profileParamVals = getProfileParameterValues(profile->paramList, count);
                    if(profileParamVals != NULL)
                    {
                        encodeParamResultInJSON(valArray, profile->paramList, profileParamVals);
                    }
                    Vector_Destroy(profileParamVals, freeProfileValues);
                }
                if(profile->topMarkerList != NULL && Vector_Size(profile->topMarkerList) > 0)
                {
                    processTopPattern(profile->name, profile->topMarkerList, 0);
                    encodeTopResultInJSON(valArray, profile->topMarkerList);
                }
                if(profile->gMarkerList != NULL && Vector_Size(profile->gMarkerList) > 0)
                {
                    getGrepResults(&(profile->grepSeekProfile), profile->gMarkerList, profile->bClearSeekMap, false, customLogPath); // Passing 4th argument as false so that it doesn't check rotated logs for the first reporting after bootup for multiprofiles.
                    encodeGrepResultInJSON(valArray, profile->gMarkerList);
                }
                if(profile->eMarkerList != NULL && Vector_Size(profile->eMarkerList) > 0)
                {
                    pthread_mutex_lock(&profile->eventMutex);
                    encodeEventMarkersInJSON(valArray, profile->eMarkerList);
                    pthread_mutex_unlock(&profile->eventMutex);
                }
                if(profile->triggerReportOnCondition && (triggercondition != NULL))
                {
                    cJSON_AddItemToArray(valArray, triggercondition);
                }
                ret = prepareJSONReport(profile->jsonReportObj, &jsonReport);
                destroyJSONReport(profile->jsonReportObj);
                profile->jsonReportObj = NULL;
                if(ret != T2ERROR_SUCCESS)
                {
                    T2Error("Unable to generate report for : %s\n", profile->name);
                    pthread_mutex_lock(&profile->reuseThreadMutex);
                    pthread_mutex_lock(&profile->reportInProgressMutex);
                    profile->reportInProgress = false;
                    pthread_cond_signal(&profile->reportInProgressCond);
                    pthread_mutex_unlock(&profile->reportInProgressMutex);
                    pthread_mutex_unlock(&profile->reuseThreadMutex);
                    if(profile->triggerReportOnCondition)
                    {
                        profile->triggerReportOnCondition = false ;
                        pthread_mutex_unlock(&profile->triggerCondMutex);

                        if(profile->callBackOnReportGenerationComplete)
                        {
                            profile->callBackOnReportGenerationComplete(profile->name);
                        }
                    }
                    else
                    {
                        T2Debug(" profile->triggerReportOnCondition is not set \n");
                    }
                    //return NULL;
                    goto reportThreadEnd;
                }
#ifdef PERSIST_LOG_MON_REF
                if(profile->saveSeekConfig)
                {
                    saveSeekConfigtoFile(profile->name, profile->grepSeekProfile);
                }
                if(profile->checkPreviousSeek)
                {
                    T2Info("Previous Logs report is sent clear the previousSeek flag\n");
                    profile->checkPreviousSeek = false;
                    customLogPath = NULL;
                    profile->bClearSeekMap = false;
                }
#endif
                long size = strlen(jsonReport);
                T2Info("cJSON Report = %s\n", jsonReport);
                cJSON *root = cJSON_Parse(jsonReport);
                if(root != NULL)
                {
                    cJSON *array = cJSON_GetObjectItem(root, profile->RootName);
                    if(cJSON_GetArraySize(array) == 0)
                    {
                        T2Warning("Array size of Report is %d. Report is empty. Cannot send empty report\n", cJSON_GetArraySize(array));
                        pthread_mutex_lock(&profile->reuseThreadMutex);
                        pthread_mutex_lock(&profile->reportInProgressMutex);
                        profile->reportInProgress = false;
                        pthread_cond_signal(&profile->reportInProgressCond);
                        pthread_mutex_unlock(&profile->reportInProgressMutex);
                        pthread_mutex_unlock(&profile->reuseThreadMutex);
                        if(profile->triggerReportOnCondition)
                        {
                            T2Info(" Unlock trigger condition mutex and set report on condition to false \n");
                            profile->triggerReportOnCondition = false ;
                            pthread_mutex_unlock(&profile->triggerCondMutex);
                            if(profile->callBackOnReportGenerationComplete)
                            {
                                profile->callBackOnReportGenerationComplete(profile->name);
                            }
                        }
                        else
                        {
                            T2Debug(" profile->triggerReportOnCondition is not set \n");
                        }
                        cJSON_Delete(root);
                        //return NULL;
                        goto reportThreadEnd;
                    }
                    cJSON_Delete(root);
                }

                T2Info("Report Size = %ld\n", size);
                if(size > DEFAULT_MAX_REPORT_SIZE)
                {
                    T2Warning("Report size is exceeding the max limit : %d\n", DEFAULT_MAX_REPORT_SIZE);
                }
                if(profile->maxUploadLatency > 0)
                {
                    pthread_mutex_lock(&profile->reuseThreadMutex);
                    memset(&profile->maxlatencyTime, 0, sizeof(struct timespec));
                    memset(&profile->currentTime, 0, sizeof(struct timespec));
                    clock_gettime(CLOCK_REALTIME, &profile->currentTime);
                    profile->maxlatencyTime.tv_sec = profile->currentTime.tv_sec;
                    pthread_mutex_unlock(&profile->reuseThreadMutex);
                    pthread_mutex_init(&profile->reportMutex, NULL);
                    pthread_cond_init(&profile->reportcond, NULL);
                    calculateMaxUploadLatency(profile, &maxuploadinmilliSec, &maxuploadinSec);
                }
                if( strcmp(profile->protocol, "HTTP") == 0 || strcmp(profile->protocol, "RBUS_METHOD") == 0 )
                {
                    char *httpUrl = NULL ;
                    if ( strcmp(profile->protocol, "HTTP") == 0 )
                    {
                        httpUrl = prepareHttpUrl(profile->t2HTTPDest); /* Append URL with http properties */
                        if(profile->maxUploadLatency > 0)
                        {
                            T2Info("waiting for %ld sec of macUploadLatency\n", (long) maxuploadinSec);
                            struct timespec timeout;
                            pthread_mutex_lock(&profile->reuseThreadMutex);
                            profile->maxlatencyTime.tv_sec += maxuploadinSec;
                            timeout = profile->maxlatencyTime;
                            pthread_mutex_unlock(&profile->reuseThreadMutex);
                            pthread_mutex_lock(&profile->reportMutex);
                            while (profile->enable && n != ETIMEDOUT)
                            {
                                n = pthread_cond_timedwait(&profile->reportcond, &profile->reportMutex, &timeout);
                            }
                            if(n == ETIMEDOUT)
                            {
                                T2Info("TIMEOUT for maxUploadLatency of profile %s\n", profile->name);
                                ret = sendReportOverHTTP(httpUrl, jsonReport);
                            }
                            else if(n == 0)
                            {
                                T2Info("Profile : %s signaled before timeout, skipping report upload\n", profile->name);
                            }
                            else
                            {
                                T2Error("Profile : %s pthread_cond_timedwait ERROR!!!\n", profile->name);
                                pthread_mutex_unlock(&profile->reportMutex);
                                pthread_cond_destroy(&profile->reportcond);
                                if(httpUrl)
                                {
                                    free(httpUrl);
                                    httpUrl = NULL;
                                }
                                pthread_mutex_lock(&profile->reuseThreadMutex);
                                pthread_mutex_lock(&profile->reportInProgressMutex);
                                profile->reportInProgress = false;
                                pthread_cond_signal(&profile->reportInProgressCond);
                                pthread_mutex_unlock(&profile->reportInProgressMutex);
                                pthread_mutex_unlock(&profile->reuseThreadMutex);
                                if(profile->triggerReportOnCondition)
                                {
                                    T2Info(" Unlock trigger condition mutex and set report on condition to false \n");
                                    profile->triggerReportOnCondition = false ;
                                    pthread_mutex_unlock(&profile->triggerCondMutex);

                                    if(profile->callBackOnReportGenerationComplete != NULL)
                                    {
                                        T2Debug("Calling callback function profile->callBackOnReportGenerationComplete \n");
                                        profile->callBackOnReportGenerationComplete(profile->name);
                                    }
                                }
                                else
                                {
                                    T2Debug(" profile->triggerReportOnCondition is not set \n");
                                }
                                //return NULL;
                                goto reportThreadEnd;
                            }
                            pthread_mutex_unlock(&profile->reportMutex);
                            pthread_cond_destroy(&profile->reportcond);
                            pthread_mutex_destroy(&profile->reportMutex);
                        }
                        else
                        {
                            ret = sendReportOverHTTP(httpUrl, jsonReport);
                        }
                    }
                    else
                    {
                        if(profile->maxUploadLatency > 0 )
                        {
                            T2Info("waiting for %ld sec of macUploadLatency\n", (long) maxuploadinSec);
                            struct timespec timeout;
                            pthread_mutex_lock(&profile->reuseThreadMutex);
                            profile->maxlatencyTime.tv_sec += maxuploadinSec;
                            timeout = profile->maxlatencyTime;
                            pthread_mutex_unlock(&profile->reuseThreadMutex);
                            pthread_mutex_lock(&profile->reportMutex);
                            while (profile->enable && n != ETIMEDOUT)
                            {
                                n = pthread_cond_timedwait(&profile->reportcond, &profile->reportMutex, &timeout);
                            }
                            if(n == ETIMEDOUT)
                            {
                                T2Info("TIMEOUT for maxUploadLatency of profile %s\n", profile->name);
                                ret = sendReportsOverRBUSMethod(profile->t2RBUSDest->rbusMethodName, profile->t2RBUSDest->rbusMethodParamList, jsonReport);
                            }
                            else if(n == 0)
                            {
                                T2Info("Profile : %s signaled before timeout, skipping report upload\n", profile->name);
                            }
                            else
                            {
                                T2Error("Profile : %s pthread_cond_timedwait ERROR!!!\n", profile->name);
                                pthread_mutex_unlock(&profile->reportMutex);
                                pthread_cond_destroy(&profile->reportcond);
                                pthread_mutex_lock(&profile->reuseThreadMutex);
                                pthread_mutex_lock(&profile->reportInProgressMutex);
                                profile->reportInProgress = false;
                                pthread_cond_signal(&profile->reportInProgressCond);
                                pthread_mutex_unlock(&profile->reportInProgressMutex);
                                pthread_mutex_unlock(&profile->reuseThreadMutex);
                                if(profile->triggerReportOnCondition)
                                {
                                    T2Info(" Unlock trigger condition mutex and set report on condition to false \n");
                                    profile->triggerReportOnCondition = false ;
                                    pthread_mutex_unlock(&profile->triggerCondMutex);

                                    if(profile->callBackOnReportGenerationComplete)
                                    {
                                        T2Debug("Calling callback function profile->callBackOnReportGenerationComplete \n");
                                        profile->callBackOnReportGenerationComplete(profile->name);
                                    }
                                }
                                else
                                {
                                    T2Debug(" profile->triggerReportOnCondition is not set \n");
                                }
                                //return NULL;
                                goto reportThreadEnd;
                            }
                            pthread_mutex_unlock(&profile->reportMutex);
                            pthread_cond_destroy(&profile->reportcond);
                        }
                        else
                        {
                            ret = sendReportsOverRBUSMethod(profile->t2RBUSDest->rbusMethodName, profile->t2RBUSDest->rbusMethodParamList, jsonReport);
                        }
                    }
                    if((ret == T2ERROR_FAILURE && strcmp(profile->protocol, "HTTP") == 0) || ret == T2ERROR_NO_RBUS_METHOD_PROVIDER)
                    {
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
                        // Before caching the report, add "REPORT_TYPE": "CACHED"
                        tagReportAsCached(&jsonReport);
                        Vector_PushBack(profile->cachedReportList, jsonReport);

                        T2Info("Report Cached, No. of reportes cached = %lu\n", (unsigned long )Vector_Size(profile->cachedReportList));
                        // Save messages from profile->cachedReportList to a file in persistent location .
                        saveCachedReportToPersistenceFolder(profile->name, profile->cachedReportList);

                        if(strcmp(profile->protocol, "RBUS_METHOD") == 0)
                        {
                            profile->SendErr++;
                            if(profile->SendErr > 3 && !(rbusCheckMethodExists(profile->t2RBUSDest->rbusMethodName)))   //to delete the profile in the next CollectAndReport or triggercondition
                            {
                                T2Debug("RBUS_METHOD doesn't exists after 3 retries\n");
                                pthread_mutex_lock(&profile->reuseThreadMutex);
                                pthread_mutex_lock(&profile->reportInProgressMutex);
                                profile->reportInProgress = false;
                                pthread_cond_signal(&profile->reportInProgressCond);
                                pthread_mutex_unlock(&profile->reportInProgressMutex);
                                pthread_mutex_unlock(&profile->reuseThreadMutex);
                                if(profile->triggerReportOnCondition)
                                {
                                    profile->triggerReportOnCondition = false ;
                                    pthread_mutex_unlock(&profile->triggerCondMutex);
                                    if(profile->callBackOnReportGenerationComplete)
                                    {
                                        profile->callBackOnReportGenerationComplete(profile->name);
                                    }
                                }
                                else
                                {
                                    T2Debug(" profile->triggerReportOnCondition is not set \n");
                                }
                                T2Error("ERROR: no method provider; profile will be deleted: %s %s\n", profile->name,
                                        profile->t2RBUSDest->rbusMethodName);
                                if(T2ERROR_SUCCESS != deleteProfile(profile->name))
                                {
                                    T2Error("Failed to delete profile after RBUS_METHOD failures: %s\n", profile->name);
                                    T2Info("%s --out\n", __FUNCTION__);
                                    //return NULL;
                                    goto reportThreadEnd;
                                }
                                T2Info("%s --out\n", __FUNCTION__);
                                //return NULL;
                                goto reportThreadEnd;
                            }
                        }
                    }
                    else if(profile->cachedReportList != NULL && Vector_Size(profile->cachedReportList) > 0)
                    {
                        T2Info("Trying to send  %lu cached reports\n", (unsigned long )Vector_Size(profile->cachedReportList));
                        if(strcmp(profile->protocol, "HTTP") == 0)
                        {
                            ret = sendCachedReportsOverHTTP(httpUrl, profile->cachedReportList);
                        }
                        else
                        {
                            ret = sendCachedReportsOverRBUSMethod(profile->t2RBUSDest->rbusMethodName, profile->t2RBUSDest->rbusMethodParamList,
                                                                  profile->cachedReportList);
                        }

                        if(ret == T2ERROR_SUCCESS)
                        {
                            removeProfileFromDisk(CACHED_MESSAGE_PATH, profile->name);
                        }
                    }
                    if(httpUrl)
                    {
                        free(httpUrl);
                        httpUrl = NULL;
                    }
                }
                else
                {
                    T2Error("Unsupported report send protocol : %s\n", profile->protocol);
                }
            }
        }
        else
        {
            T2Error("Unsupported encoding format : %s\n", profile->encodingType);
        }
        clockReturn |= clock_gettime(CLOCK_MONOTONIC, &endTime);
        if(clockReturn)
        {
            T2Warning("Failed to get time from clock_gettime()");
        }
        else
        {
            getLapsedTime(&elapsedTime, &endTime, &startTime);
            T2Info("Elapsed Time for : %s = %lu.%lu (Sec.NanoSec)\n", profile->name, (unsigned long )elapsedTime.tv_sec, elapsedTime.tv_nsec);
        }
        if(ret == T2ERROR_SUCCESS && jsonReport)
        {
            free(jsonReport);
            jsonReport = NULL;
        }

        pthread_mutex_lock(&profile->reuseThreadMutex);
        pthread_mutex_lock(&profile->reportInProgressMutex);
        profile->reportInProgress = false;
        pthread_cond_signal(&profile->reportInProgressCond);
        pthread_mutex_unlock(&profile->reportInProgressMutex);
        pthread_mutex_unlock(&profile->reuseThreadMutex);
        if(profile->triggerReportOnCondition)
        {
            T2Info(" Unlock trigger condition mutex and set report on condition to false \n");
            profile->triggerReportOnCondition = false ;
            pthread_mutex_unlock(&profile->triggerCondMutex);

            if(profile->callBackOnReportGenerationComplete)
            {
                T2Debug("Calling callback function profile->callBackOnReportGenerationComplete \n");
                profile->callBackOnReportGenerationComplete(profile->name);
            }
        }
        else
        {
            T2Debug(" profile->triggerReportOnCondition is not set \n");
        }
reportThreadEnd :
        T2Info("%s while Loop -- END; wait for restart event\n", __FUNCTION__);
        T2Info("%s --out\n", __FUNCTION__);
        pthread_mutex_lock(&profile->reuseThreadMutex);
        while(profile->enable && !profile->restartRequested)
        {
            pthread_cond_wait(&profile->reuseThread, &profile->reuseThreadMutex);
        }
        profile->restartRequested = false;
    }
    while(profile->enable);
    pthread_mutex_unlock(&profile->reuseThreadMutex);
    T2Info("%s --out Exiting collect and report Thread\n", __FUNCTION__);
    pthread_mutex_lock(&profile->reuseThreadMutex);
    pthread_mutex_lock(&profile->reportInProgressMutex);
    profile->reportInProgress = false;
    pthread_cond_signal(&profile->reportInProgressCond);
    pthread_mutex_unlock(&profile->reportInProgressMutex);
    pthread_mutex_unlock(&profile->reuseThreadMutex);
    pthread_mutex_lock(&profile->reuseThreadMutex);
    profile->threadExists = false;
    pthread_mutex_unlock(&profile->reuseThreadMutex);
    pthread_mutex_destroy(&profile->reuseThreadMutex);
    pthread_cond_destroy(&profile->reuseThread);
    return NULL;
}

void NotifyTimeout(const char* profileName, bool isClearSeekMap)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    
    /* DEADLOCK PREVENTION: Use safe global lock acquisition */
    if (safe_acquire_global_lock("read", __FUNCTION__) != 0)
    {
        T2Error("Failed to acquire global read lock in %s\n", __FUNCTION__);
        return;
    }

    Profile *profile = NULL;
    if(T2ERROR_SUCCESS != getProfile(profileName, &profile))
    {
        T2Error("Profile : %s not found\n", profileName);
        pthread_rwlock_unlock(&plRwLock);
        return ;
    }

    /* DEADLOCK PREVENTION: Release global lock before acquiring per-profile locks */
    pthread_rwlock_unlock(&plRwLock);
    
    T2Info("%s: profile %s is in %s state\n", __FUNCTION__, profileName, profile->enable ? "Enabled" : "Disabled");
    
    /* Safe per-profile lock acquisition */
    if (safe_acquire_profile_lock(&profile->reportInProgressMutex, "reportInProgressMutex", __FUNCTION__) != 0)
    {
        return;
    }
    if(profile->enable && !profile->reportInProgress)
    {
        profile->reportInProgress = true;
        profile->bClearSeekMap = isClearSeekMap;
        pthread_mutex_unlock(&profile->reportInProgressMutex);

        /* DEADLOCK PREVENTION: Proper lock ordering for thread synchronization */
        if (safe_acquire_profile_lock(&profile->reuseThreadMutex, "reuseThreadMutex", __FUNCTION__) != 0)
        {
            pthread_mutex_unlock(&profile->reportInProgressMutex);
            return;
        }
        if (profile->threadExists)
        {
            T2Info("Signal Thread To restart\n");
            profile->restartRequested = true;
            pthread_cond_signal(&profile->reuseThread);
            pthread_mutex_unlock(&profile->reuseThreadMutex);
        }
        else
        {
            pthread_mutex_unlock(&profile->reuseThreadMutex);
            pthread_create(&profile->reportThread, NULL, CollectAndReport, (void*)profile);
        }
    }
    else
    {
        T2Warning("Either profile is disabled or report generation still in progress - ignoring the request\n");
        pthread_mutex_unlock(&profile->reportInProgressMutex);
    }
    T2Debug("%s --out\n", __FUNCTION__);
}

T2ERROR Profile_storeMarkerEvent(const char *profileName, T2Event *eventInfo)
{
    T2Debug("%s ++in\n", __FUNCTION__);

    /* DEADLOCK PREVENTION: Use safe global lock acquisition */
    if (safe_acquire_global_lock("read", __FUNCTION__) != 0)
    {
        return T2ERROR_FAILURE;
    }
    
    Profile *profile = NULL;
    if(T2ERROR_SUCCESS != getProfile(profileName, &profile))
    {
        T2Error("Profile : %s not found\n", profileName);
        pthread_rwlock_unlock(&plRwLock);
        return T2ERROR_FAILURE;
    }
    
    /* DEADLOCK PREVENTION: Release global lock before per-profile operations */
    pthread_rwlock_unlock(&plRwLock);
    if(!profile->enable)
    {
        T2Warning("Profile : %s is disabled, ignoring the event\n", profileName);
        return T2ERROR_FAILURE;
    }
    size_t eventIndex = 0;
    EventMarker *lookupEvent = NULL;
    for(; eventIndex < Vector_Size(profile->eMarkerList); eventIndex++)
    {
        EventMarker *tempEventMarker = (EventMarker *)Vector_At(profile->eMarkerList, eventIndex);
        if(!strcmp(tempEventMarker->markerName, eventInfo->name))
        {
            lookupEvent = tempEventMarker;
            break;
        }
    }
    int arraySize = 0;
    if(lookupEvent != NULL)
    {
        char buf[256] = {'\0'};
        char timebuf[256] = {'\0'};
        time_t timestamp = 0;
        pthread_mutex_lock(&profile->eventMutex);
        switch(lookupEvent->mType)
        {
        case MTYPE_COUNTER:
            lookupEvent->u.count++;
            T2Debug("Increment marker count to : %d\n", lookupEvent->u.count);
            if(lookupEvent->reportTimestampParam == REPORTTIMESTAMP_UNIXEPOCH)
            {
                if(lookupEvent->timestamp)
                {
                    free(lookupEvent->timestamp);
                    lookupEvent->timestamp = NULL;
                }

                timestamp = time(NULL);
                if(lookupEvent->markerName_CT == NULL)
                {
                    if(lookupEvent->alias != NULL)
                    {
                        snprintf(buf, MAX_LEN, "%s_CT", lookupEvent->alias);
                    }
                    else
                    {
                        snprintf(buf, MAX_LEN, "%s_CT", lookupEvent->markerName);
                    }
                    lookupEvent->markerName_CT = strdup(buf);
                }
                snprintf(timebuf, MAX_LEN, "%ld", (long) timestamp);
                lookupEvent->timestamp = strdup(timebuf);
                T2Debug("Timestamp for %s is %s\n", lookupEvent->markerName_CT, lookupEvent->timestamp);
            }
            break;

        case MTYPE_ACCUMULATE:
            T2Debug("Marker type is ACCUMULATE Event Value : %s\n", eventInfo->value);
            arraySize = Vector_Size(lookupEvent->u.accumulatedValues);
            T2Debug("Current array size : %d \n", arraySize);
            if( arraySize < MAX_ACCUMULATE)
            {
                Vector_PushBack(lookupEvent->u.accumulatedValues, strdup(eventInfo->value));
                T2Debug("Sucessfully added value into vector New Size : %d\n", ++arraySize);
                if(lookupEvent->reportTimestampParam == REPORTTIMESTAMP_UNIXEPOCH)
                {
                    timestamp = time(NULL);
                    if(lookupEvent->markerName_CT == NULL)
                    {
                        if(lookupEvent->alias != NULL)
                        {
                            snprintf(buf, MAX_LEN, "%s_CT", lookupEvent->alias);
                        }
                        else
                        {
                            snprintf(buf, MAX_LEN, "%s_CT", lookupEvent->markerName);
                        }
                        lookupEvent->markerName_CT = strdup(buf);
                    }
                    snprintf(timebuf, MAX_LEN, "%ld", (long) timestamp);
                    T2Debug("Timestamp for %s is %ld\n", lookupEvent->markerName_CT, (long) timestamp);
                    Vector_PushBack(lookupEvent->accumulatedTimestamp,  strdup(timebuf));
                    T2Debug("Vector_PushBack for accumulatedTimestamp is done\n");
                }
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

        case MTYPE_ABSOLUTE:
        default:
            if(lookupEvent->u.markerValue)
            {
                free(lookupEvent->u.markerValue);
                lookupEvent->u.markerValue = NULL;
            }

            lookupEvent->u.markerValue = strdup(eventInfo->value);
            T2Debug("New marker value saved : %s\n", lookupEvent->u.markerValue);
            if(lookupEvent->reportTimestampParam == REPORTTIMESTAMP_UNIXEPOCH)
            {
                if(lookupEvent->timestamp)
                {
                    free(lookupEvent->timestamp);
                    lookupEvent->timestamp = NULL;
                }

                timestamp = time(NULL);
                if(lookupEvent->markerName_CT == NULL)
                {
                    if(lookupEvent->alias != NULL)
                    {
                        snprintf(buf, MAX_LEN, "%s_CT", lookupEvent->alias);
                    }
                    else
                    {
                        snprintf(buf, MAX_LEN, "%s_CT", lookupEvent->markerName);
                    }
                    lookupEvent->markerName_CT = strdup(buf);
                }
                snprintf(timebuf, MAX_LEN, "%ld", (long)timestamp);
                lookupEvent->timestamp = strdup(timebuf);
                T2Debug("Timestamp for %s is %s\n", lookupEvent->markerName_CT, lookupEvent->timestamp);
            }
            break;
        }
        pthread_mutex_unlock(&profile->eventMutex);
    }
    else
    {
        T2Error("Event name : %s value : %s\n", eventInfo->name, eventInfo->value);
        T2Error("Event doens't match any marker information, shouldn't come here\n");
        return T2ERROR_FAILURE;
    }

    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

T2ERROR addProfile(Profile *profile)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if(!initialized)
    {
        T2Error("profile list is not initialized yet, ignoring\n");
        return T2ERROR_FAILURE;
    }
    pthread_rwlock_wrlock(&plRwLock);
    Vector_PushBack(profileList, profile);

    pthread_rwlock_unlock(&plRwLock);
    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

T2ERROR enableProfile(const char *profileName)
{
    T2Debug("%s ++in \n", __FUNCTION__);
    if(!initialized)
    {
        T2Error("profile list is not initialized yet, ignoring\n");
        return T2ERROR_FAILURE;
    }
    pthread_rwlock_wrlock(&plRwLock);
    Profile *profile = NULL;
    if(T2ERROR_SUCCESS != getProfile(profileName, &profile))
    {
        T2Error("Profile : %s not found\n", profileName);
        pthread_rwlock_unlock(&plRwLock);
        return T2ERROR_FAILURE;
    }
    if(profile->enable)
    {
        T2Info("Profile : %s is already enabled - ignoring duplicate request\n", profileName);
        pthread_rwlock_unlock(&plRwLock);
        return T2ERROR_SUCCESS;
    }
    else
    {
        /* DEADLOCK PREVENTION: Initialize profile locks first, then release global
         * lock before calling external functions to avoid lock ordering deadlocks */
        profile->enable = true;
        if(pthread_mutex_init(&profile->triggerCondMutex, NULL) != 0)
        {
            T2Error(" %s Mutex init has failed\n", __FUNCTION__);
            pthread_rwlock_unlock(&plRwLock);
            return T2ERROR_FAILURE;
        }
        if(pthread_mutex_init(&profile->reportInProgressMutex, NULL) != 0)
        {
            T2Error(" %s Mutex init has failed\n", __FUNCTION__);
            pthread_rwlock_unlock(&plRwLock);
            return T2ERROR_FAILURE;
        }
        if(pthread_cond_init(&profile->reportInProgressCond, NULL) != 0)
        {
            T2Error(" %s Cond init has failed\n", __FUNCTION__);
            pthread_rwlock_unlock(&plRwLock);
            return T2ERROR_FAILURE;
        }

        /* DEADLOCK PREVENTION: Copy necessary data while holding lock, then release
         * before calling external functions to prevent deadlock scenarios */
        size_t eMarkerCount = Vector_Size(profile->eMarkerList);
        char *profileNameCopy = strdup(profile->name);
        
        /* Prepare external call parameters while holding lock */
        typedef struct {
            char *markerName;
            char *compName;
            int skipFreq;
        } marker_info_t;
        
        marker_info_t *markerInfos = NULL;
        if (eMarkerCount > 0)
        {
            markerInfos = malloc(eMarkerCount * sizeof(marker_info_t));
            if (markerInfos)
            {
                for(size_t emIndex = 0; emIndex < eMarkerCount; emIndex++)
                {
                    EventMarker *eMarker = (EventMarker *)Vector_At(profile->eMarkerList, emIndex);
                    markerInfos[emIndex].markerName = strdup(eMarker->markerName);
                    markerInfos[emIndex].compName = strdup(eMarker->compName);
                    markerInfos[emIndex].skipFreq = eMarker->skipFreq;
                }
            }
        }
        
        /* Copy scheduler parameters */
        int reportingInterval = profile->reportingInterval;
        int activationTimeoutPeriod = profile->activationTimeoutPeriod;
        bool deleteOnTimeout = profile->deleteonTimeout;
        bool reportOnUpdate = profile->reportOnUpdate;
        int firstReportingInterval = profile->firstReportingInterval;
        char *timeRefCopy = profile->timeRef ? strdup(profile->timeRef) : NULL;
        
        /* CRITICAL: Release global lock before external function calls */
        pthread_rwlock_unlock(&plRwLock);
        
        /* Now make external calls without holding any global locks */
        if (markerInfos)
        {
            for(size_t i = 0; i < eMarkerCount; i++)
            {
                if (markerInfos[i].markerName)
                {
                    addT2EventMarker(markerInfos[i].markerName, markerInfos[i].compName, 
                                   profileNameCopy, markerInfos[i].skipFreq);
                    free(markerInfos[i].markerName);
                    free(markerInfos[i].compName);
                }
            }
            free(markerInfos);
        }
        
        /* Register with scheduler without holding global lock */
        T2ERROR schedResult = registerProfileWithScheduler(profileNameCopy, reportingInterval, 
                                                          activationTimeoutPeriod, deleteOnTimeout, 
                                                          true, reportOnUpdate, firstReportingInterval, timeRefCopy);
        
        /* Clean up copied data */
        free(profileNameCopy);
        if (timeRefCopy) free(timeRefCopy);
        
        if(schedResult != T2ERROR_SUCCESS)
        {
            /* Re-acquire lock to clean up on failure */
            pthread_rwlock_wrlock(&plRwLock);
            Profile *failedProfile = NULL;
            if(T2ERROR_SUCCESS == getProfile(profileName, &failedProfile))
            {
                failedProfile->enable = false;
            }
            pthread_rwlock_unlock(&plRwLock);
            T2Error("Unable to register profile : %s with Scheduler\n", profileName);
            return T2ERROR_FAILURE;
        }
        T2ER_StartDispatchThread();

        T2Info("Successfully enabled profile : %s\n", profileName);
        return T2ERROR_SUCCESS;  /* Already released lock above */
    }
    T2Debug("%s --out\n", __FUNCTION__);
    pthread_rwlock_unlock(&plRwLock);
    return T2ERROR_SUCCESS;
}


void updateMarkerComponentMap()
{
    T2Debug("%s ++in\n", __FUNCTION__);

    size_t profileIndex = 0;
    Profile *tempProfile = NULL;

    pthread_rwlock_rdlock(&plRwLock);
    for(; profileIndex < Vector_Size(profileList); profileIndex++)
    {
        tempProfile = (Profile *)Vector_At(profileList, profileIndex);
        if(tempProfile->enable)
        {
            T2Debug("Updating component map for profile %s \n", tempProfile->name);
            size_t emIndex = 0;
            EventMarker *eMarker = NULL;
            for(; emIndex < Vector_Size(tempProfile->eMarkerList); emIndex++)
            {
                eMarker = (EventMarker *)Vector_At(tempProfile->eMarkerList, emIndex);
                addT2EventMarker(eMarker->markerName, eMarker->compName, tempProfile->name, eMarker->skipFreq);
            }
        }
    }
    pthread_rwlock_unlock(&plRwLock);
    T2Debug("%s --out\n", __FUNCTION__);
}

T2ERROR disableProfile(const char *profileName, bool *isDeleteRequired)
{
    T2Debug("%s ++in \n", __FUNCTION__);

    if(!initialized)
    {
        T2Error("profile list is not initialized yet, ignoring\n");
        return T2ERROR_FAILURE;
    }

    pthread_rwlock_wrlock(&plRwLock);
    Profile *profile = NULL;
    if(T2ERROR_SUCCESS != getProfile(profileName, &profile))
    {
        T2Error("Profile : %s not found\n", profileName);
        pthread_rwlock_unlock(&plRwLock);
        return T2ERROR_FAILURE;
    }

    if (profile->generateNow)
    {
        *isDeleteRequired = true;
    }
    else
    {
        profile->enable = false;
    }
#ifdef PERSIST_LOG_MON_REF
    removeProfileFromDisk(SEEKFOLDER, profile->name);
#endif
    profile->isSchedulerstarted = false;
    pthread_rwlock_unlock(&plRwLock);
    T2Debug("%s --out\n", __FUNCTION__);

    return T2ERROR_SUCCESS;
}

T2ERROR deleteAllProfiles(bool delFromDisk)
{
    T2Debug("%s ++in\n", __FUNCTION__);

    int count = 0;
    int profileIndex = 0;
    Profile *tempProfile = NULL;

    pthread_rwlock_wrlock(&plRwLock);
    if(profileList == NULL)
    {
        T2Error("profile list is not initialized yet or profileList is empty, ignoring\n");
        pthread_rwlock_unlock(&plRwLock);
        return T2ERROR_FAILURE;
    }

    count = Vector_Size(profileList);
    pthread_rwlock_unlock(&plRwLock);

    for(; profileIndex < count; profileIndex++)
    {
        pthread_rwlock_rdlock(&plRwLock);
        tempProfile = (Profile *)Vector_At(profileList, profileIndex);
        tempProfile->enable = false;
        tempProfile->isSchedulerstarted = false;
        pthread_rwlock_unlock(&plRwLock);

        if(T2ERROR_SUCCESS != unregisterProfileFromScheduler(tempProfile->name))
        {
            T2Error("Profile : %s failed to  unregister from scheduler\n", tempProfile->name);
        }

        /* Read threadExists under reuseThreadMutex: threadExists is written under
         * reuseThreadMutex in CollectAndReport, so reads must use the same lock.
         * The thread sets threadExists = false and then destroys reuseThreadMutex
         * before returning, so reuseThreadMutex must not be accessed after join. */
        pthread_mutex_lock(&tempProfile->reuseThreadMutex);
        bool threadStillExists = tempProfile->threadExists;
        if (threadStillExists)
        {
            tempProfile->restartRequested = true;
            pthread_cond_signal(&tempProfile->reuseThread);
        }
        pthread_mutex_unlock(&tempProfile->reuseThreadMutex);

        if (threadStillExists)
        {
            pthread_join(tempProfile->reportThread, NULL);
            /* Do not access reuseThreadMutex after join: the thread destroys it
             * after setting threadExists = false (see CollectAndReport cleanup). */
        }

        /* Re-acquire plRwLock for profile cleanup */
        pthread_rwlock_rdlock(&plRwLock);
        if(tempProfile->grepSeekProfile)
        {
            freeGrepSeekProfile(tempProfile->grepSeekProfile);
        }
        pthread_rwlock_unlock(&plRwLock);
        if(delFromDisk == true)
        {
            removeProfileFromDisk(REPORTPROFILES_PERSISTENCE_PATH, tempProfile->name);
#ifdef PERSIST_LOG_MON_REF
            removeProfileFromDisk(SEEKFOLDER, tempProfile->name);
#endif
        }
    }
    if(delFromDisk == true)
    {
        removeProfileFromDisk(REPORTPROFILES_PERSISTENCE_PATH, MSGPACK_REPORTPROFILES_PERSISTENT_FILE);
    }

    pthread_rwlock_rdlock(&plRwLock);
    T2Debug("Deleting all profiles from the profileList\n");
    Vector_Destroy(profileList, freeProfile);
    profileList = NULL;
    Vector_Create(&profileList);
    pthread_rwlock_unlock(&plRwLock);

    T2Debug("%s --out\n", __FUNCTION__);

    return T2ERROR_SUCCESS;
}

bool isProfileEnabled(const char *profileName)
{
    bool is_profile_enable = false;
    Profile *get_profile = NULL;
    pthread_rwlock_rdlock(&plRwLock);
    if(T2ERROR_SUCCESS != getProfile(profileName, &get_profile))
    {
        T2Error("Profile : %s not found\n", profileName);
        T2Debug("%s --out\n", __FUNCTION__);
        pthread_rwlock_unlock(&plRwLock);
        return false;
    }
    is_profile_enable = get_profile->enable;
    T2Debug("is_profile_enable = %d \n", is_profile_enable);
    pthread_rwlock_unlock(&plRwLock);
    return is_profile_enable;
}


T2ERROR deleteProfile(const char *profileName)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if(!initialized)
    {
        T2Error("profile list is not initialized yet, ignoring\n");
        return T2ERROR_FAILURE;
    }

    Profile *profile = NULL;
    pthread_rwlock_wrlock(&plRwLock);
    if(T2ERROR_SUCCESS != getProfile(profileName, &profile))
    {
        T2Error("Profile : %s not found\n", profileName);
        pthread_rwlock_unlock(&plRwLock);
        return T2ERROR_FAILURE;
    }

    if(profile->enable)
    {
        profile->enable = false;
    }
    if(profile->isSchedulerstarted)
    {
        profile->isSchedulerstarted = false;
    }
    pthread_rwlock_unlock(&plRwLock);
    if(T2ERROR_SUCCESS != unregisterProfileFromScheduler(profileName))
    {
        T2Info("Profile : %s already removed from scheduler\n", profileName);
    }

    T2Info("Waiting for CollectAndReport to be complete : %s\n", profileName);
    pthread_rwlock_rdlock(&plRwLock);

    // Wait for any in-progress report to finish under reportInProgressMutex.
    // threadExists must NOT be read here — it is written under reuseThreadMutex
    // in CollectAndReport, so mixing the two mutexes causes a data race (Coverity
    // LOCK_EVASION).  Read threadExists separately under reuseThreadMutex below.
    pthread_mutex_lock(&profile->reportInProgressMutex);
    while (profile->reportInProgress)
    {
        pthread_cond_wait(&profile->reportInProgressCond, &profile->reportInProgressMutex);
    }
    pthread_mutex_unlock(&profile->reportInProgressMutex);

    /* Read threadExists under reuseThreadMutex: it is written under the same mutex
     * in CollectAndReport.  The thread sets threadExists = false and then destroys
     * reuseThreadMutex before returning, so reuseThreadMutex must not be used after
     * pthread_join returns. */
    pthread_mutex_lock(&profile->reuseThreadMutex);
    bool threadStillExists = profile->threadExists;
    pthread_mutex_unlock(&profile->reuseThreadMutex);

    /* Release plRwLock before pthread_join to avoid deadlock.
     * pthread_join can block indefinitely if the CollectAndReport thread
     * is stuck (e.g., waiting on rbusMethodMutex). Holding plRwLock during
     * pthread_join prevents other threads (timeout callbacks, other profile
     * operations) from making progress, creating a deadlock.
     */
    pthread_rwlock_unlock(&plRwLock);

    if (threadStillExists)
    {
        pthread_mutex_lock(&profile->reuseThreadMutex);
        profile->restartRequested = true;
        pthread_cond_signal(&profile->reuseThread);
        pthread_mutex_unlock(&profile->reuseThreadMutex);
        pthread_join(profile->reportThread, NULL);
        /* Do not access reuseThreadMutex after join: the thread destroys it
         * after setting threadExists = false (see CollectAndReport cleanup). */
    }

    /* Re-acquire plRwLock for profile cleanup operations */
    pthread_rwlock_rdlock(&plRwLock);

    if(Vector_Size(profile->triggerConditionList) > 0)
    {
        rbusT2ConsumerUnReg(profile->triggerConditionList);
    }

    if(profile->grepSeekProfile)
    {
        freeGrepSeekProfile(profile->grepSeekProfile);
    }

    pthread_mutex_destroy(&profile->reportInProgressMutex);
    pthread_cond_destroy(&profile->reportInProgressCond);

    T2Info("removing profile : %s from profile list\n", profile->name);
#ifdef PERSIST_LOG_MON_REF
    removeProfileFromDisk(SEEKFOLDER, profile->name);
#endif
    Vector_RemoveItem(profileList, profile, freeProfile);

    pthread_rwlock_unlock(&plRwLock);
    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

void sendLogUploadInterruptToScheduler()
{
    size_t profileIndex = 0;
    Profile *tempProfile = NULL;
    T2Debug("%s ++in\n", __FUNCTION__);

    pthread_rwlock_rdlock(&plRwLock);
    for(; profileIndex < Vector_Size(profileList); profileIndex++)
    {
        tempProfile = (Profile *)Vector_At(profileList, profileIndex);
        if (Vector_Size(tempProfile->gMarkerList) > 0)
        {
            SendInterruptToTimeoutThread(tempProfile->name);
        }
    }
    pthread_rwlock_unlock(&plRwLock);
    T2Debug("%s --out\n", __FUNCTION__);
}

static void loadReportProfilesFromDisk(bool checkPreviousSeek)
{
    (void)checkPreviousSeek;
    fetchLocalConfigs(SHORTLIVED_PROFILES_PATH, NULL);   //API used for creating /tmp/t2reportprofiles dir
#if defined(FEATURE_SUPPORT_WEBCONFIG)
    T2Info("loadReportProfilesFromDisk \n");
    char filePath[REPORTPROFILES_FILE_PATH_SIZE] = {'\0'};
    snprintf(filePath, sizeof(filePath), "%s%s", REPORTPROFILES_PERSISTENCE_PATH, MSGPACK_REPORTPROFILES_PERSISTENT_FILE);
    /* CID: 157386 Time of check time of use (TOCTOU) */
    FILE *fp;
    fp = fopen (filePath, "rb");
    if(fp != NULL)
    {
        T2Info("Msgpack: loadReportProfilesFromDisk \n");
        struct __msgpack__ msgpack;
        fseek(fp, 0L, SEEK_END);
        msgpack.msgpack_blob_size = ftell(fp);
        if(msgpack.msgpack_blob_size < 0)
        {
            T2Error("Unable to detect the file pointer position for file %s\n", filePath);
            fclose(fp);
            return;
        }
        msgpack.msgpack_blob = malloc(sizeof(char) * msgpack.msgpack_blob_size);
        if (NULL == msgpack.msgpack_blob)
        {
            T2Error("Unable to allocate %d bytes of memory at Line %d on %s \n",
                    msgpack.msgpack_blob_size, __LINE__, __FILE__);
            fclose (fp);
            return;
        }
        fseek(fp, 0L, SEEK_SET);
        if(fread(msgpack.msgpack_blob, sizeof(char), msgpack.msgpack_blob_size, fp) < (size_t) msgpack.msgpack_blob_size)
        {
            T2Error("fread is returning fewer bytes than expected from the file %s\n", filePath);
            free(msgpack.msgpack_blob);
            fclose(fp);
            return;
        }
        fclose (fp);
        __ReportProfiles_ProcessReportProfilesMsgPackBlob((void *)&msgpack, checkPreviousSeek);
        free(msgpack.msgpack_blob);
        clearPersistenceFolder(CACHED_MESSAGE_PATH);
        return;
    }
    T2Info("JSON: loadReportProfilesFromDisk \n");
#endif
#if defined(PRIVACYMODES_CONTROL)
    char* paramValue = NULL;
    getPrivacyMode(&paramValue);
    if(strcmp(paramValue, "DO_NOT_SHARE") == 0)
    {
        T2Warning("PrivacyModes is DO_NOT_SHARE. Reportprofiles is not supported\n");
        free(paramValue);
        paramValue = NULL;
        return;
    }
    free(paramValue);
    paramValue = NULL;
#endif

    size_t configIndex = 0;
    Vector *configList = NULL;
    Config *config = NULL;
    T2Debug("%s ++in\n", __FUNCTION__);

    Vector_Create(&configList);
    fetchLocalConfigs(REPORTPROFILES_PERSISTENCE_PATH, configList);

    for(; configIndex < Vector_Size(configList); configIndex++)
    {
        config = Vector_At(configList, configIndex);
        Profile *profile = 0;
        T2Debug("Processing config with name : %s\n", config->name);
        T2Debug("Config Size = %lu\n", (unsigned long)strlen(config->configData));

        if(T2ERROR_SUCCESS == processConfiguration(&config->configData, config->name, NULL, &profile))
        {
            if(T2ERROR_SUCCESS == addProfile(profile))
            {
#ifdef PERSIST_LOG_MON_REF
                if(checkPreviousSeek && profile->generateNow == false && profile->triggerConditionList == NULL && profile->grepSeekProfile && loadSavedSeekConfig(profile->name, profile->grepSeekProfile) == T2ERROR_SUCCESS && firstBootStatus() )
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
                T2Info("Successfully created/added new profile : %s\n", profile->name);
                if(T2ERROR_SUCCESS != enableProfile(profile->name))
                {
                    T2Error("Failed to enable profile name : %s\n", profile->name);
                }
                else
                {

#ifdef PERSIST_LOG_MON_REF
                    if(profile->checkPreviousSeek)
                    {
                        T2Info("Previous Seek is enabled so generate the report for %s\n", profile->name);
                        // Trigger the report generation for the profile if previous seek is valid
                        NotifyTimeout(profile->name, true);
                    }
#endif
                    // Load the cached messages from previous boot, if any
                    populateCachedReportList(profile->name, profile->cachedReportList);
                }
            }
            else
            {
                T2Error("Unable to create and add new profile for name : %s\n", config->name);
                if (profile != NULL)
                {
                    freeProfile(profile);
                    profile = NULL;
                }
            }
        }
    }
    T2Info("Completed processing %lu profiles on the disk,trying to fetch new/updated profiles\n", (unsigned long)Vector_Size(configList));
    T2totalmem_calculate();
    Vector_Destroy(configList, freeReportProfileConfig);
    clearPersistenceFolder(CACHED_MESSAGE_PATH);

    T2Debug("%s --out\n", __FUNCTION__);
}

T2ERROR initProfileList(bool checkPreviousSeek)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if(initialized)
    {
        T2Info("profile list is already initialized\n");
        return T2ERROR_SUCCESS;
    }
    initialized = true;
    if(pthread_rwlock_init(&plRwLock, NULL) != 0)
    {
        T2Error("%s rwlock init has failed\n", __FUNCTION__);
        return T2ERROR_FAILURE;
    }
    if(pthread_mutex_init(&reportLock, NULL) != 0 )
    {
        T2Error("%s mutex init has failed\n", __FUNCTION__);
        return T2ERROR_FAILURE;
    }

    pthread_rwlock_wrlock(&plRwLock);
    Vector_Create(&profileList);
    pthread_rwlock_unlock(&plRwLock);

    registerConditionalReportCallBack(&triggerReportOnCondtion);

    loadReportProfilesFromDisk(checkPreviousSeek);

    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

int getProfileCount()
{
    int count = 0;
    T2Debug("%s ++in\n", __FUNCTION__);
    if(!initialized)
    {
        T2Info("profile list isn't initialized\n");
        return count;
    }
    pthread_rwlock_rdlock(&plRwLock);
    count = Vector_Size(profileList);
    pthread_rwlock_unlock(&plRwLock);

    T2Debug("%s --out\n", __FUNCTION__);
    return count;
}

hash_map_t *getProfileHashMap()
{
    size_t profileIndex = 0;
    hash_map_t *profileHashMap = NULL;
    Profile *tempProfile = NULL;
    T2Debug("%s ++in\n", __FUNCTION__);

    pthread_rwlock_rdlock(&plRwLock);
    profileHashMap = hash_map_create();
    for(; profileIndex < Vector_Size(profileList); profileIndex++)
    {
        tempProfile = (Profile *)Vector_At(profileList, profileIndex);
        char *profileName = strdup(tempProfile->name);
        char *profileHash = strdup(tempProfile->hash);
        hash_map_put(profileHashMap, profileName, profileHash, free);
    }
    pthread_rwlock_unlock(&plRwLock);

    T2Debug("%s --out\n", __FUNCTION__);
    return profileHashMap;
}

T2ERROR uninitProfileList()
{
    T2Debug("%s ++in\n", __FUNCTION__);

    if(!initialized)
    {
        T2Info("profile list is not initialized yet, ignoring\n");
        return T2ERROR_SUCCESS;
    }

    initialized = false;
    deleteAllProfiles(false); // avoid removing multiProfiles from Disc

    if(!pthread_mutex_trylock(&triggerConditionQueMutex))
    {
        if(triggerConditionQueue)
        {
            t2_queue_destroy(triggerConditionQueue, free);
        }

        pthread_mutex_unlock(&triggerConditionQueMutex);
    }

    pthread_mutex_destroy(&reportLock);
    pthread_rwlock_destroy(&plRwLock);

    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

T2ERROR registerTriggerConditionConsumer()
{

    T2Debug("%s ++in\n", __FUNCTION__);
#define MAX_RETRY_COUNT 3
    size_t profileIndex = 0;
    int retry_count = 0;
    int retry = 0;
    int timer = 16;
    int ret = T2ERROR_SUCCESS;
    Profile *tempProfile = NULL;

    while(retry_count <= MAX_RETRY_COUNT)
    {
        pthread_rwlock_rdlock(&plRwLock);
        profileIndex = 0;
        for(; profileIndex < Vector_Size(profileList); profileIndex++)
        {
            tempProfile = (Profile *)Vector_At(profileList, profileIndex);
            if(tempProfile->triggerConditionList)
            {
                ret = rbusT2ConsumerReg(tempProfile->triggerConditionList);
                T2Debug("rbusT2ConsumerReg return = %d\n", ret);
                if(ret != T2ERROR_SUCCESS)
                {
                    retry = 1;
                }
            }

        }
        pthread_rwlock_unlock(&plRwLock);
        if(retry == 1)
        {
            if(retry_count >= MAX_RETRY_COUNT)
            {
                break;
            }
            T2Debug("Retry Consumer Registration in %d sec\n", timer);
            retry_count++;
            retry = 0;
            sleep(timer);
            timer = timer / 2;
        }
        else
        {
            break;
        }
    }
    T2Debug("%s --out\n", __FUNCTION__);
    return ret;
}

void NotifySchedulerstart(char* profileName, bool isschedulerstarted)
{
    size_t profileIndex = 0;
    Profile *tempProfile = NULL;
    pthread_rwlock_rdlock(&plRwLock);
    T2Debug("plRwLock is locked  %s\n", __FUNCTION__);
    for(; profileIndex < Vector_Size(profileList); profileIndex++)
    {
        tempProfile = (Profile *)Vector_At(profileList, profileIndex);
        if(tempProfile != NULL)
        {
            if(strncmp(tempProfile->name, profileName, strlen(profileName) + 1) == 0)
            {
                tempProfile->isSchedulerstarted = isschedulerstarted;
            }
        }
    }
    pthread_rwlock_unlock(&plRwLock);
    T2Debug("plRwLock is unlocked  %s\n", __FUNCTION__);
    return;
}

T2ERROR appendTriggerCondition (Profile *tempProfile, const char *referenceName, const char *referenceValue)
{
    T2Debug("%s ++in\n", __FUNCTION__);

    T2ERROR status = T2ERROR_SUCCESS ;

    // triggerCondMutex is purposefully left unlocked from this function.
    // It gets released from CollectAndReport thread once trigger condition based report is sent out.
    // Reports from multiple trigger condition has to be sent mutually exclusively as separate reports.
    if(!pthread_mutex_trylock(&tempProfile->triggerCondMutex))
    {
        T2Debug("%s : Lock acquisition succeeded for tempProfile->triggerCondMutex\n ", __FUNCTION__);
        cJSON *temparrayItem = cJSON_CreateObject();
        cJSON_AddStringToObject(temparrayItem, "reference", referenceName);
        cJSON_AddStringToObject(temparrayItem, "value", referenceValue);
        cJSON *temparrayItem1 = cJSON_CreateObject();

        cJSON_AddItemToObject(temparrayItem1, "TriggerConditionResult", temparrayItem);
        tempProfile->jsonReportObj = temparrayItem1;
        //pthread_mutex_unlock(&tempProfile->triggerCondMutex);

    }
    else
    {
        T2Warning("%s : Failed to get a lock on tempProfile->triggerCondMutex for condition %s \n ", __FUNCTION__, referenceName);
        status =  T2ERROR_FAILURE;
        // Push this trigger condition to a que.
        // Implement a callback function to trigger report generation for referenceName & value

        if(!pthread_mutex_trylock(&triggerConditionQueMutex))
        {
            T2Debug("%s : Lock on triggerConditionQueMutex\n", __FUNCTION__);
            if(NULL == triggerConditionQueue)
            {
                triggerConditionQueue = t2_queue_create();
                if(!triggerConditionQueue)
                {
                    T2Error("Failed to create triggerConditionQueue, not proceeding further\n");
                    T2Debug("%s : Unlock on triggerConditionQueMutex\n", __FUNCTION__);
                    pthread_mutex_unlock(&triggerConditionQueMutex);
                    T2Debug("%s --out\n", __FUNCTION__);
                    return status ;
                }
                T2Debug("%s : Que for storing the trigger conditions is created \n", __FUNCTION__);
            }
            triggerConditionObj *triggerCond = (triggerConditionObj*) malloc(sizeof(triggerConditionObj));
            if(triggerCond)
            {
                if(NULL != referenceName && NULL != referenceValue)
                {
                    T2Info("%s : Push referenceName = %s  & referenceValue = %s to que \n ", __FUNCTION__, referenceName, referenceValue);
                    memset(triggerCond, 0, sizeof(triggerConditionObj));
                    strncpy(triggerCond->referenceName, referenceName, (MAX_LEN - 1));
                    strncpy(triggerCond->referenceValue, referenceValue, (MAX_LEN - 1));
                    t2_queue_push(triggerConditionQueue, (void*) triggerCond);
                    // Ownership of triggerCond is transferred to triggerConditionQueue and will be freed after being popped.
                }
                else
                {
                    T2Warning("%s : referenceName or referenceValue is published as null, ignoring trigger condition \n ", __FUNCTION__);
                }

            }
            else
            {
                T2Error("Failed to allocate memory for triggerConditionObj\n");
            }

            T2Debug("%s : Unlock on triggerConditionQueMutex\n", __FUNCTION__);
            pthread_mutex_unlock(&triggerConditionQueMutex);

            /* ref RDKB-55438 Commenting following code as this is causing crash, we can't free triggerCond here as this
            has been added to the queue, this is freed in the reportGenerationCompleteReceiver function after queue pop

            if(triggerCond != NULL) {
            free(triggerCond);//CID 335291: Resource leak (RESOURCE_LEAK)
            }
            */

        }
        else
        {
            T2Warning("%s : Failed to get a lock on triggerConditionQueMutex for condition %s \n ", __FUNCTION__, referenceName);

        }
    }

    T2Debug("%s --out\n", __FUNCTION__);
    return status;
}

void reportGenerationCompleteReceiver(char *profileName)
{

    T2Debug("%s ++in\n", __FUNCTION__);
    T2Info("%s called with argument %s\n", __FUNCTION__, profileName);

    if(!pthread_mutex_lock(&triggerConditionQueMutex))
    {
        if(NULL != triggerConditionQueue)
        {
            T2Info("%s : %d report generating triggers pending in que \n ", __FUNCTION__, t2_queue_count(triggerConditionQueue));
            if(t2_queue_count(triggerConditionQueue) > 0)
            {
                triggerConditionObj *triggerCond = (triggerConditionObj*) t2_queue_pop(triggerConditionQueue);
                if(triggerCond)
                {
                    // Que has to be unlocked before generating a trigger condition
                    pthread_mutex_unlock(&triggerConditionQueMutex);
                    triggerReportOnCondtion(triggerCond->referenceName, triggerCond->referenceValue);
                    free(triggerCond);
                    triggerCond = NULL;
                }
                else
                {
                    T2Debug("%s : Trigger condition is null \n", __FUNCTION__);
                    pthread_mutex_unlock(&triggerConditionQueMutex);
                }
            }
            else
            {
                T2Debug("%s : Unlock on triggerConditionQueMutex\n", __FUNCTION__);
                pthread_mutex_unlock(&triggerConditionQueMutex);
                T2Debug("No more report on condition events present in que \n");
            }
        }
        else
        {
            pthread_mutex_unlock(&triggerConditionQueMutex);
        }
    }
    else
    {
        T2Error("Failed to get lock on triggerConditionQueMutex \n");
    }

    T2Debug("%s --out\n", __FUNCTION__);
}

T2ERROR triggerReportOnCondtion(const char *referenceName, const char *referenceValue)
{
    T2Debug("%s ++in\n", __FUNCTION__);

    T2Debug("referenceName = %s  referenceValue = %s \n", referenceName, referenceValue);

    size_t j, profileIndex = 0;
    Profile *tempProfile = NULL;

    pthread_rwlock_rdlock(&plRwLock);
    for(; profileIndex < Vector_Size(profileList); profileIndex++)
    {
        tempProfile = (Profile *)Vector_At(profileList, profileIndex);
        if(tempProfile->triggerConditionList && (tempProfile->triggerConditionList->count > 0))
        {
            for( j = 0; j < tempProfile->triggerConditionList->count; j++ )
            {
                TriggerCondition *triggerCondition = ((TriggerCondition *) Vector_At(tempProfile->triggerConditionList, j));
                if(strcmp(triggerCondition->reference, referenceName) == 0)
                {
                    if(triggerCondition->report)
                    {
                        if ( T2ERROR_SUCCESS == appendTriggerCondition(tempProfile, referenceName, referenceValue))
                        {
                            tempProfile->triggerReportOnCondition = true;
                            tempProfile->minThresholdDuration = triggerCondition->minThresholdDuration;
                            T2Debug("%s : Assign callback function as reportGenerationCompleteReceiver \n", __FUNCTION__);
                            tempProfile->callBackOnReportGenerationComplete = reportGenerationCompleteReceiver;

                            char *tempProfilename = strdup(tempProfile->name); //RDKB-42640

                            // plRwLock should be unlocked before sending interrupt for report generation
                            T2Debug("%s : Release lock on &plMutex\n ", __FUNCTION__);
                            pthread_rwlock_unlock(&plRwLock);
                            T2Info("Triggering report on condition for %s with %s operator, %d threshold\n", triggerCondition->reference,
                                   triggerCondition->oprator, triggerCondition->threshold);
                            if(tempProfile->isSchedulerstarted)
                            {
                                SendInterruptToTimeoutThread(tempProfilename);
                                // triggerCondMutex will be unlocked by CollectAndReport after report generation
                            }
                            else
                            {
                                T2Info("For Profile %s scheduler is not enabled yet so triggering the condition is ignored now\n", tempProfilename);
                                tempProfile->triggerReportOnCondition = false;
                                pthread_mutex_unlock(&tempProfile->triggerCondMutex);

                            }
                            free(tempProfilename); //RDKB-42640
                            return T2ERROR_SUCCESS ;
                        }
                        else
                        {
                            T2Info("Report generation will take place by popping up from the que by callback function \n");
                            T2Debug("%s : Release lock on &plMutex\n ", __FUNCTION__);
                            pthread_rwlock_unlock(&plRwLock);
                            return T2ERROR_SUCCESS;
                        }
                    }
                }
            }
        }
    }
    pthread_rwlock_unlock(&plRwLock);
    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

unsigned int getMinThresholdDuration(char *profileName)
{
    unsigned int minThresholdDuration = 0;
    Profile *get_profile = NULL;
    T2Debug("%s --in\n", __FUNCTION__);
    pthread_rwlock_rdlock(&plRwLock);
    if(T2ERROR_SUCCESS != getProfile(profileName, &get_profile))
    {
        T2Error("Profile : %s not found\n", profileName);
        T2Debug("%s --out\n", __FUNCTION__);
        pthread_rwlock_unlock(&plRwLock);
        return 0;
    }
    minThresholdDuration = get_profile->minThresholdDuration;
    get_profile->minThresholdDuration = 0; // reinit the value
    T2Debug("minThresholdDuration = %u \n", minThresholdDuration);
    pthread_rwlock_unlock(&plRwLock);
    T2Debug("%s --out\n", __FUNCTION__);
    return minThresholdDuration;
}


