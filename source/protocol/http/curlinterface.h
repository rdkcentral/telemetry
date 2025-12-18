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

#ifndef _CURLINTERFACE_H_
#define _CURLINTERFACE_H_

#include <curl/curl.h>
#include "telemetry2_0.h"
#include "vector.h"
#define TIMEOUT        30
#define INTERFACE      "erouter0"
#define TLSVERSION     CURL_SSLVERSION_TLSv1_2


#define CURL_OUTPUT_FILE    "/tmp/output.txt"

#define HTTP_METHOD         "POST"

#define HEADER_ACCEPT       "Accept: application/json"
#define HEADER_CONTENTTYPE  "Content-type: application/json"

#if defined(WAN_FAILOVER_SUPPORTED) || defined(FEATURE_RDKB_CONFIGURABLE_WAN_INTERFACE)
#define TR181_DEVICE_CURRENT_WAN_IFNAME             "Device.X_RDK_WanManager.CurrentActiveInterface"
#endif

T2ERROR sendReportOverHTTP(char *httpUrl, char* payload, pid_t* outForkedPid);

T2ERROR sendCachedReportsOverHTTP(char *httpUrl, Vector *reportList);

#if 0//GTEST_ENABLE
typedef size_t (*WriteToFileFunc)(void *, size_t, size_t, void *);
WriteToFileFunc getWriteToFileCallback(void);
#endif

#ifdef LIBRDKCERTSEL_BUILD
void curlCertSelectorFree();
#endif
#endif /* _CURLINTERFACE_H_ */
