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

extern "C" {

// Linked C functions (must match exactly!)
void __wrap_T2RbusReportEventConsumer(const char* a, const char* b) {
    if (g_profileMock) g_profileMock->T2RbusReportEventConsumer(a, b);
}

int __wrap_rbusCheckMethodExists(const char* a, const char* b, const char* c) {
    if (g_profileMock) return g_profileMock->rbusCheckMethodExists(a, b, c);
    return 0;
}

int __wrap_rbusT2ConsumerUnReg(void* p1, void* p2) {
    if (g_profileMock) return g_profileMock->rbusT2ConsumerUnReg(p1, p2);
    return 0;
}

void __wrap_registerConditionalReportCallBack(void* p) {
    if (g_profileMock) g_profileMock->registerConditionalReportCallBack(p);
}

int __wrap_rbusT2ConsumerReg(const char* a, void* b, void* c) {
    if (g_profileMock) return g_profileMock->rbusT2ConsumerReg(a, b, c);
    return 0;
}

int __wrap_rbusMethodCaller() {
    if (g_profileMock) return g_profileMock->rbusMethodCaller();
    return 0;
}

void __wrap_setT2EventReceiveState(int state) {
    if (g_profileMock) g_profileMock->setT2EventReceiveState(state);
}

void __wrap_regDEforCompEventList(void) {
    if (g_profileMock) g_profileMock->regDEforCompEventList();
}

void __wrap_unregisterDEforCompEventList(void) {
    if (g_profileMock) g_profileMock->unregisterDEforCompEventList();
}

void __wrap_publishEventsProfileUpdates(void) {
    if (g_profileMock) g_profileMock->publishEventsProfileUpdates();
}

void __wrap_regDEforProfileDataModel(void* handlers) {
    if (g_profileMock) g_profileMock->regDEforProfileDataModel(handlers);
}

void __wrap_createComponentDataElements(void) {
    if (g_profileMock) g_profileMock->createComponentDataElements();
}

void __wrap_PushBlobRequest(void* exec) {
    if (g_profileMock) g_profileMock->PushBlobRequest(exec);
}

void __wrap_publishReportUploadStatus(const char* a, int b) {
    if (g_profileMock) g_profileMock->publishReportUploadStatus(a, b);
}

int __wrap_getRbusParameterVal(const char* a, char** b) {
    if (g_profileMock) return g_profileMock->getRbusParameterVal(a, b);
    return 0;
}

void* __wrap_getRbusProfileParamValues(void* paramList, int count) {
    if (g_profileMock) return g_profileMock->getRbusProfileParamValues(paramList, count);
    return nullptr;
}

int __wrap_registerRbusT2EventListener(void* cb) {
    if (g_profileMock) return g_profileMock->registerRbusT2EventListener(cb);
    return 0;
}

void __wrap_unregisterRbusT2EventListener(void) {
    if (g_profileMock) g_profileMock->unregisterRbusT2EventListener();
}

}
