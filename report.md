# Telemetry Marker Inventory
**Branch**: develop
**Organizations**: rdkcentral
**Generated**: 2026-03-16 06:31:31 UTC

## Summary
- **Total Markers**: 913
- **Static Markers**: 909
- **Dynamic Markers**: 4 (contain shell variables)
- **Components Scanned**: 594
- **Duplicate Markers**: 56 ⚠️

## Marker Inventory
| Marker Name | Component | File Path | Line | API |
|-------------|-----------|-----------|------|-----|
| 2GRxPackets_split | OneWifi | scripts/process_monitor_atom.sh | 592 | t2ValNotify |
| 2GTxPackets_split | OneWifi | scripts/process_monitor_atom.sh | 587 | t2ValNotify |
| 5GclientMac_split | telemetry | source/testApp/testCommonLibApi.c | 73 | t2_event_s |
| acs_split | tr069-protocol-agent | source-embedded/DslhManagementServer/ccsp_management_server_pa_api.c | 767 | t2_event_s |
| APP_ERROR_Crashed_accum | crashupload | c_sourcecode/src/scanner/scanner.c | 617 | t2ValNotify→t2_event_s |
| APP_ERROR_Crashed_accum | crashupload | runDumpUpload.sh | 826 | t2ValNotify |
| APP_ERROR_Crashed_split | crashupload | c_sourcecode/src/scanner/scanner.c | 616 | t2ValNotify→t2_event_s |
| APP_ERROR_Crashed_split | crashupload | runDumpUpload.sh | 825 | t2ValNotify |
| APP_ERROR_Crashed_split | crashupload | uploadDumps_TestCases.md | 1618 | t2ValNotify |
| APP_ERROR_CrashInfo | crashupload | c_sourcecode/src/scanner/scanner.c | 620 | t2ValNotify→t2_event_s |
| APP_ERROR_CrashInfo | crashupload | runDumpUpload.sh | 828 | t2ValNotify |
| APP_ERROR_CrashInfo | crashupload | uploadDumps_TestCases.md | 1619 | t2ValNotify |
| APP_ERROR_CrashInfo_status | crashupload | c_sourcecode/src/scanner/scanner.c | 622 | t2ValNotify→t2_event_s |
| APP_ERROR_CrashInfo_status | crashupload | runDumpUpload.sh | 830 | t2ValNotify |
| APP_ERROR_CrashInfo_status | crashupload | uploadDumps_TestCases.md | 1620 | t2ValNotify |
| APPARMOR_C_split: | rdk-apparmor-profiles | apparmor_parse.sh | 119 | t2ValNotify |
| APPARMOR_E_split: | rdk-apparmor-profiles | apparmor_parse.sh | 126 | t2ValNotify |
| BasicBridgeMode_NotSupported | provisioning-and-management | source/TR-181/middle_layer_src/cosa_x_cisco_com_devicecontrol_dml.c | 2187 | t2_event_d |
| Board_temperature_split | sysint | lib/rdk/temperature-telemetry.sh | 27 | t2ValNotify |
| bootuptime_dnsIpChanged_split | utopia | source/service_udhcpc/service_udhcpc.c | 561 | t2_event_s |
| bootuptime_SNMPV2Ready_split | utopia | source/util/print_uptime/print_uptime.c | 139 | t2_event_d |
| bootuptime_wifi_split | utopia | source/util/print_uptime/print_uptime.c | 151 | t2_event_d |
| BT_ERR_BatteryThreadFail | bluetooth | src/btrCore.c | 5280 | telemetry_event_d→t2_event_d |
| BT_ERR_DiscStartFail | bluetooth | src/bt-ifce/btrCore_gdbus_bluez5.c | 5351 | telemetry_event_d→t2_event_d |
| BT_ERR_DiscStopFail | bluetooth | src/bt-ifce/btrCore_gdbus_bluez5.c | 5374 | telemetry_event_d→t2_event_d |
| BT_ERR_FailToPair | bluetooth | src/btrCore.c | 4479 | telemetry_event_d→t2_event_d |
| BT_ERR_GetBTAdapterFail | bluetooth | src/btrCore.c | 3768 | telemetry_event_d→t2_event_d |
| BT_INFO_NotSupp_split | bluetooth | src/btrCore.c | 3155 | telemetry_event_s→t2_event_s |
| BTconn_split | bluetooth | src/bt-ifce/btrCore_gdbus_bluez5.c | 906 | telemetry_event_s→t2_event_s |
| btime_clientconn_split | lan-manager-lite | source/lm/lm_main.c | 824 | t2_event_d |
| btime_cpenter_split | provisioning-and-management | arch/intel_usg/boards/arm_shared/scripts/network_response.sh | 557 | t2ValNotify |
| btime_cpexit_split | provisioning-and-management | arch/intel_usg/boards/arm_shared/scripts/network_response.sh | 508 | t2ValNotify |
| btime_eth_split | utopia | source/util/print_uptime/print_uptime.c | 127 | t2_event_d |
| btime_ipacqEth_split | sysint | lib/rdk/ipv6addressChange.sh | 62 | t2ValNotify |
| btime_ipacqWifi_split | sysint | lib/rdk/ipv6addressChange.sh | 65 | t2ValNotify |
| btime_laninit_split | utopia | source/service_dhcp/lan_handler.c | 795 | t2_event_d |
| btime_laninit_split | utopia | source/scripts/init/service.d/lan_handler.sh | 323 | t2ValNotify |
| btime_mesh_split | utopia | source/util/print_uptime/print_uptime.c | 131 | t2_event_d |
| btime_moca_split | utopia | source/util/print_uptime/print_uptime.c | 135 | t2_event_d |
| btime_waninit_split | utopia | source/service_wan/service_wan.c | 1155 | t2_event_d |
| btime_wanup_spit | utopia | source/util/print_uptime/print_uptime.c | 143 | t2_event_d |
| btime_wcpenter_split ⚠️ | provisioning-and-management | arch/intel_usg/boards/arm_shared/scripts/network_response.sh | 680 | t2ValNotify |
| btime_wcpenter_split ⚠️ | sysint-broadband | webgui_arm.sh | 325 | t2ValNotify |
| btime_wcpenter_split ⚠️ | webui | source/Styles/xb3/config/webgui.sh | 313 | t2ValNotify |
| btime_wcpexit_split | provisioning-and-management | arch/intel_usg/boards/arm_shared/scripts/revert_redirect.sh | 66 | t2ValNotify |
| btime_webpa_split | utopia | source/util/print_uptime/print_uptime.c | 147 | t2_event_d |
| btime_xhome_split | utopia | source/util/print_uptime/print_uptime.c | 155 | t2_event_d |
| BTpair_split | bluetooth | src/bt-ifce/btrCore_gdbus_bluez5.c | 904 | telemetry_event_s→t2_event_s |
| BTPairFail_split | bluetooth | src/btrCore.c | 3122 | telemetry_event_s→t2_event_s |
| BUFFER_MEMORY_split | test-and-diagnostic | scripts/log_mem_cpu_info.sh | 292 | t2ValNotify |
| CACHE_MEMORY_split | test-and-diagnostic | scripts/log_mem_cpu_info.sh | 291 | t2ValNotify |
| cachedMem_split | test-and-diagnostic | scripts/resource_monitor.sh | 464 | t2ValNotify |
| CDL_INFO_inprogressExit | sysint | lib/rdk/userInitiatedFWDnld.sh | 755 | t2CountNotify |
| CDLrdkportal_split ⚠️ | rdkfwupdater | src/device_status_helper.c | 377 | t2CountNotify→t2_event_d |
| CDLrdkportal_split ⚠️ | sysint | lib/rdk/userInitiatedFWDnld.sh | 178 | t2ValNotify |
| CDLsuspended_split | rdkfwupdater | src/rdkv_upgrade.c | 146 | Upgradet2CountNotify→t2_event_d |
| certerr_split ⚠️ | crashupload | c_sourcecode/src/upload/upload.c | 267 | t2ValNotify→t2_event_s |
| certerr_split ⚠️ | dcm-agent | uploadstblogs/src/path_handler.c | 458 | t2_val_notify→t2_event_s |
| certerr_split ⚠️ | rdkfwupdater | src/rdkv_upgrade.c | 284 | Upgradet2ValNotify→t2_event_s |
| certerr_split ⚠️ | rdm-agent | scripts/downloadUtils.sh | 417 | t2ValNotify |
| certerr_split ⚠️ | sysint | lib/rdk/uploadSTBLogs.sh | 307 | t2ValNotify |
| certerr_split ⚠️ | sysint | lib/rdk/xconfImageCheck.sh | 408 | t2ValNotify |
| certerr_split ⚠️ | sysint-broadband | stateRedRecoveryUtils.sh | 78 | t2ValNotify |
| certerr_split ⚠️ | sysint-broadband | stateRedRecoveryUtils.sh | 107 | t2ValNotify |
| certerr_split ⚠️ | xconf-client | scripts/cbr_firmwareDwnld.sh | 764 | t2ValNotify |
| certerr_split ⚠️ | xconf-client | scripts/cbr_firmwareDwnld.sh | 805 | t2ValNotify |
| certerr_split ⚠️ | xconf-client | scripts/cbr_firmwareDwnld.sh | 1472 | t2ValNotify |
| certerr_split ⚠️ | xconf-client | scripts/cbr_firmwareDwnld.sh | 1488 | t2ValNotify |
| certerr_split ⚠️ | xconf-client | scripts/xb6_firmwareDwnld.sh | 865 | t2ValNotify |
| certerr_split ⚠️ | xconf-client | scripts/xb6_firmwareDwnld.sh | 908 | t2ValNotify |
| certerr_split ⚠️ | xconf-client | scripts/xb6_firmwareDwnld.sh | 1628 | t2ValNotify |
| certerr_split ⚠️ | xconf-client | scripts/xb6_firmwareDwnld.sh | 1642 | t2ValNotify |
| cloudFWFile_split | rdkfwupdater | src/device_status_helper.c | 878 | t2ValNotify→t2_event_s |
| core_split | sysint | lib/rdk/core_shell.sh | 165 | t2ValNotify |
| CoredumpFail_split | crashupload | c_sourcecode/src/upload/upload.c | 286 | t2ValNotify→t2_event_s |
| coreUpld_split | crashupload | c_sourcecode/src/upload/upload.c | 228 | t2ValNotify→t2_event_s |
| cpuinfo_split | sysint | lib/rdk/system_info_collector.sh | 56 | t2ValNotify |
| cpuinfo_split | sysint | lib/rdk/cpu-statistics.sh | 30 | t2ValNotify |
| crashedContainerAppname_split | crashupload | c_sourcecode/src/scanner/scanner.c | 603 | t2ValNotify→t2_event_s |
| crashedContainerAppname_split | crashupload | runDumpUpload.sh | 817 | t2ValNotify |
| crashedContainerAppname_split | crashupload | uploadDumps_TestCases.md | 1615 | t2ValNotify |
| crashedContainerName_split | crashupload | c_sourcecode/src/scanner/scanner.c | 601 | t2ValNotify→t2_event_s |
| crashedContainerName_split | crashupload | runDumpUpload.sh | 815 | t2ValNotify |
| crashedContainerName_split | crashupload | uploadDumps_TestCases.md | 1613 | t2ValNotify |
| crashedContainerProcessName_split | crashupload | c_sourcecode/src/scanner/scanner.c | 604 | t2ValNotify→t2_event_s |
| crashedContainerProcessName_split | crashupload | runDumpUpload.sh | 818 | t2ValNotify |
| crashedContainerProcessName_split | crashupload | uploadDumps_TestCases.md | 1616 | t2ValNotify |
| crashedContainerStatus_split | crashupload | c_sourcecode/src/scanner/scanner.c | 602 | t2ValNotify→t2_event_s |
| crashedContainerStatus_split | crashupload | runDumpUpload.sh | 816 | t2ValNotify |
| crashedContainerStatus_split | crashupload | uploadDumps_TestCases.md | 1614 | t2ValNotify |
| CrashedProc_split | sysint | lib/rdk/core_shell.sh | 149 | t2ValNotify |
| CurlRet_split ⚠️ | rdkfwupdater | src/rdkv_main.c | 1166 | t2CountNotify→t2_event_d |
| CurlRet_split ⚠️ | sysint | lib/rdk/xconfImageCheck.sh | 418 | t2ValNotify |
| dadrecoverypartner_split | test-and-diagnostic | scripts/task_health_monitor.sh | 3886 | t2ValNotify |
| DCACBCurlFail_split | sysint-broadband | dcaSplunkUpload.sh | 324 | t2ValNotify |
| DCACurlFail_split | sysint-broadband | dcaSplunkUpload.sh | 258 | t2ValNotify |
| DCMCBCurlFail_split | sysint-broadband | DCMscript.sh | 382 | t2ValNotify |
| DCMXCONFCurlFail_split | sysint-broadband | DCMscript.sh | 318 | t2ValNotify |
| Discovered_MngdDev_split | secure-upnp | src/xdiscovery.c | 3517 | t2_event_s |
| emmcNoFile_split | sysint | lib/rdk/eMMC_Upgrade.sh | 131 | t2ValNotify |
| emmcVer_split | sysint | lib/rdk/eMMC_Upgrade.sh | 66 | t2ValNotify |
| factoryPartnerid_split | sysint-broadband | log_factoryPartnerId.sh | 38 | t2ValNotify |
| FileNr_split | test-and-diagnostic | scripts/FileHandle_Monitor.sh | 34 | t2ValNotify |
| Filesize_split | rdkfwupdater | src/rdkv_upgrade.c | 623 | Upgradet2CountNotify→t2_event_d |
| FREE_MEM_split | sysint | lib/rdk/system_info_collector.sh | 59 | t2ValNotify |
| FREE_MEM_split | sysint | lib/rdk/cpu-statistics.sh | 33 | t2ValNotify |
| FreeCPU_split | test-and-diagnostic | scripts/log_mem_cpu_info.sh | 243 | t2ValNotify |
| FW_ACTIVEBANK_split | miscellaneous-broadband | source/FwBankInfo/FwBank_Info.c | 69 | t2_event_s |
| FW_INACTIVEBANK_split | miscellaneous-broadband | source/FwBankInfo/FwBank_Info.c | 97 | t2_event_s |
| getcurrentpartner_split | utopia | source/scripts/init/src/apply_system_defaults_helper.c | 887 | t2_event_s |
| getcurrentpartner_split | utopia | source/scripts/init/src/apply_system_defaults/apply_system_defaults.c | 994 | t2_event_s |
| getfactorypartner_split | utopia | source/scripts/init/src/apply_system_defaults_helper.c | 879 | t2_event_s |
| getfactorypartner_split | utopia | source/scripts/init/src/apply_system_defaults_helper.c | 884 | t2_event_s |
| getfactorypartner_split | utopia | source/scripts/init/src/apply_system_defaults/apply_system_defaults.c | 985 | t2_event_s |
| getfactorypartner_split | utopia | source/scripts/init/src/apply_system_defaults/apply_system_defaults.c | 990 | t2_event_s |
| HDMI_DeviceInfo_split ⚠️ | entservices-hdmicecsink | plugin/HdmiCecSinkImplementation.cpp | 295 | t2_event_s |
| HDMI_DeviceInfo_split ⚠️ | entservices-hdmicecsource | plugin/HdmiCecSourceImplementation.cpp | 215 | t2_event_s |
| HDMI_INFO_PORT1connected | entservices-hdmicecsink | plugin/HdmiCecSinkImplementation.cpp | 1965 | t2_event_d |
| HDMI_INFO_PORT2connected | entservices-hdmicecsink | plugin/HdmiCecSinkImplementation.cpp | 1968 | t2_event_d |
| HDMI_INFO_PORT3connected | entservices-hdmicecsink | plugin/HdmiCecSinkImplementation.cpp | 1971 | t2_event_d |
| HDMI_WARN_CEC_InvalidParamExcptn | hdmicec | ccec/src/Bus.cpp | 346 | t2_event_s |
| HighDnldBytes_split | test-and-diagnostic | scripts/rxtx_cur.sh | 78 | t2ValNotify |
| HighDnldBytes_split | test-and-diagnostic | scripts/rxtx_cur.sh | 83 | t2ValNotify |
| HighDnldMAC_split | test-and-diagnostic | scripts/rxtx_cur.sh | 79 | t2ValNotify |
| HighDnldMAC_split | test-and-diagnostic | scripts/rxtx_cur.sh | 84 | t2ValNotify |
| HighUpldBytes_split | test-and-diagnostic | scripts/rxtx_cur.sh | 65 | t2ValNotify |
| HighUpldBytes_split | test-and-diagnostic | scripts/rxtx_cur.sh | 70 | t2ValNotify |
| HighUpldMAC_split | test-and-diagnostic | scripts/rxtx_cur.sh | 66 | t2ValNotify |
| HighUpldMAC_split | test-and-diagnostic | scripts/rxtx_cur.sh | 71 | t2ValNotify |
| IHC:AuthenticationConfigChanged_WAPInstance2.4G | test-and-diagnostic | source/ImageHealthChecker/ImagehealthChecker.c | 952 | report_t2→t2_event_d |
| IHC:AuthenticationConfigChanged_WAPInstance5G | test-and-diagnostic | source/ImageHealthChecker/ImagehealthChecker.c | 973 | report_t2→t2_event_d |
| IHC:AuthenticationConfigChanged_WAPInstance6G | test-and-diagnostic | source/ImageHealthChecker/ImagehealthChecker.c | 995 | report_t2→t2_event_d |
| IHC:ConnectedPrivateClientsBelow60P_Radio2.4G | test-and-diagnostic | source/ImageHealthChecker/ImagehealthChecker.c | 810 | report_t2→t2_event_d |
| IHC:ConnectedPrivateClientsBelow60P_Radio5G | test-and-diagnostic | source/ImageHealthChecker/ImagehealthChecker.c | 824 | report_t2→t2_event_d |
| IHC:ConnectedPrivateClientsBelow60P_Radio6G | test-and-diagnostic | source/ImageHealthChecker/ImagehealthChecker.c | 839 | report_t2→t2_event_d |
| IHC:ConnectedPrivateClientsBelow60P_RadioETH | test-and-diagnostic | source/ImageHealthChecker/ImagehealthChecker.c | 854 | report_t2→t2_event_d |
| IHC:ConnectedPrivateClientsZero_Radio2.4G | test-and-diagnostic | source/ImageHealthChecker/ImagehealthChecker.c | 804 | report_t2→t2_event_d |
| IHC:ConnectedPrivateClientsZero_Radio5G | test-and-diagnostic | source/ImageHealthChecker/ImagehealthChecker.c | 818 | report_t2→t2_event_d |
| IHC:ConnectedPrivateClientsZero_Radio6G | test-and-diagnostic | source/ImageHealthChecker/ImagehealthChecker.c | 833 | report_t2→t2_event_d |
| IHC:ConnectedPrivateClientsZero_RadioETH | test-and-diagnostic | source/ImageHealthChecker/ImagehealthChecker.c | 848 | report_t2→t2_event_d |
| IHC:EthernetPOD_Radio | test-and-diagnostic | source/ImageHealthChecker/ImagehealthChecker.c | 1768 | report_t2→t2_event_d |
| IHC:EthernetPOD_Radio | test-and-diagnostic | source/ImageHealthChecker/ImagehealthChecker.c | 1787 | report_t2→t2_event_d |
| IHC:SSIDChanged_WAPInstance2.4G_PRIV | test-and-diagnostic | source/ImageHealthChecker/ImagehealthChecker.c | 1206 | report_t2→t2_event_d |
| IHC:SSIDChanged_WAPInstance2.4G_PUBLIC | test-and-diagnostic | source/ImageHealthChecker/ImagehealthChecker.c | 1289 | report_t2→t2_event_d |
| IHC:SSIDChanged_WAPInstance2.4G_SPUBLIC | test-and-diagnostic | source/ImageHealthChecker/ImagehealthChecker.c | 1373 | report_t2→t2_event_d |
| IHC:SSIDChanged_WAPInstance5G_PRIV | test-and-diagnostic | source/ImageHealthChecker/ImagehealthChecker.c | 1227 | report_t2→t2_event_d |
| IHC:SSIDChanged_WAPInstance5G_PUBLIC | test-and-diagnostic | source/ImageHealthChecker/ImagehealthChecker.c | 1310 | report_t2→t2_event_d |
| IHC:SSIDChanged_WAPInstance5G_SPUBLIC | test-and-diagnostic | source/ImageHealthChecker/ImagehealthChecker.c | 1394 | report_t2→t2_event_d |
| IHC:SSIDChanged_WAPInstance6G_PRIV | test-and-diagnostic | source/ImageHealthChecker/ImagehealthChecker.c | 1249 | report_t2→t2_event_d |
| IHC:WirelessPOD_Radio2.4G_POD | test-and-diagnostic | source/ImageHealthChecker/ImagehealthChecker.c | 1654 | report_t2→t2_event_d |
| IHC:WirelessPOD_Radio2.4G_POD | test-and-diagnostic | source/ImageHealthChecker/ImagehealthChecker.c | 1683 | report_t2→t2_event_d |
| IHC:WirelessPOD_Radio2.4G_POD | test-and-diagnostic | source/ImageHealthChecker/ImagehealthChecker.c | 1719 | report_t2→t2_event_d |
| IHC:WirelessPOD_Radio5G_POD | test-and-diagnostic | source/ImageHealthChecker/ImagehealthChecker.c | 1664 | report_t2→t2_event_d |
| IHC:WirelessPOD_Radio5G_POD | test-and-diagnostic | source/ImageHealthChecker/ImagehealthChecker.c | 1709 | report_t2→t2_event_d |
| IHC:WirelessPOD_Radio5G_POD | test-and-diagnostic | source/ImageHealthChecker/ImagehealthChecker.c | 1738 | report_t2→t2_event_d |
| LinkQualityNonServiceable_Lan2WanBlocked | provisioning-and-management | source/TR-181/middle_layer_src/subscribeForRbusEvents.c | 85 | t2_event_d |
| LinkQualityServiceable_Lan2WanAllowed | provisioning-and-management | source/TR-181/middle_layer_src/subscribeForRbusEvents.c | 72 | t2_event_d |
| LOAD_AVG_ATOM_split ⚠️ | sysint-broadband | log_mem_cpu_info_atom.sh | 104 | t2ValNotify |
| LOAD_AVG_ATOM_split ⚠️ | test-and-diagnostic | scripts/log_mem_cpu_info.sh | 163 | t2ValNotify |
| LoadAvg_split | test-and-diagnostic | scripts/log_mem_cpu_info.sh | 161 | t2ValNotify |
| LUCurlErr_split ⚠️ | dcm-agent | uploadstblogs/src/path_handler.c | 214 | t2_val_notify→t2_event_s |
| LUCurlErr_split ⚠️ | dcm-agent | uploadstblogs/src/path_handler.c | 342 | t2_val_notify→t2_event_s |
| LUCurlErr_split ⚠️ | dcm-agent | uploadstblogs/src/path_handler.c | 431 | t2_val_notify→t2_event_s |
| LUCurlErr_split ⚠️ | dcm-agent | uploadstblogs/src/path_handler.c | 518 | t2_val_notify→t2_event_s |
| LUCurlErr_split ⚠️ | sysint | lib/rdk/uploadSTBLogs.sh | 338 | t2ValNotify |
| LUCurlErr_split ⚠️ | sysint | lib/rdk/uploadSTBLogs.sh | 614 | t2ValNotify |
| LUCurlErr_split ⚠️ | sysint | lib/rdk/uploadSTBLogs.sh | 645 | t2ValNotify |
| lxybundleversion_split | rdkfwupdater | src/json_process.c | 284 | t2ValNotify→t2_event_s |
| MAP-T_NotSupported | provisioning-and-management | source/TR-181/middle_layer_src/cosa_deviceinfo_dml.c | 22570 | t2_event_d |
| MAP-T_NotSupported | provisioning-and-management | source/TR-181/middle_layer_src/cosa_deviceinfo_dml.c | 22576 | t2_event_d |
| marker | rdkfwupdater | unittest/basic_rdkv_main_gtest.cpp | 246 | t2_event_s |
| marker | rdkfwupdater | unittest/basic_rdkv_main_gtest.cpp | 247 | t2ValNotify→t2_event_s |
| MFR_ERR_MFRSV_coredetected | sysint | lib/rdk/core_shell.sh | 85 | t2CountNotify |
| MPSTAT_SOFT_split | test-and-diagnostic | scripts/log_mem_cpu_info.sh | 241 | t2ValNotify |
| MTA_DHCP_ENABLE_FAIL_AfterRetries | media-terminal-adapter-agent | source/TR-181/middle_layer_src/cosa_rbus_apis.c | 305 | t2_event_d |
| MTA_WAN_GET_FAIL_AfterRetries | media-terminal-adapter-agent | source/TR-181/middle_layer_src/cosa_rbus_apis.c | 264 | t2_event_d |
| NF_ERR_rdm_filenotfound_extraction | rdm-agent | scripts/downloadUtils.sh | 622 | t2CountNotify |
| NF_INFO_codedumped | sysint | lib/rdk/core_shell.sh | 121 | t2CountNotify |
| NF_INFO_rdm_package_failure | rdm-agent | src/rdm_downloadmgr.c | 287 | t2CountNotify→t2_event_d |
| NF_INFO_rdm_package_failure | rdm-agent | src/rdm_packagemgr.c | 193 | t2CountNotify→t2_event_d |
| NF_INFO_rdm_success | rdm-agent | src/rdm_downloadmgr.c | 327 | t2ValNotify→t2_event_s |
| NF_INFO_rdm_success | rdm-agent | src/rdm_downloadmgr.c | 343 | t2ValNotify→t2_event_s |
| NVRAM_USE_PERCENTAGE_split | test-and-diagnostic | scripts/log_mem_cpu_info.sh | 284 | t2ValNotify |
| OneToOneNAT_NotSupported | provisioning-and-management | source/TR-181/middle_layer_src/cosa_deviceinfo_dml.c | 11652 | t2_event_d |
| PciEnumeration_split | test-and-diagnostic | scripts/selfheal_bootup.sh | 989 | t2ValNotify |
| PciEnumeration_split | test-and-diagnostic | scripts/log_twice_day.sh | 37 | t2ValNotify |
| PDRI_Version_split ⚠️ | rdkfwupdater | src/deviceutils/device_api.c | 163 | t2ValNotify→t2_event_s |
| PDRI_Version_split ⚠️ | sysint | lib/rdk/xconfImageCheck.sh | 458 | t2ValNotify |
| PortMappingEnable_split | test-and-diagnostic | scripts/log_twice_day.sh | 31 | t2ValNotify |
| processCrash_split | crashupload | c_sourcecode/src/scanner/scanner.c | 367 | t2ValNotify→t2_event_s |
| processCrash_split | crashupload | runDumpUpload.sh | 759 | t2ValNotify |
| processCrash_split | crashupload | uploadDumps_TestCases.md | 1588 | t2ValNotify |
| RCU_FWver_split | sysint | lib/rdk/xconfImageCheck.sh | 395 | t2ValNotify |
| RCU_FWver_split | sysint | lib/rdk/xconfImageCheck.sh | 541 | t2ValNotify |
| rdkb_rebootreason_split | provisioning-and-management | source/TR-181/middle_layer_src/plugin_main_apis.c | 826 | t2_event_s |
| RDKB_SELFHEAL:WAN_Link_Heal | test-and-diagnostic | scripts/check_gw_health.sh | 469 | t2ValNotify |
| RDKLOGS_USE_PERCENTAGE_split | test-and-diagnostic | scripts/log_mem_cpu_info.sh | 281 | t2ValNotify |
| RDM_ERR_package_failed | rdm-agent | src/rdm_downloadutils.c | 296 | t2CountNotify→t2_event_d |
| RDM_ERR_package_notfound | rdm-agent | src/rdm_downloadmgr.c | 167 | t2CountNotify→t2_event_d |
| RDM_ERR_rdm_retry_fail | rdm-agent | scripts/downloadUtils.sh | 378 | t2CountNotify |
| RDM_ERR_rsa_signature_failed | rdm-agent | src/rdm_downloadmgr.c | 316 | t2CountNotify→t2_event_d |
| RDM_ERR_rsa_signature_failed | rdm-agent | src/rdm_downloadmgr.c | 332 | t2CountNotify→t2_event_d |
| RDM_ERR_rsa_signature_failed | rdm-agent | src/rdm_packagemgr.c | 232 | t2CountNotify→t2_event_d |
| RDM_INFO_AppDownloadComplete | rdm-agent | rdm_main.c | 426 | t2ValNotify→t2_event_s |
| RDM_INFO_AppDownloadComplete | rdm-agent | rdm_main.c | 522 | t2ValNotify→t2_event_s |
| RDM_INFO_AppDownloadSuccess | rdm-agent | rdm_main.c | 534 | t2CountNotify→t2_event_d |
| RDM_INFO_DefaultURL | rdm-agent | src/rdm_downloadutils.c | 102 | t2ValNotify→t2_event_s |
| RDM_INFO_DirectBlocked | rdm-agent | src/rdm_downloadutils.c | 339 | t2CountNotify→t2_event_d |
| RDM_INFO_DownloadSSRURL | rdm-agent | src/rdm_downloadutils.c | 95 | t2ValNotify→t2_event_s |
| RDM_INFO_extraction_complete | rdm-agent | src/rdm_downloadmgr.c | 307 | t2CountNotify→t2_event_d |
| RDM_INFO_package_download | rdm-agent | src/rdm_downloadutils.c | 288 | t2ValNotify→t2_event_s |
| RDM_INFO_rsa_valid_signature | rdm-agent | src/rdm_openssl.c | 981 | t2CountNotify→t2_event_d |
| RDM_INFO_rsa_valid_signature | rdm-agent | src/rdm_downloadutils.c | 634 | t2CountNotify→t2_event_d |
| RDM_INFO_rsa_valid_signature | rdm-agent | src/rdm_packagemgr.c | 63 | t2CountNotify→t2_event_d |
| RDM_INFO_rsa_verify_signature_failure | rdm-agent | src/rdm_openssl.c | 985 | t2CountNotify→t2_event_d |
| RDM_INFO_rsa_verify_signature_failure | rdm-agent | src/rdm_downloadutils.c | 637 | t2CountNotify→t2_event_d |
| RemotePortUsage_split | test-and-diagnostic | scripts/remote_port_usage.sh | 86 | t2ValNotify |
| REVSSH_CMINTERFACE_FAILURE | sysint-broadband | startTunnel.sh | 139 | t2CountNotify |
| REVSSH_FAILURE | sysint-broadband | startTunnel.sh | 169 | t2CountNotify |
| REVSSH_GETCONFIGFILE_FAILURE | sysint-broadband | startTunnel.sh | 156 | t2CountNotify |
| REVSSH_SUCCESS | sysint-broadband | startTunnel.sh | 172 | t2CountNotify |
| RF_ERROR_DHCP_Rebinding | test-and-diagnostic | scripts/self_heal_connectivity_test.sh | 394 | t2CountNotify |
| RF_ERROR_DHCP_Rebinding | test-and-diagnostic | scripts/device/tccbr/self_heal_connectivity_test.sh | 274 | t2CountNotify |
| RF_ERROR_IPV4IPV6PingFailed | test-and-diagnostic | scripts/self_heal_connectivity_test.sh | 540 | t2CountNotify |
| RF_ERROR_IPV4IPV6PingFailed | test-and-diagnostic | scripts/device/tccbr/self_heal_connectivity_test.sh | 247 | t2CountNotify |
| RF_ERROR_IPV4IPV6PingFailed | test-and-diagnostic | scripts/device/tccbr/self_heal_connectivity_test.sh | 416 | t2CountNotify |
| RF_ERROR_IPV4PingFailed | test-and-diagnostic | scripts/self_heal_connectivity_test.sh | 343 | t2CountNotify |
| RF_ERROR_IPV4PingFailed | test-and-diagnostic | scripts/self_heal_connectivity_test.sh | 381 | t2CountNotify |
| RF_ERROR_IPV4PingFailed | test-and-diagnostic | scripts/device/tccbr/self_heal_connectivity_test.sh | 260 | t2CountNotify |
| RF_ERROR_IPV6PingFailed | test-and-diagnostic | scripts/self_heal_connectivity_test.sh | 362 | t2CountNotify |
| RF_ERROR_IPV6PingFailed | test-and-diagnostic | scripts/self_heal_connectivity_test.sh | 409 | t2CountNotify |
| RF_ERROR_IPV6PingFailed | test-and-diagnostic | scripts/device/tccbr/self_heal_connectivity_test.sh | 288 | t2CountNotify |
| RF_ERROR_LAN_stop | utopia | source/scripts/init/service.d/lan_handler.sh | 129 | t2CountNotify |
| RF_ERROR_Wan_down | utopia | source/service_wan/service_wan.c | 1360 | t2_event_d |
| RF_ERROR_wan_restart | utopia | source/service_wan/service_wan.c | 1147 | t2_event_d |
| RF_ERROR_WAN_stop | gw-provisioning-application | source/gw_prov_sm.c | 2381 | t2_event_d |
| RF_ERROR_WAN_stopped | test-and-diagnostic | scripts/selfheal_aggressive.sh | 913 | t2CountNotify |
| RF_ERROR_WAN_stopped | test-and-diagnostic | scripts/task_health_monitor.sh | 4180 | t2CountNotify |
| RF_INFO_RDKB_FIREWALL_RESTART | utopia | source/scripts/init/service.d/service_ipv4.sh | 114 | t2CountNotify |
| RF_INFO_RDKB_FIREWALL_RESTART | utopia | source/scripts/init/service.d/lan_handler.sh | 268 | t2CountNotify |
| RF_INFO_RDKB_FIREWALL_RESTART | utopia | source/scripts/init/service.d/lan_handler.sh | 313 | t2CountNotify |
| RF_INFO_RDKB_FIREWALL_RESTART | utopia | source/scripts/init/service.d/service_ntpclient.sh | 189 | t2CountNotify |
| RF_INFO_RDKB_FIREWALL_RESTART | utopia | source/scripts/init/service.d/service_multinet.sh | 155 | t2CountNotify |
| RF_INFO_RDKB_FIREWALL_RESTART | utopia | source/scripts/init/service.d/service_ciscoconnect.sh | 67 | t2CountNotify |
| RF_INFO_RDKB_FIREWALL_RESTART | utopia | source/scripts/init/service.d/vlan_util_xb6.sh | 1114 | t2CountNotify |
| RF_INFO_RDKB_FIREWALL_RESTART | utopia | source/scripts/init/service.d/vlan_util_xb6.sh | 1124 | t2CountNotify |
| RF_INFO_RDKB_FIREWALL_RESTART | utopia | source/scripts/init/service.d/vlan_util_xb6.sh | 1138 | t2CountNotify |
| RF_INFO_RDKB_FIREWALL_RESTART | utopia | source/scripts/init/service.d/vlan_util_xb6.sh | 1148 | t2CountNotify |
| RF_INFO_RDKB_FIREWALL_RESTART | utopia | source/scripts/init/service.d/vlan_util_xb6.sh | 1171 | t2CountNotify |
| RF_INFO_RDKB_FIREWALL_RESTART | utopia | source/scripts/init/service.d/vlan_util_xb6.sh | 1369 | t2CountNotify |
| RF_INFO_RDKB_FIREWALL_RESTART | utopia | source/scripts/init/service.d/vlan_util_xb6.sh | 1379 | t2CountNotify |
| RF_INFO_RDKB_FIREWALL_RESTART | utopia | source/scripts/init/service.d/vlan_util_xb6.sh | 1392 | t2CountNotify |
| RF_INFO_RDKB_FIREWALL_RESTART | utopia | source/scripts/init/service.d/vlan_util_xb6.sh | 1405 | t2CountNotify |
| RF_INFO_RDKB_FIREWALL_RESTART | utopia | source/scripts/init/service.d/vlan_util_xb6.sh | 1433 | t2CountNotify |
| RF_INFO_RDKB_FIREWALL_RESTART | utopia | source/scripts/init/service.d/service_forwarding.sh | 174 | t2CountNotify |
| RF_INFO_RDKB_FIREWALL_RESTART | utopia | source/scripts/init/service.d/service_forwarding.sh | 195 | t2CountNotify |
| Router_Discovered | networkmanager | tools/upnp/UpnpDiscoveryManager.cpp | 97 | t2_event_s |
| SCARD_INFO_emmc_noUpgd | sysint | lib/rdk/eMMC_Upgrade.sh | 135 | t2CountNotify |
| SHORTS_CONN_SUCCESS | sysint | lib/rdk/startStunnel.sh | 181 | t2CountNotify |
| SHORTS_DEVICE_TYPE_PROD | sysint | lib/rdk/startStunnel.sh | 120 | t2CountNotify |
| SHORTS_DEVICE_TYPE_TEST | sysint | lib/rdk/startStunnel.sh | 115 | t2CountNotify |
| SHORTS_DEVICE_TYPE_UNKNOWN | sysint | lib/rdk/startStunnel.sh | 126 | t2CountNotify |
| SHORTS_SSH_CLIENT_FAILURE | sysint | lib/rdk/startStunnel.sh | 176 | t2CountNotify |
| SHORTS_STUNNEL_CERT_FAILURE | sysint | lib/rdk/startStunnel.sh | 101 | t2CountNotify |
| SHORTS_STUNNEL_CLIENT_FAILURE | sysint | lib/rdk/startStunnel.sh | 162 | t2CountNotify |
| Slab_split | test-and-diagnostic | scripts/resource_monitor.sh | 474 | t2ValNotify |
| SlabUsage_split | test-and-diagnostic | scripts/resource_monitor.sh | 484 | t2ValNotify |
| SPEEDTEST_IPERF_INFO_split | rdk-speedtest-cli | source/rdk-speedtest-cli.c | 565 | t2_event_s |
| SWAP_MEMORY_split | test-and-diagnostic | scripts/log_mem_cpu_info.sh | 290 | t2ValNotify |
| swdlCBCurlFail_split | xconf-client | scripts/cbr_firmwareDwnld.sh | 434 | t2ValNotify |
| swdlCBCurlFail_split | xconf-client | scripts/xb6_firmwareDwnld.sh | 450 | t2ValNotify |
| swdlCurlFail_split | xconf-client | scripts/cbr_firmwareDwnld.sh | 393 | t2ValNotify |
| swdlCurlFail_split | xconf-client | scripts/xb6_firmwareDwnld.sh | 409 | t2ValNotify |
| SYS_ERROR_5min_avg_cpu_100 | test-and-diagnostic | scripts/resource_monitor.sh | 297 | t2CountNotify |
| SYS_ERROR_ApplyDefaut_MeshStatus | mesh-agent | source/MeshAgentSsp/cosa_mesh_apis.c | 5212 | t2_event_d |
| SYS_ERROR_brlan0_not_created | test-and-diagnostic | scripts/selfheal_aggressive.sh | 463 | t2CountNotify |
| SYS_ERROR_brlan0_not_created | test-and-diagnostic | scripts/selfheal_aggressive.sh | 528 | t2CountNotify |
| SYS_ERROR_brlan0_not_created | test-and-diagnostic | scripts/task_health_monitor.sh | 2743 | t2CountNotify |
| SYS_ERROR_brlan0_not_created | test-and-diagnostic | scripts/task_health_monitor.sh | 2808 | t2CountNotify |
| SYS_ERROR_brlan0_not_created | test-and-diagnostic | scripts/device/tccbr/selfheal_bootup.sh | 447 | t2CountNotify |
| SYS_ERROR_CCSPBus_error190 | data-model-cli | source/ccsp_message_bus_client_tool.c | 901 | t2_event_d |
| SYS_ERROR_CCSPBus_error191 | data-model-cli | source/ccsp_message_bus_client_tool.c | 905 | t2_event_d |
| SYS_ERROR_CcspTandDSspHung_restart | test-and-diagnostic | scripts/task_health_monitor.sh | 5162 | t2CountNotify |
| SYS_ERROR_CM_Not_Registered | component-registry | source/CrSsp/ssp_dbus.c | 501 | t2_event_d |
| SYS_ERROR_CPU100 | test-and-diagnostic | scripts/log_mem_cpu_info.sh | 218 | t2CountNotify |
| SYS_ERROR_DHCPV4Client_notrunning ⚠️ | test-and-diagnostic | scripts/selfheal_aggressive.sh | 1118 | t2CountNotify |
| SYS_ERROR_DHCPV4Client_notrunning ⚠️ | test-and-diagnostic | scripts/selfheal_aggressive.sh | 1125 | t2CountNotify |
| SYS_ERROR_DHCPV4Client_notrunning ⚠️ | test-and-diagnostic | scripts/selfheal_aggressive.sh | 1139 | t2CountNotify |
| SYS_ERROR_DHCPV4Client_notrunning ⚠️ | test-and-diagnostic | scripts/selfheal_aggressive.sh | 1145 | t2CountNotify |
| SYS_ERROR_DHCPV4Client_notrunning ⚠️ | test-and-diagnostic | scripts/task_health_monitor.sh | 4375 | t2CountNotify |
| SYS_ERROR_DHCPV4Client_notrunning ⚠️ | wan-manager | source/WanManager/wanmgr_interface_sm.c | 514 | t2_event_d |
| SYS_ERROR_DHCPV6Client_notrunning ⚠️ | test-and-diagnostic | scripts/selfheal_aggressive.sh | 1158 | t2CountNotify |
| SYS_ERROR_DHCPV6Client_notrunning ⚠️ | test-and-diagnostic | scripts/selfheal_aggressive.sh | 1167 | t2CountNotify |
| SYS_ERROR_DHCPV6Client_notrunning ⚠️ | test-and-diagnostic | scripts/selfheal_aggressive.sh | 1173 | t2CountNotify |
| SYS_ERROR_DHCPV6Client_notrunning ⚠️ | test-and-diagnostic | scripts/task_health_monitor.sh | 4384 | t2CountNotify |
| SYS_ERROR_DHCPV6Client_notrunning ⚠️ | wan-manager | source/WanManager/wanmgr_interface_sm.c | 536 | t2_event_d |
| SYS_ERROR_Dibbler_DAD_failed | test-and-diagnostic | scripts/selfheal_aggressive.sh | 735 | t2CountNotify |
| SYS_ERROR_Dibbler_DAD_failed | test-and-diagnostic | scripts/selfheal_aggressive.sh | 807 | t2CountNotify |
| SYS_ERROR_Dibbler_DAD_failed | test-and-diagnostic | scripts/task_health_monitor.sh | 3962 | t2CountNotify |
| SYS_ERROR_Dibbler_DAD_failed | test-and-diagnostic | scripts/task_health_monitor.sh | 4027 | t2CountNotify |
| SYS_ERROR_DibblerServer_emptyconf | test-and-diagnostic | scripts/selfheal_aggressive.sh | 751 | t2CountNotify |
| SYS_ERROR_DibblerServer_emptyconf | test-and-diagnostic | scripts/selfheal_aggressive.sh | 757 | t2CountNotify |
| SYS_ERROR_DibblerServer_emptyconf | test-and-diagnostic | scripts/selfheal_aggressive.sh | 852 | t2CountNotify |
| SYS_ERROR_DibblerServer_emptyconf | test-and-diagnostic | scripts/task_health_monitor.sh | 3978 | t2CountNotify |
| SYS_ERROR_DibblerServer_emptyconf | test-and-diagnostic | scripts/task_health_monitor.sh | 4078 | t2CountNotify |
| SYS_ERROR_DmCli_Bridge_mode_error | test-and-diagnostic | scripts/task_health_monitor.sh | 1049 | t2CountNotify |
| SYS_ERROR_DNSFAIL | wan-manager | source/TR-181/middle_layer_src/wanmgr_rdkbus_apis.c | 1922 | t2_event_d |
| SYS_ERROR_Drop_cache | test-and-diagnostic | scripts/check_memory_health.sh | 43 | t2CountNotify |
| SYS_ERROR_Duplicate_crontab | test-and-diagnostic | scripts/task_health_monitor.sh | 468 | t2CountNotify |
| SYS_ERROR_ErouterDown_reboot | test-and-diagnostic | scripts/selfheal_aggressive.sh | 634 | t2CountNotify |
| SYS_ERROR_ErouterDown_reboot | test-and-diagnostic | scripts/task_health_monitor.sh | 983 | t2CountNotify |
| SYS_ERROR_Error_fetching_devicemode | test-and-diagnostic | scripts/selfheal_aggressive.sh | 394 | t2CountNotify |
| SYS_ERROR_Error_fetching_devicemode | test-and-diagnostic | scripts/selfheal_aggressive.sh | 501 | t2CountNotify |
| SYS_ERROR_Error_fetching_devicemode | test-and-diagnostic | scripts/task_health_monitor.sh | 2672 | t2CountNotify |
| SYS_ERROR_Error_fetching_devicemode | test-and-diagnostic | scripts/task_health_monitor.sh | 2781 | t2CountNotify |
| SYS_ERROR_Factory_partner_set_comcast | utopia | source/scripts/init/src/apply_system_defaults_helper.c | 843 | t2_event_d |
| SYS_ERROR_Factory_partner_set_comcast | utopia | source/scripts/init/src/apply_system_defaults/apply_system_defaults.c | 944 | t2_event_d |
| SYS_ERROR_Factorypartner_fetch_failed | utopia | source/scripts/init/src/apply_system_defaults_helper.c | 760 | t2_event_d |
| SYS_ERROR_Factorypartner_fetch_failed | utopia | source/scripts/init/src/apply_system_defaults_helper.c | 841 | t2_event_d |
| SYS_ERROR_Factorypartner_fetch_failed | utopia | source/scripts/init/src/apply_system_defaults/apply_system_defaults.c | 743 | t2_event_d |
| SYS_ERROR_Factorypartner_fetch_failed | utopia | source/scripts/init/src/apply_system_defaults/apply_system_defaults.c | 941 | t2_event_d |
| SYS_ERROR_FW_ACTIVEBANK_FETCHFAILED | miscellaneous-broadband | source/FwBankInfo/FwBank_Info.c | 56 | t2_event_d |
| SYS_ERROR_FW_INACTIVEBANK_FETCHFAILED | miscellaneous-broadband | source/FwBankInfo/FwBank_Info.c | 84 | t2_event_d |
| SYS_ERROR_GRETunnel_restored | test-and-diagnostic | scripts/task_health_monitor.sh | 1866 | t2CountNotify |
| SYS_ERROR_GRETunnel_restored | test-and-diagnostic | scripts/task_health_monitor.sh | 1911 | t2CountNotify |
| SYS_ERROR_GRETunnel_restored | test-and-diagnostic | scripts/task_health_monitor.sh | 1979 | t2CountNotify |
| SYS_ERROR_GRETunnel_restored | test-and-diagnostic | scripts/task_health_monitor.sh | 2017 | t2CountNotify |
| SYS_ERROR_GRETunnel_restored | test-and-diagnostic | scripts/task_health_monitor.sh | 2063 | t2CountNotify |
| SYS_ERROR_ICC_ABOVE_THRESHOLD | test-and-diagnostic | scripts/log_mem_cpu_info.sh | 142 | t2CountNotify |
| SYS_ERROR_ICC_BELOW_THRESHOLD | test-and-diagnostic | scripts/log_mem_cpu_info.sh | 145 | t2CountNotify |
| SYS_ERROR_INVALID_PARTNER_ID_DETECTED | utopia | source/scripts/init/src/apply_system_defaults_helper.c | 1089 | t2_event_d |
| SYS_ERROR_INVALID_PARTNER_ID_DETECTED | utopia | source/scripts/init/src/apply_system_defaults/apply_system_defaults.c | 848 | t2_event_d |
| SYS_ERROR_INVALID_PARTNER_ID_RECOVERY_FAILURE | utopia | source/scripts/init/src/apply_system_defaults_helper.c | 1114 | t2_event_d |
| SYS_ERROR_INVALID_PARTNER_ID_RECOVERY_FAILURE | utopia | source/scripts/init/src/apply_system_defaults/apply_system_defaults.c | 873 | t2_event_d |
| SYS_ERROR_IPAOR | lan-manager-lite | source/lm/lm_main.c | 720 | t2_event_d |
| SYS_ERROR_iptable_corruption | test-and-diagnostic | scripts/selfheal_bootup.sh | 625 | t2CountNotify |
| SYS_ERROR_iptable_corruption | test-and-diagnostic | scripts/task_health_monitor.sh | 3581 | t2CountNotify |
| SYS_ERROR_iptable_corruption | test-and-diagnostic | scripts/device/tccbr/selfheal_bootup.sh | 471 | t2CountNotify |
| SYS_ERROR_linkLocalDad_failed | test-and-diagnostic | scripts/task_health_monitor.sh | 3875 | t2CountNotify |
| SYS_ERROR_LoadAbove2 | test-and-diagnostic | scripts/log_mem_cpu_info.sh | 169 | t2CountNotify |
| SYS_ERROR_LoadAbove3 | test-and-diagnostic | scripts/log_mem_cpu_info.sh | 173 | t2CountNotify |
| SYS_ERROR_LoadAbove4 | test-and-diagnostic | scripts/log_mem_cpu_info.sh | 177 | t2CountNotify |
| SYS_ERROR_LoadAbove5 | test-and-diagnostic | scripts/log_mem_cpu_info.sh | 181 | t2CountNotify |
| SYS_ERROR_LoadAbove8 | test-and-diagnostic | scripts/log_mem_cpu_info.sh | 185 | t2CountNotify |
| SYS_ERROR_LoadAbove9 | test-and-diagnostic | scripts/log_mem_cpu_info.sh | 189 | t2CountNotify |
| SYS_ERROR_LOGUPLOAD_FAILED ⚠️ | meta-cmf-bananapi | meta-rdk-mtk-bpir4/recipes-rdkb/sysint-broadband/files/uploadRDKBLogs.sh | 628 | t2CountNotify |
| SYS_ERROR_LOGUPLOAD_FAILED ⚠️ | meta-cmf-bananapi | meta-rdk-mtk-bpir4/recipes-rdkb/sysint-broadband/files/uploadRDKBLogs.sh | 665 | t2CountNotify |
| SYS_ERROR_LOGUPLOAD_FAILED ⚠️ | sysint-broadband | opsLogUpload.sh | 348 | t2CountNotify |
| SYS_ERROR_LOGUPLOAD_FAILED ⚠️ | sysint-broadband | opsLogUpload.sh | 482 | t2CountNotify |
| SYS_ERROR_LOGUPLOAD_FAILED ⚠️ | sysint-broadband | onboardLogUpload.sh | 273 | t2CountNotify |
| SYS_ERROR_LOGUPLOAD_FAILED ⚠️ | sysint-broadband | onboardLogUpload.sh | 315 | t2CountNotify |
| SYS_ERROR_LOGUPLOAD_FAILED ⚠️ | sysint-broadband | uploadRDKBLogs.sh | 588 | t2CountNotify |
| SYS_ERROR_LOGUPLOAD_FAILED ⚠️ | sysint-broadband | uploadRDKBLogs.sh | 829 | t2CountNotify |
| SYS_ERROR_LOW_FREE_MEMORY | test-and-diagnostic | scripts/log_mem_cpu_info.sh | 402 | t2CountNotify |
| SYS_ERROR_MemAbove550 | test-and-diagnostic | scripts/log_mem_cpu_info.sh | 91 | get_high_mem_processes→t2ValNotify |
| SYS_ERROR_MemAbove600 | test-and-diagnostic | scripts/log_mem_cpu_info.sh | 91 | get_high_mem_processes→t2ValNotify |
| SYS_ERROR_MissingMgmtCRPwdID | tr069-protocol-agent | source-embedded/DslhManagementServer/ccsp_management_server_pa_api.c | 303 | t2_event_d |
| SYS_ERROR_NoConnectivity_reboot ⚠️ | meta-rdk-qcom | meta-cmf-ipq/recipes-ccsp/ccsp/sysint/device/lib/rdk/logfiles.sh | 386 | t2CountNotify |
| SYS_ERROR_NoConnectivity_reboot ⚠️ | sysint-broadband | logfiles.sh | 500 | t2CountNotify |
| SYS_ERROR_NotGenMgmtCRPwdID | tr069-protocol-agent | source-embedded/DslhManagementServer/ccsp_management_server.c | 2603 | t2_event_d |
| SYS_ERROR_NotRegisteredOnCMTS | test-and-diagnostic | scripts/corrective_action.sh | 247 | t2CountNotify |
| SYS_ERROR_NotRegisteredOnCMTS | test-and-diagnostic | scripts/corrective_action.sh | 266 | t2CountNotify |
| SYS_ERROR_NotRegisteredOnCMTS | test-and-diagnostic | scripts/corrective_action.sh | 285 | t2CountNotify |
| SYS_ERROR_NotRegisteredOnCMTS | test-and-diagnostic | scripts/corrective_action.sh | 350 | t2CountNotify |
| SYS_ERROR_NotRegisteredOnCMTS | test-and-diagnostic | scripts/corrective_action.sh | 369 | t2CountNotify |
| SYS_ERROR_NotRegisteredOnCMTS | test-and-diagnostic | scripts/corrective_action.sh | 386 | t2CountNotify |
| SYS_ERROR_NTP_UNSYNC | utopia | source/scripts/init/service.d/service_ntpd.sh | 295 | t2CountNotify |
| SYS_ERROR_NVRAM2_NOT_AVAILABLE | test-and-diagnostic | scripts/log_mem_cpu_info.sh | 53 | t2CountNotify |
| SYS_ERROR_parodus_TimeStampExpired | test-and-diagnostic | scripts/task_health_monitor.sh | 2271 | t2CountNotify |
| SYS_ERROR_PartnerId_missing_sycfg | sysint-broadband | getpartnerid.sh | 50 | t2CountNotify |
| SYS_ERROR_PnM_Not_Responding | test-and-diagnostic | scripts/task_health_monitor.sh | 1077 | t2CountNotify |
| SYS_ERROR_PnM_Not_Responding | test-and-diagnostic | scripts/task_health_monitor.sh | 1101 | t2CountNotify |
| SYS_ERROR_PnM_Not_Responding | test-and-diagnostic | scripts/task_health_monitor.sh | 3128 | t2CountNotify |
| SYS_ERROR_PSM_Not_Registered | component-registry | source/CrSsp/ssp_dbus.c | 505 | t2_event_d |
| SYS_ERROR_PSMCrash_reboot ⚠️ | provisioning-and-management | arch/intel_usg/boards/arm_shared/scripts/process_monitor.sh | 388 | t2CountNotify |
| SYS_ERROR_PSMCrash_reboot ⚠️ | test-and-diagnostic | scripts/selfheal_bootup.sh | 513 | t2CountNotify |
| SYS_ERROR_PSMCrash_reboot ⚠️ | test-and-diagnostic | scripts/device/tccbr/selfheal_bootup.sh | 402 | t2CountNotify |
| SYS_ERROR_S3CoreUpload_Failed | crashupload | c_sourcecode/src/upload/upload.c | 275 | t2CountNotify→t2_event_d |
| SYS_ERROR_SnmpCMHighCPU_reboot | test-and-diagnostic | scripts/resource_monitor.sh | 263 | t2CountNotify |
| SYS_ERROR_snmpSubagentcrash | test-and-diagnostic | scripts/task_health_monitor.sh | 2480 | t2CountNotify |
| SYS_ERROR_snmpSubagentcrash | test-and-diagnostic | scripts/task_health_monitor.sh | 2588 | t2CountNotify |
| SYS_ERROR_SYSCFG_Open_failed | mesh-agent | source/MeshAgentSsp/cosa_mesh_apis.c | 5189 | t2_event_d |
| SYS_ERROR_SyscfgGet_retry_failed | mesh-agent | source/MeshAgentSsp/cosa_mesh_apis.c | 5183 | t2_event_d |
| SYS_ERROR_SyscfgSet_retry_failed | mesh-agent | source/MeshAgentSsp/cosa_mesh_apis.c | 2941 | t2_event_d |
| SYS_ERROR_syseventdBadState | test-and-diagnostic | scripts/task_health_monitor.sh | 2537 | t2CountNotify |
| SYS_ERROR_syseventdCrashed | test-and-diagnostic | scripts/task_health_monitor.sh | 2496 | t2CountNotify |
| SYS_ERROR_SYSTIME_FAIL | test-and-diagnostic | source/TandDSsp/current_time.c | 171 | t2_event_d |
| SYS_ERROR_SYSTIME_FAIL | test-and-diagnostic | source/TandDSsp/current_time.c | 431 | t2_event_d |
| SYS_ERROR_TMPFS_ABOVE85 | test-and-diagnostic | scripts/log_mem_cpu_info.sh | 65 | t2CountNotify |
| SYS_ERROR_TR69_Not_Registered | component-registry | source/CCSP_CR/ccsp_cr_utility.c | 405 | t2_event_d |
| SYS_ERROR_wanmanager_crash_reboot | test-and-diagnostic | scripts/task_health_monitor.sh | 1418 | t2CountNotify |
| SYS_ERROR_WIFI_Not_Registered | component-registry | source/CrSsp/ssp_dbus.c | 509 | t2_event_d |
| SYS_ERROR_Xdns_restart | utopia | source/scripts/init/service.d/service_xdns.sh | 50 | t2CountNotify |
| SYS_ERROR_Zombie_dnsmasq | test-and-diagnostic | scripts/selfheal_aggressive.sh | 280 | t2CountNotify |
| SYS_ERROR_Zombie_dnsmasq | test-and-diagnostic | scripts/task_health_monitor.sh | 3697 | t2CountNotify |
| SYS_ERROR_Zombie_dnsmasq | test-and-diagnostic | scripts/task_health_monitor.sh | 3702 | t2CountNotify |
| SYS_ERROR_Zombie_dnsmasq | test-and-diagnostic | scripts/task_health_monitor.sh | 3717 | t2CountNotify |
| SYS_ERROR_Zombie_dnsmasq | test-and-diagnostic | scripts/task_health_monitor.sh | 3846 | t2CountNotify |
| SYS_INFO_bootup ⚠️ | meta-rdk-qcom | meta-cmf-ipq/recipes-ccsp/ccsp/sysint/rdkbLogMonitor.sh | 876 | t2CountNotify |
| SYS_INFO_bootup ⚠️ | sysint-broadband | rdkbLogMonitor_cron.sh | 907 | t2CountNotify |
| SYS_INFO_bootup ⚠️ | sysint-broadband | rdkbLogMonitor.sh | 833 | t2CountNotify |
| SYS_INFO_BridgeMode | test-and-diagnostic | scripts/task_health_monitor.sh | 3430 | t2CountNotify |
| SYS_INFO_CANARY_Update | rdkfwupdater | src/flash.c | 396 | flashT2CountNotify→t2_event_d |
| SYS_INFO_CaptivePortal ⚠️ | dhcp-manager | source/DHCPServerUtils/DHCPv4Server/dhcp_server_functions.c | 1401 | t2_event_d |
| SYS_INFO_CaptivePortal ⚠️ | dhcp-manager | source/DHCPServerUtils/DHCPv4Server/dhcp_server_functions.c | 1421 | t2_event_d |
| SYS_INFO_CaptivePortal ⚠️ | utopia | source/service_dhcp/dhcp_server_functions.c | 1425 | t2_event_d |
| SYS_INFO_CaptivePortal ⚠️ | utopia | source/service_dhcp/dhcp_server_functions.c | 1445 | t2_event_d |
| SYS_INFO_CaptivePortal ⚠️ | utopia | source/scripts/init/service.d/service_dhcp_server/dhcp_server_functions.sh | 994 | t2CountNotify |
| SYS_INFO_CodBPASS ⚠️ | rdkfwupdater | src/rdkv_upgrade.c | 825 | Upgradet2CountNotify→t2_event_d |
| SYS_INFO_CodBPASS ⚠️ | sysint | lib/rdk/userInitiatedFWDnld.sh | 538 | t2CountNotify |
| SYS_INFO_CodBPASS ⚠️ | sysint | lib/rdk/userInitiatedFWDnld.sh | 620 | t2CountNotify |
| SYS_INFO_CrashedContainer | crashupload | c_sourcecode/src/scanner/scanner.c | 605 | t2CountNotify→t2_event_d |
| SYS_INFO_CrashedContainer | crashupload | runDumpUpload.sh | 819 | t2CountNotify |
| SYS_INFO_CrashedContainer | crashupload | uploadDumps_TestCases.md | 1617 | t2CountNotify |
| SYS_INFO_Create_GRE_Tunnel | hotspot | source/hotspotfd/hotspotfd.c | 2134 | t2_event_d |
| SYS_INFO_DBCleanup | test-and-diagnostic | scripts/syscfg_cleanup.sh | 91 | t2CountNotify |
| SYS_INFO_DEFER_CANARY_REBOOT | rdkfwupdater | src/flash.c | 392 | flashT2CountNotify→t2_event_d |
| SYS_INFO_differentSSID | test-and-diagnostic | scripts/getSsidNames.sh | 96 | t2CountNotify |
| SYS_INFO_differentSSID | test-and-diagnostic | scripts/getSsidNames.sh | 118 | t2CountNotify |
| SYS_INFO_DirectSuccess | rdkfwupdater | src/rdkv_upgrade.c | 1156 | Upgradet2CountNotify→t2_event_d |
| SYS_INFO_DirectSuccess | rdkfwupdater | src/rdkv_upgrade.c | 1226 | Upgradet2CountNotify→t2_event_d |
| SYS_INFO_DNS_updated | wan-manager | source/TR-181/middle_layer_src/wanmgr_rdkbus_apis.c | 2156 | t2_event_d |
| SYS_INFO_DNSSTART_split | wan-manager | source/TR-181/middle_layer_src/wanmgr_rdkbus_apis.c | 2173 | t2_event_s |
| SYS_INFO_ERouter_Mode_2 ⚠️ | cable-modem-agent | source/CMAgentSsp/gw_prov_sm.c | 1081 | t2_event_d |
| SYS_INFO_ERouter_Mode_2 ⚠️ | gw-provisioning-application | source/gw_prov_sm.c | 1017 | t2_event_d |
| SYS_INFO_ErouterMode2 ⚠️ | cable-modem-agent | source/CMAgentSsp/gw_prov_sm.c | 1719 | t2_event_d |
| SYS_INFO_ErouterMode2 ⚠️ | gw-provisioning-application | source/gw_prov_sm.c | 1629 | t2_event_d |
| SYS_INFO_ErouterMode2 ⚠️ | gw-provisioning-application | source/gw_prov_sm_generic.c | 802 | t2_event_d |
| SYS_INFO_FRMode | utopia | source/scripts/init/src/apply_system_defaults_psm/apply_system_defaults_psm.c | 172 | t2_event_d |
| SYS_INFO_FRMode | utopia | source/scripts/init/src/apply_system_defaults/apply_system_defaults.c | 3498 | t2_event_d |
| SYS_INFO_FRMode | utopia | source/scripts/init/src/apply_system_defaults/apply_system_defaults_syscfg.c | 177 | t2_event_d |
| SYS_INFO_FW_Dwld_500Error | xconf-client | source/cm_http_dl.c | 481 | t2_event_d |
| SYS_INFO_Hostname_changed | lan-manager-lite | source/lm/lm_wrapper.c | 2020 | t2_event_d |
| SYS_INFO_Hostname_changed | lan-manager-lite | source/lm/lm_wrapper.c | 2142 | t2_event_d |
| SYS_INFO_Hotspot_MaxClients | hotspot | source/hotspotfd/dhcpsnooper.c | 834 | t2_event_d |
| SYS_INFO_INTERNETRDY_split | utopia | source/scripts/init/service.d/service_connectivitycheck.sh | 104 | t2ValNotify |
| SYS_INFO_Invoke_batterymode ⚠️ | telemetry | source/testApp/testCommonLibApi.c | 75 | t2_event_s |
| SYS_INFO_Invoke_batterymode ⚠️ | test-and-diagnostic | scripts/resource_monitor.sh | 421 | t2CountNotify |
| SYS_INFO_LOGS_UPLOADED ⚠️ | meta-cmf-bananapi | meta-rdk-mtk-bpir4/recipes-rdkb/sysint-broadband/files/uploadRDKBLogs.sh | 623 | t2CountNotify |
| SYS_INFO_LOGS_UPLOADED ⚠️ | sysint-broadband | opsLogUpload.sh | 396 | t2CountNotify |
| SYS_INFO_LOGS_UPLOADED ⚠️ | sysint-broadband | opsLogUpload.sh | 471 | t2CountNotify |
| SYS_INFO_LOGS_UPLOADED ⚠️ | sysint-broadband | onboardLogUpload.sh | 311 | t2CountNotify |
| SYS_INFO_LOGS_UPLOADED ⚠️ | sysint-broadband | uploadRDKBLogs.sh | 673 | t2CountNotify |
| SYS_INFO_LOGS_UPLOADED ⚠️ | sysint-broadband | uploadRDKBLogs.sh | 807 | t2CountNotify |
| SYS_INFO_MESHWIFI_DISABLED | mesh-agent | source/MeshAgentSsp/cosa_mesh_apis.c | 4935 | t2_event_d |
| SYS_INFO_MTLS_enable | rdkfwupdater | src/rdkv_upgrade.c | 1028 | Upgradet2CountNotify→t2_event_d |
| SYS_INFO_MTLS_enable | rdkfwupdater | src/rdkv_upgrade.c | 1050 | Upgradet2CountNotify→t2_event_d |
| SYS_INFO_NoIPv6_Address | test-and-diagnostic | scripts/self_heal_connectivity_test.sh | 359 | t2CountNotify |
| SYS_INFO_NoIPv6_Address | test-and-diagnostic | scripts/self_heal_connectivity_test.sh | 414 | t2CountNotify |
| SYS_INFO_NoIPv6_Address | test-and-diagnostic | scripts/device/tccbr/self_heal_connectivity_test.sh | 291 | t2CountNotify |
| SYS_INFO_NTP_SYNC_split | utopia | source/scripts/init/service.d/service_ntpd.sh | 279 | t2ValNotify |
| SYS_INFO_NTPDELAY_split | sysint-broadband | ntp-data-collector.sh | 25 | t2ValNotify |
| SYS_INFO_NTPSTART_split | utopia | source/scripts/init/service.d/service_ntpd.sh | 632 | t2ValNotify |
| SYS_INFO_S3CoreUploaded | crashupload | c_sourcecode/src/upload/upload.c | 297 | t2CountNotify→t2_event_d |
| SYS_INFO_sameSSID | test-and-diagnostic | scripts/getSsidNames.sh | 93 | t2CountNotify |
| SYS_INFO_sameSSID | test-and-diagnostic | scripts/getSsidNames.sh | 115 | t2CountNotify |
| SYS_INFO_SETSYSTIME_split ⚠️ | test-and-diagnostic | source/TandDSsp/current_time.c | 181 | t2_event_s |
| SYS_INFO_SETSYSTIME_split ⚠️ | utopia | source/scripts/init/service.d/service_systemtimeset.sh | 44 | t2ValNotify |
| SYS_INFO_snmp_subagent_restart | test-and-diagnostic | scripts/corrective_action.sh | 745 | t2CountNotify |
| SYS_INFO_snmp_subagent_restart | test-and-diagnostic | scripts/corrective_action.sh | 756 | t2CountNotify |
| SYS_INFO_snmpsubagent_restart | provisioning-and-management | arch/intel_usg/boards/arm_shared/scripts/process_monitor.sh | 225 | t2CountNotify |
| SYS_INFO_snmpsubagent_restart | provisioning-and-management | arch/intel_usg/boards/arm_shared/scripts/systemd/sysd_process_monitor.sh | 72 | t2CountNotify |
| SYS_INFO_StaticIP_setMso | utopia | source/service_dhcp/service_ipv4.c | 940 | t2_event_d |
| SYS_INFO_StaticIP_setMso | utopia | source/scripts/init/service.d/service_ipv4.sh | 578 | t2CountNotify |
| SYS_INFO_StaticIP_setMso | utopia | source/scripts/init/service.d/service_ipv4_bci.sh | 477 | t2CountNotify |
| SYS_INFO_SW_upgrade_reboot ⚠️ | meta-rdk-qcom | meta-cmf-ipq/recipes-ccsp/ccsp/sysint/rdkbLogMonitor.sh | 879 | t2CountNotify |
| SYS_INFO_SW_upgrade_reboot ⚠️ | sysint-broadband | rdkbLogMonitor_cron.sh | 910 | t2CountNotify |
| SYS_INFO_SW_upgrade_reboot ⚠️ | sysint-broadband | rdkbLogMonitor.sh | 836 | t2CountNotify |
| SYS_INFO_swdltriggered | rdkfwupdater | src/rdkv_upgrade.c | 434 | Upgradet2CountNotify→t2_event_d |
| SYS_INFO_swdltriggered | rdkfwupdater | src/rdkv_upgrade.c | 439 | Upgradet2CountNotify→t2_event_d |
| SYS_INFO_SYSBUILD | test-and-diagnostic | source/TandDSsp/current_time.c | 274 | t2_event_d |
| SYS_INFO_SYSCFG_get_passed | mesh-agent | source/MeshAgentSsp/cosa_mesh_apis.c | 5157 | t2_event_d |
| SYS_INFO_SYSLKG_split | test-and-diagnostic | source/TandDSsp/current_time.c | 334 | t2_event_s |
| SYS_INFO_TGZDUMP | crashupload | c_sourcecode/src/scanner/scanner.c | 489 | t2CountNotify→t2_event_d |
| SYS_INFO_TGZDUMP | crashupload | runDumpUpload.sh | 792 | t2CountNotify |
| SYS_INFO_TGZDUMP | crashupload | uploadDumps_TestCases.md | 1641 | t2CountNotify |
| SYS_INFO_WaitingFor_Stack_Init ⚠️ | meta-rdk-qcom | meta-cmf-ipq/recipes-ccsp/ccsp/sysint/rdkbLogMonitor.sh | 336 | t2CountNotify |
| SYS_INFO_WaitingFor_Stack_Init ⚠️ | meta-rdk-qcom | meta-cmf-ipq/recipes-ccsp/ccsp/sysint/rdkbLogMonitor.sh | 546 | t2CountNotify |
| SYS_INFO_WaitingFor_Stack_Init ⚠️ | meta-rdk-qcom | meta-cmf-ipq/recipes-ccsp/ccsp/sysint/rdkbLogMonitor.sh | 665 | t2CountNotify |
| SYS_INFO_WaitingFor_Stack_Init ⚠️ | sysint-broadband | rdkbLogMonitor_cron.sh | 318 | t2CountNotify |
| SYS_INFO_WaitingFor_Stack_Init ⚠️ | sysint-broadband | rdkbLogMonitor_cron.sh | 490 | t2CountNotify |
| SYS_INFO_WaitingFor_Stack_Init ⚠️ | sysint-broadband | rdkbLogMonitor.sh | 249 | t2CountNotify |
| SYS_INFO_WaitingFor_Stack_Init ⚠️ | sysint-broadband | rdkbLogMonitor.sh | 463 | t2CountNotify |
| SYS_INFO_WaitingFor_Stack_Init ⚠️ | sysint-broadband | rdkbLogMonitor.sh | 584 | t2CountNotify |
| SYS_INFO_WanManager_HW_reconfigure_reboot | test-and-diagnostic | scripts/task_health_monitor.sh | 3902 | t2CountNotify |
| SYS_INFO_XDNS_RESOLV_CONF_SYNC | xdns | source/dmlxdns/cosa_xdns_apis.c | 493 | t2_event_d |
| SYS_INVALID_PARTNER_ID_RECOVERY_SUCCESS | utopia | source/scripts/init/src/apply_system_defaults_helper.c | 1092 | t2_event_d |
| SYS_INVALID_PARTNER_ID_RECOVERY_SUCCESS | utopia | source/scripts/init/src/apply_system_defaults/apply_system_defaults.c | 851 | t2_event_d |
| SYS_SH_akerCrash | test-and-diagnostic | scripts/task_health_monitor.sh | 2312 | t2CountNotify |
| SYS_SH_brlan0_restarted | test-and-diagnostic | scripts/selfheal_aggressive.sh | 505 | t2CountNotify |
| SYS_SH_brlan0_restarted | test-and-diagnostic | scripts/selfheal_aggressive.sh | 566 | t2CountNotify |
| SYS_SH_brlan0_restarted | test-and-diagnostic | scripts/task_health_monitor.sh | 2785 | t2CountNotify |
| SYS_SH_brlan0_restarted | test-and-diagnostic | scripts/task_health_monitor.sh | 2846 | t2CountNotify |
| SYS_SH_CMReset_PingFailed ⚠️ | telemetry | source/testApp/testCommonLibApi.c | 71 | t2_event_s |
| SYS_SH_CMReset_PingFailed ⚠️ | test-and-diagnostic | scripts/corrective_action.sh | 428 | t2CountNotify |
| SYS_SH_CUJO_restart | test-and-diagnostic | scripts/corrective_action.sh | 915 | t2CountNotify |
| SYS_SH_DhcpArp_restart | provisioning-and-management | arch/intel_usg/boards/arm_shared/scripts/process_monitor.sh | 247 | t2CountNotify |
| SYS_SH_DhcpArp_restart | provisioning-and-management | arch/intel_usg/boards/arm_shared/scripts/process_monitor.sh | 274 | t2CountNotify |
| SYS_SH_DhcpArp_restart | provisioning-and-management | arch/intel_usg/boards/arm_shared/scripts/systemd/sysd_process_monitor.sh | 94 | t2CountNotify |
| SYS_SH_DhcpArpProcess_restart | test-and-diagnostic | scripts/task_health_monitor.sh | 1762 | t2CountNotify |
| SYS_SH_DhcpSnooper_restart | provisioning-and-management | arch/intel_usg/boards/arm_shared/scripts/process_monitor.sh | 268 | t2CountNotify |
| SYS_SH_Dibbler_restart | test-and-diagnostic | scripts/selfheal_aggressive.sh | 722 | t2CountNotify |
| SYS_SH_Dibbler_restart | test-and-diagnostic | scripts/selfheal_aggressive.sh | 801 | t2CountNotify |
| SYS_SH_Dibbler_restart | test-and-diagnostic | scripts/task_health_monitor.sh | 3956 | t2CountNotify |
| SYS_SH_Dibbler_restart | test-and-diagnostic | scripts/task_health_monitor.sh | 4021 | t2CountNotify |
| SYS_SH_dnsmasq_restart ⚠️ | test-and-diagnostic | scripts/selfheal_bootup.sh | 735 | t2CountNotify |
| SYS_SH_dnsmasq_restart ⚠️ | test-and-diagnostic | scripts/selfheal_aggressive.sh | 253 | t2CountNotify |
| SYS_SH_dnsmasq_restart ⚠️ | test-and-diagnostic | scripts/task_health_monitor.sh | 3663 | t2CountNotify |
| SYS_SH_dnsmasq_restart ⚠️ | test-and-diagnostic | scripts/task_health_monitor.sh | 3744 | t2CountNotify |
| SYS_SH_dnsmasq_restart ⚠️ | test-and-diagnostic | scripts/device/tccbr/selfheal_bootup.sh | 520 | t2CountNotify |
| SYS_SH_dnsmasq_restart ⚠️ | utopia | source/pmon/pmon.c | 183 | t2_event_d |
| SYS_SH_Dropbear_restart | test-and-diagnostic | scripts/selfheal_aggressive.sh | 1378 | t2CountNotify |
| SYS_SH_Dropbear_restart | test-and-diagnostic | scripts/selfheal_aggressive.sh | 1389 | t2CountNotify |
| SYS_SH_Dropbear_restart | test-and-diagnostic | scripts/task_health_monitor.sh | 2335 | t2CountNotify |
| SYS_SH_FirewallRecovered | utopia | source/scripts/init/service.d/misc_handler.sh | 60 | t2CountNotify |
| SYS_SH_HomeSecurity_restart | test-and-diagnostic | scripts/task_health_monitor.sh | 1480 | t2CountNotify |
| SYS_SH_HomeSecurity_restart | test-and-diagnostic | scripts/task_health_monitor.sh | 1486 | t2CountNotify |
| SYS_SH_lighttpdCrash ⚠️ | telemetry | source/testApp/testCommonLibApi.c | 77 | t2_event_d |
| SYS_SH_lighttpdCrash ⚠️ | test-and-diagnostic | scripts/task_health_monitor.sh | 2194 | t2CountNotify |
| SYS_SH_lighttpdCrash ⚠️ | test-and-diagnostic | scripts/task_health_monitor.sh | 2395 | t2CountNotify |
| SYS_SH_LM_restart ⚠️ | provisioning-and-management | arch/intel_usg/boards/arm_shared/scripts/process_monitor.sh | 213 | t2CountNotify |
| SYS_SH_LM_restart ⚠️ | test-and-diagnostic | scripts/task_health_monitor.sh | 1250 | t2CountNotify |
| SYS_SH_LM_restart ⚠️ | test-and-diagnostic | scripts/task_health_monitor.sh | 1673 | t2CountNotify |
| SYS_SH_MOCA_add_brlan0 | test-and-diagnostic | scripts/task_health_monitor.sh | 1008 | t2CountNotify |
| SYS_SH_MOCA_add_brlan10 | test-and-diagnostic | scripts/task_health_monitor.sh | 1020 | t2CountNotify |
| SYS_SH_MTA_restart | test-and-diagnostic | scripts/task_health_monitor.sh | 1186 | t2CountNotify |
| SYS_SH_MTA_restart | test-and-diagnostic | scripts/task_health_monitor.sh | 1595 | t2CountNotify |
| SYS_SH_PAM_CRASH_RESTART | test-and-diagnostic | scripts/selfheal_bootup.sh | 552 | t2CountNotify |
| SYS_SH_PAM_CRASH_RESTART | test-and-diagnostic | scripts/task_health_monitor.sh | 1173 | t2CountNotify |
| SYS_SH_PAM_CRASH_RESTART | test-and-diagnostic | scripts/task_health_monitor.sh | 1586 | t2CountNotify |
| SYS_SH_PAM_CRASH_RESTART | test-and-diagnostic | scripts/device/tccbr/selfheal_bootup.sh | 424 | t2CountNotify |
| SYS_SH_PAM_restart | test-and-diagnostic | scripts/corrective_action.sh | 767 | t2CountNotify |
| SYS_SH_Parodus_Killed | test-and-diagnostic | scripts/task_health_monitor.sh | 2285 | t2CountNotify |
| SYS_SH_Parodus_Killed | test-and-diagnostic | scripts/task_health_monitor.sh | 2296 | t2CountNotify |
| SYS_SH_pingPeerIP_Failed | test-and-diagnostic | scripts/selfheal_bootup.sh | 426 | t2CountNotify |
| SYS_SH_pingPeerIP_Failed | test-and-diagnostic | scripts/selfheal_bootup.sh | 430 | t2CountNotify |
| SYS_SH_pingPeerIP_Failed | test-and-diagnostic | scripts/selfheal_aggressive.sh | 215 | t2CountNotify |
| SYS_SH_pingPeerIP_Failed | test-and-diagnostic | scripts/selfheal_aggressive.sh | 219 | t2CountNotify |
| SYS_SH_pingPeerIP_Failed | test-and-diagnostic | scripts/task_health_monitor.sh | 765 | t2CountNotify |
| SYS_SH_pingPeerIP_Failed | test-and-diagnostic | scripts/task_health_monitor.sh | 773 | t2CountNotify |
| SYS_SH_pingPeerIP_Failed | test-and-diagnostic | scripts/task_health_monitor.sh | 848 | t2CountNotify |
| SYS_SH_pingPeerIP_Failed | test-and-diagnostic | scripts/task_health_monitor.sh | 852 | t2CountNotify |
| SYS_SH_pingPeerIP_Failed | test-and-diagnostic | scripts/task_health_monitor.sh | 4696 | t2CountNotify |
| SYS_SH_pingPeerIP_Failed | test-and-diagnostic | scripts/task_health_monitor.sh | 4699 | t2CountNotify |
| SYS_SH_pingPeerIP_Failed | test-and-diagnostic | scripts/device/tccbr/selfheal_bootup.sh | 319 | t2CountNotify |
| SYS_SH_pingPeerIP_Failed | test-and-diagnostic | scripts/device/tccbr/selfheal_bootup.sh | 323 | t2CountNotify |
| SYS_SH_PSMHung | test-and-diagnostic | scripts/task_health_monitor.sh | 1152 | t2CountNotify |
| SYS_SH_PSMProcess_restart | test-and-diagnostic | scripts/corrective_action.sh | 834 | t2CountNotify |
| SYS_SH_RDKB_FIREWALL_RESTART | utopia | source/service_routed/service_routed.c | 237 | t2_event_d |
| SYS_SH_RDKB_FIREWALL_RESTART | utopia | source/service_dhcp/service_ipv4.c | 1559 | t2_event_d |
| SYS_SH_RDKB_FIREWALL_RESTART | utopia | source/service_dhcp/lan_handler.c | 692 | t2_event_d |
| SYS_SH_RDKB_FIREWALL_RESTART | utopia | source/service_dhcp/lan_handler.c | 789 | t2_event_d |
| SYS_SH_RDKB_FIREWALL_RESTART | utopia | source/utapi/lib/utapi.c | 2805 | t2_event_d |
| SYS_SH_RDKB_FIREWALL_RESTART | utopia | source/service_wan/service_wan.c | 1118 | t2_event_d |
| SYS_SH_RDKB_FIREWALL_RESTART | utopia | source/service_wan/service_wan.c | 1854 | t2_event_d |
| SYS_SH_RDKB_FIREWALL_RESTART | utopia | source/service_multinet/ev_access.c | 213 | t2_event_d |
| SYS_SH_RDKB_FIREWALL_RESTART | utopia | source/trigger/trigger.c | 461 | t2_event_d |
| SYS_SH_RDKB_FIREWALL_RESTART | utopia | source/trigger/trigger.c | 564 | t2_event_d |
| SYS_SH_RDKB_FIREWALL_RESTART | utopia | source/trigger/trigger.c | 705 | t2_event_d |
| SYS_SH_RDKB_FIREWALL_RESTART | utopia | source/trigger/trigger.c | 720 | t2_event_d |
| SYS_SH_RDKB_FIREWALL_RESTART | utopia | source/trigger/trigger.c | 826 | t2_event_d |
| SYS_SH_RDKB_FIREWALL_RESTART | utopia | source/scripts/init/service.d/service_lan.sh | 687 | t2CountNotify |
| SYS_SH_RDKB_FIREWALL_RESTART | utopia | source/scripts/init/service.d/iot_service.sh | 80 | t2CountNotify |
| SYS_SH_RDKB_FIREWALL_RESTART | utopia | source/scripts/init/service.d/service_wan/l2tp_wan.sh | 121 | t2CountNotify |
| SYS_SH_RDKB_FIREWALL_RESTART | utopia | source/scripts/init/service.d/service_wan/l2tp_wan.sh | 207 | t2CountNotify |
| SYS_SH_RDKB_FIREWALL_RESTART | utopia | source/scripts/init/service.d/service_wan/telstra_wan.sh | 99 | t2CountNotify |
| SYS_SH_RDKB_FIREWALL_RESTART | utopia | source/scripts/init/service.d/service_wan/static_wan.sh | 70 | t2CountNotify |
| SYS_SH_RDKB_FIREWALL_RESTART | utopia | source/scripts/init/service.d/service_wan/static_wan.sh | 98 | t2CountNotify |
| SYS_SH_RDKB_FIREWALL_RESTART | utopia | source/scripts/init/service.d/service_wan/pptp_wan.sh | 139 | t2CountNotify |
| SYS_SH_RDKB_FIREWALL_RESTART | utopia | source/scripts/init/service.d/service_wan/pptp_wan.sh | 202 | t2CountNotify |
| SYS_SH_RDKB_FIREWALL_RESTART | utopia | source/scripts/init/service.d/service_wan/dhcp_wan.sh | 72 | t2CountNotify |
| SYS_SH_RDKB_FIREWALL_RESTART | utopia | source/scripts/init/service.d/service_wan/dhcp_wan.sh | 120 | t2CountNotify |
| SYS_SH_RDKB_FIREWALL_RESTART | utopia | source/scripts/init/service.d/service_wan/pppoe_wan.sh | 139 | t2CountNotify |
| SYS_SH_RDKB_FIREWALL_RESTART | utopia | source/scripts/init/service.d/service_multinet/handle_gre.sh | 912 | t2CountNotify |
| SYS_SH_RDKB_FIREWALL_RESTART | utopia | source/scripts/init/service.d/service_multinet/handle_gre.sh | 1187 | t2CountNotify |
| SYS_SH_RDKB_FIREWALL_RESTART | utopia | source/scripts/init/service.d/service_lan/dhcp_lan.sh | 416 | t2CountNotify |
| SYS_SH_ResourceMonitor_restart | test-and-diagnostic | scripts/syscfg_recover.sh | 83 | t2CountNotify |
| SYS_SH_ResourceMonitor_restart | test-and-diagnostic | scripts/resource_monitor_recover.sh | 39 | t2CountNotify |
| SYS_SH_SERestart | test-and-diagnostic | scripts/selfheal_aggressive.sh | 1638 | t2CountNotify |
| SYS_SH_SERestart | test-and-diagnostic | scripts/task_health_monitor.sh | 492 | t2CountNotify |
| SYS_SH_SNMP_NotRunning | test-and-diagnostic | scripts/task_health_monitor.sh | 1289 | t2CountNotify |
| SYS_SH_SNMP_NotRunning | test-and-diagnostic | scripts/task_health_monitor.sh | 1733 | t2CountNotify |
| SYS_SH_TR69Restart | test-and-diagnostic | scripts/task_health_monitor.sh | 1233 | t2CountNotify |
| SYS_SH_WebPA_restart ⚠️ | OneWifi | scripts/process_monitor_atom.sh | 870 | t2CountNotify |
| SYS_SH_WebPA_restart ⚠️ | provisioning-and-management | arch/intel_usg/boards/arm_shared/scripts/process_monitor.sh | 294 | t2CountNotify |
| SYS_SH_WebPA_restart ⚠️ | provisioning-and-management | arch/intel_usg/boards/arm_shared/scripts/systemd/sysd_process_monitor.sh | 112 | t2CountNotify |
| SYS_SH_WIFIAGENT_restart | test-and-diagnostic | scripts/corrective_action.sh | 779 | t2CountNotify |
| SYS_SH_XDNS_dnsoverride_miss_restart | test-and-diagnostic | scripts/task_health_monitor.sh | 1359 | t2CountNotify |
| SYS_SH_XDNS_dnsoverride_miss_restart | test-and-diagnostic | scripts/task_health_monitor.sh | 1709 | t2CountNotify |
| SYS_SH_XDNS_dnsoverride_populate_fail | test-and-diagnostic | scripts/task_health_monitor.sh | 1365 | t2CountNotify |
| SYS_SH_XDNS_dnsoverride_populate_fail | test-and-diagnostic | scripts/task_health_monitor.sh | 1715 | t2CountNotify |
| SYS_SH_Zebra_restart | test-and-diagnostic | scripts/task_health_monitor.sh | 4116 | t2CountNotify |
| SYS_WARN_connectivitycheck_nourl_set | utopia | source/scripts/init/service.d/service_connectivitycheck.sh | 64 | t2CountNotify |
| SYS_WARN_connectivitycheck_time_expire | utopia | source/scripts/init/service.d/service_connectivitycheck.sh | 84 | t2CountNotify |
| syscfg_partner_split | sysint-broadband | log_factoryPartnerId.sh | 43 | t2ValNotify |
| SYST_ERR_ | sysint | lib/rdk/core_shell.sh | 132 | t2CountNotify |
| SYST_ERR_10Times_reboot | sysint | lib/rdk/update_previous_reboot_info.sh | 127 | t2CountNotify |
| SYST_ERR_10Times_reboot | sysint | lib/rdk/update_previous_reboot_info.sh | 140 | t2CountNotify |
| SYST_ERR_CCNotRepsonding_reboot | sysint | lib/rdk/rebootNow.sh | 140 | t2CountNotify |
| SYST_ERR_cdl_ssr | rdkfwupdater | src/rdkv_upgrade.c | 153 | Upgradet2CountNotify→t2_event_d |
| SYST_ERR_CDLFail ⚠️ | rdkfwupdater | src/rdkv_upgrade.c | 594 | Upgradet2CountNotify→t2_event_d |
| SYST_ERR_CDLFail ⚠️ | rdkfwupdater | src/rdkv_upgrade.c | 646 | Upgradet2CountNotify→t2_event_d |
| SYST_ERR_CDLFail ⚠️ | sysint | lib/rdk/userInitiatedFWDnld.sh | 661 | t2CountNotify |
| SYST_ERR_CECBusEx | hdmicec | ccec/src/MessageDecoder.cpp | 194 | t2_event_s |
| SYST_ERR_CompFail | crashupload | runDumpUpload.sh | 1021 | t2CountNotify |
| SYST_ERR_COREGZIP | sysint | lib/rdk/core_shell.sh | 177 | t2CountNotify |
| SYST_ERR_Curl28 ⚠️ | dcm-agent | uploadstblogs/src/path_handler.c | 216 | t2_count_notify→t2_event_d |
| SYST_ERR_Curl28 ⚠️ | dcm-agent | uploadstblogs/src/path_handler.c | 344 | t2_count_notify→t2_event_d |
| SYST_ERR_Curl28 ⚠️ | dcm-agent | uploadstblogs/src/path_handler.c | 433 | t2_count_notify→t2_event_d |
| SYST_ERR_Curl28 ⚠️ | dcm-agent | uploadstblogs/src/path_handler.c | 520 | t2_count_notify→t2_event_d |
| SYST_ERR_Curl28 ⚠️ | sysint | lib/rdk/uploadSTBLogs.sh | 370 | t2CountNotify |
| SYST_ERR_Cyclic_reboot | sysint | lib/rdk/rebootNow.sh | 279 | t2CountNotify |
| SYST_ERR_DiffFWCTN_FLdnld | rdkfwupdater | src/chunk.c | 175 | t2CountNotify→t2_event_d |
| SYST_ERR_DNSFileEmpty | sysint | lib/rdk/networkConnectionRecovery.sh | 339 | t2CountNotify |
| SYST_ERR_DSMGR_reboot | sysint | lib/rdk/rebootNow.sh | 152 | t2CountNotify |
| SYST_ERR_FW_RFC_disabled | sysint | lib/rdk/userInitiatedFWDnld.sh | 694 | t2CountNotify |
| SYST_ERR_FWCTNFetch | rdkfwupdater | src/chunk.c | 114 | t2CountNotify→t2_event_d |
| SYST_ERR_FWdnldFail | sysint | lib/rdk/userInitiatedFWDnld.sh | 401 | t2ValNotify |
| SYST_ERR_IARMDEMON_reboot | sysint | lib/rdk/rebootNow.sh | 155 | t2CountNotify |
| SYST_ERR_imageflsfail | rdkfwupdater | src/flash.c | 141 | flashT2CountNotify→t2_event_d |
| SYST_ERR_LogUpload_Failed ⚠️ | dcm-agent | uploadstblogs/src/event_manager.c | 166 | t2_count_notify→t2_event_d |
| SYST_ERR_LogUpload_Failed ⚠️ | dcm-agent | uploadstblogs/src/path_handler.c | 552 | t2_count_notify→t2_event_d |
| SYST_ERR_LogUpload_Failed ⚠️ | sysint | lib/rdk/uploadSTBLogs.sh | 657 | t2CountNotify |
| SYST_ERR_LogUpload_Failed ⚠️ | sysint | lib/rdk/uploadSTBLogs.sh | 780 | t2CountNotify |
| SYST_ERR_LogUpload_Failed ⚠️ | sysint | lib/rdk/uploadSTBLogs.sh | 871 | t2CountNotify |
| SYST_ERR_MaintNetworkFail | entservices-softwareupdate | MaintenanceManager/MaintenanceManager.cpp | 455 | t2_event_d |
| SYST_ERR_MINIDPZEROSIZE | crashupload | c_sourcecode/src/archive/archive.c | 306 | t2CountNotify→t2_event_d |
| SYST_ERR_MINIDPZEROSIZE | crashupload | runDumpUpload.sh | 979 | t2CountNotify |
| SYST_ERR_OPTFULL | sysint | lib/rdk/disk_threshold_check.sh | 354 | t2CountNotify |
| SYST_ERR_OverflowMon_crash | sysint | lib/rdk/core_shell.sh | 88 | t2CountNotify |
| SYST_ERR_PC_Conn169 | sysint | lib/rdk/core_shell.sh | 91 | t2CountNotify |
| SYST_ERR_PC_MAF | sysint | lib/rdk/core_shell.sh | 94 | t2CountNotify |
| SYST_ERR_PC_RBI | sysint | lib/rdk/core_shell.sh | 97 | t2CountNotify |
| SYST_ERR_PC_Systemd | sysint | lib/rdk/core_shell.sh | 100 | t2CountNotify |
| SYST_ERR_PC_TTSEngine | sysint | lib/rdk/core_shell.sh | 103 | t2CountNotify |
| SYST_ERR_PDRI_VFail | sysint | lib/rdk/xconfImageCheck.sh | 453 | t2CountNotify |
| SYST_ERR_PDRIUpg_failure | rdkfwupdater | src/rdkv_upgrade.c | 589 | Upgradet2CountNotify→t2_event_d |
| SYST_ERR_PrevCDL_InProg | sysint | lib/rdk/swupdate_utility.sh | 157 | t2CountNotify |
| SYST_ERR_Process_Crash_accum | crashupload | c_sourcecode/src/scanner/scanner.c | 368 | t2ValNotify→t2_event_s |
| SYST_ERR_Process_Crash_accum | crashupload | runDumpUpload.sh | 760 | t2ValNotify |
| SYST_ERR_Process_Crash_accum | crashupload | uploadDumps_TestCases.md | 1589 | t2ValNotify |
| SYST_ERR_ProcessCrash ⚠️ | crashupload | c_sourcecode/src/scanner/scanner.c | 369 | t2CountNotify→t2_event_d |
| SYST_ERR_ProcessCrash ⚠️ | crashupload | runDumpUpload.sh | 761 | t2CountNotify |
| SYST_ERR_ProcessCrash ⚠️ | crashupload | uploadDumps_TestCases.md | 1590 | t2CountNotify |
| SYST_ERR_ProcessCrash ⚠️ | sysint | lib/rdk/core_shell.sh | 81 | t2CountNotify |
| SYST_ERR_RDMMISSING | rdm-agent | src/rdm_downloadutils.c | 118 | t2ValNotify→t2_event_s |
| SYST_ERR_RFC | entservices-softwareupdate | MaintenanceManager/MaintenanceManager.cpp | 1652 | t2_event_d |
| SYST_ERR_Rmfstreamer_crash | sysint | lib/rdk/core_shell.sh | 128 | t2CountNotify |
| SYST_ERR_Rmfstreamer_reboot | sysint | lib/rdk/rebootNow.sh | 158 | t2CountNotify |
| SYST_ERR_RunPod_reboot | sysint | lib/rdk/rebootNow.sh | 137 | t2CountNotify |
| SYST_ERR_RunPod_reboot | sysint | lib/rdk/rebootNow.sh | 161 | t2CountNotify |
| SYST_ERR_syslogng_crash | sysint | lib/rdk/core_shell.sh | 109 | t2CountNotify |
| SYST_ERR_VodApp_restart | sysint | lib/rdk/core_shell.sh | 106 | t2CountNotify |
| SYST_ERR_XCALDevice_crash | sysint | lib/rdk/core_shell.sh | 124 | t2CountNotify |
| SYST_ERR_Xconf28 | sysint | lib/rdk/xconfImageCheck.sh | 512 | t2CountNotify |
| SYST_ERR_Xconf28 | sysint | lib/rdk/xconfImageCheck.sh | 561 | t2CountNotify |
| SYST_ERR_xraudio_crash | sysint | lib/rdk/core_shell.sh | 112 | t2CountNotify |
| SYST_ERROR_WAI_InitERR | entservices-softwareupdate | MaintenanceManager/MaintenanceManager.cpp | 648 | t2_event_d |
| SYST_INFO_C_CDL | rdkfwupdater | src/rdkFwupdateMgr.c | 1132 | t2CountNotify→t2_event_d |
| SYST_INFO_C_CDL | rdkfwupdater | src/rdkv_main.c | 1090 | t2CountNotify→t2_event_d |
| SYST_INFO_cb_xconf ⚠️ | rdkfwupdater | src/rdkv_upgrade.c | 733 | Upgradet2CountNotify→t2_event_d |
| SYST_INFO_cb_xconf ⚠️ | sysint | lib/rdk/xconfImageCheck.sh | 591 | t2CountNotify |
| SYST_INFO_cb_xconf ⚠️ | sysint | lib/rdk/xconfImageCheck.sh | 667 | t2CountNotify |
| SYST_INFO_CDLSuccess ⚠️ | rdkfwupdater | src/flash.c | 137 | flashT2CountNotify→t2_event_d |
| SYST_INFO_CDLSuccess ⚠️ | sysint | lib/rdk/userInitiatedFWDnld.sh | 664 | t2CountNotify |
| SYST_INFO_Core_accum | sysint | lib/rdk/core_shell.sh | 166 | t2ValNotify |
| SYST_INFO_CoreFull_accum | sysint | lib/rdk/core_shell.sh | 157 | t2ValNotify |
| SYST_INFO_CoreIMP_accum | sysint | lib/rdk/core_shell.sh | 158 | t2ValNotify |
| SYST_INFO_CoreNotProcessed | sysint | lib/rdk/core_shell.sh | 365 | t2CountNotify |
| SYST_INFO_CoreProcessed | sysint | lib/rdk/core_shell.sh | 184 | t2CountNotify |
| SYST_INFO_CoreProcessed_accum | sysint | lib/rdk/core_shell.sh | 185 | t2ValNotify |
| SYST_INFO_CoreUpldSkipped | crashupload | c_sourcecode/src/utils/system_utils.c | 145 | t2CountNotify→t2_event_d |
| SYST_INFO_CoreUpldSkipped | crashupload | runDumpUpload.sh | 662 | t2CountNotify |
| SYST_INFO_CrashedProc_accum | sysint | lib/rdk/core_shell.sh | 150 | t2ValNotify |
| SYST_INFO_CURL6 | crashupload | c_sourcecode/src/upload/upload.c | 278 | t2CountNotify→t2_event_d |
| SYST_INFO_ETHConn | sysint | lib/rdk/networkConnectionRecovery.sh | 162 | t2CountNotify |
| SYST_INFO_FetchFWCTN | rdkfwupdater | src/chunk.c | 95 | t2CountNotify→t2_event_d |
| SYST_INFO_FWCOMPLETE ⚠️ | rdkfwupdater | src/rdkv_upgrade.c | 605 | Upgradet2CountNotify→t2_event_d |
| SYST_INFO_FWCOMPLETE ⚠️ | sysint | lib/rdk/userInitiatedFWDnld.sh | 423 | t2CountNotify |
| SYST_INFO_FWUpgrade_Exit ⚠️ | rdkfwupdater | src/device_status_helper.c | 80 | t2CountNotify→t2_event_d |
| SYST_INFO_FWUpgrade_Exit ⚠️ | sysint | lib/rdk/swupdate_utility.sh | 130 | t2CountNotify |
| SYST_INFO_Http302 | rdkfwupdater | src/rdkv_upgrade.c | 156 | Upgradet2CountNotify→t2_event_d |
| SYST_INFO_ImgFlashOK | rdkfwupdater | src/flash.c | 163 | flashT2CountNotify→t2_event_d |
| SYST_INFO_JSPPShutdown | entservices-monitor | plugin/Monitor.h | 963 | t2_event_d |
| SYST_INFO_lu_success ⚠️ | dcm-agent | uploadstblogs/src/event_manager.c | 135 | t2_count_notify→t2_event_d |
| SYST_INFO_lu_success ⚠️ | sysint | lib/rdk/uploadSTBLogs.sh | 517 | t2CountNotify |
| SYST_INFO_LUattempt ⚠️ | dcm-agent | uploadstblogs/src/retry_logic.c | 55 | t2_count_notify→t2_event_d |
| SYST_INFO_LUattempt ⚠️ | sysint | lib/rdk/uploadSTBLogs.sh | 511 | t2CountNotify |
| SYST_INFO_MaintnceIncmpl | entservices-softwareupdate | MaintenanceManager/MaintenanceManager.cpp | 1929 | t2_event_d |
| SYST_INFO_MaintnceIncmpl | entservices-softwareupdate | MaintenanceManager/MaintenanceManager.cpp | 2745 | t2_event_d |
| SYST_INFO_MemAvailable_split | sysint | lib/rdk/system_info_collector.sh | 70 | t2ValNotify |
| SYST_INFO_minidumpUpld | crashupload | c_sourcecode/src/upload/upload.c | 429 | t2CountNotify→t2_event_d |
| SYST_INFO_minidumpUpld | crashupload | runDumpUpload.sh | 1141 | t2CountNotify |
| SYST_INFO_minidumpUpld | crashupload | uploadDumps_TestCases.md | 1795 | t2CountNotify |
| SYST_INFO_mtls_xpki ⚠️ | dcm-agent | uploadstblogs/src/path_handler.c | 100 | t2_count_notify→t2_event_d |
| SYST_INFO_mtls_xpki ⚠️ | sysint | lib/rdk/uploadSTBLogs.sh | 355 | t2CountNotify |
| SYST_INFO_NoConsentFlash | rdkfwupdater | src/rdkFwupdateMgr.c | 670 | t2CountNotify→t2_event_d |
| SYST_INFO_NoConsentFlash | rdkfwupdater | src/rdkv_main.c | 619 | t2CountNotify→t2_event_d |
| SYST_INFO_NotifyMotion | entservices-peripherals | MotionDetection/MotionDetection.cpp | 461 | t2_event_d |
| SYST_INFO_PartnerId | sysint | lib/rdk/getDeviceId.sh | 77 | t2ValNotify |
| SYST_INFO_PC_RF4CE | sysint | lib/rdk/core_shell.sh | 115 | t2CountNotify |
| SYST_INFO_PDRILogUpload ⚠️ | dcm-agent | uploadstblogs/src/strategies.c | 953 | t2_count_notify→t2_event_d |
| SYST_INFO_PDRILogUpload ⚠️ | sysint | lib/rdk/uploadSTBLogs.sh | 883 | t2CountNotify |
| SYST_INFO_PDRILogUpload ⚠️ | sysint | lib/rdk/uploadSTBLogs.sh | 886 | t2CountNotify |
| SYST_INFO_PDRIUpgSuccess | rdkfwupdater | src/rdkv_upgrade.c | 629 | Upgradet2CountNotify→t2_event_d |
| SYST_INFO_PRXR_Ver_split | rdkfwupdater | src/json_process.c | 281 | t2ValNotify→t2_event_s |
| SYST_INFO_RedStateRecovery | rdkfwupdater | src/rdkv_upgrade.c | 1058 | Upgradet2CountNotify→t2_event_d |
| SYST_INFO_RedstateSet | rdkfwupdater | src/device_status_helper.c | 370 | t2CountNotify→t2_event_d |
| SYST_INFO_SAME_FWCTN | rdkfwupdater | src/chunk.c | 101 | t2CountNotify→t2_event_d |
| SYST_INFO_SigDump_split | sysint | lib/rdk/core_shell.sh | 152 | t2ValNotify |
| SYST_INFO_SOMT | entservices-softwareupdate | MaintenanceManager/MaintenanceManager.cpp | 477 | t2_event_d |
| SYST_INFO_SwapCached_split | sysint | lib/rdk/system_info_collector.sh | 80 | t2ValNotify |
| SYST_INFO_SwapFree_split | sysint | lib/rdk/system_info_collector.sh | 86 | t2ValNotify |
| SYST_INFO_SwapTotal_split | sysint | lib/rdk/system_info_collector.sh | 83 | t2ValNotify |
| SYST_INFO_swdlSameImg ⚠️ | rdkfwupdater | src/device_status_helper.c | 912 | t2CountNotify→t2_event_d |
| SYST_INFO_swdlSameImg ⚠️ | sysint | lib/rdk/userInitiatedFWDnld.sh | 515 | t2CountNotify |
| SYST_INFO_SwdlSameImg_Stndby ⚠️ | rdkfwupdater | src/device_status_helper.c | 905 | t2CountNotify→t2_event_d |
| SYST_INFO_SwdlSameImg_Stndby ⚠️ | sysint | lib/rdk/userInitiatedFWDnld.sh | 519 | t2CountNotify |
| SYST_INFO_SWUpgrdChck | rdkfwupdater | src/rdkFwupdateMgr.c | 1181 | t2CountNotify→t2_event_d |
| SYST_INFO_SWUpgrdChck | rdkfwupdater | src/rdkv_main.c | 1123 | t2CountNotify→t2_event_d |
| SYST_INFO_SYSBUILD | systemtimemgr | systimerfactory/rdkdefaulttimesync.cpp | 131 | t2CountNotify→t2_event_d |
| SYST_INFO_Thrtl_Enable | rdkfwupdater | src/rdkv_upgrade.c | 989 | Upgradet2CountNotify→t2_event_d |
| SYST_INFO_TLS_xconf | rdkfwupdater | src/rdkv_upgrade.c | 978 | Upgradet2CountNotify→t2_event_d |
| SYST_INFO_WIFIConn | sysint | lib/rdk/networkConnectionRecovery.sh | 144 | t2CountNotify |
| SYST_INFO_WIFIConn | sysint | lib/rdk/networkConnectionRecovery.sh | 155 | t2CountNotify |
| SYST_INFO_Xconf200 | sysint | lib/rdk/xconfImageCheck.sh | 514 | t2CountNotify |
| SYST_INFO_Xconf200 | sysint | lib/rdk/xconfImageCheck.sh | 563 | t2CountNotify |
| SYST_INFO_XCONFConnect ⚠️ | rdkfwupdater | src/rdkv_upgrade.c | 378 | Upgradet2CountNotify→t2_event_d |
| SYST_INFO_XCONFConnect ⚠️ | sysint | lib/rdk/xconfImageCheck.sh | 505 | t2CountNotify |
| SYST_SWDL_Retry_split | sysint | lib/rdk/userInitiatedFWDnld.sh | 598 | t2ValNotify |
| SYST_WARN_CompFail | crashupload | c_sourcecode/src/archive/archive.c | 414 | t2ValNotify→t2_event_s |
| SYST_WARN_CompFail | crashupload | runDumpUpload.sh | 1012 | t2CountNotify |
| SYST_WARN_CoreNP_accum | sysint | lib/rdk/core_shell.sh | 366 | t2ValNotify |
| SYST_WARN_dcm_curl28 | sysint | lib/rdk/xconfImageCheck.sh | 416 | t2CountNotify |
| SYST_WARN_GW100PERC_PACKETLOSS | sysint | lib/rdk/networkConnectionRecovery.sh | 249 | t2CountNotify |
| SYST_WARN_NoMinidump | crashupload | c_sourcecode/src/utils/lock_manager.c | 42 | t2CountNotify→t2_event_d |
| SYST_WARN_NoMinidump | crashupload | runDumpUpload.sh | 231 | t2CountNotify |
| SYST_WARN_UPGD_SKIP ⚠️ | rdkfwupdater | src/deviceutils/device_api.c | 921 | t2ValNotify→t2_event_s |
| SYST_WARN_UPGD_SKIP ⚠️ | sysint | lib/rdk/xconfImageCheck.sh | 158 | t2ValNotify |
| TEST_EVENT_1 | telemetry | source/testApp/testCommonLibApi.c | 79 | t2_event_d |
| TEST_EVENT_2 | telemetry | source/testApp/testCommonLibApi.c | 81 | t2_event_s |
| TEST_lu_success ⚠️ | dcm-agent | uploadstblogs/src/path_handler.c | 532 | t2_count_notify→t2_event_d |
| TEST_lu_success ⚠️ | sysint | lib/rdk/uploadSTBLogs.sh | 619 | t2CountNotify |
| TEST_lu_success ⚠️ | sysint | lib/rdk/uploadSTBLogs.sh | 648 | t2CountNotify |
| Test_StartEndEqual | xconf-client | scripts/cbr_firmwareDwnld.sh | 885 | t2CountNotify |
| Test_StartEndEqual | xconf-client | scripts/cbr_firmwareDwnld.sh | 1075 | t2CountNotify |
| Test_StartEndEqual | xconf-client | scripts/xb6_firmwareDwnld.sh | 985 | t2CountNotify |
| Test_StartEndEqual | xconf-client | scripts/xb6_firmwareDwnld.sh | 1180 | t2CountNotify |
| Test_StartEndEqual | xconf-client | scripts/xb3_firmwareDwnld.sh | 745 | t2CountNotify |
| Test_StartEndEqual | xconf-client | scripts/xb3_firmwareDwnld.sh | 933 | t2CountNotify |
| Test_StartEndEqual | xconf-client | scripts/xf3_firmwareDwnld.sh | 945 | t2CountNotify |
| Test_StartEndEqual | xconf-client | scripts/xf3_firmwareDwnld.sh | 1136 | t2CountNotify |
| Test_StartEndEqual | xconf-client | scripts/xb3_codebig_firmwareDwnld.sh | 961 | t2CountNotify |
| Test_StartEndEqual | xconf-client | scripts/xb3_codebig_firmwareDwnld.sh | 1150 | t2CountNotify |
| Test_SWReset | sysint | lib/rdk/update_previous_reboot_info.sh | 201 | t2CountNotify |
| TimeZone_split | sysint | lib/rdk/getTimeZone.sh | 69 | t2ValNotify |
| TMPFS_USAGE_ATOM | sysint-broadband | log_mem_cpu_info_atom.sh | 158 | t2ValNotify |
| TMPFS_USAGE_ATOM | sysint-broadband | log_mem_cpu_info_atom.sh | 169 | t2ValNotify |
| TMPFS_USAGE_ATOM_PERIODIC | sysint-broadband | log_mem_cpu_info_atom.sh | 154 | t2ValNotify |
| TMPFS_USAGE_PERIODIC | test-and-diagnostic | scripts/log_mem_cpu_info.sh | 59 | t2ValNotify |
| TMPFS_USE_PERCENTAGE_split | test-and-diagnostic | scripts/log_mem_cpu_info.sh | 73 | t2ValNotify |
| TopCPU_split | test-and-diagnostic | scripts/resource_monitor.sh | 174 | t2ValNotify |
| TopCPU_split | test-and-diagnostic | scripts/resource_monitor.sh | 289 | t2ValNotify |
| Total_2G_PodClients_split | OneWifi | scripts/mesh_status.sh | 36 | t2ValNotify |
| Total_5G_PodClients_split | OneWifi | scripts/mesh_status.sh | 38 | t2ValNotify |
| Total_devices_connected_split ⚠️ | lan-manager-lite | source/lm/lm_main.c | 2786 | t2_event_d |
| Total_devices_connected_split ⚠️ | meta-rdk-qcom (patch) | meta-cmf-ipq/recipes-ccsp/util/001-task_health.patch | 19 | t2ValNotify |
| Total_devices_connected_split ⚠️ | meta-rdk-qcom (patch) | meta-cmf-ipq/recipes-ccsp/util/001-task_health.patch | 32 | t2ValNotify |
| Total_Ethernet_Clients_split | lan-manager-lite | source/lm/lm_main.c | 2790 | t2_event_d |
| Total_MoCA_Clients_split | lan-manager-lite | source/lm/lm_main.c | 2792 | t2_event_d |
| Total_offline_clients_split ⚠️ | lan-manager-lite | source/lm/lm_main.c | 2788 | t2_event_d |
| Total_offline_clients_split ⚠️ | meta-rdk-qcom (patch) | meta-cmf-ipq/recipes-ccsp/util/001-task_health.patch | 18 | t2ValNotify |
| Total_offline_clients_split ⚠️ | meta-rdk-qcom (patch) | meta-cmf-ipq/recipes-ccsp/util/001-task_health.patch | 31 | t2ValNotify |
| Total_online_clients_split | lan-manager-lite | source/lm/lm_main.c | 2787 | t2_event_d |
| Total_wifi_clients_split ⚠️ | lan-manager-lite | source/lm/lm_main.c | 2789 | t2_event_d |
| Total_wifi_clients_split ⚠️ | meta-rdk-qcom (patch) | meta-cmf-ipq/recipes-ccsp/util/001-task_health.patch | 20 | t2ValNotify |
| Total_wifi_clients_split ⚠️ | meta-rdk-qcom (patch) | meta-cmf-ipq/recipes-ccsp/util/001-task_health.patch | 33 | t2ValNotify |
| TR69HOSTIF_GET_1000_WITHIN_5MIN | tr69hostif | src/hostif/handlers/src/hostIf_msgHandler.cpp | 171 | t2CountNotify→t2_event_d |
| TR69HOSTIF_GET_200_WITHIN_1MIN | tr69hostif | src/hostif/handlers/src/hostIf_msgHandler.cpp | 160 | t2CountNotify→t2_event_d |
| TR69HOSTIF_GET_TIMEOUT_PARAM | tr69hostif | src/hostif/handlers/src/hostIf_msgHandler.cpp | 205 | t2ValNotify→t2_event_s |
| TR69HOSTIF_SET_1000_WITHIN_5MIN | tr69hostif | src/hostif/handlers/src/hostIf_msgHandler.cpp | 260 | t2CountNotify→t2_event_d |
| TR69HOSTIF_SET_200_WITHIN_1MIN | tr69hostif | src/hostif/handlers/src/hostIf_msgHandler.cpp | 249 | t2CountNotify→t2_event_d |
| TR69HOSTIF_SET_TIMEOUT_PARAM | tr69hostif | src/hostif/handlers/src/hostIf_msgHandler.cpp | 291 | t2ValNotify→t2_event_s |
| TrueStatic_NotSupported | provisioning-and-management | source/TR-181/middle_layer_src/cosa_apis_util.c | 1885 | t2_event_d |
| TrueStatic_NotSupported | provisioning-and-management | source/TR-181/middle_layer_src/cosa_apis_util.c | 1891 | t2_event_d |
| TrueStatic_NotSupported | provisioning-and-management | source/TR-181/middle_layer_src/cosa_x_cisco_com_security_dml.c | 1775 | t2_event_d |
| UPDays_split | test-and-diagnostic | scripts/uptime.sh | 76 | t2ValNotify |
| USED_CPU_ATOM_split ⚠️ | sysint-broadband | log_mem_cpu_info_atom.sh | 129 | t2ValNotify |
| USED_CPU_ATOM_split ⚠️ | test-and-diagnostic | scripts/log_mem_cpu_info.sh | 224 | t2ValNotify |
| USED_MEM_ATOM_split ⚠️ | sysint-broadband | log_mem_cpu_info_atom.sh | 98 | t2ValNotify |
| USED_MEM_ATOM_split ⚠️ | test-and-diagnostic | scripts/log_mem_cpu_info.sh | 107 | t2ValNotify |
| UsedCPU_split | test-and-diagnostic | scripts/log_mem_cpu_info.sh | 222 | t2ValNotify |
| UsedMem_split | test-and-diagnostic | scripts/log_mem_cpu_info.sh | 105 | t2ValNotify |
| vmstats_split | sysint | lib/rdk/vm-statistics.sh | 32 | t2ValNotify |
| WAN_FAILOVER_FAIL_COUNT | wan-manager | source/WanManager/wanmgr_policy_autowan_impl.c | 4413 | t2_event_d |
| WAN_FAILOVER_FAIL_COUNT | wan-manager | source/WanManager/wanmgr_policy_autowan_impl.c | 4430 | t2_event_d |
| WAN_FAILOVER_FAIL_COUNT | wan-manager | source/WanManager/wanmgr_policy_autowan_impl.c | 4641 | t2_event_d |
| WAN_FAILOVER_FAIL_COUNT | wan-manager | source/WanManager/wanmgr_policy_autowan_impl.c | 4688 | t2_event_d |
| WAN_FAILOVER_FAIL_COUNT | wan-manager | source/WanManager/wanmgr_policy_autowan_impl.c | 4747 | t2_event_d |
| WAN_FAILOVER_SUCCESS_COUNT | wan-manager | source/WanManager/wanmgr_policy_autowan_impl.c | 4466 | t2_event_d |
| WAN_RESTORE_FAIL_COUNT | wan-manager | source/WanManager/wanmgr_policy_autowan_impl.c | 3110 | t2_event_d |
| WAN_RESTORE_FAIL_COUNT | wan-manager | source/WanManager/wanmgr_policy_autowan_impl.c | 3137 | t2_event_d |
| WAN_RESTORE_SUCCESS_COUNT | wan-manager | source/WanManager/wanmgr_policy_autowan_impl.c | 1401 | t2_event_d |
| WAN_RESTORE_SUCCESS_COUNT | wan-manager | source/WanManager/wanmgr_policy_autowan_impl.c | 3154 | t2_event_d |
| WAN_RESTORE_SUCCESS_COUNT | wan-manager | source/WanManager/wanmgr_policy_autowan_impl.c | 3417 | t2_event_d |
| Wifi_2G_utilization_split | meta-rdk-qcom | meta-cmf-ipq/recipes-ccsp/ccsp/ccsp-wifi-agent/radiohealth.sh | 246 | t2ValNotify |
| Wifi_2G_utilization_split | meta-rdk-qcom | meta-cmf-ipq/recipes-ccsp/ccsp/ccsp-wifi-agent/radiohealth.sh | 259 | t2ValNotify |
| Wifi_5G_utilization_split | meta-rdk-qcom | meta-cmf-ipq/recipes-ccsp/ccsp/ccsp-wifi-agent/radiohealth.sh | 245 | t2ValNotify |
| Wifi_5G_utilization_split | meta-rdk-qcom | meta-cmf-ipq/recipes-ccsp/ccsp/ccsp-wifi-agent/radiohealth.sh | 277 | t2ValNotify |
| WIFI_ERROR_atomConsoleDown_2G | test-and-diagnostic | scripts/getSsidNames.sh | 54 | t2CountNotify |
| WIFI_ERROR_atomConsoleDown_5G | test-and-diagnostic | scripts/getSsidNames.sh | 63 | t2CountNotify |
| WIFI_ERROR_atomConsoleDown_6G | test-and-diagnostic | scripts/getSsidNames.sh | 74 | t2CountNotify |
| WIFI_ERROR_DMCLI_crash_2G_Status | test-and-diagnostic | scripts/task_health_monitor.sh | 3420 | t2CountNotify |
| WIFI_ERROR_DMCLI_crash_5G_Status | test-and-diagnostic | scripts/task_health_monitor.sh | 3293 | t2CountNotify |
| WIFI_ERROR_MESH_FAILED | mesh-agent | source/MeshAgentSsp/cosa_mesh_apis.c | 2380 | t2_event_d |
| WIFI_ERROR_meshwifiservice_failure | mesh-agent | source/MeshAgentSsp/cosa_mesh_apis.c | 4852 | t2_event_d |
| WIFI_ERROR_NvramCorrupt ⚠️ | meta-rdk-qcom | meta-cmf-ipq/recipes-ccsp/ccsp/ccsp-wifi-agent/radiohealth.sh | 150 | t2CountNotify |
| WIFI_ERROR_NvramCorrupt ⚠️ | OneWifi | scripts/radiohealth.sh | 114 | t2CountNotify |
| WIFI_ERROR_PSM_GetRecordFail | telemetry | source/testApp/testCommonLibApi.c | 69 | t2_event_s |
| WIFI_ERROR_QTN_driver_not_loaded | utopia | source/scripts/init/service.d/vlan_util_xb6.sh | 137 | t2CountNotify |
| WIFI_ERROR_Wifi_query_timeout | test-and-diagnostic | scripts/task_health_monitor.sh | 629 | t2CountNotify |
| WIFI_ERROR_WifiDmCliError | test-and-diagnostic | scripts/task_health_monitor.sh | 3015 | t2CountNotify |
| WIFI_ERROR_WifiDmCliError | test-and-diagnostic | scripts/task_health_monitor.sh | 3045 | t2CountNotify |
| WIFI_ERROR_WifiDmCliError | test-and-diagnostic | scripts/task_health_monitor.sh | 3084 | t2CountNotify |
| WIFI_ERROR_WL0_NotFound ⚠️ | meta-rdk-qcom | meta-cmf-ipq/recipes-ccsp/ccsp/ccsp-wifi-agent/radiohealth.sh | 102 | t2CountNotify |
| WIFI_ERROR_WL0_NotFound ⚠️ | OneWifi | scripts/radiohealth.sh | 66 | t2CountNotify |
| WIFI_ERROR_WL0_SSIDEmpty ⚠️ | meta-rdk-qcom | meta-cmf-ipq/recipes-ccsp/ccsp/ccsp-wifi-agent/radiohealth.sh | 115 | t2CountNotify |
| WIFI_ERROR_WL0_SSIDEmpty ⚠️ | OneWifi | scripts/radiohealth.sh | 79 | t2CountNotify |
| WIFI_ERROR_WL1_NotFound ⚠️ | meta-rdk-qcom | meta-cmf-ipq/recipes-ccsp/ccsp/ccsp-wifi-agent/radiohealth.sh | 106 | t2CountNotify |
| WIFI_ERROR_WL1_NotFound ⚠️ | OneWifi | scripts/radiohealth.sh | 70 | t2CountNotify |
| WIFI_ERROR_WL1_SSIDEmpty ⚠️ | meta-rdk-qcom | meta-cmf-ipq/recipes-ccsp/ccsp/ccsp-wifi-agent/radiohealth.sh | 123 | t2CountNotify |
| WIFI_ERROR_WL1_SSIDEmpty ⚠️ | OneWifi | scripts/radiohealth.sh | 87 | t2CountNotify |
| WIFI_INFO_2G_DISABLED | test-and-diagnostic | scripts/task_health_monitor.sh | 3309 | t2CountNotify |
| WIFI_INFO_2GPrivateSSID_OFF | test-and-diagnostic | scripts/task_health_monitor.sh | 3402 | t2CountNotify |
| WIFI_INFO_5G_DISABLED | test-and-diagnostic | scripts/task_health_monitor.sh | 3009 | t2CountNotify |
| WIFI_INFO_5G_DISABLED | test-and-diagnostic | scripts/task_health_monitor.sh | 3039 | t2CountNotify |
| WIFI_INFO_5G_DISABLED | test-and-diagnostic | scripts/task_health_monitor.sh | 3078 | t2CountNotify |
| WIFI_INFO_5GPrivateSSID_OFF | test-and-diagnostic | scripts/task_health_monitor.sh | 3275 | t2CountNotify |
| WIFI_INFO_6G_DISABLED | test-and-diagnostic | scripts/task_health_monitor.sh | 3179 | t2CountNotify |
| WIFI_INFO_BSEnabled | OneWifi | scripts/bandsteering.sh | 44 | t2CountNotify |
| WIFI_INFO_clientdisconnect | lan-manager-lite | source/lm/lm_main.c | 649 | t2_event_d |
| WIFI_INFO_ClientTransitionToXfininityWifi | hotspot | source/hotspotfd/dhcpsnooper.c | 735 | t2_event_d |
| WIFI_INFO_Hotspot_client_connected | hotspot | source/hotspotfd/cosa_hotspot_dml.c | 145 | t2_event_d |
| WIFI_INFO_Hotspot_client_disconnected | hotspot | source/hotspotfd/cosa_hotspot_dml.c | 151 | t2_event_d |
| WIFI_INFO_mesh_enabled ⚠️ | OneWifi | scripts/mesh_status.sh | 30 | t2CountNotify |
| WIFI_INFO_mesh_enabled ⚠️ | test-and-diagnostic | scripts/task_health_monitor.sh | 2908 | t2CountNotify |
| WIFI_INFO_mesh_enabled ⚠️ | test-and-diagnostic | scripts/task_health_monitor.sh | 4494 | t2CountNotify |
| WIFI_INFO_MeshDisabled_syscfg0 | mesh-agent | source/MeshAgentSsp/cosa_mesh_apis.c | 2208 | t2_event_d |
| WIFI_INFO_MeshInit | mesh-agent | source/MeshAgentSsp/cosa_mesh_apis.c | 7308 | t2_event_d |
| WIFI_INFO_skipSSID | test-and-diagnostic | scripts/getSsidNames.sh | 127 | t2CountNotify |
| WIFI_INFO_XHCAM_offline | lan-manager-lite | source/lm/lm_main.c | 3656 | t2_event_d |
| WIFI_INFO_XHCAM_online | lan-manager-lite | source/lm/lm_main.c | 3634 | t2_event_d |
| WIFI_INFO_XHclient_offline | lan-manager-lite | source/lm/lm_main.c | 3664 | t2_event_d |
| WIFI_INFO_XHclient_online | lan-manager-lite | source/lm/lm_main.c | 3642 | t2_event_d |
| WIFI_INFO_XHTS_offline | lan-manager-lite | source/lm/lm_main.c | 3660 | t2_event_d |
| WIFI_INFO_XHTS_online | lan-manager-lite | source/lm/lm_main.c | 3638 | t2_event_d |
| WIFI_SH_5G_wifi_reset | provisioning-and-management | arch/intel_usg/boards/arm_shared/scripts/process_monitor.sh | 359 | t2CountNotify |
| WIFI_SH_5G_wifi_reset | provisioning-and-management | arch/intel_usg/boards/arm_shared/scripts/systemd/sysd_process_monitor.sh | 181 | t2CountNotify |
| WIFI_SH_CcspWifiHung_restart | test-and-diagnostic | scripts/selfheal_aggressive.sh | 1476 | t2CountNotify |
| WIFI_SH_CcspWifiHung_restart | test-and-diagnostic | scripts/selfheal_aggressive.sh | 1481 | t2CountNotify |
| WIFI_SH_CcspWifiHung_restart | test-and-diagnostic | scripts/task_health_monitor.sh | 1450 | t2CountNotify |
| WIFI_SH_CcspWifiHung_restart | test-and-diagnostic | scripts/task_health_monitor.sh | 1454 | t2CountNotify |
| WIFI_SH_CcspWifiHung_restart | test-and-diagnostic | scripts/task_health_monitor.sh | 1617 | t2CountNotify |
| WIFI_SH_hotspot_restart | test-and-diagnostic | scripts/corrective_action.sh | 788 | t2CountNotify |
| WIFI_SH_Parodus_restart | test-and-diagnostic | scripts/task_health_monitor.sh | 2213 | t2CountNotify |
| WIFI_SH_WIFIAGENT_Restart | OneWifi | scripts/process_monitor_atom.sh | 263 | t2CountNotify |
| WIFI_VAPPERC_split | OneWifi | scripts/apshealth.sh | 105 | t2ValNotify |
| WIFIV_ERR_reassoc | sysint | lib/rdk/networkConnectionRecovery.sh | 183 | t2CountNotify |
| WIFIV_WARN_PL_ | sysint | lib/rdk/networkConnectionRecovery.sh | 271 | t2CountNotify |
| WIFIV_WARN_PL_10PERC | sysint | lib/rdk/networkConnectionRecovery.sh | 280 | t2CountNotify |
| WPE_ERR_rtrmfplayer_crash | sysint | lib/rdk/core_shell.sh | 118 | t2CountNotify |
| WPE_INFO_MigStatus_split | entservices-migration | plugin/MigrationImplementation.cpp | 74 | t2_event_s |
| WPE_INFO_MigStatus_split | entservices-migration | plugin/MigrationImplementation.cpp | 114 | t2_event_s |
| xconf_couldnt_resolve | rdkfwupdater | src/rdkv_upgrade.c | 591 | Upgradet2CountNotify→t2_event_d |
| xconf_download_success | xconf-client | scripts/cbr_firmwareDwnld.sh | 1465 | t2ValNotify |
| xconf_download_success | xconf-client | scripts/xb6_firmwareDwnld.sh | 1621 | t2ValNotify |
| xconf_download_success | xconf-client | scripts/xb3_firmwareDwnld.sh | 1247 | t2ValNotify |
| xconf_download_success | xconf-client | scripts/xf3_firmwareDwnld.sh | 1520 | t2ValNotify |
| xconf_download_success | xconf-client | scripts/xb3_codebig_firmwareDwnld.sh | 1472 | t2ValNotify |
| XCONF_Dwld_failed | xconf-client | scripts/cbr_firmwareDwnld.sh | 1484 | t2CountNotify |
| XCONF_Dwld_failed | xconf-client | scripts/xb6_firmwareDwnld.sh | 1638 | t2CountNotify |
| XCONF_Dwld_failed | xconf-client | scripts/xb3_firmwareDwnld.sh | 1258 | t2CountNotify |
| XCONF_Dwld_failed | xconf-client | scripts/xf3_firmwareDwnld.sh | 1531 | t2CountNotify |
| XCONF_Dwld_failed | xconf-client | scripts/xb3_codebig_firmwareDwnld.sh | 1483 | t2CountNotify |
| XCONF_Dwld_Ignored_Not_EnoughMem | miscellaneous-broadband | source/FwDownloadChk/fw_download_check.c | 214 | t2_event_d |
| XCONF_Dwld_success | xconf-client | scripts/cbr_firmwareDwnld.sh | 1462 | t2CountNotify |
| XCONF_Dwld_success | xconf-client | scripts/xb6_firmwareDwnld.sh | 1618 | t2CountNotify |
| XCONF_Dwld_success | xconf-client | scripts/xb3_firmwareDwnld.sh | 1244 | t2CountNotify |
| XCONF_Dwld_success | xconf-client | scripts/xf3_firmwareDwnld.sh | 1517 | t2CountNotify |
| XCONF_Dwld_success | xconf-client | scripts/xb3_codebig_firmwareDwnld.sh | 1469 | t2CountNotify |
| XCONF_Dwnld_error | xconf-client | source/cm_http_dl.c | 483 | t2_event_d |
| XCONF_flash_failed | xconf-client | scripts/xf3_firmwareDwnld.sh | 1581 | t2CountNotify |
| xconf_reaching_ssr | xconf-client | scripts/cbr_firmwareDwnld.sh | 513 | t2ValNotify |
| xconf_reaching_ssr | xconf-client | scripts/xb6_firmwareDwnld.sh | 572 | t2ValNotify |
| xconf_reaching_ssr | xconf-client | scripts/xb3_firmwareDwnld.sh | 390 | t2ValNotify |
| xconf_reaching_ssr | xconf-client | scripts/xf3_firmwareDwnld.sh | 418 | t2ValNotify |
| xconf_reaching_ssr | xconf-client | scripts/xb3_codebig_firmwareDwnld.sh | 510 | t2ValNotify |
| Xi_wifiMAC_split | sysint | lib/rdk/NM_Dispatcher.sh | 120 | t2ValNotify |
| xr_fwdnld_split | rdkfwupdater | src/rdkFwupdateMgr.c | 576 | t2ValNotify→t2_event_s |
| xr_fwdnld_split | rdkfwupdater | src/rdkv_main.c | 526 | t2ValNotify→t2_event_s |
| XWIFI_Active_Tunnel | hotspot | source/hotspotfd/hotspotfd.c | 2137 | t2_event_s |
| XWIFI_Active_Tunnel | hotspot | source/hotspotfd/hotspotfd.c | 2208 | t2_event_s |
| XWIFI_Active_Tunnel | hotspot | source/hotspotfd/hotspotfd.c | 2289 | t2_event_s |
| XWIFI_Active_Tunnel | hotspot | source/hotspotfd/hotspotfd.c | 2395 | t2_event_s |
| XWIFI_VLANID_10_split | hotspot | source/HotspotApi/HotspotApi.c | 771 | t2_event_d |
| XWIFI_VLANID_19_split | hotspot | source/HotspotApi/HotspotApi.c | 774 | t2_event_d |
| XWIFI_VLANID_21_split | hotspot | source/HotspotApi/HotspotApi.c | 777 | t2_event_d |
| XWIFI_VLANID_6_split | hotspot | source/HotspotApi/HotspotApi.c | 768 | t2_event_d |
| ZeroUptime | test-and-diagnostic | scripts/uptime.sh | 74 | t2CountNotify |

## Dynamic Markers
Markers containing shell variables (`$var`, `${var}`) that resolve at runtime.

| Marker Pattern | Component | File Path | Line | API |
|----------------|-----------|-----------|------|-----|
| SYST_ERR_$source | sysint | lib/rdk/rebootNow.sh | 143 | t2CountNotify |
| SYST_ERR_$source_reboot | sysint | lib/rdk/rebootNow.sh | 164 | t2CountNotify |
| SYST_ERR_CrashSig$2 | sysint | lib/rdk/core_shell.sh | 153 | t2CountNotify |
| WIFIV_INFO_NO${version}ROUTE | sysint | lib/rdk/networkConnectionRecovery.sh | 258 | t2CountNotify |

## Duplicate Markers
⚠️ **btime_wcpenter_split** - Found in 3 components:
- provisioning-and-management: arch/intel_usg/boards/arm_shared/scripts/network_response.sh:680 (`t2ValNotify`)
- sysint-broadband: webgui_arm.sh:325 (`t2ValNotify`)
- webui: source/Styles/xb3/config/webgui.sh:313 (`t2ValNotify`)

⚠️ **CDLrdkportal_split** - Found in 2 components:
- rdkfwupdater: src/device_status_helper.c:377 (`t2CountNotify→t2_event_d`)
- sysint: lib/rdk/userInitiatedFWDnld.sh:178 (`t2ValNotify`)

⚠️ **certerr_split** - Found in 7 components:
- crashupload: c_sourcecode/src/upload/upload.c:267 (`t2ValNotify→t2_event_s`)
- dcm-agent: uploadstblogs/src/path_handler.c:458 (`t2_val_notify→t2_event_s`)
- rdkfwupdater: src/rdkv_upgrade.c:284 (`Upgradet2ValNotify→t2_event_s`)
- rdm-agent: scripts/downloadUtils.sh:417 (`t2ValNotify`)
- sysint: lib/rdk/uploadSTBLogs.sh:307 (`t2ValNotify`)
- sysint: lib/rdk/xconfImageCheck.sh:408 (`t2ValNotify`)
- sysint-broadband: stateRedRecoveryUtils.sh:78 (`t2ValNotify`)
- sysint-broadband: stateRedRecoveryUtils.sh:107 (`t2ValNotify`)
- xconf-client: scripts/cbr_firmwareDwnld.sh:764 (`t2ValNotify`)
- xconf-client: scripts/cbr_firmwareDwnld.sh:805 (`t2ValNotify`)
- xconf-client: scripts/cbr_firmwareDwnld.sh:1472 (`t2ValNotify`)
- xconf-client: scripts/cbr_firmwareDwnld.sh:1488 (`t2ValNotify`)
- xconf-client: scripts/xb6_firmwareDwnld.sh:865 (`t2ValNotify`)
- xconf-client: scripts/xb6_firmwareDwnld.sh:908 (`t2ValNotify`)
- xconf-client: scripts/xb6_firmwareDwnld.sh:1628 (`t2ValNotify`)
- xconf-client: scripts/xb6_firmwareDwnld.sh:1642 (`t2ValNotify`)

⚠️ **CurlRet_split** - Found in 2 components:
- rdkfwupdater: src/rdkv_main.c:1166 (`t2CountNotify→t2_event_d`)
- sysint: lib/rdk/xconfImageCheck.sh:418 (`t2ValNotify`)

⚠️ **HDMI_DeviceInfo_split** - Found in 2 components:
- entservices-hdmicecsink: plugin/HdmiCecSinkImplementation.cpp:295 (`t2_event_s`)
- entservices-hdmicecsource: plugin/HdmiCecSourceImplementation.cpp:215 (`t2_event_s`)

⚠️ **LOAD_AVG_ATOM_split** - Found in 2 components:
- sysint-broadband: log_mem_cpu_info_atom.sh:104 (`t2ValNotify`)
- test-and-diagnostic: scripts/log_mem_cpu_info.sh:163 (`t2ValNotify`)

⚠️ **LUCurlErr_split** - Found in 2 components:
- dcm-agent: uploadstblogs/src/path_handler.c:214 (`t2_val_notify→t2_event_s`)
- dcm-agent: uploadstblogs/src/path_handler.c:342 (`t2_val_notify→t2_event_s`)
- dcm-agent: uploadstblogs/src/path_handler.c:431 (`t2_val_notify→t2_event_s`)
- dcm-agent: uploadstblogs/src/path_handler.c:518 (`t2_val_notify→t2_event_s`)
- sysint: lib/rdk/uploadSTBLogs.sh:338 (`t2ValNotify`)
- sysint: lib/rdk/uploadSTBLogs.sh:614 (`t2ValNotify`)
- sysint: lib/rdk/uploadSTBLogs.sh:645 (`t2ValNotify`)

⚠️ **PDRI_Version_split** - Found in 2 components:
- rdkfwupdater: src/deviceutils/device_api.c:163 (`t2ValNotify→t2_event_s`)
- sysint: lib/rdk/xconfImageCheck.sh:458 (`t2ValNotify`)

⚠️ **SYS_ERROR_DHCPV4Client_notrunning** - Found in 2 components:
- test-and-diagnostic: scripts/selfheal_aggressive.sh:1118 (`t2CountNotify`)
- test-and-diagnostic: scripts/selfheal_aggressive.sh:1125 (`t2CountNotify`)
- test-and-diagnostic: scripts/selfheal_aggressive.sh:1139 (`t2CountNotify`)
- test-and-diagnostic: scripts/selfheal_aggressive.sh:1145 (`t2CountNotify`)
- test-and-diagnostic: scripts/task_health_monitor.sh:4375 (`t2CountNotify`)
- wan-manager: source/WanManager/wanmgr_interface_sm.c:514 (`t2_event_d`)

⚠️ **SYS_ERROR_DHCPV6Client_notrunning** - Found in 2 components:
- test-and-diagnostic: scripts/selfheal_aggressive.sh:1158 (`t2CountNotify`)
- test-and-diagnostic: scripts/selfheal_aggressive.sh:1167 (`t2CountNotify`)
- test-and-diagnostic: scripts/selfheal_aggressive.sh:1173 (`t2CountNotify`)
- test-and-diagnostic: scripts/task_health_monitor.sh:4384 (`t2CountNotify`)
- wan-manager: source/WanManager/wanmgr_interface_sm.c:536 (`t2_event_d`)

⚠️ **SYS_ERROR_LOGUPLOAD_FAILED** - Found in 2 components:
- meta-cmf-bananapi: meta-rdk-mtk-bpir4/recipes-rdkb/sysint-broadband/files/uploadRDKBLogs.sh:628 (`t2CountNotify`)
- meta-cmf-bananapi: meta-rdk-mtk-bpir4/recipes-rdkb/sysint-broadband/files/uploadRDKBLogs.sh:665 (`t2CountNotify`)
- sysint-broadband: opsLogUpload.sh:348 (`t2CountNotify`)
- sysint-broadband: opsLogUpload.sh:482 (`t2CountNotify`)
- sysint-broadband: onboardLogUpload.sh:273 (`t2CountNotify`)
- sysint-broadband: onboardLogUpload.sh:315 (`t2CountNotify`)
- sysint-broadband: uploadRDKBLogs.sh:588 (`t2CountNotify`)
- sysint-broadband: uploadRDKBLogs.sh:829 (`t2CountNotify`)

⚠️ **SYS_ERROR_NoConnectivity_reboot** - Found in 2 components:
- meta-rdk-qcom: meta-cmf-ipq/recipes-ccsp/ccsp/sysint/device/lib/rdk/logfiles.sh:386 (`t2CountNotify`)
- sysint-broadband: logfiles.sh:500 (`t2CountNotify`)

⚠️ **SYS_ERROR_PSMCrash_reboot** - Found in 2 components:
- provisioning-and-management: arch/intel_usg/boards/arm_shared/scripts/process_monitor.sh:388 (`t2CountNotify`)
- test-and-diagnostic: scripts/selfheal_bootup.sh:513 (`t2CountNotify`)
- test-and-diagnostic: scripts/device/tccbr/selfheal_bootup.sh:402 (`t2CountNotify`)

⚠️ **SYS_INFO_bootup** - Found in 2 components:
- meta-rdk-qcom: meta-cmf-ipq/recipes-ccsp/ccsp/sysint/rdkbLogMonitor.sh:876 (`t2CountNotify`)
- sysint-broadband: rdkbLogMonitor_cron.sh:907 (`t2CountNotify`)
- sysint-broadband: rdkbLogMonitor.sh:833 (`t2CountNotify`)

⚠️ **SYS_INFO_CaptivePortal** - Found in 2 components:
- dhcp-manager: source/DHCPServerUtils/DHCPv4Server/dhcp_server_functions.c:1401 (`t2_event_d`)
- dhcp-manager: source/DHCPServerUtils/DHCPv4Server/dhcp_server_functions.c:1421 (`t2_event_d`)
- utopia: source/service_dhcp/dhcp_server_functions.c:1425 (`t2_event_d`)
- utopia: source/service_dhcp/dhcp_server_functions.c:1445 (`t2_event_d`)
- utopia: source/scripts/init/service.d/service_dhcp_server/dhcp_server_functions.sh:994 (`t2CountNotify`)

⚠️ **SYS_INFO_CodBPASS** - Found in 2 components:
- rdkfwupdater: src/rdkv_upgrade.c:825 (`Upgradet2CountNotify→t2_event_d`)
- sysint: lib/rdk/userInitiatedFWDnld.sh:538 (`t2CountNotify`)
- sysint: lib/rdk/userInitiatedFWDnld.sh:620 (`t2CountNotify`)

⚠️ **SYS_INFO_ERouter_Mode_2** - Found in 2 components:
- cable-modem-agent: source/CMAgentSsp/gw_prov_sm.c:1081 (`t2_event_d`)
- gw-provisioning-application: source/gw_prov_sm.c:1017 (`t2_event_d`)

⚠️ **SYS_INFO_ErouterMode2** - Found in 2 components:
- cable-modem-agent: source/CMAgentSsp/gw_prov_sm.c:1719 (`t2_event_d`)
- gw-provisioning-application: source/gw_prov_sm.c:1629 (`t2_event_d`)
- gw-provisioning-application: source/gw_prov_sm_generic.c:802 (`t2_event_d`)

⚠️ **SYS_INFO_Invoke_batterymode** - Found in 2 components:
- telemetry: source/testApp/testCommonLibApi.c:75 (`t2_event_s`)
- test-and-diagnostic: scripts/resource_monitor.sh:421 (`t2CountNotify`)

⚠️ **SYS_INFO_LOGS_UPLOADED** - Found in 2 components:
- meta-cmf-bananapi: meta-rdk-mtk-bpir4/recipes-rdkb/sysint-broadband/files/uploadRDKBLogs.sh:623 (`t2CountNotify`)
- sysint-broadband: opsLogUpload.sh:396 (`t2CountNotify`)
- sysint-broadband: opsLogUpload.sh:471 (`t2CountNotify`)
- sysint-broadband: onboardLogUpload.sh:311 (`t2CountNotify`)
- sysint-broadband: uploadRDKBLogs.sh:673 (`t2CountNotify`)
- sysint-broadband: uploadRDKBLogs.sh:807 (`t2CountNotify`)

⚠️ **SYS_INFO_SETSYSTIME_split** - Found in 2 components:
- test-and-diagnostic: source/TandDSsp/current_time.c:181 (`t2_event_s`)
- utopia: source/scripts/init/service.d/service_systemtimeset.sh:44 (`t2ValNotify`)

⚠️ **SYS_INFO_SW_upgrade_reboot** - Found in 2 components:
- meta-rdk-qcom: meta-cmf-ipq/recipes-ccsp/ccsp/sysint/rdkbLogMonitor.sh:879 (`t2CountNotify`)
- sysint-broadband: rdkbLogMonitor_cron.sh:910 (`t2CountNotify`)
- sysint-broadband: rdkbLogMonitor.sh:836 (`t2CountNotify`)

⚠️ **SYS_INFO_WaitingFor_Stack_Init** - Found in 2 components:
- meta-rdk-qcom: meta-cmf-ipq/recipes-ccsp/ccsp/sysint/rdkbLogMonitor.sh:336 (`t2CountNotify`)
- meta-rdk-qcom: meta-cmf-ipq/recipes-ccsp/ccsp/sysint/rdkbLogMonitor.sh:546 (`t2CountNotify`)
- meta-rdk-qcom: meta-cmf-ipq/recipes-ccsp/ccsp/sysint/rdkbLogMonitor.sh:665 (`t2CountNotify`)
- sysint-broadband: rdkbLogMonitor_cron.sh:318 (`t2CountNotify`)
- sysint-broadband: rdkbLogMonitor_cron.sh:490 (`t2CountNotify`)
- sysint-broadband: rdkbLogMonitor.sh:249 (`t2CountNotify`)
- sysint-broadband: rdkbLogMonitor.sh:463 (`t2CountNotify`)
- sysint-broadband: rdkbLogMonitor.sh:584 (`t2CountNotify`)

⚠️ **SYS_SH_CMReset_PingFailed** - Found in 2 components:
- telemetry: source/testApp/testCommonLibApi.c:71 (`t2_event_s`)
- test-and-diagnostic: scripts/corrective_action.sh:428 (`t2CountNotify`)

⚠️ **SYS_SH_dnsmasq_restart** - Found in 2 components:
- test-and-diagnostic: scripts/selfheal_bootup.sh:735 (`t2CountNotify`)
- test-and-diagnostic: scripts/selfheal_aggressive.sh:253 (`t2CountNotify`)
- test-and-diagnostic: scripts/task_health_monitor.sh:3663 (`t2CountNotify`)
- test-and-diagnostic: scripts/task_health_monitor.sh:3744 (`t2CountNotify`)
- test-and-diagnostic: scripts/device/tccbr/selfheal_bootup.sh:520 (`t2CountNotify`)
- utopia: source/pmon/pmon.c:183 (`t2_event_d`)

⚠️ **SYS_SH_lighttpdCrash** - Found in 2 components:
- telemetry: source/testApp/testCommonLibApi.c:77 (`t2_event_d`)
- test-and-diagnostic: scripts/task_health_monitor.sh:2194 (`t2CountNotify`)
- test-and-diagnostic: scripts/task_health_monitor.sh:2395 (`t2CountNotify`)

⚠️ **SYS_SH_LM_restart** - Found in 2 components:
- provisioning-and-management: arch/intel_usg/boards/arm_shared/scripts/process_monitor.sh:213 (`t2CountNotify`)
- test-and-diagnostic: scripts/task_health_monitor.sh:1250 (`t2CountNotify`)
- test-and-diagnostic: scripts/task_health_monitor.sh:1673 (`t2CountNotify`)

⚠️ **SYS_SH_WebPA_restart** - Found in 2 components:
- OneWifi: scripts/process_monitor_atom.sh:870 (`t2CountNotify`)
- provisioning-and-management: arch/intel_usg/boards/arm_shared/scripts/process_monitor.sh:294 (`t2CountNotify`)
- provisioning-and-management: arch/intel_usg/boards/arm_shared/scripts/systemd/sysd_process_monitor.sh:112 (`t2CountNotify`)

⚠️ **SYST_ERR_CDLFail** - Found in 2 components:
- rdkfwupdater: src/rdkv_upgrade.c:594 (`Upgradet2CountNotify→t2_event_d`)
- rdkfwupdater: src/rdkv_upgrade.c:646 (`Upgradet2CountNotify→t2_event_d`)
- sysint: lib/rdk/userInitiatedFWDnld.sh:661 (`t2CountNotify`)

⚠️ **SYST_ERR_Curl28** - Found in 2 components:
- dcm-agent: uploadstblogs/src/path_handler.c:216 (`t2_count_notify→t2_event_d`)
- dcm-agent: uploadstblogs/src/path_handler.c:344 (`t2_count_notify→t2_event_d`)
- dcm-agent: uploadstblogs/src/path_handler.c:433 (`t2_count_notify→t2_event_d`)
- dcm-agent: uploadstblogs/src/path_handler.c:520 (`t2_count_notify→t2_event_d`)
- sysint: lib/rdk/uploadSTBLogs.sh:370 (`t2CountNotify`)

⚠️ **SYST_ERR_LogUpload_Failed** - Found in 2 components:
- dcm-agent: uploadstblogs/src/event_manager.c:166 (`t2_count_notify→t2_event_d`)
- dcm-agent: uploadstblogs/src/path_handler.c:552 (`t2_count_notify→t2_event_d`)
- sysint: lib/rdk/uploadSTBLogs.sh:657 (`t2CountNotify`)
- sysint: lib/rdk/uploadSTBLogs.sh:780 (`t2CountNotify`)
- sysint: lib/rdk/uploadSTBLogs.sh:871 (`t2CountNotify`)

⚠️ **SYST_ERR_ProcessCrash** - Found in 2 components:
- crashupload: c_sourcecode/src/scanner/scanner.c:369 (`t2CountNotify→t2_event_d`)
- crashupload: runDumpUpload.sh:761 (`t2CountNotify`)
- crashupload: uploadDumps_TestCases.md:1590 (`t2CountNotify`)
- sysint: lib/rdk/core_shell.sh:81 (`t2CountNotify`)

⚠️ **SYST_INFO_cb_xconf** - Found in 2 components:
- rdkfwupdater: src/rdkv_upgrade.c:733 (`Upgradet2CountNotify→t2_event_d`)
- sysint: lib/rdk/xconfImageCheck.sh:591 (`t2CountNotify`)
- sysint: lib/rdk/xconfImageCheck.sh:667 (`t2CountNotify`)

⚠️ **SYST_INFO_CDLSuccess** - Found in 2 components:
- rdkfwupdater: src/flash.c:137 (`flashT2CountNotify→t2_event_d`)
- sysint: lib/rdk/userInitiatedFWDnld.sh:664 (`t2CountNotify`)

⚠️ **SYST_INFO_FWCOMPLETE** - Found in 2 components:
- rdkfwupdater: src/rdkv_upgrade.c:605 (`Upgradet2CountNotify→t2_event_d`)
- sysint: lib/rdk/userInitiatedFWDnld.sh:423 (`t2CountNotify`)

⚠️ **SYST_INFO_FWUpgrade_Exit** - Found in 2 components:
- rdkfwupdater: src/device_status_helper.c:80 (`t2CountNotify→t2_event_d`)
- sysint: lib/rdk/swupdate_utility.sh:130 (`t2CountNotify`)

⚠️ **SYST_INFO_lu_success** - Found in 2 components:
- dcm-agent: uploadstblogs/src/event_manager.c:135 (`t2_count_notify→t2_event_d`)
- sysint: lib/rdk/uploadSTBLogs.sh:517 (`t2CountNotify`)

⚠️ **SYST_INFO_LUattempt** - Found in 2 components:
- dcm-agent: uploadstblogs/src/retry_logic.c:55 (`t2_count_notify→t2_event_d`)
- sysint: lib/rdk/uploadSTBLogs.sh:511 (`t2CountNotify`)

⚠️ **SYST_INFO_mtls_xpki** - Found in 2 components:
- dcm-agent: uploadstblogs/src/path_handler.c:100 (`t2_count_notify→t2_event_d`)
- sysint: lib/rdk/uploadSTBLogs.sh:355 (`t2CountNotify`)

⚠️ **SYST_INFO_PDRILogUpload** - Found in 2 components:
- dcm-agent: uploadstblogs/src/strategies.c:953 (`t2_count_notify→t2_event_d`)
- sysint: lib/rdk/uploadSTBLogs.sh:883 (`t2CountNotify`)
- sysint: lib/rdk/uploadSTBLogs.sh:886 (`t2CountNotify`)

⚠️ **SYST_INFO_swdlSameImg** - Found in 2 components:
- rdkfwupdater: src/device_status_helper.c:912 (`t2CountNotify→t2_event_d`)
- sysint: lib/rdk/userInitiatedFWDnld.sh:515 (`t2CountNotify`)

⚠️ **SYST_INFO_SwdlSameImg_Stndby** - Found in 2 components:
- rdkfwupdater: src/device_status_helper.c:905 (`t2CountNotify→t2_event_d`)
- sysint: lib/rdk/userInitiatedFWDnld.sh:519 (`t2CountNotify`)

⚠️ **SYST_INFO_XCONFConnect** - Found in 2 components:
- rdkfwupdater: src/rdkv_upgrade.c:378 (`Upgradet2CountNotify→t2_event_d`)
- sysint: lib/rdk/xconfImageCheck.sh:505 (`t2CountNotify`)

⚠️ **SYST_WARN_UPGD_SKIP** - Found in 2 components:
- rdkfwupdater: src/deviceutils/device_api.c:921 (`t2ValNotify→t2_event_s`)
- sysint: lib/rdk/xconfImageCheck.sh:158 (`t2ValNotify`)

⚠️ **TEST_lu_success** - Found in 2 components:
- dcm-agent: uploadstblogs/src/path_handler.c:532 (`t2_count_notify→t2_event_d`)
- sysint: lib/rdk/uploadSTBLogs.sh:619 (`t2CountNotify`)
- sysint: lib/rdk/uploadSTBLogs.sh:648 (`t2CountNotify`)

⚠️ **Total_devices_connected_split** - Found in 2 components:
- lan-manager-lite: source/lm/lm_main.c:2786 (`t2_event_d`)
- meta-rdk-qcom (patch): meta-cmf-ipq/recipes-ccsp/util/001-task_health.patch:19 (`t2ValNotify`)
- meta-rdk-qcom (patch): meta-cmf-ipq/recipes-ccsp/util/001-task_health.patch:32 (`t2ValNotify`)

⚠️ **Total_offline_clients_split** - Found in 2 components:
- lan-manager-lite: source/lm/lm_main.c:2788 (`t2_event_d`)
- meta-rdk-qcom (patch): meta-cmf-ipq/recipes-ccsp/util/001-task_health.patch:18 (`t2ValNotify`)
- meta-rdk-qcom (patch): meta-cmf-ipq/recipes-ccsp/util/001-task_health.patch:31 (`t2ValNotify`)

⚠️ **Total_wifi_clients_split** - Found in 2 components:
- lan-manager-lite: source/lm/lm_main.c:2789 (`t2_event_d`)
- meta-rdk-qcom (patch): meta-cmf-ipq/recipes-ccsp/util/001-task_health.patch:20 (`t2ValNotify`)
- meta-rdk-qcom (patch): meta-cmf-ipq/recipes-ccsp/util/001-task_health.patch:33 (`t2ValNotify`)

⚠️ **USED_CPU_ATOM_split** - Found in 2 components:
- sysint-broadband: log_mem_cpu_info_atom.sh:129 (`t2ValNotify`)
- test-and-diagnostic: scripts/log_mem_cpu_info.sh:224 (`t2ValNotify`)

⚠️ **USED_MEM_ATOM_split** - Found in 2 components:
- sysint-broadband: log_mem_cpu_info_atom.sh:98 (`t2ValNotify`)
- test-and-diagnostic: scripts/log_mem_cpu_info.sh:107 (`t2ValNotify`)

⚠️ **WIFI_ERROR_NvramCorrupt** - Found in 2 components:
- meta-rdk-qcom: meta-cmf-ipq/recipes-ccsp/ccsp/ccsp-wifi-agent/radiohealth.sh:150 (`t2CountNotify`)
- OneWifi: scripts/radiohealth.sh:114 (`t2CountNotify`)

⚠️ **WIFI_ERROR_WL0_NotFound** - Found in 2 components:
- meta-rdk-qcom: meta-cmf-ipq/recipes-ccsp/ccsp/ccsp-wifi-agent/radiohealth.sh:102 (`t2CountNotify`)
- OneWifi: scripts/radiohealth.sh:66 (`t2CountNotify`)

⚠️ **WIFI_ERROR_WL0_SSIDEmpty** - Found in 2 components:
- meta-rdk-qcom: meta-cmf-ipq/recipes-ccsp/ccsp/ccsp-wifi-agent/radiohealth.sh:115 (`t2CountNotify`)
- OneWifi: scripts/radiohealth.sh:79 (`t2CountNotify`)

⚠️ **WIFI_ERROR_WL1_NotFound** - Found in 2 components:
- meta-rdk-qcom: meta-cmf-ipq/recipes-ccsp/ccsp/ccsp-wifi-agent/radiohealth.sh:106 (`t2CountNotify`)
- OneWifi: scripts/radiohealth.sh:70 (`t2CountNotify`)

⚠️ **WIFI_ERROR_WL1_SSIDEmpty** - Found in 2 components:
- meta-rdk-qcom: meta-cmf-ipq/recipes-ccsp/ccsp/ccsp-wifi-agent/radiohealth.sh:123 (`t2CountNotify`)
- OneWifi: scripts/radiohealth.sh:87 (`t2CountNotify`)

⚠️ **WIFI_INFO_mesh_enabled** - Found in 2 components:
- OneWifi: scripts/mesh_status.sh:30 (`t2CountNotify`)
- test-and-diagnostic: scripts/task_health_monitor.sh:2908 (`t2CountNotify`)
- test-and-diagnostic: scripts/task_health_monitor.sh:4494 (`t2CountNotify`)
