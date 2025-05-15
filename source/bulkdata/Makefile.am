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

lib_LTLIBRARIES = libbulkdata.la

libbulkdata_la_SOURCES = reportprofiles.c profile.c profilexconf.c t2eventreceiver.c t2markers.c datamodel.c
libbulkdata_la_LDFLAGS = -shared -fPIC -lcjson -lmsgpackc
libbulkdata_la_LIBADD = ${top_builddir}/source/utils/libt2utils.la ${top_builddir}/source/dcautil/libdcautil.la ${top_builddir}/source/protocol/http/libhttp.la ${top_builddir}/source/protocol/rbusMethod/librbusmethod.la ${top_builddir}/source/ccspinterface/libccspinterface.la ${top_builddir}/source/reportgen/libreportgen.la ${top_builddir}/source/scheduler/libscheduler.la ${top_builddir}/source/xconf-client/libxconfclient.la
libbulkdata_la_CPPFLAGS = -fPIC -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/dbus-1.0 \
                                -I${PKG_CONFIG_SYSROOT_DIR}$(libdir)/dbus-1.0/include \
				-I${PKG_CONFIG_SYSROOT_DIR}$(includedir) \
				-I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/rbus \
                                -I${top_srcdir}/source/utils \
                                -I${top_srcdir}/source/dcautil \
                                -I${top_srcdir}/source/reportgen \
                                -I${top_srcdir}/source/protocol/http \
                                -I${top_srcdir}/source/protocol/rbusMethod \
                                -I${top_srcdir}/source/scheduler \
                                -I${top_srcdir}/include \
                                -I${top_srcdir}/source/ccspinterface \
                                -I${top_srcdir}/source/t2parser \
                                -I${top_srcdir}/source/xconf-client

if WEBCONFIG_FRAMEWORK_ENABLE
libbulkdata_la_LDFLAGS += -lwebconfig_framework
libbulkdata_la_CFLAGS = -DFEATURE_SUPPORT_WEBCONFIG
endif

if ENABLE_CCSP_SUPPORT
libbulkdata_la_CPPFLAGS += -I${top_srcdir}/source/t2ssp \
                                -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/ccsp

endif

libbulkdata_la_DEPENDENCIES = ${top_builddir}/source/utils/libt2utils.la ${top_builddir}/source/dcautil/libdcautil.la ${top_builddir}/source/protocol/http/libhttp.la ${top_builddir}/source/protocol/rbusMethod/librbusmethod.la ${top_builddir}/source/ccspinterface/libccspinterface.la ${top_builddir}/source/reportgen/libreportgen.la ${top_builddir}/source/scheduler/libscheduler.la ${top_builddir}/source/xconf-client/libxconfclient.la

if IS_PRIVACYCONTROL_ENABLED
libbulkdata_la_CFLAGS = $(PRIVACYCONTROL_FLAG)
libbulkdata_la_CPPFLAGS +=  -I${top_srcdir}/source/privacycontrol
libbulkdata_la_DEPENDENCIES += ${top_builddir}/source/privacycontrol/libt2thunder_privacycontrol.la
libbulkdata_la_LIBADD += ${top_builddir}/source/privacycontrol/libt2thunder_privacycontrol.la
endif

if IS_LIBRDKCERTSEL_ENABLED
libbulkdata_la_CFLAGS = $(LIBRDKCERTSEL_FLAG)
endif

if IS_MTLS_ENABLED
libbulkdata_la_CFLAGS = $(MTLS_FLAG)
endif
