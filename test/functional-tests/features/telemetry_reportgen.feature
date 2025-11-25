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


Feature: Telemetry Report Generation - JSON Encoding and HTTP URL Preparation

  Background:
    Given the telemetry daemon is running
    And profiles are configured with parameters and markers

  # ============================================================================
  # Empty String Validation
  # ============================================================================

  Scenario: Check for empty string with NULL value
    Given a parameter value is NULL
    When checkForEmptyString is called
    Then the function should return true
    And the value should be considered empty

  Scenario: Check for empty string with zero length
    Given a parameter value has strlen < 1
    When checkForEmptyString is called
    Then the function should return true
    And the value should be considered empty

  Scenario: Check for empty string with single space
    Given a parameter value is " " (single space)
    When checkForEmptyString is called
    Then the function should return true
    And the value should be considered empty

  Scenario: Check for empty string with "NULL" literal
    Given a parameter value is "NULL" string
    When checkForEmptyString is called
    Then the function should return true
    And rbusInterface explicitly adds "NULL" for non-existent parameters
    And the value should be considered empty

  Scenario: Check for non-empty string
    Given a parameter value has valid content
    When checkForEmptyString is called
    Then the function should return false
    And the value should be considered valid

  # ============================================================================
  # Memory Management for Parameter Values
  # ============================================================================

  Scenario: Free parameter value structures
    Given tr181ValStruct_t array with multiple entries
    When freeParamValueSt is called
    Then each parameterName should be freed
    And each parameterValue should be freed
    And each valStruct should be freed
    And the array itself should be freed
    And all pointers should be set to NULL

  Scenario: Free profile values structure
    Given a profileValues structure with paramValues
    When freeProfileValues is called
    Then freeParamValueSt should be called with paramValues and paramValueCount
    And the profileValues structure should be freed
    And no memory leaks should occur

  Scenario: Handle NULL in freeParamValueSt
    Given valStructs parameter is NULL
    When freeParamValueSt is called
    Then the function should return immediately
    And no segmentation fault should occur

  Scenario: Handle zero valSize in freeParamValueSt
    Given valSize is 0
    When freeParamValueSt is called
    Then no iteration should occur
    And the array should be freed

  # ============================================================================
  # Vector to JSON Conversion
  # ============================================================================

  Scenario: Convert Vector to JSON array
    Given a Vector containing string values
    When convertVectorToJson is called
    Then a cJSON array should be created if output is NULL
    And each Vector element should be added as cJSON string
    And cJSON_AddItemToArray should be called for each element
    And the JSON array should contain all Vector values

  Scenario: Handle NULL output in convertVectorToJson
    Given output parameter is NULL
    When convertVectorToJson is called
    Then cJSON_CreateArray should be called
    And if creation fails, error should be logged
    And function should return without crash

  Scenario: Convert empty Vector to JSON
    Given a Vector with size 0
    When convertVectorToJson is called
    Then an empty JSON array should be created
    And no items should be added

  # ============================================================================
  # JSON Report Lifecycle
  # ============================================================================

  Scenario: Destroy JSON report object
    Given a valid cJSON report object
    When destroyJSONReport is called
    Then cJSON_Delete should be called on the object
    And T2ERROR_SUCCESS should be returned
    And all JSON memory should be freed

  Scenario: Handle NULL in destroyJSONReport
    Given jsonObj parameter is NULL
    When destroyJSONReport is called
    Then error "jsonObj Argument is NULL" should be logged
    And T2ERROR_INVALID_ARGS should be returned
    And no crash should occur

  # ============================================================================
  # String Trimming
  # ============================================================================

  Scenario: Trim leading whitespace from string
    Given a string with leading spaces and tabs
    When trimLeadingAndTrailingws is called
    Then leading spaces should be removed
    And leading tabs should be removed
    And first non-space character should be at index 0

  Scenario: Trim trailing whitespace from string
    Given a string with trailing spaces and tabs
    When trimLeadingAndTrailingws is called
    Then trailing spaces should be removed
    And trailing tabs should be removed
    And string should end at last non-space character

  Scenario: Trim both leading and trailing whitespace
    Given a string with both leading and trailing whitespace
    When trimLeadingAndTrailingws is called
    Then both leading and trailing whitespace should be removed
    And only content between first and last non-space characters should remain

  Scenario: Handle string with no whitespace
    Given a string with no leading or trailing whitespace
    When trimLeadingAndTrailingws is called
    Then the string should remain unchanged
    And no characters should be removed

  # ============================================================================
  # TR-181 Parameter Encoding in JSON
  # ============================================================================

  Scenario: Validate input parameters for encodeParamResultInJSON
    Given encodeParamResultInJSON is called
    When valArray is NULL
    Then error "Invalid or NULL arguments" should be logged
    And T2ERROR_INVALID_ARGS should be returned

  Scenario: Validate paramNameList parameter
    Given encodeParamResultInJSON is called
    When paramNameList is NULL
    Then error "Invalid or NULL arguments" should be logged
    And T2ERROR_INVALID_ARGS should be returned

  Scenario: Validate paramValueList parameter
    Given encodeParamResultInJSON is called
    When paramValueList is NULL
    Then error "Invalid or NULL arguments" should be logged
    And T2ERROR_INVALID_ARGS should be returned

  Scenario: Encode single TR-181 parameter with value
    Given a Param with name and single value
    And paramValueCount is 1
    And parameter value is not empty
    When encodeParamResultInJSON is called
    Then cJSON_CreateObject should create arrayItem
    And cJSON_AddStringToObject should add parameter name and value
    And cJSON_AddItemToArray should add arrayItem to valArray
    And T2ERROR_SUCCESS should be returned

  Scenario: Handle NULL parameter in paramNameList
    Given a Param entry is NULL in paramNameList
    When encodeParamResultInJSON processes the entry
    Then the entry should be skipped with continue
    And no JSON object should be created for NULL param
    And processing should continue with next parameter

  Scenario: Handle NULL paramValues in paramValueList
    Given paramValues is NULL for a parameter
    When encodeParamResultInJSON processes the entry
    Then the entry should be skipped with continue
    And no JSON object should be created
    And processing should continue with next parameter

  Scenario: Encode parameter with zero paramValCount and reportEmptyParam true
    Given a parameter has paramValCount = 0
    And param->reportEmptyParam is true
    When encodeParamResultInJSON is called
    Then cJSON_CreateObject should create arrayItem
    And cJSON_AddStringToObject should add parameter name with "NULL" value
    And info "Paramter was not successfully retrieved" should be logged
    And arrayItem should be added to valArray

  Scenario: Skip parameter with zero paramValCount and reportEmptyParam false
    Given a parameter has paramValCount = 0
    And param->reportEmptyParam is false
    When encodeParamResultInJSON is called
    Then the parameter should be skipped with continue
    And no JSON object should be created

  Scenario: Encode single parameter with empty string and reportEmptyParam true
    Given a parameter has single value
    And checkForEmptyString returns true
    And param->reportEmptyParam is true
    When encodeParamResultInJSON is called
    Then the parameter should be encoded in JSON
    And the empty value should be included

  Scenario: Skip single parameter with empty string and reportEmptyParam false
    Given a parameter has single value
    And checkForEmptyString returns true
    And param->reportEmptyParam is false
    When encodeParamResultInJSON is called
    Then the parameter should be skipped
    And no JSON object should be created

  Scenario: Apply trim parameter to single value
    Given a parameter has trimParam set to true
    And parameter has single value with whitespace
    When encodeParamResultInJSON is called
    Then trimLeadingAndTrailingws should be called on parameterValue
    And whitespace should be removed before encoding
    And trimmed value should be added to JSON

  Scenario: Apply regex parameter to single value with match
    Given a parameter has regexParam set
    And parameter has single value
    When encodeParamResultInJSON is called
    Then regcomp should compile the regex pattern with REG_EXTENDED
    And regexec should execute pattern matching
    And if match is found, matched substring should be extracted
    And original value should be freed
    And matched substring should be set as new value
    And regfree should release regex resources

  Scenario: Apply regex parameter to single value with no match
    Given a parameter has regexParam set
    And parameter has single value
    And regex pattern does not match
    When encodeParamResultInJSON is called
    Then regexec should return non-zero
    And warning "Failed to match" should be logged
    And original value should be freed
    And empty string should be set as new value

  Scenario: Handle regcomp failure for single value
    Given a parameter has invalid regexParam
    When encodeParamResultInJSON is called
    Then regcomp should return non-zero
    And warning "regcomp() failed" should be logged
    And regex processing should be skipped
    And original value should be used

  Scenario: Encode multiple TR-181 parameter values (table)
    Given a parameter has paramValCount > 1
    When encodeParamResultInJSON is called
    Then cJSON_CreateObject should create arrayItem
    And cJSON_CreateArray should create valList
    And cJSON_AddItemToObject should add valList to arrayItem with param name
    And each paramValue should be processed in loop

  Scenario: Process table entry with valid value
    Given a table parameter has multiple values
    And a table entry has non-empty value
    When processing table entries
    Then cJSON_CreateObject should create valItem
    And cJSON_AddStringToObject should add parameterName and parameterValue
    And cJSON_AddItemToArray should add valItem to valList
    And isTableEmpty should be set to false

  Scenario: Apply trim parameter to table values
    Given a table parameter has trimParam set to true
    And table has multiple values with whitespace
    When processing table entries
    Then trimLeadingAndTrailingws should be called on each value
    And whitespace should be removed before encoding

  Scenario: Apply regex parameter to table values
    Given a table parameter has regexParam set
    And table has multiple values
    When processing table entries
    Then regex should be compiled once
    And regexec should be called for each table value
    And matched substrings should be extracted
    And regfree should be called after all values processed

  Scenario: Handle empty table with reportEmptyParam
    Given a table parameter has all empty values
    And param->reportEmptyParam is false
    When processing table entries
    Then isTableEmpty should remain true
    And arrayItem should be freed with cJSON_free
    And table should not be added to valArray

  Scenario: Add non-empty table to report
    Given a table parameter has at least one non-empty value
    When processing completes
    Then isTableEmpty should be false
    And cJSON_AddItemToArray should add arrayItem to valArray
    And table should be included in report

  Scenario: Handle cJSON_CreateObject failure in single value
    Given cJSON_CreateObject returns NULL
    When encoding single parameter value
    Then error "cJSON_CreateObject failed" should be logged
    And T2ERROR_FAILURE should be returned
    And processing should stop

  Scenario: Handle cJSON_AddStringToObject failure in single value
    Given cJSON_AddStringToObject returns NULL
    When adding parameter to JSON
    Then error "cJSON_AddStringToObject failed" should be logged
    And arrayItem should be deleted with cJSON_Delete
    And T2ERROR_FAILURE should be returned

  Scenario: Handle cJSON_CreateObject failure in table
    Given cJSON_CreateObject returns NULL for arrayItem
    When encoding table parameter
    Then error "cJSON_CreateObject failed" should be logged
    And T2ERROR_FAILURE should be returned

  Scenario: Handle cJSON_CreateObject failure for table entry
    Given cJSON_CreateObject returns NULL for valItem
    When processing table entry
    Then error "cJSON_CreateObject failed" should be logged
    And arrayItem should be deleted
    And T2ERROR_FAILURE should be returned

  Scenario: Handle cJSON_AddStringToObject failure in table entry
    Given cJSON_AddStringToObject returns NULL for table entry
    When adding table entry to JSON
    Then error "cJSON_AddStringToObject failed" should be logged
    And both arrayItem and valItem should be deleted
    And T2ERROR_FAILURE should be returned

  # ============================================================================
  # Static Parameter Encoding in JSON
  # ============================================================================

  Scenario: Validate input parameters for encodeStaticParamsInJSON
    Given encodeStaticParamsInJSON is called
    When valArray is NULL
    Then error "Invalid or NULL Arguments" should be logged
    And T2ERROR_INVALID_ARGS should be returned

  Scenario: Validate staticParamList parameter
    Given encodeStaticParamsInJSON is called
    When staticParamList is NULL
    Then error "Invalid or NULL Arguments" should be logged
    And T2ERROR_INVALID_ARGS should be returned

  Scenario: Encode static parameter with name and value
    Given a StaticParam with name and value
    When encodeStaticParamsInJSON is called
    Then cJSON_CreateObject should create arrayItem
    And cJSON_AddStringToObject should add name and value
    And cJSON_AddItemToArray should add arrayItem to valArray
    And T2ERROR_SUCCESS should be returned

  Scenario: Skip static parameter with NULL name
    Given a StaticParam has NULL name
    When encodeStaticParamsInJSON processes the entry
    Then the entry should be skipped with continue
    And no JSON object should be created

  Scenario: Skip static parameter with NULL value
    Given a StaticParam has NULL value
    When encodeStaticParamsInJSON processes the entry
    Then the entry should be skipped with continue
    And no JSON object should be created

  Scenario: Handle cJSON_CreateObject failure for static param
    Given cJSON_CreateObject returns NULL
    When encoding static parameter
    Then error "cJSON_CreateObject failed" should be logged
    And T2ERROR_FAILURE should be returned

  Scenario: Handle cJSON_AddStringToObject failure for static param
    Given cJSON_AddStringToObject returns NULL
    When adding static parameter to JSON
    Then error "cJSON_AddStringToObject failed" should be logged
    And arrayItem should be deleted
    And T2ERROR_FAILURE should be returned

  # ============================================================================
  # Grep Result Encoding in JSON
  # ============================================================================

  Scenario: Validate input parameters for encodeGrepResultInJSON
    Given encodeGrepResultInJSON is called
    When valArray is NULL
    Then error "Invalid or NULL Arguments" should be logged
    And T2ERROR_INVALID_ARGS should be returned

  Scenario: Validate grepResult parameter
    Given encodeGrepResultInJSON is called
    When grepResult is NULL
    Then error "Invalid or NULL Arguments" should be logged
    And T2ERROR_INVALID_ARGS should be returned

  Scenario: Encode grep result with marker name and value
    Given a GrepResult with markerName and markerValue
    When encodeGrepResultInJSON is called
    Then cJSON_CreateObject should create arrayItem
    And cJSON_AddStringToObject should add markerName and markerValue
    And cJSON_AddItemToArray should add arrayItem to valArray
    And T2ERROR_SUCCESS should be returned

  Scenario: Skip grep result with NULL markerName
    Given a GrepResult has NULL markerName
    When encodeGrepResultInJSON processes the entry
    Then the entry should be skipped with continue
    And no JSON object should be created

  Scenario: Skip grep result with NULL markerValue
    Given a GrepResult has NULL markerValue
    When encodeGrepResultInJSON processes the entry
    Then the entry should be skipped with continue
    And no JSON object should be created

  Scenario: Apply trim parameter to grep result
    Given a GrepResult has trimParameter set to true
    And markerValue has whitespace
    When encodeGrepResultInJSON is called
    Then trimLeadingAndTrailingws should be called on markerValue
    And whitespace should be removed before encoding

  Scenario: Apply regex parameter to grep result with match
    Given a GrepResult has regexParameter set
    When encodeGrepResultInJSON is called
    Then regcomp should compile the regex pattern with REG_EXTENDED
    And regexec should execute pattern matching
    And if match is found, matched substring should be extracted
    And original markerValue should be freed
    And matched substring should be set as new markerValue
    And regfree should release regex resources

  Scenario: Apply regex parameter to grep result with no match
    Given a GrepResult has regexParameter set
    And regex pattern does not match markerValue
    When encodeGrepResultInJSON is called
    Then regexec should return non-zero
    And warning "Failed to match" should be logged
    And original markerValue should be freed
    And empty string should be set as new markerValue

  Scenario: Handle regcomp failure for grep result
    Given a GrepResult has invalid regexParameter
    When encodeGrepResultInJSON is called
    Then regcomp should return non-zero
    And warning "regcomp() failed" should be logged
    And regex processing should be skipped

  Scenario: Handle cJSON_CreateObject failure for grep result
    Given cJSON_CreateObject returns NULL
    When encoding grep result
    Then error "cJSON_CreateObject failed" should be logged
    And T2ERROR_FAILURE should be returned

  Scenario: Handle cJSON_AddStringToObject failure for grep result
    Given cJSON_AddStringToObject returns NULL
    When adding grep result to JSON
    Then error "cJSON_AddStringToObject failed" should be logged
    And arrayItem should be deleted
    And T2ERROR_FAILURE should be returned
  # ============================================================================
  # Event Marker Encoding in JSON
  # ============================================================================

  Scenario: Validate input parameters for encodeEventMarkersInJSON
    Given encodeEventMarkersInJSON is called
    When valArray is NULL
    Then error "Invalid or NULL Arguments" should be logged
    And T2ERROR_INVALID_ARGS should be returned

  Scenario: Validate eventMarkerList parameter
    Given encodeEventMarkersInJSON is called
    When eventMarkerList is NULL
    Then error "Invalid or NULL Arguments" should be logged
    And T2ERROR_INVALID_ARGS should be returned

  Scenario: Encode COUNTER type event marker with positive count
    Given an EventMarker with mType = MTYPE_COUNTER
    And u.count > 0
    When encodeEventMarkersInJSON is called
    Then count should be converted to string with sprintf
    And cJSON_CreateObject should create arrayItem
    And cJSON_AddStringToObject should add marker name and count string
    And cJSON_AddItemToArray should add arrayItem to valArray
    And u.count should be reset to 0

  Scenario: Skip COUNTER type event marker with zero count
    Given an EventMarker with mType = MTYPE_COUNTER
    And u.count = 0
    When encodeEventMarkersInJSON is called
    Then the marker should be skipped
    And no JSON object should be created
    And u.count should remain 0

  Scenario: Use alias for COUNTER event marker if available
    Given an EventMarker with mType = MTYPE_COUNTER
    And alias is set
    When encodeEventMarkersInJSON is called
    Then cJSON_AddStringToObject should use alias instead of markerName
    And alias should be the key in JSON

  Scenario: Use markerName for COUNTER event marker if no alias
    Given an EventMarker with mType = MTYPE_COUNTER
    And alias is NULL
    When encodeEventMarkersInJSON is called
    Then cJSON_AddStringToObject should use markerName
    And markerName should be the key in JSON

  Scenario: Add timestamp for COUNTER with REPORTTIMESTAMP_UNIXEPOCH
    Given an EventMarker with mType = MTYPE_COUNTER
    And reportTimestampParam = REPORTTIMESTAMP_UNIXEPOCH
    When encodeEventMarkersInJSON is called
    Then cJSON_AddStringToObject should add markerName_CT with timestamp
    And timestamp should be included in JSON

  Scenario: Apply trim parameter to COUNTER value
    Given an EventMarker with mType = MTYPE_COUNTER
    And trimParam is true
    When encodeEventMarkersInJSON is called
    Then trimLeadingAndTrailingws should be called on stringValue
    And whitespace should be removed from count string

  Scenario: Apply regex parameter to COUNTER value with match
    Given an EventMarker with mType = MTYPE_COUNTER
    And regexParam is set
    When encodeEventMarkersInJSON is called
    Then regcomp should compile the regex pattern
    And regexec should execute on stringValue
    And matched substring should be extracted
    And regfree should release resources

  Scenario: Apply regex parameter to COUNTER value with no match
    Given an EventMarker with mType = MTYPE_COUNTER
    And regexParam is set
    And regex does not match
    When encodeEventMarkersInJSON is called
    Then warning "Failed to match" should be logged
    And empty string should be set as stringValue

  Scenario: Encode ACCUMULATE type event marker with values
    Given an EventMarker with mType = MTYPE_ACCUMULATE
    And u.accumulatedValues is not NULL
    And Vector_Size(u.accumulatedValues) > 0
    When encodeEventMarkersInJSON is called
    Then cJSON_CreateObject should create arrayItem
    And cJSON_CreateArray should create vectorToarray
    And convertVectorToJson should convert accumulated values
    And cJSON_AddItemToObject should add vectorToarray with marker name
    And Vector_Clear should clear u.accumulatedValues with freeAccumulatedParam

  Scenario: Skip ACCUMULATE type event marker with NULL values
    Given an EventMarker with mType = MTYPE_ACCUMULATE
    And u.accumulatedValues is NULL
    When encodeEventMarkersInJSON is called
    Then the marker should be skipped
    And no JSON object should be created

  Scenario: Skip ACCUMULATE type event marker with empty Vector
    Given an EventMarker with mType = MTYPE_ACCUMULATE
    And Vector_Size(u.accumulatedValues) = 0
    When encodeEventMarkersInJSON is called
    Then the marker should be skipped
    And no JSON object should be created

  Scenario: Apply trim parameter to ACCUMULATE values
    Given an EventMarker with mType = MTYPE_ACCUMULATE
    And trimParam is true
    When encodeEventMarkersInJSON is called
    Then trimLeadingAndTrailingws should be called on each accumulated value
    And whitespace should be removed from all values

  Scenario: Apply regex parameter to ACCUMULATE values
    Given an EventMarker with mType = MTYPE_ACCUMULATE
    And regexParam is set
    When encodeEventMarkersInJSON is called
    Then regcomp should compile the regex pattern once
    And Vector_Create should create regaccumulateValues
    And regexec should be called for each accumulated value
    And matched substrings should be pushed to regaccumulateValues
    And regfree should release resources
    And convertVectorToJson should use regaccumulateValues
    And regaccumulateValues should be cleared and freed

  Scenario: Handle "maximum accumulation reached" in regex processing
    Given an EventMarker with mType = MTYPE_ACCUMULATE
    And regexParam is set
    And a value is "maximum accumulation reached"
    When processing accumulated values
    Then the value should be preserved as-is
    And regex matching should be skipped for this value

  Scenario: Use alias for ACCUMULATE event marker if available
    Given an EventMarker with mType = MTYPE_ACCUMULATE
    And alias is set
    When encodeEventMarkersInJSON is called
    Then cJSON_AddItemToObject should use alias
    And alias should be the key for the array

  Scenario: Use markerName for ACCUMULATE event marker if no alias
    Given an EventMarker with mType = MTYPE_ACCUMULATE
    And alias is NULL
    When encodeEventMarkersInJSON is called
    Then cJSON_AddItemToObject should use markerName
    And markerName should be the key for the array

  Scenario: Add timestamp array for ACCUMULATE with REPORTTIMESTAMP_UNIXEPOCH
    Given an EventMarker with mType = MTYPE_ACCUMULATE
    And reportTimestampParam = REPORTTIMESTAMP_UNIXEPOCH
    When encodeEventMarkersInJSON is called
    Then cJSON_CreateArray should create TimevectorToarray
    And convertVectorToJson should convert accumulatedTimestamp
    And Vector_Clear should clear accumulatedTimestamp
    And cJSON_AddItemToObject should add TimevectorToarray with markerName_CT

  Scenario: Encode ABSOLUTE type event marker with value
    Given an EventMarker with mType = MTYPE_ABSOLUTE
    And u.markerValue is not NULL
    When encodeEventMarkersInJSON is called
    Then cJSON_CreateObject should create arrayItem
    And cJSON_AddStringToObject should add marker name and markerValue
    And cJSON_AddItemToArray should add arrayItem to valArray
    And u.markerValue should be freed
    And u.markerValue should be set to NULL

  Scenario: Skip ABSOLUTE type event marker with NULL value
    Given an EventMarker with mType = MTYPE_ABSOLUTE
    And u.markerValue is NULL
    When encodeEventMarkersInJSON is called
    Then the marker should be skipped
    And no JSON object should be created

  Scenario: Apply trim parameter to ABSOLUTE value
    Given an EventMarker with mType = MTYPE_ABSOLUTE
    And trimParam is true
    When encodeEventMarkersInJSON is called
    Then trimLeadingAndTrailingws should be called on u.markerValue
    And whitespace should be removed

  Scenario: Apply regex parameter to ABSOLUTE value with match
    Given an EventMarker with mType = MTYPE_ABSOLUTE
    And regexParam is set
    When encodeEventMarkersInJSON is called
    Then regcomp should compile the regex pattern
    And regexec should execute on u.markerValue
    And matched substring should be extracted
    And original u.markerValue should be freed
    And matched substring should be set as new u.markerValue
    And regfree should release resources

  Scenario: Apply regex parameter to ABSOLUTE value with no match
    Given an EventMarker with mType = MTYPE_ABSOLUTE
    And regexParam is set
    And regex does not match
    When encodeEventMarkersInJSON is called
    Then warning "Failed to match" should be logged
    And original u.markerValue should be freed
    And empty string should be set as new u.markerValue

  Scenario: Use alias for ABSOLUTE event marker if available
    Given an EventMarker with mType = MTYPE_ABSOLUTE
    And alias is set
    When encodeEventMarkersInJSON is called
    Then cJSON_AddStringToObject should use alias
    And alias should be the key in JSON

  Scenario: Use markerName for ABSOLUTE event marker if no alias
    Given an EventMarker with mType = MTYPE_ABSOLUTE
    And alias is NULL
    When encodeEventMarkersInJSON is called
    Then cJSON_AddStringToObject should use markerName
    And markerName should be the key in JSON

  Scenario: Add timestamp for ABSOLUTE with REPORTTIMESTAMP_UNIXEPOCH
    Given an EventMarker with mType = MTYPE_ABSOLUTE
    And reportTimestampParam = REPORTTIMESTAMP_UNIXEPOCH
    When encodeEventMarkersInJSON is called
    Then cJSON_AddStringToObject should add markerName_CT with timestamp
    And timestamp should be included in JSON

  Scenario: Handle cJSON_CreateObject failure for COUNTER
    Given cJSON_CreateObject returns NULL for COUNTER marker
    When encoding COUNTER event marker
    Then error "cJSON_CreateObject failed" should be logged
    And T2ERROR_FAILURE should be returned

  Scenario: Handle cJSON_AddStringToObject failure for COUNTER
    Given cJSON_AddStringToObject returns NULL for COUNTER marker
    When adding COUNTER to JSON
    Then error "cJSON_AddStringToObject failed" should be logged
    And arrayItem should be deleted
    And T2ERROR_FAILURE should be returned

  Scenario: Handle cJSON_CreateObject failure for ACCUMULATE
    Given cJSON_CreateObject returns NULL for ACCUMULATE marker
    When encoding ACCUMULATE event marker
    Then error "cJSON_CreateObject failed" should be logged
    And T2ERROR_FAILURE should be returned

  Scenario: Handle cJSON_CreateArray failure for ACCUMULATE
    Given cJSON_CreateArray returns NULL for vectorToarray
    When encoding ACCUMULATE event marker
    Then error "cJSON_CreateArray failed" should be logged
    And arrayItem should be deleted
    And T2ERROR_FAILURE should be returned

  Scenario: Handle cJSON_CreateArray failure for ACCUMULATE timestamp
    Given cJSON_CreateArray returns NULL for TimevectorToarray
    When adding timestamp array for ACCUMULATE
    Then error "cJSON_CreateArray failed" should be logged
    And arrayItem should be deleted
    And regaccumulateValues should be cleared and freed if exists
    And T2ERROR_FAILURE should be returned

  Scenario: Handle cJSON_CreateObject failure for ABSOLUTE
    Given cJSON_CreateObject returns NULL for ABSOLUTE marker
    When encoding ABSOLUTE event marker
    Then error "cJSON_CreateObject failed" should be logged
    And T2ERROR_FAILURE should be returned

  Scenario: Handle cJSON_AddStringToObject failure for ABSOLUTE
    Given cJSON_AddStringToObject returns NULL for ABSOLUTE marker
    When adding ABSOLUTE to JSON
    Then error "cJSON_AddStringToObject failed" should be logged
    And arrayItem should be deleted
    And T2ERROR_FAILURE should be returned

  # ============================================================================
  # JSON Report Preparation
  # ============================================================================

  Scenario: Prepare JSON report from cJSON object
    Given a valid cJSON report object
    When prepareJSONReport is called
    Then cJSON_PrintUnformatted should be called
    And reportBuff should be set to unformatted JSON string
    And T2ERROR_SUCCESS should be returned

  Scenario: Handle NULL jsonObj in prepareJSONReport
    Given jsonObj parameter is NULL
    When prepareJSONReport is called
    Then error "jsonObj is NULL" should be logged
    And T2ERROR_INVALID_ARGS should be returned

  Scenario: Handle cJSON_PrintUnformatted failure
    Given cJSON_PrintUnformatted returns NULL
    When prepareJSONReport is called
    Then error "Failed to get unformatted json" should be logged
    And T2ERROR_FAILURE should be returned

  # ============================================================================
  # HTTP URL Preparation with Request Parameters
  # ============================================================================

  Scenario: Prepare HTTP URL with NULL T2HTTP parameter
    Given http parameter is NULL
    When prepareHttpUrl is called
    Then error "Http parameters are NULL" should be logged
    And NULL should be returned

  Scenario: Handle curl_easy_init failure in prepareHttpUrl
    Given curl_easy_init returns NULL
    When prepareHttpUrl is called
    Then error "Curl init failed Response is NULL" should be logged
    And NULL should be returned

  Scenario: Prepare HTTP URL with base URL only
    Given a T2HTTP structure with URL
    And RequestURIparamList is NULL or empty
    When prepareHttpUrl is called
    Then httpUrl should be set to strdup of http->URL
    And no parameters should be appended
    And curl_easy_cleanup should be called
    And httpUrl should be returned

  Scenario: Prepare HTTP URL with static request parameters
    Given a T2HTTP structure with URL
    And RequestURIparamList contains HTTPReqParam entries
    And HTTPReqParam has HttpValue (static parameter)
    When prepareHttpUrl is called
    Then curl_easy_escape should encode HttpValue
    And parameter should be formatted as "HttpName=encodedValue"
    And parameters should be joined with "&"
    And URL should be appended with "?" and parameters

  Scenario: Prepare HTTP URL with dynamic request parameters
    Given a T2HTTP structure with URL
    And HTTPReqParam has HttpRef (dynamic parameter)
    And HttpValue is NULL
    When prepareHttpUrl is called
    Then getParameterValue should be called with HttpRef
    And if successful, parameter value should be retrieved
    And curl_easy_escape should encode the retrieved value
    And parameter should be formatted as "HttpName=encodedValue"

  Scenario: Handle getParameterValue failure for dynamic parameter
    Given HTTPReqParam has HttpRef
    And getParameterValue returns T2ERROR_FAILURE
    When processing dynamic parameter
    Then error "Failed to retrieve param" should be logged
    And parameter should be skipped with continue
    And processing should continue with next parameter

  Scenario: Handle empty parameter value for dynamic parameter
    Given HTTPReqParam has HttpRef
    And getParameterValue succeeds but returns empty string
    When processing dynamic parameter
    Then paramValue should be freed
    And error "Param value is empty" should be logged
    And parameter should be skipped with continue

  Scenario: Build URL parameters string with multiple parameters
    Given RequestURIparamList has multiple HTTPReqParam entries
    When prepareHttpUrl is called
    Then url_params should be allocated with realloc
    And each parameter should be appended as "name=value&"
    And params_len should track total length
    And memory should be reallocated as needed

  Scenario: Handle realloc failure for url_params
    Given realloc returns NULL for url_params
    When building URL parameters
    Then error "Unable to allocate memory" should be logged
    And httpParamVal should be freed with curl_free
    And parameter should be skipped with continue

  Scenario: Finalize URL with parameters
    Given url_params is built with parameters
    When all parameters are processed
    Then last "&" should be replaced with null terminator
    And modified_url_len should be calculated
    And httpUrl should be reallocated to fit parameters
    And "?" and url_params should be appended to httpUrl
    And url_params should be freed

  Scenario: Free curl-escaped parameter values
    Given curl_easy_escape allocated httpParamVal
    When parameter is processed
    Then curl_free should be called on httpParamVal
    And memory should be released properly

  Scenario: Cleanup curl handle after URL preparation
    Given curl handle was initialized
    When prepareHttpUrl completes
    Then curl_easy_cleanup should be called
    And curl resources should be released
    And modified URL should be logged

  Scenario: Log default and modified URLs
    Given prepareHttpUrl is processing URL
    When URL is prepared
    Then default URL should be logged with T2Debug
    And modified URL with parameters should be logged
    And both URLs should be visible for debugging
