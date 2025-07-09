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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "dca.h"
#include "dcautil.h"
#include "telemetry2_0.h"
#include "t2log_wrapper.h"
#include "t2common.h"
#include "legacyutils.h"
#include "persistence.h"

/**
 * @brief Get the Grep Results object. Main function called by rest of the consumers.
 *
 * @param profileName
 * @param markerList
 * @param grepResultList
 * @param isClearSeekMap
 * @param check_rotated
 * @param customLogPath
 * @return T2ERROR
 */

T2ERROR
getGrepResults (char *profileName, Vector *markerList, Vector **grepResultList, bool isClearSeekMap, bool check_rotated, char *customLogPath)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if(profileName == NULL || markerList == NULL || grepResultList == NULL)
    {
        T2Error("Invalid Args or Args are NULL\n");
        return T2ERROR_FAILURE;
    }

    getDCAResultsInVector(profileName, markerList, grepResultList, check_rotated, customLogPath);
    if (isClearSeekMap)
    {
        removeProfileFromSeekMap(profileName);
    }

    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

void removeGrepConfig(char* profileName, bool clearSeekMap, bool clearExecMap)
{
    T2Debug("%s ++in\n", __FUNCTION__);

    if(clearSeekMap)
    {
        removeProfileFromSeekMap(profileName);
    }

    if (clearExecMap)
    {
        removeProfileFromExecMap(profileName);
    }
    T2Debug("%s ++out\n", __FUNCTION__);
}

// dcaFlagReportCompleation this function is used to create legacy DCA Flag DCADONEFLAG
void dcaFlagReportCompleation()
{
    T2Debug("%s --in creating flag %s\n", __FUNCTION__, DCADONEFLAG);
    FILE *fileCheck = fopen(DCADONEFLAG, "w+");
    if (fileCheck == NULL )
    {
        T2Error(" Error in creating the Flag :  %s\n", DCADONEFLAG);
    }
    else
    {
        fclose(fileCheck);
    }
    T2Debug("%s --out\n", __FUNCTION__);
}

# ifdef PERSIST_LOG_MON_REF
T2ERROR saveSeekConfigtoFile(char* profileName)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if(profileName == NULL)
    {
        T2Error("Profile Name is not available\n");
        return T2ERROR_FAILURE;
    }
    GrepSeekProfile *ProfileSeekMap = getLogSeekMapForProfile(profileName);
    if(ProfileSeekMap == NULL)
    {
        T2Error("ProfileSeekMap is NULL\n");
        return T2ERROR_FAILURE;
    }
    hash_map_t *logfileMap = ProfileSeekMap->logFileSeekMap;
    if(logfileMap == NULL)
    {
        T2Error("logfileMap is NULL\n");
        return T2ERROR_FAILURE;
    }

    unsigned int count = (unsigned int) hash_map_count(logfileMap);

    cJSON *valArray = cJSON_CreateArray();
    for (unsigned int i = 0; i < count ; i++)
    {
        char *logFileName = NULL;
        long *seekvalue = NULL;
        logFileName = hash_map_lookupKey(logfileMap, i);
        seekvalue = hash_map_lookup(logfileMap, i);
        cJSON *logFileObj = cJSON_CreateObject();
        cJSON_AddNumberToObject(logFileObj, logFileName, (double)*seekvalue);
        cJSON_AddItemToArray(valArray, logFileObj);
    }
    char *jsonReport = cJSON_PrintUnformatted(valArray);
    if(T2ERROR_SUCCESS != saveConfigToFile(SEEKFOLDER, profileName, jsonReport))
    {
        T2Error("Failed to save config to file\n");
        cJSON_Delete(valArray);
        free(jsonReport);
        return T2ERROR_FAILURE;
    }
    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

T2ERROR loadSavedSeekConfig(char *profileName)
{
    T2Debug("%s ++in\n", __FUNCTION__);

    if(profileName == NULL)
    {
        T2Error("Profile Name is not available\n");
        return T2ERROR_FAILURE;
    }
    int len = strlen(profileName) + strlen(SEEKFOLDER) + 2;
    char *seekFile = (char *)malloc(len);
    snprintf(seekFile, len, "%s/%s", SEEKFOLDER, profileName);
    FILE *file = fopen(seekFile, "rb");
    if(file == NULL)
    {
        T2Error("Failed to open file\n");
        free(seekFile);
        return T2ERROR_FAILURE;
    }
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *data = malloc(fileSize + 1);
    if (data == NULL)
    {
        T2Error("Memory allocation failed\n");
        fclose(file);
        return T2ERROR_FAILURE;
    }
    fread(data, 1, fileSize, file);
    fclose(file);
    data[fileSize] = '\0';
    cJSON *json = cJSON_Parse(data);
    cJSON *item = NULL;
    GrepSeekProfile *ProfileSeekMap = NULL;
    ProfileSeekMap = (GrepSeekProfile *) getLogSeekMapForProfile(profileName);
    if (ProfileSeekMap == NULL)
    {
        ProfileSeekMap = (GrepSeekProfile *) addToProfileSeekMap(profileName);
    }
    cJSON_ArrayForEach(item, json)
    {
        // Each `item` is an object in the array
        if (item->child != NULL)
        {
            const char *key = item->child->string;
            cJSON *value = item->child;

            if (key != NULL)
            {
                // Check the value type and print it
                if (cJSON_IsNumber(value))
                {
                    long *tempnum;
                    double val = value->valuedouble;
                    tempnum = (long *)malloc(sizeof(long));
                    *tempnum = (long)val;
                    hash_map_put(ProfileSeekMap->logFileSeekMap, strdup(key), tempnum, NULL);
                    //printf("Key: %s, Value: %ld\n", key, *tempnum);
                }
            }

        }
    }
    cJSON_Delete(json);
    free(data);
    free(seekFile);
    return T2ERROR_SUCCESS;
    T2Debug("%s --out\n", __FUNCTION__);
}
#endif

bool firstBootStatus()
{
    T2Debug("%s ++in\n", __FUNCTION__);
    bool status = true;
    if(access(BOOTFLAG, F_OK) != -1)
    {
        status = false;
    }
    T2Debug("%s --out\n", __FUNCTION__);
    return status;
}
