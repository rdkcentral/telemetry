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

  Scenario: WebConfig blob arrives while CollectAndReportXconf is running and
            a regular trigger-condition profile is being deregistered
            (regression: TriggerCondition-Hang — three-mutex deadlock)

    Given the device has three active profiles:
      | Profile             | Type    | Interval |
      | XLE_Telemetry       | xconf   | 900 s    |
      | LTE_Failover_notify | regular | trigger  |
      | LTE_Failover_onboot | regular | 240 s    |
    And a CollectAndReportXconf for XLE_Telemetry is in progress
    And a fresh xconf response returning an updated XLE_Telemetry profile is available

    When a WebConfig blob update arrives containing updated versions of
         LTE_Failover_notify, LTE_Failover_onboot, and XLE_Telemetry within
         the same processing window

    Then the WebConfig handler completes within the ACK timeout (180 seconds)
    And  no deadlock occurs between the scheduler, xconf, and WebConfig threads
    And  a success ACK (not NACK 301) is sent to the WebConfig upstream
    And  the new profiles begin sending reports on their next trigger or interval

    # Failure observable (the bug):
    # WebConfig timer expires → NACK 301 "Blob Execution Timedout"
    # Subsequent LTE trigger-condition reports are never sent
    # No ReportProfiles_TimeoutCb --out is logged for any regular profile
    # Process requires SelfHeal restart to recover
