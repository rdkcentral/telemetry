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

lib_LTLIBRARIES = libxconfclient.la

libxconfclient_la_SOURCES = xconfclient.c
libxconfclient_la_LDFLAGS = -shared -fPIC -lcjson -lcurl -lrdk_otlp
if IS_LIBRDKCERTSEL_ENABLED
libxconfclient_la_CFLAGS = $(LIBRDKCERTSEL_FLAG)
libxconfclient_la_LDFLAGS += -lRdkCertSelector
endif
libxconfclient_la_LIBADD = ${top_builddir}/source/t2parser/libt2parser.la ${top_builddir}/source/ccspinterface/libccspinterface.la
libxconfclient_la_CPPFLAGS = -fPIC -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/dbus-1.0 \
                                -I${PKG_CONFIG_SYSROOT_DIR}$(libdir)/dbus-1.0/include \
                                -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/ccsp \
                                -I${top_srcdir}/include \
                                -I${top_srcdir}/source \
                                -I${top_srcdir}/source/bulkdata \
                                -I${top_srcdir}/source/reportgen \
                                -I${top_srcdir}/source/ccspinterface \
                                -I${top_srcdir}/source/t2parser \
                                -I${top_srcdir}/source/dcautil \
                                -I${top_srcdir}/source/utils

libxconfclient_la_DEPENDENCIES = ${top_builddir}/source/ccspinterface/libccspinterface.la ${top_builddir}/source/t2parser/libt2parser.la
