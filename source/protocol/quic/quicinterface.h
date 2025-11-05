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

#ifndef _QUICINTERFACE_H_
#define _QUICINTERFACE_H_

#include "telemetry2_0.h"
#include "vector.h"

#define QUIC_DEFAULT_TIMEOUT        30
#define QUIC_DEFAULT_RETRY_COUNT    3
#define QUIC_MAX_URL_LENGTH         2048
#define QUIC_MAX_RESPONSE_SIZE      4096

/**
 * @brief Send a telemetry report over QUIC protocol
 * 
 * This function sends a JSON report to a QUIC endpoint via HTTP POST to localhost proxy.
 * The proxy service handles the QUIC connection to the remote endpoint.
 * 
 * @param quicUrl The QUIC endpoint URL (http://localhost:port)
 * @param payload The JSON payload to send
 * @param outForkedPid Pointer to store the forked process ID (can be NULL)
 * @return T2ERROR_SUCCESS on success, appropriate error code on failure
 */
T2ERROR sendReportOverQUIC(char *quicUrl, char* payload, pid_t* outForkedPid);

/**
 * @brief Send cached telemetry reports over QUIC protocol
 * 
 * This function sends multiple cached reports to a QUIC endpoint.
 * 
 * @param quicUrl The QUIC endpoint URL (http://localhost:port)
 * @param reportList Vector containing cached reports
 * @return T2ERROR_SUCCESS on success, appropriate error code on failure
 */
T2ERROR sendCachedReportsOverQUIC(char *quicUrl, Vector *reportList);

#endif /* _QUICINTERFACE_H_ */
