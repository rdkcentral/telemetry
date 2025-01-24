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

#include "plugin_main_apis.h"
#include "cosa_telemetry_internal.h"

/**********************************************************************

 caller:     owner of the object

 prototype:

 ANSC_HANDLE
 CosaTelemetryCreate
 (
 );

 description:

 This function constructs cosa gre object and return handle.

 argument:

 return:     newly created gre object.

 **********************************************************************/

ANSC_HANDLE CosaTelemetryCreate( VOID) {
    PCOSA_DATAMODEL_TELEMETRY pMyObject = (PCOSA_DATAMODEL_TELEMETRY) NULL;

    pMyObject = (PCOSA_DATAMODEL_TELEMETRY) AnscAllocateMemory(sizeof(COSA_DATAMODEL_TELEMETRY));
    if(!pMyObject) {
        return (ANSC_HANDLE) NULL;
    }

    pMyObject->Oid = COSA_DATAMODEL_TELEMETRY_OID;
    pMyObject->Create = CosaTelemetryCreate;
    pMyObject->Remove = CosaTelemetryRemove;
    pMyObject->Initialize = CosaTelemetryInitialize;

    pMyObject->Initialize((ANSC_HANDLE) pMyObject);
    return (ANSC_HANDLE) pMyObject;
}

/**********************************************************************

 caller:     self

 prototype:

 ANSC_STATUS
 CosaTelemetryInitialize
 (
 ANSC_HANDLE                 hThisObject
 );

 description:

 This function initiate  cosa gre object and return handle.

 argument:   ANSC_HANDLE                 hThisObject
 This handle is actually the pointer of this object
 itself.

 return:     operation status.

 **********************************************************************/
ANSC_STATUS CosaTelemetryInitialize(ANSC_HANDLE hThisObject) {
    PCOSA_DATAMODEL_TELEMETRY pMyObject = (PCOSA_DATAMODEL_TELEMETRY) hThisObject;
    ANSC_STATUS returnStatus = ANSC_STATUS_SUCCESS;

    pMyObject->JsonBlob = NULL;
    pMyObject->JsonTmpBlob = NULL;
    pMyObject->MsgpackBlob = NULL;
    return returnStatus;
}

/**********************************************************************

 caller:     self

 prototype:

 ANSC_STATUS
 CosaTelemetryRemove
 (
 ANSC_HANDLE                 hThisObject
 );

 description:

 This function initiate  cosa telemetry object and return handle.

 argument:   ANSC_HANDLE                 hThisObject
 This handle is actually the pointer of this object
 itself.

 return:     operation status.

 **********************************************************************/

ANSC_STATUS CosaTelemetryRemove(ANSC_HANDLE hThisObject) {
    ANSC_STATUS returnStatus = ANSC_STATUS_SUCCESS;
    PCOSA_DATAMODEL_TELEMETRY pMyObject = (PCOSA_DATAMODEL_TELEMETRY) hThisObject;

    /* Free internal data */
    if (pMyObject->JsonBlob)
        AnscFreeMemory((ANSC_HANDLE) (pMyObject->JsonBlob));

    if (pMyObject->JsonTmpBlob)
        AnscFreeMemory((ANSC_HANDLE) (pMyObject->JsonTmpBlob));
    if (pMyObject->MsgpackBlob)
        AnscFreeMemory((ANSC_HANDLE) (pMyObject->MsgpackBlob));
    /* Remove self */
    AnscFreeMemory((ANSC_HANDLE) pMyObject);

    return returnStatus;
}

