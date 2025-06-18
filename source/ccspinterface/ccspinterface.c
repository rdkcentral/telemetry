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


#include <stdbool.h>
#include <syslog.h>
#include <ccsp/ansc_platform.h>

#include "busInterface.h"
#include "ccspinterface.h"
#include "t2log_wrapper.h"
#include "vector.h"
#include "t2common.h"
#include "ssp_global.h"
#include "ccsp_memory.h"
#include "ccsp_base_api.h"

static void *bus_handle = NULL;

static T2ERROR ccspGetParameterValues(const char **paramNames, const int paramNamesCount, parameterValStruct_t ***valStructs, int *valSize);

#if 0
static T2ERROR getParameterNames(const char *objName, parameterInfoStruct_t ***paramNamesSt, int *paramNamesLength);
#endif

void freeParamInfoSt(parameterInfoStruct_t **paramNamesSt, int paramNamesLength);

static T2ERROR CCSPInterface_Init()
{
    T2Debug("%s ++in\n", __FUNCTION__);
    char *pCfg = CCSP_MSG_BUS_CFG;
    char *componentId = NULL;


    if (componentId == NULL)
    {
        componentId = strdup(CCSP_COMPONENT_ID);
    }

    int ret = CCSP_Message_Bus_Init(componentId, pCfg, &bus_handle, (CCSP_MESSAGE_BUS_MALLOC)Ansc_AllocateMemory_Callback, Ansc_FreeMemory_Callback);
    free(componentId);

    if (ret == -1)
    {
        T2Error("%s:%d, init failed\n", __func__, __LINE__);
        return T2ERROR_FAILURE;
    }
    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

static int findDestComponent(char *paramName, char **destCompName, char **destPath)
{
    int ret, size = 0;
    char dst_pathname_cr[256] = {0};
    componentStruct_t **ppComponents = NULL;
    T2Debug("%s ++in for paramName : %s\n", __FUNCTION__, paramName);
    snprintf(dst_pathname_cr, sizeof(dst_pathname_cr), "eRT.%s", CCSP_DBUS_INTERFACE_CR);
    ret = CcspBaseIf_discComponentSupportingNamespace(bus_handle, dst_pathname_cr, paramName, "", &ppComponents, &size);
    if ( ret == CCSP_SUCCESS && size >= 1)
    {
        *destCompName = strdup(ppComponents[0]->componentName);
        *destPath = strdup(ppComponents[0]->dbusPath);
        T2Debug("destCompName = %s destPath = %s \n", *destCompName, *destPath);
    }
    else
    {
        T2Error("Failed to get component for %s ret: %d\n", paramName, ret);
        return ret;
    }
    free_componentStruct_t(bus_handle, size, ppComponents);

    T2Debug("%s --out\n", __FUNCTION__);
    return ret;
}

bool isCCSPInitialized()
{

    return bus_handle != NULL ? true : false ;
}

T2ERROR ccspGetParameterValues(const char **paramNames, const int paramNamesCount, parameterValStruct_t ***valStructs, int *valSize)
{
    char *destCompName = NULL, *destCompPath = NULL;
    T2ERROR retErrCode = T2ERROR_FAILURE;
    T2Debug("%s ++in\n", __FUNCTION__);
    *valSize = 0 ;
    if(!bus_handle && T2ERROR_SUCCESS != CCSPInterface_Init())
    {
        return T2ERROR_FAILURE;
    }
    else if(paramNames == NULL || paramNamesCount <= 0 )
    {
        T2Error("paramNames is NULL or paramNamesCount <= 0 - returning\n");
        return T2ERROR_INVALID_ARGS;
    }
    if(CCSP_SUCCESS == findDestComponent((char*)paramNames[0], &destCompName, &destCompPath))
    {
        T2Debug("Calling CcspBaseIf_getParameterValues for : %s, paramCount : %d Destination name : %s and path %s\n", paramNames[0], paramNamesCount, destCompName, destCompPath);
        int ret = CcspBaseIf_getParameterValues(bus_handle, destCompName, destCompPath, (char**)paramNames, paramNamesCount, valSize, valStructs);
        if (ret != CCSP_SUCCESS)
        {
            T2Error("CcspBaseIf_getParameterValues failed for : %s with ret = %d\n", paramNames[0], ret);
        }
        else
        {
            retErrCode = T2ERROR_SUCCESS;
        }
    }
    else
    {
        T2Error("Unable to find supporting component for parameter : %s\n", paramNames[0]);
        retErrCode = T2ERROR_FAILURE;
    }
    if(destCompName)
    {
        free(destCompName);
        destCompName = NULL;
    }
    if(destCompPath)
    {
        free(destCompPath);
        destCompPath = NULL;
    }

    T2Debug("%s --out \n", __FUNCTION__);
    return retErrCode;
}

#if 0
T2ERROR getParameterNames(const char *objName, parameterInfoStruct_t ***paramNamesSt, int *paramNamesLength)
{
    T2ERROR ret = T2ERROR_FAILURE;
    char *destCompName = NULL, *destCompPath = NULL;
    T2Debug("%s ++in\n", __FUNCTION__);

    if(!bus_handle && T2ERROR_SUCCESS != CCSPInterface_Init())
    {
        return T2ERROR_FAILURE;
    }
    else if(objName == NULL || objName[strlen(objName) - 1] != '.')
    {
        T2Error("Invalid objectName, doesn't end with a wildcard '.'\n");
        return T2ERROR_INVALID_ARGS;
    }
    if(CCSP_SUCCESS == findDestComponent((char*)objName, &destCompName, &destCompPath))
    {
        if ( CCSP_SUCCESS != CcspBaseIf_getParameterNames(bus_handle, destCompName, destCompPath, (char*)objName, 1, paramNamesLength, paramNamesSt))
        {
            T2Error("CcspBaseIf_getParameterValues failed for : %s\n", objName);
        }
        else
        {
            ret = T2ERROR_SUCCESS;
        }
    }
    else
    {
        T2Error("Unable to find supporting component for parameter : %s\n", objName);
    }
    if(destCompName)
    {
        free(destCompName);
        destCompName = NULL;
    }
    if(destCompPath)
    {
        free(destCompPath);
        destCompPath = NULL;
    }
    T2Debug("%s --out \n", __FUNCTION__);
    return ret;
}
#endif

void freeParamInfoSt(parameterInfoStruct_t **paramInfoSt, int paramNamesLength)
{
    free_parameterInfoStruct_t(bus_handle, paramNamesLength, paramInfoSt);
}

static void freeCCSPParamValueSt(parameterValStruct_t **valStructs, int valSize)
{
    free_parameterValStruct_t(bus_handle, valSize, valStructs);
}

T2ERROR getCCSPParamVal(const char* paramName, char **paramValue)
{
    T2Debug("%s ++in \n", __FUNCTION__);
    parameterValStruct_t **valStructs = NULL;
    int valSize = 0;
    char *paramNames[1] = {NULL};
    if(!bus_handle && T2ERROR_SUCCESS != CCSPInterface_Init())
    {
        return T2ERROR_FAILURE;
    }

    paramNames[0] = strdup(paramName);
    if(T2ERROR_SUCCESS != ccspGetParameterValues((const char**)paramNames, 1, &valStructs, &valSize))
    {
        T2Error("Unable to get %s\n", paramName);
        free(paramNames[0]);
        return T2ERROR_FAILURE;
    }
    T2Debug("%s = %s\n", paramName, valStructs[0]->parameterValue);
    *paramValue = strdup(valStructs[0]->parameterValue);
    free(paramNames[0]);
    freeCCSPParamValueSt(valStructs, valSize);
    T2Debug("%s --out \n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

Vector* getCCSPProfileParamValues(Vector *paramList)
{
    Vector *profileValueList = NULL;
    Vector_Create(&profileValueList);

    T2Debug("%s ++in\n", __FUNCTION__);
    if(!bus_handle && T2ERROR_SUCCESS != CCSPInterface_Init())
    {
        return profileValueList;
    }


    for(int i = 0; i < Vector_Size(paramList); i++) {
        /* Preparing the storage for the result */
        tr181ValStruct_t **paramValues = NULL;
        parameterValStruct_t **ccspParamValues = NULL;
        int paramValCount  = 0;
        profileValues *profVals = calloc(1, sizeof(profileValues));
        if(profVals == NULL)
        {
            T2Error("Unable allocate memory for profVals\n");
            continue;
        }

        /* Retrieving the TR-181 alias and duplicating it */
        const char *alias     = ((Param *)Vector_At(paramList, i))->alias;
        char       *paramName = alias ? strdup(alias) : NULL;
        /* paramNames passed to the CCSP API (must be char*[], not const) */
        char *paramNames[1]   = { paramName ? strdup(paramName) : NULL };

        /* --- CCSP component discovery --- */
        char *destCompName = NULL;
        char *destCompPath = NULL;
        if(paramName)
        {
            int discRet = findDestComponent(paramName, &destCompName, &destCompPath);
            if(discRet != CCSP_SUCCESS) {
                T2Error("Component discovery failed for %s (ret=%d)\n", paramName, discRet);
                paramValCount = 0;
            } 
            else
            {
                T2Debug("Discovered comp %s at %s\n", destCompName, destCompPath);
            }
        }

        /* --- Call to the CCSP API to retrieve the values --- */
        if(paramNames[0]) 
        {
            T2Debug("CcspBaseIf_getParameterValues for: %s\n", paramNames[0]);
            int ret = CcspBaseIf_getParameterValues(
                          bus_handle,
                          destCompName,
                          destCompPath,
                          (char**)paramNames,
                          1,
                          &paramValCount,
                          &ccspParamValues);
            if(ret != CCSP_SUCCESS)
            {
                T2Error("CcspBaseIf_getParameterValues failed for %s (ret=%d)\n", paramNames[0], ret);
                paramValCount = 0;
            } 
            else
            {
                T2Info("ParameterName : %s Retrieved value count : %d\n", paramNames[0], paramValCount);
            }
        }

        profVals->paramValueCount = paramValCount;
        T2Debug("Received %d parameters for %s (CCSP)\n",
                 paramValCount,
                 paramNames[0] ? paramNames[0] : "(null)");

        /* --- Constructing the TR-181 values array --- */
        if(paramValCount == 0)
        {
            /* Parameters found → copy each one */
            paramValues = calloc(1, sizeof(tr181ValStruct_t*));
            paramValues[0] = calloc(1, sizeof(tr181ValStruct_t));
            paramValues[0]->parameterName  = strdup(paramName ? paramName : "");
            paramValues[0]->parameterValue = strdup("NULL");
            paramValues[0]->type           = TR181_TYPE_STRING;
            profVals->paramValueCount      = 1;
        }
        else
        {
            /* Parameters found → copy each one */
            paramValues = calloc(paramValCount, sizeof(tr181ValStruct_t*));
            for(int j = 0; j < paramValCount; j++)
            {
                parameterValStruct_t *cc = ccspParamValues[j];
                paramValues[j] = calloc(1, sizeof(tr181ValStruct_t));
                /* Copy name and value */
                paramValues[j]->parameterName  = strdup(cc->parameterName);
                paramValues[j]->parameterValue = strdup(cc->parameterValue);
                /* Assign the native TR-181 type */
                switch(ccspParamValues[j]->type) {
                    case ccsp_boolean:
                        paramValues[j]->type = TR181_TYPE_BOOLEAN;
                        break;
                    case ccsp_int:
                        paramValues[j]->type = TR181_TYPE_INT;
                        break;
                    case ccsp_unsignedInt:
                        paramValues[j]->type = TR181_TYPE_UNSIGNED;
                        break;
                    case ccsp_long:
                        paramValues[j]->type = TR181_TYPE_LONG;
                        break;
                    case ccsp_unsignedLong:
                        paramValues[j]->type = TR181_TYPE_UNSIGNED_LONG;
                        break;
                    case ccsp_float:
                        paramValues[j]->type = TR181_TYPE_FLOAT;
                        break;
                    case ccsp_double:
                        paramValues[j]->type = TR181_TYPE_DOUBLE;
                        break;
                    case ccsp_dateTime:
                        paramValues[j]->type = TR181_TYPE_DATETIME;
                        break;
                    case ccsp_base64:
                        paramValues[j]->type = TR181_TYPE_BASE64;
                        break;
                    case ccsp_string:
                    default:
                        paramValues[j]->type = TR181_TYPE_STRING;
                        break;
                }
            }
        }

        /* --- CCSP cleanup and profile finalization --- */
        if(ccspParamValues)
        {
            free_parameterValStruct_t(bus_handle, paramValCount, ccspParamValues);
        }
        profVals->paramValues = paramValues;
        // End of populating bus independent parameter value array
        Vector_PushBack(profileValueList, profVals);

        /* Free the temporary strings */
        free(paramName);
        free(paramNames[0]);
        free(destCompName);
        free(destCompPath);
    }

    T2Debug("%s --Out\n", __FUNCTION__);
    return profileValueList;
}

T2ERROR registerCcspT2EventListener(TelemetryEventCallback eventCB)
{
    int ret;
    T2Debug("%s ++in\n", __FUNCTION__);
    if(!bus_handle && T2ERROR_SUCCESS != CCSPInterface_Init())
    {
        return T2ERROR_FAILURE;
    }
    CcspBaseIf_SetCallback2(bus_handle, "telemetryDataSignal",
                            eventCB, NULL);

    ret = CcspBaseIf_Register_Event(bus_handle, NULL, "telemetryDataSignal");
    if (ret != CCSP_Message_Bus_OK)
    {
        T2Error("CcspBaseIf_Register_Event failed\n");
        return T2ERROR_FAILURE;
    }
    else
    {
        T2Info("Registration with CCSP Bus successful, waiting for Telemetry Events from components...\n");
    }
    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

T2ERROR unregisterCcspT2EventListener()
{
    return T2ERROR_SUCCESS;
}

