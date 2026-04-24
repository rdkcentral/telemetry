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


Feature: Telemetry bootup sequence and Xconf communication

  Scenario: Telemetry bootup sequence
    Given When the telemetry daemon is not already running
    Then clean all the persistant flags as precondition
    When the telemetry binary is invoked
    Then the telemetry should be running as a daemon
    Then the telemetry should be initing important flags
    Then the telemetry should be communicating with Xconf server
    When the telemetry communicates with Xconf server
    Then the telemetry should be sending important information such as mac, firmware, env
    Then the telemetry should be saving the xconf data in persistant folder
    When the telemetry bootup is completed 
    Then the telemetry should be exposing all the rbus interface Api's
    When the telemetry recives EXEC RELOAD signal
    Then the telemetry should re fetch the Xconf communication
    When the telemetry gets SIGTERM signal 
    Then the telemetry should be removing all Temporary flags and terminate

  Scenario: Boot Sequence
    Given the telemetry system is running
    When boot sequence is executed
    Then the system should handle it correctly
    And no errors or crashes should occur

  Scenario: Persistant Data
    Given the telemetry system is running
    When persistant data is executed
    Then the system should handle it correctly
    And no errors or crashes should occur

  Scenario: Bootup Flags
    Given the telemetry system is running
    When bootup flags is executed
    Then the system should handle it correctly
    And no errors or crashes should occur

  Scenario: Rbus Data
    Given the telemetry system is running
    When rbus data is executed
    Then the system should handle it correctly
    And no errors or crashes should occur

  Scenario: Xconf Request
    Given the telemetry system is running
    When xconf request is executed
    Then the system should handle it correctly
    And no errors or crashes should occur

  Scenario: Exec Reload
    Given the telemetry system is running
    When exec reload is executed
    Then the system should handle it correctly
    And no errors or crashes should occur

  Scenario: Terminal Signal
    Given the telemetry system is running
    When terminal signal is executed
    Then the system should handle it correctly
    And no errors or crashes should occur

  Scenario: Telemetry Exit
    Given the telemetry system is running
    When telemetry exit is executed
    Then the system should handle it correctly
    And no errors or crashes should occur
