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

#include "curlinterface.h"
#include "reportprofiles.h"
#include "t2MtlsUtils.h"
#include "t2log_wrapper.h"
#include "busInterface.h"
#include "../xconf-client/multicurlinterface.h"
#ifdef LIBRDKCERTSEL_BUILD
#include "rdkcertselector.h"
#define FILESCHEME "file://"
#endif
#ifdef LIBRDKCONFIG_BUILD
#include "rdkconfig.h"
#endif
#ifdef GTEST_ENABLE
#define curl_easy_setopt curl_easy_setopt_mock
#define curl_easy_getinfo curl_easy_getinfo_mock
#endif
extern sigset_t blocking_signal;

typedef struct
{
    bool curlStatus;
    CURLcode curlResponse;
    CURLcode curlSetopCode;
    long http_code;
    int lineNumber;

} childResponse ;

#ifdef LIBRDKCERTSEL_BUILD
#if 0
static rdkcertselector_h curlCertSelector = NULL;
static rdkcertselector_h curlRcvryCertSelector = NULL;
#endif
#endif

#if defined(ENABLE_RDKB_SUPPORT) && !defined(RDKB_EXTENDER)

#if defined(WAN_FAILOVER_SUPPORTED) || defined(FEATURE_RDKB_CONFIGURABLE_WAN_INTERFACE)
static char waninterface[256];
#endif
#endif
//static pthread_once_t curlFileMutexOnce = PTHREAD_ONCE_INIT;
//static pthread_mutex_t curlFileMutex;

typedef enum _ADDRESS_TYPE
{
    ADDR_UNKNOWN,
    ADDR_IPV4,
    ADDR_IPV6
} ADDRESS_TYPE;

#if 0
static void sendOverHTTPInit()
{
    pthread_mutex_init(&curlFileMutex, NULL);
}



static size_t writeToFile(void *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t written = fwrite(ptr, size, nmemb, (FILE *) stream);
    return written;
}
#endif

#if 0
static T2ERROR setHeader(CURL *curl, const char* destURL, struct curl_slist **headerList, childResponse *childCurlResponse)
{

    //T2Debug("%s ++in\n", __FUNCTION__);
    if(curl == NULL || destURL == NULL)
    {
        childCurlResponse->lineNumber = __LINE__;
        return T2ERROR_FAILURE;
    }

    //T2Debug("%s DEST URL %s \n", __FUNCTION__, destURL);
    CURLcode code = CURLE_OK;
    code = curl_easy_setopt(curl, CURLOPT_URL, destURL);
    if(code != CURLE_OK)
    {
        childCurlResponse->curlSetopCode = code;
        childCurlResponse->lineNumber = __LINE__;
        return T2ERROR_FAILURE;
    }
    code = curl_easy_setopt(curl, CURLOPT_SSLVERSION, TLSVERSION);
    if(code != CURLE_OK)
    {
        childCurlResponse->curlSetopCode = code;
        childCurlResponse->lineNumber = __LINE__;
        return T2ERROR_FAILURE;
    }
    code = curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, HTTP_METHOD);
    if(code != CURLE_OK)
    {
        childCurlResponse->curlSetopCode = code;
        childCurlResponse->lineNumber = __LINE__;
        return T2ERROR_FAILURE;
    }
    code = curl_easy_setopt(curl, CURLOPT_TIMEOUT, TIMEOUT);
    if(code != CURLE_OK)
    {
        childCurlResponse->curlSetopCode = code;
        childCurlResponse->lineNumber = __LINE__;
        return T2ERROR_FAILURE;
    }

#if defined(ENABLE_RDKB_SUPPORT) && !defined(RDKB_EXTENDER)

#if defined(WAN_FAILOVER_SUPPORTED) || defined(FEATURE_RDKB_CONFIGURABLE_WAN_INTERFACE)
    code = curl_easy_setopt(curl, CURLOPT_INTERFACE, waninterface);
    if(code != CURLE_OK)
    {
        childCurlResponse->curlSetopCode = code;
        childCurlResponse->lineNumber = __LINE__;
        return T2ERROR_FAILURE;
    }
#else
    /* CID 125287: Unchecked return value from library */
    code = curl_easy_setopt(curl, CURLOPT_INTERFACE, INTERFACE);
    if(code != CURLE_OK)
    {
        childCurlResponse->curlSetopCode = code;
        childCurlResponse->lineNumber = __LINE__;
        return T2ERROR_FAILURE;
    }

#endif
#endif
    *headerList = curl_slist_append(NULL, "Accept: application/json");
    curl_slist_append(*headerList, "Content-type: application/json");

    code = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, *headerList);
    if(code != CURLE_OK)
    {
        childCurlResponse->curlSetopCode = code;
        childCurlResponse->lineNumber = __LINE__;
        return T2ERROR_FAILURE;
    }

    code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToFile);
    if(code != CURLE_OK)
    {
        childCurlResponse->curlSetopCode = code;
        childCurlResponse->lineNumber = __LINE__;
        return T2ERROR_FAILURE;
    }
    childCurlResponse->curlSetopCode = code;
    childCurlResponse->lineNumber = __LINE__;
    //T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

static T2ERROR setMtlsHeaders(CURL *curl, const char* certFile, const char* pPasswd, childResponse *childCurlResponse)
{
    if(curl == NULL || certFile == NULL || pPasswd == NULL)
    {
        childCurlResponse->lineNumber = __LINE__;
        return T2ERROR_FAILURE;
    }
    CURLcode code = CURLE_OK;
#ifndef LIBRDKCERTSEL_BUILD
    code = curl_easy_setopt(curl, CURLOPT_SSLENGINE_DEFAULT, 1L);
    if(code != CURLE_OK)
    {
        childCurlResponse->curlSetopCode = code;
        childCurlResponse->lineNumber = __LINE__;
        return T2ERROR_FAILURE;
    }
#endif
    code = curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "P12");
    if(code != CURLE_OK)
    {
        childCurlResponse->curlSetopCode = code;
        childCurlResponse->lineNumber = __LINE__;
        return T2ERROR_FAILURE;
    }
    /* set the cert for client authentication */
    code = curl_easy_setopt(curl, CURLOPT_SSLCERT, certFile);
    if(code != CURLE_OK)
    {
        childCurlResponse->curlSetopCode = code;
        childCurlResponse->lineNumber = __LINE__;
        return T2ERROR_FAILURE;
    }
    code = curl_easy_setopt(curl, CURLOPT_KEYPASSWD, pPasswd);
    if(code != CURLE_OK)
    {
        childCurlResponse->curlSetopCode = code;
        childCurlResponse->lineNumber = __LINE__;
        return T2ERROR_FAILURE;
    }
    /* disconnect if we cannot authenticate */
    code = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    if(code != CURLE_OK)
    {
        childCurlResponse->curlSetopCode = code;
        childCurlResponse->lineNumber = __LINE__;
        return T2ERROR_FAILURE;
    }
    childCurlResponse->curlSetopCode = code;
    childCurlResponse->lineNumber = __LINE__;
    return T2ERROR_SUCCESS;
}

static T2ERROR setPayload(CURL *curl, const char* payload, childResponse *childCurlResponse)
{
    if(curl == NULL || payload == NULL)
    {
        childCurlResponse->lineNumber = __LINE__;
        return T2ERROR_FAILURE;
    }
    CURLcode code = CURLE_OK ;
    code = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
    if(code != CURLE_OK)
    {
        childCurlResponse->curlSetopCode = code;
        childCurlResponse->lineNumber = __LINE__;
        return T2ERROR_FAILURE;
    }
    code = curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(payload));
    if(code != CURLE_OK)
    {
        childCurlResponse->curlSetopCode = code;
        childCurlResponse->lineNumber = __LINE__;
        return T2ERROR_FAILURE;
    }
    childCurlResponse->curlSetopCode = code;
    childCurlResponse->lineNumber = __LINE__;
    return T2ERROR_SUCCESS;
}
#ifdef LIBRDKCERTSEL_BUILD
bool isStateRedEnabled(void)
{
    return access("/tmp/stateRedEnabled", F_OK) == 0;
}
void curlCertSelectorFree()
{
    rdkcertselector_free(&curlCertSelector);
    rdkcertselector_free(&curlRcvryCertSelector);
    if(curlCertSelector == NULL || curlRcvryCertSelector == NULL)
    {
        T2Info("%s, T2:Cert selector memory free\n", __func__);
    }
    else
    {
        T2Info("%s, T2:Cert selector memory free failed\n", __func__);
    }
}
static void curlCertSelectorInit()
{
    bool state_red_enable = isStateRedEnabled();
    if (state_red_enable && curlRcvryCertSelector == NULL )
    {
        curlRcvryCertSelector = rdkcertselector_new( NULL, NULL, "RCVRY" );
        if (curlRcvryCertSelector == NULL)
        {
            T2Error("%s, T2:statered Cert selector initialization failed\n", __func__);
        }
        else
        {
            T2Info("%s, T2:statered Cert selector initialization successfully\n", __func__);
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

#endif

T2ERROR sendReportOverHTTP(char *httpUrl, char *payload, pid_t* outForkedPid)
{
    T2ERROR ret = T2ERROR_FAILURE;

    T2Info("%s ++in\n", __FUNCTION__);
    if(httpUrl == NULL || payload == NULL)
    {
        return ret;
    }

    // Use new dedicated POST API
    ret = http_pool_post(httpUrl, payload);

    if(ret == T2ERROR_SUCCESS)
    {
        T2Info("Report Sent Successfully over HTTP using connection pool\n");
    }
    else
    {
        T2Error("Failed to send report using connection pool\n");
    }

    // Set outForkedPid to 0 since we're not forking anymore
    if(outForkedPid)
    {
        *outForkedPid = 0;
    }

    T2Info("%s --out\n", __FUNCTION__);
    return ret;
}

T2ERROR sendCachedReportsOverHTTP(char *httpUrl, Vector *reportList)
{
    if(httpUrl == NULL || reportList == NULL)
    {
        return T2ERROR_FAILURE;
    }
    while(Vector_Size(reportList) > 0)
    {
        char* payload = (char *)Vector_At(reportList, 0);
        if(T2ERROR_FAILURE == sendReportOverHTTP(httpUrl, payload, NULL))
        {
            T2Error("Failed to send cached report, left with %lu reports in cache \n", (unsigned long)Vector_Size(reportList));
            return T2ERROR_FAILURE;
        }
        Vector_RemoveItem(reportList, payload, NULL);
        free(payload);
    }
    return T2ERROR_SUCCESS;
}
