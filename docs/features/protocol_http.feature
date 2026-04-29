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

# Source: source/protocol/http/multicurlinterface.c, multicurlinterface.h, curlinterface.c, curlinterface.h

Feature: HTTP Protocol - Report Transmission

  Background:
    Given the telemetry daemon is running
    And the HTTP protocol module is initialized

  Scenario: Initialize HTTP connection pool
    Given the HTTP module is starting
    When init_connection_pool is called
    Then a connection pool should be created
    And the pool should be ready for HTTP requests

  Scenario: HTTP GET request via connection pool
    Given the connection pool is initialized
    When http_pool_get is called with URL and response data pointer
    Then an HTTP GET request should be made
    And the response data should be stored in the provided pointer
    And the request status should be returned

  Scenario: HTTP GET request with file output
    Given the connection pool is initialized
    When http_pool_get is called with enable_file_output=true
    Then the response should be written to a file
    And the writeToFile callback should be used

  Scenario: HTTP POST request via connection pool
    Given the connection pool is initialized
    When http_pool_post is called with URL and payload
    Then an HTTP POST request should be made
    And the payload should be sent in the request body
    And the request status should be returned

  Scenario: HTTP connection pool cleanup
    Given the connection pool is initialized
    And connections are active
    When http_pool_cleanup is called
    Then all connections should be closed
    And all pool resources should be freed

  Scenario: Write response to file
    Given an HTTP response is being received
    When writeToFile is called with data chunks
    Then the data should be written to the specified stream
    And the number of bytes written should be returned

  Scenario: Handle HTTP response data
    Given an HTTP request is in progress
    When response data is received in chunks
    Then the curlResponseData structure should accumulate the data
    And the data pointer should be updated
    And the size should be incremented

  Scenario: HTTP request with WAN failover support
    Given WAN_FAILOVER_SUPPORTED is enabled
    When making an HTTP request
    Then the current WAN interface should be determined
    And the request should use the active WAN interface

  Scenario: HTTP request with configurable WAN interface
    Given FEATURE_RDKB_CONFIGURABLE_WAN_INTERFACE is enabled
    When making an HTTP request
    Then the WAN interface should be retrieved from TR181
    And Device.X_RDK_WanManager.CurrentActiveInterface should be queried

  Scenario: curlResponseData structure
    Given HTTP response data is being managed
    Then the curlResponseData structure should contain
      | Field | Type   | Description                    |
      | data  | char*  | Pointer to response data       |
      | size  | size_t | Size of response data          |

  Scenario: Handle HTTP connection errors
    Given an HTTP request is being made
    When the connection fails
    Then an appropriate error code should be returned
    And the error should be logged
    And the connection pool should remain stable

  Scenario: Handle HTTP timeout
    Given an HTTP request is being made
    When the request times out
    Then a timeout error should be returned
    And the request should be marked as failed
    And retry logic should be triggered if configured

  Scenario: HTTP request with mTLS
    Given mTLS is enabled for the device
    When making an HTTP request
    Then client certificates should be used
    And the connection should be mutually authenticated
