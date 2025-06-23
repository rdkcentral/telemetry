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

#include "t2parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "xconfclient.h"
#include "reportprofiles.h"
#include "t2log_wrapper.h"
#include "rbusInterface.h"
#include "t2common.h"

#define DEFAULT_TIME_REFERENCE "0001-01-01T00:00:00Z"

static const int MAX_STATIC_PROP_VAL_LEN = 128 ;
char *msgpack_strdup(msgpack_object *obj);
msgpack_object *msgpack_get_map_value(msgpack_object *obj, char *key);
msgpack_object *msgpack_get_array_element(msgpack_object *obj, int index);


static char * getProfileParameter(Profile * profile, const char *ref)
{
    char *pValue = "NULL";
    char *pName = strrchr(ref, '.') + 1;

    pValue = (char*) calloc(MAX_STATIC_PROP_VAL_LEN, sizeof(char));
    if( pValue == NULL)
    {
        T2Error("Unable to allocate memory for profile parameter value \n");
        return pValue ;
    }

    if(!strcmp(pName, "Name"))
    {
        if(profile->name)
        {
            strncpy(pValue, profile->name, MAX_STATIC_PROP_VAL_LEN);
        }
    }
    else if(!strcmp(pName, "Version"))
    {
        if(profile->version)
        {
            strncpy(pValue, profile->version, MAX_STATIC_PROP_VAL_LEN);
        }
    }
    else if(!strcmp(pName, "Description"))
    {
        if(profile->Description)
        {
            strncpy(pValue, profile->Description, MAX_STATIC_PROP_VAL_LEN);
        }
    }
    else if(!strcmp(pName, "Protocol"))
    {
        if(profile->protocol)
        {
            strncpy(pValue, profile->protocol, MAX_STATIC_PROP_VAL_LEN);
        }
    }
    else if(!strcmp(pName, "RootName"))
    {
        if(profile->RootName)
        {
            strncpy(pValue, profile->RootName, MAX_STATIC_PROP_VAL_LEN);
        }
    }
    else if(!strcmp(pName, "EncodingType"))
    {
        if(profile->encodingType)
        {
            strncpy(pValue, profile->encodingType, MAX_STATIC_PROP_VAL_LEN);
        }
    }
    else if(!strcmp(pName, "ReportingInterval"))
    {
        if(profile->reportingInterval)
        {
            snprintf(pValue, MAX_STATIC_PROP_VAL_LEN, "%d", profile->reportingInterval);
        }
    }
    else if(!strcmp(pName, "TimeReference"))
    {
        if(profile->timeRef)
        {
            snprintf(pValue, MAX_STATIC_PROP_VAL_LEN, "%s", profile->timeRef);
        }
    }
    else if(!strcmp(pName, "ActivationTimeOut"))
    {
        if(profile->activationTimeoutPeriod)
        {
            snprintf(pValue, MAX_STATIC_PROP_VAL_LEN, "%d", profile->activationTimeoutPeriod);
        }
    }
    else
    {
        snprintf(pValue, MAX_STATIC_PROP_VAL_LEN, "No Such Parameter");
    }

    return pValue;
}

static T2ERROR addhttpURIreqParameter(Profile *profile, const char* Hname, const char* Href)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if(NULL == Hname || NULL == Href )
    {
        T2Error("Hname or Href is NULL\n");
        return T2ERROR_FAILURE ;
    }

    char* value = NULL;

    HTTPReqParam *httpreqparam = (HTTPReqParam *) malloc(sizeof(HTTPReqParam));
    if(!httpreqparam)
    {
        T2Error("failed to allocate memory \n");
        return T2ERROR_FAILURE;

    }
    httpreqparam->HttpRef = strdup(Href);
    httpreqparam->HttpName = strdup(Hname);
    if (strstr(Href, "Profile.") == Href)
    {
        value = getProfileParameter(profile, Href);
        httpreqparam->HttpValue = strdup(value);
    }
    else
    {
        httpreqparam->HttpValue = NULL;
    }

    if(value != NULL)
    {
        free(value);
        value = NULL;
    }
    Vector_PushBack(profile->t2HTTPDest->RequestURIparamList, httpreqparam);
    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

static T2ERROR addRbusMethodParameter(Profile *profile, const char* name, const char* value)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if(NULL == name || NULL == value )
    {
        return T2ERROR_SUCCESS ;
    }

    RBUSMethodParam *rbusMethodParam = (RBUSMethodParam *) malloc(sizeof(RBUSMethodParam));
    char* val = NULL;

    if(!rbusMethodParam)
    {
        T2Error("failed to allocate memory \n");
        return T2ERROR_FAILURE;

    }
    rbusMethodParam->name = strdup(name);
    if (strstr(value, "Profile.") == value)
    {
        val = getProfileParameter(profile, value);
        rbusMethodParam->value = strdup(val);
    }
    else
    {
        rbusMethodParam->value = strdup(value);
    }
    if(val != NULL)
    {
        free(val);
        val = NULL;
    }
    Vector_PushBack(profile->t2RBUSDest->rbusMethodParamList, rbusMethodParam);
    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

static T2ERROR addParameter(Profile *profile, const char* name, const char* ref, const char* fileName, int skipFreq, int firstSeekFromEOF, const char* ptype,
                            const char* use, bool ReportEmpty, reportTimestampFormat reportTimestamp, bool trim, const char* regex)
{

    T2Debug("%s ++in\n", __FUNCTION__);
    char* value = NULL;

    if(!(strcmp(ptype, "dataModel")))
    {
        // T2Debug("Adding TR-181 Parameter : %s\n", ref);

        if (strstr(ref, "Profile.") == ref)          //Static parameters from profile configuration
        {
            StaticParam *sparam = (StaticParam *) malloc(sizeof(StaticParam));
            if(sparam == NULL)
            {
                T2Error("Unable to allocate memory for Static \n");
                return T2ERROR_FAILURE;
            }
            sparam->paramType = strdup(ptype);
            sparam->name = strdup(name);
            value = getProfileParameter(profile, ref);
            sparam->value = strdup(value);
            if(value != NULL)
            {
                free(value);
                value = NULL;
            }

            Vector_PushBack(profile->staticParamList, sparam);
        }
        else
        {
            Param *param = (Param *) malloc(sizeof(Param));
            if(param == NULL)
            {
                T2Error("Unable to allocate memory for TR-181 Parameter \n");
                return T2ERROR_FAILURE;
            }
            param->name = strdup(name);
            param->alias = strdup(ref);
            param->paramType = strdup(ptype);
            param->reportEmptyParam = ReportEmpty;
            param->trimParam = trim; //RDKB-53161
            param->regexParam = NULL;
            if(regex != NULL)
            {
                param->regexParam = strdup(regex);
            }
            Vector_PushBack(profile->paramList, param);
        }
    }
    else if(!(strcmp(ptype, "event")))
    {
        // T2Debug("Adding Event Marker :: Param/Marker Name : %s ref/pattern/Comp : %s skipFreq : %d\n", name, ref, skipFreq);
        EventMarker *eMarker = (EventMarker *) malloc(sizeof(EventMarker));
        if(eMarker == NULL)
        {
            T2Error("Unable to allocate memory for EventMarker \n");
            return T2ERROR_FAILURE;
        }
        eMarker->markerName = strdup(name);
        eMarker->compName = strdup(ref);
        if(fileName)
        {
            eMarker->alias = strdup(fileName);
        }
        else
        {
            eMarker->alias = NULL;
        }
        eMarker->paramType = strdup(ptype);
        eMarker->reportEmptyParam = ReportEmpty;
        eMarker->trimParam = trim; //RDKB-53161
        eMarker->markerName_CT = NULL;
        eMarker->timestamp = NULL;
        eMarker->regexParam = NULL;
        if(regex != NULL)
        {
            eMarker->regexParam = strdup(regex);
        }
        eMarker->reportTimestampParam = reportTimestamp;
        if((use == NULL) || (0 == strcmp(use, "absolute")))
        {
            eMarker->mType = MTYPE_ABSOLUTE;
            eMarker->u.markerValue = NULL;
        }
        else if (0 == strcmp(use, "count"))
        {
            eMarker->mType = MTYPE_COUNTER;
            eMarker->u.count = 0;
        }
        else if (0 == strcmp(use, "accumulate"))
        {
            eMarker->mType = MTYPE_ACCUMULATE;
            Vector_Create(&eMarker->u.accumulatedValues);
            if(eMarker->reportTimestampParam == REPORTTIMESTAMP_UNIXEPOCH)
            {
                Vector_Create(&eMarker->accumulatedTimestamp);
            }
        }
        else
        {
            T2Info("Unsupported marker type. Defaulting to absolute \n");
            eMarker->mType = MTYPE_ABSOLUTE;
            eMarker->u.markerValue = NULL;
        }
        eMarker->skipFreq = skipFreq;

        Vector_PushBack(profile->eMarkerList, eMarker);
    }
    else    //Grep Marker
    {

        // T2Debug("Adding Grep Marker :: Param/Marker Name : %s ref/pattern/Comp : %s fileName : %s skipFreq : %d\n", name, ref, fileName, skipFreq);

        GrepMarker *gMarker = (GrepMarker *) malloc(sizeof(GrepMarker));
        if(gMarker == NULL)
        {
            T2Error("Unable to allocate memory for GrepMarker \n");
            return T2ERROR_FAILURE;
        }
        gMarker->markerName = strdup(name);
        gMarker->searchString = strdup(ref);
        if(fileName)
        {
            gMarker->logFile = strdup(fileName);
        }
        else
        {
            gMarker->logFile = NULL;
        }
        gMarker->paramType = strdup(ptype);
        gMarker->reportEmptyParam = ReportEmpty;
        gMarker->trimParam = trim;
        gMarker->regexParam = NULL;
        if(regex != NULL)
        {
            gMarker->regexParam = strdup(regex);
        }
        if((use == NULL) || (0 == strcmp(use, "absolute")))
        {
            gMarker->mType = MTYPE_ABSOLUTE;
            gMarker->u.markerValue = NULL;
        }
        else if (0 == strcmp(use, "count"))
        {
            gMarker->mType = MTYPE_COUNTER;
            gMarker->u.count = 0;
        }
        else
        {
            T2Info("Unsupported marker type. Defaulting to absolute \n");
            gMarker->mType = MTYPE_ABSOLUTE;
            gMarker->u.markerValue = NULL;
        }
        gMarker->skipFreq = skipFreq;
        gMarker->firstSeekFromEOF = firstSeekFromEOF;
        Vector_PushBack(profile->gMarkerList, gMarker);
#ifdef PERSIST_LOG_MON_REF
        profile->saveSeekConfig = true;
#endif
    }

    profile->paramNumOfEntries++;

    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

T2ERROR addMsgPckTriggerCondition(Profile *profile, msgpack_object *value_map)
{

    msgpack_object *TriggerConditionArray;
    msgpack_object *tcArrayMap;
    msgpack_object *tcTypeStr;
    msgpack_object *tcOperatorStr;
    msgpack_object *tcReferenceStr;
    msgpack_object *tcThresholdInt;
    msgpack_object *tcMinThresholdDurationInt;
    msgpack_object *tcReportbool;
    uint32_t TcArraySize = 0;
    char* tcType = NULL;
    char* tcOperator = NULL;
    char* tcReference = NULL;
    int tcThreshold = 0;
    int tcMinThresholdDuration = 0;
    uint32_t i;
    bool tcReport;

    if((profile == NULL) || (value_map == NULL))
    {
        T2Error("%s NULL value returning \n", __FUNCTION__);
        return T2ERROR_FAILURE;
    }
    T2Debug("%s ++in\n", __FUNCTION__);

    TriggerConditionArray = msgpack_get_map_value(value_map, "TriggerCondition");
    MSGPACK_GET_ARRAY_SIZE(TriggerConditionArray, TcArraySize);

    if(TcArraySize)
    {
        Vector_Create(&profile->triggerConditionList);
    }

    for( i = 0; i < TcArraySize; i++ )
    {

        tcType = NULL;
        tcOperator = NULL;
        tcReference = NULL;

        tcThreshold = 0;
        tcMinThresholdDuration = 0;
        tcReport = true;

        TriggerCondition *triggerCondition = (TriggerCondition *) malloc(sizeof(TriggerCondition));

        if(triggerCondition == NULL)
        {
            T2Error("%s Error adding MsgPck Trigger Condition to profile %s \n", __FUNCTION__, profile->name);
            T2Debug("%s ++out\n", __FUNCTION__);
            return T2ERROR_FAILURE;
        }

        memset(triggerCondition, 0, sizeof(TriggerCondition));

        tcArrayMap = msgpack_get_array_element(TriggerConditionArray, i);

        tcTypeStr = msgpack_get_map_value(tcArrayMap, "type");
        if(tcTypeStr)
        {
            msgpack_print(tcTypeStr, msgpack_get_obj_name(tcTypeStr));
            tcType = msgpack_strdup(tcTypeStr);
            /* CID 175341: Explicit null dereferenced */
            if(tcType == NULL)
            {
                T2Debug("Null tcType %s ++out\n", __FUNCTION__);
                free(triggerCondition);
                return T2ERROR_FAILURE;
            }

        }

        tcOperatorStr = msgpack_get_map_value(tcArrayMap, "operator");
        if(tcOperatorStr)
        {

            msgpack_print(tcOperatorStr, msgpack_get_obj_name(tcOperatorStr));
            tcOperator = msgpack_strdup(tcOperatorStr);
            /* CID 175346: Explicit null dereferenced */
            if(tcOperator == NULL)
            {
                T2Debug("Null tcOperator %s ++out\n", __FUNCTION__);
                free(triggerCondition);
                return T2ERROR_FAILURE;
            }

        }

        tcReferenceStr = msgpack_get_map_value(tcArrayMap, "reference");
        if(tcReferenceStr)
        {
            msgpack_print(tcReferenceStr, msgpack_get_obj_name(tcReferenceStr));
            tcReference = msgpack_strdup(tcReferenceStr);
            /* CID 175347 : Explicit null dereferenced */
            if(tcReference == NULL)
            {
                T2Debug("Null tcReference %s ++out\n", __FUNCTION__);
                free(triggerCondition);
                return T2ERROR_FAILURE;
            }

        }

        tcThresholdInt = msgpack_get_map_value(tcArrayMap, "threshold");
        if(tcThresholdInt)
        {
            msgpack_print(tcThresholdInt, msgpack_get_obj_name(tcThresholdInt));
            MSGPACK_GET_NUMBER(tcThresholdInt, tcThreshold);
            T2Debug("tcThreshold: %d\n", tcThreshold);
        }

        tcMinThresholdDurationInt = msgpack_get_map_value(tcArrayMap, "minThresholdDuration");
        if(tcMinThresholdDurationInt)
        {
            msgpack_print(tcMinThresholdDurationInt, msgpack_get_obj_name(tcMinThresholdDurationInt));
            MSGPACK_GET_NUMBER(tcMinThresholdDurationInt, tcMinThresholdDuration);
            T2Debug("tcMinThresholdDuration: %d\n", tcMinThresholdDuration);
        }

        tcReportbool = msgpack_get_map_value(tcArrayMap, "report");
        if(tcReportbool)
        {
            msgpack_print(tcReportbool, msgpack_get_obj_name(tcReportbool));
            MSGPACK_GET_NUMBER(tcReportbool, tcReport);
            T2Debug("tcReport: %s\n", (tcReport ? "True" : "False"));
        }

        T2Debug("Adding MsgPck Trigger Condition:%s type %s operator %s \n", tcReference, tcType, tcOperator);

        if(tcType)
        {
            triggerCondition->type = strdup(tcType);
        }
        if(tcOperator)
        {
            triggerCondition->oprator = strdup(tcOperator);
        }
        if(tcReference)
        {
            triggerCondition->reference = strdup(tcReference);
        }

        triggerCondition->threshold = tcThreshold;
        triggerCondition->minThresholdDuration = tcMinThresholdDuration;
        triggerCondition->isSubscribed = false;
        triggerCondition->report = tcReport;

        Vector_PushBack(profile->triggerConditionList, triggerCondition);
        T2Debug("[[Added MsgPck Trigger Condition:%s]]\n", tcReference);

        if(tcType)
        {
            free(tcType);
        }
        if(tcOperator)
        {
            free(tcOperator);
        }
        if(tcReference)
        {
            free(tcReference);
        }
    }
    T2Debug("%s ++out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

T2ERROR addTriggerCondition(Profile *profile, cJSON *jprofileTriggerCondition)
{

    T2Debug("%s ++in\n", __FUNCTION__);
    int triggerConditionIndex = 0;
    int triggerCondition_count = 0;
    if(jprofileTriggerCondition == NULL)
    {
        T2Debug("%s ++out\n", __FUNCTION__);
        return T2ERROR_FAILURE;
    }

    triggerCondition_count = cJSON_GetArraySize(jprofileTriggerCondition);
    Vector_Create(&profile->triggerConditionList);

    for( triggerConditionIndex = 0; triggerConditionIndex < triggerCondition_count; triggerConditionIndex++ )
    {
        char *tcType = NULL;
        char *tcOperator = NULL;
        int tcThreshold = 0;
        int tcMinThresholdDuration = 0;
        char *tcReference = NULL;
        bool tcReport = true;

        cJSON* tcSubitem = cJSON_GetArrayItem(jprofileTriggerCondition, triggerConditionIndex);

        if(tcSubitem != NULL)
        {
            cJSON *jTcSubItemtype = cJSON_GetObjectItem(tcSubitem, "type");
            if(jTcSubItemtype)
            {
                tcType = jTcSubItemtype->valuestring;
            }
            cJSON *jTcSubItemOperator = cJSON_GetObjectItem(tcSubitem, "operator");
            if(jTcSubItemOperator)
            {
                tcOperator = jTcSubItemOperator->valuestring;
            }

            cJSON *jTcSubItemThreshold = cJSON_GetObjectItem(tcSubitem, "threshold");
            if(jTcSubItemThreshold)
            {
                tcThreshold = jTcSubItemThreshold->valueint;
            }

            cJSON *jTcSubItemMinThresholdDuration = cJSON_GetObjectItem(tcSubitem, "minThresholdDuration");
            if(jTcSubItemMinThresholdDuration)
            {
                tcMinThresholdDuration = jTcSubItemMinThresholdDuration->valueint;
            }

            cJSON *jTcSubItemReference = cJSON_GetObjectItem(tcSubitem, "reference");
            if(jTcSubItemReference)
            {
                tcReference = jTcSubItemReference->valuestring;
            }
            T2Debug("Adding Trigger Condition:%s type %s operator %s \n", tcReference, tcType, tcOperator);
            TriggerCondition *triggerCondition = (TriggerCondition *) malloc(sizeof(TriggerCondition));

            if(triggerCondition == NULL)
            {
                T2Error("%s ++out Error adding Trigger Condition to profile %s \n", __FUNCTION__, profile->name);
                return T2ERROR_FAILURE;
            }

            cJSON *jTcSubItemReport = cJSON_GetObjectItem(tcSubitem, "report");
            if(jTcSubItemReport)
            {
                tcReport = cJSON_IsTrue(jTcSubItemReport);
            }

            /*CID 175311, 175312 and 175330 -Explicit null dereferenced */
            if(tcType)
            {
                triggerCondition->type = strdup(tcType);
            }
            if(tcOperator)
            {
                triggerCondition->oprator = strdup(tcOperator);
            }
            if(tcReference)
            {
                triggerCondition->reference = strdup(tcReference);
            }
            triggerCondition->threshold = tcThreshold;
            triggerCondition->minThresholdDuration = tcMinThresholdDuration;
            triggerCondition->isSubscribed = false;
            triggerCondition->report = tcReport;
            Vector_PushBack(profile->triggerConditionList, triggerCondition);
            T2Debug("[[Added Trigger Condition:%s]]\n", tcReference);
        }

    }

    T2Debug("%s ++out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

T2ERROR verifyTriggerCondition(cJSON *jprofileTriggerCondition)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    int triggerConditionIndex = 0;
    int triggerCondition_count = 0;
    int ret = T2ERROR_SUCCESS;
    char *operator = NULL;
    char *type = NULL;
    char *reference = NULL;
    triggerCondition_count = cJSON_GetArraySize(jprofileTriggerCondition);

    for( triggerConditionIndex = 0; triggerConditionIndex < triggerCondition_count; triggerConditionIndex++ )
    {
        cJSON* tcSubitem = cJSON_GetArrayItem(jprofileTriggerCondition, triggerConditionIndex);
        if(tcSubitem != NULL)
        {
            cJSON *jTcSubItemtype = cJSON_GetObjectItem(tcSubitem, "type");
            if(jTcSubItemtype == NULL)
            {
                T2Debug("Null type %s ++out\n", __FUNCTION__);
                return T2ERROR_FAILURE;
            }

            type = jTcSubItemtype->valuestring;
            if(strncmp(type, "dataModel", strlen("dataModel") + 1) != 0)
            {
                T2Debug("Unexpected type %s ++out\n", __FUNCTION__);
                return T2ERROR_FAILURE;
            }

            cJSON *jTcSubItemOperator = cJSON_GetObjectItem(tcSubitem, "operator");
            if(jTcSubItemOperator == NULL)
            {
                T2Debug("Null operator %s ++out\n", __FUNCTION__);
                return T2ERROR_FAILURE;
            }

            operator = jTcSubItemOperator->valuestring;
            if((strncmp(operator, "any", strlen("any") + 1) != 0) && (strncmp(operator, "lt", strlen("lt") + 1) != 0)
                    && (strncmp(operator, "gt", strlen("gt") + 1) != 0) && (strncmp(operator, "eq", strlen("eq") + 1) != 0))
            {
                T2Debug("Unexpected operator %s ++out\n", __FUNCTION__);
                return T2ERROR_FAILURE;
            }

            if((strncmp(operator, "any", strlen("any") + 1) != 0))
            {
                cJSON *jTcSubItemThreshold = cJSON_GetObjectItem(tcSubitem, "threshold");
                if(jTcSubItemThreshold == NULL)
                {
                    T2Debug("Null threshold %s ++out\n", __FUNCTION__);
                    return T2ERROR_FAILURE;
                }
            }

            cJSON *jTcSubItemReference = cJSON_GetObjectItem(tcSubitem, "reference");
            if(jTcSubItemReference == NULL)
            {
                T2Debug("Null reference %s ++out\n", __FUNCTION__);
                return T2ERROR_FAILURE;
            }

            reference = jTcSubItemReference->valuestring;
            if(strlen(reference) == 0)
            {
                T2Debug("Unexpected reference %s ++out\n", __FUNCTION__);
                return T2ERROR_FAILURE;
            }
        }
    }

    T2Debug("%s ++out\n", __FUNCTION__);
    return ret;
}

void time_param_Reporting_Adjustments_valid_set(Profile *profile, cJSON *jprofileReportingInterval, cJSON *jprofileActivationTimeout, cJSON *jprofileTimeReference, cJSON *jprofileReportOnUpdate, cJSON *jprofilefirstReportingInterval, cJSON *jprofilemaxUploadLatency, cJSON* jprofileReportingAdjustments)
{
    if(profile->generateNow)   //If GenerateNow is true and ReportingAdjustments is present, ReportingAdjustments shall be ignored
    {
        profile->reportingInterval = 0;
        profile->reportOnUpdate = false;
        profile->firstReportingInterval = 0;
        profile->maxUploadLatency = 0;
    }
    else
    {
        if(jprofileReportingInterval)
        {
            int reportIntervalInSec = jprofileReportingInterval->valueint;
            profile->reportingInterval = reportIntervalInSec;
        }

        if(jprofileActivationTimeout)
        {
            profile->activationTimeoutPeriod = jprofileActivationTimeout->valueint;
            T2Debug("[[ profile->activationTimeout:%d]]\n", profile->activationTimeoutPeriod);
        }

        if(jprofileTimeReference && jprofileReportingInterval)
        {
            profile->timeRef = strdup(jprofileTimeReference->valuestring);
        }

        if(jprofileReportingAdjustments)
        {
            if(jprofileReportOnUpdate)
            {
                profile->reportOnUpdate = (cJSON_IsTrue(jprofileReportOnUpdate) == 1);
                T2Debug("[[profile->reportOnUpdate:%u]]\n",  profile->reportOnUpdate);
            }
            if((profile->timeRef == NULL) || strcmp(profile->timeRef, DEFAULT_TIME_REFERENCE) == 0)  //Property 'FirstReportingInterval' if present along with Timereference,  FirstReportingInterval shall be ignored
            {
                if(jprofilefirstReportingInterval)
                {
                    int firstReportIntervalInSec = jprofilefirstReportingInterval->valueint;
                    profile->firstReportingInterval = firstReportIntervalInSec;
                    T2Debug("[[profile->firstReportingInterval:%d]]\n",  profile->firstReportingInterval);
                }
            }
            if(profile->timeRef != NULL && strcmp(profile->timeRef, DEFAULT_TIME_REFERENCE) != 0)  //This value is only valid when TimeReference is different from the default.
            {
                if(jprofilemaxUploadLatency)  //randomize the upload time of a generated report.
                {
                    int maxUploadLatencyInMilliSec = jprofilemaxUploadLatency->valueint;
                    profile->maxUploadLatency = maxUploadLatencyInMilliSec;
                    T2Debug("[[profile->maxUploadLatency:%dms]]\n",  profile->maxUploadLatency);
                }
            }
        }
    }
}

T2ERROR addParameter_marker_config(Profile* profile, cJSON *jprofileParameter, int ThisProfileParameter_count)
{
    if(profile == NULL || jprofileParameter == NULL)
    {
        T2Error("Invalid Arguments\n");
        return T2ERROR_INVALID_ARGS;
    }
    Vector_Create(&profile->paramList);
    Vector_Create(&profile->staticParamList);
    Vector_Create(&profile->eMarkerList);
    Vector_Create(&profile->gMarkerList);
    Vector_Create(&profile->cachedReportList);

    char* paramtype = NULL;
    char* use = NULL;
    bool reportEmpty = false;
    bool trim = false;
    char* header = NULL;
    char* content = NULL;
    char* logfile = NULL;
    char* regex = NULL;
    int skipFrequency = 0;
    int firstSeekFromEOF = 0;
    size_t profileParamCount = 0;
    int ProfileParameterIndex = 0;
    char* method = NULL;
    reportTimestampFormat rtformat;
    T2ERROR ret = T2ERROR_SUCCESS;

    for( ProfileParameterIndex = 0; ProfileParameterIndex < ThisProfileParameter_count; ProfileParameterIndex++ )
    {
        header = NULL;
        content = NULL;
        logfile = NULL;
        skipFrequency = 0;
        firstSeekFromEOF = 0;
        paramtype = NULL;
        use = NULL;
        reportEmpty = false;
        trim = false;
        rtformat = REPORTTIMESTAMP_NONE;

        cJSON* pSubitem = cJSON_GetArrayItem(jprofileParameter, ProfileParameterIndex);
        if(pSubitem != NULL)
        {
            cJSON *jpSubitemtype = cJSON_GetObjectItem(pSubitem, "type");
            if(jpSubitemtype)
            {
                paramtype = jpSubitemtype->valuestring;
            }
            else    /*TODO:Confirm if type is not configured. For now avoid corresponding marker*/
            {
                continue;
            }

            /* Confirm if "use" is considerable for datamodel type markers.*/
            cJSON *jpSubitemuse = cJSON_GetObjectItem(pSubitem, "use");
            if(jpSubitemuse)
            {
                use = jpSubitemuse->valuestring;
            }
            cJSON *jpSubitemreportEmpty = cJSON_GetObjectItem(pSubitem, "reportEmpty");
            if(jpSubitemreportEmpty)
            {
                reportEmpty = cJSON_IsTrue(jpSubitemreportEmpty);
            }
            cJSON *jpSubitemtrim = cJSON_GetObjectItem(pSubitem, "trim"); //RDKB-53161 adding trim parameter in multiprofiles
            if(jpSubitemtrim)
            {
                trim =  cJSON_IsTrue(jpSubitemtrim);
            }
            T2Debug("trimParam: %u\n", trim);
            cJSON *jpSubitemregex = cJSON_GetObjectItem(pSubitem, "regex");
            if(jpSubitemregex)
            {
                regex = jpSubitemregex->valuestring;
            }
            T2Debug("regexParam: %s\n", regex);
            if(!(strcmp(paramtype, "dataModel")))   /*Marker is TR181 param*/
            {
                cJSON *jpSubitemname = cJSON_GetObjectItem(pSubitem, "name");
                if(jpSubitemname)
                {
                    header = jpSubitemname->valuestring;
                }
                cJSON *jpSubitemreference = cJSON_GetObjectItem(pSubitem, "reference");
                if(jpSubitemreference == NULL)
                {
                    T2Error("Reference property is mandatory for datamodel parameter. Ignore the adding of this parameter to paramlist\n");
                    continue;
                }
                content = jpSubitemreference->valuestring;
                if(!jpSubitemname)
                {
                    header = jpSubitemreference->valuestring; /*Default Name can be reference*/
                }
                cJSON *jpMethod = cJSON_GetObjectItem(pSubitem, "method");
                cJSON *jpSubitemreportTimestamp = cJSON_GetObjectItem(pSubitem, "reportTimestamp");
                if(jpMethod)
                {
                    method = jpMethod->valuestring;
                    T2Debug("Method property is present and the value is %s\n", method);
                    if(!(strcmp(method, "subscribe")))
                    {
                        T2Info("Method is subscribe converting the parmeter to event type\n");
                        paramtype = "event";
                        //CID 337455: Dereference after null check (FORWARD_NULL)
                        if (jpSubitemreference != NULL)
                        {
                            header = jpSubitemreference->valuestring;
                        }
                        content = T2REPORTCOMPONENT;
                        if(jpSubitemname)
                        {
                            logfile = jpSubitemname->valuestring;
                        }

                        if(jpSubitemreportTimestamp)
                        {
                            T2Info("reportTimestamp is present and value is %s \n", jpSubitemreportTimestamp->valuestring);
                            if(!(strcmp(jpSubitemreportTimestamp->valuestring, "Unix-Epoch")))
                            {
                                rtformat = REPORTTIMESTAMP_UNIXEPOCH;
                            }
                        }
                    }
                }
            }
            else if(!(strcmp(paramtype, "event")))
            {

                cJSON *jpSubitemname = cJSON_GetObjectItem(pSubitem, "name"); // Optional repalcement name in report
                cJSON *jpSubitemeventName = cJSON_GetObjectItem(pSubitem, "eventName");
                cJSON *jpSubitemcomponent = cJSON_GetObjectItem(pSubitem, "component");

                if(jpSubitemname)
                {
                    logfile = jpSubitemname->valuestring;
                }
                if(jpSubitemeventName)
                {
                    header = jpSubitemeventName->valuestring; /*Default Name can be eventName*/
                }

                if(jpSubitemcomponent)
                {
                    content = jpSubitemcomponent->valuestring;
                }
                cJSON *jpSubitemreportTimestamp = cJSON_GetObjectItem(pSubitem, "reportTimestamp");
                if(jpSubitemreportTimestamp)
                {
                    if(!(strcmp(jpSubitemreportTimestamp->valuestring, "Unix-Epoch")))
                    {
                        rtformat = REPORTTIMESTAMP_UNIXEPOCH;
                    }
                }
            }
            else if(!(strcmp(paramtype, "grep")))    //grep Marker
            {

                cJSON *jpSubitemname = cJSON_GetObjectItem(pSubitem, "marker");
                cJSON *jpSubitSearchString = cJSON_GetObjectItem(pSubitem, "search");
                cJSON *jpSubitemLogFile = cJSON_GetObjectItem(pSubitem, "logFile");
                cJSON *jpSubitemFirstSeekFromEOF = cJSON_GetObjectItem(pSubitem, "firstSeekFromEOF");
                if(jpSubitemname)
                {
                    header = jpSubitemname->valuestring;
                }
                if(jpSubitSearchString)
                {
                    content = jpSubitSearchString->valuestring;
                }
                if(jpSubitemLogFile)
                {
                    logfile = jpSubitemLogFile->valuestring;
                }
                if(jpSubitemFirstSeekFromEOF)
                {
                    if (cJSON_IsNumber(jpSubitemFirstSeekFromEOF))
                    {
                        firstSeekFromEOF = (int) jpSubitemFirstSeekFromEOF->valuedouble;
                    }
                    else
                    {
                        T2Error("%s First SeekFromEOF is not a number ignoring the value \n", __FUNCTION__);
                    }
                }
            }
            else
            {
                T2Error("%s Unknown parameter type %s \n", __FUNCTION__, paramtype);
                continue;
            }

            T2Debug("%s : reportTimestamp = %d\n", __FUNCTION__, rtformat);
            //CID 337454: Explicit null dereferenced (FORWARD_NULL) ;CID 337448: Explicit null dereferenced (FORWARD_NULL)
            if (content != NULL && header != NULL)
            {
                ret = addParameter(profile, header, content, logfile, skipFrequency, firstSeekFromEOF, paramtype, use, reportEmpty, rtformat, trim, regex); //add Multiple Report Profile Parameter
            }
            else
            {
                T2Error("%s Error in adding parameter to profile %s \n", __FUNCTION__, profile->name); // header and content is mandatory for adding the parameter
                continue;
            }
            if(ret != T2ERROR_SUCCESS)
            {
                T2Error("%s Error in adding parameter to profile %s \n", __FUNCTION__, profile->name);
                continue;
            }
            else
            {
                T2Debug("[[Added parameter:%s]]\n", header);
            }
            profileParamCount++;
        }
    }
    // GrepMarkerlist expects the list to be sorted based on logfile names
    Vector_Sort(profile->gMarkerList,  sizeof(GrepMarker*), compareLogFileNames);

    T2Info("Number of tr181params/markers successfully added in profile = %lu \n", (unsigned long)profileParamCount);
    return T2ERROR_SUCCESS;
}

T2ERROR encodingSet(Profile* profile, cJSON *jprofileEncodingType, cJSON *jprofileJSONReportFormat, cJSON *jprofileJSONReportTimestamp)
{
    if(profile == NULL)
    {
        T2Error("Profile is NULL\n");
        return T2ERROR_INVALID_ARGS;
    }
    if((profile->jsonEncoding) && (strcmp(jprofileEncodingType->valuestring, "JSON") == 0))
    {

        if(!(strcmp(jprofileJSONReportFormat->valuestring, "NameValuePair")))
        {
            profile->jsonEncoding->reportFormat = JSONRF_KEYVALUEPAIR;
        }
        else if(!(strcmp(jprofileJSONReportFormat->valuestring, "ObjectHierarchy")))
        {
            profile->jsonEncoding->reportFormat = JSONRF_OBJHIERARCHY;
        }
        else    /* defaulting it to NameValuePair */
        {
            profile->jsonEncoding->reportFormat = JSONRF_KEYVALUEPAIR;
        }

        T2Debug("[[profile->jsonEncoding->reportFormat:%d]]\n", profile->jsonEncoding->reportFormat);
        if(jprofileJSONReportTimestamp)
        {
            if(!(strcmp(jprofileJSONReportTimestamp->valuestring, "None")))
            {
                profile->jsonEncoding->tsFormat = TIMESTAMP_NONE;
            }
            else if(!(strcmp(jprofileJSONReportTimestamp->valuestring, "Unix-Epoch")))
            {
                profile->jsonEncoding->tsFormat = TIMESTAMP_UNIXEPOCH;
            }
            else if(!(strcmp(jprofileJSONReportTimestamp->valuestring, "ISO-8601")))
            {
                profile->jsonEncoding->tsFormat = TIMESTAMP_ISO_8601;
            }
            else   /*default value for profile->jsonEncoding->tsFormat is None */
            {
                profile->jsonEncoding->tsFormat = TIMESTAMP_NONE;
            }
            T2Debug("[[profile->jsonEncoding->tsFormat:%d]]\n", profile->jsonEncoding->tsFormat);

        }
    }
    return T2ERROR_SUCCESS;
}

T2ERROR protocolSet (Profile *profile, cJSON *jprofileProtocol, cJSON *jprofileHTTPURL, cJSON *jprofileHTTPRequestURIParameter, int ThisprofileHTTPRequestURIParameter_count, cJSON *jprofileRBUSMethodName, cJSON *jprofileRBUSMethodParamArr, int rbusMethodParamArrCount)
{
    if(profile == NULL)
    {
        T2Info("Profile is NULL\n");
        return T2ERROR_INVALID_ARGS;
    }

    if((profile->t2HTTPDest) && (strcmp(jprofileProtocol->valuestring, "HTTP") == 0))
    {
        profile->t2HTTPDest->URL = strdup(jprofileHTTPURL->valuestring);
        profile->t2HTTPDest->Compression = COMP_NONE; /*in 1911_sprint default none*/
        profile->t2HTTPDest->Method = HTTP_POST; /*1911_sprint default to POST */

        T2Debug("[[profile->t2HTTPDest->URL:%s]]\n", profile->t2HTTPDest->URL);
        T2Debug("[[profile->t2HTTPDest->Compression:%d]]\n", profile->t2HTTPDest->Compression);
        T2Debug("[[profile->t2HTTPDest->Method:%d]]\n", profile->t2HTTPDest->Method);

        if(jprofileHTTPRequestURIParameter)
        {

            Vector_Create(&(profile->t2HTTPDest->RequestURIparamList));
            char* href = NULL;
            char* hname = NULL;
            int httpret = 0;
            int httpParamCount = 0;
            int profileHTTPRequestURIParameterIndex = 0;

            for( profileHTTPRequestURIParameterIndex = 0; profileHTTPRequestURIParameterIndex < ThisprofileHTTPRequestURIParameter_count;
                    profileHTTPRequestURIParameterIndex++ )
            {
                href = NULL;
                hname = NULL;
                cJSON* pHTTPRPsubitem = cJSON_GetArrayItem(jprofileHTTPRequestURIParameter, profileHTTPRequestURIParameterIndex);
                if(pHTTPRPsubitem != NULL)
                {
                    cJSON *pHTTPRPsubitemReference = cJSON_GetObjectItem(pHTTPRPsubitem, "Reference");
                    if(pHTTPRPsubitemReference)
                    {
                        href = pHTTPRPsubitemReference->valuestring;
                        hname = pHTTPRPsubitemReference->valuestring; // default value for Name
                        if(!(strcmp(href, "")))   /*if reference is empty avoid adding this object*/
                        {
                            continue;
                        }
                    }
                    else
                    {
                        continue;
                    }

                    cJSON *pHTTPRPsubitemName = cJSON_GetObjectItem(pHTTPRPsubitem, "Name");
                    if(pHTTPRPsubitemName)
                    {
                        hname = pHTTPRPsubitemName->valuestring;
                    }
                    httpret = addhttpURIreqParameter(profile, hname, href);
                    if(httpret != T2ERROR_SUCCESS)
                    {
                        T2Error("%s Error in adding request URIparameterReference: %s for the profile: %s \n", __FUNCTION__, href, profile->name);
                        continue;
                    }
                    else
                    {
                        T2Debug("[[Added  request URIparameterReference: %s]]\n", href);
                    }
                    httpParamCount++;
                }

            }
            T2Info("Profile Name: %s\nConfigured httpURIReqParam count = %d \n", profile->name, ThisprofileHTTPRequestURIParameter_count);
            T2Info("Number of httpURIReqParam added  = %d \n", httpParamCount);
        }
    }

    if((profile->t2RBUSDest) && (strcmp(jprofileProtocol->valuestring, "RBUS_METHOD") == 0))
    {
        profile->t2RBUSDest->rbusMethodName = strdup(jprofileRBUSMethodName->valuestring);
        T2Debug("[[profile->t2RBUSDest->rbusMethodName:%s]]\n", profile->t2RBUSDest->rbusMethodName);

        if(jprofileRBUSMethodParamArr)
        {
            char* name = NULL;
            char* value = NULL;
            int rbusMethodParamIndex = 0;
            int rbusParamAdded = 0 ;
            T2ERROR rbusParamRet = T2ERROR_FAILURE;

            Vector_Create(&(profile->t2RBUSDest->rbusMethodParamList));
            for( rbusMethodParamIndex = 0; rbusMethodParamIndex < rbusMethodParamArrCount; rbusMethodParamIndex++ )
            {
                name = NULL;
                value = NULL;
                cJSON* jRbusParamElement = cJSON_GetArrayItem(jprofileRBUSMethodParamArr, rbusMethodParamIndex);
                if(jRbusParamElement != NULL)
                {
                    cJSON *jName = cJSON_GetObjectItem(jRbusParamElement, "name");
                    cJSON *jValue = cJSON_GetObjectItem(jRbusParamElement, "value");
                    if(jName && jValue)
                    {
                        name = jName->valuestring;
                        value = jValue->valuestring; // default value for Name
                    }
                    else
                    {
                        continue;
                    }
                    rbusParamRet = addRbusMethodParameter(profile, name, value);
                    if(rbusParamRet != T2ERROR_SUCCESS)
                    {
                        T2Error("%s Error in adding request URIparameterReference: %s for the profile: %s \n", __FUNCTION__, name, profile->name);
                        continue;
                    }
                    else
                    {
                        T2Debug("[[Added  request URIparameterReference: %s]]\n", name);
                    }
                    rbusParamAdded++;
                }
            }
            T2Info("Profile Name: %s\nConfigured rbus method param count = %d \n", profile->name, rbusMethodParamArrCount);
            T2Info("Number of rbus method param added  = %d \n", rbusParamAdded);
        }
    }
    return T2ERROR_SUCCESS;

}

T2ERROR processConfiguration(char** configData, char *profileName, char* profileHash, Profile **localProfile)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    //REPORTPROFILE CJson PARSING
    T2ERROR ret = T2ERROR_SUCCESS, retvalue = T2ERROR_SUCCESS;
    int ThisProfileParameter_count = 0;
    cJSON *json_root = cJSON_Parse(*configData);

    cJSON *jprofileHash = cJSON_GetObjectItem(json_root, "Hash");
    if(jprofileHash == NULL)
    {
        jprofileHash = cJSON_GetObjectItem(json_root, "versionHash");
    }
    cJSON *jprofileDescription = cJSON_GetObjectItem(json_root, "Description");
    cJSON *jprofileVersion = cJSON_GetObjectItem(json_root, "Version");
    cJSON *jprofileProtocol = cJSON_GetObjectItem(json_root, "Protocol");
    cJSON *jprofileEncodingType = cJSON_GetObjectItem(json_root, "EncodingType");
    cJSON *jprofileReportingInterval = cJSON_GetObjectItem(json_root, "ReportingInterval");
    cJSON *jprofileRootName = cJSON_GetObjectItem(json_root, "RootName");
    cJSON *jprofileActivationTimeout = cJSON_GetObjectItem(json_root, "ActivationTimeOut");
    cJSON *jprofileTimeReference = cJSON_GetObjectItem(json_root, "TimeReference");
    cJSON *jprofileParameter = cJSON_GetObjectItem(json_root, "Parameter");
    cJSON *jprofileGenerateNow = cJSON_GetObjectItem(json_root, "GenerateNow");
    cJSON *jprofileDeleteOnTimeout = cJSON_GetObjectItem(json_root, "DeleteOnTimeout");
    cJSON *jprofileTriggerCondition = cJSON_GetObjectItem(json_root, "TriggerCondition");
    cJSON *jprofileReportingAdjustments =  cJSON_GetObjectItem(json_root, "ReportingAdjustments");
    cJSON *jprofileReportOnUpdate = NULL;
    cJSON *jprofilefirstReportingInterval =  NULL;
    cJSON *jprofilemaxUploadLatency = NULL;

    if(jprofileReportingAdjustments)
    {
        jprofileReportOnUpdate = cJSON_GetObjectItem(jprofileReportingAdjustments, "ReportOnUpdate");
        jprofilefirstReportingInterval = cJSON_GetObjectItem(jprofileReportingAdjustments, "FirstReportingInterval");
        jprofilemaxUploadLatency = cJSON_GetObjectItem(jprofileReportingAdjustments, "MaxUploadLatency");
    }

    if(jprofileParameter)
    {
        ThisProfileParameter_count = cJSON_GetArraySize(jprofileParameter);
    }
    if(profileName == NULL || jprofileProtocol == NULL || jprofileEncodingType == NULL || jprofileParameter == NULL || (profileHash == NULL && jprofileHash == NULL))
    {
        T2Error("Incomplete profile information, unable to create profile\n");
        cJSON_Delete(json_root);
        return T2ERROR_FAILURE;
    }
    if(jprofileReportingInterval && jprofilemaxUploadLatency && ((jprofileReportingInterval->valueint * 1000) < jprofilemaxUploadLatency->valueint))
    {
        T2Error("MaxUploadLatency is greater than reporting interval. Invalid Profile\n");
        cJSON_Delete(json_root);
        return T2ERROR_FAILURE;
    }
    if(!jprofileTriggerCondition && !jprofileReportingInterval)
    {
        T2Error("If TriggerCondition is not given ReportingInterval parameter is mandatory. Reporting interval is not configured for this profile..Invalid Profile\n");
        cJSON_Delete(json_root);
        return T2ERROR_FAILURE;
    }
    if(!jprofileHash)
    {
        cJSON_AddStringToObject(json_root, "Hash", profileHash); // updating the versionHash to persist the hash along with profile configuration
        free(*configData);
        *configData = NULL;
        *configData = cJSON_PrintUnformatted(json_root);
    }

    cJSON *jprofileHTTP = NULL;
    cJSON *jprofileHTTPURL = NULL;
    cJSON *jprofileHTTPCompression = NULL;
    cJSON *jprofileHTTPMethod = NULL;
    cJSON *jprofileHTTPRequestURIParameter = NULL;
    int ThisprofileHTTPRequestURIParameter_count = 0;

    cJSON *jprofileRBUSMethod = NULL;
    cJSON *jprofileRBUSMethodName = NULL ;
    cJSON *jprofileRBUSMethodParamArr = NULL ;
    int rbusMethodParamArrCount = 0 ;

    if(jprofileProtocol)
    {
        if(strcmp(jprofileProtocol->valuestring, "HTTP") == 0)
        {
            jprofileHTTP = cJSON_GetObjectItem(json_root, "HTTP");
            jprofileHTTPURL = cJSON_GetObjectItem(jprofileHTTP, "URL");
            jprofileHTTPCompression = cJSON_GetObjectItem(jprofileHTTP, "Compression");
            jprofileHTTPMethod = cJSON_GetObjectItem(jprofileHTTP, "Method");
            if(jprofileHTTPURL == NULL || jprofileHTTPCompression == NULL || jprofileHTTPMethod == NULL)
            {
                T2Error("Incomplete profile information, unable to create profile\n");
                cJSON_Delete(json_root);
                return T2ERROR_FAILURE;
            }
            jprofileHTTPRequestURIParameter = cJSON_GetObjectItem(jprofileHTTP, "RequestURIParameter");
            if(jprofileHTTPRequestURIParameter)
            {
                ThisprofileHTTPRequestURIParameter_count = cJSON_GetArraySize(jprofileHTTPRequestURIParameter);
            }
        }
        else if(strcmp(jprofileProtocol->valuestring, "RBUS_METHOD") == 0)
        {
            jprofileRBUSMethod = cJSON_GetObjectItem(json_root, "RBUS_METHOD");
            jprofileRBUSMethodName = cJSON_GetObjectItem(jprofileRBUSMethod, "Method");
            if(jprofileRBUSMethodName == NULL || jprofileRBUSMethodName->valuestring == NULL || strcmp(jprofileRBUSMethodName->valuestring, "") == 0)
            {
                T2Error("RBUS Method not configured . Ignoring profile %s \n", profileName);
                cJSON_Delete(json_root);
                return T2ERROR_FAILURE;
            }
            if (jprofileRBUSMethodName->valuestring && !rbusCheckMethodExists(jprofileRBUSMethodName->valuestring) )
            {
                T2Warning("WARN: no method provider: %s %s\n", profileName, jprofileRBUSMethodName->valuestring);
            }
            jprofileRBUSMethodParamArr = cJSON_GetObjectItem(jprofileRBUSMethod, "Parameters");
            if(jprofileRBUSMethodParamArr)
            {
                rbusMethodParamArrCount = cJSON_GetArraySize(jprofileRBUSMethodParamArr);
            }
        }
        else
        {
            T2Error("Unsupported protocol : %s \n", jprofileProtocol->valuestring);
            cJSON_Delete(json_root);
            return T2ERROR_FAILURE ;
        }
    }

    cJSON *jprofileJSONEncoding = NULL;
    cJSON *jprofileJSONReportFormat = NULL;
    cJSON *jprofileJSONReportTimestamp = NULL;

    if((jprofileEncodingType) && (strcmp(jprofileEncodingType->valuestring, "JSON") == 0))
    {
        jprofileJSONEncoding = cJSON_GetObjectItem(json_root, "JSONEncoding");
        jprofileJSONReportFormat = cJSON_GetObjectItem(jprofileJSONEncoding, "ReportFormat");
        if(jprofileJSONReportFormat == NULL)
        {
            T2Error("Incomplete profile information, unable to create profile\n");
            cJSON_Delete(json_root);
            return T2ERROR_FAILURE;

        }
        jprofileJSONReportTimestamp = cJSON_GetObjectItem(jprofileJSONEncoding, "ReportTimestamp");
    }

    if(jprofileReportingInterval && jprofileActivationTimeout && jprofileActivationTimeout->valueint < jprofileReportingInterval->valueint)
    {

        T2Error("activationTimeoutPeriod is less than reporting interval. invalid profile: %s \n", profileName);
        cJSON_Delete(json_root);
        return T2ERROR_FAILURE;
    }

    if(jprofileTriggerCondition)
    {
        int ret;
        ret = verifyTriggerCondition(jprofileTriggerCondition);
        if(ret == T2ERROR_FAILURE)
        {
            T2Error("TriggerCondition is invalid, unable to create profile\n");
            cJSON_Delete(json_root);
            return T2ERROR_FAILURE;
        }
    }

    //PROFILE CREATION
    Profile *profile = (Profile *) malloc(sizeof(Profile));
    if(profile == NULL)
    {
        T2Error("Malloc error\n can not allocate memory to create profile\n exiting \n");
        cJSON_Delete(json_root);
        return T2ERROR_FAILURE;
    }

    memset(profile, 0, sizeof(Profile));

    //Default values
    profile->generateNow = false;
    profile->deleteonTimeout = false;
    profile->activationTimeoutPeriod = INFINITE_TIMEOUT;
    // Default values for Reporting Adjustments and timeRef
    profile->reportOnUpdate = false;
    profile->firstReportingInterval = 0;
    profile->maxUploadLatency = 0;
    profile->timeRef = NULL;
    profile->isSchedulerstarted = false;
    profile->saveSeekConfig = false;
    profile->checkPreviousSeek = false;
    profile->reportScheduled = false;
    pthread_mutex_init(&profile->reportScheduledMutex, NULL);
    pthread_cond_init(&profile->reportScheduledCond, NULL);

    if(jprofileDeleteOnTimeout)
    {
        profile->deleteonTimeout = (cJSON_IsTrue(jprofileDeleteOnTimeout) == 1);
        T2Info("profile->deleteonTimeout: %i\n", profile->deleteonTimeout);
    }
    if(jprofileGenerateNow)
    {
        profile->generateNow = (cJSON_IsTrue(jprofileGenerateNow) == 1);
    }

    time_param_Reporting_Adjustments_valid_set(profile, jprofileReportingInterval, jprofileActivationTimeout, jprofileTimeReference, jprofileReportOnUpdate, jprofilefirstReportingInterval, jprofilemaxUploadLatency, jprofileReportingAdjustments);
    T2Info("Reporting Adjustments parameters check done successfully\n");

    if(jprofileTriggerCondition && (!jprofileReportingInterval) && (!profile->generateNow))
    {
        profile->reportingInterval = UINT_MAX;
    }

    if(profile->reportingInterval > profile->activationTimeoutPeriod)  //When Reporting interval and activation time is not given in the profile it takes the max value INFINITE TIMEOUT or UINT_MAX
    {
        T2Error("activationTimeoutPeriod is less than reporting interval. Max values assigned is inappropriate .. Invalid profile: %s \n", profileName);
        cJSON_Delete(json_root);
        if(profile->timeRef != NULL)
        {
            free(profile->timeRef);
        }
        if(profile != NULL)
        {
            free(profile);
        }
        return T2ERROR_FAILURE;
    }

    if((strcmp(jprofileEncodingType->valuestring, "JSON") == 0))
    {
        profile->jsonEncoding = (JSONEncoding *) malloc(sizeof(JSONEncoding));
        if(profile->jsonEncoding == NULL)
        {
            T2Error("jsonEncoding malloc error\n");
            if(profile->timeRef != NULL)
            {
                free(profile->timeRef);
            }
            if(profile)
            {
                free(profile);
            }
            cJSON_Delete(json_root);
            return T2ERROR_FAILURE;
        }
    }

    if((strcmp(jprofileProtocol->valuestring, "HTTP") == 0))
    {
        profile->t2HTTPDest = (T2HTTP *) malloc(sizeof(T2HTTP));
    }
    else if ((strcmp(jprofileProtocol->valuestring, "RBUS_METHOD") == 0))
    {
        profile->t2RBUSDest = malloc(sizeof(T2RBUS));
    }

    if ( (profile->t2HTTPDest == NULL) && (profile->t2RBUSDest  == NULL) )
    {
        T2Error("t2 protocol destination object malloc error\n");
        if(profile->jsonEncoding)
        {
            free(profile->jsonEncoding);
        }
        if(profile->timeRef != NULL)
        {
            free(profile->timeRef);
        }
        if(profile)
        {
            free(profile);
        }
        cJSON_Delete(json_root);
        return T2ERROR_FAILURE;
    }

    profile->minThresholdDuration = 0;
    profile->name = strdup(profileName);
    if(jprofileDescription)
    {
        profile->Description = strdup(jprofileDescription->valuestring);
    }
    if(jprofileRootName)
    {
        profile->RootName = strdup(jprofileRootName->valuestring);
    }
    else
    {
        profile->RootName = strdup("Report");
    }
    if(jprofileVersion)
    {
        profile->version = strdup(jprofileVersion->valuestring);
    }
    if(!jprofileHash)
    {
        profile->hash = strdup(profileHash);
    }
    else
    {
        profile->hash = strdup(jprofileHash->valuestring);
    }

    profile->protocol = strdup(jprofileProtocol->valuestring);
    profile->encodingType = strdup(jprofileEncodingType->valuestring);


    T2Debug("[[profile->name:%s]]\n", profile->name);
    T2Debug("[[ profile->Description:%s]]\n", profile->Description);
    T2Debug("[[profile->version:%s]]\n", profile->version);
    T2Debug("[[profile->protocol:%s]]\n", profile->protocol);
    T2Debug("[[profile->encodingType:%s]]\n", profile->encodingType);
    T2Debug("[[profile->RootName:%s]]\n", profile->RootName);
    T2Debug("[[profile->timeRef:%s]]\n", profile->timeRef);
    T2Debug("[[ profile->activationTimeout:%d]]\n", profile->activationTimeoutPeriod);

    if(profile->reportingInterval)
    {
        T2Debug("[[ profile->reportingInterval:%d]]\n", profile->reportingInterval);
    }

    //Encoding Configuration
    retvalue = encodingSet(profile, jprofileEncodingType, jprofileJSONReportFormat, jprofileJSONReportTimestamp);
    if(retvalue != T2ERROR_SUCCESS)
    {
        T2Error("Encoding parameters are not configured.Profile is invalid\n");
        return retvalue;
    }

    //Protocol Configuration
    retvalue = protocolSet (profile, jprofileProtocol, jprofileHTTPURL, jprofileHTTPRequestURIParameter, ThisprofileHTTPRequestURIParameter_count, jprofileRBUSMethodName, jprofileRBUSMethodParamArr, rbusMethodParamArrCount);
    if(retvalue != T2ERROR_SUCCESS)
    {
        T2Error("Protocol Configuration is invalid\n");
        return retvalue;
    }
    //Parameter Marker Configuration
    retvalue = addParameter_marker_config(profile, jprofileParameter, ThisProfileParameter_count);
    if(retvalue != T2ERROR_SUCCESS)
    {
        T2Error("Parameter marker configuration is invalid\n");
    }

    int triggerCondition_count = 0;
    if(jprofileTriggerCondition)
    {
        triggerCondition_count = cJSON_GetArraySize(jprofileTriggerCondition);
        T2Debug("TC found in profile\n");
    }
    T2Debug("triggerCondition_count %d\n", triggerCondition_count);
    if(triggerCondition_count)
    {
        ret = addTriggerCondition(profile, jprofileTriggerCondition); //add
        if(ret != T2ERROR_SUCCESS)
        {
            T2Error("%s Error adding Trigger Condition to profile %s \n", __FUNCTION__, profile->name);
        }
    }

    // Not included for RDKB-25008 . DCA utils expects the list to be sorted based on logfile names
    cJSON_Delete(json_root);
    json_root = NULL;
    *localProfile = profile;
    T2Debug("%s --out\n", __FUNCTION__);
    return retvalue;

}

char *msgpack_strdup(msgpack_object *obj)
{
    if (NULL == obj || MSGPACK_OBJECT_STR != obj->type)
    {
        return NULL;
    }
    return strndup(obj->via.str.ptr, obj->via.str.size);
}

int msgpack_strcmp(msgpack_object *obj, char *str)
{
    if (NULL == obj || MSGPACK_OBJECT_STR != obj->type)
    {
        return -1;
    }
    return strncmp(str, obj->via.str.ptr, obj->via.str.size);
}

void msgpack_print(msgpack_object *obj, char *obj_name)
{
    if (obj)
    {
        switch(obj->type)
        {
        case MSGPACK_OBJECT_BOOLEAN:
            T2Debug("%s: %s\n", obj_name, obj->via.boolean ? "true" : "false");
            break;
        case MSGPACK_OBJECT_POSITIVE_INTEGER:
            T2Debug("%s: %lu\n", obj_name, (unsigned long)obj->via.u64);
            break;
        case MSGPACK_OBJECT_NEGATIVE_INTEGER:
            T2Debug("%s: %ld\n", obj_name, (signed long)obj->via.i64);
            break;
        case MSGPACK_OBJECT_FLOAT:
            T2Debug("%s: %f\n", obj_name, obj->via.f64);
            break;
        case MSGPACK_OBJECT_STR:
            T2Debug("%s: %.*s\n", obj_name, obj->via.str.size, obj->via.str.ptr);
            break;
        case MSGPACK_OBJECT_ARRAY:
            T2Debug("%s size: %d\n", obj_name, obj->via.array.size);
            break;
        case MSGPACK_OBJECT_MAP:
            T2Debug("%s size: %d\n", obj_name, obj->via.map.size);
            break;
        default:
            T2Info("%s->type is Invalid\n", obj_name);
            T2Info("%s->type is %u \n", obj_name, obj->type);
        }
    }
    else
    {
        T2Debug("%s Object is NULL\n", obj_name);
    }
}

msgpack_object *msgpack_get_map_value(msgpack_object *obj, char *key)
{
    if (NULL == obj)
    {
        return NULL;
    }
    if (MSGPACK_OBJECT_MAP == obj->type)
        if(obj->via.map.size != 0)
        {
            msgpack_object_kv* current = obj->via.map.ptr;
            msgpack_object_kv* const end = obj->via.map.ptr + obj->via.map.size;
            for(; current < end; current++)
                if (current->key.type == MSGPACK_OBJECT_STR)
                {
                    if ( 0 == strncmp(key, current->key.via.str.ptr, current->key.via.str.size))
                    {
                        return &current->val;
                    }
                }
        }
    return NULL;
}

msgpack_object *msgpack_get_array_element(msgpack_object *obj, int index)
{
    if (NULL == obj)
    {
        return NULL;
    }
    if (MSGPACK_OBJECT_ARRAY == obj->type)
    {
        if(obj->via.array.size != 0)
        {
            msgpack_object* current = obj->via.array.ptr;
            msgpack_object* const end = obj->via.array.ptr + obj->via.array.size;
            for (; current < end && index; current++, index--)
                ;
            return current;
        }
    }
    else if (MSGPACK_OBJECT_MAP == obj->type)
    {
        if(obj->via.map.size != 0)
        {
            msgpack_object_kv* current = obj->via.map.ptr;
            msgpack_object_kv* const end = obj->via.map.ptr + obj->via.map.size;
            for(; current < end && index; current++, index--)
                ;
            return &current->val;
        }
    }
    return NULL;
}

T2ERROR verifyMsgPckTriggerCondition(msgpack_object *value_map)
{

    msgpack_object *TriggerConditionArray;
    msgpack_object *tcArrayMap;
    msgpack_object *tcTypeStr;
    msgpack_object *tcOperatorStr;
    msgpack_object *tcReferenceStr;
    msgpack_object *tcThresholdInt;
    uint32_t i, TcArraySize = 0;
    int ret = T2ERROR_SUCCESS;
    char* tcType;
    char* tcOperator;
    char* tcReference;

    if((value_map == NULL))
    {
        T2Error("%s NULL object returning failure\n", __FUNCTION__);
        return T2ERROR_FAILURE;
    }
    T2Debug("%s ++in\n", __FUNCTION__);

    TriggerConditionArray = msgpack_get_map_value(value_map, "TriggerCondition");
    MSGPACK_GET_ARRAY_SIZE(TriggerConditionArray, TcArraySize);

    for(i = 0; i < TcArraySize; i++)
    {

        tcType = NULL;
        tcOperator = NULL;
        tcReference = NULL;

        tcArrayMap = msgpack_get_array_element(TriggerConditionArray, i);
        tcTypeStr = msgpack_get_map_value(tcArrayMap, "type");
        if(tcTypeStr == NULL)
        {
            T2Debug("Null type %s ++out\n", __FUNCTION__);
            ret = T2ERROR_FAILURE;
            goto error;
        }

        msgpack_print(tcTypeStr, msgpack_get_obj_name(tcTypeStr));
        tcType = msgpack_strdup(tcTypeStr);
        if(strncmp(tcType, "dataModel", strlen("dataModel") + 1) != 0)
        {
            T2Debug("Unexpected type %s ++out\n", __FUNCTION__);
            ret = T2ERROR_FAILURE;
            goto error;
        }

        tcOperatorStr = msgpack_get_map_value(tcArrayMap, "operator");
        if(tcOperatorStr == NULL)
        {
            T2Debug("Null operator %s ++out\n", __FUNCTION__);
            ret = T2ERROR_FAILURE;
            goto error;
        }

        msgpack_print(tcOperatorStr, msgpack_get_obj_name(tcOperatorStr));
        tcOperator = msgpack_strdup(tcOperatorStr);
        if((strncmp(tcOperator, "any", strlen("any") + 1) != 0) &&
                (strncmp(tcOperator, "lt", strlen("lt") + 1) != 0) &&
                (strncmp(tcOperator, "gt", strlen("gt") + 1) != 0) &&
                (strncmp(tcOperator, "eq", strlen("eq") + 1) != 0))
        {
            T2Debug("Unexpected operator %s ++out\n", __FUNCTION__);
            ret = T2ERROR_FAILURE;
            goto error;
        }

        if(strncmp(tcOperator, "any", strlen("any") + 1) != 0)
        {
            tcThresholdInt = msgpack_get_map_value(tcArrayMap, "threshold");
            if(tcThresholdInt == NULL)
            {
                T2Debug("Null threshold %s ++out\n", __FUNCTION__);
                ret = T2ERROR_FAILURE;
                goto error;
            }
        }

        tcReferenceStr = msgpack_get_map_value(tcArrayMap, "reference");
        if(tcReferenceStr == NULL)
        {
            T2Debug("Null reference %s ++out\n", __FUNCTION__);
            ret = T2ERROR_FAILURE;
            goto error;
        }

        msgpack_print(tcReferenceStr, msgpack_get_obj_name(tcReferenceStr));
        tcReference = msgpack_strdup(tcReferenceStr);
        if(strlen(tcReference) == 0)
        {
            T2Debug("Unexpected reference %s ++out\n", __FUNCTION__);
            ret = T2ERROR_FAILURE;
            goto error;
        }

        /*CID 178509, 178510 and 178511: Dereference before null check */
        free(tcType);
        free(tcOperator);
        free(tcReference);
    }

error:
    if(ret == T2ERROR_FAILURE)
    {
        if(tcType)
        {
            free(tcType);
        }
        if(tcOperator)
        {
            free(tcOperator);
        }
        if(tcReference)
        {
            free(tcReference);
        }
    }

    T2Debug("%s ++out\n", __FUNCTION__);
    return ret;
}

T2ERROR time_param_MsgPackReporting_Adjustments_valid_set(Profile *profile, msgpack_object* ReportingInterval_u64, msgpack_object* ActivationTimeout_u64, msgpack_object* TimeReference_str, msgpack_object* ReportOnUpdate_boolean, msgpack_object* FirstReportInterval_u64, msgpack_object* MaxUploadLatency_u64)
{
    if(profile->generateNow)   ////If GenerateNow is true and ReportingAdjustments is present, ReportingAdjustments shall be ignored
    {
        profile->reportingInterval = 0;
        profile->reportOnUpdate = false;
        profile->firstReportingInterval = 0;
        profile->maxUploadLatency = 0;
    }
    else
    {
        if (ReportingInterval_u64 && ActivationTimeout_u64 &&
                profile->activationTimeoutPeriod < profile->reportingInterval)
        {

            T2Error("activationTimeoutPeriod is less than reporting interval. invalid profile: %s \n", profile->name);
            free(profile->name);
            free(profile->hash);
            if (profile->t2HTTPDest)
            {
                free(profile->t2HTTPDest);
            }
            if (profile->t2RBUSDest)
            {
                free(profile->t2RBUSDest);
            }
            free(profile->Description);
            free(profile->version);
            free(profile);
            return T2ERROR_FAILURE;
        }
        //TimeReference is only valid for profiles that specify a ReportingInterval
        if(TimeReference_str != NULL && ReportingInterval_u64)
        {
            profile->timeRef = msgpack_strdup(TimeReference_str);
        }
        if(ReportOnUpdate_boolean != NULL)
        {
            msgpack_print(ReportOnUpdate_boolean, msgpack_get_obj_name(ReportOnUpdate_boolean));
            MSGPACK_GET_NUMBER(ReportOnUpdate_boolean, profile->reportOnUpdate);
            T2Debug("profile->reportingAdjust->reportOnUpdate: %u\n", profile->reportOnUpdate);
        }
        if(profile->timeRef == NULL || strcmp(profile->timeRef, DEFAULT_TIME_REFERENCE) == 0)  //Property 'FirstReportingInterval' if present along with Timereference,  FirstReportingInterval shall be ignored
        {
            if(FirstReportInterval_u64 != NULL)
            {
                msgpack_print(FirstReportInterval_u64, msgpack_get_obj_name(FirstReportInterval_u64));
                MSGPACK_GET_NUMBER(FirstReportInterval_u64, profile->firstReportingInterval);
                T2Debug("profile->reportingAdjust->firstReportingInterval: %d\n", profile->firstReportingInterval);
            }
        }
        if(profile->timeRef != NULL && strcmp(profile->timeRef, DEFAULT_TIME_REFERENCE) != 0)  //This value is only valid when TimeReference is different from the default.
        {
            if(MaxUploadLatency_u64 != NULL)
            {
                msgpack_print(MaxUploadLatency_u64, msgpack_get_obj_name(MaxUploadLatency_u64));
                MSGPACK_GET_NUMBER(MaxUploadLatency_u64, profile->maxUploadLatency);
                T2Debug("profile->reportingAdjust->maxUploadLatency: %dms\n", profile->maxUploadLatency);
            }
        }
    }
    return T2ERROR_SUCCESS;
}

T2ERROR addParameterMsgpack_marker_config(Profile* profile, msgpack_object* value_map)
{
    if(profile == NULL || value_map == NULL)
    {
        T2Error("Profile or value_map is NULL\n");
        return T2ERROR_INVALID_ARGS;
    }
    msgpack_object *Parameter_array;
    msgpack_object *Parameter_array_map;
    msgpack_object *Parameter_type_str;
    msgpack_object *Parameter_marker_str;
    msgpack_object *Parameter_search_str;
    msgpack_object *Parameter_logFile_str;
    msgpack_object *Parameter_use_str;
    msgpack_object *Parameter_reference_str;
    msgpack_object *Parameter_eventName_str;
    msgpack_object *Parameter_component_str;
    msgpack_object *Parameter_name_str;
    msgpack_object *Parameter_method_str;
    msgpack_object *Parameter_reportEmpty_boolean;
    msgpack_object *Parameter_trim_boolean;
    msgpack_object *Parameter_regex_str;
    msgpack_object *Parameter_reportTimestamp_str;
    msgpack_object *Parameter_firstSeekFromEOF_int;

    uint32_t i, Parameter_array_size = 0;
    int ret = 0;

    Vector_Create(&profile->paramList);
    Vector_Create(&profile->staticParamList);
    Vector_Create(&profile->eMarkerList);
    Vector_Create(&profile->gMarkerList);
    Vector_Create(&profile->cachedReportList);

    Parameter_array = msgpack_get_map_value(value_map, "Parameter");
    if(Parameter_array == NULL)
    {
        T2Error("Parameter is NULL. Invalid Profile\n");
        return T2ERROR_FAILURE;
    }
    int profileParamCount = 0;
    MSGPACK_GET_ARRAY_SIZE(Parameter_array, Parameter_array_size);

    // Iterate through Map of multi profile configuration
    for( i = 0; i < Parameter_array_size; i++ )
    {

        char* paramtype;
        char* use;
        char* header;
        char* content;
        char* logfile;
        char* method;
        bool reportEmpty;
        bool trim;
        char* regex;
        int skipFrequency;
        int firstSeekFromEOF;
        reportTimestampFormat rtformat;

        header = NULL;
        content = NULL;
        logfile = NULL;
        skipFrequency = 0;
        firstSeekFromEOF = 0;
        paramtype = NULL;
        use = NULL;
        method = NULL;
        reportEmpty = false;
        trim = false;
        regex = NULL;
        rtformat =  REPORTTIMESTAMP_NONE;

        Parameter_array_map = msgpack_get_array_element(Parameter_array, i);

        Parameter_type_str = msgpack_get_map_value(Parameter_array_map, "type");
        if(NULL == Parameter_type_str)
        {
            continue;
        }
        msgpack_print(Parameter_type_str, msgpack_get_obj_name(Parameter_type_str));
        paramtype = msgpack_strdup(Parameter_type_str);

        Parameter_use_str = msgpack_get_map_value(Parameter_array_map, "use");
        msgpack_print(Parameter_use_str, msgpack_get_obj_name(Parameter_use_str));
        use = msgpack_strdup(Parameter_use_str);

        Parameter_reportEmpty_boolean = msgpack_get_map_value(Parameter_array_map, "reportEmpty");
        msgpack_print(Parameter_reportEmpty_boolean, msgpack_get_obj_name(Parameter_reportEmpty_boolean));
        MSGPACK_GET_NUMBER(Parameter_reportEmpty_boolean, reportEmpty);
        T2Debug("reportEmpty: %u\n", reportEmpty);

        Parameter_trim_boolean = msgpack_get_map_value(Parameter_array_map, "trim");
        msgpack_print(Parameter_trim_boolean, msgpack_get_obj_name(Parameter_trim_boolean));
        MSGPACK_GET_NUMBER(Parameter_trim_boolean, trim);
        T2Debug("trimParam: %u\n", trim);

        Parameter_regex_str =  msgpack_get_map_value(Parameter_array_map, "regex");
        msgpack_print(Parameter_regex_str, msgpack_get_obj_name(Parameter_regex_str));
        if(Parameter_regex_str != NULL)
        {
            regex = msgpack_strdup(Parameter_regex_str);
            T2Debug("regex param = %s\n", regex);
        }

        if(0 == msgpack_strcmp(Parameter_type_str, "dataModel"))
        {
            Parameter_name_str = msgpack_get_map_value(Parameter_array_map, "name");
            msgpack_print(Parameter_name_str, msgpack_get_obj_name(Parameter_name_str));
            header = msgpack_strdup(Parameter_name_str);

            Parameter_reference_str = msgpack_get_map_value(Parameter_array_map, "reference");
            msgpack_print(Parameter_reference_str, msgpack_get_obj_name(Parameter_reference_str));
            if(Parameter_reference_str == NULL)
            {
                T2Error("Reference property is mandatory for datamodel parameter. Ignore the adding of this parameter to paramlist\n");
                if(header != NULL)
                {
                    free(header);
                }
                if(paramtype != NULL)
                {
                    free(paramtype);
                }
                if(use != NULL)
                {
                    free(use);
                }
                if(regex != NULL)
                {
                    free(regex);
                }
                continue;
            }
            content = msgpack_strdup(Parameter_reference_str);
            if(NULL == header)
            {
                header = msgpack_strdup(Parameter_reference_str);
            }

            Parameter_method_str = msgpack_get_map_value(Parameter_array_map, "method");
            msgpack_print(Parameter_method_str, msgpack_get_obj_name(Parameter_method_str));
            method = msgpack_strdup(Parameter_method_str);
            Parameter_reportTimestamp_str = msgpack_get_map_value(Parameter_array_map, "reportTimestamp");
            if(method)
            {
                T2Debug("Method property is present and the value is %s\n", method);
                if(!(strcmp(method, "subscribe")))
                {
                    T2Info("Method is subscribe converting the parameter to event type\n");
                    // free now as we will change the values in the below code which will be freed later
                    free(header);
                    free(paramtype);
                    free(content);
                    //reportTimestamp parameter is applicable only when method is subscribe
                    if(Parameter_reportTimestamp_str != NULL)
                    {
                        msgpack_print(Parameter_reportTimestamp_str, msgpack_get_obj_name(Parameter_reportTimestamp_str));
                        if(0 == msgpack_strcmp(Parameter_reportTimestamp_str, "Unix-Epoch"))
                        {
                            rtformat =  REPORTTIMESTAMP_UNIXEPOCH;
                        }
                    }
                    header = msgpack_strdup(Parameter_reference_str);
                    paramtype = strdup("event");
                    content = strdup(T2REPORTCOMPONENT);
                    if(Parameter_name_str)
                    {
                        logfile = msgpack_strdup(Parameter_name_str);
                    }
                }
            }
        }
        else if(0 == msgpack_strcmp(Parameter_type_str, "event"))
        {

            Parameter_name_str = msgpack_get_map_value(Parameter_array_map, "name");
            msgpack_print(Parameter_name_str, msgpack_get_obj_name(Parameter_name_str));
            logfile = msgpack_strdup(Parameter_name_str);

            Parameter_eventName_str = msgpack_get_map_value(Parameter_array_map, "eventName");
            msgpack_print(Parameter_eventName_str, msgpack_get_obj_name(Parameter_eventName_str));
            header = msgpack_strdup(Parameter_eventName_str);

            Parameter_component_str = msgpack_get_map_value(Parameter_array_map, "component");
            msgpack_print(Parameter_component_str, msgpack_get_obj_name(Parameter_component_str));
            content = msgpack_strdup(Parameter_component_str);

            Parameter_reportTimestamp_str = msgpack_get_map_value(Parameter_array_map, "reportTimestamp");
            if(Parameter_reportTimestamp_str != NULL)
            {
                msgpack_print(Parameter_reportTimestamp_str, msgpack_get_obj_name(Parameter_reportTimestamp_str));
                if(0 == msgpack_strcmp(Parameter_reportTimestamp_str, "Unix-Epoch"))
                {
                    rtformat =  REPORTTIMESTAMP_UNIXEPOCH;
                }
            }

        }
        else if(0 == msgpack_strcmp(Parameter_type_str, "grep"))
        {

            Parameter_marker_str = msgpack_get_map_value(Parameter_array_map, "marker");
            msgpack_print(Parameter_marker_str, msgpack_get_obj_name(Parameter_marker_str));
            header = msgpack_strdup(Parameter_marker_str);

            Parameter_search_str = msgpack_get_map_value(Parameter_array_map, "search");
            msgpack_print(Parameter_search_str, msgpack_get_obj_name(Parameter_search_str));
            content = msgpack_strdup(Parameter_search_str);

            Parameter_logFile_str = msgpack_get_map_value(Parameter_array_map, "logFile");
            msgpack_print(Parameter_logFile_str, msgpack_get_obj_name(Parameter_logFile_str));
            logfile = msgpack_strdup(Parameter_logFile_str);

            if((Parameter_firstSeekFromEOF_int = msgpack_get_map_value(Parameter_array_map, "firstSeekFromEOF")) != NULL )
            {
                msgpack_print(Parameter_firstSeekFromEOF_int, msgpack_get_obj_name(firstSeekFromEOF));
                MSGPACK_GET_NUMBER(Parameter_firstSeekFromEOF_int, firstSeekFromEOF);
            }

        }
        else
        {
            T2Error("%s Unknown parameter type %s \n", __FUNCTION__, paramtype);
            free(paramtype);
            free(use);
            if(regex != NULL)
            {
                free(regex);
            }
            continue;
        }

        T2Debug("%s : reportTimestamp = %d\n", __FUNCTION__, rtformat);
        if(header != NULL && content != NULL)
        {
            ret = addParameter(profile, header, content, logfile, skipFrequency, firstSeekFromEOF, paramtype, use, reportEmpty, rtformat, trim, regex);
        }
        else
        {
            T2Error("%s Error in adding parameter to profile %s \n", __FUNCTION__, profile->name); // header and content is mandatory for adding the parameter
            if(header != NULL)
            {
                free(header);
            }
            if(content != NULL)
            {
                free(content);
            }
            if(logfile != NULL)
            {
                free(logfile);
            }
            if(use != NULL)
            {
                free(use);
            }
            if(paramtype != NULL)
            {
                free(paramtype);
            }
            if(method != NULL)
            {
                free(method);
            }
            if(regex != NULL)
            {
                free(regex);
            }
            continue;
        }
        /* Add Multiple Report Profile Parameter */
        if(T2ERROR_SUCCESS != ret)
        {
            T2Error("%s Error in adding parameter to profile %s \n", __FUNCTION__, profile->name);
        }
        else
        {
            T2Debug("Added parameter:%s \n", header);
            profileParamCount++;
        }
        free(header);
        free(content);
        free(logfile);
        free(use);
        free(paramtype);
        free(method);
    }
    // GrepMarkerlist expects the list to be sorted based on logfile names
    Vector_Sort(profile->gMarkerList,  sizeof(GrepMarker*), compareLogFileNames);

    T2Debug("Added parameter count:%d \n", profileParamCount);
    return T2ERROR_SUCCESS;
}

T2ERROR encodingSetMsgpack (Profile *profile, msgpack_object* value_map)
{
    if(profile == NULL || value_map == NULL)
    {
        T2Error("Profile or value is NULL\n");
        return T2ERROR_INVALID_ARGS;
    }
    msgpack_object *JSONEncoding_map;
    msgpack_object *ReportFormat_str;
    msgpack_object *ReportTimestamp_str;
    JSONEncoding_map = msgpack_get_map_value(value_map, "JSONEncoding");

    ReportFormat_str = msgpack_get_map_value(JSONEncoding_map, "ReportFormat");
    msgpack_print(ReportFormat_str, msgpack_get_obj_name(ReportFormat_str));
    if(0 == msgpack_strcmp(ReportFormat_str, "NameValuePair"))
    {
        profile->jsonEncoding->reportFormat = JSONRF_KEYVALUEPAIR;
    }
    else if(0 == msgpack_strcmp(ReportFormat_str, "ObjectHierarchy"))
    {
        profile->jsonEncoding->reportFormat = JSONRF_OBJHIERARCHY;
    }
    else    /* defaulting it to NameValuePair */
    {
        profile->jsonEncoding->reportFormat = JSONRF_KEYVALUEPAIR;
    }

    ReportTimestamp_str = msgpack_get_map_value(JSONEncoding_map, "ReportTimestamp");
    msgpack_print(ReportTimestamp_str, msgpack_get_obj_name(ReportTimestamp_str));
    if(0 == msgpack_strcmp(ReportTimestamp_str, "None"))
    {
        profile->jsonEncoding->tsFormat = TIMESTAMP_NONE;
    }
    else if(0 == msgpack_strcmp(ReportTimestamp_str, "Unix-Epoch"))
    {
        profile->jsonEncoding->tsFormat = TIMESTAMP_UNIXEPOCH;
    }
    else if(0 == msgpack_strcmp(ReportTimestamp_str, "ISO-8601"))
    {
        profile->jsonEncoding->tsFormat = TIMESTAMP_ISO_8601;
    }
    else   /*default value for profile->jsonEncoding->tsFormat is None */
    {
        profile->jsonEncoding->tsFormat = TIMESTAMP_NONE;
    }
    return T2ERROR_SUCCESS;
}

T2ERROR protocolSetMsgpack(Profile *profile, msgpack_object* value_map, msgpack_object* Protocol_str)
{
    if(profile == NULL || value_map == NULL)
    {
        T2Error("Profile or value_map is  NULL\n");
        return T2ERROR_INVALID_ARGS;
    }
    uint32_t RequestURIParameter_array_size = 0;
    uint32_t i, RbusMethodParameter_array_size = 0;
    msgpack_object *HTTP_map;
    msgpack_object *URL_str;
    msgpack_object *Compression_str;
    msgpack_object *Method_str;
    msgpack_object *RbusMethod_map;
    msgpack_object *RequestURIParameter_array;
    msgpack_object *RbusMethodParameter_array;
    msgpack_object *RequestURIParameter_array_map;
    msgpack_object *RbusMethodIParameter_array_map;
    msgpack_object *RequestURIParameter_Name_str;
    msgpack_object *RequestURIParameter_Reference_str;
    msgpack_object *RequestRbusParameter_Name_str;
    msgpack_object *RequestRbusParameter_Value_str;

    if(0 == msgpack_strcmp(Protocol_str, "HTTP"))
    {
        HTTP_map = msgpack_get_map_value(value_map, "HTTP");

        URL_str = msgpack_get_map_value(HTTP_map, "URL");
        msgpack_print(URL_str, msgpack_get_obj_name(URL_str));

        Compression_str = msgpack_get_map_value(HTTP_map, "Compression");
        msgpack_print(Compression_str, msgpack_get_obj_name(Compression_str));

        Method_str = msgpack_get_map_value(HTTP_map, "Method");
        msgpack_print(Method_str, msgpack_get_obj_name(Method_str));

        profile->t2HTTPDest->URL = msgpack_strdup(URL_str);
        profile->t2HTTPDest->Compression = COMP_NONE;  /*in 1911_sprint default none*/
        profile->t2HTTPDest->Method = HTTP_POST; /*1911_sprint default to POST */

        Vector_Create(&(profile->t2HTTPDest->RequestURIparamList));
        RequestURIParameter_array = msgpack_get_map_value(HTTP_map, "RequestURIParameter");
        int httpParamCount = 0;
        MSGPACK_GET_ARRAY_SIZE(RequestURIParameter_array, RequestURIParameter_array_size);
        for( i = 0; i < RequestURIParameter_array_size; i++ )
        {
            char* href;
            char* hname;
            int httpret = 0;

            href = NULL;
            hname = NULL;
            RequestURIParameter_array_map = msgpack_get_array_element(RequestURIParameter_array, i);

            RequestURIParameter_Reference_str = msgpack_get_map_value(RequestURIParameter_array_map, "Reference");
            msgpack_print(RequestURIParameter_Reference_str, msgpack_get_obj_name(RequestURIParameter_Reference_str));
            href = msgpack_strdup(RequestURIParameter_Reference_str);
            if(NULL != href) /*if reference is empty avoid adding this object*/
            {
                RequestURIParameter_Name_str = msgpack_get_map_value(RequestURIParameter_array_map, "Name");
                msgpack_print(RequestURIParameter_Name_str, msgpack_get_obj_name(RequestURIParameter_Name_str));
                hname = msgpack_strdup(RequestURIParameter_Name_str);
                if(NULL == hname)
                {
                    hname = msgpack_strdup(RequestURIParameter_Reference_str);
                }
                httpret = addhttpURIreqParameter(profile, hname, href);
                if(T2ERROR_SUCCESS != httpret)
                {
                    T2Error("%s Error in adding request URIparameterReference: %s for the profile: %s \n", __FUNCTION__, href, profile->name);
                }
                else
                {
                    T2Debug("Added  request URIparameterReference: %s \n", href);
                    httpParamCount++;
                }

                free(href);
                free(hname);
            }
        }
        T2Debug("Added URI parameter count:%d \n", httpParamCount);
    }
    if ( 0 == msgpack_strcmp(Protocol_str, "RBUS_METHOD") )
    {

        RbusMethod_map = msgpack_get_map_value(value_map, "RBUS_METHOD");
        Vector_Create(&(profile->t2RBUSDest->rbusMethodParamList));
        RbusMethodParameter_array = msgpack_get_map_value(RbusMethod_map, "Parameters");
        int rbusMethodParamCount = 0;
        MSGPACK_GET_ARRAY_SIZE(RbusMethodParameter_array, RbusMethodParameter_array_size);

        for( i = 0; i < RbusMethodParameter_array_size; i++ )
        {
            char* name = NULL;
            char* value = NULL;
            int httpret = 0;

            RbusMethodIParameter_array_map = msgpack_get_array_element(RbusMethodParameter_array, i);

            RequestRbusParameter_Name_str = msgpack_get_map_value(RbusMethodIParameter_array_map, "name");
            msgpack_print(RequestRbusParameter_Name_str, msgpack_get_obj_name(RequestRbusParameter_Name_str));
            name = msgpack_strdup(RequestRbusParameter_Name_str);

            RequestRbusParameter_Value_str = msgpack_get_map_value(RbusMethodIParameter_array_map, "value");
            msgpack_print(RequestRbusParameter_Value_str, msgpack_get_obj_name(RequestRbusParameter_Value_str));
            value = msgpack_strdup(RequestRbusParameter_Value_str);

            if( name && value )
            {
                httpret = addRbusMethodParameter(profile, name, value);
                if(T2ERROR_SUCCESS != httpret)
                {
                    T2Error("%s Error in adding request URIparameterReference: %s for the profile: %s \n", __FUNCTION__, name, profile->name);
                }
                else
                {
                    T2Debug("Added  request URIparameterReference: %s \n", name);
                    rbusMethodParamCount++;
                }
            }
            free(name);
            free(value);
        }
        T2Debug("Added Rbus_method parameter count:%d \n", rbusMethodParamCount);
    }
    return T2ERROR_SUCCESS;
}

T2ERROR processMsgPackConfiguration(msgpack_object *profiles_array_map, Profile **profile_dp)
{
    msgpack_object *name_str;
    msgpack_object *hash_str;
    msgpack_object *value_map;
    msgpack_object *Description_str;
    msgpack_object *Version_str;
    msgpack_object *Protocol_str;
    msgpack_object *EncodingType_str;
    msgpack_object *ReportingInterval_u64;
    msgpack_object *TimeReference_str;
    msgpack_object *ActivationTimeout_u64;
    msgpack_object *RootName_str;
    msgpack_object *GenerateNow_boolean;
    msgpack_object *DeleteOnTimout_boolean;
    msgpack_object *TriggerConditionArray;
    msgpack_object *RbusMethod_str;
    msgpack_object *ReportingAdjustments_map;
    msgpack_object *ReportOnUpdate_boolean = NULL;
    msgpack_object *FirstReportInterval_u64 = NULL;
    msgpack_object *MaxUploadLatency_u64 = NULL;
    msgpack_object *RbusMethod_map;
    int ret = 0;

    T2Debug("%s --in\n", __FUNCTION__);
    Profile *profile = (Profile *) malloc(sizeof(Profile));
    if(profile == NULL)
    {
        T2Error("Malloc error exiting: can not allocate memory to create profile \n");
        return T2ERROR_MEMALLOC_FAILED;
    }
    memset(profile, 0, sizeof(Profile));


    name_str = msgpack_get_map_value(profiles_array_map, "name");
    msgpack_print(name_str, msgpack_get_obj_name(name_str));
    profile->name = msgpack_strdup(name_str);

    hash_str = msgpack_get_map_value(profiles_array_map, "hash");
    if(NULL == hash_str)
    {
        hash_str = msgpack_get_map_value(profiles_array_map, "versionHash");
    }
    msgpack_print(hash_str, msgpack_get_obj_name(hash_str));
    profile->hash = msgpack_strdup(hash_str);

    value_map = msgpack_get_map_value(profiles_array_map, "value");

    if(value_map)
    {
        ret = verifyMsgPckTriggerCondition(value_map);
        if(ret == T2ERROR_FAILURE)
        {
            if(profile->name)
            {
                free(profile->name);
            }
            if(profile->hash)
            {
                free(profile->hash);
            }
            if(profile->timeRef != NULL)
            {
                free(profile->timeRef);
            }
            if(profile)
            {
                free(profile);
            }
            T2Error("TriggerCondition is invalid, unable to create profile\n");
            return T2ERROR_FAILURE;
        }
    }
    ReportingAdjustments_map = msgpack_get_map_value(value_map, "ReportingAdjustments");
    if(ReportingAdjustments_map)
    {
        ReportOnUpdate_boolean = msgpack_get_map_value(ReportingAdjustments_map, "ReportOnUpdate");
        FirstReportInterval_u64 = msgpack_get_map_value(ReportingAdjustments_map, "FirstReportingInterval");
        MaxUploadLatency_u64 = msgpack_get_map_value(ReportingAdjustments_map, "MaxUploadLatency");
    }

    ReportingInterval_u64 = msgpack_get_map_value(value_map, "ReportingInterval");
    msgpack_print(ReportingInterval_u64, msgpack_get_obj_name(ReportingInterval_u64));
    MSGPACK_GET_NUMBER(ReportingInterval_u64, profile->reportingInterval);

    msgpack_print(MaxUploadLatency_u64, msgpack_get_obj_name(MaxUploadLatency_u64));
    MSGPACK_GET_NUMBER(MaxUploadLatency_u64, profile->maxUploadLatency);

    if(ReportingInterval_u64 && MaxUploadLatency_u64 && ((profile->reportingInterval * 1000) < profile->maxUploadLatency))
    {
        T2Error("MaxUploadLatency is greater than reporting interval. Invalid Profile\n");
        if(profile->name)
        {
            free(profile->name);
        }
        if(profile->hash)
        {
            free(profile->hash);
        }
        if(profile->timeRef != NULL)
        {
            free(profile->timeRef);
        }
        if(profile)
        {
            free(profile);
        }
        return T2ERROR_FAILURE;
    }

    TriggerConditionArray = msgpack_get_map_value(value_map, "TriggerCondition");
    if(TriggerConditionArray == NULL && !ReportingInterval_u64)
    {
        T2Error("If TriggerCondition is not given ReportingInterval parameter is mandatory. Reporting interval is not configured for this profile..Invalid Profile\n");
        if(profile->name)
        {
            free(profile->name);
        }
        if(profile->hash)
        {
            free(profile->hash);
        }
        if(profile->timeRef != NULL)
        {
            free(profile->timeRef);
        }
        if(profile)
        {
            free(profile);
        }
        return T2ERROR_FAILURE;
    }

    //Default values
    profile->activationTimeoutPeriod = INFINITE_TIMEOUT;
    profile->generateNow = false;
    profile->deleteonTimeout = false;
    profile->reportOnUpdate = false;
    profile->firstReportingInterval = 0;
    profile->maxUploadLatency = 0;
    profile->timeRef = NULL;
    profile->isSchedulerstarted = false;
    profile->saveSeekConfig = false;
    profile->checkPreviousSeek = false;

    DeleteOnTimout_boolean = msgpack_get_map_value(value_map, "DeleteOnTimeout");
    msgpack_print(DeleteOnTimout_boolean, msgpack_get_obj_name(DeleteOnTimeout_boolean));
    MSGPACK_GET_NUMBER(DeleteOnTimout_boolean, profile->deleteonTimeout);
    T2Debug("profile->deleteonTimeout: %u\n", profile->deleteonTimeout);

    GenerateNow_boolean = msgpack_get_map_value(value_map, "GenerateNow");
    msgpack_print(GenerateNow_boolean, msgpack_get_obj_name(GenerateNow_boolean));
    MSGPACK_GET_NUMBER(GenerateNow_boolean, profile->generateNow);
    T2Debug("profile->generateNow: %u\n", profile->generateNow);

    ActivationTimeout_u64 = msgpack_get_map_value(value_map, "ActivationTimeOut");
    msgpack_print(ActivationTimeout_u64, msgpack_get_obj_name(ActivationTimeout_u64));
    MSGPACK_GET_NUMBER(ActivationTimeout_u64, profile->activationTimeoutPeriod);

    TimeReference_str = msgpack_get_map_value(value_map, "TimeReference");
    msgpack_print(TimeReference_str, msgpack_get_obj_name(TimeReference_str));

    T2ERROR retval = time_param_MsgPackReporting_Adjustments_valid_set(profile, ReportingInterval_u64, ActivationTimeout_u64, TimeReference_str, ReportOnUpdate_boolean,  FirstReportInterval_u64, MaxUploadLatency_u64 );
    if(retval != T2ERROR_SUCCESS)
    {
        T2Error("Time parameters are not valid\n");
        return retval;
    }
    if(TriggerConditionArray && (!profile->generateNow) && (!profile->reportingInterval))
    {
        profile->reportingInterval = UINT_MAX;
    }
    if(profile->reportingInterval)
    {
        T2Debug("profile->reportingInterval: %u\n", profile->reportingInterval);
    }
    if(profile->reportingInterval > profile->activationTimeoutPeriod)  //When Reporting interval and activation time is not given in the profile it takes the max value INFINITE TIMEOUT or UINT_MAX
    {
        T2Error("activationTimeoutPeriod is less than reporting interval. Max values assigned is inappropriate .. Invalid profile: %s \n", profile->name);
        if(profile->name != NULL)
        {
            free(profile->name);
        }
        if(profile->hash != NULL)
        {
            free(profile->hash);
        }
        if(profile->timeRef != NULL)
        {
            free(profile->timeRef);
        }
        if(profile != NULL)
        {
            free(profile);
        }
        return T2ERROR_FAILURE;
    }

    Protocol_str = msgpack_get_map_value(value_map, "Protocol");
    EncodingType_str = msgpack_get_map_value(value_map, "EncodingType");
    if(Protocol_str == NULL || EncodingType_str == NULL)
    {
        T2Error("Incomplete Profile information, ignoring profile");
        if(profile->name != NULL)
        {
            free(profile->name);
        }
        if(profile->hash != NULL)
        {
            free(profile->hash);
        }
        if(profile->timeRef != NULL)
        {
            free(profile->timeRef);
        }
        if(profile != NULL)
        {
            free(profile);
        }
        return T2ERROR_FAILURE;
    }
    msgpack_print(EncodingType_str, msgpack_get_obj_name(EncodingType_str)); //ENCODING PARAMETERS
    profile->encodingType = msgpack_strdup(EncodingType_str);
    if(0 == msgpack_strcmp(EncodingType_str, "JSON"))
    {
        msgpack_object *JSONEncoding_map = msgpack_get_map_value(value_map, "JSONEncoding");
        msgpack_object *ReportFormat_str = msgpack_get_map_value(JSONEncoding_map, "ReportFormat");
        if(ReportFormat_str == NULL)
        {
            T2Error("Incomplete Profile Information, unable to create profile\n");
            free(profile->name);
            free(profile->hash);
            if(profile->timeRef != NULL)
            {
                free(profile->timeRef);
            }
            free(profile);
            return T2ERROR_FAILURE;
        }

        profile->jsonEncoding = (JSONEncoding *) malloc(sizeof(JSONEncoding));
        if(NULL == profile->jsonEncoding)
        {
            free(profile->name);
            free(profile->hash);
            if(profile->timeRef != NULL)
            {
                free(profile->timeRef);
            }
            free(profile);
            T2Error("Malloc error exiting: can not allocate memory to create profile->jsonEncoding \n");
            return T2ERROR_MEMALLOC_FAILED;
        }
    }

    msgpack_print(Protocol_str, msgpack_get_obj_name(Protocol_str)); //PROTOCOL PARAMETERS
    profile->protocol = msgpack_strdup(Protocol_str);
    if(0 == msgpack_strcmp(Protocol_str, "HTTP"))
    {
        profile->t2HTTPDest = (T2HTTP *) malloc(sizeof(T2HTTP));
        if(NULL == profile->t2HTTPDest)
        {
            free(profile->name);
            free(profile->hash);
            if(profile->timeRef != NULL)
            {
                free(profile->timeRef);
            }
            if(profile->protocol != NULL)
            {
                free(profile->protocol);
            }
            free(profile);
            T2Error("Malloc error exiting: can not allocate memory to create profile->t2HTTPDest \n");
            return T2ERROR_MEMALLOC_FAILED;
        }
        msgpack_object *HTTP_map = msgpack_get_map_value(value_map, "HTTP");
        msgpack_object *URL_str = msgpack_get_map_value(HTTP_map, "URL");
        msgpack_object *Compression_str = msgpack_get_map_value(HTTP_map, "Compression");
        msgpack_object *Method_str = msgpack_get_map_value(HTTP_map, "Method");
        if(URL_str == NULL || Compression_str == NULL || Method_str == NULL)
        {
            T2Error("Incomplete Profile Information, unable to create profile\n");
            free(profile->name);
            free(profile->hash);
            if(profile->timeRef != NULL)
            {
                free(profile->timeRef);
            }
            free(profile->t2HTTPDest);
            free(profile->jsonEncoding);
            free(profile);
            return T2ERROR_FAILURE;
        }
    }
    else if(0 == msgpack_strcmp(Protocol_str, "RBUS_METHOD"))
    {
        profile->t2RBUSDest = (T2RBUS *) malloc(sizeof(RBUSMethodParam));
        if(NULL == profile->t2RBUSDest)
        {
            free(profile->name);
            free(profile->hash);
            if(profile->timeRef != NULL)
            {
                free(profile->timeRef);
            }
            if(profile->protocol != NULL)
            {
                free(profile->protocol);
            }
            free(profile);
            T2Error("Malloc error exiting: can not allocate memory to create profile->t2RBUSDest \n");
            return T2ERROR_MEMALLOC_FAILED;
        }
        RbusMethod_map = msgpack_get_map_value(value_map, "RBUS_METHOD");
        RbusMethod_str = msgpack_get_map_value(RbusMethod_map, "Method");
        msgpack_print(RbusMethod_str, msgpack_get_obj_name(RbusMethod_str));
        profile->t2RBUSDest->rbusMethodName = msgpack_strdup(RbusMethod_str);
        if(profile->t2RBUSDest->rbusMethodName == NULL || strcmp(profile->t2RBUSDest->rbusMethodName, "" ) == 0)
        {
            T2Error("RBUS Method not configured. Ignoring profile %s \n", profile->name);
            free(profile->t2RBUSDest->rbusMethodName );
            free(profile->t2RBUSDest);
            if(profile->timeRef != NULL)
            {
                free(profile->timeRef);
            }
            if(profile->protocol != NULL)
            {
                free(profile->protocol);
            }
            free(profile->name);
            free(profile->hash);
            free(profile->jsonEncoding);
            free(profile);
            return T2ERROR_FAILURE;
        }

        if(profile->t2RBUSDest->rbusMethodName && !rbusCheckMethodExists(profile->t2RBUSDest->rbusMethodName))
        {
            T2Warning("WARN: no method provider: %s %s\n", profile->name, profile->t2RBUSDest->rbusMethodName);
        }
    }
    else
    {
        T2Error("Unsupported report sending protocol \n" );
        free(profile->name);
        free(profile->hash);

        if(profile->timeRef != NULL)
        {
            free(profile->timeRef);
        }
        if(profile->protocol != NULL)
        {
            free(profile->protocol);
        }
        if(profile->encodingType != NULL)
        {
            free(profile->encodingType);
        }
        free(profile);
        return T2ERROR_FAILURE;
    }

    Description_str = msgpack_get_map_value(value_map, "Description");
    msgpack_print(Description_str, msgpack_get_obj_name(Description_str));
    profile->Description = msgpack_strdup(Description_str);

    Version_str = msgpack_get_map_value(value_map, "Version");
    msgpack_print(Version_str, msgpack_get_obj_name(Version_str));
    profile->version = msgpack_strdup(Version_str);

    RootName_str = msgpack_get_map_value(value_map, "RootName");
    msgpack_print(RootName_str, msgpack_get_obj_name(RootName_str));
    profile->RootName = msgpack_strdup(RootName_str);
    if(profile->RootName == NULL)
    {
        profile->RootName = strdup("Report");
    }
    T2Debug("profile->RootName: %s\n", profile->RootName);
    //Protocol Configuration
    retval = protocolSetMsgpack(profile, value_map, Protocol_str); //Protocol Parameter
    if(retval != T2ERROR_SUCCESS)
    {
        T2Error("Protocol Parameter is invalid\n");
        return retval;
    }
    //Encoding Configuration
    retval = encodingSetMsgpack(profile, value_map); //Encoding parameter
    if(retval != T2ERROR_SUCCESS)
    {
        T2Error("Encoding parameter is invalid\n");
        return retval;
    }
    /* Parameter Markers configuration */
    retval = addParameterMsgpack_marker_config(profile, value_map);
    if(retval != T2ERROR_SUCCESS)
    {
        T2Error("Invalid parameter in the profile\n");
    }
    addMsgPckTriggerCondition(profile, value_map);
    *profile_dp = profile;
    T2Debug("%s --out\n", __FUNCTION__);
    return retval;
}
