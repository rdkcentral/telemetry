/*
* Copyright 2020 Comcast Cable Communications Management, LLC
** Licensed under the Apache License, Version 2.0 (the "License");
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
//#ifndef SOURCE_TEST_MOCKS_SYSTEMMOCK_H_
//#define SOURCE_TEST_MOCKS_SYSTEMMOCK_H_

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "telemetry2_0.h"
#include "profilexconf.h"
typedef void (*T2EventMarkerListCallback)(const char* componentName, void **eventMarkerList);

class profilexconfMock
{
public:
    
    MOCK_METHOD(T2ERROR, processConfigurationXConf, (char* configData, ProfileXConf **localProfile), ());
    MOCK_METHOD(Vector*, getProfileParameterValues, (Vector *paramList, int count), ());
    MOCK_METHOD(T2ERROR, regDEforCompEventList, (const char* componentName, T2EventMarkerListCallback callBackHandler), ());
    //MOCK_METHOD(void, freeGrepSeekProfile, (GrepSeekProfile *gsProfile), ());
    MOCK_METHOD(T2ERROR, getParameterValue, (const char* paramName, char **paramValue), ());
    MOCK_METHOD(void, publishReportUploadStatus, (char* status), ());   
    MOCK_METHOD(void, freeProfileValues, (void *data), ()); 
    MOCK_METHOD(int, processTopPattern, (char* profileName,  Vector* topMarkerList, Vector* out_grepResultList, int profileExecCounter), ());
    MOCK_METHOD(T2ERROR, getGrepResults, (GrepSeekProfile **GSP, Vector *markerList, Vector **grepResultList, bool isClearSeekMap, bool check_rotated, char *customLogPath), ());
    MOCK_METHOD(void, dcaFlagReportCompleation, (), ());
    MOCK_METHOD(GrepSeekProfile *, createGrepSeekProfile, (int execCounter), ());
    MOCK_METHOD(T2ERROR, sendReportOverHTTP, (char *httpUrl, char *payload, pid_t* outForkedPid), ());
    MOCK_METHOD(T2ERROR, sendCachedReportsOverHTTP, (char *httpUrl, Vector *reportList), ());
    MOCK_METHOD(T2ERROR, saveSeekConfigtoFile, (char* profileName, GrepSeekProfile *ProfileSeekMap), ());
    MOCK_METHOD(T2ERROR, loadSavedSeekConfig, (char *profileName, GrepSeekProfile *ProfileSeekMap), ());
    MOCK_METHOD(bool, firstBootStatus, (), ());
};  

extern profilexconfMock *g_profileXConfMock;

