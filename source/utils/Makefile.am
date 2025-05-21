##########################################################################
# If not stated otherwise in this file or this component's LICENSE
# file the following copyright and licenses apply:
#
# Copyright 2015 RDK Management
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
lib_LTLIBRARIES = libt2utils.la

libt2utils_la_SOURCES = vector.c t2collection.c t2log_wrapper.c persistence.c t2common.c
libt2utils_la_LDFLAGS = -shared -fPIC
libt2utils_la_CFLAGS = -DUNUSED

if IS_LIBRDKCONFIG_ENABLED
libt2utils_la_CFLAGS += $(LIBRDKCONFIG_FLAG)
libt2utils_la_LDFLAGS += -lrdkconfig
endif
if IS_LIBSYSWRAPPER_ENABLED
libt2utils_la_CFLAGS += $(LIBSYSWRAPPER_FLAG)
libt2utils_la_LDFLAGS += -lsecure_wrapper
endif
if IS_MTLS_ENABLED
libt2utils_la_CFLAGS = $(MTLS_FLAG)
libt2utils_la_SOURCES += t2MtlsUtils.c
endif

if IS_MTLS_ENABLED
libt2utils_la_CFLAGS = $(MTLS_FLAG)
libt2utils_la_SOURCES += t2MtlsUtils.c
endif

if RDKLOGGER_ENABLE
libt2utils_la_CFLAGS += -DRDK_LOGGER 
libt2utils_la_LDFLAGS += -lrdkloggers
endif


libt2utils_la_CPPFLAGS = -fPIC -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/dbus-1.0 \
                                -I${PKG_CONFIG_SYSROOT_DIR}$(libdir)/dbus-1.0/include \
                                -I${PKG_CONFIG_SYSROOT_DIR}$(includedir) \
                                -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/ccsp \
                                -I${top_srcdir}/include \
                                -I${top_srcdir}/source/utils \
                                -I${top_srcdir}/source/dcautil
