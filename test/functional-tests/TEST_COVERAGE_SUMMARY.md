# Telemetry 2.0 Functional Test Coverage Summary

## Overview
This document provides a comprehensive mapping between the functional test implementations and their corresponding BDD feature file scenarios. This ensures that validation engineers and architects can easily understand what tests are being performed and their expected outcomes.

---

## Test File to Feature File Mapping

### 1. test_bootup_sequence.py → telemetry_bootup_sequence.feature

**Test Coverage:**
- ✅ Telemetry daemon startup and process management
- ✅ Persistent flag initialization and cleanup
- ✅ Xconf server communication during bootup
- ✅ RBUS interface API exposure
- ✅ EXEC RELOAD signal handling (signal 12)
- ✅ SIGTERM signal handling (signal 15)
- ✅ Xconf response persistence

**Key Test Cases:**
1. `test_boot_sequence()` - Validates complete bootup sequence
2. `test_persistant_data()` - Verifies Xconf data persistence
3. `test_Bootup_Flags()` - Checks bootup flag creation
4. `test_rbus_data()` - Validates RBUS interface functionality
5. `test_xconf_request()` - Verifies Xconf request parameters
6. `test_exec_reload()` - Tests configuration reload mechanism
7. `test_Terminal_signal()` - Validates graceful shutdown
8. `test_Telemetry_exit()` - Confirms process termination

---

### 2. test_runs_as_daemon.py → telemetry_runs_as_daemon.feature

**Test Coverage:**
- ✅ Daemon process initialization
- ✅ Single instance enforcement (prevents multiple instances)
- ✅ Process lifecycle management

**Key Test Cases:**
1. `test_check_telemetry_is_starting()` - Validates daemon startup
2. `test_second_telemetry_instance_is_not_started()` - Ensures single instance
3. `test_tear_down()` - Verifies clean shutdown

---

### 3. test_xconf_communications.py → telemetry_xconf_communication.feature

**Test Coverage:**
- ✅ HTTPS-only URL validation
- ✅ Empty/invalid URL handling
- ✅ HTTP 404 error handling
- ✅ Profile change detection
- ✅ URL encoding validation
- ✅ Reporting schedule verification
- ✅ Marker validation (grep and event markers)
- ✅ Report generation and upload
- ✅ Log upload on demand (signal 10 and 29)
- ✅ Connection retry mechanism for failures
- ✅ Datamodel marker with accumulate
- ✅ Split marker validation
- ✅ Parallel DCM and multiprofile execution

**Key Test Cases:**
1. `test_precondition()` - Setup test environment
2. `test_xconf_connection_with_empty_url()` - Invalid URL handling
3. `test_xconf_http()` - HTTPS enforcement
4. `test_xconf_404()` - 404 error response handling
5. `test_change_profile()` - Profile update mechanism
6. `test_verify_urlencoding()` - URL parameter encoding
7. `test_verify_schedule()` - Reporting interval validation
8. `test_verify_markers()` - Marker data collection
9. `test_verify_report()` - Report generation
10. `test_log_upload()` - Signal 10 log upload
11. `test_log_upload_on_demand()` - Signal 29 on-demand upload
12. `test_verify_persistant_file()` - Persistence validation
13. `test_xconf_retry_for_connection_errors()` - Retry mechanism
14. `test_xconf_datamodel()` - Datamodel marker testing
15. `test_xconf_split_markers()` - Split marker validation

---

### 4. test_multiprofile_msgpacket.py → telemetry_process_multiprofile.feature

**Test Coverage:**

#### Profile Validation (Negative Tests)
- ✅ Missing/empty name field rejection
- ✅ Missing/empty hash field rejection
- ✅ Missing/invalid hash value handling
- ✅ Missing/empty version field acceptance
- ✅ Missing/invalid protocol field rejection
- ✅ Empty/missing encodingType handling
- ✅ Missing ActivationTimeout acceptance
- ✅ Missing ReportingInterval rejection
- ✅ Missing GenerateNow acceptance
- ✅ ActivationTimeout < ReportingInterval rejection

#### Profile Configuration (Positive Tests)
- ✅ Message pack format parsing (TC 202)
- ✅ JSON format parsing (TC 201)
- ✅ Multiple profiles simultaneous execution (TC 215)
- ✅ ReportingInterval functionality (TC 218)
- ✅ FirstReportingInterval support (TC 240)
- ✅ ActivationTimeout support (TC 219)
- ✅ GenerateNow immediate reporting (TC 235)
- ✅ DeleteOnTimeout profile removal (TC 232)

#### Marker Types
- ✅ Event markers with count (TC 234)
- ✅ Event markers with accumulate (TC 212, 248)
- ✅ Grep markers with count (TC 236)
- ✅ Grep markers with absolute (TC 237)
- ✅ Grep markers with trim (TC 238)
- ✅ Datamodel markers (TC 206)
- ✅ Datamodel markers with subscribe (TC 207, 214)
- ✅ Split markers (TC 209, 211)

#### Advanced Features
- ✅ Regex support for grep markers (TC 222, 250)
- ✅ Regex support for event markers (TC 242)
- ✅ Regex support for datamodel markers (TC 243)
- ✅ ReportEmpty functionality (TC 239)
- ✅ MaxUploadLatency/delayed reporting (TC 217, 245)
- ✅ TriggerCondition support (TC 220, 244, 251)
- ✅ TriggerCondition validation (negative cases)
- ✅ Previous logs folder harvesting (TC 210, 231, 246)
- ✅ Log file rotation handling (TC 247)
- ✅ Duplicate hash detection (TC 203)
- ✅ Profile persistence (TC 229)
- ✅ Forced on-demand reporting (TC 221, 252)
- ✅ Stress testing for deadlocks (TC 253)

#### Reporting Protocols
- ✅ HTTP protocol (TC 223)
- ✅ RBUS_METHOD protocol (TC 228)
- ✅ Report caching on failure (TC 225)
- ✅ Configurable endpoints (TC 226)
- ✅ Configurable URL parameters (TC 227)

#### Parallel Execution
- ✅ DCM profile and multiprofile parallel execution (TC 241)

**Key Test Cases:**
1. `test_without_namefield()` - Profile validation
2. `test_without_hashvalue()` - Hash validation
3. `test_with_wrong_protocol_value()` - Protocol validation
4. `test_without_EncodingType_ActivationTimeout_values()` - Field validation
5. `test_reporting_interval_working()` - Interval-based reporting
6. `test_for_Generate_Now()` - Immediate report generation
7. `test_for_invalid_activation_timeout()` - Timeout validation
8. `test_with_delete_on_timeout()` - Profile deletion
9. `test_for_first_reporting_interval_Maxlatency()` - Advanced timing
10. `test_for_triggerCondition_negative_case()` - Trigger validation
11. `test_for_subscribe_tr181()` - Datamodel subscription
12. `test_for_triggerCondition_working_case()` - Trigger functionality
13. `test_for_duplicate_hash()` - Duplicate detection
14. `test_stress_test()` - Stress testing

---

### 5. test_temp_profile.py → telemetry_process_tempProfile.feature

**Test Coverage:**

#### Profile Validation (Negative Tests)
- ✅ Invalid protocol rejection
- ✅ Empty protocol rejection
- ✅ Missing encodingType rejection
- ✅ Empty ActivationTimeout rejection (differs from report profile)
- ✅ Missing ReportingInterval rejection
- ✅ ActivationTimeout < ReportingInterval rejection

#### Profile Configuration (Positive Tests)
- ✅ JSON format parsing (TC 301)
- ✅ Multiple profiles simultaneous execution (TC 313)
- ✅ ReportingInterval functionality (TC 314)
- ✅ ActivationTimeout support (TC 315)
- ✅ GenerateNow with forced upload (TC 317)
- ✅ DeleteOnTimeout profile removal

#### Marker Types
- ✅ Event markers with count
- ✅ Event markers with accumulate (TC 312)
- ✅ Grep markers with count
- ✅ Grep markers with absolute
- ✅ Grep markers with trim
- ✅ Datamodel markers (TC 304)
- ✅ Datamodel markers with subscribe (TC 305, 312)
- ✅ Split markers (TC 307)

#### Advanced Features
- ✅ Regex support for grep markers (TC 318)
- ✅ Regex support for event markers (TC 309)
- ✅ Regex support for datamodel markers (TC 304)
- ✅ ReportEmpty functionality
- ✅ FirstReportingInterval support
- ✅ MaxUploadLatency support
- ✅ TriggerCondition support (TC 316)
- ✅ TriggerCondition validation (negative cases)
- ✅ Previous logs folder harvesting (TC 308, 327)

#### Reporting Protocols
- ✅ HTTP protocol (TC 319)
- ✅ RBUS_METHOD protocol (TC 324)
- ✅ Report caching on failure (TC 321)
- ✅ Configurable endpoints (TC 322)
- ✅ Configurable URL parameters (TC 323)

#### Profile Lifecycle
- ✅ Profile non-persistence (TC 326)
- ✅ Forced on-demand reporting (TC 317)

**Key Test Cases:**
1. `test_with_wrong_protocol_value()` - Protocol validation
2. `test_without_EncodingType_ActivationTimeout_values()` - Field validation
3. `test_reporting_interval_working()` - Interval-based reporting
4. `test_for_Generate_Now()` - Immediate reporting with forced upload
5. `test_for_invalid_activation_timeout()` - Timeout validation
6. `test_with_delete_on_timeout()` - Profile deletion
7. `test_for_first_reporting_interval_Maxlatency()` - Advanced timing
8. `test_for_triggerCondition_negative_case()` - Trigger validation
9. `test_for_subscribe_tr181()` - Datamodel subscription
10. `test_for_triggerCondition_working_case()` - Trigger functionality
11. `test_for_profile_non_persistence()` - Non-persistence validation

---

## Feature File Summary

### Total Scenarios by Feature File:

1. **telemetry_bootup_sequence.feature**: 1 scenario
   - Covers complete bootup lifecycle

2. **telemetry_runs_as_daemon.feature**: 1 scenario
   - Covers daemon process management

3. **telemetry_xconf_communication.feature**: 12 scenarios
   - Covers Xconf communication, retry, profiles, and markers

4. **telemetry_process_multiprofile.feature**: 50+ scenarios
   - Comprehensive coverage of multiprofile functionality
   - Negative and positive test cases
   - All marker types and protocols
   - Advanced features and edge cases

5. **telemetry_process_tempProfile.feature**: 35+ scenarios
   - Comprehensive coverage of temporary profile functionality
   - Validation tests
   - All marker types
   - Profile lifecycle management

6. **telemetry_process_singleprofile.feature**: 4 scenarios
   - Basic single profile functionality

---

## Test Categories

### 1. Profile Configuration & Validation
- Profile parsing (JSON and MessagePack formats)
- Required field validation
- Optional field handling
- Protocol validation
- Timeout validation
- Multiple profile handling

### 2. Data Collection Markers
- **Event Markers**: count, accumulate, regex
- **Grep Markers**: count, absolute, trim, regex
- **Datamodel Markers**: get, subscribe, accumulate, regex
- **Split Markers**: multiple markers per line

### 3. Reporting Mechanisms
- Interval-based reporting
- First reporting interval
- Activation timeout
- Generate now (immediate)
- Trigger conditions
- Forced on-demand (signals 10, 29)

### 4. Advanced Features
- MaxUploadLatency (delayed reporting)
- ReportEmpty (NULL value reporting)
- DeleteOnTimeout
- Profile persistence/non-persistence
- Duplicate hash detection
- Log file rotation handling
- Previous logs harvesting

### 5. Communication Protocols
- HTTP protocol
- RBUS_METHOD protocol
- Report caching on failure
- Configurable endpoints
- URL parameter encoding

### 6. System Integration
- Xconf communication
- RBUS interface
- Signal handling (10, 12, 15, 29)
- Parallel profile execution
- Stress testing

---


## Test Execution Order

Tests are executed in specific order using `@pytest.mark.run(order=X)`:

1. **Bootup Sequence Tests** (order 1-8)
2. **Daemon Tests** (order 1-3)
3. **Xconf Communication Tests** (order 1-15)
4. **Multiprofile Tests** (order 1-14)
5. **Temp Profile Tests** (order 3-13)

This ensures proper test isolation and dependency management.

---

## Continuous Improvement

### Areas for Enhancement:
1. Add performance benchmarking tests
2. Expand stress testing scenarios
3. Add security validation tests
4. Include memory leak detection tests
5. Add network failure simulation tests

### Documentation Updates:
- Keep this summary updated with new test additions
- Update feature files when requirements change
- Maintain traceability between requirements and tests
- Document any test environment prerequisites

---

## Conclusion

The functional test suite provides comprehensive coverage of Telemetry 2.0 functionality with clear documentation through BDD feature files. Stakeholders can easily understand test scenarios, validation criteria, and system behavior through the feature files, while developers have detailed test implementations for reference.

All tests are designed to be:
- **Readable**: Clear scenario descriptions
- **Maintainable**: Modular helper functions
- **Traceable**: Mapped to requirements (TC numbers)
- **Comprehensive**: Covering positive, negative, and edge cases
- **Automated**: Executable via pytest framework

---

*Document Version: 1.0*  
*Last Updated: 20 November 2025*  
*Maintained by: RDK Telemetry Team*
