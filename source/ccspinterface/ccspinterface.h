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

#if defined(CCSP_SUPPORT_ENABLED)

#ifndef _CCSPINTERFACE_H_
#define _CCSPINTERFACE_H_

#include <ccsp/ccsp_base_api.h>
#include <stdio.h>
#include <vector.h>

#include "busInterface.h"
#include "telemetry2_0.h"

bool isCCSPInitialized();

T2ERROR getCCSPParamVal(const char* paramName, char **paramValue);

Vector* getCCSPProfileParamValues(Vector *paramList, int execount);

T2ERROR registerCcspT2EventListener(TelemetryEventCallback eventCB);

#endif
#endif // CCSP_SUPPORT_ENABLED 
