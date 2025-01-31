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


Feature: Telemetry multiprofile configuration and report generation

  Scenario: Multiprofile configuration with empty name or hash
    Given When the telemetry daemon is already running
    When a multiprofile is configured with an empty/NULL name or hash value
    Then the multiprofile should be rejected

  Scenario: Multiprofile configuration without name or hash field
    Given When the telemetry daemon is already running
    When a multiprofile is configured without a name/hash field
    Then the multiprofile should be rejected

  Scenario: Multiprofile configuration with empty version or without version field
    Given When the telemetry daemon is already running
    When a multiprofile is configured with empty version or without version field
    Then the multiprofile should not be rejected

  Scenario: Multiprofile configuration with incorrect protocol or without protocol field
    Given When the telemetry daemon is already running
    When a multiprofile is configured with an incorrect protocol or without protocol field
    Then the multiprofile should be rejected

  Scenario: Multiprofile configuration with an empty ActivationTimeout or without ActivationTimeout
    Given When the telemetry daemon is already running
    When a multiprofile is configured with an empty ActivationTimeout or without ActivationTimeout
    Then the multiprofile should not be rejected

  Scenario: Multiprofile configuration with an empty encodingType
    Given When the telemetry daemon is already running
    When a multiprofile is configured with an empty encodingType
    Then the multiprofile should not be rejected

  Scenario: Multiprofile configuration without encodingType field
    Given When the telemetry daemon is already running
    When a multiprofile is configured without encodingType field
    Then the multiprofile should be rejected

  Scenario: Multiprofile configuration without ReportingInterval field
    Given When the telemetry daemon is already running
    When a multiprofile is configured without ReportingInterval field
    Then the multiprofile should be rejected

  Scenario: Multiprofile configuration without GenerateNow field
    Given When the telemetry daemon is already running
    When a multiprofile is configured without GenerateNow field
    Then the multiprofile should not be rejected

  Scenario: Multiprofile configuration with ReportingInterval
    Given When the telemetry daemon is already running
    When a multiprofile is configured with ReportingInterval
    Then the multiprofile should be enabled
    Then report should be generated after ReportingInterval

  Scenario: Multiprofile configuration with event marker and use as count
    Given When the telemetry daemon is already running
    When a multiprofile is configured with event marker and use as count
    Then the multiprofile should be enabled
    Then generated report should contain the number of times the marker is reported

  Scenario: Multiprofile configuration with event marker and use as count
    Given When the telemetry daemon is already running
    When a multiprofile is configured with event marker and use as count
    Then the multiprofile should be enabled
    Then generated report should contain the number of times the event has occurred

  Scenario: Multiprofile configuration with event marker and use as accumulate
    Given When the telemetry daemon is already running
    When a multiprofile is configured with event marker and use as accumulate
    Then the multiprofile should be enabled
    Then generated report should contain the values for all occurrences of the marker

  Scenario: Multiprofile configuration with GenerateNow as true
    Given When the telemetry daemon is already running
    When a multiprofile is configured with GenerateNow as true
    Then the multiprofile should be enabled
    Then the report shpuld be generated immediately without waiting period

  Scenario: Multiprofile configuration with grep marker and use as count
    Given When the telemetry daemon is already running
    When a multiprofile is configured with grep marker and use as count
    Then the multiprofile should be enabled
    Then generated report should contain the number of times the search string is present

  Scenario: Multiprofile configuration with grep marker and use as absolute
    Given When the telemetry daemon is already running
    When a multiprofile is configured with grep marker and use as absolute
    Then the multiprofile should be enabled
    Then generated report should contain the the content after the search string until the end of the line

  Scenario: Multiprofile configuration with grep marker, use as absolute and trim as true
    Given When the telemetry daemon is already running
    When a multiprofile is configured with grep marker and use as absolute and trim as true
    Then the multiprofile should be enabled
    Then generated report should contain the the content after the search string until the end of the line without preceding and trailing space

  Scenario: Multiprofile configuration with Datamodel marker
    Given When the telemetry daemon is already running
    When a multiprofile is configured with Datamodel marker
    Then the multiprofile should be enabled
    Then generated report should contain the value of the tr181 parameter given in the reference

  Scenario: Multiprofile configuration with ActivationTimeout less than ReportingInterval
    Given When the telemetry daemon is already running
    When a multiprofile is configured with ActivationTimeoutless than ReportingInterval
    Then the multiprofile should be rejected

  Scenario: Multiprofile configuration with reportEmpty as true
    Given When the telemetry daemon is already running
    When a multiprofile is configured with reportEmpty as true
    Then report should contain the marker as NULL if the marker is not reported

  Scenario: Multiprofile configuration with FirstReportingInterval
    Given When the telemetry daemon is already running
    When a multiprofile is configured with FirstReportingInterval
    Then the multiprofile should be enabled
    Then first report should be generated after the FirstReportingInterval rather than the ReportingInterval

  Scenario: Multiprofile configuration with ActivationTimeout
    Given When the telemetry daemon is already running
    When a multiprofile is configured with ActivationTimeout
    Then the multiprofile should be enabled
    Then the profile should be disabled after the expiring of ActivationTimeout

  Scenario: Multiprofile configuration with grep marker, use as absolute and with regex
    Given When the telemetry daemon is already running
    When a multiprofile is configured with grep marker, use as absolute and with regex
    Then the multiprofile should be enabled
    Then generated report should contain the content after the search string until the end of the line matching the given regex 

  Scenario: Multiprofile configuration with event marker, use as absolute and with regex
    Given When the telemetry daemon is already running
    When a multiprofile is configured with even marker, use as absolute and with regex
    Then the multiprofile should be enabled
    Then generated report should contain the values matching the given regex for the marker

  Scenario: Multiprofile configuration with Datamodel marker and with regex
    Given When the telemetry daemon is already running
    When a multiprofile is configured with Datamodel marker and with regex
    Then the multiprofile should be enabled
    Then generated report should contain the value of the tr181 parameter given in the reference, matching the given regex 

  Scenario: Parallel execution of multiple report profile and XCONF based DCM profile
    Given When the telemetry daemon is already running
    When a multiprofile is configured and a DCM profile is fetched from xconf
    Then both the multiprofile and the DCM profile should be enabled
    Then report should be generated for both the multiprofile and DCM profile separately
    
