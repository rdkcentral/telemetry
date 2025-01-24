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
#include <stdlib.h>
#include <rbus/rbus.h>

#include "t2log_wrapper.h"
#include "busInterface.h"

#if defined(CCSP_SUPPORT_ENABLED)
#include "ccspinterface.h"
#endif

#include "rbusInterface.h"

static bool isRbus = false ;
static bool isBusInit = false ;

bool isRbusEnabled( ) {
    T2Debug("%s ++in \n", __FUNCTION__);
    if(RBUS_ENABLED == rbus_checkStatus()) {
        isRbus = true;
    } else {
        isRbus = false;
    }
    T2Debug("RBUS mode active status = %s \n", isRbus ? "true":"false");
    T2Debug("%s --out \n", __FUNCTION__);
    return isRbus;
}

static bool busInit( ) {
    T2Debug("%s ++in \n", __FUNCTION__);
    if(!isBusInit) {
        if (isRbusEnabled())
	    T2Debug("%s --RBUS mode is active \n", __FUNCTION__); //CID 158206:Unchecked return value
        isBusInit = true;
    }
    T2Debug("%s --out \n", __FUNCTION__);
    return isBusInit;
}

T2ERROR getParameterValue(const char* paramName, char **paramValue)
{
    T2Debug("%s ++in \n", __FUNCTION__);
    T2ERROR ret = T2ERROR_FAILURE ;
    if(!isBusInit)
        busInit();

    if(isRbus)
        ret = getRbusParameterVal(paramName,paramValue);
#if defined(CCSP_SUPPORT_ENABLED)
    else
        ret = getCCSPParamVal(paramName, paramValue);
#endif

    T2Debug("%s --out \n", __FUNCTION__);
    return ret;
}

Vector* getProfileParameterValues(Vector *paramList)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    Vector *profileValueList = NULL;
    if(!isBusInit)
        busInit();

    if(isRbus)
    	profileValueList = getRbusProfileParamValues(paramList);
#if defined(CCSP_SUPPORT_ENABLED)
    else
        profileValueList = getCCSPProfileParamValues(paramList);
#endif

    T2Debug("%s --Out\n", __FUNCTION__);
    return profileValueList;
}

/**
 * Register with right bus call back dpending on dbus/rbus mode
 */
T2ERROR registerForTelemetryEvents(TelemetryEventCallback eventCB)
{
	T2ERROR ret = T2ERROR_FAILURE;
    T2Debug("%s ++in\n", __FUNCTION__);
    if(!isBusInit)
        busInit();

    if (isRbus) 
    {
    	ret = registerRbusT2EventListener(eventCB);
		/* Register DCM Events */
		ret = registerRbusDCMEventListener();

    }
#if defined(CCSP_SUPPORT_ENABLED) 
    else {
    	ret = registerCcspT2EventListener(eventCB);
    }
#endif

    T2Debug("%s --out\n", __FUNCTION__);
    return ret;
}

T2ERROR unregisterForTelemetryEvents()
{
    return T2ERROR_SUCCESS;
}


T2ERROR busUninit()
{
    if (isRbus)
    {
    	unregisterRbusT2EventListener();
    }
    return T2ERROR_SUCCESS;
}
