#!/bin/bash
##########################################################################
# If not stated otherwise in this file or this component's LICENSE
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


# Build container that provides mock xconf service
cd mock-xconf
docker build --no-cache -t mock-xconf:latest -f Dockerfile .
cd -


cd native-platform
rm -rf rdk_logger
rm -rf WebconfigFramework

git clone https://github.com/rdkcentral/rdk_logger.git
git clone https://github.com/rdkcentral/WebconfigFramework.git
# Dump the contents of /etc/xconf/certs/mock-xconf-server-cert.pem from above container into a file called mock-xconf-server-cert.pem
docker run --rm mockxconf:latest cat /etc/xconf/certs/mock-xconf-server-cert.pem > mock-xconf-server-cert.pem

docker build -t native-platform:latest -f Dockerfile .

rm -f mock-xconf-server-cert.pem
rm -rf rdk_logger
rm -rf WebconfigFramework
cd -



