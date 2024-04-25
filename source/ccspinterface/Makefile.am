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

lib_LTLIBRARIES = libccspinterface.la

libccspinterface_la_SOURCES = busInterface.c rbusInterface.c
libccspinterface_la_CFLAGS = $(GLIB_CFLAGS)
libccspinterface_la_LDFLAGS = -shared -fPIC -lrbus $(GLIB_LIBS)
if ENABLE_CCSP_SUPPORT
libccspinterface_la_LDFLAGS += -lccsp_common
libccspinterface_la_SOURCES += ccspinterface.c
endif

libccspinterface_la_CPPFLAGS = -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/ \
                                -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/ccsp \
                                -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/rbus \
                                -I${top_srcdir}/include \
                                -I${top_srcdir}/source/utils \
                                -I${top_srcdir}/source/t2ssp \
                                -I${top_srcdir}/source/bulkdata \
                                -I${top_srcdir}/source/reportgen

if IS_PRIVACYCONTROL_ENABLED
libccspinterface_la_CFLAGS += $(PRIVACYCONTROL_FLAG)
libccspinterface_la_CPPFLAGS +=  -I${top_srcdir}/source/privacycontrol
libccspinterface_la_DEPENDENCIES = ${top_builddir}/source/privacycontrol/libt2thunder_privacycontrol.la
libccspinterface_la_LIBADD = ${top_builddir}/source/privacycontrol/libt2thunder_privacycontrol.la
endif
