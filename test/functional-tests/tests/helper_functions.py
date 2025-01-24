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
from basic_constants import *
import os
import time
import re
import signal


def run_telemetry():
    return subprocess.run("/usr/local/bin/telemetry2_0", shell=True)

def kill_telemetry(signal: int=9):
    print(f"Recived Signal to kill telemetry2_0 {signal} with pid {get_pid('telemetry2_0')}")
    resp = subprocess.run(f"kill -{signal} {get_pid('telemetry2_0')}", shell=True, capture_output=True)
    print(resp.stdout.decode('utf-8'))
    print(resp.stderr.decode('utf-8'))
    return ""
"""
def sigterm_telemetry(pid:str):
    resp = subprocess.run(f"echo 'this is a test'; kill -15 `pidof telemetry2_0`; echo $?; echo !!", shell=True, capture_output=True)
    print(resp.stdout.decode('utf-8'))
    print(resp.stderr.decode('utf-8'))
    print(get_pid("telemetry2_0"))
    cmd=["kill","-15",str(pid)]
    print(f"Running command: {' '.join(cmd)}")
    result = subprocess.run(cmd, capture_output=True, text=True)
    print("Output:", result.stdout)
    print("Error:", result.stderr)
"""

def sigterm_telemetry(pid):
    try:
        # Print the command being executed
        print(f"Sending SIGTERM to process {pid}")
        
        # Send the SIGTERM signal to the process
        os.kill(int(pid), signal.SIGTERM)
        
        print(f"Process {pid} has been terminated.")
    except ProcessLookupError:
        print(f"No process with PID {pid} found.")
    except PermissionError:
        print(f"Permission denied to kill process {pid}.")
    except Exception as e:
        print(f"An error occurred: {e}")


def adminSupport_cache(save: bool = True):
    if(requests.get(ADMIN_SUPPORT_SET, verify=False,params={ADMIN_CACHE_ARG: str(save).lower()})):
        return True
    return False

def adminSupport_requestData():
    return_data = requests.get(ADMIN_SUPPORT_GET, verify=False,params={ADMIN_RQUEST_DATA: "true"})
    try:
        return_json = return_data.json()
    except:
        return_json = []
    return return_json

def clear_persistant_files():
    subprocess.run("rm -rf "+XCONF_PERSISTANT_REGX, shell=True)

def rbus_get_data(param: str):
    return subprocess.run(RBUSCLI_GET_CMD + param, shell=True, capture_output=True).stdout.decode('utf-8')

def rbus_set_data(param: str, type:str, value: str):
    return subprocess.run(f"{RBUSCLI_SET_CMD} {param} {type} {value}", shell=True, capture_output=True).stdout.decode('utf-8')

def get_process_uptime(pid):
    return time.time() - os.path.getctime(f"/proc/{pid}")

def get_pid(name: str):
    return subprocess.run(f"pidof {name}", shell=True, capture_output=True).stdout.decode('utf-8').strip()

"""
def grep_T2logs(search: str):
    search_escaped = search.replace("'", "\'")
    search_quoted=f'"{search_escaped}"'
    return subprocess.run(f"cat {LOG_FILE} | grep -i {search_quoted} ", shell=True, capture_output=True).stdout.decode('utf-8')
"""

def clear_T2logs():
    subprocess.run(f"echo '' > {LOG_FILE}", shell=True)

def grep_log_file(search: str, log_file: str):
    return subprocess.run(f"grep -i {search} {log_file}", shell=True, capture_output=True).stdout.decode('utf-8')

def remove_T2bootup_flag():
    FILE_LIST = BOOT_FLAGS_LIST
    for file in FILE_LIST:
        try:
            os.remove(file)
        except:
            pass

def grep_T2logs(search: str):
    search_result = ""
    search_pattern = re.compile(re.escape(search), re.IGNORECASE)
    try:
        with open(LOG_FILE, 'r', encoding='utf-8', errors='ignore') as file:
            for line_number, line in enumerate(file, start=1):
                if search_pattern.search(line):
                    search_result = search_result + " \n" + line
    except Exception as e:
        print(f"Could not read file {LOG_FILE}: {e}")
    return search_result

def check_Rbus_data():
    result = subprocess.run(RBUSCLI_GET_CMD+T2_REPORT_PROFILE_PARAM, shell=True, capture_output=True, text=True)
    assert RBUS_EXCEPTION_STRING not in result.stdout
    result = subprocess.run(RBUSCLI_GET_CMD+T2_REPORT_PROFILE_PARAM_MSG_PCK, shell=True, capture_output=True, text=True)
    assert RBUS_EXCEPTION_STRING not in result.stdout
    result = subprocess.run(RBUSCLI_GET_CMD+T2_TEMP_REPORT_PROFILE_PARAM, shell=True, capture_output=True, text=True)
    assert RBUS_EXCEPTION_STRING not in result.stdout

def adminSupport_DLXconf_requestData():
    return_data = requests.get(ADMIN_SUPPORT_URL, verify=False,params={ADMIN_RQUEST_DATA: "true"})
    try:
        return_json = return_data.json()
    except:
        return_json = []
    return return_json

def adminSupport_DLReport_requestData():
    return_data = requests.get(ADMIN_SUPPORT_URL, verify=False,params={ADMIN_RQUEST_DATA: "true"})
    try:
        return_json = return_data.json()
    except:
        return_json = []
    return return_json

def adminSupport_DLXconf_cache(save:bool = True):
    if(requests.get(DL_ADMIN_URL +"Xconf", verify=False,params={ADMIN_CACHE_ARG: str(save).lower()})):
        return True
    return False

def adminSupport_DLReport_cache(save:bool = True):
    if(requests.get(DL_ADMIN_URL +"Report", verify=False,params={ADMIN_CACHE_ARG: str(save).lower()})):
        return True
    return False

def adminSupport_DLXconf_requestData():
    return_data = requests.get(DL_ADMIN_URL+"Xconf", verify=False,params={ADMIN_RQUEST_DATA: "true"})
    try:
        return_json = return_data.json()
    except:
        return_json = []
    return return_json

def adminSupport_DLReport_requestData():
    return_data = requests.get(DL_ADMIN_URL+"Report", verify=False,params={ADMIN_RQUEST_DATA: "true"})
    try:
        return_json = return_data.json()
    except:
        return_json = []
    return return_json
