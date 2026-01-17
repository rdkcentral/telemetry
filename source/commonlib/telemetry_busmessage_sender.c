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
#include <dbus/dbus.h>

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

/* D-Bus Service Configuration */
#define T2_DBUS_SERVICE_NAME        "telemetry.t2"
#define T2_DBUS_OBJECT_PATH         "/telemetry/t2"
#define T2_DBUS_INTERFACE_NAME      "telemetry.t2.interface"
#define T2_DBUS_SIGNAL_EVENT        "TelemetryEvent"
#define T2_DBUS_SIGNAL_PROFILE_UPDATE "ProfileUpdate"
#define T2_DBUS_DEFAULT_TIMEOUT_MS  5000

/* D-Bus Connection Handle */
typedef struct {
    DBusConnection *connection;
    char *unique_name;
    bool is_initialized;
} T2DbusHandle_t;

static T2DbusHandle_t t2dbus_handle = {NULL, NULL, false};
static void (*profileUpdateCallback)(void) = NULL;
static pthread_mutex_t dbusMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t dbusListenerThread;
static bool stopListenerThread = false;

static char *componentName = NULL;
static bool isRFCT2Enable = false ;
static bool isT2Ready = false;
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

/**
 * @brief Check if D-Bus is initialized
 */
static bool isDbusInitialized(void) {
    return t2dbus_handle.is_initialized;
}

/**
 * @brief D-Bus filter function for incoming messages
 */
static DBusHandlerResult dbusMessageFilter(DBusConnection *connection, 
                                           DBusMessage *message, 
                                           void *user_data) {
    (void)connection;
    (void)user_data;
    
    if (dbus_message_is_signal(message, T2_DBUS_INTERFACE_NAME, T2_DBUS_SIGNAL_PROFILE_UPDATE)) {
        EVENT_DEBUG("Received ProfileUpdate signal\n");
        if (profileUpdateCallback) {
            profileUpdateCallback();
        }
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

/**
 * @brief D-Bus listener thread function
 */
static void* dbusListenerThreadFunc(void *arg) {
    (void)arg;
    
    EVENT_DEBUG("%s ++in\n", __FUNCTION__);
    
    while (!stopListenerThread && t2dbus_handle.connection) {
        /* Process messages with timeout */
        dbus_connection_read_write_dispatch(t2dbus_handle.connection, 100);
    }
    
    EVENT_DEBUG("%s --out\n", __FUNCTION__);
    return NULL;
}

/**
 * @brief Initialize D-Bus connection
 */
static T2ERROR dBusInterface_Init(const char *component_name) {
    EVENT_DEBUG("%s ++in\n", __FUNCTION__);
    
    pthread_mutex_lock(&dbusMutex);
    
    if (t2dbus_handle.is_initialized) {
        EVENT_DEBUG("D-Bus interface already initialized\n");
        pthread_mutex_unlock(&dbusMutex);
        return T2ERROR_SUCCESS;
    }
    
    DBusError error;
    dbus_error_init(&error);
    
    /* Connect to session bus for local IPC */
    t2dbus_handle.connection = dbus_bus_get(DBUS_BUS_SESSION, &error);
    if (dbus_error_is_set(&error)) {
        EVENT_ERROR("D-Bus connection error: %s\n", error.message);
        dbus_error_free(&error);
        pthread_mutex_unlock(&dbusMutex);
        return T2ERROR_FAILURE;
    }
    
    if (!t2dbus_handle.connection) {
        EVENT_ERROR("Failed to get D-Bus connection\n");
        pthread_mutex_unlock(&dbusMutex);
        return T2ERROR_FAILURE;
    }
    
    /* Request well-known name */
    char service_name[256];
    if (component_name) {
        snprintf(service_name, sizeof(service_name), "%s.%s", 
                 T2_DBUS_SERVICE_NAME, component_name);
    } else {
        snprintf(service_name, sizeof(service_name), "%s", T2_DBUS_SERVICE_NAME);
    }
    
    int ret = dbus_bus_request_name(t2dbus_handle.connection, service_name,
                                     DBUS_NAME_FLAG_REPLACE_EXISTING, &error);
    if (dbus_error_is_set(&error)) {
        EVENT_ERROR("D-Bus name request error: %s\n", error.message);
        dbus_error_free(&error);
        dbus_connection_unref(t2dbus_handle.connection);
        t2dbus_handle.connection = NULL;
        pthread_mutex_unlock(&dbusMutex);
        return T2ERROR_FAILURE;
    }
    
    if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        EVENT_DEBUG("Not primary owner of D-Bus name\n");
    }
    
    /* Store unique name */
    t2dbus_handle.unique_name = strdup(dbus_bus_get_unique_name(t2dbus_handle.connection));
    
    /* Add message filter */
    dbus_connection_add_filter(t2dbus_handle.connection, dbusMessageFilter, NULL, NULL);
    
    t2dbus_handle.is_initialized = true;
    
    /* Start listener thread */
    stopListenerThread = false;
    if (pthread_create(&dbusListenerThread, NULL, dbusListenerThreadFunc, NULL) != 0) {
        EVENT_ERROR("Failed to create D-Bus listener thread\n");
        dbus_connection_remove_filter(t2dbus_handle.connection, dbusMessageFilter, NULL);
        dbus_connection_unref(t2dbus_handle.connection);
        if (t2dbus_handle.unique_name) {
            free(t2dbus_handle.unique_name);
            t2dbus_handle.unique_name = NULL;
        }
        t2dbus_handle.connection = NULL;
        t2dbus_handle.is_initialized = false;
        pthread_mutex_unlock(&dbusMutex);
        return T2ERROR_FAILURE;
    }
    
    pthread_mutex_unlock(&dbusMutex);
    
    EVENT_DEBUG("D-Bus interface initialized successfully with name: %s\n", service_name);
    EVENT_DEBUG("%s --out\n", __FUNCTION__);
    
    return T2ERROR_SUCCESS;
}

/**
 * @brief Uninitialize D-Bus interface
 */
static void dBusInterface_Uninit(void) {
    EVENT_DEBUG("%s ++in\n", __FUNCTION__);
    
    pthread_mutex_lock(&dbusMutex);
    
    if (!t2dbus_handle.is_initialized) {
        pthread_mutex_unlock(&dbusMutex);
        return;
    }
    
    /* Stop listener thread */
    stopListenerThread = true;
    pthread_mutex_unlock(&dbusMutex);
    
    if (dbusListenerThread) {
        pthread_join(dbusListenerThread, NULL);
    }
    
    pthread_mutex_lock(&dbusMutex);
    
    /* Clean up D-Bus connection */
    if (t2dbus_handle.connection) {
        dbus_connection_remove_filter(t2dbus_handle.connection, dbusMessageFilter, NULL);
        dbus_connection_unref(t2dbus_handle.connection);
        t2dbus_handle.connection = NULL;
    }
    
    if (t2dbus_handle.unique_name) {
        free(t2dbus_handle.unique_name);
        t2dbus_handle.unique_name = NULL;
    }
    
    t2dbus_handle.is_initialized = false;
    
    pthread_mutex_unlock(&dbusMutex);
    
    EVENT_DEBUG("%s --out\n", __FUNCTION__);
}

/**
 * @brief Publish telemetry event via D-Bus signal
 */
static T2ERROR dbusPublishEvent(const char* eventName, const char* eventValue) {
    EVENT_DEBUG("%s ++in\n", __FUNCTION__);
    
    if (!eventName || !eventValue) {
        EVENT_ERROR("Invalid event parameters\n");
        return T2ERROR_INVALID_ARGS;
    }
    
    EVENT_DEBUG("Publishing event - Name: %s, Value length: %zu\n", eventName, strlen(eventValue));
    
    if (!t2dbus_handle.is_initialized) {
        EVENT_ERROR("D-Bus not initialized\n");
        return T2ERROR_INTERNAL_ERROR;
    }
    
    EVENT_DEBUG("D-Bus handle initialized, connection: %p\n", (void*)t2dbus_handle.connection);
    
    DBusMessage *signal = NULL;
    
    /* Create signal */
    EVENT_DEBUG("Creating D-Bus signal on path: %s, interface: %s\n", T2_DBUS_OBJECT_PATH, T2_DBUS_INTERFACE_NAME);
    signal = dbus_message_new_signal(T2_DBUS_OBJECT_PATH,
                                     T2_DBUS_INTERFACE_NAME,
                                     T2_DBUS_SIGNAL_EVENT);
    if (!signal) {
        EVENT_ERROR("Failed to create D-Bus signal\n");
        return T2ERROR_FAILURE;
    }
    
    EVENT_DEBUG("D-Bus signal created successfully, appending arguments\n");
    
    /* Append arguments */
    if (!dbus_message_append_args(signal,
                                  DBUS_TYPE_STRING, &eventName,
                                  DBUS_TYPE_STRING, &eventValue,
                                  DBUS_TYPE_INVALID)) {
        EVENT_ERROR("Failed to append signal arguments\n");
        dbus_message_unref(signal);
        return T2ERROR_FAILURE;
    }
    
    EVENT_DEBUG("Arguments appended successfully to signal\n");
    
    /* Send signal */
    EVENT_DEBUG("Sending D-Bus signal to connection\n");
    if (!dbus_connection_send(t2dbus_handle.connection, signal, NULL)) {
        EVENT_ERROR("Failed to send D-Bus signal\n");
        dbus_message_unref(signal);
        return T2ERROR_FAILURE;
    }
    
    EVENT_DEBUG("D-Bus signal sent, flushing connection\n");
    dbus_connection_flush(t2dbus_handle.connection);
    EVENT_DEBUG("D-Bus connection flushed successfully\n");
    dbus_message_unref(signal);
    
    EVENT_DEBUG("Published event: %s = %s\n", eventName, eventValue);
    EVENT_DEBUG("%s --out\n", __FUNCTION__);
    
    return T2ERROR_SUCCESS;
}

/**
 * @brief Get marker list for component via D-Bus method call
 */
static T2ERROR dbusGetMarkerList(const char* component, char** markerList) {
    EVENT_DEBUG("%s ++in\n", __FUNCTION__);
    
    if (!component || !markerList) {
        EVENT_ERROR("Invalid arguments\n");
        return T2ERROR_INVALID_ARGS;
    }
    
    if (!t2dbus_handle.is_initialized) {
        EVENT_ERROR("D-Bus not initialized\n");
        return T2ERROR_INTERNAL_ERROR;
    }
    
    DBusMessage *msg = NULL;
    DBusMessage *reply = NULL;
    DBusError error;
    dbus_error_init(&error);
    
    /* Create method call message */
    msg = dbus_message_new_method_call(T2_DBUS_SERVICE_NAME,
                                       T2_DBUS_OBJECT_PATH,
                                       T2_DBUS_INTERFACE_NAME,
                                       "GetMarkerList");
    if (!msg) {
        EVENT_ERROR("Failed to create method call message\n");
        return T2ERROR_FAILURE;
    }
    
    /* Append component name */
    if (!dbus_message_append_args(msg, DBUS_TYPE_STRING, &component, DBUS_TYPE_INVALID)) {
        EVENT_ERROR("Failed to append arguments\n");
        dbus_message_unref(msg);
        return T2ERROR_FAILURE;
    }
    
    /* Send message and get reply */
    reply = dbus_connection_send_with_reply_and_block(t2dbus_handle.connection, msg,
                                                       T2_DBUS_DEFAULT_TIMEOUT_MS, &error);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&error)) {
        EVENT_ERROR("D-Bus method call failed: %s\n", error.message);
        dbus_error_free(&error);
        return T2ERROR_FAILURE;
    }

    if (!reply) {
        EVENT_ERROR("No reply received\n");
        return T2ERROR_FAILURE;
    }

    /* Parse reply - expecting a string containing the marker list */
    char *value = NULL;
    if (dbus_message_get_args(reply, &error, 
                             DBUS_TYPE_STRING, &value,
                             DBUS_TYPE_INVALID)) {
        *markerList = strdup(value);
        EVENT_DEBUG("Retrieved marker list for %s\n", component);
    } else {
        EVENT_ERROR("Failed to parse reply: %s\n", error.message);
        dbus_error_free(&error);
        dbus_message_unref(reply);
        return T2ERROR_FAILURE;
    }

    dbus_message_unref(reply);

    EVENT_DEBUG("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

/**
 * @brief Get operational status via D-Bus method call
 */
static T2ERROR dbusGetOperationalStatus(uint32_t* status) {
    EVENT_DEBUG("%s ++in\n", __FUNCTION__);
    
    if (!status) {
        EVENT_ERROR("Invalid arguments\n");
        return T2ERROR_INVALID_ARGS;
    }

    if (!t2dbus_handle.is_initialized) {
        EVENT_ERROR("D-Bus not initialized\n");
        return T2ERROR_INTERNAL_ERROR;
    }

    DBusMessage *msg = NULL;
    DBusMessage *reply = NULL;
    DBusError error;
    dbus_error_init(&error);
    
    /* Create method call message */
    msg = dbus_message_new_method_call(T2_DBUS_SERVICE_NAME,
                                       T2_DBUS_OBJECT_PATH,
                                       T2_DBUS_INTERFACE_NAME,
                                       "GetOperationalStatus");
    if (!msg) {
        EVENT_ERROR("Failed to create method call message\n");
        return T2ERROR_FAILURE;
    }
    
    /* Send message and get reply */
    reply = dbus_connection_send_with_reply_and_block(t2dbus_handle.connection, msg,
                                                       T2_DBUS_DEFAULT_TIMEOUT_MS, &error);
    dbus_message_unref(msg);
    
    if (dbus_error_is_set(&error)) {
        EVENT_ERROR("D-Bus method call failed: %s\n", error.message);
        dbus_error_free(&error);
        return T2ERROR_FAILURE;
    }
    
    if (!reply) {
        EVENT_ERROR("No reply received\n");
        return T2ERROR_FAILURE;
    }
    
    /* Parse reply - expecting UINT32 */
    if (dbus_message_get_args(reply, &error, 
                             DBUS_TYPE_UINT32, status,
                             DBUS_TYPE_INVALID)) {
        EVENT_DEBUG("Retrieved operational status: %u\n", *status);
    } else {
        EVENT_ERROR("Failed to parse reply: %s\n", error.message);
        dbus_error_free(&error);
        dbus_message_unref(reply);
        return T2ERROR_FAILURE;
    }
    
    dbus_message_unref(reply);
    
    EVENT_DEBUG("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

/**
 * @brief Subscribe to profile update notifications
 */
static T2ERROR dbusSubscribeProfileUpdate(void (*callback)(void)) {
    EVENT_DEBUG("%s ++in\n", __FUNCTION__);
    
    if (!callback) {
        EVENT_ERROR("Invalid callback\n");
        return T2ERROR_INVALID_ARGS;
    }

    if (!t2dbus_handle.is_initialized) {
        EVENT_ERROR("D-Bus not initialized\n");
        return T2ERROR_INTERNAL_ERROR;
    }

    /* Store callback */
    profileUpdateCallback = callback;
    
    /* Subscribe to ProfileUpdate signal */
    char rule[512];
    DBusError error;
    dbus_error_init(&error);
    
    snprintf(rule, sizeof(rule), 
             "type='signal',interface='%s',member='%s'",
             T2_DBUS_INTERFACE_NAME, T2_DBUS_SIGNAL_PROFILE_UPDATE);
    
    dbus_bus_add_match(t2dbus_handle.connection, rule, &error);
    
    if (dbus_error_is_set(&error)) {
        EVENT_ERROR("Failed to add match rule: %s\n", error.message);
        dbus_error_free(&error);
        return T2ERROR_FAILURE;
    }
    
    EVENT_DEBUG("Subscribed to ProfileUpdate signal\n");
    EVENT_DEBUG("%s --out\n", __FUNCTION__);
    
    return T2ERROR_SUCCESS;
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
    fs = popen ("cat /tmp/t2_caching_file | wc -l", "r");
    if(fs != NULL)
    {
        fgets(path, 100, fs);
        count = atoi ( path );
        pclose(fs);
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
    // // Check for RFC and proceed - if true - else return now .
    // if(!bus_handle)
    // {
    //     if(initMessageBus() != 0)
    //     {
    //         EVENT_ERROR("initMessageBus failed\n");
    //         status = false ;
    //     }
    //     else
    //     {
    //         status = true;
    //     }
    //     isRFCT2Enable = true;
    // }

    return status;
}

/**
 * In rbus mode, should be using rbus subscribed param
 * from telemetry 2.0 instead of direct api for event sending
 */
int filtered_event_send(const char* data, const char *markerName)
{
    T2ERROR ret = T2ERROR_SUCCESS;
    int status = 0 ;
    EVENT_DEBUG("%s ++in\n", __FUNCTION__);
    
    if(!isDbusInitialized())
    {
        EVENT_ERROR("DBUS not initialized .. exiting !!! \n");
        return ret;
    }

    // Filter data from marker list
    if(componentName && (0 != strcmp(componentName, T2_SCRIPT_EVENT_COMPONENT)))   // Events from scripts needs to be sent without filtering
    {
        EVENT_DEBUG("%s markerListMutex lock & get list of marker for component %s \n", __FUNCTION__, componentName);
        EVENT_DEBUG("Filtering event: checking if marker '%s' is in allowed list\n", markerName);
        pthread_mutex_lock(&markerListMutex);
        bool isEventingEnabled = false;
        if(markerName && eventMarkerMap)
        {
            EVENT_DEBUG("Searching marker '%s' in eventMarkerMap\n", markerName);
            if(hash_map_get(eventMarkerMap, markerName))
            {
                isEventingEnabled = true;
                EVENT_DEBUG("Marker '%s' found in allowed list, event enabled\n", markerName);
            }
            else
            {
                EVENT_DEBUG("Marker '%s' NOT found in allowed list, event will be filtered\n", markerName);
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
            EVENT_DEBUG("Event FILTERED OUT - marker '%s' not enabled for component '%s'\n", markerName, componentName);
            return status;
        }
        EVENT_DEBUG("Event ALLOWED - marker '%s' passed filter for component '%s'\n", markerName, componentName);
    }
    else
    {
        EVENT_DEBUG("Skipping filter - component is '%s' (script events or no component)\n", componentName ? componentName : "NULL");
    }
    // End of event filtering

    EVENT_DEBUG("Calling dbusPublishEvent with marker [%s] and value [%s]\n", markerName, data);
    ret = dbusPublishEvent(markerName, data);
    if(ret != T2ERROR_SUCCESS)
    {
        EVENT_ERROR("dbusPublishEvent Failed for marker [%s] with error [%d]\n", markerName, ret);
        EVENT_DEBUG(" !!! Error !!! dbusPublishEvent Failed for marker [%s] with error [%d]\n", markerName, ret);
        status = -1 ;
    }
    else
    {
        status = 0 ;
    }

    EVENT_DEBUG("%s --out with status %d \n", __FUNCTION__, status);
    return status;
}

/**
 * Get marker list from T2 daemon via DBUS
 */
static T2ERROR doPopulateEventMarkerList(void)
{
    EVENT_DEBUG("%s ++in\n", __FUNCTION__);
    T2ERROR status = T2ERROR_SUCCESS;
    
    if(!isDbusInitialized())
    {
        EVENT_ERROR("DBUS not initialized\n");
        return T2ERROR_FAILURE;
    }
    
    pthread_mutex_lock(&markerListMutex);
    EVENT_DEBUG("Lock markerListMutex & Clean up eventMarkerMap \n");
    
    if(eventMarkerMap != NULL)
    {
        hash_map_destroy(eventMarkerMap, free);
        eventMarkerMap = NULL;
    }
    
    // Get marker list via DBUS method call
    char* markerListStr = NULL;
    status = dbusGetMarkerList(componentName, &markerListStr);
    
    if(status != T2ERROR_SUCCESS || !markerListStr)
    {
        EVENT_ERROR("dbusGetMarkerList failed for component %s\n", componentName);
        pthread_mutex_unlock(&markerListMutex);
        EVENT_DEBUG("Unlock markerListMutex\n");
        EVENT_DEBUG("%s --out\n", __FUNCTION__);
        return T2ERROR_FAILURE;
    }
    
    // Parse marker list string (format: comma-separated marker names)
    eventMarkerMap = hash_map_create();
    EVENT_DEBUG("\t Update event map for component %s with below events : \n", componentName);
    
    char* markerListCopy = strdup(markerListStr);
    EVENT_DEBUG("Marker List String : %s \n", markerListStr);
    char* token = strtok(markerListCopy, ",");
    while(token != NULL)
    {
        // Trim leading whitespace
        while(*token == ' ' || *token == '\t') token++;
        // Trim trailing whitespace
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
    free(markerListStr);
    pthread_mutex_unlock(&markerListMutex);
    EVENT_DEBUG("Unlock markerListMutex\n");
    EVENT_DEBUG("%s --out\n", __FUNCTION__);
    return status;
}

static void dbusProfileUpdateHandler(void)
{
    EVENT_DEBUG("Profile update notification received via DBUS\n");
    doPopulateEventMarkerList();
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
    T2ERROR retVal = dbusGetOperationalStatus(&t2ReadyStatus);

    if(retVal != T2ERROR_SUCCESS)
    {
        return true;
    }
    else
    {
        EVENT_DEBUG("Operational status: %d\n", t2ReadyStatus);
        if((t2ReadyStatus & T2_STATE_COMPONENT_READY) == 0)
            {
                return true;
            }
    }

    if(!isT2Ready)
    {
        if(componentName && (0 != strcmp(componentName, "telemetry_client")))
        {
            // From other binary applications in rbus mode if t2 daemon is yet to determine state of component specific config from cloud, enable cache
            if((t2ReadyStatus & T2_STATE_COMPONENT_READY) == 0)
            {
                return true;
            }
            else
            {
                // Fetch marker list and subscribe to profile updates
                doPopulateEventMarkerList();
                
                T2ERROR ret = dbusSubscribeProfileUpdate(dbusProfileUpdateHandler);
                if(ret != T2ERROR_SUCCESS)
                {
                    EVENT_ERROR("Unable to subscribe to ProfileUpdate signal with error : %d\n", ret);
                    EVENT_DEBUG("Unable to subscribe to ProfileUpdate signal with error : %d\n", ret);
                }
                isT2Ready = true;
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
    EVENT_DEBUG("%s ++in - marker: %s, data length: %zu\n", __FUNCTION__, markerName, strlen(telemetry_data));
    pthread_mutex_lock(&eventMutex);
    EVENT_DEBUG("Checking if caching is required...\n");
    if(isCachingRequired())
    {
        EVENT_DEBUG("Caching IS required - will cache event to file\n");
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

    EVENT_DEBUG("Caching NOT required - will send event via D-Bus\n");
    if(isT2Ready)
    {
        EVENT_DEBUG("T2 is ready - sending event via D-Bus: marker=%s\n", markerName);
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
    
    // Initialize DBUS connection
    T2ERROR ret = dBusInterface_Init(componentName);
    if(ret != T2ERROR_SUCCESS)
    {
        EVENT_ERROR("DBUS initialization failed for %s\n", componentName);
    }
}

void t2_uninit(void)
{
    if(componentName)
    {
        free(componentName);
        componentName = NULL ;
    }
    
    // Uninitialize DBUS
    dBusInterface_Uninit();
    
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
