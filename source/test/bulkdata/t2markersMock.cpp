/*
* Copyright 2020 Comcast Cable Communications Management, LLC
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* SPDX-License-Identifier: Apache-2.0
*/
#include <stdbool.h>
#include <cjson/cJSON.h>
#include "test/bulkdata/t2markersMock.h"



// Mock Method
extern "C" T2ERROR ReportProfiles_storeMarkerEvent(char *profileName, T2Event *eventInfo)
{
    if (!g_t2markersMock)
    {
        return T2ERROR_FAILURE;
    }
    return g_t2markersMock->ReportProfiles_storeMarkerEvent(profileName, eventInfo);
}

extern "C" bool isRbusEnabled()
{
    if (!g_t2markersMock)
    {
        return false;
    }
    return g_t2markersMock->isRbusEnabled();
}

extern "C" T2ERROR registerForTelemetryEvents(TelemetryEventCallback eventCB)
{
    if (!g_t2markersMock)
    {
        return T2ERROR_FAILURE;
    }
    return g_t2markersMock->registerForTelemetryEvents(eventCB);
}
