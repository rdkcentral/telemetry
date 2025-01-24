#!/bin/sh

####################################################################################
# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2019 RDK Management
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

. /etc/device.properties

T2_MSG_CLIENT=/usr/bin/telemetry2_0_client

t2CountNotify() {

    if [ -f "$T2_MSG_CLIENT" ]; then
        marker=$1
        $T2_MSG_CLIENT  "$marker" "1" &
    fi
}

t2ValNotify() {

    if [ -f "$T2_MSG_CLIENT" ]; then
        marker=$1
        shift
        $T2_MSG_CLIENT "$marker" "$*" &
    fi
}
