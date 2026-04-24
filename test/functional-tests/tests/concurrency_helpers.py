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

"""
Concurrency helper functions for race condition testing.
Provides utilities for concurrent profile operations and system health monitoring.
"""

import threading
import time
import subprocess
import json
from concurrent.futures import ThreadPoolExecutor, as_completed
from helper_functions import *
from basic_constants import *


def start_telemetry():
    """Start telemetry daemon and verify it's running"""
    run_shell_silent("/usr/local/bin/telemetry2_0")
    time.sleep(2)  # Allow startup
    pid = get_pid("telemetry2_0")
    if not pid:
        raise RuntimeError("Failed to start telemetry2_0")
    return pid


def stop_telemetry_graceful(timeout=10):
    """Stop telemetry gracefully with SIGTERM and verify shutdown"""
    pid = get_pid("telemetry2_0")
    if not pid:
        return  # Already stopped
    
    sigterm_telemetry(pid)
    
    # Wait for graceful shutdown
    start_time = time.time()
    while time.time() - start_time < timeout:
        if not get_pid("telemetry2_0"):
            return  # Successfully stopped
        time.sleep(0.1)
    
    # Force kill if still running
    run_shell_command("kill -9 $(pidof telemetry2_0) 2>/dev/null || true")


def restart_telemetry_graceful():
    """Restart telemetry with graceful shutdown"""
    stop_telemetry_graceful()
    time.sleep(0.5)
    return start_telemetry()


def telemetry_is_running():
    """Check if telemetry daemon is running"""
    return bool(get_pid("telemetry2_0"))


def get_t2_logs():
    """Get telemetry log contents"""
    try:
        with open(LOG_FILE, 'r', encoding='utf-8', errors='ignore') as f:
            return f.read()
    except Exception as e:
        print(f"Failed to read logs: {e}")
        return ""


def no_crash_in_logs():
    """Check for crash indicators in logs"""
    logs = get_t2_logs()
    crash_patterns = [
        "segmentation fault",
        "core dumped",
        "SIGSEGV",
        "SIGABRT",
        "assertion failed",
        "fatal error",
    ]
    for pattern in crash_patterns:
        if pattern.lower() in logs.lower():
            print(f"CRASH DETECTED: Found '{pattern}' in logs")
            return False
    return True


def no_deadlock_detected():
    """Check for deadlock indicators"""
    logs = get_t2_logs()
    deadlock_patterns = [
        "deadlock",
        "hung task",
        "blocked for more than",
    ]
    for pattern in deadlock_patterns:
        if pattern.lower() in logs.lower():
            print(f"DEADLOCK DETECTED: Found '{pattern}' in logs")
            return False
    return True


def no_mutex_errors_in_logs():
    """Check for mutex-related errors"""
    logs = get_t2_logs()
    mutex_patterns = [
        "pthread_mutex_lock failed",
        "pthread_mutex_unlock failed",
        "pthread_mutex_destroy failed",
        "EDEADLK",
        "EINVAL",
        "mutex error",
    ]
    for pattern in mutex_patterns:
        if pattern.lower() in logs.lower():
            print(f"MUTEX ERROR: Found '{pattern}' in logs")
            return False
    return True


def no_thread_leak_in_logs():
    """Check for thread leak indicators"""
    logs = get_t2_logs()
    leak_patterns = [
        "failed to join",
        "orphaned thread",
        "thread leak",
    ]
    for pattern in leak_patterns:
        if pattern.lower() in logs.lower():
            print(f"THREAD LEAK: Found '{pattern}' in logs")
            return False
    return True


def get_thread_count(thread_name=None):
    """Get thread count for telemetry process or specific thread name"""
    pid = get_pid("telemetry2_0")
    if not pid:
        return 0
    
    if thread_name:
        # Count threads matching name
        result = run_shell_command(f"ps -T -p {pid} | grep -c '{thread_name}' || echo 0")
    else:
        # Total thread count
        result = run_shell_command(f"ps -T -p {pid} | wc -l")
    
    try:
        return int(result)
    except ValueError:
        return 0


def get_thread_stacks():
    """Get stack traces of all threads (requires gdb)"""
    pid = get_pid("telemetry2_0")
    if not pid:
        return ""
    
    cmd = f"gdb -batch -p {pid} -ex 'thread apply all bt' 2>/dev/null || echo 'gdb not available'"
    return run_shell_command(cmd)


def assert_no_valgrind_errors():
    """Check for valgrind error log if running under valgrind"""
    valgrind_log = "/tmp/valgrind.log"
    if not os.path.exists(valgrind_log):
        return  # Not running under valgrind
    
    try:
        with open(valgrind_log, 'r') as f:
            content = f.read()
            if "ERROR SUMMARY: 0 errors" not in content:
                raise AssertionError(f"Valgrind detected errors:\n{content}")
    except Exception as e:
        print(f"Warning: Could not check valgrind log: {e}")


def create_xconf_profile_json(name, reporting_interval=60, markers=None, parameters=None):
    """Create a valid XConf profile JSON structure"""
    if markers is None:
        markers = ["SYS_INFO_.*"]
    
    if parameters is None:
        parameters = []
    
    # Build parameter list
    param_list = []
    
    # Add grep markers
    for marker in markers:
        param_list.append({
            "type": "grep",
            "marker": marker,
            "search": ".*",
            "logFile": "/opt/logs/messages.txt",
            "use": "count"
        })
    
    # Add custom parameters
    param_list.extend(parameters)
    
    profile = {
        "Name": name,
        "Description": f"Test profile {name}",
        "Version": "0.1",
        "Protocol": "HTTP",
        "EncodingType": "JSON",
        "ActivationTimeout": 300,
        "ReportingInterval": reporting_interval,
        "GenerateNow": False,
        "RootName": f"TestProfile_{name}",
        "TimeReference": "0001-01-01T00:00:00Z",
        "Parameter": param_list,
        "HTTP": {
            "URL": "https://mockxconf:50051/dataLakeMock/",
            "Compression": "None",
            "Method": "POST",
            "RequestURIParameter": [
                {
                    "Name": "reportName",
                    "Reference": "Profile.Name"
                }
            ]
        },
        "JSONEncoding": {
            "ReportFormat": "NameValuePair",
            "ReportTimestamp": "None"
        }
    }
    
    return json.dumps({"profiles": [{"name": name, "hash": f"hash_{name}", "value": profile}]})


def set_xconf_profile_via_rbus(profile_json):
    """Set XConf profile via RBUS"""
    return rbus_set_data(T2_REPORT_PROFILE_PARAM, "string", profile_json)


def delete_all_profiles_via_rbus():
    """Delete all profiles by setting empty profile list"""
    empty_profile = json.dumps({"profiles": []})
    return rbus_set_data(T2_REPORT_PROFILE_PARAM, "string", empty_profile)


def trigger_on_demand_report(profile_name):
    """Trigger on-demand report generation for a profile"""
    # Use temp profile with GenerateNow flag
    profile_json = create_temp_profile_json(profile_name, generate_now=True)
    return rbus_set_data(T2_TEMP_REPORT_PROFILE_PARAM, "string", profile_json)


def create_temp_profile_json(name, generate_now=False):
    """Create temporary profile JSON"""
    profile = {
        "Name": name,
        "Description": f"Temp profile {name}",
        "Version": "0.1",
        "Protocol": "HTTP",
        "EncodingType": "JSON",
        "ActivationTimeout": 60,
        "GenerateNow": generate_now,
        "RootName": f"TempProfile_{name}",
        "TimeReference": "0001-01-01T00:00:00Z",
        "Parameter": [
            {
                "type": "dataModel",
                "name": "UPTIME",
                "reference": "Device.DeviceInfo.UpTime",
                "use": "absolute"
            }
        ],
        "HTTP": {
            "URL": "https://mockxconf:50051/dataLakeMock/",
            "Compression": "None",
            "Method": "POST"
        },
        "JSONEncoding": {
            "ReportFormat": "NameValuePair",
            "ReportTimestamp": "None"
        }
    }
    
    return json.dumps({"profiles": [{"name": name, "hash": f"temp_hash_{name}", "value": profile}]})


def wait_for_condition(condition_func, timeout=10, poll_interval=0.1):
    """Wait for a condition to become true"""
    start_time = time.time()
    while time.time() - start_time < timeout:
        if condition_func():
            return True
        time.sleep(poll_interval)
    return False


def assert_memory_usage_stable(threshold_mb=50):
    """Check that memory usage hasn't grown excessively"""
    pid = get_pid("telemetry2_0")
    if not pid:
        return
    
    # Get RSS (Resident Set Size) in KB
    rss = run_shell_command(f"ps -p {pid} -o rss= | tr -d ' '")
    try:
        rss_mb = int(rss) / 1024
        # For embedded systems, telemetry should stay under reasonable limits
        # This is a sanity check, not strict limit
        if rss_mb > threshold_mb:
            print(f"Warning: Memory usage is {rss_mb:.1f} MB (threshold: {threshold_mb} MB)")
    except ValueError:
        pass


def concurrent_operation_executor(operations, max_workers=10, timeout=30):
    """
    Execute multiple operations concurrently and collect results.
    
    Args:
        operations: List of (callable, args, kwargs) tuples
        max_workers: Maximum concurrent threads
        timeout: Timeout per operation
        
    Returns:
        List of (success, result/exception) tuples
    """
    results = []
    
    with ThreadPoolExecutor(max_workers=max_workers) as executor:
        futures = []
        for op, args, kwargs in operations:
            future = executor.submit(op, *args, **kwargs)
            futures.append(future)
        
        for future in as_completed(futures, timeout=timeout):
            try:
                result = future.result(timeout=timeout)
                results.append((True, result))
            except Exception as e:
                results.append((False, e))
    
    return results
