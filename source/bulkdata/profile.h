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

#ifndef _PROFILE_H_
#define _PROFILE_H_

#include <stdbool.h>
#include <pthread.h>
#include <cjson/cJSON.h>

#include "t2collection.h"
#include "telemetry2_0.h"
#include "reportprofiles.h"
#include "t2eventreceiver.h"
#include "t2common.h"
#include "vector.h"
#include "reportgen.h"

typedef struct _JSONEncoding
{
    JSONFormat reportFormat;
    TimeStampFormat tsFormat;
} JSONEncoding;

typedef struct _Profile
{
    bool enable;
    bool isSchedulerstarted;
    bool isUpdated;
    bool reportInProgress;
    bool generateNow;
    bool deleteonTimeout;
    bool bClearSeekMap;
    bool checkPreviousSeek; // To support Previous_Logs report post reboot
    bool saveSeekConfig; // To save the Seek config to persistant storage
    bool triggerReportOnCondition;
    bool trim;
    void (*callBackOnReportGenerationComplete)(char*);
    char* hash;
    char* name;
    char* protocol;
    char* encodingType;
    char* Description;
    char* version;
    char* RootName;
    char* timeRef;
    JSONEncoding *jsonEncoding;
    bool reportOnUpdate;
    unsigned int firstReportingInterval;
    unsigned int SendErr;
    unsigned int reportingInterval;
    unsigned int activationTimeoutPeriod;
    unsigned int maxUploadLatency;
    unsigned int paramNumOfEntries;
    unsigned int minThresholdDuration;
    Vector *paramList;
    Vector *staticParamList;
    T2HTTP *t2HTTPDest;
    T2RBUS *t2RBUSDest;
    Vector *eMarkerList;
    Vector *gMarkerList;
    Vector *topMarkerList;
    Vector *cachedReportList;
    cJSON *jsonReportObj;
    pthread_t reportThread;
    pthread_mutex_t triggerCondMutex;
    pthread_mutex_t eventMutex;
    pthread_mutex_t reportMutex;
    pthread_cond_t reportcond;
    struct timespec currentTime;
    struct timespec maxlatencyTime;
    Vector *triggerConditionList;
    pthread_cond_t reuseThread;
    pthread_mutex_t reuseThreadMutex;
    bool threadExists;
} Profile;

T2ERROR initProfileList(bool checkPreviousSeek);

T2ERROR uninitProfileList();

T2ERROR addProfile(Profile *profile);

int getProfileCount();

T2ERROR profileWithNameExists(const char *profileName, bool *bProfileExists);

T2ERROR Profile_storeMarkerEvent(const char *profileName, T2Event *eventInfo);

T2ERROR enableProfile(const char *profileName);

T2ERROR disableProfile(const char *profileName, bool *isDeleteRequired);

T2ERROR deleteProfile(const char *profileName);

T2ERROR deleteAllProfiles(bool delFromDisk);

void updateMarkerComponentMap();

hash_map_t *getProfileHashMap();

void sendLogUploadInterruptToScheduler();

void NotifyTimeout(const char* profileName, bool isClearSeekMap);

void getMarkerCompRbusSub(bool subscription);

bool isProfileEnabled(const char *profileName);

T2ERROR registerTriggerConditionConsumer();

T2ERROR triggerReportOnCondtion(const char *referenceName, const char *referenceValue);

unsigned int getMinThresholdDuration(char *profileName);

void reportGenerationCompleteReceiver(char* profileName);

void NotifySchedulerstart(char* profileName, bool isschedulerstarted);
#endif /* _PROFILE_H_ */
