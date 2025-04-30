
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
                "ReportingInterval": 45,
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

