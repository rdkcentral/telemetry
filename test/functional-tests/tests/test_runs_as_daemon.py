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
from time import sleep

# TODO Helper functions to be moved to common utils class
def run_shell_silent(command):
    subprocess.run(command, shell=True, capture_output=False, text=False)
    return 

def run_shell_command(command):
    result = subprocess.run(command, shell=True, capture_output=True, text=True)
    return result.stdout.strip()

# End of Helper functions

def test_check_telemetry_is_starting():
    print("Starting telemetry process")
    command_to_start = "/usr/local/bin/telemetry2_0"
    run_shell_silent(command_to_start)
    command_to_get_pid = "pidof telemetry2_0"
    pid = run_shell_command(command_to_get_pid)
    assert pid != ""

def test_second_telemetry_instance_is_not_started():
    command_to_get_pid = "pidof telemetry2_0"
    pid1 = run_shell_command(command_to_get_pid)

    command_to_start = "/usr/local/bin/telemetry2_0"
    run_shell_silent(command_to_start)
    sleep(2)
    pid2 = run_shell_command(command_to_get_pid)
    assert pid1 == pid2


def test_tear_down():
    command_to_stop = "kill -9 `pidof telemetry2_0`"
    run_shell_command(command_to_stop)
    command_to_get_pid = "pidof telemetry2_0"
    pid = run_shell_command(command_to_get_pid)
    assert pid == ""
