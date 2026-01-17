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
#include <string.h>
#include "t2common.h"
#include "t2log_wrapper.h"
#include "dcautil.h"

bool whoami_support = false;

void freeParam(void *data)
{
    if(data != NULL)
    {
        Param *param = (Param *)data;
        if(param->name)
        {
            free(param->name);
        }
        if(param->alias)
        {
            free((char*)param->alias);
        }
        if(param->paramType)
        {
            free(param->paramType);
        }
        if(param->regexParam)
        {
            free(param->regexParam);
        }
        free(param);
    }
}

void freeDataModelParam(void *data)
{
    if (data)
    {
        DataModelParam *param = (DataModelParam *)data;
        if (param->reference)
        {
            free(param->reference);
        }
        if (param->name)
        {
            free(param->name);
        }
        free(param);
    }
}

void freeDataModelTable(void *data)
{
    if (data)
    {
        DataModelTable *table = (DataModelTable *)data;
        if (table->reference)
        {
            free(table->reference);
        }
        if (table->index)
        {
            free(table->index);
        }
        if (table->paramList)
        {
            Vector_Destroy(table->paramList, freeDataModelParam);
        }
        free(table);
    }
}

void freeStaticParam(void *data)
{
    if(data != NULL)
    {
        StaticParam *sparam = (StaticParam *)data;
        if(sparam->name)
        {
            free(sparam->name);
        }
        if (sparam->paramType)
        {
            free(sparam->paramType);
        }
        if (sparam->value)
        {
            free(sparam->value);
        }

        free(sparam);
    }
}

void freeEMarker(void *data)
{
    if(data != NULL)
    {
        EventMarker *eMarker = (EventMarker *)data;
        if(eMarker->alias)
        {
            free(eMarker->alias);
        }
        if(eMarker->compName)
        {
            free(eMarker->compName);
        }
        if(eMarker->markerName)
        {
            free(eMarker->markerName);
        }
        if(eMarker->paramType)
        {
            free(eMarker->paramType);
        }
        if(eMarker->reportTimestampParam == REPORTTIMESTAMP_UNIXEPOCH)
        {
            if(eMarker->markerName_CT != NULL)
            {
                free(eMarker->markerName_CT);
            }
            if(eMarker->timestamp != NULL)
            {
                free(eMarker->timestamp);
            }
        }
        if(eMarker->regexParam != NULL)
        {
            free(eMarker->regexParam);
        }
        if(eMarker->mType == MTYPE_ABSOLUTE && eMarker->u.markerValue)
        {
            free(eMarker->u.markerValue);
        }
        if(eMarker->mType == MTYPE_ACCUMULATE && eMarker->u.accumulatedValues)
        {
            Vector_Destroy(eMarker->u.accumulatedValues, freeAccumulatedParam);
            if(eMarker->accumulatedTimestamp != NULL && eMarker->reportTimestampParam == REPORTTIMESTAMP_UNIXEPOCH)
            {
                Vector_Destroy(eMarker->accumulatedTimestamp, freeAccumulatedParam);
            }
        }
        free(eMarker);
    }
}

void freeAccumulatedParam(void* data)
{
    if(data != NULL)
    {
        char* dataStr = data;
        free(dataStr);
    }
}

void freeGMarker(void *data)
{
    if(data != NULL)
    {
        GrepMarker *gMarker = (GrepMarker *)data;
        if(gMarker->logFile)
        {
            free(gMarker->logFile);
        }
        if(gMarker->markerName)
        {
            free(gMarker->markerName);
        }
        if(gMarker->searchString)
        {
            free(gMarker->searchString);
        }
        if(gMarker->paramType)
        {
            free(gMarker->paramType);
        }
        if(gMarker->mType == MTYPE_ABSOLUTE && gMarker->u.markerValue)
        {
            free(gMarker->u.markerValue);
        }
        if(gMarker->regexParam != NULL)
        {
            free(gMarker->regexParam);
        }
        free(gMarker);
    }
}

/**
 * Comparator eventually called by qsort
 */
int compareLogFileNames(const void *g1, const void *g2)
{
    GrepMarker** p1 = (GrepMarker**) g1 ;
    GrepMarker** p2 = (GrepMarker**) g2 ;

    if ( NULL != p1 && NULL != p2)
    {
        return strcmp((*p1)->logFile, (*p2)->logFile);
    }
    else
    {
        T2Error("compareLogFileNames : either p1 or p2 is NULL \n");
        return -1 ;
    }
}

void freeTriggerCondition(void *data)
{
    if(data != NULL)
    {
        TriggerCondition *tCondition = (TriggerCondition *)data;
        if(tCondition->type)
        {
            free(tCondition->type);
        }
        if(tCondition->oprator)
        {
            free(tCondition->oprator);
        }
        if(tCondition->reference)
        {
            free(tCondition->reference);
        }
        free(tCondition);
    }
}

void freeGResult(void *data)
{
    if(data != NULL)
    {
        GrepResult *grepResult = (GrepResult *) data;
        if(grepResult->markerName)
        {
            free((char*)grepResult->markerName);
        }

        if(grepResult->markerValue)
        {
            free((char*)grepResult->markerValue);
        }

        free(grepResult);
        grepResult = NULL;
    }
}
/* Description: Getting device property by reading device.property file
 * @param: dev_prop_name : Device property name to get from file
 * @param: out_data : pointer to hold the device property get from file
 * @param: buff_size : Buffer size of the out_data.
 * @return bool: Success: true  and Fail : false
 * */
bool getDevicePropertyData(const char *dev_prop_name, char *out_data, unsigned int buff_size)
{
    bool ret = false;
    FILE *fp;
    char tbuff[MAX_DEVICE_PROP_BUFF_SIZE];
    char *tmp;
    int index;
    if (out_data == NULL || dev_prop_name == NULL)
    {
        T2Error("%s : parameter is NULL\n", __FUNCTION__);
        return ret;
    }
    if (buff_size == 0 || buff_size > MAX_DEVICE_PROP_BUFF_SIZE)
    {
        T2Error("%s : buff size not in the range. size should be < %d\n", __FUNCTION__, MAX_DEVICE_PROP_BUFF_SIZE);
        return ret;
    }
    T2Debug("%s : Trying device property data for %s and buf size=%u\n", __FUNCTION__, dev_prop_name, buff_size);
    fp = fopen(DEVICE_PROPERTIES_FILE, "r");
    if(fp == NULL)
    {
        T2Error("%s : device.property File not found\n", __FUNCTION__);
        return ret;
    }
    while((fgets(tbuff, sizeof(tbuff), fp) != NULL))
    {
        if(strstr(tbuff, dev_prop_name))
        {
            index = strcspn(tbuff, "\n");
            if (index > 0)
            {
                tbuff[index] = '\0';
            }
            tmp = strchr(tbuff, '=');
            if(tmp != NULL)
            {
                snprintf(out_data, buff_size, "%s", tmp + 1);
                T2Debug("%s : %s=%s\n", __FUNCTION__, dev_prop_name, out_data);
                ret = true;
                break;
            }
            else
            {
                T2Error("%s : strchr failed. '=' not found. str=%s\n", __FUNCTION__, tbuff);
            }
        }
    }
    fclose(fp);
    return ret;
}

void initWhoamiSupport(void)
{
    char buf[8] = {0};
    if (getDevicePropertyData("WHOAMI_SUPPORT", buf, sizeof(buf)) && strcmp(buf, "true") == 0)
    {
        whoami_support = true;
        T2Info("WhoAmI feature is enabled\n");
    }
    else
    {
        whoami_support = false;
        T2Info("WhoAmI feature is disabled\n");
    }
}

bool isWhoAmiEnabled(void)
{
    return whoami_support;
}

// Function to check if configured parameter matches actual RBUS parameter
bool matchesParameter(const char* pattern, const char* paramName)
{
    if (!pattern || !paramName)
    {
        return false;
    }

    const char* lastPatternSegment = strrchr(pattern, '.');
    const char* lastParamSegment = strrchr(paramName, '.');

    if (lastPatternSegment && lastParamSegment)
    {
        if (strncmp(lastPatternSegment + 1, lastParamSegment + 1, strlen(lastPatternSegment + 1)) != 0)
        {
            return false;
        }
    }

    while (*pattern && *paramName)
    {
        if (*pattern == '*')
        {
            pattern++;
            if (*pattern == '\0')
            {
                return true;
            }
            while (*paramName && *paramName != *pattern)
            {
                paramName++;
            }
        }
        else
        {
            if (*pattern != *paramName)
            {
                return false;
            }
            pattern++;
            paramName++;
        }
    }

    return (*pattern == '\0' && *paramName == '\0');
}
