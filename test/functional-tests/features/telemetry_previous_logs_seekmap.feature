####################################################################################
# If not stated otherwise in this file or this component's Licenses
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


Feature: Previous logs reporting and seekmap lifecycle management

  Background:
    Given the telemetry daemon is configured with a profile containing grep markers
    And the profile is configured to save seekmap

  Scenario: Previous logs report with successful HTTP send - seekmap file deleted
    Given the device has performed regular reporting and saved seekmap file to disk
    And markers exist in log files that were reported before reboot
    When the device reboots and logs are moved to PreviousLogs folder
    And the telemetry daemon starts and detects existing seekmap file
    Then a "previous logs" report is generated with PREVIOUS_LOG tag
    And the HTTP send succeeds
    And the seekmap file is deleted from /opt/.t2seekmap/ folder
    And the checkPreviousSeek flag is cleared
    When the next regular report is generated
    Then a fresh seekmap is created starting from position 0
    And no PREVIOUS_LOG tag is present in the report
    When the device reboots again
    Then no seekmap file exists in /opt/.t2seekmap/ folder
    And no "previous logs" report is triggered on startup

  Scenario: Previous logs report with failed HTTP send - seekmap file still deleted
    Given the device has performed regular reporting and saved seekmap file to disk
    And markers exist in log files that were reported before reboot
    When the device reboots and logs are moved to PreviousLogs folder
    And the telemetry daemon starts and detects existing seekmap file
    Then a "previous logs" report is generated with PREVIOUS_LOG tag
    And the HTTP send fails with timeout or network error
    And the report is cached for retry
    And the seekmap file is deleted from /opt/.t2seekmap/ folder
    And the checkPreviousSeek flag is cleared
    When the next regular report is generated
    Then a fresh seekmap is created starting from position 0
    And no PREVIOUS_LOG tag is present in the regular report
    When the device reboots again
    Then no seekmap file exists in /opt/.t2seekmap/ folder
    And no "previous logs" report is triggered on startup

  Scenario: Previous logs report does not contain duplicate markers
    Given the device has generated a report containing marker "prev_reboot_split" with value "Boot1_timestamp"
    And the seekmap was saved with positions after scanning those markers
    When the device reboots and logs are moved to PreviousLogs folder
    And the telemetry daemon starts and generates "previous logs" report
    Then the report should NOT contain marker "prev_reboot_split" with value "Boot1_timestamp"
    And the report should only contain markers written between last seekmap save and reboot
    And markers that were already reported should not appear again

  Scenario: Regular report with failed HTTP send - seekmap still saved
    Given the telemetry daemon has generated a report by scanning log files
    And the in-memory seekmap has been updated with new file positions
    When the HTTP send fails with timeout or network error
    Then the report is cached for retry
    And the seekmap is saved to disk with updated positions
    When the next report cycle begins
    Then the grep scan starts from the saved seekmap positions
    And no duplicate markers are included in the next report
    And incremental data collection continues correctly

  Scenario: Regular report with successful HTTP send - seekmap saved
    Given the telemetry daemon has generated a report by scanning log files
    And the in-memory seekmap has been updated with new file positions
    When the HTTP send succeeds
    Then the report is sent successfully
    And the seekmap is saved to disk with updated positions
    When the next report cycle begins
    Then the grep scan starts from the saved seekmap positions
    And only new log content is scanned

  Scenario: Previous logs scan only executes once per boot
    Given the device has rebooted with seekmap file present
    And the telemetry daemon generates a "previous logs" report
    And the HTTP send fails and report is cached
    When the cached report is retried and sent successfully
    Then no additional "previous logs" report is generated
    And subsequent reports are regular reports without PREVIOUS_LOG tag
    When telemetry daemon is restarted without reboot
    Then no "previous logs" report is triggered
    And only regular reporting continues

  Scenario: Seekmap file correspondence with scan behavior
    Given a profile performs regular reporting every 60 seconds
    And log file "test.log" has 1000 bytes of content
    When first report scans from position 0 to 1000
    Then seekmap is saved with {"test.log": 1000}
    When new content is written from byte 1000 to 1500
    And second report is generated
    Then grep starts scanning from position 1000
    And only content from byte 1000-1500 is scanned
    And seekmap is updated with {"test.log": 1500}

  Scenario: Previous logs with empty incremental content
    Given the device has saved seekmap with positions at end of log files
    And no new log content was written between last scan and reboot
    When the device reboots and "previous logs" report is generated
    Then the report may contain empty or minimal marker values
    And the seekmap file is still deleted
    And no error occurs due to empty scan results

  Scenario: Multiple profiles with independent seekmap files
    Given two profiles "Profile_A" and "Profile_B" are configured
    And both profiles have saved their own seekmap files
    When the device reboots
    Then both profiles generate independent "previous logs" reports
    And seekmap file "/opt/.t2seekmap/Profile_A" is deleted after Profile_A scan
    And seekmap file "/opt/.t2seekmap/Profile_B" is deleted after Profile_B scan
    And both profiles create fresh seekmaps on next regular report
