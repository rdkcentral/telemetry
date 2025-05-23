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
AUTOMAKE_OPTIONS = subdir-objects


SUBDIRS = t2parser reportgen scheduler dcautils

AM_CXXFLAGS = -std=c++11 -DGTEST_ENABLE -DGTEST_HAS_PTHREAD=0

AM_CFLAGS = -DGTEST_ENABLE -DGTEST_HAS_PTHREAD=0

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = telemetry_gtest.bin

telemetry_gtest_bin_CPPFLAGS = -I$(PKG_CONFIG_SYSROOT_DIR)/usr/include/gtest -I$(PKG_CONFIG_SYSROOT_DIR)/usr/include/glib-2.0 -I$(PKG_CONFIG_SYSROOT_DIR)/usr/lib/x86_64-linux-gnu/glib-2.0/include -I$(PKG_CONFIG_SYSROOT_DIR)/usr/local/lib -I$(PKG_CONFIG_SYSROOT_DIR)$(includedir)/gtest -I${top_srcdir}/gtest/include -I${top_srcdir}/source/include -I${top_srcdir}/source -I${top_srcdir}/source/test/mocks -I${top_srcdir}/source/test/rbus -I${top_srcdir}/source/test/rdk_logger
telemetry_gtest_bin_CPPFLAGS +=  -I${top_srcdir}/source/test/mocks/include/rbus -I${top_srcdir}/include -I${top_srcdir}/source/utils -I${top_srcdir}/source/protocol -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/rbus
telemetry_gtest_bin_CPPFLAGS += -I${top_srcdir}/source/bulkdata -I${top_srcdir}/source/privacycontrol -I${top_srcdir}/source/ccspinterface -I${top_srcdir}/source/reportgen  -I${PKG_CONFIG_SYSROOT_DIR}$(includedir) -I${top_srcdir}/source/t2ssp
telemetry_gtest_bin_CPPFLAGS +=  -I${PKG_CONFIG_SYSROOT_DIR}/usr/src/googletest/googlemock/include
telemetry_gtest_bin_CPPFLAGS += -I${top_srcdir}/source/dcautil -I${RDK_PROJECT_ROOT_PATH}/$(GLIB_CFLAGS)
telemetry_gtest_bin_CPPFLAGS += -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/glib-2.0 -I${PKG_CONFIG_SYSROOT_DIR}$(libdir)/glib-2.0/include

if ENABLE_CCSP_SUPPORT
telemetry_gtest_bin_SOURCES = gtest_main.cpp mocks/FileioMock.cpp mocks/SystemMock.cpp mocks/rdklogMock.cpp utils/UtilsTest.cpp ../privacycontrol/rdkservices_privacyutils.c ../t2ssp/ssp_main.c ../t2ssp/ssp_action.c ../t2ssp/ssp_messagebus_interface.c ../t2ssp/dm_pack_datamodel.c
else
telemetry_gtest_bin_SOURCES = gtest_main.cpp mocks/FileioMock.cpp mocks/SystemMock.cpp mocks/rdklogMock.cpp utils/UtilsTest.cpp ../privacycontrol/rdkservices_privacyutils.c ../utils/persistence.c ../utils/t2common.c ../utils/t2collection.c ../utils/vector.c ../utils/t2MtlsUtils.c ../utils/t2log_wrapper.c
endif

telemetry_gtest_bin_LDFLAGS = -L/usr/src/googletest/googletest/lib/.libs  -lgcov -L/usr/src/googletest/googlemock/lib/.libs  -L/usr/include/glib-2.0 -lgmock -lcjson -lcurl -lmsgpackc -lgtest -lgtest_main -lglib-2.0
