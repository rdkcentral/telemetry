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

#include "t2MtlsUtils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef LIBSYSWRAPPER_BUILD
#include <secure_wrapper.h>
#endif
#include <stdbool.h>

#include "t2log_wrapper.h"
#include "t2common.h"
#ifdef LIBRDKCONFIG_BUILD
#include "rdkconfig.h"
#endif


#if !defined(ENABLE_RDKC_SUPPORT)
static const char* staticMtlsCert = "/etc/ssl/certs/staticXpkiCrt.pk12";
#ifdef LIBRDKCONFIG_BUILD
static const char* staticMtlsDestFile = "/tmp/.cfgStaticxpki";
static const char* dynamicMtlsDestFile = "/tmp/.cfgDynamicxpki";
#endif

static bool UsedynamicMtlsCert2 = false;
#if defined(ENABLE_RDKB_SUPPORT)
static const char* dynamicMtlsCert = "/nvram/certs/devicecert_1.pk12";
#else
static const char* dynamicMtlsCert = "/opt/certs/devicecert_1.pk12";
#endif

#else
static char* staticMtlsCert = "";
static char* staticPassPhrase = "";
static char* dynamicMtlsCert = "";
static char* dynamicPassPhrase = "";
#endif

void initMtls()
{
    T2Debug("%s ++in\n", __FUNCTION__);
    // Prepare certs required for mTls commmunication
#if !defined (MTLS_FROM_ENV)
    char UseHWBasedCert[8] = { '\0' };
    bool ret = false;
    ret = getDevicePropertyData("UseSEBasedCert", UseHWBasedCert, sizeof(UseHWBasedCert));
    if (ret == true)
    {
        T2Info("UseSEBasedCert = %s\n", UseHWBasedCert);
        if((strncasecmp(UseHWBasedCert, "true", 4) == 0))
        {
            UsedynamicMtlsCert2 = true;
        }
    }
    else
    {
        T2Info("getDevicePropertyData() failed for UseSEBasedCert\n");
    }
#endif
    T2Debug("%s --out\n", __FUNCTION__);

}

void uninitMtls()
{
    T2Debug("%s ++in\n", __FUNCTION__);

    T2Debug("%s --out\n", __FUNCTION__);
}

/**
 * read system uptime
 */
double get_system_uptime()
{
    double uptime = 0.0;
    FILE* uptime_file = fopen("/proc/uptime", "r");
    if (uptime_file != NULL)
    {
        if (fscanf(uptime_file, "%lf", &uptime) == 1)
        {
            fclose(uptime_file);
            return uptime;
        }
        fclose(uptime_file);
    }
    return uptime;
}

/**
 * Retrieves the certs and keys associated to respective end points
 * Camera's today gets these values from environment variables.
 * So there are two variants of the same function depending on the logic used to retrieve values.
 */
#if defined (MTLS_FROM_ENV)
T2ERROR getMtlsCerts(char **certName, char **phrase)
{
    T2ERROR ret = T2ERROR_FAILURE;
    T2Debug("%s ++in\n", __FUNCTION__);

    char buf[124];
    memset(buf, 0, sizeof(buf));

    if(getenv("XPKI") != NULL)
    {
        dynamicMtlsCert = getenv("XPKI_CERT");
        if (dynamicMtlsCert != NULL)   // Dynamic cert
        {
            *certName = strdup(dynamicMtlsCert);
            T2Info("Using xpki Dynamic Certs connection certname: %s\n", dynamicMtlsCert);

            dynamicPassPhrase = getenv("XPKI_PASS");
            if (dynamicPassPhrase != NULL)
            {
                *phrase = strdup(dynamicPassPhrase);
                ret = T2ERROR_SUCCESS;
            }
        }
    }
    else if (getenv("STATICXPKI") != NULL)
    {
        staticMtlsCert = getenv("STATIC_XPKI_CERT");
        if (staticMtlsCert != NULL)   // Static cert
        {
            *certName = strdup(staticMtlsCert);
            T2Info("Using xpki Static Certs connection certname: %s\n", staticMtlsCert);

            staticPassPhrase = getenv("STATIC_XPKI_PASS");
            if(staticPassPhrase != NULL)
            {
                *phrase = strdup(staticPassPhrase);
                ret = T2ERROR_SUCCESS;
            }
        }
    }
    else
    {
        T2Error("Certs not found\n");
    }
    T2Debug("%s --out\n", __FUNCTION__);
    return ret;
}

#else
T2ERROR getMtlsCerts(char **certName, char **phrase)
{

    T2ERROR ret = T2ERROR_FAILURE;

    T2Debug("%s ++in\n", __FUNCTION__);

#ifdef LIBRDKCONFIG_BUILD
    uint8_t *MtlsBuf = NULL;
    size_t MtlsSize;
#endif


    if( certName == NULL || phrase == NULL )
    {
        T2Error("Input args are NULL \n");
        T2Debug("%s --out\n", __FUNCTION__);
        return ret;
    }

    if(access(dynamicMtlsCert, F_OK) != -1)   // Dynamic cert
    {
        *certName = strdup(dynamicMtlsCert);
        T2Info("Using xpki Dynamic Certs connection certname: %s\n", dynamicMtlsCert);
#ifdef LIBRDKCONFIG_BUILD
        if(rdkconfig_get(&MtlsBuf, &MtlsSize, dynamicMtlsDestFile) == RDKCONFIG_FAIL)
        {
            T2Error("%s,%d: Failed to extract cfgDynamicxpki cred\n", __FUNCTION__, __LINE__);
            return T2ERROR_FAILURE;
        }
        MtlsBuf[strcspn((char *)MtlsBuf, "\n")] = '\0';
        *phrase = (char *) MtlsBuf;
#else
        T2Info("rdkconfig is not available - %s will act as pass through call. \n", __FUNCTION__);
#endif

        ret = T2ERROR_SUCCESS;
    }
    else if(access(staticMtlsCert, F_OK) != -1)    // Static cert
    {
        T2Info("Using xpki Static Certs connection certname: %s\n", staticMtlsCert);
        T2Info("xPKIStaticCert: /etc/ssl/certs/staticDeviceCert.pk12 uptime %.2lf seconds,telemetry", get_system_uptime());
        *certName = strdup(staticMtlsCert);
#ifdef LIBRDKCONFIG_BUILD

        /* CID: 189984 Time of check time of use (TOCTOU) */
        if(rdkconfig_get(&MtlsBuf, &MtlsSize, staticMtlsDestFile) == RDKCONFIG_FAIL)
        {
            T2Error("%s,%d: Failed to extract cfgStaticxpki cred\n", __FUNCTION__, __LINE__);
            return T2ERROR_FAILURE;
        }

        MtlsBuf[strcspn((char *)MtlsBuf, "\n")] = '\0';
        *phrase = (char *)MtlsBuf;
#else
        T2Info("rdkconfig is not available - %s will act as pass through call. \n", __FUNCTION__);
#endif

        ret = T2ERROR_SUCCESS;
    }
    else
    {
        T2Error("Certs not found\n");
    }

    T2Debug("%s --out\n", __FUNCTION__);
    return ret;
}
#endif
