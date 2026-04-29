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

# Source: source/bulkdata/t2eventreceiver.c, t2eventreceiver.h

Feature: Telemetry Event Receiver

  Background:
    Given the telemetry daemon is running
    And the event receiver module is initialized

  Scenario: Event receiver initialization
    Given the event receiver is not initialized
    When T2ER_Init is called
    Then the event queue should be created
    And the event receiver should be ready to accept events

  Scenario: Event receiver uninitialization
    Given the event receiver is initialized
    When T2ER_Uninit is called
    Then the event queue should be cleared
    And all event receiver resources should be freed

  Scenario: Start event dispatch thread
    Given the event receiver is initialized
    When T2ER_StartDispatchThread is called
    Then the dispatch thread should be started
    And the thread should begin processing events from the queue

  Scenario: Stop event dispatch thread
    Given the dispatch thread is running
    When T2ER_StopDispatchThread is called
    Then the dispatch thread should be signaled to stop
    And the thread should terminate gracefully

  Scenario: Push event with delimiter
    Given the event receiver is running
    When T2ER_PushDataWithDelim is called with event info and user data
    Then the event should be parsed using the delimiter
    And the event should be added to the event queue
    And the dispatch thread should be notified

  Scenario: Push event with name and value
    Given the event receiver is running
    When T2ER_Push is called with event name and event value
    Then a T2Event should be created with the name and value
    And the event should be added to the event queue
    And the dispatch thread should be notified

  Scenario: Event dispatch thread processing
    Given events are in the queue
    When T2ER_EventDispatchThread processes the queue
    Then each event should be dispatched to registered profiles
    And the event should be stored in matching profile event lists
    And the event should be removed from the queue after processing

  Scenario: Free T2Event structure
    Given a T2Event structure exists
    When freeT2Event is called with the event data
    Then the event name should be freed
    And the event value should be freed
    And the event structure should be freed

  Scenario: T2Event structure
    Given an event is being created
    Then the T2Event structure should contain
      | Field | Type   | Description        |
      | name  | char*  | Event marker name  |
      | value | char*  | Event value        |

  Scenario: Event receiver handles high event volume
    Given the event receiver is running
    When a large number of events are pushed rapidly
    Then all events should be queued
    And the dispatch thread should process all events
    And no events should be lost

  Scenario: Event receiver thread safety
    Given multiple threads are pushing events
    When events are pushed concurrently
    Then the event queue should remain consistent
    And no race conditions should occur
    And all events should be processed correctly
