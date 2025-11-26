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

import subprocess
import requests
from time import sleep
from datetime import datetime as dt
import os
import time
from basic_constants import *
from helper_functions import *
from report_profiles import *
import pytest
import msgpack
import json
import base64

RUN_START_TIME = None
LOG_PROFILE_ENABLE = "Successfully enabled profile :"
LOG_PROFILE_SET = "Successfully set profile :"

def tomsgpack(json_string):
    # Convert JSON string to Python dictionary
    data = json.loads(json_string)

    # Convert Python dictionary to MessagePack format
    msgpack_data = msgpack.packb(data)

    # Encode MessagePack binary data to base64 string
    base64_data = base64.b64encode(msgpack_data).decode('utf-8')

    print(base64_data)
    return base64_data

#negative case without name field, Empty string in namefield & without hash field
@pytest.mark.run(order=1)
def test_without_namefield():
    clear_T2logs()
    kill_telemetry(9)
    RUN_START_TIME = dt.now()
    remove_T2bootup_flag()
    clear_persistant_files()
    run_telemetry()
    #Enabling debug log lines to get the HASH_ERROR_MSG in the logs
    sleep(2)

    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_without_namefield))
    sleep(10) 

    #Verify that profile is running
    ERROR_MSG = "Incomplete profile object information, unable to create profile"
    LOG_MSG = "Successfully enabled profile : \n"
    HASH_ERROR_MSG = "Hash value is null checking for versionHash value"
    assert ERROR_MSG in grep_T2logs(ERROR_MSG) #Without name field
    assert LOG_MSG not in grep_T2logs(LOG_MSG) #Empty string in namefield 
    assert HASH_ERROR_MSG in grep_T2logs(HASH_ERROR_MSG) #without hash field


#negative case without hashvalue, without version field & without Protocol field
@pytest.mark.run(order=2)
def test_without_hashvalue():
    clear_T2logs()
    RUN_START_TIME = dt.now()
    run_shell_command("rdklogctrl telemetry2_0 LOG.RDK.T2 ~DEBUG")
    sleep(2)
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_without_hashvalue))
    sleep(20) #All profiles are erorr case profiles so adding 20 sec for processing of profiles
    PROTOCOL_ERROR_MSG = "Incomplete Profile information, ignoring profile"
    assert "TR_AC12" not in grep_T2logs(LOG_PROFILE_ENABLE)  # without hash value
    assert "TR_AC14" in grep_T2logs(LOG_PROFILE_ENABLE)  # without version field
    assert "TR_AC15" not in grep_T2logs(LOG_PROFILE_ENABLE)  # without Protocol field
    assert PROTOCOL_ERROR_MSG in grep_T2logs(PROTOCOL_ERROR_MSG) # verify whether the protocol is given

#negative cases:
# random value for Protocol 
# empty string for version
# empty string for protocol
@pytest.mark.run(order=3)
def test_with_wrong_protocol_value():
    clear_T2logs()
    kill_telemetry(9)
    RUN_START_TIME = dt.now()
    remove_T2bootup_flag()
    clear_persistant_files()
    run_telemetry()
    run_shell_command("rdklogctrl telemetry2_0 LOG.RDK.T2 ~DEBUG")
    sleep(2)
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_with_wrong_protocol_value))
    sleep(10)
    ERROR_WRONG_PROTOCOL = "Unsupported report sending protocol"
    assert ERROR_WRONG_PROTOCOL in grep_T2logs(ERROR_WRONG_PROTOCOL) #Verify the right protocol is given
    assert "TR_AC16" not in grep_T2logs(LOG_PROFILE_ENABLE) # Verify profile is not enabled with an incorrect protocol
    assert "TR_AC17" in grep_T2logs(LOG_PROFILE_ENABLE) # Verify Profile can be enabled for empty version
    assert "TR_AC13" not in grep_T2logs(LOG_PROFILE_ENABLE) # Verify Profile cannot be enabled for empty protocol
    sleep(2)

#negative cases
# without EncodingType & ActivationTimeout values
# without encodingType param
# without ActivationTimeout param
# without ReportingInterval param
# without GenerateNow param

@pytest.mark.run(order=4)
def test_without_EncodingType_ActivationTimeout_values():
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_empty_profile)) # instead of telemetry restart giving empty profile to clear previous profile data 
    sleep(2)
    ERROR_REPORTING_INTERVAL = "If TriggerCondition is not given ReportingInterval parameter is mandatory"
    ERROR_ENCODING = "Incomplete Profile information, ignoring profile"
    clear_T2logs()
    kill_telemetry(9)
    RUN_START_TIME = dt.now()
    remove_T2bootup_flag()
    clear_persistant_files()
    run_telemetry()
    run_shell_command("rdklogctrl telemetry2_0 LOG.RDK.T2 ~DEBUG")
    sleep(2)
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_without_EncodingType_ActivationTimeout_values))
    sleep(5)
    # 215 - Multiple profiles configured simultaneously
    # 202 - Profile setting and parsing in message pack format
    assert "TR_AC18" in grep_T2logs(LOG_PROFILE_ENABLE) # Verify profile is enabled with an empty encodingType
    assert "TR_AC19" in grep_T2logs(LOG_PROFILE_ENABLE) # Verify profile is enabled with an empty ActivationTimeout
    assert "TR_AC20" not in grep_T2logs(LOG_PROFILE_ENABLE) # Verify profile is not enabled without encodingType param
    assert ERROR_ENCODING in grep_T2logs(ERROR_ENCODING)
    assert "TR_AC21" in grep_T2logs(LOG_PROFILE_ENABLE) # Verify profile is enabled without ActivationTimeout param
    assert ERROR_REPORTING_INTERVAL in grep_T2logs(ERROR_REPORTING_INTERVAL) # Verify ReportingInterval error is thrown
    assert "TR_AC22" not in grep_T2logs(LOG_PROFILE_ENABLE) # Verify profile is not enabled without ReportingInterval param
    assert "TR_AC23" in grep_T2logs(LOG_PROFILE_ENABLE) # Verify profile is enabled without GenerateNow param
    sleep(5)
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_empty_profile)) # instead of telemetry restart giving empty profile to clear previous profile data 
    rbus_set_data(T2_TEMP_REPORT_PROFILE_PARAM, "string", data_empty_profile)
    sleep(2)

#1).positive case for working of Reporting Interval
#2).positive case for event marker & with count
#3).positive case for event marker with accumulate
@pytest.mark.run(order=11)
def test_reporting_interval_working():
    clear_T2logs()
    kill_telemetry(9)
    RUN_START_TIME = dt.now()
    remove_T2bootup_flag()
    clear_persistant_files()
    run_telemetry()
    run_shell_command("rdklogctrl telemetry2_0 LOG.RDK.T2 ~DEBUG")
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_empty_profile))
    sleep(2)
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_with_reporting_interval))
    sleep(2)
    rbus_set_data(T2_TEMP_REPORT_PROFILE_PARAM, "string", data_temp_with_reporting_interval)
    sleep(2)
    REPORTING_INTERVAL_LOG1 = grep_T2logs("reporting interval is taken - TR_AC732")
    run_shell_command("echo WIFI_MAC_10_TOTAL_COUNT:2 >> /opt/logs/wifihealth.txt")
    sleep(2)
    run_shell_command("echo WIFI_MAC_17_TOTAL_COUNT:0 >> /opt/logs/wifihealth.txt")
    command1 = ["telemetry2_0_client TEST_EVENT_MARKER_1 300"]
    command2 = ["telemetry2_0_client TEST_EVENT_MARKER_2 occurrance1"]
    command3 = ["telemetry2_0_client TEST_EVENT_MARKER_2 occurrance2"]

    run_shell_command(command1)
    sleep(2)
    run_shell_command(command1)
    sleep(2)
    run_shell_command(command2)
    sleep(2)
    run_shell_command(command3)
    sleep(2)
    assert "20 sec" in REPORTING_INTERVAL_LOG1
    sleep(15)
    assert "TIMEOUT for profile" in grep_T2logs("TR_AC732") # 218 -Report on interval  
    assert "TEST_EVENT_MARKER_1\":\"2" in grep_T2logs("FR2_US_TC3") # 234 -Include data from data source T2 events as count
    assert "occurrance1\",\"occurrance2" in grep_T2logs("FR2_US_TC3") # 212 - Include data from data source as T2 events - 1
    assert "TEST_EVENT_MARKER_2_CT" in grep_T2logs("FR2_US_TC3") # 248 - Event accumulate with and without timestamp in report profiles for event markers.
    assert "Total_6G_clients_split" not in grep_T2logs("FR2_US_TC3")
    assert "XWIFIS_CNT_2_split" in grep_T2logs("FR2_US_TC3")
                                                                    # 216 - Epoch time/UTC time support
    assert "Device.X_RDK_Xmidt.SendData" in grep_T2logs("T2 asyncMethodHandler called: ") # 228 - Report sending with protocol as RBUS_METHOD in report profiles.
    assert "send via rbusMethod is failure" in grep_T2logs("send via rbusMethod is failure") # 225 - Caching of upload failed reports - 1
    assert "Report Cached, No. of reportes cached = " in grep_T2logs("Report Cached, No. of reportes cached = ") # 225 - Caching of upload failed reports - 2

    assert "TIMEOUT for profile" in grep_T2logs("temp_AC732") # 314 - Report on interval
    assert "occurrance1\",\"occurrance2" in grep_T2logs("temp_AC732") # 212 - Include data from data source as T2 events - 1
    assert "Device.X_RDK_Xmidt.SendData" in grep_T2logs("T2 asyncMethodHandler called: ") # 324 - Report sending over RBUS_METHOD
    assert "send via rbusMethod is failure" in grep_T2logs("send via rbusMethod is failure") # 321 - Caching of upload failed reports
    assert "Report Cached, No. of reportes cached = " in grep_T2logs("Report Cached, No. of reportes cached = ") # 321 - Caching of upload failed reports

    run_shell_command("/usr/local/bin/rbus_timeout.sh")
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_empty_profile)) # instead of telemetry restart giving empty profile to clear previous profile data 
    rbus_set_data(T2_TEMP_REPORT_PROFILE_PARAM, "string", data_empty_profile)
    sleep(2)

# verification for GenerateNow
# count - grep marker validation
# absolute - grep marker validation
# Trim - grep marker validation
# Datamodel validation
@pytest.mark.run(order=6)
def test_for_Generate_Now():
    clear_T2logs()
    os.makedirs('/opt/logs', exist_ok=True)
	# Create log file with the logs needed for grep marker
    file = open('/opt/logs/core_log.txt', 'w')
    file.write(
	    	"Success uploading report 300\n"
		    "Success uploading report 200\n"
    		"random string1\n"
	    	"rando\n"
		    "file uploading newfile1 20%\n"
    		"file reading newfile2 line 10\n"
    		"file writing to file.txt 22 lines\n"
    		"file writing to file.txt 23 lines\n"
	    	)
    file.close()
    sleep(2)

    LOG_GENERATE_NOW = "Waiting for 0 sec for next TIMEOUT for profile"
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_with_Generate_Now))
    sleep(2)
    assert "TR_AC777" in grep_T2logs(LOG_GENERATE_NOW)  # 235 - Support for Generate Now of profiles
    assert "SYS_INFO_CrashPortalUpload_success\":\"2" in grep_T2logs("cJSON Report ") # 236 - Include data from data source as log files with string match pattern as Count
    assert "FILE_Upload_Progress\":\" newfile1 20%" in grep_T2logs("cJSON Report ") # 237 - Include data from data source as log files with string match pattern as absolute
    assert "FILE_Read_Progress\":\"newfile2 line 10" in grep_T2logs("cJSON Report ") # 238 - Include data from data source as log files with string match pattern as with Trim 
    assert "MODEL_NAME" in grep_T2logs("cJSON Report ") # 206 - Include data from data source as TR181 Parameter

# Negative case with activation timeout less than reporting interval
# Postive case for Empty report sent when reportEmpty is true
# Positive case for FirstReporting Interval 
# DCM profile is running parallel to the Multiprofile - positve
@pytest.mark.run(order=7)
def test_for_invalid_activation_timeout():
    ERROR_PROFILE_TIMEOUT = "activationTimeoutPeriod is less than reporting interval. invalid profile: "
    rbus_set_data("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.ConfigURL", "string", "https://mockxconf:50050/loguploader2/getT2DCMSettings")
    os.makedirs('/opt/logs/PreviousLogs', exist_ok=True)
    file = open('/opt/logs/session0.txt', 'w')
    file.write("This log file is for previous logs\n")
    file.write("Second line in the previous logs\n")
    file.close()
    clear_T2logs()
    kill_telemetry(9)
    RUN_START_TIME = dt.now()
    remove_T2bootup_flag()
    clear_persistant_files()
    run_telemetry()
    sleep(5)
    run_shell_command("rdklogctrl telemetry2_0 LOG.RDK.T2 ~DEBUG")
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_with_less_activation_timeout))
    sleep(2)
    rbus_set_data("Device.DeviceInfo.X_RDKCENTRAL-COM.IUI.Version", "string", "T2_Container_0.0.2")
    rbus_set_data("Device.DeviceInfo.X_RDKCENTRAL-COM.IUI.Version", "string", "T2_Container_0.0.3")
    command2 = ["telemetry2_0_client SYS_EVENT_TEST_accum 7"]
    run_shell_command(command2)
    sleep(2)
    command2 = ["telemetry2_0_client SYS_EVENT_TEST_accum 6"]
    run_shell_command(command2)
    sleep(60)
    assert "TR_AC88" in grep_T2logs(ERROR_PROFILE_TIMEOUT) # Verify profile not set if activation timeout is less than reporting interval
    assert "MODEL_NAME\":\"NULL" in grep_T2logs("cJSON Report ") # 239 - Support for reportEmpty of profiles
    assert "TR_AC6919" in grep_T2logs("firstreporting interval is given") # 240 - Support for First Reporting Interval -1 
    assert "5 sec" in grep_T2logs("firstreporting interval is given") # 240 - Support for First Reporting Interval - 2
    assert "NEW TEST PROFILE" in grep_T2logs(LOG_PROFILE_SET) # 101 - Report fetch and parse via HTTP
    assert "60 sec" in grep_T2logs("reporting interval is taken - NEW TEST PROFILE") # 107 - Configurable reporting interval 
                                                                                     # 241 - Support for DCM profile and multiprofile parallel execution
    assert "AccountId\":\"Platform_Container_Test_DEVICE" in grep_T2logs("cJSON Report ") # 102 - Include data from data source as TR181 Parameter
    assert "SYS_GREP_TEST" in grep_T2logs("cJSON Report ") # 110, 111 - Data harvesting from previous logs folder for DCA profiles with log file search markers. 
    assert "SYS_GREP_TEST_2" in grep_T2logs("cJSON Report ") # 104 - Capability to support multiple split markers for the same log line
    assert "SYS_EVENT_TEST_accum\":[\"7\",\"6\"" in grep_T2logs("cJSON Report ") # 105 - Include data from data source as T2 events
                                                                                 # 106 - Include data from data source T2 events as Accumulate
    assert "SYS_TEST_ReportUpload" in grep_T2logs("cJSON Report ") # 103 - Include data from data source as log files with string match pattern
    assert "Report Cached, No. of reportes cached = " in grep_T2logs("Report Cached, No. of reportes cached = ") # 108 - Caching of upload failed reports - xconf
    rbus_set_data("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.ConfigURL", "string", "https://mockxconf:50050/loguploader1/getT2DCMSettings")


#1).positive case for activation timeout
#2).regex - grep marker validation
#3).regex - Datamodel validation
#4).regex - Event marker validation
#5).positive case with delete on timeout
@pytest.mark.run(order=8)
def test_with_delete_on_timeout():
    #clear_T2logs()
    RUN_START_TIME = dt.now()
    run_shell_command("rdklogctrl telemetry2_0 LOG.RDK.T2 ~DEBUG")
    sleep(2)
    os.makedirs('/opt/logs', exist_ok=True)
    # Create log file with the logs needed for grep marker
    file = open('/opt/logs/core_log.txt', 'w')
    file.write("Success uploading report 200\n")
    file.close()
    sleep(2)
    LOG_PROFILE_TIMEOUT = "Profile activation timeout"
    LOG_DELETE_PROFILE = "removing profile :"
    rbus_set_data(T2_REPORT_PROFILE_PARAM, "string", data_with_delete_on_timeout)
    sleep(2)
    rbus_set_data(T2_TEMP_REPORT_PROFILE_PARAM, "string", data_temp_with_delete_on_timeout)
    sleep(2)
    command2 = ["telemetry2_0_client TEST_EVENT_MARKER_2 occurrance17"]
    run_shell_command(command2)
    sleep(30)

    assert "rp_TR_AC66" in grep_T2logs(LOG_PROFILE_ENABLE)  # 201 - Profile setting and parsing in JSON format
    assert "rp_TR_AC66" in grep_T2logs(LOG_PROFILE_TIMEOUT) # 219 - Support for activation timeout of profiles
    assert "SYS_INFO_CrashPortalUpload_success\":\"200" in grep_T2logs("rp_TR_AC66") # 222, 250 - Regex support for data formating on log grep patterns in report profiles.
    assert "MODEL_NAME\":\"DOCKER" in grep_T2logs("rp_TR_AC66") # 243 - Include data from data source as TR181 Parameter with regex
    assert "TEST_EVENT_MARKER_2\":\"17" in grep_T2logs("rp_TR_AC66") # 242 - Include data from data source as T2 events with regex
    assert "rp_TR_AC66" in grep_T2logs(LOG_DELETE_PROFILE) # 232 -Support for Delete on Timeout of profiles
    
    assert "temp_TR_AC66" in grep_T2logs(LOG_PROFILE_ENABLE)  # 301 - Profile setting and parsing in JSON format
    assert "temp_TR_AC66" in grep_T2logs(LOG_PROFILE_TIMEOUT) # 315 - Support for activation timeout of profiles
    assert "SYS_INFO_CrashPortalUpload_success\":\"200" in grep_T2logs("temp_TR_AC66") # 318 - Regex support for log grep patterns
    assert "MODEL_NAME\":\"DOCKER" in grep_T2logs("temp_TR_AC66") # 304 - Include data from data source as TR181 Parameter
    assert "TEST_EVENT_MARKER_2\":\"17" in grep_T2logs("temp_TR_AC66") # 309 - Include data from data source as T2 events

#1.First reporting interval is applicable only when time ref is default - non-working case
#2.Maxlatency is applicable only when time ref is not default - non- working case
#3.Maxlatency is greater than reporting interval - non-working case 
#4.Parameter array is entirely empty - non-working case
@pytest.mark.run(order=9)
def test_for_first_reporting_interval_Maxlatency():
    MLU_ERROR_LOG = "MaxUploadLatency is greater than reporting interval. Invalid Profile"
    TIMEOUT_LOG = "TIMEOUT for profile"
    PARAM_ERROR_LOG = "Incomplete profile information, unable to create profile"
    RUN_START_TIME = dt.now()
    run_shell_command("rdklogctrl telemetry2_0 LOG.RDK.T2 ~DEBUG")
    sleep(2)
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_empty_profile)) # instead of telemetry restart giving empty profile to clear previous profile data 
    rbus_set_data(T2_TEMP_REPORT_PROFILE_PARAM, "string", data_empty_profile)
    sleep(2)
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_with_first_reporting_interval_max_latency))
    sleep(5)
    assert "PARAM_NULL" not in grep_T2logs(LOG_PROFILE_ENABLE)
    assert "NA_FRI" in grep_T2logs(LOG_PROFILE_ENABLE) #verify when timeref is not default first reporting inetrval is not accepted
    
    assert "NA_MLU" in grep_T2logs(LOG_PROFILE_ENABLE)
    assert "NA_FRI" not in grep_T2logs("Waiting for 5 sec for next TIMEOUT for profile as firstreporting interval is given")
    sleep(10)
    assert "NA_FRI" in grep_T2logs(TIMEOUT_LOG)
    assert "NA_MLU" in grep_T2logs(TIMEOUT_LOG) # verify when timeref is not default max uploadlatency is accepted 
                                                # 217, 245 - Delayed reporting support/ Maxlatency
    assert MLU_ERROR_LOG in grep_T2logs(MLU_ERROR_LOG)

@pytest.mark.run(order=10)
def test_for_triggerCondition_negative_case():
    TC_ERROR_LOG = "TriggerCondition is invalid, unable to create profile"
    RUN_START_TIME = dt.now()
    run_shell_command("rdklogctrl telemetry2_0 LOG.RDK.T2 DEBUG")
    sleep(2)
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_empty_profile))
    sleep(2)
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_with_triggerconditon_neg))
    sleep(5)
    assert TC_ERROR_LOG in grep_T2logs(TC_ERROR_LOG)
    assert "Null type verifyMsgPckTriggerCondition ++out" in grep_T2logs("Null type verifyMsgPckTriggerCondition ++out")
    assert "Null reference verifyMsgPckTriggerCondition ++out" in grep_T2logs("Null reference verifyMsgPckTriggerCondition ++out")
    assert "Unexpected type verifyMsgPckTriggerCondition ++out" in grep_T2logs("Unexpected type verifyMsgPckTriggerCondition ++out")
    assert "Null operator verifyMsgPckTriggerCondition ++out" in grep_T2logs("Null operator verifyMsgPckTriggerCondition ++out")
    assert "Unexpected operator verifyMsgPckTriggerCondition ++out" in grep_T2logs("Unexpected operator verifyMsgPckTriggerCondition ++out")
    assert "Null threshold verifyMsgPckTriggerCondition ++out" in grep_T2logs("Null threshold verifyMsgPckTriggerCondition ++out")
    assert "Unexpected reference verifyMsgPckTriggerCondition ++out" in grep_T2logs("Unexpected reference verifyMsgPckTriggerCondition ++out")

@pytest.mark.run(order=12)
def test_for_subscribe_tr181():
    clear_T2logs()
    kill_telemetry(9)
    RUN_START_TIME = dt.now()
    remove_T2bootup_flag()
    clear_persistant_files()
    os.makedirs('/opt/logs/PreviousLogs', exist_ok=True)
    file = open('/opt/logs/PreviousLogs/session0.txt', 'w')
    file.write("This log file is for previous logs\n")
    file.write("Second line in the previous logs\n")
    file.close()
    run_telemetry()
    sleep(2)
    run_shell_command("rdklogctrl telemetry2_0 LOG.RDK.T2 DEBUG")
    sleep(2)
    rbus_set_data("Device.DeviceInfo.X_RDKCENTRAL-COM.IUI.Version", "string", "T2_Container_0.0.1")
    sleep(1)
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_with_split_markers))
    sleep(1)
    rbus_set_data(T2_TEMP_REPORT_PROFILE_PARAM, "string", (data_temp_with_split_markers))
    sleep(2)
    rbus_set_data("Device.DeviceInfo.X_RDKCENTRAL-COM.IUI.Version", "string", "T2_Container_0.0.2")
    sleep(2)
    rbus_set_data("Device.DeviceInfo.X_RDKCENTRAL-COM.IUI.Version", "string", "T2_Container_0.0.3")
    sleep(10)
    assert "SYS_INFO_WhoAmI" in grep_T2logs("rp_Split_Marker") # 209 - Include data from data source as log files with string match pattern
    assert "SYS_INFO_WhoAmI_Status" in grep_T2logs("rp_Split_Marker") # 211 - Capability to support multiple split markers for the same log line
    assert "SYS_INFO_PreviousLogs" in grep_T2logs("rp_Split_Marker") # 210, 231, 246 - Data harvesting from previous logs folder for report profiles with log file search markers.
    assert "T2_Container_0.0.2" in grep_T2logs("rp_Split_Marker") # 207 - Support for subscribing to TR181 Parameter value change
                                                                 # 214 -Include data from data source Tr181 parameters as Accumulate
    assert "T2_Container_0.0.3" in grep_T2logs("rp_Split_Marker") # 214 -Include data from data source Tr181 parameters as Accumulate
    assert "IUI_VERSION_CT" in grep_T2logs("rp_Split_Marker") # 249 - Event accumulate with and without timestamp in report profiles for datamodel markers.
    assert "Report Sent Successfully over HTTP" in grep_T2logs ("Report Sent Successfully over HTTP") # 223 - Report sending over HTTP protocol

    assert "SYS_INFO_WhoAmI" in grep_T2logs("temp_Split_Marker") #  307 - Include data from data source as log files with string match pattern
    assert "SYS_INFO_PreviousLogs" in grep_T2logs("temp_Split_Marker") #  308, 327 - Capability to read backwards from previous logs
    assert "T2_Container_0.0.2" in grep_T2logs("temp_Split_Marker") # 305 - Support for subscribing to TR181 Parameter value change
                                                                    # 312 - Include data from data source Tr181 parameters as Accumulate
    assert "T2_Container_0.0.3" in grep_T2logs("temp_Split_Marker") # 312 - Include data from data source Tr181 parameters as Accumulate
    assert "Report Sent Successfully over HTTP" in grep_T2logs ("Report Sent Successfully over HTTP") # 319 - Report sending over HTTP protocol

@pytest.mark.run(order=11)
def test_for_triggerCondition_working_case():
    clear_T2logs()
    RUN_START_TIME = dt.now()
    kill_telemetry(9)
    remove_T2bootup_flag()
    clear_persistant_files()
    run_telemetry()
    sleep(5)
    run_shell_command("rdklogctrl telemetry2_0 LOG.RDK.T2 ~DEBUG")
    subprocess.run("rbuscli set Device.DeviceInfo.X_RDKCENTRAL-COM_FirmwareDownloadDeferReboot  bool false", shell=True)
    subprocess.run("rbuscli set Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.Enable  bool true", shell=True)

    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_with_triggerconditon_pos))
    rbus_set_data(T2_TEMP_REPORT_PROFILE_PARAM, "string", (data_temp_with_triggerconditon_pos))
    sleep(2)
    subprocess.run("rbuscli set Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.Enable  bool false", shell=True)
    subprocess.run("rbuscli set Device.DeviceInfo.X_RDKCENTRAL-COM_FirmwareDownloadDeferReboot  bool true", shell=True)
    sleep(2)
    assert "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.Enable" in grep_T2logs("TriggerConditionResult") # 220, 244 - Report on trigger condition
    assert "false" in grep_T2logs("TriggerConditionResult") # 251 - Report generation on trigger condition with stress testing for covering deadlock scenarios - 1
    assert "Device.DeviceInfo.X_RDKCENTRAL-COM_FirmwareDownloadDeferReboot" in grep_T2logs("TriggerConditionResult") # 316 - Report on trigger condition - 1
    assert "true" in grep_T2logs("TriggerConditionResult") # 316 - Report on trigger condition - 2
    subprocess.run("rbuscli set Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.Enable  bool true", shell=True)
    sleep(2)
    assert "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.Enable" in grep_T2logs("TriggerConditionResult")
    assert "true" in grep_T2logs("TriggerConditionResult") # 251 - Report generation on trigger condition with stress testing for covering deadlock scenarios - 2
    sleep(1)
    subprocess.run("rbuscli set Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.Enable  bool false", shell=True)
    subprocess.run("rbuscli set Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.Enable  bool true", shell=True)
    subprocess.run("rbuscli set Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.Enable  bool false", shell=True)

@pytest.mark.run(order=13)
def test_for_duplicate_hash():
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_empty_profile))
    sleep(2)
    run_shell_command("rdklogctrl telemetry2_0 LOG.RDK.T2 DEBUG")
    run_shell_command("cp /opt/logs/core_log.txt /opt/logs/core_log.txt.0")
    run_shell_command("cp /opt/logs/core_log.txt /opt/logs/core_log.txt.1")
    run_shell_command("echo Rotated_log_line >> /opt/logs/core_log.txt.1")
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_for_persistence))
    rbus_set_data(T2_TEMP_REPORT_PROFILE_PARAM, "string", (data_temp_for_persistence))
    sleep(2)
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_for_persistence))
    sleep(2)
    assert "per_66" in grep_T2logs("hash already exist") # 203 - Check for HASH value matches of profile to avoid duplicate processing
    run_shell_command("cp test/functional-tests/tests/rotated.txt /opt/logs/")
    sleep(6)
    assert "SYS_INFO_Rotated_Log\":\"1" in grep_T2logs("cJSON Report ") 
    run_shell_command("cp test/functional-tests/tests/rotated.txt.1 /opt/logs/")
    run_shell_command("cp test/functional-tests/tests/rotated.txt.reduced /opt/logs/rotated.txt")
    sleep(10)
    assert "SYS_INFO_Rotated_Log\":\"1" in grep_T2logs("cJSON Report ") # 247 - Report generation for profiles with log grep markers during log file rotation scenarios.
    assert "per_66" in grep_T2logs("URL: https://mockxconf:50051/dataLookeMock") # 226 - Configurable reporting end points
                                                                                  # 227 - Configurable URL parameters for HTTP Protocol
    #assert "temp_Split66" in grep_T2logs("URL: https://mockxconf:50051/dataTempLookeMock") # 322 - Configurable reporting end points
                                                                                  # 323 - Configurable URL parameters for HTTP Protocol
    assert "per_66" in grep_T2logs("removing profile :") # 229 - Profile persistence - 1
    assert "temp_per_66" in grep_T2logs("removing profile :") # 326 - Profile persistence - 1
    clear_T2logs()
    RUN_START_TIME = dt.now()
    kill_telemetry(9)
    remove_T2bootup_flag()
    run_telemetry()
    run_shell_command("rdklogctrl telemetry2_0 LOG.RDK.T2 ~DEBUG")
    sleep(5)
    assert "per_66" in grep_T2logs(LOG_PROFILE_ENABLE)  # 229 - Profile persistence - 2
    assert "temp_per_66" not in grep_T2logs(LOG_PROFILE_ENABLE)  # 326 - Profile persistence - 2
    kill_telemetry(29)
    sleep(2)
    assert "LOG_UPLOAD_ONDEMAND received" in grep_T2logs("LOG_UPLOAD_ONDEMAND received") # 221, 252- Forced on demand reporting outside the regular reporting intervals. - 1
    assert "per_66" in grep_T2logs("Sending Interrupt signal to Timeout Thread of profile") # 252 - Forced on demand reporting outside the regular reporting intervals. - 2
    assert "rp_Split_Marker" in grep_T2logs("cJSON Report ") # 247 - Report generation for profiles with log grep markers during log file rotation scenarios.

@pytest.mark.run(order=14)
def test_stress_test():
    command_to_get_pid = "pidof telemetry2_0"
    pid1 = run_shell_command(command_to_get_pid)
    run_shell_command("test/functional-tests/tests/t2_app 9999")
    sleep(5)
    pid2 = run_shell_command(command_to_get_pid)
    assert pid1==pid2 #  253 - Stress testing of interaction with rbus interface to check for any deadlocks or rbus timeouts.

@pytest.mark.run(order=15)
def test_grep_accumulate():
    os.makedirs('/opt/logs', exist_ok=True)
    run_shell_command("rm -rf /opt/logs/accum.log")
    run_shell_command("cp test/functional-tests/tests/accum.log /opt/logs/")
    sleep(1)

    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_with_grep_accumulate_timestamp))
    sleep(20)
    assert "SYS_INFO_Accum_Time" in grep_T2logs("cJSON Report ") # Verify that the values are accumulated upto 20 values and a warning message is added. 
    assert "SYS_INFO_Accum_Time_CT" in grep_T2logs("cJSON Report ") # Matched timetamp will be reported as a different marker
    assert "SYS_INFO_Accum_No_Time" in grep_T2logs("cJSON Report ") # Marker is reporting even without matching timestamp
    assert "SYS_INFO_Accum_CT" not in grep_T2logs("cJSON Report ") # timestamp marker is not reporting without a matching timestamp
    assert "SYS_INFO_Accum_Alone" in grep_T2logs("cJSON Report ") # Accumulate marker is reporting when reporttimestamp is not configured
    assert "Load_Average" in grep_T2logs("cJSON Report ") # 
    assert "cpu_telemetry2_0" in grep_T2logs("cJSON Report ") # 
    assert "mem_telemetry2_0" in grep_T2logs("cJSON Report ") # 
    file = open('/opt/logs/accum.log', 'a')
    file.write(
            "251007-09:29:39.441 INFO     identifier:thevalue23\n"
            "251007-09:29:40.441 ERROR     identifier:thevalue24\n"
            "filler updated line\n"
            "251007-09:29:41.335 INFO     identifier:thevalue25\n"
            "filler line\n"
            )
    file.close()
    sleep(15)
    assert "SYS_INFO_Accum_Time\":[\"thevalue23" in grep_T2logs("cJSON Report ") #Marker is reporting  in the next cycle even if the maximum accumulation is reached in the previous report 
