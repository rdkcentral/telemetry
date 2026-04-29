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

# Source: source/t2parser/t2parser.c, t2parser.h, t2parserxconf.c, t2parserxconf.h

Feature: Profile Configuration Parsing

  Background:
    Given the telemetry daemon is running
    And the parser module is available

  Scenario: Parse JSON configuration
    Given a JSON configuration string is provided
    When processConfiguration is called with config data, profile name, and hash
    Then the JSON should be parsed
    And a Profile structure should be created
    And all profile fields should be populated from the JSON

  Scenario: Parse MessagePack configuration
    Given a MessagePack configuration blob is provided
    When processMsgPackConfiguration is called with the msgpack object
    Then the MessagePack should be deserialized
    And a Profile structure should be created
    And all profile fields should be populated from the MessagePack

  Scenario: Get map value from MessagePack object
    Given a MessagePack map object exists
    When msgpack_get_map_value is called with the object and key
    Then the value for the key should be returned
    And NULL should be returned if the key doesn't exist

  Scenario: Get array element from MessagePack object
    Given a MessagePack array object exists
    When msgpack_get_array_element is called with the object and index
    Then the element at the index should be returned
    And NULL should be returned if the index is out of bounds

  Scenario: Duplicate string from MessagePack object
    Given a MessagePack string object exists
    When msgpack_strdup is called with the object
    Then a new string should be allocated
    And the string content should be copied
    And the caller should own the new string

  Scenario: Print MessagePack object for debugging
    Given a MessagePack object exists
    When msgpack_print is called with the object and name
    Then the object should be printed for debugging purposes
    And the object name should be included in the output

  Scenario: Compare MessagePack string with C string
    Given a MessagePack string object exists
    When msgpack_strcmp is called with the object and a C string
    Then the comparison result should be returned
    And 0 should be returned if strings are equal

  Scenario: Parse profile with required fields
    Given a configuration contains required fields
    When the configuration is parsed
    Then the following required fields should be extracted
      | Field            | Description                    |
      | name             | Profile name                   |
      | hash             | Unique profile identifier      |
      | protocol         | Communication protocol         |
      | encodingType     | Report encoding type           |
      | reportingInterval| Reporting interval in seconds  |

  Scenario: Parse profile with optional fields
    Given a configuration contains optional fields
    When the configuration is parsed
    Then the following optional fields should be extracted if present
      | Field                   | Default     | Description                    |
      | version                 | empty       | Profile version                |
      | Description             | empty       | Profile description            |
      | activationTimeoutPeriod | 0           | Activation timeout             |
      | firstReportingInterval  | 0           | First reporting interval       |
      | maxUploadLatency        | 0           | Maximum upload latency         |
      | generateNow             | false       | Generate report immediately    |
      | deleteonTimeout         | false       | Delete on timeout              |
      | reportOnUpdate          | false       | Report on update               |

  Scenario: Parse profile with event markers
    Given a configuration contains event markers
    When the configuration is parsed
    Then each event marker should be extracted
    And the marker name should be stored
    And the marker component should be stored
    And the use type (count/accumulate) should be stored

  Scenario: Parse profile with grep markers
    Given a configuration contains grep markers
    When the configuration is parsed
    Then each grep marker should be extracted
    And the search string should be stored
    And the log file path should be stored
    And the use type (count/absolute) should be stored
    And the regex pattern should be stored if present

  Scenario: Parse profile with datamodel markers
    Given a configuration contains datamodel (TR181) markers
    When the configuration is parsed
    Then each datamodel marker should be extracted
    And the TR181 parameter reference should be stored
    And the method (get/subscribe) should be stored

  Scenario: Parse profile with trigger conditions
    Given a configuration contains trigger conditions
    When the configuration is parsed
    Then each trigger condition should be extracted
    And the reference parameter should be stored
    And the operator should be stored
    And the threshold value should be stored

  Scenario: Parse profile with HTTP destination
    Given a configuration specifies HTTP protocol
    When the configuration is parsed
    Then the HTTP URL should be extracted
    And the HTTP method should be determined
    And request URI parameters should be extracted

  Scenario: Parse profile with RBUS_METHOD destination
    Given a configuration specifies RBUS_METHOD protocol
    When the configuration is parsed
    Then the RBUS method name should be extracted
    And RBUS method parameters should be extracted

  Scenario: Handle invalid configuration
    Given an invalid configuration is provided
    When the configuration is parsed
    Then an error should be returned
    And no profile should be created
    And the error should be logged

  Scenario: Handle missing required fields
    Given a configuration is missing required fields
    When the configuration is parsed
    Then an error should be returned
    And the missing field should be identified in the error

  Scenario: Parse XConf-specific configuration
    Given an XConf configuration response is received
    When the XConf parser processes the response
    Then profiles should be extracted from the response
    And each profile should be passed to the main parser
