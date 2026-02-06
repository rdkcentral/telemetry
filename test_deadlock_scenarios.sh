#!/bin/bash

################################################################################
# Telemetry 2.0 Deadlock Simulation Test Script
#
# Purpose: Simulate and detect potential deadlock scenarios identified in the
#          thread synchronization analysis
#
# Usage: ./test_deadlock_scenarios.sh [scenario_number|all]
#
# Scenarios:
#   1: CollectAndReportXconf with network delay deadlock
#   2: unregisterProfileFromScheduler polling deadlock
#   3: deleteAllProfiles pthread_join deadlock
#   4: Concurrent profile operations (lock ordering)
#   5: Profile deletion during report generation
#   6: Rapid enable/disable cycles
#   7: Multiple interrupt signals
#   8: Configuration update during report
#
################################################################################

set -o pipefail

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
TIMEOUT_SECONDS=30
TELEMETRY_BINARY="${TELEMETRY_BINARY:-/usr/bin/telemetry}"
TELEMETRY_PID=""
LOG_DIR="/tmp/telemetry_deadlock_tests"
NETWORK_DELAY_SECONDS=60
MOCK_SERVER_PORT=8888

# Test counters
TESTS_PASSED=0
TESTS_FAILED=0
DEADLOCKS_DETECTED=0

################################################################################
# Utility Functions
################################################################################

print_header() {
    echo ""
    echo -e "${BLUE}========================================================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}========================================================================${NC}"
    echo ""
}

print_test() {
    echo -e "${YELLOW}[TEST]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[PASS]${NC} $1"
    ((TESTS_PASSED++))
}

print_failure() {
    echo -e "${RED}[FAIL]${NC} $1"
    ((TESTS_FAILED++))
}

print_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_deadlock() {
    echo -e "${RED}[DEADLOCK DETECTED]${NC} $1"
    ((DEADLOCKS_DETECTED++))
    ((TESTS_FAILED++))
}

# Initialize test environment
setup_test_env() {
    print_header "Setting up test environment"
    
    mkdir -p "$LOG_DIR"
    rm -f "$LOG_DIR"/*.log 2>/dev/null
    
    echo "Log directory: $LOG_DIR"
    echo "Telemetry binary: $TELEMETRY_BINARY"
    echo "Timeout: ${TIMEOUT_SECONDS}s"
    echo ""
}

# Cleanup function
cleanup() {
    print_header "Cleaning up"
    
    # Kill mock HTTP server
    if [ -n "$MOCK_SERVER_PID" ]; then
        kill $MOCK_SERVER_PID 2>/dev/null || true
        wait $MOCK_SERVER_PID 2>/dev/null || true
    fi
    
    # Kill telemetry daemon
    if [ -n "$TELEMETRY_PID" ]; then
        kill -9 $TELEMETRY_PID 2>/dev/null || true
    fi
    
    pkill -9 -f "t2agent" 2>/dev/null || true
    pkill -9 -f "telemetry" 2>/dev/null || true
    
    # Cleanup profiles
    rm -rf /opt/secure/RFC/.t2persistentfolder/* 2>/dev/null || true
    rm -rf /tmp/.t2reportprofiles/* 2>/dev/null || true
    
    echo "Cleanup complete"
}

trap cleanup EXIT INT TERM

# Start mock HTTP server that introduces delays
start_mock_http_server() {
    local delay=$1
    
    print_test "Starting mock HTTP server with ${delay}s delay on port $MOCK_SERVER_PORT"
    
    python3 - <<EOF &
import http.server
import socketserver
import time
import sys

class DelayedHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    def do_POST(self):
        content_length = int(self.headers.get('Content-Length', 0))
        post_data = self.rfile.read(content_length)
        
        print(f"[Mock Server] Received POST, sleeping for ${delay}s...", file=sys.stderr)
        sys.stderr.flush()
        time.sleep(${delay})
        
        self.send_response(200)
        self.send_header('Content-type', 'text/plain')
        self.end_headers()
        self.wfile.write(b'OK')
        print(f"[Mock Server] Response sent", file=sys.stderr)
        sys.stderr.flush()

    def log_message(self, format, *args):
        pass  # Suppress default logging

PORT = ${MOCK_SERVER_PORT}
with socketserver.TCPServer(("", PORT), DelayedHTTPRequestHandler) as httpd:
    print(f"[Mock Server] Serving on port {PORT}", file=sys.stderr)
    sys.stderr.flush()
    httpd.serve_forever()
EOF
    
    MOCK_SERVER_PID=$!
    sleep 2
    
    if ps -p $MOCK_SERVER_PID > /dev/null 2>&1; then
        echo "Mock server started (PID: $MOCK_SERVER_PID)"
    else
        print_failure "Failed to start mock server"
        return 1
    fi
}

# Monitor process for deadlock (hung state)
monitor_for_deadlock() {
    local pid=$1
    local timeout=$2
    local operation=$3
    local check_interval=1
    
    echo "Monitoring PID $pid for deadlock (timeout: ${timeout}s)..."
    
    local elapsed=0
    local prev_threads=""
    local stuck_count=0
    
    while [ $elapsed -lt $timeout ]; do
        if ! ps -p $pid > /dev/null 2>&1; then
            echo "Process $pid terminated normally"
            return 0
        fi
        
        # Check thread states
        local curr_threads=$(ps -T -p $pid 2>/dev/null | tail -n +2 | wc -l)
        
        # Check for D state (uninterruptible sleep - often indicates deadlock)
        local blocked_threads=$(ps -T -p $pid -o state 2>/dev/null | grep -c "D" || true)
        
        if [ $blocked_threads -gt 0 ]; then
            ((stuck_count++))
            echo "  [$elapsed s] Detected $blocked_threads blocked threads (D state)"
            
            if [ $stuck_count -ge 5 ]; then
                print_deadlock "$operation - Process has blocked threads in D state"
                
                # Capture stack traces
                echo "Capturing stack traces..."
                pstack $pid > "$LOG_DIR/deadlock_stack_${pid}.log" 2>&1 || \
                    gdb -batch -ex "thread apply all bt" -p $pid > "$LOG_DIR/deadlock_stack_${pid}.log" 2>&1 || \
                    echo "Could not capture stack trace (requires gdb or pstack)"
                
                return 1
            fi
        else
            stuck_count=0
        fi
        
        sleep $check_interval
        ((elapsed+=check_interval))
    done
    
    # Timeout reached
    if ps -p $pid > /dev/null 2>&1; then
        print_deadlock "$operation - Process still running after ${timeout}s timeout"
        return 1
    fi
    
    return 0
}

# Check if telemetry is responsive
check_telemetry_responsive() {
    local timeout=5
    
    # Try to query telemetry status via RBUS/dbus
    timeout $timeout dmcli eRT getv Device.X_RDKCENTRAL-COM_T2.ReportProfiles 2>/dev/null || {
        return 1
    }
    
    return 0
}

################################################################################
# Test Scenario 1: CollectAndReportXconf with network delay deadlock
################################################################################

test_scenario_1() {
    print_header "SCENARIO 1: CollectAndReportXconf Network Delay Deadlock"
    
    print_test "This tests if plMutex is held during HTTP upload causing system hang"
    
    # Start mock server with long delay
    start_mock_http_server $NETWORK_DELAY_SECONDS || return 1
    
    # Create XConf profile pointing to mock server
    cat > /tmp/test_xconf_profile.json <<EOF
{
    "Profile": "TestXConfProfile",
    "Protocol": "HTTP",
    "EncodingType": "JSON",
    "ReportingInterval": 10,
    "TimeReference": "0001-01-01T00:00:00Z",
    "HTTP": {
        "URL": "http://localhost:${MOCK_SERVER_PORT}/upload"
    },
    "Parameter": [
        {"name": "Device.DeviceInfo.SoftwareVersion", "reference": "SoftwareVersion"}
    ]
}
EOF
    
    # Upload profile via RFC
    print_test "Uploading XConf profile to telemetry"
    dmcli eRT setv Device.X_RDKCENTRAL-COM_T2.ReportProfiles string "$(cat /tmp/test_xconf_profile.json)" 2>&1 | tee "$LOG_DIR/upload_profile.log"
    
    sleep 5
    
    # Trigger report generation (this will start HTTP upload with delay)
    print_test "Triggering report generation..."
    dmcli eRT setv Device.X_RDKCENTRAL-COM_T2.ReportProfiles.TestXConfProfile.TriggerNow bool true 2>&1 &
    local trigger_pid=$!
    
    sleep 3
    
    # While upload is in progress (plMutex held), try to query profile status
    print_test "Attempting to query profile status while upload in progress..."
    
    local start_time=$(date +%s)
    timeout 10 dmcli eRT getv Device.X_RDKCENTRAL-COM_T2.ReportProfiles.TestXConfProfile.Enable 2>&1 > "$LOG_DIR/query_during_upload.log" &
    local query_pid=$!
    
    # Wait for query with timeout
    local query_timeout=10
    local elapsed=0
    while ps -p $query_pid > /dev/null 2>&1 && [ $elapsed -lt $query_timeout ]; do
        sleep 1
        ((elapsed++))
    done
    
    if ps -p $query_pid > /dev/null 2>&1; then
        kill -9 $query_pid 2>/dev/null
        print_deadlock "Profile query blocked for ${query_timeout}s while report upload in progress (plMutex held)"
    else
        local end_time=$(date +%s)
        local duration=$((end_time - start_time))
        
        if [ $duration -lt 3 ]; then
            print_success "Profile query completed quickly (${duration}s) - no deadlock"
        else
            print_warning "Profile query took ${duration}s - possible contention but no deadlock"
        fi
    fi
    
    # Wait for upload to complete
    wait $trigger_pid 2>/dev/null || true
    
    # Cleanup
    rm -f /tmp/test_xconf_profile.json
}

################################################################################
# Test Scenario 2: unregisterProfileFromScheduler polling deadlock
################################################################################

test_scenario_2() {
    print_header "SCENARIO 2: unregisterProfileFromScheduler Polling Deadlock"
    
    print_test "This tests if scMutex is held during sleep polling causing deadlock"
    
    # Create multiple profiles
    for i in {1..3}; do
        cat > /tmp/test_profile_$i.json <<EOF
{
    "Profile": "TestProfile$i",
    "Protocol": "HTTP",
    "EncodingType": "JSON",
    "ReportingInterval": 300,
    "HTTP": {
        "URL": "http://localhost:${MOCK_SERVER_PORT}/upload"
    },
    "Parameter": [
        {"name": "Device.DeviceInfo.Manufacturer", "reference": "Manufacturer"}
    ]
}
EOF
        dmcli eRT setv Device.X_RDKCENTRAL-COM_T2.ReportProfiles string "$(cat /tmp/test_profile_$i.json)" 2>&1 | tee -a "$LOG_DIR/upload_profiles.log"
        sleep 1
    done
    
    sleep 5
    
    # Start unregistering profile (will hold scMutex and poll)
    print_test "Unregistering TestProfile1 (may hold scMutex during polling)..."
    dmcli eRT setv Device.X_RDKCENTRAL-COM_T2.ReportProfiles.TestProfile1.Enable bool false 2>&1 &
    local unreg_pid=$!
    
    sleep 2
    
    # While unregister is polling, try to register new profile (needs scMutex)
    print_test "Attempting to register new profile while unregister is polling..."
    
    cat > /tmp/test_profile_new.json <<EOF
{
    "Profile": "TestProfileNew",
    "Protocol": "HTTP",
    "EncodingType": "JSON",
    "ReportingInterval": 60,
    "HTTP": {
        "URL": "http://localhost:${MOCK_SERVER_PORT}/upload"
    },
    "Parameter": [
        {"name": "Device.DeviceInfo.ModelName", "reference": "ModelName"}
    ]
}
EOF
    
    local start_time=$(date +%s)
    timeout 15 dmcli eRT setv Device.X_RDKCENTRAL-COM_T2.ReportProfiles string "$(cat /tmp/test_profile_new.json)" 2>&1 > "$LOG_DIR/register_during_unreg.log" &
    local reg_pid=$!
    
    # Monitor both operations
    local timeout=15
    local elapsed=0
    while [ $elapsed -lt $timeout ]; do
        local unreg_alive=$(ps -p $unreg_pid > /dev/null 2>&1 && echo 1 || echo 0)
        local reg_alive=$(ps -p $reg_pid > /dev/null 2>&1 && echo 1 || echo 0)
        
        if [ $unreg_alive -eq 0 ] && [ $reg_alive -eq 0 ]; then
            break
        fi
        
        sleep 1
        ((elapsed++))
    done
    
    # Check if both completed
    if ps -p $unreg_pid > /dev/null 2>&1 || ps -p $reg_pid > /dev/null 2>&1; then
        print_deadlock "Profile operations deadlocked - scMutex held during polling"
        kill -9 $unreg_pid $reg_pid 2>/dev/null || true
    else
        local end_time=$(date +%s)
        local duration=$((end_time - start_time))
        print_success "Both operations completed in ${duration}s - no deadlock"
    fi
    
    # Cleanup
    rm -f /tmp/test_profile_*.json
}

################################################################################
# Test Scenario 3: deleteAllProfiles pthread_join deadlock
################################################################################

test_scenario_3() {
    print_header "SCENARIO 3: deleteAllProfiles pthread_join Deadlock"
    
    print_test "This tests if plMutex is held during pthread_join causing circular wait"
    
    # Create profiles
    for i in {1..3}; do
        cat > /tmp/del_profile_$i.json <<EOF
{
    "Profile": "DelProfile$i",
    "Protocol": "HTTP",
    "EncodingType": "JSON",
    "ReportingInterval": 600,
    "HTTP": {
        "URL": "http://localhost:${MOCK_SERVER_PORT}/upload"
    },
    "Parameter": [
        {"name": "Device.DeviceInfo.SoftwareVersion", "reference": "SoftwareVersion"}
    ]
}
EOF
        dmcli eRT setv Device.X_RDKCENTRAL-COM_T2.ReportProfiles string "$(cat /tmp/del_profile_$i.json)" 2>&1 | tee -a "$LOG_DIR/create_profiles.log"
        sleep 1
    done
    
    sleep 5
    
    # Trigger reports on all profiles simultaneously
    print_test "Triggering reports on all profiles..."
    for i in {1..3}; do
        dmcli eRT setv Device.X_RDKCENTRAL-COM_T2.ReportProfiles.DelProfile$i.TriggerNow bool true 2>&1 &
    done
    
    sleep 2
    
    # While reports are being generated, delete all profiles
    print_test "Deleting all profiles while reports in progress..."
    
    local start_time=$(date +%s)
    timeout 20 dmcli eRT setv Device.X_RDKCENTRAL-COM_T2.ClearAllProfiles bool true 2>&1 > "$LOG_DIR/delete_all.log" &
    local delete_pid=$!
    
    # Monitor for deadlock
    local timeout=20
    local elapsed=0
    while ps -p $delete_pid > /dev/null 2>&1 && [ $elapsed -lt $timeout ]; do
        sleep 1
        ((elapsed++))
        
        if [ $((elapsed % 5)) -eq 0 ]; then
            echo "  Still deleting... (${elapsed}s)"
        fi
    done
    
    if ps -p $delete_pid > /dev/null 2>&1; then
        print_deadlock "deleteAllProfiles hung for ${timeout}s - plMutex held during pthread_join"
        kill -9 $delete_pid 2>/dev/null || true
    else
        local end_time=$(date +%s)
        local duration=$((end_time - start_time))
        print_success "All profiles deleted in ${duration}s - no deadlock"
    fi
    
    # Cleanup
    rm -f /tmp/del_profile_*.json
}

################################################################################
# Test Scenario 4: Concurrent profile operations (lock ordering)
################################################################################

test_scenario_4() {
    print_header "SCENARIO 4: Concurrent Profile Operations (Lock Ordering)"
    
    print_test "This tests for lock ordering violations with concurrent operations"
    
    # Create profile
    cat > /tmp/concurrent_profile.json <<EOF
{
    "Profile": "ConcurrentProfile",
    "Protocol": "HTTP",
    "EncodingType": "JSON",
    "ReportingInterval": 60,
    "HTTP": {
        "URL": "http://localhost:${MOCK_SERVER_PORT}/upload"
    },
    "Parameter": [
        {"name": "Device.DeviceInfo.Manufacturer", "reference": "Manufacturer"}
    ]
}
EOF
    
    dmcli eRT setv Device.X_RDKCENTRAL-COM_T2.ReportProfiles string "$(cat /tmp/concurrent_profile.json)" 2>&1 | tee "$LOG_DIR/create_concurrent.log"
    sleep 3
    
    # Launch multiple concurrent operations
    print_test "Launching 10 concurrent operations..."
    
    local pids=()
    
    # Trigger reports
    for i in {1..3}; do
        dmcli eRT setv Device.X_RDKCENTRAL-COM_T2.ReportProfiles.ConcurrentProfile.TriggerNow bool true 2>&1 > "$LOG_DIR/trigger_$i.log" &
        pids+=($!)
    done
    
    # Query status
    for i in {1..3}; do
        dmcli eRT getv Device.X_RDKCENTRAL-COM_T2.ReportProfiles.ConcurrentProfile.Enable 2>&1 > "$LOG_DIR/query_$i.log" &
        pids+=($!)
    done
    
    # Update profile
    for i in {1..2}; do
        dmcli eRT setv Device.X_RDKCENTRAL-COM_T2.ReportProfiles string "$(cat /tmp/concurrent_profile.json)" 2>&1 > "$LOG_DIR/update_$i.log" &
        pids+=($!)
    done
    
    # Disable/enable
    for i in {1..2}; do
        dmcli eRT setv Device.X_RDKCENTRAL-COM_T2.ReportProfiles.ConcurrentProfile.Enable bool false 2>&1 > "$LOG_DIR/disable_$i.log" &
        pids+=($!)
    done
    
    # Wait for all with timeout
    local timeout=30
    local start_time=$(date +%s)
    local all_completed=true
    
    for pid in "${pids[@]}"; do
        local elapsed=0
        while ps -p $pid > /dev/null 2>&1 && [ $elapsed -lt $timeout ]; do
            sleep 1
            ((elapsed++))
        done
        
        if ps -p $pid > /dev/null 2>&1; then
            kill -9 $pid 2>/dev/null || true
            all_completed=false
        fi
    done
    
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    
    if $all_completed; then
        print_success "All concurrent operations completed in ${duration}s - no deadlock"
    else
        print_deadlock "Some operations hung - possible lock ordering violation"
    fi
    
    # Cleanup
    rm -f /tmp/concurrent_profile.json
}

################################################################################
# Test Scenario 5: Profile deletion during report generation
################################################################################

test_scenario_5() {
    print_header "SCENARIO 5: Profile Deletion During Report Generation"
    
    print_test "This tests deletion while CollectAndReport is active"
    
    # Start mock server with delay
    start_mock_http_server 30 || return 1
    
    # Create profile
    cat > /tmp/delete_during_report.json <<EOF
{
    "Profile": "DeleteDuringReport",
    "Protocol": "HTTP",
    "EncodingType": "JSON",
    "ReportingInterval": 300,
    "HTTP": {
        "URL": "http://localhost:${MOCK_SERVER_PORT}/upload"
    },
    "Parameter": [
        {"name": "Device.DeviceInfo.Manufacturer", "reference": "Manufacturer"}
    ]
}
EOF
    
    dmcli eRT setv Device.X_RDKCENTRAL-COM_T2.ReportProfiles string "$(cat /tmp/delete_during_report.json)" 2>&1 | tee "$LOG_DIR/create_delete_profile.log"
    sleep 3
    
    # Trigger report (will take 30s due to HTTP delay)
    print_test "Triggering report generation (30s HTTP delay)..."
    dmcli eRT setv Device.X_RDKCENTRAL-COM_T2.ReportProfiles.DeleteDuringReport.TriggerNow bool true 2>&1 &
    
    sleep 3
    
    # Try to delete profile while report is being generated
    print_test "Deleting profile while report in progress..."
    
    local start_time=$(date +%s)
    timeout 45 dmcli eRT setv Device.X_RDKCENTRAL-COM_T2.ReportProfiles.DeleteDuringReport.Enable bool false 2>&1 > "$LOG_DIR/delete_during_report.log" &
    local delete_pid=$!
    
    # Monitor
    local timeout=45
    local elapsed=0
    while ps -p $delete_pid > /dev/null 2>&1 && [ $elapsed -lt $timeout ]; do
        sleep 1
        ((elapsed++))
        
        if [ $((elapsed % 10)) -eq 0 ]; then
            echo "  Waiting for deletion... (${elapsed}s)"
        fi
    done
    
    if ps -p $delete_pid > /dev/null 2>&1; then
        print_deadlock "Profile deletion hung - waiting for report completion with plMutex held"
        kill -9 $delete_pid 2>/dev/null || true
    else
        local end_time=$(date +%s)
        local duration=$((end_time - start_time))
        print_success "Profile deleted in ${duration}s - proper synchronization"
    fi
    
    # Cleanup
    rm -f /tmp/delete_during_report.json
}

################################################################################
# Test Scenario 6: Rapid enable/disable cycles
################################################################################

test_scenario_6() {
    print_header "SCENARIO 6: Rapid Enable/Disable Cycles"
    
    print_test "This tests rapid state changes for race conditions"
    
    # Create profile
    cat > /tmp/rapid_profile.json <<EOF
{
    "Profile": "RapidProfile",
    "Protocol": "HTTP",
    "EncodingType": "JSON",
    "ReportingInterval": 60,
    "HTTP": {
        "URL": "http://localhost:${MOCK_SERVER_PORT}/upload"
    },
    "Parameter": [
        {"name": "Device.DeviceInfo.Manufacturer", "reference": "Manufacturer"}
    ]
}
EOF
    
    dmcli eRT setv Device.X_RDKCENTRAL-COM_T2.ReportProfiles string "$(cat /tmp/rapid_profile.json)" 2>&1 | tee "$LOG_DIR/create_rapid.log"
    sleep 3
    
    print_test "Performing 20 rapid enable/disable cycles..."
    
    local cycles=20
    local failed=false
    
    for i in $(seq 1 $cycles); do
        # Disable
        timeout 5 dmcli eRT setv Device.X_RDKCENTRAL-COM_T2.ReportProfiles.RapidProfile.Enable bool false 2>&1 > "$LOG_DIR/rapid_disable_$i.log" &
        local disable_pid=$!
        
        sleep 0.1
        
        # Enable
        timeout 5 dmcli eRT setv Device.X_RDKCENTRAL-COM_T2.ReportProfiles.RapidProfile.Enable bool true 2>&1 > "$LOG_DIR/rapid_enable_$i.log" &
        local enable_pid=$!
        
        # Wait for both
        wait $disable_pid 2>/dev/null || {
            print_warning "Disable operation $i timed out"
            failed=true
        }
        
        wait $enable_pid 2>/dev/null || {
            print_warning "Enable operation $i timed out"
            failed=true
        }
        
        if [ $((i % 5)) -eq 0 ]; then
            echo "  Completed $i/$cycles cycles"
        fi
    done
    
    if $failed; then
        print_failure "Some rapid operations timed out - possible race condition"
    else
        print_success "Completed $cycles rapid enable/disable cycles without deadlock"
    fi
    
    # Cleanup
    rm -f /tmp/rapid_profile.json
}

################################################################################
# Test Scenario 7: Multiple interrupt signals
################################################################################

test_scenario_7() {
    print_header "SCENARIO 7: Multiple Interrupt Signals"
    
    print_test "This tests SendInterruptToTimeoutThread with trylock behavior"
    
    # Create profile with long interval
    cat > /tmp/interrupt_profile.json <<EOF
{
    "Profile": "InterruptProfile",
    "Protocol": "HTTP",
    "EncodingType": "JSON",
    "ReportingInterval": 3600,
    "HTTP": {
        "URL": "http://localhost:${MOCK_SERVER_PORT}/upload"
    },
    "Parameter": [
        {"name": "Device.DeviceInfo.Manufacturer", "reference": "Manufacturer"}
    ]
}
EOF
    
    dmcli eRT setv Device.X_RDKCENTRAL-COM_T2.ReportProfiles string "$(cat /tmp/interrupt_profile.json)" 2>&1 | tee "$LOG_DIR/create_interrupt.log"
    sleep 3
    
    print_test "Sending 10 interrupt signals rapidly..."
    
    local pids=()
    for i in $(seq 1 10); do
        dmcli eRT setv Device.X_RDKCENTRAL-COM_T2.ReportProfiles.InterruptProfile.TriggerNow bool true 2>&1 > "$LOG_DIR/interrupt_$i.log" &
        pids+=($!)
        sleep 0.2
    done
    
    # Wait for all with timeout
    local all_completed=true
    local timeout=30
    
    for pid in "${pids[@]}"; do
        local elapsed=0
        while ps -p $pid > /dev/null 2>&1 && [ $elapsed -lt $timeout ]; do
            sleep 1
            ((elapsed++))
        done
        
        if ps -p $pid > /dev/null 2>&1; then
            kill -9 $pid 2>/dev/null || true
            all_completed=false
        fi
    done
    
    if $all_completed; then
        print_success "All interrupt signals handled without deadlock"
    else
        print_deadlock "Some interrupt operations hung - trylock may have failed"
    fi
    
    # Cleanup
    rm -f /tmp/interrupt_profile.json
}

################################################################################
# Test Scenario 8: Configuration update during report
################################################################################

test_scenario_8() {
    print_header "SCENARIO 8: Configuration Update During Report"
    
    print_test "This tests profile update while report is being generated"
    
    # Start mock server with delay
    start_mock_http_server 20 || return 1
    
    # Create initial profile
    cat > /tmp/update_profile_v1.json <<EOF
{
    "Profile": "UpdateProfile",
    "Protocol": "HTTP",
    "EncodingType": "JSON",
    "ReportingInterval": 300,
    "HTTP": {
        "URL": "http://localhost:${MOCK_SERVER_PORT}/upload"
    },
    "Parameter": [
        {"name": "Device.DeviceInfo.Manufacturer", "reference": "Manufacturer"}
    ]
}
EOF
    
    dmcli eRT setv Device.X_RDKCENTRAL-COM_T2.ReportProfiles string "$(cat /tmp/update_profile_v1.json)" 2>&1 | tee "$LOG_DIR/create_update_profile.log"
    sleep 3
    
    # Trigger report
    print_test "Triggering report generation (20s delay)..."
    dmcli eRT setv Device.X_RDKCENTRAL-COM_T2.ReportProfiles.UpdateProfile.TriggerNow bool true 2>&1 &
    
    sleep 3
    
    # Update profile while report is being generated
    print_test "Updating profile configuration while report in progress..."
    
    cat > /tmp/update_profile_v2.json <<EOF
{
    "Profile": "UpdateProfile",
    "Protocol": "HTTP",
    "EncodingType": "JSON",
    "ReportingInterval": 600,
    "HTTP": {
        "URL": "http://localhost:${MOCK_SERVER_PORT}/upload"
    },
    "Parameter": [
        {"name": "Device.DeviceInfo.Manufacturer", "reference": "Manufacturer"},
        {"name": "Device.DeviceInfo.ModelName", "reference": "ModelName"}
    ]
}
EOF
    
    local start_time=$(date +%s)
    timeout 30 dmcli eRT setv Device.X_RDKCENTRAL-COM_T2.ReportProfiles string "$(cat /tmp/update_profile_v2.json)" 2>&1 > "$LOG_DIR/update_during_report.log" &
    local update_pid=$!
    
    # Monitor
    local timeout=30
    local elapsed=0
    while ps -p $update_pid > /dev/null 2>&1 && [ $elapsed -lt $timeout ]; do
        sleep 1
        ((elapsed++))
        
        if [ $((elapsed % 5)) -eq 0 ]; then
            echo "  Waiting for update... (${elapsed}s)"
        fi
    done
    
    if ps -p $update_pid > /dev/null 2>&1; then
        print_deadlock "Profile update hung - lock contention with active report"
        kill -9 $update_pid 2>/dev/null || true
    else
        local end_time=$(date +%s)
        local duration=$((end_time - start_time))
        print_success "Profile updated in ${duration}s - no deadlock"
    fi
    
    # Cleanup
    rm -f /tmp/update_profile_v*.json
}

################################################################################
# Main execution
################################################################################

main() {
    local scenario="${1:-all}"
    
    print_header "Telemetry 2.0 Deadlock Simulation Tests"
    echo "Test Date: $(date)"
    echo "Scenario: $scenario"
    
    setup_test_env
    
    # Check if telemetry is running
    if ! pgrep -f "t2agent|telemetry" > /dev/null; then
        print_warning "Telemetry daemon not running. Some tests may not work."
        print_warning "Start telemetry daemon and retry."
    fi
    
    case "$scenario" in
        1)
            test_scenario_1
            ;;
        2)
            test_scenario_2
            ;;
        3)
            test_scenario_3
            ;;
        4)
            test_scenario_4
            ;;
        5)
            test_scenario_5
            ;;
        6)
            test_scenario_6
            ;;
        7)
            test_scenario_7
            ;;
        8)
            test_scenario_8
            ;;
        all)
            test_scenario_1
            test_scenario_2
            test_scenario_3
            test_scenario_4
            test_scenario_5
            test_scenario_6
            test_scenario_7
            test_scenario_8
            ;;
        *)
            echo "Usage: $0 [1-8|all]"
            echo "  1: CollectAndReportXconf network delay deadlock"
            echo "  2: unregisterProfileFromScheduler polling deadlock"
            echo "  3: deleteAllProfiles pthread_join deadlock"
            echo "  4: Concurrent profile operations"
            echo "  5: Profile deletion during report"
            echo "  6: Rapid enable/disable cycles"
            echo "  7: Multiple interrupt signals"
            echo "  8: Configuration update during report"
            echo "  all: Run all scenarios"
            exit 1
            ;;
    esac
    
    # Print summary
    print_header "Test Summary"
    echo "Tests Passed: $TESTS_PASSED"
    echo "Tests Failed: $TESTS_FAILED"
    echo "Deadlocks Detected: $DEADLOCKS_DETECTED"
    echo ""
    echo "Logs saved to: $LOG_DIR"
    
    if [ $DEADLOCKS_DETECTED -gt 0 ]; then
        echo ""
        print_warning "DEADLOCKS DETECTED! Review logs and stack traces in $LOG_DIR"
        exit 1
    elif [ $TESTS_FAILED -gt 0 ]; then
        exit 1
    else
        print_success "All tests passed!"
        exit 0
    fi
}

# Run main with all arguments
main "$@"
