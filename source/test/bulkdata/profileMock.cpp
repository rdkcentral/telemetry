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
#include "vector.h"
#include "msgpack.h"
#include "test/bulkdata/profileMock.h"


//protocol mock functions
extern "C" T2ERROR __wrap_sendReportOverHTTP(char *httpUrl, char *payload)
{ 
    if(!g_profileMock)
    {
        return T2ERROR_FAILURE;
    }
    // No mock function needed, just a stub
    return g_profileMock->sendReportOverHTTP(httpUrl, payload);
}

extern "C" T2ERROR __wrap_sendCachedReportsOverHTTP(char *httpUrl, Vector *reportList)
{
    if(!g_profileMock)
    {
        return T2ERROR_FAILURE;
    }
    // No mock function needed, just a stub
    return g_profileMock->sendCachedReportsOverHTTP(httpUrl, reportList);
}

