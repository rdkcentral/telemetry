#!/bin/sh

####################################################################################
# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2025 RDK Management
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



INPUT=$1

EXAMPLE_REPORT_PROFILE='{"profiles":[{"name":"EncodingCheck","hash":"Hash77","value":{"Name":"EncodingCheck2","Description":"RDKB_Profile","Version":"0.1","Protocol":"HTTP","EncodingType":"JSON","ActivationTimeOut":40,"DeleteOnTimeout":true,"ReportingInterval":10,"GenerateNow":false,"RootName":"FR2_US_TC3","TimeReference":"2023-01-25T13:47:00Z","Parameter":[{"type":"dataModel","name":"MODEL_NAME","reference":"Device.DeviceInfo.ModelName","use":"absolute","regex":"[A-Z]+"},{"type":"event","eventName":"TEST_EVENT_MARKER_2","component":"sysint","use":"absolute","regex":"[0-9]+"},{"type":"grep","marker":"SYS_INFO_CrashPortalUpload_success","search":"Success uploading","logFile":"core_log.txt","use":"absolute"}],"ReportingAdjustments":{"ReportOnUpdate":true,"FirstReportingInterval":5,"MaxUploadLatency":10},"HTTP":{"URL":"https://mockxconf:50051/dataLakeMock","Compression":"None","Method":"POST","RequestURIParameter":[{"Name":"reportName","Reference":"Profile.Name"}]},"JSONEncoding":{"ReportFormat":"NameValuePair","ReportTimestamp":"None"}}}]}'

# Minified version of the DOCSIS reference profile for testing purposes.
DOCSIS_REFERENCE_PROFILE='{"profiles":[{"name":"DOCSIS_Sample","hash":"hash1","value":{"Name":"DOCSIS_Sample","Description":"Reference profile collecting DOCSIS parameters every 10 minutes","Version":"1","Protocol":"HTTP","EncodingType":"JSON","ReportingInterval":600,"TimeReference":"0001-01-01T00:00:00Z","Parameter":[{"type":"dataModel","name":"Device_Time","reference":"Device.DeviceInfo.X_RDKCENTRAL-COM_SystemTime"},{"type":"dataModel","reference":"Profile.Name"},{"type":"dataModel","reference":"Profile.Version"},{"type":"dataModel","name":"DS1Frequency","reference":"Device.X_CISCO_COM_CableModem.DownstreamChannel.1.Frequency"},{"type":"dataModel","name":"DS1LockStatus","reference":"Device.X_CISCO_COM_CableModem.DownstreamChannel.1.LockStatus"},{"type":"dataModel","name":"DS1Uncorrectables","reference":"Device.X_CISCO_COM_CableModem.DownstreamChannel.1.Uncorrectables"},{"type":"dataModel","name":"DS10Frequency","reference":"Device.X_CISCO_COM_CableModem.DownstreamChannel.10.Frequency"},{"type":"dataModel","name":"DS10LockStatus","reference":"Device.X_CISCO_COM_CableModem.DownstreamChannel.10.LockStatus"},{"type":"dataModel","name":"DS10Uncorrectables","reference":"Device.X_CISCO_COM_CableModem.DownstreamChannel.10.Uncorrectables"}],"HTTP":{"URL":"https://mockxconf:50051/dataLakeMock","Compression":"None","Method":"POST","RequestURIParameter":[{"Name":"profileName","Reference":"Profile.Name"},{"Name":"profileDescription","Reference":"Profile.Description"},{"Name":"reportVersion","Reference":"Profile.Version"}]},"JSONEncoding":{"ReportFormat":"NameValuePair","ReportTimestamp":"None"}}}]}'

if [ -z "$INPUT" ]; then
    echo "Usage: $0 ${EXAMPLE_REPORT_PROFILE}"
    exit 1
fi

if [ "$INPUT" = "example" ]; then
    INPUT=$EXAMPLE_REPORT_PROFILE
elif [ "$INPUT" = "docsis" ]; then
    INPUT=$DOCSIS_REFERENCE_PROFILE
elif [ "$INPUT" = "empty" ]; then
    INPUT='{"profiles":[]}'
fi

rbuscli setv "Device.X_RDKCENTRAL-COM_T2.Temp_ReportProfiles" string "$INPUT"
