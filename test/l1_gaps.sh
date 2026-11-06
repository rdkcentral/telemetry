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
# Rank uncovered L1 coverage gaps for one module (see test/l1_modules.txt).
#
# On a host (for example macOS) the script starts the CI test container, copies the
# workspace into it (host tree is mounted read-only and never modified), builds with
# coverage, runs the module's binaries, and prints the gaps. Inside the container,
# or with --local, it runs directly.
#
# Usage: test/l1_gaps.sh <module> [--build] [--top N] [--local]
#        test/l1_gaps.sh --list
#
# Env:   L1_OUT              output directory (default /tmp/l1)
#        L1_IMAGE            container image (default: CI native-platform image)
#        L1_DOCKER_PLATFORM  e.g. linux/amd64 (auto-retried on Apple Silicon if pull fails)
#        LCOV_EXTRA          extra lcov args, e.g. "--ignore-errors mismatch" for lcov 2.x
# Exit:  0 ok, 2 usage, 3 missing tool/binary/docker or lcov failure, 4 test binary failed, 5 build failed

set -eu

ROOT=$(cd "$(dirname "$0")/.." && pwd)
MAP="$ROOT/test/l1_modules.txt"
OUT="${L1_OUT:-/tmp/l1}"

usage() {
    echo "Usage: $0 <module> [--build] [--top N] [--local] | --list" >&2
    exit 2
}

list_modules() {
    grep -v -e '^#' -e '^$' "$MAP" | cut -d'|' -f1
}

IMAGE="${L1_IMAGE:-ghcr.io/rdkcentral/docker-device-mgt-service-test/native-platform:latest}"
CONTAINER_DIR=/mnt/L1_CONTAINER_SHARED_VOLUME

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

    # Match the image's own platform so amd64-only images run (emulated) on Apple Silicon.
    if [ -z "$platform" ]; then
        platform="linux/$(docker image inspect "$IMAGE" --format '{{.Architecture}}')"
    fi

    cname="l1-gaps-$$"
    trap 'docker rm -f "$cname" >/dev/null 2>&1 || true' EXIT
    trap 'exit 130' INT TERM
    # Same start pattern as .github/workflows/L1-tests.yml, but the host tree is read-only.
    docker run -d --name "$cname" --platform "$platform" -v "$ROOT:/mnt/host_src:ro" "$IMAGE" >/dev/null

    docker exec "$cname" /bin/bash -c "mkdir -p $CONTAINER_DIR && cd /mnt/host_src && tar --exclude=./.git --exclude=autom4te.cache --exclude='*.o' --exclude='*.gcno' --exclude='*.gcda' -cf - . | tar -xf - -C $CONTAINER_DIR"

    rc=0
    docker exec -i -e L1_OUT=/tmp/l1 "$cname" /bin/bash -c "cd $CONTAINER_DIR && sh test/l1_gaps.sh \"\$@\"" _ \
        "$MODULE" --top "$TOP" --build --local || rc=$?

    mkdir -p "$OUT"
    docker cp "$cname:/tmp/l1/." "$OUT" >/dev/null 2>&1 || true
    echo "Artifacts copied to $OUT (paths above refer to the container's /tmp/l1)"
    exit "$rc"
}

MODULE=""
BUILD=0
LOCAL=0
TOP=15
while [ $# -gt 0 ]; do
    case "$1" in
        --list) list_modules; exit 0 ;;
        --build) BUILD=1 ;;
        --local) LOCAL=1 ;;
        --top) shift; [ $# -gt 0 ] || usage; TOP="$1" ;;
        -*) usage ;;
        *) [ -z "$MODULE" ] || usage; MODULE="$1" ;;
    esac
    shift
done
[ -n "$MODULE" ] || usage
case "$TOP" in ''|*[!0-9]*) usage ;; esac


## Locate the module entry in the map file and extract relevant fields
entry=$(grep -v -e '^#' -e '^$' "$MAP" | awk -F'|' -v m="$MODULE" '$1 == m { print; exit }')
if [ -z "$entry" ]; then
    echo "Unknown module '$MODULE'. Known modules:" >&2
    list_modules >&2
    exit 2
fi
IFS='|' read -r _ test_dir bins src_dirs <<EOF
$entry
EOF

if [ "$LOCAL" -eq 0 ] && [ ! -f /.dockerenv ]; then
    run_in_docker
fi

command -v lcov >/dev/null 2>&1 || { echo "lcov not found; use the container (omit --local on a host)" >&2; exit 3; }

cd "$ROOT"
mkdir -p "$OUT"

if [ "$BUILD" -eq 1 ]; then
    CFLAGS="-g -O0 -fprofile-arcs -ftest-coverage"
    CXXFLAGS="$CFLAGS"
    LDFLAGS="-lgcov --coverage"
    export CFLAGS CXXFLAGS LDFLAGS top_srcdir="$ROOT"
    build_log="$OUT/build.log"
    if ! { autoreconf --install && ./configure --enable-gtestapp && make -C "$test_dir"; } >"$build_log" 2>&1; then
        echo "Build failed; last lines of $build_log:" >&2
        tail -n 40 "$build_log" >&2
        exit 5
    fi
    echo "Build OK (log: $build_log)"
fi

LCOV_OPTS="--rc lcov_branch_coverage=1 ${LCOV_EXTRA:-}"
lcov_log="$OUT/lcov.log"
: >"$lcov_log"

run_lcov() {
    if ! "$@" >>"$lcov_log" 2>&1; then
        echo "lcov failed; last lines of $lcov_log:" >&2
        tail -n 20 "$lcov_log" >&2
        exit 3
    fi
}

# shellcheck disable=SC2086
run_lcov lcov --zerocounters --directory . $LCOV_OPTS

mkdir -p /tmp/Gtest_Report
failed=0
for b in $(echo "$bins" | tr ',' ' '); do
    bin="$test_dir/$b"
    if [ ! -x "$bin" ]; then
        echo "Missing test binary $bin (re-run with --build)" >&2
        exit 3
    fi
    log="$OUT/$MODULE.$b.log"
    if "$bin" >"$log" 2>&1; then
        echo "PASS $b"
    else
        echo "FAIL $b (see $log)" >&2
        failed=1
    fi
done
[ "$failed" -eq 0 ] || { echo "Fix failing tests before gap analysis." >&2; exit 4; }

all_info="$OUT/$MODULE.all.info"
info="$OUT/$MODULE.info"
# shellcheck disable=SC2086
run_lcov lcov --capture --directory . $LCOV_OPTS --output-file "$all_info"

set --
for d in $(echo "$src_dirs" | tr ',' ' '); do
    set -- "$@" "*/$d/*"
done
# shellcheck disable=SC2086
run_lcov lcov --extract "$all_info" "$@" $LCOV_OPTS --output-file "$info"

## Analyze coverage gaps and generate a TSV report
# Per function: count uncovered lines (DA hit=0) and untaken branches (BRDA taken 0 or -).
# A function spans from its FN start to the next function start (or FN end on lcov 2.x).
# Rows: uncovered_lines, uncovered_branches, UNCALLED|PARTIAL, file:line, function.
gaps="$OUT/$MODULE.gaps.tsv"
: >"$gaps"

awk -v gaps="$gaps" '
function relpath(p) { sub(/^.*\/source\//, "source/", p); return p }
function flush(   i, j, k, t, e, l, ul, ub, kind, n) {
    n = nfn
    for (i = 1; i <= n; i++) idx[i] = i
    for (i = 2; i <= n; i++) {
        t = idx[i]; j = i - 1
        while (j >= 1 && fstart[idx[j]] > fstart[t]) { idx[j + 1] = idx[j]; j-- }
        idx[j + 1] = t
    }
    for (i = 1; i <= n; i++) {
        k = idx[i]
        if (fend[k] > 0) e = fend[k]
        else if (i < n) e = fstart[idx[i + 1]] - 1
        else e = maxline
        ul = 0; ub = 0
        for (l = fstart[k]; l <= e; l++) {
            if ((l in da) && da[l] == 0) ul++
            if (l in brn) ub += brn[l]
        }
        kind = (fcount[fname[k]] + 0 == 0) ? "UNCALLED" : "PARTIAL"
        if (ul > 0 || ub > 0 || kind == "UNCALLED")
            printf "%d\t%d\t%s\t%s:%d\t%s\n", ul, ub, kind, relpath(file), fstart[k], fname[k] >> gaps
    }
    for (k in da) delete da[k]
    for (k in brn) delete brn[k]
    for (k in fcount) delete fcount[k]
    nfn = 0; maxline = 0
}
BEGIN { nfn = 0; maxline = 0 }
/^SF:/ { file = substr($0, 4) }
/^FN:/ {
    nparts = split(substr($0, 4), a, ",")
    nfn++
    fstart[nfn] = a[1] + 0
    if (nparts == 3) { fend[nfn] = a[2] + 0; fname[nfn] = a[3] }
    else { fend[nfn] = 0; fname[nfn] = a[2] }
}
/^FNDA:/ { split(substr($0, 6), a, ","); fcount[a[2]] = a[1] + 0 }
/^DA:/ {
    split(substr($0, 4), a, ",")
    da[a[1] + 0] = a[2] + 0
    if (a[1] + 0 > maxline) maxline = a[1] + 0
}
/^BRDA:/ {
    split(substr($0, 6), a, ",")
    if (a[4] == "-" || a[4] + 0 == 0) brn[a[1] + 0]++
}
/^end_of_record/ { flush() }
' "$info"

echo
echo "== Coverage summary: $MODULE (lcov --list, module source files only) =="
# shellcheck disable=SC2086
lcov --list "$info" $LCOV_OPTS 2>>"$lcov_log" | grep -v '^Reading tracefile'

echo
echo "== Top $TOP gaps, ranked by uncovered lines (full list: $gaps) =="
if [ -s "$gaps" ]; then
    sort -t "$(printf '\t')" -k1,1nr -k2,2nr "$gaps" | head -n "$TOP" |
        awk -F'\t' '{ printf "%-9s %-52s %-34s uncovered_lines=%s uncovered_branches=%s\n", $3, $4, $5, $1, $2 }'
else
    echo "No gaps found."
fi
echo
echo "Coverage data: $info"
