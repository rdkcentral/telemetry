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

T2ERROR init_connection_pool();
T2ERROR http_pool_request(const char *url, const char *payload, curlResponseData* response);
T2ERROR http_pool_cleanup(void); 

#endif /* _MULTI_CURL_INTERFACE_H_ */
