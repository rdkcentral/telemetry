#!/bin/sh
##########################################################################
# If not stated otherwise in this file or this component's LICENSE
# file the following copyright and licenses apply:
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
##########################################################################
#
# check-arch.sh — run by devcontainer.json's initializeCommand (host-side).
#
# On Apple Silicon (arm64) the GHCR images are amd64-only and running them
# under Rosetta is unreliable.  This script checks whether native arm64
# images have been built locally and aborts with actionable instructions
# when they are absent.
#
# On x86_64 hosts the GHCR images are used directly; no action needed.

set -e

ARCH=$(uname -m)

if [ "$ARCH" != "arm64" ] && [ "$ARCH" != "aarch64" ]; then
    echo "[devcontainer] Host architecture: $ARCH — using pre-built GHCR images."
    exit 0
fi

echo ""
echo "╔══════════════════════════════════════════════════════════════════╗"
echo "║          Apple Silicon detected ($(uname -m))                     "
echo "╠══════════════════════════════════════════════════════════════════╣"
echo "║  The GHCR images are published as linux/amd64 only.             ║"
echo "║  Running them under Rosetta is unreliable for this workload.    ║"
echo "║  Native arm64 images must be built from source before VS Code   ║"
echo "║  can open this devcontainer.                                     ║"
echo "╠══════════════════════════════════════════════════════════════════╣"

# Check whether local arm64 images are already present.
NP_IMG="ghcr.io/rdkcentral/docker-device-mgt-service-test/native-platform:latest"
MX_IMG="ghcr.io/rdkcentral/docker-device-mgt-service-test/mockxconf:latest"

check_image_arch() {
    img="$1"
    # Inspect the first layer's architecture; empty means image not present.
    arch=$(docker inspect --format '{{.Architecture}}' "$img" 2>/dev/null || true)
    echo "$arch"
}

np_arch=$(check_image_arch "$NP_IMG")
mx_arch=$(check_image_arch "$MX_IMG")

if [ "$np_arch" = "arm64" ] && [ "$mx_arch" = "arm64" ]; then
    echo "║  ✓ Local arm64 images found — proceeding.                       ║"
    echo "╚══════════════════════════════════════════════════════════════════╝"
    echo ""
    exit 0
fi

# Report which images are missing or wrong arch.
if [ "$np_arch" != "arm64" ]; then
    if [ -z "$np_arch" ]; then
        echo "║  ✗ native-platform: NOT FOUND locally                           ║"
    else
        echo "║  ✗ native-platform: found but arch=$np_arch (need arm64)        ║"
    fi
fi
if [ "$mx_arch" != "arm64" ]; then
    if [ -z "$mx_arch" ]; then
        echo "║  ✗ mockxconf: NOT FOUND locally                                 ║"
    else
        echo "║  ✗ mockxconf: found but arch=$mx_arch (need arm64)              ║"
    fi
fi

echo "╠══════════════════════════════════════════════════════════════════╣"
echo "║  Build native arm64 images with these commands, then re-open    ║"
echo "║  the devcontainer:                                               ║"
echo "║                                                                  ║"
echo "║    git clone https://github.com/rdkcentral/\\                    ║"
echo "║              docker-device-mgt-service-test                     ║"
echo "║    cd docker-device-mgt-service-test                            ║"
echo "║    docker build -t \\                                            ║"
echo "║      ghcr.io/rdkcentral/docker-device-mgt-service-test/\\        ║"
echo "║      mockxconf:latest -f Dockerfile.mockxconf .                 ║"
echo "║    docker build -t \\                                            ║"
echo "║      ghcr.io/rdkcentral/docker-device-mgt-service-test/\\        ║"
echo "║      native-platform:latest -f Dockerfile.native-platform .     ║"
echo "║                                                                  ║"
echo "║  After building, re-run: Dev Containers: Reopen in Container    ║"
echo "╚══════════════════════════════════════════════════════════════════╝"
echo ""
exit 1
