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
AM_CFLAGS =

AM_CXXFLAGS = -std=c++11

AM_CPPFLAGS = -fno-permissive

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = xconfclient_gtest.bin

xconfclient_gtest_bin_CPPFLAGS = -I$(PKG_CONFIG_SYSROOT_DIR)$(includedir)/gtest \
                                 -I${top_srcdir}/gtest/include \
                                 -I${top_srcdir}/source/include \
                                 -I${top_srcdir}/source \
                                 -I${top_srcdir}/include \
                                 -I${top_srcdir}/source/utils \
                                 -I${top_srcdir}/source/protocol \
                                 -I${top_srcdir}/source/t2parser \
                                 -I${top_srcdir}/source/xconf-client \
                                 -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/rbus \
                                 -I${top_srcdir}/source/bulkdata \
                                 -I${top_srcdir}/source/ccspinterface \
                                 -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/ccsp \
                                 -I${top_srcdir}/source/reportgen \
                                 -I${PKG_CONFIG_SYSROOT_DIR}$(includedir) \
                                 -I${top_srcdir}/source/t2ssp \
                                 -I${top_srcdir}/source/dcautil \
                                 -I${RDK_PROJECT_ROOT_PATH}/opensource/include \
                                 -I${RDK_PROJECT_ROOT_PATH}/opensource/include/glib-2.0/ \
                                 -I${RDK_PROJECT_ROOT_PATH}/opensource/lib/glib-2.0/include/ \
                                 -I${RDK_PROJECT_ROOT_PATH}/opensource/include/glib-2.0/glib \
                                 -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/glib-2.0 \
                                 -I${PKG_CONFIG_SYSROOT_DIR}$(libdir)/glib-2.0/include

if ENABLE_CCSP_SUPPORT
xconfclient_gtest_bin_SOURCES = gtest_main.cpp xconfclientTest.cpp xconfclientMocks/fileMock.cpp ../../t2ssp/ssp_main.c ../../t2ssp/ssp_action.c ../../t2ssp/ssp_messagebus_interface.c ../../t2ssp/dm_pack_datamodel.c
else
xconfclient_gtest_bin_SOURCES = gtest_main.cpp xconfclientTest.cpp xconfclientMocks/fileMock.cpp
endif

xconfclient_gtest_bin_LDFLAGS = -lgtest -lgmock -lgcov -lcurl -lrbus $(GLIB_LIBS) -lcjson -lmsgpackc

xconfclient_gtest_bin_LDADD = ${top_builddir}/source/utils/libt2utils.la ${top_builddir}/source/dcautil/libdcautil.la ${top_builddir}/source/protocol/http/libhttp.la ${top_builddir}/source/protocol/rbusMethod/librbusmethod.la ${top_builddir}/source/ccspinterface/libccspinterface.la ${top_builddir}/source/reportgen/libreportgen.la ${top_builddir}/source/scheduler/libscheduler.la ${top_builddir}/source/bulkdata/libbulkdata.la ${top_builddir}/source/t2parser/libt2parser.la ${top_builddir}/source/xconf-client/libxconfclient.la ${top_builddir}/source/interChipHelper/libinterChipHelper.la ${top_builddir}/source/dcautil/libdcautil.la

xconfclient_gtest_bin_DEPENDENCIES = ${top_builddir}/source/xconf-client/libxconfclient.la ${top_builddir}/source/ccspinterface/libccspinterface.la ${top_builddir}/source/t2parser/libt2parser.la

xconfclient_gtest_bin_la_DEPENDENCIES = ${top_builddir}/source/xconf-client/libxconfclient.la ${top_builddir}/source/ccspinterface/libccspinterface.la ${top_builddir}/source/t2parser/libt2parser.la
