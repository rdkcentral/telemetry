##########################################################################
# If not stated otherwise in this file or this component's LICENSE
# file the following copyright and licenses apply:
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
##########################################################################
AM_CFLAGS = -fno-permissive

lib_LTLIBRARIES = libt2thunder_privacycontrol.la

libt2thunder_privacycontrol_la_SOURCES = rdkservices_privacyutils.c
libt2thunder_privacycontrol_la_LDFLAGS = -shared -fPIC -lcurl -lcjson
libt2thunder_privacycontrol_la_LIBADD = -lrbus ${top_builddir}/source/utils/libt2utils.la
libt2thunder_privacycontrol_la_CFLAGS = $(PRIVACYCONTROL_FLAG)

if IS_LIBSYSWRAPPER_ENABLED
libt2thunder_privacycontrol_la_CFLAGS += $(LIBSYSWRAPPER_FLAG)
libt2thunder_privacycontrol_la_LDFLAGS += -lsecure_wrapper
endif

libt2thunder_privacycontrol_la_CPPFLAGS = -g -fPIC -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/ \
                                        -I${top_srcdir}/source/utils \
                                        -I${top_srcdir}/include

libt2thunder_privacycontrol_la_DEPENDENCIES = ${top_builddir}/source/utils/libt2utils.la
