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
#include "test/bulkdata/profilexconfMock.h"

extern "C" cJSON* cJSON_CreateObject(void)
{
    if(!g_profileXConfMock) {
        return nullptr;
    }
    return g_profileXConfMock->cJSON_CreateObject();
}

extern "C" cJSON* cJSON_CreateArray(void)
{
    if(!g_profileXConfMock) return nullptr;
    return g_profileXConfMock->cJSON_CreateArray();
}

extern "C" cJSON_bool cJSON_AddItemToObject(cJSON* object, const char* string, cJSON* item)
{
    if(!g_profileXConfMock) return 0;
    return g_profileXConfMock->cJSON_AddItemToObject(object, string, item);
}

extern "C" cJSON_bool cJSON_AddItemToArray(cJSON* array, cJSON* item)
{
    if(!g_profileXConfMock) return 0;
    return g_profileXConfMock->cJSON_AddItemToArray(array, item);
}

extern "C" cJSON* cJSON_AddStringToObject(cJSON* object, const char* string, const char* value)
{
    if(!g_profileXConfMock) return nullptr;
    return g_profileXConfMock->cJSON_AddStringToObject(object, string, value);
}
// Mock Method
//t2parser mock functions
extern "C" T2ERROR processConfigurationXConf(char* configData, ProfileXConf **localProfile)
{
    if (!g_profileXConfMock)
    {
        return T2ERROR_FAILURE;
    }
    return g_profileXConfMock->processConfigurationXConf(configData, localProfile);
}

//ccspinterface mock functions
extern "C" Vector* getProfileParameterValues(Vector *paramList, int count)
{
    if (!g_profileXConfMock )
    {
        return nullptr;
    }
    return g_profileXConfMock->getProfileParameterValues(paramList, count);
}

extern "C" void publishReportUploadStatus(char* status)
{
    if(!g_profileXConfMock)
    {
        return;
    }
    return g_profileXConfMock->publishReportUploadStatus(status);
}

extern "C" T2ERROR regDEforCompEventList(const char* componentName, T2EventMarkerListCallback callBackHandler)
{
    if(!g_profileXConfMock)
    {
        return T2ERROR_FAILURE;
    }
    return g_profileXConfMock->regDEforCompEventList(componentName, callBackHandler);
}

extern "C" T2ERROR getParameterValue(const char* paramName, char **paramValue)
{
    if(!g_profileXConfMock)
    {
        return T2ERROR_FAILURE;
    }
    return g_profileXConfMock->getParameterValue(paramName, paramValue);
}


//dcautil mock functions


extern "C" int processTopPattern(char* profileName,  Vector* topMarkerList, int profileExecCounter)
{
    if (!g_profileXConfMock)
    {
        return -1;
    }
    return g_profileXConfMock->processTopPattern(profileName, topMarkerList, profileExecCounter);
}

extern "C" T2ERROR getGrepResults (GrepSeekProfile **GSP, Vector *markerList, bool isClearSeekMap, bool check_rotated, char *customLogPath)
{
    if (!g_profileXConfMock)
    {
        return T2ERROR_FAILURE;
    }
    return g_profileXConfMock->getGrepResults(GSP, markerList, isClearSeekMap, check_rotated, customLogPath);
}

extern "C" void dcaFlagReportCompleation()
{
    if(!g_profileXConfMock)
    {
        return;
    }
    // No mock function needed, just a stub
    return;
}

extern "C" T2ERROR saveSeekConfigtoFile(char* profileName, GrepSeekProfile *ProfileSeekMap)
{
    if(!g_profileXConfMock)
    {
        return T2ERROR_FAILURE;
    }
    return g_profileXConfMock->saveSeekConfigtoFile(profileName, ProfileSeekMap);
}

extern "C" T2ERROR loadSavedSeekConfig(char *profileName, GrepSeekProfile *ProfileSeekMap)
{
    if(!g_profileXConfMock)
    {
        return T2ERROR_FAILURE;
    }
    return g_profileXConfMock->loadSavedSeekConfig(profileName, ProfileSeekMap);
}

extern "C" bool firstBootStatus()
{
    if(!g_profileXConfMock)
    {
        return false;
    }
    return g_profileXConfMock->firstBootStatus();
}



//protocol mock functions
extern "C" T2ERROR __wrap_sendReportOverHTTP(char *httpUrl, char *payload, pid_t* outForkedPid)
{ 
    if(!g_profileXConfMock)
    {
        return T2ERROR_FAILURE;
    }
    // No mock function needed, just a stub
    return g_profileXConfMock->sendReportOverHTTP(httpUrl, payload, outForkedPid);
}

extern "C" T2ERROR __wrap_sendCachedReportsOverHTTP(char *httpUrl, Vector *reportList)
{
    if(!g_profileXConfMock)
    {
        return T2ERROR_FAILURE;
    }
    // No mock function needed, just a stub
    return g_profileXConfMock->sendCachedReportsOverHTTP(httpUrl, reportList);
}
