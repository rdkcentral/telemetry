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

# Source: source/bulkdata/profile.c, profile.h

Feature: Profile Lifecycle Management

  Background:
    Given the telemetry daemon is running
    And the profile list is initialized

  Scenario: Initialize profile list
    Given the profile list is not initialized
    When initProfileList is called with checkPreviousSeek flag
    Then the profile list should be created
    And the profile hash map should be initialized
    And the profile list lock should be initialized

  Scenario: Uninitialize profile list
    Given the profile list is initialized
    And profiles are registered
    When uninitProfileList is called
    Then all profiles should be deleted
    And the profile hash map should be destroyed
    And all resources should be freed

  Scenario: Add a new profile
    Given the profile list is initialized
    And a valid profile structure is prepared
    When addProfile is called with the profile
    Then the profile should be added to the profile list
    And the profile count should be incremented

  Scenario: Get profile count
    Given profiles are registered
    When getProfileCount is called
    Then the number of registered profiles should be returned

  Scenario: Check if profile with name exists
    Given a profile named "TestProfile" is registered
    When profileWithNameExists is called with "TestProfile"
    Then bProfileExists should be set to true

  Scenario: Check if profile with name does not exist
    Given no profile named "NonExistent" is registered
    When profileWithNameExists is called with "NonExistent"
    Then bProfileExists should be set to false

  Scenario: Store marker event in profile
    Given a profile with event markers is registered
    When Profile_storeMarkerEvent is called with profile name and event info
    Then the event should be stored in the profile's event marker list
    And the event mutex should be used for thread safety

  Scenario: Enable a profile
    Given a profile is registered but not enabled
    When enableProfile is called with the profile name
    Then the profile's enable flag should be set to true
    And the profile should be registered with the scheduler

  Scenario: Disable a profile
    Given a profile is registered and enabled
    When disableProfile is called with the profile name
    Then the profile's enable flag should be set to false
    And the profile should be unregistered from the scheduler
    And isDeleteRequired should indicate if deletion is needed

  Scenario: Delete a profile
    Given a profile is registered
    When deleteProfile is called with the profile name
    Then the profile should be removed from the profile list
    And all profile resources should be freed
    And the profile should be removed from disk if persistent

  Scenario: Delete all profiles
    Given multiple profiles are registered
    When deleteAllProfiles is called with delFromDisk flag
    Then all profiles should be deleted
    And if delFromDisk is true, persistent storage should be cleared

  Scenario: Update marker component map
    Given profiles with event markers are registered
    When updateMarkerComponentMap is called
    Then the marker-to-component mapping should be updated
    And the mapping should reflect current profile configurations

  Scenario: Get profile hash map
    Given profiles are registered
    When getProfileHashMap is called
    Then the profile hash map should be returned
    And the map should contain all registered profiles

  Scenario: Send log upload interrupt to scheduler
    Given profiles are registered with the scheduler
    When sendLogUploadInterruptToScheduler is called
    Then all scheduled profiles should be interrupted
    And immediate report generation should be triggered

  Scenario: Notify profile timeout
    Given a profile is registered with a timeout
    When NotifyTimeout is called with profile name and clear seek map flag
    Then the profile timeout should be processed
    And the seek map should be cleared if requested

  Scenario: Get marker component RBUS subscription
    Given profiles with TR181 markers are registered
    When getMarkerCompRbusSub is called with subscription flag
    Then RBUS subscriptions should be managed based on the flag

  Scenario: Check if profile is enabled
    Given a profile is registered
    When isProfileEnabled is called with the profile name
    Then the enabled status should be returned

  Scenario: Register trigger condition consumer
    Given profiles with trigger conditions are registered
    When registerTriggerConditionConsumer is called
    Then the trigger condition consumer should be registered
    And trigger events should be monitored

  Scenario: Trigger report on condition
    Given a profile with trigger conditions is registered
    When triggerReportOnCondtion is called with reference name and value
    Then the trigger condition should be evaluated
    And if matched, a report should be generated

  Scenario: Get minimum threshold duration
    Given a profile with threshold duration is registered
    When getMinThresholdDuration is called with profile name
    Then the minimum threshold duration should be returned

  Scenario: Report generation complete notification
    Given a profile is generating a report
    When reportGenerationCompleteReceiver is called with profile name
    Then the report completion should be acknowledged
    And the profile should be ready for the next reporting cycle

  Scenario: Notify scheduler start
    Given a profile is being scheduled
    When NotifySchedulerstart is called with profile name and scheduler status
    Then the profile's scheduler status should be updated

  Scenario: Append trigger condition to profile
    Given a profile is being configured
    When appendTriggerCondition is called with profile, reference name, and value
    Then the trigger condition should be added to the profile's trigger list

  Scenario: Profile thread safety with lock hierarchy
    Given multiple threads access profiles concurrently
    Then the following lock hierarchy should be maintained
      | Level | Lock Name              | Purpose                           |
      | L0    | profileListLock        | Protects profileList              |
      | L1    | reuseThreadMutex       | Protects thread lifecycle         |
      | L2    | reportInProgressMutex  | Protects reportInProgress flag    |
      | L3    | triggerCondMutex       | Protects trigger conditions       |
      | L3    | eventMutex             | Protects event list               |
      | L3    | reportMutex            | Protects report generation        |

  Scenario: Profile structure contains all required fields
    Given a profile is being created
    Then the profile should support the following fields
      | Field                  | Type          | Description                        |
      | enable                 | bool          | Profile enabled status             |
      | hash                   | char*         | Unique profile hash                |
      | name                   | char*         | Profile name                       |
      | protocol               | char*         | Communication protocol             |
      | encodingType           | char*         | Report encoding type               |
      | reportingInterval      | unsigned int  | Reporting interval in seconds      |
      | activationTimeoutPeriod| unsigned int  | Activation timeout in seconds      |
      | firstReportingInterval | unsigned int  | First report interval              |
      | maxUploadLatency       | unsigned int  | Maximum upload latency             |
      | generateNow            | bool          | Generate report immediately        |
      | deleteonTimeout        | bool          | Delete profile on timeout          |
      | triggerReportOnCondition| bool         | Trigger-based reporting            |
