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
    rbus_set_data(T2_TEMP_REPORT_PROFILE_PARAM, "string", (data_with_wrong_protocol_value))
    sleep(10)
    ERROR_WRONG_PROTOCOL = "Unsupported protocol"
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
    ERROR_ENCODING = "Incomplete profile information, unable to create profile"
    clear_T2logs()
    kill_telemetry(9)
    RUN_START_TIME = dt.now()
    remove_T2bootup_flag()
    clear_persistant_files()
    run_telemetry()
    run_shell_command("rdklogctrl telemetry2_0 LOG.RDK.T2 ~DEBUG")
    sleep(2)
    rbus_set_data(T2_TEMP_REPORT_PROFILE_PARAM, "string", (data_without_EncodingType_ActivationTimeout_values))
    sleep(25)
    #Multiple profiles configured simultaneously
    assert "TR_AC18" in grep_T2logs(LOG_PROFILE_ENABLE) # Verify profile is enabled with an empty encodingType
    assert "TR_AC19" not in grep_T2logs(LOG_PROFILE_ENABLE) # Verify profile is not enabled with an empty ActivationTimeout #differs to rp behaviour
    assert "TR_AC20" not in grep_T2logs(LOG_PROFILE_ENABLE) # Verify profile is not enabled without encodingType param
    assert ERROR_ENCODING in grep_T2logs(ERROR_ENCODING)
    assert "TR_AC21" in grep_T2logs(LOG_PROFILE_ENABLE) # Verify profile is enabled without ActivationTimeout param
                                                        # 313 - Multiple profiles configured simultaneously - 1
    assert ERROR_REPORTING_INTERVAL in grep_T2logs(ERROR_REPORTING_INTERVAL) # Verify ReportingInterval error is thrown
    assert "TR_AC22" not in grep_T2logs(LOG_PROFILE_ENABLE) # Verify profile is not enabled without ReportingInterval param
    assert "TR_AC23" in grep_T2logs(LOG_PROFILE_ENABLE) # Verify profile is enabled without GenerateNow param
                                                        # 313 - Multiple profiles configured simultaneously - 2
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
    rbus_set_data(T2_TEMP_REPORT_PROFILE_PARAM, "string", data_with_reporting_interval)
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
    assert "TIMEOUT for profile" in grep_T2logs("TR_AC732") # 314 - Report on interval  
    assert "TEST_EVENT_MARKER_1\":\"2" in grep_T2logs("cJSON Report ") #verify event marker for count
    assert "occurrance1" in grep_T2logs("TEST_EVENT_MARKER_2") # 312 - Include data from data source T2 events parameters as Accumulate
    assert "occurrance2" in grep_T2logs("TEST_EVENT_MARKER_2") # 312 - Include data from data source T2 events parameters as Accumulate
    assert "TEST_EVENT_MARKER_2_CT" in grep_T2logs("cJSON Report ") #Epoch time/UTC time support
    assert "Device.X_RDK_Xmidt.SendData" in grep_T2logs("T2 asyncMethodHandler called: ") # 324 - Report sending over RBUS_METHOD
    assert "send via rbusMethod is failure" in grep_T2logs("send via rbusMethod is failure") # 321 - Caching of upload failed reports
    assert "Report Cached, No. of reportes cached = " in grep_T2logs("Report Cached, No. of reportes cached = ") # 321 - Caching of upload failed reports
    run_shell_command("/usr/local/bin/rbus_timeout.sh")

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

    LOG_GENERATE_NOW = "Waiting for timeref or reporting interval for the profile"
    rbus_set_data(T2_TEMP_REPORT_PROFILE_PARAM, "string", (data_with_Generate_Now))
    sleep(5)
    assert "TR_AC777" in grep_T2logs(LOG_GENERATE_NOW)  # verification for GenerateNow
    kill_telemetry(29)
    sleep(2)
    assert "LOG_UPLOAD_ONDEMAND received" in grep_T2logs("LOG_UPLOAD_ONDEMAND received") # 317 - Forced on demand reporting to support log upload  - 1
    assert "TR_AC767" in grep_T2logs("Interrupted before TIMEOUT for profile") # 317 - Forced on demand reporting to support log upload  - 2
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
    rbus_set_data(T2_TEMP_REPORT_PROFILE_PARAM, "string", (data_with_less_activation_timeout))
    sleep(2)
    rbus_set_data("Device.DeviceInfo.X_RDKCENTRAL-COM.IUI.Version", "string", "T2_Container_0.0.2")
    rbus_set_data("Device.DeviceInfo.X_RDKCENTRAL-COM.IUI.Version", "string", "T2_Container_0.0.3")
    command2 = ["telemetry2_0_client SYS_EVENT_TEST_accum 7"]
    run_shell_command(command2)
    sleep(2)
    command2 = ["telemetry2_0_client SYS_EVENT_TEST_accum 6"]
    run_shell_command(command2)
    sleep(60)
    #kill_telemetry(29)
    assert "TR_AC88" in grep_T2logs(ERROR_PROFILE_TIMEOUT) # Verify profile not set if activation timeout is less than reporting interval
    assert "MODEL_NAME\":\"NULL" in grep_T2logs("cJSON Report ") # verify Empty report is sent for reportEmpty is true
    assert "TR_AC6919" in grep_T2logs("firstreporting interval is given") #
    assert "5 sec" in grep_T2logs("firstreporting interval is given") #} Verify Firstreporting Interval is working
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
    RET = rbus_set_data(T2_TEMP_REPORT_PROFILE_PARAM, "string", data_with_delete_on_timeout)
    print(f"RET: {RET}")
    sleep(5)
    command2 = ["telemetry2_0_client TEST_EVENT_MARKER_2 occurrance17"]
    run_shell_command(command2)
    sleep(30)
    assert "TR_AC66" in grep_T2logs(LOG_PROFILE_ENABLE)  # 301 - Profile setting and parsing in JSON format
    assert "TR_AC66" in grep_T2logs(LOG_PROFILE_TIMEOUT) # 315 - Support for activation timeout of profiles
    assert "SYS_INFO_CrashPortalUpload_success\":\"200" in grep_T2logs("cJSON Report ") # 318 - Regex support for log grep patterns
    assert "MODEL_NAME\":\"DOCKER" in grep_T2logs("cJSON Report ") # 304 - Include data from data source as TR181 Parameter
    assert "TEST_EVENT_MARKER_2\":\"17" in grep_T2logs("cJSON Report ") # 309 - Include data from data source as T2 events
    assert "TR_AC66" in grep_T2logs(LOG_DELETE_PROFILE) # verify profile is removed from active profile list if DeleteOnTimeout is true

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
    rbus_set_data(T2_TEMP_REPORT_PROFILE_PARAM, "string", (data_empty_profile)) # instead of telemetry restart giving empty profile to clear previous profile data 
    sleep(2)
    rbus_set_data(T2_TEMP_REPORT_PROFILE_PARAM, "string", (data_with_first_reporting_interval_max_latency))
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
    rbus_set_data(T2_TEMP_REPORT_PROFILE_PARAM, "string", (data_empty_profile))
    sleep(2)
    rbus_set_data(T2_TEMP_REPORT_PROFILE_PARAM, "string", (data_with_triggerconditon_neg))
    sleep(5)
    assert TC_ERROR_LOG in grep_T2logs(TC_ERROR_LOG)
    assert "Null type verifyTriggerCondition ++out" in grep_T2logs("Null type verifyTriggerCondition ++out")
    assert "Null reference verifyTriggerCondition ++out" in grep_T2logs("Null reference verifyTriggerCondition ++out")
    assert "Unexpected type verifyTriggerCondition ++out" in grep_T2logs("Unexpected type verifyTriggerCondition ++out")
    assert "Null operator verifyTriggerCondition ++out" in grep_T2logs("Null operator verifyTriggerCondition ++out")
    assert "Unexpected operator verifyTriggerCondition ++out" in grep_T2logs("Unexpected operator verifyTriggerCondition ++out")
    assert "Null threshold verifyTriggerCondition ++out" in grep_T2logs("Null threshold verifyTriggerCondition ++out")
    assert "Unexpected reference verifyTriggerCondition ++out" in grep_T2logs("Unexpected reference verifyTriggerCondition ++out")

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
    rbus_set_data(T2_TEMP_REPORT_PROFILE_PARAM, "string", (data_with_split_markers))
    sleep(2)
    rbus_set_data("Device.DeviceInfo.X_RDKCENTRAL-COM.IUI.Version", "string", "T2_Container_0.0.2")
    sleep(1)
    rbus_set_data("Device.DeviceInfo.X_RDKCENTRAL-COM.IUI.Version", "string", "T2_Container_0.0.3")
    #sleep(1)
    #rbus_set_data("Device.DeviceInfo.X_RDKCENTRAL-COM.IUI.Version", "string", "T2_Container_0.0.4")
    sleep(10)
    assert "SYS_INFO_WhoAmI" in grep_T2logs("cJSON Report ") #  307 - Include data from data source as log files with string match pattern
    assert "SYS_INFO_WhoAmI_Status" in grep_T2logs("cJSON Report ") #  multiple Split markers in the same line
    assert "SYS_INFO_PreviousLogs" in grep_T2logs("cJSON Report ") #  308, 327 - Capability to read backwards from previous logs
    assert "T2_Container_0.0.1" in grep_T2logs("IUI_VERSION\":") #  305 - Support for subscribing to TR181 Parameter value change
    assert "T2_Container_0.0.2" in grep_T2logs("IUI_VERSION\":") #  312 - Include data from data source Tr181 parameters as Accumulate
    assert "T2_Container_0.0.3" in grep_T2logs("IUI_VERSION\":") #  312 - Include data from data source Tr181 parameters as Accumulate
    assert "IUI_VERSION_CT" in grep_T2logs("cJSON Report ") # report timestamp
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
    subprocess.run("rbuscli set Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.Enable  bool true", shell=True)
    sleep(2)
    rbus_set_data(T2_TEMP_REPORT_PROFILE_PARAM, "string", (data_with_triggerconditon_pos))
    sleep(5)
    subprocess.run("rbuscli set Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.Enable  bool false", shell=True)
    sleep(2)
    assert "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.Enable" in grep_T2logs("TriggerConditionResult") # 316 - Report on trigger condition - 1
    assert "false" in grep_T2logs("TriggerConditionResult") # 316 - Report on trigger condition - 2

@pytest.mark.run(order=13)
def test_for_profile_non_persistence():
    run_shell_command("rdklogctrl telemetry2_0 LOG.RDK.T2 DEBUG")
    sleep(1)
    rbus_set_data(T2_TEMP_REPORT_PROFILE_PARAM, "string", (data_with_split_markers))
    sleep(20)
    assert "Split66" in grep_T2logs("URL: https://mockxconf:50051/dataLookeMock") # 322 - Configurable reporting end points
                                                                                  # 323 - Configurable URL parameters for HTTP Protocol
    assert "Split66" in grep_T2logs("removing profile :") # 326 - Profile non persistence - 1
    clear_T2logs()
    RUN_START_TIME = dt.now()
    kill_telemetry(9)
    remove_T2bootup_flag()
    run_telemetry()
    run_shell_command("rdklogctrl telemetry2_0 LOG.RDK.T2 ~DEBUG")
    sleep(5)
    assert "Split66" not in grep_T2logs(LOG_PROFILE_ENABLE)  # 326 - Profile non persistence - 2
