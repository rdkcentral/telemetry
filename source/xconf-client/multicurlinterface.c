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
typedef struct {
    CURLM *multi_handle;
    CURL *easy_handles[MAX_POOL_SIZE];
    bool handle_available[MAX_POOL_SIZE];
    pthread_mutex_t pool_mutex;
} http_connection_pool_t;

typedef struct http_pool_config {
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

T2ERROR init_connection_pool()
{
    CURLcode code = CURLE_OK;
    T2Info("%s ++in\n", __FUNCTION__);
    char *pCertFile = NULL;
    char *pPasswd = NULL;
    pool.multi_handle = curl_multi_init();
    
    // Pre-allocate easy handles
    for(int i = 0; i < MAX_POOL_SIZE; i++) {
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
    T2Info("%s ++out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

T2ERROR http_pool_request(const char *url, const char *payload, char **data)
{
    T2Info("%s ++in\n", __FUNCTION__);
    T2Info("%s ; url = %s\n", __FUNCTION__, url);
    CURL *easy;
    //CURLcode res;
    int idx = -1;
    ssize_t readBytes = 0;

    curlResponseData* response = (curlResponseData *) malloc(sizeof(curlResponseData));
    response->data = (char*)malloc(1);
    response->data[0] = '\0'; //CID 282084 : Uninitialized scalar variable (UNINIT)
    response->size = 0;

    if(payload)
    {
        T2Info("%s ; payload = %s\n", __FUNCTION__, payload);
    }
    //(void *) response;
    pthread_mutex_lock(&pool.pool_mutex);
    
    // Find an available handle
    for(int i = 0; i < MAX_POOL_SIZE; i++) {
        if(pool.handle_available[i]) {
            T2Info("%s ; Available handle = %d\n", __FUNCTION__, i);
            idx = i;
            pool.handle_available[i] = false;
            break;
        }
    }
    
    pthread_mutex_unlock(&pool.pool_mutex);
    
    if(idx == -1) {
        T2Error("No available HTTP handles\n");
        return T2ERROR_FAILURE;
    }
    
    easy = pool.easy_handles[idx];
    
    // Set URL and payload
    curl_easy_setopt(easy, CURLOPT_URL, url);
    //curl_easy_setopt(easy, CURLOPT_POSTFIELDS, payload);
    
    // Response handling
    curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, httpGetCallBack);
    curl_easy_setopt(easy, CURLOPT_WRITEDATA, (void *) response);
    
    // Add to multi handle
    curl_multi_add_handle(pool.multi_handle, easy);
    
    int still_running;
    do {
        T2Info("%s ; Going to Perform curl request\n", __FUNCTION__);
        curl_multi_perform(pool.multi_handle, &still_running);
        T2Info("%s ; Performing curl request for %s\n", __FUNCTION__, url);
        // Wait for activity or timeout
        curl_multi_wait(pool.multi_handle, NULL, 0, 1000, NULL);
        T2Info("%s ; After wait\n", __FUNCTION__);
    } while(still_running);

    T2Info("%s ; Performed curl request\n", __FUNCTION__);
    if (response->data)
    {
        T2Info("%s ; Response data size = %zu\nResponse data = %s\n", __FUNCTION__, response->size, response->data);
    }
    else
    {
        T2Error("%s ; No response data received\n", __FUNCTION__);
    }

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

    *data = NULL;
    if(response->size <= SIZE_MAX)
    {
        *data = (char*)malloc(response->size + 1);
    }
    if(*data == NULL)
    {
        T2Error("Unable to allocate memory for XCONF config data \n");
        //ret = T2ERROR_FAILURE;
    }
    else
    {
        if(response->size <= SIZE_MAX)
        {
            memset(*data, '\0', response->size + 1);
        }
        FILE *httpOutput = fopen(HTTP_RESPONSE_FILE, "r+");
        if(httpOutput)
        {
            // Read the whole file content
            if(response->size <= SIZE_MAX)
            {
                readBytes = fread(*data, response->size, 1, httpOutput);
            }
            if(readBytes == -1)
            {
                T2Error("Failed to read from pipe\n");
                return T2ERROR_FAILURE;
            }
            T2Debug("Configuration obtained from http server : \n %s \n", *data);
            fclose(httpOutput);
        }
    }

    free(response->data);
    free(response);

    // Cleanup
    curl_multi_remove_handle(pool.multi_handle, easy);
    
    pthread_mutex_lock(&pool.pool_mutex);
    pool.handle_available[idx] = true;
    pthread_mutex_unlock(&pool.pool_mutex);
    
    T2Info("%s ++out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

T2ERROR http_pool_cleanup(void)
{
    for(int i = 0; i < MAX_POOL_SIZE; i++) {
        curl_easy_cleanup(pool.easy_handles[i]);
    }
    curl_multi_cleanup(pool.multi_handle);
    pthread_mutex_destroy(&pool.pool_mutex);
    return T2ERROR_SUCCESS;
}