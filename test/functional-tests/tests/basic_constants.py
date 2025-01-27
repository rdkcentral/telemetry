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


ADMIN_SUPPORT_URL = "https://mockxconf:50050/adminSupport"
CONFIG_URL = "https://mockxconf:50050/"
ADMIN_CACHE_ARG = "saveRequest"
ADMIN_RQUEST_DATA = "returnData"
RUN_START_TIME = None
ADMIN_SUPPORT_GET = ADMIN_SUPPORT_URL + "Get"
ADMIN_SUPPORT_SET = ADMIN_SUPPORT_URL + "Set"

XCONF_PERSISTANT_REGX = "/opt/.t2*"
XCONF_PERSISTANT_FILE = "/opt/.t2persistentfolder/DCMresponse.txt"

T2_CONFIG_READY = "/tmp/.t2ConfigReady"
BOOTUP_FLAG = "/tmp/telemetry_initialized_bootup"
T2_READY_TO_RECIVE = "/tmp/.t2ReadyToReceiveEvents"
BOOT_FLAGS_LIST = [T2_CONFIG_READY, BOOTUP_FLAG, T2_READY_TO_RECIVE]

RBUSCLI_GET_CMD = "rbuscli get "
RBUSCLI_SET_CMD = "rbuscli set "

T2_REPORT_PROFILE_PARAM="Device.X_RDKCENTRAL-COM_T2.ReportProfiles"
T2_REPORT_PROFILE_PARAM_MSG_PCK="Device.X_RDKCENTRAL-COM_T2.ReportProfilesMsgPack"
T2_TEMP_REPORT_PROFILE_PARAM="Device.X_RDKCENTRAL-COM_T2.Temp_ReportProfiles"
RBUS_EXCEPTION_STRING = "Failed to get the data"

LOG_FILE = "/opt/logs/telemetry2_0.txt.0"

DATA_LAKE_URL = "https://mockxconf:50051"
DL_ADMIN_URL = "https://mockxconf:50051/adminSupport"


"""
ADMIDIO_URL = "https://mockxconf:50050/adminSupport"
ADMIN_ENABLE_CACHE = ADMIDIO_URL + "?saveRequest=true"
ADMIN_DISABLE_CACHE = ADMIDIO_URL + "?saveRequest=false"
ADMIN_RQUEST_DATA = ADMIDIO_URL + "?returnData=true"
RUN_START_TIME = None

XCONF_PERSISTANT_REGX = "/opt/.t2*"
XCONF_PERSISTANT_FILE = "/opt/.t2persistentfolder/DCMresponse.txt"

T2_CONFIG_READY = "/tmp/.t2ConfigReady"
BOOTUP_FLAG = "/tmp/telemetry_initialized_bootup"
T2_READY_TO_RECIVE = "/tmp/.t2ReadyToReceiveEvents"
RBUSCLI_GET_CMD = "rbuscli get "

T2_REPORT_PROFILE_PARAM="Device.X_RDKCENTRAL-COM_T2.ReportProfiles"
T2_REPORT_PROFILE_PARAM_MSG_PCK="Device.X_RDKCENTRAL-COM_T2.ReportProfilesMsgPack"
T2_TEMP_REPORT_PROFILE_PARAM="Device.X_RDKCENTRAL-COM_T2.Temp_ReportProfiles"
RBUS_EXCEPTION_STRING = "Failed to get the data"
"""
m_data = '''{ "profiles": [ { "name": "TR_AC1", "hash": "Hash1", "value": { "Name": "RDKB_Profile_1", "Description": "RDKB_Profile", "Version": "0.1", "Protocol": "HTTP", "EncodingType": "JSON", "ActivationTimeout": 1200, "ReportingInterval": 20, "GenerateNow": false, "RootName": "FR2_US_TC3", "TimeReference": "2023-01-25T13:47:00Z", "Parameter": [ { "type": "dataModel", "name": "UPTIME", "reference": "Device.DeviceInfo.UpTime", "use": "absolute" }, { "type": "event", "eventName": "USED_MEM1_split", "component": "sysint", "use": "absolute" }, { "type": "grep", "marker": "SYS_INFO_CrashPortalUpload_success", "search": "Success uploading", "logFile": "core_log.txt", "use": "count" } ], "ReportingAdjustments": [ { "ReportOnUpdate": false, "FirstReportingInterval": 15, "MaxUploadLatency": 20000 } ], "HTTP": { "URL": "https://stbrtl.r53.xcal.tv/", "Compression": "None", "Method": "POST", "RequestURIParameter": [ { "Name": "reportName", "Reference": "Profile.Name" } ] }, "JSONEncoding": { "ReportFormat": "NameValuePair", "ReportTimestamp": "None" } } }, { "name": "TR_AC2", "hash": "Hash2", "value": { "Name": "RDKB_Profile_2", "Description": "RDKB_Profile", "Version": "0.1", "Protocol": "HTTP", "EncodingType": "JSON", "ActivationTimeout": 3600, "ReportingInterval": 35, "GenerateNow": false, "RootName": "FR2_US_TC3", "TimeReference": "2023-01-25T13:47:00Z", "Parameter": [ { "type": "dataModel", "name": "UPTIME", "reference": "Device.DeviceInfo.UpTime", "use": "absolute" }, { "type": "event", "eventName": "USED_MEM2_split", "component": "sysint", "use": "absolute" }, { "type": "grep", "marker": "SYS_INFO_CrashPortal_success", "search": "Success uploading", "logFile": "core_log.txt", "use": "count" } ], "ReportingAdjustments": [ { "ReportOnUpdate": false, "FirstReportingInterval": 15, "MaxUploadLatency": 20000 } ], "HTTP": { "URL": "https://stbrtl.r53.xcal.tv/", "Compression": "None", "Method": "POST", "RequestURIParameter": [ { "Name": "reportName", "Reference": "Profile.Name" } ] }, "JSONEncoding": { "ReportFormat": "NameValuePair", "ReportTimestamp": "None" } } }, { "name": "TR_AC3", "hash": "Hash3", "value": { "Name": "RDKB_Profile_3", "Description": "RDKB_Profile", "Version": "0.1", "Protocol": "HTTP", "EncodingType": "JSON", "ActivationTimeout": 3600, "ReportingInterval": 45, "GenerateNow": false, "RootName": "FR2_US_TC3", "TimeReference": "2023-01-25T13:47:00Z", "Parameter": [ { "type": "dataModel", "name": "UPTIME", "reference": "Device.DeviceInfo.UpTime", "use": "absolute" }, { "type": "event", "eventName": "USED_MEM1_split", "component": "sysint", "use": "absolute" }, { "type": "grep", "marker": "SYS_INFO_CrashPortalUpload_success", "search": "Success uploading", "logFile": "core_log.txt", "use": "count" } ], "ReportingAdjustments": [ { "ReportOnUpdate": false, "FirstReportingInterval": 15, "MaxUploadLatency": 20000 } ], "HTTP": { "URL": "https://stbrtl.r53.xcal.tv/", "Compression": "None", "Method": "POST", "RequestURIParameter": [ { "Name": "reportName", "Reference": "Profile.Name" } ] }, "JSONEncoding": { "ReportFormat": "NameValuePair", "ReportTimestamp": "None" } } } ] }'''

data_without_namefield = '''{
    "profiles": [
        {
            "hash": "Hash7",
            "value": {
                "Name": "RDKB_Profile_4",
                "Description": "RDKB_Profile",
                "Version": "0.1",
                "Protocol": "HTTP",
                "EncodingType": "JSON",
                "ActivationTimeout": 1200,
                "ReportingInterval": 20,
                "GenerateNow": false,
                "RootName": "FR2_US_TC3",
                "TimeReference": "2023-01-25T13:47:00Z",
                "Parameter": [
                    {
                        "type": "dataModel",
                        "name": "UPTIME",
                        "reference": "Device.DeviceInfo.UpTime",
                        "use": "absolute"
                    },
                    {
                        "type": "event",
                        "eventName": "USED_MEM1_split",
                        "component": "sysint",
                        "use": "absolute"
                    },
                    {
                        "type": "grep",
                        "marker": "SYS_INFO_CrashPortalUpload_success",
                        "search": "Success uploading",
                        "logFile": "core_log.txt",
                        "use": "count"
                    }
                ],
                "ReportingAdjustments": [
                    {
                        "ReportOnUpdate": false,
                        "FirstReportingInterval": 15,
                        "MaxUploadLatency": 20000
                    }
                ],
                "HTTP": {
                    "URL": "https://stbrtl.r53.xcal.tv/",
                    "Compression": "None",
                    "Method": "POST",
                    "RequestURIParameter": [
                        {
                            "Name": "reportName",
                            "Reference": "Profile.Name"
                        }
                    ]
                },
                "JSONEncoding": {
                    "ReportFormat": "NameValuePair",
                    "ReportTimestamp": "None"
                }
            }
        },
        {
            "name": "",
            "hash": "Hash8",
            "value": {
                "Name": "RDKB_Profile_7",
                "Description": "RDKB_Profile",
                "Version": "0.1",
                "Protocol": "HTTP",
                "EncodingType": "JSON",
                "ActivationTimeout": 1200,
                "ReportingInterval": 20,
                "GenerateNow": false,
                "RootName": "FR2_US_TC3",
                "TimeReference": "2023-01-25T13:47:00Z",
                "Parameter": [
                    {
                        "type": "dataModel",
                        "name": "UPTIME",
                        "reference": "Device.DeviceInfo.UpTime",
                        "use": "absolute"
                    },
                    {
                        "type": "event",
                        "eventName": "USED_MEM1_split",
                        "component": "sysint",
                        "use": "absolute"
                    },
                    {
                        "type": "grep",
                        "marker": "SYS_INFO_CrashPortalUpload_success",
                        "search": "Success uploading",
                        "logFile": "core_log.txt",
                        "use": "count"
                    }
                ],
                "ReportingAdjustments": [
                    {
                        "ReportOnUpdate": false,
                        "FirstReportingInterval": 15,
                        "MaxUploadLatency": 20000
                    }
                ],
                "HTTP": {
                    "URL": "https://stbrtl.r53.xcal.tv/",
                    "Compression": "None",
                    "Method": "POST",
                    "RequestURIParameter": [
                        {
                            "Name": "reportName",
                            "Reference": "Profile.Name"
                        }
                    ]
                },
                "JSONEncoding": {
                    "ReportFormat": "NameValuePair",
                    "ReportTimestamp": "None"
                }
            }
        },
        {
            "name": "TR_AC10",
            "value": {
                "Name": "RDKB_Profile_10",
                "Description": "RDKB_Profile",
                "Version": "0.1",
                "Protocol": "HTTP",
                "EncodingType": "JSON",
                "ActivationTimeout": 1200,
                "ReportingInterval": 20,
                "GenerateNow": false,
                "RootName": "FR2_US_TC3",
                "TimeReference": "2023-01-25T13:47:00Z",
                "Parameter": [
                    {
                        "type": "dataModel",
                        "name": "UPTIME",
                        "reference": "Device.DeviceInfo.UpTime",
                        "use": "absolute"
                    },
                    {
                        "type": "event",
                        "eventName": "USED_MEM1_split",
                        "component": "sysint",
                        "use": "absolute"
                    },
                    {
                        "type": "grep",
                        "marker": "SYS_INFO_CrashPortalUpload_success",
                        "search": "Success uploading",
                        "logFile": "core_log.txt",
                        "use": "count"
                    }
                ],
                "ReportingAdjustments": [
                    {
                        "ReportOnUpdate": false,
                        "FirstReportingInterval": 15,
                        "MaxUploadLatency": 20000
                    }
                ],
                "HTTP": {
                    "URL": "https://stbrtl.r53.xcal.tv/",
                    "Compression": "None",
                    "Method": "POST",
                    "RequestURIParameter": [
                        {
                            "Name": "reportName",
                            "Reference": "Profile.Name"
                        }
                    ]
                },
                "JSONEncoding": {
                    "ReportFormat": "NameValuePair",
                    "ReportTimestamp": "None"
                }
            }
        }
    ]
}'''

#TR_AC12 ===> without hash value
#TR_AC14 ===> without version field
#TR_AC15 ===> without Protocol field
data_without_hashvalue = '''{
    "profiles": [
        {
            "name": "TR_AC12",
            "hash": "",
            "value": {
                "Name": "RDKB_Profile_12",
                "Description": "RDKB_Profile",
                "Version": "0.1",
                "Protocol": "HTTP",
                "EncodingType": "JSON",
                "ActivationTimeout": 1200,
                "ReportingInterval": 20,
                "GenerateNow": false,
                "RootName": "FR2_US_TC3",
                "TimeReference": "2023-01-25T13:47:00Z",
                "Parameter": [
                    {
                        "type": "dataModel",
                        "name": "UPTIME",
                        "reference": "Device.DeviceInfo.UpTime",
                        "use": "absolute"
                    },
                    {
                        "type": "event",
                        "eventName": "USED_MEM1_split",
                        "component": "sysint",
                        "use": "absolute"
                    },
                    {
                        "type": "grep",
                        "marker": "SYS_INFO_CrashPortalUpload_success",
                        "search": "Success uploading",
                        "logFile": "core_log.txt",
                        "use": "count"
                    }
                ],
                "ReportingAdjustments": [
                    {
                        "ReportOnUpdate": false,
                        "FirstReportingInterval": 15,
                        "MaxUploadLatency": 20000
                    }
                ],
                "HTTP": {
                    "URL": "https://stbrtl.r53.xcal.tv/",
                    "Compression": "None",
                    "Method": "POST",
                    "RequestURIParameter": [
                        {
                            "Name": "reportName",
                            "Reference": "Profile.Name"
                        }
                    ]
                },
                "JSONEncoding": {
                    "ReportFormat": "NameValuePair",
                    "ReportTimestamp": "None"
                }
            }
        },
        {
            "name": "TR_AC14",
            "hash": "Hash14",
            "value": {
                "Name": "RDKB_Profile_14",
                "Description": "RDKB_Profile",
                "Protocol": "HTTP",
                "EncodingType": "JSON",
                "ActivationTimeout": 1200,
                "ReportingInterval": 20,
                "GenerateNow": false,
                "RootName": "FR2_US_TC3",
                "TimeReference": "2023-01-25T13:47:00Z",
                "Parameter": [
                    {
                        "type": "dataModel",
                        "name": "UPTIME",
                        "reference": "Device.DeviceInfo.UpTime",
                        "use": "absolute"
                    },
                    {
                        "type": "event",
                        "eventName": "USED_MEM1_split",
                        "component": "sysint",
                        "use": "absolute"
                    },
                    {
                        "type": "grep",
                        "marker": "SYS_INFO_CrashPortalUpload_success",
                        "search": "Success uploading",
                        "logFile": "core_log.txt",
                        "use": "count"
                    }
                ],
                "ReportingAdjustments": [
                    {
                        "ReportOnUpdate": false,
                        "FirstReportingInterval": 15,
                        "MaxUploadLatency": 20000
                    }
                ],
                "HTTP": {
                    "URL": "https://stbrtl.r53.xcal.tv/",
                    "Compression": "None",
                    "Method": "POST",
                    "RequestURIParameter": [
                        {
                            "Name": "reportName",
                            "Reference": "Profile.Name"
                        }
                    ]
                },
                "JSONEncoding": {
                    "ReportFormat": "NameValuePair",
                    "ReportTimestamp": "None"
                }
            }
        },
        {
            "name": "TR_AC15",
            "hash": "Hash15",
            "value": {
                "Name": "RDKB_Profile_15",
                "Description": "RDKB_Profile",
                "Version": "0.1",
                "EncodingType": "JSON",
                "ActivationTimeout": 3600,
                "ReportingInterval": 35,
                "GenerateNow": false,
                "RootName": "FR2_US_TC3",
                "TimeReference": "2023-01-25T13:47:00Z",
                "Parameter": [
                    {
                        "type": "dataModel",
                        "name": "UPTIME",
                        "reference": "Device.DeviceInfo.UpTime",
                        "use": "absolute"
                    },
                    {
                        "type": "event",
                        "eventName": "USED_MEM2_split",
                        "component": "sysint",
                        "use": "absolute"
                    },
                    {
                        "type": "grep",
                        "marker": "SYS_INFO_CrashPortal_success",
                        "search": "Success uploading",
                        "logFile": "core_log.txt",
                        "use": "count"
                    }
                ],
                "ReportingAdjustments": [
                    {
                        "ReportOnUpdate": false,
                        "FirstReportingInterval": 15,
                        "MaxUploadLatency": 20000
                    }
                ],
                "HTTP": {
                    "URL": "https://stbrtl.r53.xcal.tv/",
                    "Compression": "None",
                    "Method": "POST",
                    "RequestURIParameter": [
                        {
                            "Name": "reportName",
                            "Reference": "Profile.Name"
                        }
                    ]
                },
                "JSONEncoding": {
                    "ReportFormat": "NameValuePair",
                    "ReportTimestamp": "None"
                }
            }
        }
    ]
}'''


#TR_AC16 ===> with incorrect Protocol
#TR_AC17 ===> without version value
#TR_AC13 ===> without Protocol value

data_with_wrong_protocol_value = '''{
    "profiles": [
        {
            "name": "TR_AC16",
            "hash": "Hash16",
            "value": {
                "Name": "RDKB_Profile_1",
                "Description": "RDKB_Profile",
                "Version": "0.1",
                "Protocol": "HTTPS",
                "EncodingType": "JSON",
                "ActivationTimeout": 1200,
                "ReportingInterval": 20,
                "GenerateNow": false,
                "RootName": "FR2_US_TC3",
                "TimeReference": "2023-01-25T13:47:00Z",
                "Parameter": [
                    {
                        "type": "dataModel",
                        "name": "UPTIME",
                        "reference": "Device.DeviceInfo.UpTime",
                        "use": "absolute"
                    },
                    {
                        "type": "event",
                        "eventName": "USED_MEM1_split",
                        "component": "sysint",
                        "use": "absolute"
                    },
                    {
                        "type": "grep",
                        "marker": "SYS_INFO_CrashPortalUpload_success",
                        "search": "Success uploading",
                        "logFile": "core_log.txt",
                        "use": "count"
                    }
                ],
                "ReportingAdjustments": [
                    {
                        "ReportOnUpdate": false,
                        "FirstReportingInterval": 15,
                        "MaxUploadLatency": 20000
                    }
                ],
                "HTTP": {
                    "URL": "https://stbrtl.r53.xcal.tv/",
                    "Compression": "None",
                    "Method": "POST",
                    "RequestURIParameter": [
                        {
                            "Name": "reportName",
                            "Reference": "Profile.Name"
                        }
                    ]
                },
                "JSONEncoding": {
                    "ReportFormat": "NameValuePair",
                    "ReportTimestamp": "None"
                }
            }
        },
        {
            "name": "TR_AC17",
            "hash": "Hash17",
            "value": {
                "Name": "RDKB_Profile_1",
                "Description": "RDKB_Profile",
                "Version": "",
                "Protocol": "HTTP",
                "EncodingType": "JSON",
                "ActivationTimeout": 1200,
                "ReportingInterval": 20,
                "GenerateNow": false,
                "RootName": "FR2_US_TC3",
                "TimeReference": "2023-01-25T13:47:00Z",
                "Parameter": [
                    {
                        "type": "dataModel",
                        "name": "UPTIME",
                        "reference": "Device.DeviceInfo.UpTime",
                        "use": "absolute"
                    },
                    {
                        "type": "event",
                        "eventName": "USED_MEM1_split",
                        "component": "sysint",
                        "use": "absolute"
                    },
                    {
                        "type": "grep",
                        "marker": "SYS_INFO_CrashPortalUpload_success",
                        "search": "Success uploading",
                        "logFile": "core_log.txt",
                        "use": "count"
                    }
                ],
                "ReportingAdjustments": [
                    {
                        "ReportOnUpdate": false,
                        "FirstReportingInterval": 15,
                        "MaxUploadLatency": 20000
                    }
                ],
                "HTTP": {
                    "URL": "https://stbrtl.r53.xcal.tv/",
                    "Compression": "None",
                    "Method": "POST",
                    "RequestURIParameter": [
                        {
                            "Name": "reportName",
                            "Reference": "Profile.Name"
                        }
                    ]
                },
                "JSONEncoding": {
                    "ReportFormat": "NameValuePair",
                    "ReportTimestamp": "None"
                }
            }
        },
        {
            "name": "TR_AC13",
            "hash": "Hash13",
            "value": {
                "Name": "RDKB_Profile_2",
                "Description": "RDKB_Profile",
                "Version": "0.1",
                "Protocol": "",
                "EncodingType": "JSON",
                "ActivationTimeout": 3600,
                "ReportingInterval": 35,
                "GenerateNow": false,
                "RootName": "FR2_US_TC3",
                "TimeReference": "2023-01-25T13:47:00Z",
                "Parameter": [
                    {
                        "type": "dataModel",
                        "name": "UPTIME",
                        "reference": "Device.DeviceInfo.UpTime",
                        "use": "absolute"
                    },
                    {
                        "type": "event",
                        "eventName": "USED_MEM2_split",
                        "component": "sysint",
                        "use": "absolute"
                    },
                    {
                        "type": "grep",
                        "marker": "SYS_INFO_CrashPortal_success",
                        "search": "Success uploading",
                        "logFile": "core_log.txt",
                        "use": "count"
                    }
                ],
                "ReportingAdjustments": [
                    {
                        "ReportOnUpdate": false,
                        "FirstReportingInterval": 15,
                        "MaxUploadLatency": 20000
                    }
                ],
                "HTTP": {
                    "URL": "https://stbrtl.r53.xcal.tv/",
                    "Compression": "None",
                    "Method": "POST",
                    "RequestURIParameter": [
                        {
                            "Name": "reportName",
                            "Reference": "Profile.Name"
                        }
                    ]
                },
                "JSONEncoding": {
                    "ReportFormat": "NameValuePair",
                    "ReportTimestamp": "None"
                }
            }
        }
    ]
}'''

# without EncodingType value
# without ActivationTimeout value
# without EncodingType field 
#without ActivationTimeout Field
#without reportingInterval Field
#without GeneratingNow Field

data_without_EncodingType_ActivationTimeout_values = '''{
    "profiles": [
        {
            "name": "TR_AC18",
            "hash": "Hash18",
            "value": {
                "Name": "RDKB_Profile_1",
                "Description": "RDKB_Profile",
                "Version": "0.1",
                "Protocol": "HTTP",
                "EncodingType": "",
                "ActivationTimeOut": 1200,
                "ReportingInterval": 20,
                "GenerateNow": false,
                "RootName": "FR2_US_TC3",
                "TimeReference": "2023-01-25T13:47:00Z",
                "Parameter": [
                    {
                        "type": "dataModel",
                        "name": "UPTIME",
                        "reference": "Device.DeviceInfo.UpTime",
                        "use": "absolute"
                    },
                    {
                        "type": "event",
                        "eventName": "USED_MEM1_split",
                        "component": "sysint",
                        "use": "absolute"
                    },
                    {
                        "type": "grep",
                        "marker": "SYS_INFO_CrashPortalUpload_success",
                        "search": "Success uploading",
                        "logFile": "core_log.txt",
                        "use": "count"
                    }
                ],
                "ReportingAdjustments": [
                    {
                        "ReportOnUpdate": false,
                        "FirstReportingInterval": 15,
                        "MaxUploadLatency": 20000
                    }
                ],
                "HTTP": {
                    "URL": "https://stbrtl.r53.xcal.tv/",
                    "Compression": "None",
                    "Method": "POST",
                    "RequestURIParameter": [
                        {
                            "Name": "reportName",
                            "Reference": "Profile.Name"
                        }
                    ]
                },
                "JSONEncoding": {
                    "ReportFormat": "NameValuePair",
                    "ReportTimestamp": "None"
                }
            }
        },
        {
            "name": "TR_AC19",
            "hash": "Hash19",
            "value": {
                "Name": "RDKB_Profile_2",
                "Description": "RDKB_Profile",
                "Version": "0.1",
                "Protocol": "HTTP",
                "EncodingType": "JSON",
                "ActivationTimeout": "",
                "ReportingInterval": 35,
                "GenerateNow": false,
                "RootName": "FR2_US_TC3",
                "TimeReference": "2023-01-25T13:47:00Z",
                "Parameter": [
                    {
                        "type": "dataModel",
                        "name": "UPTIME",
                        "reference": "Device.DeviceInfo.UpTime",
                        "use": "absolute"
                    },
                    {
                        "type": "event",
                        "eventName": "USED_MEM2_split",
                        "component": "sysint",
                        "use": "absolute"
                    },
                    {
                        "type": "grep",
                        "marker": "SYS_INFO_CrashPortal_success",
                        "search": "Success uploading",
                        "logFile": "core_log.txt",
                        "use": "count"
                    }
                ],
                "ReportingAdjustments": [
                    {
                        "ReportOnUpdate": false,
                        "FirstReportingInterval": 15,
                        "MaxUploadLatency": 20000
                    }
                ],
                "HTTP": {
                    "URL": "https://stbrtl.r53.xcal.tv/",
                    "Compression": "None",
                    "Method": "POST",
                    "RequestURIParameter": [
                        {
                            "Name": "reportName",
                            "Reference": "Profile.Name"
                        }
                    ]
                },
                "JSONEncoding": {
                    "ReportFormat": "NameValuePair",
                    "ReportTimestamp": "None"
                }
            }
        },
        {
            "name": "TR_AC20",
            "hash": "Hash20",
            "value": {
                "Name": "RDKB_Profile_1",
                "Description": "RDKB_Profile",
                "Version": "0.1",
                "Protocol": "HTTP",
                "ActivationTimeOut": 1200,
                "ReportingInterval": 20,
                "GenerateNow": false,
                "RootName": "FR2_US_TC3",
                "TimeReference": "2023-01-25T13:47:00Z",
                "Parameter": [
                    {
                        "type": "dataModel",
                        "name": "UPTIME",
                        "reference": "Device.DeviceInfo.UpTime",
                        "use": "absolute"
                    },
                    {
                        "type": "event",
                        "eventName": "USED_MEM1_split",
                        "component": "sysint",
                        "use": "absolute"
                    },
                    {
                        "type": "grep",
                        "marker": "SYS_INFO_CrashPortalUpload_success",
                        "search": "Success uploading",
                        "logFile": "core_log.txt",
                        "use": "count"
                    }
                ],
                "ReportingAdjustments": [
                    {
                        "ReportOnUpdate": false,
                        "FirstReportingInterval": 15,
                        "MaxUploadLatency": 20000
                    }
                ],
                "HTTP": {
                    "URL": "https://stbrtl.r53.xcal.tv/",
                    "Compression": "None",
                    "Method": "POST",
                    "RequestURIParameter": [
                        {
                            "Name": "reportName",
                            "Reference": "Profile.Name"
                        }
                    ]
                },
                "JSONEncoding": {
                    "ReportFormat": "NameValuePair",
                    "ReportTimestamp": "None"
                }
            }
        },
        {
            "name": "TR_AC21",
            "hash": "Hash21",
            "value": {
                "Name": "RDKB_Profile_2",
                "Description": "RDKB_Profile",
                "Version": "0.1",
                "Protocol": "HTTP",
                "EncodingType": "JSON",
                "ReportingInterval": 35,
                "GenerateNow": false,
                "RootName": "FR2_US_TC3",
                "TimeReference": "2023-01-25T13:47:00Z",
                "Parameter": [
                    {
                        "type": "dataModel",
                        "name": "UPTIME",
                        "reference": "Device.DeviceInfo.UpTime",
                        "use": "absolute"
                    },
                    {
                        "type": "event",
                        "eventName": "USED_MEM2_split",
                        "component": "sysint",
                        "use": "absolute"
                    },
                    {
                        "type": "grep",
                        "marker": "SYS_INFO_CrashPortal_success",
                        "search": "Success uploading",
                        "logFile": "core_log.txt",
                        "use": "count"
                    }
                ],
                "ReportingAdjustments": [
                    {
                        "ReportOnUpdate": false,
                        "FirstReportingInterval": 15,
                        "MaxUploadLatency": 20000
                    }
                ],
                "HTTP": {
                    "URL": "https://stbrtl.r53.xcal.tv/",
                    "Compression": "None",
                    "Method": "POST",
                    "RequestURIParameter": [
                        {
                            "Name": "reportName",
                            "Reference": "Profile.Name"
                        }
                    ]
                },
                "JSONEncoding": {
                    "ReportFormat": "NameValuePair",
                    "ReportTimestamp": "None"
                }
            }
        },
        {
            "name": "TR_AC22",
            "hash": "Hash22",
            "value": {
                "Name": "RDKB_Profile_1",
                "Description": "RDKB_Profile",
                "Version": "0.1",
                "Protocol": "HTTP",
                "EncodingType": "JSON",
                "ActivationTimeout": 1200,
                "GenerateNow": false,
                "RootName": "FR2_US_TC3",
                "TimeReference": "2023-01-25T13:47:00Z",
                "Parameter": [
                    {
                        "type": "dataModel",
                        "name": "UPTIME",
                        "reference": "Device.DeviceInfo.UpTime",
                        "use": "absolute"
                    },
                    {
                        "type": "event",
                        "eventName": "USED_MEM1_split",
                        "component": "sysint",
                        "use": "absolute"
                    },
                    {
                        "type": "grep",
                        "marker": "SYS_INFO_CrashPortalUpload_success",
                        "search": "Success uploading",
                        "logFile": "core_log.txt",
                        "use": "count"
                    }
                ],
                "ReportingAdjustments": [
                    {
                        "ReportOnUpdate": false,
                        "FirstReportingInterval": 15,
                        "MaxUploadLatency": 20000
                    }
                ],
                "HTTP": {
                    "URL": "https://stbrtl.r53.xcal.tv/",
                    "Compression": "None",
                    "Method": "POST",
                    "RequestURIParameter": [
                        {
                            "Name": "reportName",
                            "Reference": "Profile.Name"
                        }
                    ]
                },
                "JSONEncoding": {
                    "ReportFormat": "NameValuePair",
                    "ReportTimestamp": "None"
                }
            }
        },
        {
            "name": "TR_AC23",
            "hash": "Hash23",
            "value": {
                "Name": "RDKB_Profile_3",
                "Description": "RDKB_Profile",
                "Version": "0.1",
                "Protocol": "HTTP",
                "EncodingType": "JSON",
                "ActivationTimeout": 3600,
                "ReportingInterval": 45,
                "RootName": "FR2_US_TC3",
                "TimeReference": "2023-01-25T13:47:00Z",
                "Parameter": [
                    {
                        "type": "dataModel",
                        "name": "UPTIME",
                        "reference": "Device.DeviceInfo.UpTime",
                        "use": "absolute"
                    },
                    {
                        "type": "event",
                        "eventName": "USED_MEM1_split",
                        "component": "sysint",
                        "use": "count"
                    },
                    {
                        "type": "grep",
                        "marker": "SYS_INFO_CrashPortalUpload_success",
                        "search": "Success uploading",
                        "logFile": "core_log.txt",
                        "use": "count"
                    }
                ],
                "ReportingAdjustments": [
                    {
                        "ReportOnUpdate": false,
                        "FirstReportingInterval": 15,
                        "MaxUploadLatency": 20000
                    }
                ],
                "HTTP": {
                    "URL": "https://stbrtl.r53.xcal.tv/",
                    "Compression": "None",
                    "Method": "POST",
                    "RequestURIParameter": [
                        {
                            "Name": "reportName",
                            "Reference": "Profile.Name"
                        }
                    ]
                },
                "JSONEncoding": {
                    "ReportFormat": "NameValuePair",
                    "ReportTimestamp": "None"
                }
            }
        }
    ]
}'''

data_with_reporting_interval = '''{ "profiles": [ { "name": "TR_AC222", "hash": "Hash222", "value": { "Name": "RDKB_Profile_3", "Description": "RDKB_Profile", "Version": "0.1", "Protocol": "HTTP", "EncodingType": "JSON", "ActivationTimeout": 3600, "ReportingInterval": 45, "GenerateNow": false, "RootName": "FR2_US_TC3", "TimeReference": "2023-01-25T13:47:00Z", "Parameter": [ { "type": "dataModel", "name": "UPTIME", "reference": "Device.DeviceInfo.UpTime", "use": "absolute" }, { "type": "event", "eventName": "TEST_EVENT_MARKER_1", "component": "sysint", "use": "count" }, { "type": "grep", "marker": "SYS_INFO_CrashPortalUpload_success", "search": "Success uploading", "logFile": "core_log.txt", "use": "count" } ], "ReportingAdjustments": [ { "ReportOnUpdate": false, "FirstReportingInterval": 15, "MaxUploadLatency": 20000 } ], "HTTP": { "URL": "https://stbrtl.r53.xcal.tv/", "Compression": "None", "Method": "POST", "RequestURIParameter": [ { "Name": "reportName", "Reference": "Profile.Name" } ] }, "JSONEncoding": { "ReportFormat": "NameValuePair", "ReportTimestamp": "None" } } } ] }'''

data_with_activation_timeout = '''{
    "profiles": [
        {
            "name": "TR_AC77",
            "hash": "Hash77",
            "value": {
                "Name": "RDKB_Profile_1",
                "Description": "RDKB_Profile",
                "Version": "0.1",
                "Protocol": "HTTP",
                "EncodingType": "JSON",
                "ActivationTimeOut": 50,
                "ReportingInterval": 20,
                "GenerateNow": true,
                "RootName": "FR2_US_TC3",
                "Parameter": [
                    {
                        "type": "dataModel",
                        "name": "MODEL_NAME",
                        "reference": "Device.DeviceInfo.ModelName",
                        "use": "absolute"
                    },
                    {
                        "type": "event",
                        "eventName": "USED_MEM1_split",
                        "component": "sysint",
                        "use": "absolute"
                    },
                    {
                        "type": "grep",
                        "marker": "SYS_INFO_CrashPortalUpload_success",
                        "search": "Success uploading",
                        "logFile": "core_log.txt",
                        "use": "count"
                    }
                ],
                "ReportingAdjustments": [
                    {
                        "ReportOnUpdate": false,
                        "FirstReportingInterval": 15,
                        "MaxUploadLatency": 20000
                    }
                ],
                "HTTP": {
                    "URL": "https://stbrtl.r53.xcal.tv/",
                    "Compression": "None",
                    "Method": "POST",
                    "RequestURIParameter": [
                        {
                            "Name": "reportName",
                            "Reference": "Profile.Name"
                        }
                    ]
                },
                "JSONEncoding": {
                    "ReportFormat": "NameValuePair",
                    "ReportTimestamp": "None"
                }
            }
        }
    ]
}'''

data_with_full_log_path = '''{
    "profiles": [
        {
            "name": "TR_AC1000",
            "hash": "Hash10000",
            "value": {
                "Name": "RDKB_Profile_3",
                "Description": "RDKB_Profile",
                "Version": "0.1",
                "Protocol": "HTTP",
                "EncodingType": "JSON",
                "ActivationTimeout": 3600,
                "ReportingInterval": 45,
                "GenerateNow": true,
                "RootName": "FR2_US_TC3",
                "TimeReference": "2023-01-25T13:47:00Z",
                "Parameter": [
                    {
                        "type": "dataModel",
                        "name": "UPTIME",
                        "reference": "Device.DeviceInfo.UpTime",
                        "use": "absolute"
                    },
                    {
                        "type": "event",
                        "eventName": "TEST_EVENT_MARKER_1",
                        "component": "sysint",
                        "use": "count"
                    },
                    {
                        "type": "grep",
                        "marker": "SYS_INFO_CrashPortalUpload_success",
                        "search": "Success uploading",
                        "logFile": "core_log.txt",
                        "use": "count"
                    },
                    {
                        "type": "grep",
                        "marker": "SYS_INFO_CrashPortalUpload_success_2",
                        "search": "random",
                        "logFile": "/opt/logs/core_log.txt",
                        "use": "count"
                    }
                ],
                "ReportingAdjustments": [
                    {
                        "ReportOnUpdate": false,
                        "FirstReportingInterval": 15,
                        "MaxUploadLatency": 20000
                    }
                ],
                "HTTP": {
                    "URL": "https://stbrtl.r53.xcal.tv/",
                    "Compression": "None",
                    "Method": "POST",
                    "RequestURIParameter": [
                        {
                            "Name": "reportName",
                            "Reference": "Profile.Name"
                        }
                    ]
                },
                "JSONEncoding": {
                    "ReportFormat": "NameValuePair",
                    "ReportTimestamp": "None"
                }
            }
        }
    ]
}'''

data_with_less_activation_timeout = '''{
    "profiles": [
        {
            "name": "TR_AC88",
            "hash": "Hash88",
            "value": {
                "Name": "RDKB_Profile_1",
                "Description": "RDKB_Profile",
                "Version": "0.1",
                "Protocol": "HTTP",
                "EncodingType": "JSON",
                "ActivationTimeOut": 50,
                "DeleteOnTimeOut": true,
                "ReportingInterval": 200,
                "GenerateNow": false,
                "RootName": "FR2_US_TC3",
                "TimeReference": "2023-01-25T13:47:00Z",
                "Parameter": [
                    {
                        "type": "dataModel",
                        "name": "UPTIME",
                        "reference": "Device.DeviceInfo.UpTime",
                        "use": "absolute"
                    },
                    {
                        "type": "event",
                        "eventName": "USED_MEM1_split",
                        "component": "sysint",
                        "use": "absolute"
                    },
                    {
                        "type": "grep",
                        "marker": "SYS_INFO_CrashPortalUpload_success",
                        "search": "Success uploading",
                        "logFile": "core_log.txt",
                        "use": "count"
                    }
                ],
                "ReportingAdjustments": [
                    {
                        "ReportOnUpdate": false,
                        "FirstReportingInterval": 15,
                        "MaxUploadLatency": 20000
                    }
                ],
                "HTTP": {
                    "URL": "https://stbrtl.r53.xcal.tv/",
                    "Compression": "None",
                    "Method": "POST",
                    "RequestURIParameter": [
                        {
                            "Name": "reportName",
                            "Reference": "Profile.Name"
                        }
                    ]
                },
                "JSONEncoding": {
                    "ReportFormat": "NameValuePair",
                    "ReportTimestamp": "None"
                }
            }
        }
    ]
}'''

data_with_delete_on_timeout = '''{
    "profiles": [
        {
            "name": "TR_AC66",
            "hash": "Hash66",
            "value": {
                "Name": "RDKB_Profile_1",
                "Description": "RDKB_Profile",
                "Version": "0.1",
                "Protocol": "HTTP",
                "EncodingType": "JSON",
                "ActivationTimeOut": 50,
                "DeleteOnTimeout": true,
                "ReportingInterval": 20,
                "GenerateNow": false,
                "RootName": "FR2_US_TC3",
                "TimeReference": "2023-01-25T13:47:00Z",
                "Parameter": [
                    {
                        "type": "dataModel",
                        "name": "UPTIME",
                        "reference": "Device.DeviceInfo.UpTime",
                        "use": "absolute"
                    },
                    {
                        "type": "event",
                        "eventName": "USED_MEM1_split",
                        "component": "sysint",
                        "use": "absolute"
                    },
                    {
                        "type": "grep",
                        "marker": "SYS_INFO_CrashPortalUpload_success",
                        "search": "Success uploading",
                        "logFile": "core_log.txt",
                        "use": "count"
                    }
                ],
                "ReportingAdjustments": [
                    {
                        "ReportOnUpdate": false,
                        "FirstReportingInterval": 15,
                        "MaxUploadLatency": 20000
                    }
                ],
                "HTTP": {
                    "URL": "https://stbrtl.r53.xcal.tv/",
                    "Compression": "None",
                    "Method": "POST",
                    "RequestURIParameter": [
                        {
                            "Name": "reportName",
                            "Reference": "Profile.Name"
                        }
                    ]
                },
                "JSONEncoding": {
                    "ReportFormat": "NameValuePair",
                    "ReportTimestamp": "None"
                }
            }
        }
    ]
}'''
