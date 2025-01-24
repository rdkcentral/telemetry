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

bin_PROGRAMS = testModules testCommonLib

testModules_SOURCES = testModules.c busInterfaceTests.c ../t2ssp/ssp_main.c ../t2ssp/ssp_action.c ../t2ssp/ssp_messagebus_interface.c ../t2ssp/dm_pack_datamodel.c
testModules_LDFLAGS = -lcjson -lccsp_common -lwebconfig_framework
testModules_CPPFLAGS = -fPIC -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/dbus-1.0 \
                                -I${PKG_CONFIG_SYSROOT_DIR}$(libdir)/dbus-1.0/include \
                                -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/ \
                                -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/ccsp \
                                -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/rbus \
                                -I${top_srcdir}/include \
                                -I${top_srcdir}/source/ccspinterface \
                                -I${top_srcdir}/source/bulkdata \
                                -I${top_srcdir}/source/dcautil \
                                -I${top_srcdir}/source/t2ssp \
                                -I${top_srcdir}/source/reportgen \
                                -I${top_srcdir}/source/utils
                                
testModules_LDADD = ${top_builddir}/source/dcautil/libdcautil.la ${top_builddir}/source/utils/libt2utils.la

testCommonLib_SOURCES = testCommonLibApi.c
testCommonLib_LDFLAGS = -L${top_builddir}/source/commonlib/.libs/ -ltelemetry_msgsender

testCommonLib_CPPFLAGS = -fPIC -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/dbus-1.0 \
                                -I${PKG_CONFIG_SYSROOT_DIR}$(libdir)/dbus-1.0/include \
                                -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/ \
                                -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/ccsp \
                                -I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/rbus \
                                -I${top_srcdir}/source/utils \
                                -I${top_srcdir}/include 
                                
                                
#testCommonLib_LDADD = ${top_builddir}/source/commonlib/libtelemetry_msgsender.la
