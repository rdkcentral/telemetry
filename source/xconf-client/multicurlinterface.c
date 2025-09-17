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
#define MAX_POOL_SIZE 5
#define HTTP_RESPONSE_FILE "/tmp/httpOutput.txt"

// High-level design for connection pooling
typedef struct
{
    CURLM *multi_handle;
    CURL *easy_handles[MAX_POOL_SIZE];
    bool handle_available[MAX_POOL_SIZE];
    pthread_mutex_t pool_mutex;
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

    char *ptr = (char*) realloc(httpResponse->data,
                                httpResponse->size + realsize + 1);
    if (!ptr)
    {
        T2Error("%s:%u , T2:memory realloc failed\n", __func__, __LINE__);
        return 0;
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

T2ERROR init_connection_pool()
{
    static bool pool_initialized = false;

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
        curl_easy_setopt(pool.easy_handles[i], CURLOPT_TIMEOUT, 30);
        curl_easy_setopt(pool.easy_handles[i], CURLOPT_CONNECTTIMEOUT, 10);
        curl_easy_setopt(pool.easy_handles[i], CURLOPT_TCP_KEEPALIVE, 1L);

        curl_easy_setopt(pool.easy_handles[i], CURLOPT_TCP_KEEPIDLE, 120L);
        curl_easy_setopt(pool.easy_handles[i], CURLOPT_TCP_KEEPINTVL, 60L);
        curl_easy_setopt(pool.easy_handles[i], CURLOPT_MAXCONNECTS, 5L);

        code = curl_easy_setopt(pool.easy_handles[i], CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
        if(code != CURLE_OK)
        {
            T2Error("%s : Curl set opts failed with error %s \n", __FUNCTION__, curl_easy_strerror(code));
        }
        code = curl_easy_setopt(pool.easy_handles[i], CURLOPT_WRITEFUNCTION, httpGetCallBack);
        if(code != CURLE_OK)
        {
            T2Error("%s : Curl set opts failed with error %s \n", __FUNCTION__, curl_easy_strerror(code));
        }
        if(T2ERROR_SUCCESS == getMtlsCerts(&pCertFile, &pPasswd))
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
    }

    pthread_mutex_init(&pool.pool_mutex, NULL);
    pool_initialized = true;
    T2Info("%s ++out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
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

    CURL *easy;
    //CURLcode code;
    int idx = -1;
    //ssize_t readBytes = 0;
    struct curl_slist *headers = NULL;
    char *pCertFile = NULL;
    char *pPasswd = NULL;
    //FILE *fp = NULL;

    curlResponseData* response = (curlResponseData *) malloc(sizeof(curlResponseData));
    response->data = (char*)malloc(1);
    response->data[0] = '\0';
    response->size = 0;

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
        free(response->data);
        free(response);
        return T2ERROR_FAILURE;
    }

    easy = pool.easy_handles[idx];

    // Reset handle to clean state
    //curl_easy_reset(easy);

    // Set common options
    curl_easy_setopt(easy, CURLOPT_URL, config->url);
    curl_easy_setopt(easy, CURLOPT_TIMEOUT, 30);
    curl_easy_setopt(easy, CURLOPT_CONNECTTIMEOUT, 10);
    curl_easy_setopt(easy, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(easy, CURLOPT_TCP_KEEPIDLE, 120L);
    curl_easy_setopt(easy, CURLOPT_TCP_KEEPINTVL, 60L);
    curl_easy_setopt(easy, CURLOPT_MAXCONNECTS, 5L);
    curl_easy_setopt(easy, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);

#if defined(ENABLE_RDKB_SUPPORT) && !defined(RDKB_EXTENDER)
#if defined(WAN_FAILOVER_SUPPORTED) || defined(FEATURE_RDKB_CONFIGURABLE_WAN_INTERFACE)
    curl_easy_setopt(easy, CURLOPT_INTERFACE, waninterface);
#else
    curl_easy_setopt(easy, CURLOPT_INTERFACE, "erouter0");
#endif
#endif

    // Configure based on request type
    if (config->type == HTTP_REQUEST_POST)
    {
        // POST request configuration (for sendReportOverHTTP)
        curl_easy_setopt(easy, CURLOPT_CUSTOMREQUEST, "POST");

        // Set headers for POST
        headers = curl_slist_append(NULL, "Accept: application/json");
        headers = curl_slist_append(headers, "Content-type: application/json");
        curl_easy_setopt(easy, CURLOPT_HTTPHEADER, headers);

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

    // Handle mTLS if enabled
    if (config->enable_mtls && T2ERROR_SUCCESS == getMtlsCerts(&pCertFile, &pPasswd))
    {
        curl_easy_setopt(easy, CURLOPT_SSLCERTTYPE, "P12");
        curl_easy_setopt(easy, CURLOPT_SSLCERT, pCertFile);
        curl_easy_setopt(easy, CURLOPT_KEYPASSWD, pPasswd);
        curl_easy_setopt(easy, CURLOPT_SSL_VERIFYPEER, 1L);
    }

    // Add to multi handle
    curl_multi_add_handle(pool.multi_handle, easy);

    int still_running;
    do
    {
        T2Info("%s ; Performing curl request\n", __FUNCTION__);
        curl_multi_perform(pool.multi_handle, &still_running);
        curl_multi_wait(pool.multi_handle, NULL, 0, 1000, NULL);
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

        if (response->data)
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
    free(response->data);
    free(response);

    if (headers)
    {
        curl_slist_free_all(headers);
    }

    curl_multi_remove_handle(pool.multi_handle, easy);

    pthread_mutex_lock(&pool.pool_mutex);
    pool.handle_available[idx] = true;
    pthread_mutex_unlock(&pool.pool_mutex);

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
    for(int i = 0; i < MAX_POOL_SIZE; i++)
    {
        curl_easy_cleanup(pool.easy_handles[i]);
    }
    curl_multi_cleanup(pool.multi_handle);
    pthread_mutex_destroy(&pool.pool_mutex);
    return T2ERROR_SUCCESS;
}
