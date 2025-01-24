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

/**********************************************************************

    module: ssp_messagebus_interface.c

        For CCSP Secure Software Download

    ---------------------------------------------------------------

    description:

        SSP implementation of the CCSP Message Bus Interface
        Service.

        *   ssp_T2Mbi_MessageBusEngage
        *   ssp_T2Mbi_EventCallback
        
    ---------------------------------------------------------------

    environment:

        Embedded Linux

    ---------------------------------------------------------------

    author:

    ---------------------------------------------------------------

    revision:

        11/18/2019  initial revision.

**********************************************************************/

#include "ssp_global.h"

extern  ANSC_HANDLE         bus_handle;
extern  BOOL                g_bActive;
extern  char                g_Subsystem[32];
extern  PCOMPONENT_COMMON_DM g_pComponent_Common_Dm;

BOOLEAN waitConditionReady
    (
        void*                           hMBusHandle,
        const char*                     dst_component_id,
        char*                           dbus_path,
        char*                           src_component_id
    );

int ssp_T2Mbi_GetHealth ( )
{
    return g_pComponent_Common_Dm->Health;
}

ANSC_STATUS
ssp_T2Mbi_MessageBusEngage
    (
        char * component_id,
        char * config_file,
        char * path
    )
{
    ANSC_STATUS                 returnStatus       = ANSC_STATUS_SUCCESS;
    
    char PsmName[256];

    if ( ! component_id || ! path )
    {
        CcspTraceError((" !!! ssp_T2Mbi_MessageBusEngage: component_id or path is NULL !!!\n"));
        return ANSC_STATUS_FAILURE;
    }

    if (bus_handle == NULL)
    {
        /* Connect to message bus */
        returnStatus = 
            CCSP_Message_Bus_Init
            (
             component_id,
             config_file,
             &bus_handle,
             (CCSP_MESSAGE_BUS_MALLOC)Ansc_AllocateMemory_Callback,       /* mallocfc, use default */
             Ansc_FreeMemory_Callback            /* freefc,   use default */
            );

        if ( returnStatus != ANSC_STATUS_SUCCESS )
        {
            CcspTraceError((" !!! T2 Message Bus Init ERROR !!!\n"));

            return returnStatus;
        }
    }

    _ansc_snprintf(PsmName, sizeof(PsmName), "%s%s", g_Subsystem,CCSP_DBUS_PSM);

    /* Wait for PSM */
    waitConditionReady(bus_handle, PsmName, CCSP_DBUS_PATH_PSM, component_id);

    CcspTraceInfo(("!!! Connected to message bus... bus_handle: 0x%8p !!!\n", bus_handle));

    CCSP_Msg_SleepInMilliSeconds(1000);

    /* Base interface implementation that will be used cross components */
    CcspBaseIf_SetCallback2(bus_handle, "getParameterValues", CcspCcMbi_GetParameterValues, NULL);
    CcspBaseIf_SetCallback2(bus_handle, "setParameterValues", CcspCcMbi_SetParameterValues, NULL);
    CcspBaseIf_SetCallback2(bus_handle, "setParameterAttributes", CcspCcMbi_SetParameterAttributes, NULL);
    CcspBaseIf_SetCallback2(bus_handle, "getParameterAttributes", CcspCcMbi_GetParameterAttributes, NULL);
    CcspBaseIf_SetCallback2(bus_handle, "getParameterNames", CcspCcMbi_GetParameterNames, NULL);
    CcspBaseIf_SetCallback2(bus_handle, "currentSessionIDSignal", CcspCcMbi_CurrentSessionIdSignal, NULL);

    /* Base interface implementation that will only be used by this component */
    CcspBaseIf_SetCallback2(bus_handle, "initialize", ssp_T2Mbi_Initialize, NULL);
    CcspBaseIf_SetCallback2(bus_handle, "finalize", ssp_T2Mbi_Finalize, NULL);
    CcspBaseIf_SetCallback2(bus_handle, "freeResources", ssp_T2Mbi_FreeResources, NULL);
    CcspBaseIf_SetCallback2(bus_handle, "busCheck", ssp_T2Mbi_Buscheck, NULL);


    /* Register service callback functions */
    returnStatus =
        CCSP_Message_Bus_Register_Path
            (
                bus_handle,
                path,
                T2_path_message_func,
                bus_handle
            );

    if ( returnStatus != CCSP_Message_Bus_OK )
    {
        CcspTraceError((" !!! CCSP_Message_Bus_Register_Path ERROR returnStatus: %lu\n!!!\n", returnStatus));

        return returnStatus;
    }


    /* Register event/signal */
    returnStatus = 
        CcspBaseIf_Register_Event
            (
                bus_handle,
                0,
                "currentSessionIDSignal"
            );

    if ( returnStatus != CCSP_Message_Bus_OK )
    {
        CcspTraceError((" !!! CCSP_Message_Bus_Register_Event: CurrentSessionIDSignal ERROR returnStatus: %lu!!!\n", returnStatus));

        return returnStatus;
    }

    return ANSC_STATUS_SUCCESS;
}

DBusHandlerResult
T2_path_message_func
    (
        DBusConnection  *conn,
        DBusMessage     *message,
        void            *user_data
    )
{
    CCSP_MESSAGE_BUS_INFO *bus_info =(CCSP_MESSAGE_BUS_INFO *) user_data;
    const char *interface = dbus_message_get_interface(message);
    const char *method   = dbus_message_get_member(message);
    DBusMessage *reply;
    reply = dbus_message_new_method_return (message);
    if (reply == NULL)
    {
        return DBUS_HANDLER_RESULT_HANDLED;
    }

    return CcspBaseIf_base_path_message_func
               (
                   conn,
                   message,
                   reply,
                   interface,
                   method,
                   bus_info
               );
}

int
ssp_T2Mbi_Initialize
    (
        void * user_data
    )
{
    printf("In %s()\n", __FUNCTION__);
    
    return 0;
}

int
ssp_T2Mbi_Finalize
    (
        void * user_data
    )
{
    printf("In %s()\n", __FUNCTION__);

    return 0;
}


int
ssp_T2Mbi_Buscheck
    (
        void * user_data
    )
{
    printf("In %s()\n", __FUNCTION__);

    return 0;
}


int
ssp_T2Mbi_FreeResources
    (
        int priority,
        void * user_data
    )
{
    printf("In %s()\n", __FUNCTION__);

    return 0;
}

