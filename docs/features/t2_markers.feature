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

# Source: source/bulkdata/t2markers.c, t2markers.h

Feature: Telemetry Marker Management

  Background:
    Given the telemetry daemon is running
    And the marker component map is initialized

  Scenario: Initialize T2 marker component map
    Given the marker map is not initialized
    When initT2MarkerComponentMap is called
    Then the marker-to-component hash map should be created
    And the map should be ready to store marker registrations

  Scenario: Destroy T2 marker component map
    Given the marker map is initialized
    And markers are registered
    When destroyT2MarkerComponentMap is called
    Then all marker entries should be freed
    And the hash map should be destroyed

  Scenario: Clear T2 marker component map
    Given the marker map contains entries
    When clearT2MarkerComponentMap is called
    Then all marker entries should be removed
    And the map should be empty but still usable

  Scenario: Update event map with marker
    Given a T2Marker structure is prepared
    When updateEventMap is called with marker name and T2Marker
    Then the marker should be added to the event map
    And the marker should be retrievable by name

  Scenario: Add T2 event marker
    Given a profile requires an event marker
    When addT2EventMarker is called with marker name, component, profile, and skip frequency
    Then the marker should be registered
    And the marker should be associated with the component
    And the marker should be linked to the profile

  Scenario: Get component marker list
    Given markers are registered for a component
    When getComponentMarkerList is called with component name
    Then the list of markers for the component should be returned
    And the marker list should be usable for event routing

  Scenario: Get components with event markers
    Given event markers are registered
    When getComponentsWithEventMarkers is called
    Then a vector of component names should be returned
    And only components with event markers should be included

  Scenario: Get marker profile list
    Given a marker is registered with multiple profiles
    When getMarkerProfileList is called with the marker name
    Then the list of profiles using the marker should be returned
    And all associated profiles should be included

  Scenario: Create component data elements
    Given markers are registered with components
    When createComponentDataElements is called
    Then data elements should be created for RBUS registration
    And components should be ready to receive events

  Scenario: T2Marker structure
    Given a marker is being registered
    Then the T2Marker structure should contain
      | Field         | Type    | Description                    |
      | markerName    | char*   | Name of the marker             |
      | componentName | char*   | Component that sends the event |
      | profileList   | Vector* | List of profiles using marker  |

  Scenario: Maximum event marker name length
    Given an event marker is being registered
    Then the marker name should not exceed MAX_EVENT_MARKER_NAME_LEN (150 characters)
    And longer names should be truncated or rejected

  Scenario: Marker registration with skip frequency
    Given a marker is registered with a skip frequency
    When events are received for the marker
    Then events should be skipped according to the frequency
    And only non-skipped events should be processed
