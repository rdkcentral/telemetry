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


Feature: Telemetry Event Marker Sending and Receiving System

  Background:
    Given the telemetry daemon is initialized
    And event receiver module is ready

  # ============================================================================
  # Event Sender Client Library (commonlib)
  # ============================================================================

  Scenario: Initialize event sender with component name
    Given a component wants to send telemetry events
    When t2_init is called with component name
    Then the component name should be stored globally
    And the component should be registered for event sending
    And mutexes should be initialized for thread safety
    And the component should be ready to send events

  Scenario: Send string event marker
    Given event sender is initialized with component name
    When t2_event_s is called with marker name and string value
    Then the marker and value should be validated
    And if value is empty or "0", event should not be sent
    And trailing newline should be stripped from value
    And the event should be sent via report_or_cache_data
    And T2ERROR_SUCCESS should be returned on success

  Scenario: Send integer event marker
    Given event sender is initialized with component name
    When t2_event_d is called with marker name and integer value
    Then the marker should be validated
    And if value is 0, event should not be sent (field triage requirement)
    And integer value should be converted to string
    And the event should be sent via report_or_cache_data
    And T2ERROR_SUCCESS should be returned on success

  Scenario: Send floating point event marker
    Given event sender is initialized with component name
    When t2_event_f is called with marker name and double value
    Then the marker should be validated
    And double value should be converted to string with %f format
    And the event should be sent via report_or_cache_data
    And T2ERROR_SUCCESS should be returned on success

  Scenario: Reject event when component name is not initialized
    Given t2_init has not been called
    When t2_event_s/d/f is called
    Then T2ERROR_COMPONENT_NULL should be returned
    And the event should not be sent
    And an error should be logged with component PID

  Scenario: Uninitialize event sender
    Given event sender is initialized
    When t2_uninit is called
    Then component name should be freed
    And if RBUS is enabled, rbus_close should be called
    And all mutexes should be destroyed
    And the component should no longer be able to send events

  # ============================================================================
  # Event Caching Mechanism
  # ============================================================================

  Scenario: Cache event when T2 daemon is not ready
    Given T2 daemon is not initialized
    When an event is sent via t2_event_s
    Then isCachingRequired should return true
    And the event should be cached to T2_CACHE_FILE
    And event format should be "markerName<#=#>value"
    And file locking should be used to prevent corruption
    And T2ERROR_SUCCESS should be returned

  Scenario: Cache event when PAM is not initialized (RDKB)
    Given PAM is not ready (/tmp/pam_initialized does not exist)
    When an event is sent
    Then isCachingRequired should return true
    And the event should be cached to file
    And deadlock with PAM should be avoided

  Scenario: Cache event when RFC check fails
    Given RFC initialization fails
    When an event is sent
    Then isCachingRequired should return true
    And the event should be cached
    And error should be logged

  Scenario: Do not cache when T2 is disabled by RFC
    Given RFC check succeeds but T2 is disabled
    When an event is sent
    Then isCachingRequired should return false
    And the event should not be cached or sent
    And the event should be dropped

  Scenario: Check T2 operational status before sending
    Given T2 daemon is running
    When an event is sent
    Then T2_OPERATIONAL_STATUS should be queried via RBUS
    And if T2_STATE_COMPONENT_READY bit is not set, event should be cached
    And if T2_STATE_COMPONENT_READY bit is set, event should be sent
    And operational status should be checked on every event

  Scenario: Handle cache file with file locking
    Given multiple components are caching events simultaneously
    When cacheEventToFile is called
    Then T2_CACHE_LOCK_FILE should be opened
    And F_WRLCK file lock should be acquired using fcntl
    And event should be written to T2_CACHE_FILE
    And file lock should be released
    And file descriptor should be closed
    And thread should be detached

  Scenario: Enforce maximum cache limit
    Given T2_CACHE_FILE has 200 cached events (MAX_EVENT_CACHE)
    When a new event needs to be cached
    Then the cache count should be checked via "wc -l"
    And if count >= 200, event should not be cached
    And a warning should be logged about max cache limit
    And the event should be dropped

  Scenario: Flush cached events when T2 becomes ready
    Given events are cached in T2_CACHE_FILE
    When T2ER_StopDispatchThread is called
    Then flushCacheFromFile should be invoked
    And each line from T2_CACHE_FILE should be read
    And trailing newline should be stripped
    And event should be sent via T2ER_PushDataWithDelim
    And T2_CACHE_FILE should be removed after flushing

  # ============================================================================
  # RBUS Event Filtering
  # ============================================================================

  Scenario: Populate event marker list from T2 daemon
    Given component is initialized in RBUS mode
    When doPopulateEventMarkerList is called
    Then data element "{T2_ROOT_PARAMETER}{componentName}{T2_EVENT_LIST_PARAM_SUFFIX}" should be queried
    And rbus_get should retrieve RBUS_OBJECT with event marker properties
    And eventMarkerMap hash map should be created
    And each event marker name should be added to hash map
    And markerListMutex should protect map access

  Scenario: Filter events based on marker list
    Given eventMarkerMap contains allowed markers for component
    When filtered_event_send is called with a marker
    Then markerListMutex should be acquired
    And marker should be looked up in eventMarkerMap
    And if marker is not found, event should be dropped
    And if marker is found, event should be sent via RBUS
    And markerListMutex should be released

  Scenario: Skip filtering for script events
    Given component name is "telemetry_client"
    When filtered_event_send is called
    Then event filtering should be skipped
    And event should be sent without checking eventMarkerMap
    And all script events should be allowed through

  Scenario: Subscribe to profile update notifications
    Given component is initialized in RBUS mode
    And T2 daemon is ready
    When isT2Ready becomes true
    Then rbusEvent_Subscribe should be called for T2_PROFILE_UPDATED_NOTIFY
    And rbusEventReceiveHandler should be registered as callback
    And when profile is updated, doPopulateEventMarkerList should be called
    And eventMarkerMap should be refreshed with new markers

  Scenario: Handle profile update event
    Given component is subscribed to T2_PROFILE_UPDATED_NOTIFY
    When T2 daemon sends profile update notification
    Then rbusEventReceiveHandler should be invoked
    And event name should be checked for T2_PROFILE_UPDATED_NOTIFY
    And doPopulateEventMarkerList should be called to refresh markers
    And new event marker list should be retrieved from T2 daemon

  # ============================================================================
  # RBUS Event Sending
  # ============================================================================

  Scenario: Send event via RBUS
    Given RBUS is enabled
    And event passes marker filtering
    When filtered_event_send is called
    Then rbusProperty should be created with marker name
    And rbusValue should be set with event data
    And rbus_set should be called with T2_EVENT_PARAM
    And if rbus_set succeeds, status 0 should be returned
    And if rbus_set fails, status -1 should be returned and error logged

  Scenario: Send event via CCSP (DBUS mode)
    Given CCSP support is enabled
    And RBUS is not enabled
    When filtered_event_send is called
    Then event data should be formatted as "markerName<#=#>value"
    And CcspBaseIf_SendTelemetryDataSignal should be called
    And if send succeeds, status 0 should be returned
    And if send fails, status -1 should be returned

  # ============================================================================
  # Event Receiver (T2 Daemon Side)
  # ============================================================================

  Scenario: Initialize event receiver
    Given T2 daemon is starting
    When T2ER_Init is called
    Then if already initialized, T2ERROR_SUCCESS should be returned
    And event queue (eQueue) should be created
    And sTDMutex should be initialized
    And erMutex should be initialized
    And erCond condition variable should be initialized
    And EREnabled flag should be set to true
    And appropriate callback should be registered based on bus type
    And /tmp/.t2ReadyToReceiveEvents file should be created
    And T2_STATE_COMPONENT_READY should be set

  Scenario: Register RBUS event callback
    Given RBUS is enabled
    When T2ER_Init is called
    Then T2ER_Push should be registered as callback
    And registerForTelemetryEvents should be called with T2ER_Push

  Scenario: Register DBUS event callback
    Given RBUS is not enabled
    When T2ER_Init is called
    Then T2ER_PushDataWithDelim should be registered as callback
    And registerForTelemetryEvents should be called with T2ER_PushDataWithDelim

  Scenario: Push event with delimiter (DBUS mode)
    Given event receiver is initialized
    When T2ER_PushDataWithDelim is called with "markerName<#=#>value"
    Then erMutex should be acquired
    And eventInfo should be split by MESSAGE_DELIMITER "<#=#>"
    And T2Event structure should be created with name and value
    And if queue count > T2EVENTQUEUE_MAX_LIMIT (200), event should be dropped
    And otherwise, event should be pushed to eQueue
    And erCond should be signaled to wake dispatch thread
    And erMutex should be released

  Scenario: Push event directly (RBUS mode)
    Given event receiver is initialized
    When T2ER_Push is called with eventName and eventValue
    Then erMutex should be acquired
    And if eventName or eventValue is NULL, error should be logged
    And if queue count > 200, event should be dropped with warning
    And T2Event should be created and pushed to eQueue
    And erCond should be signaled
    And erMutex should be released
    And eventName and eventValue should be freed

  Scenario: Handle event queue overflow
    Given eQueue has 200 events (T2EVENTQUEUE_MAX_LIMIT)
    When a new event is pushed
    Then queue count should be checked
    And event should be dropped
    And warning should be logged with marker name and value
    And T2Event structure should be freed

  Scenario: Handle missing delimiter in event
    Given T2ER_PushDataWithDelim is called
    When eventInfo does not contain "<#=#>" delimiter
    Then strSplit should return NULL
    And error "Missing delimiter in the event received" should be logged
    And event should not be added to queue

  Scenario: Handle missing event value
    Given T2ER_PushDataWithDelim is called
    When eventInfo has marker but no value after delimiter
    Then second strSplit should return NULL
    And error "Missing event value" should be logged
    And event name should be freed
    And event should not be added to queue

  # ============================================================================
  # Event Dispatch Thread
  # ============================================================================

  Scenario: Start event dispatch thread
    Given event receiver is initialized
    When T2ER_StartDispatchThread is called
    Then sTDMutex should be acquired
    And if EREnabled is false, T2ERROR_FAILURE should be returned
    And if stopDispatchThread is false, thread is already running
    And stopDispatchThread should be set to false
    And pthread_create should create T2ER_EventDispatchThread
    And sTDMutex should be released

  Scenario: Event dispatch thread processes events
    Given dispatch thread is running
    And events are in eQueue
    When T2ER_EventDispatchThread executes
    Then erMutex should be acquired
    And event should be popped from eQueue
    And erMutex should be released
    And getMarkerProfileList should be called with event name
    And for each profile in profileList
    Then ReportProfiles_storeMarkerEvent should be called
    And event should be stored in profile's eMarkerList
    And event should be freed after processing

  Scenario: Event dispatch thread waits when queue is empty
    Given dispatch thread is running
    And eQueue is empty
    When T2ER_EventDispatchThread checks queue
    Then pthread_cond_wait should be called on erCond
    And thread should block until erCond is signaled
    And when T2ER_Push signals erCond, thread should wake up
    And thread should check queue again

  Scenario: Handle event with no matching profiles
    Given dispatch thread is processing an event
    When getMarkerProfileList returns no matching profiles
    Then a debug message should be logged
    And event should be freed without storing
    And thread should continue to next event

  Scenario: Handle NULL event from queue
    Given dispatch thread pops an event
    When event pointer is NULL
    Then error should be logged
    And erMutex should be unlocked
    And thread should continue to next event

  Scenario: Stop event dispatch thread
    Given dispatch thread is running
    When T2ER_StopDispatchThread is called
    Then sTDMutex should be acquired
    And if EREnabled is false or stopDispatchThread is true, return failure
    And stopDispatchThread should be set to true
    And erMutex should be acquired
    And erCond should be signaled to wake thread
    And erMutex should be released
    And pthread_detach should be called on erThread
    And flushCacheFromFile should be called
    And sTDMutex should be released

  # ============================================================================
  # Event Receiver Uninitialization
  # ============================================================================

  Scenario: Uninitialize event receiver
    Given event receiver is initialized
    When T2ER_Uninit is called
    Then if not initialized, function should return
    And EREnabled should be set to false
    And if dispatch thread is running, it should be stopped
    And stopDispatchThread should be set to true
    And erCond should be signaled
    And pthread_join should wait for thread termination
    And erMutex should be destroyed
    And sTDMutex should be destroyed
    And erCond should be destroyed
    And eQueue should be destroyed with freeT2Event cleanup
    And eQueue should be set to NULL

  # ============================================================================
  # Thread Safety
  # ============================================================================

  Scenario: Thread-safe event queue access
    Given multiple threads are pushing events
    When T2ER_Push is called concurrently
    Then erMutex should serialize access to eQueue
    And queue operations should be atomic
    And no race conditions should occur

  Scenario: Thread-safe marker list access
    Given multiple components are checking marker lists
    When doPopulateEventMarkerList is called
    Then markerListMutex should protect eventMarkerMap
    And map should be destroyed and recreated safely
    And no race conditions should occur

  Scenario: Thread-safe cache file access
    Given multiple components are caching events
    When cacheEventToFile is called concurrently
    Then FileCacheMutex should protect file operations
    And file locking (fcntl) should prevent corruption
    And each thread should write atomically

  Scenario: Thread-safe event sending
    Given multiple threads are sending events
    When t2_event_s/d/f is called concurrently
    Then sMutex/fMutex/dMutex should serialize event sending
    And eventMutex should protect caching decision
    And no events should be lost or corrupted

  # ============================================================================
  # Error Handling
  # ============================================================================

  Scenario: Handle mutex lock failure in T2ER_Push
    Given event receiver is initialized
    When pthread_mutex_lock fails for erMutex
    Then error should be logged
    And function should return immediately
    And event should not be added to queue

  Scenario: Handle mutex unlock failure in T2ER_Push
    Given event is being pushed
    When pthread_mutex_unlock fails for erMutex
    Then error should be logged
    And function should return
    And mutex may remain in inconsistent state

  Scenario: Handle condition variable signal failure
    Given event is pushed to queue
    When pthread_cond_signal fails for erCond
    Then error should be logged with error code
    And function should continue (non-fatal)
    And dispatch thread may not wake immediately

  Scenario: Handle thread creation failure
    Given T2ER_StartDispatchThread is called
    When pthread_create fails
    Then error should be logged
    And stopDispatchThread should remain locked
    And function should return after unlocking sTDMutex

  Scenario: Handle RBUS connection failure
    Given component tries to send event
    When initMessageBus fails
    Then T2ERROR_FAILURE should be returned
    And event should be cached if caching is enabled
    And error should be logged

  Scenario: Handle rbus_set failure
    Given event is being sent via RBUS
    When rbus_set returns error
    Then error should be logged with error code
    And status -1 should be returned
    And event may be retried or dropped

  # ============================================================================
  # Message Bus Integration
  # ============================================================================

  Scenario: Initialize message bus with RBUS
    Given RBUS is enabled (rbus_checkStatus returns RBUS_ENABLED)
    When initMessageBus is called
    Then unique bus handle name should be created as "t2_lib_{componentName}"
    And rbus_open should be called with bus handle
    And if successful, isRbusEnabled should be set to true
    And T2ERROR_SUCCESS should be returned

  Scenario: Initialize message bus with CCSP
    Given RBUS is not enabled
    And CCSP support is enabled
    When initMessageBus is called
    Then CCSP_Message_Bus_Init should be called
    And bus_handle should be initialized
    And if successful, T2ERROR_SUCCESS should be returned

  Scenario: Get parameter value via RBUS
    Given RBUS is enabled
    When getParamValue is called with parameter name
    Then getRbusParameterVal should be called
    And rbus_get should retrieve parameter value
    And value should be converted to string based on type
    And if type is RBUS_BOOLEAN, "true" or "false" should be returned
    And rbusValue should be released after use

  Scenario: Get parameter value via CCSP
    Given CCSP is enabled
    And RBUS is not enabled
    When getParamValue is called
    Then getCCSPParamVal should be called
    And CcspBaseIf_getParameterValues should retrieve value
    And parameter value should be duplicated and returned
    And valStructs should be freed after use

  # ============================================================================
  # Telemetry Client Utility
  # ============================================================================

  Scenario: Send event from command line script
    Given telemetry_client binary is executed
    When command is run as "telemetry_client <marker> <value>"
    Then t2_init should be called with "telemetry_client"
    And t2_event_s should be called with argv[1] and argv[2]
    And t2_uninit should be called
    And event should be sent to T2 daemon
    And script events should bypass marker filtering

  # ============================================================================
  # Mutex Management
  # ============================================================================

  Scenario: Initialize mutexes on first use
    Given mutexes are not initialized
    When initMutex is called
    Then initMtx should be locked
    And if not initialized, isMutexInitialized should be set to true
    And mutexAttr should be initialized with PTHREAD_MUTEX_RECURSIVE
    And sMutex, fMutex, dMutex, eventMutex should be initialized
    And FileCacheMutex, markerListMutex, loggerMutex should be initialized
    And initMtx should be unlocked

  Scenario: Uninitialize mutexes on cleanup
    Given mutexes are initialized
    When uninitMutex is called
    Then initMtx should be locked
    And if initialized, isMutexInitialized should be set to false
    And all mutexes should be destroyed
    And mutexAttr should be destroyed
    And initMtx should be unlocked

  Scenario: Use recursive mutexes for nested locking
    Given mutexes are initialized with PTHREAD_MUTEX_RECURSIVE
    When same thread locks mutex multiple times
    Then mutex should allow recursive locking
    And thread should not deadlock
    And mutex should be unlocked same number of times

  # ============================================================================
  # Debug Logging
  # ============================================================================

  Scenario: Log debug messages when enabled
    Given ENABLE_DEBUG_FLAG file exists
    When EVENT_DEBUG is called
    Then loggerMutex should be acquired
    And SENDER_LOG_FILE should be opened in append mode
    And timestamp should be formatted as "YYYY-MM-DD HH:MM:SS"
    And formatted message should be written to file
    And file should be closed
    And loggerMutex should be released

  Scenario: Skip debug logging when disabled
    Given ENABLE_DEBUG_FLAG file does not exist
    When EVENT_DEBUG is called
    Then access check should fail
    And function should return immediately
    And no file operations should be performed

  # ============================================================================
  # Integration with Profile System
  # ============================================================================

  Scenario: Store event in profile marker list
    Given dispatch thread receives an event
    And event matches markers in multiple profiles
    When ReportProfiles_storeMarkerEvent is called for each profile
    Then if profile is Xconf, ProfileXConf_storeMarkerEvent should be called
    And if profile is report profile, Profile_storeMarkerEvent should be called
    And event should be added to profile's eMarkerList
    And event should be available for next report generation

  Scenario: Include event markers in report
    Given profile has stored event markers
    When report generation occurs
    Then encodeEventMarkersInJSON should be called
    And each event marker should be encoded as name-value pair
    And events should be included in JSON report
    And events should be cleared after report is sent

  # ============================================================================
  # RFC and Feature Control
  # ============================================================================

  Scenario: Check RFC for T2 enable status
    Given RFC needs to be checked
    When initRFC is called
    Then message bus should be initialized if not already
    And RFC parameter should be queried
    And isRFCT2Enable flag should be set based on RFC value
    And if RFC is disabled, events should not be sent or cached

  Scenario: Handle RFC check failure
    Given message bus initialization fails
    When initRFC is called
    Then error should be logged
    And false should be returned
    And events should be cached as fallback

  # ============================================================================
  # Value Validation and Formatting
  # ============================================================================

  Scenario: Strip trailing newline from string values
    Given t2_event_s is called with value ending in newline
    When value is processed
    Then trailing '\n' should be removed
    And cleaned value should be sent

  Scenario: Reject empty string values
    Given t2_event_s is called with empty string
    When value length is 0
    Then event should not be sent
    And T2ERROR_SUCCESS should be returned (no-op)

  Scenario: Reject zero string values
    Given t2_event_s is called with value "0"
    When value is compared to "0"
    Then event should not be sent
    And T2ERROR_SUCCESS should be returned

  Scenario: Reject zero integer values
    Given t2_event_d is called with value 0
    When value is checked
    Then event should not be sent (field triage requirement)
    And T2ERROR_SUCCESS should be returned
    And debug message should indicate value is 0

  Scenario: Format floating point values
    Given t2_event_f is called with double value
    When value is formatted
    Then snprintf should use "%f" format
    And formatted string should be sent

  Scenario: Format integer values
    Given t2_event_d is called with integer value
    When value is formatted
    Then snprintf should use "%d" format
    And formatted string should be sent
