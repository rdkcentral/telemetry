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

# Source: source/telemetry2_0.c

Feature: Telemetry 2.0 Daemon Core Functionality

  Background:
    Given the telemetry2_0 binary is available on the system
    And the device has network connectivity

  Scenario: Telemetry daemon starts as a background process
    Given no telemetry instance is currently running
    When the telemetry2_0 binary is executed
    Then a child process should be forked
    And the parent process should exit with code 0
    And the child process should create a new session using setsid
    And the working directory should be changed to root
    And the process should run as a daemon

  Scenario: Telemetry daemon prevents multiple instances
    Given a telemetry instance is already running
    And the lock file /var/run/telemetry2_0.lock exists
    When another telemetry2_0 binary execution is attempted
    Then the new instance should fail to acquire the lock
    And the new instance should exit with code 1
    And only one telemetry daemon should remain running

  Scenario: Telemetry daemon initializes core components
    Given the telemetry daemon is starting
    When the initialization sequence begins
    Then the HTTP connection pool should be initialized
    And the report profiles module should be initialized
    And the XConf client should be initialized
    And the DCA report should be generated with delay flag
    And the initialization complete flag should be set

  Scenario: Telemetry daemon handles SIGTERM signal
    Given the telemetry daemon is running
    When a SIGTERM signal is received
    Then the shutdown flag should be set
    And the /tmp/.t2ReadyToReceiveEvents file should be removed
    And the /tmp/telemetry_initialized_bootup file should be removed
    And the T2_CONFIG_READY file should be removed
    And the XConf client should be uninitialized
    And the report profiles should be uninitialized
    And the HTTP pool should be cleaned up
    And the process should exit gracefully

  Scenario: Telemetry daemon handles SIGINT signal
    Given the telemetry daemon is running
    When a SIGINT signal is received
    Then the daemon should perform the same shutdown sequence as SIGTERM

  Scenario: Telemetry daemon handles LOG_UPLOAD signal (signal 10)
    Given the telemetry daemon is running
    And report profiles are configured
    When signal 10 (LOG_UPLOAD) is received
    Then the retain seek map should be set to false
    And the report profiles interrupt should be triggered
    And all running profiles should generate reports immediately

  Scenario: Telemetry daemon handles LOG_UPLOAD_ONDEMAND signal (signal 29)
    Given the telemetry daemon is running
    And report profiles are configured
    When signal 29 (LOG_UPLOAD_ONDEMAND) is received
    Then the report profiles interrupt should be triggered
    And on-demand reports should be generated for all profiles

  Scenario: Telemetry daemon handles EXEC_RELOAD signal (signal 12)
    Given the telemetry daemon is running
    And the XConf client is initialized
    When signal 12 (EXEC_RELOAD) is received
    Then the /tmp/telemetry_logupload file should be created
    And the XConf client should be stopped
    And the XConf client should be restarted
    And the configuration should be reloaded from XConf server

  Scenario: Telemetry daemon signal handler is async-signal-safe
    Given the telemetry daemon is running
    When any signal is received
    Then only signal-safe flags should be set in the handler
    And no mutex operations should occur in the handler
    And no memory allocation should occur in the handler
    And the main loop should poll and dispatch signal actions

  Scenario: Child process handles fatal signals
    Given the telemetry daemon has forked a child process
    When the child process receives a fatal signal
    Then the child process should exit immediately with code 1
    And the parent daemon should continue running
