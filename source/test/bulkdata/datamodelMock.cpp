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
#include "test/bulkdata/datamodelMock.h"



// Mock Method
extern "C" void __wrap_ReportProfiles_ProcessReportProfilesBlob(cJSON *profiles_root, bool rprofiletypes)
{
    if (!g_datamodelMock)
    {
        return;
    }
    return g_datamodelMock->ReportProfiles_ProcessReportProfilesBlob(profiles_root, rprofiletypes);
}

extern "C" void __wrap_ReportProfiles_ProcessReportProfilesMsgPackBlob(char *msgpack_blob, int msgpack_blob_size)
{
    if (!g_datamodelMock)
    {
        return;
    }
    return g_datamodelMock->ReportProfiles_ProcessReportProfilesMsgPackBlob(msgpack_blob, msgpack_blob_size);
}

