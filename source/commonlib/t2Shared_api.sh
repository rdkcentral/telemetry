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

# This API helps to send events to T2 without exposing the actual variable names used.
# It is particularly useful when the final markers do not follow a common syntax.
#
# Before calling this function, the required variables must be initialized with the common names.
# These common names can be created by sourcing from a file (without exposing in the code) or by defining the variables directly.
# For example:
#   T2_CRASH_telemetry2_0="SYS_ERR_T2_CRASH"
#   T2_CRASH_rtrouted="SYST_INFO_RBUS_CRASH"
#
# After defining the common names, the function can be called as follows:
#   appname="telemetry2_0"
#   t2FilterMapCountNotify T2_CRASH_${appname}
#
# In this example, the function call will internally call t2CountNotify with the argument "SYS_ERR_T2_CRASH".


t2FilterMapCountNotify(){

    common_name="$1"
    if [ -n "${!common_name-}" ]; then
        t2CountNotify "${!common_name}"
    fi
}
