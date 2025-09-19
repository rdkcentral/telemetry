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

#ifndef _MULTI_CURL_INTERFACE_H_
#define _MULTI_CURL_INTERFACE_H_

#include <curl/curl.h>
#include "xconfclient.h"
#include "telemetry2_0.h"

// Request types for different HTTP operations
typedef enum
{
    HTTP_REQUEST_GET,    // For doHttpGet (XCONF client)
    HTTP_REQUEST_POST    // For sendReportOverHTTP
} http_request_type_t;

// Request configuration structure
typedef struct
{
    http_request_type_t type;
    const char *url;
    const char *payload;           // NULL for GET requests
    char **response_data;          // For storing response (GET requests)
    bool enable_mtls;              // Enable mTLS authentication
    bool enable_file_output;       // Write to file (for GET requests)
} http_pool_request_config_t;

T2ERROR init_connection_pool();

// New dedicated APIs for better separation of concerns
T2ERROR http_pool_get(const char *url, char **response_data, bool enable_file_output);
T2ERROR http_pool_post(const char *url, const char *payload);

// Legacy APIs (maintained for backward compatibility)
T2ERROR http_pool_request_ex(const http_pool_request_config_t *config);
T2ERROR http_pool_request(const char *url, const char *payload, char** data); // Backward compatibility
T2ERROR http_pool_cleanup(void);

#endif /* _MULTI_CURL_INTERFACE_H_ */
