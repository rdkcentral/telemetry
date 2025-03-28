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

#ifndef _TELEMETRY2_0_H_
#define _TELEMETRY2_0_H_

#ifdef __cplusplus
extern "C" {
#endif

#define COMPONENT_NAME "telemetry2_0"

// Data elements provided by telemetry 2.0
#define T2_ROOT_PARAMETER "Telemetry.ReportProfiles."
#define T2_EVENT_LIST_PARAM_SUFFIX ".EventMarkerList"
#define T2_EVENT_PARAM "Telemetry.ReportProfiles.EventMarker"
#define T2_PROFILE_UPDATED_NOTIFY "Telemetry.ReportProfiles.ProfilesUpdated"
#define T2_OPERATIONAL_STATUS "Telemetry.OperationalStatus"
#define T2_REPORT_PROFILE_PARAM "Device.X_RDKCENTRAL-COM_T2.ReportProfiles"
#define T2_REPORT_PROFILE_PARAM_MSG_PCK                                        \
  "Device.X_RDKCENTRAL-COM_T2.ReportProfilesMsgPack"
#define T2_TEMP_REPORT_PROFILE_PARAM                                           \
  "Device.X_RDKCENTRAL-COM_T2.Temp_ReportProfiles"
#define T2_TOTAL_MEM_USAGE "Device.X_RDK_T2.TotalUsedMem"

#define T2_MTLS_RFC                                                            \
  "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.MTLS.Enable"
#define PRIVACYMODES_RFC "Device.X_RDKCENTRAL-COM_Privacy.PrivacyMode"
#define T2_ON_DEMAND_REPORT "Device.X_RDKCENTRAL-COM_T2.UploadDCMReport"
#define T2_ABORT_ON_DEMAND_REPORT "Device.X_RDKCENTRAL-COM_T2.AbortDCMReport"

#ifdef DCMAGENT
/* DCM Rbus Events */
#define T2_EVENT_DCM_SETCONF "Device.DCM.Setconfig"
#define T2_EVENT_DCM_PROCCONF "Device.DCM.Processconfig"
#define T2_DCM_RELOAD_EVENT "Device.X_RDKCENTREL-COM.Reloadconfig"
#define T2_DCM_SET_CONFIG "dcmSetConfig"
#define T2_DCM_START_CONFIG "dcmStartConfig"
#endif

#define INFINITE_TIMEOUT (unsigned int)~0

#define T2_RP 0
#define T2_TEMP_RP 1

#define ON_DEMAND_ACTION_UPLOAD "UPLOAD"
#define ON_DEMAND_ACTION_ABORT "ABORT"

typedef enum {
  T2ERROR_SUCCESS,
  T2ERROR_FAILURE,
  T2ERROR_INVALID_PROFILE,
  T2ERROR_PROFILE_NOT_FOUND,
  T2ERROR_PROFILE_NOT_SET,
  T2ERROR_MAX_PROFILES_REACHED,
  T2ERROR_MEMALLOC_FAILED,
  T2ERROR_INVALID_ARGS,
  T2ERROR_INTERNAL_ERROR,
  T2ERROR_NO_RBUS_METHOD_PROVIDER,
  T2ERROR_COMPONENT_NULL
} T2ERROR;

#define T2_CACHE_FILE "/tmp/t2_caching_file"
#define T2_CACHE_LOCK_FILE "/tmp/t2_lock_file"
#define T2_CONFIG_READY "/tmp/.t2ConfigReady"

typedef enum {
  T2_STATE_NOT_READY,
  T2_STATE_COMPONENT_READY,
  T2_STATE_CONFIG_READY,
  T2_STATE_READY
} T2_OPERATIONAL_STATE;

#ifdef __cplusplus
}
#endif

#endif /* _TELEMETRY2_0_H_ */
