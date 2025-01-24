/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2018 RDK Management
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

/**************************************************************************

 module: cosa_telemetry_dml.h

 For COSA Data Model Library Development

 -------------------------------------------------------------------

 description:

 This file defines the apis for objects to support Data Model Library.

 -------------------------------------------------------------------


 author:

 -------------------------------------------------------------------

 revision:

 11/18/2019    initial revision.

 **************************************************************************/

#ifndef  _COSA_TELEMETRY_DML_H
#define  _COSA_TELEMETRY_DML_H

#include "cosa_telemetry_internal.h"

/***********************************************************************

 APIs for Object:

 Device.Telemetry.

 *  Telemetry_GetParamStringValue
 *  Telemetry_SetParamStringValue

 ***********************************************************************/
ULONG
Telemetry_GetParamStringValue(ANSC_HANDLE hInsContext, char* ParamName, char* pValue, ULONG* pUlSize);

BOOL
Telemetry_SetParamStringValue(ANSC_HANDLE hInsContext, char* ParamName, char* pString);

BOOL
Telemetry_GetParamUlongValue(ANSC_HANDLE hInsContext, char* ParamName, unsigned int* pValue);
#endif
