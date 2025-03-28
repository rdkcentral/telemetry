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

/**************************************************************************

 module: cosa_telemetry_internal.h

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

#ifndef _COSA_TELEMETRY_INTERNAL_H
#define _COSA_TELEMETRY_INTERNAL_H

#include "cosa_apis.h"
#include "plugin_main_apis.h"

#define COSA_DATAMODEL_TELEMETRY_CLASS_CONTENT                                 \
  /* duplication of the base object class content */                           \
  COSA_BASE_CONTENT                                                            \
  char *JsonBlob;                                                              \
  char *MsgpackBlob;                                                           \
  char *JsonTmpBlob;

typedef struct _COSA_DATAMODEL_TELEMETRY {
  COSA_DATAMODEL_TELEMETRY_CLASS_CONTENT
} COSA_DATAMODEL_TELEMETRY, *PCOSA_DATAMODEL_TELEMETRY;

/*
 Standard function declaration
 */
ANSC_HANDLE
CosaTelemetryCreate(VOID);

ANSC_STATUS
CosaTelemetryInitialize(ANSC_HANDLE hThisObject);

ANSC_STATUS
CosaTelemetryRemove(ANSC_HANDLE hThisObject);

#endif
