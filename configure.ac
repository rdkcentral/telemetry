##########################################################################
# If not stated otherwise in this file or this component's LICENSE
# file the following copyright and licenses apply:
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
AC_PREREQ([2.53])
AC_INIT(telemetry, 2.0)
AC_CONFIG_SRCDIR([source])
AC_CONFIG_SRCDIR([source/bulkdata])
AC_CONFIG_SRCDIR([source/reportgen])
AC_CONFIG_SRCDIR([source/scheduler])
AC_CONFIG_SRCDIR([source/ccspinterface])
AC_CONFIG_SRCDIR([source/t2parser])
AC_CONFIG_SRCDIR([source/xconf-client])
AC_CONFIG_SRCDIR([source/utils])
AC_CONFIG_SRCDIR([source/commonlib])
AC_CONFIG_SRCDIR([source/protocol])
AC_CONFIG_SRCDIR([source/protocol/http])
AC_CONFIG_SRCDIR([source/protocol/rbusMethod])
AC_CONFIG_SRCDIR([source/dcautil])
AM_INIT_AUTOMAKE([foreign no-dist-gzip dist-bzip2 1.9])

LT_INIT
GTEST_ENABLE_FLAG =" "
LIBSYSWRAPPER_FLAG=" "
IS_LIBSYSWRAPPER_ENABLED=" "
IS_LIBRDKCONFIG_ENABLED=" "
IS_LIBRDKCERTSEL_ENABLED=" "

AC_ARG_ENABLE([gtestapp],
             AS_HELP_STRING([--enable-gtestapp],[enable Gtest support (default is no)]),
             [
               case "${enableval}" in
                yes) GTEST_SUPPORT_ENABLED=true
                     GTEST_ENABLE_FLAG=" -DGTEST_ENABLE"
                     m4_if(m4_sysval,[0],[AC_CONFIG_FILES([source/test/Makefile source/test/dcautils/Makefile source/test/t2parser/Makefile source/test/scheduler/Makefile source/test/reportgen/Makefile source/test/bulkdata/Makefile source/test/ccspinterface/Makefile])]);;
                no) GTEST_SUPPORT_ENABLED=false AC_MSG_ERROR([Gtest support is disabled]);;
                 *) AC_MSG_ERROR([bad value ${enableval} for --enable-gtestapp ]);;
               esac
             ],
             [echo "Gtestapp is disabled"])
AM_CONDITIONAL([WITH_GTEST_SUPPORT], [test x$GTEST_SUPPORT_ENABLED = xtrue])

dnl Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_CPP
AC_PROG_CXXCPP

dnl use pretty build output with automake >= 1.11
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])],
  [AM_DEFAULT_VERBOSITY=1
   AC_SUBST(AM_DEFAULT_VERBOSITY)])


dnl **********************************
dnl checks for dependencies
dnl **********************************
AC_HEADER_STDC

dnl Checks for typedefs, structures, and compiler characteristics.
dnl AC_CHECK_HEADER_STDBOOL
AC_TYPE_UINT8_T
dnl Checks for library functions.
AC_FUNC_MALLOC

PKG_CHECK_MODULES([GLIB], [glib-2.0 >= 2.24.0])

ENABLE_TESTS=false
AM_CONDITIONAL([ENABLE_TESTS], [test x$ENABLE_TESTS = xtrue])

ENABLE_CCSP_SUPPORT=false
AC_ARG_ENABLE([ccspsupport],
              AS_HELP_STRING([--enable-ccspsupport],[enable ccsp support (default is no)]),
              [
                case "${enableval}" in
                 yes) ENABLE_CCSP_SUPPORT=true;;
                  no) AC_MSG_ERROR([ccsp support is disabled]) ;;
                   *) AC_MSG_ERROR([bad value ${enableval} for --enable-ccspsupport]) ;;
                esac
              ],
              [echo "ccsp support is disabled"])

AM_CONDITIONAL([ENABLE_CCSP_SUPPORT], [test x$ENABLE_CCSP_SUPPORT = xtrue])


AC_ARG_WITH([rdklogger],
    AC_HELP_STRING([--without-rdklogger],[disable rdklogger support]),
    [
       echo "rdklogger is disabled"
       rdklogger_enable=false

    ],
    [
       echo "rdklogger is enabled"
        rdklogger_enable=true
    ]
)
AM_CONDITIONAL([RDKLOGGER_ENABLE], [test x$rdklogger_enable = xtrue])

AC_ARG_WITH([webconfig-framework],
    AC_HELP_STRING([--without-webconfig-framework],[disable webconfig-framework support]),
    [
       echo "webconfig-framework is disabled"
       webconfig_framework=false

    ],
    [
       echo "webconfig-framework is enabled"
       webconfig_framework=true
    ]
)
AM_CONDITIONAL([WEBCONFIG_FRAMEWORK_ENABLE], [test x$webconfig_framework = xtrue])


ENABLE_DEVICE_EXTENDER=false
AC_ARG_ENABLE([deviceextender],
              AS_HELP_STRING([--enable-deviceextender],[enable device extender (default is no)]),
              [
                case "${enableval}" in
                 yes) ENABLE_DEVICE_EXTENDER=true;;
                  no) AC_MSG_ERROR([device extender support is disabled]) ;;
                   *) AC_MSG_ERROR([bad value ${enableval} for --enable-deviceextender]) ;;
                esac
              ],
              [echo "Extender device  support is disabled"])

AM_CONDITIONAL([ENABLE_DEVICE_EXTENDER], [test x$ENABLE_DEVICE_EXTENDER = xtrue])

AC_ARG_ENABLE([libsyswrapper],
        AS_HELP_STRING([--enable-libsyswrapper],[enables system wrapper replacement (default is no)]),
        [
          case "${enableval}" in
           yes) IS_LIBSYSWRAPPER_ENABLED=true
                LIBSYSWRAPPER_FLAG=" -DLIBSYSWRAPPER_BUILD ";;
           no)  IS_LIBSYSWRAPPER_ENABLED=false ;;
          *) AC_MSG_ERROR([bad value ${enableval} for --enable-libsyswrapper]) ;;
           esac
           ],
        [echo "libsyswrapper is disabled"])
AM_CONDITIONAL([IS_LIBSYSWRAPPER_ENABLED], [test x$IS_LIBSYSWRAPPER_ENABLED = xtrue])
AC_SUBST(LIBSYSWRAPPER_FLAG)

#privacy control flag
IS_PRIVACYCONTROL_ENABLED=false
AC_ARG_ENABLE([privacycontrol],
          AS_HELP_STRING([--enable-privacycontrol],[enables privacy control functions (default is no)]),
          [
            case "${enableval}" in
            yes) IS_PRIVACYCONTROL_ENABLED=true
                 PRIVACYCONTROL_FLAG=" -DPRIVACYMODES_CONTROL ";;
            no)  IS_PRIVACYCONTROL_ENABLED=false ;;
            *) AC_MSG_ERROR([bad value ${enableval} for --enable-privacycontrol]) ;;
            esac
          ],
          [echo "privacycontrol is disabled"])
AM_CONDITIONAL([IS_PRIVACYCONTROL_ENABLED], [test x$IS_PRIVACYCONTROL_ENABLED = xtrue])
AC_SUBST(PRIVACYCONTROL_FLAG)

#privacy control add the CFLAGS to all the files
if test x$IS_PRIVACYCONTROL_ENABLED = xtrue; then
    CPPFLAGS="$CPPFLAGS $PRIVACYCONTROL_FLAG"
    CFLAGS="$CFLAGS $PRIVACYCONTROL_FLAG"
fi

AC_ARG_ENABLE([mountutils],
        AS_HELP_STRING([--enable-mountutils],[enables mountutils replacement (default is no)]),
        [
          case "${enableval}" in
           yes) IS_LIBRDKCONFIG_ENABLED=true
                LIBRDKCONFIG_FLAG=" -DLIBRDKCONFIG_BUILD ";;
           no)  IS_LIBRDKCONFIG_ENABLED=false ;;
          *) AC_MSG_ERROR([bad value ${enableval} for --enable-mountutils]) ;;
           esac
           ],
        [echo "mountutils is disabled"])
AM_CONDITIONAL([IS_LIBRDKCONFIG_ENABLED], [test x$IS_LIBRDKCONFIG_ENABLED = xtrue])
AC_SUBST(LIBRDKCONFIG_FLAG)

AC_ARG_ENABLE([rdkcertselector],
        AS_HELP_STRING([--enable-rdkcertselector],[enables rdkcertselector replacement (default is no)]),
        [
          case "${enableval}" in
           yes) IS_LIBRDKCERTSEL_ENABLED=true
                LIBRDKCERTSEL_FLAG=" -DLIBRDKCERTSEL_BUILD ";;
           no)  IS_LIBRDKCERTSEL_ENABLED=false ;;
          *) AC_MSG_ERROR([bad value ${enableval} for --enable-rdkcertselector]) ;;
           esac
           ],
        [echo "rdkcertselector is disabled"])
AM_CONDITIONAL([IS_LIBRDKCERTSEL_ENABLED], [test x$IS_LIBRDKCERTSEL_ENABLED = xtrue])
AC_SUBST(LIBRDKCERTSEL_FLAG)

AC_CONFIG_FILES([Makefile
                 source/Makefile
                 source/bulkdata/Makefile
                 source/reportgen/Makefile
                 source/scheduler/Makefile
                 source/ccspinterface/Makefile
                 source/t2parser/Makefile
                 source/xconf-client/Makefile
                 source/utils/Makefile
                 source/commonlib/Makefile
                 source/protocol/Makefile
                 source/dcautil/Makefile
                 source/protocol/http/Makefile
                 source/protocol/rbusMethod/Makefile])

AM_COND_IF([ENABLE_CCSP_SUPPORT], [AC_CONFIG_FILES([source/t2dm/Makefile])])
AM_COND_IF([ENABLE_CCSP_SUPPORT], [AC_CONFIG_SRCDIR([source/t2dm])])

#privacy control add the source dir and Makefile
AM_COND_IF([IS_PRIVACYCONTROL_ENABLED], [AC_CONFIG_FILES([source/privacycontrol/Makefile])])
AM_COND_IF([IS_PRIVACYCONTROL_ENABLED], [AC_CONFIG_SRCDIR([source/privacycontrol])])

AC_SUBST(GTEST_ENABLE_FLAG)
AC_OUTPUT
