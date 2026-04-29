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

# Source: source/protocol/rbusMethod/rbusmethodinterface.c, rbusmethodinterface.h

Feature: RBUS Protocol - Report Transmission via RBUS Method

  Background:
    Given the telemetry daemon is running
    And the RBUS interface is initialized

  Scenario: Send report over RBUS method
    Given a profile is configured with RBUS_METHOD protocol
    And a report payload is ready
    When sendReportsOverRBUSMethod is called with method name, input params, and payload
    Then the RBUS method should be invoked
    And the payload should be passed as a parameter
    And the method result should be returned

  Scenario: Send cached reports over RBUS method
    Given a profile has cached reports
    When sendCachedReportsOverRBUSMethod is called with method name, params, and report list
    Then each cached report should be sent via RBUS method
    And successfully sent reports should be removed from the cache
    And failed reports should remain in the cache

  Scenario: RBUS method with input parameters
    Given a profile is configured with RBUS method parameters
    When sending a report
    Then all configured input parameters should be included
    And the parameters should be passed to the RBUS method

  Scenario: Handle RBUS method not found
    Given a profile specifies a non-existent RBUS method
    When attempting to send a report
    Then an error should be returned
    And the report should be cached for retry

  Scenario: Handle RBUS method invocation failure
    Given a profile is configured with a valid RBUS method
    When the RBUS method invocation fails
    Then an error should be returned
    And the failure should be logged
    And the report should be cached for retry
