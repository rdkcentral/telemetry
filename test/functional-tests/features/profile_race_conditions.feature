Feature: Profile Race Conditions

  As a QA engineer
  I want to validate profile race conditions functionality
  So that the telemetry system operates correctly

  Scenario: Toctou Deleteallprofiles Concurrent Modification
    # Test Case 1: Verify TOCTOU race fix in deleteAllProfiles()
    Given the telemetry system is running
    When toctou deleteallprofiles concurrent modification is executed
    Then the system should handle it correctly
    And no errors or crashes should occur

  Scenario: Reportinprogress Concurrent Access
    # Test Case 2: Verify reportInProgress flag protected by single mutex
    Given the telemetry system is running
    When reportinprogress concurrent access is executed
    Then the system should handle it correctly
    And no errors or crashes should occur

  Scenario: Uninit Joins Active Report Thread
    # Test Case 4a: Verify uninit properly joins active report thread
    Given the telemetry system is running
    When uninit joins active report thread is executed
    Then the system should handle it correctly
    And no errors or crashes should occur

  Scenario: Uninit Joins Idle Report Thread
    # Test Case 4b: Verify uninit properly joins idle report thread
    Given the telemetry system is running
    When uninit joins idle report thread is executed
    Then the system should handle it correctly
    And no errors or crashes should occur

  Scenario: Rapid Init Uninit With Reports
    # Test Case 4c: Stress test - rapid init/uninit cycles during report generation
    Given the telemetry system is running
    When rapid init uninit with reports is executed
    Then the system should handle it correctly
    And no errors or crashes should occur
