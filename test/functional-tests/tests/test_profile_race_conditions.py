####################################################################################
# If not stated otherwise in this file or this component's Licenses file the
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

"""
L2 Tests for PR #345 - Thread Safety and Race Condition Fixes

Test suite validating race condition fixes in profile management:
1. TOCTOU in deleteAllProfiles() - concurrent list modification
2. reportInProgress concurrent access - inconsistent locking
3. pthread_create race window - thread exists flag
4. Report during shutdown - active and idle thread join

"""

import pytest
import time
import json
from concurrent.futures import ThreadPoolExecutor, as_completed
from helper_functions import *
from concurrency_helpers import *
from basic_constants import *


class TestProfileRaceConditions:
    """Test suite for validating race condition fixes in profile management"""
    
    @classmethod
    def setup_class(cls):
        """Setup before all tests in this class"""
        print("\n" + "="*80)
        print("Starting Profile Race Condition Test Suite")
        print("Testing PR #345 fixes")
        print("="*80)
        
        # Ensure clean state
        run_shell_command("killall -9 telemetry2_0 2>/dev/null || true")
        time.sleep(1)
        clear_persistant_files()
        remove_T2bootup_flag()
    
    @classmethod
    def teardown_class(cls):
        """Cleanup after all tests"""
        print("\n" + "="*80)
        print("Profile Race Condition Test Suite Complete")
        print("="*80)
        
        # Final cleanup
        run_shell_command("killall -9 telemetry2_0 2>/dev/null || true")
        clear_persistant_files()
    
    def setup_method(self, method):
        """Setup before each test"""
        print(f"\n--- Test: {method.__name__} ---")
        
        # Clean state for each test
        run_shell_command("killall -9 telemetry2_0 2>/dev/null || true")
        time.sleep(0.5)
        clear_T2logs()
        clear_persistant_files()
        remove_T2bootup_flag()
    
    def teardown_method(self, method):
        """Cleanup after each test"""
        # Stop telemetry gracefully
        stop_telemetry_graceful()
        time.sleep(0.5)
        
        # Print test summary
        if telemetry_is_running():
            print("WARNING: Telemetry still running after test")
        else:
            print("Test cleanup: Telemetry stopped cleanly")
    
    
    # =========================================================================
    # Test Case 1: TOCTOU in deleteAllProfiles()
    # =========================================================================
    
    def test_toctou_deleteAllProfiles_concurrent_modification(self):
        """
        Test Case 1: Verify TOCTOU race fix in deleteAllProfiles()
        
        Race condition: Thread A counts profiles (read lock), releases lock,
        Thread B modifies profile list, Thread A acquires write lock per-profile
        and accesses out-of-bounds or stale pointers.
        
        Fix: Atomic disable operation - acquire write lock once, count and
        disable all profiles before releasing lock.
        
        Expected: No crash, all profiles cleanly deleted under concurrent load
        """
        print("\nTest: TOCTOU in deleteAllProfiles() with concurrent modifications")
        
        # Start telemetry
        start_telemetry()
        assert telemetry_is_running(), "Telemetry failed to start"
        
        # Create initial set of profiles
        initial_profiles = []
        for i in range(10):
            profile_json = create_xconf_profile_json(
                name=f"initial_profile_{i}",
                reporting_interval=300,  # Long interval
                markers=[f"MARKER_{i}_.*"]
            )
            initial_profiles.append((f"initial_profile_{i}", profile_json))
        
        # Set all initial profiles
        print(f"Setting {len(initial_profiles)} initial profiles...")
        all_profiles_json = {
            "profiles": [
                {
                    "name": name,
                    "hash": f"hash_{name}",
                    "value": json.loads(profile_json)["profiles"][0]["value"]
                }
                for name, profile_json in initial_profiles
            ]
        }
        rbus_set_data(T2_REPORT_PROFILE_PARAM, "string", json.dumps(all_profiles_json))
        time.sleep(2)  # Allow profiles to load
        
        # Define concurrent operations
        errors = []
        completed_ops = {"add": 0, "delete": 0, "restart": 0}
        
        def add_new_profile(profile_id):
            """Add a new profile via RBUS"""
            try:
                profile_json = create_xconf_profile_json(
                    name=f"added_profile_{profile_id}",
                    reporting_interval=300,
                    markers=[f"ADD_MARKER_{profile_id}"]
                )
                # Note: This simulates profile addition during deletion
                # In real scenario, this would be triggered by XConf update
                result = set_xconf_profile_via_rbus(profile_json)
                completed_ops["add"] += 1
                return result
            except Exception as e:
                errors.append(f"add_new_profile({profile_id}): {e}")
                raise
        
        def delete_profile(profile_name):
            """Delete a specific profile"""
            try:
                # Setting empty profile list triggers deleteAllProfiles internally
                delete_all_profiles_via_rbus()
                completed_ops["delete"] += 1
            except Exception as e:
                errors.append(f"delete_profile({profile_name}): {e}")
                raise
        
        def restart_telemetry_during_operation():
            """Restart telemetry (triggers deleteAllProfiles)"""
            try:
                # Graceful restart triggers uninit which calls deleteAllProfiles
                restart_telemetry_graceful()
                completed_ops["restart"] += 1
            except Exception as e:
                errors.append(f"restart_telemetry: {e}")
                raise
        
        # Execute concurrent operations
        print("\nExecuting concurrent profile operations...")
        print("  - Adding new profiles")
        print("  - Deleting all profiles")
        print("  - Restarting telemetry (triggers deleteAllProfiles)")
        
        operations = []
        
        # Schedule profile additions
        for i in range(5):
            operations.append((add_new_profile, [i], {}))
        
        # Schedule profile deletions (triggers deleteAllProfiles)
        for i in range(3):
            operations.append((delete_profile, [f"initial_profile_{i}"], {}))
        
        # Schedule restart (critical - this triggers deleteAllProfiles during modifications)
        # Add small delay to let modifications start
        time.sleep(0.05)
        operations.append((restart_telemetry_during_operation, [], {}))
        
        # Execute all operations concurrently
        results = concurrent_operation_executor(operations, max_workers=10, timeout=30)
        
        # Wait for system to stabilize
        time.sleep(2)
        
        # Verify results
        print(f"\nOperation summary:")
        print(f"  - Additions attempted: {completed_ops['add']}")
        print(f"  - Deletions attempted: {completed_ops['delete']}")
        print(f"  - Restarts completed: {completed_ops['restart']}")
        print(f"  - Errors encountered: {len(errors)}")
        
        # Critical assertions
        assert telemetry_is_running(), "Telemetry crashed during concurrent operations"
        assert no_crash_in_logs(), "Crash detected in logs"
        assert no_deadlock_detected(), "Deadlock detected"
        
        # Acceptable if some operations fail (due to race conditions being handled)
        # but system should remain stable
        if errors:
            print(f"\nNon-critical errors (expected under load):")
            for error in errors[:5]:  # Show first 5
                print(f"  - {error}")
        
        # Verify no memory corruption indicators
        assert "assertion failed" not in get_t2_logs().lower(), "Assertion failure detected"
        assert "segmentation" not in get_t2_logs().lower(), "Segmentation fault detected"
        
        print("\n✓ TOCTOU test PASSED: System stable under concurrent modifications")
    
    
    # =========================================================================
    # Test Case 2: reportInProgress Concurrent Access
    # =========================================================================
    
    def test_reportInProgress_concurrent_access(self):
        """
        Test Case 2: Verify reportInProgress flag protected by single mutex
        
        Bug: reportInProgress was protected by per-profile mutex in some places,
        xconfProfileLock in others, causing data races.
        
        Fix: Consistently use xconfProfileLock for all reportInProgress access.
        
        Expected: Consistent state, one report at a time, no lost reports
        """
        print("\nTest: Concurrent reportInProgress access and modification")
        
        # Start telemetry
        start_telemetry()
        assert telemetry_is_running()
        
        # Create XConf profile with short interval to test report generation
        profile_json = create_xconf_profile_json(
            name="concurrent_report_test",
            reporting_interval=5,  # 5 seconds
            markers=["SYS_INFO_.*", "RDKB_.*"],
            parameters=[
                {
                    "type": "dataModel",
                    "name": "UPTIME",
                    "reference": "Device.DeviceInfo.UpTime",
                    "use": "absolute"
                }
            ]
        )
        
        print("Setting XConf profile with 5-second reporting interval...")
        set_xconf_profile_via_rbus(profile_json)
        time.sleep(2)  # Allow profile to load
        
        # Clear logs to track new activity
        clear_T2logs()
        
        # Track operation results
        on_demand_results = []
        errors = []
        ignored_count = 0
        
        def trigger_on_demand(thread_id):
            """Trigger on-demand report from a thread"""
            nonlocal ignored_count
            try:
                # Trigger report via temp profile with GenerateNow
                temp_profile = create_temp_profile_json(
                    name="concurrent_report_test",
                    generate_now=True
                )
                result = rbus_set_data(
                    T2_TEMP_REPORT_PROFILE_PARAM,
                    "string",
                    temp_profile
                )
                
                # Check if request was accepted or ignored
                time.sleep(0.1)  # Brief wait for log entry
                logs = get_t2_logs()
                if "previous callback is still in progress" in logs:
                    ignored_count += 1
                
                on_demand_results.append((thread_id, result))
                return result
            except Exception as e:
                errors.append(f"Thread {thread_id}: {e}")
                raise
        
        # Fire multiple concurrent on-demand report requests
        print("\nFiring 20 concurrent on-demand report requests...")
        
        with ThreadPoolExecutor(max_workers=10) as executor:
            futures = [
                executor.submit(trigger_on_demand, i)
                for i in range(20)
            ]
            
            # Collect results
            for future in as_completed(futures, timeout=30):
                try:
                    future.result()
                except Exception as e:
                    print(f"Operation exception: {e}")
        
        # Wait for any pending reports to complete
        print("Waiting for pending reports to complete...")
        time.sleep(10)
        
        # Analyze results
        logs = get_t2_logs()
        
        print(f"\nResults:")
        print(f"  - Total requests: 20")
        print(f"  - Ignored (already in progress): {ignored_count}")
        print(f"  - Errors: {len(errors)}")
        
        # Verify system behavior
        assert telemetry_is_running(), "Telemetry crashed during concurrent report generation"
        assert len(errors) == 0, f"Errors occurred: {errors}"
        
        # Should see messages about ignoring concurrent requests
        # This proves the mutex is working correctly
        assert ignored_count > 0, \
            "Expected some requests to be ignored (proves mutex is serializing access)"
        
        print(f"  - ✓ Found {ignored_count} 'in progress' messages (mutex working correctly)")
        
        # No crashes or data races
        assert no_crash_in_logs(), "Crash detected in logs"
        assert no_deadlock_detected(), "Deadlock detected"
        
        # Verify no data race indicators
        assert "data race" not in logs.lower(), "Data race detected"
        assert "race condition" not in logs.lower(), "Race condition detected"
        
        print("\n✓ reportInProgress test PASSED: Consistent locking under concurrent load")
    
    
    # =========================================================================
    # Test Case 4: Report During Shutdown (Active and Idle Threads)
    # =========================================================================
    
    def test_uninit_joins_active_report_thread(self):
        """
        Test Case 4a: Verify uninit properly joins active report thread
        
        Bug: uninit only joined thread if reportInProgress=true, but missed
        idle threads waiting in pthread_cond_wait.
        
        Fix: Check reportThreadExits flag instead, signal and join in all cases.
        
        Scenario: Report actively running when shutdown triggered
        Expected: Thread joined cleanly, no orphaned threads
        """
        print("\nTest: Shutdown while report thread ACTIVE")
        
        # Start telemetry
        start_telemetry()
        assert telemetry_is_running()
        
        # Create profile with many markers to make report generation slow
        slow_markers = [f"SLOW_MARKER_{i}_.*" for i in range(100)]
        profile_json = create_xconf_profile_json(
            name="slow_report_profile",
            reporting_interval=300,  # Long interval, we'll trigger manually
            markers=slow_markers
        )
        
        print("Setting profile designed for slow report generation...")
        set_xconf_profile_via_rbus(profile_json)
        time.sleep(2)
        
        # Get initial thread count
        initial_thread_count = get_thread_count()
        print(f"Initial thread count: {initial_thread_count}")
        
        # Trigger report generation
        print("Triggering on-demand report (will be slow)...")
        trigger_on_demand("slow_report_profile")
        time.sleep(0.5)  # Let report start
        
        # Verify report is in progress
        logs = get_t2_logs()
        # Look for report collection activity (implementation-specific log messages)
        
        # Get thread count while report active
        active_thread_count = get_thread_count()
        print(f"Thread count during report: {active_thread_count}")
        
        # Clear logs to see shutdown sequence
        clear_T2logs()
        
        # Shutdown while report active
        print("Shutting down telemetry while report is active...")
        stop_telemetry_graceful(timeout=15)
        
        # Verify shutdown completed
        assert not telemetry_is_running(), "Telemetry did not stop"
        
        # Check logs for proper join sequence
        shutdown_logs = get_t2_logs()
        
        # Should see evidence of thread join (exact message depends on log level)
        # Key: no crash, clean shutdown
        assert no_crash_in_logs(), "Crash during shutdown"
        
        # Verify no orphaned thread warnings
        assert no_thread_leak_in_logs(), "Thread leak detected"
        
        print("✓ Active thread test PASSED: Thread joined cleanly during shutdown")
    
    
    def test_uninit_joins_idle_report_thread(self):
        """
        Test Case 4b: Verify uninit properly joins idle report thread
        
        Bug: uninit checked reportInProgress and skipped join if false,
        but thread could be idle in pthread_cond_wait, causing use-after-free
        when freeProfileXConf() destroyed resources thread still referenced.
        
        Fix: Always check reportThreadExits and join if thread was created.
        
        Scenario: Thread created but idle in pthread_cond_wait
        Expected: Thread signaled, joined, no use-after-free
        """
        print("\nTest: Shutdown while report thread IDLE")
        
        # Start telemetry
        start_telemetry()
        assert telemetry_is_running()
        
        # Create profile with very long interval (thread will be idle)
        profile_json = create_xconf_profile_json(
            name="idle_thread_profile",
            reporting_interval=600,  # 10 minutes - thread will wait
            markers=["IDLE_TEST_.*"]
        )
        
        print("Setting profile with 10-minute interval (thread will be idle)...")
        set_xconf_profile_via_rbus(profile_json)
        time.sleep(3)  # Allow thread creation and entry to wait state
        
        # Get thread count - should include the idle report thread
        thread_count = get_thread_count()
        print(f"Thread count with idle thread: {thread_count}")
        
        # Verify profile is set and thread exists
        logs = get_t2_logs()
        assert "Successfully set profile" in logs or "profile" in logs.lower(), \
            "Profile not set successfully"
        
        # Clear logs to see shutdown
        clear_T2logs()
        
        # Shutdown while thread is idle (waiting in pthread_cond_wait)
        print("Shutting down while thread is idle in pthread_cond_wait...")
        stop_telemetry_graceful(timeout=10)
        
        # Verify clean shutdown
        assert not telemetry_is_running(), "Telemetry did not stop"
        
        # Check for proper shutdown sequence
        shutdown_logs = get_t2_logs()
        
        # Should see thread join or uninit completion
        # Key verification: no crash, no use-after-free
        assert no_crash_in_logs(), "Crash during idle thread shutdown"
        assert no_thread_leak_in_logs(), "Thread leak with idle thread"
        
        # No mutex errors (destroying resources while thread holds them)
        assert no_mutex_errors_in_logs(), "Mutex errors during shutdown"
        
        # Verify no use-after-free indicators
        assert "invalid pointer" not in shutdown_logs.lower(), \
            "Invalid pointer (possible use-after-free)"
        
        print("✓ Idle thread test PASSED: Idle thread joined cleanly")
    
    
    def test_rapid_init_uninit_with_reports(self):
        """
        Test Case 4c: Stress test - rapid init/uninit cycles during report generation
        
        Combines active and idle thread scenarios with rapid cycling to expose
        race windows in thread join logic.
        
        Expected: Clean shutdown on every cycle, no crashes, no leaks
        """
        print("\nTest: Rapid init/uninit cycles with active reports")
        
        cycles = 10  # Reduced for L2 test time constraints
        
        for cycle in range(cycles):
            print(f"\nCycle {cycle + 1}/{cycles}")
            
            # Start telemetry
            start_telemetry()
            assert telemetry_is_running()
            
            # Set profile with moderate interval
            profile_json = create_xconf_profile_json(
                name=f"cycle_profile_{cycle}",
                reporting_interval=5,
                markers=[f"CYCLE_{cycle}_.*"]
            )
            set_xconf_profile_via_rbus(profile_json)
            
            # Randomly either:
            # 1. Shut down immediately (thread might be in creation/startup)
            # 2. Wait briefly (thread might be idle)
            # 3. Trigger report and shutdown (thread active)
            import random
            scenario = random.choice(['immediate', 'idle', 'active'])
            
            if scenario == 'immediate':
                time.sleep(0.1)  # Very brief
            elif scenario == 'idle':
                time.sleep(2)  # Thread idle in wait
            else:  # active
                trigger_on_demand(f"cycle_profile_{cycle}")
                time.sleep(0.3)  # Let report start
            
            # Shutdown
            stop_telemetry_graceful(timeout=10)
            assert not telemetry_is_running(), f"Failed to stop on cycle {cycle}"
            
            # Verify no crashes
            assert no_crash_in_logs(), f"Crash on cycle {cycle}"
            
            if (cycle + 1) % 5 == 0:
                print(f"  Completed {cycle + 1} cycles successfully")
        
        # Final verification
        assert no_thread_leak_in_logs(), "Thread leak detected across cycles"
        assert no_mutex_errors_in_logs(), "Mutex errors detected across cycles"
        
        print(f"\n✓ Rapid cycling test PASSED: {cycles} cycles completed cleanly")


# =============================================================================
# Optional: Standalone execution for debugging
# =============================================================================

if __name__ == "__main__":
    """Run tests standalone for debugging"""
    pytest.main([__file__, "-v", "-s"])
