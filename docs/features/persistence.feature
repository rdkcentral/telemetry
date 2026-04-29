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

# Source: source/utils/persistence.c, persistence.h

Feature: Persistent Storage Management

  Background:
    Given the telemetry daemon is running
    And the persistence module is available

  Scenario: Fetch local configurations from disk
    Given configuration files exist in the persistence path
    When fetchLocalConfigs is called with the path and config list
    Then all configuration files should be read
    And configurations should be added to the config list
    And the Config structures should contain name and configData

  Scenario: Save configuration to file
    Given a profile configuration needs to be persisted
    When saveConfigToFile is called with path, profile name, and configuration
    Then the configuration should be written to disk
    And the file should be named after the profile
    And the file should be readable on next startup

  Scenario: Save cached report to persistence folder
    Given a report failed to send
    When saveCachedReportToPersistenceFolder is called with profile name and report list
    Then the reports should be saved to the cached messages path
    And the reports should be retrievable later

  Scenario: Populate cached report list from disk
    Given cached reports exist on disk for a profile
    When populateCachedReportList is called with profile name and output list
    Then the cached reports should be read from disk
    And the reports should be added to the output list

  Scenario: Clear persistence folder
    Given files exist in a persistence folder
    When clearPersistenceFolder is called with the path
    Then all files in the folder should be deleted
    And the folder should be empty

  Scenario: Remove profile from disk
    Given a profile configuration exists on disk
    When removeProfileFromDisk is called with path and profile name
    Then the profile configuration file should be deleted
    And associated cached reports should be removed

  Scenario: Save MessagePack configuration
    Given a MessagePack configuration blob is ready
    When MsgPackSaveConfig is called with path, filename, blob, and size
    Then the MessagePack blob should be written to disk
    And the file should be binary format

  Scenario: Get privacy mode from persistent folder
    Given privacy mode settings exist on disk
    When getPrivacyModeFromPersistentFolder is called
    Then the privacy mode data should be read
    And the data should be returned to the caller

  Scenario: Save privacy mode to persistent folder
    Given privacy mode settings need to be persisted
    When savePrivacyModeToPersistentFolder is called with data
    Then the privacy mode should be saved to PRIVACYMODE_PATH
    And the settings should survive daemon restarts

  Scenario: Persistence paths for RDKB devices
    Given the device is an RDKB device (ENABLE_RDKB_SUPPORT defined)
    Then PERSISTENCE_PATH should be /nvram
    And XCONFPROFILE_PERSISTENCE_PATH should be /nvram/.t2persistentfolder/
    And REPORTPROFILES_PERSISTENCE_PATH should be /nvram/.t2reportprofiles/

  Scenario: Persistence paths for Extender devices
    Given the device is an Extender device (DEVICE_EXTENDER defined)
    Then PERSISTENCE_PATH should be /mnt/data/pstore
    And XCONFPROFILE_PERSISTENCE_PATH should be empty
    And REPORTPROFILES_PERSISTENCE_PATH should be /mnt/data/pstore/.t2reportprofiles/

  Scenario: Persistence paths for other devices
    Given the device is not RDKB or Extender
    Then PERSISTENCE_PATH should be /opt
    And XCONFPROFILE_PERSISTENCE_PATH should be /opt/.t2persistentfolder/
    And REPORTPROFILES_PERSISTENCE_PATH should be /opt/.t2reportprofiles/

  Scenario: Short-lived profiles path
    Given a temporary profile is being stored
    Then SHORTLIVED_PROFILES_PATH should be /tmp/t2reportprofiles/
    And temporary profiles should not persist across reboots

  Scenario: Cached messages path
    Given reports need to be cached
    Then CACHED_MESSAGE_PATH should be PERSISTENCE_PATH/.t2cachedmessages/
    And cached messages should persist across daemon restarts

  Scenario: Seek map persistence path
    Given PERSIST_LOG_MON_REF is enabled
    Then SEEKFOLDER should be PERSISTENCE_PATH/.t2seekmap/
    And seek positions should be persisted for log processing

  Scenario: Config structure
    Given configuration data is being managed
    Then the Config structure should contain
      | Field      | Type   | Description              |
      | name       | char*  | Configuration name       |
      | configData | char*  | Configuration content    |
