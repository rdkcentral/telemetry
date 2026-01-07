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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <poll.h>
#include <fcntl.h>

#if defined(CCSP_SUPPORT_ENABLED)
#include <ccsp/ccsp_memory.h>
#include <ccsp/ccsp_custom.h>
#include <ccsp/ccsp_base_api.h>
#endif
#include <rbus/rbus.h>

#include "telemetry_busmessage_sender.h"
#include "t2_transport_interface.h"

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
#define T2_TCP_PORT 12345
#define SERVER_IP "127.0.0.1"

const char destCompName[64] = "eRT.com.cisco.spvtg.ccsp.pam";
const char destCompPath[64] = "/com/cisco/spvtg/ccsp/pam";

static char *componentName = NULL;

static bool isRbusEnabled = false ;
static pthread_mutex_t initMtx = PTHREAD_MUTEX_INITIALIZER;
static bool isMutexInitialized = false ;

static pthread_mutexattr_t mutexAttr;

static pthread_mutex_t eventMutex ;
static pthread_mutex_t sMutex ;
static pthread_mutex_t fMutex ;
static pthread_mutex_t dMutex ;
static pthread_mutex_t FileCacheMutex ;
static pthread_mutex_t markerListMutex ;
static pthread_mutex_t loggerMutex ;

typedef enum {
    T2_REQ_SUBSCRIBE = 1,
    T2_REQ_PROFILE_DATA = 2,
    T2_REQ_MARKER_LIST = 3,
    T2_REQ_DAEMON_STATUS = 4,
    T2_MSG_EVENT_DATA = 5
} T2RequestType;

typedef struct {
    uint32_t request_type;
    uint32_t data_length;
    uint32_t client_id;
    uint32_t last_known_version;
} T2RequestHeader;

// Response header for server responses
typedef struct {
    uint32_t response_status; // 0=success, 1=failure
    uint32_t data_length;     // Length of response data
    uint32_t sequence_id;     // Matches request sequence
    uint32_t reserved;        // For future use
} T2ResponseHeader;

static hash_map_t *clientEventMarkerMap = NULL;
static pthread_mutex_t clientMarkerMutex = PTHREAD_MUTEX_INITIALIZER;

void t2_parse_and_store_markers(const char* marker_data);

static void EVENT_DEBUG(char* format, ...)
{

    if(access(ENABLE_DEBUG_FLAG, F_OK) == -1)
    {
        return;
    }

    FILE *logHandle = NULL ;

    pthread_mutex_lock(&loggerMutex);
    logHandle = fopen(SENDER_LOG_FILE, "a+");
    if(logHandle)
    {
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

static void initMutex()
{
    pthread_mutex_lock(&initMtx);
    if ( !isMutexInitialized )
    {
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

static void uninitMutex()
{
    pthread_mutex_lock(&initMtx);
    pthread_mutex_lock(&FileCacheMutex);
    pthread_mutex_unlock(&FileCacheMutex);
    if ( isMutexInitialized )
    {
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
    if (paramNames == NULL || paramNamesCount <= 0)
    {
        EVENT_ERROR("paramNames is NULL or paramNamesCount <= 0 - returning\n");
        return T2ERROR_INVALID_ARGS;
    }

    int ret = CcspBaseIf_getParameterValues(bus_handle, destCompName, (char*)destCompPath, paramNames,
                                            paramNamesCount, valSize, valStructs);
    if (ret != CCSP_SUCCESS)
    {
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



/**
 * Initialize the component name with unique name (SUPPORTS ALL 3 MODES)
 */
void t2_init(char *component)
{
    componentName = strdup(component);
    initMutex();
    
    // Set transport mode from environment if available
    t2_set_transport_mode_from_env();
    
    const char* mode_name = t2_get_transport_mode_name();
    EVENT_DEBUG("Initializing T2 for component: %s (transport: %s)\n", component, mode_name);
    printf("Initializing T2 for component: %s (transport: %s)\n", component, mode_name);

    // Initialize transport layer using unified interface
    if (t2_communication_init(componentName) != T2ERROR_SUCCESS) {
        EVENT_ERROR("Transport initialization failed for %s\n", component);
        printf("Transport initialization failed for %s\n", component);
    } else {
        EVENT_DEBUG("Successfully initialized transport for %s\n", component);
        printf("Successfully initialized transport for %s\n", component);
    }
}

void t2_uninit(void)
{
    EVENT_ERROR("Un Initializing T2 communication\n");
    
    // Use the new unified communication cleanup
    t2_communication_cleanup();
    
    EVENT_ERROR("Un Initialized T2 communication\n");

    if(componentName)
    {
        free(componentName);
        componentName = NULL ;
    }

    if(isRbusEnabled)
    {
        rBusInterface_Uninit();
    }

    // Clean up client event marker map if it exists
    if (clientEventMarkerMap) {
        pthread_mutex_lock(&clientMarkerMutex);
        hash_map_destroy(clientEventMarkerMap, free);
        clientEventMarkerMap = NULL;
        pthread_mutex_unlock(&clientMarkerMutex);
    }

    uninitMutex();
}

T2ERROR t2_event_s(const char* marker, const char* value)
{

    int ret;
    T2ERROR retStatus = T2ERROR_FAILURE ;
    EVENT_DEBUG("%s ++in\n", __FUNCTION__);
    char* strvalue = NULL;
    if(componentName == NULL)
    {
        EVENT_DEBUG("%s:%d, T2:component with pid = %d is trying to send event %s with value %s without component name \n", __func__, __LINE__, (int) getpid(), marker, value);
        return T2ERROR_COMPONENT_NULL;
    }
    initMutex();
    pthread_mutex_lock(&sMutex);
    if ( NULL == marker || NULL == value)
    {
        EVENT_ERROR("%s:%d Error in input parameters \n", __func__, __LINE__);
        pthread_mutex_unlock(&sMutex);
        EVENT_DEBUG("%s --out\n", __FUNCTION__);
        return T2ERROR_FAILURE;
    }

    EVENT_DEBUG("marker = %s : value = %s \n", marker, value);
    // If data is empty should not be sending the empty marker over bus
    if ( 0 == strlen(value) || strcmp(value, "0") == 0 )
    {
        pthread_mutex_unlock(&sMutex);
        return T2ERROR_SUCCESS;
    }
    strvalue = strdup(value);
    if( strvalue[strlen(strvalue) - 1] == '\n' )
    {
        strvalue[strlen(strvalue) - 1] = '\0';
    }
    ret = report_or_cache_data(strvalue, marker);
    if(strvalue != NULL)
    {
        free(strvalue);
    }
    if(ret != -1)
    {
        retStatus = T2ERROR_SUCCESS;
    }
    pthread_mutex_unlock(&sMutex);
    EVENT_DEBUG("%s --out\n", __FUNCTION__);
    return retStatus;
}

T2ERROR t2_event_f(const char* marker, double value)
{

    int ret;
    T2ERROR retStatus = T2ERROR_FAILURE ;
    EVENT_DEBUG("%s ++in\n", __FUNCTION__);

    if(componentName == NULL)
    {
        EVENT_DEBUG("%s:%d, T2:component with pid = %d is trying to send event %s with value %lf without component name \n", __func__, __LINE__, (int) getpid(), marker, value);
        return T2ERROR_COMPONENT_NULL;
    }

    initMutex();
    pthread_mutex_lock(&fMutex);
    if ( NULL == marker )
    {
        EVENT_ERROR("%s:%d Error in input parameters \n", __func__, __LINE__);
        pthread_mutex_unlock(&fMutex);
        EVENT_DEBUG("%s --out\n", __FUNCTION__);
        return T2ERROR_FAILURE;
    }

    EVENT_DEBUG("marker = %s : value = %f \n", marker, value);
    char *buffer = (char*) malloc(MAX_DATA_LEN * sizeof(char));
    if (NULL != buffer)
    {
        snprintf(buffer, MAX_DATA_LEN, "%f", value);
        ret = report_or_cache_data(buffer, marker);
        if (ret != -1)
        {
            retStatus = T2ERROR_SUCCESS ;
        }
        free(buffer);
    }
    else
    {
        EVENT_ERROR("%s:%d Error unable to allocate memory \n", __func__, __LINE__);
    }
    pthread_mutex_unlock(&fMutex);
    EVENT_DEBUG("%s --out\n", __FUNCTION__);
    return retStatus ;
}

T2ERROR t2_event_d(const char* marker, int value)
{

    int ret;
    T2ERROR retStatus = T2ERROR_FAILURE ;
    EVENT_DEBUG("%s ++in\n", __FUNCTION__);
    printf("%s ++in\n", __FUNCTION__);

    if(componentName == NULL)
    {
        EVENT_DEBUG("%s:%d, T2:component with pid = %d is trying to send event %s with value %d without component name \n", __func__, __LINE__, (int) getpid(), marker, value);
        return T2ERROR_COMPONENT_NULL;
    }

    initMutex();
    pthread_mutex_lock(&dMutex);
    if ( NULL == marker )
    {
        EVENT_ERROR("%s:%d Error in input parameters \n", __func__, __LINE__);
        pthread_mutex_unlock(&dMutex);
        EVENT_DEBUG("%s --out\n", __FUNCTION__);
        return T2ERROR_FAILURE;
    }

    EVENT_DEBUG("marker = %s : value = %d \n", marker, value);
    printf("marker = %s : value = %d \n", marker, value);

    if (value == 0)    // Requirement from field triage to ignore reporting 0 values
    {
        pthread_mutex_unlock(&dMutex);
        EVENT_DEBUG("%s Value is 0 , do not event .\n", __FUNCTION__);
        EVENT_DEBUG("%s --out\n", __FUNCTION__);
        return T2ERROR_SUCCESS;
    }

    char *buffer = (char*) malloc(MAX_DATA_LEN * sizeof(char));
    if (NULL != buffer)
    {
        snprintf(buffer, MAX_DATA_LEN, "%d", value);
        ret = report_or_cache_data(buffer, marker);
        if (ret != -1)
        {
            retStatus = T2ERROR_SUCCESS ;
        }
        free(buffer);
    }
    else
    {
        EVENT_ERROR("%s:%d Error unable to allocate memory \n", __func__, __LINE__);
    }
    EVENT_DEBUG("%s --out\n", __FUNCTION__);
    pthread_mutex_unlock(&dMutex);
    return retStatus ;
}
