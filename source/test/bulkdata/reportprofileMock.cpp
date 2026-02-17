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

void __wrap_unregisterDEforCompEventList(void) {
    if (g_reportprofileMock) g_reportprofileMock->unregisterDEforCompEventList();
}
void __wrap_publishEventsProfileUpdates(void) {
    if (g_reportprofileMock) g_reportprofileMock->publishEventsProfileUpdates();
}
void __wrap_setT2EventReceiveState(int state) {
    if (g_reportprofileMock) g_reportprofileMock->setT2EventReceiveState(state);
}
void __wrap_regDEforProfileDataModel(void* handlers) {
    if (g_reportprofileMock) g_reportprofileMock->regDEforProfileDataModel(handlers);
}
void __wrap_regDEforCompEventList(void) {
    if (g_reportprofileMock) g_reportprofileMock->regDEforCompEventList();
}
void __wrap_createComponentDataElements(void) {
    if (g_reportprofileMock) g_reportprofileMock->createComponentDataElements();
}
void __wrap_PushBlobRequest(void* exec) {
    if (g_reportprofileMock) g_reportprofileMock->PushBlobRequest(exec);
}
int __wrap_rbusMethodCaller(...) { // Use specific argument list as needed
    if (g_reportprofileMock) return g_reportprofileMock->rbusMethodCaller();
    return 0;
}
int __wrap_rbusCheckMethodExists(const char* a, const char* b, const char* c) {
    if (g_reportprofileMock) return g_reportprofileMock->rbusCheckMethodExists(a, b, c);
    return 0;
}
void __wrap_T2RbusReportEventConsumer(const char* a, const char* b) {
    if (g_reportprofileMock) g_reportprofileMock->T2RbusReportEventConsumer(a, b);
}
int __wrap_rbusT2ConsumerUnReg(void* p1, void* p2) {
    if (g_reportprofileMock) return g_reportprofileMock->rbusT2ConsumerUnReg(p1, p2);
    return 0;
}
int __wrap_rbusT2ConsumerReg(const char* a, void* b, void* c) {
    if (g_reportprofileMock) return g_reportprofileMock->rbusT2ConsumerReg(a, b, c);
    return 0;
}
void __wrap_registerConditionalReportCallBack(void* p) {
    if (g_reportprofileMock) g_reportprofileMock->registerConditionalReportCallBack(p);
}
void __wrap_publishReportUploadStatus(const char* a, int b) {
    if (g_reportprofileMock) g_reportprofileMock->publishReportUploadStatus(a, b);
}
int __wrap_getRbusParameterVal(const char* a, char** b) {
    if (g_reportprofileMock) return g_reportprofileMock->getRbusParameterVal(a, b);
    return 0;
}
void* __wrap_getRbusProfileParamValues(void* paramList, int count) {
    if (g_reportprofileMock) return g_reportprofileMock->getRbusProfileParamValues(paramList, count);
    return nullptr;
}
int __wrap_registerRbusT2EventListener(void* cb) {
    if (g_reportprofileMock) return g_reportprofileMock->registerRbusT2EventListener(cb);
    return 0;
}
void __wrap_unregisterRbusT2EventListener(void) {
    if (g_reportprofileMock) g_reportprofileMock->unregisterRbusT2EventListener();
}
T2ERROR __wrap_sendReportOverHTTP(char *httpUrl, char *payload, pid_t* outForkedPid)
{
    if (g_profileMock) {
        return g_profileMock->sendReportOverHTTP(httpUrl, payload, outForkedPid);
    }
    return T2ERROR_FAILURE;
}

T2ERROR __wrap_sendCachedReportsOverHTTP(char *httpUrl, Vector *reportList)
{
    if (g_profileMock) {
        return g_profileMock->sendCachedReportsOverHTTP(httpUrl, reportList);
    }
    return T2ERROR_FAILURE;
}
} // extern "C"
