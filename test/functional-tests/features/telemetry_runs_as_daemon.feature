####################################################################################
# If not stated otherwise in this file or this component's Licenses.txt file the
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


Feature: Telemetry runs as daemon to collect data

  Scenario: Telemetry runs as daemon
    Given When the telemetry daemon is not already running
    When the telemetry binary is invoked
    Then the telemetry should be running as a daemon
    And  when the telemetry is attempted to be started again
    Then the telemetry should not be start another instance

  Scenario: Check Telemetry Is Starting
    Given the telemetry system is running
    When check telemetry is starting is executed
    Then the system should handle it correctly
    And no errors or crashes should occur

  Scenario: Second Telemetry Instance Is Not Started
    Given the telemetry system is running
    When second telemetry instance is not started is executed
    Then the system should handle it correctly
    And no errors or crashes should occur

  Scenario: Tear Down
    Given the telemetry system is running
    When tear down is executed
    Then the system should handle it correctly
    And no errors or crashes should occur
