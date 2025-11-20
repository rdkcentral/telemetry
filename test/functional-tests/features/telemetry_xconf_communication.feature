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

Feature: Telemetry Xconf communication

  Scenario: Telemetry Xconf communication with valid URL
    Given Paramater Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.ConfigURL is set to a valid https URL
    Then clean all the persistant flags as precondition
    When the telemetry binary is invoked
    Then the telemetry should be running as a daemon
    Then the telemetry should be initing important flags
    Then the telemetry should construct Xconf URI with ConfigURL
    Then the telemetry should add meta data such as mac, firmware, env in URL encoded format
    Then the telemetry should be communicating with Xconf server with the constructed URI
    When the telemetry communicates with Xconf server
    Then the telemetry should be sending important information such as mac, firmware, env
    Then the telemetry should be saving the xconf data in persistant folder
    When the telemetry bootup is completed 
    Then the telemetry should be exposing all the rbus interface Api's

  Scenario: Telemetry Xconf communication with invalid URL
    Given Paramater Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.ConfigURL is set to an invalid URL or empty
    Then clean all the persistant flags as precondition
    When the telemetry binary is invoked
    Then the telemetry should be running as a daemon
    Then the telemetry should be initing important flags
    Then the telemetry should not be attempting to communicate with Xconf server

  Scenario: Telemetry Xconf communication with a valid URL but server responds with 404 error (No configuration offered for the device)
    Given Paramater Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.ConfigURL is set to a valid https URL
    When the telemetry binary is invoked or reload is initiated with signal reload
    Then the telemetry should be running as a daemon
    Then the telemetry should be communicating with Xconf server
    When the telemetry communicates with Xconf server
    Then the telemetry should respect the 404 error and not save any data
    Then telemetry should terminate all reporting based on xconf profiles and not attempt to communicate with xconf server

  Scenario: Telemetry Xconf communication with valid URL check for logupload and logupload_on_demand
    Given Paramater Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.ConfigURL is set to a valid https URL
    When the telemetry binary is invoked or reload is initiated with signal reload
    Then the telemetry should be running as a daemon
    Then the telemetry should be communicating with Xconf server
    When the telemetry communicates with Xconf server
    Then the telemetry should be generating the report when it received the kill signal 10

  Scenario: Telemetry Xconf communication retry mechanism for connection errors
    Given Paramater Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.ConfigURL is set to a valid https URL
    When the telemetry attempts to communicate with Xconf server but connection fails
    Then the telemetry should retry the connection after a configured wait period
    Then the telemetry should log the retry attempts with appropriate wait time messages
    Then the telemetry should continue retrying until connection is established or max retries reached

  Scenario: Telemetry Xconf profile with datamodel markers and accumulate
    Given Paramater Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.ConfigURL is set to a valid https URL
    When the telemetry fetches an Xconf profile with datamodel markers configured as subscribe with accumulate
    Then the telemetry should subscribe to the TR181 parameter value changes
    When the TR181 parameter value changes multiple times during reporting interval
    Then all the value changes should be accumulated and reported in the generated report
    Then the report should contain all accumulated values for the datamodel marker

  Scenario: Telemetry Xconf profile with split markers validation
    Given Paramater Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.ConfigURL is set to a valid https URL
    When the telemetry fetches an Xconf profile with split markers configured for log files
    Then the telemetry should parse and enable the split markers
    When log files contain lines matching the split marker patterns
    Then only the content matching the split marker criteria should be extracted
    Then the report should contain the split marker data as configured
    Then split markers with conditions should be evaluated correctly

  Scenario: Telemetry Xconf profile with grep markers from previous logs
    Given Paramater Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.ConfigURL is set to a valid https URL
    When the telemetry fetches an Xconf profile with grep markers for log files
    Then the telemetry should search both current logs and PreviousLogs folder
    When generating reports for the Xconf profile
    Then log content from PreviousLogs folder should be included in the report
    Then grep markers should successfully extract data from rotated log files

  Scenario: Parallel execution of Xconf DCM profile and multiprofile
    Given Paramater Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.ConfigURL is set to a valid https URL
    When the telemetry fetches an Xconf DCM profile
    And a multiprofile is configured via RBUS parameter
    Then both the Xconf DCM profile and multiprofile should be enabled simultaneously
    Then both profiles should operate independently without interference
    Then reports should be generated separately for each profile according to their configurations
    Then each profile should maintain its own reporting schedule and markers

  Scenario: Telemetry Xconf profile with event markers and accumulate
    Given Paramater Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.ConfigURL is set to a valid https URL
    When the telemetry fetches an Xconf profile with event markers configured with accumulate
    Then the telemetry should accept and accumulate event marker data
    When event markers are sent to telemetry daemon during reporting interval
    Then all event occurrences should be accumulated with their values
    Then the generated report should contain all accumulated event marker data

  Scenario: Telemetry Xconf profile with configurable reporting interval
    Given Paramater Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.ConfigURL is set to a valid https URL
    When the telemetry fetches an Xconf profile with specific reporting interval configured
    Then the telemetry should honor the configured reporting interval
    Then reports should be generated at the specified interval
    Then the telemetry logs should confirm the reporting interval being used

  Scenario: Telemetry Xconf profile caching of upload failed reports
    Given Paramater Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.ConfigURL is set to a valid https URL
    When the telemetry generates a report for Xconf profile
    And the report upload to configured endpoint fails
    Then the telemetry should cache the failed report
    Then the cached report should be sent along with the next successful report
    Then the telemetry logs should indicate the number of cached reports
