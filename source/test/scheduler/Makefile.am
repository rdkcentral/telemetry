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

CFLAGS := $(filter-out -DFEATURE_SUPPORT_WEBCONFIG, $(CFLAGS))

AUTOMAKE_OPTIONS = subdir-objects

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = scheduler_gtest.bin

scheduler_gtest_bin_CPPFLAGS = -I$(PKG_CONFIG_SYSROOT_DIR)$(includedir)/gtest -I$(PKG_CONFIG_SYSROOT_DIR)$(includedir)/gtest/internal -I${top_srcdir}/gtest/include -I${top_srcdir}/source/include -I${top_srcdir}/source -I${top_srcdir}/include -I${top_srcdir}/source/utils -I${top_srcdir}/source/protocol -I${top_srcdir}/source/t2parser -I${top_srcdir}/source/xconf-client -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/rbus -I${top_srcdir}/source/bulkdata -I${top_srcdir}/source/ccspinterface -I${top_srcdir}/source/test/rbus -I${top_srcdir}/source/test/rdk_logger -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/dbus-1.0 -I${PKG_CONFIG_SYSROOT_DIR}$(libdir)/dbus-1.0/include  -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/ccsp -I${top_srcdir}/source/reportgen  -I${PKG_CONFIG_SYSROOT_DIR}$(includedir) -I${top_srcdir}/source/t2ssp -I${top_srcdir}/source/dcautil -I${RDK_PROJECT_ROOT_PATH}/opensource/include -I${RDK_PROJECT_ROOT_PATH}/opensource/include/glib-2.0/ -I${RDK_PROJECT_ROOT_PATH}/opensource/lib/glib-2.0/include/ -I${RDK_PROJECT_ROOT_PATH}/opensource/include/glib-2.0/glib -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/glib-2.0 -I${PKG_CONFIG_SYSROOT_DIR}$(libdir)/glib-2.0/includei -I$(PKG_CONFIG_SYSROOT_DIR)/usr/src/googletest/googlemock/include

scheduler_gtest_bin_SOURCES = gtest_main.cpp schedulerTest.cpp ../../scheduler/scheduler.c ../../utils/t2log_wrapper.c ../../utils/vector.c ../mocks/rdklogMock.cpp schedulerMock.cpp

scheduler_gtest_bin_LDFLAGS = -lgtest -lgcov -L/src/googletest/googlemock/lib -L/usr/src/googletest/googlemock/lib/.libs -lgmock -lcjson -lcurl -lmsgpackc
 



