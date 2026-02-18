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

#include <stdbool.h>
#include <cjson/cJSON.h>
#include "vector.h"
#include "test/bulkdata/reportprofileMock.h"



// Mock Method

extern "C" bool __wrap_isRbusEnabled()
{
    if (!g_reportprofileMock)
    {
        return false;
    }
    return g_reportprofileMock->isRbusEnabled();
}

extern "C" {
T2ERROR __wrap_sendReportOverHTTP(char *httpUrl, char *payload)
{
    if (g_reportprofileMock) {
        return g_reportprofileMock->sendReportOverHTTP(httpUrl, payload);
    }
    return T2ERROR_FAILURE;
}

T2ERROR __wrap_sendCachedReportsOverHTTP(char *httpUrl, Vector *reportList)
{
    if (g_reportprofileMock) {
        return g_reportprofileMock->sendCachedReportsOverHTTP(httpUrl, reportList);
    }
    return T2ERROR_FAILURE;
}
}

