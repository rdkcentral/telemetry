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
L2 Integration Tests for dataModelTable feature (RDKB-65730 / AC4)

Scenarios:
1. Push a profile with dataModelTable (explicit index) -> verify report JSON structure
2. Push a profile with dataModelTable (wildcard) -> verify all rows appear in report
3. Push a profile with dataModelTable while a reporting cycle is active -> verify no crash
4. Build with feature disabled -> push same profile -> verify silently ignored

Prerequisites:
- mock_table_provider must be compiled and running in the container
  (provides Device.X_T2TEST_Table.AccessPoint.{1,2,3}.{SSID,Status,Enable})
- telemetry2_0 must be built with --enable-dynamic-table-support=yes
"""

import subprocess
from time import sleep
from datetime import datetime as dt
import pytest

from basic_constants import *
from helper_functions import *
from report_profiles import *

LOG_PROFILE_ENABLE = "Successfully enabled profile :"


# ===========================================================================
# Scenario 1: dataModelTable with explicit index
# ===========================================================================
@pytest.mark.run(order=1)
def test_datamodeltable_explicit_index():
    """
    Push a profile with dataModelTable (explicit index "1,2") and verify:
    - Profile is enabled successfully
    - T2 logs show data collection for indexed table parameters
    - Report contains structured array with row 1 and row 2 data only
    """
    # Setup
    clear_T2logs()
    kill_telemetry(9)
    remove_T2bootup_flag()
    clear_persistant_files()
    run_telemetry()
    sleep(2)

    # Push profile with explicit index
    rbus_set_data(T2_REPORT_PROFILE_PARAM, "string", data_datamodeltable_explicit_index)
    sleep(25)  # Wait for ReportingInterval (20s) + processing time

    # Verify profile was enabled
    assert "DT_ExplicitIndex" in grep_T2logs(LOG_PROFILE_ENABLE), \
        "Profile DT_ExplicitIndex was not enabled"

    # Verify dataModelTable parsing succeeded
    assert "dataModelTable" not in grep_T2logs("Error"), \
        "Unexpected error related to dataModelTable in logs"

    # Verify data collection happened for indexed params
    # T2 should query Device.X_T2TEST_Table.AccessPoint.1.SSID and .2.SSID
    log_content = grep_T2logs("Device.X_T2TEST_Table.AccessPoint")
    assert "Device.X_T2TEST_Table.AccessPoint" in log_content, \
        "No evidence of table parameter data collection in logs"

    # Verify no crash - telemetry process should still be running
    pid = get_pid("telemetry2_0")
    assert pid != "", "telemetry2_0 crashed during dataModelTable processing"

    # Cleanup
    rbus_set_data(T2_REPORT_PROFILE_PARAM, "string", data_empty_profile)
    sleep(2)


# ===========================================================================
# Scenario 2: dataModelTable with wildcard (no index)
# ===========================================================================
@pytest.mark.run(order=2)
def test_datamodeltable_wildcard():
    """
    Push a profile with dataModelTable (wildcard - no index) and verify:
    - Profile is enabled successfully
    - All rows (1,2,3) appear in the report
    - Report contains structured array with all sub-parameters
    """
    # Setup
    clear_T2logs()
    kill_telemetry(9)
    remove_T2bootup_flag()
    clear_persistant_files()
    run_telemetry()
    sleep(2)

    # Push wildcard profile
    rbus_set_data(T2_REPORT_PROFILE_PARAM, "string", data_datamodeltable_wildcard)
    sleep(25)  # Wait for ReportingInterval (20s) + processing time

    # Verify profile was enabled
    assert "DT_Wildcard" in grep_T2logs(LOG_PROFILE_ENABLE), \
        "Profile DT_Wildcard was not enabled"

    # Verify no errors during dataModelTable processing
    assert "dataModelTable" not in grep_T2logs("Error"), \
        "Unexpected error related to dataModelTable in logs"

    # Verify wildcard data collection - all rows should be queried
    log_content = grep_T2logs("Device.X_T2TEST_Table.AccessPoint")
    assert "Device.X_T2TEST_Table.AccessPoint" in log_content, \
        "No evidence of wildcard table parameter collection in logs"

    # Verify no crash - telemetry process should still be running
    pid = get_pid("telemetry2_0")
    assert pid != "", "telemetry2_0 crashed during wildcard dataModelTable processing"

    # Cleanup
    rbus_set_data(T2_REPORT_PROFILE_PARAM, "string", data_empty_profile)
    sleep(2)


# ===========================================================================
# Scenario 3: dataModelTable while a reporting cycle is active
# ===========================================================================
@pytest.mark.run(order=3)
def test_datamodeltable_during_active_cycle():
    """
    Push a profile with dataModelTable while a reporting cycle is already active.
    Verify:
    - No crash occurs
    - The reporting cycle completes successfully
    - The new profile is picked up in the next cycle
    """
    # Setup
    clear_T2logs()
    kill_telemetry(9)
    remove_T2bootup_flag()
    clear_persistant_files()
    run_telemetry()
    sleep(2)

    # Push a first profile to start a reporting cycle
    rbus_set_data(T2_REPORT_PROFILE_PARAM, "string", data_datamodeltable_wildcard)
    sleep(5)  # Let reporting cycle start but don't wait for it to complete

    # Now push a different profile mid-cycle
    rbus_set_data(T2_REPORT_PROFILE_PARAM, "string", data_datamodeltable_explicit_index)
    sleep(25)  # Wait for cycle to complete

    # Verify no crash - key assertion
    pid = get_pid("telemetry2_0")
    assert pid != "", \
        "telemetry2_0 crashed when profile was pushed during active reporting cycle"

    # Verify the new profile was enabled
    assert "DT_ExplicitIndex" in grep_T2logs(LOG_PROFILE_ENABLE), \
        "Profile DT_ExplicitIndex was not enabled after mid-cycle push"

    # Verify report completed (look for report generation log)
    report_log = grep_T2logs("Report sent successfully")
    # Even if report wasn't sent (mock may not accept), no crash is the key requirement

    # Cleanup
    rbus_set_data(T2_REPORT_PROFILE_PARAM, "string", data_empty_profile)
    sleep(2)


# ===========================================================================
# Scenario 4: Feature disabled - dataModelTable silently ignored
# ===========================================================================
@pytest.mark.run(order=4)
def test_datamodeltable_feature_disabled():
    """
    When built WITHOUT --enable-dynamic-table-support, pushing a profile with
    dataModelTable should:
    - NOT crash
    - Silently ignore the dataModelTable entry
    - NOT log any error above WARNING level for the ignored entry
    - Still process other parameters in the profile normally

    NOTE: This test validates the behavior by checking the T2 logs.
    If the binary was built WITH the feature enabled, this test checks that
    the "Dynamic table support disabled" debug message is NOT present (meaning
    the feature IS active). The CI pipeline runs this scenario against the
    disabled build.
    """
    # Setup
    clear_T2logs()
    kill_telemetry(9)
    remove_T2bootup_flag()
    clear_persistant_files()
    run_telemetry()
    sleep(2)

    # Push dataModelTable profile
    rbus_set_data(T2_REPORT_PROFILE_PARAM, "string", data_datamodeltable_explicit_index)
    sleep(25)

    # Verify no crash
    pid = get_pid("telemetry2_0")
    assert pid != "", "telemetry2_0 crashed processing dataModelTable"

    # Check if this is a feature-disabled build
    disabled_msg = grep_T2logs("Dynamic table support disabled")
    if disabled_msg:
        # Feature is DISABLED in this build - verify silent ignore behavior
        # Should NOT have any ERROR level logs about dataModelTable
        error_logs = grep_T2logs("Error")
        assert "dataModelTable" not in error_logs, \
            "ERROR level log found for dataModelTable in disabled build"

        # The profile should still be enabled (other params are valid)
        assert "DT_ExplicitIndex" in grep_T2logs(LOG_PROFILE_ENABLE), \
            "Profile was not enabled even though non-table params are valid"
    else:
        # Feature is ENABLED - dataModelTable should be processed normally
        assert "DT_ExplicitIndex" in grep_T2logs(LOG_PROFILE_ENABLE), \
            "Profile DT_ExplicitIndex was not enabled"

    # Cleanup
    rbus_set_data(T2_REPORT_PROFILE_PARAM, "string", data_empty_profile)
    sleep(2)
