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


Feature: Telemetry multiprofile configuration and report generation

  Scenario: Multiprofile configuration with empty name or hash
    Given When the telemetry daemon is already running
    When a multiprofile is configured with an empty/NULL name or hash value
    Then the multiprofile should not be set and enabled

  Scenario: Multiprofile configuration without name or hash field
    Given When the telemetry daemon is already running
    When a multiprofile is configured without a name/hash field
    Then the multiprofile should not be set and enabled

  Scenario: Multiprofile configuration without name or hash field
    Given When the telemetry daemon is already running
    When a multiprofile is configured without a name/hash field
    Then the multiprofile should not be set and enabled
