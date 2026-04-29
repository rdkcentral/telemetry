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

# Source: source/bulkdata/reportprofiles.c, reportprofiles.h, profile.c, profile.h

Feature: Report Profiles Management

  Background:
    Given the telemetry daemon is running
    And the report profiles module is initialized

  Scenario: Report profiles module initialization
    Given the report profiles module is not initialized
    When initReportProfiles is called
    Then the profile list should be initialized
    And the scheduler should be initialized with timeout callbacks
    And the event receiver should be initialized
    And the module should be ready to accept profiles

  Scenario: Report profiles module uninitialization
    Given the report profiles module is initialized
    And profiles are registered
    When ReportProfiles_uninit is called
    Then all profiles should be deleted
    And the scheduler should be uninitialized
    And all resources should be freed

  Scenario: Process JSON report profiles blob
    Given the report profiles module is initialized
    When ReportProfiles_ProcessReportProfilesBlob is called with a cJSON profiles object
    Then each profile in the blob should be parsed
    And valid profiles should be added to the profile list
    And invalid profiles should be rejected with appropriate logging

  Scenario: Process MessagePack report profiles blob
    Given the report profiles module is initialized
    When ReportProfiles_ProcessReportProfilesMsgPackBlob is called with msgpack data
    Then the msgpack blob should be deserialized
    And each profile should be processed
    And valid profiles should be registered

  Scenario: Store marker event for a profile
    Given a profile with event markers is registered
    When ReportProfiles_storeMarkerEvent is called with profile name and event info
    Then the event should be stored in the profile's event list
    And the event count should be incremented

  Scenario: Delete a specific profile
    Given a profile is registered with a specific name
    When ReportProfiles_deleteProfile is called with the profile name
    Then the profile should be removed from the profile list
    And the profile's scheduler registration should be removed
    And all profile resources should be freed

  Scenario: Delete all report profiles
    Given multiple profiles are registered
    When deleteAllReportProfiles is called
    Then all profiles should be removed
    And the profile list should be empty

  Scenario: Interrupt report profiles for immediate reporting
    Given profiles are registered and running
    When ReportProfiles_Interrupt is called
    Then all running profiles should be interrupted
    And immediate report generation should be triggered

  Scenario: Generate DCA report
    Given the DCA utility is available
    When generateDcaReport is called with delay and on-demand flags
    Then the DCA report should be generated based on the flags
    And the report should be sent to the configured destination

  Scenario: Profile activation timeout callback
    Given a profile is registered with an activation timeout
    When the activation timeout expires
    Then ReportProfiles_ActivationTimeoutCb should be called
    And the profile should be deactivated
    And the profile should be deleted if deleteonTimeout is true

  Scenario: Profile reporting timeout callback
    Given a profile is registered with a reporting interval
    When the reporting interval expires
    Then ReportProfiles_TimeoutCb should be called
    And a report should be generated for the profile

  Scenario: Check mTLS enablement
    Given the telemetry system is configured
    When isMtlsEnabled is called
    Then the mTLS status should be returned based on configuration

  Scenario: Calculate profile memory usage
    Given profiles are registered
    When profilemem_usage is called
    Then the total memory used by profiles should be calculated
    And the value should be returned

  Scenario: Privacy mode - do not share
    Given privacy mode control is enabled
    When privacymode_do_not_share is called
    Then telemetry data sharing should be disabled
    And the privacy mode should be persisted

  Scenario: Profile encoding types supported
    Given the report profiles module is initialized
    Then the following encoding types should be supported
      | Encoding Type |
      | XML           |
      | XDR           |
      | CSV           |
      | JSON          |
      | MESSAGE_PACK  |

  Scenario: JSON report format options
    Given a profile is configured with JSON encoding
    Then the following JSON formats should be supported
      | Format             |
      | JSONRF_OBJHIERARCHY |
      | JSONRF_KEYVALUEPAIR |

  Scenario: Timestamp format options
    Given a profile is configured with timestamp reporting
    Then the following timestamp formats should be supported
      | Timestamp Format    |
      | TIMESTAMP_UNIXEPOCH |
      | TIMESTAMP_ISO_8601  |
      | TIMESTAMP_NONE      |

  Scenario: Minimum report interval enforcement
    Given a profile is being configured
    When the reporting interval is set below MIN_REPORT_INTERVAL (10 seconds)
    Then the profile should be rejected
    And an error should be logged

  Scenario: Maximum cached reports limit
    Given a profile has failed to send reports
    When the number of cached reports exceeds MAX_CACHED_REPORTS (5)
    Then the oldest cached report should be discarded
    And the new report should be cached
