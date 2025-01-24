####################################################################################
# If not stated otherwise in this file or this component's Licenses file the
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


from helper_functions import *




# Add a main function to run the tests.
# This is the entry point for the test suite.

if __name__ == "__main__":

    GREP_STRING = "Calling rbus_invokeRemoteMethod2 for"
    response = grep_T2logs(GREP_STRING)
    print(response)

    print("Clearing T2 logs")
    clear_T2logs()
    print("Messages after clearing")
    response = grep_T2logs(GREP_STRING)
    print(response)
    