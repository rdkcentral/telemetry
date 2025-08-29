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

#ifndef _BULKDATA_H_
#define _BULKDATA_H_

#include <stdbool.h>
#include <cjson/cJSON.h>
#include "telemetry2_0.h"
#include "t2eventreceiver.h"

//Including Webconfig Framework For Telemetry 2.0 As part of RDKB-28897
#if defined(FEATURE_SUPPORT_WEBCONFIG)
#include <webconfig_framework.h>
#include <webconfig_err.h>
#endif

#define MIN_REPORT_INTERVAL     10
#define MAX_PARAM_REFERENCES    100
#define DEFAULT_MAX_REPORT_SIZE 51200
#define MAX_CACHED_REPORTS 5


typedef enum
{
    XML,
    XDR,
    CSV,
    JSON,
    MESSAGE_PACK
} ENCODING_TYPE;


typedef enum
{
    JSONRF_OBJHIERARCHY,
    JSONRF_KEYVALUEPAIR
} JSONFormat;

typedef enum
{
    TIMESTAMP_UNIXEPOCH,
    TIMESTAMP_ISO_8601,
    TIMESTAMP_NONE
} TimeStampFormat;

typedef struct _BulkData
{
    bool enable;
    unsigned int minReportInterval;
    char *protocols;
    char *encodingTypes;
    bool parameterWildcardSupported;
    int maxNoOfParamReferences;
    unsigned int maxReportSize;
} BulkData;

void ReportProfiles_ActivationTimeoutCb(char* profileName);

void ReportProfiles_TimeoutCb(char* profileName, bool isClearSeekMap);

typedef struct _ReportProfile
{
    char *hash;
    char *config;
} ReportProfile;

T2ERROR initReportProfiles();

T2ERROR ReportProfiles_uninit();

T2ERROR deleteAllReportProfiles();

void ReportProfiles_ProcessReportProfilesBlob(cJSON *profiles_root, bool rprofiletypes);

void ReportProfiles_Interrupt();

void generateDcaReport(bool isDelayed, bool isOnDemand);

/* MSGPACK Declarations */

struct __msgpack__
{
    char *msgpack_blob;
    int msgpack_blob_size;
};

int __ReportProfiles_ProcessReportProfilesMsgPackBlob(void *msgpack, bool checkPreviousSeek);

void ReportProfiles_ProcessReportProfilesMsgPackBlob(char *msgpack_blob, int msgpack_blob_size);

T2ERROR ReportProfiles_storeMarkerEvent(char *profileName, T2Event *eventInfo);

T2ERROR privacymode_do_not_share ();

T2ERROR ReportProfiles_deleteProfile(const char* profileName);

bool isMtlsEnabled(void);

void profilemem_usage(unsigned int *value);

void T2totalmem_calculate();
#if defined(PRIVACYMODES_CONTROL)
void createPrivacyModepath();
#endif

#define msgpack_get_obj_name(obj) #obj

#define MSGPACK_GET_ARRAY_SIZE(obj, item)		\
    if (obj && MSGPACK_OBJECT_ARRAY == obj->type)	\
        item = obj->via.array.size

#define MSGPACK_GET_U64(obj, item)				     \
    if (NULL != obj && MSGPACK_OBJECT_POSITIVE_INTEGER == obj->type) \
        item = obj->via.u64

#define MSGPACK_GET_BOOLEAN(obj, item)					\
    if (NULL != obj && MSGPACK_OBJECT_BOOLEAN == obj->type)		\
        item = obj->via.boolean

#define MSGPACK_GET_NUMBER(obj, item) do {				\
    if(NULL == obj)							\
	; /* Do nothing if obj is NULL*/				\
    else if (MSGPACK_OBJECT_BOOLEAN == obj->type)			\
	item = obj->via.boolean;					\
    else if (MSGPACK_OBJECT_POSITIVE_INTEGER == obj->type)		\
	item = obj->via.u64;						\
    else if (MSGPACK_OBJECT_NEGATIVE_INTEGER == obj->type)		\
	item = obj->via.i64;						\
    else if (MSGPACK_OBJECT_FLOAT == obj->type)				\
	item = obj->via.f64;						\
    } while(0)

#endif /* _BULKDATA_H_ */
