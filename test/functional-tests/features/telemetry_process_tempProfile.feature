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


Feature: Telemetry temporary profile configuration and report generation

Scenario: Verify co-existence of Report profile and Temporary report profile
    Given a temp profile is configured and running
    When a temporary profile is configured
    Then both profiles should be running without any interference from each other

  Scenario: temp profile configuration with Datamodel marker
    Given When the telemetry daemon is already running
    When a temp profile is configured with Datamodel marker
    Then the temp profile should be enabled
    Then generated report should contain the value of the tr181 parameter given in the reference

Scenario: Support for subscribing to TR181 Parameter value change
    Given a datamodel marker is configured as method subscribe
    When the tr181 parameter value changes
    Then the value change will be sent as an event to the telemetry daemon

Scenario: Data harvesting from previous logs folder for report profiles with log file search markers
    Given the device has logs from the previous session in the PreviousLogs folder
    When a temp profile goes through log files for report generation
    Then the log files in PreviousLogs folder will also be grepped for log lines

  Scenario: temp profile configuration with event marker and use as accumulate
    Given When the telemetry daemon is already running
    When a temp profile is configured with event marker and use as accumulate
    Then the temp profile should be enabled
    Then generated report should contain the values for all occurrences of the marker


Scenario: Include data from data source Tr181 parameters as Accumulate
    Given a datamodel marker is configured as method subscribe and use accumulate
    When the tr181 parameter value changes multiple time inside the reporting interval
    Then all the changes will be reported with values

Scenario: Multiple profiles configured simultaneously
    When a temporary profile configuration has multiple profiles
    Then all the profiles will be processed and run if applicable

  Scenario: temp profile configuration with ReportingInterval
    Given When the telemetry daemon is already running
    When a temp profile is configured with ReportingInterval
    Then the temp profile should be enabled
    Then report should be generated after ReportingInterval

  Scenario: temp profile configuration with ActivationTimeout
    Given When the telemetry daemon is already running
    When a temp profile is configured with ActivationTimeout
    Then the temp profile should be enabled
    Then the profile should be disabled after the expiring of ActivationTimeout

Scenario: temp profile with TriggerConditions                         
    Given When the telemetry daemon is already running                                                     
    When a temp profile is configured with TriggerConditions
    Then temp profile should be accepted and report should be generated whenever trigger condition is triggered

  Scenario: temp profile configuration with grep marker, use as absolute and with regex
    Given When the telemetry daemon is already running
    When a temp profile is configured with grep marker, use as absolute and with regex
    Then the temp profile should be enabled
    Then generated report should contain the content after the search string until the end of the line matching the given regex 

Scenario: Caching of upload failed reports
    Given a json report is attemplted to be sent the configured method
    When the attempt to send the report fails
    Then the report will be cached to be sent later along with the next report

Scenario: Report sending with protocol set as RBUS_METHOD in report profiles.
    Given a profile is confugred with report sending protocol as HTTP along with the respective datamodel
    Then the report will be configured to the respective datamodel

Scenario: profile non persistence
    Given a temp profile is expired
    When the telemetry is restarted 
    Then the profile will not be enabled after restart
