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

# Source: source/ccspinterface/rbusInterface.c, rbusInterface.h

Feature: RBUS Interface - Data Model and Event Handling

  Background:
    Given the telemetry daemon is running
    And the RBUS interface is initialized

  Scenario: Check RBUS initialization status
    Given the RBUS module may or may not be initialized
    When isRbusInitialized is called
    Then the initialization status should be returned

  Scenario: Get RBUS parameter value
    Given a TR181 parameter exists
    When getRbusParameterVal is called with the parameter name
    Then the parameter value should be retrieved via RBUS
    And the value should be returned to the caller

  Scenario: Get multiple RBUS profile parameter values
    Given a list of TR181 parameters is provided
    When getRbusProfileParamValues is called with the parameter list
    Then all parameter values should be retrieved
    And the values should be returned as a vector

  Scenario: Register RBUS T2 event listener
    Given the telemetry event callback is defined
    When registerRbusT2EventListener is called with the callback
    Then the event listener should be registered with RBUS
    And telemetry events should be routed to the callback

  Scenario: Unregister RBUS T2 event listener
    Given the T2 event listener is registered
    When unregisterRbusT2EventListener is called
    Then the event listener should be unregistered
    And no more events should be received

  Scenario: Register trigger condition consumer
    Given a profile has trigger conditions configured
    When rbusT2ConsumerReg is called with the trigger condition list
    Then RBUS subscriptions should be created for each trigger
    And trigger events should be monitored

  Scenario: Unregister trigger condition consumer
    Given trigger condition consumers are registered
    When rbusT2ConsumerUnReg is called with the trigger condition list
    Then RBUS subscriptions should be removed
    And trigger monitoring should stop

  Scenario: Invoke RBUS method
    Given an RBUS method exists
    When rbusMethodCaller is called with method name, params, payload, and callback
    Then the RBUS method should be invoked
    And the callback should be called with the result

  Scenario: Check if RBUS method exists
    Given an RBUS method name is provided
    When rbusCheckMethodExists is called with the method name
    Then the existence of the method should be checked
    And true/false should be returned accordingly

  Scenario: Subscribe to TR181 report events
    Given a TR181 parameter supports events
    When T2RbusReportEventConsumer is called with reference and subscription=true
    Then an RBUS subscription should be created
    And parameter changes should trigger events

  Scenario: Unsubscribe from TR181 report events
    Given a TR181 subscription exists
    When T2RbusReportEventConsumer is called with reference and subscription=false
    Then the RBUS subscription should be removed
    And no more events should be received for the parameter

  Scenario: Publish report upload status
    Given a report has been uploaded
    When publishReportUploadStatus is called with status
    Then the status should be published via RBUS
    And subscribers should receive the status update

  Scenario: Set T2 event receive state
    Given the telemetry system state changes
    When setT2EventReceiveState is called with the new state
    Then the event receive state should be updated
    And event processing should be enabled/disabled accordingly

  Scenario: Handle RBUS event subscription
    Given an RBUS event subscription request is received
    When eventSubHandler is called
    Then the subscription should be processed
    And autoPublish should be set appropriately

  Scenario: RBUS log handler
    Given RBUS generates log messages
    When logHandler is called with log level and message
    Then the log should be processed according to the level
    And the message should be logged appropriately

  Scenario: Register DCM agent event listener
    Given DCMAGENT is defined
    When registerRbusDCMEventListener is called
    Then the DCM event listener should be registered
    And DCM events should be received
