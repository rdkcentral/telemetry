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

typedef struct {
    DBusConnection *connection;
    char *unique_name;
    bool is_initialized;
} T2DbusHandle_t;

typedef void (*dbusMethodCallBackPtr)(DBusMessage *reply, int retStatus);

bool isDbusInitialized(void);
T2ERROR dBusInterface_Init(const char *component_name);
void dBusInterface_Uninit(void);

T2ERROR getDbusParameterVal(const char* paramName, char **paramValue);
T2ERROR setDbusParameterVal(const char* paramName, const char* paramValue, int paramType);
Vector* getDbusProfileParamValues(Vector *paramList, int execcount);

T2ERROR registerDbusT2EventListener(TelemetryEventCallback eventCB);
T2ERROR unregisterDbusT2EventListener(void);

T2ERROR dbusSubscribeSignal(const char* signal_name);
T2ERROR dbusUnsubscribeSignal(const char* signal_name);

void dbusPublishReportUploadStatus(const char* status);

T2ERROR registerDbusMethodProviders(void);
T2ERROR registerGetMarkerListCallback(T2EventMarkerListCallback callback);

#endif /* _DBUSINTERFACE_H_ */
