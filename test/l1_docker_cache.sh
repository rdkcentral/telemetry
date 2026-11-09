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
# Shared helpers for test/l1_check.sh and test/l1_gaps.sh: a persistent Docker volume
# and container, reused across invocations, synced incrementally with rsync instead of
# a fresh full tar copy every run. Build artifacts (*.o/*.gcno/*.gcda) are excluded from
# the sync and therefore survive between runs, giving incremental `make` for free.
# No baseline tree is cached here; both scripts only ever measure the synced working tree.
#
# To force a clean rebuild: docker rm -f l1-check-runner; docker volume rm l1-check-vol
#
# Sourced, not executed: . "$(dirname "$0")/l1_docker_cache.sh"

L1_VOLUME="${L1_VOLUME:-l1-check-vol}"
L1_CONTAINER="${L1_CONTAINER:-l1-check-runner}"
L1_VOLUME_MOUNT=/mnt/l1_volume

l1_ensure_volume() {
    docker volume inspect "$L1_VOLUME" >/dev/null 2>&1 || docker volume create "$L1_VOLUME" >/dev/null
}

# l1_ensure_container <image> <platform> <root>: create/start the persistent container
# bind-mounting <root> read-only plus the shared volume; recreated whenever the image,
# platform or host root path differ from the ones it was last created with.
l1_ensure_container() {
    img=$1
    plat=$2
    root=$3
    l1_ensure_volume
    want="$img|$plat|$root"
    have=""
    if docker ps -a --filter "name=^${L1_CONTAINER}\$" --format '{{.Names}}' | grep -qx "$L1_CONTAINER"; then
        have=$(docker exec "$L1_CONTAINER" cat "$L1_VOLUME_MOUNT/.meta" 2>/dev/null || true)
        if [ "$have" != "$want" ]; then
            docker rm -f "$L1_CONTAINER" >/dev/null 2>&1 || true
            have=""
        fi
    fi
    if [ -z "$have" ]; then
        docker run -d --name "$L1_CONTAINER" --platform "$plat" \
            -v "$root:/mnt/host_src:ro" -v "$L1_VOLUME:$L1_VOLUME_MOUNT" "$img" tail -f /dev/null >/dev/null
        docker exec "$L1_CONTAINER" /bin/sh -c \
            "mkdir -p '$L1_VOLUME_MOUNT/src' && printf '%s' '$want' >'$L1_VOLUME_MOUNT/.meta'"
    elif [ "$(docker inspect -f '{{.State.Running}}' "$L1_CONTAINER" 2>/dev/null)" != "true" ]; then
        docker start "$L1_CONTAINER" >/dev/null
    fi
}

# l1_sync_tree: incrementally mirror the read-only host mount into the volume's src dir.
# rsync --exclude also protects excluded paths from --delete, so prior build artifacts
# are left in place; falls back to a full tar copy if rsync isn't in the image.
l1_sync_tree() {
    if docker exec "$L1_CONTAINER" sh -c 'command -v rsync >/dev/null 2>&1'; then
        docker exec "$L1_CONTAINER" rsync -a --delete \
            --exclude=.git --exclude=autom4te.cache --exclude='*.o' --exclude='*.gcno' --exclude='*.gcda' \
            /mnt/host_src/ "$L1_VOLUME_MOUNT/src/"
    else
        docker exec "$L1_CONTAINER" /bin/sh -c \
            "cd /mnt/host_src && tar --exclude=./.git --exclude=autom4te.cache --exclude='*.o' --exclude='*.gcno' --exclude='*.gcda' -cf - . | tar -xf - -C '$L1_VOLUME_MOUNT/src'"
    fi
}

l1_src_dir() {
    echo "$L1_VOLUME_MOUNT/src"
}
