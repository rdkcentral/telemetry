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
#include <dbus/dbus.h>
#if defined(RBUS_SUPPORT_ENABLED)
#include <rbus/rbus.h>
#endif

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

/* D-Bus Configuration */
#define T2_DBUS_SERVICE_NAME        "telemetry.t2"
#define T2_DBUS_OBJECT_PATH         "/telemetry/t2"
#define T2_DBUS_INTERFACE_NAME      "telemetry.t2.interface"
#define T2_DBUS_EVENT_INTERFACE_NAME "telemetry.t2.event.interface"

static const char* CCSP_FIXED_COMP_ID = "com.cisco.spvtg.ccsp.t2commonlib" ;

static char *componentName = NULL;
static void *bus_handle = NULL;              /* For method calls (main thread) */
static void *signal_bus_handle = NULL;       /* For signal listening (event thread) */
static bool isRFCT2Enable = false ;
static bool isT2Ready = false;
#if defined(RBUS_SUPPORT_ENABLED)
static bool isRbusEnabled = false ;
#endif
static bool isDbusEnabled = false ;
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

// D-Bus event loop thread
static pthread_t dbus_event_thread;
static bool dbus_event_thread_running = false;

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
        struct timespec ts;
        struct tm timeinfo;

        if(clock_gettime(CLOCK_REALTIME, &ts) == -1)
        {
            fclose(logHandle);
            pthread_mutex_unlock(&loggerMutex);
            return;
        }

        char timeBuffer[24] = { '\0' };
        long msecs;

        localtime_r(&ts.tv_sec, &timeinfo);
        msecs = ts.tv_nsec / 1000000;
        strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
        snprintf(timeBuffer + strlen(timeBuffer), sizeof(timeBuffer) - strlen(timeBuffer), ".%03ld", msecs);
        fprintf(logHandle, "%s : ", timeBuffer);
        va_list argList;
        va_start(argList, format);
        vfprintf(logHandle, format, argList);
        va_end(argList);
        fclose(logHandle);
    }
    pthread_mutex_unlock(&loggerMutex);

}
#if 0
#define EVENT_DEBUG(format, ...) do { \
    struct timespec ts; \
    struct tm timeinfo; \
    char timeBuffer[32]; \
    if (clock_gettime(CLOCK_REALTIME, &ts) == 0) { \
        localtime_r(&ts.tv_sec, &timeinfo); \
        long msecs = ts.tv_nsec / 1000000; \
        strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", &timeinfo); \
        snprintf(timeBuffer + strlen(timeBuffer), sizeof(timeBuffer) - strlen(timeBuffer), ".%03ld", msecs); \
        fprintf(stderr, "[%s] T2DEBUG:%s %s:%d: ", timeBuffer, __func__ , __FILE__, __LINE__ ); \
    } else { \
        fprintf(stderr, "T2DEBUG:%s %s:%d: ", __func__ , __FILE__, __LINE__ ); \
    } \
    fprintf(stderr, (format), ##__VA_ARGS__ ); \
} while(0)
#endif

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

#if defined(RBUS_SUPPORT_ENABLED)
static void rBusInterface_Uninit( )
{
    rbus_close(bus_handle);
}
#endif

static void dBusInterface_Uninit(void)
{
    if(isDbusEnabled)
    {
        // Stop D-Bus event loop thread
        if (dbus_event_thread_running)
        {
            EVENT_DEBUG("D-Bus: Stopping event loop thread\n");
            dbus_event_thread_running = false;
            // Thread is detached and will exit on its own - no need to wait
        }
        
        // Flush all pending messages before closing connections
        if (bus_handle)
        {
            EVENT_DEBUG("D-Bus: Flushing pending method call messages\n");
            dbus_connection_flush((DBusConnection*)bus_handle);
        }
        
        if (signal_bus_handle)
        {
            EVENT_DEBUG("D-Bus: Flushing pending signal messages\n");
            dbus_connection_flush((DBusConnection*)signal_bus_handle);
            dbus_connection_unref((DBusConnection*)signal_bus_handle);
            signal_bus_handle = NULL;
        }
        
        if (bus_handle)
        {
            dbus_connection_unref((DBusConnection*)bus_handle);
            bus_handle = NULL;
        }

        isDbusEnabled = false;
    }
}

static int dbus_checkStatus(void)
{
    // Check if D-Bus is available by attempting to connect
    DBusError error;
    dbus_error_init(&error);
    DBusConnection *test_conn = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    
    if (dbus_error_is_set(&error))
    {
        dbus_error_free(&error);
        return -1; // D-Bus not available
    }
    
    if (test_conn)
    {
        dbus_connection_unref(test_conn);
        isDbusEnabled = true;
        return 0; // D-Bus available
    }
    
    return -1;
}

static T2ERROR dbus_getGetOperationalStatus(const char* paramName, uint32_t* value)
{
    if (!paramName || !value)
    {
        return T2ERROR_INVALID_ARGS;
    }
    
    if (!bus_handle)
    {
        return T2ERROR_FAILURE;
    }
    
    // D-Bus method call to get uint32 parameter
    DBusMessage *msg = NULL;
    DBusMessage *reply = NULL;
    DBusError error;
    dbus_error_init(&error);
    
    msg = dbus_message_new_method_call(T2_DBUS_SERVICE_NAME,
                                       T2_DBUS_OBJECT_PATH,
                                       T2_DBUS_INTERFACE_NAME,
                                       "GetOperationalStatus");
    if (!msg)
    {
        EVENT_ERROR("%s:%d, D-Bus failed to create method call message\n", __func__, __LINE__);
        return T2ERROR_FAILURE;
    }
    
    if (!dbus_message_append_args(msg, DBUS_TYPE_STRING, &paramName, DBUS_TYPE_INVALID))
    {
        dbus_message_unref(msg);
        return T2ERROR_FAILURE;
    }
    
    // Timeout: 1000ms - GetOperationalStatus should respond quickly
    // This prevents hanging if server is down/unresponsive
    reply = dbus_connection_send_with_reply_and_block((DBusConnection*)bus_handle, msg, 1000, &error);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&error))
    {
        EVENT_ERROR("%s:%d, D-Bus error: %s\n", __func__, __LINE__, error.message);
        dbus_error_free(&error);
        return T2ERROR_FAILURE;
    }
    
    if (!reply)
    {
        return T2ERROR_FAILURE;
    }

    if (dbus_message_get_args(reply, &error, DBUS_TYPE_UINT32, value, DBUS_TYPE_INVALID))
    {
        dbus_message_unref(reply);
        EVENT_DEBUG("%s:%d, D-Bus got uint32 value: %u\n", __func__, __LINE__, *value);
        return T2ERROR_SUCCESS;
    }
    else
    {
        if (dbus_error_is_set(&error))
        {
            EVENT_ERROR("%s:%d, D-Bus error: %s\n", __func__, __LINE__, error.message);
            dbus_error_free(&error);
        }
        dbus_message_unref(reply);
        return T2ERROR_FAILURE;
    }
}

static T2ERROR initMessageBus( )
{
    EVENT_DEBUG("%s ++in\n", __FUNCTION__);
    T2ERROR status = T2ERROR_SUCCESS;
    char* component_id = (char*)CCSP_FIXED_COMP_ID;
#if defined(CCSP_SUPPORT_ENABLED)
    char *pCfg = (char*)CCSP_MSG_BUS_CFG;
#endif

#if defined(RBUS_SUPPORT_ENABLED)
    if(RBUS_ENABLED == rbus_checkStatus())
    {
        // EVENT_DEBUG("%s:%d, T2:rbus is enabled\n", __func__, __LINE__);
        char commonLibName[124] = { '\0' };
        // Bus handles should be unique across the system
        if(componentName)
        {
            snprintf(commonLibName, 124, "%s%s", "t2_lib_", componentName);
        }
        else
        {
            snprintf(commonLibName, 124, "%s", component_id);
        }
        rbusError_t status_rbus =  rbus_open((rbusHandle_t*) &bus_handle, commonLibName);
        if(status_rbus != RBUS_ERROR_SUCCESS)
        {
            EVENT_ERROR("%s:%d, init using component name %s failed with error code %d \n", __func__, __LINE__, commonLibName, status);
            status = T2ERROR_FAILURE;
        }
        isRbusEnabled = true;
    }
    else
#endif
    if(0 == dbus_checkStatus())
    {
        // D-Bus is available - initialize threading support first
        if (!dbus_threads_init_default())
        {
            EVENT_ERROR("%s:%d, Failed to initialize D-Bus threading\n", __func__, __LINE__);
            return T2ERROR_FAILURE;
        }
        EVENT_DEBUG("%s:%d, D-Bus threading initialized\n", __func__, __LINE__);
        
        char dbusName[124] = { '\0' };
        char signalDbusName[124] = { '\0' };
        if(componentName)
        {
            snprintf(dbusName, 124, "telemetry.t2.lib_%s", componentName);
            snprintf(signalDbusName, 124, "telemetry.t2.lib_%s_signals", componentName);
        }
        else
        {
            snprintf(dbusName, 124, "%s", component_id);
            snprintf(signalDbusName, 124, "%s_signals", component_id);
        }
        
        DBusError error;
        dbus_error_init(&error);
        
        /* Initialize METHOD call connection */
        bus_handle = (void*)dbus_bus_get(DBUS_BUS_SYSTEM, &error);
        
        if (dbus_error_is_set(&error))
        {
            EVENT_ERROR("%s:%d, D-Bus method call connection init failed: %s\n", __func__, __LINE__, error.message);
            dbus_error_free(&error);
            status = T2ERROR_FAILURE;
        }
            
        /*  Initialize SIGNAL connection separately */
        signal_bus_handle = (void*)dbus_bus_get(DBUS_BUS_SYSTEM, &error);
        
        if (dbus_error_is_set(&error))
        {
            EVENT_ERROR("%s:%d, D-Bus signal connection init failed: %s\n", __func__, __LINE__, error.message);
            dbus_error_free(&error);
            /* Continue without signal support */
            signal_bus_handle = NULL;
        }

        if (signal_bus_handle && bus_handle)
        {
            status = T2ERROR_SUCCESS;
            EVENT_DEBUG("%s:%d, D-Bus initialized successfully with separate connections\n", __func__, __LINE__);
        }
        else
        {
            EVENT_ERROR("%s:%d, D-Bus initialization failed\n", __func__, __LINE__);
            status = T2ERROR_FAILURE;
        }
    }
    else
    {
        EVENT_ERROR("%s:%d, T2:No supported dbus available\n", __func__, __LINE__);
        status = T2ERROR_FAILURE;
    }
#if defined(CCSP_SUPPORT_ENABLED)
    else
    {
        int ret = 0 ;
        ret = CCSP_Message_Bus_Init(component_id, pCfg, &bus_handle, (CCSP_MESSAGE_BUS_MALLOC)Ansc_AllocateMemory_Callback, Ansc_FreeMemory_Callback);
        if(ret == -1)
        {
            EVENT_ERROR("%s:%d, T2:initMessageBus failed\n", __func__, __LINE__);
            status = T2ERROR_FAILURE ;
        }
        else
        {
            status = T2ERROR_SUCCESS ;
        }
    }
#endif // CCSP_SUPPORT_ENABLED 
    EVENT_DEBUG("%s --out\n", __FUNCTION__);
    return status;
}

#if defined(RBUS_SUPPORT_ENABLED)
static T2ERROR getRbusParameterVal(const char* paramName, char **paramValue)
{

    rbusError_t ret = RBUS_ERROR_SUCCESS;
    rbusValue_t paramValue_t;
    rbusValueType_t rbusValueType ;
    char *stringValue = NULL;
#if 0
    rbusSetOptions_t opts;
    opts.commit = true;
#endif

    if(!bus_handle && T2ERROR_SUCCESS != initMessageBus())
    {
        return T2ERROR_FAILURE;
    }

    ret = rbus_get(bus_handle, paramName, &paramValue_t);
    if(ret != RBUS_ERROR_SUCCESS)
    {
        EVENT_ERROR("Unable to get %s\n", paramName);
        return T2ERROR_FAILURE;
    }
    rbusValueType = rbusValue_GetType(paramValue_t);
    if(rbusValueType == RBUS_BOOLEAN)
    {
        if (rbusValue_GetBoolean(paramValue_t))
        {
            stringValue = strdup("true");
        }
        else
        {
            stringValue = strdup("false");
        }
    }
    else
    {
        stringValue = rbusValue_ToString(paramValue_t, NULL, 0);
    }
    *paramValue = stringValue;
    rbusValue_Release(paramValue_t);

    return T2ERROR_SUCCESS;
}
#endif

T2ERROR getParamValue(const char* paramName, char **paramValue)
{
    T2ERROR ret = T2ERROR_FAILURE ;
#if defined(RBUS_SUPPORT_ENABLED)
    if(isRbusEnabled)
    {
        ret = getRbusParameterVal(paramName, paramValue);
    }
    else
#endif
#if defined(CCSP_SUPPORT_ENABLED)
    {
        ret = getCCSPParamVal(paramName, paramValue);
    }
#else
    {
        // D-Bus mode - not implemented for parameter get
        (void)paramName;
        (void)paramValue;
        ret = T2ERROR_FAILURE;
    }
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
    pthread_detach(pthread_self());
    int ch;
    int count = 0;
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
        if (ret != 0)
        {
            EVENT_ERROR("%s:%d, T2:close failed with error %d\n", __func__, __LINE__, ret);
        }
        free(telemetry_data);
        return NULL;
    }

    FILE *fp = fopen(T2_CACHE_FILE, "a");
    if (fp == NULL)
    {
        EVENT_ERROR("%s: File open error %s\n", __FUNCTION__, T2_CACHE_FILE);
        goto unlock;
    }

    fs = fopen(T2_CACHE_FILE, "r");
    if (fs != NULL)
    {
        while ((ch = fgetc(fs)) != EOF)
        {
            if (ch == '\n')
            {
                count++;
            }
        }

        //If the file is not empty and does not contain a newline, call it one line
        if (count == 0 && ftell(fs) > 0)
        {
            count++;
        }
        fclose(fs);
        fs = NULL;
    }
    if(count < MAX_EVENT_CACHE)
    {
        fprintf(fp, "%s\n", telemetry_data);
    }
    else
    {
        EVENT_DEBUG("Reached Max cache limit of 200, Caching is not done\n");
    }
    fclose(fp);

unlock:

    fl.l_type = F_UNLCK;  /* set to unlock same region */
    if (fcntl(fd, F_SETLK, &fl) == -1)
    {
        EVENT_ERROR("fcntl failed \n");
    }
    int ret = close(fd);
    if (ret != 0)
    {
        EVENT_ERROR("%s:%d, T2:close failed with error %d\n", __func__, __LINE__, ret);
    }
    pthread_mutex_unlock(&FileCacheMutex);
    free(telemetry_data);
    return NULL;
}

static bool initRFC( )
{
    bool status = true ;
    // Check for RFC and proceed - if true - else return now .
    //TODO: Implement RFC check here
    if(!bus_handle && !signal_bus_handle)
    {
        EVENT_DEBUG("%s:%d, T2: Initializing Message Bus\n", __func__, __LINE__);
        if(initMessageBus() != 0)
        {
            EVENT_ERROR("initMessageBus failed\n");
            status = false;
        }
        else
        {
            EVENT_DEBUG("initMessageBus successful\n");
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
int filtered_event_send(const char* data, const char *markerName)
{
    int status = 0 ;
    EVENT_DEBUG("%s ++in\n", __FUNCTION__);
    if(!bus_handle)
    {
        EVENT_ERROR("bus_handle is null .. exiting !!! \n");
        return status;
    }

#if defined(RBUS_SUPPORT_ENABLED)
    if(isRbusEnabled)
    {

        // Filter data from marker list
        if(componentName && (0 != strcmp(componentName, T2_SCRIPT_EVENT_COMPONENT)))   // Events from scripts needs to be sent without filtering
        {

            EVENT_DEBUG("%s markerListMutex lock & get list of marker for component %s \n", __FUNCTION__, componentName);
            pthread_mutex_lock(&markerListMutex);
            bool isEventingEnabled = false;
            if(markerName && eventMarkerMap)
            {
                if(hash_map_get(eventMarkerMap, markerName))
                {
                    isEventingEnabled = true;
                }
            }
            else
            {
                EVENT_DEBUG("%s eventMarkerMap for component %s is empty \n", __FUNCTION__, componentName );
            }
            EVENT_DEBUG("%s markerListMutex unlock\n", __FUNCTION__ );
            pthread_mutex_unlock(&markerListMutex);
            if(!isEventingEnabled)
            {
                EVENT_DEBUG("%s markerName %s not found in event list for component %s . Unlock markerListMutex . \n", __FUNCTION__, markerName, componentName);
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
        if(ret != RBUS_ERROR_SUCCESS)
        {
            EVENT_ERROR("rbus_set Failed for [%s] with error [%d]\n", T2_EVENT_PARAM, ret);
            EVENT_DEBUG(" !!! Error !!! rbus_set Failed for [%s] with error [%d]\n", T2_EVENT_PARAM, ret);
            status = -1 ;
        }
        else
        {
            status = 0 ;
        }
        // Release all rbus data structures
        rbusValue_Release(value);
        rbusProperty_Release(objProperty);
        rbusValue_Release(objVal);

    }
    else
#endif
    if(isDbusEnabled && bus_handle)
    {
        // Filter data from marker list
        if(componentName && (0 != strcmp(componentName, T2_SCRIPT_EVENT_COMPONENT)))   // Events from scripts needs to be sent without filtering
        {
            EVENT_DEBUG("markerListMutex lock & component %s , marker %s\n", componentName, markerName);
            pthread_mutex_lock(&markerListMutex);
            bool isEventingEnabled = false;
            if(markerName && eventMarkerMap)
            {
                if(hash_map_get(eventMarkerMap, markerName))
                {
                    isEventingEnabled = true;
                }
            }
            else
            {
                EVENT_DEBUG("eventMarkerMap for component %s is empty \n", componentName );
            }
            EVENT_DEBUG("%s markerListMutex unlock\n", __FUNCTION__ );
            pthread_mutex_unlock(&markerListMutex);
            if(!isEventingEnabled)
            {
                EVENT_DEBUG("markerName %s not found in event list for component %s \n", markerName, componentName);
                return status;
            }
        }

        // D-Bus method call to send event
        DBusMessage *msg = NULL;
        DBusMessage *reply = NULL;
        DBusError error;
        dbus_error_init(&error);
        
        msg = dbus_message_new_method_call(T2_DBUS_SERVICE_NAME,
                                           T2_DBUS_OBJECT_PATH,
                                           T2_DBUS_INTERFACE_NAME,
                                           "SendT2Event");
        if (!msg)
        {
            EVENT_ERROR("Failed to create D-Bus method call message\n");
            status = -1;
        }
        else
        {
            if (!dbus_message_append_args(msg,
                                         DBUS_TYPE_STRING, &markerName,
                                         DBUS_TYPE_STRING, &data,
                                         DBUS_TYPE_INVALID))
            {
                EVENT_ERROR("Failed to append D-Bus method call arguments\n");
                dbus_message_unref(msg);
                status = -1;
            }
            else
            {
                // Send method call and wait for reply with timeout (1000 ms)
                reply = dbus_connection_send_with_reply_and_block((DBusConnection*)bus_handle, msg, 1000, &error);
                dbus_message_unref(msg);
                
                if (dbus_error_is_set(&error))
                {
                    EVENT_ERROR("D-Bus method call failed: %s\n", error.message);
                    dbus_error_free(&error);
                    status = -1;
                }
                else if (!reply)
                {
                    EVENT_ERROR("No reply received from SendT2Event\n");
                    status = -1;
                }
                else
                {
                    // Parse boolean success status from reply
                    dbus_bool_t success = FALSE;
                    if (dbus_message_get_args(reply, &error,
                                             DBUS_TYPE_BOOLEAN, &success,
                                             DBUS_TYPE_INVALID))
                    {
                        if (success)
                        {
                            EVENT_DEBUG("SendT2Event succeeded for marker [%s] with data [%s]\n", markerName, data);
                            status = 0;
                        }
                        else
                        {
                            EVENT_ERROR("SendT2Event returned failure for marker [%s]\n", markerName);
                            status = -1;
                        }
                    }
                    else
                    {
                        EVENT_ERROR("Failed to parse reply: %s\n", error.message);
                        dbus_error_free(&error);
                        status = -1;
                    }
                    dbus_message_unref(reply);
                }
            }
        }
    }
    else
    {
        EVENT_ERROR("No supported bus available for sending event\n");
        status = -1 ;
    }
#if defined(CCSP_SUPPORT_ENABLED)
    else
    {
        int eventDataLen = strlen(markerName) + strlen(data) + strlen(MESSAGE_DELIMITER) + 1;
        char* buffer = (char*) malloc(eventDataLen * sizeof(char));
        if(buffer)
        {
            snprintf(buffer, eventDataLen, "%s%s%s", markerName, MESSAGE_DELIMITER, data);
            int ret = CcspBaseIf_SendTelemetryDataSignal(bus_handle, buffer);
            if(ret != CCSP_SUCCESS)
            {
                status = -1;
            }
            free(buffer);
        }
        else
        {
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
static T2ERROR doPopulateEventMarkerList( )
{

#if defined(RBUS_SUPPORT_ENABLED)
    char deNameSpace[1][124] = {{ '\0' }};
    if(!isRbusEnabled)
    {
        // Fall through to D-Bus implementation
    }
    else
    {
        EVENT_DEBUG("%s ++in\n", __FUNCTION__);
        rbusError_t ret = RBUS_ERROR_SUCCESS;
        rbusValue_t paramValue_t;

        if(!bus_handle && T2ERROR_SUCCESS != initMessageBus())
        {
            EVENT_ERROR("Unable to get message bus handles \n");
            EVENT_DEBUG("%s --out\n", __FUNCTION__);
            return T2ERROR_FAILURE;
        }

        snprintf(deNameSpace[0], 124, "%s%s%s", T2_ROOT_PARAMETER, componentName, T2_EVENT_LIST_PARAM_SUFFIX);
        EVENT_DEBUG("rbus mode : Query marker list with data element = %s \n", deNameSpace[0]);

        pthread_mutex_lock(&markerListMutex);
        EVENT_DEBUG("Lock markerListMutex & Clean up eventMarkerMap \n");
        if(eventMarkerMap != NULL)
        {
            hash_map_destroy(eventMarkerMap, free);
            eventMarkerMap = NULL;
        }

        ret = rbus_get(bus_handle, deNameSpace[0], &paramValue_t);
        if(ret != RBUS_ERROR_SUCCESS)
        {
            EVENT_ERROR("rbus mode : No event list configured in profiles %s and return value %d\n", deNameSpace[0], ret);
            pthread_mutex_unlock(&markerListMutex);
            EVENT_DEBUG("rbus mode : No event list configured in profiles %s and return value %d. Unlock markerListMutex\n", deNameSpace[0], ret);
            EVENT_DEBUG("%s --out\n", __FUNCTION__);
            return T2ERROR_SUCCESS;
        }

        rbusValueType_t type_t = rbusValue_GetType(paramValue_t);
        if(type_t != RBUS_OBJECT)
        {
            EVENT_ERROR("rbus mode : Unexpected data object received for %s get query \n", deNameSpace[0]);
            rbusValue_Release(paramValue_t);
            pthread_mutex_unlock(&markerListMutex);
            EVENT_DEBUG("Unlock markerListMutex\n");
            EVENT_DEBUG("%s --out\n", __FUNCTION__);
            return T2ERROR_FAILURE;
        }

        rbusObject_t objectValue = rbusValue_GetObject(paramValue_t);
        if(objectValue)
        {
            eventMarkerMap = hash_map_create();
            rbusProperty_t rbusPropertyList = rbusObject_GetProperties(objectValue);
            EVENT_DEBUG("\t rbus mode :  Update event map for component %s with below events : \n", componentName);
            while(NULL != rbusPropertyList)
            {
                const char* eventname = rbusProperty_GetName(rbusPropertyList);
                if(eventname && strlen(eventname) > 0)
                {
                    EVENT_DEBUG("\t %s\n", eventname);
                    hash_map_put(eventMarkerMap, (void*) strdup(eventname), (void*) strdup(eventname), free);
                }
                rbusPropertyList = rbusProperty_GetNext(rbusPropertyList);
            }
        }
        else
        {
            EVENT_ERROR("rbus mode : No configured event markers for %s \n", componentName);
        }
        EVENT_DEBUG("Unlock markerListMutex\n");
        pthread_mutex_unlock(&markerListMutex);
        rbusValue_Release(paramValue_t);
        EVENT_DEBUG("%s --out\n", __FUNCTION__);
        return status;
    }
#endif

    // D-Bus implementation
    if(isDbusEnabled && bus_handle)
    {
        EVENT_DEBUG("%s ++in \n", __FUNCTION__);
        
        if(!bus_handle && T2ERROR_SUCCESS != initMessageBus())
        {
            EVENT_ERROR("Unable to get message bus handles \n");
            EVENT_DEBUG("%s --out\n", __FUNCTION__);
            return T2ERROR_FAILURE;
        }

        pthread_mutex_lock(&markerListMutex);
        EVENT_DEBUG("Lock markerListMutex & Clean up eventMarkerMap \n");
        if(eventMarkerMap != NULL)
        {
            hash_map_destroy(eventMarkerMap, free);
            eventMarkerMap = NULL;
        }

        // Get marker list via D-Bus method call
        DBusMessage *msg = NULL;
        DBusMessage *reply = NULL;
        DBusError error;
        dbus_error_init(&error);
        
        msg = dbus_message_new_method_call(T2_DBUS_SERVICE_NAME,
                                           T2_DBUS_OBJECT_PATH,
                                           T2_DBUS_INTERFACE_NAME,
                                           "GetMarkerList");
        if (!msg)
        {
            EVENT_ERROR("D-Bus mode: Failed to create method call message\n");
            pthread_mutex_unlock(&markerListMutex);
            return T2ERROR_FAILURE;
        }
        
        if (!dbus_message_append_args(msg, DBUS_TYPE_STRING, &componentName, DBUS_TYPE_INVALID))
        {
            EVENT_ERROR("D-Bus mode: Failed to append arguments\n");
            dbus_message_unref(msg);
            pthread_mutex_unlock(&markerListMutex);
            return T2ERROR_FAILURE;
        }
        // TODO : check markers list size and set timeout accordingly
        // Timeout: 500ms
        reply = dbus_connection_send_with_reply_and_block((DBusConnection*)bus_handle, msg, 500, &error);
        dbus_message_unref(msg);

        if (dbus_error_is_set(&error))
        {
            EVENT_ERROR("D-Bus mode: Method call failed: %s\n", error.message);
            dbus_error_free(&error);
            pthread_mutex_unlock(&markerListMutex);
            EVENT_DEBUG("D-Bus mode: No event list configured. Unlock markerListMutex\n");
            return T2ERROR_SUCCESS;
        }

        if (!reply)
        {
            EVENT_ERROR("D-Bus mode: No reply received\n");
            pthread_mutex_unlock(&markerListMutex);
            return T2ERROR_SUCCESS;
        }

        // Parse reply - expecting a string containing comma-separated marker list
        char *markerListStr = NULL;
        if (dbus_message_get_args(reply, &error, 
                                 DBUS_TYPE_STRING, &markerListStr,
                                 DBUS_TYPE_INVALID))
        {
            if(markerListStr && strlen(markerListStr) > 0)
            {
                eventMarkerMap = hash_map_create();
                EVENT_DEBUG("Update event map for component %s with below events :\n", componentName);

                char* markerListCopy = strdup(markerListStr);
                char* token = strtok(markerListCopy, ",");
                while(token != NULL)
                {
                    // Trim whitespace
                    while(*token == ' ' || *token == '\t') token++;
                    char* end = token + strlen(token) - 1;
                    while(end > token && (*end == ' ' || *end == '\t' || *end == '\n')) {
                        *end = '\0';
                        end--;
                    }
                    
                    if(strlen(token) > 0)
                    {
                        EVENT_DEBUG("\t %s\n", token);
                        hash_map_put(eventMarkerMap, (void*)strdup(token), (void*)strdup(token), free);
                    }
                    token = strtok(NULL, ",");
                }
                free(markerListCopy);
            }
            else
            {
                EVENT_ERROR("D-Bus mode: No configured event markers for %s\n", componentName);
            }
        }
        else
        {
            EVENT_ERROR("D-Bus mode: Failed to parse reply: %s\n", error.message);
            dbus_error_free(&error);
        }

        dbus_message_unref(reply);
        EVENT_DEBUG("Unlock markerListMutex\n");
        pthread_mutex_unlock(&markerListMutex);
        EVENT_DEBUG("%s --out\n", __FUNCTION__);
        return T2ERROR_SUCCESS;
    }
    else
    {
        EVENT_ERROR("No dbus supported message bus available\n");
    }

    return T2ERROR_FAILURE;
}

#if defined(RBUS_SUPPORT_ENABLED)
static void rbusEventReceiveHandler(rbusHandle_t handle, rbusEvent_t const* event, rbusEventSubscription_t* subscription)
{
    (void)handle;//To fix compiler warning.
    (void)subscription;//To fix compiler warning.
    const char* eventName = event->name;
    if(eventName)
    {
        if(0 == strcmp(eventName, T2_PROFILE_UPDATED_NOTIFY))
        {
            doPopulateEventMarkerList();
        }
    }
    else
    {
        EVENT_ERROR("eventName is null \n");
    }
}
#endif

static DBusHandlerResult dbusEventReceiveHandler(DBusConnection *connection, DBusMessage *message, void *user_data)
{
    (void)connection;
    (void)user_data;
    
    if (dbus_message_is_signal(message, T2_DBUS_EVENT_INTERFACE_NAME, "ProfileUpdate"))
    {
        EVENT_DEBUG("D-Bus: *** ProfileUpdate signal RECEIVED - updating marker list ***\n");
        doPopulateEventMarkerList();
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

// D-Bus event loop thread function - processes BOTH connections
static void* dbus_event_loop_thread(void *arg)
{
    (void)arg;
    
    if (!signal_bus_handle || !bus_handle)
    {
        EVENT_ERROR("Signal bus handle is NULL\n");
        return NULL;
    }
    
    EVENT_DEBUG("D-Bus: Event loop thread started (processing both connections)\n");
    
    while (dbus_event_thread_running)
    {
        // Process signal connection (for ProfileUpdate signals)
        dbus_connection_read_write_dispatch((DBusConnection*)signal_bus_handle, 0);
        dbus_connection_read_write_dispatch((DBusConnection*)bus_handle, 0);

        // Small sleep to avoid busy-waiting
        usleep(100000); // 100ms
    }
    
    EVENT_DEBUG("D-Bus: Event loop thread exiting\n");
    return NULL;
}

static bool isCachingRequired( )
{

    /**
     * Attempts to read from PAM before its ready creates deadlock .
     * PAM not ready is a definite case for caching the event and avoid bus traffic
     * */
#if defined(ENABLE_RDKB_SUPPORT)
    if (access( "/tmp/pam_initialized", F_OK ) != 0)
    {
        return true;
    }
#endif

    if(!initRFC())
    {
        EVENT_ERROR("initRFC failed - cache the events\n");
        return true;
    }

    // If feature is disabled by RFC, caching is always disabled
    if(!isRFCT2Enable)
    {
        return false ;
    }

    // Always check for t2 is ready to accept events. Shutdown target can bring down t2 process at runtime
    uint32_t t2ReadyStatus;
    T2ERROR retVal = T2ERROR_FAILURE;

#if defined(RBUS_SUPPORT_ENABLED)
    if(isRbusEnabled)
    {
        rbusError_t rbusRetVal = rbus_getUint(bus_handle, T2_OPERATIONAL_STATUS, &t2ReadyStatus);
        retVal = (rbusRetVal == RBUS_ERROR_SUCCESS) ? T2ERROR_SUCCESS : T2ERROR_FAILURE;
    }
    else
#endif
    if(isDbusEnabled && bus_handle)
    {
       retVal = dbus_getGetOperationalStatus(T2_OPERATIONAL_STATUS, &t2ReadyStatus);
    //    retVal = T2ERROR_SUCCESS; // Temporarily bypass D-Bus get for operational status
    //    t2ReadyStatus = T2_STATE_COMPONENT_READY; // Assume ready for now
        EVENT_DEBUG("%s:%d, D-Bus t2ReadyStatus: %u\n", __func__, __LINE__, t2ReadyStatus);
    }

    if(retVal != T2ERROR_SUCCESS)
    {
        EVENT_ERROR("Unable to get %s\n", T2_OPERATIONAL_STATUS);
        return true;
    }
    else
    {
        EVENT_DEBUG("value for  %s is : %d\n", T2_OPERATIONAL_STATUS, t2ReadyStatus);
        if((t2ReadyStatus & T2_STATE_READY) == 0)
        {
            return true;
        }
    }

#if defined(RBUS_SUPPORT_ENABLED)
    if(!isRbusEnabled)
    {
        // Fall through to D-Bus handling
    }
    else
    {
        isT2Ready = true;
    }
#endif

    if(!isT2Ready)
    {
        EVENT_DEBUG("T2 is not ready yet, subscribe to profile update event/signals \n");
        if(componentName && (0 != strcmp(componentName, "telemetry_client")))
        {
            // From other binary applications in rbus mode if t2 daemon is yet to determine state of component specific config from cloud, enable cache
            if((t2ReadyStatus & T2_STATE_COMPONENT_READY) == 0)
            {
                return true;
            }
            else
            {
#if defined(RBUS_SUPPORT_ENABLED)
                if(isRbusEnabled)
                {
                    rbusError_t ret = RBUS_ERROR_SUCCESS;
                    doPopulateEventMarkerList();
                    ret = rbusEvent_Subscribe(bus_handle, T2_PROFILE_UPDATED_NOTIFY, rbusEventReceiveHandler, "T2Event", 0);
                    if(ret != RBUS_ERROR_SUCCESS)
                    {
                        EVENT_ERROR("Unable to subscribe to event %s with rbus error code : %d\n", T2_PROFILE_UPDATED_NOTIFY, ret);
                        EVENT_DEBUG("Unable to subscribe to event %s with rbus error code : %d\n", T2_PROFILE_UPDATED_NOTIFY, ret);
                    }
                    isT2Ready = true;
                }
                else
#endif
                if(isDbusEnabled && signal_bus_handle)
                {
                    EVENT_DEBUG("D-Bus: Starting ProfileUpdate signal subscription setup\n");
                    doPopulateEventMarkerList();
                    
                    // Subscribe to D-Bus ProfileUpdate signal using SIGNAL connection
                    char rule[512];
                    DBusError error;
                    dbus_error_init(&error);
                    
                    snprintf(rule, sizeof(rule), 
                             "type='signal',path='%s',interface='%s',member='ProfileUpdate'",
                             T2_DBUS_OBJECT_PATH, T2_DBUS_EVENT_INTERFACE_NAME);
                    
                    dbus_bus_add_match((DBusConnection*)signal_bus_handle, rule, &error);
                    
                    if (dbus_error_is_set(&error))
                    {
                        EVENT_ERROR("Unable to subscribe to ProfileUpdate signal: %s\n", error.message);
                        dbus_error_free(&error);
                    }
                    else
                    {
                        dbus_connection_add_filter((DBusConnection*)signal_bus_handle, dbusEventReceiveHandler, NULL, NULL);
                        EVENT_DEBUG("Now listening for ProfileUpdate signals on interface '%s'\n", T2_DBUS_EVENT_INTERFACE_NAME);
                        
                        // Start D-Bus event loop thread to process incoming signals
                        if (!dbus_event_thread_running)
                        {
                            dbus_event_thread_running = true;
                            if (pthread_create(&dbus_event_thread, NULL, dbus_event_loop_thread, NULL) == 0)
                            {
                                pthread_detach(dbus_event_thread);
                            }
                            else
                            {
                                EVENT_ERROR("D-Bus: Failed to create event loop thread\n");
                                dbus_event_thread_running = false;
                            }
                        }
                    }
                    isT2Ready = true;
                }
            }
        }
        else
        {
            isT2Ready = true;
        }
    }

    return false;
}

static int report_or_cache_data(char* telemetry_data, const char* markerName)
{
    int ret = 0;
    pthread_t tid;
    pthread_mutex_lock(&eventMutex);
    if(isCachingRequired())
    {
        EVENT_DEBUG("Caching the event : %s \n", telemetry_data);
        int eventDataLen = strlen(markerName) + strlen(telemetry_data) + strlen(MESSAGE_DELIMITER) + 1;
        char* buffer = (char*) malloc(eventDataLen * sizeof(char));
        if(buffer)
        {
            // Caching format needs to be same for operation between rbus/dbus modes across reboots
            snprintf(buffer, eventDataLen, "%s%s%s", markerName, MESSAGE_DELIMITER, telemetry_data);
            pthread_create(&tid, NULL, cacheEventToFile, (void *)buffer);
        }
        pthread_mutex_unlock(&eventMutex);
        return T2ERROR_SUCCESS ;
    }
    pthread_mutex_unlock(&eventMutex);

    if(isT2Ready)
    {
        // EVENT_DEBUG("T2: Sending event : %s\n", telemetry_data);
        ret = filtered_event_send(telemetry_data, markerName);
        if(0 != ret)
        {
            EVENT_ERROR("%s:%d, T2:telemetry data send failed, status = %d \n", __func__, __LINE__, ret);
        }
    }
    return ret;
}

/**
 * Initialize the component name with unique name
 */
void t2_init(char *component)
{
    componentName = strdup(component);
}

void t2_uninit(void)
{
    if(componentName)
    {
        free(componentName);
        componentName = NULL ;
    }

#if defined(RBUS_SUPPORT_ENABLED)
    if(isRbusEnabled)
    {
        rBusInterface_Uninit();
    }
    else
#endif
    if(isDbusEnabled)
    {
        dBusInterface_Uninit();
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
