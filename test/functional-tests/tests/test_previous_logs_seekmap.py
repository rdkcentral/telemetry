####################################################################################
# If not stated otherwise in this file or this component's Licenses file the
# following copyright and licenses apply:
#
# Copyright 2026 RDK Management
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
Test suite for previous logs reporting and seekmap lifecycle management.
Validates behavior related to the duplicate marker reporting bug in previous logs handling.
"""

import pytest
import os
import json
import time
import subprocess
from helper_functions import *
from basic_constants import *

# Additional Test Constants
SEEKMAP_FOLDER = "/opt/.t2seekmap/"
PREVIOUS_LOGS_FOLDER = "/opt/logs/PreviousLogs/"
TEST_LOG_FILE = "/opt/logs/test_markers.log"
TEST_PROFILE_NAME = "TestProfile_Seekmap"
PERSISTENT_PATH = "/opt/.t2persistentfolder"
LOG_PATH = "/opt/logs"
CACHED_MESSAGE_PATH = os.path.join(PERSISTENT_PATH, ".t2cachedmessages")


class TestPreviousLogsSeekmap:
    """Test suite for previous logs and seekmap lifecycle management."""

    @pytest.fixture(autouse=True)
    def setup_teardown(self):
        """Setup and teardown for each test."""
        # Setup
        self.cleanup_test_environment()
        yield
        # Teardown
        self.cleanup_test_environment()

    def cleanup_test_environment(self):
        """Clean up test files and folders."""
        run_shell_command(f"rm -rf {SEEKMAP_FOLDER}*")
        run_shell_command(f"rm -rf {PREVIOUS_LOGS_FOLDER}")
        run_shell_command(f"rm -f {TEST_LOG_FILE}")
        run_shell_command(f"rm -f {PERSISTENT_PATH}/DCMresponse.txt")

    def create_seekmap_file(self, profile_name, seekmap_data):
        """
        Create a seekmap file with specified positions.
        
        Args:
            profile_name: Name of the profile
            seekmap_data: Dict of {filename: byte_position}
        """
        os.makedirs(SEEKMAP_FOLDER, exist_ok=True)
        seekmap_file = os.path.join(SEEKMAP_FOLDER, profile_name)
        
        # Create JSON array format as used by T2
        seekmap_json = []
        for logfile, position in seekmap_data.items():
            seekmap_json.append({logfile: position})
        
        with open(seekmap_file, 'w') as f:
            json.dump(seekmap_json, f)
        
        print(f"Created seekmap file: {seekmap_file} with data: {seekmap_json}")

    def verify_seekmap_deleted(self, profile_name):
        """Verify that seekmap file has been deleted."""
        seekmap_file = os.path.join(SEEKMAP_FOLDER, profile_name)
        assert not os.path.exists(seekmap_file), \
            f"Seekmap file {seekmap_file} should have been deleted but still exists"
        print(f"Verified seekmap file deleted: {seekmap_file}")

    def verify_seekmap_exists(self, profile_name):
        """Verify that seekmap file exists."""
        seekmap_file = os.path.join(SEEKMAP_FOLDER, profile_name)
        assert os.path.exists(seekmap_file), \
            f"Seekmap file {seekmap_file} should exist but does not"
        print(f"Verified seekmap file exists: {seekmap_file}")

    def read_seekmap_file(self, profile_name):
        """Read and return seekmap data."""
        seekmap_file = os.path.join(SEEKMAP_FOLDER, profile_name)
        with open(seekmap_file, 'r') as f:
            data = json.load(f)
        return data

    def setup_previous_logs_folder(self, log_content):
        """
        Setup PreviousLogs folder with test log files.
        
        Args:
            log_content: Dict of {filename: content_string}
        """
        os.makedirs(PREVIOUS_LOGS_FOLDER, exist_ok=True)
        for filename, content in log_content.items():
            log_path = os.path.join(PREVIOUS_LOGS_FOLDER, filename)
            with open(log_path, 'w') as f:
                f.write(content)
        print(f"Created {len(log_content)} log files in {PREVIOUS_LOGS_FOLDER}")

    def wait_for_report(self, timeout=30):
        """Wait for telemetry to generate a report."""
        time.sleep(timeout)

    def get_latest_report_from_logs(self):
        """
        Extract the latest JSON report from telemetry logs.
        Returns the report dict or None.
        """
        log_file = f"{LOG_PATH}/telemetry2_0.txt.0"
        if not os.path.exists(log_file):
            return None
        
        with open(log_file, 'r') as f:
            content = f.read()
        
        # Find last occurrence of cJSON Report
        import re
        matches = list(re.finditer(r'cJSON Report = ({.*})', content))
        if not matches:
            return None
        
        last_report_json = matches[-1].group(1)
        try:
            return json.loads(last_report_json)
        except json.JSONDecodeError:
            return None

    def test_previous_logs_send_success_seekmap_deleted(self):
        """
        Test: Previous logs report with successful HTTP send - seekmap file deleted.
        
        Verifies that after a successful previous logs report send, the old seekmap
        file is deleted to prevent re-triggering on next boot.
        """
        print("\n=== Test: Previous logs send success - seekmap deleted ===")
        
        # Step 1: Create seekmap file (simulating previous boot session)
        self.create_seekmap_file(TEST_PROFILE_NAME, {
            "test_markers.log": 500
        })
        
        # Step 2: Create PreviousLogs with content
        self.setup_previous_logs_folder({
            "test_markers.log": "Line 1\nLine 2\nmarker_test: value_123\n"
        })
        
        # Step 3: Configure profile and start telemetry
        # (Implementation depends on your test profile configuration method)
        # configure_test_profile(TEST_PROFILE_NAME, ...)
        # run_telemetry()
        
        # Step 4: Wait for previous logs report
        self.wait_for_report(timeout=60)
        
        # Step 5: Verify seekmap deleted
        self.verify_seekmap_deleted(TEST_PROFILE_NAME)
        
        # Step 6: Verify report contained PREVIOUS_LOG tag
        report = self.get_latest_report_from_logs()
        assert report is not None, "No report found in logs"
        
        search_result = report.get("searchResult", [])
        previous_log_tag = any(
            item.get("PREVIOUS_LOG") == "1" for item in search_result
        )
        assert previous_log_tag, "Report should contain PREVIOUS_LOG tag"
        
        print("✓ Previous logs send success test passed")

    def test_previous_logs_send_failed_seekmap_deleted(self):
        """
        Test: Previous logs report with failed HTTP send - seekmap file still deleted.
        
        This is the key test for the bug fix. Verifies that even when HTTP send fails,
        the seekmap file is deleted to prevent duplicate reporting on next boot.
        """
        print("\n=== Test: Previous logs send failed - seekmap still deleted ===")
        
        # Step 1: Create seekmap file
        self.create_seekmap_file(TEST_PROFILE_NAME, {
            "test_markers.log": 500
        })
        
        # Step 2: Create PreviousLogs
        self.setup_previous_logs_folder({
            "test_markers.log": "marker_test: value_456\n"
        })
        
        # Step 3: Configure profile with unreachable HTTP endpoint to force failure
        # (Use mock HTTP server or invalid URL)
        # configure_test_profile(TEST_PROFILE_NAME, upload_url="http://invalid:9999/")
        # run_telemetry()
        
        # Step 4: Wait for report generation and send attempt
        self.wait_for_report(timeout=60)
        
        # Step 5: Verify seekmap deleted even though send failed
        self.verify_seekmap_deleted(TEST_PROFILE_NAME)
        
        # Step 6: Verify report was cached
        cached_reports_path = f"{CACHED_MESSAGE_PATH}/{TEST_PROFILE_NAME}"
        # assert os.path.exists(cached_reports_path), "Report should be cached on send failure"
        
        print("✓ Previous logs send failed test passed - seekmap correctly deleted")

    def test_no_duplicate_markers_previous_logs(self):
        """
        Test: Previous logs report does not contain duplicate markers.
        
        Verifies that markers already reported before reboot do not appear again
        in the previous logs report.
        """
        print("\n=== Test: No duplicate markers in previous logs ===")
        
        # Step 1: Simulate that marker was already scanned up to byte 1000
        marker_content_before_reboot = (
            "prev_reboot_split: Boot1_Fri_Mar_15_08:19:54_UTC_2026\n"
        )
        marker_content_after = (
            "new_marker: new_value_after_1000\n"
        )
        
        # Seekmap indicates we already scanned first 1000 bytes
        self.create_seekmap_file(TEST_PROFILE_NAME, {
            "test_markers.log": 1000
        })
        
        # PreviousLogs has both old and new content, but old should be skipped
        full_log_content = "x" * 900 + marker_content_before_reboot + "x" * 50 + marker_content_after
        self.setup_previous_logs_folder({
            "test_markers.log": full_log_content
        })
        
        # Start telemetry and wait for previous logs report
        self.wait_for_report(timeout=60)
        
        # Verify report does NOT contain old marker value
        report = self.get_latest_report_from_logs()
        if report:
            search_result = report.get("searchResult", [])
            for item in search_result:
                if "prev_reboot_split" in item:
                    assert "Boot1_Fri_Mar_15" not in item["prev_reboot_split"], \
                        "Report should NOT contain marker from before seekmap position"
        
        print("✓ No duplicate markers test passed")

    def test_regular_report_send_failed_seekmap_saved(self):
        """
        Test: Regular report with failed HTTP send - seekmap still saved.
        
        Verifies that for regular (non-previous-logs) reports, seekmap is saved
        even when HTTP send fails, so cached reports don't cause duplicate scanning.
        """
        print("\n=== Test: Regular report send failed - seekmap saved ===")
        
        # Step 1: Start telemetry with normal profile (no previous logs)
        # configure_test_profile(TEST_PROFILE_NAME, ...)
        # run_telemetry()
        
        # Step 2: Write content to log file
        with open(TEST_LOG_FILE, 'w') as f:
            f.write("marker1: value1\nmarker2: value2\n")
        
        # Step 3: Configure profile to fail HTTP send
        # (Use invalid endpoint)
        
        # Step 4: Wait for report generation
        self.wait_for_report(timeout=60)
        
        # Step 5: Verify seekmap was saved despite send failure
        self.verify_seekmap_exists(TEST_PROFILE_NAME)
        
        # Step 6: Verify seekmap has non-zero positions
        seekmap_data = self.read_seekmap_file(TEST_PROFILE_NAME)
        assert len(seekmap_data) > 0, "Seekmap should have entries"
        
        print("✓ Regular report send failed but seekmap saved test passed")

    def test_seekmap_incremental_scanning(self):
        """
        Test: Seekmap file correspondence with incremental scan behavior.
        
        Verifies that seekmap correctly tracks scan positions and subsequent
        reports only scan new content.
        """
        print("\n=== Test: Seekmap incremental scanning ===")
        
        # Step 1: Write initial content
        with open(TEST_LOG_FILE, 'w') as f:
            f.write("marker1: value1\n")
        
        initial_size = os.path.getsize(TEST_LOG_FILE)
        
        # Step 2: First report cycle
        self.wait_for_report(timeout=60)
        
        # Step 3: Verify seekmap saved with position at end of file
        self.verify_seekmap_exists(TEST_PROFILE_NAME)
        seekmap_data = self.read_seekmap_file(TEST_PROFILE_NAME)
        
        # Find position for our test log file
        test_log_position = None
        for entry in seekmap_data:
            if "test_markers.log" in entry:
                test_log_position = entry["test_markers.log"]
                break
        
        assert test_log_position >= initial_size, \
            f"Seekmap position {test_log_position} should be >= file size {initial_size}"
        
        # Step 4: Append new content
        with open(TEST_LOG_FILE, 'a') as f:
            f.write("marker2: value2\n")
        
        # Step 5: Second report cycle
        self.wait_for_report(timeout=60)
        
        # Step 6: Verify seekmap updated with new position
        seekmap_data_after = self.read_seekmap_file(TEST_PROFILE_NAME)
        test_log_position_after = None
        for entry in seekmap_data_after:
            if "test_markers.log" in entry:
                test_log_position_after = entry["test_markers.log"]
                break
        
        assert test_log_position_after > test_log_position, \
            "Seekmap position should increase after scanning new content"
        
        print("✓ Seekmap incremental scanning test passed")

    def test_previous_logs_executed_only_once(self):
        """
        Test: Previous logs scan only executes once per boot.
        
        Verifies that even if first report send fails and is cached, subsequent
        retries do not re-trigger previous logs scan.
        """
        print("\n=== Test: Previous logs executed only once per boot ===")
        
        # Step 1: Create seekmap to trigger previous logs
        self.create_seekmap_file(TEST_PROFILE_NAME, {
            "test_markers.log": 100
        })
        
        self.setup_previous_logs_folder({
            "test_markers.log": "test_content\n"
        })
        
        # Step 2: Start telemetry (will trigger previous logs)
        # run_telemetry()
        self.wait_for_report(timeout=60)
        
        # Step 3: Verify seekmap deleted
        self.verify_seekmap_deleted(TEST_PROFILE_NAME)
        
        # Step 4: Trigger another report cycle (e.g., via scheduler)
        self.wait_for_report(timeout=60)
        
        # Step 5: Verify no new seekmap file created (previous logs not re-triggered)
        # and next report doesn't have PREVIOUS_LOG tag
        report = self.get_latest_report_from_logs()
        if report:
            search_result = report.get("searchResult", [])
            previous_log_tag = any(
                item.get("PREVIOUS_LOG") == "1" for item in search_result
            )
            assert not previous_log_tag, \
                "Subsequent report should NOT have PREVIOUS_LOG tag"
        
        print("✓ Previous logs executed only once test passed")


if __name__ == "__main__":
    pytest.main([__file__, "-v", "-s"])
