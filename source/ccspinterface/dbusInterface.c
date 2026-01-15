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

/* D-Bus Service Configuration */
#define T2_DBUS_SERVICE_NAME        "telemetry.t2"
#define T2_DBUS_OBJECT_PATH         "/telemetry/t2"
#define T2_DBUS_INTERFACE_NAME      "telemetry.t2.interface"
#define T2_DBUS_SIGNAL_EVENT        "TelemetryEvent"
#define T2_DBUS_SIGNAL_PROFILE_UPDATE "ProfileUpdate"

/* Global D-Bus handle */
static T2DbusHandle_t t2dbus_handle = {NULL, NULL, false};
static void (*profileUpdateCallback)(void) = NULL;

/* Callback handlers */
static TelemetryEventCallback eventCallBack = NULL;
static T2EventMarkerListCallback getMarkerListCallBack = NULL;
static dataModelCallBack dmProcessingCallBack = NULL;
static dataModelMsgPckCallBack dmMsgPckProcessingCallBack = NULL;
static dataModelSavedJsonCallBack dmSavedJsonProcessingCallBack = NULL;
static dataModelSavedMsgPackCallBack dmSavedMsgPackProcessingCallBack = NULL;
static profilememCallBack profilememUsedCallBack = NULL;
static dataModelReportOnDemandCallBack reportOnDemandCallBack = NULL;
static triggerReportOnCondtionCallBack reportOnConditionCallBack = NULL;
static xconfPrivacyModesDoNotShareCallBack privacyModesDoNotShareCallBack = NULL;
static ReportProfilesDeleteDNDCallBack mprofilesDeleteCallBack = NULL;

/* State variables */
static uint32_t t2ReadyStatus = T2_STATE_NOT_READY;
static char* reportProfileVal = NULL;
static char* tmpReportProfileVal = NULL;
static char* reportProfilemsgPckVal = NULL;

/* Threading */
static pthread_mutex_t dbusMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t dbusListenerThread;
static bool stopListenerThread = false;

/* Component to parameter map */
static hash_map_t *compTr181ParamMap = NULL;
static pthread_mutex_t compParamMap = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief Check if D-Bus is initialized
 */
bool isDbusInitialized(void) {
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
    
    T2Debug("%s ++in\n", __FUNCTION__);
    
    if (dbus_message_is_signal(message, T2_DBUS_INTERFACE_NAME, T2_DBUS_SIGNAL_EVENT)) {
        DBusMessageIter args;
        char *eventName = NULL;
        char *eventValue = NULL;
        
        if (!dbus_message_iter_init(message, &args)) {
            T2Error("Event signal has no arguments\n");
        } else {
            /* Get event name */
            if (dbus_message_iter_get_arg_type(&args) == DBUS_TYPE_STRING) {
                dbus_message_iter_get_basic(&args, &eventName);
                dbus_message_iter_next(&args);
            }
            
            /* Get event value */
            if (dbus_message_iter_get_arg_type(&args) == DBUS_TYPE_STRING) {
                dbus_message_iter_get_basic(&args, &eventValue);
            }
            
            if (eventName && eventValue && eventCallBack) {
                T2Debug("Received event: name=%s, value=%s\n", eventName, eventValue);
                eventCallBack(strdup(eventName), strdup(eventValue));
            }
        }
        
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    else if (dbus_message_is_signal(message, T2_DBUS_INTERFACE_NAME, T2_DBUS_SIGNAL_PROFILE_UPDATE)) {
        T2Info("Received ProfileUpdate signal\n");
        /* Invoke profile update callback */
        if (profileUpdateCallback) {
            profileUpdateCallback();
        }
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    
    T2Debug("%s --out\n", __FUNCTION__);
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

/**
 * @brief D-Bus listener thread function
 */
static void* dbusListenerThreadFunc(void *arg) {
    (void)arg;
    
    T2Debug("%s ++in\n", __FUNCTION__);
    
    while (!stopListenerThread && t2dbus_handle.connection) {
        /* Process messages with timeout */
        dbus_connection_read_write_dispatch(t2dbus_handle.connection, 100);
    }
    
    T2Debug("%s --out\n", __FUNCTION__);
    return NULL;
}

/**
 * @brief Initialize D-Bus interface
 */
T2ERROR dBusInterface_Init(const char *component_name) {
    T2Debug("%s ++in\n", __FUNCTION__);
    
    pthread_mutex_lock(&dbusMutex);
    
    if (t2dbus_handle.is_initialized) {
        T2Warning("D-Bus interface already initialized\n");
        pthread_mutex_unlock(&dbusMutex);
        return T2ERROR_SUCCESS;
    }
    
    DBusError error;
    dbus_error_init(&error);
    
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
        T2Error("D-Bus name request error: %s\n", error.message);
        dbus_error_free(&error);
        dbus_connection_unref(t2dbus_handle.connection);
        t2dbus_handle.connection = NULL;
        pthread_mutex_unlock(&dbusMutex);
        return T2ERROR_FAILURE;
    }
    
    if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        T2Warning("Not primary owner of D-Bus name\n");
    }
    
    /* Store unique name */
    t2dbus_handle.unique_name = strdup(dbus_bus_get_unique_name(t2dbus_handle.connection));
    
    /* Add message filter */
    dbus_connection_add_filter(t2dbus_handle.connection, dbusMessageFilter, NULL, NULL);
    
    t2dbus_handle.is_initialized = true;
    
    /* Start listener thread */
    stopListenerThread = false;
    if (pthread_create(&dbusListenerThread, NULL, dbusListenerThreadFunc, NULL) != 0) {
        T2Error("Failed to create D-Bus listener thread\n");
        dBusInterface_Uninit();
        pthread_mutex_unlock(&dbusMutex);
        return T2ERROR_FAILURE;
    }
    
    pthread_mutex_unlock(&dbusMutex);
    
    T2Info("D-Bus interface initialized successfully with name: %s\n", service_name);
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
        if (dBusInterface_Init(NULL) != T2ERROR_SUCCESS) {
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
        if (dBusInterface_Init(NULL) != T2ERROR_SUCCESS) {
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
 * @brief Get multiple profile parameter values via D-Bus
 */
Vector* getDbusProfileParamValues(Vector *paramList, int execcount) {
    T2Debug("%s ++in\n", __FUNCTION__);
    
    if (!paramList) {
        T2Error("Invalid parameter list\n");
        return NULL;
    }
    
    if (!t2dbus_handle.is_initialized) {
        if (dBusInterface_Init(NULL) != T2ERROR_SUCCESS) {
            return NULL;
        }
    }
    
    Vector *profileValueList = NULL;
    Vector_Create(&profileValueList);
    
    /* Iterate through parameters and fetch values */
    for (size_t i = 0; i < Vector_Size(paramList); i++) {
        Param *param = (Param *)Vector_At(paramList, i);
        if (!param) continue;
        
        profileValues *profVals = (profileValues *)malloc(sizeof(profileValues));
        if (!profVals) continue;
        
        /* Check skip frequency */
        if (param->skipFreq > 0 && (execcount % (param->skipFreq + 1) != 0)) {
            T2Debug("Skipping parameter: %s as per skipFreq: %d\n", 
                    param->name, param->skipFreq);
            profVals->paramValues = NULL;
            profVals->paramValueCount = 0;
            Vector_PushBack(profileValueList, profVals);
            continue;
        }
        
        /* Get parameter value */
        char *value = NULL;
        if (getDbusParameterVal(param->alias, &value) == T2ERROR_SUCCESS && value) {
            tr181ValStruct_t **paramValues = (tr181ValStruct_t **)malloc(sizeof(tr181ValStruct_t *));
            paramValues[0] = (tr181ValStruct_t *)malloc(sizeof(tr181ValStruct_t));
            paramValues[0]->parameterName = strdup(param->alias);
            paramValues[0]->parameterValue = value;
            
            profVals->paramValues = paramValues;
            profVals->paramValueCount = 1;
        } else {
            /* Parameter not found */
            tr181ValStruct_t **paramValues = (tr181ValStruct_t **)malloc(sizeof(tr181ValStruct_t *));
            paramValues[0] = (tr181ValStruct_t *)malloc(sizeof(tr181ValStruct_t));
            paramValues[0]->parameterName = strdup(param->alias);
            paramValues[0]->parameterValue = strdup("NULL");
            
            profVals->paramValues = paramValues;
            profVals->paramValueCount = 1;
        }
        
        Vector_PushBack(profileValueList, profVals);
    }
    
    T2Debug("%s --out\n", __FUNCTION__);
    return profileValueList;
}

/**
 * @brief Publish telemetry event via D-Bus signal
 */
T2ERROR dbusPublishEvent(const char* eventName, const char* eventValue) {
    T2Debug("%s ++in\n", __FUNCTION__);
    
    if (!eventName || !eventValue) {
        T2Error("Invalid event parameters\n");
        return T2ERROR_INVALID_ARGS;
    }
    
    if (!t2dbus_handle.is_initialized) {
        if (dBusInterface_Init(NULL) != T2ERROR_SUCCESS) {
            return T2ERROR_FAILURE;
        }
    }
    
    DBusMessage *signal = NULL;
    
    /* Create signal */
    signal = dbus_message_new_signal(T2_DBUS_OBJECT_PATH,
                                     T2_DBUS_INTERFACE_NAME,
                                     T2_DBUS_SIGNAL_EVENT);
    if (!signal) {
        T2Error("Failed to create D-Bus signal\n");
        return T2ERROR_FAILURE;
    }
    
    /* Append arguments */
    if (!dbus_message_append_args(signal,
                                  DBUS_TYPE_STRING, &eventName,
                                  DBUS_TYPE_STRING, &eventValue,
                                  DBUS_TYPE_INVALID)) {
        T2Error("Failed to append signal arguments\n");
        dbus_message_unref(signal);
        return T2ERROR_FAILURE;
    }
    
    /* Send signal */
    if (!dbus_connection_send(t2dbus_handle.connection, signal, NULL)) {
        T2Error("Failed to send D-Bus signal\n");
        dbus_message_unref(signal);
        return T2ERROR_FAILURE;
    }
    
    dbus_connection_flush(t2dbus_handle.connection);
    dbus_message_unref(signal);
    
    T2Debug("Published event: %s = %s\n", eventName, eventValue);
    T2Debug("%s --out\n", __FUNCTION__);
    
    return T2ERROR_SUCCESS;
}

/**
 * @brief Subscribe to D-Bus signal
 */
T2ERROR dbusSubscribeSignal(const char* signal_name) {
    T2Debug("%s ++in\n", __FUNCTION__);
    
    if (!signal_name) {
        T2Error("Invalid signal name\n");
        return T2ERROR_INVALID_ARGS;
    }
    
    if (!t2dbus_handle.is_initialized) {
        if (dBusInterface_Init(NULL) != T2ERROR_SUCCESS) {
            return T2ERROR_FAILURE;
        }
    }
    
    char rule[512];
    DBusError error;
    dbus_error_init(&error);
    
    /* Create match rule */
    snprintf(rule, sizeof(rule), 
             "type='signal',interface='%s',member='%s'",
             T2_DBUS_INTERFACE_NAME, signal_name);
    
    dbus_bus_add_match(t2dbus_handle.connection, rule, &error);
    
    if (dbus_error_is_set(&error)) {
        T2Error("Failed to add match rule: %s\n", error.message);
        dbus_error_free(&error);
        return T2ERROR_FAILURE;
    }
    
    T2Info("Subscribed to signal: %s\n", signal_name);
    T2Debug("%s --out\n", __FUNCTION__);
    
    return T2ERROR_SUCCESS;
}

/**
 * @brief Unsubscribe from D-Bus signal
 */
T2ERROR dbusUnsubscribeSignal(const char* signal_name) {
    T2Debug("%s ++in\n", __FUNCTION__);
    
    if (!signal_name) {
        T2Error("Invalid signal name\n");
        return T2ERROR_INVALID_ARGS;
    }
    
    if (!t2dbus_handle.is_initialized) {
        return T2ERROR_NOT_INITIALIZED;
    }
    
    char rule[512];
    DBusError error;
    dbus_error_init(&error);
    
    /* Create match rule */
    snprintf(rule, sizeof(rule),
             "type='signal',interface='%s',member='%s'",
             T2_DBUS_INTERFACE_NAME, signal_name);
    
    dbus_bus_remove_match(t2dbus_handle.connection, rule, &error);
    
    if (dbus_error_is_set(&error)) {
        T2Error("Failed to remove match rule: %s\n", error.message);
        dbus_error_free(&error);
        return T2ERROR_FAILURE;
    }
    
    T2Info("Unsubscribed from signal: %s\n", signal_name);
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
    
    /* Subscribe to event signal */
    if (dbusSubscribeSignal(T2_DBUS_SIGNAL_EVENT) != T2ERROR_SUCCESS) {
        T2Error("Failed to subscribe to event signal\n");
        return T2ERROR_FAILURE;
    }
    
    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

/**
 * @brief Unregister from telemetry event notifications
 */
T2ERROR unregisterDbusT2EventListener(void) {
    T2Debug("%s ++in\n", __FUNCTION__);
    
    eventCallBack = NULL;
    
    /* Unsubscribe from event signal */
    dbusUnsubscribeSignal(T2_DBUS_SIGNAL_EVENT);
    
    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

/**
 * @brief Publish report upload status
 */
void dbusPublishReportUploadStatus(const char* status) {
    T2Debug("%s ++in\n", __FUNCTION__);
    
    if (!status) {
        T2Error("Invalid status\n");
        return;
    }
    
    if (!t2dbus_handle.is_initialized) {
        T2Warning("D-Bus not initialized\n");
        return;
    }
    
    DBusMessage *signal = dbus_message_new_signal(T2_DBUS_OBJECT_PATH,
                                                  T2_DBUS_INTERFACE_NAME,
                                                  T2_DBUS_SIGNAL_UPLOAD_STATUS);
    if (!signal) {
        T2Error("Failed to create signal\n");
        return;
    }
    
    if (!dbus_message_append_args(signal, DBUS_TYPE_STRING, &status, DBUS_TYPE_INVALID)) {
        T2Error("Failed to append arguments\n");
        dbus_message_unref(signal);
        return;
    }
    
    dbus_connection_send(t2dbus_handle.connection, signal, NULL);
    dbus_connection_flush(t2dbus_handle.connection);
    dbus_message_unref(signal);
    
    T2Info("Published upload status: %s\n", status);
    T2Debug("%s --out\n", __FUNCTION__);
}

/**
 * @brief Set T2 event receiver state
 */
void dbusSetT2EventReceiveState(int t2_state) {
    t2ReadyStatus = t2_state;
    T2Info("T2 state set to: %d\n", t2_state);
}

/**
 * @brief Check if a D-Bus method exists
 */
bool dbusCheckMethodExists(const char* methodName) {
    T2Debug("%s ++in: %s\n", __FUNCTION__, methodName);
    
    if (!methodName) {
        return false;
    }
    
    /* For now, return true for standard methods */
    /* In production, this should introspect the remote object */
    
    T2Debug("%s --out\n", __FUNCTION__);
    return true;
}

/** * @brief Get marker list for component via D-Bus method call
 */
T2ERROR dbusGetMarkerList(const char* component, char** markerList) {
    T2Debug("%s ++in\n", __FUNCTION__);
    
    if (!component || !markerList) {
        T2Error("Invalid arguments\n");
        return T2ERROR_INVALID_ARGS;
    }
    
    if (!t2dbus_handle.is_initialized) {
        T2Error("D-Bus not initialized\n");
        return T2ERROR_NOT_INITIALIZED;
    }
    
    DBusMessage *msg = NULL;
    DBusMessage *reply = NULL;
    DBusError error;
    dbus_error_init(&error);
    
    /* Construct parameter name: Telemetry.ReportProfiles.{component}.EventMarkerList */
    char paramName[256];
    snprintf(paramName, sizeof(paramName), "Telemetry.ReportProfiles.%s.EventMarkerList", component);
    
    /* Create method call message */
    msg = dbus_message_new_method_call(T2_DBUS_SERVICE_NAME,
                                       T2_DBUS_OBJECT_PATH,
                                       T2_DBUS_INTERFACE_NAME,
                                       "GetMarkerList");
    if (!msg) {
        T2Error("Failed to create method call message\n");
        return T2ERROR_FAILURE;
    }
    
    /* Append component name */
    if (!dbus_message_append_args(msg, DBUS_TYPE_STRING, &component, DBUS_TYPE_INVALID)) {
        T2Error("Failed to append arguments\n");
        dbus_message_unref(msg);
        return T2ERROR_FAILURE;
    }
    
    /* Send message and get reply */
    reply = dbus_connection_send_with_reply_and_block(t2dbus_handle.connection, msg,
                                                       T2_DBUS_DEFAULT_TIMEOUT_MS, &error);
    dbus_message_unref(msg);
    
    if (dbus_error_is_set(&error)) {
        T2Error("D-Bus method call failed: %s\n", error.message);
        dbus_error_free(&error);
        return T2ERROR_FAILURE;
    }
    
    if (!reply) {
        T2Error("No reply received\n");
        return T2ERROR_FAILURE;
    }
    
    /* Parse reply - expecting a string containing the marker list */
    char *value = NULL;
    if (dbus_message_get_args(reply, &error, 
                             DBUS_TYPE_STRING, &value,
                             DBUS_TYPE_INVALID)) {
        *markerList = strdup(value);
        T2Debug("Retrieved marker list for %s\n", component);
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
 * @brief Get operational status via D-Bus method call
 */
T2ERROR dbusGetOperationalStatus(uint32_t* status) {
    T2Debug("%s ++in\n", __FUNCTION__);
    
    if (!status) {
        T2Error("Invalid arguments\n");
        return T2ERROR_INVALID_ARGS;
    }
    
    if (!t2dbus_handle.is_initialized) {
        T2Error("D-Bus not initialized\n");
        return T2ERROR_NOT_INITIALIZED;
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
        T2Error("Failed to create method call message\n");
        return T2ERROR_FAILURE;
    }
    
    /* Send message and get reply */
    reply = dbus_connection_send_with_reply_and_block(t2dbus_handle.connection, msg,
                                                       T2_DBUS_DEFAULT_TIMEOUT_MS, &error);
    dbus_message_unref(msg);
    
    if (dbus_error_is_set(&error)) {
        T2Error("D-Bus method call failed: %s\n", error.message);
        dbus_error_free(&error);
        return T2ERROR_FAILURE;
    }
    
    if (!reply) {
        T2Error("No reply received\n");
        return T2ERROR_FAILURE;
    }
    
    /* Parse reply - expecting UINT32 */
    if (dbus_message_get_args(reply, &error, 
                             DBUS_TYPE_UINT32, status,
                             DBUS_TYPE_INVALID)) {
        T2Debug("Retrieved operational status: %u\n", *status);
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
 * @brief Subscribe to profile update notifications
 */
T2ERROR dbusSubscribeProfileUpdate(void (*callback)(void)) {
    T2Debug("%s ++in\n", __FUNCTION__);
    
    if (!callback) {
        T2Error("Invalid callback\n");
        return T2ERROR_INVALID_ARGS;
    }
    
    if (!t2dbus_handle.is_initialized) {
        T2Error("D-Bus not initialized\n");
        return T2ERROR_NOT_INITIALIZED;
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
        T2Error("Failed to add match rule: %s\n", error.message);
        dbus_error_free(&error);
        return T2ERROR_FAILURE;
    }
    
    T2Info("Subscribed to ProfileUpdate signal\n");
    T2Debug("%s --out\n", __FUNCTION__);
    
    return T2ERROR_SUCCESS;
}

/** * @brief Process incoming D-Bus messages
 */
T2ERROR dbusProcessMessages(int timeout_ms) {
    if (!t2dbus_handle.is_initialized || !t2dbus_handle.connection) {
        return T2ERROR_NOT_INITIALIZED;
    }
    
    dbus_connection_read_write_dispatch(t2dbus_handle.connection, timeout_ms);
    
    return T2ERROR_SUCCESS;
}

/**
 