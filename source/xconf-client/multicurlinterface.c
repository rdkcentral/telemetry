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

//Global variables
#define MAX_POOL_SIZE 5

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

T2ERROR init_connection_pool()
{
    T2Info("%s ++in\n", __FUNCTION__);
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
        curl_easy_setopt(pool.easy_handles[i], CURLOPT_MAXCONNECTS, 5L);  // Connection cache size
    }
    
    pthread_mutex_init(&pool.pool_mutex, NULL);
    T2Info("%s ++out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

T2ERROR http_pool_request(const char *url, const char *payload, curlResponseData *response)
{
    T2Info("%s ++in\n", __FUNCTION__);
    T2Info("%s ; url = %s\n", __FUNCTION__, url);
    CURL *easy;
    //CURLcode res;
    int idx = -1;

    //(void *) payload;
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
    curl_easy_setopt(easy, CURLOPT_POSTFIELDS, payload);
    
    // Response handling
    //curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(easy, CURLOPT_WRITEDATA, (void *) response);
    
    // Add to multi handle
    curl_multi_add_handle(pool.multi_handle, easy);
    
    int still_running;
    do {
        curl_multi_perform(pool.multi_handle, &still_running);
        T2Info("%s ; Performing curl request for %s\n", __FUNCTION__, url);
        // Wait for activity or timeout
        curl_multi_wait(pool.multi_handle, NULL, 0, 1000, NULL);
    } while(still_running);
    size_t len = strlen(response->data);
    T2Info("%s ; Response data size = %zu\n", __FUNCTION__, len);
    if (response->data)
    {
        T2Info("%s ; Response data size = %zu\n", __FUNCTION__, response->size);
    }
    else
    {
        T2Error("%s ; No response data received\n", __FUNCTION__);
    }
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
