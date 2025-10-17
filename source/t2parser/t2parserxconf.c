
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
#include <string.h>

#include "t2parserxconf.h"
#include "xconfclient.h"
#include "rbusInterface.h"
#include "busInterface.h"
#include "reportprofiles.h"
#include "t2log_wrapper.h"
#include "t2common.h"
#include "persistence.h"

#if defined (PRIVACYMODES_CONTROL)
#include "rdkservices_privacyutils.h"
#endif

#define MT_EVENT_PATTERN   "<event>"
#define MT_EVENT_PATTERN_LENGTH 7
#define MT_TR181PARAM_PATTERN   "<message_bus>"
#define ALIAS_DATAMODEL "<DM>"
#define MT_TR181PATAM_PATTERN_LENGTH 13
#define SPLITMARKER_SUFFIX  "_split"
#define ACCUMULATE_MARKER_SUFFIX  "_accum"
#define MAX_PARAM_LEN 15

static int getScheduleInSeconds(const char* cronPattern)
{
    unsigned int scheduleIntervalInMin = 15; // Default value from field settings
    unsigned int scheduleIntervalInSec = 0 ;
    char* input = NULL;
    if (NULL != cronPattern)
    {
        input = strdup(cronPattern);
        char* inputMatchPattern = strstr(input, "/");
        if (NULL != inputMatchPattern)
        {
            inputMatchPattern = inputMatchPattern + 1; // 1 offset value
            int inputMatchSize = strlen(inputMatchPattern);
            char *discardPattern = strstr(inputMatchPattern, " *");
            if (NULL != discardPattern)
            {
                int discardPatternLen = 0;
                discardPatternLen = strlen(discardPattern);
                int timeInMinDataLen = inputMatchSize - discardPatternLen;
                char scheduleString[10] = { '\0' };
                strncpy(scheduleString, inputMatchPattern, timeInMinDataLen);
                scheduleIntervalInMin = atoi(scheduleString);
            }

        }
        /* CID 159640 -Dereference before null check */
        free(input);
    }
    // Convert minutes to seconds
    scheduleIntervalInSec = scheduleIntervalInMin * 60 ;
    return scheduleIntervalInSec ;
}

static T2ERROR addParameter(ProfileXConf *profile, const char* name, const char* ref, const char* fileName, int skipFreq)
{
    if(fileName == NULL) //Event Marker
    {
        char *splitSuffix = NULL;
        char *accumulateSuffix = NULL;
        // T2Debug("Adding Event Marker :: Param/Marker Name : %s ref/pattern/Comp : %s skipFreq : %d\n", name, ref, skipFreq);
        EventMarker *eMarker = (EventMarker *)malloc(sizeof(EventMarker));
        memset(eMarker, 0, sizeof(EventMarker));
        eMarker->markerName = strdup(name);
        eMarker->compName = strdup(ref);
        eMarker->regexParam = NULL;
        splitSuffix = strstr(name, SPLITMARKER_SUFFIX);
        accumulateSuffix = strstr(name, ACCUMULATE_MARKER_SUFFIX);
        if(splitSuffix != NULL && strcmp(splitSuffix, SPLITMARKER_SUFFIX) == 0)
        {
            eMarker->mType = (MarkerType)MTYPE_XCONF_ABSOLUTE;
            eMarker->u.markerValue = NULL;
        }
        else if(accumulateSuffix != NULL && strcmp(accumulateSuffix, ACCUMULATE_MARKER_SUFFIX) == 0)
        {
            eMarker->mType = (MarkerType)MTYPE_XCONF_ACCUMULATE;
            Vector_Create(&eMarker->u.accumulatedValues);
        }
        else
        {
            eMarker->mType = (MarkerType)MTYPE_XCONF_COUNTER;
            eMarker->u.count = 0;
        }
        eMarker->skipFreq = skipFreq;

        Vector_PushBack(profile->eMarkerList, eMarker);
    }
    else if(skipFreq == -1 || strncmp(fileName, ALIAS_DATAMODEL, 5) == 0)
    {
        // T2Debug("Adding TR-181 Parameter : %s\n", ref);
        Param *param = (Param *)malloc(sizeof(Param));
        memset(param, 0, sizeof(Param));
        param->name = strdup(name);
        param->alias = strdup(ref);
        param->regexParam = NULL;
        param->skipFreq = skipFreq;
        Vector_PushBack(profile->paramList, param);
    }
    else //Grep Marker
    {
        if(strncmp("top_log.txt", fileName, sizeof("top_log.txt")) == 0)
        {
            T2Debug("This is a TopMarker name :%s and value: %s add it to topmarker list \n", name, ref);
            char *splitSuffix = NULL;
            char *accumulateSuffix = NULL;
            // T2Debug("Adding Grep Marker :: Param/Marker Name : %s ref/pattern/Comp : %s fileName : %s skipFreq : %d\n", name, ref, fileName, skipFreq);
            TopMarker *tMarker = (TopMarker *)malloc(sizeof(TopMarker));
            memset(tMarker, 0, sizeof(TopMarker));
            tMarker->markerName = strdup(name);
            tMarker->searchString = strdup(ref);
            tMarker->logFile = strdup(fileName);
            tMarker->firstSeekFromEOF = 0;// memset will already set to 0 just a safeguard
            tMarker->regexParam = NULL;
	    tMarker->memValue = NULL;
            tMarker->cpuValue = NULL;
            tMarker->loadAverage = NULL;
            splitSuffix = strstr(name, SPLITMARKER_SUFFIX);
            accumulateSuffix = strstr(name, ACCUMULATE_MARKER_SUFFIX);
            if(splitSuffix != NULL && strcmp(splitSuffix, SPLITMARKER_SUFFIX) == 0)
            {
                tMarker->mType = MTYPE_ABSOLUTE;
                tMarker->u.markerValue = NULL;
            }
            else if(accumulateSuffix != NULL && strcmp(accumulateSuffix, ACCUMULATE_MARKER_SUFFIX) == 0)
            {
                tMarker->mType = (MarkerType)MTYPE_XCONF_ACCUMULATE;
                Vector_Create(&tMarker->u.accumulatedValues);
            }
            else
            {
                tMarker->mType = MTYPE_COUNTER;
                tMarker->u.count = 0;
            }
            tMarker->skipFreq = skipFreq;
            Vector_PushBack(profile->topMarkerList, tMarker);
        }
        else
        {
            char *splitSuffix = NULL;
            char *accumulateSuffix = NULL;
            // T2Debug("Adding Grep Marker :: Param/Marker Name : %s ref/pattern/Comp : %s fileName : %s skipFreq : %d\n", name, ref, fileName, skipFreq);
            GrepMarker *gMarker = (GrepMarker *)malloc(sizeof(GrepMarker));
            memset(gMarker, 0, sizeof(GrepMarker));
            gMarker->markerName = strdup(name);
            gMarker->searchString = strdup(ref);
            gMarker->logFile = strdup(fileName);
            gMarker->firstSeekFromEOF = 0;// memset will already set to 0 just a safeguard
            gMarker->regexParam = NULL;
            splitSuffix = strstr(name, SPLITMARKER_SUFFIX);
            accumulateSuffix = strstr(name, ACCUMULATE_MARKER_SUFFIX);
            if(splitSuffix != NULL && strcmp(splitSuffix, SPLITMARKER_SUFFIX) == 0)
            {
                gMarker->mType = MTYPE_ABSOLUTE;
                gMarker->u.markerValue = NULL;
            }
            else if(accumulateSuffix != NULL && strcmp(accumulateSuffix, ACCUMULATE_MARKER_SUFFIX) == 0)
            {
                gMarker->mType = (MarkerType)MTYPE_XCONF_ACCUMULATE;
                Vector_Create(&gMarker->u.accumulatedValues);
            }
            else
            {
                gMarker->mType = MTYPE_COUNTER;
                gMarker->u.count = 0;
            }
            gMarker->skipFreq = skipFreq;
            Vector_PushBack(profile->gMarkerList, gMarker);
        }
    }
    profile->paramNumOfEntries++;
    return T2ERROR_SUCCESS;
}

T2ERROR processConfigurationXConf(char* configData, ProfileXConf **localProfile)
{
    // configData is the raw data from curl http get request to xconf

    T2Debug("%s ++in\n", __FUNCTION__);

    T2ERROR ret = T2ERROR_SUCCESS;
    int marker_count = 0;
    T2Debug("config data = %s\n", configData);
    cJSON *json_root = cJSON_Parse(configData);
    cJSON *telemetry_data = cJSON_GetObjectItem(json_root, "urn:settings:TelemetryProfile");

//    cJSON *jprofileID = cJSON_GetObjectItem(telemetry_data, "id");
    cJSON *jprofileName = cJSON_GetObjectItem(telemetry_data, "telemetryProfile:name");
    cJSON *juploadUrl = cJSON_GetObjectItem(telemetry_data, "uploadRepository:URL");
    cJSON *jschedule = cJSON_GetObjectItem(telemetry_data, "schedule");

    cJSON *profileData = cJSON_GetObjectItem(telemetry_data, "telemetryProfile");
    if(profileData != NULL)
    {
        marker_count = cJSON_GetArraySize(profileData);
    }
//    if(jprofileID)
//        T2Debug("profile id = %s\n", jprofileID->valuestring);
    if(jprofileName)
    {
        T2Debug("profile name = %s\n", jprofileName->valuestring);
    }
    if(juploadUrl)
    {
        T2Debug("upload url = %s\n", juploadUrl->valuestring);
    }
    if(jschedule)
    {
        T2Debug("schedule = %s\n", jschedule->valuestring);
    }
    T2Debug("marker count = %d\n", marker_count);
//    if(jprofileID == NULL || jprofileName == NULL || juploadUrl == NULL || jschedule == NULL || profileData == NULL || marker_count == 0)
    if(jprofileName == NULL || juploadUrl == NULL || jschedule == NULL || profileData == NULL || marker_count == 0)
    {
        T2Error("Incomplete profile information, unable to create profile\n");
        cJSON_Delete(json_root);
        return T2ERROR_FAILURE;
    }

    int reportIntervalInSec = getScheduleInSeconds(jschedule->valuestring);

    T2Info("Received profile name : %s with interval of : %d secs and upload url : %s \n", jprofileName->valuestring, reportIntervalInSec, juploadUrl->valuestring);

    ProfileXConf *profile = (ProfileXConf *)malloc(sizeof(ProfileXConf));
    memset(profile, 0, sizeof(ProfileXConf));
//    profile->id = strdup(jprofileID->valuestring);
    profile->name = strdup(jprofileName->valuestring);
    profile->reportingInterval = reportIntervalInSec;
    profile->isUpdated = false;

    profile->protocol = strdup("HTTP");
    profile->t2HTTPDest = (T2HTTP *)malloc(sizeof(T2HTTP));

    profile->t2HTTPDest->URL = strdup(juploadUrl->valuestring);
    profile->encodingType = strdup("JSON");

    profile->grepSeekProfile = createGrepSeekProfile(0);

    Vector_Create(&profile->paramList);
    Vector_Create(&profile->eMarkerList);
    Vector_Create(&profile->gMarkerList);
    Vector_Create(&profile->topMarkerList);
    Vector_Create(&profile->cachedReportList);

#if defined(PRIVACYMODES_CONTROL)
    char* paramValue = NULL;
    getPrivacyMode(&paramValue);
    if(strncmp(paramValue, "DO_NOT_SHARE", MAX_PARAM_LEN) == 0)
    {
        addParameter(profile, "PrivacyMode", PRIVACYMODES_RFC, ALIAS_DATAMODEL, -1);
    }
    else
    {
        addParameter(profile, "mac", TR181_DEVICE_WAN_MAC, ALIAS_DATAMODEL, -1);
        addParameter(profile, "StbIp", TR181_DEVICE_WAN_IPv6, ALIAS_DATAMODEL, -1);
        isWhoAmiEnabled() ? addParameter(profile, "PartnerId", TR181_DEVICE_PARTNER_NAME, ALIAS_DATAMODEL, -1) : addParameter(profile, "PartnerId", TR181_DEVICE_PARTNER_ID, ALIAS_DATAMODEL, -1);
        addParameter(profile, "Version", TR181_DEVICE_FW_VERSION, ALIAS_DATAMODEL, -1);
        addParameter(profile, "AccountId", TR181_DEVICE_ACCOUNT_ID, ALIAS_DATAMODEL, -1);
        addParameter(profile, "immui_ver_split", TR181_IUI_VERSION, ALIAS_DATAMODEL, -1);
    }
    free(paramValue);
    paramValue = NULL;
#else
    addParameter(profile, "mac", TR181_DEVICE_WAN_MAC, ALIAS_DATAMODEL, -1);
#if defined(ENABLE_RDKB_SUPPORT)
    addParameter(profile, "erouterIpv4", TR181_DEVICE_WAN_IPv4, ALIAS_DATAMODEL, -1);
    addParameter(profile, "erouterIpv6", TR181_DEVICE_WAN_IPv6, ALIAS_DATAMODEL, -1);
#elif defined (ENABLE_RDKC_SUPPORT)
    addParameter(profile, "camIp", TR181_DEVICE_WAN_IPv4, ALIAS_DATAMODEL, -1);
    addParameter(profile, "camIpv6", TR181_DEVICE_WAN_IPv6, ALIAS_DATAMODEL, -1);
#else
    addParameter(profile, "StbIp", TR181_DEVICE_WAN_IPv6, ALIAS_DATAMODEL, -1);
    addParameter(profile, "immui_ver_split", TR181_IUI_VERSION, ALIAS_DATAMODEL, -1);
#endif
    isWhoAmiEnabled() ? addParameter(profile, "PartnerId", TR181_DEVICE_PARTNER_NAME, ALIAS_DATAMODEL, -1) : addParameter(profile, "PartnerId", TR181_DEVICE_PARTNER_ID, ALIAS_DATAMODEL, -1);
    addParameter(profile, "Version", TR181_DEVICE_FW_VERSION, ALIAS_DATAMODEL, -1);
    addParameter(profile, "AccountId", TR181_DEVICE_ACCOUNT_ID, ALIAS_DATAMODEL, -1);
#endif
    int markerIndex = 0;
    char* header = NULL;
    char* content = NULL;
    char* logfile = NULL;
    int skipFrequency = 0;
    size_t profileParamCount = 0;
    for (markerIndex = 0; markerIndex < marker_count; markerIndex++)
    {

        cJSON* pSubitem = cJSON_GetArrayItem(profileData, markerIndex);
        if (pSubitem != NULL)
        {
            cJSON* jHeader = cJSON_GetObjectItem(pSubitem, "header");
            cJSON* jContent = cJSON_GetObjectItem(pSubitem, "content");
            cJSON* jLogfile = cJSON_GetObjectItem(pSubitem, "type");
            cJSON* jSkipFrequency = cJSON_GetObjectItem(pSubitem, "pollingFrequency");
            if(jHeader)
            {
                header = jHeader->valuestring;
            }

            if(jContent)
            {
                content = jContent->valuestring;
            }

            if(jLogfile)
            {
                logfile = jLogfile->valuestring;
            }

            if(jSkipFrequency)
            {
                skipFrequency = atoi(jSkipFrequency->valuestring);
            }
            else
            {
                skipFrequency = 0 ;
            }

            if(header != NULL && content != NULL && logfile != NULL)
            {
                if(!strncmp(logfile, MT_TR181PARAM_PATTERN, MT_TR181PATAM_PATTERN_LENGTH))
                {
                    ret = addParameter(profile, header, content, ALIAS_DATAMODEL, skipFrequency);
                }
                else if(!strncmp(logfile, MT_EVENT_PATTERN, MT_EVENT_PATTERN_LENGTH))
                {
                    ret = addParameter(profile, header, content, NULL, 0); //skip freq is not supported for event markers
                }
                else
                {
                    ret = addParameter(profile, header, content, logfile, skipFrequency);
                }
            }
            if (ret != T2ERROR_SUCCESS)
            {
                T2Error("%s Error in adding parameter to profile %s \n", __FUNCTION__, profile->name);
                continue;
            }
            profileParamCount++;
        }
    }
    // Legacy DCA utils expects the list to be sorted based on logfile names
    Vector_Sort(profile->gMarkerList,  sizeof(GrepMarker*), compareLogFileNames);

    T2Info("Number of tr181params/markers successfully added in profile = %lu \n", (unsigned long)profileParamCount);

    cJSON_Delete(json_root);

#ifdef PERSIST_LOG_MON_REF
    profile->saveSeekConfig = true;
    profile->checkPreviousSeek = false;
#else
    profile->saveSeekConfig = false;
    profile->checkPreviousSeek = false;
#endif

    *localProfile = profile;

    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}
