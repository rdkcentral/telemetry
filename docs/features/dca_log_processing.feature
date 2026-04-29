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

# Source: source/dcautil/dca.c, dca.h, dcautil.c, dcautil.h

Feature: DCA Log Processing and Pattern Matching

  Background:
    Given the telemetry daemon is running
    And log files are available for processing

  Scenario: Initialize DCA properties
    Given the DCA module is starting
    When T2InitProperties is called
    Then the DCA properties should be initialized from device.properties
    And log file paths should be configured

  Scenario: Get DCA results in vector
    Given a grep seek profile is configured
    And marker list is provided
    When getDCAResultsInVector is called
    Then log files should be searched for markers
    And matching results should be added to the vector
    And the result count should be returned

  Scenario: Process grep markers with count use type
    Given a grep marker is configured with use type "count"
    When the log file is processed
    Then the number of occurrences should be counted
    And the count should be stored as the marker value

  Scenario: Process grep markers with absolute use type
    Given a grep marker is configured with use type "absolute"
    When the log file is processed
    Then the content after the search string should be extracted
    And the extracted content should be stored as the marker value

  Scenario: Process grep markers with regex
    Given a grep marker is configured with a regex pattern
    When the log file is processed
    Then the regex should be applied to filter results
    And only matching content should be extracted

  Scenario: Process grep markers with trim
    Given a grep marker is configured with trim enabled
    When the log file is processed
    Then whitespace should be trimmed from the result
    And the trimmed value should be stored

  Scenario: Process top command patterns
    Given a profile has top markers configured
    When processTopPattern is called with profile name and marker list
    Then the top command output should be processed
    And CPU and memory usage should be extracted
    And the results should be stored in the marker list

  Scenario: Handle log file rotation during processing
    Given a log file is being processed
    When the log file is rotated during processing
    Then the rotated file should also be checked
    And data from both files should be collected
    And no data should be lost

  Scenario: Get grep results with seek map
    Given a grep seek profile exists
    When getGrepResults is called with the profile
    Then the seek map should be used to track file positions
    And only new content since last seek should be processed
    And the seek map should be updated after processing

  Scenario: Clear seek map on request
    Given a grep seek profile with seek map exists
    When getGrepResults is called with isClearSeekMap=true
    Then the seek map should be cleared
    And the entire file should be processed from the beginning

  Scenario: Save grep configuration
    Given a profile with grep markers is configured
    When saveGrepConfig is called with profile name and marker list
    Then the grep configuration should be saved
    And the configuration should be retrievable later

  Scenario: Remove grep configuration
    Given a grep configuration exists for a profile
    When removeGrepConfig is called with profile name
    Then the grep configuration should be removed
    And associated seek maps should be cleared if requested

  Scenario: Process previous logs folder
    Given the device has logs in /opt/logs/PreviousLogs/
    When a profile is configured to check previous logs
    Then the PreviousLogs folder should be searched
    And markers should be matched against previous log content

  Scenario: Get process CPU information
    Given a process name is specified
    When getCPUInfo is called with process info structure
    Then the CPU usage for the process should be calculated
    And the CPU percentage should be stored

  Scenario: Get process memory information
    Given a process name is specified
    When getMemInfo is called with process info structure
    Then the memory usage for the process should be retrieved
    And the memory value should be stored

  Scenario: Get process PID statistics
    Given a process ID is specified
    When getProcPidStat is called with the PID
    Then the process statistics should be retrieved
    And user time, system time, and RSS should be populated

  Scenario: Get total CPU times
    Given the system is running
    When getTotalCpuTimes is called
    Then the total CPU time should be calculated
    And the value should be returned

  Scenario: Save seek configuration to file
    Given PERSIST_LOG_MON_REF is enabled
    And a grep seek profile exists
    When saveSeekConfigtoFile is called
    Then the seek configuration should be persisted to disk
    And the configuration should survive daemon restarts

  Scenario: Load saved seek configuration
    Given PERSIST_LOG_MON_REF is enabled
    And a saved seek configuration exists
    When loadSavedSeekConfig is called
    Then the seek configuration should be loaded from disk
    And the profile should resume from the saved position

  Scenario: Check first boot status
    Given PERSIST_LOG_MON_REF is enabled
    When firstBootStatus is called
    Then the first boot status should be determined
    And appropriate seek behavior should be applied

  Scenario: Flag DCA report completion
    Given a DCA report is being generated
    When dcaFlagReportCompleation is called
    Then the DCA done flag should be set at /tmp/.dca_done

  Scenario: GrepResult structure
    Given grep results are being processed
    Then the GrepResult structure should contain
      | Field          | Type        | Description                    |
      | markerName     | const char* | Name of the marker             |
      | markerValue    | const char* | Value extracted from log       |
      | trimParameter  | bool        | Whether to trim whitespace     |
      | regexParameter | char*       | Regex pattern for filtering    |

  Scenario: Process memory and CPU info structure
    Given process information is being collected
    Then the procMemCpuInfo structure should contain
      | Field          | Type   | Description                    |
      | pid            | pid_t* | Process ID array               |
      | processName    | char[] | Name of the process            |
      | cpuUse         | char[] | CPU usage string               |
      | memUse         | char[] | Memory usage string            |
      | total_instance | int    | Total instances of process     |

  Scenario: Memory-mapped file processing with bounds checking
    Given a log file is being memory-mapped for processing
    When the file is accessed
    Then bounds checking should be performed
    And file locking should prevent race conditions
    And safe cleanup should occur after processing
