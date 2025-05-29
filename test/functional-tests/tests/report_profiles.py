
data_empty_profile = '''{
    "profiles": [{}]
    }'''

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
                    "URL": "https://mockxconf:50051/dataLakeMock/",
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
                    "URL": "https://mockxconf:50051/dataLakeMock/",
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
                    "URL": "https://mockxconf:50051/dataLakeMock/",
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
                    "URL": "https://mockxconf:50051/dataLakeMock/",
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
                        "type": "event",
                        "eventName": "USED_MEM3_split",
                        "component": "sysint",
                        "use": "absolute"
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
                    "URL": "https://mockxconf:50051/dataLakeMock/",
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
                        "type": "grep",
                        "marker": "SYS_INFO_CCrashPortal_success",
                        "search": "Success upliooading",
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
                    "URL": "https://mockxconf:50051/dataLakeMock/",
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
                    "URL": "https://mockxconf:50051/dataLakeMock/",
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
                "ReportingInterval": 10,
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
                    "URL": "https://mockxconf:50051/dataLakeMock/",
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
                    "URL": "https://mockxconf:50051/dataLakeMock/",
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
                    "URL": "https://mockxconf:50051/dataLakeMock/",
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
                "ReportingInterval": 15,
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
                    "URL": "https://mockxconf:50051/dataLakeMock/",
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
                    "URL": "https://mockxconf:50051/dataLakeMock/",
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
                    "URL": "https://mockxconf:50051/dataLakeMock/",
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
                    "URL": "https://mockxconf:50051/dataLakeMock/",
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
                "ReportingInterval": 30,
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
                    "URL": "https://mockxconf:50051/dataLakeMock/",
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

data_with_reporting_interval = '''{
    "profiles": [
        {
            "name": "TR_AC732",
            "hash": "Hash732",
            "value": {
                "Name": "RDKB_Profile_3",
                "Description": "RDKB_Profile",
                "Version": "0.1",
                "Protocol": "HTTP",
                "EncodingType": "JSON",
                "ActivationTimeout": 3600,
                "ReportingInterval": 20,
                "GenerateNow": false,
                "RootName": "FR2_US_TC3",
                "Parameter": [
                    {
                        "type": "event",
                        "eventName": "TEST_EVENT_MARKER_1",
                        "component": "sysint",
                        "use": "count"
                    },
                    {
                        "type": "event",
                        "eventName": "TEST_EVENT_MARKER_2",
                        "component": "sysint",
                        "use": "accumulate"
                    },
                    {
                        "type": "grep",
                        "marker": "SYS_INFO_CrashPortalUpload_success",
                        "search": "Success loading",
                        "logFile": "core_log.txt",
                        "use": "count",
                        "reportEmpty":true
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
                    "URL": "https://mockxconf:50051/dataLakeMock/",
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

data_with_Generate_Now = '''{
    "profiles": [
        {
            "name": "TR_AC767",
            "hash": "Hash767",
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
                        "use": "absolute",
                        "reportEmpty":true
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
                        "marker": "FILE_Upload_Progress",
                        "search": "file uploading",
                        "logFile": "core_log.txt",
                        "use": "absolute"
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
                    "URL": "https://mockxconf:50051/dataLakeMock/",
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
            "name": "TR_AC777",
            "hash": "Hash777",
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
                        "type": "grep",
                        "marker": "FILE_Read_Progress",
                        "search": "file reading",
                        "logFile": "core_log.txt",
                        "use": "absolute",
                        "trim":true
                    },
                    {
                        "type": "grep",
                        "marker": "FILE_Write_Progress",
                        "search": "file writing",
                        "logFile": "core_log.txt",
                        "use": "accumulate"
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
                    "URL": "https://mockxconf:50051/dataLakeMock/",
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
                    "URL": "https://mockxconf:50051/dataLakeMock/",
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
                    "URL": "https://mockxconf:50051/dataLakeMock/",
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
            "name": "TR_AC6919",
            "hash": "Hash6919",
            "value": {
                "Name": "RDKB_Profile_1",
                "Description": "RDKB_Profile",
                "Version": "0.1",
                "Protocol": "HTTP",
                "EncodingType": "JSON",
                "ActivationTimeOut": 20,
                "DeleteOnTimeout": true,
                "ReportingInterval": 10,
                "GenerateNow": false,
                "RootName": "FR2_US_TC3",
                "Parameter": [
                    {
                        "type": "dataModel",
                        "name": "MODEL_NAME",
                        "reference": "Device.DeviceInfo.ModelName11",
                        "use": "absolute",
                        "reportEmpty":true
                    }
                ],
                "ReportingAdjustments":
                    {
                        "ReportOnUpdate": false,
                        "FirstReportingInterval": 5,
                        "MaxUploadLatency": 5
                    },
                "HTTP": {
                    "URL": "https://mockxconf:50051/dataLakeMock/",
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
                "ActivationTimeOut": 20,
                "DeleteOnTimeout": true,
                "ReportingInterval": 10,
                "GenerateNow": false,
                "RootName": "FR2_US_TC3",
                "TimeReference": "2023-01-25T13:47:00Z",
                "Parameter": [
                    {
                        "type": "dataModel",
                        "name": "MODEL_NAME",
                        "reference": "Device.DeviceInfo.ModelName",
                        "use": "absolute",
                        "regex":"[A-Z]+"
                    },
                    {
                        "type": "event",
                        "eventName": "TEST_EVENT_MARKER_2",
                        "component": "sysint",
                        "use": "absolute",
                        "regex":"[0-9]+"
                    },
                    {
                        "type": "grep",
                        "marker": "SYS_INFO_CrashPortalUpload_success",
                        "search": "Success uploading",
                        "logFile": "core_log.txt",
                        "use": "absolute",
                        "regex":"[0-9]+"
                    }
                ],
                "ReportingAdjustments": 
                    {
                        "ReportOnUpdate": true,
                        "FirstReportingInterval": 5,
                        "MaxUploadLatency": 10
                    },
                "HTTP": {
                    "URL": "https://mockxconf:50051/dataLakeMock/",
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
#json test cases
#profile with timeref and first reporting Interval shouldn't be taken
data_with_first_reporting_interval_neg = '''{
    "profiles": [
        {
            "name": "NA_FRI",
            "hash": "Hash89",
            "value": {
                "Name": "RDKB_FRI_Profile",
                "Description": "RDKB_Profile",
                "Version": "0.1",
                "Protocol": "HTTP",
                "EncodingType": "JSON",
                "ActivationTimeOut": 30,
                "DeleteOnTimeout": true,
                "ReportingInterval": 10,
                "GenerateNow": false,
                "RootName": "FR2_US_TC3",
                "TimeReference": "2023-01-25T13:47:00Z",
                "Parameter": [
                    {
                        "type": "dataModel",
                        "name": "MODEL_NAME",
                        "reference": "Device.DeviceInfo.ModelName",
                        "use": "absolute",
                        "regex":"[A-Z]+"
                    },
                    {
                        "type": "event",
                        "eventName": "TEST_EVENT_MARKER_2",
                        "component": "sysint",
                        "use": "absolute",
                        "regex":"[0-9]+"
                    },
                    {
                        "type": "grep",
                        "marker": "SYS_INFO_CrashPortalUpload_success",
                        "search": "Success uploading",
                        "logFile": "core_log.txt",
                        "use": "absolute",
                        "regex":"[0-9]+"
                    }
                ],
                "ReportingAdjustments": 
                    {
                        "ReportOnUpdate": true,
                        "FirstReportingInterval": 5,
                        "MaxUploadLatency": 10
                    },
                "HTTP": {
                    "URL": "https://mockxconf:50051/dataLakeMock/",
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

    ]
}'''

#profile with timeref default and max reporting interval shouldn't be taken
data_with_maxuploadlatency_neg = '''{
    "profiles": [
        {
            "name": "NA_MLU",
            "hash": "Hash90",
            "value": {
                "Name": "RDKB_ML_Profile",
                "Description": "RDKB_Profile",
                "Version": "0.1",
                "Protocol": "HTTP",
                "EncodingType": "JSON",
                "ActivationTimeOut": 20,
                "DeleteOnTimeout": true,
                "ReportingInterval": 10,
                "GenerateNow": false,
                "RootName": "FR2_US_TC3",
                "TimeReference": "0001-01-01T00:00:00Z",
                "Parameter": [
                    {
                        "type": "dataModel",
                        "name": "MODEL_NAME",
                        "reference": "Device.DeviceInfo.ModelName",
                        "use": "absolute",
                        "regex":"[A-Z]+"
                    },
                    {
                        "type": "event",
                        "eventName": "TEST_EVENT_MARKER_2",
                        "component": "sysint",
                        "use": "absolute",
                        "regex":"[0-9]+"
                    },
                    {
                        "type": "grep",
                        "marker": "SYS_INFO_CrashPortalUpload_success",
                        "search": "Success uploading",
                        "logFile": "core_log.txt",
                        "use": "absolute",
                        "regex":"[0-9]+"
                    }
                ],
                "ReportingAdjustments":
                    {
                        "ReportOnUpdate": true,
                        "FirstReportingInterval": 5,
                        "MaxUploadLatency": 10
                    },
                "HTTP": {
                    "URL": "https://mockxconf:50051/dataLakeMock/",
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

data_with_first_reporting_interval_max_latency = '''{
    "profiles": [
        {
            "name": "NA_FRI",
            "hash": "Hash89",
            "value": {
                "Name": "RDKB_FRI_Profile",
                "Description": "RDKB_Profile",
                "Version": "0.1",
                "Protocol": "HTTP",
                "EncodingType": "JSON",
                "ActivationTimeOut": 30,
                "DeleteOnTimeout": true,
                "ReportingInterval": 10,
                "GenerateNow": false,
                "RootName": "FR2_US_TC3",
                "TimeReference": "2023-01-25T13:47:00Z",
                "Parameter": [
                    {
                        "type": "dataModel",
                        "name": "MODEL_NAME",
                        "reference": "Device.DeviceInfo.ModelName",
                        "use": "absolute",
                        "regex":"[A-Z]+"
                    },
                    {
                        "type": "event",
                        "eventName": "TEST_EVENT_MARKER_2",
                        "component": "sysint",
                        "use": "absolute",
                        "regex":"[0-9]+"
                    },
                    {
                        "type": "grep",
                        "marker": "SYS_INFO_CrashPortalUpload_success",
                        "search": "Success uploading",
                        "logFile": "core_log.txt",
                        "use": "absolute",
                        "regex":"[0-9]+"
                    }
                ],
                "ReportingAdjustments":
                    {
                        "ReportOnUpdate": true,
                        "FirstReportingInterval": 5,
                        "MaxUploadLatency": 10
                    },
                "HTTP": {
                    "URL": "https://mockxconf:50051/dataLakeMock/",
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
            "name": "NA_MLU",
            "hash": "Hash90",
            "value": {
                "Name": "RDKB_ML_Profile",
                "Description": "RDKB_Profile",
                "Version": "0.1",
                "Protocol": "HTTP",
                "EncodingType": "JSON",
                "ActivationTimeOut": 20,
                "DeleteOnTimeout": true,
                "ReportingInterval": 10,
                "GenerateNow": false,
                "RootName": "FR2_US_TC3",
                "TimeReference": "0001-01-01T00:00:00Z",
                "Parameter": [
                    {
                        "type": "dataModel",
                        "name": "MODEL_NAME",
                        "reference": "Device.DeviceInfo.ModelName",
                        "use": "absolute",
                        "regex":"[A-Z]+"
                    },
                    {
                        "type": "event",
                        "eventName": "TEST_EVENT_MARKER_2",
                        "component": "sysint",
                        "use": "absolute",
                        "regex":"[0-9]+"
                    },
                    {
                        "type": "grep",
                        "marker": "SYS_INFO_CrashPortalUpload_success",
                        "search": "Success uploading",
                        "logFile": "core_log.txt",
                        "use": "absolute",
                        "regex":"[0-9]+"
                    }
                ],
                "ReportingAdjustments":
                    {
                        "ReportOnUpdate": true,
                        "FirstReportingInterval": 5,
                        "MaxUploadLatency": 10
                    },
                "HTTP": {
                    "URL": "https://mockxconf:50051/dataLakeMock/",
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
            "name": "NA_MLU_GT_RI",
            "hash": "Hash91",
            "value": {
                "Name": "RDKB_ML_Profile",
                "Description": "RDKB_Profile",
                "Version": "0.1",
                "Protocol": "HTTP",
                "EncodingType": "JSON",
                "ActivationTimeOut": 20,
                "DeleteOnTimeout": true,
                "ReportingInterval": 10,
                "GenerateNow": false,
                "RootName": "FR2_US_TC3",
                "TimeReference": "2023-01-25T13:47:00Z",
                "Parameter": [
                    {
                        "type": "dataModel",
                        "name": "MODEL_NAME",
                        "reference": "Device.DeviceInfo.ModelName",
                        "use": "absolute",
                        "regex":"[A-Z]+"
                    },
                    {
                        "type": "event",
                        "eventName": "TEST_EVENT_MARKER_2",
                        "component": "sysint",
                        "use": "absolute",
                        "regex":"[0-9]+"
                    },
                    {
                        "type": "grep",
                        "marker": "SYS_INFO_CrashPortalUpload_success",
                        "search": "Success uploading",
                        "logFile": "core_log.txt",
                        "use": "absolute",
                        "regex":"[0-9]+"
                    }
                ],
                "ReportingAdjustments":
                    {
                        "ReportOnUpdate": true,
                        "FirstReportingInterval": 5,
                        "MaxUploadLatency": 20000
                    },
                "HTTP": {
                    "URL": "https://mockxconf:50051/dataLakeMock/",
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
            "name": "PARAM_NULL",
            "hash": "Hash90",
            "value": {
                "Name": "RDKB_ML_Profile",
                "Description": "RDKB_Profile",
                "Version": "0.1",
                "Protocol": "HTTP",
                "EncodingType": "JSON",
                "ActivationTimeOut": 20,
                "DeleteOnTimeout": true,
                "ReportingInterval": 10,
                "GenerateNow": false,
                "RootName": "FR2_US_TC3",
                "TimeReference": "0001-01-01T00:00:00Z",
                "Parameter": []
                }
        }
    ]
}'''

#profile with timeref default and max reporting interval shouldn't be taken
data_with_triggerconditon_neg = '''{
    "profiles": [
     {
      "name": "TC_TYPE_NULL",
      "hash": "Hash_Default",
      "value": {
        "Description": "Trigger condition type NULL case",
        "Version": ".01",
        "Protocol": "RBUS_METHOD",
        "EncodingType": "JSON",
        "GenerateNow": false,
        "ReportingInterval": 10,
        "ActivationTimeOut": 360,
        "TimeReference": "0001-01-01T00:00:00Z",
        "Parameter": [],
        "TriggerCondition": [
          {
            "reference": "Device.X_RDK_WanManager.InterfaceAvailableStatus"
          },
          {
            "type": "dataModel",
            "reference": "Device.X_RDK_WanManager.InterfaceActiveStatus"
          },
          {
            "type": "dataModel",
            "reference": "Device.X_RDK_Remote.Device.2.Status"
          }
        ],
        "RBUS_METHOD": {
          "Method": "Device.X_RDK_Xmidt.SendData",
          "Parameters": [
            {
              "name": "msg_type",
              "value": "event"
            },
            {
              "name": "source",
              "value": "telemetry2"
            },
            {
              "name": "dest",
              "value": "event:/profile-report/LTE-report"
            },
            {
              "name": "content_type",
              "value": "application/json"
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
      "name": "TC_REF_NULL",
      "hash": "Hash_Default",
      "value": {
        "Description": "Trigger Condition Reference NULL case",
        "Version": ".01",
        "Protocol": "RBUS_METHOD",
        "EncodingType": "JSON",
        "GenerateNow": false,
        "ReportingInterval": 10,
        "ActivationTimeOut": 360,
        "TimeReference": "0001-01-01T00:00:00Z",
        "Parameter": [],
        "TriggerCondition": [
          {
            "type": "dataModel",
            "operator": "any"
          },
          {
            "type": "dataModel",
            "reference": "Device.X_RDK_WanManager.InterfaceActiveStatus"
          },
          {
            "type": "dataModel",
            "reference": "Device.X_RDK_Remote.Device.2.Status"
          }
        ],
        "RBUS_METHOD": {
          "Method": "Device.X_RDK_Xmidt.SendData",
          "Parameters": [
            {
              "name": "msg_type",
              "value": "event"
            },
            {
              "name": "source",
              "value": "telemetry2"
            },
            {
              "name": "dest",
              "value": "event:/profile-report/LTE-report"
            },
            {
              "name": "content_type",
              "value": "application/json"
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
      "name": "TC_TYPE_NOT_DETAMODEL",
      "hash": "Hash_Default",
      "value": {
        "Description": "Trigger Condition type is not datamodel",
        "Version": ".01",
        "Protocol": "RBUS_METHOD",
        "EncodingType": "JSON",
        "GenerateNow": false,
        "ReportingInterval": 240,
        "ActivationTimeOut": 360,
        "TimeReference": "0001-01-01T00:00:00Z",
        "Parameter": [],
        "TriggerCondition": [
          {
            "type": "grep",
            "reference": "Device.X_RDK_WanManager.InterfaceAvailableStatus"
          },
          {
            "type": "dataModel",
            "reference": "Device.X_RDK_WanManager.InterfaceActiveStatus"
          }
        ],
        "RBUS_METHOD": {
          "Method": "Device.X_RDK_Xmidt.SendData",
          "Parameters": [
            {
              "name": "msg_type",
              "value": "event"
            },
            {
              "name": "source",
              "value": "telemetry2"
            },
            {
              "name": "dest",
              "value": "event:/profile-report/LTE-report"
            },
            {
              "name": "content_type",
              "value": "application/json"
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
      "name": "TC_OPERATOR_NULL",
      "hash": "Hash_Default",
      "value": {
        "Description": "Trigger condition operator is NULL",
        "Version": ".01",
        "Protocol": "RBUS_METHOD",
        "EncodingType": "JSON",
        "GenerateNow": false,
        "ReportingInterval": 25,
        "ActivationTimeOut": 360,
        "TimeReference": "0001-01-01T00:00:00Z",
        "Parameter": [],
        "TriggerCondition": [
          {
            "type": "dataModel",
            "reference": "Device.X_RDK_WanManager.InterfaceAvailableStatus"
          },
          {
            "type": "dataModel",
            "reference": "Device.X_RDK_WanManager.InterfaceActiveStatus"
          }
        ],
        "RBUS_METHOD": {
          "Method": "Device.X_RDK_Xmidt.SendData",
          "Parameters": [
            {
              "name": "msg_type",
              "value": "event"
            },
            {
              "name": "source",
              "value": "telemetry2"
            },
            {
              "name": "dest",
              "value": "event:/profile-report/LTE-report"
            },
            {
              "name": "content_type",
              "value": "application/json"
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
      "name": "TC_OPERATOR_WRONG",
      "hash": "Hash_Default",
      "value": {
        "Description": "Trigger Condition operator invalid case",
        "Version": ".01",
        "Protocol": "RBUS_METHOD",
        "EncodingType": "JSON",
        "GenerateNow": false,
        "ReportingInterval": 10,
        "ActivationTimeOut": 360,
        "TimeReference": "0001-01-01T00:00:00Z",
        "Parameter": [],
        "TriggerCondition": [
          {
            "type": "dataModel",
            "reference": "Device.X_RDK_WanManager.InterfaceAvailableStatus",
            "operator": "less"
          },
          {
            "type": "dataModel",
            "reference": "Device.X_RDK_WanManager.InterfaceActiveStatus"
          }
        ],
        "RBUS_METHOD": {
          "Method": "Device.X_RDK_Xmidt.SendData",
          "Parameters": [
            {
              "name": "msg_type",
              "value": "event"
            },
            {
              "name": "source",
              "value": "telemetry2"
            },
            {
              "name": "dest",
              "value": "event:/profile-report/LTE-report"
            },
            {
              "name": "content_type",
              "value": "application/json"
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
      "name": "TC_OPERATOR_THRESHOLD",
      "hash": "Hash_Default",
      "value": {
        "Description": "Trigger Condition threshold is NULL",
        "Version": ".01",
        "Protocol": "RBUS_METHOD",
        "EncodingType": "JSON",
        "GenerateNow": false,
        "ActivationTimeOut": 360,
        "TimeReference": "0001-01-01T00:00:00Z",
        "Parameter": [],
        "TriggerCondition": [
          {
            "type": "dataModel",
            "reference": "Device.X_RDK_WanManager.InterfaceAvailableStatus",
            "operator": "any"
          },
          {
            "type": "dataModel",
            "reference": "Device.X_RDK_WanManager.InterfaceActiveStatus",
            "operator": "gt"
          },
          {
            "type": "dataModel",
            "reference": "Device.X_RDK_GatewayManagement.Gateway.1.ActiveStatus"
          }
        ],
        "RBUS_METHOD": {
          "Method": "Device.X_RDK_Xmidt.SendData",
          "Parameters": [
            {
              "name": "msg_type",
              "value": "event"
            },
            {
              "name": "source",
              "value": "telemetry2"
            },
            {
              "name": "dest",
              "value": "event:/profile-report/LTE-report"
            },
            {
              "name": "content_type",
              "value": "application/json"
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
      "name": "TC_REFERENCE_WRONG",
      "hash": "Hash_Default",
      "value": {
        "Description": "Trigger Condition threshold is NULL",
        "Version": ".01",
        "Protocol": "RBUS_METHOD",
        "EncodingType": "JSON",
        "GenerateNow": false,
        "ActivationTimeOut": 360,
        "TimeReference": "0001-01-01T00:00:00Z",
        "Parameter": [],
        "TriggerCondition": [
          {
            "type": "dataModel",
            "reference": "",
            "operator": "any"
          },
          {
            "type": "dataModel",
            "reference": "Device.X_RDK_WanManager.InterfaceActiveStatus",
            "operator": "gt"
          },
          {
            "type": "dataModel",
            "reference": "Device.X_RDK_GatewayManagement.Gateway.1.ActiveStatus"
          }
        ],
        "RBUS_METHOD": {
          "Method": "Device.X_RDK_Xmidt.SendData",
            "Parameters": [
            {
              "name": "msg_type",
              "value": "event"
            },
            {
              "name": "source",
              "value": "telemetry2"
            },
            {
              "name": "dest",
              "value": "event:/profile-report/LTE-report"
            },
            {
              "name": "content_type",
              "value": "application/json"
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

data_with_triggerconditon_pos = '''{
    "profiles": [
     {
      "name": "TC_pos",
      "hash": "Hash_default",
      "value": {
        "Description": "Trigger condition working case",
        "Version": ".01",
        "Protocol": "RBUS_METHOD",
        "EncodingType": "JSON",
        "GenerateNow": false,
        "TimeReference": "0001-01-01T00:00:00Z",
        "ActivationTimeout": 120,
        "Parameter": [],
        "TriggerCondition": [
          {
            "type": "dataModel",
            "reference": "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.RDKRemoteDebugger.Enable",
            "operator": "any"
          }
        ],
        "RBUS_METHOD": {
         "Method": "Device.X_RDK_Xmidt.SendData",
          "Parameters": [
            {
              "name": "msg_type",
              "value": "event"
            },
            {
              "name": "source",
              "value": "telemetry2"
            },
            {
              "name": "dest",
              "value": "event:/profile-report/LTE-report"
            },
            {
              "name": "content_type",
              "value": "application/json"
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
