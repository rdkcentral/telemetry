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
 * @file dbusInterface.h
 * @brief D-Bus Interface for Telemetry 2.0
 *
 * This header provides D-Bus based inter-process communication APIs
 * as an alternative to RBUS for the Telemetry 2.0 framework.
 *
 * Key Features:
 * - Parameter get/set operations
 * - Event publishing and subscription
 * - Method invocation
 * - Signal handling
 */

#ifndef _DBUSINTERFACE_H_
#define _DBUSINTERFACE_H_

#include <stdio.h>
#include <stdbool.h>
#include <vector.h>
#include <dbus/dbus.h>
#include "busInterface.h"
#include "telemetry2_0.h"

/* D-Bus Service Information */
#define T2_DBUS_SERVICE_NAME        "telemetry.t2"
#define T2_DBUS_OBJECT_PATH         "/telemetry/t2"
#define T2_DBUS_INTERFACE_NAME      "telemetry.t2.interface"

/* D-Bus Method Names */
#define T2_DBUS_METHOD_GET_PARAM    "GetParameter"
#define T2_DBUS_METHOD_SET_PARAM    "SetParameter"
#define T2_DBUS_METHOD_SUBSCRIBE    "SubscribeEvent"
#define T2_DBUS_METHOD_UNSUBSCRIBE  "UnsubscribeEvent"
#define T2_DBUS_METHOD_CALL         "CallMethod"

/* D-Bus Signal Names */
#define T2_DBUS_SIGNAL_EVENT        "TelemetryEvent"
#define T2_DBUS_SIGNAL_PROFILE_UPDATE "ProfileUpdate"
#define T2_DBUS_SIGNAL_UPLOAD_STATUS  "UploadStatus"

/* D-Bus Error Codes */
#define T2_DBUS_ERROR_SUCCESS           0
#define T2_DBUS_ERROR_FAILURE          -1
#define T2_DBUS_ERROR_OUT_OF_MEMORY    -2
#define T2_DBUS_ERROR_INVALID_PARAM    -3
#define T2_DBUS_ERROR_NOT_INITIALIZED  -4
#define T2_DBUS_ERROR_TIMEOUT          -5
#define T2_DBUS_ERROR_NO_ELEMENT       -6

/* Timeout values */
#define T2_DBUS_DEFAULT_TIMEOUT_MS     10000  /* 10 seconds */
#define T2_DBUS_METHOD_TIMEOUT_MS      10000  /* 10 seconds */

/**
 * @brief D-Bus connection handle
 */
typedef struct {
    DBusConnection *connection;
    char *unique_name;
    bool is_initialized;
} T2DbusHandle_t;

/**
 * @brief D-Bus method callback pointer
 */
typedef void (*dbusMethodCallBackPtr)(DBusMessage *reply, int retStatus);

/**
 * @brief Check if D-Bus is initialized
 * @return true if initialized, false otherwise
 */
bool isDbusInitialized(void);

/**
 * @brief Initialize D-Bus interface for Telemetry 2.0
 * @param component_name Unique component name for registration
 * @return T2ERROR_SUCCESS on success, T2ERROR_FAILURE on failure
 */
T2ERROR dBusInterface_Init(const char *component_name);

/**
 * @brief Uninitialize D-Bus interface
 */
void dBusInterface_Uninit(void);

/**
 * @brief Get parameter value via D-Bus
 * @param paramName Parameter name to query
 * @param paramValue Pointer to store retrieved value (caller must free)
 * @return T2ERROR_SUCCESS on success, T2ERROR_FAILURE on failure
 */
T2ERROR getDbusParameterVal(const char* paramName, char **paramValue);

/**
 * @brief Set parameter value via D-Bus
 * @param paramName Parameter name to set
 * @param paramValue Value to set
 * @param paramType Type of parameter (string, int, boolean, etc.)
 * @return T2ERROR_SUCCESS on success, T2ERROR_FAILURE on failure
 */
T2ERROR setDbusParameterVal(const char* paramName, const char* paramValue, int paramType);

/**
 * @brief Get multiple profile parameter values via D-Bus
 * @param paramList Vector containing list of parameters to query
 * @param execcount Execution count for skip frequency calculation
 * @return Vector of profileValues structures
 */
Vector* getDbusProfileParamValues(Vector *paramList, int execcount);

/**
 * @brief Register for telemetry event notifications via D-Bus
 * @param eventCB Callback function to handle events
 * @return T2ERROR_SUCCESS on success, T2ERROR_FAILURE on failure
 */
T2ERROR registerDbusT2EventListener(TelemetryEventCallback eventCB);

/**
 * @brief Unregister from telemetry event notifications
 * @return T2ERROR_SUCCESS on success, T2ERROR_FAILURE on failure
 */
T2ERROR unregisterDbusT2EventListener(void);

/**
 * @brief Subscribe to a specific D-Bus signal
 * @param signal_name Name of the signal to subscribe
 * @return T2ERROR_SUCCESS on success, T2ERROR_FAILURE on failure
 */
T2ERROR dbusSubscribeSignal(const char* signal_name);

/**
 * @brief Unsubscribe from a specific D-Bus signal
 * @param signal_name Name of the signal to unsubscribe
 * @return T2ERROR_SUCCESS on success, T2ERROR_FAILURE on failure
 */
T2ERROR dbusUnsubscribeSignal(const char* signal_name);

/**
 * @brief Publish telemetry event via D-Bus signal
 * @param eventName Event name/marker
 * @param eventValue Event value
 * @return T2ERROR_SUCCESS on success, T2ERROR_FAILURE on failure
 */
T2ERROR dbusPublishEvent(const char* eventName, const char* eventValue);

/**
 * @brief Get marker list for component via D-Bus method call
 * @param component Component name
 * @param markerList Pointer to store marker list string (caller must free)
 * @return T2ERROR_SUCCESS on success, T2ERROR_FAILURE on failure
 */
T2ERROR dbusGetMarkerList(const char* component, char** markerList);

/**
 * @brief Get operational status via D-Bus method call
 * @param status Pointer to store status value
 * @return T2ERROR_SUCCESS on success, T2ERROR_FAILURE on failure
 */
T2ERROR dbusGetOperationalStatus(uint32_t* status);

/**
 * @brief Subscribe to profile update notifications
 * @param callback Callback function to invoke on profile update
 * @return T2ERROR_SUCCESS on success, T2ERROR_FAILURE on failure
 */
T2ERROR dbusSubscribeProfileUpdate(void (*callback)(void));

/**
 * @brief Call a D-Bus method
 * @param methodName Method name to invoke
 * @param inputParams Input parameters as D-Bus message
 * @param payload Payload data
 * @param callback Callback function for asynchronous response
 * @return T2ERROR_SUCCESS on success, T2ERROR_FAILURE on failure
 */
T2ERROR dbusMethodCaller(const char *methodName, DBusMessage* inputParams, 
                         const char* payload, dbusMethodCallBackPtr callback);

/**
 * @brief Check if a D-Bus method exists
 * @param methodName Name of the method to check
 * @return true if method exists, false otherwise
 */
bool dbusCheckMethodExists(const char* methodName);

/**
 * @brief Subscribe to trigger condition reports
 * @param reference Reference parameter name
 * @param subscription true to subscribe, false to unsubscribe
 * @return T2ERROR_SUCCESS on success, T2ERROR_FAILURE on failure
 */
T2ERROR T2DbusReportEventConsumer(const char* reference, bool subscription);

/**
 * @brief Register consumer for trigger conditions
 * @param triggerConditionList List of trigger conditions
 * @return T2ERROR_SUCCESS on success, T2ERROR_FAILURE on failure
 */
T2ERROR dbusT2ConsumerReg(Vector *triggerConditionList);

/**
 * @brief Unregister consumer for trigger conditions
 * @param triggerConditionList List of trigger conditions
 * @return T2ERROR_SUCCESS on success, T2ERROR_FAILURE on failure
 */
T2ERROR dbusT2ConsumerUnReg(Vector *triggerConditionList);

/**
 * @brief Publish report upload status via D-Bus
 * @param status Status string to publish
 */
void dbusPublishReportUploadStatus(const char* status);

/**
 * @brief Set T2 event receiver state
 * @param t2_state State value to set
 */
void dbusSetT2EventReceiveState(int t2_state);

/**
 * @brief Process incoming D-Bus messages
 * @param timeout_ms Timeout in milliseconds (-1 for blocking)
 * @return T2ERROR_SUCCESS on success, T2ERROR_FAILURE on failure
 */
T2ERROR dbusProcessMessages(int timeout_ms);

/**
 * @brief Create D-Bus message iterator for parameters
 * @param iter D-Bus message iterator
 * @param paramName Parameter name
 * @param paramValue Parameter value
 * @param paramType Parameter type
 * @return T2ERROR_SUCCESS on success, T2ERROR_FAILURE on failure
 */
T2ERROR dbusAppendParameter(DBusMessageIter *iter, const char* paramName, 
                           const char* paramValue, int paramType);

#endif /* _DBUSINTERFACE_H_ */
