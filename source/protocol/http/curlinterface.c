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
#include "../../xconf-client/multicurlinterface.h"
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

} childResponse ;

typedef enum _ADDRESS_TYPE
{
    ADDR_UNKNOWN,
    ADDR_IPV4,
    ADDR_IPV6
} ADDRESS_TYPE;

T2ERROR sendReportOverHTTP(char *httpUrl, char *payload)
{
    T2ERROR ret = T2ERROR_FAILURE;

    T2Debug("%s ++in\n", __FUNCTION__);
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

    T2Debug("%s --out\n", __FUNCTION__);
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
        if(T2ERROR_FAILURE == sendReportOverHTTP(httpUrl, payload))
        {
            T2Error("Failed to send cached report, left with %lu reports in cache \n", (unsigned long)Vector_Size(reportList));
            return T2ERROR_FAILURE;
        }
        Vector_RemoveItem(reportList, payload, NULL);
        free(payload);
    }
    return T2ERROR_SUCCESS;
}

#ifdef GTEST_ENABLE
typedef size_t (*WriteToFileFunc)(void *, size_t, size_t, void *);
typedef T2ERROR (*SetHeaderFunc)(CURL *, const char *, struct curl_slist **, childResponse *);
typedef T2ERROR (*SetMtlsHeadersFunc)(CURL *, const char *, const char *, childResponse *);
typedef T2ERROR (*SetPayloadFunc)(CURL *, const char *, childResponse *);
WriteToFileFunc getWriteToFileCallback()
{
    return writeToFile;
}

SetHeaderFunc getSetHeaderCallback(void)
{
    return setHeader;
}

SetMtlsHeadersFunc getSetMtlsHeadersCallback(void)
{
    return setMtlsHeaders;
}
SetPayloadFunc getSetPayloadCallback(void)
{
    return setPayload;
}
#endif
