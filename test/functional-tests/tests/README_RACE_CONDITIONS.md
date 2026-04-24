# Race Condition Test Suite - PR #345

## Overview

This test suite validates the thread safety fixes implemented in PR #345:
- https://github.com/rdkcentral/telemetry/pull/345

## Tests Implemented

### Test Case 1: TOCTOU in `deleteAllProfiles()`
**File**: `test_profile_race_conditions.py::test_toctou_deleteAllProfiles_concurrent_modification`

**Race Condition Fixed**:
- Old code: Read lock to count profiles → release → write lock per profile
- Race window: Profile list could change between count and disable operations
- Fix: Single write lock acquisition to atomically count and disable all profiles

**Test Strategy**:
- Create 10 initial profiles
- Spawn threads to add/delete profiles concurrently
- Trigger telemetry restart (calls `deleteAllProfiles()`) during modifications
- Verify no crashes, deadlocks, or assertion failures

**Success Criteria**:
- ✓ No crashes under concurrent load
- ✓ System remains stable
- ✓ All profiles cleanly deleted
- ✓ No data races detected

---

### Test Case 2: `reportInProgress` Concurrent Access
**File**: `test_profile_race_conditions.py::test_reportInProgress_concurrent_access`

**Race Condition Fixed**:
- Old code: Mixed locking - `reportInProgressMutex` in some places, `xconfProfileLock` in others
- Race: Inconsistent mutex usage led to data races
- Fix: Consistently use `xconfProfileLock` for all `reportInProgress` access

**Test Strategy**:
- Configure XConf profile with 5-second reporting interval
- Fire 20 concurrent on-demand report requests
- Monitor for "previous callback is still in progress" messages
- Verify mutex serializes access correctly

**Success Criteria**:
- ✓ Only one report at a time (mutex prevents concurrent access)
- ✓ Concurrent requests properly rejected with "in progress" message
- ✓ No data races
- ✓ No lost reports

---

### Test Case 4a: Active Report Thread Shutdown
**File**: `test_profile_race_conditions.py::test_uninit_joins_active_report_thread`

**Race Condition Fixed**:
- Old bug: `uninit` only joined thread if `reportInProgress == true`
- Missed case: Idle threads waiting in `pthread_cond_wait`
- Fix: Always check `reportThreadExits` flag and join if thread was created

**Test Strategy**:
- Create profile with many markers (slow report generation)
- Trigger on-demand report
- Immediately shutdown while report is actively running
- Verify thread joined cleanly

**Success Criteria**:
- ✓ Thread joined before uninit completes
- ✓ No orphaned threads
- ✓ No use-after-free
- ✓ Clean shutdown

---

### Test Case 4b: Idle Report Thread Shutdown
**File**: `test_profile_race_conditions.py::test_uninit_joins_idle_report_thread`

**Race Condition Fixed**:
- Same bug as 4a, but tests idle thread scenario
- Thread in `pthread_cond_wait`, `reportInProgress == false`
- Old code would skip join, causing use-after-free when resources destroyed

**Test Strategy**:
- Create profile with 10-minute interval (thread will wait)
- Let thread enter idle state in `pthread_cond_wait`
- Shutdown telemetry
- Verify thread is signaled and joined

**Success Criteria**:
- ✓ Idle thread signaled via condition variable
- ✓ Thread joined successfully
- ✓ No resource leaks
- ✓ No use-after-free

---

### Test Case 4c: Rapid Init/Uninit Stress Test
**File**: `test_profile_race_conditions.py::test_rapid_init_uninit_with_reports`

**Combined Stress Test**:
- Rapid init/uninit cycles (10 iterations)
- Random scenarios: immediate shutdown, idle thread, active report
- Exposes edge cases in thread lifecycle management

---

## Running the Tests

### Run All L2 Tests (including race condition tests)
```bash
./test/run_l2.sh
```

### Run Only Race Condition Tests
```bash
pytest -v test/functional-tests/tests/test_profile_race_conditions.py
```

### Run Specific Test Case
```bash
# Test Case 1: TOCTOU
pytest -v test/functional-tests/tests/test_profile_race_conditions.py::TestProfileRaceConditions::test_toctou_deleteAllProfiles_concurrent_modification

# Test Case 2: reportInProgress
pytest -v test/functional-tests/tests/test_profile_race_conditions.py::TestProfileRaceConditions::test_reportInProgress_concurrent_access

# Test Case 4a: Active thread
pytest -v test/functional-tests/tests/test_profile_race_conditions.py::TestProfileRaceConditions::test_uninit_joins_active_report_thread

# Test Case 4b: Idle thread
pytest -v test/functional-tests/tests/test_profile_race_conditions.py::TestProfileRaceConditions::test_uninit_joins_idle_report_thread

# Test Case 4c: Stress test
pytest -v test/functional-tests/tests/test_profile_race_conditions.py::TestProfileRaceConditions::test_rapid_init_uninit_with_reports
```

### Run with Detailed Output
```bash
pytest -v -s test/functional-tests/tests/test_profile_race_conditions.py
```

---

### Common Failure Patterns

#### Crash During Test
```
AssertionError: Telemetry crashed during concurrent operations
```
**Cause**: Race condition not fully fixed, data corruption  
**Action**: Review fix implementation, check for additional race windows

#### Deadlock Detected
```
AssertionError: Deadlock detected
```
**Cause**: Lock ordering issue or mutex acquired multiple times  
**Action**: Review lock hierarchy, ensure consistent ordering

#### Thread Leak
```
AssertionError: Thread leak detected across cycles
```
**Cause**: Thread not properly joined on some code path  
**Action**: Verify all exit paths join threads

#### Assertion Failure
```
AssertionError: Assertion failure detected
```
**Cause**: Internal consistency check failed (e.g., NULL pointer, invalid state)  
**Action**: Review assertion message in logs for root cause

---

## Advanced Testing

### Running Under Valgrind (Memory Safety)
```bash
# Start telemetry under valgrind
valgrind --leak-check=full --track-origins=yes \
         --log-file=/tmp/valgrind.log \
         /usr/local/bin/telemetry2_0 &

# Run tests
pytest -v test/functional-tests/tests/test_profile_race_conditions.py

# Check results
cat /tmp/valgrind.log
```

### Running Under ThreadSanitizer (Race Detection)
```bash
# Rebuild with TSAN
export CFLAGS="-fsanitize=thread -g -O1"
export LDFLAGS="-fsanitize=thread"
./configure && make clean && make

# Run tests
TSAN_OPTIONS="log_path=/tmp/tsan.log second_deadlock_stack=1" \
    pytest -v test/functional-tests/tests/test_profile_race_conditions.py

# Review TSAN reports
cat /tmp/tsan.log.*
```

### GDB Thread Analysis
```bash
# Start telemetry
/usr/local/bin/telemetry2_0 &

# Attach GDB
gdb -p $(pidof telemetry2_0)

# Analyze threads
(gdb) info threads
(gdb) thread apply all bt
```

---

### Local Pre-Commit Testing
```bash
# Quick validation before commit
pytest -v test/functional-tests/tests/test_profile_race_conditions.py

# Full L2 suite
./test/run_l2.sh
```

---

## Maintenance

### Adding New Race Condition Tests

1. **Identify the race condition** - Document the bug and fix
2. **Create test case** - Add to `test_profile_race_conditions.py`
3. **Implement test** - Use `concurrency_helpers` utilities
4. **Verify** - Run under valgrind and TSAN
5. **Document** - Update this README

### Test Evolution
- Update tests when threading model changes
- Add regression tests when new issues discovered
- Keep tests realistic (don't artificially widen race windows)

---
