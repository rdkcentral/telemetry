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

# Source: source/xconf-client/xconfclient.c, xconfclient.h

Feature: XConf Client - Remote Configuration Management

  Background:
    Given the telemetry daemon is running
    And the device has valid network connectivity
    And the TR181 data model is accessible

  Scenario: XConf client initialization
    Given the XConf client module is not initialized
    When initXConfClient is called
    Then the configuration URL should be retrieved from TR181
    And the client should be ready to fetch remote configuration

  Scenario: XConf client retrieves device metadata for request
    Given the XConf client is initialized
    When preparing a configuration request
    Then the device model should be retrieved from Device.DeviceInfo.ModelName
    And the firmware version should be retrieved from the appropriate TR181 parameter
    And the device uptime should be retrieved from Device.DeviceInfo.UpTime
    And the WAN MAC address should be retrieved
    And the CM MAC address should be retrieved
    And the partner ID should be retrieved from Device.DeviceInfo.X_RDKCENTRAL-COM_Syndication.PartnerId
    And the account ID should be retrieved from Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.AccountInfo.AccountID
    And the build type should be determined

  Scenario: XConf client appends URL-encoded request parameters
    Given the XConf client is preparing a request
    When appendRequestParams is called with a CURLU buffer
    Then all device metadata should be URL-encoded
    And the parameters should be appended to the request URL
    And special characters should be properly escaped

  Scenario: XConf client fetches remote configuration via HTTP GET
    Given the XConf client is initialized
    And a valid configuration URL is available
    When fetchRemoteConfiguration is called
    Then an HTTP GET request should be made to the XConf server
    And the response should contain the configuration data
    And the configuration data should be returned to the caller

  Scenario: XConf client handles HTTP GET failure
    Given the XConf client is initialized
    When the HTTP GET request fails
    Then an appropriate error code should be returned
    And the failure should be logged

  Scenario: XConf client retrieves configuration URL from TR181
    Given the TR181 parameter Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.ConfigURL is set
    When getRemoteConfigURL is called
    Then the configuration URL should be retrieved
    And the URL should be returned to the caller

  Scenario: XConf client handles missing configuration URL
    Given the TR181 configuration URL parameter is not set
    When getRemoteConfigURL is called
    Then an error should be returned indicating missing configuration

  Scenario: XConf client starts periodic configuration polling
    Given the XConf client is initialized
    When startXConfClient is called
    Then the client should begin polling for configuration updates
    And the polling should occur at configured intervals

  Scenario: XConf client stops configuration polling
    Given the XConf client is actively polling
    When stopXConfClient is called
    Then the polling thread should be stopped
    And resources should be released

  Scenario: XConf client uninitializes cleanly
    Given the XConf client is initialized
    When uninitXConfClient is called
    Then all XConf client resources should be freed
    And the client should be marked as uninitialized

  Scenario: XConf client determines build type
    Given the device properties file exists at /etc/device.properties
    When getBuildType is called
    Then the build type should be extracted from device properties
    And the build type should be returned (e.g., dev, prod, vbn)

  Scenario: XConf client supports RDKB device parameters
    Given the device is an RDKB device (ENABLE_RDKB_SUPPORT defined)
    When retrieving device parameters
    Then Device.DeviceInfo.SoftwareVersion should be used for firmware
    And Device.DeviceInfo.X_COMCAST-COM_WAN_MAC should be used for WAN MAC
    And Device.DeviceInfo.X_COMCAST-COM_CM_MAC should be used for CM MAC

  Scenario: XConf client supports RDKC device parameters
    Given the device is an RDKC device (ENABLE_RDKC_SUPPORT defined)
    When retrieving device parameters
    Then Device.DeviceInfo.X_RDK_FirmwareName should be used for firmware
    And Device.DeviceInfo.X_RDKCENTRAL-COM_MAC should be used for MAC addresses

  Scenario: XConf client supports RDK-V device parameters
    Given the device is an RDK-V device (neither RDKB nor RDKC)
    When retrieving device parameters
    Then Device.DeviceInfo.X_COMCAST-COM_FirmwareFilename should be used for firmware
    And Device.DeviceInfo.X_COMCAST-COM_STB_MAC should be used for MAC addresses
