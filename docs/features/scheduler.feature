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

# Source: source/scheduler/scheduler.c, scheduler.h

Feature: Report Scheduling and Timing

  Background:
    Given the telemetry daemon is running
    And the scheduler module is initialized

  Scenario: Scheduler initialization
    Given the scheduler is not initialized
    When initScheduler is called with callback functions
    Then the timeout notification callback should be registered
    And the activation timeout callback should be registered
    And the scheduler start notification callback should be registered
    And the scheduler should be ready to accept profile registrations

  Scenario: Scheduler uninitialization
    Given the scheduler is initialized
    And profiles are registered with the scheduler
    When uninitScheduler is called
    Then all scheduled profiles should be unregistered
    And all scheduler resources should be freed

  Scenario: Register profile with scheduler
    Given the scheduler is initialized
    When registerProfileWithScheduler is called with profile parameters
    Then a scheduler profile should be created with the following parameters
      | Parameter              | Description                              |
      | profileName            | Name of the profile                      |
      | timeInterval           | Reporting interval in seconds            |
      | activationTimeout      | Activation timeout in seconds            |
      | deleteonTimeout        | Delete profile on timeout flag           |
      | repeat                 | Repeat scheduling flag                   |
      | reportOnUpdate         | Report on update flag                    |
      | firstReportingInterval | First reporting interval                 |
      | timeRef                | Time reference for scheduling            |
    And a scheduler thread should be created for the profile

  Scenario: Unregister profile from scheduler
    Given a profile is registered with the scheduler
    When unregisterProfileFromScheduler is called with the profile name
    Then the scheduler profile should be terminated
    And the scheduler thread should be stopped
    And the scheduler profile resources should be freed

  Scenario: Scheduler triggers timeout notification
    Given a profile is registered with a reporting interval
    When the reporting interval expires
    Then the timeout notification callback should be invoked
    And the profile name should be passed to the callback
    And the clear seek map flag should be passed to the callback

  Scenario: Scheduler triggers activation timeout
    Given a profile is registered with an activation timeout
    When the activation timeout expires
    Then the activation timeout callback should be invoked
    And the profile should be marked for deactivation

  Scenario: Send interrupt to timeout thread
    Given a profile is registered and waiting for timeout
    When SendInterruptToTimeoutThread is called with the profile name
    Then the timeout thread should be interrupted
    And the profile should generate a report immediately

  Scenario: Scheduler handles first reporting interval
    Given a profile is registered with a first reporting interval
    When the profile is first enabled
    Then the first report should be generated after firstReportingInterval
    And subsequent reports should follow the regular reportingInterval

  Scenario: Scheduler handles time reference
    Given a profile is configured with a time reference
    When the scheduler calculates the next timeout
    Then the time reference should be used to align reporting
    And reports should be generated at the specified time reference

  Scenario: Scheduler handles repeat flag
    Given a profile is registered with repeat=true
    When the reporting interval expires
    Then a report should be generated
    And the scheduler should reset the timer for the next interval
    And the profile should continue reporting

  Scenario: Scheduler handles non-repeat profile
    Given a profile is registered with repeat=false
    When the reporting interval expires
    Then a report should be generated
    And the scheduler should not reset the timer
    And the profile should stop reporting

  Scenario: Scheduler handles delete on timeout
    Given a profile is registered with deleteonTimeout=true
    When the activation timeout expires
    Then the profile should be deleted
    And the scheduler profile should be removed

  Scenario: Get lapsed time calculation
    Given two timespec values are provided
    When getLapsedTime is called with result, x, and y
    Then the difference between x and y should be calculated
    And the result should be stored in the result timespec

  Scenario: Retain seek map flag management
    Given the scheduler is managing seek maps
    When set_retainseekmap is called with a value
    Then the retain seek map flag should be updated
    And get_retainseekmap should return the current value

  Scenario: Scheduler profile structure
    Given a scheduler profile is being created
    Then the scheduler profile should contain
      | Field           | Type            | Description                    |
      | name            | char*           | Profile name                   |
      | timeRef         | char*           | Time reference                 |
      | timeOutDuration | unsigned int    | Timeout duration in seconds    |
      | timeToLive      | unsigned int    | Time to live                   |
      | timeRefinSec    | unsigned int    | Time reference in seconds      |
      | repeat          | bool            | Repeat flag                    |
      | terminated      | bool            | Termination flag               |
      | deleteonTime    | bool            | Delete on timeout flag         |
      | reportonupdate  | bool            | Report on update flag          |
      | firstreportint  | unsigned int    | First reporting interval       |
      | firstexecution  | bool            | First execution flag           |
      | tId             | pthread_t       | Thread ID                      |
      | tMutex          | pthread_mutex_t | Thread mutex                   |
      | tCond           | pthread_cond_t  | Thread condition variable      |

  Scenario: Scheduler thread synchronization
    Given multiple profiles are scheduled
    Then each profile should have its own thread
    And thread synchronization should use mutex and condition variables
    And no deadlocks should occur during concurrent operations
