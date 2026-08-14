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
#
# Helper to manage the local development containers defined in
# containers/docker-compose.yml

set -e

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
COMPOSE_FILE="$SCRIPT_DIR/containers/docker-compose.yml"
SERVICE="native-platform"
WORKDIR="/mnt/L1_CONTAINER_SHARED_VOLUME"

usage() {
    cat <<EOF
Usage: $0 <command> [args]

Commands:
  up            Start the containers (pulls images on first run)
  shell         Open an interactive shell inside $SERVICE
  run <cmd>     Run a command inside $SERVICE
  ut            Run the L1 unit tests
  cov           Run the L1 unit tests with coverage and print the summary
  l2            Build and run the L2 integration tests
  logs          Follow container logs
  down          Stop and remove the containers
EOF
}

compose() {
    docker compose -f "$COMPOSE_FILE" "$@"
}

exec_in_container() {
    compose exec -w "$WORKDIR" "$SERVICE" /bin/bash -c "$1"
}

ensure_up() {
    if [ -z "$(compose ps -q "$SERVICE")" ]; then
        compose up -d
    fi
}

COMMAND="$1"
[ $# -gt 0 ] && shift

case "$COMMAND" in
    up)
        compose up -d
        compose ps
        ;;
    shell)
        ensure_up
        compose exec -w "$WORKDIR" "$SERVICE" /bin/bash
        ;;
    run)
        if [ $# -eq 0 ]; then
            echo "ERROR: 'run' requires a command" >&2
            exit 1
        fi
        ensure_up
        exec_in_container "$*"
        ;;
    ut)
        ensure_up
        exec_in_container "sh test/run_ut.sh"
        ;;
    cov)
        ensure_up
        exec_in_container "sh test/run_ut.sh --enable-cov && lcov --list coverage.info"
        ;;
    l2)
        ensure_up
        exec_in_container "sh build_inside_container.sh && sh test/run_l2.sh"
        ;;
    logs)
        compose logs -f
        ;;
    down)
        compose down
        ;;
    ""|-h|--help|help)
        usage
        ;;
    *)
        echo "ERROR: Unknown command '$COMMAND'" >&2
        usage
        exit 1
        ;;
esac
