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

#ifndef _T2COMMON_H_
#define _T2COMMON_H_

#include <stdbool.h>
#include <stdio.h>
#include <regex.h>
#include "vector.h"

#define MAX_ACCUMULATE 20
#define MAX_ACCUMULATE_MSG "maximum accumulation reached"
#define MAX_DEVICE_PROP_BUFF_SIZE 80
#define DEVICE_PROPERTIES_FILE  "/etc/device.properties"


typedef enum
{
    MTYPE_NONE,
    MTYPE_COUNTER,
    MTYPE_ABSOLUTE,
    MTYPE_ACCUMULATE
} MarkerType;

typedef enum
{
    REPORTTIMESTAMP_UNIXEPOCH = 1,
    REPORTTIMESTAMP_NONE
} reportTimestampFormat;

typedef struct _Param
{
    bool reportEmptyParam;
    char* paramType;
    char* name;
    const char* alias;
    bool trimParam;
    char* regexParam;
} Param;

typedef struct _StaticParam
{
    char* paramType;
    char* name;
    char* value;
} StaticParam;

typedef struct _EventMarker
{
    bool reportEmptyParam;
    bool trimParam;
    char* alias;
    char* paramType;
    char* markerName;
    char* compName;
    reportTimestampFormat reportTimestampParam;
    char* timestamp;
    char* markerName_CT;
    char* regexParam;
    MarkerType mType;
    union
    {
        unsigned int count;
        char* markerValue;
        Vector* accumulatedValues;
    } u;
    Vector* accumulatedTimestamp;
    unsigned int skipFreq;
} EventMarker;


typedef struct _GrepMarker
{
    bool reportEmptyParam;
    bool trimParam;
    char* paramType;
    char* markerName;
    char* searchString;
    char* logFile;
    char* regexParam;
    MarkerType mType;
    union
    {
        unsigned int count;
        char* markerValue;
    } u;
    unsigned int skipFreq;
    int firstSeekFromEOF;
} GrepMarker;

typedef struct _TriggerCondition
{
    char* type;
    char* oprator;
    int threshold;
    int minThresholdDuration;
    char* reference;
    bool isSubscribed;
    bool report;
} TriggerCondition;

void freeParam(void *data);

void freeStaticParam(void *data);

void freeEMarker(void *data);

void freeGMarker(void *data);

void freeTriggerCondition(void *data);

void freeAccumulatedParam(void *data);

int compareLogFileNames(const void *g1, const void *g2);

bool getDevicePropertyData(const char *dev_prop_name, char *out_data, unsigned int buff_size);

void initWhoamiSupport(void);

bool isWhoAmiEnabled(void);

#endif /* _T2COMMON_H_ */
