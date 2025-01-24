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
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#if defined(CCSP_SUPPORT_ENABLED)
#include <ccsp/ccsp_memory.h>
#include <ccsp/ccsp_custom.h>
#include <ccsp/ccsp_base_api.h>
#endif
#include <rbus/rbus.h>

#include "telemetry_busmessage_sender.h"

#include "t2collection.h"
#include "vector.h"
#include "telemetry2_0.h"
#include "t2log_wrapper.h"
#include "telemetry_busmessage_internal.h"

#define MESSAGE_DELIMITER "<#=#>"
#define MAX_EVENT_CACHE 200
#define T2_COMPONENT_READY    "/tmp/.t2ReadyToReceiveEvents"
#define T2_SCRIPT_EVENT_COMPONENT "telemetry_client"
#define SENDER_LOG_FILE "/tmp/t2_sender_debug.log"

static const char* CCSP_FIXED_COMP_ID = "com.cisco.spvtg.ccsp.t2commonlib" ;

static char *componentName = NULL;
static void *bus_handle = NULL;
static bool isRFCT2Enable = false ;
static bool isT2Ready = false;
static bool isRbusEnabled = false ;
static int count = 0;
static pthread_mutex_t initMtx = PTHREAD_MUTEX_INITIALIZER;
static bool isMutexInitialized = false ;

static hash_map_t *eventMarkerMap = NULL;

static pthread_mutexattr_t mutexAttr;

static pthread_mutex_t eventMutex ;
static pthread_mutex_t sMutex ;
static pthread_mutex_t fMutex ;
static pthread_mutex_t dMutex ;
static pthread_mutex_t FileCacheMutex ;
static pthread_mutex_t markerListMutex ;
static pthread_mutex_t loggerMutex ;

static void EVENT_DEBUG(char* format, ...) {

    if(access(ENABLE_DEBUG_FLAG, F_OK) == -1) {
        return;
    }

    FILE *logHandle = NULL ;

    pthread_mutex_lock(&loggerMutex);
    logHandle = fopen(SENDER_LOG_FILE, "a+");
    if(logHandle) {
        time_t rawtime;
        struct tm* timeinfo;

        time(&rawtime);
        timeinfo = localtime(&rawtime);
        static char timeBuffer[20] = { '\0' };
        strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", timeinfo);
        fprintf(logHandle, "%s : ", timeBuffer);
        va_list argList;
        va_start(argList, format);
        vfprintf(logHandle, format, argList);
        va_end(argList);
        fclose(logHandle);
    }
    pthread_mutex_unlock(&loggerMutex);

}

static void initMutex() {
    pthread_mutex_lock(&initMtx);
    if ( !isMutexInitialized ) {
        isMutexInitialized = true ;
        pthread_mutexattr_init(&mutexAttr);
        pthread_mutexattr_settype(&mutexAttr, PTHREAD_MUTEX_RECURSIVE);

        pthread_mutex_init(&sMutex, &mutexAttr);
        pthread_mutex_init(&fMutex, &mutexAttr);
        pthread_mutex_init(&dMutex, &mutexAttr);
        pthread_mutex_init(&eventMutex, &mutexAttr);
        pthread_mutex_init(&FileCacheMutex, &mutexAttr);
        pthread_mutex_init(&markerListMutex, &mutexAttr);
        pthread_mutex_init(&loggerMutex, &mutexAttr);
    }
    pthread_mutex_unlock(&initMtx);
}

static void uninitMutex() {
    pthread_mutex_lock(&initMtx);
    if ( isMutexInitialized ) {
        isMutexInitialized = false ;

        pthread_mutex_destroy(&sMutex);
        pthread_mutex_destroy(&fMutex);
        pthread_mutex_destroy(&dMutex);
        pthread_mutex_destroy(&FileCacheMutex);
        pthread_mutex_destroy(&markerListMutex);
        pthread_mutex_destroy(&loggerMutex);

        pthread_mutexattr_destroy(&mutexAttr);
    }
    pthread_mutex_unlock(&initMtx);
}

#if defined(CCSP_SUPPORT_ENABLED)
T2ERROR getParamValues(char **paramNames, const int paramNamesCount, parameterValStruct_t ***valStructs, int *valSize)
{
    if (paramNames == NULL || paramNamesCount <= 0) {
        EVENT_ERROR("paramNames is NULL or paramNamesCount <= 0 - returning\n");
        return T2ERROR_INVALID_ARGS;
    }

    int ret = CcspBaseIf_getParameterValues(bus_handle, destCompName, (char*)destCompPath, paramNames,
                                            paramNamesCount, valSize, valStructs);
    if (ret != CCSP_SUCCESS) {
        EVENT_ERROR("CcspBaseIf_getParameterValues failed for : %s with ret = %d\n", paramNames[0],
                ret);
        return T2ERROR_FAILURE;
    }
    return T2ERROR_SUCCESS;
}

static void freeParamValue(parameterValStruct_t **valStructs, int valSize)
{
    free_parameterValStruct_t(bus_handle, valSize, valStructs);
}

static T2ERROR getCCSPParamVal(const char* paramName, char **paramValue)
{
    parameterValStruct_t **valStructs = NULL;
    int valSize = 0;
    char *paramNames[1] = {NULL};
    paramNames[0] = strdup(paramName);
    if(T2ERROR_SUCCESS != getParamValues(paramNames, 1, &valStructs, &valSize))
    {
        EVENT_ERROR("Unable to get %s\n", paramName);
        return T2ERROR_FAILURE;
    }
    *paramValue = strdup(valStructs[0]->parameterValue);
    free(paramNames[0]);
    freeParamValue(valStructs, valSize);
    return T2ERROR_SUCCESS;
}
#endif


static void rBusInterface_Uninit( ) {
    rbus_close(bus_handle);
}

static T2ERROR initMessageBus( ) {
    // EVENT_DEBUG("%s ++in\n", __FUNCTION__);
    T2ERROR status = T2ERROR_SUCCESS;
    char* component_id = (char*)CCSP_FIXED_COMP_ID;
#if defined(CCSP_SUPPORT_ENABLED)
    char *pCfg = (char*)CCSP_MSG_BUS_CFG;
#endif

    if(RBUS_ENABLED == rbus_checkStatus())
    {
        // EVENT_DEBUG("%s:%d, T2:rbus is enabled\n", __func__, __LINE__);
        char commonLibName[124] = { '\0' };
        // Bus handles should be unique across the system
        if(componentName) {
            snprintf(commonLibName, 124, "%s%s", "t2_lib_", componentName);
        }else {
            snprintf(commonLibName, 124, "%s", component_id);
        }
        rbusError_t status_rbus =  rbus_open((rbusHandle_t*) &bus_handle, commonLibName);
        if(status_rbus != RBUS_ERROR_SUCCESS) {
            EVENT_ERROR("%s:%d, init using component name %s failed with error code %d \n", __func__, __LINE__, commonLibName, status);
            status = T2ERROR_FAILURE;
        }
        isRbusEnabled = true;
    }
#if defined(CCSP_SUPPORT_ENABLED) 
    else {
        int ret = 0 ;
        ret = CCSP_Message_Bus_Init(component_id, pCfg, &bus_handle, (CCSP_MESSAGE_BUS_MALLOC)Ansc_AllocateMemory_Callback, Ansc_FreeMemory_Callback);
        if(ret == -1) {
            EVENT_ERROR("%s:%d, T2:initMessageBus failed\n", __func__, __LINE__);
            status = T2ERROR_FAILURE ;
        } else {
            status = T2ERROR_SUCCESS ;
        }
    }
#endif // CCSP_SUPPORT_ENABLED 
    // EVENT_DEBUG("%s --out\n", __FUNCTION__);
    return status;
}

static T2ERROR getRbusParameterVal(const char* paramName, char **paramValue) {
  
    rbusError_t ret = RBUS_ERROR_SUCCESS;
    rbusValue_t paramValue_t;
    rbusValueType_t rbusValueType ;
    char *stringValue = NULL;
#if 0
    rbusSetOptions_t opts;
    opts.commit = true;
#endif

    if(!bus_handle && T2ERROR_SUCCESS != initMessageBus()) {
        return T2ERROR_FAILURE;
    }

    ret = rbus_get(bus_handle, paramName, &paramValue_t);
    if(ret != RBUS_ERROR_SUCCESS) {
        EVENT_ERROR("Unable to get %s\n", paramName);
        return T2ERROR_FAILURE;
    }
    rbusValueType = rbusValue_GetType(paramValue_t);
    if(rbusValueType == RBUS_BOOLEAN) {
        if (rbusValue_GetBoolean(paramValue_t)){
            stringValue = strdup("true");
        } else {
            stringValue = strdup("false");
        }
    } else {
        stringValue = rbusValue_ToString(paramValue_t, NULL, 0);
    }
    *paramValue = stringValue;
    rbusValue_Release(paramValue_t);

    return T2ERROR_SUCCESS;
}

T2ERROR getParamValue(const char* paramName, char **paramValue)
{
    T2ERROR ret = T2ERROR_FAILURE ;
    if(isRbusEnabled)
        ret = getRbusParameterVal(paramName,paramValue);
#if defined(CCSP_SUPPORT_ENABLED)
    else
        ret = getCCSPParamVal(paramName, paramValue);
#endif

    return ret;
}

void *cacheEventToFile(void *arg)
{
	char *telemetry_data = (char *)arg;
        int fd;
        struct flock fl;
        fl.l_type = F_WRLCK;
        fl.l_whence = SEEK_SET;
        fl.l_start = 0;
        fl.l_len = 0;
        fl.l_pid = 0;
        FILE *fs = NULL;
        char path[100];
        pthread_detach(pthread_self());
        EVENT_ERROR("%s:%d, Caching the event to File\n", __func__, __LINE__);
	if(telemetry_data == NULL)
	{
		EVENT_ERROR("%s:%d, Data is NULL\n", __func__, __LINE__);
                return NULL;
	}
        pthread_mutex_lock(&FileCacheMutex);

        if ((fd = open(T2_CACHE_LOCK_FILE, O_RDWR | O_CREAT, 0666)) == -1)
        {
                EVENT_ERROR("%s:%d, T2:open failed\n", __func__, __LINE__);
                pthread_mutex_unlock(&FileCacheMutex);
		free(telemetry_data);
                return NULL;
        }

        if(fcntl(fd, F_SETLKW, &fl) == -1)  /* set the lock */
        {
                EVENT_ERROR("%s:%d, T2:fcntl failed\n", __func__, __LINE__);
                pthread_mutex_unlock(&FileCacheMutex);
                int ret = close(fd);
                if (ret != 0){
                    EVENT_ERROR("%s:%d, T2:close failed with error %d\n", __func__, __LINE__,ret);
                }  
		free(telemetry_data);
                return NULL;
        }

        FILE *fp = fopen(T2_CACHE_FILE, "a");
        if (fp == NULL) {
               EVENT_ERROR("%s: File open error %s\n", __FUNCTION__, T2_CACHE_FILE);
               goto unlock;
        }
        fs = popen ("cat /tmp/t2_caching_file | wc -l","r");
        if(fs != NULL){
            fgets(path,100,fs);
            count = atoi ( path );
            pclose(fs);
        }
        if(count < MAX_EVENT_CACHE){
            fprintf(fp, "%s\n", telemetry_data);
        }else{
            EVENT_DEBUG("Reached Max cache limit of 200, Caching is not done\n");
        }
        fclose(fp);

unlock:

        fl.l_type = F_UNLCK;  /* set to unlock same region */
        if (fcntl(fd, F_SETLK, &fl) == -1) {
                EVENT_ERROR("fcntl failed \n");
        }
        int ret = close(fd);
        if (ret != 0){
            EVENT_ERROR("%s:%d, T2:close failed with error %d\n", __func__, __LINE__,ret);
        }
        pthread_mutex_unlock(&FileCacheMutex);
	free(telemetry_data);
        return NULL;
}

static bool initRFC( ) {
    bool status = true ;
    // Check for RFC and proceed - if true - else return now .
    if(!bus_handle) {
        if(initMessageBus() != 0) {
            EVENT_ERROR("initMessageBus failed\n");
            status = false ;
        }
        else {
            status = true;
        }
        isRFCT2Enable = true;
    }

    return status;
}

/**
 * In rbus mode, should be using rbus subscribed param
 * from telemetry 2.0 instead of direct api for event sending
 */
int filtered_event_send(const char* data, char *markerName) {
    rbusError_t ret = RBUS_ERROR_SUCCESS;
    int status = 0 ;
    EVENT_DEBUG("%s ++in\n", __FUNCTION__);
    if(!bus_handle) {
        EVENT_ERROR("bus_handle is null .. exiting !!! \n");
        return ret;
    }

    if(isRbusEnabled)
    {

        // Filter data from marker list
        if(componentName && (0 != strcmp(componentName, T2_SCRIPT_EVENT_COMPONENT))) { // Events from scripts needs to be sent without filtering

            EVENT_DEBUG("%s markerListMutex lock & get list of marker for component %s \n", __FUNCTION__, componentName);
            pthread_mutex_lock(&markerListMutex);
            bool isEventingEnabled = false;
            if(markerName && eventMarkerMap) {
                if(hash_map_get(eventMarkerMap, markerName)) {
                    isEventingEnabled = true;
                }
            } else {
                EVENT_DEBUG("%s eventMarkerMap for component %s is empty \n", __FUNCTION__ , componentName );
            }
            EVENT_DEBUG("%s markerListMutex unlock\n", __FUNCTION__ );
            pthread_mutex_unlock(&markerListMutex);
            if(!isEventingEnabled) {
                EVENT_DEBUG("%s markerName %s not found in event list for component %s . Unlock markerListMutex . \n", __FUNCTION__ , markerName , componentName);
                return status;
            }
        }
        // End of event filtering

        rbusProperty_t objProperty = NULL ;
        rbusValue_t objVal, value;
        rbusSetOptions_t options = {0};
        options.commit = true;

        rbusValue_Init(&objVal);
        rbusValue_SetString(objVal, data);
        rbusProperty_Init(&objProperty, markerName, objVal);

        rbusValue_Init(&value);
        rbusValue_SetProperty(value, objProperty);

        EVENT_DEBUG("rbus_set with param [%s] with %s and value [%s]\n", T2_EVENT_PARAM, markerName, data);
        ret = rbus_set(bus_handle, T2_EVENT_PARAM, value, &options);
        if(ret != RBUS_ERROR_SUCCESS) {
            EVENT_ERROR("rbus_set Failed for [%s] with error [%d]\n", T2_EVENT_PARAM, ret);
            EVENT_DEBUG(" !!! Error !!! rbus_set Failed for [%s] with error [%d]\n", T2_EVENT_PARAM, ret);
            status = -1 ;
        }else {
            status = 0 ;
        }
        // Release all rbus data structures
        rbusValue_Release(value);
        rbusProperty_Release(objProperty);
        rbusValue_Release(objVal);

    }
#if defined(CCSP_SUPPORT_ENABLED)
    else {
        int eventDataLen = strlen(markerName) + strlen(data) + strlen(MESSAGE_DELIMITER) + 1;
        char* buffer = (char*) malloc(eventDataLen * sizeof(char));
        if(buffer) {
            snprintf(buffer, eventDataLen, "%s%s%s", markerName, MESSAGE_DELIMITER, data);
            ret = CcspBaseIf_SendTelemetryDataSignal(bus_handle, buffer);
            if(ret != CCSP_SUCCESS)
                status = -1;
            free(buffer);
        }else {
            EVENT_ERROR("Unable to allocate meory for event [%s]\n", markerName);
            status = -1 ;
        }
    }
#endif // CCSP_SUPPORT_ENABLED 
    EVENT_DEBUG("%s --out with status %d \n", __FUNCTION__, status);
    return status;
}

/**
 * Receives an rbus object as value which conatins a list of rbusPropertyObject
 * rbusProperty name will the eventName and value will be null
 */
static T2ERROR doPopulateEventMarkerList( ) {

    T2ERROR status = T2ERROR_SUCCESS;
    char deNameSpace[1][124] = {{ '\0' }};
    if(!isRbusEnabled)
        return T2ERROR_SUCCESS;

    EVENT_DEBUG("%s ++in\n", __FUNCTION__);
    rbusError_t ret = RBUS_ERROR_SUCCESS;
    rbusValue_t paramValue_t;

    if(!bus_handle && T2ERROR_SUCCESS != initMessageBus()) {
        EVENT_ERROR("Unable to get message bus handles \n");
        EVENT_DEBUG("%s --out\n", __FUNCTION__);
        return T2ERROR_FAILURE;
    }

    snprintf(deNameSpace[0], 124, "%s%s%s", T2_ROOT_PARAMETER, componentName, T2_EVENT_LIST_PARAM_SUFFIX);
    EVENT_DEBUG("rbus mode : Query marker list with data element = %s \n", deNameSpace[0]);

    pthread_mutex_lock(&markerListMutex);
    EVENT_DEBUG("Lock markerListMutex & Clean up eventMarkerMap \n");
    if(eventMarkerMap != NULL){
        hash_map_destroy(eventMarkerMap, free);
        eventMarkerMap = NULL;
    }

    ret = rbus_get(bus_handle, deNameSpace[0], &paramValue_t);
    if(ret != RBUS_ERROR_SUCCESS) {
        EVENT_ERROR("rbus mode : No event list configured in profiles %s and return value %d\n", deNameSpace[0], ret);
        pthread_mutex_unlock(&markerListMutex);
        EVENT_DEBUG("rbus mode : No event list configured in profiles %s and return value %d. Unlock markerListMutex\n", deNameSpace[0], ret);
        EVENT_DEBUG("%s --out\n", __FUNCTION__);
        return T2ERROR_SUCCESS;
    }

    rbusValueType_t type_t = rbusValue_GetType(paramValue_t);
    if(type_t != RBUS_OBJECT) {
        EVENT_ERROR("rbus mode : Unexpected data object received for %s get query \n", deNameSpace[0]);
        rbusValue_Release(paramValue_t);
        pthread_mutex_unlock(&markerListMutex);
        EVENT_DEBUG("Unlock markerListMutex\n");
        EVENT_DEBUG("%s --out\n", __FUNCTION__);
        return T2ERROR_FAILURE;
    }

    rbusObject_t objectValue = rbusValue_GetObject(paramValue_t);
    if(objectValue) {
        eventMarkerMap = hash_map_create();
        rbusProperty_t rbusPropertyList = rbusObject_GetProperties(objectValue);
        EVENT_DEBUG("\t rbus mode :  Update event map for component %s with below events : \n", componentName);
        while(NULL != rbusPropertyList) {
            const char* eventname = rbusProperty_GetName(rbusPropertyList);
            if(eventname && strlen(eventname) > 0) {
                EVENT_DEBUG("\t %s\n", eventname);
                hash_map_put(eventMarkerMap, (void*) strdup(eventname), (void*) strdup(eventname), free);
            }
            rbusPropertyList = rbusProperty_GetNext(rbusPropertyList);
        }
    }else {
        EVENT_ERROR("rbus mode : No configured event markers for %s \n", componentName);
    }
    EVENT_DEBUG("Unlock markerListMutex\n");
    pthread_mutex_unlock(&markerListMutex);
    rbusValue_Release(paramValue_t);
    EVENT_DEBUG("%s --out\n", __FUNCTION__);
    return status;

}

static void rbusEventReceiveHandler(rbusHandle_t handle, rbusEvent_t const* event, rbusEventSubscription_t* subscription) {
    (void)handle;//To fix compiler warning.
    (void)subscription;//To fix compiler warning.
    const char* eventName = event->name;
    if(eventName) {
        if(0 == strcmp(eventName, T2_PROFILE_UPDATED_NOTIFY))
            doPopulateEventMarkerList();
    }else {
        EVENT_ERROR("eventName is null \n");
    }
}

static bool isCachingRequired( ) {

    /**
     * Attempts to read from PAM before its ready creates deadlock .
     * PAM not ready is a definite case for caching the event and avoid bus traffic
     * */
    #if defined(ENABLE_RDKB_SUPPORT)
    if (access( "/tmp/pam_initialized", F_OK ) != 0) {
        return true;
    }
    #endif

    if(!initRFC()) {
        EVENT_ERROR("initRFC failed - cache the events\n");
        return true;
    }

    // If feature is disabled by RFC, caching is always disabled
    if(!isRFCT2Enable) {
        return false ;
    }

    // Always check for t2 is ready to accept events. Shutdown target can bring down t2 process at runtime
    if(access( T2_COMPONENT_READY, F_OK) == -1) {
        return true ;
    }

    if(!isRbusEnabled){
        isT2Ready = true;
    }

    if(!isT2Ready) {
        if(componentName && (0 != strcmp(componentName, "telemetry_client"))) {
            // From other binary applications in rbus mode if t2 daemon is yet to determine state of component specific config from cloud, enable cache
            if( access( T2_CONFIG_READY, F_OK) == -1 ) {
                return true;
            }else {
                rbusError_t ret = RBUS_ERROR_SUCCESS;
                doPopulateEventMarkerList();
                ret = rbusEvent_Subscribe(bus_handle, T2_PROFILE_UPDATED_NOTIFY, rbusEventReceiveHandler, "T2Event", 0);
                if(ret != RBUS_ERROR_SUCCESS) {
                    EVENT_ERROR("Unable to subscribe to event %s with rbus error code : %d\n", T2_PROFILE_UPDATED_NOTIFY, ret);
                    EVENT_DEBUG("Unable to subscribe to event %s with rbus error code : %d\n", T2_PROFILE_UPDATED_NOTIFY, ret);
                }
                isT2Ready = true;
            }
        }else {
            isT2Ready = true;
        }
    }

    return false;
}

static int report_or_cache_data(char* telemetry_data, char* markerName) {
    int ret = 0;
    pthread_t tid;
    pthread_mutex_lock(&eventMutex);
    if(isCachingRequired()) {
        EVENT_DEBUG("Caching the event : %s \n", telemetry_data);
        int eventDataLen = strlen(markerName) + strlen(telemetry_data) + strlen(MESSAGE_DELIMITER) + 1;
        char* buffer = (char*) malloc(eventDataLen * sizeof(char));
        if(buffer) {
            // Caching format needs to be same for operation between rbus/dbus modes across reboots
            snprintf(buffer, eventDataLen, "%s%s%s", markerName, MESSAGE_DELIMITER, telemetry_data);
            pthread_create(&tid, NULL, cacheEventToFile, (void *)buffer);
        }
        pthread_mutex_unlock(&eventMutex);
        return T2ERROR_SUCCESS ;
    }
    pthread_mutex_unlock(&eventMutex);

    if(isT2Ready) {
        // EVENT_DEBUG("T2: Sending event : %s\n", telemetry_data);
        ret = filtered_event_send(telemetry_data, markerName);
        if(0 != ret) {
            EVENT_ERROR("%s:%d, T2:telemetry data send failed, status = %d \n", __func__, __LINE__, ret);
        }
    }
    return ret;
}

/**
 * Initialize the component name with unique name
 */
void t2_init(char *component) {
    componentName = strdup(component);
}

void t2_uninit(void) {
    if(componentName) {
        free(componentName);
        componentName = NULL ;
    }

    if(isRbusEnabled)
        rBusInterface_Uninit();

    uninitMutex();
}

T2ERROR t2_event_s(char* marker, char* value) {

    int ret;
    T2ERROR retStatus = T2ERROR_FAILURE ;
    EVENT_DEBUG("%s ++in\n", __FUNCTION__);

    if(componentName == NULL){
        EVENT_DEBUG("%s:%d, T2:component with pid = %d is trying to send event %s with value %s without component name \n", __func__, __LINE__, (int) getpid(), marker,value);
        return T2ERROR_COMPONENT_NULL;
    }
    initMutex();
    pthread_mutex_lock(&sMutex);
    if ( NULL == marker || NULL == value) {
        EVENT_ERROR("%s:%d Error in input parameters \n", __func__, __LINE__);
        pthread_mutex_unlock(&sMutex);
        EVENT_DEBUG("%s --out\n", __FUNCTION__);
        return T2ERROR_FAILURE;
    }

    EVENT_DEBUG("marker = %s : value = %s \n", marker, value);
    // If data is empty should not be sending the empty marker over bus
    if ( 0 == strlen(value) || strcmp(value, "0") == 0 ) {
        pthread_mutex_unlock(&sMutex);
        return T2ERROR_SUCCESS;
    }
    if( value[strlen(value)-1] == '\n' ){
        value[strlen(value)-1] = '\0';
    }
    ret = report_or_cache_data(value, marker);
    if(ret != -1) {
        retStatus = T2ERROR_SUCCESS;
    }
    pthread_mutex_unlock(&sMutex);
    EVENT_DEBUG("%s --out\n", __FUNCTION__);
    return retStatus;
}

T2ERROR t2_event_f(char* marker, double value) {

    int ret;
    T2ERROR retStatus = T2ERROR_FAILURE ;
    EVENT_DEBUG("%s ++in\n", __FUNCTION__);

    if(componentName == NULL){
        EVENT_DEBUG("%s:%d, T2:component with pid = %d is trying to send event %s with value %lf without component name \n", __func__, __LINE__, (int) getpid(), marker,value);
        return T2ERROR_COMPONENT_NULL;
    }

     initMutex();
     pthread_mutex_lock(&fMutex);
     if ( NULL == marker ) {
         EVENT_ERROR("%s:%d Error in input parameters \n", __func__, __LINE__);
         pthread_mutex_unlock(&fMutex);
         EVENT_DEBUG("%s --out\n", __FUNCTION__);
         return T2ERROR_FAILURE;
     }

     EVENT_DEBUG("marker = %s : value = %f \n", marker, value);
     char *buffer = (char*) malloc(MAX_DATA_LEN * sizeof(char));
     if (NULL != buffer) {
         snprintf(buffer, MAX_DATA_LEN, "%f", value);
         ret = report_or_cache_data(buffer, marker);
         if (ret != -1) {
             retStatus = T2ERROR_SUCCESS ;
         }
         free(buffer);
     } else {
         EVENT_ERROR("%s:%d Error unable to allocate memory \n", __func__, __LINE__);
     }
     pthread_mutex_unlock(&fMutex);
     EVENT_DEBUG("%s --out\n", __FUNCTION__);
     return retStatus ;
}

T2ERROR t2_event_d(char* marker, int value) {

    int ret;
    T2ERROR retStatus = T2ERROR_FAILURE ;
    EVENT_DEBUG("%s ++in\n", __FUNCTION__);

    if(componentName == NULL){
        EVENT_DEBUG("%s:%d, T2:component with pid = %d is trying to send event %s with value %d without component name \n", __func__, __LINE__, (int) getpid(), marker,value);
        return T2ERROR_COMPONENT_NULL;
    }

     initMutex();
     pthread_mutex_lock(&dMutex);
     if ( NULL == marker ) {
         EVENT_ERROR("%s:%d Error in input parameters \n", __func__, __LINE__);
         pthread_mutex_unlock(&dMutex);
         EVENT_DEBUG("%s --out\n", __FUNCTION__);
         return T2ERROR_FAILURE;
     }

     EVENT_DEBUG("marker = %s : value = %d \n", marker, value);

     if (value == 0) {  // Requirement from field triage to ignore reporting 0 values
         pthread_mutex_unlock(&dMutex);
         EVENT_DEBUG("%s Value is 0 , do not event .\n", __FUNCTION__);
         EVENT_DEBUG("%s --out\n", __FUNCTION__);
         return T2ERROR_SUCCESS;
     }

     char *buffer = (char*) malloc(MAX_DATA_LEN * sizeof(char));
     if (NULL != buffer) {
         snprintf(buffer, MAX_DATA_LEN, "%d", value);
         ret = report_or_cache_data(buffer, marker);
         if (ret != -1) {
             retStatus = T2ERROR_SUCCESS ;
         }
         free(buffer);
     } else {
         EVENT_ERROR("%s:%d Error unable to allocate memory \n", __func__, __LINE__);
     }
     EVENT_DEBUG("%s --out\n", __FUNCTION__);
     pthread_mutex_unlock(&dMutex);
     return retStatus ;
}
