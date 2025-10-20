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
    //printf("%ld %ld\n", Vector_Size(paramNameList), Vector_Size(paramValueList));

    for(; index < Vector_Size(paramNameList); index++)
    {
        Param* param = (Param *)Vector_At(paramNameList, index);
        //printf("Parameter Name : %s\n", param->name);
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

                    if(cJSON_AddStringToObject(arrayItem, param->name, paramValues[0]->parameterValue)  == NULL)
                    {
                        T2Error("cJSON_AddStringToObject failed.\n");
                        cJSON_Delete(arrayItem);
                        return T2ERROR_FAILURE;
                    }
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

                        if(cJSON_AddStringToObject(valItem, paramValues[valIndex]->parameterName, paramValues[valIndex]->parameterValue) == NULL)
                        {
                            T2Error("cJSON_AddStringToObject failed\n");
                            cJSON_Delete(arrayItem);
                            cJSON_Delete(valItem);
                            return T2ERROR_FAILURE;
                        }
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

T2ERROR encodeGrepResultInJSON(cJSON *valArray, Vector *grepMarkerList)
{
    T2Info("%s ++in \n", __FUNCTION__);
    if(valArray == NULL || grepMarkerList == NULL)
    {
        T2Error("Invalid or NULL Arguments\n");
        return T2ERROR_INVALID_ARGS;
    }

    size_t index = 0;
    cJSON *arrayItem = NULL;
    for(; index < Vector_Size(grepMarkerList); index++)
    {
        GrepMarker* grepMarker = (GrepMarker *)Vector_At(grepMarkerList, index);
        if (!grepMarker)
        {
            continue;
        }

        switch(grepMarker->mType)
        {
        case MTYPE_COUNTER:
            T2Info("%d ++in \n", __LINE__);
            if(grepMarker->u.count > 0)
            {
                char stringValue[10] = {'\0'};
                sprintf(stringValue, "%d", grepMarker->u.count);
                arrayItem = cJSON_CreateObject();
                if(arrayItem == NULL)
                {
                    T2Error("cJSON_CreateObject failed .. arrayItem is NULL\n");
                    return T2ERROR_FAILURE;
                }
                if(grepMarker->trimParam)
                {
                    trimLeadingAndTrailingws(stringValue);
                }
                if(grepMarker->regexParam != NULL)
                {
                    regex_t regpattern;
                    int rc = 0;
                    size_t nmatch = 1;
                    regmatch_t pmatch[2];
                    char string[10] = {'\0'};
                    rc = regcomp(&regpattern, grepMarker->regexParam, REG_EXTENDED);
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
                            T2Warning("regexec() failed, Failed to match '%s' with '%s',returning %d.\n", stringValue, grepMarker->regexParam, rc);
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

                if(cJSON_AddStringToObject(arrayItem, grepMarker->markerName, stringValue) == NULL)
                {
                    T2Error("cJSON_AddStringToObject failed\n");
                    cJSON_Delete(arrayItem);
                    return T2ERROR_FAILURE;
                }
                cJSON_AddItemToArray(valArray, arrayItem);
                T2Debug("Marker value for : %s is %d\n", grepMarker->markerName, grepMarker->u.count);
                grepMarker->u.count = 0;
            }
            break;

        case MTYPE_ACCUMULATE:
            T2Info("%d ++in \n", __LINE__);
            if(grepMarker->u.accumulatedValues != NULL && Vector_Size(grepMarker->u.accumulatedValues))
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
                if(grepMarker->trimParam)
                {
                    size_t i;
                    for(i = 0; i < Vector_Size(grepMarker->u.accumulatedValues); i++)
                    {
                        char* stringValue = (char*)Vector_At(grepMarker->u.accumulatedValues, i);
                        if (stringValue)
                        {
                            trimLeadingAndTrailingws(stringValue);
                        }
                    }
                }
                Vector* regaccumulateValues = NULL;
                if(grepMarker->regexParam != NULL)
                {
                    regex_t regpattern;
                    int rc = 0;
                    size_t nmatch = 1;
                    regmatch_t pmatch[2];
                    char string[21][256];
                    memset(string, '\0', sizeof(char) * 21 * 256);
                    rc = regcomp(&regpattern, grepMarker->regexParam, REG_EXTENDED);
                    if(rc != 0)
                    {
                        T2Warning("regcomp() failed, returning nonzero (%d)\n", rc);
                    }
                    else
                    {
                        T2Debug("regcomp() successful, returning value (%d)\n", rc);
                        Vector_Create(&regaccumulateValues);
                        for(size_t i = 0; i < Vector_Size(grepMarker->u.accumulatedValues); i++)
                        {
                            char* stringValue = (char*)Vector_At(grepMarker->u.accumulatedValues, i);
                            if (stringValue)
                            {
                                rc = regexec(&regpattern, stringValue, nmatch, pmatch, 0);
                                if(strcmp(stringValue, "maximum accumulation reached") == 0)
                                {
                                    sprintf(string[i], "%s", stringValue);
                                }
                                else if(rc != 0)
                                {
                                    T2Warning("regexec() failed, Failed to match '%s' with '%s',returning %d.\n", stringValue, grepMarker->regexParam, rc);
                                    sprintf(string[i], "%s", "");
                                }
                                else
                                {
                                    T2Debug("regexec successful, Match is found %.*s\n", pmatch[0].rm_eo - pmatch[0].rm_so, &stringValue[pmatch[0].rm_so]);
                                    sprintf(string[i], "%.*s", pmatch[0].rm_eo - pmatch[0].rm_so, &stringValue[pmatch[0].rm_so]);
                                }
                                Vector_PushBack(regaccumulateValues, string[i]);
                            }
                        }
                        regfree(&regpattern);
                    }
                }
                T2Info("%d ++in \n", __LINE__);
                if(regaccumulateValues != NULL && Vector_Size(regaccumulateValues) > 0)
                {
                    convertVectorToJson(vectorToarray, regaccumulateValues);
                }
                else
                {
                    T2Info("%d ++in \n", __LINE__);
                    convertVectorToJson(vectorToarray, grepMarker->u.accumulatedValues);
                }
                Vector_Clear(grepMarker->u.accumulatedValues, freeAccumulatedParam);
                T2Info("%d ++in \n", __LINE__);
                cJSON_AddItemToObject(arrayItem, grepMarker->markerName, vectorToarray);
                T2Info("%d ++in \n", __LINE__);
                // Add timestamp support for MTYPE_ACCUMULATE
                if((grepMarker->reportTimestampParam == REPORTTIMESTAMP_UNIXEPOCH) && (grepMarker->accumulatedTimestamp != NULL) && (Vector_Size(grepMarker->accumulatedTimestamp) > 0))
                {
                    T2Info("%d ++in \n", __LINE__);

                    // Populate markerName_CT if not already set
                    if(grepMarker->markerName_CT == NULL)
                    {
                        T2Info("%d ++in \n", __LINE__);
                        char buf[512];
                        snprintf(buf, sizeof(buf), "%s_CT", grepMarker->markerName);
                        grepMarker->markerName_CT = strdup(buf);
                    }
                    //T2Info("Timestamp is taken for %s with size %ld\n", grepMarker->markerName_CT, Vector_Size(grepMarker->accumulatedTimestamp));

                    cJSON *TimevectorToarray = cJSON_CreateArray();
                    if(TimevectorToarray == NULL)
                    {
                        T2Error("cJSON_CreateArray failed .. TimevectorToarray is NULL\n");
                        cJSON_Delete(arrayItem);
                        if(regaccumulateValues != NULL)
                        {
                            Vector_Clear(regaccumulateValues, NULL);
                            free(regaccumulateValues);
                        }
                        return T2ERROR_FAILURE;
                    }
                    convertVectorToJson(TimevectorToarray, grepMarker->accumulatedTimestamp);
                    T2Info("convertVectorToJson is successful for timestamps\n");
                    Vector_Clear(grepMarker->accumulatedTimestamp, freeAccumulatedParam);
                    cJSON_AddItemToObject(arrayItem, grepMarker->markerName_CT, TimevectorToarray);
                }

                cJSON_AddItemToArray(valArray, arrayItem);
                char *temp = cJSON_Print(vectorToarray);
                if(temp)
                {
                    T2Debug("Marker value Array for : %s is %s\n", grepMarker->markerName, temp);
                    free(temp);
                }
                if(regaccumulateValues != NULL)
                {
                    Vector_Clear(regaccumulateValues, NULL);
                    free(regaccumulateValues);
                }
            }
            break;

        case MTYPE_ABSOLUTE:
        default:
            if(grepMarker->u.markerValue != NULL)
            {
                arrayItem = cJSON_CreateObject();
                if(arrayItem == NULL)
                {
                    T2Error("cJSON_CreateObject failed.. arrayItem is NULL\n");
                    return T2ERROR_FAILURE;
                }
                if(grepMarker->trimParam)
                {
                    trimLeadingAndTrailingws(grepMarker->u.markerValue);
                }
                if(grepMarker->regexParam != NULL)
                {
                    regex_t regpattern;
                    int rc = 0;
                    size_t nmatch = 1;
                    regmatch_t pmatch[2];
                    char string[256] = {'\0'};
                    rc = regcomp(&regpattern, grepMarker->regexParam, REG_EXTENDED);
                    if(rc != 0)
                    {
                        T2Warning("regcomp() failed, returning nonzero (%d)\n", rc);
                    }
                    else
                    {
                        T2Debug("regcomp() successful, returning value (%d)\n", rc);
                        rc = regexec(&regpattern, grepMarker->u.markerValue, nmatch, pmatch, 0);
                        if(rc != 0)
                        {
                            T2Warning("regexec() failed, Failed to match '%s' with '%s',returning %d.\n", grepMarker->u.markerValue, grepMarker->regexParam, rc);
                            free(grepMarker->u.markerValue);
                            grepMarker->u.markerValue = strdup("");
                        }
                        else
                        {
                            T2Debug("regexec successful, Match is found %.*s\n", pmatch[0].rm_eo - pmatch[0].rm_so, &grepMarker->u.markerValue[pmatch[0].rm_so]);
                            sprintf(string, "%.*s", pmatch[0].rm_eo - pmatch[0].rm_so, &grepMarker->u.markerValue[pmatch[0].rm_so]);
                            free(grepMarker->u.markerValue);
                            grepMarker->u.markerValue = strdup(string);
                        }
                        regfree(&regpattern);
                    }
                }
                if(cJSON_AddStringToObject(arrayItem, grepMarker->markerName, grepMarker->u.markerValue) == NULL)
                {
                    T2Error("cJSON_AddStringToObject failed\n");
                    cJSON_Delete(arrayItem);
                    return T2ERROR_FAILURE;
                }
                cJSON_AddItemToArray(valArray, arrayItem);
                T2Debug("Marker value for : %s is %s\n", grepMarker->markerName, grepMarker->u.markerValue);
                free(grepMarker->u.markerValue);
                grepMarker->u.markerValue = NULL;
            }
        }
    }
    T2Debug("%s --Out \n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

T2ERROR encodeTopResultInJSON(cJSON *valArray, Vector *topMarkerList)
{
    T2Info("%s ++in \n", __FUNCTION__);
    if(valArray == NULL || topMarkerList == NULL)
    {
        T2Error("Invalid or NULL Arguments\n");
        return T2ERROR_INVALID_ARGS;
    }

    size_t index = 0;
    cJSON *arrayItem = NULL;
    for(; index < Vector_Size(topMarkerList); index++)
    {
        TopMarker* topMarker = (TopMarker *)Vector_At(topMarkerList, index);
        if (!topMarker)
        {
            continue;
        }

        // Handle Load Average case (single value)
        if(topMarker->loadAverage != NULL)
        {
            arrayItem = cJSON_CreateObject();
            if(arrayItem == NULL)
            {
                T2Error("cJSON_CreateObject failed.. arrayItem is NULL\n");
                return T2ERROR_FAILURE;
            }

            // Create a working copy for processing
            char* workingValue = strdup(topMarker->loadAverage);
            if(workingValue == NULL)
            {
                T2Error("Failed to duplicate load average value\n");
                cJSON_Delete(arrayItem);
                return T2ERROR_FAILURE;
            }

            // Apply trimming if specified
            if(topMarker->trimParam)
            {
                trimLeadingAndTrailingws(workingValue);
            }

            // Apply regex processing if specified
            if(topMarker->regexParam != NULL)
            {
                regex_t regpattern;
                int rc = 0;
                size_t nmatch = 1;
                regmatch_t pmatch[2];
                char string[256] = {'\0'};
                rc = regcomp(&regpattern, topMarker->regexParam, REG_EXTENDED);
                if(rc != 0)
                {
                    T2Warning("regcomp() failed for load average marker, returning nonzero (%d)\n", rc);
                }
                else
                {
                    T2Debug("regcomp() successful for load average marker, returning value (%d)\n", rc);
                    rc = regexec(&regpattern, workingValue, nmatch, pmatch, 0);
                    if(rc != 0)
                    {
                        T2Warning("regexec() failed for load average marker, Failed to match '%s' with '%s',returning %d.\n",
                                  workingValue, topMarker->regexParam, rc);
                        free(workingValue);
                        workingValue = strdup("");
                    }
                    else
                    {
                        T2Debug("regexec successful for load average marker, Match is found %.*s\n",
                                pmatch[0].rm_eo - pmatch[0].rm_so, &workingValue[pmatch[0].rm_so]);
                        sprintf(string, "%.*s", pmatch[0].rm_eo - pmatch[0].rm_so, &workingValue[pmatch[0].rm_so]);
                        free(workingValue);
                        workingValue = strdup(string);
                    }
                    regfree(&regpattern);
                }
            }

            // Add the load average marker value to JSON with searchString
            if(cJSON_AddStringToObject(arrayItem, topMarker->searchString, workingValue) == NULL)
            {
                T2Error("cJSON_AddStringToObject failed for load average marker\n");
                cJSON_Delete(arrayItem);
                free(workingValue);
                return T2ERROR_FAILURE;
            }

            cJSON_AddItemToArray(valArray, arrayItem);
            T2Debug("Load average marker value for : %s is %s\n", topMarker->searchString, workingValue);

            // Clean up the working copy
            free(workingValue);
        }
        // Handle CPU & Memory package case (both values present)
        else if(topMarker->cpuValue != NULL && topMarker->memValue != NULL)
        {
            // Process CPU Value
            arrayItem = cJSON_CreateObject();
            if(arrayItem == NULL)
            {
                T2Error("cJSON_CreateObject failed for CPU marker.. arrayItem is NULL\n");
                return T2ERROR_FAILURE;
            }

            // Create CPU marker name with prefix
            size_t cpuNameLen = strlen("cpu_") + strlen(topMarker->searchString) + 1;
            char* cpuMarkerName = malloc(cpuNameLen);
            if(cpuMarkerName == NULL)
            {
                T2Error("Failed to allocate memory for CPU marker name\n");
                cJSON_Delete(arrayItem);
                return T2ERROR_FAILURE;
            }
            snprintf(cpuMarkerName, cpuNameLen, "cpu_%s", topMarker->searchString);

            // Create a working copy for CPU processing
            char* cpuWorkingValue = strdup(topMarker->cpuValue);
            if(cpuWorkingValue == NULL)
            {
                T2Error("Failed to duplicate CPU value\n");
                cJSON_Delete(arrayItem);
                free(cpuMarkerName);
                return T2ERROR_FAILURE;
            }

            // Apply trimming if specified
            if(topMarker->trimParam)
            {
                trimLeadingAndTrailingws(cpuWorkingValue);
            }

            // Apply regex processing if specified
            if(topMarker->regexParam != NULL)
            {
                regex_t regpattern;
                int rc = 0;
                size_t nmatch = 1;
                regmatch_t pmatch[2];
                char string[256] = {'\0'};
                rc = regcomp(&regpattern, topMarker->regexParam, REG_EXTENDED);
                if(rc != 0)
                {
                    T2Warning("regcomp() failed for CPU marker, returning nonzero (%d)\n", rc);
                }
                else
                {
                    T2Debug("regcomp() successful for CPU marker, returning value (%d)\n", rc);
                    rc = regexec(&regpattern, cpuWorkingValue, nmatch, pmatch, 0);
                    if(rc != 0)
                    {
                        T2Warning("regexec() failed for CPU marker, Failed to match '%s' with '%s',returning %d.\n",
                                  cpuWorkingValue, topMarker->regexParam, rc);
                        free(cpuWorkingValue);
                        cpuWorkingValue = strdup("");
                    }
                    else
                    {
                        T2Debug("regexec successful for CPU marker, Match is found %.*s\n",
                                pmatch[0].rm_eo - pmatch[0].rm_so, &cpuWorkingValue[pmatch[0].rm_so]);
                        sprintf(string, "%.*s", pmatch[0].rm_eo - pmatch[0].rm_so, &cpuWorkingValue[pmatch[0].rm_so]);
                        free(cpuWorkingValue);
                        cpuWorkingValue = strdup(string);
                    }
                    regfree(&regpattern);
                }
            }

            // Add the CPU marker value to JSON
            if(cJSON_AddStringToObject(arrayItem, cpuMarkerName, cpuWorkingValue) == NULL)
            {
                T2Error("cJSON_AddStringToObject failed for CPU marker\n");
                cJSON_Delete(arrayItem);
                free(cpuMarkerName);
                free(cpuWorkingValue);
                return T2ERROR_FAILURE;
            }

            cJSON_AddItemToArray(valArray, arrayItem);
            T2Debug("CPU marker value for : %s is %s\n", cpuMarkerName, cpuWorkingValue);

            // Clean up CPU processing resources
            free(cpuMarkerName);
            free(cpuWorkingValue);

            // Process Memory Value
            arrayItem = cJSON_CreateObject();
            if(arrayItem == NULL)
            {
                T2Error("cJSON_CreateObject failed for Memory marker.. arrayItem is NULL\n");
                return T2ERROR_FAILURE;
            }

            // Create Memory marker name with prefix
            size_t memNameLen = strlen("mem_") + strlen(topMarker->searchString) + 1;
            char* memMarkerName = malloc(memNameLen);
            if(memMarkerName == NULL)
            {
                T2Error("Failed to allocate memory for Memory marker name\n");
                cJSON_Delete(arrayItem);
                return T2ERROR_FAILURE;
            }
            snprintf(memMarkerName, memNameLen, "mem_%s", topMarker->searchString);

            // Create a working copy for Memory processing
            char* memWorkingValue = strdup(topMarker->memValue);
            if(memWorkingValue == NULL)
            {
                T2Error("Failed to duplicate Memory value\n");
                cJSON_Delete(arrayItem);
                free(memMarkerName);
                return T2ERROR_FAILURE;
            }

            // Apply trimming if specified
            if(topMarker->trimParam)
            {
                trimLeadingAndTrailingws(memWorkingValue);
            }

            // Apply regex processing if specified
            if(topMarker->regexParam != NULL)
            {
                regex_t regpattern;
                int rc = 0;
                size_t nmatch = 1;
                regmatch_t pmatch[2];
                char string[256] = {'\0'};
                rc = regcomp(&regpattern, topMarker->regexParam, REG_EXTENDED);
                if(rc != 0)
                {
                    T2Warning("regcomp() failed for Memory marker, returning nonzero (%d)\n", rc);
                }
                else
                {
                    T2Debug("regcomp() successful for Memory marker, returning value (%d)\n", rc);
                    rc = regexec(&regpattern, memWorkingValue, nmatch, pmatch, 0);
                    if(rc != 0)
                    {
                        T2Warning("regexec() failed for Memory marker, Failed to match '%s' with '%s',returning %d.\n",
                                  memWorkingValue, topMarker->regexParam, rc);
                        free(memWorkingValue);
                        memWorkingValue = strdup("");
                    }
                    else
                    {
                        T2Debug("regexec successful for Memory marker, Match is found %.*s\n",
                                pmatch[0].rm_eo - pmatch[0].rm_so, &memWorkingValue[pmatch[0].rm_so]);
                        sprintf(string, "%.*s", pmatch[0].rm_eo - pmatch[0].rm_so, &memWorkingValue[pmatch[0].rm_so]);
                        free(memWorkingValue);
                        memWorkingValue = strdup(string);
                    }
                    regfree(&regpattern);
                }
            }

            // Add the Memory marker value to JSON
            if(cJSON_AddStringToObject(arrayItem, memMarkerName, memWorkingValue) == NULL)
            {
                T2Error("cJSON_AddStringToObject failed for Memory marker\n");
                cJSON_Delete(arrayItem);
                free(memMarkerName);
                free(memWorkingValue);
                return T2ERROR_FAILURE;
            }

            cJSON_AddItemToArray(valArray, arrayItem);
            T2Debug("Memory marker value for : %s is %s\n", memMarkerName, memWorkingValue);

            // Clean up Memory processing resources
            free(memMarkerName);
            free(memWorkingValue);
        }
        else
        {
            T2Debug("Top marker %s has no valid values to report (loadAverage=%p, cpuValue=%p, memValue=%p)\n", 
                   topMarker->searchString ? topMarker->searchString : "unknown",
                   topMarker->loadAverage, topMarker->cpuValue, topMarker->memValue);
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
                        if(regaccumulateValues != NULL)
                        {
                            Vector_Clear(regaccumulateValues, NULL);
                            free(regaccumulateValues);
                        }
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
                if(regaccumulateValues != NULL)
                {
                    Vector_Clear(regaccumulateValues, NULL);
                    free(regaccumulateValues);
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
