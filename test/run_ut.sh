#!/bin/sh

####################################################################################
# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2023 RDK Management
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
####################################################################################



export top_srcdir=`pwd`

ENABLE_COV=false

if [ "x$1" = "x--enable-cov" ]; then
      echo "Enabling coverage options"
      export CXXFLAGS="-g -O0 -fprofile-arcs -ftest-coverage"
      export CFLAGS="-g -O0 -fprofile-arcs -ftest-coverage"
      export LDFLAGS="-lgcov --coverage"
      ENABLE_COV=true
fi

autoreconf --install

./configure --enable-gtestapp

make -C source/test

# Execute test suites for different sub-modules

# - Disabled for now as the gtest framework ends up in a segfault at tear down
# ./source/test/dcautils/dcautil_gtest.bin
# ./source/test/protocol/protocol_gtest.bin
# ./source/test/xconf-client/xconfclient_gtest.bin
# ./source/test/telemetry_gtest.bin

./source/test/reportgen/reportgen_gtest.bin
./source/test/scheduler/scheduler_gtest.bin
./source/test/t2parser/t2parser_gtest.bin


#### Generate the coverage report ####
if [ "$ENABLE_COV" = true ]; then
    echo "Generating coverage report"
    lcov --directory . --capture --output-file coverage.info
    lcov --remove coverage.info "${PWD}/source/test/*" --output-file coverage.info

    lcov --remove coverage.info "$HOME/usr/*" --output-file coverage.info
    lcov --remove coverage.info "/usr/*" --output-file coverage.info
    lcov --list coverage.info
fi


