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

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <net/if.h>
#include <netdb.h>
#include <string.h>
#include <ifaddrs.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <curl/curl.h>
#include <signal.h>
#include "multicurlinterface.h"
#include "t2log_wrapper.h"
#include "xconfclient.h"
#include "t2MtlsUtils.h"

//Global variables
#define MAX_POOL_SIZE 2
#define HTTP_RESPONSE_FILE "/tmp/httpOutput.txt"
#define RESPONSE_BUFFER_SIZE 8192
static bool pool_initialized = false;

// High-level design for connection pooling - Memory optimized
typedef struct
{
    CURLM *multi_handle;
    CURL *easy_handles[MAX_POOL_SIZE];
    bool handle_available[MAX_POOL_SIZE];
    pthread_mutex_t pool_mutex;
    struct curl_slist *post_headers;
    // Response buffers removed - only used by GET requests locally
} http_connection_pool_t;

typedef struct http_pool_config
{
    int max_connections;
    int connection_timeout;
    int keep_alive_timeout;
    bool enable_mtls;
} http_pool_config_t;

http_connection_pool_t pool;

static size_t httpGetCallBack(void *response, size_t len, size_t nmemb,
                              void *stream)
{
    T2Info("%s ++in\n", __FUNCTION__);
    size_t realsize = len * nmemb;
    curlResponseData* httpResponse = (curlResponseData*) stream;

    // FIX: Check for NULL stream to prevent crashes
    if (!httpResponse) {
        T2Error("httpGetCallBack: NULL stream parameter\n");
        return 0;
    }

    char *ptr = (char*) realloc(httpResponse->data,
                                httpResponse->size + realsize + 1);
    if (!ptr)
    {
        T2Error("%s:%u , T2:memory realloc failed\n", __func__, __LINE__);
        // FIX: Don't return 0 on realloc failure - this can cause curl to retry indefinitely
        // Keep the original data intact
        return realsize; // Tell curl we processed the data even though we couldn't store it
    }
    httpResponse->data = ptr;
    memcpy(&(httpResponse->data[httpResponse->size]), response, realsize);
    httpResponse->size += realsize;
    httpResponse->data[httpResponse->size] = 0;

    T2Info("%s ++out\n", __FUNCTION__);
    return realsize;
}

static size_t writeToFile(void *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t written = fwrite(ptr, size, nmemb, (FILE *) stream);
    return written;
}

// Add this debug callback function
static int curl_debug_callback_func(CURL *handle, curl_infotype type, char *data, size_t size, void *userptr)
{
    (void)handle;
    (void)userptr;
    
    switch (type) {
    case CURLINFO_TEXT:
        T2Info("curl: %.*s", (int)size, data);
        break;
    case CURLINFO_HEADER_OUT:
        T2Info("curl: => Send header: %.*s", (int)size, data);
        break;
    case CURLINFO_DATA_OUT:
        T2Info("curl: => Send data: %zu bytes", size);
        break;
    case CURLINFO_SSL_DATA_OUT:
        T2Info("curl: => Send SSL data: %zu bytes", size);
        break;
    case CURLINFO_HEADER_IN:
        T2Info("curl: <= Recv header: %.*s", (int)size, data);
        break;
    case CURLINFO_DATA_IN:
        T2Info("curl: <= Recv data: %zu bytes", size);
        break;
    case CURLINFO_SSL_DATA_IN:
        T2Info("curl: <= Recv SSL data: %zu bytes", size);
        break;
    default:
        break;
    }
    return 0;
}

T2ERROR init_connection_pool()
{
    if(pool_initialized)
    {
        T2Info("Connection pool already initialized\n");
        return T2ERROR_SUCCESS;
    }

    CURLcode code = CURLE_OK;
    T2Info("%s ++in\n", __FUNCTION__);
    char *pCertFile = NULL;
    char *pPasswd = NULL;
    pool.multi_handle = curl_multi_init();

    // Pre-allocate easy handles
    for(int i = 0; i < MAX_POOL_SIZE; i++)
    {
        pool.easy_handles[i] = curl_easy_init();
        pool.handle_available[i] = true;

        // Set common options once
        curl_easy_setopt(pool.easy_handles[i], CURLOPT_TIMEOUT, 30L);
        curl_easy_setopt(pool.easy_handles[i], CURLOPT_CONNECTTIMEOUT, 30L);
        

#if 0
        curl_easy_setopt(pool.easy_handles[i], CURLOPT_TCP_KEEPALIVE, 1L);
        curl_easy_setopt(pool.easy_handles[i], CURLOPT_TCP_KEEPIDLE, 120L); //TODO: get the internal call details

        curl_easy_setopt(pool.easy_handles[i], CURLOPT_TCP_KEEPINTVL, 60L); //TODO: libcurl dev
#endif

#ifdef CURLOPT_TCP_KEEPCNT
        curl_easy_setopt(pool.easy_handles[i], CURLOPT_TCP_KEEPCNT, 15L);  // Allow up to 15 probes
#endif

        curl_easy_setopt(pool.easy_handles[i], CURLOPT_MAXCONNECTS, 5L);  
#if 0
        curl_easy_setopt(pool.easy_handles[i], CURLOPT_FORBID_REUSE, 0L);
        curl_easy_setopt(pool.easy_handles[i], CURLOPT_FRESH_CONNECT, 0L);
#endif
        code = curl_easy_setopt(pool.easy_handles[i], CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
        if(code != CURLE_OK)
        {
            T2Error("%s : Curl set opts failed with error %s \n", __FUNCTION__, curl_easy_strerror(code));
        }

        // Certificate selector and SSL/TLS specific options from original code
        curl_easy_setopt(pool.easy_handles[i], CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(pool.easy_handles[i], CURLOPT_SSLENGINE_DEFAULT, 1L);
        
        curl_easy_setopt(pool.easy_handles[i], CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(pool.easy_handles[i], CURLOPT_DEBUGFUNCTION, curl_debug_callback_func);
        curl_easy_setopt(pool.easy_handles[i], CURLOPT_DEBUGDATA, NULL);

    }

    //mtls
    if(T2ERROR_SUCCESS == getMtlsCerts(&pCertFile, &pPasswd))
    {
        for(int i = 0; i < MAX_POOL_SIZE; i++)
        {
            code = curl_easy_setopt(pool.easy_handles[i], CURLOPT_SSLCERTTYPE, "P12");
            if(code != CURLE_OK)
            {
                T2Error("%s : Curl set opts failed with error %s \n", __FUNCTION__, curl_easy_strerror(code));
            }
            code = curl_easy_setopt(pool.easy_handles[i], CURLOPT_SSLCERT, pCertFile);
            if(code != CURLE_OK)
            {
                T2Error("%s : Curl set opts failed with error %s \n", __FUNCTION__, curl_easy_strerror(code));
            }
            code = curl_easy_setopt(pool.easy_handles[i], CURLOPT_KEYPASSWD, pPasswd);
            if(code != CURLE_OK)
            {
                T2Error("%s : Curl set opts failed with error %s \n", __FUNCTION__, curl_easy_strerror(code));
            }
            /* disconnect if authentication fails */
            code = curl_easy_setopt(pool.easy_handles[i], CURLOPT_SSL_VERIFYPEER, 1L);
            if(code != CURLE_OK)
            {
                T2Error("%s : Curl set opts failed with error %s \n", __FUNCTION__, curl_easy_strerror(code));
            }
        }
        
        if(pCertFile) {
            free(pCertFile);
            pCertFile = NULL;
        }
        if(pPasswd) {
            free(pPasswd);
            pPasswd = NULL;
        }
    }
    
    pool.post_headers = curl_slist_append(NULL, "Accept: application/json");
    pool.post_headers = curl_slist_append(pool.post_headers, "Content-type: application/json");

    pthread_mutex_init(&pool.pool_mutex, NULL);
    pool_initialized = true;
    T2Info("%s ++out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

// Shared multi_perform execution function - simplified
static T2ERROR http_pool_execute_request(CURL *easy, int idx)
{
    T2Info("%s ++in\n", __FUNCTION__);
    
    // Add to multi handle
    curl_multi_add_handle(pool.multi_handle, easy);

    int still_running;
    do
    {
        T2Info("%s ; Performing curl request\n", __FUNCTION__);
        curl_multi_perform(pool.multi_handle, &still_running);
        curl_multi_wait(pool.multi_handle, NULL, 0, 100, NULL); // Reduced wait time for better performance
    }
    while(still_running);

    T2Info("%s ; Curl request completed\n", __FUNCTION__);

    long http_code;
    curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &http_code);
    T2Info("%s ; HTTP response code: %ld\n", __FUNCTION__, http_code);

    T2ERROR ret = T2ERROR_FAILURE;
    if (http_code == 200)
    {
        ret = T2ERROR_SUCCESS;
        T2Info("%s ; Request successful\n", __FUNCTION__);
    }
    else
    {
        T2Error("%s ; HTTP request failed with code: %ld\n", __FUNCTION__, http_code);
    }

    // Cleanup
    curl_multi_remove_handle(pool.multi_handle, easy);

    pthread_mutex_lock(&pool.pool_mutex);
    pool.handle_available[idx] = true;
    pthread_mutex_unlock(&pool.pool_mutex);

    T2Info("%s ++out\n", __FUNCTION__);
    return ret;
}

// Helper function to acquire handle only
static T2ERROR acquire_pool_handle(CURL **easy, int *idx)
{
    if (!pool_initialized)
    {
        T2ERROR ret = init_connection_pool();
        if(ret != T2ERROR_SUCCESS)
        {
            T2Error("Failed to initialize connection pool\n");
            return ret;
        }
    }

    *idx = -1;

    pthread_mutex_lock(&pool.pool_mutex);

    // Find an available handle
    for(int i = 0; i < MAX_POOL_SIZE; i++)
    {
        if(pool.handle_available[i])
        {
            T2Info("acquire_pool_handle ; Available handle = %d\n", i);
            *idx = i;
            pool.handle_available[i] = false;
            break;
        }
    }

    pthread_mutex_unlock(&pool.pool_mutex);

    if(*idx == -1)
    {
        T2Error("No available HTTP handles\n");
        return T2ERROR_FAILURE;
    }

    *easy = pool.easy_handles[*idx];
    return T2ERROR_SUCCESS;
}

// Dedicated GET API
T2ERROR http_pool_get(const char *url, char **response_data, bool enable_file_output)
{
    T2Info("%s ++in\n", __FUNCTION__);

    if (!url)
    {
        T2Error("Invalid URL parameter\n");
        return T2ERROR_FAILURE;
    }

    T2Info("%s ; GET url = %s\n", __FUNCTION__, url);

    CURL *easy;
    curlResponseData *response;
    int idx;

    T2ERROR ret = acquire_pool_handle(&easy, &idx);
    if (ret != T2ERROR_SUCCESS)
    {
        return ret;
    }

    // Allocate response buffer locally for GET requests
    response = (curlResponseData*)malloc(sizeof(curlResponseData));
    if (response)
    {
        response->data = (char*)malloc(1);
        if (response->data)
        {
            response->data[0] = '\0';
            response->size = 0;
        }
        else
        {
            free(response);
            response = NULL;
            T2Error("Failed to allocate memory for response buffer\n");
            return T2ERROR_FAILURE;
        }
    }
    else
    {
        T2Error("Failed to allocate memory for response structure\n");
        return T2ERROR_FAILURE;
    }

    // Configure for GET request
    curl_easy_setopt(easy, CURLOPT_URL, url);
    curl_easy_setopt(easy, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, httpGetCallBack);
    curl_easy_setopt(easy, CURLOPT_WRITEDATA, (void *) response);

#if defined(ENABLE_RDKB_SUPPORT) && !defined(RDKB_EXTENDER)
#if defined(WAN_FAILOVER_SUPPORTED) || defined(FEATURE_RDKB_CONFIGURABLE_WAN_INTERFACE)
    curl_easy_setopt(easy, CURLOPT_INTERFACE, waninterface);
#else
    curl_easy_setopt(easy, CURLOPT_INTERFACE, "erouter0");
#endif
#endif

    // Execute the request
    ret = http_pool_execute_request(easy, idx);

    if (ret == T2ERROR_SUCCESS && response->data)
    {
        T2Info("%s ; Response data size = %zu\n", __FUNCTION__, response->size);

        // Handle file output for GET requests
        if (enable_file_output)
        {
            FILE *httpOutput = fopen(HTTP_RESPONSE_FILE, "w+");
            if(httpOutput)
            {
                T2Debug("Update config data in response file %s \n", HTTP_RESPONSE_FILE);
                fputs(response->data, httpOutput);
                fclose(httpOutput);
            }
            else
            {
                T2Error("Unable to open %s file \n", HTTP_RESPONSE_FILE);
            }
        }

        // Copy response data if requested
        if (response_data)
        {
            *response_data = NULL;
            if(response->size <= SIZE_MAX)
            {
                *response_data = (char*)malloc(response->size + 1);
                if(*response_data)
                {
                    memcpy(*response_data, response->data, response->size);
                    (*response_data)[response->size] = '\0';
                }
            }
        }
    }
    else if (ret == T2ERROR_SUCCESS)
    {
        T2Error("%s ; No response data received\n", __FUNCTION__);
        ret = T2ERROR_FAILURE;
    }

    // Free local response buffer
    if (response)
    {
        if (response->data)
        {
            free(response->data);
        }
        free(response);
    }

    T2Info("%s ++out\n", __FUNCTION__);
    return ret;
}

// Dedicated POST API
T2ERROR http_pool_post(const char *url, const char *payload)
{
    T2Info("%s ++in\n", __FUNCTION__);

    if (!url || !payload)
    {
        T2Error("Invalid URL or payload parameter\n");
        return T2ERROR_FAILURE;
    }

    T2Info("%s ; POST url = %s\n", __FUNCTION__, url);

    CURL *easy;
    int idx;

    T2ERROR ret = acquire_pool_handle(&easy, &idx);
    if (ret != T2ERROR_SUCCESS)
    {
        return ret;
    }

    // Configure for POST request
    curl_easy_setopt(easy, CURLOPT_URL, url);
    curl_easy_setopt(easy, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(easy, CURLOPT_HTTPHEADER, pool.post_headers);
    curl_easy_setopt(easy, CURLOPT_POSTFIELDS, payload);
    curl_easy_setopt(easy, CURLOPT_POSTFIELDSIZE, strlen(payload));
    curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, writeToFile);

#if defined(ENABLE_RDKB_SUPPORT) && !defined(RDKB_EXTENDER)
#if defined(WAN_FAILOVER_SUPPORTED) || defined(FEATURE_RDKB_CONFIGURABLE_WAN_INTERFACE)
    curl_easy_setopt(easy, CURLOPT_INTERFACE, waninterface);
#else
    curl_easy_setopt(easy, CURLOPT_INTERFACE, "erouter0");
#endif
#endif

    // Execute the request
    ret = http_pool_execute_request(easy, idx);

    T2Info("%s ++out\n", __FUNCTION__);
    return ret;
}

T2ERROR http_pool_request_ex(const http_pool_request_config_t *config)
{
    T2Info("%s ++in\n", __FUNCTION__);

    if (!config || !config->url)
    {
        T2Error("Invalid configuration parameters\n");
        return T2ERROR_FAILURE;
    }

    T2Info("%s ; url = %s, type = %s\n", __FUNCTION__, config->url,
           (config->type == HTTP_REQUEST_GET) ? "GET" : "POST");

    if (!pool_initialized)
    {
        T2ERROR ret = init_connection_pool();
        if(ret != T2ERROR_SUCCESS)
        {
            T2Error("Failed to initialize connection pool\n");
            return ret;
        }
        pool_initialized = true;
    }
    CURL *easy;
    int idx = -1;

    pthread_mutex_lock(&pool.pool_mutex);

    // Find an available handle
    for(int i = 0; i < MAX_POOL_SIZE; i++)
    {
        if(pool.handle_available[i])
        {
            T2Info("%s ; Available handle = %d\n", __FUNCTION__, i);
            idx = i;
            pool.handle_available[i] = false;
            break;
        }
    }

    pthread_mutex_unlock(&pool.pool_mutex);

    if(idx == -1)
    {
        T2Error("No available HTTP handles\n");
        return T2ERROR_FAILURE;
    }

    easy = pool.easy_handles[idx];

    curlResponseData* response = NULL;
    if (config->type == HTTP_REQUEST_GET)
    {
        // Allocate response buffer locally for GET requests
        response = (curlResponseData*)malloc(sizeof(curlResponseData));
        if (response)
        {
            response->data = (char*)malloc(RESPONSE_BUFFER_SIZE);
            if (response->data)
            {
                response->data[0] = '\0';
                response->size = 0;
            }
            else
            {
                free(response);
                response = NULL;
                T2Error("Failed to allocate memory for response buffer\n");
                return T2ERROR_FAILURE;
            }
        }
        else
        {
            T2Error("Failed to allocate memory for response structure\n");
            return T2ERROR_FAILURE;
        }
    }

    // Set common options
    curl_easy_setopt(easy, CURLOPT_URL, config->url);

#if defined(ENABLE_RDKB_SUPPORT) && !defined(RDKB_EXTENDER)
#if defined(WAN_FAILOVER_SUPPORTED) || defined(FEATURE_RDKB_CONFIGURABLE_WAN_INTERFACE)
    curl_easy_setopt(easy, CURLOPT_INTERFACE, waninterface);
#else
    curl_easy_setopt(easy, CURLOPT_INTERFACE, "erouter0");
#endif
#endif

    //TODO: separate POST and GET into 2 apis
    if (config->type == HTTP_REQUEST_POST)
    {
        // POST request configuration (for sendReportOverHTTP)
        curl_easy_setopt(easy, CURLOPT_CUSTOMREQUEST, "POST");

        curl_easy_setopt(easy, CURLOPT_HTTPHEADER, pool.post_headers);

        if (config->payload)
        {
            curl_easy_setopt(easy, CURLOPT_POSTFIELDS, config->payload);
            curl_easy_setopt(easy, CURLOPT_POSTFIELDSIZE, strlen(config->payload));
        }

        curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, writeToFile);
    }
    else
    {
        // GET request configuration (for doHttpGet)
        curl_easy_setopt(easy, CURLOPT_HTTPGET, 1L);
        curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, httpGetCallBack);
        curl_easy_setopt(easy, CURLOPT_WRITEDATA, (void *) response);
    }

    // Add to multi handle
    curl_multi_add_handle(pool.multi_handle, easy);

    int still_running;
    do
    {
        T2Info("%s ; Performing curl request\n", __FUNCTION__);
        curl_multi_perform(pool.multi_handle, &still_running);
        curl_multi_wait(pool.multi_handle, NULL, 0, 500, NULL); // TODO: Wait has to be revisited
    }
    while(still_running);

    T2Info("%s ; Curl request completed\n", __FUNCTION__);

    long http_code;
    curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &http_code);
    T2Info("%s ; HTTP response code: %ld\n", __FUNCTION__, http_code);

    T2ERROR ret = T2ERROR_FAILURE;
    if (http_code == 200)
    {
        ret = T2ERROR_SUCCESS;

        if (response && response->data)
        {
            T2Info("%s ; Response data size = %zu\n", __FUNCTION__, response->size);

            // Handle file output for GET requests
            if (config->type == HTTP_REQUEST_GET && config->enable_file_output)
            {
                FILE *httpOutput = fopen(HTTP_RESPONSE_FILE, "w+");
                if(httpOutput)
                {
                    T2Debug("Update config data in response file %s \n", HTTP_RESPONSE_FILE);
                    fputs(response->data, httpOutput);
                    fclose(httpOutput);
                }
                else
                {
                    T2Error("Unable to open %s file \n", HTTP_RESPONSE_FILE);
                }
            }

            // Copy response data if requested
            if (config->response_data && config->type == HTTP_REQUEST_GET)
            {
                *config->response_data = NULL;
                if(response->size <= SIZE_MAX)
                {
                    *config->response_data = (char*)malloc(response->size + 1);
                    if(*config->response_data)
                    {
                        memcpy(*config->response_data, response->data, response->size);
                        (*config->response_data)[response->size] = '\0';
                    }
                }
            }
        }
        else
        {
            T2Error("%s ; No response data received\n", __FUNCTION__);
        }
    }
    else
    {
        T2Error("%s ; HTTP request failed with code: %ld\n", __FUNCTION__, http_code);
    }

    // Cleanup
    curl_multi_remove_handle(pool.multi_handle, easy);

    pthread_mutex_lock(&pool.pool_mutex);
    pool.handle_available[idx] = true;
    pthread_mutex_unlock(&pool.pool_mutex);

    // Free local response buffer
    if (response)
    {
        if (response->data)
        {
            free(response->data);
        }
        free(response);
    }

    T2Info("%s ++out\n", __FUNCTION__);
    return ret;
}

T2ERROR http_pool_request(const char *url, const char *payload, char **data)
{
    // Backward compatibility wrapper
    http_pool_request_config_t config =
    {
        .type = payload ? HTTP_REQUEST_POST : HTTP_REQUEST_GET,
        .url = url,
        .payload = payload,
        .response_data = data,
        .enable_mtls = true,
        .enable_file_output = (payload == NULL) // Enable file output for GET requests
    };

    return http_pool_request_ex(&config);
}

T2ERROR http_pool_cleanup(void)
{
    if (!pool_initialized) {
        T2Info("Pool not initialized, nothing to cleanup\n");
        return T2ERROR_SUCCESS;
    }

    T2Info("%s ++in\n", __FUNCTION__);
    
    // FIX: Free all header lists before cleanup
    if(pool.post_headers) {
        curl_slist_free_all(pool.post_headers);
        pool.post_headers = NULL;
    }

    // Cleanup all easy handles
    for(int i = 0; i < MAX_POOL_SIZE; i++)
    {
        if(pool.easy_handles[i]) {
            curl_easy_cleanup(pool.easy_handles[i]);
            pool.easy_handles[i] = NULL;
        }
    }
    
    // Cleanup multi handle
    if(pool.multi_handle) {
        curl_multi_cleanup(pool.multi_handle);
        pool.multi_handle = NULL;
    }
    
    // Destroy mutex
    pthread_mutex_destroy(&pool.pool_mutex);
    
    // Reset initialization flag
    pool_initialized = false;
    
    T2Info("%s ++out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}