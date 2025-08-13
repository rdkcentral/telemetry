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

typedef void (*rbusMethodCallBackPtr) (rbusHandle_t handle, char const* methodName, rbusError_t retStatus, rbusObject_t params);
class rbusMock
{
public:
    MOCK_METHOD(rbusObject_t, rbusObject_Init, (rbusObject_t* obj, char const * str), ());
    MOCK_METHOD(rbusValue_t, rbusValue_Init, (rbusValue_t* value), ());
    MOCK_METHOD(void, rbusValue_SetString, (rbusValue_t value, char const* str), ());
    MOCK_METHOD(void, rbusObject_SetValue, (rbusObject_t obj, char const* name, rbusValue_t value), ());
    MOCK_METHOD(void, rbusValue_SetInt32, (rbusValue_t value, int32_t val), ());
    MOCK_METHOD(void, rbusValue_Release, (rbusValue_t value), ());
    MOCK_METHOD(void, rbusObject_Release, (rbusObject_t obj), ());
    MOCK_METHOD(rbusError_t, rbusMethod_Invoke, (rbusHandle_t handle, char const* methodName, rbusObject_t input, rbusObject_t* output), ());
    MOCK_METHOD(rbusError_t, rbus_get, (rbusHandle_t handle, char const* name, rbusValue_t* value), ());
    MOCK_METHOD(rbusStatus_t, rbus_checkStatus, (), ());
    MOCK_METHOD(rbusError_t, rbus_open, (rbusHandle_t* handle, char const* componentName), ());
    MOCK_METHOD(rbusValueType_t, rbusValue_GetType, (rbusValue_t v), ());
    MOCK_METHOD(struct _rbusObject*, rbusValue_GetObject, (rbusValue_t v), ());
    MOCK_METHOD(bool, rbusValue_GetBoolean, (rbusValue_t value), ());
    MOCK_METHOD(rbusProperty_t, rbusProperty_Init, (rbusProperty_t* pproperty, char const* name, rbusValue_t value), ());
    MOCK_METHOD(void, rbusValue_SetProperty, (rbusValue_t value, struct _rbusProperty* property), ());
    MOCK_METHOD(rbusError_t, rbus_set, (rbusHandle_t handle, char const* name,rbusValue_t value, rbusSetOptions_t* opts), ());
    MOCK_METHOD(void, rbusProperty_Release, (rbusProperty_t property), ());
    MOCK_METHOD(rbusError_t, rbus_getUint, (rbusHandle_t handle, char const* paramName, unsigned int* paramVal), ());
    MOCK_METHOD(rbusError_t,  rbusEvent_Subscribe,(rbusHandle_t handle, char const* eventName, rbusEventHandler_t handler, void* userData, int timeout), ());
    MOCK_METHOD(rbusError_t, rbus_close, (rbusHandle_t handle), ());
    MOCK_METHOD(rbusProperty_t, rbusObject_GetProperties, (rbusObject_t object), ());
    MOCK_METHOD(rbusProperty_t, rbusProperty_GetNext, (rbusProperty_t property), ());
    MOCK_METHOD(char const*, rbusProperty_GetName, (rbusProperty_t property), ());
    MOCK_METHOD(char*, rbusValue_ToString, (rbusValue_t v, char* buf, size_t buflen), ());
    MOCK_METHOD(const char*, rbusError_ToString, (rbusError_t error), ());
    MOCK_METHOD(bool, rbusCheckMethodExists, (const char* methodName), ());
    MOCK_METHOD(T2ERROR, rbusMethodCaller, (char *methodName, rbusObject_t* input, char* output, rbusMethodCallBackPtr rbusMethodCallBack), ());
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

extern "C" rbusError_t rbus_get(rbusHandle_t handle, char const* name, rbusValue_t* value);
extern "C" rbusStatus_t rbus_checkStatus();
extern "C" rbusError_t rbus_open(rbusHandle_t* handle, char const* componentName);
extern "C" rbusValueType_t rbusValue_GetType(rbusValue_t v);
extern "C" struct _rbusObject* rbusValue_GetObject(rbusValue_t v);
extern "C" bool rbusValue_GetBoolean(rbusValue_t value);
extern "C" rbusProperty_t rbusProperty_Init(rbusProperty_t* pproperty, char const* name, rbusValue_t value);
extern "C" void rbusValue_SetProperty(rbusValue_t value, struct _rbusProperty* property);
extern "C" rbusError_t rbus_set(rbusHandle_t handle, char const* name,rbusValue_t value, rbusSetOptions_t* opts);
extern "C" void rbusProperty_Release(rbusProperty_t property);
extern "C" rbusError_t rbus_getUint (rbusHandle_t handle, char const* paramName, unsigned int* paramVal);
extern "C" rbusError_t  rbusEvent_Subscribe(
    rbusHandle_t        handle,
    char const*         eventName,
    rbusEventHandler_t  handler,
    void*               userData,
    int                 timeout);
extern "C" rbusError_t rbus_close(rbusHandle_t handle);
extern "C" rbusProperty_t rbusObject_GetProperties(rbusObject_t object);
extern "C" rbusProperty_t rbusProperty_GetNext(rbusProperty_t property);
extern "C" char const* rbusProperty_GetName(rbusProperty_t property);
extern "C" char* rbusValue_ToString(rbusValue_t v, char* buf, size_t buflen);

extern "C" const char* rbusError_ToString(rbusError_t error);
extern "C" bool rbusCheckMethodExists(const char* methodName);
extern "C" T2ERROR rbusMethodCaller(char *methodName, rbusObject_t* input, char* output, rbusMethodCallBackPtr rbusMethodCallBack );
