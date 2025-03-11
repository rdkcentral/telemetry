##########################################################################
# If not stated otherwise in this file or this component's LICENSE
# file the following copyright and licenses apply:
#
# Copyright 2016 RDK Management
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
lib_LTLIBRARIES = libdcautil.la

libdcautil_la_SOURCES = dcautil.c dca.c dcalist.c legacyutils.c dcaproc.c dcajson.c rdk_linkedlist.c
libdcautil_la_CFLAGS =
libdcautil_la_LDFLAGS = -shared -fPIC -lcjson
libdcautil_la_CFLAGS += $(DBUS_CFLAGS) -DUSE_TR181_CCSP_MESSAGEBUS
libdcautil_la_LDFLAGS += $(DBUS_LIBS)

libdcautil_la_LIBADD = ${top_builddir}/source/ccspinterface/libccspinterface.la ${top_builddir}/source/utils/libt2utils.la

libdcautil_la_CFLAGS += -fPIC -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/dbus-1.0 \
                                -I${PKG_CONFIG_SYSROOT_DIR}$(libdir)/dbus-1.0/include \
				-I${PKG_CONFIG_SYSROOT_DIR}$(includedir) \
				-I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/rbus \
                                -I${top_srcdir}/include \
                                -I${top_srcdir}/source/ccspinterface \
                                -I${top_srcdir}/source/utils \
                                -I${top_srcdir}/source/bulkdata \
                                -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/ccsp

libdcautil_la_DEPENDENCIES = ${top_builddir}/source/ccspinterface/libccspinterface.la ${top_builddir}/source/utils/libt2utils.la
