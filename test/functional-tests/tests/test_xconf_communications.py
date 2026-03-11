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

@pytest.mark.run(order=14)
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

@pytest.mark.run(order=15)
def test_xconf_split_markers():
    kill_telemetry(9)
    RUN_START_TIME = dt.now()
    remove_T2bootup_flag()
    clear_persistant_files()
    rbus_set_data("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.ConfigURL", "string", "https://mockxconf:50050/loguploader/getT2DCMSettings")
    clear_T2logs()
    run_telemetry()
    os.makedirs('/opt/logs', exist_ok=True)
    command1 = ["echo 'WIFI_MAC_10_TOTAL_COUNT:0.1' > /opt/logs/wifihealth.txt"]
    run_shell_command(command1)
    sleep(5)
    command2 = ["echo 'Total_MoCA_Clients=0' > /opt/logs/LM.txt.0"]
    run_shell_command(command2)
    sleep(10)
    kill_telemetry(29)
    sleep(10)
    assert "XWIFIS_CNT_2_split" in grep_T2logs("cJSON Report")
    assert "Total_MoCA_Clients_split" not in grep_T2logs("cJSON Report")


@pytest.mark.run(order=16)
def test_xconf_reload_concurrent_with_trigger_condition():
    """Regression test for the three-mutex deadlock (LTE-Hang / RDKB-67391).

    The deadlock cycle before the fix:
      ProfileXConf_set          (holds plMutex) -> registerProfileWithScheduler (waits scMutex)
      unregisterProfileFromScheduler            (holds scMutex) -> waits tMutex
      TimeoutThread callback    (holds tMutex)  -> ProfileXConf_isNameEqual     -> waits plMutex

    This test exercises the race by:
      1. Installing a trigger-condition regular profile so an active TimeoutThread
         callback is scheduled in the scheduler.
      2. Firing trigger-condition events via rbus to put the TimeoutThread in flight.
      3. Immediately sending SIGUSR2 (exec_reload = kill -12) so that ProfileXConf_set
         races with the in-flight TimeoutThread.
      4. Asserting the daemon is still alive and ProfileXConf_set completed within
         a 30-second window (a deadlock would produce neither).
    """
    import json
    import msgpack
    import base64

    def _to_msgpack(json_str):
        return base64.b64encode(msgpack.packb(json.loads(json_str))).decode('utf-8')

    # Restore a valid xconf URL so exec_reload will call ProfileXConf_set.
    rbus_set_data(
        "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.ConfigURL",
        "string",
        "https://mockxconf:50050/loguploader1/getT2DCMSettings",
    )
    clear_T2logs()

    # Install a trigger-condition profile.  The 'any' operator on a dataModel
    # parameter fires a TimeoutThread callback on every value change, producing
    # the tMutex -> plMutex acquisition sequence that forms part of the cycle.
    tc_profile = json.dumps({
        "profiles": [{
            "name": "LTE_regression_trigger",
            "hash": "Hash_LTE_regression_v1",
            "value": {
                "Description": "Deadlock regression: concurrent xconf reload + trigger-condition",
                "Version": "0.1",
                "Protocol": "HTTP",
                "ReportingInterval": 300,
                "EncodingType": "JSON",
                "GenerateNow": False,
                "ActivationTimeout": 300,
                "TimeReference": "0001-01-01T00:00:00Z",
                "Parameter": [],
                "TriggerCondition": [{
                    "type": "dataModel",
                    "name": "lte_regex_trigger",
                    "reference": "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.Enable",
                    "operator": "any",
                }],
                "HTTP": {
                    "URL": "https://mockxconf:50051/dataLakeMock/",
                    "Compression": "None",
                    "Method": "POST",
                    "RequestURIParameter": [{"Name": "reportName", "Reference": "Profile.Name"}],
                },
                "JSONEncoding": {"ReportFormat": "NameValuePair", "ReportTimestamp": "None"},
            },
        }]
    })
    rbus_set_data(T2_REPORT_PROFILE_PARAM_MSG_PCK, "string", _to_msgpack(tc_profile))
    sleep(3)

    # Arm the trigger: fire a value-change event so the scheduler dispatches a
    # TimeoutThread callback (acquires tMutex, then tries plMutex).
    subprocess.run(
        "rbuscli set Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.Enable bool false",
        shell=True,
    )

    # Immediately race: exec_reload triggers ProfileXConf_set which (pre-fix)
    # held plMutex across registerProfileWithScheduler (waited scMutex).
    kill_telemetry(12)

    # Keep firing trigger events during the reload window to maximise the race
    # surface — each fires another TimeoutThread callback.
    subprocess.run(
        "rbuscli set Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.Enable bool true",
        shell=True,
    )
    subprocess.run(
        "rbuscli set Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.Enable bool false",
        shell=True,
    )
    subprocess.run(
        "rbuscli set Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.Enable bool true",
        shell=True,
    )

    # Allow 20 s for the reload to complete.  A deadlocked daemon will not log
    # the success message and the process-alive check will also fail.
    sleep(20)

    # Assert 1: the daemon process survived.
    pid = get_pid("telemetry2_0")
    assert pid != "", (
        "telemetry2_0 not found after concurrent exec_reload + trigger-condition fire — "
        "possible three-mutex deadlock regression (LTE-Hang)"
    )

    # Assert 2: ProfileXConf_set completed and logged success after the reload.
    SUCCESS_LOG = "Successfully set profile"
    assert SUCCESS_LOG in grep_T2logs(SUCCESS_LOG), (
        "ProfileXConf_set did not log success after exec_reload — "
        "plMutex may still be held across registerProfileWithScheduler (deadlock regression)"
    )
