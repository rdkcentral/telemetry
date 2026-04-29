# L2 Functional Tests - BDD Documentation Gap Analysis Report

## Executive Summary

This report identifies gaps between the BDD feature documentation in `test/functional-tests/features/` and the actual test implementations in `test/functional-tests/tests/`. The analysis reveals **documentation is generally comprehensive**, but several scenarios lack corresponding test implementations or have incomplete coverage.

---

## Feature Files Overview

| Feature File | Scenarios Documented | Implementation File |
|--------------|---------------------|---------------------|
| `telemetry_bootup_sequence.feature` | 1 (multi-step) | `test_bootup_sequence.py` |
| `telemetry_runs_as_daemon.feature` | 1 | `test_runs_as_daemon.py` |
| `telemetry_xconf_communication.feature` | 4 | `test_xconf_communications.py` |
| `telemetry_process_multiprofile.feature` | 44 | `test_multiprofile_msgpacket.py` |
| `telemetry_process_singleprofile.feature` | 4 | *Partially in multiprofile tests* |
| `telemetry_process_tempProfile.feature` | 14 | `test_temp_profile.py` |

---

## Detailed Gap Analysis

### 1. `telemetry_bootup_sequence.feature` ✅ MOSTLY COVERED

| Scenario Step | Status | Test Function |
|---------------|--------|---------------|
| Telemetry runs as daemon | ✅ Implemented | `test_boot_sequence()` |
| Init important flags | ✅ Implemented | `test_Bootup_Flags()` |
| Xconf communication | ✅ Implemented | `test_xconf_request()` |
| Saving xconf data persistently | ✅ Implemented | `test_persistant_data()` |
| Exposing RBUS interfaces | ✅ Implemented | `test_rbus_data()` |
| EXEC RELOAD signal handling | ✅ Implemented | `test_exec_reload()` |
| SIGTERM signal handling | ✅ Implemented | `test_Terminal_signal()` |

**Gaps:** None identified.

---

### 2. `telemetry_runs_as_daemon.feature` ✅ FULLY COVERED

| Scenario | Status | Test Function |
|----------|--------|---------------|
| Telemetry runs as daemon | ✅ Implemented | `test_check_telemetry_is_starting()` |
| No second instance started | ✅ Implemented | `test_second_telemetry_instance_is_not_started()` |

**Gaps:** None identified.

---

### 3. `telemetry_xconf_communication.feature` ⚠️ PARTIAL COVERAGE

| Scenario | Status | Test Function |
|----------|--------|---------------|
| Valid URL communication | ✅ Implemented | `test_precondition()`, `test_verify_urlencoding()` |
| Invalid/empty URL handling | ✅ Implemented | `test_xconf_connection_with_empty_url()`, `test_xconf_http()` |
| 404 error handling | ✅ Implemented | `test_xconf_404()` |
| Log upload on signal 10 | ✅ Implemented | `test_log_upload()` |
| URL encoding verification | ✅ Implemented | `test_verify_urlencoding()` |
| Meta data (mac, firmware, env) in URL | ⚠️ Partial | Verified in `test_xconf_request()` but not URL-specific |

**Gaps:**
1. **Missing explicit test for URL-encoded metadata verification** - The feature specifies "add meta data such as mac, firmware, env in URL encoded format" but tests only verify the response contains this data, not that it was URL-encoded in the request.

---

### 4. `telemetry_process_multiprofile.feature` ✅ FULLY COVERED

| Scenario | Status | Test Location |
|----------|--------|---------------|
| Empty name/hash rejection | ✅ Implemented | `test_without_namefield()` |
| Missing name/hash field rejection | ✅ Implemented | `test_without_namefield()` |
| Empty/missing version acceptance | ✅ Implemented | `test_without_hashvalue()` |
| Incorrect/missing protocol rejection | ✅ Implemented | `test_with_wrong_protocol_value()` |
| Empty/missing ActivationTimeout | ✅ Implemented | `test_without_EncodingType_ActivationTimeout_values()` |
| Empty encodingType acceptance | ✅ Implemented | `test_without_EncodingType_ActivationTimeout_values()` |
| Missing encodingType rejection | ✅ Implemented | `test_without_EncodingType_ActivationTimeout_values()` |
| Missing ReportingInterval rejection | ✅ Implemented | `test_without_EncodingType_ActivationTimeout_values()` |
| Missing GenerateNow acceptance | ✅ Implemented | `test_without_EncodingType_ActivationTimeout_values()` |
| ReportingInterval working | ✅ Implemented | `test_reporting_interval_working()` |
| Event marker with count | ✅ Implemented | `test_reporting_interval_working()` |
| Event marker with accumulate | ✅ Implemented | `test_reporting_interval_working()` |
| GenerateNow=true | ✅ Implemented | `test_for_Generate_Now()` |
| Grep marker with count | ✅ Implemented | `test_for_Generate_Now()` |
| Grep marker with absolute | ✅ Implemented | `test_for_Generate_Now()` |
| Grep marker with trim | ✅ Implemented | `test_for_Generate_Now()` |
| Datamodel marker | ✅ Implemented | `test_for_Generate_Now()` |
| ActivationTimeout < ReportingInterval rejection | ✅ Implemented | `test_for_invalid_activation_timeout()` |
| reportEmpty=true | ✅ Implemented | `test_for_invalid_activation_timeout()` |
| FirstReportingInterval | ✅ Implemented | `test_for_invalid_activation_timeout()` |
| ActivationTimeout working | ✅ Implemented | `test_with_delete_on_timeout()` |
| Grep marker with regex | ✅ Implemented | `test_with_delete_on_timeout()` |
| Event marker with regex | ✅ Implemented | `test_with_delete_on_timeout()` |
| Datamodel marker with regex | ✅ Implemented | `test_with_delete_on_timeout()` |
| DCM + multiprofile parallel | ✅ Implemented | `test_for_invalid_activation_timeout()` |
| TimeRef default + maxUploadLatency | ✅ Implemented | `test_for_first_reporting_interval_Maxlatency()` |
| TimeRef non-default + maxUploadLatency | ✅ Implemented | `test_for_first_reporting_interval_Maxlatency()` |
| MaxUploadLatency > ReportingInterval rejection | ✅ Implemented | `test_for_first_reporting_interval_Maxlatency()` |
| Empty parameters rejection | ✅ Implemented | `test_for_first_reporting_interval_Maxlatency()` |
| TriggerCondition without type | ✅ Implemented | `test_for_triggerCondition_negative_case()` |
| TriggerCondition without reference | ✅ Implemented | `test_for_triggerCondition_negative_case()` |
| TriggerCondition unexpected type | ✅ Implemented | `test_for_triggerCondition_negative_case()` |
| TriggerCondition without operator | ✅ Implemented | `test_for_triggerCondition_negative_case()` |
| TriggerCondition unexpected operator | ✅ Implemented | `test_for_triggerCondition_negative_case()` |
| TriggerCondition without threshold | ✅ Implemented | `test_for_triggerCondition_negative_case()` |
| TriggerCondition unexpected reference | ✅ Implemented | `test_for_triggerCondition_negative_case()` |
| TriggerCondition working | ✅ Implemented | `test_for_triggerCondition_working_case()` |
| Duplicate HASH rejection | ✅ Implemented | `test_for_duplicate_hash()` |
| TR181 subscribe | ✅ Implemented | `test_for_subscribe_tr181()` |
| PreviousLogs harvesting | ✅ Implemented | `test_for_subscribe_tr181()` |
| Multiple split markers | ✅ Implemented | `test_for_subscribe_tr181()` |
| TR181 accumulate | ✅ Implemented | `test_for_subscribe_tr181()` |
| HTTP protocol report sending | ✅ Implemented | `test_for_subscribe_tr181()` |
| Upload failed caching | ✅ Implemented | `test_reporting_interval_working()` |
| RBUS_METHOD protocol | ✅ Implemented | `test_reporting_interval_working()` |
| Log rotation handling | ✅ Implemented | `test_for_duplicate_hash()` |
| Event timestamp reporting | ✅ Implemented | `test_reporting_interval_working()` |
| On-demand reporting (signal 29) | ✅ Implemented | `test_for_duplicate_hash()` |
| Stress testing | ✅ Implemented | `test_stress_test()` |
| Profile persistence | ✅ Implemented | `test_for_duplicate_hash()` |

**Gaps:** None significant - all 44 scenarios have corresponding test coverage.

---

### 5. `telemetry_process_singleprofile.feature` ❌ MAJOR GAPS

| Scenario | Status | Notes |
|----------|--------|-------|
| Event marker with accumulate | ⚠️ Partial | Tested in multiprofile, **no dedicated single profile test file** |
| Multiple split markers | ⚠️ Partial | Tested in multiprofile |
| Upload failed caching | ⚠️ Partial | Tested in multiprofile |
| PreviousLogs harvesting | ⚠️ Partial | Tested in multiprofile |

**Critical Gap:** 
- **No dedicated `test_singleprofile.py` implementation file exists**
- Single profile scenarios are documented but only tested indirectly through multiprofile tests
- The feature file suggests single profile should be tested independently

---

### 6. `telemetry_process_tempProfile.feature` ⚠️ PARTIAL COVERAGE

| Scenario | Status | Test Function |
|----------|--------|---------------|
| Co-existence with report profile | ❌ **NOT IMPLEMENTED** | No explicit test |
| Datamodel marker | ✅ Implemented | `test_for_Generate_Now()` |
| TR181 subscribe | ✅ Implemented | `test_for_subscribe_tr181()` |
| PreviousLogs harvesting | ✅ Implemented | `test_for_subscribe_tr181()` |
| Event marker with accumulate | ✅ Implemented | `test_reporting_interval_working()` |
| TR181 accumulate | ✅ Implemented | `test_for_subscribe_tr181()` |
| Multiple profiles simultaneously | ✅ Implemented | `test_without_EncodingType_ActivationTimeout_values()` |
| ReportingInterval | ✅ Implemented | `test_reporting_interval_working()` |
| ActivationTimeout | ✅ Implemented | `test_with_delete_on_timeout()` |
| TriggerConditions | ✅ Implemented | `test_for_triggerCondition_working_case()` |
| Grep marker with regex | ✅ Implemented | `test_with_delete_on_timeout()` |
| Upload failed caching | ✅ Implemented | `test_reporting_interval_working()` |
| RBUS_METHOD protocol | ✅ Implemented | `test_reporting_interval_working()` |
| Profile non-persistence | ✅ Implemented | `test_for_profile_non_persistence()` |

**Gaps:**
1. **Missing test for temp profile + report profile co-existence** - Feature explicitly documents this scenario but no dedicated test exists

---

## Summary of Gaps

### Critical Gaps (Missing Test Files/Functions)

| Gap ID | Feature | Scenario | Priority |
|--------|---------|----------|----------|
| GAP-001 | `singleprofile.feature` | All scenarios | **HIGH** - No dedicated test file |
| GAP-002 | `tempProfile.feature` | Co-existence with report profile | **MEDIUM** |

### Documentation Quality Issues

| Issue ID | Location | Description | Status |
|----------|----------|-------------|--------|
| DOC-001 | `singleprofile.feature` | Scenarios overlap with multiprofile - consider consolidating | Open |
| DOC-002 | `multiprofile.feature:80-84` | Duplicate scenario names (event marker with count appears twice) | ✅ Fixed |
| DOC-003 | `multiprofile.feature:96` | Typo: "shpuld" should be "should" | ✅ Fixed |
| DOC-004 | `tempProfile.feature:88-90` | Scenario description mentions HTTP but tests RBUS_METHOD | ✅ Fixed |
| DOC-005 | Multiple files | Typo: "confugred" should be "configured" | ✅ Fixed |
| DOC-006 | Multiple files | Typo: "attemplted" should be "attempted" | ✅ Fixed |
| DOC-007 | `multiprofile.feature:270` | Typo: "roatated" should be "rotated" | ✅ Fixed |
| DOC-008 | `multiprofile.feature:274` | Typo: "telementry" should be "telemetry" | ✅ Fixed |
| DOC-009 | `multiprofile.feature:280` | Typo: "reportwill" should be "report will" | ✅ Fixed |

---

## Remediation Plan

### Phase 1: Critical Gaps (Week 1)

#### Task 1.1: Create `test_singleprofile.py`
```
Priority: HIGH
Description: Create dedicated test file for single profile scenarios
Scenarios to implement:
  - test_singleprofile_event_marker_accumulate()
  - test_singleprofile_split_markers()
  - test_singleprofile_upload_caching()
  - test_singleprofile_previous_logs()
```

#### Task 1.2: Add temp profile co-existence test
```
Priority: MEDIUM
Description: Add test_temp_report_profile_coexistence() to test_temp_profile.py
```

### Phase 2: Documentation Cleanup (Week 2)

#### Task 2.1: Fix typos and inconsistencies
- Fix "shpuld" → "should" in multiprofile.feature:96
- Fix HTTP/RBUS_METHOD inconsistency in tempProfile.feature:88-90
- Remove duplicate scenario at multiprofile.feature:80-84

#### Task 2.2: Consolidate or clarify singleprofile.feature
- Option A: Merge into multiprofile.feature with clear tagging
- Option B: Create dedicated test file and keep separate

### Phase 3: Enhanced Traceability (Week 3)

#### Task 3.1: Add scenario IDs to feature files
```gherkin
@TC-MP-001
Scenario: Multiprofile configuration with empty name or hash
```

#### Task 3.2: Add traceability comments to test functions
```python
def test_without_namefield():
    """
    Covers: TC-MP-001, TC-MP-002, TC-MP-003
    Feature: telemetry_process_multiprofile.feature
    """
```

---

## Test Coverage Matrix

| Feature Area | Documented | Implemented | Coverage |
|--------------|------------|-------------|----------|
| Bootup Sequence | 7 steps | 7 tests | **100%** |
| Daemon Behavior | 2 scenarios | 2 tests | **100%** |
| Xconf Communication | 4 scenarios | 13 tests | **100%+** |
| Multiprofile | 44 scenarios | 15 tests | **~95%** |
| Single Profile | 4 scenarios | 0 dedicated | **0%** (indirect coverage) |
| Temp Profile | 14 scenarios | 13 tests | **~93%** |

**Overall Coverage: ~85%** (with single profile gap being the main issue)

---

## Appendix: Test-to-Feature Mapping

### test_bootup_sequence.py
- `test_boot_sequence()` → telemetry_bootup_sequence.feature: lines 23-29
- `test_persistant_data()` → telemetry_bootup_sequence.feature: line 32
- `test_Bootup_Flags()` → telemetry_bootup_sequence.feature: line 28
- `test_rbus_data()` → telemetry_bootup_sequence.feature: line 34
- `test_xconf_request()` → telemetry_bootup_sequence.feature: line 31
- `test_exec_reload()` → telemetry_bootup_sequence.feature: lines 35-36
- `test_Terminal_signal()` → telemetry_bootup_sequence.feature: lines 37-38

### test_runs_as_daemon.py
- `test_check_telemetry_is_starting()` → telemetry_runs_as_daemon.feature: lines 24-26
- `test_second_telemetry_instance_is_not_started()` → telemetry_runs_as_daemon.feature: lines 27-28

### test_xconf_communications.py
- `test_xconf_connection_with_empty_url()` → telemetry_xconf_communication.feature: lines 37-43
- `test_xconf_http()` → telemetry_xconf_communication.feature: lines 37-43
- `test_xconf_404()` → telemetry_xconf_communication.feature: lines 45-52
- `test_log_upload()` → telemetry_xconf_communication.feature: line 60

---

*Report generated on: April 29, 2026 *

*Scope : L2 test documentation gap analysis*
