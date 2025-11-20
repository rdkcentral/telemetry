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


Feature: Telemetry Bulk Data Profile Management and Processing

  Background:
    Given the telemetry daemon is initialized
    And bulk data module is ready to process profiles

  # ============================================================================
  # Profile Lifecycle Management
  # ============================================================================

  Scenario: Initialize profile list on daemon startup
    Given the telemetry daemon is starting
    When initProfileList is called with checkPreviousSeek parameter
    Then the profile list should be created as a Vector
    And the profile list mutex (plMutex) should be initialized
    And the report lock mutex (reportLock) should be initialized
    And the initialized flag should be set to true
    And profiles should be loaded from persistent storage if available

  Scenario: Add new profile to profile list
    Given the profile list is initialized
    When a new profile is added via addProfile function
    Then the profile should be validated for required fields
    And the profile should be added to the profileList Vector
    And the profile count should be incremented
    And the profile should be accessible by name
    And appropriate mutexes should be initialized for the profile

  Scenario: Enable and disable profile lifecycle
    Given a profile exists in the profile list
    When enableProfile is called
    Then the profile's enable flag should be set to true
    And the profile's scheduler should be started
    And marker component map should be updated
    And when disableProfile is called
    Then the profile's enable flag should be set to false
    And the scheduler should be stopped

  Scenario: Delete profile from profile list
    Given a profile exists in the profile list
    When deleteProfile is called with the profile name
    Then the profile should be removed from profileList Vector
    And all profile resources should be freed
    And the profile's mutexes should be destroyed
    And marker component map should be cleared and rebuilt

  # ============================================================================
  # Profile Data Collection and Reporting
  # ============================================================================

  Scenario: Collect and report with JSON encoding
    Given a profile is configured with JSON encoding type
    And the profile has TR-181 parameters, grep markers, and event markers
    When CollectAndReport thread is triggered
    Then a JSON report object should be initialized with RootName
    And TR-181 parameter values should be fetched
    And grep results should be collected
    And event marker values should be retrieved
    And all results should be encoded in JSON format
    And the JSON report should be sent to configured endpoint

  Scenario: Handle report generation with trigger condition
    Given a profile has triggerReportOnCondition set to true
    When a trigger condition is met
    Then the triggerCondMutex should be acquired
    And the existing jsonReportObj should be saved
    And a new JSON report should be initialized
    And trigger condition data should be included
    And triggerReportOnCondition should be reset to false

  Scenario: Cache reports when profile is updated
    Given a profile is being updated
    And report generation is in progress
    When the profile's isUpdated flag is set to true
    Then the generated report should be cached in cachedReportList
    And maximum of MAX_CACHED_REPORTS (5) should be maintained
    And cached reports should be sent after profile update completes

  # ============================================================================
  # Report Profile Processing Module
  # ============================================================================

  Scenario: Initialize report profiles module
    Given the telemetry daemon is starting
    When initReportProfiles is called
    Then bulk data configuration should be initialized
    And minimum report interval should be set to 10 seconds
    And maximum parameter references should be set to 100
    And maximum report size should be set to 51200 bytes
    And mTLS configuration should be checked if enabled

  Scenario: Process report profiles from JSON blob
    Given a JSON blob containing report profiles is received
    When ReportProfiles_ProcessReportProfilesBlob is called
    Then the JSON should be parsed and validated
    And each profile should be processed
    And profiles should be added via addProfile
    And profiles should be enabled
    And event dispatch thread should be started

  Scenario: Process report profiles from MessagePack blob
    Given a MessagePack blob is received
    When ReportProfiles_ProcessReportProfilesMsgPackBlob is called
    Then the MessagePack data should be unpacked
    And profiles should be parsed from MessagePack format
    And profiles should be added and enabled
    And the blob should be saved to persistent storage

  Scenario: Handle activation timeout for profile
    Given a profile has activationTimeoutPeriod configured
    When the activation timeout expires
    Then the profile should be disabled
    And if deleteonTimeout is true, profile should be deleted from disk
    And marker component map should be cleared and rebuilt

  Scenario: Handle reporting timeout for profile
    Given a profile has reportingInterval configured
    When the reporting interval timeout occurs
    Then NotifyTimeout should be called
    And isClearSeekMap flag should determine seek map clearing
    And report generation should be triggered

  # ============================================================================
  # Xconf Profile Management
  # ============================================================================

  Scenario: Set and manage Xconf profile
    Given an Xconf profile configuration is received
    When ProfileXConf_set is called
    Then if singleProfile exists, it should be freed first
    And the new profile should be assigned
    And marker component map should be updated
    And event dispatch thread should be restarted

  Scenario: Collect and report for Xconf profile
    Given an Xconf profile is configured
    When CollectAndReportXconf thread is triggered
    Then a JSON report should be initialized with "searchResult"
    And T2 header with version "1.0" should be added
    And Profile field should be added based on platform
    And timestamp should be added
    And all markers should be collected and encoded

  # ============================================================================
  # Data Model Processing
  # ============================================================================

  Scenario: Initialize datamodel module
    Given the telemetry daemon is starting
    When datamodel_init is called
    Then rpQueue should be created for report profiles
    And tmpRpQueue should be created for temporary profiles
    And rpMsgPkgQueue should be created for MessagePack profiles
    And processing threads should be started
    And stopProcessing flag should be set to false

  Scenario: Process profile via datamodel JSON interface
    Given a JSON blob with profiles is received
    When datamodel_processProfile is called
    Then the JSON should be parsed and validated
    And profiles should be pushed to appropriate queue
    And the processing thread should be signaled
    And profiles should be processed asynchronously

  Scenario: Get saved profiles from persistent storage
    Given profiles are saved in persistent storage
    When datamodel_getSavedJsonProfilesasString is called
    Then profiles should be loaded from disk
    And a JSON object should be created
    And profiles should be formatted with name, Hash, and value
    And the JSON string should be returned

  # ============================================================================
  # T2 Marker Component Map Management
  # ============================================================================

  Scenario: Initialize and manage marker component map
    Given the telemetry daemon is starting
    When initT2MarkerComponentMap is called
    Then markerCompMap hash map should be created
    And componentList Vector should be created
    And mutexes should be initialized

  Scenario: Add event marker to component map
    Given a profile has event markers configured
    When addT2EventMarker is called
    Then if marker doesn't exist, a new T2Marker should be created
    And the marker should be associated with component name
    And profile name should be added to marker's profileList
    And component should be added to componentList if not present

  Scenario: Retrieve marker and component information
    Given event markers are configured
    When getMarkerProfileList is called with marker name
    Then the profileList Vector should be returned
    And when getComponentMarkerList is called
    Then all markers for that component should be returned

  # ============================================================================
  # Event Receiver and Dispatch
  # ============================================================================

  Scenario: Initialize and manage event receiver
    Given the telemetry daemon is starting
    When T2ER_Init is called
    Then event receiver structures should be initialized
    And event queues should be created
    And when T2ER_StartDispatchThread is called
    Then the dispatch thread should start listening for events

  Scenario: Store and dispatch marker events
    Given a profile has event markers configured
    When an event is received for a marker
    Then the event should be stored in profile's eMarkerList
    And if marker is in multiple profiles
    Then the event should be dispatched to all profiles

  # ============================================================================
  # Webconfig Integration
  # ============================================================================

  Scenario: Initialize Webconfig framework
    Given Webconfig support is enabled
    When tele_web_config_init is called
    Then subdoc "telemetry" should be registered
    And version callbacks should be registered
    And register_sub_docs should be called successfully

  Scenario: Manage telemetry blob version
    Given Webconfig is managing telemetry profiles
    When getTelemetryBlobVersion is called
    Then version should be read from file
    And when setTelemetryBlobVersion is called
    Then version should be written to file

  # ============================================================================
  # Thread Safety and Concurrency
  # ============================================================================

  Scenario: Thread-safe profile list access
    Given multiple threads are accessing profile list
    When profile operations are performed
    Then plMutex should be acquired before access
    And the mutex should be released after operations
    And no race conditions should occur

  Scenario: Thread-safe report generation
    Given a profile is generating a report
    When another thread attempts to trigger generation
    Then reportInProgressMutex should be acquired
    And reportInProgress flag should be checked
    And only one report should be generated at a time

  Scenario: Thread-safe marker component map access
    Given multiple profiles are updating marker map
    When addT2EventMarker is called concurrently
    Then t2MarkersMutex should protect map access
    And no race conditions should occur

  # ============================================================================
  # Memory Management
  # ============================================================================

  Scenario: Free profile resources on deletion
    Given a profile is being deleted
    When freeProfile is called
    Then all string fields should be freed
    And marker lists should be destroyed
    And parameter lists should be destroyed
    And cached reports should be freed
    And no memory leaks should occur

  Scenario: Calculate and monitor memory usage
    Given multiple profiles are configured
    When profilemem_usage is called
    Then total memory used by profiles should be calculated
    And when T2totalmem_calculate is called
    Then total T2 memory usage should be calculated

  # ============================================================================
  # Encoding and Report Format
  # ============================================================================

  Scenario: Encode report in JSON name-value pair format
    Given a profile uses JSON encoding with JSONRF_KEYVALUEPAIR
    When report is generated
    Then each marker should be encoded as name-value pair
    And entries should be in a JSON array
    And timestamp format should follow configuration

  Scenario: Encode report in MessagePack format
    Given a profile uses MessagePack encoding
    When report is generated
    Then data should be serialized in MessagePack binary format
    And the format should be compact and efficient

  # ============================================================================
  # Advanced Features
  # ============================================================================

  Scenario: Handle report caching and retry
    Given a report upload fails
    When the report is cached
    Then it should be stored in cachedReportList
    And maximum of 5 cached reports should be maintained
    And cached reports should be retried on next interval

  Scenario: Support privacy mode filtering
    Given privacy mode is enabled
    When report is generated
    Then privacy-sensitive markers should be excluded
    And only privacy-compliant markers should be included

  Scenario: Support mTLS for secure communication
    Given mTLS is enabled
    When a profile uploads to HTTPS endpoint
    Then mTLS certificates should be loaded
    And secure communication should be established

  Scenario: Drop root privileges after initialization
    Given DROP_ROOT_PRIV feature is enabled
    When daemon completes initialization
    Then root privileges should be dropped
    And process should run with reduced capabilities

  # ============================================================================
  # Report Thread Management
  # ============================================================================

  Scenario: Reuse report thread for multiple reports
    Given a profile has completed report generation
    When the next reporting interval occurs
    Then the existing report thread should be reused
    And the thread should wait on reuseThread condition variable
    And threadExists flag should remain true
    And thread resources should not be recreated

  Scenario: Handle report thread termination
    Given a report thread is running
    When the profile is deleted
    Then the thread should be signaled to terminate
    And the thread should complete current report if in progress
    And thread resources should be cleaned up
    And threadExists flag should be set to false
