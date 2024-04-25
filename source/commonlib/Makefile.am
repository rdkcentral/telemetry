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
AM_CFLAGS += -DCCSP_INC_no_asm_sigcontext_h

ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libtelemetry_msgsender.la

libtelemetry_msgsender_la_SOURCES = telemetry_busmessage_sender.c
libtelemetry_msgsender_la_LDFLAGS = -shared
libtelemetry_msgsender_la_LIBADD = -lrbus ${top_builddir}/source/utils/libt2utils.la
if ENABLE_CCSP_SUPPORT
libtelemetry_msgsender_la_LIBADD += -lccsp_common
endif
libtelemetry_msgsender_la_CPPFLAGS = -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/rbus \
                                -I${top_srcdir}/source/utils \
                                -I${top_srcdir}/include
libtelemetry_msgsender_la_DEPENDENCIES = ${top_builddir}/source/utils/libt2utils.la

bin_PROGRAMS = telemetry2_0_client

telemetry2_0_client_SOURCES = telemetry_client.c telemetry_busmessage_sender.c
telemetry2_0_client_LDADD = -lrbus ${top_builddir}/source/utils/libt2utils.la -lpthread
if ENABLE_CCSP_SUPPORT
telemetry2_0_client_LDADD += -lccsp_common
endif
telemetry2_0_client_CPPFLAGS = -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/ccsp \
                                -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/rbus \
                                -I${top_srcdir}/source/utils \
                                -I${top_srcdir}/include
telemetry2_0_client_DEPENDENCIES = ${top_builddir}/source/utils/libt2utils.la
