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
from test_runs_as_daemon import run_shell_command

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

'''
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
    sleep(25)
    #Multiple profiles configured simultaneously
    assert "TR_AC18" in grep_T2logs(LOG_PROFILE_ENABLE) # Verify profile is enabled with an empty encodingType
    assert "TR_AC19" in grep_T2logs(LOG_PROFILE_ENABLE) # Verify profile is enabled with an empty ActivationTimeout
    assert "TR_AC20" not in grep_T2logs(LOG_PROFILE_ENABLE) # Verify profile is not enabled without encodingType param
    assert ERROR_ENCODING in grep_T2logs(ERROR_ENCODING)
    assert "TR_AC21" in grep_T2logs(LOG_PROFILE_ENABLE) # Verify profile is enabled without ActivationTimeout param
    assert ERROR_REPORTING_INTERVAL in grep_T2logs(ERROR_REPORTING_INTERVAL) # Verify ReportingInterval error is thrown
    assert "TR_AC22" not in grep_T2logs(LOG_PROFILE_ENABLE) # Verify profile is not enabled without ReportingInterval param
    assert "TR_AC23" in grep_T2logs(LOG_PROFILE_ENABLE) # Verify profile is enabled without GenerateNow param
    sleep(5)

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
    sleep(2)
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_with_reporting_interval))
    sleep(5)
    REPORTING_INTERVAL_LOG1 = grep_T2logs("reporting interval is taken - TR_AC732")

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
    sleep(10)
    assert "TIMEOUT for profile" in grep_T2logs("TR_AC732") #Verify reporting interval 
    assert "TEST_EVENT_MARKER_1\":\"2" in grep_T2logs("cJSON Report ") #verify event marker for count
    assert "occurrance1" in grep_T2logs("TEST_EVENT_MARKER_2") #verify event marker for accummulate - 1
    assert "occurrance2" in grep_T2logs("TEST_EVENT_MARKER_2") #verify event marker for accummulate - 2
    assert "TEST_EVENT_MARKER_2_CT" in grep_T2logs("cJSON Report ") #Epoch time/UTC time support
    sleep(2) #wait for 2 sec to verify whether this valid profile is running and generating report

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
    sleep(5)
    assert "TR_AC777" in grep_T2logs(LOG_GENERATE_NOW)  # verification for GenerateNow
    kill_telemetry(29)
    sleep(2)
    assert "LOG_UPLOAD_ONDEMAND received" in grep_T2logs("LOG_UPLOAD_ONDEMAND received") 
    assert "TR_AC767" in grep_T2logs("Interrupted before TIMEOUT for profile")
    assert "SYS_INFO_CrashPortalUpload_success\":\"2" in grep_T2logs("cJSON Report ") #  count - grep marker validation
    assert "FILE_Upload_Progress\":\" newfile1 20%" in grep_T2logs("cJSON Report ") #  absolute - grep marker validation
    assert "FILE_Read_Progress\":\"newfile2 line 10" in grep_T2logs("cJSON Report ") #  Trim - grep marker validation
    assert "MODEL_NAME" in grep_T2logs("cJSON Report ") #  Datamodel validation

# Negative case with activation timeout less than reporting interval
# Postive case for Empty report sent when reportEmpty is true
# Positive case for FirstReporting Interval 
# DCM profile is running parallel to the Multiprofile - positve
@pytest.mark.run(order=7)
def test_for_invalid_activation_timeout():
    ERROR_PROFILE_TIMEOUT = "activationTimeoutPeriod is less than reporting interval. invalid profile: "
    rbus_set_data("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.ConfigURL", "string", "https://mockxconf:50050/loguploader1/getT2DCMSettings")
    clear_T2logs()
    kill_telemetry(9)
    RUN_START_TIME = dt.now()
    remove_T2bootup_flag()
    clear_persistant_files()
    run_telemetry()
    sleep(5)
    run_shell_command("rdklogctrl telemetry2_0 LOG.RDK.T2 ~DEBUG")
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_with_less_activation_timeout))
    command2 = ["telemetry2_0_client SYS_EVENT_TEST 1"]
    run_shell_command(command2)
    sleep(60)
    #kill_telemetry(29)
    assert "TR_AC88" in grep_T2logs(ERROR_PROFILE_TIMEOUT) # Verify profile not set if activation timeout is less than reporting interval
    assert "MODEL_NAME\":\"NULL" in grep_T2logs("cJSON Report ") # verify Empty report is sent for reportEmpty is true
    assert "TR_AC6919" in grep_T2logs("firstreporting interval is given") #
    assert "5 sec" in grep_T2logs("firstreporting interval is given") #} Verify Firstreporting Interval is working
    assert "NEW TEST PROFILE" in grep_T2logs(LOG_PROFILE_SET) # Report fetch and parse via HTTP
    assert "60 sec" in grep_T2logs("reporting interval is taken - NEW TEST PROFILE") #Verify DCM profile is running
    assert "AccountId\":\"Platform_Container_Test_DEVICE" in grep_T2logs("cJSON Report ") #verify report generated for DCM profile
    assert "SYS_GREP_TEST" in grep_T2logs("cJSON Report ") # Data harvesting from previous logs folder for DCA profiles with log file search markers. 
    assert "SYS_GREP_TEST_2" in grep_T2logs("cJSON Report ") # Capability to support multiple split markers for the same log line
    assert "SYS_EVENT_TEST" in grep_T2logs("cJSON Report ") # Include data from data source as T2 events 
    assert "SYS_TEST_ReportUpload" in grep_T2logs("cJSON Report ") # Include data from data source as log files with string match pattern

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
    RET = rbus_set_data(T2_REPORT_PROFILE_PARAM, "string", data_with_delete_on_timeout)
    print(f"RET: {RET}")
    sleep(5)
    command2 = ["telemetry2_0_client TEST_EVENT_MARKER_2 occurrance17"]
    run_shell_command(command2)
    sleep(30)
    assert "TR_AC66" in grep_T2logs(LOG_PROFILE_ENABLE)  # Profile set in JSON format
    assert "TR_AC66" in grep_T2logs(LOG_PROFILE_TIMEOUT) # verification for activation timeout
    assert "SYS_INFO_CrashPortalUpload_success\":\"200" in grep_T2logs("cJSON Report ") #  regex - grep marker validation
    assert "MODEL_NAME\":\"DOCKER" in grep_T2logs("cJSON Report ") #  regex - Datamodel validation
    assert "TEST_EVENT_MARKER_2\":\"17" in grep_T2logs("cJSON Report ") #  regex - Event marker validation 
    assert "TR_AC66" in grep_T2logs(LOG_DELETE_PROFILE) #verify profile is removed from active profile list if DeleteOnTimeout is true

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
    sleep(2)
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_with_first_reporting_interval_max_latency))
    sleep(5)
    assert "PARAM_NULL" not in grep_T2logs(LOG_PROFILE_ENABLE)
    assert "NA_FRI" in grep_T2logs(LOG_PROFILE_ENABLE) #verify when timeref is not default first reporting inetrval is not accepted
    
    assert "NA_MLU" in grep_T2logs(LOG_PROFILE_ENABLE)
    assert "NA_FRI" not in grep_T2logs("Waiting for 5 sec for next TIMEOUT for profile as firstreporting interval is given")
    sleep(10)
    assert "NA_FRI" in grep_T2logs(TIMEOUT_LOG)
    assert "NA_MLU" in grep_T2logs(TIMEOUT_LOG) #verify when timeref is not default max uploadlatency is accepted
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
    run_telemetry()
    sleep(2)
    run_shell_command("rdklogctrl telemetry2_0 LOG.RDK.T2 DEBUG")
    sleep(2)
    rbus_set_data("Device.DeviceInfo.X_RDKCENTRAL-COM.IUI.Version", "string", "T2_Container_0.0.1")
    sleep(1)
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_with_split_markers))
    sleep(2)
    rbus_set_data("Device.DeviceInfo.X_RDKCENTRAL-COM.IUI.Version", "string", "T2_Container_0.0.2")
    rbus_set_data("Device.DeviceInfo.X_RDKCENTRAL-COM.IUI.Version", "string", "T2_Container_0.0.3")
    sleep(10)
    assert "SYS_INFO_WhoAmI" in grep_T2logs("cJSON Report ") # Split marker validation
    assert "SYS_INFO_WhoAmI_Status" in grep_T2logs("cJSON Report ") #  multiple Split markers in the same line
    #assert "SYS_INFO_PreviousLogs" in grep_T2logs("cJSON Report ") #  Previous Logs support for grep
    assert "IUI_VERSION" in grep_T2logs("cJSON Report ") #  tr181 subscribe
    assert "IUI_VERSION_CT" in grep_T2logs("cJSON Report ") #  tr181 subscribe
    assert "Report Sent Successfully over HTTP" in grep_T2logs ("Report Sent Successfully over HTTP") #Report Sending over HTTP

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
    sleep(2)
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_with_triggerconditon_pos))
    sleep(5)
    subprocess.run("rbuscli set Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.Enable  bool false", shell=True)
    sleep(2)
    assert "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.Enable" in grep_T2logs("TriggerConditionResult")
    assert "false" in grep_T2logs("TriggerConditionResult")
    subprocess.run("rbuscli set Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.Enable  bool true", shell=True)
    sleep(2)
    assert "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.Enable" in grep_T2logs("TriggerConditionResult")
    assert "true" in grep_T2logs("TriggerConditionResult") 

'''
@pytest.mark.run(order=11)
def test_for_duplicate_hash():
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_with_split_markers))
    sleep(2)
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_with_split_markers))
    sleep(2)
    assert "Split66" in grep_T2logs("hash already exist")
