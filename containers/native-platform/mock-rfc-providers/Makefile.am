
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

bin_PROGRAMS = rfc_provider
RBUS_INSTALL_DIR = /usr/local
LOCAL_DIR = /usr/local

rfc_provider_SOURCES = rfc_provider.c
rfc_provider_CFLAGS = -I$(RBUS_INSTALL_DIR)/include/rtmessage -I$(RBUS_INSTALL_DIR)/include/msgpack -I$(RBUS_INSTALL_DIR)/include/rbus -I$(RBUS_INSTALL_DIR)/include
rfc_provider_LDFLAGS = -L${RBUS_INSTALL_DIR}/lib  -L$(LOCAL_DIR)/lib -lpthread -lm -lrt -lrtMessage -lrbus -lmsgpackc


rfc_provider_LDADD = -L${RBUS_INSTALL_DIR}/lib -lrtMessage -lrbus -lmsgpackc -lpthread -lm -lrt