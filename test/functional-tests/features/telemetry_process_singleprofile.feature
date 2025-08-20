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


Feature: Telemetry Single profile configuration and report generation

  Scenario: Single profile configuration with event marker and use as accumulate
    Given When the telemetry daemon is already running
    When a single profile is configured with event marker and use as accumulate
    Then generated report should contain the values for all occurrences of the marker

  Scenario: Capability to support multiple split markers for the same log line
    When two split markers are configured for the same log line in a file
    Then both the markers will be reported

  Scenario: Caching of upload failed reports
    Given a json report is attemplted to be sent the configured method
    When the attempt to send the report fails
    Then the report will be cached to be sent later along with the next report

Scenario: Data harvesting from previous logs folder for report profiles with log file search markers
    Given the device has logs from the previous session in the PreviousLogs folder
    When a profile goes through log files for report generation
    Then the log files in PreviousLogs folder will also be grepped for log lines
