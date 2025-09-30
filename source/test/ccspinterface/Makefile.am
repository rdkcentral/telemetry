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

AM_CXXFLAGS = -fno-permissive -std=c++11 -DGTEST_ENABLE -DPERSIST_LOG_MON_REF -fPIC -DPRIVACYMODES_CONTROL -DBULKDATA -DCCSP_SUPPORT_ENABLED -DFEATURE_SUPPORT_WEBCONFIG -DDROP_ROOT_PRIV -DDCMAGENT

AM_CFLAGS = -DGTEST_ENABLE -DPERSIST_LOG_MON_REF -fPIC -DCCSP_SUPPORT_ENABLED -DPRIVACYMODES_CONTROL -DDCMAGENT

AUTOMAKE_OPTIONS = subdir-objects

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = ccspinterface_gtest.bin

ccspinterface_gtest_bin_CPPFLAGS = -I$(PKG_CONFIG_SYSROOT_DIR)/usr/include/gtest -I$(PKG_CONFIG_SYSROOT_DIR)/usr/include/glib-2.0 -I$(PKG_CONFIG_SYSROOT_DIR)/usr/lib/x86_64-linux-gnu/glib-2.0/include -I$(PKG_CONFIG_SYSROOT_DIR)/usr/local/lib -I$(PKG_CONFIG_SYSROOT_DIR)$(includedir)/gtest -I${top_srcdir}/gtest/include -I${top_srcdir}/source/include -I${top_srcdir}/source -I${top_srcdir}/source/test/mocks -I$(PKG_CONFIG_SYSROOT_DIR)/usr/local/include/rbus -I${top_srcdir}/source/test/rdk_logger -I${top_srcdir}/include -I${top_srcdir}/source/utils -I${top_srcdir}/source/privacycontrol -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/rbus -I${top_srcdir}/source/dcautil -I${top_srcdir}/source/ccspinterface -I${top_srcdir}/source/reportgen -I${top_srcdir}/source/xconf-client -I${top_srcdir}/source/protocol/http -I${top_srcdir}/source/protocol/rbusMethod -I${top_srcdir}/source/t2parser -I${top_srcdir}/source/bulkdata -I${top_srcdir}/source/scheduler -I${top_srcdir}/source/ccspinterface -I${PKG_CONFIG_SYSROOT_DIR}$(includedir) -I${top_srcdir}/source/test/ccspinterface -I${PKG_CONFIG_SYSROOT_DIR}/usr/src/googletest/googlemock/include -I${RDK_PROJECT_ROOT_PATH}/$(GLIB_CFLAGS) -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/glib-2.0 -I${PKG_CONFIG_SYSROOT_DIR}$(libdir)/glib-2.0/include

ccspinterface_gtest_bin_SOURCES = gtest_main.cpp ../mocks/SystemMock.cpp ../mocks/FileioMock.cpp ../mocks/rdklogMock.cpp ../mocks/rbusMock.cpp ../mocks/rdkconfigMock.cpp ../mocks/VectorMock.cpp CcspInterfaceMock.cpp ccspinterfaceTest.cpp ../../utils/t2log_wrapper.c ../../privacycontrol/rdkservices_privacyutils.c ../../utils/persistence.c ../../utils/t2common.c ../../utils/t2collection.c ../../utils/t2MtlsUtils.c ../../ccspinterface/rbusInterface.c ../../ccspinterface/busInterface.c ../../reportgen/reportgen.c

ccspinterface_gtest_bin_LDFLAGS = -L/usr/src/googletest/googletest/lib/.libs  -lgcov -L/src/googletest/googlemock/lib -L/usr/src/googletest/googlemock/lib/.libs -L/usr/include/glib-2.0 -lgmock -lcjson -lcurl -lmsgpackc -lgtest -lgtest_main -lglib-2.0
