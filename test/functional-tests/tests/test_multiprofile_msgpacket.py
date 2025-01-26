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
import pytest
import msgpack
import json
import base64

RUN_START_TIME = None
LOG_PROFILE_ENABLE = "Successfully enabled profile :"

def tomsgpack(json_string):
    # Convert JSON string to Python dictionary
    data = json.loads(json_string)

    # Convert Python dictionary to MessagePack format
    msgpack_data = msgpack.packb(data)

    # Encode MessagePack binary data to base64 string
    base64_data = base64.b64encode(msgpack_data).decode('utf-8')

    print(base64_data)
    return base64_data

#negative case without name field
@pytest.mark.run(order=1)
def test_without_namefield():
    clear_T2logs()
    kill_telemetry(9)
    RUN_START_TIME = dt.now()
    remove_T2bootup_flag()
    clear_persistant_files()
    run_telemetry()
    sleep(10)
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_without_namefield))
    sleep(5)

    #Verify that profile is running
    ERROR_MSG = "Incomplete profile object information, unable to create profile"
    LOG_MSG = "new profileName  to profileList"
    HASH_ERROR_MSG = "Hash value is null checking for versionHash value"
    assert ERROR_MSG in grep_T2logs(ERROR_MSG) #Without name field
    assert LOG_MSG not in grep_T2logs(LOG_MSG) #Empty string in namefield 
    assert HASH_ERROR_MSG in grep_T2logs(HASH_ERROR_MSG) #without hash field
    sleep(10)

#negative case without hashvalue
@pytest.mark.run(order=2)
def test_without_hashvalue():
    clear_T2logs()
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_without_hashvalue))
    sleep(5)
    assert "TR_AC12" in grep_T2logs(LOG_PROFILE_ENABLE) # ===> without hash value
    assert "TR_AC14" in grep_T2logs(LOG_PROFILE_ENABLE) # ===> without version field
    assert "TR_AC15" not in grep_T2logs(LOG_PROFILE_ENABLE) # ===> without Protocol field
    sleep(10)

#negative - random value for Protocol 
@pytest.mark.run(order=3)
def test_with_wrong_protocol_value():
    clear_T2logs()
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_with_wrong_protocol_value))
    sleep(5)
    ERROR_WRONG_PROTOCOL = "Unsupported report sending protocol"
    assert ERROR_WRONG_PROTOCOL in grep_T2logs(ERROR_WRONG_PROTOCOL)
    assert "TR_AC16" not in grep_T2logs(LOG_PROFILE_ENABLE) # ===> 
    assert "TR_AC17" in grep_T2logs(LOG_PROFILE_ENABLE) # ===> To confirm for version
    assert "TR_AC13" not in grep_T2logs(LOG_PROFILE_ENABLE) # ===> To confirm for Protocol
    sleep(5)

#negative case without EncodingType & ActivationTimeout values
@pytest.mark.run(order=4)
def test_without_EncodingType_ActivationTimeout_values():
    ERROR_REPORTING_INTERVAL = "If TriggerCondition is not given ReportingInterval parameter is mandatory"
    clear_T2logs()
    kill_telemetry(9)
    RUN_START_TIME = dt.now()
    remove_T2bootup_flag()
    clear_persistant_files()
    run_telemetry()
    sleep(10)
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_without_EncodingType_ActivationTimeout_values))
    sleep(5)
    assert "TR_AC18" in grep_T2logs(LOG_PROFILE_ENABLE) # ===> To confirm 
    assert "TR_AC19" in grep_T2logs(LOG_PROFILE_ENABLE) # ===> To confirm 
    assert "TR_AC20" not in grep_T2logs(LOG_PROFILE_ENABLE) # ===> To confirm 
    assert "TR_AC21" in grep_T2logs(LOG_PROFILE_ENABLE) # ===> To confirm
    assert ERROR_REPORTING_INTERVAL in grep_T2logs(ERROR_REPORTING_INTERVAL)
    assert "TR_AC22" not in grep_T2logs(LOG_PROFILE_ENABLE) # ===> To confirm 
    assert "TR_AC23" in grep_T2logs(LOG_PROFILE_ENABLE) # ===> To confirm
    sleep(5)

#positive case for Reporting Interval
@pytest.mark.run(order=5)
def test_reporting_interval_working():
    clear_T2logs()
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_with_reporting_interval))
    TEST_LOG1 = grep_T2logs("reporting interval is taken - TR_AC23")

    command1 = ["telemetry2_0_client", "TEST_EVENT_MARKER_1", "300"]

   # Execute the command
    result = subprocess.run(command1, capture_output=True, text=True)
    sleep(10)
    result = subprocess.run(command1, capture_output=True, text=True)
    assert "45 sec" in TEST_LOG1
    sleep(50)
    assert "TIMEOUT for profile" in grep_T2logs("TR_AC23") #Verify reporting interval 
    assert "TEST_EVENT_MARKER_1\":\"2" in grep_T2logs("cJSON Report ") #verify event marker & count as well
    sleep(10)

#positive case for activation timeout
@pytest.mark.run(order=6)
def test_for_activation_timeout():

    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_with_activation_timeout))
    sleep(10)
    assert "TR_AC77" in grep_T2logs(LOG_PROFILE_ENABLE) 
    #TODO - Add verification for GenerateNow
    assert "SYS_INFO_CrashPortalUpload_success" in grep_T2logs("cJSON Report ")
    assert "MODEL_NAME" in grep_T2logs("cJSON Report ")
    sleep(10)

#Negative case with activation timeout less than reporting interval
@pytest.mark.run(order=7)
def test_for_invalid_activation_timeout():
    ERROR_PROFILE_TIMEOUT = "activationTimeoutPeriod is less than reporting interval. invalid profile: "
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_with_less_activation_timeout))
    assert "TR_AC88" in grep_T2logs(ERROR_PROFILE_TIMEOUT)
    sleep(10)

#positive case with delete on timeout
@pytest.mark.run(order=8)
def test_with_delete_on_timeout():
    LOG_PROFILE_TIMEOUT = "Profile activation timeout"
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", tomsgpack(data_with_delete_on_timeout))
    sleep(100)
    assert "TR_AC66" in grep_T2logs(LOG_PROFILE_TIMEOUT)
    sleep(10)
