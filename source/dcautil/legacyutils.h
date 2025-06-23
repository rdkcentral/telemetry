/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2016 RDK Management
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

/**
 * @defgroup dca
 * @{
 **/

/**
 * @defgroup dca
 * @{
 * @defgroup src
 * @{
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <cjson/cJSON.h>

#include "t2collection.h"
#include "vector.h"
#include "telemetry2_0.h"


#define MAXLINE 1024
#define MAXLEN 512
#define LEN 14

#define USLEEP_SEC 100
#define MAX_PROCESS 1
#define RDK_EC_MAXLEN 5 /* RDK Error code maximum length */

#define INCLUDE_PROPERTIES "/etc/include.properties"
#define DEVICE_PROPERTIES "/etc/device.properties"

#define DEFAULT_SEEK_PREFIX "/opt/.telemetry/tmp/rtl_"
#if defined(DEVICE_EXTENDER)
#define DEFAULT_LOG_PATH "/var/log/messages"
#else
#define DEFAULT_LOG_PATH "/opt/logs/"
#endif

/*
 * Used to store the log file seek position for each profile. 
 * The key is the profile name and 
 *   value is a hash_map_t which contains the log file name as key and the seek position as value.
 */
typedef struct _GrepSeekProfile
{
    hash_map_t *logFileSeekMap;
    int execCounter;
} GrepSeekProfile;


/** If this can access the Profile object, then seek map can be updated accordingly */

extern cJSON *SEARCH_RESULT_JSON;
extern cJSON *ROOT_JSON;

/* utility functions */
int getLoadAvg(Vector* grepResultList, bool trim, char* regex);

void removeProfileFromSeekMap(char *profileName);

void removeProfileFromExecMap(char *profileName);

GrepSeekProfile *addToProfileSeekMap(char* profileName);

GrepSeekProfile *getLogSeekMapForProfile(char* profileName);

/**
 * Get log line from log file including the rotated log file if applicable
 */
char* getLogLine(hash_map_t *logSeekMap, char *buf, int buflen, char *name, int *seekFromEOF, bool check_rotated_logs); //SERXIONE-4074: Missing markers in telemetry json

void clearConfVal(void);

void updatePropsFromIncludeFile(char *logpath, char *perspath);

void initProperties(char *logpath, char *perspath, long *pagesize);

T2ERROR updateLogSeek(hash_map_t *logSeekMap, const char *name, const long logfileSize);


/* JSON functions */
void initSearchResultJson(cJSON **root, cJSON **sr);

void addToSearchResult(char *key, char *value);

void clearSearchResultJson(cJSON **root);

int getProcUsage(char *processName, Vector* grepResultList, bool trim, char* regex, char* filename);

bool isPropsInitialized();

/** @} */

/** @} */
/** @} */
/** @} */

/** @} */
/** @} */
