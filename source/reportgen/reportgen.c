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

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <stdbool.h>

#include "reportgen.h"
#include "t2log_wrapper.h"
#include "telemetry2_0.h"
#include "t2common.h"
#include "dcautil.h"
#include "busInterface.h"

static bool checkForEmptyString( char* valueString )
{
    bool isEmpty = false ;
    T2Debug("%s ++in \n", __FUNCTION__);

    if(valueString)
    {
        // rbusInterface implementation explicitly adds string as "NULL" for parameters which doesn't exist on device
        // Consider "NULL" string value as qualified for empty string
        if(strlen(valueString) < 1 || !strncmp(valueString, " ", 1) || !strncmp(valueString, "NULL", 4))
        {
            isEmpty = true ;
        }
    }
    else
    {
        isEmpty = true;
    }
    T2Debug("%s --Out with return status as %s \n", __FUNCTION__, (isEmpty ? "true" : "false"));
    return isEmpty;
}

void freeParamValueSt(tr181ValStruct_t **valStructs, int valSize)
{
    int i;
    if(valStructs == NULL)
    {
        return ;
    }
    if(valSize)
    {
        for( i = 0; i < valSize; i++ )
        {
            if(valStructs[i])
            {
                if(valStructs[i]->parameterName)
                {
                    free(valStructs[i]->parameterName);
                }
                if(valStructs[i]->parameterValue)
                {
                    free(valStructs[i]->parameterValue);
                }
                free(valStructs[i]);
                valStructs[i] = NULL ;
            }
        }
        free(valStructs);
        valStructs = NULL;
    }
}

void freeProfileValues(void *data)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if(data != NULL)
    {
        profileValues *profVal = (profileValues *) data;
        freeParamValueSt(profVal->paramValues, profVal->paramValueCount);
        free(profVal);
    }
    T2Debug("%s --Out\n", __FUNCTION__);
}

// convert vector to json array it is responsbility of
// caller function to free the vector
void convertVectorToJson(cJSON *output, Vector *input)
{
    if(!output)
    {
        output = cJSON_CreateArray();
        if(output == NULL)
        {
            T2Error("cJSON_CreateArray failed.\n");
            return;
        }
    }
    int i, vectorSize = (int) Vector_Size(input);
    for(i = 0; i < vectorSize; i++)
    {
        char* eventValue = Vector_At(input, i);
        cJSON_AddItemToArray(output, cJSON_CreateString(eventValue));
    }
}

T2ERROR destroyJSONReport(cJSON *jsonObj)
{
    if(jsonObj == NULL)
    {
        T2Error("jsonObj Argument is NULL\n");
        return T2ERROR_INVALID_ARGS;
    }
    cJSON_Delete(jsonObj);
    return T2ERROR_SUCCESS;
}

void trimLeadingAndTrailingws(char* string)
{
    T2Debug("%s ++in \n", __FUNCTION__);
    int i = 0, j = 0;

    for(i = 0; string[i] == ' ' || string[i] == '\t'; i++); // To find first non-space character in the string

    for(j = 0; string[i]; i++) //To remove the leading whitespaces
    {
        string[j++] = string[i];
    }
    string[j] = '\0';

    for(i = 0; string[i] != '\0'; i++)
    {
        if(string[i] != ' ' && string[i] != '\t') // To remove the trailing whitespaces
        {
            j = i; // To get the last non-space character in the string
        }
    }

    string[j + 1] = '\0';

    T2Debug("%s --Out \n", __FUNCTION__);

}

T2ERROR encodeParamResultInJSON(cJSON *valArray, Vector *paramNameList, Vector *paramValueList)
{
    if(valArray == NULL || paramNameList == NULL || paramValueList == NULL)
    {
        T2Error("Invalid or NULL arguments\n");
        return T2ERROR_INVALID_ARGS;
    }
    size_t index = 0;
    T2Debug("%s ++in \n", __FUNCTION__);

    for(; index < Vector_Size(paramNameList); index++)
    {
        Param* param = (Param *)Vector_At(paramNameList, index);
        tr181ValStruct_t **paramValues = ((profileValues *)Vector_At(paramValueList, index))->paramValues;
        if(param == NULL || paramValues == NULL )
        {
            // Ignore tr181 params returning null values in report
            continue ;
        }
        int paramValCount = ((profileValues *)Vector_At(paramValueList, index))->paramValueCount;
        T2Debug("Parameter Name : %s valueCount = %d\n", param->name, paramValCount);
        if(paramValCount == 0)
        {
            if(param->reportEmptyParam)
            {
                cJSON *arrayItem = cJSON_CreateObject();
                if(arrayItem == NULL)
                {
                    T2Error("cJSON_CreateObject failed.. arrayItem is NULL \n");
                    return T2ERROR_FAILURE;
                }
                T2Info("Paramter was not successfully retrieved... \n");
                if(cJSON_AddStringToObject(arrayItem, param->name, "NULL")  == NULL)
                {
                    T2Error("cJSON_AddStringToObject failed.\n");
                    cJSON_Delete(arrayItem);
                    return T2ERROR_FAILURE;
                }
                cJSON_AddItemToArray(valArray, arrayItem);
            }
            else
            {
                continue ;
            }
        }
        else if(paramValCount == 1) // Single value
        {
            if(paramValues[0])
            {
                if(param->reportEmptyParam || !checkForEmptyString(paramValues[0]->parameterValue))
                {
                    cJSON *arrayItem = cJSON_CreateObject();
                    if(arrayItem == NULL)
                    {
                        T2Error("cJSON_CreateObject failed.. arrayItem is NULL \n");
                        return T2ERROR_FAILURE;
                    }
                    if(param->trimParam)
                    {
                        trimLeadingAndTrailingws(paramValues[0]->parameterValue);
                    }
                    if(param->regexParam != NULL)
                    {
                        regex_t regpattern;
                        int rc = 0;
                        size_t nmatch = 1;
                        regmatch_t pmatch[2];
                        char string[256] = {'\0'};
                        rc = regcomp(&regpattern, param->regexParam, REG_EXTENDED);
                        if(rc != 0)
                        {
                            T2Warning("regcomp() failed, returning nonzero (%d)\n", rc);
                        }
                        else
                        {
                            T2Debug("regcomp() successful, returning value (%d)\n", rc);
                            rc = regexec(&regpattern, paramValues[0]->parameterValue, nmatch, pmatch, 0);
                            if(rc != 0)
                            {
                                T2Warning("regexec() failed, Failed to match '%s' with '%s',returning %d.\n", paramValues[0]->parameterValue, param->regexParam, rc);
                                free(paramValues[0]->parameterValue);
                                paramValues[0]->parameterValue = strdup("");
                            }
                            else
                            {
                                T2Debug("regexec successful, Match is found %.*s\n", pmatch[0].rm_eo - pmatch[0].rm_so, &paramValues[0]->parameterValue[pmatch[0].rm_so]);
                                sprintf(string, "%.*s", pmatch[0].rm_eo - pmatch[0].rm_so, &paramValues[0]->parameterValue[pmatch[0].rm_so]);
                                free(paramValues[0]->parameterValue);
                                paramValues[0]->parameterValue = strdup(string);
                            }
                            regfree(&regpattern);
                        }
                    }
                    switch(paramValues[0]->type) {
                        case TR181_TYPE_BOOLEAN: {
                            bool b = false;
                            // Convert "true"/"false"/"1"/"0" to boolean
                            if(paramValues[0]->parameterValue && 
                               (strcasecmp(paramValues[0]->parameterValue, "true") == 0 || 
                                strcmp(paramValues[0]->parameterValue, "1") == 0)) {
                                b = true;
                            }
                            if(cJSON_AddBoolToObject(arrayItem, param->name, b) == NULL){
                                T2Error("cJSON_AddBoolToObject failed.\n");
                                cJSON_Delete(arrayItem);
                                return T2ERROR_FAILURE;
                            }
                            break;
                        }
                        case TR181_TYPE_INT:
                        case TR181_TYPE_LONG: {
                            long long ll = paramValues[0]->parameterValue ? strtoll(paramValues[0]->parameterValue, NULL, 10) : 0;
                            if(cJSON_AddNumberToObject(arrayItem, param->name, (double) ll) == NULL){
                                T2Error("cJSON_AddNumberToObject failed.\n");
                                cJSON_Delete(arrayItem);
                                return T2ERROR_FAILURE;
                            }
                            break;
                        }
                        case TR181_TYPE_UNSIGNED:
                        case TR181_TYPE_UNSIGNED_LONG: {
                            unsigned long long ull = paramValues[0]->parameterValue ? strtoull(paramValues[0]->parameterValue, NULL, 10) : 0;
                            if(cJSON_AddNumberToObject(arrayItem, param->name, (double) ull) == NULL){
                                T2Error("cJSON_AddNumberToObject failed.\n");
                                cJSON_Delete(arrayItem);
                                return T2ERROR_FAILURE;
                            }
                            break;
                        }
                        case TR181_TYPE_FLOAT:
                        case TR181_TYPE_DOUBLE: {
                            double d = paramValues[0]->parameterValue ? strtod(paramValues[0]->parameterValue, NULL) : 0.0;
                            if(cJSON_AddNumberToObject(arrayItem, param->name, d) == NULL){
                                T2Error("cJSON_AddNumberToObject failed.\n");
                                cJSON_Delete(arrayItem);
                                return T2ERROR_FAILURE;
                            }
                            break;
                        }
                        case TR181_TYPE_STRING:
                        case TR181_TYPE_DATETIME:
                        case TR181_TYPE_BASE64:
                        default: {
                                if(cJSON_AddStringToObject(arrayItem, param->name, paramValues[0]->parameterValue) == NULL){
                                T2Error("cJSON_AddStringToObject failed.\n");
                                cJSON_Delete(arrayItem);
                                return T2ERROR_FAILURE;
                            }
                        }
                    } // end switch
                    cJSON_AddItemToArray(valArray, arrayItem);
                }
            }
        }
        else
        {
            cJSON *valList = NULL;
            cJSON *valItem = NULL;
            int valIndex = 0;
            bool isTableEmpty = true ;
            cJSON *arrayItem = cJSON_CreateObject();
            if(arrayItem == NULL)
            {
                T2Error("cJSON_CreateObject failed.. arrayItem is NULL \n");
                return T2ERROR_FAILURE;
            }
            cJSON_AddItemToObject(arrayItem, param->name, valList = cJSON_CreateArray());
            for (; valIndex < paramValCount; valIndex++)
            {
                if(paramValues[valIndex])
                {
                    if(param->reportEmptyParam || !checkForEmptyString(paramValues[0]->parameterValue))
                    {
                        valItem = cJSON_CreateObject();
                        if(valItem  == NULL)
                        {
                            T2Error("cJSON_CreateObject failed.. valItem is NULL \n");
                            cJSON_Delete(arrayItem);
                            return T2ERROR_FAILURE;
                        }
                        if(param->trimParam)
                        {
                            trimLeadingAndTrailingws(paramValues[valIndex]->parameterValue);
                        }
                        if(param->regexParam != NULL)
                        {
                            regex_t regpattern;
                            int rc = 0;
                            size_t nmatch = 1;
                            regmatch_t pmatch[2];
                            char string[256] = {'\0'};
                            rc = regcomp(&regpattern, param->regexParam, REG_EXTENDED);
                            if(rc != 0)
                            {
                                T2Warning("regcomp() failed, returning nonzero (%d)\n", rc);
                            }
                            else
                            {
                                T2Debug("regcomp() successful, returning value (%d)\n", rc);
                                rc = regexec(&regpattern, paramValues[valIndex]->parameterValue, nmatch, pmatch, 0);
                                if(rc != 0)
                                {
                                    T2Warning("regexec() failed, Failed to match '%s' with '%s',returning %d.\n", paramValues[valIndex]->parameterValue, param->regexParam, rc);
                                    free(paramValues[valIndex]->parameterValue);
                                    paramValues[valIndex]->parameterValue = strdup("");
                                }
                                else
                                {
                                    T2Debug("regexec successful, Match is found %.*s\n", pmatch[0].rm_eo - pmatch[0].rm_so, &paramValues[valIndex]->parameterValue[pmatch[0].rm_so]);
                                    sprintf(string, "%.*s", pmatch[0].rm_eo - pmatch[0].rm_so, &paramValues[valIndex]->parameterValue[pmatch[0].rm_so]);
                                    free(paramValues[valIndex]->parameterValue);
                                    paramValues[valIndex]->parameterValue = strdup(string);
                                }
                                regfree(&regpattern);
                            }
                        }

		        switch(paramValues[valIndex]->type) {
                            case TR181_TYPE_BOOLEAN: {
                                bool b = false;
                                if(paramValues[valIndex]->parameterValue && 
                                   (strcasecmp(paramValues[valIndex]->parameterValue, "true") == 0 || 
                                    strcmp(paramValues[valIndex]->parameterValue, "1") == 0)) {
                                    b = true;
                                }
                                if(cJSON_AddBoolToObject(valItem, paramValues[valIndex]->parameterName, b) == NULL){
                                    T2Error("cJSON_AddBoolToObject failed\n");
                                    cJSON_Delete(arrayItem);
                                    cJSON_Delete(valItem);
                                    return T2ERROR_FAILURE;
			        }
                                break;
                            }
                            case TR181_TYPE_INT:
                            case TR181_TYPE_LONG: {
                                long long ll = paramValues[valIndex]->parameterValue ? strtoll(paramValues[valIndex]->parameterValue, NULL, 10) : 0;
                                if(cJSON_AddNumberToObject(valItem, paramValues[valIndex]->parameterName, (double) ll) == NULL){
                                    T2Error("cJSON_AddNumberToObject failed\n");
                                    cJSON_Delete(arrayItem);
                                    cJSON_Delete(valItem);
                                    return T2ERROR_FAILURE;
                                }
                                break;
                            }
                            case TR181_TYPE_UNSIGNED:
                            case TR181_TYPE_UNSIGNED_LONG: {
                                unsigned long long ull = paramValues[valIndex]->parameterValue ? strtoull(paramValues[valIndex]->parameterValue, NULL, 10) : 0;
                                if(cJSON_AddNumberToObject(valItem, paramValues[valIndex]->parameterName, (double) ull) == NULL){
                                    T2Error("cJSON_AddNumberToObject failed\n");
                                    cJSON_Delete(arrayItem);
                                    cJSON_Delete(valItem);
                                    return T2ERROR_FAILURE;
                                }
                                break;
                            }
                            case TR181_TYPE_FLOAT:
                            case TR181_TYPE_DOUBLE: {
                                double d = paramValues[valIndex]->parameterValue ? strtod(paramValues[valIndex]->parameterValue, NULL) : 0.0;
                                if(cJSON_AddNumberToObject(valItem, paramValues[valIndex]->parameterName, d) == NULL){
                                    T2Error("cJSON_AddNumberToObject failed\n");
                                    cJSON_Delete(arrayItem);
                                    cJSON_Delete(valItem);
                                    return T2ERROR_FAILURE;
                                }
                                break;
                            }
                            case TR181_TYPE_STRING:
                            case TR181_TYPE_DATETIME:
                            case TR181_TYPE_BASE64:
                            default: {
                                if(cJSON_AddStringToObject(valItem, paramValues[valIndex]->parameterName, paramValues[valIndex]->parameterValue) == NULL){
                                    T2Error("cJSON_AddStringToObject failed\n");
                                    cJSON_Delete(arrayItem);
                                    cJSON_Delete(valItem);
                                    return T2ERROR_FAILURE;
                                }
                                break;
                            }
                        } 
                        // end switch
                        cJSON_AddItemToArray(valList, valItem);
                        isTableEmpty = false ;
                    }
                }
            }

            if(!isTableEmpty)
            {
                cJSON_AddItemToArray(valArray, arrayItem);
            }
            else
            {
                cJSON_free(arrayItem);
            }
        }
    }
    T2Debug("%s --Out \n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

T2ERROR encodeStaticParamsInJSON(cJSON *valArray, Vector *staticParamList)
{
    T2Debug("%s ++in \n", __FUNCTION__);
    if(valArray == NULL || staticParamList == NULL)
    {
        T2Error("Invalid or NULL Arguments\n");
        return T2ERROR_INVALID_ARGS;
    }

    size_t index = 0;
    cJSON *arrayItem = NULL;
    for(; index < Vector_Size(staticParamList); index++)
    {
        StaticParam *sparam = (StaticParam *)Vector_At(staticParamList, index);
        if(sparam)
        {
            if(sparam->name == NULL || sparam->value == NULL )
            {
                continue ;
            }
            arrayItem = cJSON_CreateObject();
            if(arrayItem == NULL)
            {
                T2Error("cJSON_CreateObject failed.. arrayItem is NULL \n");
                return T2ERROR_FAILURE;
            }
            if(cJSON_AddStringToObject(arrayItem, sparam->name, sparam->value) == NULL)
            {
                T2Error("cJSON_AddStringToObject failed.\n");
                cJSON_Delete(arrayItem);
                return T2ERROR_FAILURE;
            }
            cJSON_AddItemToArray(valArray, arrayItem);
        }
    }

    T2Debug("%s --Out \n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

T2ERROR encodeGrepResultInJSON(cJSON *valArray, Vector *grepResult)
{
    T2Debug("%s ++in \n", __FUNCTION__);
    if(valArray == NULL || grepResult == NULL)
    {
        T2Error("Invalid or NULL Arguments\n");
        return T2ERROR_INVALID_ARGS;
    }
    size_t index = 0;
    cJSON *arrayItem = NULL;
    for(; index < Vector_Size(grepResult); index++)
    {
        GrepResult* grep = (GrepResult *)Vector_At(grepResult, index);
        if(grep)
        {
            if(grep->markerName == NULL || grep->markerValue == NULL ) // Ignore null values
            {
                continue ;
            }
            arrayItem = cJSON_CreateObject();
            if(arrayItem == NULL)
            {
                T2Error("cJSON_CreateObject failed..arrayItem is NULL \n");
                return T2ERROR_FAILURE;
            }
            if(grep->trimParameter)
            {
                trimLeadingAndTrailingws((char*)grep->markerValue);
            }
            if(grep->regexParameter != NULL)
            {
                regex_t regpattern;
                int rc = 0;
                size_t nmatch = 1;
                regmatch_t pmatch[2];
                char string[256] = {'\0'};
                rc = regcomp(&regpattern, grep->regexParameter, REG_EXTENDED);
                if(rc != 0)
                {
                    T2Warning("regcomp() failed, returning nonzero (%d)\n", rc);
                }
                else
                {
                    T2Debug("regcomp() successful, returning value (%d)\n", rc);
                    rc = regexec(&regpattern, grep->markerValue, nmatch, pmatch, 0);
                    if(rc != 0)
                    {
                        T2Warning("regexec() failed, Failed to match '%s' with '%s',returning %d.\n", grep->markerValue, grep->regexParameter, rc);
                        free((char*)grep->markerValue);
                        grep->markerValue = strdup("");
                    }
                    else
                    {
                        T2Debug("regexec successful, Match is found %.*s\n", pmatch[0].rm_eo - pmatch[0].rm_so, &grep->markerValue[pmatch[0].rm_so]);
                        sprintf(string, "%.*s", pmatch[0].rm_eo - pmatch[0].rm_so, &grep->markerValue[pmatch[0].rm_so]);
                        free((char*)grep->markerValue);
                        grep->markerValue = strdup(string);
                    }
                    regfree(&regpattern);
                }
            }
            if(cJSON_AddStringToObject(arrayItem, grep->markerName, grep->markerValue)  == NULL)
            {
                T2Error("cJSON_AddStringToObject failed.\n");
                cJSON_Delete(arrayItem);
                return T2ERROR_FAILURE;
            }
            cJSON_AddItemToArray(valArray, arrayItem);
        }
    }
    T2Debug("%s --Out \n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

T2ERROR encodeEventMarkersInJSON(cJSON *valArray, Vector *eventMarkerList)
{
    T2Debug("%s ++in \n", __FUNCTION__);
    if(valArray == NULL || eventMarkerList == NULL)
    {
        T2Error("Invalid or NULL Arguments\n");
        return T2ERROR_INVALID_ARGS;
    }
    size_t index = 0;
    cJSON *arrayItem = NULL;
    for(; index < Vector_Size(eventMarkerList); index++)
    {
        EventMarker* eventMarker = (EventMarker *)Vector_At(eventMarkerList, index);
        switch(eventMarker->mType)
        {
        case MTYPE_COUNTER:
            if(eventMarker->u.count > 0)
            {
                char stringValue[10] = {'\0'};
                sprintf(stringValue, "%d", eventMarker->u.count);
                arrayItem = cJSON_CreateObject();
                if(arrayItem == NULL)
                {
                    T2Error("cJSON_CreateObject failed .. arrayItem is NULL\n");
                    return T2ERROR_FAILURE;
                }
                if(eventMarker->trimParam)
                {
                    trimLeadingAndTrailingws(stringValue);
                }
                if(eventMarker->regexParam != NULL)
                {
                    regex_t regpattern;
                    int rc = 0;
                    size_t nmatch = 1;
                    regmatch_t pmatch[2];
                    char string[10] = {'\0'};
                    rc = regcomp(&regpattern, eventMarker->regexParam, REG_EXTENDED);
                    if(rc != 0)
                    {
                        T2Warning("regcomp() failed, returning nonzero (%d)\n", rc);
                    }
                    else
                    {
                        T2Debug("regcomp() successful, returning value (%d)\n", rc);
                        rc = regexec(&regpattern, stringValue, nmatch, pmatch, 0);
                        if(rc != 0)
                        {
                            T2Warning("regexec() failed, Failed to match '%s' with '%s',returning %d.\n", stringValue, eventMarker->regexParam, rc);
                            strncpy(stringValue, "", 1);
                        }
                        else
                        {
                            T2Debug("regexec successful, Match is found %.*s\n", pmatch[0].rm_eo - pmatch[0].rm_so, &stringValue[pmatch[0].rm_so]);
                            sprintf(string, "%.*s", pmatch[0].rm_eo - pmatch[0].rm_so, &stringValue[pmatch[0].rm_so]);
                            strncpy(stringValue, string, strlen(string) + 1);
                        }
                        regfree(&regpattern);
                    }
                }

                if (eventMarker->alias)
                {
                    if(cJSON_AddStringToObject(arrayItem, eventMarker->alias, stringValue) == NULL)
                    {
                        T2Error("cJSON_AddStringToObject failed\n");
                        cJSON_Delete(arrayItem);
                        return T2ERROR_FAILURE;
                    }

                }
                else
                {
                    if(cJSON_AddStringToObject(arrayItem, eventMarker->markerName, stringValue) == NULL)
                    {
                        T2Error("cJSON_AddStringToObject failed\n");
                        cJSON_Delete(arrayItem);
                        return T2ERROR_FAILURE;
                    }
                }
                if(eventMarker->reportTimestampParam == REPORTTIMESTAMP_UNIXEPOCH)
                {
                    if(cJSON_AddStringToObject(arrayItem, eventMarker->markerName_CT, eventMarker->timestamp) == NULL)
                    {
                        T2Error("cJSON_AddStringToObject failed\n");
                        cJSON_Delete(arrayItem);
                        return T2ERROR_FAILURE;
                    }
                }
                cJSON_AddItemToArray(valArray, arrayItem);
                T2Debug("Marker value for : %s is %d\n", eventMarker->markerName, eventMarker->u.count);
                eventMarker->u.count = 0;
            }
            break;

        case MTYPE_ACCUMULATE:
            if( eventMarker->u.accumulatedValues != NULL && Vector_Size(eventMarker->u.accumulatedValues))
            {
                arrayItem = cJSON_CreateObject();
                if(arrayItem == NULL)
                {
                    T2Error("cJSON_CreateObject failed .. arrayItem is NULL\n");
                    return T2ERROR_FAILURE;
                }
                cJSON *vectorToarray = cJSON_CreateArray();
                if(vectorToarray == NULL)
                {
                    T2Error("cJSON_CreateArray failed .. vectorToarray is NULL\n");
                    cJSON_Delete(arrayItem);
                    return T2ERROR_FAILURE;
                }
                if(eventMarker->trimParam)
                {
                    size_t i;
                    for(i = 0; i < Vector_Size(eventMarker->u.accumulatedValues); i++)
                    {
                        char* stringValue = (char*)Vector_At(eventMarker->u.accumulatedValues, i);
                        trimLeadingAndTrailingws(stringValue);
                    }
                }
                Vector* regaccumulateValues = NULL;
                if(eventMarker->regexParam != NULL)
                {
                    regex_t regpattern;
                    int rc = 0;
                    size_t nmatch = 1;
                    regmatch_t pmatch[2];
                    char string[21][256];
                    memset(string, '\0', sizeof(char) * 21 * 256);
                    rc = regcomp(&regpattern, eventMarker->regexParam, REG_EXTENDED);
                    if(rc != 0)
                    {
                        T2Warning("regcomp() failed, returning nonzero (%d)\n", rc);
                    }
                    else
                    {
                        T2Debug("regcomp() successful, returning value (%d)\n", rc);
                        Vector_Create(&regaccumulateValues);
                        for(size_t i = 0; i < Vector_Size(eventMarker->u.accumulatedValues); i++)
                        {
                            char* stringValue = (char*)Vector_At(eventMarker->u.accumulatedValues, i);
                            rc = regexec(&regpattern, stringValue, nmatch, pmatch, 0);
                            if(strcmp(stringValue, "maximum accumulation reached") == 0)
                            {
                                sprintf(string[i], "%s", stringValue);
                            }
                            else if(rc != 0)
                            {
                                T2Warning("regexec() failed, Failed to match '%s' with '%s',returning %d.\n", stringValue, eventMarker->regexParam, rc);
                                sprintf(string[i], "%s", "");
                            }
                            else
                            {
                                T2Debug("regexec successful, Match is found %.*s\n", pmatch[0].rm_eo - pmatch[0].rm_so, &stringValue[pmatch[0].rm_so]);
                                sprintf(string[i], "%.*s", pmatch[0].rm_eo - pmatch[0].rm_so, &stringValue[pmatch[0].rm_so]);
                            }
                            Vector_PushBack(regaccumulateValues, string[i]);
                        }
                        regfree(&regpattern);
                    }
                }

                if(regaccumulateValues != NULL && Vector_Size(regaccumulateValues) > 0)
                {
                    convertVectorToJson(vectorToarray, regaccumulateValues);
                }
                else
                {
                    convertVectorToJson(vectorToarray, eventMarker->u.accumulatedValues);
                }
                Vector_Clear(eventMarker->u.accumulatedValues, freeAccumulatedParam);
                T2Debug("eventMarker->reportTimestampParam type is %d \n", eventMarker->reportTimestampParam);

                if(eventMarker->alias)
                {
                    cJSON_AddItemToObject(arrayItem, eventMarker->alias, vectorToarray);
                }
                else
                {
                    cJSON_AddItemToObject(arrayItem, eventMarker->markerName, vectorToarray);
                }
                if(eventMarker->reportTimestampParam == REPORTTIMESTAMP_UNIXEPOCH)
                {
                    cJSON *TimevectorToarray = cJSON_CreateArray();
                    if(TimevectorToarray == NULL)
                    {
                        T2Error("cJSON_CreateArray failed .. TimevectorToarray is NULL\n");
                        cJSON_Delete(arrayItem);
                        return T2ERROR_FAILURE;
                    }
                    convertVectorToJson(TimevectorToarray, eventMarker->accumulatedTimestamp);
                    T2Debug("convertVectorToJson is successful\n");
                    Vector_Clear(eventMarker->accumulatedTimestamp, freeAccumulatedParam);
                    cJSON_AddItemToObject(arrayItem, eventMarker->markerName_CT, TimevectorToarray);
                }
                cJSON_AddItemToArray(valArray, arrayItem);
                char *temp =  cJSON_Print(vectorToarray);
                if(temp)
                {
                    T2Debug("Marker value Array for : %s is %s\n", eventMarker->markerName, temp);
                    free(temp);
                }
            }
            break;

        case MTYPE_ABSOLUTE:
        default:
            if(eventMarker->u.markerValue != NULL)
            {
                arrayItem = cJSON_CreateObject();
                if(arrayItem == NULL)
                {
                    T2Error("cJSON_CreateObject failed.. arrayItem is NULL\n");
                    return T2ERROR_FAILURE;
                }
                if(eventMarker->trimParam)
                {
                    trimLeadingAndTrailingws(eventMarker->u.markerValue);
                }
                if(eventMarker->regexParam != NULL)
                {
                    regex_t regpattern;
                    int rc = 0;
                    size_t nmatch = 1;
                    regmatch_t pmatch[2];
                    char string[256] = {'\0'};
                    rc = regcomp(&regpattern, eventMarker->regexParam, REG_EXTENDED);
                    if(rc != 0)
                    {
                        T2Warning("regcomp() failed, returning nonzero (%d)\n", rc);
                    }
                    else
                    {
                        T2Debug("regcomp() successful, returning value (%d)\n", rc);
                        rc = regexec(&regpattern, eventMarker->u.markerValue, nmatch, pmatch, 0);
                        if(rc != 0)
                        {
                            T2Warning("regexec() failed, Failed to match '%s' with '%s',returning %d.\n", eventMarker->u.markerValue, eventMarker->regexParam, rc);
                            free(eventMarker->u.markerValue);
                            eventMarker->u.markerValue = strdup("");
                        }
                        else
                        {
                            T2Debug("regexec successful, Match is found %.*s\n", pmatch[0].rm_eo - pmatch[0].rm_so, &eventMarker->u.markerValue[pmatch[0].rm_so]);
                            sprintf(string, "%.*s", pmatch[0].rm_eo - pmatch[0].rm_so, &eventMarker->u.markerValue[pmatch[0].rm_so]);
                            free(eventMarker->u.markerValue);
                            eventMarker->u.markerValue = strdup(string);
                        }
                        regfree(&regpattern);
                    }
                }
                if (eventMarker->alias)
                {
                    if(cJSON_AddStringToObject(arrayItem, eventMarker->alias, eventMarker->u.markerValue) == NULL)
                    {
                        T2Error("cJSON_AddStringToObject failed\n");
                        cJSON_Delete(arrayItem);
                        return T2ERROR_FAILURE;
                    }
                }
                else
                {
                    if(cJSON_AddStringToObject(arrayItem, eventMarker->markerName, eventMarker->u.markerValue) == NULL)
                    {
                        T2Error("cJSON_AddStringToObject failed\n");
                        cJSON_Delete(arrayItem);
                        return T2ERROR_FAILURE;
                    }
                }
                if(eventMarker->reportTimestampParam == REPORTTIMESTAMP_UNIXEPOCH)
                {
                    if(cJSON_AddStringToObject(arrayItem, eventMarker->markerName_CT, eventMarker->timestamp) == NULL)
                    {
                        T2Error("cJSON_AddStringToObject failed\n");
                        cJSON_Delete(arrayItem);
                        return T2ERROR_FAILURE;
                    }
                }
                cJSON_AddItemToArray(valArray, arrayItem);
                T2Debug("Marker value for : %s is %s\n", eventMarker->markerName, eventMarker->u.markerValue);
                free(eventMarker->u.markerValue);
                eventMarker->u.markerValue = NULL;
            }
        }
    }
    T2Debug("%s --Out \n", __FUNCTION__);
    return T2ERROR_SUCCESS;

}

T2ERROR prepareJSONReport(cJSON* jsonObj, char** reportBuff)
{
    if(jsonObj == NULL)
    {
        T2Error("jsonObj is NULL\n");
        return T2ERROR_INVALID_ARGS;
    }
    T2Debug("%s ++in\n", __FUNCTION__);
    *reportBuff = cJSON_PrintUnformatted(jsonObj);
    if(*reportBuff == NULL)
    {
        T2Error("Failed to get unformatted json\n");
        return T2ERROR_FAILURE;
    }
    T2Debug("%s --Out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

char *prepareHttpUrl(T2HTTP *http)
{
    if(http == NULL)
    {
        T2Error("Http parameters are NULL\n");
        return NULL;
    }
    CURL *curl = curl_easy_init();
    if(curl == NULL)
    {
        T2Error("Curl init failed Response is NULL\n");
        return NULL;
    }
    char *httpUrl = strdup(http->URL);
    T2Debug("%s: Default URL: %s \n", __FUNCTION__, httpUrl);

    if (http->RequestURIparamList && http->RequestURIparamList->count)
    {
        unsigned int index = 0;
        int params_len = 0;
        char *url_params = NULL;

        for(; index < http->RequestURIparamList->count; index++)
        {
            HTTPReqParam * httpParam = (HTTPReqParam *)(Vector_At)(http->RequestURIparamList, index);
            char *httpParamVal = NULL;
            char *paramValue = NULL;

            if (httpParam->HttpValue)		//Static parameter
            {
                httpParamVal = curl_easy_escape(curl, httpParam->HttpValue, 0);
            }
            else				//Dynamic parameter
            {
                if(T2ERROR_SUCCESS != getParameterValue(httpParam->HttpRef, &paramValue))
                {
                    T2Error("Failed to retrieve param : %s\n", httpParam->HttpRef);
                    continue;
                }
                else
                {
                    if (paramValue[0] == '\0')
                    {
                        free(paramValue);
                        T2Error("Param value is empty for : %s\n", httpParam->HttpRef);
                        continue;
                    }

                    httpParamVal = curl_easy_escape(curl, paramValue, 0);
                    free(paramValue);
                    paramValue = NULL;
                }
            }

            int new_params_len = params_len /* current params length */
                                 + strlen(httpParam->HttpName) /* Length of parameter 'Name' */ + strlen("=")
                                 + strlen("&") /* Add '&' for next paramter */ + 1;
            if(httpParamVal)
            {
                new_params_len += strlen(httpParamVal);
            }
            url_params = realloc(url_params, new_params_len);
            if(url_params == NULL)
            {
                T2Error("Unable to allocate %d bytes of memory at Line %d on %s \n", new_params_len, __LINE__, __FILE__);
                curl_free(httpParamVal);
                continue;
            }
            params_len += snprintf(url_params + params_len, new_params_len - params_len, "%s=%s&", httpParam->HttpName, httpParamVal);

            curl_free(httpParamVal);
        }

        if (params_len > 0 && url_params != NULL)
        {
            url_params[params_len - 1] = '\0';

            int url_len = strlen(httpUrl);
            int modified_url_len = url_len + strlen("?") + strlen(url_params) + 1;

            httpUrl = realloc(httpUrl, modified_url_len);
            snprintf(httpUrl + url_len, modified_url_len - url_len, "?%s", url_params);

        }
        if(url_params)
        {
            free(url_params);
        }
    }

    curl_easy_cleanup(curl);
    T2Debug("%s: Modified URL: %s \n", __FUNCTION__, httpUrl);

    return httpUrl;
}
