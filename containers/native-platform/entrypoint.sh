#!/usr/bin/env bash

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

export RBUS_ROOT=/ 
export RBUS_INSTALL_DIR=/usr/local
export PATH=${RBUS_INSTALL_DIR}/bin:${PATH}
export LD_LIBRARY_PATH=${RBUS_INSTALL_DIR}/lib:${LD_LIBRARY_PATH}

# Build and install RFC parameter provider

rt_pid=`pidof rtrouted`
if [ ! -z "$rt_routed" ]; then
    kill -9 `pidof rtrouted`
fi

rm -fr /tmp/rtroute*
rtrouted -l DEBUG 

/usr/local/bin/rfc_provider

/bin/bash