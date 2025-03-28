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

#ifndef _RBUSINTERFACE_H_
#define _RBUSINTERFACE_H_

#include <stdbool.h>
#include <stdio.h>
#include <vector.h>
#ifdef GTEST_ENABLE
#include "test/rbus/include/rbus.h"
#else
#include <rbus/rbus.h>
#endif
#include "busInterface.h"
#include "telemetry2_0.h"

typedef void (*rbusMethodCallBackPtr)(rbusHandle_t handle,
                                      char const *methodName,
                                      rbusError_t retStatus,
                                      rbusObject_t params);

T2ERROR getRbusParameterVal(const char *paramName, char **paramValue);

Vector *getRbusProfileParamValues(Vector *paramList);

T2ERROR registerRbusT2EventListener(TelemetryEventCallback eventCB);

#ifdef DCMAGENT
T2ERROR registerRbusDCMEventListener();
#endif

T2ERROR unregisterRbusT2EventListener();

T2ERROR rbusT2ConsumerReg(Vector *triggerConditionList);

T2ERROR rbusT2ConsumerUnReg(Vector *triggerConditionList);

T2ERROR rbusMethodCaller(char *methodName, rbusObject_t *inputParams,
                         char *payload,
                         rbusMethodCallBackPtr rbusMethodCallBack);

bool rbusCheckMethodExists(const char *rbusMethodName);

T2ERROR T2RbusReportEventConsumer(char *reference, bool subscription);

#endif
