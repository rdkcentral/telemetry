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

Scenario: Profile non persistence
    Given a temp profile is expired
    When the telemetry is restarted 
    Then the profile will not be enabled after restart

Scenario: Multiple profiles configured simultaneously
    Given the telemetry daemon is already running
    When multiple temporary profiles are configured in a single configuration
    Then all valid profiles should be enabled and processed simultaneously
    Then each profile should operate independently without interference

Scenario: Configurable reporting end points
    Given a temp profile is configured with custom HTTP endpoint URL
    When the profile generates a report
    Then the report should be sent to the configured custom endpoint
    Then the endpoint URL should be correctly constructed with configured parameters

Scenario: Configurable URL parameters for HTTP Protocol
    Given a temp profile is configured with custom URL parameters
    When the profile generates a report for HTTP protocol
    Then the report should be sent with the configured URL parameters appended
    Then the URL should be properly formatted with all custom parameters

Scenario: Forced on demand reporting to support log upload
    Given a temporary profile is running with configured reporting interval
    When kill signal 29 (LOG_UPLOAD_ONDEMAND) is sent to the telemetry daemon
    Then the profile should be interrupted before the scheduled timeout
    Then a report should be generated immediately for the temporary profile
    Then the report should contain all collected data up to that point

Scenario: Temporary profile configuration with incorrect protocol or without protocol field
    Given the telemetry daemon is already running
    When a temp profile is configured with an incorrect protocol or without protocol field
    Then the temp profile should be rejected with appropriate error message

Scenario: Temporary profile configuration without encodingType field
    Given the telemetry daemon is already running
    When a temp profile is configured without encodingType field
    Then the temp profile should be rejected with appropriate error message

Scenario: Temporary profile configuration without ReportingInterval field
    Given the telemetry daemon is already running
    When a temp profile is configured without ReportingInterval field and without TriggerCondition
    Then the temp profile should be rejected with appropriate error message

Scenario: Temporary profile configuration with empty ActivationTimeout
    Given the telemetry daemon is already running
    When a temp profile is configured with empty ActivationTimeout value
    Then the temp profile should be rejected (differs from report profile behavior)

Scenario: Temporary profile configuration with ActivationTimeout less than ReportingInterval
    Given the telemetry daemon is already running
    When a temp profile is configured with ActivationTimeout less than ReportingInterval
    Then the temp profile should be rejected with appropriate error message

Scenario: Temporary profile configuration with reportEmpty as true
    Given the telemetry daemon is already running
    When a temp profile is configured with reportEmpty set to true
    Then report should contain markers with NULL value if the marker data is not available

Scenario: Temporary profile configuration with FirstReportingInterval
    Given the telemetry daemon is already running
    When a temp profile is configured with FirstReportingInterval
    Then the temp profile should be enabled
    Then first report should be generated after the FirstReportingInterval rather than the ReportingInterval

Scenario: Temporary profile with timeref as default and maxUploadLatency
    Given the telemetry daemon is already running
    When a temp profile is configured with timeref as default and maxUploadLatency given
    Then if first reporting interval is given, it will be taken for reporting
    Then maxUploadLatency will not be accepted when timeref is default

Scenario: Temporary profile with timeref not default and maxUploadLatency
    Given the telemetry daemon is already running
    When a temp profile is configured with timeref as not default and maxUploadLatency given
    Then first reporting interval will not be accepted
    Then maxUploadLatency will be used for delayed report sending

Scenario: Temporary profile with grep marker and use as count
    Given the telemetry daemon is already running
    When a temp profile is configured with grep marker and use as count
    Then the temp profile should be enabled
    Then generated report should contain the number of times the search string is present in log files

Scenario: Temporary profile with grep marker, use as absolute and trim
    Given the telemetry daemon is already running
    When a temp profile is configured with grep marker, use as absolute and trim as true
    Then the temp profile should be enabled
    Then generated report should contain content after search string without preceding and trailing spaces

Scenario: Temporary profile with event marker and use as count
    Given the telemetry daemon is already running
    When a temp profile is configured with event marker and use as count
    Then the temp profile should be enabled
    Then generated report should contain the number of times the event has occurred

Scenario: Temporary profile with Datamodel marker and regex
    Given the telemetry daemon is already running
    When a temp profile is configured with Datamodel marker and regex pattern
    Then the temp profile should be enabled
    Then generated report should contain TR181 parameter value matching the given regex

Scenario: Temporary profile with event marker and regex
    Given the telemetry daemon is already running
    When a temp profile is configured with event marker and regex pattern
    Then the temp profile should be enabled
    Then generated report should contain event values matching the given regex

Scenario: Temporary profile with grep marker and regex
    Given the telemetry daemon is already running
    When a temp profile is configured with grep marker and regex pattern
    Then the temp profile should be enabled
    Then generated report should contain log content matching the given regex pattern

Scenario: Temporary profile with multiple split markers for same log line
    Given the telemetry daemon is already running
    When a temp profile is configured with multiple split markers for the same log line
    Then the temp profile should be enabled
    Then all configured split markers should be extracted and reported

Scenario: Temporary profile with TriggerCondition validation - negative cases
    Given the telemetry daemon is already running
    When a temp profile is configured with invalid TriggerCondition parameters
    Then the temp profile should be rejected with specific error messages for each validation failure
    Then errors should be logged for null type, null reference, unexpected type, null operator, unexpected operator, null threshold, and unexpected reference
