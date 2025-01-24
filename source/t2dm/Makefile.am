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
AM_CFLAGS =

ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libT2_tr181.la
libT2_tr181_la_SOURCES = cosa_telemetry_dml.c cosa_telemetry_internal.c plugin_main_apis.c plugin_main.c
libT2_tr181_la_LDFLAGS = -shared -fPIC -lcjson -lpthread -lrt
if ENABLE_CCSP_SUPPORT
libT2_tr181_la_LDFLAGS += -lccsp_common
endif
libT2_tr181_la_CPPFLAGS = -DFEATURE_SUPPORT_RDKLOG -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/dbus-1.0 \
                          -I${PKG_CONFIG_SYSROOT_DIR}$(libdir)/dbus-1.0/include \
                          -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/ccsp \
                          -I${top_srcdir}/include \
                          -I${top_srcdir}/source/bulkdata \
                          -I${top_srcdir}/source/t2parser \
                          -I${top_srcdir}/source/utils \
                          -I${top_srcdir}/source/t2ssp \
                          -I${top_srcdir}/source/t2dm

libT2_tr181_la_LIBADD = ${top_builddir}/source/bulkdata/libbulkdata.la
libT2_tr181_la_DEPENDENCIES = ${top_builddir}/source/bulkdata/libbulkdata.la
