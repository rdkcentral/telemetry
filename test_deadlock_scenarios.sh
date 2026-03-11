#!/bin/sh
##########################################################################
# test_deadlock_scenarios.sh
#
# Reproduce HIGH-priority thread synchronization issues in Telemetry 2.0
# using the standard container development environment.
#
# Issues covered:
#   A [CRITICAL] profilexconf.c:CollectAndReportXconf acquires plMutex at
#               line 208 and never releases it through data collection,
#               JSON encoding, and HTTP upload.  All intermediate unlock
#               calls (lines 257, 325, 359, 504) are commented out.
#               Any concurrent caller that needs plMutex is blocked for
#               the full upload duration.
#
#   B [HIGH]    profile.c:deleteAllProfiles (line 1190) sends cond_signal
#               before pthread_join.  If CollectAndReport is mid-report
#               the signal is lost; the thread finishes, calls cond_wait,
#               and blocks forever.  pthread_join never returns.
#               Additionally plMutex is held across pthread_join (line
#               1193), risking circular wait if the report thread needs
#               plMutex to complete.
#
#   C [HIGH]    3-way lock cycle:
#                 enableProfile:           plMutex -> scMutex
#                 unregisterFromScheduler: scMutex -> tMutex
#                 TimeoutThread callback:  tMutex  -> plMutex
#               Under the right interleaving all three threads block on
#               the next lock in the cycle.
#
# Usage:  ./test_deadlock_scenarios.sh [A|B|C|all]
#
# Prerequisites (all present in the standard build container):
#   - telemetry2_0 running  (start with: /usr/local/bin/telemetry2_0 &)
#   - rbuscli on PATH
#   - python3 on PATH
#
# For deterministic race/order detection rebuild with ThreadSanitizer:
#   CFLAGS="-fsanitize=thread -g" ./build_inside_container.sh
#   Then re-run any scenario; TSan reports violations on the first run.
#
# See: docs/architecture/threading-model.md for full analysis.
##########################################################################

set -u

MOCK_PORT="${MOCK_PORT:-18888}"
LOG_DIR="/tmp/t2_deadlock_repro"
MOCK_SCRIPT="$LOG_DIR/mock_server.py"
MOCK_PID=""

# Seconds the mock server delays every POST (simulates a slow upload).
# Must be long enough that concurrent operations can be observed while
# the upload is still in flight.
HTTP_DELAY=30

# How long to wait before declaring an operation hung.
# Set slightly beyond HTTP_DELAY so a legitimate slow upload is not a
# false positive.
HANG_TIMEOUT=$(( HTTP_DELAY + 12 ))

PASS=0
FAIL=0


##########################################################################
# Utilities
##########################################################################

log_info() { printf '[INFO]  %s\n'   "$*"; }
log_pass() { printf '[PASS]  %s\n'   "$*"; PASS=$(( PASS + 1 )); }
log_fail() { printf '[FAIL]  %s\n'   "$*"; FAIL=$(( FAIL + 1 )); }
log_warn() { printf '[WARN]  %s\n'   "$*" >&2; }
log_sep()  { printf '\n--- %s ---\n' "$*"; }

cleanup() {
    stop_mock
    rm -f "$MOCK_SCRIPT"
}
trap cleanup EXIT INT TERM

check_prereqs() {
    mkdir -p "$LOG_DIR"
    for cmd in rbuscli python3; do
        if ! command -v "$cmd" > /dev/null 2>&1; then
            printf '[ERROR] %s not found on PATH\n' "$cmd" >&2
            exit 1
        fi
    done
    if ! pgrep -f telemetry2_0 > /dev/null 2>&1; then
        log_warn "telemetry2_0 is not running — start with: /usr/local/bin/telemetry2_0 &"
    fi
}

# Write a Python3 HTTP server to a temp file.
#   GET  any path -> XConf telemetry profile JSON (upload URL = this server)
#   POST any path -> sleep DELAY seconds, then HTTP 200
# The delay simulates a slow data-lake upload so concurrent operations can
# be observed while the upload is still in flight.
write_mock_server() {
    local delay="$1"
    cat > "$MOCK_SCRIPT" << PYEOF
import http.server, socketserver, time, json, sys

PORT  = $MOCK_PORT
DELAY = $delay

PROFILE = json.dumps({
    "profiles": [{
        "name":  "ReproXConf",
        "hash":  "hash-repro-1",
        "value": {
            "Name":              "ReproXConf",
            "Protocol":         "HTTP",
            "EncodingType":     "JSON",
            "ReportingInterval": 10,
            "RootName":         "T2Report",
            "TimeReference":    "0001-01-01T00:00:00Z",
            "Parameter": [{"type": "dataModel", "name": "SWVersion",
                           "reference": "Device.DeviceInfo.SoftwareVersion"}],
            "HTTP": {
                "URL":         "http://127.0.0.1:" + str(PORT) + "/upload",
                "Compression": "None",
                "Method":      "POST"
            },
            "JSONEncoding": {"ReportFormat": "NameValuePair",
                             "ReportTimestamp": "None"}
        }
    }]
}).encode()

class Handler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.send_header("Content-Type",   "application/json")
        self.send_header("Content-Length", str(len(PROFILE)))
        self.end_headers()
        self.wfile.write(PROFILE)

    def do_POST(self):
        n = int(self.headers.get("Content-Length", "0"))
        self.rfile.read(n)
        sys.stderr.write("[mock] POST received; sleeping %ds\n" % DELAY)
        sys.stderr.flush()
        time.sleep(DELAY)
        self.send_response(200)
        self.end_headers()
        self.wfile.write(b"OK")

    def log_message(self, *args):
        pass  # suppress per-request access log

socketserver.TCPServer.allow_reuse_address = True
with socketserver.TCPServer(("", PORT), Handler) as srv:
    sys.stderr.write("[mock] listening on port %d (delay=%ds)\n" % (PORT, DELAY))
    sys.stderr.flush()
    srv.serve_forever()
PYEOF
}

start_mock() {
    local delay="${1:-$HTTP_DELAY}"
    write_mock_server "$delay"
    python3 "$MOCK_SCRIPT" 2>> "$LOG_DIR/mock_server.log" &
    MOCK_PID=$!
    sleep 1
    if ! kill -0 "$MOCK_PID" 2>/dev/null; then
        log_fail "Mock HTTP server failed to start — see $LOG_DIR/mock_server.log"
        MOCK_PID=""
        return 1
    fi
    log_info "Mock HTTP server running  PID=$MOCK_PID  port=$MOCK_PORT  delay=${delay}s"
}

stop_mock() {
    if [ -n "$MOCK_PID" ]; then
        kill "$MOCK_PID" 2>/dev/null || true
        wait "$MOCK_PID" 2>/dev/null || true
        MOCK_PID=""
    fi
}

# wait_with_timeout PID SECONDS
# Returns 0 if PID exits within SECONDS.
# Kills PID and returns 1 if still running after SECONDS.
wait_with_timeout() {
    local pid="$1"
    local limit="$2"
    local elapsed=0
    while kill -0 "$pid" 2>/dev/null && [ "$elapsed" -lt "$limit" ]; do
        sleep 1
        elapsed=$(( elapsed + 1 ))
    done
    if kill -0 "$pid" 2>/dev/null; then
        kill -9 "$pid" 2>/dev/null || true
        return 1
    fi
    return 0
}

##########################################################################
# SCENARIO A — CRITICAL
#
# Root cause: profilexconf.c CollectAndReportXconf() acquires plMutex at
# line 208 and holds it for the entire report loop.  Every intermediate
# pthread_mutex_unlock(&plMutex) is commented out (lines 257, 325, 359,
# 504).  The mutex is released only when pthread_cond_wait is called at
# line 507 — after the HTTP upload has completely finished.
#
# Impact: updateProfileXConf, ProfileXConf_notifyTimeout, ProfileXConf_isSet
# and ProfileXconf_getName all block for the full upload duration, making
# the XConf layer unresponsive during every report transmission.
#
# How reproduced:
#   1. Start mock HTTP server: GET -> profile JSON, POST -> sleep HTTP_DELAY s.
#   2. Set XConf ConfigURL to mock; telemetry fetches a profile with a 10 s
#      reporting interval whose upload URL also points to the mock.
#   3. Wait 15 s for profile activation and first upload to start.
#   4. Issue a second ConfigURL write (needs profilexconf.plMutex).
#   5. Under the bug: blocks ~25 s.  Without the bug: completes in < 2 s.
##########################################################################
scenario_a() {
    log_sep "SCENARIO A [CRITICAL] — XConf plMutex held for entire HTTP upload"

    start_mock "$HTTP_DELAY" || return

    log_info "Setting XConf ConfigURL to mock server..."
    rbuscli set \
        "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.ConfigURL" \
        string \
        "http://127.0.0.1:$MOCK_PORT/loguploader/getT2DCMSettings" \
        > "$LOG_DIR/A_set_url.log" 2>&1

    # Allow time for: xconf fetch, profile activation, 10 s reporting interval
    # to fire, and CollectAndReportXconf to acquire plMutex and begin upload.
    log_info "Waiting 15s for profile activation and first upload to start..."
    sleep 15

    # Issue a new ConfigURL write; updateProfileXConf needs profilexconf.plMutex.
    # Without bug: < 2 s.   With bug: blocks until the 30 s upload finishes.
    log_info "Updating ConfigURL while upload in-flight (expect < 2s without bug)..."
    local t0
    t0=$(date +%s)
    rbuscli set \
        "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.ConfigURL" \
        string \
        "http://127.0.0.1:$MOCK_PORT/loguploader/getT2DCMSettings" \
        > "$LOG_DIR/A_update_url.log" 2>&1 &
    local update_pid=$!

    if wait_with_timeout "$update_pid" 5; then
        local elapsed=$(( $(date +%s) - t0 ))
        if [ "$elapsed" -le 2 ]; then
            log_pass "Scenario A: update completed in ${elapsed}s — plMutex not held during upload"
        else
            log_fail "Scenario A: update took ${elapsed}s — plMutex contention with upload confirmed"
        fi
    else
        log_fail "Scenario A: update blocked >5s — plMutex held throughout HTTP upload [CRITICAL]"
        log_info "  Fix: release plMutex before network I/O in CollectAndReportXconf"
        log_info "  Ref: profilexconf.c lines 208-511  threading-model.md Scenario 2"
    fi

    stop_mock
    sleep 2
}

##########################################################################
# SCENARIO B — HIGH
#
# Root cause 1 (signal loss): deleteAllProfiles (profile.c:1190) sends
# pthread_cond_signal on reuseThread then immediately calls pthread_join.
# If CollectAndReport is mid-execution when the signal fires (not yet at
# cond_wait), the signal is silently dropped.  The thread finishes its
# cycle, calls cond_wait, and blocks forever.  pthread_join never returns.
#
# Root cause 2 (mutex under join): plMutex is held across pthread_join at
# line 1193.  If CollectAndReport needs plMutex to finish its current
# cycle before reaching cond_wait, the calls deadlock.
#
# How reproduced:
#   1. Create 3 multi-profiles with a 5 s reporting interval pointing at the
#      slow mock server (HTTP_DELAY seconds per upload).
#   2. Wait 8 s: CollectAndReport threads are mid-execution (they have set
#      reportInProgress=true but are still uploading, NOT at cond_wait yet).
#   3. Push an empty profile list, triggering deleteAllReportProfiles()
#      -> deleteAllProfiles() for all three profiles.
#   4. Under the bug: hangs indefinitely.
#      Without: completes after the in-flight uploads finish (~22 s).
##########################################################################
scenario_b() {
    log_sep "SCENARIO B [HIGH] — deleteAllProfiles signal-loss / pthread_join deadlock"

    start_mock "$HTTP_DELAY" || return

    log_info "Creating 3 multi-profiles with 5-second reporting interval..."
    local i=1
    while [ "$i" -le 3 ]; do
        local json
        json=$(printf \
            '{"profiles":[{"name":"ReproDelProf%d","hash":"hB%d","value":{"Name":"ReproDelProf%d","Protocol":"HTTP","EncodingType":"JSON","ReportingInterval":5,"RootName":"T2Report","TimeReference":"0001-01-01T00:00:00Z","Parameter":[{"type":"dataModel","name":"SWVer","reference":"Device.DeviceInfo.SoftwareVersion"}],"HTTP":{"URL":"http://127.0.0.1:%d/upload","Compression":"None","Method":"POST"},"JSONEncoding":{"ReportFormat":"NameValuePair","ReportTimestamp":"None"}}}]}' \
            "$i" "$i" "$i" "$MOCK_PORT")
        rbuscli set Device.X_RDKCENTRAL-COM_T2.Temp_ReportProfiles string "$json" \
            > "$LOG_DIR/B_create_${i}.log" 2>&1
        i=$(( i + 1 ))
    done

    # Wait long enough for the 5 s interval to fire and CollectAndReport to
    # start — but NOT long enough for it to reach cond_wait at end of cycle.
    log_info "Waiting 8s for report threads to start (mid-execution, not at cond_wait)..."
    sleep 8

    log_info "Pushing empty profile list to trigger deleteAllProfiles..."
    local t0
    t0=$(date +%s)
    rbuscli set Device.X_RDKCENTRAL-COM_T2.Temp_ReportProfiles string '{"profiles":[]}' \
        > "$LOG_DIR/B_delete_all.log" 2>&1 &
    local del_pid=$!

    if wait_with_timeout "$del_pid" "$HANG_TIMEOUT"; then
        local elapsed=$(( $(date +%s) - t0 ))
        log_pass "Scenario B: deleteAllProfiles completed in ${elapsed}s"
    else
        log_fail "Scenario B: deleteAllProfiles hung for >${HANG_TIMEOUT}s — signal-loss or deadlock [HIGH]"
        log_info "  Fix 1: use cond_timedwait in CollectAndReport (reliable wake-up)"
        log_info "  Fix 2: release plMutex before pthread_join in deleteAllProfiles"
        log_info "  Ref: profile.c lines 1184-1200  threading-model.md Scenario 4"
        local t2pid
        t2pid=$(pgrep -f telemetry2_0 2>/dev/null || true)
        if [ -n "$t2pid" ]; then
            cat /proc/"$t2pid"/status > "$LOG_DIR/B_proc_status.log" 2>/dev/null || true
            log_info "  Process status saved: $LOG_DIR/B_proc_status.log"
        fi
    fi

    stop_mock
    sleep 2
}

##########################################################################
# SCENARIO C — HIGH
#
# Root cause: three mutexes form a cycle:
#
#   enableProfile (profile.c):
#     Holds plMutex, calls registerProfileWithScheduler -> acquires scMutex.
#     Order: plMutex -> scMutex
#
#   unregisterProfileFromScheduler (scheduler.c):
#     Holds scMutex, acquires tMutex to signal the TimeoutThread.
#     Order: scMutex -> tMutex
#
#   TimeoutThread callback (scheduler.c -> profile.c):
#     Holds tMutex during timeoutNotificationCb -> NotifyTimeout which
#     briefly acquires plMutex.
#     Order: tMutex -> plMutex
#
#   Cycle: plMutex -> scMutex -> tMutex -> plMutex
#
# How reproduced:
#   1. Create a profile with a 5 s reporting interval; TimeoutThread fires
#      frequently, keeping tMutex contested.
#   2. Run 20 rounds of concurrent disable (unregister -> scMutex -> tMutex)
#      and enable (plMutex -> scMutex).
#   3. Any operation hanging > 10 s indicates the cycle may have formed.
#
# NOTE: This is timing-dependent.  For deterministic detection build with
# ThreadSanitizer — it reports the lock-order inversion on the first run:
#   CFLAGS="-fsanitize=thread -g" ./build_inside_container.sh
##########################################################################
scenario_c() {
    log_sep "SCENARIO C [HIGH] — 3-way lock cycle (plMutex->scMutex->tMutex->plMutex)"

    # 0-second delay: uploads complete immediately so TimeoutThread keeps
    # firing at 5 s intervals, maximising the chance of hitting the cycle.
    start_mock 0 || return

    log_info "Creating 'ReproCycle' profile with 5-second reporting interval..."
    local json
    json=$(printf \
        '{"profiles":[{"name":"ReproCycle","hash":"hC","value":{"Name":"ReproCycle","Protocol":"HTTP","EncodingType":"JSON","ReportingInterval":5,"RootName":"T2Report","TimeReference":"0001-01-01T00:00:00Z","Parameter":[{"type":"dataModel","name":"SWVer","reference":"Device.DeviceInfo.SoftwareVersion"}],"HTTP":{"URL":"http://127.0.0.1:%d/upload","Compression":"None","Method":"POST"},"JSONEncoding":{"ReportFormat":"NameValuePair","ReportTimestamp":"None"}}}]}' \
        "$MOCK_PORT")
    rbuscli set Device.X_RDKCENTRAL-COM_T2.Temp_ReportProfiles string "$json" \
        > "$LOG_DIR/C_create.log" 2>&1

    # Let at least one full TimeoutThread cycle fire.
    sleep 7

    log_info "Running 20 rounds of concurrent enable/disable..."
    local round=1
    local any_hung=false
    while [ "$round" -le 20 ]; do
        # disable triggers: unregisterProfileFromScheduler -> scMutex -> tMutex
        rbuscli set "Device.X_RDKCENTRAL-COM_T2.ReportProfiles.ReproCycle.Enable" \
            bool false > "$LOG_DIR/C_disable_${round}.log" 2>&1 &
        local pid_dis=$!

        # enable triggers: enableProfile -> plMutex -> scMutex
        rbuscli set "Device.X_RDKCENTRAL-COM_T2.ReportProfiles.ReproCycle.Enable" \
            bool true > "$LOG_DIR/C_enable_${round}.log" 2>&1 &
        local pid_en=$!

        local op_timeout=10
        local round_hung=false

        if ! wait_with_timeout "$pid_dis" "$op_timeout"; then
            log_fail "Scenario C round $round: disable hung >${op_timeout}s — lock cycle [HIGH]"
            round_hung=true
            any_hung=true
        fi
        if ! wait_with_timeout "$pid_en" "$op_timeout"; then
            log_fail "Scenario C round $round: enable hung >${op_timeout}s — lock cycle [HIGH]"
            round_hung=true
            any_hung=true
        fi

        if $round_hung; then
            log_info "  Ref: profile.c:enableProfile  scheduler.c:unregisterProfileFromScheduler"
            log_info "      threading-model.md Scenario 1 (3-way cycle)"
            break
        fi

        if [ $(( round % 5 )) -eq 0 ]; then
            log_info "  Round $round/20 complete — no hang"
        fi

        round=$(( round + 1 ))
        sleep 1
    done

    if ! $any_hung; then
        log_pass "Scenario C: 20 concurrent enable/disable rounds without deadlock"
        log_info "  Note: cycle is timing-dependent; TSan gives deterministic detection:"
        log_info "    CFLAGS=\"-fsanitize=thread -g\" ./build_inside_container.sh"
    fi

    stop_mock
    sleep 1
}

##########################################################################
# Main
##########################################################################
main() {
    local scenario="${1:-all}"

    printf '\n'
    printf '================================================================\n'
    printf 'Telemetry 2.0 — Thread Synchronization Issue Reproducer\n'
    printf 'Date:     %s\n' "$(date)"
    printf 'Scenario: %s\n' "$scenario"
    printf '================================================================\n\n'

    check_prereqs

    case "$scenario" in
        A|a) scenario_a ;;
        B|b) scenario_b ;;
        C|c) scenario_c ;;
        all) scenario_a; scenario_b; scenario_c ;;
        *)
            printf 'Usage: %s [A|B|C|all]\n\n' "$0"
            printf '  A  CRITICAL  XConf plMutex held for entire HTTP upload duration\n'
            printf '  B  HIGH      deleteAllProfiles signal-loss + pthread_join deadlock\n'
            printf '  C  HIGH      3-way lock cycle (concurrent enable/disable)\n'
            printf '  all          run all three scenarios\n\n'
            exit 1
            ;;
    esac

    printf '\n================================================================\n'
    printf 'Results:  PASS=%d  FAIL=%d\n' "$PASS" "$FAIL"
    printf 'Logs:     %s\n' "$LOG_DIR"
    printf '================================================================\n\n'

    if [ "$FAIL" -gt 0 ]; then
        printf 'For deterministic lock-order detection (recommended for Scenario C):\n'
        printf '  CFLAGS="-fsanitize=thread -g" ./build_inside_container.sh\n\n'
        exit 1
    fi
    exit 0
}

main "$@"
