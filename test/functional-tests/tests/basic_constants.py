####################################################################################
# If not stated otherwise in this file or this component's Licenses file the
# following copyright and licenses apply:
#
# Copyright 2024 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
####################################################################################


ADMIN_SUPPORT_URL = "https://mockxconf:50050/adminSupport"
CONFIG_URL = "https://mockxconf:50050/"
ADMIN_CACHE_ARG = "saveRequest"
ADMIN_RQUEST_DATA = "returnData"
RUN_START_TIME = None
ADMIN_SUPPORT_GET = ADMIN_SUPPORT_URL + "Get"
ADMIN_SUPPORT_SET = ADMIN_SUPPORT_URL + "Set"

XCONF_PERSISTANT_REGX = "/opt/.t2*"
XCONF_PERSISTANT_FILE = "/opt/.t2persistentfolder/DCMresponse.txt"

T2_CONFIG_READY = "/tmp/.t2ConfigReady"
BOOTUP_FLAG = "/tmp/telemetry_initialized_bootup"
T2_READY_TO_RECIVE = "/tmp/.t2ReadyToReceiveEvents"
BOOT_FLAGS_LIST = [T2_CONFIG_READY, BOOTUP_FLAG, T2_READY_TO_RECIVE]

RBUSCLI_GET_CMD = "rbuscli get "
RBUSCLI_SET_CMD = "rbuscli set "

T2_REPORT_PROFILE_PARAM="Device.X_RDKCENTRAL-COM_T2.ReportProfiles"
T2_REPORT_PROFILE_PARAM_MSG_PCK="Device.X_RDKCENTRAL-COM_T2.ReportProfilesMsgPack"
T2_TEMP_REPORT_PROFILE_PARAM="Device.X_RDKCENTRAL-COM_T2.Temp_ReportProfiles"
RBUS_EXCEPTION_STRING = "Failed to get the data"

LOG_FILE = "/opt/logs/telemetry2_0.txt.0"

DATA_LAKE_URL = "https://mockxconf:50051"
DL_ADMIN_URL = "https://mockxconf:50051/adminSupport"


"""
ADMIDIO_URL = "https://mockxconf:50050/adminSupport"
ADMIN_ENABLE_CACHE = ADMIDIO_URL + "?saveRequest=true"
ADMIN_DISABLE_CACHE = ADMIDIO_URL + "?saveRequest=false"
ADMIN_RQUEST_DATA = ADMIDIO_URL + "?returnData=true"
RUN_START_TIME = None

XCONF_PERSISTANT_REGX = "/opt/.t2*"
XCONF_PERSISTANT_FILE = "/opt/.t2persistentfolder/DCMresponse.txt"

T2_CONFIG_READY = "/tmp/.t2ConfigReady"
BOOTUP_FLAG = "/tmp/telemetry_initialized_bootup"
T2_READY_TO_RECIVE = "/tmp/.t2ReadyToReceiveEvents"
RBUSCLI_GET_CMD = "rbuscli get "

T2_REPORT_PROFILE_PARAM="Device.X_RDKCENTRAL-COM_T2.ReportProfiles"
T2_REPORT_PROFILE_PARAM_MSG_PCK="Device.X_RDKCENTRAL-COM_T2.ReportProfilesMsgPack"
T2_TEMP_REPORT_PROFILE_PARAM="Device.X_RDKCENTRAL-COM_T2.Temp_ReportProfiles"
RBUS_EXCEPTION_STRING = "Failed to get the data"
"""
