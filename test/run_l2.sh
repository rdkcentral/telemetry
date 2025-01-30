#!/bin/sh

####################################################################################
# If not stated otherwise in this file or this component's Licenses.txt file the
# following copyright and licenses apply:
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
####################################################################################

export top_srcdir=`pwd`
RESULT_DIR="/tmp/l2_test_report"
mkdir -p "$RESULT_DIR"
#pytest -v --json-report --json-report-summary --json-report-file $RESULT_DIR/runs_as_daemon.json test/functional-tests/tests/test_runs_as_daemon.py --exitfirst
#pytest -v --json-report --json-report-summary --json-report-file $RESULT_DIR/bootup_sequence.json test/functional-tests/tests/test_bootup_sequence.py --exitfirst
#pytest -v --json-report --json-report-summary --json-report-file $RESULT_DIR/xconf_communications.json test/functional-tests/tests/test_xconf_communications.py --exitfirst
#pytest -v --json-report --json-report-summary --json-report-file $RESULT_DIR/msg_packet.json test/functional-tests/tests/test_multiprofile_msgpacket.py --exitfirst
