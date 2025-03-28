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

#include "cosa_telemetry_dml.h"
#include "ansc_platform.h"
#include "datamodel.h"
#include "reportprofiles.h"
#include "t2log_wrapper.h"
#include "telemetry2_0.h"

/***********************************************************************

 APIs for Object:

 Telemetry.AddReportProfile

 *  AddProfile_GetParamStringValue
 *  AddProfile_SetParamStringValue

 ***********************************************************************/

ULONG Telemetry_GetParamStringValue(ANSC_HANDLE hInsContext, char *ParamName,
                                    char *pValue, ULONG *pUlSize) {
  PCOSA_DATAMODEL_TELEMETRY pMyObject =
      (PCOSA_DATAMODEL_TELEMETRY)g_pCosaBEManager->hTelemetry;
  if (strcmp(ParamName, "ReportProfiles") == 0) {
    char *temp = NULL;
    if (pMyObject->JsonBlob == NULL) {
      if (pMyObject->MsgpackBlob != NULL)
        return 0;
      datamodel_getSavedJsonProfilesasString(&temp);
      if (temp != NULL) {
        pMyObject->JsonBlob =
            (char *)AnscAllocateMemory(AnscSizeOfString(temp) + 1);
        strncpy(pMyObject->JsonBlob, temp, strlen(temp));
        pMyObject->JsonBlob[strlen(temp)] = '\0';
        free(temp);
      } else
        return 0;
    }
    if (*pUlSize < strlen(pMyObject->JsonBlob)) {
      *pUlSize = strlen(pMyObject->JsonBlob);
      return 1;
    }

    AnscCopyString(pValue, pMyObject->JsonBlob);
    *pUlSize = strlen(pMyObject->JsonBlob);
    return 0;
  }

  if (strcmp(ParamName, "ReportProfilesMsgPack") == 0) {
    char *temp = NULL;
    int size;

    if (pMyObject->MsgpackBlob == NULL) {
      if (pMyObject->JsonBlob != NULL)
        return 0;
      size = datamodel_getSavedMsgpackProfilesasString(&temp);
      if (temp != NULL && size > 0) {
        ULONG stringSize = size;
        char *Unpack = AnscBase64Encode(temp, stringSize);
        pMyObject->MsgpackBlob =
            (char *)AnscAllocateMemory(AnscSizeOfString(Unpack) + 1);
        strncpy(pMyObject->MsgpackBlob, Unpack, strlen(Unpack));
        pMyObject->MsgpackBlob[strlen(Unpack)] = '\0';
        free(temp);
        free(Unpack);
      } else {
        if (temp != NULL) {
          free(temp);
        }
        return 0;
      }
    }
    if (*pUlSize < strlen(pMyObject->MsgpackBlob)) {
      *pUlSize = strlen(pMyObject->MsgpackBlob);
      return 1;
    }

    AnscCopyString(pValue, pMyObject->MsgpackBlob);
    *pUlSize = strlen(pMyObject->MsgpackBlob);
    return 0;
  }

  if (strcmp(ParamName, "Temp_ReportProfiles") == 0) {
    if (pMyObject->JsonTmpBlob == NULL)
      return 0;

    if (*pUlSize < strlen(pMyObject->JsonTmpBlob)) {
      *pUlSize = strlen(pMyObject->JsonTmpBlob);
      return 1;
    }

    AnscCopyString(pValue, pMyObject->JsonTmpBlob);
    *pUlSize = strlen(pMyObject->JsonTmpBlob);
    return 0;
  }
  return -1;
}

BOOL Telemetry_SetParamStringValue(ANSC_HANDLE hInsContext, char *ParamName,
                                   char *pString) {
  PCOSA_DATAMODEL_TELEMETRY pMyObject =
      (PCOSA_DATAMODEL_TELEMETRY)g_pCosaBEManager->hTelemetry;
  /* CID 159575 : Dereference after null check */
  if (pString == NULL) {
    return FALSE;
  }
  if (strcmp(ParamName, "ReportProfiles") == 0) {
    if (T2ERROR_SUCCESS != datamodel_processProfile(pString, T2_RP)) {
      return FALSE;
    }

    if (pMyObject->JsonBlob != NULL) {
      AnscFreeMemory((ANSC_HANDLE)(pMyObject->JsonBlob));
      pMyObject->JsonBlob = NULL;
    }

    pMyObject->JsonBlob =
        (char *)AnscAllocateMemory(AnscSizeOfString(pString) + 1);
    strncpy(pMyObject->JsonBlob, pString, strlen(pString));
    pMyObject->JsonBlob[strlen(pString)] = '\0';
    /**
     * New profiles were set via JSON blob. Free up the static mem holding stale
     * message pack blob in sync with Rbus mode
     */
    if (pMyObject->MsgpackBlob != NULL) {
      AnscFreeMemory((ANSC_HANDLE)(pMyObject->MsgpackBlob));
      pMyObject->MsgpackBlob = NULL;
    }
    return TRUE;
  }

  if (strcmp(ParamName, "ReportProfilesMsgPack") == 0) {
    char *webConfigString = NULL;
    ULONG stringSize = 0;
    webConfigString = AnscBase64Decode(pString, &stringSize);

    if (T2ERROR_SUCCESS !=
        datamodel_MsgpackProcessProfile(webConfigString, stringSize)) {
      return FALSE;
    }

    if (pMyObject->MsgpackBlob != NULL) {
      AnscFreeMemory((ANSC_HANDLE)(pMyObject->MsgpackBlob));
      pMyObject->MsgpackBlob = NULL;
    }

    pMyObject->MsgpackBlob =
        (char *)AnscAllocateMemory(AnscSizeOfString(pString) + 1);
    strncpy(pMyObject->MsgpackBlob, pString, strlen(pString));
    pMyObject->MsgpackBlob[strlen(pString)] = '\0';
    /**
     * New profiles were set via msgpack blob. Free up the static mem holding
     * stale JSON blob in sync with Rbus mode
     */
    if (pMyObject->JsonBlob != NULL) {
      AnscFreeMemory((ANSC_HANDLE)(pMyObject->JsonBlob));
      pMyObject->JsonBlob = NULL;
    }
    return TRUE;
  }

  if (strcmp(ParamName, "Temp_ReportProfiles") == 0) {
    if (T2ERROR_SUCCESS != datamodel_processProfile(pString, T2_TEMP_RP)) {
      return FALSE;
    }

    if (pMyObject->JsonTmpBlob != NULL) {
      AnscFreeMemory((ANSC_HANDLE)(pMyObject->JsonTmpBlob));
      pMyObject->JsonTmpBlob = NULL;
    }

    pMyObject->JsonTmpBlob =
        (char *)AnscAllocateMemory(AnscSizeOfString(pString) + 1);
    strncpy(pMyObject->JsonTmpBlob, pString, strlen(pString));
    pMyObject->JsonTmpBlob[strlen(pString)] = '\0';
    return TRUE;
  }

  return FALSE;
}

BOOL Telemetry_GetParamUlongValue(ANSC_HANDLE hInsContext, char *ParamName,
                                  unsigned int *pValue) {
  if (strcmp(ParamName, "TotalUsedMem") == 0) {
    profilemem_usage(pValue);
    return TRUE;
  }
  return FALSE;
}
