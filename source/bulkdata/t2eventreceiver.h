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

#ifndef _T2EVENTRECEIVER_H_
#define _T2EVENTRECEIVER_H_

#include "telemetry2_0.h"

typedef struct _T2Event
{
    char* name;
    char* value;
} T2Event;

T2ERROR T2ER_Init();

void T2ER_Uninit();

T2ERROR T2ER_StartDispatchThread();

T2ERROR T2ER_StopDispatchThread();

void freeT2Event(void *data);

void* T2ER_EventDispatchThread(void *arg);

void T2ER_PushDataWithDelim(char* eventInfo, char* user_data);

void T2ER_Push(char* eventName, char* eventValue);

#endif /* SOURCE_BULKDATA_T2EVENTRECEIVER_H_ */
