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


Feature: Telemetry DCA Grep Marker Processing and Log File Management

  Background:
    Given the telemetry daemon is running with DCA utility enabled
    And log files are available in the configured log path

  # ============================================================================
  # Log File Seek Position Management
  # ============================================================================

  Scenario: Initialize seek position for new log file
    Given a grep marker is configured for a log file that has not been processed before
    When the DCA utility processes the log file for the first time
    Then the seek position should be initialized to 0 for that log file
    And the seek position should be stored in the profile's seek map
    And subsequent reads should start from the last processed position

  Scenario: Maintain seek position across multiple profile executions
    Given a grep marker has been processed and seek position is stored
    When the profile is executed again in the next reporting interval
    Then the DCA utility should retrieve the stored seek position
    And processing should continue from the last seek position
    And only new log entries since last execution should be processed
    And the seek position should be updated after processing

  Scenario: Handle seek position when log file grows
    Given a log file has been processed with seek position at end of file
    When new log entries are appended to the log file
    And the profile is executed again
    Then the DCA utility should detect the file has grown
    And processing should start from the stored seek position
    And only the newly appended log entries should be processed
    And the new seek position should be stored

  Scenario: Reset seek position when log file is truncated or recreated
    Given a log file has been processed with seek position stored
    When the log file is truncated or recreated with smaller size
    And the stored seek position is greater than the current file size
    Then the DCA utility should detect the file size mismatch
    And the seek position should be reset to 0 for broadband devices
    And for other device types, rotated log files should be checked
    And processing should handle the file rotation scenario appropriately

  Scenario: Seek position management with firstSeekFromEOF parameter
    Given a grep marker is configured with firstSeekFromEOF parameter set to N bytes
    When the log file is processed for the first time
    Then the seek position should be calculated as (fileSize - N) bytes from start
    And if N is greater than fileSize, seek position should be set to 0
    And the firstSeekFromEOF parameter should be reset to 0 after first use
    And subsequent executions should use normal seek position tracking

  # ============================================================================
  # Log File Rotation Handling
  # ============================================================================

  Scenario: Detect and process rotated log files on first execution
    Given a grep marker is configured with check_rotated_logs flag enabled
    When the profile is executed for the first time after bootup
    And the current log file has a rotated version (e.g., logfile.1)
    Then the DCA utility should detect the rotated log file
    And processing should start from the rotated log file first
    And after processing rotated log, it should continue with current log file
    And the check_rotated_logs flag should be disabled for subsequent executions

  Scenario: Handle log file rotation with .0 extension
    Given a log file with .0 extension (e.g., messages.txt.0)
    When log rotation occurs and creates messages.txt.1
    And the stored seek position exceeds the current file size
    Then the DCA utility should detect the .0 extension pattern
    And it should construct rotated filename by changing .0 to .1
    And processing should continue from the rotated file
    And the is_rotated_log flag should be set to track rotation state

  Scenario: Handle log file rotation with standard extension
    Given a log file without .0 extension (e.g., application.log)
    When log rotation occurs and creates application.log.1
    And the stored seek position exceeds the current file size
    Then the DCA utility should append .1 extension to construct rotated filename
    And processing should continue from the rotated file at stored seek position
    And after EOF in rotated file, processing should switch to current log file
    And the is_rotated_log flag should be cleared after switching

  Scenario: Seamless transition from rotated to current log file
    Given processing is ongoing in a rotated log file (e.g., logfile.1)
    When end of file is reached in the rotated log
    Then the DCA utility should automatically switch to current log file (logfile)
    And processing should start from beginning of current log file
    And the is_rotated_log flag should be cleared
    And seek position should be updated for the current log file

  Scenario: Handle missing rotated log files gracefully
    Given a log file rotation is detected based on seek position
    When the DCA utility attempts to open the rotated log file
    And the rotated log file does not exist or cannot be opened
    Then the DCA utility should log the error
    And processing should fall back to the current log file
    And seek position should be reset appropriately
    And no crash or data loss should occur

  # ============================================================================
  # Grep Marker Pattern Matching
  # ============================================================================

  Scenario: Process grep marker with count type (OCCURENCE)
    Given a grep marker is configured with marker type as count
    When the DCA utility processes log files
    And multiple log lines match the search pattern
    Then the count of matching lines should be incremented
    And the final count should be added to the grep result list
    And the marker name and count value should be included in the report

  Scenario: Process grep marker with string type (STR)
    Given a grep marker is configured with marker type as string
    When the DCA utility processes log files
    And a log line matches the search pattern
    Then the matched string value should be extracted
    And the marker name and string value should be added to grep result list
    And the value should be included in the telemetry report

  Scenario: Handle grep marker with split parameter extraction
    Given a grep marker is configured with split parameter enabled
    When a log line matches the search pattern
    Then the content after the search pattern should be extracted
    And if trim parameter is enabled, leading and trailing spaces should be removed
    And if regex parameter is provided, only regex-matching content should be extracted
    And the extracted value should be stored in the marker data field

  Scenario: Validate split parameter value extraction
    Given a log line contains the search pattern followed by value
    When getSplitParameterValue function is called
    Then the value after the pattern should be extracted correctly
    And single character values that are whitespace should be ignored
    And the extracted value should not exceed MAXLINE buffer size
    And boundary safety should be ensured with null termination

  Scenario: Handle empty or whitespace-only split parameter values
    Given a log line matches the search pattern
    When the content after the pattern is only whitespace
    Then the split parameter extraction should return 0 (no value)
    And no data should be stored for that marker
    And the marker should not be included in the report

  # ============================================================================
  # RDK Error Code Processing
  # ============================================================================

  Scenario: Detect and count RDK error codes in log files
    Given log files contain RDK error codes in format "RDK-XXXXX"
    When the DCA utility processes log lines
    And a line contains "RDK-" followed by valid error code pattern
    Then the error code should be extracted using getErrorCode function
    And if the error code already exists in rdkec_head list, count should be incremented
    And if the error code is new, a new node should be created with count 1
    And all RDK error codes should be included in the telemetry report

  Scenario: Validate RDK error code format
    Given a log line contains "RDK-" prefix
    When getErrorCode function processes the line
    Then it should validate the error code starts with 0 or 1
    And the second digit should be 0 or 3
    And subsequent characters should be digits
    And the error code length should not exceed RDK_EC_MAXLEN (5 characters)
    And only valid RDK error codes should be extracted

  Scenario: Handle invalid RDK error code patterns
    Given a log line contains "RDK-" but with invalid format
    When getErrorCode function processes the line
    Then the function should return -1 indicating no valid error code
    And no error code node should be created
    And processing should continue with next log lines

  # ============================================================================
  # TR-181 Parameter Processing via Message Bus
  # ============================================================================

  Scenario: Process single instance TR-181 parameter
    Given a grep marker is configured with pattern as TR-181 parameter without {i} token
    When the DCA utility processes the marker with logfile as "<message_bus>"
    Then getParameterValue should be called for the TR-181 parameter
    And if successful, the parameter value should be appended to marker data
    And the marker should be added to grep result list or JSON output

  Scenario: Process multi-instance TR-181 parameter
    Given a grep marker is configured with pattern containing {i} token (e.g., "Device.WiFi.AccessPoint.{i}.SSID")
    When the DCA utility processes the marker
    Then it should query NumberOfEntries for the multi-instance object
    And for each instance from 1 to NumberOfEntries
    Then the {i} token should be replaced with instance number
    And getParameterValue should be called for each instance
    And all instance values should be appended to marker data with comma separation

  Scenario: Handle TR-181 parameter not found
    Given a grep marker is configured with TR-181 parameter pattern
    When getParameterValue is called and parameter does not exist
    Then T2ERROR_FAILURE should be returned
    And a debug message should be logged indicating data source not found
    And no data should be appended to the marker
    And the marker should not be included in the report

  Scenario: Validate multi-instance TR-181 parameter format
    Given a grep marker contains {i} token in the pattern
    When the pattern is validated for multi-instance processing
    Then only one {i} token should be present in the pattern
    And if multiple {i} tokens are found, the pattern should be skipped
    And a debug message should be logged about invalid format
    And no processing should occur for invalid multi-instance patterns

  # ============================================================================
  # Top Command Output Processing
  # ============================================================================

  Scenario: Process Load Average from top command output
    Given a grep marker is configured with header containing "Load_Average"
    And logfile is specified as "top_log.txt"
    When the DCA utility processes the marker
    Then getLoadAvg function should be called
    And load average values should be extracted from top output
    And the values should be added to grep result list
    And trim and regex parameters should be applied if configured

  Scenario: Process specific process usage from top command output
    Given a grep marker is configured with process name pattern
    And logfile is specified as "top_log.txt"
    When the DCA utility processes the marker
    Then saveTopOutput should be called to capture current top data
    And getProcUsage should be called with the process name pattern
    And CPU and memory usage for the process should be extracted
    And the usage data should be added to grep result list
    And removeTopOutput should be called after processing all markers

  Scenario: Thread-safe top output processing
    Given multiple profiles are processing top_log.txt simultaneously
    When saveTopOutput is called
    Then topOutputMutex should be acquired before saving top data
    And the mutex should be held during getProcUsage processing
    And the mutex should be released after processing
    And no race conditions should occur in top data access

  # ============================================================================
  # Profile and Execution Counter Management
  # ============================================================================

  Scenario: Initialize execution counter for new profile
    Given a new profile with grep markers is configured
    When the profile is executed for the first time
    Then a GrepSeekProfile should be created for the profile
    And the execution counter should be initialized to 0
    And the counter should be incremented after each execution
    And the counter should be stored in profileExecCountMap

  Scenario: Maintain execution counter across profile lifecycle
    Given a profile has been executed multiple times
    When the profile is removed from active profiles
    Then the execution counter should be saved to profileExecCountMap
    And when the profile is re-enabled with same name
    Then the execution counter should be restored from profileExecCountMap
    And execution should continue with the restored counter value

  Scenario: Handle skip frequency with execution counter
    Given a grep marker is configured with skipFreq parameter set to N
    When the profile execution counter modulo (N+1) equals 0
    Then the marker should be processed normally
    And when the modulo does not equal 0
    Then the marker processing should be skipped
    And seek values should still be updated for log file markers
    And the execution counter should be incremented regardless

  # ============================================================================
  # Memory Management and Resource Cleanup
  # ============================================================================

  Scenario: Proper cleanup of grep result list
    Given grep results have been collected in a vector
    When the results are added to the report
    Then each GrepResult structure should be properly allocated
    And markerName and markerValue should be duplicated strings
    And after processing, freeGResult should be called to free memory
    And no memory leaks should occur

  Scenario: Cleanup of profile seek map on profile removal
    Given a profile has an associated GrepSeekProfile with seek map
    When the profile is removed
    Then removeProfileFromSeekMap should be called
    And the execution counter should be saved before cleanup
    And the logFileSeekMap should be destroyed with freeLogFileSeekMap
    And the GrepSeekProfile structure should be freed
    And all associated memory should be released

  Scenario: Thread-safe access to profile seek map
    Given multiple threads are accessing profileSeekMap
    When adding or retrieving profile seek data
    Then pSeekLock mutex should be acquired before access
    And the mutex should be held during map operations
    And the mutex should be released after operations complete
    And no race conditions should occur in seek map access

  Scenario: Handle buffer overflow in count values
    Given a grep marker count exceeds 9999
    When the count is converted to string for reporting
    Then the count should be capped and set to INVALID_COUNT (-406)
    And a debug message should be logged about buffer overflow
    And the INVALID_COUNT value should be reported for metrics collection
    And no buffer overflow should occur in string conversion

  # ============================================================================
  # File Handle Management
  # ============================================================================

  Scenario: Reuse file handle for same log file
    Given a log file is being processed
    When multiple grep markers reference the same log file
    Then the file handle (pcurrentLogFile) should be reused
    And the file should not be reopened for each marker
    And seek position should be maintained across marker processing
    And the file should be closed only when switching to different log file

  Scenario: Close and reopen file handle on log file switch
    Given processing is ongoing for logfile A
    When the next marker references logfile B
    Then the seek position for logfile A should be updated
    And the file handle for logfile A should be closed
    And a new file handle should be opened for logfile B
    And processing should continue with logfile B

  Scenario: Handle file open failures gracefully
    Given a grep marker references a log file
    When fopen fails to open the log file
    Then NULL should be returned from getLogLine
    And LAST_SEEK_VALUE should be set to 0
    And appropriate error logging should occur
    And processing should continue with next markers
    And no crash should occur due to NULL file handle

  # ============================================================================
  # Absolute Path Handling
  # ============================================================================

  Scenario: Process log file with absolute path
    Given a grep marker specifies log file with absolute path (starts with /)
    When the DCA utility constructs the full file path
    Then LOG_PATH should not be prepended to the filename
    And the absolute path should be used as-is
    And the file should be opened using the absolute path
    And processing should work correctly with absolute paths

  Scenario: Process log file with relative path
    Given a grep marker specifies log file with relative path
    When the DCA utility constructs the full file path
    Then LOG_PATH should be prepended to the filename
    And the full path should be constructed as LOG_PATH + filename
    And the file should be opened using the constructed path
    And processing should work correctly with relative paths

  # ============================================================================
  # Edge Cases and Error Handling
  # ============================================================================

  Scenario: Handle NULL or empty input parameters
    Given DCA utility functions are called with NULL parameters
    When getLogLine is called with NULL logSeekMap or buf
    Then the function should return NULL immediately
    And appropriate debug messages should be logged
    And no segmentation fault or crash should occur

  Scenario: Handle empty search pattern or log file name
    Given a grep marker has empty search pattern or log file name
    When the marker is processed in parseMarkerList
    Then the marker should be skipped with continue statement
    And no processing should occur for the marker
    And execution should continue with next markers

  Scenario: Handle SNMP log file type
    Given a grep marker specifies log file type as "snmp" (case-insensitive)
    When the marker is processed
    Then the marker should be skipped with continue statement
    And no SNMP processing should occur (not supported in this context)
    And execution should continue with next markers

  Scenario: Handle insufficient memory allocation
    Given memory allocation fails during string duplication or buffer allocation
    When strdup or malloc returns NULL
    Then appropriate error messages should be logged
    And the function should handle the failure gracefully
    And processing should continue or terminate safely
    And no use of NULL pointers should occur

  # ============================================================================
  # Concurrent Profile Processing
  # ============================================================================

  Scenario: Serialize grep result processing across profiles
    Given multiple profiles are attempting to get grep results simultaneously
    When getDCAResultsInVector is called by multiple threads
    Then dcaMutex should be acquired before processing
    And only one profile should process grep results at a time
    And the mutex should be released after processing completes
    And no synchronization issues should occur

  Scenario: Initialize properties once for all profiles
    Given multiple profiles are being processed
    When isPropsInitialized returns false
    Then initProperties should be called once with logPath and persistentPath
    And subsequent calls should skip initialization
    And all profiles should use the same initialized properties
    And no redundant initialization should occur

  Scenario: Support custom log path per profile
    Given a profile specifies a custom log path
    When getDCAResultsInVector is called with customLogPath parameter
    Then initProperties should be called with the custom log path
    And the profile should use the custom path for log file access
    And other profiles should not be affected by the custom path
    And the custom path should be used only for that specific profile execution

  # ============================================================================
  # Data Type Determination
  # ============================================================================

  Scenario: Determine data type for count marker
    Given a grep marker has MarkerType set to MTYPE_COUNTER
    And the log file is not "top_log.txt" or "<message_bus>"
    When getDType function is called
    Then the data type should be set to OCCURENCE
    And the marker should track count of pattern occurrences

  Scenario: Determine data type for string marker
    Given a grep marker has MarkerType not set to MTYPE_COUNTER
    When getDType function is called
    Then the data type should be set to STR
    And the marker should capture string values

  Scenario: Determine data type for special log files
    Given a grep marker references "top_log.txt" or "<message_bus>"
    When getDType function is called
    Then the data type should always be set to STR regardless of MarkerType
    And the marker should capture string values from top or TR-181 data

  # ============================================================================
  # Integration with Telemetry Reporting
  # ============================================================================

  Scenario: Add grep results to JSON output (legacy mode)
    Given grep results have been collected
    When grepResultList parameter is NULL in processing functions
    Then addToJson should be called instead of addToVector
    And results should be added to SEARCH_RESULT_JSON
    And the JSON output should be returned to caller

  Scenario: Add grep results to Vector output (current mode)
    Given grep results have been collected
    When grepResultList parameter is not NULL in processing functions
    Then addToVector should be called
    And results should be added to the provided Vector
    And each result should be a GrepResult structure
    And the Vector should be returned to caller for further processing

  Scenario: Handle zero count or empty string values
    Given a grep marker has count of 0 or data value of "0"
    When adding results to JSON or Vector
    Then the marker should be skipped and not added to results
    And only non-zero counts and non-"0" string values should be reported
    And this prevents reporting of markers with no meaningful data
