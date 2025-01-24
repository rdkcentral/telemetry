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

#ifndef _PROFILEXCONF_H_
#define _PROFILEXCONF_H_

#include <stdbool.h>
#include <pthread.h>
#include <cjson/cJSON.h>
#include "telemetry2_0.h"
#include "reportprofiles.h"
#include "t2common.h"
#include "t2eventreceiver.h"
#include "vector.h"
#include "reportgen.h"

typedef enum
{
    MTYPE_XCONF_NONE,
    MTYPE_XCONF_COUNTER,
    MTYPE_XCONF_ABSOLUTE,
    MTYPE_XCONF_ACCUMULATE
}MarkerTypeXConf;

typedef struct _JSONEncodingXConf
{
    JSONFormat reportFormat;
    TimeStampFormat tsFormat;
}JSONEncodingXConf;

typedef struct _ProfileXConf
{
    bool isUpdated;
    bool reportInProgress;
    bool bClearSeekMap;
    char* name;
    char* protocol;
    char* encodingType;
    unsigned int reportingInterval;
    unsigned int timeRef;
    unsigned int paramNumOfEntries;
    Vector *paramList;
    T2HTTP *t2HTTPDest;
    Vector *eMarkerList;
    Vector *gMarkerList;
    Vector *cachedReportList;
    cJSON *jsonReportObj;
    pthread_t reportThread;
}ProfileXConf;

T2ERROR ProfileXConf_init();
T2ERROR ProfileXConf_uninit();
T2ERROR ProfileXConf_set(ProfileXConf *profile);
T2ERROR ProfileXConf_delete(ProfileXConf *profile);
bool    ProfileXConf_isSet();
bool    ProfileXConf_isNameEqual(char* profileName);
void    ProfileXConf_updateMarkerComponentMap();
void    ProfileXConf_notifyTimeout(bool isClearSeekMap, bool isOnDemand);
T2ERROR ProfileXConf_storeMarkerEvent(T2Event *eventInfo);
char*   ProfileXconf_getName();
T2ERROR ProfileXConf_terminateReport();


#endif /* _PROFILE_H_ */
