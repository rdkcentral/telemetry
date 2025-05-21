/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2024 RDK Management
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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "telemetry2_0.h"
#include "test/rbus/include/rbus.h"
#include "test/rbus/include/rbus_value.h"
/*
class rbusInterface
{
public:
    virtual ~rbusInterface() {}
    //virtual rbusError_t rbus_registerLogHandler(rbusLogHandler) = 0;
    //virtual rbusError_t rbus_open(rbusHandle_t*, char const*) = 0;
    //virtual rbusError_t rbus_close(rbusHandle_t) = 0;
    virtual rbusObject_t rbusObject_Init(rbusObject_t*, char const*) = 0;
    virtual rbusValue_t rbusValue_Init(rbusValue_t*) = 0;
    virtual void rbusValue_SetString(rbusValue_t, char const*) = 0;
    virtual void rbusObject_SetValue(rbusObject_t, char const*, rbusValue_t) = 0;
    virtual void rbusValue_SetInt32(rbusValue_t, int32_t) = 0;
    virtual void rbusValue_Release(rbusValue_t) = 0;
    virtual void rbusObject_Release(rbusObject_t) = 0;
    virtual rbusError_t rbusMethod_Invoke(rbusHandle_t, char const*, rbusObject_t, rbusObject_t*) = 0;
    virtual const char* rbusError_ToString(rbusError_t) = 0;
    virtual bool rbusCheckMethodExists(const char*) = 0;
    virtual T2ERROR rbusMethodCaller(char *, rbusObject_t*, char*) = 0;
     virtual bool isRbusEnabled() = 0;
     virtual void publishReportUploadStatus(char*) = 0;
     virtual Vector* getProfileParameterValues(Vector *) = 0;
     virtual int regDEforCompEventList(const char*, T2EventMarkerListCallback) = 0;
    virtual int publishEventsProfileUpdates() = 0;
           virtual void unregisterDEforCompEventList() = 0;
           virtual int regDEforProfileDataModel(callBackHandlers*) = 0;
    virtual int registerForTelemetryEvents(TelemetryEventCallback) = 0;
           virtual int T2RbusReportEventConsumer(char*, bool) = 0;
    virtual int rbusT2ConsumerUnReg(Vector *) = 0;
           virtual int rbusT2ConsumerReg(Vector *) = 0;
           virtual void registerConditionalReportCallBack(triggerReportOnCondtionCallBack) = 0;
           virtual int sendReportsOverRBUSMethod(char *, Vector*, char*) = 0;
           virtual int sendCachedReportsOverRBUSMethod(char *, Vector*, Vector*) = 0;
};


class rbusMock: public rbusInterface
{
public:
    virtual ~rbusMock() {}
    MOCK_METHOD1(rbusError_ToString, const char*(rbusError_t));
    //MOCK_METHOD1(rbus_registerLogHandler, rbusError_t(rbusLogHandler));
    //MOCK_METHOD2(rbus_open, rbusError_t(rbusHandle_t*, char const*));
    //MOCK_METHOD1(rbus_close, rbusError_t(rbusHandle_t));
    MOCK_METHOD2(rbusObject_Init, rbusObject_t(rbusObject_t*, char const*));
    MOCK_METHOD1(rbusValue_Init, rbusValue_t(rbusValue_t*));
    MOCK_METHOD2(rbusValue_SetString, void(rbusValue_t, char const*));
    MOCK_METHOD3(rbusObject_SetValue, void(rbusObject_t, char const*, rbusValue_t));
    MOCK_METHOD2(rbusValue_SetInt32, void(rbusValue_t, int32_t));
    MOCK_METHOD1(rbusValue_Release, void(rbusValue_t));
    MOCK_METHOD1(rbusObject_Release, void(rbusObject_t));
    MOCK_METHOD4(rbusMethod_Invoke, rbusError_t(rbusHandle_t, char const*, rbusObject_t, rbusObject_t*));
    MOCK_METHOD1(rbusCheckMethodExists, bool(const char*));
    MOCK_METHOD3(rbusMethodCaller, T2ERROR(char *, rbusObject_t*, char*));
     MOCK_METHOD0(isRbusEnabled, bool());
     MOCK_METHOD1(publishReportUploadStatus, void(char* ));
     MOCK_METHOD1(getProfileParameterValues, Vector*(Vector *));
    MOCK_METHOD2(regDEforCompEventList, int(const char*, T2EventMarkerListCallback));
           MOCK_METHOD0(publishEventsProfileUpdates, int());
    MOCK_METHOD0(unregisterDEforCompEventList, void());
    MOCK_METHOD1(regDEforProfileDataModel, int(callBackHandlers*));
           MOCK_METHOD2(T2RbusReportEventConsumer, int(char*, bool));
    MOCK_METHOD1(rbusT2ConsumerUnReg, int(Vector*));
    MOCK_METHOD1(rbusT2ConsumerReg, int(Vector *));
    MOCK_METHOD1(registerConditionalReportCallBack, void(triggerReportOnCondtionCallBack));
           MOCK_METHOD3(sendReportsOverRBUSMethod, int(char *, Vector*, char*));
           MOCK_METHOD3(sendCachedReportsOverRBUSMethod, int(char *, Vector*, Vector*));
};

*/

class rbusMock {
public:
    MOCK_METHOD(rbusObject_t, rbusObject_Init, (rbusObject_t* obj, char const * str), ());
    MOCK_METHOD(rbusValue_t, rbusValue_Init, (rbusValue_t* value),());
    MOCK_METHOD(void, rbusValue_SetString, (rbusValue_t value, char const* str), ());
    MOCK_METHOD(void, rbusObject_SetValue, (rbusObject_t obj, char const* name, rbusValue_t value), ());
    MOCK_METHOD(void, rbusValue_SetInt32, (rbusValue_t value, int32_t val), ());
    MOCK_METHOD(void, rbusValue_Release, (rbusValue_t value), ());
    MOCK_METHOD(void, rbusObject_Release, (rbusObject_t obj), ());
    MOCK_METHOD(rbusError_t, rbusMethod_Invoke, (rbusHandle_t handle, char const* methodName, rbusObject_t input, rbusObject_t* output), ());
    MOCK_METHOD(const char*, rbusError_ToString, (rbusError_t error), ());
    MOCK_METHOD(bool, rbusCheckMethodExists, (const char* methodName), ());
    MOCK_METHOD(T2ERROR, rbusMethodCaller, (char *methodName, rbusObject_t* input, char* output), ());
};

extern rbusMock* g_rbusMock;

extern "C" rbusObject_t rbusObject_Init(rbusObject_t* obj, char const * str);
extern "C" rbusValue_t rbusValue_Init(rbusValue_t* value);
extern "C" void rbusValue_SetString(rbusValue_t value, char const* str);
extern "C" void rbusObject_SetValue(rbusObject_t obj, char const* name, rbusValue_t value);
extern "C" void rbusValue_SetInt32(rbusValue_t value, int32_t val);
extern "C" void rbusValue_Release(rbusValue_t value);
extern "C" void rbusObject_Release(rbusObject_t obj);
extern "C" rbusError_t rbusMethod_Invoke(rbusHandle_t handle, char const* methodName, rbusObject_t input, rbusObject_t* output);
extern "C" const char* rbusError_ToString(rbusError_t error);
extern "C" bool rbusCheckMethodExists(const char* methodName);
extern "C" T2ERROR rbusMethodCaller(char *methodName, rbusObject_t* input, char* output);
