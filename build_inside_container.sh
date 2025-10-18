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


export INSTALL_DIR='/usr/local'
export top_srcdir=`pwd`
export top_builddir=`pwd`

autoreconf --install

# FLags to print compiler warnings
DEBUG_CFLAGS="-Wall -Werror -Wextra"

export CFLAGS=" ${DEBUG_CFLAGS} -I${INSTALL_DIR}/include/rtmessage -I${INSTALL_DIR}/include/msgpack -I${INSTALL_DIR}/include/rbus -I${INSTALL_DIR}/include -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/local/include -DFEATURE_SUPPORT_WEBCONFIG -DRDK_LOGGER -DPERSIST_LOG_MON_REF -DDCMAGENT" 

export LDFLAGS="-L/usr/lib/x86_64-linux-gnu -lglib-2.0"

./configure --prefix=${INSTALL_DIR} --without-rdklogger CFLAGS="-I/usr/local/include" && make && make install
