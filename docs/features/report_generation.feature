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

# Source: source/reportgen/reportgen.c, reportgen.h

Feature: Report Generation and Encoding

  Background:
    Given the telemetry daemon is running
    And report profiles are configured

  Scenario: Encode parameter results in JSON
    Given a profile has parameter markers configured
    And parameter values have been collected
    When encodeParamResultInJSON is called with value array and parameter lists
    Then the parameter names should be encoded in the JSON array
    And the parameter values should be encoded in the JSON array
    And the JSON structure should be valid

  Scenario: Encode static parameters in JSON
    Given a profile has static parameters configured
    When encodeStaticParamsInJSON is called with value array and static param list
    Then the static parameters should be encoded in the JSON array
    And the values should be included as configured

  Scenario: Encode grep results in JSON
    Given a profile has grep markers configured
    And grep results have been collected
    When encodeGrepResultInJSON is called with value array and grep marker list
    Then the grep marker names should be encoded
    And the grep values should be encoded
    And the JSON structure should be valid

  Scenario: Encode top results in JSON
    Given a profile has top markers configured
    And top command results have been collected
    When encodeTopResultInJSON is called with value array and top marker list
    Then the top marker names should be encoded
    And the CPU/memory values should be encoded
    And the JSON structure should be valid

  Scenario: Encode event markers in JSON
    Given a profile has event markers configured
    And events have been received
    When encodeEventMarkersInJSON is called with value array and event marker list
    Then the event marker names should be encoded
    And the event values should be encoded
    And the event counts should be included if applicable

  Scenario: Prepare JSON report
    Given a JSON report object has been populated
    When prepareJSONReport is called with the JSON object
    Then the JSON should be serialized to a string
    And the report buffer should contain the serialized JSON
    And the JSON object should remain valid

  Scenario: Destroy JSON report
    Given a JSON report object exists
    When destroyJSONReport is called with the JSON object
    Then the JSON object should be freed
    And all associated memory should be released

  Scenario: Prepare HTTP URL with parameters
    Given a T2HTTP structure is configured with URL and parameters
    When prepareHttpUrl is called with the T2HTTP structure
    Then the base URL should be used
    And request URI parameters should be appended
    And the complete URL should be returned

  Scenario: Tag report as cached
    Given a JSON report string exists
    When tagReportAsCached is called with the report
    Then the report should be marked as a cached report
    And the cached flag should be included in the report

  Scenario: Free profile values
    Given profile value data exists
    When freeProfileValues is called with the data
    Then all profile value memory should be freed

  Scenario: HTTP request parameter structure
    Given an HTTP request is being prepared
    Then the HTTPReqParam structure should contain
      | Field     | Type   | Description              |
      | HttpName  | char*  | Parameter name           |
      | HttpRef   | char*  | Parameter reference      |
      | HttpValue | char*  | Parameter value          |

  Scenario: RBUS method parameter structure
    Given an RBUS method call is being prepared
    Then the RBUSMethodParam structure should contain
      | Field | Type   | Description      |
      | name  | char*  | Parameter name   |
      | value | char*  | Parameter value  |

  Scenario: HTTP method types supported
    Given a profile is configured with HTTP protocol
    Then the following HTTP methods should be supported
      | Method   |
      | HTTP_PUT |
      | HTTP_POST|

  Scenario: T2HTTP destination structure
    Given a profile is configured with HTTP destination
    Then the T2HTTP structure should contain
      | Field              | Type    | Description                    |
      | URL                | char*   | Destination URL                |
      | Compression        | HTTPComp| Compression type               |
      | Method             | HTTPMethod| HTTP method (PUT/POST)       |
      | RequestURIparamList| Vector* | List of URI parameters         |

  Scenario: T2RBUS destination structure
    Given a profile is configured with RBUS destination
    Then the T2RBUS structure should contain
      | Field              | Type    | Description                    |
      | rbusMethodName     | char*   | RBUS method name               |
      | rbusMethodParamList| Vector* | List of method parameters      |
