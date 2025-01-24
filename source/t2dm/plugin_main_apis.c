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

 module: plugin_main_apis.c

 Implement COSA Data Model Library Init and Unload apis.
 This files will hold all data in it.
 
 ---------------------------------------------------------------

 description:

 This module implements the advanced state-access functions
 of the Dslh Var Record Object.

 *   CosaBackEndManagerCreate
 *   CosaBackEndManagerInitialize
 *   CosaBackEndManagerRemove
 ---------------------------------------------------------------

 author:

 ---------------------------------------------------------------

 revision:

 11/18/2019    initial revision.

 **********************************************************************/

#include "plugin_main_apis.h"
#include "cosa_telemetry_internal.h"

COSAGetInterfaceByNameProc         g_GetInterfaceByName;
COSAGetHandleProc                  g_GetMessageBusHandle;
COSAGetSubsystemPrefixProc         g_GetSubsystemPrefix;
PCCSP_CCD_INTERFACE                g_pT2CcdIf;
ANSC_HANDLE                        g_MessageBusHandle;
char*                              g_SubsystemPrefix;
COSARegisterCallBackAfterInitDmlProc  g_RegisterCallBackAfterInitDml;

/**********************************************************************

 caller:     owner of the object

 prototype:

 ANSC_HANDLE
 CosaBackEndManagerCreate
 (
 );

 description:

 This function constructs cosa datamodel object and return handle.

 argument:

 return:     newly created nat object.

 **********************************************************************/

ANSC_HANDLE CosaBackEndManagerCreate( VOID) {
    PCOSA_BACKEND_MANAGER_OBJECT pMyObject = (PCOSA_BACKEND_MANAGER_OBJECT) NULL;

    /*
     * We create object by first allocating memory for holding the variables and member functions.
     */
    pMyObject = (PCOSA_BACKEND_MANAGER_OBJECT) AnscAllocateMemory(sizeof(COSA_BACKEND_MANAGER_OBJECT));

    if(!pMyObject) {
        return (ANSC_HANDLE) NULL;
    }

    /*
     * Initialize the common variables and functions for a container object.
     */
    pMyObject->Oid               = COSA_DATAMODEL_BASE_OID;
    pMyObject->Create            = CosaBackEndManagerCreate;
    pMyObject->Remove            = CosaBackEndManagerRemove;
    pMyObject->Initialize        = CosaBackEndManagerInitialize;
    /*pMyObject->Initialize   ((ANSC_HANDLE)pMyObject);*/

    return  (ANSC_HANDLE)pMyObject;
}

/**********************************************************************

 caller:     self

 prototype:

 ANSC_STATUS
 CosaBackEndManagerInitialize
 (
 ANSC_HANDLE                 hThisObject
 );

 description:

 This function initiate cosa manager object and return handle.

 argument:   ANSC_HANDLE                 hThisObject
 This handle is actually the pointer of this object
 itself.

 return:     operation status.

 **********************************************************************/

ANSC_STATUS CosaBackEndManagerInitialize(ANSC_HANDLE hThisObject) {
    ANSC_STATUS returnStatus = ANSC_STATUS_SUCCESS;
    PCOSA_BACKEND_MANAGER_OBJECT pMyObject = (PCOSA_BACKEND_MANAGER_OBJECT) hThisObject;

    AnscTraceWarning(("%s...\n", __FUNCTION__));
    printf("Telemetry DM initialize...\n");

    /* Create DM object */
    pMyObject->hTelemetry = (ANSC_HANDLE) CosaTelemetryCreate();
    AnscTraceWarning(("  CosaBackEndManagerInitialize done!\n"));

    return returnStatus;
}

/**********************************************************************

 caller:     self

 prototype:

 ANSC_STATUS
 CosaBackEndManagerRemove
 (
 ANSC_HANDLE                 hThisObject
 );

 description:

 This function remove cosa manager object and return handle.

 argument:   ANSC_HANDLE                 hThisObject
 This handle is actually the pointer of this object
 itself.

 return:     operation status.

 **********************************************************************/

ANSC_STATUS CosaBackEndManagerRemove(ANSC_HANDLE hThisObject) {
    ANSC_STATUS returnStatus = ANSC_STATUS_SUCCESS;
    PCOSA_BACKEND_MANAGER_OBJECT pMyObject = (PCOSA_BACKEND_MANAGER_OBJECT) hThisObject;

    /* Remove all objects */
    if(pMyObject->hTelemetry) {
        CosaTelemetryRemove(pMyObject->hTelemetry);
    }

    AnscFreeMemory((ANSC_HANDLE) pMyObject);

    return returnStatus;
}

