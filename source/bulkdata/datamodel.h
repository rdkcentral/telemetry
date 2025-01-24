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

#ifndef  _DATAMODEL_H_
#define  _DATAMODEL_H_

#include <stdbool.h>
#include "telemetry2_0.h"

T2ERROR datamodel_init(void);

void datamodel_unInit(void);

T2ERROR datamodel_processProfile(char *JsonBlob , bool rprofiletypes);

void datamodel_getSavedJsonProfilesasString(char** SavedProfiles);

int datamodel_getSavedMsgpackProfilesasString(char** SavedProfiles);

T2ERROR datamodel_MsgpackProcessProfile(char *str , int strSize);
#endif
