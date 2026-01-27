/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2026 RDK Management
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

/**
 * @file dbusInterface.c
 * @brief D-Bus Interface Implementation for Telemetry 2.0
 *
 * This file implements D-Bus based inter-process communication
 * as an alternative to RBUS for telemetry operations.
 */

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <glib.h>

#include "dbusInterface.h"
#include "t2collection.h"
#include "t2common.h"
#include "busInterface.h"
#include "telemetry2_0.h"
#include "t2log_wrapper.h"
#include "profile.h"

#define BUFF_LEN 1024
#define MAX_PARAM_LEN 128

/* Global D-Bus handle */
static T2DbusHandle_t t2dbus_handle = {NULL, NULL, false};

/* Callback handlers */
static TelemetryEventCallback eventCallBack = NULL;
static T2EventMarkerListCallback getMarkerListCallBack = NULL;

/* State variables */
static uint32_t t2ReadyStatus = T2_STATE_NOT_READY;

/* Threading */
static pthread_mutex_t dbusMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t dbusListenerThread;
static bool stopListenerThread = false;

/**
 * @brief Check if D-Bus is initialized
 */
bool isDbusInitialized(void) {
    return t2dbus_handle.is_initialized;
}

/* Handle GetOperationalStatus Method */
static DBusHandlerResult handle_get_operational_status(DBusConnection *connection, DBusMessage *message) {
    T2Debug("handle_get_operational_status: Received GetOperationalStatus method call");

    DBusError error;
    dbus_error_init(&error);

    const char* param_name = NULL;
    if (!dbus_message_get_args(message, &error,
                               DBUS_TYPE_STRING, &param_name,
                               DBUS_TYPE_INVALID)) {
        T2Error("Failed to parse GetOperationalStatus arguments: %s", error.message);
        dbus_error_free(&error);
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    T2Info("GetOperationalStatus called with param_name: %s", param_name);

    uint32_t value = 0;
    /* TODO check oprtational status of specific component param_name will componet name */
    value = t2ReadyStatus;
    T2Info("Returning operational status for %s: 0x%08X", param_name, value);

    /* Create reply */
    DBusMessage *reply = dbus_message_new_method_return(message);
    if (!reply) {
        T2Error("Failed to create reply message");
        return DBUS_HANDLER_RESULT_NEED_MEMORY;
    }

    if (!dbus_message_append_args(reply,
                                  DBUS_TYPE_UINT32, &value,
                                  DBUS_TYPE_INVALID)) {
        T2Error("Failed to append reply arguments");
        dbus_message_unref(reply);
        return DBUS_HANDLER_RESULT_NEED_MEMORY;
    }

    if (!dbus_connection_send(connection, reply, NULL)) {
        T2Error("Failed to send reply");
        dbus_message_unref(reply);
        return DBUS_HANDLER_RESULT_NEED_MEMORY;
    }

    T2Debug("GetOperationalStatus: Reply sent successfully");
    dbus_message_unref(reply);
    // dbus_connection_flush(connection);

    return DBUS_HANDLER_RESULT_HANDLED;
}

/* Handle SendT2Event Method */
static DBusHandlerResult handle_send_t2_event(DBusConnection *connection, DBusMessage *message) {
    T2Debug("handle_send_t2_event: Received SendT2Event method call");

    DBusError error;
    dbus_error_init(&error);

    const char* marker_name = NULL;
    const char* data = NULL;

    if (!dbus_message_get_args(message, &error,
                               DBUS_TYPE_STRING, &marker_name,
                               DBUS_TYPE_STRING, &data,
                               DBUS_TYPE_INVALID)) {
        T2Error("Failed to parse SendT2Event arguments: %s", error.message);
        dbus_error_free(&error);
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    if (marker_name && data && eventCallBack) {
        T2Debug("Received event: name=%s, value=%s\n", marker_name, data);
        eventCallBack(strdup(marker_name), strdup(data));
    }

    /* Create empty reply (method returns void) */
    DBusMessage *reply = dbus_message_new_method_return(message);
    if (!reply) {
        T2Error("Failed to create reply message");
        return DBUS_HANDLER_RESULT_NEED_MEMORY;
    }

    if (!dbus_connection_send(connection, reply, NULL)) {
        T2Error("Failed to send reply");
        dbus_message_unref(reply);
        return DBUS_HANDLER_RESULT_NEED_MEMORY;
    }

    T2Debug("SendT2Event: Reply sent successfully");
    dbus_message_unref(reply);
    // dbus_connection_flush(connection);

    return DBUS_HANDLER_RESULT_HANDLED;
}

/* Handle GetMarkerList Method */
static DBusHandlerResult handle_get_marker_list(DBusConnection *connection, DBusMessage *message) {
    T2Debug("handle_get_marker_list: Received GetMarkerList method call");

    DBusMessage *reply = NULL;
    DBusError error;
    dbus_error_init(&error);

    const char* component_name = NULL;
    if (!dbus_message_get_args(message, &error,
        DBUS_TYPE_STRING, &component_name,
        DBUS_TYPE_INVALID)) {
            T2Error("Failed to parse GetMarkerList arguments: %s", error.message);
            dbus_error_free(&error);
            return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }
        
    T2Info("GetMarkerList called for component: %s", component_name);

    if (!getMarkerListCallBack) {
        T2Error("GetMarkerList callback not registered\n");
        reply = dbus_message_new_error(message, DBUS_ERROR_FAILED,
                                        "Marker list callback not initialized");
    }
    else
    {
        Vector *markerList = NULL;
        getMarkerListCallBack(component_name, (void**)&markerList); 
        char markerListStr[4096] = "";
        if (markerList && Vector_Size(markerList) > 0) {
            for (size_t i = 0; i < Vector_Size(markerList); i++) {
                char *marker = (char*)Vector_At(markerList, i);
                if (marker) {
                    if (i > 0) strcat(markerListStr, ",");
                    strcat(markerListStr, marker);
                }
            }
            Vector_Destroy(markerList, free);
        }

        reply = dbus_message_new_method_return(message);
        if (!reply) {
            T2Error("Failed to create reply message");
            return DBUS_HANDLER_RESULT_NEED_MEMORY;
        }
        const char *result = markerListStr;

        if (!dbus_message_append_args(reply,
                                  DBUS_TYPE_STRING, &result,
                                  DBUS_TYPE_INVALID)) {
            T2Error("Failed to append reply arguments");
            dbus_message_unref(reply);
            return DBUS_HANDLER_RESULT_NEED_MEMORY;
        }

        T2Debug("Returning marker list: %s\n", markerListStr);

        if (!dbus_connection_send(connection, reply, NULL)) {
            T2Error("Failed to send reply");
            dbus_message_unref(reply);
            return DBUS_HANDLER_RESULT_NEED_MEMORY;
        }

        // dbus_connection_flush(connection);
        T2Debug("GetMarkerList: Reply sent successfully");
        dbus_message_unref(reply);
    }

    return DBUS_HANDLER_RESULT_HANDLED;
}

/* Message Handler */
static DBusHandlerResult message_handler(DBusConnection *connection, DBusMessage *message, void *user_data) {
    (void)user_data;

    const char* interface = dbus_message_get_interface(message);
    const char* member = dbus_message_get_member(message);
    const char* path = dbus_message_get_path(message);

    T2Debug("Received D-Bus message: interface=%s, member=%s, path=%s",
              interface ? interface : "NULL",
              member ? member : "NULL", 
              path ? path : "NULL");

    /* Check if message is for our interface */
    if (interface && strcmp(interface, T2_DBUS_INTERFACE_NAME) == 0)
    {
        if (dbus_message_is_method_call(message, T2_DBUS_INTERFACE_NAME, "GetOperationalStatus")) {
            return handle_get_operational_status(connection, message);
        }
        else if (dbus_message_is_method_call(message, T2_DBUS_INTERFACE_NAME, "SendT2Event")) {
            return handle_send_t2_event(connection, message);
        }
        else if (dbus_message_is_method_call(message, T2_DBUS_INTERFACE_NAME, "GetMarkerList")) {
            return handle_get_marker_list(connection, message);
        }
    }

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

/**
 * @brief D-Bus listener thread function
 */
static void* dbusListenerThreadFunc(void *arg) {
    (void)arg;
    
    T2Debug("%s ++in\n", __FUNCTION__);
    
    while (!stopListenerThread && t2dbus_handle.connection) {
        dbus_connection_read_write_dispatch(t2dbus_handle.connection, 100);
        usleep(1000);
    }
    T2Debug("%s --out\n", __FUNCTION__);
    return NULL;
}

T2ERROR publishdbusEventsProfileUpdates(void)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if(!t2dbus_handle.is_initialized)   
    {
        T2Error("D-Bus not initialized\n");
        return T2ERROR_INTERNAL_ERROR;
    }

    DBusMessage *signal = dbus_message_new_signal(T2_DBUS_OBJECT_PATH,
                                                   T2_DBUS_EVENT_INTERFACE_NAME,
                                                   T2_DBUS_SIGNAL_PROFILE_UPDATE);
    if (!signal) {
        T2Error("Failed to create ProfileUpdate signal");
        return T2ERROR_FAILURE;
    }

    /* Send signal - this queues the message */
    dbus_uint32_t serial = 0;
    if (!dbus_connection_send(t2dbus_handle.connection, signal, &serial)) {
        T2Error("Failed to send ProfileUpdate signal - out of memory");
        dbus_message_unref(signal);
        return T2ERROR_FAILURE;
    }

    //dbus_message_unref(signal);

    /* Flush to ensure signal is sent immediately */
    //dbus_connection_flush(t2dbus_handle.connection);

    T2Debug("ProfileUpdate signal sent successfully (serial=%u)", serial);
    return T2ERROR_SUCCESS;
}

/**
 * @brief Initialize D-Bus interface
 */
T2ERROR dBusInterface_Init() {
    T2Debug("%s ++in\n", __FUNCTION__);
    
    pthread_mutex_lock(&dbusMutex);
    
    if (t2dbus_handle.is_initialized) {
        T2Warning("D-Bus interface already initialized\n");
        pthread_mutex_unlock(&dbusMutex);
        return T2ERROR_SUCCESS;
    }
    
    DBusError error;
    dbus_error_init(&error);

    if (!dbus_threads_init_default()) {
        T2Error("Failed to initialize D-Bus threading");
        return 1;
    }
    
    /* Connect to system bus */
    t2dbus_handle.connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    if (dbus_error_is_set(&error)) {
        T2Error("D-Bus connection error: %s\n", error.message);
        dbus_error_free(&error);
        pthread_mutex_unlock(&dbusMutex);
        return T2ERROR_FAILURE;
    }
    
    if (!t2dbus_handle.connection) {
        T2Error("Failed to get D-Bus connection\n");
        pthread_mutex_unlock(&dbusMutex);
        return T2ERROR_FAILURE;
    }
    

    
    int ret = dbus_bus_request_name(t2dbus_handle.connection, T2_DBUS_SERVICE_NAME,
                                     DBUS_NAME_FLAG_REPLACE_EXISTING, &error);
    if (dbus_error_is_set(&error)) {
        T2Error("D-Bus name request error: %s\n", error.message);
        dbus_error_free(&error);
        dbus_connection_unref(t2dbus_handle.connection);
        t2dbus_handle.connection = NULL;
        pthread_mutex_unlock(&dbusMutex);
        return T2ERROR_FAILURE;
    }
    
    if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        T2Error("Not primary owner of the name (ret=%d)", ret);
        dbus_connection_unref(t2dbus_handle.connection);
        t2dbus_handle.connection = NULL;
        pthread_mutex_unlock(&dbusMutex);
        return T2ERROR_FAILURE;
    }
    
    T2Info("Acquired service name: %s", T2_DBUS_SERVICE_NAME);
    /* Store unique name */
    t2dbus_handle.unique_name = strdup(dbus_bus_get_unique_name(t2dbus_handle.connection));

        /* Register object path */
    DBusObjectPathVTable vtable = {
        .message_function = message_handler,
        .unregister_function = NULL
    };

    if (!dbus_connection_register_object_path(t2dbus_handle.connection, T2_DBUS_OBJECT_PATH,
                                              &vtable, NULL)) {
        T2Error("Failed to register object path");
        dbus_connection_unref(t2dbus_handle.connection);
        return T2ERROR_FAILURE;
    }
    T2Info("Registered object path: %s", T2_DBUS_OBJECT_PATH);

    //TODO check ready status based on component initialization
    t2ReadyStatus = T2_STATE_COMPONENT_READY; 
    t2dbus_handle.is_initialized = true;
    
    /* Start listener thread */
    stopListenerThread = false;
    if (pthread_create(&dbusListenerThread, NULL, dbusListenerThreadFunc, NULL) != 0) {
        T2Error("Failed to create D-Bus listener thread\n");
        dBusInterface_Uninit();
        pthread_mutex_unlock(&dbusMutex);
        return T2ERROR_FAILURE;
    }
    //TODO change to detached thread
    
    pthread_mutex_unlock(&dbusMutex);
    
    T2Info("D-Bus interface initialized successfully with name: %s\n", T2_DBUS_SERVICE_NAME);
    T2Debug("%s --out\n", __FUNCTION__);
    
    return T2ERROR_SUCCESS;
}

/**
 * @brief Uninitialize D-Bus interface
 */
void dBusInterface_Uninit(void) {
    T2Debug("%s ++in\n", __FUNCTION__);
    
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
        dbus_connection_unref(t2dbus_handle.connection);
        t2dbus_handle.connection = NULL;
    }
    
    if (t2dbus_handle.unique_name) {
        free(t2dbus_handle.unique_name);
        t2dbus_handle.unique_name = NULL;
    }
    
    t2dbus_handle.is_initialized = false;
    
    pthread_mutex_unlock(&dbusMutex);
    
    T2Debug("%s --out\n", __FUNCTION__);
}

/**
 * @brief Get parameter value via D-Bus
 */
T2ERROR getDbusParameterVal(const char* paramName, char **paramValue) {
    T2Debug("%s ++in\n", __FUNCTION__);
    
    if (!paramName || !paramValue) {
        T2Error("Invalid parameters\n");
        return T2ERROR_INVALID_ARGS;
    }
    
    if (!t2dbus_handle.is_initialized) {
        if (dBusInterface_Init() != T2ERROR_SUCCESS) {
            return T2ERROR_FAILURE;
        }
    }
    
    DBusMessage *msg = NULL;
    DBusMessage *reply = NULL;
    DBusError error;
    dbus_error_init(&error);
    
    /* Create method call message */
    msg = dbus_message_new_method_call(T2_DBUS_SERVICE_NAME,
                                       T2_DBUS_OBJECT_PATH,
                                       T2_DBUS_INTERFACE_NAME,
                                       T2_DBUS_METHOD_GET_PARAM);
    if (!msg) {
        T2Error("Failed to create D-Bus message\n");
        return T2ERROR_FAILURE;
    }
    
    /* Append parameter name */
    if (!dbus_message_append_args(msg, DBUS_TYPE_STRING, &paramName, DBUS_TYPE_INVALID)) {
        T2Error("Failed to append arguments\n");
        dbus_message_unref(msg);
        return T2ERROR_FAILURE;
    }
    
    /* Send message and get reply */
    reply = dbus_connection_send_with_reply_and_block(t2dbus_handle.connection, msg,
                                                       T2_DBUS_DEFAULT_TIMEOUT_MS, &error);
    dbus_message_unref(msg);
    
    if (dbus_error_is_set(&error)) {
        T2Error("D-Bus error: %s\n", error.message);
        dbus_error_free(&error);
        return T2ERROR_FAILURE;
    }
    
    if (!reply) {
        T2Error("No reply received\n");
        return T2ERROR_FAILURE;
    }
    
    /* Parse reply */
    char *value = NULL;
    if (dbus_message_get_args(reply, &error, 
                             DBUS_TYPE_STRING, &value, 
                             DBUS_TYPE_INVALID)) {
        *paramValue = strdup(value);
        T2Debug("Retrieved value: %s = %s\n", paramName, *paramValue);
    } else {
        T2Error("Failed to parse reply: %s\n", error.message);
        dbus_error_free(&error);
        dbus_message_unref(reply);
        return T2ERROR_FAILURE;
    }
    
    dbus_message_unref(reply);
    
    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

/**
 * @brief Set parameter value via D-Bus
 */
T2ERROR setDbusParameterVal(const char* paramName, const char* paramValue, int paramType) {
    T2Debug("%s ++in\n", __FUNCTION__);
    
    if (!paramName || !paramValue) {
        T2Error("Invalid parameters\n");
        return T2ERROR_INVALID_ARGS;
    }
    
    if (!t2dbus_handle.is_initialized) {
        if (dBusInterface_Init() != T2ERROR_SUCCESS) {
            return T2ERROR_FAILURE;
        }
    }
    
    DBusMessage *msg = NULL;
    DBusMessage *reply = NULL;
    DBusError error;
    dbus_error_init(&error);
    
    /* Create method call message */
    msg = dbus_message_new_method_call(T2_DBUS_SERVICE_NAME,
                                       T2_DBUS_OBJECT_PATH,
                                       T2_DBUS_INTERFACE_NAME,
                                       T2_DBUS_METHOD_SET_PARAM);
    if (!msg) {
        T2Error("Failed to create D-Bus message\n");
        return T2ERROR_FAILURE;
    }
    
    /* Append arguments */
    if (!dbus_message_append_args(msg, 
                                  DBUS_TYPE_STRING, &paramName,
                                  DBUS_TYPE_STRING, &paramValue,
                                  DBUS_TYPE_INT32, &paramType,
                                  DBUS_TYPE_INVALID)) {
        T2Error("Failed to append arguments\n");
        dbus_message_unref(msg);
        return T2ERROR_FAILURE;
    }
    
    /* Send message and get reply */
    reply = dbus_connection_send_with_reply_and_block(t2dbus_handle.connection, msg,
                                                       T2_DBUS_DEFAULT_TIMEOUT_MS, &error);
    dbus_message_unref(msg);
    
    if (dbus_error_is_set(&error)) {
        T2Error("D-Bus error: %s\n", error.message);
        dbus_error_free(&error);
        return T2ERROR_FAILURE;
    }
    
    if (reply) {
        dbus_message_unref(reply);
    }
    
    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}


/**
 * @brief Register for telemetry event notifications
 */
T2ERROR registerDbusT2EventListener(TelemetryEventCallback eventCB) {
    T2Debug("%s ++in\n", __FUNCTION__);
    
    if (!eventCB) {
        T2Error("Invalid callback\n");
        return T2ERROR_INVALID_ARGS;
    }
    
    eventCallBack = eventCB;
    
    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

/**
 * @brief Unregister from telemetry event notifications
 */
T2ERROR unregisterDbusT2EventListener(void) {
    T2Debug("%s ++in\n", __FUNCTION__);
    
    eventCallBack = NULL;
    
    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

T2ERROR registerGetMarkerListCallback(T2EventMarkerListCallback callback) {
    T2Debug("%s ++in\n", __FUNCTION__);
    
    if (!callback) {
        T2Error("Invalid callback\n");
        return T2ERROR_INVALID_ARGS;
    }
    
    getMarkerListCallBack = callback;
    T2Info("Registered GetMarkerList callback\n");
    T2Debug("%s --out\n", __FUNCTION__);
    
    return T2ERROR_SUCCESS;
}
