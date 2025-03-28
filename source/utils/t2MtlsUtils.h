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

#ifndef _T2UTILS_H_
#define _T2UTILS_H_

#include <stdbool.h>
#include <stdint.h>

#include "telemetry2_0.h"

#define MAX_URL_LENGTH 1024

void initMtls();

void uninitMtls();

T2ERROR getMtlsCerts(char **certName, char **phrase);
double get_system_uptime();

#endif // _T2UTILS_H_
