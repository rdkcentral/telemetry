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

#ifdef __GNUC__
#if (!defined _BUILD_ANDROID) && (!defined _NO_EXECINFO_H_)
#include <execinfo.h>
#endif
#endif

#include "ssp_global.h"
#include "stdlib.h"
#include "ccsp_dm_api.h"
#ifdef USE_PCD_API_EXCEPTION_HANDLING
#include "pcdapi.h"
#endif

#ifdef INCLUDE_BREAKPAD
#include "breakpad_wrapper.h"
#endif

#include <semaphore.h>
#include <fcntl.h>
#include <mqueue.h>

PDSLH_CPE_CONTROLLER_OBJECT     pDslhCpeController      = NULL;
PCOMPONENT_COMMON_DM            g_pComponent_Common_Dm  = NULL;
char                            g_Subsystem[32]         = {0};
PCCSP_COMPONENT_CFG             gpT2StartCfg           = NULL;
PCCSP_FC_CONTEXT                pT2FcContext           = (PCCSP_FC_CONTEXT           )NULL;
PCCSP_CCD_INTERFACE             pT2CcdIf               = (PCCSP_CCD_INTERFACE        )NULL;
PCCC_MBI_INTERFACE              pT2MbiIf               = (PCCC_MBI_INTERFACE         )NULL;
BOOL                            g_bActive               = FALSE;
extern  ANSC_HANDLE             bus_handle;


int  cmd_dispatch(int  command)
{
    char                            CName[256];

    _ansc_snprintf(CName, sizeof(CName), "%s%s", g_Subsystem, gpT2StartCfg->ComponentId);

    ssp_T2Mbi_MessageBusEngage
        ( 
         CName,
         CCSP_MSG_BUS_CFG,
         gpT2StartCfg->DbusPath
        );

    ssp_create_t2(gpT2StartCfg);
    ssp_engage_t2(gpT2StartCfg);

    g_bActive = TRUE;

    CcspTraceInfo(("Telemetry 2.0 Module loaded successfully...\n"));

    return 0;
}

char *getComponentId()
{
    char *componentId = NULL;

    if (gpT2StartCfg == NULL)
    {
        gpT2StartCfg = (PCCSP_COMPONENT_CFG)AnscAllocateMemory(sizeof(CCSP_COMPONENT_CFG));
        if (gpT2StartCfg)
        {
            CcspComponentLoadCfg(CCSP_T2_START_CFG_FILE, gpT2StartCfg);
        }
        else
        {
            return componentId;
        }
    }

    if (gpT2StartCfg->ComponentId[0] != '\0')
    {
        int id_len = strlen(gpT2StartCfg->ComponentId)+strlen("eRT.")+1;
        componentId = malloc(id_len);
        if (componentId)
        {
            snprintf(componentId, id_len, "eRT.%s", gpT2StartCfg->ComponentId);
        }
    }

    return componentId;
}

int initTR181_dm()
{
    DmErr_t                         err;
    char                            *subSys            = NULL;

    /*
     *  Load the start configuration
     */
    if (gpT2StartCfg == NULL)
    {
        gpT2StartCfg = (PCCSP_COMPONENT_CFG)AnscAllocateMemory(sizeof(CCSP_COMPONENT_CFG));
    
        if ( gpT2StartCfg )
        {
            CcspComponentLoadCfg(CCSP_T2_START_CFG_FILE, gpT2StartCfg);
        }
        else
        {
            CcspTraceError(("RDKB_SYSTEM_BOOT_UP_LOG : Telemetry 2.0, Insufficient resources for start configuration, quit!\n"));
            printf("Insufficient resources for start configuration, quit!\n");
            return -1;
        }
    }
    
    /* Set the global pComponentName */
    pComponentName = gpT2StartCfg->ComponentName;

    AnscCopyString(g_Subsystem, "eRT.");

    cmd_dispatch('e');

    err = Cdm_Init(bus_handle, subSys, NULL, NULL, pComponentName);
    if (err != CCSP_SUCCESS)
    {
        fprintf(stderr, "Cdm_Init: %s\n", Cdm_StrError(err));
	CcspTraceError(("RDKB_SYSTEM_BOOT_UP_LOG : Error in Cdm_Init in telemetry \n"));
        return -1;
    }

    return 0;
}

int unInitTR181_dm()
{
    DmErr_t err = Cdm_Term();
    if (err != CCSP_SUCCESS)
    {
        fprintf(stderr, "Cdm_Term: %s\n", Cdm_StrError(err));
        exit(1);
    }
    if (g_bActive) {
        ssp_cancel_t2(gpT2StartCfg);
        g_bActive = FALSE;
    }

    return 0;
}
