/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2015 RDK Management
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

/**********************************************************************
 Copyright [2014] [Cisco Systems, Inc.]
 
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 
 http://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 **********************************************************************/

/*********************************************************************** 

 module: plugin_main.c

 Implement COSA Data Model Library Init and Unload apis.
 
 ---------------------------------------------------------------

 author:

 ---------------------------------------------------------------

 revision:

 11/18/2019    initial revision.

 **********************************************************************/

#include "ansc_platform.h"
#include "ansc_load_library.h"
#include "cosa_plugin_api.h"
#include "plugin_main.h"
#include "plugin_main_apis.h"


PCOSA_BACKEND_MANAGER_OBJECT                        g_pCosaBEManager;
void *                                              g_pDslhDmlAgent;
extern ANSC_HANDLE                                  g_MessageBusHandle_Irep;
extern char                                         g_SubSysPrefix_Irep[32];

#define THIS_PLUGIN_VERSION                         1

int ANSC_EXPORT_API
COSA_Init(ULONG uMaxVersionSupported, void* hCosaPlugInfo /* PCOSA_PLUGIN_INFO passed in by the caller */
) {
    PCOSA_PLUGIN_INFO pPlugInfo = (PCOSA_PLUGIN_INFO) hCosaPlugInfo;
    COSAGetInterfaceByNameProc pGetInterfaceByNameProc = (COSAGetInterfaceByNameProc) NULL;

    if(uMaxVersionSupported < THIS_PLUGIN_VERSION) {
        /* this version is not supported */
        return -1;
    }   
    
    pPlugInfo->uPluginVersion       = THIS_PLUGIN_VERSION;
    g_pDslhDmlAgent                 = pPlugInfo->hDmlAgent;

    pGetInterfaceByNameProc = (COSAGetInterfaceByNameProc) pPlugInfo->AcquireFunction("COSAGetInterfaceByName");

    if(pGetInterfaceByNameProc != NULL) {
        g_GetInterfaceByName = pGetInterfaceByNameProc;
    }else {
        goto EXIT;
    }

    g_pT2CcdIf = g_GetInterfaceByName(g_pDslhDmlAgent, CCSP_CCD_INTERFACE_NAME);

    if(!g_pT2CcdIf) {
        CcspTraceError(("g_pT2CcdIf is NULL !\n"));

        goto EXIT;
    }

    /* Get Message Bus Handle */
    g_GetMessageBusHandle = (COSAGetHandleProc) pPlugInfo->AcquireFunction("COSAGetMessageBusHandle");
    if(g_GetMessageBusHandle == NULL) {
        goto EXIT;
    }

    g_MessageBusHandle = (ANSC_HANDLE) g_GetMessageBusHandle(g_pDslhDmlAgent);
    if(g_MessageBusHandle == NULL) {
        goto EXIT;
    }
    g_MessageBusHandle_Irep = g_MessageBusHandle;

    /* Get Subsystem prefix */
    g_GetSubsystemPrefix = (COSAGetSubsystemPrefixProc) pPlugInfo->AcquireFunction("COSAGetSubsystemPrefix");
    if(g_GetSubsystemPrefix != NULL) {
        char* tmpSubsystemPrefix;

        if((tmpSubsystemPrefix = g_GetSubsystemPrefix(g_pDslhDmlAgent))) {
            strncpy(g_SubSysPrefix_Irep, tmpSubsystemPrefix, sizeof(g_SubSysPrefix_Irep) - 1);
	    g_SubSysPrefix_Irep[sizeof(g_SubSysPrefix_Irep) - 1] = '\0';
        }

        /* retrieve the subsystem prefix */
        g_SubsystemPrefix = g_GetSubsystemPrefix(g_pDslhDmlAgent);
    }

    /* Create backend framework */
    g_pCosaBEManager = (PCOSA_BACKEND_MANAGER_OBJECT) CosaBackEndManagerCreate();

    if(g_pCosaBEManager && g_pCosaBEManager->Initialize) {
        g_pCosaBEManager->hCosaPluginInfo = pPlugInfo;

        g_pCosaBEManager->Initialize   ((ANSC_HANDLE)g_pCosaBEManager);
    }
    
    return  0;

EXIT:

    return -1;
    
}

void ANSC_EXPORT_API
COSA_Unload(void) {
    ANSC_STATUS returnStatus = ANSC_STATUS_SUCCESS;

    /* unload the memory here */

    returnStatus = CosaBackEndManagerRemove(g_pCosaBEManager);

    if(returnStatus == ANSC_STATUS_SUCCESS) {
        g_pCosaBEManager = NULL;
    }else {
        /* print error trace*/
        g_pCosaBEManager = NULL;
    }
}
