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
#define DEFAULT_POOL_SIZE 2
#define MAX_ALLOWED_POOL_SIZE 5  // Maximum allowed pool size
#define MIN_ALLOWED_POOL_SIZE 1  // Minimum allowed pool size
#define HTTP_RESPONSE_FILE "/tmp/httpOutput.txt"
static bool pool_initialized = false;

// Static initialization of mutex and condition variable to avoid race conditions
static pthread_mutex_t pool_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t pool_cond = PTHREAD_COND_INITIALIZER;

// Single pool entry structure
typedef struct
{
    CURL *easy_handle;               // Single CURL handle for this pool entry
    bool handle_available;           // Availability flag for this pool entry
#ifdef LIBRDKCERTSEL_BUILD
    rdkcertselector_h cert_selector;        // Single cert selector for this pool entry
    rdkcertselector_h rcvry_cert_selector;  // Single recovery cert selector for this pool entry
#endif
} http_pool_entry_t;

// Global pool variables
static http_pool_entry_t *pool_entries = NULL;  // Dynamic array of pool entries
static struct curl_slist *post_headers = NULL;  // Shared POST headers
static int pool_size = 0;                       // Number of pool entries

#ifdef LIBRDKCERTSEL_BUILD
#if defined(ENABLE_RED_RECOVERY_SUPPORT)
bool isStateRedEnabled(void)
{
    return access("/tmp/stateRedEnabled", F_OK) == 0;
}
#endif
#endif

// Helper function to read pool size from environment variable
static int get_configured_pool_size(void)
{
    int configured_size = DEFAULT_POOL_SIZE;

    // Check environment variable T2_CONNECTION_POOL_SIZE to have the size
    // of the connection pool based on the device type
    const char *env_size = getenv("T2_CONNECTION_POOL_SIZE");
    if (env_size != NULL)
    {
        int env_value = atoi(env_size);
        if (env_value >= MIN_ALLOWED_POOL_SIZE && env_value <= MAX_ALLOWED_POOL_SIZE)
        {
            configured_size = env_value;
            T2Info("Curl connection pool size set from environment: %d\n", configured_size);
        }
        else
        {
            T2Error("Invalid pool size in T2_CONNECTION_POOL_SIZE=%s, must be between %d and %d. Using default: %d\n",
                    env_size, MIN_ALLOWED_POOL_SIZE, MAX_ALLOWED_POOL_SIZE, DEFAULT_POOL_SIZE);
        }
    }
    else
    {
        T2Info("T2_CONNECTION_POOL_SIZE not set, using default pool size: %d\n", DEFAULT_POOL_SIZE);
    }

    return configured_size;
}

static size_t httpGetCallBack(void *responseBuffer, size_t len, size_t nmemb,
                              void *stream)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    size_t realsize = len * nmemb;
    curlResponseData* response = (curlResponseData*) stream;

    if (!response)
    {
        T2Error("httpGetCallBack: NULL stream parameter\n");
        return 0;
    }

    char *ptr = (char*) realloc(response->data,
                                response->size + realsize + 1);
    if (!ptr)
    {
        T2Error("%s:%u , T2:memory realloc failed\n", __func__, __LINE__);
        return 0;
    }
    response->data = ptr;
    memcpy(&(response->data[response->size]), responseBuffer, realsize);
    response->size += realsize;
    response->data[response->size] = 0;

    T2Debug("%s ++out\n", __FUNCTION__);
    return realsize;
}

static void cleanup_curl_handles(void)
{
    T2Debug("%s ++in\n", __FUNCTION__);

    if(post_headers)
    {
        curl_slist_free_all(post_headers);
        post_headers = NULL;
    }

#ifdef LIBRDKCERTSEL_BUILD
    // Clean up per-entry certificate selectors
    if(pool_entries)
    {
        for(int i = 0; i < pool_size; i++)
        {
            if(pool_entries[i].cert_selector)
            {
                T2Info("Freeing cert_selector for entry %d\n", i);
                rdkcertselector_free(&pool_entries[i].cert_selector);
                pool_entries[i].cert_selector = NULL;
            }
            if(pool_entries[i].rcvry_cert_selector)
            {
                T2Info("Freeing rcvry_cert_selector for entry %d\n", i);
                rdkcertselector_free(&pool_entries[i].rcvry_cert_selector);
                pool_entries[i].rcvry_cert_selector = NULL;
            }
        }
    }
#endif

    if(pool_entries)
    {
        for(int i = 0; i < pool_size; i++)
        {
            if(pool_entries[i].easy_handle)
            {
                curl_easy_cleanup(pool_entries[i].easy_handle);
                pool_entries[i].easy_handle = NULL;
            }
        }
        free(pool_entries);
        pool_entries = NULL;
    }

    pool_size = 0;

    T2Debug("%s ++out\n", __FUNCTION__);
}

T2ERROR init_connection_pool()
{
    T2Debug("%s ++in\n", __FUNCTION__);

    pthread_mutex_lock(&pool_mutex);

    if(pool_initialized)
    {
        T2Info("Connection pool already initialized with size %d\n", pool_size);
        pthread_mutex_unlock(&pool_mutex);
        return T2ERROR_SUCCESS;
    }

    //Get configured pool size from environment variable for low end devices
    pool_size = get_configured_pool_size();
    T2Debug("Initializing connection pool with size: %d\n", pool_size);
    pool_entries = (http_pool_entry_t *)calloc(pool_size, sizeof(http_pool_entry_t));

    if (!pool_entries)
    {
        T2Error("Failed to allocate memory for connection pool of size %d\n", pool_size);
        cleanup_curl_handles();
        pthread_mutex_unlock(&pool_mutex);
        return T2ERROR_FAILURE;
    }

    // Allocating easy handles and initialize pool entries
    for(int i = 0; i < pool_size; i++)
    {
        pool_entries[i].easy_handle = curl_easy_init();
        if(pool_entries[i].easy_handle == NULL)
        {
            T2Error("%s : Failed to initialize curl handle %d\n", __FUNCTION__, i);
            // Cleanup previously initialized handles to prevent memory leak
            cleanup_curl_handles();
            pthread_mutex_unlock(&pool_mutex);
            return T2ERROR_FAILURE;
        }
        pool_entries[i].handle_available = true;

        // Set common options for each handle that persist across requests
        CURL_SETOPT_CHECK(pool_entries[i].easy_handle, CURLOPT_TIMEOUT, 30L);
        CURL_SETOPT_CHECK(pool_entries[i].easy_handle, CURLOPT_CONNECTTIMEOUT, 10L);

        // TCP keepalive settings
        CURL_SETOPT_CHECK(pool_entries[i].easy_handle, CURLOPT_TCP_KEEPALIVE, 1L);
        CURL_SETOPT_CHECK(pool_entries[i].easy_handle, CURLOPT_TCP_KEEPIDLE, 50L);
        CURL_SETOPT_CHECK(pool_entries[i].easy_handle, CURLOPT_TCP_KEEPINTVL, 30L);

#ifdef CURLOPT_TCP_KEEPCNT
        CURL_SETOPT_CHECK(pool_entries[i].easy_handle, CURLOPT_TCP_KEEPCNT, 15L);
#endif

        // Connection reuse settings
        CURL_SETOPT_CHECK(pool_entries[i].easy_handle, CURLOPT_FORBID_REUSE, 0L);
        CURL_SETOPT_CHECK(pool_entries[i].easy_handle, CURLOPT_FRESH_CONNECT, 0L);

        // Socket options
        CURL_SETOPT_CHECK(pool_entries[i].easy_handle, CURLOPT_NOSIGNAL, 1L);

        // HTTP version and SSL settings
        CURL_SETOPT_CHECK(pool_entries[i].easy_handle, CURLOPT_PIPEWAIT, 0L);
        CURL_SETOPT_CHECK(pool_entries[i].easy_handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
        //SSL automatically negotiates the highest SSL/TLS version supported by both client and server
        CURL_SETOPT_CHECK(pool_entries[i].easy_handle, CURLOPT_SSLVERSION, CURL_SSLVERSION_DEFAULT);
        CURL_SETOPT_CHECK(pool_entries[i].easy_handle, CURLOPT_SSL_VERIFYPEER, 1L);

#ifdef LIBRDKCERTSEL_BUILD
        // Initialize certificate selectors for each easy handle
        bool state_red_enable = false;
#if defined(ENABLE_RED_RECOVERY_SUPPORT)
        state_red_enable = isStateRedEnabled();
#endif

        if (state_red_enable)
        {
            // Initialize recovery certificate selector while in state red
            pool_entries[i].rcvry_cert_selector = rdkcertselector_new(NULL, NULL, "RCVRY");
            if (pool_entries[i].rcvry_cert_selector == NULL)
            {
                T2Error("%s: Failed to initialize recovery cert selector for entry %d\n", __func__, i);
                cleanup_curl_handles();
                pthread_mutex_unlock(&pool_mutex);
                return T2ERROR_FAILURE;
            }
            else
            {
                T2Info("%s: Initialized recovery cert selector for entry %d\n", __func__, i);
            }
            pool_entries[i].cert_selector = NULL;
        }
        else
        {
            // Initialize normal certificate selector while not in red state
            pool_entries[i].cert_selector = rdkcertselector_new(NULL, NULL, "MTLS");
            if (pool_entries[i].cert_selector == NULL)
            {
                T2Error("%s: Failed to initialize cert selector for entry %d\n", __func__, i);
                cleanup_curl_handles();
                pthread_mutex_unlock(&pool_mutex);
                return T2ERROR_FAILURE;
            }
            else
            {
                T2Info("%s: Initialized cert selector for entry %d\n", __func__, i);
            }
            pool_entries[i].rcvry_cert_selector = NULL;
        }
#endif
    }

    post_headers = curl_slist_append(NULL, "Accept: application/json");
    if (post_headers == NULL)
    {
        T2Error("%s : Failed to append HTTP header: Accept: application/json\n", __FUNCTION__);
        cleanup_curl_handles();
        pthread_mutex_unlock(&pool_mutex);
        return T2ERROR_FAILURE;
    }

    post_headers = curl_slist_append(post_headers, "Content-type: application/json");
    if (post_headers == NULL)
    {
        T2Error("%s : Failed to append HTTP header: Content-type: application/json\n", __FUNCTION__);
        curl_slist_free_all(post_headers);
        post_headers = NULL;
        cleanup_curl_handles();
        pthread_mutex_unlock(&pool_mutex);
        return T2ERROR_FAILURE;
    }

    pool_initialized = true;
    pthread_mutex_unlock(&pool_mutex);
    T2Debug("%s ++out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}


static T2ERROR acquire_pool_handle(CURL **easy, int *idx)
{
    pthread_mutex_lock(&pool_mutex);
    if (!pool_initialized)
    {
        pthread_mutex_unlock(&pool_mutex);
        T2ERROR ret = init_connection_pool();
        if(ret != T2ERROR_SUCCESS)
        {
            T2Error("Failed to initialize connection pool\n");
            return ret;
        }
        pthread_mutex_lock(&pool_mutex);
    }

    // Waits until a handle becomes available or the pool is being cleaned up
    // exits if the pool has been cleaned up in shutdown sequence
    while(1)
    {
        if (!pool_initialized)
        {
            T2Info("Pool is being cleaned up, aborting handle acquisition\n");
            pthread_mutex_unlock(&pool_mutex);
            return T2ERROR_FAILURE;
        }

        for(int i = 0; i < pool_size; i++)
        {
            if(pool_entries[i].handle_available)
            {
                T2Info("Available handle = %d (pool size: %d)\n", i, pool_size);
                *idx = i;
                pool_entries[i].handle_available = false;
                *easy = pool_entries[i].easy_handle;

                pthread_mutex_unlock(&pool_mutex);
                return T2ERROR_SUCCESS;
            }
        }

        T2Info("No curl handle available (pool size: %d), waiting for one to become free.\n", pool_size);
        pthread_cond_wait(&pool_cond, &pool_mutex);
        // After waking up, loop back to check pool_initialized status
    }
}

static void release_pool_handle(int idx)
{
    pthread_mutex_lock(&pool_mutex);
    if (idx >= 0 && idx < pool_size)
    {
        pool_entries[idx].handle_available = true;
        //Signal waiting threads to check for handle availability
        pthread_cond_signal(&pool_cond);
        T2Info("Released curl handle = %d\n", idx);
    }
    else
    {
        T2Error("Invalid curl handle index = %d (pool size: %d)\n", idx, pool_size);
    }
    pthread_mutex_unlock(&pool_mutex);
}

#if defined(ENABLE_RDKB_SUPPORT) && !defined(RDKB_EXTENDER)
static void configure_wan_interface(CURL *easy)
{
    T2Debug("%s ++in\n", __FUNCTION__);
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
        }
        free(paramVal);
        paramVal = NULL;
    }
    else
    {
        T2Error("Failed to get Value for %s\n", TR181_DEVICE_CURRENT_WAN_IFNAME);
    }

    CURL_SETOPT_CHECK_STR(easy, CURLOPT_INTERFACE, waninterface);
    T2Debug("TR181_DEVICE_CURRENT_WAN_IFNAME ---- %s\n", waninterface);
#else
    CURL_SETOPT_CHECK(easy, CURLOPT_INTERFACE, "erouter0");
#endif
}
#endif


T2ERROR http_pool_get(const char *url, char **response_data, bool enable_file_output)
{
    T2Debug("%s ++in\n", __FUNCTION__);

    if (!url)
    {
        T2Error("Invalid URL parameter\n");
        return T2ERROR_FAILURE;
    }

    T2Info("GET url = %s\n", url);

    CURL *easy;
    int idx = -1;

    T2ERROR ret = acquire_pool_handle(&easy, &idx);
    if(ret != T2ERROR_SUCCESS)
    {
        T2Error("Failed to acquire pool handle\n");
        return ret;
    }

    // Clear any POST-specific settings so that the handle can be used for GET operations
    // curl_easy_perform ends up in crash when POST specific options are not cleared
    CURL_SETOPT_CHECK(easy, CURLOPT_CUSTOMREQUEST, NULL);
    CURL_SETOPT_CHECK(easy, CURLOPT_POSTFIELDS, NULL);
    CURL_SETOPT_CHECK(easy, CURLOPT_POSTFIELDSIZE, 0L);
    CURL_SETOPT_CHECK(easy, CURLOPT_HTTPHEADER, NULL);

    // Allocate response buffer locally for GET requests only
    curlResponseData* response = (curlResponseData *) malloc(sizeof(curlResponseData));
    if (!response)
    {
        T2Error("Failed to allocate response structure\n");
        release_pool_handle(idx);
        return T2ERROR_FAILURE;
    }

    response->data = (char*)malloc(1);
    if (!response->data)
    {
        T2Error("Failed to allocate response data buffer\n");
        free(response);
        release_pool_handle(idx);
        return T2ERROR_FAILURE;
    }

    response->data[0] = '\0';
    response->size = 0;

    // Configure request-specific options for GET
    CURL_SETOPT_CHECK_STR(easy, CURLOPT_URL, url);
    CURL_SETOPT_CHECK(easy, CURLOPT_HTTPGET, 1L);
    CURL_SETOPT_CHECK(easy, CURLOPT_WRITEFUNCTION, httpGetCallBack);
    CURL_SETOPT_CHECK(easy, CURLOPT_WRITEDATA, (void *) response);

#if defined(ENABLE_RDKB_SUPPORT) && !defined(RDKB_EXTENDER)
    configure_wan_interface(easy);
#endif
    bool mtls_enable = isMtlsEnabled();
    char *pCertFile = NULL;
    char *pCertPC = NULL;
#ifdef LIBRDKCERTSEL_BUILD
    char *pCertURI = NULL;
#endif
    CURLcode curl_code = CURLE_OK;

    if(mtls_enable == true)
    {
#ifdef LIBRDKCERTSEL_BUILD
        rdkcertselector_h handleCertSelector = pool_entries[idx].cert_selector;
        rdkcertselectorStatus_t xcGetCertStatus;

        T2Debug("%s: Using cert selector for entry %d\n", __func__, idx);
        do
        {
            pCertURI = NULL;
            pCertPC = NULL;
            pCertFile = NULL;

            xcGetCertStatus = rdkcertselector_getCert(handleCertSelector, &pCertURI, &pCertPC);
            if(xcGetCertStatus != certselectorOk)
            {
                T2Error("%s, T2:Failed to retrieve the certificate for entry %d.\n", __func__, idx);
                if (response)
                {
                    if (response->data)
                    {
                        free(response->data);
                    }
                    free(response);
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

                // Execute the request and retry incase of certificate related error
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
        while(rdkcertselector_setCurlStatus(handleCertSelector, curl_code, (const char*)url) == TRY_ANOTHER);
#else
        // Fallback to getMtlsCerts if certificate selector not available
        if(T2ERROR_SUCCESS == getMtlsCerts(&pCertFile, &pCertPC))
        {
            CURL_SETOPT_CHECK_STR(easy, CURLOPT_SSLCERTTYPE, "P12");
            CURL_SETOPT_CHECK_STR(easy, CURLOPT_SSLCERT, pCertFile);
            CURL_SETOPT_CHECK_STR(easy, CURLOPT_KEYPASSWD, pCertPC);
            CURL_SETOPT_CHECK(easy, CURLOPT_SSL_VERIFYPEER, 1L);

            curl_code = curl_easy_perform(easy);
        }
        else
        {
            T2Error("mTLS_get failure\n");
            if (response)
            {
                if (response->data)
                {
                    free(response->data);
                }
                free(response);
            }
            release_pool_handle(idx);
            return T2ERROR_FAILURE;
        }
#endif
        //reset the security options set to curl handle
        CURL_SETOPT_CHECK(easy, CURLOPT_SSLCERT, NULL);
        CURL_SETOPT_CHECK(easy, CURLOPT_KEYPASSWD, NULL);
    }
    else
    {
        T2Info("Attempting curl communication without mtls\n");
        curl_code = curl_easy_perform(easy);
    }

    if (curl_code != CURLE_OK)
    {
        T2Error("curl_easy_perform failed: %s\n", curl_easy_strerror(curl_code));
        if (response)
        {
            if (response->data)
            {
                free(response->data);
            }
            free(response);
        }
#ifndef LIBRDKCERTSEL_BUILD
        if(NULL != pCertFile)
        {
            free(pCertFile);
        }
        if(NULL != pCertPC)
        {
            free(pCertPC);
        }
#endif
        release_pool_handle(idx);
        return T2ERROR_FAILURE;
    }

    long http_code;
    curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &http_code);

    T2ERROR result = T2ERROR_FAILURE;
    if (http_code == 200)
    {
        result = T2ERROR_SUCCESS;
        T2Info("%s:%d, T2:Telemetry XCONF communication success\n", __func__, __LINE__);

        if (response->data)
        {
            T2Debug("%s ; Response data size = %zu\n", __FUNCTION__, response->size);

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
                        fputs(response->data, httpOutput);
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

            // Copy response data to return to the caller function
            if (response_data)
            {
                *response_data = NULL;
                if(response->size <= SIZE_MAX - 1)
                {
                    *response_data = (char*)malloc(response->size + 1);
                    if(*response_data)
                    {
                        memcpy(*response_data, response->data, response->size);
                        (*response_data)[response->size] = '\0';
                    }
                    else
                    {
                        T2Error("Failed to allocate memory for response data of size %zu\n", response->size);
                        result = T2ERROR_FAILURE;
                    }
                }
                else
                {
                    T2Error("Response size %zu exceeds maximum allowable size\n", response->size);
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
    if (response)
    {
        if (response->data)
        {
            free(response->data);
        }
        free(response);
    }

    // Clean up certificates - only for non-LIBRDKCERTSEL_BUILD path
    // In case of CertSelector, CertSelector will take care of freeing the certicates
#ifndef LIBRDKCERTSEL_BUILD
    if(pCertFile != NULL)
    {
        free(pCertFile);
        pCertFile = NULL;
    }
    if(pCertPC != NULL)
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
        pCertPC = NULL;
    }
#endif

    // Important Note: When using LIBRDKCERTSEL_BUILD, pCertURI and pCertPC are owned by the
    // cert selector object and are freed when rdkcertselector_free() is called

    release_pool_handle(idx);

    T2Debug("%s ++out\n", __FUNCTION__);
    return result;
}

T2ERROR http_pool_post(const char *url, const char *payload)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if (!url || !payload)
    {
        T2Error("Invalid URL or payload parameter\n");
        return T2ERROR_FAILURE;
    }

    T2Info("POST url = %s\n", url);

    // Acquire any available handle (with waiting)
    CURL *easy;
    int idx = -1;

    T2ERROR ret = acquire_pool_handle(&easy, &idx);
    if(ret != T2ERROR_SUCCESS)
    {
        T2Error("Failed to acquire pool handle\n");
        return ret;
    }

    // Clear any GET-specific settings from previous use
    CURL_SETOPT_CHECK(easy, CURLOPT_HTTPGET, 0L);
    CURL_SETOPT_CHECK(easy, CURLOPT_WRITEFUNCTION, NULL);
    CURL_SETOPT_CHECK(easy, CURLOPT_WRITEDATA, NULL);

    // Configure request-specific options for POST
    CURL_SETOPT_CHECK_STR(easy, CURLOPT_URL, url);
    CURL_SETOPT_CHECK_STR(easy, CURLOPT_CUSTOMREQUEST, "POST");
    CURL_SETOPT_CHECK(easy, CURLOPT_HTTPHEADER, post_headers);
    CURL_SETOPT_CHECK_STR(easy, CURLOPT_POSTFIELDS, payload);
    CURL_SETOPT_CHECK(easy, CURLOPT_POSTFIELDSIZE, strlen(payload));

#if defined(ENABLE_RDKB_SUPPORT) && !defined(RDKB_EXTENDER)
    configure_wan_interface(easy);
#endif

    // curl_easy_perform crashes without file output configuration. This can be removed once the root cause of the crash is identified and fixed.
    // For now, we will set up file output for POST requests to ensure stability.
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
#ifdef LIBRDKCERTSEL_BUILD
    char *pCertURI = NULL;
#endif
    CURLcode curl_code = CURLE_OK;

    if(mtls_enable == true)
    {
#ifdef LIBRDKCERTSEL_BUILD
        rdkcertselector_h thisCertSel = NULL;
        rdkcertselectorStatus_t curlGetCertStatus;
        bool state_red_enable = false;

#if defined(ENABLE_RED_RECOVERY_SUPPORT)
        state_red_enable = (access("/tmp/stateRedEnabled", F_OK) == 0);
        T2Info("%s: state_red_enable: %d\n", __func__, state_red_enable);
#endif

        // When status of state red changes, switch cert selectors accordingly
        if (state_red_enable)
        {
            // RED recovery mode - use recovery cert selector
            if (pool_entries[idx].rcvry_cert_selector == NULL)
            {
                T2Info("Initializing recovery cert selector for this easy handle\n", __func__);
                pool_entries[idx].rcvry_cert_selector = rdkcertselector_new(NULL, NULL, "RCVRY");
                if (pool_entries[idx].rcvry_cert_selector == NULL)
                {
                    T2Error("Failed to initialize recovery cert selector for this easy handle\n");
                    release_pool_handle(idx);
                    if(fp)
                    {
                        fclose(fp);
                    }
                    return T2ERROR_FAILURE;
                }
            }
            thisCertSel = pool_entries[idx].rcvry_cert_selector;
            T2Info("%s: Using recovery cert selector for entry %d\n", __func__, idx);

            // Clean up normal cert selector if it exists
            if (pool_entries[idx].cert_selector != NULL)
            {
                T2Info("%s: State transition detected, freeing normal cert selector for entry %d\n", __func__, idx);
                rdkcertselector_free(&pool_entries[idx].cert_selector);
                pool_entries[idx].cert_selector = NULL;
            }
        }
        else
        {
            // Device not in red recovery - use regular cert selector
            if (pool_entries[idx].cert_selector == NULL)
            {
                T2Info("%s: Initializing normal cert selector for entry %d\n", __func__, idx);
                pool_entries[idx].cert_selector = rdkcertselector_new(NULL, NULL, "MTLS");
                if (pool_entries[idx].cert_selector == NULL)
                {
                    T2Error("%s: Failed to initialize cert selector for entry %d\n", __func__, idx);
                    if(fp)
                    {
                        fclose(fp);
                    }
                    release_pool_handle(idx);
                    return T2ERROR_FAILURE;
                }
            }
            thisCertSel = pool_entries[idx].cert_selector;

            // Clean up recovery cert selector if it exists (state transition)
            if (pool_entries[idx].rcvry_cert_selector != NULL)
            {
                T2Info("%s: State transition detected, freeing recovery cert selector for entry %d\n", __func__, idx);
                rdkcertselector_free(&pool_entries[idx].rcvry_cert_selector);
                pool_entries[idx].rcvry_cert_selector = NULL;
            }
        }

        do
        {
            pCertURI = NULL;
            pCertPC = NULL;
            pCertFile = NULL;

            curlGetCertStatus = rdkcertselector_getCert(thisCertSel, &pCertURI, &pCertPC);
            if(curlGetCertStatus != certselectorOk)
            {
                T2Error("%s, T2:Failed to retrieve the certificate for entry %d.\n", __func__, idx);
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

                curl_code = curl_easy_perform(easy);

                long http_code;
                curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &http_code);

                if(curl_code != CURLE_OK || http_code != 200)
                {
                    T2Error("%s: Failed to establish connection using xPKI certificate: %s, curl failed: %d (entry %d)\n", __func__, pCertFile, curl_code, idx);
                }
                else
                {
                    T2Info("%s: Using xpki Certs connection certname : %s (entry %d)\n", __FUNCTION__, pCertFile, idx);
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
        CURL_SETOPT_CHECK(easy, CURLOPT_SSLCERT, NULL);
        CURL_SETOPT_CHECK(easy, CURLOPT_KEYPASSWD, NULL);
    }
    else
    {
        T2Info("Attempting curl communication without mtls\n");
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
        T2Debug("HTTP response code: %ld\n", http_code);

        if (http_code == 200)
        {
            result = T2ERROR_SUCCESS;
            T2Info("Report Sent Successfully over HTTP : %ld\n", http_code);
        }
        else
        {
            T2Error("HTTP request failed with code: %ld\n", http_code);
        }
    }
    else
    {
        T2Error("curl_easy_perform failed: %s\n", curl_easy_strerror(curl_code));
    }

    // Clean up certificates - only for non-LIBRDKCERTSEL_BUILD path
#ifndef LIBRDKCERTSEL_BUILD
    if(pCertFile != NULL)
    {
        free(pCertFile);
        pCertFile = NULL;
    }
    if(pCertPC != NULL)
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
        pCertPC = NULL;
    }
#endif
    // Note: When using LIBRDKCERTSEL_BUILD, pCertURI and pCertPC are owned by the
    // cert selector object and are freed when rdkcertselector_free() is called
    release_pool_handle(idx);

    T2Debug("%s ++out\n", __FUNCTION__);
    return result;
}

T2ERROR http_pool_cleanup(void)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    pthread_mutex_lock(&pool_mutex);
    if (!pool_initialized)
    {
        T2Info("Pool not initialized, nothing to cleanup\n");
        pthread_mutex_unlock(&pool_mutex);
        return T2ERROR_SUCCESS;
    }

    T2Info("Cleaning up http pool resources\n");

    // Reset initialization flag
    pool_initialized = false;

    // Signal any waiting threads to wake up
    pthread_cond_broadcast(&pool_cond);
    pthread_mutex_unlock(&pool_mutex);

    // Cleanup all curl handles and per-entry certificate selectors
    cleanup_curl_handles();

#ifndef LIBRDKCERTSEL_BUILD
    uninitMtls();
#endif

    T2Debug("%s ++out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

// Write callback function for POST file output (from original sendReportOverHTTP)
size_t writeToFile(void *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t written = fwrite(ptr, size, nmemb, (FILE *) stream);
    return written;
}
