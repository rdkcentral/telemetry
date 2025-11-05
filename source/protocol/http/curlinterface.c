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
    char certstatus[1024];

} childResponse ;

#ifdef LIBRDKCERTSEL_BUILD
static rdkcertselector_h curlCertSelector = NULL;
static rdkcertselector_h curlRcvryCertSelector = NULL;
#endif

#if defined(ENABLE_RDKB_SUPPORT) && !defined(RDKB_EXTENDER)

#if defined(WAN_FAILOVER_SUPPORTED) || defined(FEATURE_RDKB_CONFIGURABLE_WAN_INTERFACE)
static char waninterface[256];
#endif
#endif
static pthread_once_t curlFileMutexOnce = PTHREAD_ONCE_INIT;
static pthread_mutex_t curlFileMutex;

typedef enum _ADDRESS_TYPE
{
    ADDR_UNKNOWN,
    ADDR_IPV4,
    ADDR_IPV6
} ADDRESS_TYPE;

static void sendOverHTTPInit()
{
    pthread_mutex_init(&curlFileMutex, NULL);
}


static size_t writeToFile(void *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t written = fwrite(ptr, size, nmemb, (FILE *) stream);
    return written;
}

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
    bool state_red_enable = false;
#if defined(ENABLE_RED_RECOVERY_SUPPORT)
    state_red_enable = isStateRedEnabled();
#endif
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

T2ERROR sendReportOverHTTP(char *httpUrl, char *payload, pid_t* outForkedPid)
{
    CURL *curl = NULL;
    FILE *fp = NULL;
    CURLcode code = CURLE_OK;
    T2ERROR ret = T2ERROR_FAILURE;
    childResponse childCurlResponse = {0};
    char certdetails[250] = {0};
    struct curl_slist *headerList = NULL;
    CURLcode curl_code = CURLE_OK;
#ifdef LIBRDKCERTSEL_BUILD
    rdkcertselector_h thisCertSel = NULL;
    rdkcertselectorStatus_t curlGetCertStatus;
    char *pCertURI = NULL;
    bool state_red_enable = false;
#endif
    char *pCertFile = NULL;
    char *pCertPC = NULL;
#ifdef LIBRDKCONFIG_BUILD
    size_t sKey = 0;
#endif
    long http_code;
    bool mtls_enable = false;
    pid_t childPid;
    int sharedPipeFds[2];

    T2Debug("%s ++in\n", __FUNCTION__);
    if(httpUrl == NULL || payload == NULL)
    {
        return ret;
    }
    if(pipe(sharedPipeFds) != 0)
    {
        T2Error("Failed to create pipe !!! exiting...\n");
        T2Debug("%s --out\n", __FUNCTION__);
        return ret;
    }
#ifdef LIBRDKCERTSEL_BUILD
    curlCertSelectorInit();
#if defined(ENABLE_RED_RECOVERY_SUPPORT)
    state_red_enable = isStateRedEnabled();
    T2Info("%s: state_red_enable: %d\n", __func__, state_red_enable );
#endif
    if (state_red_enable)
    {
        thisCertSel = curlRcvryCertSelector;
    }
    else
    {
        thisCertSel = curlCertSelector;
    }
#endif
#if defined(ENABLE_RDKB_SUPPORT) && !defined(RDKB_EXTENDER)

#if defined(WAN_FAILOVER_SUPPORTED) || defined(FEATURE_RDKB_CONFIGURABLE_WAN_INTERFACE)
    char *paramVal = NULL;
    memset(waninterface, 0, sizeof(waninterface));
    snprintf(waninterface, sizeof(waninterface), "%s", INTERFACE);

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
#endif
#endif
    mtls_enable = isMtlsEnabled();
#ifndef LIBRDKCERTSEL_BUILD
    if(mtls_enable == true && T2ERROR_SUCCESS != getMtlsCerts(&pCertFile, &pCertPC))
    {
        T2Error("mTLS_cert get failed\n");
        if(NULL != pCertFile)
        {
            free(pCertFile);
        }
        if(NULL != pCertPC)
        {
#ifdef LIBRDKCONFIG_BUILD
            sKey = strlen(pCertPC);
            if (rdkconfig_free((unsigned char**)&pCertPC, sKey) == RDKCONFIG_FAIL)
            {
                return T2ERROR_FAILURE;
            }
#else
            free(pCertPC);
#endif
        }
        return ret;
    }
#endif
    // Block the userdefined signal handlers before fork
    pthread_sigmask(SIG_BLOCK, &blocking_signal, NULL);
    if((childPid = fork()) < 0)
    {
        T2Error("Failed to fork !!! exiting...\n");
        // Unblock the userdefined signal handler
#ifndef LIBRDKCERTSEL_BUILD
        if(NULL != pCertFile)
        {
            free(pCertFile);
        }
        if(NULL != pCertPC)
        {
#ifdef LIBRDKCONFIG_BUILD
            sKey = strlen(pCertPC);
            if (rdkconfig_free((unsigned char**)&pCertPC, sKey) == RDKCONFIG_FAIL)
            {
                return T2ERROR_FAILURE;
            }
#else
            free(pCertPC);
#endif
        }
#endif
        pthread_sigmask(SIG_UNBLOCK, &blocking_signal, NULL);
        T2Debug("%s --out\n", __FUNCTION__);
        return T2ERROR_FAILURE;
    }

    /**
     * Openssl has growing RSS which gets cleaned up only with OPENSSL_cleanup .
     * This cleanup is not thread safe and classified as run once per application life cycle.
     * Forking the libcurl calls so that it executes and terminates to release memory per execution.
     */
    if(childPid == 0)
    {
        curl = curl_easy_init();
        if(curl)
        {
            childCurlResponse.curlStatus = true;
            if(setHeader(curl, httpUrl, &headerList, &childCurlResponse) != T2ERROR_SUCCESS)
            {
                curl_easy_cleanup(curl);
                goto child_cleanReturn;
            }
            if (setPayload(curl, payload, &childCurlResponse) != T2ERROR_SUCCESS)
            {
                curl_easy_cleanup(curl); // CID 189985: Resource leak
                goto child_cleanReturn;
            }
#ifdef LIBRDKCERTSEL_BUILD
            do
            {
                pCertFile = NULL;
                pCertPC = NULL;
                pCertURI = NULL;
                curlGetCertStatus = rdkcertselector_getCert(thisCertSel, &pCertURI, &pCertPC);
                if(curlGetCertStatus != certselectorOk)
                {
                    T2Error("%s, T2:Failed to retrieve the certificate.\n", __func__);
                    curlCertSelectorFree();
                    curl_easy_cleanup(curl);
                    goto child_cleanReturn;
                }
                else
                {
                    // skip past file scheme in URI
                    pCertFile = pCertURI;
                    if ( strncmp( pCertFile, FILESCHEME, sizeof(FILESCHEME) - 1 ) == 0 )
                    {
                        pCertFile += (sizeof(FILESCHEME) - 1);
                    }
#endif
                    if((mtls_enable == true) && (setMtlsHeaders(curl, pCertFile, pCertPC, &childCurlResponse) != T2ERROR_SUCCESS))
                    {
                        curl_easy_cleanup(curl); // CID 189985: Resource leak
                        goto child_cleanReturn;
                    }
                    pthread_once(&curlFileMutexOnce, sendOverHTTPInit);
                    pthread_mutex_lock(&curlFileMutex);

                    fp = fopen(CURL_OUTPUT_FILE, "wb");
                    if(fp)
                    {
                        /* CID 143029 Unchecked return value from library */
                        code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)fp);
                        if(code != CURLE_OK)
                        {
                            // This might not be working we need to review this
                            childCurlResponse.curlSetopCode = code;
                        }
                        curl_code = curl_easy_perform(curl);
                        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
                        if(curl_code != CURLE_OK || http_code != 200)
                        {
#ifdef LIBRDKCERTSEL_BUILD
                            T2Error("%s: Failed to establish connection using xPKI certificate 1: %s, curl failed: %d\n", __func__, pCertFile, curl_code);                            
                            snprintf(certdetails, sizeof(certdetails), "Failed to establish connection using %s, curl failed:%d\n", pCertFile, curl_code);
#endif
                            fprintf(stderr, "curl failed: %s\n", curl_easy_strerror(curl_code));
                            childCurlResponse.lineNumber = __LINE__;
                        }else{
                            T2Info("%s: Using xpki Certs connection certname 1: %s\n", __func__, pCertFile);
                            snprintf(certdetails, sizeof(certdetails), "Using xpki Certs connection certname %s\n", pCertFile);
                            childCurlResponse.lineNumber = __LINE__;
                        }
                        childCurlResponse.curlResponse = curl_code;
                        childCurlResponse.http_code = http_code;
                        strcat(childCurlResponse.certstatus , certdetails);
                        fclose(fp);
                    }
                    pthread_mutex_unlock(&curlFileMutex);
#ifdef LIBRDKCERTSEL_BUILD
                }
            }
            while(rdkcertselector_setCurlStatus(thisCertSel, curl_code, (const char*)httpUrl) == TRY_ANOTHER);
#endif
            curl_slist_free_all(headerList);
            curl_easy_cleanup(curl);
        }
        else
        {
            childCurlResponse.curlStatus = false;
        }

child_cleanReturn :
#ifndef LIBRDKCERTSEL_BUILD
        if(NULL != pCertFile)
        {
            free(pCertFile);
        }

        if(NULL != pCertPC)
        {
#ifdef LIBRDKCONFIG_BUILD
            sKey = strlen(pCertPC);
            if (rdkconfig_free((unsigned char**)&pCertPC, sKey) == RDKCONFIG_FAIL)
            {
                return T2ERROR_FAILURE;
            }
#else
            free(pCertPC);
#endif
        }
#endif
        close(sharedPipeFds[0]);
        if( -1 == write(sharedPipeFds[1], &childCurlResponse, sizeof(childResponse)))
        {
            fprintf(stderr, "unable to write to shared pipe from pid : %d \n", getpid());
            T2Error("unable to write \n");
        }
        close(sharedPipeFds[1]);
        exit(0);

    }
    else
    {
        T2ERROR ret = T2ERROR_FAILURE;
        if(outForkedPid)
        {
            *outForkedPid = childPid ;
        }
        // Use waitpid insted of wait we can have multiple child process
        waitpid(childPid, NULL, 0);
        // Unblock the userdefined signal handlers before fork
        pthread_sigmask(SIG_UNBLOCK, &blocking_signal, NULL);
        // Get the return status via IPC from child process
        if ( -1 == close(sharedPipeFds[1]))
        {
            T2Error("Failed in close \n");
        }
        if( -1 == read(sharedPipeFds[0], &childCurlResponse, sizeof(childResponse)))
        {
            T2Error("unable to read from the pipe \n");
        }
        if ( -1 == close(sharedPipeFds[0]))
        {
            T2Error("Failed in close the pipe\n");
        }
#ifndef LIBRDKCERTSEL_BUILD
        if(NULL != pCertFile)
        {
            free(pCertFile);
        }
        if(NULL != pCertPC)
        {
#ifdef LIBRDKCONFIG_BUILD
            sKey = strlen(pCertPC);
            if (rdkconfig_free((unsigned char**)&pCertPC, sKey) == RDKCONFIG_FAIL)
            {
                return T2ERROR_FAILURE;
            }
#else
            free(pCertPC);
#endif
        }
#endif
        T2Info("%s", childCurlResponse.certstatus);
        T2Info("The return status from the child with pid %d is CurlStatus : %d\n", childPid, childCurlResponse.curlStatus);
        //if(childCurlResponse.curlStatus == CURLE_OK) commenting this as we are observing childCurlResponse.curlStatus as 1, from line with CID 143029 Unchecked return value from library
        T2Info("The return status from the child with pid %d SetopCode: %s; ResponseCode : %s; HTTP_CODE : %ld; Line Number : %d \n", childPid, curl_easy_strerror(childCurlResponse.curlSetopCode), curl_easy_strerror(childCurlResponse.curlResponse), childCurlResponse.http_code, childCurlResponse.lineNumber);
        if (childCurlResponse.http_code == 200 || childCurlResponse.curlResponse == CURLE_OK)
        {
            ret = T2ERROR_SUCCESS;
            T2Info("Report Sent Successfully over HTTP : %ld\n", childCurlResponse.http_code);
        }
        T2Debug("%s --out\n", __FUNCTION__);
        return ret;
    }

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
