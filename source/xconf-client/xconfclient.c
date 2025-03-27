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

#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <stdbool.h>
#include <curl/curl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include <cjson/cJSON.h>

#include "t2log_wrapper.h"
#include "reportprofiles.h"
#include "profilexconf.h"
#include "xconfclient.h"
#include "t2MtlsUtils.h"
#include "t2parserxconf.h"
#include "vector.h"
#include "persistence.h"
#include "telemetry2_0.h"
#include "busInterface.h"
#ifdef LIBRDKCERTSEL_BUILD
#include "rdkcertselector.h"
#define FILESCHEME "file://"
#endif
#ifdef LIBRDKCONFIG_BUILD
#include "rdkconfig.h"
#endif
#define RFC_RETRY_TIMEOUT 60
#define XCONF_RETRY_TIMEOUT 180
#define MAX_XCONF_RETRY_COUNT 5
#define IFINTERFACE      "erouter0"
#define XCONF_CONFIG_FILE  "DCMresponse.txt"
#define PROCESS_CONFIG_COMPLETE_FLAG "/tmp/t2DcmComplete"
#define HTTP_RESPONSE_FILE "/tmp/httpOutput.txt"
#define DCM_CONF_FULL_PATH  XCONFPROFILE_PERSISTENCE_PATH "" XCONF_CONFIG_FILE

extern sigset_t blocking_signal;

#if defined(ENABLE_RDKB_SUPPORT) && !defined(RDKB_EXTENDER)

#if defined(WAN_FAILOVER_SUPPORTED) || defined(FEATURE_RDKB_CONFIGURABLE_WAN_INTERFACE)
   static char waninterface[256];
#endif
#endif
static const int MAX_URL_LEN = 1024;
static const int MAX_URL_ARG_LEN = 128;
static int xConfRetryCount = 0;
static bool stopFetchRemoteConfiguration = false;
static bool isXconfInit = false ;

#ifdef DCMAGENT
static bool bNotifyDCM = false;
static bool bexitDCMThread = true;
static pthread_t dcmThread;
#endif

static pthread_t xcrThread;
static pthread_mutex_t xcMutex;
static pthread_mutex_t xcThreadMutex;
static pthread_cond_t xcCond;
static pthread_cond_t xcThreadCond;
#ifdef LIBRDKCERTSEL_BUILD
static rdkcertselector_h xcCertSelector = NULL;
#endif

T2ERROR ReportProfiles_deleteProfileXConf(ProfileXConf *profile);

T2ERROR ReportProfiles_setProfileXConf(ProfileXConf *profile);

typedef enum _IFADDRESS_TYPE
{
    ADDR_UNKNOWN,
    ADDR_IPV4,
    ADDR_IPV6
}IFADDRESS_TYPE;

#if 0
static IFADDRESS_TYPE getAddressType(const char *cif) {
    struct ifaddrs *ifap, *ifa;
    IFADDRESS_TYPE addressType = 0;

    getifaddrs(&ifap);
    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        if (ifa->ifa_name == NULL || strcmp(ifa->ifa_name, cif))
            continue;

        if (ifa->ifa_addr->sa_family == AF_INET)
            addressType = ADDR_IPV4;
        else
            addressType = ADDR_IPV6;

        break;
    }

    freeifaddrs(ifap);
    return addressType;
}
#endif

T2ERROR getBuildType(char* buildType) {
    char fileContent[255] = { '\0' };
    FILE *deviceFilePtr;
    char *pBldTypeStr = NULL;
    int offsetValue = 0;
	    

    if (NULL == buildType) {
       return T2ERROR_FAILURE;
    }
    deviceFilePtr = fopen( DEVICE_PROPERTIES, "r");
    if (deviceFilePtr) {
        while (fscanf(deviceFilePtr, "%254s", fileContent) != EOF) {
            if ((pBldTypeStr = strstr(fileContent, "BUILD_TYPE")) != NULL) {
                offsetValue = strlen("BUILD_TYPE=");
                pBldTypeStr = pBldTypeStr + offsetValue;
                break;
            }
        }
        fclose(deviceFilePtr);
    }
    if(pBldTypeStr != NULL){
         strncpy(buildType, pBldTypeStr, BUILD_TYPE_MAX_LENGTH - 1);
         return T2ERROR_SUCCESS;
    }
    return T2ERROR_FAILURE;
}

#if !defined(ENABLE_RDKB_SUPPORT)
static char *getTimezone () {
    T2Debug("Retrieving the timezone value\n");
    int count = 0, i = 0;
    FILE *file, *fp;
    char *zoneValue = NULL;
    char *jsonDoc = NULL;
    static const char* jsonpath = NULL;
    static char* CPU_ARCH = NULL;
    char fileContent[255] = { '\0' };
    fp = fopen( DEVICE_PROPERTIES, "r");
    if (fp) {
        while (fscanf(fp, "%254s", fileContent) != EOF) {
            char *property = NULL;
            if ((property = strstr(fileContent, "CPU_ARCH")) != NULL) {
                property = property + strlen("CPU_ARCH=");
                CPU_ARCH = strdup(property);
                T2Debug("CPU_ARCH=%s\n",CPU_ARCH);
                break;
            }
        }
        fclose(fp);
    }
    jsonpath = "/opt/output.json";
    if((NULL != CPU_ARCH) && (0 == strcmp("x86", CPU_ARCH))){
            jsonpath = "/tmp/output.json";
    }
    T2Debug("Reading Timezone value from %s file...\n", jsonpath);
    while ( zoneValue == NULL){
          T2Debug ("timezone retry:%d\n",count);
          file = fopen( jsonpath, "r");
          if (file) {
              fseek(file, 0, SEEK_END);
              long numbytes = ftell(file);
              jsonDoc = (char*)malloc(sizeof(char)*(numbytes + 1));
              fseek(file, 0, SEEK_SET);
              //CID 190258: Argument cannot be negative (NEGATIVE_RETURNS)
              if (numbytes >0 ){
		  //CID 190270 Ignoring number of bytes read
                  size_t result = fread(jsonDoc, numbytes, 1, file);
		  if (result != 1) {
			  T2Debug ("Error reading file \n");
			  fclose(file);
			  free(jsonDoc);
			  jsonDoc = NULL;
			  return NULL;
                  }
              }
              fclose(file);
              cJSON *root = cJSON_Parse(jsonDoc);
              if (root != NULL){
                  cJSON *array = cJSON_GetObjectItem(root, "xmediagateways");
                  if(array){
                      for (i = 0 ; i < cJSON_GetArraySize(array) ; i++)
                      {
                          cJSON * subarray = cJSON_GetArrayItem(array, i);
                          cJSON * timezone = cJSON_GetObjectItem(subarray, "timezone");
                          if(timezone){
                              char *time = cJSON_GetStringValue(timezone);
                              //CID 190236: Resource leak (RESOURCE_LEAK)
                              if (zoneValue != NULL){
                                  free(zoneValue);
                              }
                              zoneValue = strdup(time);
                          }
                       }
                   }
               }
               free(jsonDoc);
               jsonDoc = NULL;
               cJSON_Delete(root);
          } else {
               T2Debug("Error opening File \n");
	       return NULL;
	  }
          count++;
         if (count == 10){
             T2Debug("Timezone retry count reached the limit . Timezone data source is missing\n");
             break;
         }
     }
     if ( zoneValue == NULL) {
              T2Debug("Timezone value from %s is empty, Reading from  /opt/persistent/timeZoneDST file...\n",jsonpath);
              if (access("/opt/persistent/timeZoneDST", F_OK) != -1){
                      file = fopen ("/opt/persistent/timeZoneDST", "r");
                      if (NULL != file){
                              fseek(file, 0, SEEK_END);
                              long numbytes = ftell(file);
                              char *zone = (char*)malloc(sizeof(char)*(numbytes + 1));
                              fseek(file, 0, SEEK_SET);
			      //CID : 190237 : Calling risky function
                              while (fgets (zone, numbytes + 1, file) != NULL){
                                        if(zoneValue){
                                            free(zoneValue);
                                            zoneValue = NULL ;
                                        }
                                        zoneValue = strdup(zone);
                              }
                        fclose(file);
                        free(zone);
                      }
              }
     
     }
     if(CPU_ARCH){
	     free(CPU_ARCH);
     }
     return zoneValue;
}
#endif

T2ERROR appendRequestParams(char *buf, const int maxArgLen) {

    T2ERROR ret = T2ERROR_FAILURE;
    T2Debug("%s ++in\n", __FUNCTION__);
    if(buf == NULL)
    {
        T2Error("Buffer is NULL for appendRequestParams\n");
        return T2ERROR_FAILURE;
    }
    int avaBufSize = maxArgLen, write_size = 0, slen = 0;
    char *paramVal = NULL;
    char *tempBuf = (char*) malloc(MAX_URL_ARG_LEN);
    char build_type[BUILD_TYPE_MAX_LENGTH] = { 0 };
    #if !defined(ENABLE_RDKB_SUPPORT) && !defined(ENABLE_RDKC_SUPPORT)
    char *timezone = NULL;
   #endif
    if(tempBuf == NULL)
    {
        T2Error("Failed to allocate memory for RequestParams\n");
        return T2ERROR_FAILURE;
    }

 
    if(T2ERROR_SUCCESS == getParameterValue(TR181_DEVICE_WAN_MAC, &paramVal)) {
        memset(tempBuf, 0, MAX_URL_ARG_LEN);
        write_size = snprintf(tempBuf, MAX_URL_ARG_LEN, "estbMacAddress=%s&", paramVal);
        strncat(buf, tempBuf, avaBufSize);
        avaBufSize = avaBufSize - write_size;
        free(paramVal);
        paramVal = NULL;
    } else {
          T2Error("Failed to get Value for %s\n", TR181_DEVICE_WAN_MAC);
          goto error;
    }

    if(T2ERROR_SUCCESS == getParameterValue(TR181_DEVICE_FW_VERSION, &paramVal)) {
        memset(tempBuf, 0, MAX_URL_ARG_LEN);
        write_size = snprintf(tempBuf, MAX_URL_ARG_LEN, "firmwareVersion=%s&", paramVal);
        strncat(buf, tempBuf, avaBufSize);
        avaBufSize = avaBufSize - write_size;
        free(paramVal);
        paramVal = NULL;
    } else {
          T2Error("Failed to get Value for %s\n", TR181_DEVICE_FW_VERSION);
          goto error;
    }

    if(T2ERROR_SUCCESS == getParameterValue(TR181_DEVICE_MODEL, &paramVal)) {
        memset(tempBuf, 0, MAX_URL_ARG_LEN);
        write_size = snprintf(tempBuf, MAX_URL_ARG_LEN, "model=%s&", paramVal);
        strncat(buf, tempBuf, avaBufSize);
        avaBufSize = avaBufSize - write_size;
        free(paramVal);
        paramVal = NULL;
    } else {
          T2Error("Failed to get Value for %s\n", TR181_DEVICE_MODEL);
          goto error;
    }
#if defined(USE_SERIALIZED_MANUFACTURER_NAME)
    if(T2ERROR_SUCCESS == getParameterValue(TR181_DEVICE_MFR, &paramVal)) {
        memset(tempBuf, 0, MAX_URL_ARG_LEN);
        write_size = snprintf(tempBuf, MAX_URL_ARG_LEN, "manufacturer=%s&", paramVal);
        strncat(buf, tempBuf, avaBufSize);
        avaBufSize = avaBufSize - write_size;
        free(paramVal);
        paramVal = NULL;
    } else {
          T2Error("Failed to get Value for %s\n", TR181_DEVICE_MFR);
          goto error;
    }
#endif
    if(T2ERROR_SUCCESS == getParameterValue(TR181_DEVICE_PARTNER_ID, &paramVal)) {
        memset(tempBuf, 0, MAX_URL_ARG_LEN);
        write_size = snprintf(tempBuf, MAX_URL_ARG_LEN, "partnerId=%s&", paramVal);
        strncat(buf, tempBuf, avaBufSize);
        avaBufSize = avaBufSize - write_size;
        free(paramVal);
        paramVal = NULL;
    } else {
          T2Error("Failed to get Value for %s\n", TR181_DEVICE_PARTNER_ID);
          goto error;
    }

#if defined(WHOAMI_ENABLED)
    if(T2ERROR_SUCCESS == getParameterValue(TR181_DEVICE_OSCLASS, &paramVal)) {
        memset(tempBuf, 0, MAX_URL_ARG_LEN);
        write_size = snprintf(tempBuf, MAX_URL_ARG_LEN, "osClass=%s&", paramVal);
        strncat(buf, tempBuf, avaBufSize);
        avaBufSize = avaBufSize - write_size;
        free(paramVal);
        paramVal = NULL;
    } else {
        T2Error("Failed to get Value for %s\n", TR181_DEVICE_OSCLASS);
        goto error;
    }
#endif

    if(T2ERROR_SUCCESS == getParameterValue(TR181_DEVICE_ACCOUNT_ID, &paramVal)) {
        memset(tempBuf, 0, MAX_URL_ARG_LEN);
        write_size = snprintf(tempBuf, MAX_URL_ARG_LEN, "accountId=%s&", paramVal);
        strncat(buf, tempBuf, avaBufSize);
        avaBufSize = avaBufSize - write_size;
        free(paramVal);
        paramVal = NULL;
    } else {
          T2Error("Failed to get Value for %s\n", TR181_DEVICE_ACCOUNT_ID);
          goto error;
    }

    if(T2ERROR_SUCCESS == getParameterValue(TR181_DEVICE_CM_MAC, &paramVal)) {
        memset(tempBuf, 0, MAX_URL_ARG_LEN);
        write_size = snprintf(tempBuf, MAX_URL_ARG_LEN, "ecmMacAddress=%s&", paramVal);
        strncat(buf, tempBuf, avaBufSize);
        avaBufSize = avaBufSize - write_size;
        free(paramVal);
        paramVal = NULL;
    } else {
          T2Error("Failed to get Value for %s\n", TR181_DEVICE_CM_MAC);
          goto error;
    }

    if(T2ERROR_SUCCESS == getBuildType(build_type)) {
        memset(tempBuf, 0, MAX_URL_ARG_LEN);
        write_size = snprintf(tempBuf, MAX_URL_ARG_LEN, "env=%s&", build_type);
        strncat(buf, tempBuf, avaBufSize);
        avaBufSize = avaBufSize - write_size;
        free(paramVal);
        paramVal = NULL;
        ret = T2ERROR_SUCCESS;
    } else {
          T2Error("Failed to get Value for %s\n", "BUILD_TYPE");
          goto error;
    }

    // TODO Check relevance of this existing hardcoded data - can be removed if not used in production
     strncat(buf,
            "controllerId=2504&channelMapId=2345&vodId=15660&",
            avaBufSize);
     slen = strlen("controllerId=2504&channelMapId=2345&vodId=15660&");
    avaBufSize = avaBufSize - slen;
#if !defined(ENABLE_RDKB_SUPPORT) && !defined(ENABLE_RDKC_SUPPORT)
    timezone = getTimezone();
    if(timezone != NULL){
            memset(tempBuf, 0, MAX_URL_ARG_LEN);
            write_size = snprintf(tempBuf, MAX_URL_ARG_LEN, "timezone=%s&",timezone);
            strncat(buf, tempBuf, avaBufSize);
            avaBufSize = avaBufSize - write_size;
            free(timezone);
     } else{
	     T2Error("Failed to get Value for %s\n", "TIMEZONE");
             ret = T2ERROR_FAILURE;
	     goto error;
     }
#endif
    strncat(buf,"version=2", avaBufSize);
    slen = strlen("version=2");
    avaBufSize = avaBufSize - slen;
    T2Debug("%s:%d Final http get URL if size %d is : \n %s \n", __func__,
            __LINE__, avaBufSize, buf);
#if defined(PRIVACYMODES_CONTROL)
    if(T2ERROR_SUCCESS == getParameterValue(PRIVACYMODES_RFC, &paramVal)) {
        memset(tempBuf, 0, MAX_URL_ARG_LEN);
        write_size = snprintf(tempBuf, MAX_URL_ARG_LEN, "&privacyModes=%s", paramVal);
        strncat(buf, tempBuf, avaBufSize);
        avaBufSize = avaBufSize - write_size;
        free(paramVal);
        paramVal = NULL;
    } else {
          T2Error("Failed to get Value for %s\n", PRIVACYMODES_RFC);
          ret = T2ERROR_FAILURE;
          goto error;
    }
    T2Debug("%s:%d Final http get URL when privacymode is enabled of size %d is : \n %s \n", __func__,
            __LINE__, avaBufSize, buf);
#endif
error:
    if (NULL != tempBuf) {
        free(tempBuf);
    }

    T2Debug("%s --out\n", __FUNCTION__);
    return ret;
}

static size_t httpGetCallBack(void *response, size_t len, size_t nmemb,
        void *stream) {

    size_t realsize = len * nmemb;
    curlResponseData* httpResponse = (curlResponseData*) stream;

    char *ptr = (char*) realloc(httpResponse->data,
            httpResponse->size + realsize + 1);
    if (!ptr) {
        T2Error("%s:%u , T2:memory realloc failed\n", __func__, __LINE__);
        return 0;
    }
    httpResponse->data = ptr;
    memcpy(&(httpResponse->data[httpResponse->size]), response, realsize);
    httpResponse->size += realsize;
    httpResponse->data[httpResponse->size] = 0;

    return realsize;
}

#ifdef LIBRDKCERTSEL_BUILD
void xcCertSelectorFree()
{
    rdkcertselector_free(&xcCertSelector);
    if(xcCertSelector == NULL){
        T2Info("%s, T2:Cert selector memory free  \n", __func__);
    }else{
        T2Info("%s, T2:Cert selector memory free failed \n", __func__);
    }
}
static void xcCertSelectorInit()
{
    if(xcCertSelector == NULL)
    {
        xcCertSelector = rdkcertselector_new( NULL, NULL, "MTLS" );
        if(xcCertSelector == NULL){
            T2Error("%s, T2:Cert selector initialization failed\n", __func__);
        }else{
            T2Info("%s, T2:Cert selector initialization successfully \n", __func__);
        }
    }
}
#endif
T2ERROR doHttpGet(char* httpsUrl, char **data) {

    T2Debug("%s ++in\n", __FUNCTION__);

    T2Info("%s with url %s \n", __FUNCTION__, httpsUrl);
    CURL *curl;
    CURLcode code = CURLE_OK;
    long http_code = 0;
    CURLcode curl_code = CURLE_OK;
#ifdef LIBRDKCERTSEL_BUILD
    rdkcertselectorStatus_t xcGetCertStatus;
    char *pCertURI = NULL;
    char *pEngine=NULL;
#endif
    char *pCertFile = NULL;
    char *pPasswd = NULL;
#ifdef LIBRDKCONFIG_BUILD
    size_t sPasswdSize = 0;
#endif
    // char *pKeyType = "PEM" ;
    bool mtls_enable = false;
    pid_t childPid;
    int sharedPipeFdStatus[2];
    int sharedPipeFdDataLen[2];

    if(NULL == httpsUrl) {
        T2Error("NULL httpsUrl given, doHttpGet failed\n");
        return T2ERROR_FAILURE;
    }

    if(pipe(sharedPipeFdStatus) != 0) {
        T2Error("Failed to create pipe for status !!! exiting...\n");
        T2Debug("%s --out\n", __FUNCTION__);
        return T2ERROR_FAILURE;
    }

    if(pipe(sharedPipeFdDataLen) != 0) {
        T2Error("Failed to create pipe for data length!!! exiting...\n");
        T2Debug("%s --out\n", __FUNCTION__);
        return T2ERROR_FAILURE;
    } 
#if defined(ENABLE_RDKB_SUPPORT) && !defined(RDKB_EXTENDER)

#if defined(WAN_FAILOVER_SUPPORTED) || defined(FEATURE_RDKB_CONFIGURABLE_WAN_INTERFACE)
    char *paramVal = NULL;
    memset(waninterface, 0, sizeof(waninterface));
    snprintf(waninterface, sizeof(waninterface), "%s", IFINTERFACE); 

 if(T2ERROR_SUCCESS == getParameterValue(TR181_DEVICE_CURRENT_WAN_IFNAME, &paramVal)) {
        if(strlen(paramVal) >0) {
            memset(waninterface, 0, sizeof(waninterface));
            snprintf(waninterface, sizeof(waninterface), "%s", paramVal);
	    T2Info("TR181_DEVICE_CURRENT_WAN_IFNAME -- %s\n", waninterface);
        }
        free(paramVal);
        paramVal = NULL;
    } else {
          T2Error("Failed to get Value for %s\n", TR181_DEVICE_CURRENT_WAN_IFNAME);
    }
#endif
#endif
    mtls_enable = isMtlsEnabled();
    // block the userdefined signal handlers before fork
    pthread_sigmask(SIG_BLOCK,&blocking_signal,NULL);
    if((childPid = fork()) < 0) {
        T2Error("Failed to fork !!! exiting...\n");
        // Unblock the userdefined signal handlers 
        pthread_sigmask(SIG_UNBLOCK,&blocking_signal,NULL);
        T2Debug("%s --out\n", __FUNCTION__);
        return T2ERROR_FAILURE;
    }

    /**
     * Openssl has growing RSS which gets cleaned up only with OPENSSL_cleanup .
     * This cleanup is not thread safe and classified as run once per application life cycle.
     * Forking the libcurl calls so that it executes and terminates to release memory per execution.
     */
    if(childPid == 0) {

        T2ERROR ret = T2ERROR_FAILURE;
        curlResponseData* httpResponse = (curlResponseData *) malloc(sizeof(curlResponseData));
        httpResponse->data = (char*)malloc(1);
        httpResponse->data[0]='\0';//CID 282084 : Uninitialized scalar variable (UNINIT)
        httpResponse->size = 0;

        curl = curl_easy_init();

        if(curl) {

            code = curl_easy_setopt(curl, CURLOPT_URL, httpsUrl);
            if(code != CURLE_OK) {
                T2Error("%s : Curl set opts failed with error %s \n", __FUNCTION__, curl_easy_strerror(code));
            }
            code = curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
            if(code != CURLE_OK) {
                T2Error("%s : Curl set opts failed with error %s \n", __FUNCTION__, curl_easy_strerror(code));
            }
            code = curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L);
            if(code != CURLE_OK) {
                T2Error("%s : Curl set opts failed with error %s \n", __FUNCTION__, curl_easy_strerror(code));
            }
            code = curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
            if(code != CURLE_OK) {
                T2Error("%s : Curl set opts failed with error %s \n", __FUNCTION__, curl_easy_strerror(code));
            }
            code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, httpGetCallBack);
            if(code != CURLE_OK) {
                T2Error("%s : Curl set opts failed with error %s \n", __FUNCTION__, curl_easy_strerror(code));
            }
            code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) httpResponse);
            if(code != CURLE_OK) {
                T2Error("%s : Curl set opts failed with error %s \n", __FUNCTION__, curl_easy_strerror(code));
            }
#if defined(ENABLE_RDKB_SUPPORT) && !defined(RDKB_EXTENDER)

#if defined(WAN_FAILOVER_SUPPORTED) || defined(FEATURE_RDKB_CONFIGURABLE_WAN_INTERFACE)
            code = curl_easy_setopt(curl, CURLOPT_INTERFACE, waninterface);
            T2Info("TR181_DEVICE_CURRENT_WAN_IFNAME ---- %s\n", waninterface);
            if(code != CURLE_OK) {
                 T2Error("%s : Curl set opts failed with error %s \n", __FUNCTION__, curl_easy_strerror(code));
            }
#else
            code = curl_easy_setopt(curl, CURLOPT_INTERFACE, IFINTERFACE);
            if(code != CURLE_OK) {
                 T2Error("%s : Curl set opts failed with error %s \n", __FUNCTION__, curl_easy_strerror(code));
            }
#endif

#endif
            if(mtls_enable == true) {
#ifdef LIBRDKCERTSEL_BUILD
                pEngine= rdkcertselector_getEngine(xcCertSelector);
                if(pEngine!=NULL){
                    code = curl_easy_setopt(curl, CURLOPT_SSLENGINE, pEngine);
                }else{
                    code = curl_easy_setopt(curl, CURLOPT_SSLENGINE_DEFAULT, 1L);
                }
                if(code != CURLE_OK) {
                   T2Error("%s : Curl set opts failed with error %s \n", __FUNCTION__, curl_easy_strerror(code));
                }
                do{
                    pCertFile = NULL;
                    pPasswd = NULL;
                    pCertURI = NULL;
                    xcGetCertStatus= rdkcertselector_getCert(xcCertSelector, &pCertURI, &pPasswd);
                    if(xcGetCertStatus != certselectorOk)
                    {
                        T2Error("%s, T2:Failed to retrieve the certificate.\n", __func__);
                        xcCertSelectorFree();
                        free(httpResponse->data);
                        free(httpResponse);
                        curl_easy_cleanup(curl);
                        ret = T2ERROR_FAILURE;
                        goto status_return;
                    }else {
                        // skip past file scheme in URI
                        pCertFile = pCertURI;
                        if ( strncmp( pCertFile, FILESCHEME, sizeof(FILESCHEME)-1 ) == 0 ) {
                            pCertFile += (sizeof(FILESCHEME)-1);
                        }
#else
                if(T2ERROR_SUCCESS == getMtlsCerts(&pCertFile, &pPasswd)) {
#endif
                    code = curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "P12");
                    if(code != CURLE_OK) {
                        T2Error("%s : Curl set opts failed with error %s \n", __FUNCTION__, curl_easy_strerror(code));
                    }
                    code = curl_easy_setopt(curl, CURLOPT_SSLCERT, pCertFile);
                    if(code != CURLE_OK) {
                        T2Error("%s : Curl set opts failed with error %s \n", __FUNCTION__, curl_easy_strerror(code));
                    }
                    code = curl_easy_setopt(curl, CURLOPT_KEYPASSWD, pPasswd);
                    if(code != CURLE_OK) {
                        T2Error("%s : Curl set opts failed with error %s \n", __FUNCTION__, curl_easy_strerror(code));
                    }
                    /* disconnect if authentication fails */
                    code = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
                    if(code != CURLE_OK) {
                        T2Error("%s : Curl set opts failed with error %s \n", __FUNCTION__, curl_easy_strerror(code));
                    }
                    curl_code = curl_easy_perform(curl);
#ifdef LIBRDKCERTSEL_BUILD
                        if(curl_code != CURLE_OK){
                            T2Info("%s: Using xpki Certs connection certname : %s \n", __FUNCTION__, pCertFile);
                            T2Error("Curl failed : %d \n", curl_code);
                        }
                    }
                }while(rdkcertselector_setCurlStatus(xcCertSelector, curl_code, (const char*)httpsUrl) == TRY_ANOTHER);
#else
                }else {
                    free(httpResponse->data);
                    free(httpResponse);
                    curl_easy_cleanup(curl); //CID 189986:Resource leak
                    T2Error("mTLS_get failure\n");
                    ret = T2ERROR_FAILURE;
                    goto status_return;
                }
#endif
            }else{
                  curl_code = curl_easy_perform(curl);
            }
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

            if(http_code == 200 && curl_code == CURLE_OK) {
                T2Info("%s:%d, T2:Telemetry XCONF communication success\n", __func__, __LINE__);
                size_t len = strlen(httpResponse->data);

                // Share data with parent
                close(sharedPipeFdDataLen[0]);
                write(sharedPipeFdDataLen[1], &len, sizeof(size_t));
                close(sharedPipeFdDataLen[1]);

                FILE *httpOutput = fopen(HTTP_RESPONSE_FILE, "w+");
                if(httpOutput){
                    T2Debug("Update config data in response file %s \n", HTTP_RESPONSE_FILE);
                    fputs(httpResponse->data, httpOutput);
                    fclose(httpOutput);
                } else{
                    T2Error("Unable to open %s file \n", HTTP_RESPONSE_FILE);
                }

                free(httpResponse->data);
                free(httpResponse);
#ifndef LIBRDKCERTSEL_BUILD
                if(NULL != pCertFile)
                    free(pCertFile);

                if(NULL != pPasswd){
                  #ifdef LIBRDKCONFIG_BUILD
                    sPasswdSize = strlen(pPasswd);
                    if (rdkconfig_free((unsigned char**)&pPasswd, sPasswdSize)  == RDKCONFIG_FAIL) {
                        return T2ERROR_FAILURE;
                    }
                  #else
                    free(pPasswd);
                  #endif
                }
#endif
                curl_easy_cleanup(curl);
            }else {
                T2Error("%s:%d, T2:Telemetry XCONF communication Failed with http code : %ld Curl code : %d \n", __func__, __LINE__, http_code,
                        curl_code);
                T2Error("%s : curl_easy_perform failed with error message %s from curl \n", __FUNCTION__, curl_easy_strerror(curl_code));
                free(httpResponse->data);
                free(httpResponse);
#ifndef LIBRDKCERTSEL_BUILD
                if(NULL != pCertFile)
                    free(pCertFile);
                if(NULL != pPasswd){
                  #ifdef LIBRDKCONFIG_BUILD
                    sPasswdSize = strlen(pPasswd);
                    if (rdkconfig_free((unsigned char**)&pPasswd, sPasswdSize)  == RDKCONFIG_FAIL) {
                        return T2ERROR_FAILURE;
                    }
                  #else
                    free(pPasswd);
                  #endif
                }
#endif
                curl_easy_cleanup(curl);
                if(http_code == 404)
                    ret = T2ERROR_PROFILE_NOT_SET;
                else
                    ret = T2ERROR_FAILURE;
                goto status_return ;
            }
        }else {
            free(httpResponse->data);
            free(httpResponse);
            ret = T2ERROR_FAILURE;
            goto status_return ;
        }

        ret = T2ERROR_SUCCESS ;
        status_return :

        close(sharedPipeFdStatus[0]);
        write(sharedPipeFdStatus[1], &ret, sizeof(T2ERROR));
        close(sharedPipeFdStatus[1]);
        exit(0);

    }else { // Parent
        T2ERROR ret = T2ERROR_FAILURE;
        // Use waitpid insted of wait
        waitpid(childPid,NULL,0);
        // Unblock the userdefined signal handlers after wait
        pthread_sigmask(SIG_UNBLOCK,&blocking_signal,NULL);
        // Get the return status via IPC from child process
        close(sharedPipeFdStatus[1]);
        ssize_t readBytes = read(sharedPipeFdStatus[0], &ret, sizeof(T2ERROR));
        if(readBytes == -1) {
            T2Error("Failed to read from pipe\n");
            return T2ERROR_FAILURE;
        }
        close(sharedPipeFdStatus[0]);

        // Get the datas via IPC from child process
        if(ret == T2ERROR_SUCCESS) {
            size_t len = 0;
            close(sharedPipeFdDataLen[1]);
            readBytes = read(sharedPipeFdDataLen[0], &len, sizeof(size_t));
            if(readBytes == -1) {
                T2Error("Failed to read from pipe\n");
                return T2ERROR_FAILURE;
            }
            close(sharedPipeFdDataLen[0]);
            *data = NULL;
            if(len <= SIZE_MAX)
            {
                *data = (char*)malloc(len + 1);
            }
            if(*data == NULL) {
                T2Error("Unable to allocate memory for XCONF config data \n");
                ret = T2ERROR_FAILURE;
            }else {
                if(len <= SIZE_MAX)
                {
                    memset(*data, '\0', len + 1);
                }
                FILE *httpOutput = fopen(HTTP_RESPONSE_FILE, "r+");
                if(httpOutput){
                    // Read the whole file content
                    if(len <= SIZE_MAX)
                    {
                        readBytes = fread(*data, len, 1, httpOutput);
                    }
                    if(readBytes == -1) {
                        T2Error("Failed to read from pipe\n");
                        return T2ERROR_FAILURE;
                    }
                    T2Debug("Configuration obtained from http server : \n %s \n", *data);
                    fclose(httpOutput);
                }
            }
        }
        T2Debug("%s --out\n", __FUNCTION__);
        return ret;

    }
    
}

T2ERROR fetchRemoteConfiguration(char *configURL, char **configData) {
    // Handles the https communications with the xconf server
    T2ERROR ret = T2ERROR_FAILURE;
    T2Debug("%s ++in\n", __FUNCTION__);
    if(configURL == NULL){
        T2Error("configURL is NULL\n");
        return T2ERROR_INVALID_ARGS;
    }
    int write_size = 0, availableBufSize = MAX_URL_LEN;
    char* urlWithParams = (char*) malloc(MAX_URL_LEN * sizeof(char));
    if (NULL != urlWithParams)
    {
        memset(urlWithParams, 0, MAX_URL_LEN * sizeof(char));
        write_size = snprintf(urlWithParams, MAX_URL_LEN, "%s?", configURL);
        availableBufSize = availableBufSize - write_size;
        // Append device specific arguments to the base URL
        if(T2ERROR_SUCCESS == appendRequestParams(urlWithParams, availableBufSize))
        {
            ret = doHttpGet(urlWithParams, configData);
            if (ret != T2ERROR_SUCCESS)
            {
                T2Error("T2:Curl GET of XCONF data failed\n");
            }
        }
        free(urlWithParams);
    }
    else
    {
        T2Error("Malloc failed\n");
    }
    T2Debug("%s --out\n", __FUNCTION__);
    return ret;
}

T2ERROR getRemoteConfigURL(char **configURL) {

    T2ERROR ret = T2ERROR_FAILURE;
    T2Debug("%s ++in\n", __FUNCTION__);

    char *paramVal = NULL;
     /**
     * Attempts to read from PAM before its ready creates deadlock in PAM .
     * Long pending unresolved issue with PAM !!!
     * PAM not ready is a definite case for caching the event and avoid bus traffic
     * */
    #if defined(ENABLE_RDKB_SUPPORT)
    int count = 0 , MAX_RETRY = 20 ;
    while (access( "/tmp/pam_initialized", F_OK ) != 0) {
        sleep(6);
        if(count >= MAX_RETRY)
            break ;
        count ++ ;
    }
    #endif

    if (T2ERROR_SUCCESS == getParameterValue(TR181_CONFIG_URL, &paramVal)) {
        if (NULL != paramVal) {
            if ((strlen(paramVal) > 8) && (0 == strncmp(paramVal,"https://", 8))) {  // Enforcing https for new endpoints
                T2Info("Setting config URL base location to : %s\n", paramVal);
                *configURL = paramVal;
                ret = T2ERROR_SUCCESS ;
            } else {
                T2Error("URL doesn't start with https or is invalid !!! URL value received : %s .\n", paramVal);
                free(paramVal);
            }
        } else {
            ret = T2ERROR_FAILURE;
        }
    } else {
        T2Error("Failed to fetch value for parameter %s \n", TR181_CONFIG_URL);
        ret = T2ERROR_FAILURE ;
    }
    T2Debug("%s --out\n", __FUNCTION__);
    return ret; 
}

static void* getUpdatedConfigurationThread(void *data)
{
    (void) data;
    T2ERROR configFetch = T2ERROR_FAILURE;
    T2Debug("%s ++in\n", __FUNCTION__);
    struct timespec _ts;
    struct timespec _now;
    int n;
    char *configURL = NULL;
    char *configData = NULL;
    pthread_mutex_lock(&xcThreadMutex);
    stopFetchRemoteConfiguration = false ;
    do{
        T2Debug("%s while Loop -- START \n", __FUNCTION__);
        while(!stopFetchRemoteConfiguration && T2ERROR_SUCCESS != getRemoteConfigURL(&configURL))
        {
            pthread_mutex_lock(&xcMutex);
            memset(&_ts, 0, sizeof(struct timespec));
            memset(&_now, 0, sizeof(struct timespec));
            clock_gettime(CLOCK_REALTIME, &_now);
            _ts.tv_sec = _now.tv_sec + RFC_RETRY_TIMEOUT;

            T2Info("Waiting for %d sec before trying getRemoteConfigURL\n", RFC_RETRY_TIMEOUT);
            n = pthread_cond_timedwait(&xcCond, &xcMutex, &_ts);
            if(n == ETIMEDOUT)
            {
                T2Info("TIMEDOUT -- trying fetchConfigURLs again\n");
            }
            else if (n == 0)
            {
                T2Error("XConfClient Interrupted\n");
            }
            else
            {
                T2Error("ERROR inside startXConfClientThread for timedwait");
            }
            pthread_mutex_unlock(&xcMutex);
        }

        while(!stopFetchRemoteConfiguration)
        {
            T2ERROR ret = fetchRemoteConfiguration(configURL, &configData);
            if(ret == T2ERROR_SUCCESS)
            {
                ProfileXConf *profile = 0;
                T2Debug("Config received successfully from URL : %s\n", configURL);
                T2Debug("Config received = %s\n", configData);

                if(T2ERROR_SUCCESS == processConfigurationXConf(configData, &profile))
                {
                    clearPersistenceFolder(XCONFPROFILE_PERSISTENCE_PATH);
                    if(T2ERROR_SUCCESS != saveConfigToFile(XCONFPROFILE_PERSISTENCE_PATH, XCONF_CONFIG_FILE, configData)) // Should be removed once XCONF sends new UUID for each update.
                    {
                        T2Error("Unable to update an existing config file : %s\n", profile->name);
                    }
                    T2Debug("Disable and Delete old profile %s\n", profile->name);
                    if(T2ERROR_SUCCESS != ReportProfiles_deleteProfileXConf(profile))
                    {
                        T2Error("Unable to delete old profile of : %s\n", profile->name);
                    }

                    T2Debug("Set new profile : %s\n", profile->name);
                    if(T2ERROR_SUCCESS != ReportProfiles_setProfileXConf(profile))
                    {
                        T2Error("Failed to set profile : %s\n", profile->name);
                    }
                    else
                    {
                        T2Info("Successfully set new profile : %s\n", profile->name);
                        configFetch = T2ERROR_SUCCESS;
                    }

                    // Touch a file to indicate script based supplementary services to proceed with configuration
                    FILE *fp = NULL ;
                    fp = fopen(PROCESS_CONFIG_COMPLETE_FLAG, "w+");
                    if(fp)
                        fclose(fp);

                    #ifdef DCMAGENT
                    T2Info("Set DCM flag for sending events\n");
                    bNotifyDCM = true;
                    #endif

                }
                if(configData != NULL) {
                    free(configData);
                    configData = NULL ;
                }
                break;
            }
            else if(ret == T2ERROR_PROFILE_NOT_SET)
            {
                T2Warning("XConf Telemetry profile not set for this device, uninitProfileList.\n");
                if(configData != NULL) {
                    free(configData);
                    configData = NULL ;
                }
                break;
            }
            else
            {
                if(configData != NULL) {
                    free(configData);
                    configData = NULL ;
                }
                xConfRetryCount++;
                if(xConfRetryCount >= MAX_XCONF_RETRY_COUNT)
                {
                    T2Error("Reached max xconf retry counts : %d, Using saved profile if exists until next reboot\n", MAX_XCONF_RETRY_COUNT);
                    xConfRetryCount = 0;
                    break;
                }
                T2Info("Waiting for %d sec before trying fetchRemoteConfiguration, No.of tries : %d\n", XCONF_RETRY_TIMEOUT, xConfRetryCount);

                pthread_mutex_lock(&xcMutex);

                memset(&_ts, 0, sizeof(struct timespec));
                memset(&_now, 0, sizeof(struct timespec));
                clock_gettime(CLOCK_REALTIME, &_now);
                _ts.tv_sec = _now.tv_sec + XCONF_RETRY_TIMEOUT;

                n = pthread_cond_timedwait(&xcCond, &xcMutex, &_ts);
                if(n == ETIMEDOUT)
                {
                    T2Info("TIMEDOUT -- trying fetchConfigurations again\n");
                }
                else if (n == 0)
                {
                    T2Error("XConfClient Interrupted\n");
                }
                else
                {
                    T2Error("ERROR inside startXConfClientThread for timedwait, error code : %d\n", n);
                }
                pthread_mutex_unlock(&xcMutex);
            }
        }  // End of config fetch while
        if(configFetch == T2ERROR_FAILURE && !ProfileXConf_isSet())
        {
            T2Error("Failed to fetch updated configuration and no saved configurations on disk for XCONF, uninitializing  the process\n");
        }

        if(configURL){
            free(configURL);
            configURL = NULL;
        }
        stopFetchRemoteConfiguration = true;
        T2Debug("%s while Loop -- END; wait for restart event\n", __FUNCTION__);
        pthread_cond_wait(&xcThreadCond,&xcThreadMutex);
    }while(isXconfInit); //End of do while loop

    pthread_mutex_unlock(&xcThreadMutex);
    // pthread_detach(pthread_self()); commenting this line as thread will detached by stopXConfClient
    T2Debug("%s --out\n", __FUNCTION__);
    return NULL;
}

void uninitXConfClient()
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if(!stopFetchRemoteConfiguration)
    {
        stopFetchRemoteConfiguration = true;
        T2Info("fetchRemoteConfigurationThread signalled to stop\n");
        pthread_mutex_lock(&xcMutex);
        pthread_cond_signal(&xcCond);
        pthread_mutex_unlock(&xcMutex);

    }
    else
    {
        T2Debug("XConfClientThread is stopped already\n");
    }
    if(isXconfInit){
        pthread_mutex_lock(&xcThreadMutex);
        isXconfInit = false;
        pthread_cond_signal(&xcThreadCond);
        pthread_mutex_unlock(&xcThreadMutex);
        pthread_join(xcrThread, NULL);
        pthread_mutex_destroy(&xcMutex);
        pthread_mutex_destroy(&xcThreadMutex);
        pthread_cond_destroy(&xcCond);
        pthread_cond_destroy(&xcThreadCond);
        #ifdef DCMAGENT
        bexitDCMThread = false;
        pthread_join(dcmThread, NULL);
        #endif
#ifdef LIBRDKCERTSEL_BUILD
	    xcCertSelectorFree();
#endif
    }
    T2Debug("%s --out\n", __FUNCTION__);
    T2Info("Uninit XConf Client Successful\n");
}

#ifdef DCMAGENT
static void* nofifyDCMThread(void *data)
{
    (void) data; //To avoid compiler warning and use the pthread signature
    bool dcmEventStatus = 0;
    int count = 0;

    while(bexitDCMThread) {
        dcmEventStatus = getRbusDCMEventStatus();
        if(count > 100) {
            T2Info("flags status: bNotifyDCM: %d dcmEventStatus: %d\n", bNotifyDCM, dcmEventStatus);
            count = 0;
        }
        count++;
        if(bNotifyDCM && dcmEventStatus) {
            /* Publish DCM Events */
            T2Info("Publishing the set conf event Path: %s\n", DCM_CONF_FULL_PATH);
            if(T2ERROR_SUCCESS != publishEventsDCMSetConf(DCM_CONF_FULL_PATH)) {
                T2Error("Failed to Publish set conf event to DCM \n");
            }

            T2Info("Publishing the Process conf event\n");
            if(T2ERROR_SUCCESS != publishEventsDCMProcConf()) {
                T2Error("Failed to Publish process conf event to DCM \n");
            }
            bNotifyDCM = false;
            bexitDCMThread = false;
        }
        sleep(1);
    }
    return NULL;
}
T2ERROR startDCMClient()
{
    T2Debug("%s ++in\n", __FUNCTION__);

    pthread_create(&dcmThread, NULL, nofifyDCMThread, NULL);

    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}
#endif

T2ERROR initXConfClient()
{
    T2Debug("%s ++in\n", __FUNCTION__);
    #ifdef DCMAGENT
	startDCMClient();
    #endif
    pthread_mutex_init(&xcMutex, NULL);
    pthread_mutex_init(&xcThreadMutex, NULL);
    pthread_cond_init(&xcCond, NULL);
    pthread_cond_init(&xcThreadCond, NULL);
    isXconfInit = true ;
#ifdef LIBRDKCERTSEL_BUILD
    xcCertSelectorInit();
#endif
    pthread_create(&xcrThread, NULL, getUpdatedConfigurationThread, NULL);
    //startXConfClient(); // Removing startXConfClient as getUpdatedConfigurationThread is created in this function itself
    T2Debug("%s --out\n", __FUNCTION__);
    T2Info("Init Xconf Client Success\n");
    return T2ERROR_SUCCESS;
}

T2ERROR stopXConfClient()
{
    T2Debug("%s ++in\n", __FUNCTION__);
    //pthread_detach(xcrThread);
    pthread_mutex_lock(&xcMutex);
    stopFetchRemoteConfiguration = true;
    pthread_cond_signal(&xcCond);
    pthread_mutex_unlock(&xcMutex);
    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

T2ERROR startXConfClient()
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if (isXconfInit) {
        //pthread_create(&xcrThread, NULL, getUpdatedConfigurationThread, NULL);
	pthread_mutex_lock(&xcMutex);
	pthread_cond_signal(&xcCond);
        pthread_mutex_unlock(&xcMutex);
	pthread_mutex_lock(&xcThreadMutex);
        stopFetchRemoteConfiguration = false;
        pthread_cond_signal(&xcThreadCond);
        pthread_mutex_unlock(&xcThreadMutex);
    } else {
    	T2Info("getUpdatedConfigurationThread is still active ... Ignore xconf reload \n");
    }
    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}
