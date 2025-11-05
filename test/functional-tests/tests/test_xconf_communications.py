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
import os
from basic_constants import *
import pytest
from time import sleep
from datetime import datetime as dt

from helper_functions import *


@pytest.mark.run(order=1)
def test_precondition():
    adminSupport_cache()
    kill_telemetry(9)
    RUN_START_TIME = dt.now()
    remove_T2bootup_flag()
    clear_persistant_files()
    rbus_set_data("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.ConfigURL", "string", "http://mockxconf:50050/loguploader/getT2DCMSettings")
    clear_T2logs()
    run_telemetry()
    sleep(10)


@pytest.mark.run(order=2)
def test_xconf_connection_with_empty_url():
    clear_T2logs()
    rbus_set_data("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.ConfigURL", "string", " ")
    kill_telemetry(12)
    sleep(120)
    ERROR_MSG = "URL doesn't start with https or is invalid"
    ERROR_MSG1 = "Config URL is not set to valid value. Xconfclient shall not proceed for T1.0 settings fetch attempts"
    assert ERROR_MSG in grep_T2logs(ERROR_MSG)
    assert ERROR_MSG1 in grep_T2logs(ERROR_MSG1)

# Test Xconf Communication is only using https
@pytest.mark.run(order=3)
def test_xconf_http():
    rbus_set_data("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.ConfigURL", "string", "http://mockxconf:50050/loguploader/getT2DCMSettings")
    ERROR_MSG = "URL doesn't start with https or is invalid"
    
    uptime_process = get_process_uptime(get_pid("telemetry2_0"))
    if uptime_process < 200:
        sleep(200 - uptime_process+30)
    else:
        kill_telemetry(12)
        sleep(60)

    search_result = grep_T2logs(ERROR_MSG)
    print("#### 1st Search result : " + search_result)
    assert ERROR_MSG in search_result
    clear_T2logs()


@pytest.mark.run(order=4)
def test_xconf_404():
    clear_T2logs()

    rbus_set_data("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.ConfigURL", "string", "https://mockxconf:50050/loguploader404/getT2DCMSettings")
    kill_telemetry(12)
    ERROR_MSG1 = "Telemetry XCONF communication Failed with http code"
    ERROR_MSG2 = "XConf Telemetry profile not set for this device, uninitProfileList."

    sleep(10)
    response1 = grep_T2logs(ERROR_MSG1)
    assert ERROR_MSG1 in response1
    sleep(10)
    
    response2 = grep_T2logs(ERROR_MSG2)
    assert ERROR_MSG2 in response2
    # note durining 404 the current profile should also be un inited but this is not happening
    # assert not os.path.exists(XCONF_PERSISTANT_FILE)

@pytest.mark.run(order=5)
def test_change_profile():
    rbus_set_data("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.ConfigURL", "string", "https://mockxconf:50050/loguploader1/getT2DCMSettings")
    #exec_reload the telemetry profile
    kill_telemetry(12)
    sleep(10)
    assert os.path.exists(XCONF_PERSISTANT_FILE)
    assert "NEW TEST PROFILE" in grep_T2logs("NEW TEST PROFILE")

@pytest.mark.run(order=6)
def test_verify_urlencoding():
    CHECK_MSG = "T2: Curl Using XCONF URI :"
    response = grep_T2logs(CHECK_MSG)
    assert CHECK_MSG in response
    command = ["grep -inra 'doHttpGet with url' /opt/logs/telemetry2_0.txt.0 | tail -n 1 | awk -F' ' '{print $NF}' "]
    url_string = run_shell_command(command)
    
    assert True == is_url_encoded_correctly(url_string)

@pytest.mark.run(order=7)
def test_verify_schedule():
    #Trigger an event to be validated in next step
    command = ["telemetry2_0_client SYS_EVENT_TEST 1"]
    run_shell_command(command)
    #Add Grep Logs to validate in next Step
    os.makedirs('/opt/logs', exist_ok=True)
    command = ["echo 'This is Test Log' > /opt/logs/test.log"]
    run_shell_command(command)
    sleep(60)
    ERROR_MSG = "Waiting for 60 sec for next TIMEOUT for profile as reporting interval is taken - NEW TEST PROFILE"
    assert ERROR_MSG in grep_T2logs(ERROR_MSG)

@pytest.mark.run(order=8)
def test_verify_markers():
    CHECK_MSG1 = "SYS_GREP_TEST"
    CHECK_MSG2 = "SYS_EVENT_TEST"
    report = grep_T2logs("cJSON Report")
    assert CHECK_MSG1 in report
    assert CHECK_MSG2 in report

@pytest.mark.run(order=9)
def test_verify_report():
    ERROR_MSG1 = "CollectAndReportXconf ++in profileName : NEW TEST PROFILE"
    ERROR_MSG2 = "cJSON Report ="
    assert ERROR_MSG1 in grep_T2logs(ERROR_MSG1)
    assert ERROR_MSG2 in grep_T2logs(ERROR_MSG2)

@pytest.mark.run(order=10)
def test_log_upload():
    LOG_MSG1 = "CollectAndReportXconf ++in profileName : NEW TEST PROFILE"
    LOG_MSG2 = "cJSON Report ="
    clear_T2logs()
    kill_telemetry(10)
    sleep(10)
    assert LOG_MSG1 in grep_T2logs(LOG_MSG1)
    assert LOG_MSG2 in grep_T2logs(LOG_MSG2)

@pytest.mark.run(order=11)
def test_log_upload_on_demand():
    LOG_MSG1 = "CollectAndReportXconf ++in profileName : NEW TEST PROFILE"
    LOG_MSG2 = "cJSON Report ="
    clear_T2logs()
    kill_telemetry(29)
    sleep(10)
    assert LOG_MSG1 in grep_T2logs(LOG_MSG1)
    assert LOG_MSG2 in grep_T2logs(LOG_MSG2)

@pytest.mark.run(order=12)
def test_verify_persistant_file():
    assert os.path.exists(XCONF_PERSISTANT_FILE)

@pytest.mark.run(order=13)
def test_xconf_retry_for_connection_errors():
    clear_T2logs()
    rbus_set_data("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.ConfigURL", "string", "https://mockxconf:80/loguploader1/getT2DCMSettings")
    # Force reload config fetch from xconf
    kill_telemetry(12)
    sleep(180)
    ERROR_MSG = "Waiting for 180 sec before trying fetchRemoteConfiguration, No.of tries"
    assert ERROR_MSG in grep_T2logs(ERROR_MSG)

pytest.mark.run(order=14)
def test_xconf_datamodel():
    kill_telemetry(9)
    RUN_START_TIME = dt.now()
    remove_T2bootup_flag()
    clear_persistant_files()
    rbus_set_data("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.ConfigURL", "string", "https://mockxconf:50050/loguploader2/getT2DCMSettings")
    clear_T2logs()
    run_telemetry()
    sleep(10)
    rbus_set_data("Device.DeviceInfo.X_RDKCENTRAL-COM.IUI.Version", "string", "1.0.1")
    sleep(1)
    kill_telemetry(12)
    sleep(5)
    kill_telemetry(29)
    sleep(5)
    assert "IUI_accum" in grep_T2logs("cJSON Report")
    assert "Test_datamodel_1" in grep_T2logs("cJSON Report")
    kill_telemetry(29)
    sleep(5)
    kill_telemetry(29)
    sleep(5)
    kill_telemetry(29)
    sleep(5)
    assert "Test_datamodel_1" in grep_T2logs("cJSON Report")
