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

class profileMock
{
public:

    MOCK_METHOD(T2ERROR, sendReportOverHTTP, (char *httpUrl, char *payload), ());
    MOCK_METHOD(T2ERROR, sendCachedReportsOverHTTP, (char *httpUrl), ());
    MOCK_METHOD(void, T2RbusReportEventConsumer, (const char*, const char*));
    MOCK_METHOD(int, rbusCheckMethodExists, (const char*, const char*, const char*));
    MOCK_METHOD(int, rbusT2ConsumerUnReg, (void*, void*));
    MOCK_METHOD(void, registerConditionalReportCallBack, (void*));
    MOCK_METHOD(int, rbusT2ConsumerReg, (const char*, void*, void*));
    MOCK_METHOD(int, rbusMethodCaller, ());
    MOCK_METHOD(void, setT2EventReceiveState, (int));
    MOCK_METHOD(void, regDEforCompEventList, ());
    MOCK_METHOD(void, unregisterDEforCompEventList, ());
    MOCK_METHOD(void, publishEventsProfileUpdates, ());
    MOCK_METHOD(void, regDEforProfileDataModel, (void*));
    MOCK_METHOD(void, createComponentDataElements, ());
    MOCK_METHOD(void, PushBlobRequest, (void*));
    MOCK_METHOD(void, publishReportUploadStatus, (const char*, int));
    MOCK_METHOD(int, getRbusParameterVal, (const char*, char**));
    MOCK_METHOD(void*, getRbusProfileParamValues, (void*, int));
    MOCK_METHOD(int, registerRbusT2EventListener, (void*));
    MOCK_METHOD(void, unregisterRbusT2EventListener, ());
};

extern profileMock *g_profileMock;

