/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2021 RDK Management
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

#ifndef _RBUS_METHOD_INTERFACE_H_
#define _RBUS_METHOD_INTERFACE_H_

#include "telemetry2_0.h"
#include "vector.h"

T2ERROR sendReportsOverRBUSMethod(char *methodName, Vector* inputParams, char* payload);

T2ERROR sendCachedReportsOverRBUSMethod(char *methodName, Vector* inputParams, Vector* reportList);

#endif /* _RBUS_METHOD_INTERFACE_H_ */
