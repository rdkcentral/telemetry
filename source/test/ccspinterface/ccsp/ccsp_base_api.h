/*
 * Mock CCSP headers for test environment
 * This file provides minimal CCSP definitions to allow compilation
 * without full CCSP dependencies
 */

#ifndef _CCSP_BASE_API_H_
#define _CCSP_BASE_API_H_

#include <stdbool.h>

// Mock CCSP types and definitions
typedef void* CCSP_MESSAGE_BUS_INFO;
typedef void* CCSP_COMPONENT_INFO;
typedef int CCSP_RESULT;

#define CCSP_SUCCESS 0
#define CCSP_FAILURE -1

// Mock CCSP function declarations
#ifdef __cplusplus
extern "C" {
#endif

// Basic CCSP function stubs - these will be provided by our mock
int CcspBaseIf_getParameterValues(
    void* bus_handle,
    const char* dst_component_id,
    char* dbus_path,
    char** parameterNames,
    int parameterNum,
    int* size,
    void*** parameterValue
);

int CcspBaseIf_freeResources(
    void* bus_handle,
    const char* dst_component_id,
    char* dbus_path,
    void** parameterValue,
    int size
);

#ifdef __cplusplus
}
#endif

#endif /* _CCSP_BASE_API_H_ */
