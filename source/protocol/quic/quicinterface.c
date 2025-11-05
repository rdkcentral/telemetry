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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <curl/curl.h>

#include "quicinterface.h"
#include "t2log_wrapper.h"

#define CURL_TIMEOUT_DEFAULT    30L
#define CURL_CONNECTTIMEOUT     10L
#define CURL_MAX_RETRIES        3

/**
 * Structure to hold HTTP response data
 */
typedef struct {
    char *data;
    size_t size;
} HTTPResponse;

/**
 * Callback function to write HTTP response data
 */
static size_t writeResponseCallback(void *contents, size_t size, size_t nmemb, HTTPResponse *response) {
    size_t realsize = size * nmemb;
    char *ptr = realloc(response->data, response->size + realsize + 1);
    
    if (ptr == NULL) {
        T2Error("Failed to allocate memory for HTTP response\n");
        return 0;
    }
    
    response->data = ptr;
    memcpy(&(response->data[response->size]), contents, realsize);
    response->size += realsize;
    response->data[response->size] = '\0';
    
    return realsize;
}

/**
 * Initialize HTTP response structure
 */
static void initHTTPResponse(HTTPResponse *response) {
    response->data = malloc(1);
    response->size = 0;
    if (response->data) {
        response->data[0] = '\0';
    }
}

/**
 * Free HTTP response structure
 */
static void freeHTTPResponse(HTTPResponse *response) {
    if (response && response->data) {
        free(response->data);
        response->data = NULL;
        response->size = 0;
    }
}

/**
 * Validate QUIC URL format
 */
static T2ERROR validateQuicUrl(const char *url) {
    if (!url || strlen(url) == 0) {
        T2Error("QUIC URL is NULL or empty\n");
        return T2ERROR_INVALID_ARGS;
    }
    
    if (strlen(url) >= QUIC_MAX_URL_LENGTH) {
        T2Error("QUIC URL too long: %zu bytes\n", strlen(url));
        return T2ERROR_INVALID_ARGS;
    }
    
    // Check if URL starts with http://localhost
    if (strncmp(url, "http://localhost", 16) != 0 && strncmp(url, "http://127.0.0.1", 16) != 0) {
        T2Error("QUIC URL must start with http://localhost or http://127.0.0.1, got: %s\n", url);
        return T2ERROR_INVALID_ARGS;
    }
    
    return T2ERROR_SUCCESS;
}

/**
 * Send HTTP POST request using libcurl
 */
static T2ERROR sendHttpPostRequest(const char *url, const char *payload, HTTPResponse *response) {
    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = NULL;
    T2ERROR ret = T2ERROR_SUCCESS;
    
    curl = curl_easy_init();
    if (!curl) {
        T2Error("Failed to initialize curl\n");
        return T2ERROR_INTERNAL_ERROR;
    }
    
    // Set URL
    curl_easy_setopt(curl, CURLOPT_URL, url);
    
    // Set POST data
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(payload));
    
    // Set headers
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "Accept: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    // Set timeouts
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, CURL_TIMEOUT_DEFAULT);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CURL_CONNECTTIMEOUT);
    
    // Set response callback
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeResponseCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
    
    // Disable SSL verification for localhost
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    
    // Enable verbose output in debug builds
    #ifdef DEBUG
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    #endif
    
    // Perform the request
    res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        T2Error("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        ret = T2ERROR_INTERNAL_ERROR;
    } else {
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        
        if (response_code >= 200 && response_code < 300) {
            T2Info("QUIC proxy request successful, HTTP response code: %ld\n", response_code);
            ret = T2ERROR_SUCCESS;
        } else {
            T2Error("QUIC proxy request failed, HTTP response code: %ld\n", response_code);
            ret = T2ERROR_INVALID_RESPONSE;
        }
    }
    
    // Cleanup
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    return ret;
}

/**
 * Send report with retry logic
 */
static T2ERROR sendReportWithRetry(const char *url, const char *payload, int maxRetries) {
    T2ERROR ret = T2ERROR_FAILURE;
    HTTPResponse response;
    int attempt;
    
    for (attempt = 1; attempt <= maxRetries; attempt++) {
        T2Info("QUIC send attempt %d/%d to %s\n", attempt, maxRetries, url);
        
        initHTTPResponse(&response);
        ret = sendHttpPostRequest(url, payload, &response);
        
        if (ret == T2ERROR_SUCCESS) {
            T2Info("QUIC report sent successfully on attempt %d\n", attempt);
            if (response.data && response.size > 0) {
                T2Debug("QUIC proxy response: %s\n", response.data);
            }
            freeHTTPResponse(&response);
            break;
        } else {
            T2Error("QUIC send attempt %d failed with error: %d\n", attempt, ret);
            freeHTTPResponse(&response);
            
            if (attempt < maxRetries) {
                T2Info("Retrying QUIC send in 1 second...\n");
                sleep(1);
            }
        }
    }
    
    if (ret != T2ERROR_SUCCESS) {
        T2Error("All QUIC send attempts failed after %d retries\n", maxRetries);
    }
    
    return ret;
}

/**
 * Implementation of sendReportOverQUIC
 */
T2ERROR sendReportOverQUIC(char *quicUrl, char* payload, pid_t* outForkedPid) {
    T2ERROR ret = T2ERROR_SUCCESS;
    
    T2Info("Sending report over QUIC to: %s\n", quicUrl ? quicUrl : "NULL");
    
    // Validate input parameters
    if (!quicUrl || !payload) {
        T2Error("Invalid arguments: quicUrl=%p, payload=%p\n", (void*)quicUrl, (void*)payload);
        return T2ERROR_INVALID_ARGS;
    }
    
    // Validate URL format
    ret = validateQuicUrl(quicUrl);
    if (ret != T2ERROR_SUCCESS) {
        return ret;
    }
    
    // Validate payload
    if (strlen(payload) == 0) {
        T2Error("Payload is empty\n");
        return T2ERROR_INVALID_ARGS;
    }
    
    T2Debug("QUIC payload size: %zu bytes\n", strlen(payload));
    
    // Initialize curl globally if not already done
    static int curl_initialized = 0;
    if (!curl_initialized) {
        if (curl_global_init(CURL_GLOBAL_DEFAULT) != 0) {
            T2Error("Failed to initialize curl globally\n");
            return T2ERROR_INTERNAL_ERROR;
        }
        curl_initialized = 1;
    }
    
    // Send report with retry logic
    ret = sendReportWithRetry(quicUrl, payload, CURL_MAX_RETRIES);
    
    // Set output PID if requested (not applicable for synchronous HTTP calls)
    if (outForkedPid) {
        *outForkedPid = 0;
    }
    
    return ret;
}

/**
 * Implementation of sendCachedReportsOverQUIC
 */
T2ERROR sendCachedReportsOverQUIC(char *quicUrl, Vector *reportList) {
    T2ERROR ret = T2ERROR_SUCCESS;
    int i;
    
    T2Info("Sending cached reports over QUIC to: %s\n", quicUrl ? quicUrl : "NULL");
    
    // Validate input parameters
    if (!quicUrl || !reportList) {
        T2Error("Invalid arguments: quicUrl=%p, reportList=%p\n", (void*)quicUrl, (void*)reportList);
        return T2ERROR_INVALID_ARGS;
    }
    
    // Validate URL format
    ret = validateQuicUrl(quicUrl);
    if (ret != T2ERROR_SUCCESS) {
        return ret;
    }
    
    int reportCount = Vector_Size(reportList);
    if (reportCount == 0) {
        T2Info("No cached reports to send\n");
        return T2ERROR_SUCCESS;
    }
    
    T2Info("Sending %d cached reports over QUIC\n", reportCount);
    
    // Initialize curl globally if not already done
    static int curl_initialized = 0;
    if (!curl_initialized) {
        if (curl_global_init(CURL_GLOBAL_DEFAULT) != 0) {
            T2Error("Failed to initialize curl globally\n");
            return T2ERROR_INTERNAL_ERROR;
        }
        curl_initialized = 1;
    }
    
    // Send each cached report
    for (i = 0; i < reportCount; i++) {
        char *cachedReport = (char*)Vector_At(reportList, i);
        if (cachedReport && strlen(cachedReport) > 0) {
            T2Info("Sending cached report %d/%d\n", i + 1, reportCount);
            
            T2ERROR sendResult = sendReportWithRetry(quicUrl, cachedReport, CURL_MAX_RETRIES);
            if (sendResult != T2ERROR_SUCCESS) {
                T2Error("Failed to send cached report %d/%d, error: %d\n", i + 1, reportCount, sendResult);
                ret = sendResult; // Keep track of last error, but continue sending other reports
            } else {
                T2Info("Successfully sent cached report %d/%d\n", i + 1, reportCount);
            }
        } else {
            T2Warning("Skipping empty cached report %d/%d\n", i + 1, reportCount);
        }
    }
    
    if (ret == T2ERROR_SUCCESS) {
        T2Info("All %d cached reports sent successfully over QUIC\n", reportCount);
    } else {
        T2Error("Some cached reports failed to send over QUIC\n");
    }
    
    return ret;
}
