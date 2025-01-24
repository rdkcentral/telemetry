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

RUN_START_TIME = None

#check important persistant locations
def check_BootUp_flags(Exitstence=True):
    """
    Exitstence = "should be bool, TRUE to check if file should exist and FALSE to not exist"
    """
    #check the T2 Boot ready flag
    assert not (Exitstence ^ os.path.exists(BOOTUP_FLAG))
    #check the T2 configuration ready flag
    #assert not (Exitstence ^ os.path.exists(T2_CONFIG_READY))
    #check the T2 Events ready flag
    assert not (Exitstence ^ os.path.exists(T2_READY_TO_RECIVE)) 


# Run the telemetrt deamon and check the communication with the Xconf with all the parameters
@pytest.mark.run(order=1)
def test_boot_sequence():
    adminSupport_cache(False)
    kill_telemetry(9)
    rbus_set_data("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.ConfigURL", "string", "https://mockxconf:50050/loguploader/getT2DCMSettings")
    clear_persistant_files()
    remove_T2bootup_flag()
    adminSupport_cache(True)
    assert get_pid("telemetry2_0") == ""
    RUN_START_TIME = dt.now()
    run_telemetry()
    assert get_pid("telemetry2_0") != ""
    sleep(65)
    
# Check the Persistant xconf response
@pytest.mark.run(order=2)
def test_persistant_data():
    assert os.path.exists(XCONF_PERSISTANT_FILE)

# Check the important flags are set when telemetry has started
@pytest.mark.run(order=3)
def test_Bootup_Flags():
    check_BootUp_flags()

# Check the Rbus Interfaces are working
@pytest.mark.run(order=4)
def test_rbus_data():
    check_Rbus_data()

# Test Xconf Parameters
@pytest.mark.run(order=5)
def test_xconf_request():
    return_json = adminSupport_requestData()
    assert len(return_json) > 0
    verify_data = return_json[list(return_json.keys())[0]]
    assert verify_data["estbMacAddress"] == "AA:BB:CC:DD:EE:FF"
    assert verify_data["firmwareVersion"] == "Platform_Cotainer_1.0.0"
    assert verify_data["model"] == "DOCKER"
    assert verify_data["partnerId"] == "global"
    assert verify_data["accountId"] == "Platform_Container_Test_DEVICE"

# Check the exec reload is working
@pytest.mark.run(order=6)
def test_exec_reload():
    # Sample response:
    #{"2024-10-22T18:14:49.240Z":{"estbMacAddress":"AA:BB:CC:DD:EE:FF","firmwareVersion":"Platform_Cotainer_1.0.0","model":"DOCKER","partnerId":"global","accountId":"Platform_Container_Test_DEVICE","ecmMacAddress":"10.0.0.1","env":"PROD","controllerId":"2504","channelMapId":"2345","vodId":"15660","timezone":"US/Mountain","version":"2"}}
    return_json = adminSupport_requestData()
    current_request_count = len(return_json)
    RUN_START_TIME = dt.now()
    print(kill_telemetry(12))
    sleep(10)
    return_json= adminSupport_requestData()
    assert len(return_json) > current_request_count
    #check if the confing has been save to the persistant
    last_modified_time = os.path.getmtime(XCONF_PERSISTANT_FILE)
    assert last_modified_time > RUN_START_TIME.timestamp()

# Test SIGTERM working scenario
@pytest.mark.run(order=7)
def test_Terminal_signal():
    #sigterm_telemetry(get_pid("telemetry2_0"))
    print(kill_telemetry(15))
    sleep(30)
    #There is an issue single sigterm is not working
    print(kill_telemetry(15))
    sleep(3)
    #check_BootUp_flags(False)

# Test telemetry2_0 has exited
@pytest.mark.run(order=8)
def test_Telemetry_exit():
    pid = subprocess.run("pidof telemetry2_0", shell=True, capture_output=True, text=True)
    assert "" == pid.stdout.strip()
