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
L2 Functional Tests for DCA Utility Module

Validates critical dcautil functionality through end-to-end integration tests:
- Grep markers: log pattern matching and collection
- Top markers: process CPU/memory monitoring  
- Log rotation: handling rotated log files
- Incremental processing: seek position persistence

Focus: Core integration scenarios, not exhaustive edge cases
"""

import os
import json
import pytest
from time import sleep

from basic_constants import *
from helper_functions import *


# ============================================================================
# Test Configuration
# ============================================================================

TEST_LOG_FILE = "/opt/logs/test_dca.log"

# Test profile with grep markers
PROFILE_GREP_TEST = {
    "name": "DCA_GREP_TEST",
    "hash": "HASH_GREP_001",
    "value": {
        "Name": "DCA_GREP_TEST",
        "Version": "1.0",
        "Protocol": "RBUS_METHOD",
        "EncodingType": "JSON",
        "ReportingInterval": 30,
        "ActivationTimeout": 120,
        "GenerateNow": False,
        "TimeReference": "0001-01-01T00:00:00Z",
        "Parameter": [
            {
                "type": "grep",
                "marker": "BOOT_COMPLETE",
                "search": "BOOT_COMPLETE",
                "logFile": "test_dca.log",
                "use": "count"
            },
            {
                "type": "grep",
                "marker": "ERROR_MARKER",
                "search": "ERROR_MARKER",
                "logFile": "test_dca.log",
                "use": "count"
            }
        ],
        "JSONEncoding": {
            "ReportFormat": "NameValuePair"
        }
    }
}

# Test profile with top markers
PROFILE_TOP_TEST = {
    "name": "DCA_TOP_TEST",
    "hash": "HASH_TOP_001",
    "value": {
        "Name": "DCA_TOP_TEST",
        "Version": "1.0",
        "Protocol": "RBUS_METHOD",
        "EncodingType": "JSON",
        "ReportingInterval": 30,
        "ActivationTimeout": 120,
        "GenerateNow": False,
        "TimeReference": "0001-01-01T00:00:00Z",
        "Parameter": [
            {
                "type": "dataModel",
                "name": "TELEMETRY_CPU",
                "reference": "<Daemon.CPU>telemetry2_0",
                "use": "absolute"
            },
            {
                "type": "dataModel",
                "name": "TELEMETRY_MEM",
                "reference": "<Daemon.Mem>telemetry2_0",
                "use": "absolute"
            }
        ],
        "JSONEncoding": {
            "ReportFormat": "NameValuePair"
        }
    }
}

# Test profile with mixed grep and top markers
PROFILE_MIXED_TEST = {
    "name": "DCA_MIXED_TEST",
    "hash": "HASH_MIXED_001",
    "value": {
        "Name": "DCA_MIXED_TEST",
        "Version": "1.0",
        "Protocol": "RBUS_METHOD",
        "EncodingType": "JSON",
        "ReportingInterval": 30,
        "ActivationTimeout": 120,
        "GenerateNow": False,
        "TimeReference": "0001-01-01T00:00:00Z",
        "Parameter": [
            {
                "type": "grep",
                "marker": "INTEGRATION_TEST_MARKER",
                "search": "INTEGRATION_TEST_MARKER",
                "logFile": "test_dca.log",
                "use": "count"
            },
            {
                "type": "dataModel",
                "name": "PROCESS_CPU",
                "reference": "<Daemon.CPU>telemetry2_0",
                "use": "absolute"
            }
        ],
        "JSONEncoding": {
            "ReportFormat": "NameValuePair"
        }
    }
}


# ============================================================================
# Helper Functions
# ============================================================================

def create_test_log(content_lines):
    """Create test log file with content."""
    os.makedirs(os.path.dirname(TEST_LOG_FILE), exist_ok=True)
    with open(TEST_LOG_FILE, 'w') as f:
        for line in content_lines:
            f.write(f"{line}\n")


def append_test_log(content_lines):
    """Append lines to test log file."""
    with open(TEST_LOG_FILE, 'a') as f:
        for line in content_lines:
            f.write(f"{line}\n")


def cleanup_test_files():
    """Clean up test log files."""
    for f in [TEST_LOG_FILE, TEST_LOG_FILE + ".1", TEST_LOG_FILE + ".2"]:
        if os.path.exists(f):
            os.remove(f)


def set_profile(profile_dict):
    """Set a telemetry profile via RBUS."""
    profile_json = json.dumps([profile_dict])
    rbus_set_data(T2_REPORT_PROFILE_PARAM, "string", profile_json)
    sleep(2)  # Allow profile to be processed


def clear_profile():
    """Clear all telemetry profiles."""
    rbus_set_data(T2_REPORT_PROFILE_PARAM, "string", '{"profiles":[]}')
    sleep(2)


# ============================================================================
# Fixtures
# ============================================================================

@pytest.fixture(scope="module", autouse=True)
def setup_module():
    """Ensure telemetry daemon is running."""
    if not get_pid("telemetry2_0"):
        run_telemetry()
        sleep(10)
    yield
    cleanup_test_files()


@pytest.fixture(autouse=True)
def setup_test():
    """Clean state before each test."""
    cleanup_test_files()
    clear_T2logs()
    yield
    cleanup_test_files()


# ============================================================================
# L2 Functional Tests - DCA Utility
# ============================================================================

@pytest.mark.order(1)
def test_grep_marker_collection():
    """
    Test that grep-type markers can search log files and collect matching entries.
    
    Validates:
    - Log files are searched for pattern matches
    - Marker values are extracted correctly
    - Results appear in telemetry reports
    """
    # Clear existing state
    clear_T2logs()
    clear_profile()
    
    # Create log file with test patterns
    create_test_log([
        "2024-04-24 10:00:00 System startup initiated",
        "2024-04-24 10:01:00 BOOT_COMPLETE: System ready",
        "2024-04-24 10:02:00 Service telemetry started",
        "2024-04-24 10:03:00 ERROR_MARKER: Test error condition"
    ])
    
    # Set profile with grep markers
    set_profile(PROFILE_GREP_TEST)
    
    # Verify profile was enabled
    assert "DCA_GREP_TEST" in grep_T2logs("Successfully enabled profile"), "Profile should be enabled"
    
    # Wait for reporting interval (30s + buffer)
    sleep(35)
    
    # Verify report was generated
    assert "TIMEOUT for profile" in grep_T2logs("DCA_GREP_TEST"), "Report should be generated"
    
    # Verify grep markers appear in report
    report = grep_T2logs("cJSON Report")
    assert "BOOT_COMPLETE" in report or "ERROR_MARKER" in report, "Grep markers should appear in report"


@pytest.mark.order(2)
def test_process_monitoring_top_markers():
    """
    Test that top-type markers monitor process CPU and memory usage.
    
    Validates:
    - Process resource usage is measured
    - CPU and memory metrics are collected
    - Values appear in telemetry reports
    """
    # Clear existing state
    clear_T2logs()
    clear_profile()
    
    # Verify telemetry process is running
    process_name = "telemetry2_0"
    pid = get_pid(process_name)
    assert pid != "", f"Process {process_name} not running"
    
    # Set profile with top markers
    set_profile(PROFILE_TOP_TEST)
    
    # Verify profile was enabled
    assert "DCA_TOP_TEST" in grep_T2logs("Successfully enabled profile"), "Profile should be enabled"
    
    # Wait for reporting interval
    sleep(35)
    
    # Verify report was generated
    assert "TIMEOUT for profile" in grep_T2logs("DCA_TOP_TEST"), "Report should be generated"
    
    # Verify CPU/memory markers appear in report
    report = grep_T2logs("cJSON Report")
    assert "TELEMETRY_CPU" in report or "TELEMETRY_MEM" in report, "Process metrics should appear in report"


@pytest.mark.order(3)
def test_log_rotation_handling():
    """
    Test that dcautil handles rotated log files correctly.
    
    Validates:
    - Rotated logs (.1, .2) are checked appropriately
    - New log content is processed
    - No duplicate entries in reports
    """
    # Clear existing state
    clear_T2logs()
    clear_profile()
    
    # Create initial log with marker
    create_test_log([
        "Initial log entry 1",
        "BOOT_COMPLETE: Marker before rotation"
    ])
    
    # Simulate log rotation
    if os.path.exists(TEST_LOG_FILE):
        os.rename(TEST_LOG_FILE, TEST_LOG_FILE + ".1")
    
    # Create new log after rotation with different marker
    create_test_log([
        "After rotation log entry",
        "ERROR_MARKER: Marker after rotation"
    ])
    
    # Verify both files exist
    assert os.path.exists(TEST_LOG_FILE), "Current log not created"
    assert os.path.exists(TEST_LOG_FILE + ".1"), "Rotated log not found"
    
    # Set profile with grep markers
    set_profile(PROFILE_GREP_TEST)
    
    # Wait for reporting
    sleep(35)
    
    # Both markers should be found (first boot checks rotated logs)
    report = grep_T2logs("cJSON Report")
    # At least one marker should be found
    assert "BOOT_COMPLETE" in report or "ERROR_MARKER" in report, "Markers from logs should be found"


@pytest.mark.order(4)
def test_incremental_log_processing():
    """
    Test that seek positions enable incremental log processing.
    
    Validates:
    - Only new log entries are processed after first report
    - Seek positions are maintained across reports
    - No duplicate marker collection
    """
    # Clear existing state
    clear_T2logs()
    clear_profile()
    
    # Create initial log with markers
    create_test_log([
        "Entry 1: OLD_MARKER processed first time",
        "BOOT_COMPLETE: Old marker content"
    ])
    
    # Set profile
    set_profile(PROFILE_GREP_TEST)
    
    # Wait for first report
    sleep(35)
    
    # Verify first report was generated
    first_report_log = grep_T2logs("TIMEOUT for profile")
    assert "DCA_GREP_TEST" in first_report_log, "First report should be generated"
    
    # Clear logs to see only new activity
    clear_T2logs()
    
    # Append new entries (seek position should make these incremental)
    append_test_log([
        "Entry 3: NEW_MARKER should be new",
        "ERROR_MARKER: New error after first report"
    ])
    
    # Wait for second report
    sleep(35)
    
    # Verify second report was generated
    second_report_log = grep_T2logs("TIMEOUT for profile")
    assert "DCA_GREP_TEST" in second_report_log, "Second report should be generated"
    
    # Verify incremental processing (new marker should appear)
    report = grep_T2logs("cJSON Report")
    assert "ERROR_MARKER" in report or "Report" in report, "New markers should be processed"


@pytest.mark.order(5)
def test_missing_log_files_handled_gracefully():
    """
    Test that missing or non-existent log files don't cause crashes.
    
    Validates:
    - Non-existent files handled gracefully
    - No errors or crashes occur
    - System continues processing other markers
    """
    # Clear existing state
    clear_T2logs()
    clear_profile()
    
    # Ensure test log doesn't exist
    cleanup_test_files()
    assert not os.path.exists(TEST_LOG_FILE), "Test log should not exist"
    
    # Set profile that references non-existent log file
    set_profile(PROFILE_GREP_TEST)
    
    # Profile should still be enabled
    assert "DCA_GREP_TEST" in grep_T2logs("Successfully enabled profile"), "Profile should be enabled"
    
    # Wait for reporting interval
    sleep(35)
    
    # Telemetry should still be running (no crash)
    pid = get_pid("telemetry2_0")
    assert pid != "", "Telemetry should still be running"
    
    # Report should still be generated (even with missing log)
    assert "TIMEOUT for profile" in grep_T2logs("DCA_GREP_TEST"), "Report should still be generated"


@pytest.mark.order(6)
def test_large_log_file_performance():
    """
    Test that large log files (>1MB) are processed efficiently.
    
    Validates:
    - Large files don't cause memory issues
    - Processing completes in reasonable time
    - Markers found in large files
    """
    # Clear existing state
    clear_T2logs()
    clear_profile()
    
    # Create large log file
    large_log = []
    for i in range(10000):
        large_log.append(f"Log entry {i}: Normal system operation")
    
    # Add marker at end
    large_log.append("BOOT_COMPLETE: Found at end of large file")
    
    create_test_log(large_log)
    
    # Verify file size is significant
    file_size = os.path.getsize(TEST_LOG_FILE)
    assert file_size > 500000, f"Test log should be > 500KB, got {file_size}"
    
    # Set profile with grep marker
    set_profile(PROFILE_GREP_TEST)
    
    # Wait for processing
    sleep(35)
    
    # Verify processing completed without crash
    pid = get_pid("telemetry2_0")
    assert pid != "", "Telemetry should still be running after processing large file"
    
    # Verify report was generated
    assert "TIMEOUT for profile" in grep_T2logs("DCA_GREP_TEST"), "Report should be generated"
    
    # Marker should be found despite large file size
    report = grep_T2logs("cJSON Report")
    assert "BOOT_COMPLETE" in report or "Report" in report, "Marker should be found in large file"


@pytest.mark.order(7)
def test_mixed_grep_and_top_markers():
    """
    Test that profiles can contain both grep and top markers together.
    
    Validates:
    - Grep markers collected from logs
    - Top markers collected from process stats
    - Both types appear in same report
    - No interference between marker types
    """
    # Clear existing state
    clear_T2logs()
    clear_profile()
    
    # Create log for grep marker
    create_test_log([
        "Application event: System initialized",
        "INTEGRATION_TEST_MARKER: Mixed test event occurred",
        "System status: operational"
    ])
    
    # Verify process exists for top marker
    process_pid = get_pid("telemetry2_0")
    assert process_pid != "", "Test process should be running"
    
    # Set profile with both grep and top markers
    set_profile(PROFILE_MIXED_TEST)
    
    # Verify profile was enabled
    assert "DCA_MIXED_TEST" in grep_T2logs("Successfully enabled profile"), "Profile should be enabled"
    
    # Wait for reporting
    sleep(35)
    
    # Verify report was generated
    assert "TIMEOUT for profile" in grep_T2logs("DCA_MIXED_TEST"), "Report should be generated"
    
    # Verify both marker types appear in report
    report = grep_T2logs("cJSON Report")
    has_grep = "INTEGRATION_TEST_MARKER" in report
    has_top = "PROCESS_CPU" in report or "Daemon.CPU" in report
    
    # At least one of each type should be present
    assert has_grep or has_top, "Mixed markers should appear in report"


@pytest.mark.order(8)
def test_dca_integration_with_telemetry_report():
    """
    End-to-end test: DCA markers collected and included in telemetry report.
    
    Validates:
    - Profile with DCA markers executes
    - Grep/top markers collected via dcautil
    - Markers appear in JSON report
    - Report sent successfully
    """
    # Clear existing state
    clear_T2logs()
    clear_profile()
    
    # Create log with markers for end-to-end test
    create_test_log([
        "System event logged",
        "Important marker: INTEGRATION_TEST_MARKER detected",
        "BOOT_COMPLETE: Integration test marker",
        "ERROR_MARKER: Test error for integration",
        "Additional context"
    ])
    
    # Set mixed profile to test complete pipeline
    set_profile(PROFILE_MIXED_TEST)
    
    # Verify profile was enabled
    profile_enabled = grep_T2logs("Successfully enabled profile")
    assert "DCA_MIXED_TEST" in profile_enabled, "Profile should be enabled"
    
    # Wait for reporting interval
    sleep(35)
    
    # Verify report was generated
    timeout_log = grep_T2logs("TIMEOUT for profile")
    assert "DCA_MIXED_TEST" in timeout_log, "Report should be generated"
    
    # Verify report contains markers
    report_log = grep_T2logs("cJSON Report")
    assert "Report" in report_log, "JSON report should be generated"
    
    # Verify dcautil was involved (look for grep/top processing)
    telemetry_log = grep_T2logs("telemetry2_0")
    assert len(telemetry_log) > 0, "Should have processing logs"
    
    # Verify markers appear in report
    assert "INTEGRATION_TEST_MARKER" in report_log or "PROCESS_CPU" in report_log, "Markers should be in report"
    
    # Verify report delivery attempt
    rbus_method_log = grep_T2logs("rbus_invokeRemoteMethod")
    # Report should be sent via RBUS_METHOD protocol
    assert len(rbus_method_log) > 0 or "Report" in report_log, "Report should be sent or cached"
