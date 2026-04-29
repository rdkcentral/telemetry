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

# Source: source/utils/t2common.c, t2common.h, t2collection.c, t2collection.h, vector.c, vector.h

Feature: Telemetry Utility Functions

  Background:
    Given the telemetry daemon is running

  # Vector Operations (source/utils/vector.c, vector.h)

  Scenario: Create a new vector
    Given a vector is needed for data storage
    When a new Vector is created
    Then the vector should be initialized with default capacity
    And the vector should be ready to store elements

  Scenario: Add element to vector
    Given a vector exists
    When an element is added to the vector
    Then the element should be stored in the vector
    And the vector count should be incremented

  Scenario: Get element from vector by index
    Given a vector contains elements
    When an element is retrieved by index
    Then the element at the specified index should be returned
    And NULL should be returned for invalid indices

  Scenario: Remove element from vector
    Given a vector contains elements
    When an element is removed from the vector
    Then the element should be removed
    And the vector count should be decremented

  Scenario: Vector auto-resize
    Given a vector is at capacity
    When a new element is added
    Then the vector should automatically resize
    And the new element should be stored

  # Collection Operations (source/utils/t2collection.c, t2collection.h)

  Scenario: Create hash map
    Given a hash map is needed
    When a new hash_map is created
    Then the hash map should be initialized
    And the map should be ready for key-value storage

  Scenario: Insert into hash map
    Given a hash map exists
    When a key-value pair is inserted
    Then the value should be stored with the key
    And the value should be retrievable by key

  Scenario: Retrieve from hash map
    Given a hash map contains entries
    When a value is retrieved by key
    Then the correct value should be returned
    And NULL should be returned for non-existent keys

  Scenario: Remove from hash map
    Given a hash map contains an entry
    When the entry is removed by key
    Then the entry should be removed from the map
    And subsequent lookups should return NULL

  Scenario: Destroy hash map
    Given a hash map exists with entries
    When the hash map is destroyed
    Then all entries should be freed
    And the map structure should be freed

  # Common Utilities (source/utils/t2common.c, t2common.h)

  Scenario: Initialize whoami support
    Given the telemetry daemon is starting
    When initWhoamiSupport is called
    Then device identification should be initialized
    And device info should be available for reports

  Scenario: Get current timestamp
    Given a timestamp is needed
    When the timestamp function is called
    Then the current time should be returned
    And the format should match the configured timestamp format

  Scenario: String duplication utility
    Given a string needs to be duplicated
    When the string duplication function is called
    Then a new string should be allocated
    And the content should be copied
    And the caller should own the new string

  Scenario: Safe string copy
    Given a string needs to be copied to a buffer
    When the safe string copy function is called
    Then the string should be copied up to the buffer size
    And the destination should be null-terminated
    And buffer overflow should be prevented

  # mTLS Utilities (source/utils/t2MtlsUtils.c, t2MtlsUtils.h)

  Scenario: Check mTLS configuration
    Given mTLS may be configured
    When the mTLS check function is called
    Then the mTLS status should be determined
    And the result should indicate if mTLS is enabled

  Scenario: Get mTLS certificate paths
    Given mTLS is enabled
    When certificate paths are requested
    Then the client certificate path should be returned
    And the private key path should be returned
    And the CA certificate path should be returned

  # Logging Utilities (source/utils/t2log_wrapper.c, t2log_wrapper.h)

  Scenario: Initialize logging
    Given the telemetry daemon is starting
    When LOGInit is called
    Then the logging system should be initialized
    And log output should be configured

  Scenario: Log debug message
    Given logging is initialized
    When T2Debug is called with a message
    Then the message should be logged at debug level
    And the function name should be included

  Scenario: Log info message
    Given logging is initialized
    When T2Info is called with a message
    Then the message should be logged at info level

  Scenario: Log warning message
    Given logging is initialized
    When T2Warning is called with a message
    Then the message should be logged at warning level

  Scenario: Log error message
    Given logging is initialized
    When T2Error is called with a message
    Then the message should be logged at error level
    And the message should be prominently displayed

  Scenario: Conditional logging based on RDK logger
    Given RDK_LOGGER is defined
    When logging functions are called
    Then logs should be routed to RDK logger
    And log levels should be mapped appropriately
