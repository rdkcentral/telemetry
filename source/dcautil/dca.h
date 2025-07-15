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
#ifndef SRC_DCA_H_
#define SRC_DCA_H_
#include <cjson/cJSON.h>
#include "vector.h"
#include <stdbool.h>

/**
 * Caller should be freeing vectorMarkerList and grepResultList
 */

int getDCAResultsInVector(char* profileName, Vector* vectorMarkerList, Vector** out_grepResultList, bool check_rotated, char* customLogPath);

void T2InitProperties(); // declaration for initProperties

int processTopPattern(char* profileName, Vector* topMarkerList, Vector* out_grepResultList);

#endif /* SRC_DCA_H_ */
