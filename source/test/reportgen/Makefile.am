##########################################################################
# If not stated otherwise in this file or this component's LICENSE
# file the following copyright and licenses apply:
#
# Copyright 2018 RDK Management
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


AM_CXXFLAGS = -std=c++11 -DGTEST_ENABLE

AM_CFLAGS = -DGTEST_ENABLE

AUTOMAKE_OPTIONS = subdir-objects

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = reportgen_gtest.bin

reportgen_gtest_bin_CPPFLAGS = -I$(PKG_CONFIG_SYSROOT_DIR)/usr/include/gtest -I$(PKG_CONFIG_SYSROOT_DIR)/usr/include/glib-2.0 -I$(PKG_CONFIG_SYSROOT_DIR)/usr/lib/x86_64-linux-gnu/glib-2.0/include -I$(PKG_CONFIG_SYSROOT_DIR)/usr/local/lib -I${top_srcdir}/source/include -I${top_srcdir}/source/dcautil -I${top_srcdir}/source/utils -I${top_srcdir}/source/ccspinterface -I${top_srcdir}/source -I${top_srcdir}/include -I${top_srcdir}/source/reportgen -I${top_srcdir}/source/test/rbus -I${top_srcdir}/source/test/rdk_logger -I$(PKG_CONFIG_SYSROOT_DIR)/usr/lib -I$(PKG_CONFIG_SYSROOT_DIR)/usr/include -I$(PKG_CONFIG_SYSROOT_DIR)/usr/include/cjson -I$(PKG_CONFIG_SYSROOT_DIR)/usr/include/gmock 

reportgen_gtest_bin_SOURCES = gtest_main.cpp reportgenTest.cpp reportgenMock.cpp ../mocks/rdklogMock.cpp ../../utils/t2log_wrapper.c ../../reportgen/reportgen.c ../../utils/vector.c ../../utils/t2common.c 

reportgen_gtest_bin_LDFLAGS = -lgtest -lgcov -L/src/googletest/googlemock/lib -L/usr/src/googletest/googlemock/lib/.libs -lgmock -lcjson -lcurl -lmsgpackc
