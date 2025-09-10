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

#include "CcspInterfaceMock.h"
#include <cstring>
#include <cstdlib>

CcspInterfaceMock *g_ccspInterfaceMock = nullptr;

// Mock CCSP structures and types since they won't be available
typedef struct _componentStruct {
    char* componentName;
    char* dbusPath;
    char* subsystem_prefix;
} componentStruct_t;

typedef struct _parameterValStruct {
    char* parameterName;
    char* parameterValue;
    int type;
} parameterValStruct_t;

typedef struct _parameterInfoStruct {
    char* parameterName;
    int writable;
} parameterInfoStruct_t;

extern "C" {

// Forward declaration
T2ERROR getCCSPParamVal(const char* paramName, char **paramValue);

// Mock CCSP function implementations
int CcspBaseIf_discComponentSupportingNamespace(void* bus_handle, const char* dst_component_id, 
                                                const char* name_space, const char* subsystem_prefix,
                                                void*** ppComponents, int* psize) {
    if (g_ccspInterfaceMock) {
        return g_ccspInterfaceMock->CcspBaseIf_discComponentSupportingNamespace(
            bus_handle, dst_component_id, name_space, subsystem_prefix, ppComponents, psize);
    }
    
    // Default fallback implementation
    if (ppComponents && psize) {
        *ppComponents = nullptr;
        *psize = 0;
    }
    return 100; // CCSP_FAILURE
}

int CcspBaseIf_getParameterValues(void* bus_handle, const char* dst_component_id, 
                                 char** parameter_names, int param_size, int* val_size,
                                 void*** parametervalStruct) {
    if (g_ccspInterfaceMock) {
        return g_ccspInterfaceMock->CcspBaseIf_getParameterValues(
            bus_handle, dst_component_id, parameter_names, param_size, val_size, parametervalStruct);
    }
    
    // Default fallback implementation
    if (parametervalStruct && val_size) {
        *parametervalStruct = nullptr;
        *val_size = 0;
    }
    return 100; // CCSP_FAILURE
}

int CcspBaseIf_getParameterNames(void* bus_handle, const char* dst_component_id,
                                const char* parameter_name, int next_level, int* psize,
                                void*** ppParameterInfos) {
    if (g_ccspInterfaceMock) {
        return g_ccspInterfaceMock->CcspBaseIf_getParameterNames(
            bus_handle, dst_component_id, parameter_name, next_level, psize, ppParameterInfos);
    }
    
    // Default fallback implementation
    if (ppParameterInfos && psize) {
        *ppParameterInfos = nullptr;
        *psize = 0;
    }
    return 100; // CCSP_FAILURE
}

void free_componentStruct(void* arg) {
    if (g_ccspInterfaceMock) {
        g_ccspInterfaceMock->free_componentStruct(arg);
        return;
    }
    
    // Default fallback implementation
    if (arg) {
        free(arg);
    }
}

void free_parameterValStruct(int size, void** val) {
    if (g_ccspInterfaceMock) {
        g_ccspInterfaceMock->free_parameterValStruct(size, val);
        return;
    }
    
    // Default fallback implementation
    if (val) {
        for (int i = 0; i < size; i++) {
            if (val[i]) {
                free(val[i]);
            }
        }
        free(val);
    }
}

void free_parameterInfoStruct(int size, void** val) {
    if (g_ccspInterfaceMock) {
        g_ccspInterfaceMock->free_parameterInfoStruct(size, val);
        return;
    }
    
    // Default fallback implementation
    if (val) {
        for (int i = 0; i < size; i++) {
            if (val[i]) {
                free(val[i]);
            }
        }
        free(val);
    }
}

// Mock getCCSPParamVal function since it's in ccspinterface.c which we're not compiling
T2ERROR getCCSPParamVal(const char* paramName, char **paramValue) {
    if (g_ccspInterfaceMock) {
        return g_ccspInterfaceMock->getCCSPParamVal(paramName, paramValue);
    }
    
    // Default fallback implementation
    if (paramName && paramValue) {
        *paramValue = strdup("mock_ccsp_value");
        return T2ERROR_SUCCESS;
    }
    return T2ERROR_FAILURE;
}

// Mock getCCSPProfileParamValues function
Vector* getCCSPProfileParamValues(Vector* paramList, int count) {
    if (g_ccspInterfaceMock) {
        return g_ccspInterfaceMock->getCCSPProfileParamValues(paramList, count);
    }
    
    // Default fallback implementation
    return nullptr;
}

// Mock registerCcspT2EventListener function
T2ERROR registerCcspT2EventListener(TelemetryEventCallback eventCB) {
    if (g_ccspInterfaceMock) {
        return g_ccspInterfaceMock->registerCcspT2EventListener(eventCB);
    }
    
    // Default fallback implementation
    return T2ERROR_SUCCESS;
}

} // extern "C"
