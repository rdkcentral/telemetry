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
#include <stdbool.h>
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
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include "multicurlinterface.h"
#include "busInterface.h"
#include "t2log_wrapper.h"
#include "reportprofiles.h"
#include "t2MtlsUtils.h"
#ifdef LIBRDKCERTSEL_BUILD
#include "rdkcertselector.h"
#define FILESCHEME "file://"
#endif

// Macro to check curl_easy_setopt return value and log errors
#define CURL_SETOPT_CHECK(handle, option, value) \
    do { \
        CURLcode _curl_code = curl_easy_setopt(handle, option, value); \
        if(_curl_code != CURLE_OK) { \
            T2Error("%s : Failed to set %s=%s, error: %s\n", __FUNCTION__, #option, #value, curl_easy_strerror(_curl_code)); \
        } \
    } while(0)

// Macro for string values that need proper formatting
#define CURL_SETOPT_CHECK_STR(handle, option, value) \
    do { \
        CURLcode _curl_code = curl_easy_setopt(handle, option, value); \
        if(_curl_code != CURLE_OK) { \
            const char* _val_str = (const char*)(value); \
            T2Error("%s : Failed to set %s=%s, error: %s\n", __FUNCTION__, #option, _val_str ? _val_str : "NULL", curl_easy_strerror(_curl_code)); \
        } \
    } while(0)

//Global variables
#define IFINTERFACE      "erouter0"
#define MAX_POOL_SIZE 3
#define HTTP_RESPONSE_FILE "/tmp/httpOutput.txt"
#define RESPONSE_BUFFER_SIZE 8192
static bool pool_initialized = false;

typedef struct
{
    CURL *easy_handles[MAX_POOL_SIZE];
    bool handle_available[MAX_POOL_SIZE];
    pthread_mutex_t pool_mutex;
    pthread_cond_t handle_available_cond;
    struct curl_slist *post_headers;
} http_connection_pool_t;

static http_connection_pool_t pool = {0};

#ifdef LIBRDKCERTSEL_BUILD
static rdkcertselector_h curlCertSelector = NULL;
static rdkcertselector_h curlRcvryCertSelector = NULL;

#if defined(ENABLE_RED_RECOVERY_SUPPORT)
bool isStateRedEnabled(void)
{
    return access("/tmp/stateRedEnabled", F_OK) == 0;
}
#endif

void curlCertSelectorFree()
{
    rdkcertselector_free(&curlCertSelector);
    rdkcertselector_free(&curlRcvryCertSelector);
    if(curlCertSelector == NULL && curlRcvryCertSelector == NULL)
    {
        T2Info("%s, T2:Cert selector memory free\n", __func__);
    }
    else
    {
        T2Info("%s, T2:Cert selector memory free failed\n", __func__);
    }
}

void curlCertSelectorInit()
{
    bool state_red_enable = false;
#if defined(ENABLE_RED_RECOVERY_SUPPORT)
    state_red_enable = isStateRedEnabled();
#endif
    if (state_red_enable && curlRcvryCertSelector == NULL )
    {
        curlRcvryCertSelector = rdkcertselector_new( NULL, NULL, "RCVRY" );
        if (curlRcvryCertSelector == NULL)
        {
            T2Error("%s, T2:stateRed Cert selector initialization failed\n", __func__);
        }
        else
        {
            T2Info("%s, T2:stateRed Cert selector initialization successfully\n", __func__);
        }
    }
    else
    {
        if (curlCertSelector == NULL)
        {
            curlCertSelector = rdkcertselector_new( NULL, NULL, "MTLS" );
            if (curlCertSelector == NULL)
            {
                T2Error("%s, T2:Cert selector initialization failed\n", __func__);
            }
            else
            {
                T2Info("%s, T2:Cert selector initialization successfully\n", __func__);
            }
        }
    }
}

#endif

static size_t httpGetCallBack(void *response, size_t len, size_t nmemb,
                              void *stream)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    size_t realsize = len * nmemb;
    curlResponseData* httpResponse = (curlResponseData*) stream;

    // FIX: Check for NULL stream to prevent crashes
    if (!httpResponse)
    {
        T2Error("httpGetCallBack: NULL stream parameter\n");
        return 0;
    }

    char *ptr = (char*) realloc(httpResponse->data,
                                httpResponse->size + realsize + 1);
    if (!ptr)
    {
        T2Error("%s:%u , T2:memory realloc failed\n", __func__, __LINE__);
        // On realloc failure, abort the transfer to avoid silently dropping data
        // Return 0 to signal an error to curl
        return 0;
    }
    httpResponse->data = ptr;
    memcpy(&(httpResponse->data[httpResponse->size]), response, realsize);
    httpResponse->size += realsize;
    httpResponse->data[httpResponse->size] = 0;

    T2Debug("%s ++out\n", __FUNCTION__);
    return realsize;
}

static void cleanup_curl_handles(void)
{
    T2Debug("%s ++in\n", __FUNCTION__);

    if(pool.post_headers)
    {
        curl_slist_free_all(pool.post_headers);
        pool.post_headers = NULL;
    }

    for(int i = 0; i < MAX_POOL_SIZE; i++)
    {
        if(pool.easy_handles[i])
        {
            curl_easy_cleanup(pool.easy_handles[i]);
            pool.easy_handles[i] = NULL;
            pool.handle_available[i] = false;
        }
    }

    T2Debug("%s ++out\n", __FUNCTION__);
}

T2ERROR init_connection_pool()
{
    T2Debug("%s ++in\n", __FUNCTION__);

    // Initialize synchronization primitives first
    if(pthread_mutex_init(&pool.pool_mutex, NULL) != 0)
    {
        T2Error("%s : Failed to initialize pool mutex\n", __FUNCTION__);
        return T2ERROR_FAILURE;
    }

    if(pthread_cond_init(&pool.handle_available_cond, NULL) != 0)
    {
        T2Error("%s : Failed to initialize condition variable\n", __FUNCTION__);
        pthread_mutex_destroy(&pool.pool_mutex);
        return T2ERROR_FAILURE;
    }

    pthread_mutex_lock(&pool.pool_mutex);

#ifdef LIBRDKCERTSEL_BUILD
    // Initialize certificate selector before setting up connection pool
    curlCertSelectorInit();
#endif

    // Pre-allocate easy handles
    for(int i = 0; i < MAX_POOL_SIZE; i++)
    {
        pool.easy_handles[i] = curl_easy_init();
        if(pool.easy_handles[i] == NULL)
        {
            T2Error("%s : Failed to initialize curl handle %d\n", __FUNCTION__, i);
            // Cleanup previously initialized handles using helper function
            cleanup_curl_handles();
            pthread_mutex_unlock(&pool.pool_mutex);
            pthread_cond_destroy(&pool.handle_available_cond);
            pthread_mutex_destroy(&pool.pool_mutex);
            return T2ERROR_FAILURE;
        }
        pool.handle_available[i] = true;

        //Set common options for each handle
        CURL_SETOPT_CHECK(pool.easy_handles[i], CURLOPT_TIMEOUT, 30L);
        CURL_SETOPT_CHECK(pool.easy_handles[i], CURLOPT_CONNECTTIMEOUT, 10L);

        // More aggressive keepalive settings for your environment
        CURL_SETOPT_CHECK(pool.easy_handles[i], CURLOPT_TCP_KEEPALIVE, 1L);
        CURL_SETOPT_CHECK(pool.easy_handles[i], CURLOPT_TCP_KEEPIDLE, 50L);
        CURL_SETOPT_CHECK(pool.easy_handles[i], CURLOPT_TCP_KEEPINTVL, 30L);

#ifdef CURLOPT_TCP_KEEPCNT
        // The option CURLOPT_TCP_KEEPCNT is not available in libcurl versions older than 7.25.0
        // Set number of keepalive probes before considering the connection dead
        CURL_SETOPT_CHECK(pool.easy_handles[i], CURLOPT_TCP_KEEPCNT, 15L);
#endif

        // Add connection reuse validation
        CURL_SETOPT_CHECK(pool.easy_handles[i], CURLOPT_FORBID_REUSE, 0L);
        CURL_SETOPT_CHECK(pool.easy_handles[i], CURLOPT_FRESH_CONNECT, 0L);

        // Add low-level socket options for better connection health detection
        CURL_SETOPT_CHECK(pool.easy_handles[i], CURLOPT_NOSIGNAL, 1L);

        // Add HTTP-level keep-alive headers for better server compatibility
        CURL_SETOPT_CHECK(pool.easy_handles[i], CURLOPT_HTTPHEADER, NULL);

        // Connection management options that work with older libcurl
        CURL_SETOPT_CHECK(pool.easy_handles[i], CURLOPT_PIPEWAIT, 0L);
        CURL_SETOPT_CHECK(pool.easy_handles[i], CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
        CURL_SETOPT_CHECK(pool.easy_handles[i], CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);

        // Certificate selector and SSL/TLS specific options from original code
        CURL_SETOPT_CHECK(pool.easy_handles[i], CURLOPT_SSL_VERIFYPEER, 1L);
        CURL_SETOPT_CHECK(pool.easy_handles[i], CURLOPT_SSLENGINE_DEFAULT, 1L);
    }

    pool.post_headers = curl_slist_append(NULL, "Accept: application/json");
    if (pool.post_headers == NULL)
    {
        T2Error("%s : Failed to append HTTP header: Accept: application/json\n", __FUNCTION__);
        return T2ERROR_FAILURE;
    }

    pool.post_headers = curl_slist_append(pool.post_headers, "Content-type: application/json");
    if (pool.post_headers == NULL)
    {
        T2Error("%s : Failed to append HTTP header: Content-type: application/json\n", __FUNCTION__);
        curl_slist_free_all(pool.post_headers);
        pool.post_headers = NULL;
        return T2ERROR_FAILURE;
    }

    pool_initialized = true;
    pthread_mutex_unlock(&pool.pool_mutex);
    T2Debug("%s ++out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

// Helper function to acquire any available handle with waiting
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

    pthread_mutex_lock(&pool.pool_mutex);

    // Waits until a handle becomes available
    while(1)
    {
        // Find an available handle
        for(int i = 0; i < MAX_POOL_SIZE; i++)
        {
            if(pool.handle_available[i])
            {
                T2Debug("acquire_pool_handle ; Available handle = %d\n", i);
                *idx = i;
                pool.handle_available[i] = false;
                *easy = pool.easy_handles[i];

                pthread_mutex_unlock(&pool.pool_mutex);
                return T2ERROR_SUCCESS;
            }
        }

        // No handle available, wait for one to become free
        T2Info("No curl handle available, waiting for one to become free...\n");
        pthread_cond_wait(&pool.handle_available_cond, &pool.pool_mutex);
    }
}

// Helper function to release handle back to pool
static void release_pool_handle(int idx)
{
    pthread_mutex_lock(&pool.pool_mutex);
    pool.handle_available[idx] = true;
    pthread_cond_signal(&pool.handle_available_cond); // Signal waiting threads
    pthread_mutex_unlock(&pool.pool_mutex);
}

// GET API - Updated to use shared pool with waiting
T2ERROR http_pool_get(const char *url, char **response_data, bool enable_file_output)
{
    T2Info("%s ++in\n", __FUNCTION__);

    if (!url)
    {
        T2Error("Invalid URL parameter\n");
        return T2ERROR_FAILURE;
    }

    T2Info("%s ; GET url = %s\n", __FUNCTION__, url);

    // Acquire any available handle (with waiting)
    CURL *easy;
    int idx = -1;

    T2ERROR ret = acquire_pool_handle(&easy, &idx);
    if(ret != T2ERROR_SUCCESS)
    {
        T2Error("Failed to acquire pool handle\n");
        return ret;
    }

    // Clear any POST-specific settings so that the handle can be used for GET operations
    CURL_SETOPT_CHECK(easy, CURLOPT_CUSTOMREQUEST, NULL);
    CURL_SETOPT_CHECK(easy, CURLOPT_POSTFIELDS, NULL);
    CURL_SETOPT_CHECK(easy, CURLOPT_POSTFIELDSIZE, 0L);
    CURL_SETOPT_CHECK(easy, CURLOPT_HTTPHEADER, NULL);

    T2Info("http_pool_get using handle %d\n", idx);

    // Allocate response buffer locally for GET requests only
    curlResponseData response;
    response.data = (char*)malloc(RESPONSE_BUFFER_SIZE);
    if (!response.data)
    {
        T2Error("Failed to allocate response buffer\n");
        release_pool_handle(idx);
        return T2ERROR_FAILURE;
    }
    response.data[0] = '\0';
    response.size = 0;

    // Configure basic options for GET request
    CURL_SETOPT_CHECK_STR(easy, CURLOPT_URL, url);
    CURL_SETOPT_CHECK(easy, CURLOPT_HTTPGET, 1L);
    CURL_SETOPT_CHECK(easy, CURLOPT_WRITEFUNCTION, httpGetCallBack);
    CURL_SETOPT_CHECK(easy, CURLOPT_WRITEDATA, (void *) &response);

#if defined(ENABLE_RDKB_SUPPORT) && !defined(RDKB_EXTENDER)
#if defined(WAN_FAILOVER_SUPPORTED) || defined(FEATURE_RDKB_CONFIGURABLE_WAN_INTERFACE)
    char *paramVal = NULL;
    char waninterface[256];
    memset(waninterface, 0, sizeof(waninterface));
    snprintf(waninterface, sizeof(waninterface), "%s", IFINTERFACE);

    if(T2ERROR_SUCCESS == getParameterValue(TR181_DEVICE_CURRENT_WAN_IFNAME, &paramVal))
    {
        if(strlen(paramVal) > 0)
        {
            memset(waninterface, 0, sizeof(waninterface));
            snprintf(waninterface, sizeof(waninterface), "%s", paramVal);
            T2Info("TR181_DEVICE_CURRENT_WAN_IFNAME -- %s\n", waninterface);
        }
        free(paramVal);
        paramVal = NULL;
    }
    else
    {
        T2Error("Failed to get Value for %s\n", TR181_DEVICE_CURRENT_WAN_IFNAME);
    }

    CURL_SETOPT_CHECK_STR(easy, CURLOPT_INTERFACE, waninterface);
    T2Info("TR181_DEVICE_CURRENT_WAN_IFNAME ---- %s\n", waninterface);
#else
    CURL_SETOPT_CHECK(easy, CURLOPT_INTERFACE, "erouter0");
#endif
#endif

    //Certificate handling - check if mTLS is enabled
    bool mtls_enable = isMtlsEnabled();
    char *pCertFile = NULL;
    char *pPasswd = NULL;
    CURLcode curl_code = CURLE_OK;

    if(mtls_enable == true)
    {
#ifdef LIBRDKCERTSEL_BUILD
        // Use certificate selector if available
        rdkcertselectorStatus_t xcGetCertStatus;
        char *pCertURI = NULL;

        do
        {
            // Free previous iteration's allocations
            if(pCertURI != NULL)
            {
                free(pCertURI);
                pCertURI = NULL;
            }
            if(pPasswd != NULL)
            {
                free(pPasswd);
                pPasswd = NULL;
            }
            pCertFile = NULL;

            xcGetCertStatus = rdkcertselector_getCert(curlCertSelector, &pCertURI, &pPasswd);
            if(xcGetCertStatus != certselectorOk)
            {
                T2Error("%s, T2:Failed to retrieve the certificate.\n", __func__);
                free(response.data);
                release_pool_handle(idx);
                return T2ERROR_FAILURE;
            }
            else
            {
                // skip past file scheme in URI
                pCertFile = pCertURI;
                if ( strncmp( pCertFile, FILESCHEME, sizeof(FILESCHEME) - 1 ) == 0 )
                {
                    pCertFile += (sizeof(FILESCHEME) - 1);
                }

                // Configure mTLS certificates
                CURL_SETOPT_CHECK_STR(easy, CURLOPT_SSLCERTTYPE, "P12");
                CURL_SETOPT_CHECK_STR(easy, CURLOPT_SSLCERT, pCertFile);
                CURL_SETOPT_CHECK_STR(easy, CURLOPT_KEYPASSWD, pPasswd);
                CURL_SETOPT_CHECK(easy, CURLOPT_SSL_VERIFYPEER, 1L);

                // Execute the request directly
                curl_code = curl_easy_perform(easy);

                long http_code;
                curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &http_code);

                if(curl_code != CURLE_OK || http_code != 200)
                {
                    T2Error("%s: Failed to establish connection using xPKI certificate: %s, Curl failed : %d\n", __func__, pCertFile, curl_code);
                }
                else
                {
                    T2Info("%s: Using xpki Certs connection certname : %s \n", __FUNCTION__, pCertFile);
                }
            }
        }
        while(rdkcertselector_setCurlStatus(curlCertSelector, curl_code, (const char*)url) == TRY_ANOTHER);
#else
        // Fallback to getMtlsCerts if certificate selector not available
        if(T2ERROR_SUCCESS == getMtlsCerts(&pCertFile, &pPasswd))
        {
            // Configure mTLS certificates
            CURL_SETOPT_CHECK_STR(easy, CURLOPT_SSLCERTTYPE, "P12");
            CURL_SETOPT_CHECK_STR(easy, CURLOPT_SSLCERT, pCertFile);
            CURL_SETOPT_CHECK_STR(easy, CURLOPT_KEYPASSWD, pPasswd);
            CURL_SETOPT_CHECK(easy, CURLOPT_SSL_VERIFYPEER, 1L);

            // Execute the request
            curl_code = curl_easy_perform(easy);
        }
        else
        {
            T2Error("mTLS_get failure\n");
            free(response.data);
            release_pool_handle(idx);
            return T2ERROR_FAILURE;
        }
#endif
    }
    else
    {
        // Execute without mTLS
        curl_code = curl_easy_perform(easy);
    }

    if (curl_code != CURLE_OK)
    {
        T2Error("curl_easy_perform failed: %s\n", curl_easy_strerror(curl_code));
        free(response.data);
#ifndef LIBRDKCERTSEL_BUILD
        if(NULL != pCertFile)
        {
            free(pCertFile);
        }
        if(NULL != pPasswd)
        {
            free(pPasswd);
        }
#endif
        release_pool_handle(idx);
        return T2ERROR_FAILURE;
    }

    long http_code;
    curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &http_code);
    T2Info("%s ; HTTP response code: %ld\n", __FUNCTION__, http_code);

    T2ERROR result = T2ERROR_FAILURE;
    if (http_code == 200)
    {
        result = T2ERROR_SUCCESS;
        T2Info("%s:%d, T2:Telemetry XCONF communication success\n", __func__, __LINE__);

        if (response.data)
        {
            T2Info("%s ; Response data size = %zu\n", __FUNCTION__, response.size);

            // Handle file output for GET requests
            if (enable_file_output)
            {
                int fd = open(HTTP_RESPONSE_FILE, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
                if (fd >= 0)
                {
                    FILE *httpOutput = fdopen(fd, "w+");
                    if (httpOutput)
                    {
                        T2Debug("Update config data in response file %s \n", HTTP_RESPONSE_FILE);
                        fputs(response.data, httpOutput);
                        fclose(httpOutput);
                    }
                    else
                    {
                        T2Error("Unable to associate stream with file descriptor for %s\n", HTTP_RESPONSE_FILE);
                        close(fd);
                    }
                }
                else
                {
                    T2Error("Unable to open %s file \n", HTTP_RESPONSE_FILE);
                }
            }

            // Copy response data
            if (response_data)
            {
                *response_data = NULL;
                if(response.size <= SIZE_MAX - 1)
                {
                    *response_data = (char*)malloc(response.size + 1);
                    if(*response_data)
                    {
                        memcpy(*response_data, response.data, response.size);
                        (*response_data)[response.size] = '\0';
                    }
                    else
                    {
                        T2Error("Failed to allocate memory for response data of size %zu\n", response.size);
                        result = T2ERROR_FAILURE;
                    }
                }
                else
                {
                    T2Error("Response size %zu exceeds maximum allowable size\n", response.size);
                    result = T2ERROR_FAILURE;
                }
            }
        }
        else
        {
            T2Error("%s ; No response data received\n", __FUNCTION__);
            result = T2ERROR_FAILURE;
        }
    }
    else
    {
        T2Error("%s:%d, T2:Telemetry XCONF communication Failed with http code : %ld Curl code : %d \n", __func__, __LINE__, http_code, curl_code);
        if(http_code == 404)
        {
            result = T2ERROR_PROFILE_NOT_SET;
        }
        else
        {
            result = T2ERROR_FAILURE;
        }
    }

    // Clean up local response buffer
    free(response.data);

    // Clean up certificates if not using certificate selector
#ifndef LIBRDKCERTSEL_BUILD
    if(NULL != pCertFile)
    {
        free(pCertFile);
    }
    if(NULL != pPasswd)
    {
        free(pPasswd);
    }
#endif

    release_pool_handle(idx);

    T2Info("%s ++out\n", __FUNCTION__);
    return result;
}

// POST API - Updated to use shared pool with waiting
T2ERROR http_pool_post(const char *url, const char *payload)
{
    T2Info("%s ++in\n", __FUNCTION__);

    if (!url || !payload)
    {
        T2Error("Invalid URL or payload parameter\n");
        return T2ERROR_FAILURE;
    }

    T2Info("%s ; POST url = %s\n", __FUNCTION__, url);

    // Acquire any available handle (with waiting)
    CURL *easy;
    int idx = -1;

    T2ERROR ret = acquire_pool_handle(&easy, &idx);
    if(ret != T2ERROR_SUCCESS)
    {
        T2Error("Failed to acquire pool handle\n");
        return ret;
    }

    T2Info("http_pool_post using handle %d\n", idx);

    // Clear any GET-specific settings from previous use
    CURL_SETOPT_CHECK(easy, CURLOPT_HTTPGET, 0L);
    CURL_SETOPT_CHECK(easy, CURLOPT_WRITEFUNCTION, NULL);
    CURL_SETOPT_CHECK(easy, CURLOPT_WRITEDATA, NULL);

    // Configure basic options for POST request
    CURL_SETOPT_CHECK_STR(easy, CURLOPT_URL, url);
    CURL_SETOPT_CHECK_STR(easy, CURLOPT_CUSTOMREQUEST, "POST");
    CURL_SETOPT_CHECK(easy, CURLOPT_HTTPHEADER, pool.post_headers);
    CURL_SETOPT_CHECK_STR(easy, CURLOPT_POSTFIELDS, payload);
    CURL_SETOPT_CHECK(easy, CURLOPT_POSTFIELDSIZE, strlen(payload));

#if defined(ENABLE_RDKB_SUPPORT) && !defined(RDKB_EXTENDER)
#if defined(WAN_FAILOVER_SUPPORTED) || defined(FEATURE_RDKB_CONFIGURABLE_WAN_INTERFACE)
    char *paramVal = NULL;
    char waninterface[256];
    memset(waninterface, 0, sizeof(waninterface));
    snprintf(waninterface, sizeof(waninterface), "%s", IFINTERFACE);

    if(T2ERROR_SUCCESS == getParameterValue(TR181_DEVICE_CURRENT_WAN_IFNAME, &paramVal))
    {
        if(strlen(paramVal) > 0)
        {
            memset(waninterface, 0, sizeof(waninterface));
            snprintf(waninterface, sizeof(waninterface), "%s", paramVal);
            T2Info("TR181_DEVICE_CURRENT_WAN_IFNAME -- %s\n", waninterface);
        }
        free(paramVal);
        paramVal = NULL;
    }
    else
    {
        T2Error("Failed to get Value for %s\n", TR181_DEVICE_CURRENT_WAN_IFNAME);
    }

    CURL_SETOPT_CHECK_STR(easy, CURLOPT_INTERFACE, waninterface);
    T2Info("TR181_DEVICE_CURRENT_WAN_IFNAME ---- %s\n", waninterface);
#else
    CURL_SETOPT_CHECK(easy, CURLOPT_INTERFACE, "erouter0");
#endif
#endif

    // Set up file output for POST requests
    int curl_output_fd = open("/tmp/curlOutput.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    FILE *fp = NULL;
    if (curl_output_fd >= 0)
    {
        fp = fdopen(curl_output_fd, "wb");
        if (fp != NULL)
        {
            CURL_SETOPT_CHECK(easy, CURLOPT_WRITEFUNCTION, writeToFile);
            CURL_SETOPT_CHECK(easy, CURLOPT_WRITEDATA, (void *)fp);
        }
        else
        {
            T2Error("fdopen failed for /tmp/curlOutput.txt\n");
            close(curl_output_fd);
        }
    }
    else
    {
        T2Error("Unable to open /tmp/curlOutput.txt for writing\n");
    }

    // Certificate handling - check if mTLS is enabled
    bool mtls_enable = isMtlsEnabled();
    char *pCertFile = NULL;
    char *pCertPC = NULL;
    CURLcode curl_code = CURLE_OK;

    if(mtls_enable == true)
    {
#ifdef LIBRDKCERTSEL_BUILD
        // Use certificate selector if available
        rdkcertselector_h thisCertSel = NULL;
        rdkcertselectorStatus_t curlGetCertStatus;
        char *pCertURI = NULL;
        char *pPasswd = NULL;
        bool state_red_enable = false;

#if defined(ENABLE_RED_RECOVERY_SUPPORT)
        state_red_enable = (access("/tmp/stateRedEnabled", F_OK) == 0);
        T2Info("%s: state_red_enable: %d\n", __func__, state_red_enable);
#endif
        if (state_red_enable)
        {
            thisCertSel = curlRcvryCertSelector;
        }
        else
        {
            thisCertSel = curlCertSelector;
        }

        do
        {
            // Free previous iteration's allocations
            if(pCertURI != NULL)
            {
                free(pCertURI);
                pCertURI = NULL;
            }
            if(pPasswd != NULL)
            {
                free(pPasswd);
                pPasswd = NULL;
            }
            pCertFile = NULL;

            curlGetCertStatus = rdkcertselector_getCert(thisCertSel, &pCertURI, &pCertPC);
            if(curlGetCertStatus != certselectorOk)
            {
                T2Error("%s, T2:Failed to retrieve the certificate.\n", __func__);
                if(fp)
                {
                    fclose(fp);
                }
                release_pool_handle(idx);
                return T2ERROR_FAILURE;
            }
            else
            {
                // skip past file scheme in URI
                pCertFile = pCertURI;
                if ( strncmp( pCertFile, FILESCHEME, sizeof(FILESCHEME) - 1 ) == 0 )
                {
                    pCertFile += (sizeof(FILESCHEME) - 1);
                }

                // Configure mTLS certificates
                CURL_SETOPT_CHECK_STR(easy, CURLOPT_SSLCERTTYPE, "P12");
                CURL_SETOPT_CHECK_STR(easy, CURLOPT_SSLCERT, pCertFile);
                CURL_SETOPT_CHECK_STR(easy, CURLOPT_KEYPASSWD, pCertPC);
                CURL_SETOPT_CHECK(easy, CURLOPT_SSL_VERIFYPEER, 1L);

                // Execute the request directly
                curl_code = curl_easy_perform(easy);

                long http_code;
                curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &http_code);

                if(curl_code != CURLE_OK || http_code != 200)
                {
                    T2Error("%s: Failed to establish connection using xPKI certificate: %s, curl failed: %d\n", __func__, pCertFile, curl_code);
                }
                else
                {
                    T2Info("%s: Using xpki Certs connection certname : %s \n", __FUNCTION__, pCertFile);
                    T2Info("Report Sent Successfully over HTTP : %ld\n", http_code);
                }
            }
        }
        while(rdkcertselector_setCurlStatus(thisCertSel, curl_code, (const char*)url) == TRY_ANOTHER);
#else
        // Fallback to getMtlsCerts if certificate selector not available
        if(T2ERROR_SUCCESS == getMtlsCerts(&pCertFile, &pCertPC))
        {
            // Configure mTLS certificates
            CURL_SETOPT_CHECK_STR(easy, CURLOPT_SSLCERTTYPE, "P12");
            CURL_SETOPT_CHECK_STR(easy, CURLOPT_SSLCERT, pCertFile);
            CURL_SETOPT_CHECK_STR(easy, CURLOPT_KEYPASSWD, pCertPC);
            CURL_SETOPT_CHECK(easy, CURLOPT_SSL_VERIFYPEER, 1L);

            // Execute the request
            curl_code = curl_easy_perform(easy);
        }
        else
        {
            T2Error("mTLS_get failure\n");
            if(fp)
            {
                fclose(fp);
            }
            release_pool_handle(idx);
            return T2ERROR_FAILURE;
        }
#endif
    }
    else
    {
        // Execute without mTLS
        curl_code = curl_easy_perform(easy);
    }

    // Close output file
    if(fp)
    {
        fclose(fp);
    }

    T2ERROR result = T2ERROR_FAILURE;
    if (curl_code == CURLE_OK)
    {
        long http_code;
        curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &http_code);
        T2Info("%s ; HTTP response code: %ld\n", __FUNCTION__, http_code);

        if (http_code == 200)
        {
            result = T2ERROR_SUCCESS;
            T2Info("%s ; Request successful\n", __FUNCTION__);
        }
        else
        {
            T2Error("%s ; HTTP request failed with code: %ld\n", __FUNCTION__, http_code);
        }
    }
    else
    {
        T2Error("curl_easy_perform failed: %s\n", curl_easy_strerror(curl_code));
    }

    // Clean up certificates if not using certificate selector
#ifndef LIBRDKCERTSEL_BUILD
    if(NULL != pCertFile)
    {
        free(pCertFile);
    }
    if(NULL != pCertPC)
    {
#ifdef LIBRDKCONFIG_BUILD
        size_t sKey = strlen(pCertPC);
        if (rdkconfig_free((unsigned char**)&pCertPC, sKey) == RDKCONFIG_FAIL)
        {
            T2Error("Failed to free password using rdkconfig\n");
        }
#else
        free(pCertPC);
#endif
    }
#endif

    release_pool_handle(idx);

    T2Info("%s ++out\n", __FUNCTION__);
    return result;
}

T2ERROR http_pool_cleanup(void)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if (!pool_initialized)
    {
        T2Info("Pool not initialized, nothing to cleanup\n");
        return T2ERROR_SUCCESS;
    }

    T2Info("Cleaning up http pool resources\n");
    // Signal any waiting threads to wake up
    pthread_mutex_lock(&pool.pool_mutex);
    pthread_cond_broadcast(&pool.handle_available_cond);
    pthread_mutex_unlock(&pool.pool_mutex);

#ifdef LIBRDKCERTSEL_BUILD
    // Cleanup certificate selectors to prevent memory leak
    curlCertSelectorFree();
#endif

    // Free all header lists before cleanup
    if(pool.post_headers)
    {
        curl_slist_free_all(pool.post_headers);
        pool.post_headers = NULL;
    }

    // Cleanup all easy handles
    for(int i = 0; i < MAX_POOL_SIZE; i++)
    {
        if(pool.easy_handles[i])
        {
            curl_easy_cleanup(pool.easy_handles[i]);
            pool.easy_handles[i] = NULL;
        }
    }

    // Destroy condition variable and mutex
    pthread_cond_destroy(&pool.handle_available_cond);
    pthread_mutex_destroy(&pool.pool_mutex);

    // Reset initialization flag
    pool_initialized = false;

    T2Debug("%s ++out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

// Write callback function for POST file output (from original sendReportOverHTTP)
size_t writeToFile(void *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t written = fwrite(ptr, size, nmemb, (FILE *) stream);
    return written;
}
