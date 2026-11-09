#!/bin/sh
##########################################################################
# If not stated otherwise in this file or this component's LICENSE
# file the following copyright and licenses apply:
#
# Copyright 2025 RDK Management
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
##########################################################################
#
# Build and test one module's working tree, and report its current L1 state.
#
# The working tree is built and measured with test/l1_gaps.sh. Gates (each prints
# PASS, FAIL or NOTE):
#   registration  module binaries listed in Makefile.am, run_ut.sh and l1_modules.txt
#   tests         build succeeds and all binaries pass
#   valgrind      definitely + indirectly lost bytes or errors are zero (if enabled)
# Coverage and cppcheck findings are reported as NOTE (informational, no baseline to
# compare against).
#
# Usage: test/l1_check.sh <module> [--no-valgrind] [--local]
#
# Env:   L1_OUT              output directory (default /tmp/l1); results in $L1_OUT/check/<module>
#        L1_IMAGE            container image (default: CI native-platform image)
#        L1_DOCKER_PLATFORM  e.g. linux/amd64
#        LCOV_EXTRA          extra lcov args
# Exit:  0 all gates pass, 2 usage, 3 missing tool, 5 build failed, 6 gate failed
#
# The docker path (no --local) reuses a persistent container/volume (l1-check-runner /
# l1-check-vol) across runs, synced incrementally instead of a full copy each time. To
# force a clean rebuild: docker rm -f l1-check-runner; docker volume rm l1-check-vol

set -eu

ROOT=$(cd "$(dirname "$0")/.." && pwd)
MAP="$ROOT/test/l1_modules.txt"
OUT="${L1_OUT:-/tmp/l1}"
case "$OUT" in /*) ;; *) OUT="$PWD/$OUT" ;; esac
IMAGE="${L1_IMAGE:-ghcr.io/rdkcentral/docker-device-mgt-service-test/native-platform:latest}"
. "$ROOT/test/l1_docker_cache.sh"

usage() {
    echo "Usage: $0 <module> [--no-valgrind] [--local]" >&2
    exit 2
}

list_modules() {
    grep -v -e '^#' -e '^$' "$MAP" | cut -d'|' -f1
}

run_in_docker() {
    command -v docker >/dev/null 2>&1 || { echo "docker not found; install Docker or run inside the container with --local" >&2; exit 3; }
    docker info >/dev/null 2>&1 || { echo "Docker daemon is not running" >&2; exit 3; }

    platform="${L1_DOCKER_PLATFORM:-}"
    if ! docker image inspect "$IMAGE" >/dev/null 2>&1; then
        if [ -n "$platform" ]; then
            docker pull --platform "$platform" "$IMAGE" || exit 3
        elif ! docker pull "$IMAGE"; then
            echo "Pull failed; retrying as linux/amd64" >&2
            platform=linux/amd64
            docker pull --platform "$platform" "$IMAGE" || exit 3
        fi
    fi
    if [ -z "$platform" ]; then
        platform="linux/$(docker image inspect "$IMAGE" --format '{{.Architecture}}')"
    fi

    trap 'exit 130' INT TERM
    l1_ensure_container "$IMAGE" "$platform" "$ROOT"
    l1_sync_tree
    container_src=$(l1_src_dir)

    set -- "$MODULE" --local
    [ "$VALGRIND" -eq 1 ] || set -- "$@" --no-valgrind
    rc=0
    docker exec -i -e L1_OUT=/tmp/l1 -e LCOV_EXTRA="${LCOV_EXTRA:-}" \
        "$L1_CONTAINER" /bin/bash -c "cd '$container_src' && sh test/l1_check.sh \"\$@\"" _ "$@" || rc=$?

    mkdir -p "$OUT"
    docker cp "$L1_CONTAINER:/tmp/l1/." "$OUT" >/dev/null 2>&1 || true
    echo "Artifacts copied to $OUT/check/$MODULE (paths above refer to the container's /tmp/l1)"
    exit "$rc"
}

MODULE=""
VALGRIND=1
LOCAL=0
while [ $# -gt 0 ]; do
    case "$1" in
        --no-valgrind) VALGRIND=0 ;;
        --local) LOCAL=1 ;;
        -*) usage ;;
        *) [ -z "$MODULE" ] || usage; MODULE="$1" ;;
    esac
    shift
done
[ -n "$MODULE" ] || usage

entry=$(grep -v -e '^#' -e '^$' "$MAP" | awk -F'|' -v m="$MODULE" '$1 == m { print; exit }')
if [ -z "$entry" ]; then
    echo "Unknown module '$MODULE'. Known modules:" >&2
    list_modules >&2
    exit 2
fi
IFS='|' read -r _ test_dir bins src_dirs <<EOF
$entry
EOF
bin_list=$(echo "$bins" | tr ',' ' ')
src_list=$(echo "$src_dirs" | tr ',' ' ')

if [ "$LOCAL" -eq 0 ] && [ ! -f /.dockerenv ]; then
    run_in_docker
fi

command -v lcov >/dev/null 2>&1 || { echo "lcov not found; use the container (omit --local on a host)" >&2; exit 3; }
if [ "$VALGRIND" -eq 1 ] && ! command -v valgrind >/dev/null 2>&1; then
    echo "valgrind not found; continuing without the valgrind gate" >&2
    VALGRIND=0
fi

cd "$ROOT"
WORK="$OUT/check/$MODULE"
mkdir -p "$WORK"
cur_out="$WORK/current"
report="$WORK/report.txt"
: >"$report"

# Joins backslash-continued lines so multi-line make variables parse as one line.
join_lines() {
    sed -e ':a' -e '/\\$/N' -e 's/\\\n//' -e 'ta' "$1"
}

lost_bytes() {
    v=$(sed -n "s/.*$1 lost: \([0-9,]*\) bytes.*/\1/p" "$2" 2>/dev/null | tail -n 1 | tr -d ,)
    echo "${v:-0}"
}

file_stats() {
    awk '
    /^SF:/ { f = $0; sub(/^SF:.*\/source\//, "source/", f) }
    /^LF:/ { lf = substr($0, 4) }  /^LH:/ { lh = substr($0, 4) }
    /^BRF:/ { brf = substr($0, 5) } /^BRH:/ { brh = substr($0, 5) }
    /^FNF:/ { fnf = substr($0, 5) } /^FNH:/ { fnh = substr($0, 5) }
    /^end_of_record/ {
        printf "%s\t%d\t%d\t%d\t%d\t%d\t%d\n", f, lh, lf, brh, brf, fnh, fnf
        lf = lh = brf = brh = fnf = fnh = 0
    }' "$1"
}

# measure <source tree> <output dir>: build, test, coverage, valgrind, cppcheck.
measure() {
    tree=$1
    mout=$2
    rm -rf "$mout"
    mkdir -p "$mout"

    mrc=0
    L1_OUT="$mout" LCOV_EXTRA="${LCOV_EXTRA:-}" sh "$tree/test/l1_gaps.sh" "$MODULE" --build --local --top 0 >"$mout/gaps.out" 2>&1 || mrc=$?

    : >"$mout/tests.tsv"
    : >"$mout/vg.tsv"
    for b in $bin_list; do
        n=$(sed -n 's/^\[==========\] \([0-9][0-9]*\) tests\{0,1\} from .* ran\..*/\1/p' "$mout/$MODULE.$b.log" 2>/dev/null | tail -n 1)
        printf '%s\t%s\n' "$b" "${n:-0}" >>"$mout/tests.tsv"
    done

    if [ "$mrc" -eq 0 ]; then
        file_stats "$mout/$MODULE.info" >"$mout/files.tsv"
        if [ "$VALGRIND" -eq 1 ]; then
            for b in $bin_list; do
                vlog="$mout/vg.$b.log"
                echo "  valgrind $b"
                (cd "$tree" && timeout 1800 valgrind --leak-check=full --child-silent-after-fork=yes \
                    --log-file="$vlog" "./$test_dir/$b" >/dev/null 2>&1) || true
                errs=$(sed -n 's/.*ERROR SUMMARY: \([0-9,]*\) errors.*/\1/p' "$vlog" 2>/dev/null | tail -n 1 | tr -d ,)
                printf '%s\t%s\t%s\t%s\n' "$b" "$(lost_bytes definitely "$vlog")" "$(lost_bytes indirectly "$vlog")" "${errs:--1}" >>"$mout/vg.tsv"
            done
        fi
    fi

    : >"$mout/cppcheck.txt"
    if command -v cppcheck >/dev/null 2>&1; then
        # shellcheck disable=SC2086
        (cd "$tree" && cppcheck -q --enable=warning,style,performance,portability --inline-suppr \
            --template='{file}:{id}' $src_list "$test_dir" >/dev/null 2>"$mout/cppcheck.txt") || true
    fi

    echo "$mrc" >"$mout/gaps.rc"
}

## Registration (static check of the working tree)
check_registration() {
    mk="$test_dir/Makefile.am"
    if [ ! -f "$mk" ]; then
        echo "FAIL  registration: $mk not found" >>"$report"
        return
    fi
    built=$(join_lines "$mk" | sed -n 's/^[[:space:]]*bin_PROGRAMS[[:space:]]*+\{0,1\}=//p' | tr -s ' \t' '\n' | grep -v '^$' | sort -u)
    mapped=$(echo "$bins" | tr ',' '\n' | sort -u)
    if [ "$built" = "$mapped" ]; then
        echo "PASS  registration: bin_PROGRAMS matches test/l1_modules.txt" >>"$report"
    else
        echo "FAIL  registration: bin_PROGRAMS in $mk [$(echo "$built" | tr '\n' ' ')] differs from test/l1_modules.txt [$(echo "$mapped" | tr '\n' ' ')]" >>"$report"
    fi
    for b in $built; do
        if grep -q "^[[:space:]]*\./${test_dir}/${b}[[:space:]]*\$" test/run_ut.sh; then
            echo "PASS  registration: ./$test_dir/$b is in test/run_ut.sh" >>"$report"
        else
            echo "FAIL  registration: ./$test_dir/$b is missing from the tests list in test/run_ut.sh" >>"$report"
        fi
    done
    subdir=${test_dir#source/test/}
    if join_lines source/test/Makefile.am | sed -n 's/^[[:space:]]*SUBDIRS[[:space:]]*+\{0,1\}=//p' | tr -s ' \t' '\n' | grep -qx "$subdir"; then
        echo "PASS  registration: $subdir is in SUBDIRS of source/test/Makefile.am" >>"$report"
    else
        echo "FAIL  registration: $subdir is missing from SUBDIRS in source/test/Makefile.am" >>"$report"
    fi
    if grep -q "$test_dir/Makefile" configure.ac; then
        echo "PASS  registration: $test_dir/Makefile is in configure.ac" >>"$report"
    else
        echo "FAIL  registration: $test_dir/Makefile is missing from the gtest AC_CONFIG_FILES in configure.ac" >>"$report"
    fi
}
check_registration

## Working tree
echo "Measuring working tree..."
measure "$ROOT" "$cur_out"

cur_rc=$(cat "$cur_out/gaps.rc")

case "$cur_rc" in
    0) echo "PASS  tests: build OK and all $MODULE binaries pass" >>"$report" ;;
    4) grep '^FAIL ' "$cur_out/gaps.out" | sed 's/^/FAIL  tests: /' >>"$report" ;;
    5) cat "$report"; echo "Build failed:"; tail -n 40 "$cur_out/gaps.out"; exit 5 ;;
    *) cat "$report"; echo "l1_gaps.sh failed (exit $cur_rc):"; tail -n 20 "$cur_out/gaps.out"; exit 3 ;;
esac

awk -F'\t' '{ printf "PASS  tests: %s %d tests\n", $1, $2 }' "$cur_out/tests.tsv" >>"$report"

if [ "$cur_rc" -eq 0 ]; then
    awk -F'\t' '
    function pct(h, f) { return f > 0 ? 100 * h / f : -1 }
    function fmt(p) { return p < 0 ? "n/a" : sprintf("%.1f%%", p) }
    {
        l = pct($2, $3); br = pct($4, $5); fn = pct($6, $7)
        printf "NOTE  coverage: %s lines %s, branches %s, functions %s\n", $1, fmt(l), fmt(br), fmt(fn)
    }' "$cur_out/files.tsv" >>"$report"

    if [ "$VALGRIND" -eq 1 ]; then
        awk -F'\t' '
        {
            if ($4 < 0) { printf "NOTE  valgrind: %s did not complete\n", $1; next }
            lost = $2 + $3
            if (lost > 0 || $4 > 0) printf "FAIL  valgrind: %s lost %d bytes, %d errors\n", $1, lost, $4
            else printf "PASS  valgrind: %s lost 0 bytes, 0 errors\n", $1
        }' "$cur_out/vg.tsv" >>"$report"
    else
        echo "NOTE  valgrind: skipped" >>"$report"
    fi
fi

if command -v cppcheck >/dev/null 2>&1; then
    sed 's/.*://' "$cur_out/cppcheck.txt" | sort | uniq -c | awk '{ print $2 "\t" $1 }' >"$cur_out/cppcheck.ids"
    awk -F'\t' '{ printf "NOTE  cppcheck: %s %d findings\n", $1, $2; t += $2 } END { printf "NOTE  cppcheck: %d findings total\n", t + 0 }' "$cur_out/cppcheck.ids" >>"$report"
else
    echo "NOTE  cppcheck: not installed; skipped" >>"$report"
fi

echo
echo "== L1 check: $MODULE (current working tree) =="
cat "$report"
echo
if [ "$cur_rc" -eq 0 ]; then
    sed -n '/== Coverage summary/,/Total:/p' "$cur_out/gaps.out"
    echo
fi
echo "Details: $WORK (current/, report.txt)"
if grep -q '^FAIL' "$report"; then
    echo "RESULT: FAIL"
    exit 6
fi
echo "RESULT: PASS"
