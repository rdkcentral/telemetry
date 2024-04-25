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

lib_LTLIBRARIES = libscheduler.la

libscheduler_la_SOURCES = scheduler.c
libscheduler_la_LDFLAGS = -shared -fPIC
if ENABLE_CCSP_SUPPORT
libscheduler_la_LDFLAGS += -lccsp_common
endif
libscheduler_la_CPPFLAGS = -fPIC -I${top_srcdir}/include \
                                -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/ccsp \
                                -I${top_srcdir}/source/ccspinterface \
                                -I${top_srcdir}/source/bulkdata \
                                -I${top_srcdir}/source/reportgen \
                                -I${top_srcdir}/source/utils
libscheduler_la_LIBADD = ${top_builddir}/source/utils/libt2utils.la
