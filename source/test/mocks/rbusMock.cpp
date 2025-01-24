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

#include "test/mocks/rbusMock.h"

extern rbusMock *g_rbusMock;

extern "C" rbusObject_t rbusObject_Init(rbusObject_t* object, char const* value)
{
    if (!g_rbusMock)
    {
        return NULL;
    }
    return g_rbusMock->rbusObject_Init(object, value);
}

extern "C" rbusValue_t rbusValue_Init(rbusValue_t* value)
{
    if (!g_rbusMock)
    {
        return NULL;
    }
    return g_rbusMock->rbusValue_Init(value);
}

extern "C" void rbusValue_SetString(rbusValue_t value, char const* var)
{
    if (!g_rbusMock)
    {
        return;
    }
    return g_rbusMock->rbusValue_SetString(value, var);
}

extern "C" void rbusObject_SetValue(rbusObject_t object, char const* var, rbusValue_t value)
{
    if (!g_rbusMock)
    {
        return;
    }
    return g_rbusMock->rbusObject_SetValue(object, var, value);
}


extern "C" void rbusValue_Release(rbusValue_t value)
{
    if (!g_rbusMock)
    {
        return;
    }
    return g_rbusMock->rbusValue_Release(value);
}

extern "C" void rbusObject_Release(rbusObject_t object)
{
    if (!g_rbusMock)
    {
        return;
    }
    return g_rbusMock->rbusObject_Release(object);
}

extern "C" void rbusValue_SetInt32(rbusValue_t value, int32_t i32)
{
    if(!g_rbusMock)
    {
       return;
    }
    return g_rbusMock->rbusValue_SetInt32(value, i32);
}

extern "C" rbusError_t rbusMethod_Invoke(rbusHandle_t handle, char const* value, rbusObject_t object, rbusObject_t* objectpt)
{
    if (!g_rbusMock)
    {
        return RBUS_ERROR_SUCCESS;
    }
    return g_rbusMock->rbusMethod_Invoke(handle, value, object, objectpt);
}

extern "C" const char* rbusError_ToString(rbusError_t e)
{
    if (!g_rbusMock)
    {
        return "RBUS_ERROR_INVALID_OPERATION";
    }
    else{
	    #define rbusError_String(E, S) case E: s = S; break;

  char const * s = NULL;
  switch (e)
  {
    rbusError_String(RBUS_ERROR_SUCCESS, "ok");
    rbusError_String(RBUS_ERROR_BUS_ERROR, "generic error");
    rbusError_String(RBUS_ERROR_INVALID_INPUT, "invalid input");
    rbusError_String(RBUS_ERROR_NOT_INITIALIZED, "not initialized");
    rbusError_String(RBUS_ERROR_OUT_OF_RESOURCES, "out of resources");
    rbusError_String(RBUS_ERROR_DESTINATION_NOT_FOUND, "destination not found");
    rbusError_String(RBUS_ERROR_DESTINATION_NOT_REACHABLE, "destination not reachable");
    rbusError_String(RBUS_ERROR_DESTINATION_RESPONSE_FAILURE, "destination response failure");
    rbusError_String(RBUS_ERROR_INVALID_RESPONSE_FROM_DESTINATION, "invalid response from destination");
    rbusError_String(RBUS_ERROR_INVALID_OPERATION, "invalid operation");
    rbusError_String(RBUS_ERROR_INVALID_EVENT, "invalid event");
    rbusError_String(RBUS_ERROR_INVALID_HANDLE, "invalid handle");
    rbusError_String(RBUS_ERROR_SESSION_ALREADY_EXIST, "session already exists");
    rbusError_String(RBUS_ERROR_COMPONENT_NAME_DUPLICATE, "duplicate component name");
    rbusError_String(RBUS_ERROR_ELEMENT_NAME_DUPLICATE, "duplicate element name");
    rbusError_String(RBUS_ERROR_ELEMENT_NAME_MISSING, "name missing");
    rbusError_String(RBUS_ERROR_COMPONENT_DOES_NOT_EXIST, "component does not exist");
    rbusError_String(RBUS_ERROR_ELEMENT_DOES_NOT_EXIST, "element name does not exist");
    rbusError_String(RBUS_ERROR_ACCESS_NOT_ALLOWED, "access denied");
    rbusError_String(RBUS_ERROR_INVALID_CONTEXT, "invalid context");
    rbusError_String(RBUS_ERROR_TIMEOUT, "timeout");
    rbusError_String(RBUS_ERROR_ASYNC_RESPONSE, "async operation in progress");
    default:
      s = "unknown error";
  }
  return s;
    }
}

extern "C" bool rbusCheckMethodExists(const char* rbusMethodName)
{
    if (!g_rbusMock)
    {
        return false;
    }
    return g_rbusMock->rbusCheckMethodExists(rbusMethodName);
}


extern "C" T2ERROR rbusMethodCaller(char *methodName, rbusObject_t* inputParams, char* payload){
    if (!g_rbusMock)
    {
        return T2ERROR_FAILURE;
    }
    return g_rbusMock->rbusMethodCaller(methodName, inputParams, payload);
}
