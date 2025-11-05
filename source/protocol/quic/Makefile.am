##########################################################################
# If not stated otherwise in this file or this component's Licenses.txt
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

AM_CFLAGS = -D_ANSC_LINUX
AM_CFLAGS += -D_ANSC_USER

AM_CPPFLAGS = -Wall -Werror
AM_CFLAGS += $(AM_CPPFLAGS)

AM_CFLAGS += -I$(top_srcdir)/source/include
AM_CFLAGS += -I$(top_srcdir)/source/utils
AM_CFLAGS += -I$(top_srcdir)/source/reportgen
AM_CFLAGS += -I$(top_srcdir)/source/bulkdata
AM_CFLAGS += -I$(top_srcdir)/include

noinst_LTLIBRARIES = libquicinterface.la
libquicinterface_la_SOURCES = quicinterface.c

libquicinterface_la_CPPFLAGS = $(AM_CPPFLAGS) $(CJSON_CFLAGS) $(CURL_CFLAGS)
libquicinterface_la_LDFLAGS = $(CURL_LIBS)
