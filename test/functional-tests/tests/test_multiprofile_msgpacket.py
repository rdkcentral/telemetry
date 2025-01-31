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

#negative case without name field, Empty string in namefield & without hash field
@pytest.mark.run(order=1)
def test_without_namefield():
    clear_T2logs()
    kill_telemetry(9)
    RUN_START_TIME = dt.now()
    remove_T2bootup_flag()
    clear_persistant_files()
    run_telemetry()
    #run_shell_command("rdklogctrl telemetry2_0 LOG.RDK.T2 ~DEBUG")
    sleep(10)

    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_without_namefield))
    sleep(20)

    #Verify that profile is running
    ERROR_MSG = "Incomplete profile object information, unable to create profile"
    LOG_MSG = "new profileName  to profileList"
    HASH_ERROR_MSG = "Hash value is null checking for versionHash value"
    assert ERROR_MSG in grep_T2logs(ERROR_MSG) #Without name field
    assert LOG_MSG not in grep_T2logs(LOG_MSG) #Empty string in namefield 
    assert HASH_ERROR_MSG in grep_T2logs(HASH_ERROR_MSG) #without hash field
    sleep(10)

#negative case without hashvalue, without version field & without Protocol field
@pytest.mark.run(order=2)
def test_without_hashvalue():
    clear_T2logs()
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_without_hashvalue))
    sleep(20)
    assert "TR_AC12" in grep_T2logs(LOG_PROFILE_ENABLE)  # without hash value
    assert "TR_AC14" in grep_T2logs(LOG_PROFILE_ENABLE)  # without version field
    assert "TR_AC15" not in grep_T2logs(LOG_PROFILE_ENABLE)  # without Protocol field
    sleep(10)

#negative cases:
# random value for Protocol 
# empty string for version
# empty string for protocol
@pytest.mark.run(order=3)
def test_with_wrong_protocol_value():
    clear_T2logs()
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_with_wrong_protocol_value))
    sleep(20)
    ERROR_WRONG_PROTOCOL = "Unsupported report sending protocol"
    assert ERROR_WRONG_PROTOCOL in grep_T2logs(ERROR_WRONG_PROTOCOL) #Verify the right protocol is given
    assert "TR_AC16" not in grep_T2logs(LOG_PROFILE_ENABLE) # Verify profile is not enabled with an incorrect protocol
    assert "TR_AC17" in grep_T2logs(LOG_PROFILE_ENABLE) # Verify Profile can be enabled for empty version
    assert "TR_AC13" not in grep_T2logs(LOG_PROFILE_ENABLE) # Verify Profile cannot be enabled for empty protocol
    sleep(5)

#negative cases
# without EncodingType & ActivationTimeout values
# without encodingType param
# without ActivationTimeout param
# without ReportingInterval param
# without GenerateNow param
@pytest.mark.run(order=4)
def test_without_EncodingType_ActivationTimeout_values():
    ERROR_REPORTING_INTERVAL = "If TriggerCondition is not given ReportingInterval parameter is mandatory"
    clear_T2logs()
    kill_telemetry(9)
    RUN_START_TIME = dt.now()
    remove_T2bootup_flag()
    clear_persistant_files()
    run_telemetry()
    run_shell_command("rdklogctrl telemetry2_0 LOG.RDK.T2 ~DEBUG")
    sleep(10)
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_without_EncodingType_ActivationTimeout_values))
    sleep(20)
    sleep(5)
    assert "TR_AC18" in grep_T2logs(LOG_PROFILE_ENABLE) # Verify profile is enabled with an empty encodingType
    assert "TR_AC19" in grep_T2logs(LOG_PROFILE_ENABLE) # Verify profile is enabled with an empty ActivationTimeout
    assert "TR_AC20" not in grep_T2logs(LOG_PROFILE_ENABLE) # Verify profile is not enabled without encodingType param
    assert "TR_AC21" in grep_T2logs(LOG_PROFILE_ENABLE) # Verify profile is enabled without ActivationTimeout param
    assert ERROR_REPORTING_INTERVAL in grep_T2logs(ERROR_REPORTING_INTERVAL) # Verify ReportingInterval error is thrown
    assert "TR_AC22" not in grep_T2logs(LOG_PROFILE_ENABLE) # Verify profile is not enabled without ReportingInterval param
    assert "TR_AC23" in grep_T2logs(LOG_PROFILE_ENABLE) # Verify profile is enabled without GenerateNow param
    sleep(5)

#1).positive case for working of Reporting Interval
#2).positive case for event marker & with count
#3).positive case for event marker with accumulate
@pytest.mark.run(order=5)
def test_reporting_interval_working():
    clear_T2logs()
    kill_telemetry(9)
    RUN_START_TIME = dt.now()
    remove_T2bootup_flag()
    clear_persistant_files()
    run_telemetry()
    run_shell_command("rdklogctrl telemetry2_0 LOG.RDK.T2 ~DEBUG")
    sleep(10)
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_with_reporting_interval))
    sleep(20)
    REPORTING_INTERVAL_LOG1 = grep_T2logs("reporting interval is taken - TR_AC732")

    command1 = ["telemetry2_0_client TEST_EVENT_MARKER_1 300"]
    command2 = ["telemetry2_0_client TEST_EVENT_MARKER_2 occurrance1"]
    command3 = ["telemetry2_0_client TEST_EVENT_MARKER_2 occurrance2"]

    run_shell_command(command1)
    sleep(5)
    run_shell_command(command1)
    sleep(5)
    run_shell_command(command2)
    sleep(5)
    run_shell_command(command3)
    sleep(5)
    assert "45 sec" in REPORTING_INTERVAL_LOG1
    sleep(50)
    assert "TIMEOUT for profile" in grep_T2logs("TR_AC732") #Verify reporting interval 
    assert "TEST_EVENT_MARKER_1\":\"2" in grep_T2logs("cJSON Report ") #verify event marker for count 
    assert "occurrance1" in grep_T2logs("TEST_EVENT_MARKER_2") #verify event marker for accummulate - 1
    assert "occurrance2" in grep_T2logs("TEST_EVENT_MARKER_2") #verify event marker for accummulate - 2
    sleep(10)

# verification for GenerateNow
# count - grep marker validation
# absolute - grep marker validation
# Trim - grep marker validation
# Datamodel validation
@pytest.mark.run(order=6)
def test_for_Generate_Now():
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

    LOG_GENERATE_NOW = "Waiting for 0 sec for next TIMEOUT for profile"
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_with_Generate_Now))
    sleep(20)
    assert "TR_AC777" in grep_T2logs(LOG_GENERATE_NOW)  # verification for GenerateNow
    sleep(10)
    assert "SYS_INFO_CrashPortalUpload_success\":\"2" in grep_T2logs("cJSON Report ") #  count - grep marker validation
    assert "FILE_Upload_Progress\":\" newfile1 20%" in grep_T2logs("cJSON Report ") #  absolute - grep marker validation
    assert "FILE_Read_Progress\":\"newfile2 line 10" in grep_T2logs("cJSON Report ") #  Trim - grep marker validation
    assert "MODEL_NAME" in grep_T2logs("cJSON Report ") #  Datamodel validation
    sleep(10)

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
    run_shell_command("rdklogctrl telemetry2_0 LOG.RDK.T2 ~DEBUG")
    sleep(10)

    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_with_less_activation_timeout))
    sleep(60)
    assert "TR_AC88" in grep_T2logs(ERROR_PROFILE_TIMEOUT) # Verify profile not set if activation timeout is less than reporting interval
    assert "MODEL_NAME\":\"NULL" in grep_T2logs("cJSON Report ") # verify Empty report is sent for reportEmpty is true
    assert "TR_AC6919" in grep_T2logs("firstreporting interval is given") #
    assert "5 sec" in grep_T2logs("firstreporting interval is given") #} Verify Firstreporting Interval is working
    assert "NEW TEST PROFILE" in grep_T2logs(LOG_PROFILE_SET) # Verify DCM profile is set
    assert "60 sec" in grep_T2logs("reporting interval is taken - NEW TEST PROFILE") #Verify DCM profile is running
    assert "AccountId\":\"Platform_Container_Test_DEVICE" in grep_T2logs("cJSON Report ") #verify report generated for DCM profile
    sleep(10)

#1).positive case for activation timeout
#2).regex - grep marker validation
#3).regex - Datamodel validation
#4).regex - Event marker validation
#5).positive case with delete on timeout
@pytest.mark.run(order=8)
def test_with_delete_on_timeout():
    clear_T2logs()
    kill_telemetry(9)
    RUN_START_TIME = dt.now()
    remove_T2bootup_flag()
    clear_persistant_files()
    run_telemetry()
    run_shell_command("rdklogctrl telemetry2_0 LOG.RDK.T2 ~DEBUG")
    sleep(10)
    LOG_PROFILE_TIMEOUT = "Profile activation timeout"
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_with_delete_on_timeout))
    sleep(5)
    command2 = ["telemetry2_0_client TEST_EVENT_MARKER_2 occurrance17"]
    run_shell_command(command2)
    sleep(30)
    assert "TR_AC66" in grep_T2logs(LOG_PROFILE_TIMEOUT) # verification for activation timeout
    assert "SYS_INFO_CrashPortalUpload_success\":\"200" in grep_T2logs("cJSON Report ") #  regex - grep marker validation
    assert "MODEL_NAME\":\"DOCKER" in grep_T2logs("cJSON Report ") #  regex - Datamodel validation
    assert "TEST_EVENT_MARKER_2\":\"17" in grep_T2logs("cJSON Report ") #  regex - Event marker validation 
    #assert ""  To be updated once the DeleteOnTimeout is fixed and a LOG is added.

    sleep(5)
