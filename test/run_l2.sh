#!/bin/sh

####################################################################################
# If not stated otherwise in this file or this component's Licenses.txt file the
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

export top_srcdir=`pwd`
RESULT_DIR="/tmp/l2_test_report"
mkdir -p "$RESULT_DIR"

# ThreadSanitizer support (enabled by default, adds ~5-15x runtime overhead)
# Pass --disable-tsan to skip TSan instrumentation and reduce test execution time
ENABLE_TSAN=true
if [ "x$1" = "x--disable-tsan" ]; then
    echo "ThreadSanitizer disabled"
    ENABLE_TSAN=false
else
    echo "ThreadSanitizer enabled for race detection"
fi

if ! grep -q "LOG_PATH=/opt/logs/" /etc/include.properties; then
    echo "LOG_PATH=/opt/logs/" >> /etc/include.properties
fi

if ! grep -q "PERSISTENT_PATH=/opt/" /etc/include.properties; then
    echo "PERSISTENT_PATH=/opt/" >> /etc/include.properties
fi

gcc test/functional-tests/tests/app.c -o test/functional-tests/tests/t2_app -ltelemetry_msgsender -lt2utils

final_result=0
# removing --exitfirst flag as it is causing the test to exit after first failure
pytest -v --json-report --json-report-summary --json-report-file $RESULT_DIR/runs_as_daemon.json test/functional-tests/tests/test_runs_as_daemon.py || final_result=1
pytest -v --json-report --json-report-summary --json-report-file $RESULT_DIR/bootup_sequence.json test/functional-tests/tests/test_bootup_sequence.py || final_result=1
pytest -v --json-report --json-report-summary --json-report-file $RESULT_DIR/xconf_communications.json test/functional-tests/tests/test_xconf_communications.py || final_result=1
pytest -v --json-report --json-report-summary --json-report-file $RESULT_DIR/msg_packet.json test/functional-tests/tests/test_multiprofile_msgpacket.py || final_result=1

if [ $final_result -ne 0 ]; then
    echo "Some tests failed. Please check the JSON reports in $RESULT_DIR for details."
else
    echo "All tests passed successfully."
fi

# ThreadSanitizer: rebuild with TSan and re-run the tests to collect race reports.
# Tests above ran against the normal (fast) binary for correctness (final_result).
# TSan adds ~5-15x slowdown which may break timing-sensitive tests, so TSan test
# pass/fail is ignored — we only care about the race detection output.
if [ "$ENABLE_TSAN" = true ]; then
    echo ""
    echo "=== Rebuilding with ThreadSanitizer for race detection ==="
    autoreconf --install
    ./configure --enable-tsan
    make clean
    make
    make install
    echo "TSan-instrumented build complete."

    export TSAN_OPTIONS="halt_on_error=0 second_deadlock_stack=1 history_size=4 log_path=$RESULT_DIR/tsan.log"

    # Re-run a subset of tests against the TSan-instrumented binary to exercise
    # threaded code paths (profile processing, signal handling, report generation).
    # Pass/fail is ignored; only TSan log output matters.
    #
    # We skip test_xconf_communications (~10min without TSan, up to 2.5h with TSan)
    # as it primarily exercises HTTP retry logic with less thread coverage.
    # To run TSan against ALL suites, uncomment the xconf line below:
    #   timeout $TSAN_TEST_TIMEOUT pytest -v test/functional-tests/tests/test_xconf_communications.py || true
    #
    # Timeout guards against potential deadlocks from signal-unsafe calls
    # (sig_handler -> T2Log -> malloc can deadlock under TSan interceptors).
    # Longest remaining suite is multiprofile_msgpacket (~6min without TSan,
    # up to ~90min with TSan overhead). 1800s (30min) allows ample headroom.
    # If enabling xconf_communications (~10min normal), increase to 3600.
    TSAN_TEST_TIMEOUT=1800
    echo ""
    echo "=== Running tests with TSan-instrumented binary (pass/fail ignored, ${TSAN_TEST_TIMEOUT}s timeout per suite) ==="
    timeout $TSAN_TEST_TIMEOUT pytest -v test/functional-tests/tests/test_runs_as_daemon.py || true
    timeout $TSAN_TEST_TIMEOUT pytest -v test/functional-tests/tests/test_bootup_sequence.py || true
    timeout $TSAN_TEST_TIMEOUT pytest -v test/functional-tests/tests/test_multiprofile_msgpacket.py || true
    # Allow TSan log files to flush
    sleep 2

    # Report TSan findings (only telemetry source files, not external dependencies)
    tsan_files=$(find "$RESULT_DIR" -name "tsan.log*" -size +0c 2>/dev/null)
    if [ -n "$tsan_files" ]; then
        echo ""
        echo "=== ThreadSanitizer Report (telemetry source only) ==="
        total_warnings=0
        for f in $tsan_files; do
            # Filter to only show warnings involving telemetry source files
            telemetry_warnings=$(grep -A 20 "WARNING: ThreadSanitizer:" "$f" 2>/dev/null | grep -c "L2_CONTAINER_SHARED_VOLUME/source/" 2>/dev/null || echo 0)
            if [ "$telemetry_warnings" -gt 0 ]; then
                echo ""
                echo "--- $f ---"
                # Print only SUMMARY lines that reference telemetry source
                grep "SUMMARY: ThreadSanitizer:" "$f" 2>/dev/null | grep "L2_CONTAINER_SHARED_VOLUME/source/" | sort -u
                count=$(grep "SUMMARY: ThreadSanitizer:" "$f" 2>/dev/null | grep -c "L2_CONTAINER_SHARED_VOLUME/source/" || echo 0)
                total_warnings=$((total_warnings + count))
            fi
        done
        echo ""
        echo "=== TSan Summary: $total_warnings telemetry-specific warning(s) ==="
        echo "Full logs (including external deps): $RESULT_DIR/tsan.log*"
    else
        echo ""
        echo "=== ThreadSanitizer: No data races detected ==="
    fi
fi

exit $final_result
