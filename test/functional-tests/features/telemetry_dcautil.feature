####################################################################################
# If not stated otherwise in this file or this component's Licenses
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

Feature: DCA Utility Module (Legacy Device Configuration Analytics)

  As a telemetry system operator
  I want dcautil to collect grep and top markers reliably
  So that log patterns and process metrics appear in telemetry reports

  Background:
    Given the telemetry system is running
    And the dcautil module is available

  Scenario: Grep marker collection from log files
    Given a profile with grep-type markers is configured
    And log files contain matching patterns
    When the grep search is executed
    Then the matching log entries should be found
    And the marker values should be extracted correctly
    And the results should appear in the telemetry report

  Scenario: Process monitoring with top markers
    Given a profile with top-type markers is configured
    And the target process is running
    When process monitoring is executed
    Then CPU usage percentage should be calculated
    And memory usage (RSS) should be reported
    And the metrics should appear in the telemetry report

  Scenario: Log rotation handling
    Given log rotation has occurred
    And markers exist in both current and rotated logs
    When the grep search is executed
    Then markers from both log files should be found
    And no duplicate entries should be reported
    And rotated logs should be handled appropriately based on boot status

  Scenario: Incremental log processing with seek positions
    Given logs have been previously processed
    And seek positions have been saved
    When new log entries are appended
    Then only new entries should be processed
    And seek positions should be updated
    And previously processed entries should be skipped

  Scenario: Missing log files handled gracefully
    Given a grep marker references a non-existent log file
    When the grep search is executed
    Then no crash or error should occur
    And the system should continue processing other markers
    And an appropriate log message should be generated

  Scenario: Large log file performance
    Given a log file larger than 1MB exists
    When grep markers are collected from the large file
    Then the search should complete efficiently
    And memory-mapped file access should be used
    And markers should be found without overflow errors

  Scenario: Mixed grep and top markers in single profile
    Given a profile contains both grep and top markers
    When the report is generated
    Then grep markers should be collected from logs
    And top markers should be collected from process stats
    And both marker types should appear in the same report

  Scenario: End-to-end DCA integration with telemetry report
    Given a profile with DCA markers is active
    And the reporting interval has elapsed
    When a report is generated
    Then dcautil should collect all configured markers
    And the markers should appear in the JSON report
    And the report should be sent successfully
