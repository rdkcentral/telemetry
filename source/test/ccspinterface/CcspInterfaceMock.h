/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2019 RDK Management
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

#ifndef CCSP_INTERFACE_MOCK_H
#define CCSP_INTERFACE_MOCK_H

#include <gmock/gmock.h>

extern "C" {
#include "t2common.h"
#include "busInterface.h"
}

class CcspInterfaceMock
{
public:
    virtual ~CcspInterfaceMock() = default;

    // Mock CCSP functions that would be called
    MOCK_METHOD(int, CcspBaseIf_discComponentSupportingNamespace,
                (void* bus_handle, const char* dst_component_id, const char* name_space,
                 const char* subsystem_prefix, void*** ppComponents, int* psize), ());

    MOCK_METHOD(int, CcspBaseIf_getParameterValues,
                (void* bus_handle, const char* dst_component_id, char** parameter_names,
                 int param_size, int* val_size, void*** parametervalStruct), ());

    MOCK_METHOD(int, CcspBaseIf_getParameterNames,
                (void* bus_handle, const char* dst_component_id, const char* parameter_name,
                 int next_level, int* psize, void*** ppParameterInfos), ());

    MOCK_METHOD(void, free_componentStruct, (void* arg), ());
    MOCK_METHOD(void, free_parameterValStruct, (int size, void** val), ());
    MOCK_METHOD(void, free_parameterInfoStruct, (int size, void** val), ());

    // Mock for getCCSPParamVal function
    MOCK_METHOD(T2ERROR, getCCSPParamVal, (const char* paramName, char **paramValue), ());

    // Mock for getCCSPProfileParamValues function
    MOCK_METHOD(Vector*, getCCSPProfileParamValues, (Vector* paramList, int count), ());

    // Mock for registerCcspT2EventListener function
    MOCK_METHOD(T2ERROR, registerCcspT2EventListener, (TelemetryEventCallback eventCB), ());
};

extern CcspInterfaceMock *g_ccspInterfaceMock;

#endif // CCSP_INTERFACE_MOCK_H
