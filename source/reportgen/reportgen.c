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

// Helper function to find or create array item by index
cJSON* findOrCreateArrayItem(cJSON *array, int targetIndex)
{
    int currentSize = cJSON_GetArraySize(array);
    // Check if item already exists at any position
    for (int i = 0; i < currentSize; i++)
    {
        cJSON *item = cJSON_GetArrayItem(array, i);
        cJSON *indexObj = cJSON_GetObjectItem(item, "index");
        if (indexObj && atoi(indexObj->valuestring) == targetIndex)
        {
            return item;
        }
    }

    // If not found, create new item and add to array
    cJSON *newItem = cJSON_CreateObject();
    if(newItem == NULL)
    {
        T2Error("cJSON_CreateObject failed.. arrayItem is NULL \n");
        return NULL;
    }
    char indexStr[10];
    snprintf(indexStr, sizeof(indexStr), "%d", targetIndex);
    if(cJSON_AddStringToObject(newItem, "index", indexStr)  == NULL)
    {
        T2Error("cJSON_AddStringToObject failed.\n");
        cJSON_Delete(newItem);
        return NULL;
    }
    cJSON_AddItemToArray(array, newItem);
    return newItem;
}

//Function to get the basePath like Device.WiFi.AccessPoint.
int getBasePath(const char *input, char *basePath, size_t maxLength)
{
    const char *p = input;

    // Find the first digit in the input string
    while (*p != '\0')
    {
        if (isdigit(*p))
        {
            break;
        }
        p++;
    }

    // Length of the base path (everything before the first digit)
    size_t length = p - input;

    // Check if the base path fits within the buffer
    if (length < maxLength)
    {
        strncpy(basePath, input, length);
        basePath[length] = '\0'; // Null-terminate the string
        return 0; // Success
    }
    else
    {
        // If truncation would occur, return failure
        return -1; // Failure
    }
}

/**
 * @brief Finds the best-matching DataModelTable entry based on a full parameter path.
 *
 * This function searches the given list of DataModelTables and returns the most specific
 * table whose reference string is a prefix of the provided full parameter string.
 * It ensures that the prefix match is on a valid boundary by checking the next character
 * (must be either end-of-string, '.', or a digit indicating table index).
 *
 * @param dataModelTableList Pointer to a vector containing DataModelTable entries.
 * @param fullParam          Full TR-181 style parameter path to search for.
 *
 * @return Pointer to the best-matching DataModelTable, or NULL if no match is found.
 *
 * Example:
 *   fullParam = "Device.WiFi.AccessPoint.1.AssociatedDevice.2.MACAddress"
 *   matching table->reference = "Device.WiFi.AccessPoint."
 */
DataModelTable *findTableByReference(Vector *dataModelTableList, const char *fullParam)
{
    DataModelTable *table = NULL;
    if (dataModelTableList != NULL && Vector_Size(dataModelTableList) > 0)
    {
        size_t bestMatchLength = 0;
        for (int tableCount = 0; tableCount < Vector_Size(dataModelTableList); tableCount++)
        {
            DataModelTable *tbl = (DataModelTable *)Vector_At(dataModelTableList, tableCount);
            if (!tbl || !tbl->reference)
            {
                continue;
            }
            if (strncmp(fullParam, tbl->reference, strlen(tbl->reference)) == 0)
            {
                char nextChar = fullParam[strlen(tbl->reference)];
                if (nextChar == '\0' || nextChar == '.' || isdigit(nextChar))
                {
                    if (strlen(tbl->reference) > bestMatchLength)
                    {
                        table = tbl;
                        bestMatchLength = strlen(tbl->reference);
                        T2Debug("Match found! table->reference = %s, fullParam = %s\n", table->reference, fullParam);
                    }
                }
            }
        }
    }
    return table;
}

bool isDataModelTable(const char *paramName)
{
    // Checking if the parameter name ends with a dot '.'
    size_t paramNameLength = strlen(paramName);
    return (paramNameLength > 0 && paramName[paramNameLength - 1] == '.');
}

T2ERROR encodeParamResultInJSON(cJSON *valArray, Vector *paramNameList, Vector *paramValueList, Vector *dataModelTableList)
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
        else if(paramValCount == 1 && !isDataModelTable(param->name)) // Single value
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
            if (((dataModelTableList != NULL) && (Vector_Size(dataModelTableList) > 0)))
            {
                int valIndex = 0;
                bool isTableEmpty = true;
                cJSON *arrayItem = NULL; // Initialize to NULL
                bool isNewPattern = true;
                cJSON *existingItem = NULL;

                char basePattern[512];
                int result = getBasePath(param->name, basePattern, sizeof(basePattern));
                if (result != 0)
                {
                    T2Error("basePattern is either truncated or invalid.");
                    return T2ERROR_FAILURE;
                }

                if (valArray == NULL || cJSON_GetArraySize(valArray) == 0)
                {
                    T2Error("ValArray is NULL\n");
                }
                else
                {
                    int arraySize = cJSON_GetArraySize(valArray);
                    for (int i = 0; i < arraySize; i++)
                    {
                        cJSON *existingObj = cJSON_GetArrayItem(valArray, i);
                        if (existingObj && cJSON_HasObjectItem(existingObj, basePattern))
                        {
                            existingItem = existingObj;
                            T2Debug("Found existing object for pattern: %s\n", basePattern);
                            isNewPattern = false;
                            break;
                        }
                    }
                }

                if (isNewPattern)
                {
                    T2Debug("Creating new object for pattern: %s\n", basePattern);
                    arrayItem = cJSON_CreateObject();
                    if (arrayItem == NULL)
                    {
                        T2Error("cJSON_CreateObject failed.. arrayItem is NULL \n");
                        return T2ERROR_FAILURE;
                    }
                }
                else
                {
                    arrayItem = existingItem;
                }

                DataModelTable *table = findTableByReference(dataModelTableList, param->name);
                if (table != NULL)
                {

                    for (; valIndex < paramValCount; valIndex++)
                    {
                        if (!checkForEmptyString(paramValues[valIndex]->parameterValue))
                        {
                            char *parameterName = strdup(paramValues[valIndex]->parameterName);
                            char *parameterWild = strdup(paramValues[valIndex]->parameterName);
                            size_t basePathLength = strlen(basePattern);
                            cJSON *currentObject = arrayItem;
                            char concatenatedKey[256] = "";
                            bool firstIndexHandled = false;
                            bool parameterConfigured = false;
                            char *token = strtok(parameterName + basePathLength, ".");


                            //Filtering the list based on DataModelParam List
                            for (int listCount = 0; listCount < Vector_Size(table->paramList); listCount++)
                            {
                                DataModelParam *list = (DataModelParam *)Vector_At(table->paramList, listCount);
                                if (parameterWild && matchesParameter(list->name, parameterWild))
                                {
                                    T2Debug("Match found!: list->name = %s, parameterWild = %s\n", list->name, parameterWild);
                                    parameterConfigured = true;
                                    break; // Parameter found, proceed with processing
                                }
                            }

                            // If parameter is not configured, skip processing this parameter
                            if (!parameterConfigured)
                            {
                                if (parameterName)
                                {
                                    free(parameterName);    // Free if allocated
                                }
                                if (parameterWild)
                                {
                                    free(parameterWild);    // Free if allocated
                                }
                                continue;
                            }


                            while (token != NULL)
                            {
                                char *nextToken = strtok(NULL, ".");
                                if (nextToken == NULL)
                                {
                                    // Adding final parameter
                                    if (cJSON_AddStringToObject(currentObject, token, paramValues[valIndex]->parameterValue) == NULL)
                                    {
                                        T2Error("cJSON_AddStringToObject failed.\n");
                                        cJSON_Delete(currentObject);
                                        return T2ERROR_FAILURE;
                                    }
                                }
                                else if (isdigit(token[0]))
                                {
                                    // Handle array item
                                    int index = atoi(token);
                                    if (!firstIndexHandled)
                                    {
                                        strcpy(concatenatedKey, basePattern);
                                    }
                                    // Create or get array
                                    cJSON *array = cJSON_GetObjectItem(currentObject, concatenatedKey);
                                    if (array == NULL)
                                    {
                                        array = cJSON_CreateArray();
                                        cJSON_AddItemToObject(currentObject, concatenatedKey, array);
                                    }
                                    // Find or create array item at correct position
                                    currentObject = findOrCreateArrayItem(array, index);
                                    concatenatedKey[0] = '\0';
                                    firstIndexHandled = true;
                                }
                                else
                                {
                                    // Build concatenated key
                                    if (concatenatedKey[0] != '\0')
                                    {
                                        strcat(concatenatedKey, ".");
                                    }
                                    strcat(concatenatedKey, token);
                                    // Handle nested object creation only after the first index is done
                                    if (firstIndexHandled && nextToken != NULL && !isdigit(nextToken[0]))
                                    {
                                        cJSON *nestedObject = cJSON_GetObjectItem(currentObject, concatenatedKey);
                                        if (nestedObject == NULL)
                                        {
                                            nestedObject = cJSON_CreateObject();
                                            if (nestedObject == NULL)
                                            {
                                                T2Error("cJSON_CreateObject failed.. nestedObject is NULL \n");
                                                return T2ERROR_FAILURE;
                                            }
                                            cJSON_AddItemToObject(currentObject, concatenatedKey, nestedObject);
                                        }
                                        currentObject = nestedObject;
                                        concatenatedKey[0] = '\0';
                                    }
                                }
                                token = nextToken;
                            } // end of while

                            if (parameterName)
                            {
                                free(parameterName);    // Free if allocated
                            }
                            if (parameterWild)
                            {
                                free(parameterWild);    // Free if allocated
                            }
                            isTableEmpty = false;
                        }
                    } // end of for
                } // Table not NULL

                if (isNewPattern && !isTableEmpty)
                {
                    cJSON_AddItemToArray(valArray, arrayItem);
                }
                else if (isTableEmpty && isNewPattern && arrayItem)
                {
                    cJSON_Delete(arrayItem);
                    arrayItem = NULL;
                }
            }
            else
            {
                cJSON *valList = NULL;
                cJSON *valItem = NULL;
                int valIndex = 0;
                bool isTableEmpty = true;
                cJSON *arrayItem = cJSON_CreateObject();
                if (arrayItem == NULL)
                {
                    T2Error("cJSON_CreateObject failed.. arrayItem is NULL \n");
                    return T2ERROR_FAILURE;
                }
                cJSON_AddItemToObject(arrayItem, param->name, valList = cJSON_CreateArray());
                for (; valIndex < paramValCount; valIndex++)
                {
                    if (paramValues[valIndex])
                    {
                        if (param->reportEmptyParam || !checkForEmptyString(paramValues[0]->parameterValue))
                        {
                            valItem = cJSON_CreateObject();
                            if (valItem == NULL)
                            {
                                T2Error("cJSON_CreateObject failed.. valItem is NULL \n");
                                cJSON_Delete(arrayItem);
                                return T2ERROR_FAILURE;
                            }
                            if (param->trimParam)
                            {
                                trimLeadingAndTrailingws(paramValues[valIndex]->parameterValue);
                            }
                            if (param->regexParam != NULL)
                            {
                                regex_t regpattern;
                                int rc = 0;
                                size_t nmatch = 1;
                                regmatch_t pmatch[2];
                                char string[256] = {'\0'};
                                rc = regcomp(&regpattern, param->regexParam, REG_EXTENDED);
                                if (rc != 0)
                                {
                                    T2Warning("regcomp() failed, returning nonzero (%d)\n", rc);
                                }
                                else
                                {
                                    T2Debug("regcomp() successful, returning value (%d)\n", rc);
                                    rc = regexec(&regpattern, paramValues[valIndex]->parameterValue, nmatch, pmatch, 0);
                                    if (rc != 0)
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

                            if (cJSON_AddStringToObject(valItem, paramValues[valIndex]->parameterName, paramValues[valIndex]->parameterValue) == NULL)
                            {
                                T2Error("cJSON_AddStringToObject failed\n");
                                cJSON_Delete(arrayItem);
                                cJSON_Delete(valItem);
                                return T2ERROR_FAILURE;
                            }
                            cJSON_AddItemToArray(valList, valItem);
                            isTableEmpty = false;
                        }
                    }
                }

                if (!isTableEmpty)
                {
                    cJSON_AddItemToArray(valArray, arrayItem);
                }
                else
                {
                    cJSON_free(arrayItem);
                }
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

void tagReportAsCached(char **jsonReport)
{
    if (!jsonReport)
    {
        T2Error("jsonReport is NULL\n");
        return;
    }

    cJSON *jsonReportObj = cJSON_Parse(*jsonReport);
    if (!jsonReportObj)
    {
        T2Error("Failed to parse JSON report\n");
        return;
    }

    cJSON *reportTypeEntry = cJSON_CreateObject();
    if (!reportTypeEntry)
    {
        T2Error("Failed to create REPORT_TYPE object\n");
        destroyJSONReport(jsonReportObj);
        return;
    }
    cJSON_AddStringToObject(reportTypeEntry, "REPORT_TYPE", "CACHED");
    cJSON *searchResult = cJSON_GetObjectItemCaseSensitive(jsonReportObj, "searchResult");
    if (searchResult && cJSON_IsArray(searchResult))
    {
        T2Info("Inserting REPORT_TYPE: CACHED at index 1 of searchResult array\n");
        cJSON_InsertItemInArray(searchResult, 1, reportTypeEntry);
    }
    else
    {
        cJSON *reportArray = cJSON_GetObjectItemCaseSensitive(jsonReportObj, "Report");
        if (reportArray && cJSON_IsArray(reportArray))
        {
            T2Info("Inserting REPORT_TYPE: CACHED at beginning of Report array\n");
            cJSON_InsertItemInArray(reportArray, 0, reportTypeEntry);
        }
        else
        {
            T2Error("Neither 'searchResult' nor 'Report' arrays found in the JSON\n");
            destroyJSONReport(reportTypeEntry);
            destroyJSONReport(jsonReportObj);
            return;
        }
    }

    char *updatedJsonReport = cJSON_PrintUnformatted(jsonReportObj);
    if (updatedJsonReport)
    {
        free(*jsonReport);  // Freeing the old report only after the new one is generated
        *jsonReport = updatedJsonReport;
    }

    destroyJSONReport(jsonReportObj);
}


