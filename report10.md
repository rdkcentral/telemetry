# Telemetry Marker Inventory
**Branch**: per-component (from versions-e.txt)
**Organizations**: rdkcentral, rdk-e, rdk-common, rdk-gdcs
**Generated**: 2026-04-02 19:30:42 UTC

## Summary
- **Total Markers**: 491
- **Static Markers**: 487
- **Dynamic Markers**: 4 (contain shell variables)
- **Components Scanned**: 196
- **Unresolved Components**: 3 ⚠️
- **Duplicate Markers**: 43 ⚠️

## Unique Marker Inventory
| Marker Name | Components |
|-------------|------------|
| 5GclientMac_split | telemetry |
| ActStatus_split | rdkservices-cpc |
| APP_ERROR_Crashed_accum | crashupload |
| APP_ERROR_Crashed_split | crashupload |
| APP_ERROR_CrashInfo | crashupload |
| APP_ERROR_CrashInfo_status | crashupload |
| APPARMOR_C_split: | rdk-apparmor-profiles |
| APPARMOR_E_split: | rdk-apparmor-profiles |
| AS_ERR_Corrupted_Credential | authservice-cpc |
| AS_ERR_RSAInitFailed | authservice-cpc |
| AS_WARN_EmptyPartnerID | rdkservices-cpc |
| Board_temperature_split | sysint |
| btime_ipacqEth_split | sysint |
| btime_ipacqWifi_split | sysint |
| CA_Store_split | rdk-ca-store-cpc |
| CDL_INFO_inprogressExit | sysint, sysint-cpc |
| CDLrdkportal_split | rdkfwupdater, sysint, sysint-cpc |
| CDLsuspended_split | rdkfwupdater |
| cert_info_split | sslcerts-cpc |
| certerr_split | cpg-utils-cpc, crashupload, dcm-agent, rdkfwupdater, rdm, rdm-agent, sysint, sysint-cpc |
| cloudFWFile_split | rdkfwupdater |
| core_split | sysint |
| CoredumpFail_split | crashupload |
| coreUpld_split | crashupload |
| cpuinfo_split | sysint |
| crashedContainerAppname_split | crashupload |
| crashedContainerName_split | crashupload |
| crashedContainerProcessName_split | crashupload |
| crashedContainerStatus_split | crashupload |
| CrashedProc_split | sysint |
| Critical firmware upgrade in progress | lostandfound-cpc |
| CurlRet_split | rdkfwupdater, sysint, sysint-cpc |
| CurrentActivationStatus_split | rdkservices-cpc |
| DeviceCertUpdateFailure | ssa-cpc |
| emmcNoFile_split | sysint |
| emmcVer_split | sysint |
| FCR_split | rdkservices-cpc |
| Filesize_split | rdkfwupdater |
| FREE_MEM_split | sysint |
| FS_ssa_xpki_use_static_url | ssa-cpc |
| HDMI_DeviceInfo_split | entservices-hdmicecsink, entservices-hdmicecsource |
| HDMI_INFO_PORT1connected | entservices-hdmicecsink |
| HDMI_INFO_PORT2connected | entservices-hdmicecsink |
| HDMI_INFO_PORT3connected | entservices-hdmicecsink |
| HDMI_WARN_CEC_InvalidParamExcptn | hdmicec |
| HROT_NO_KEY | ssa-cpc |
| HROT_NON_PROD_KEY | ssa-cpc |
| HROT_SSA_DAMON_FAILED | ssa-cpc |
| HROT_ssa_xpki_use_static_url | ssa-cpc |
| lnfErr_split | lostandfound-cpc |
| LUCurlErr_split | dcm-agent, sysint |
| lxybundleversion_split | rdkfwupdater |
| marker | rdkfwupdater |
| MFR_ERR_MFRSV_coredetected | sysint |
| NF_ERR_rdm_filenotfound_extraction | rdm, rdm-agent |
| NF_INFO_codedumped | sysint |
| NF_INFO_rdm_package_failure | rdm-agent |
| NF_INFO_rdm_success | rdm, rdm-agent |
| PCR_split | rdkservices-cpc |
| PDRI_Version_split | rdkfwupdater, sysint, sysint-cpc |
| PKCS11_migration_NO_opcert | lxy-cpc |
| PKCS11_migration_removing_SEcert | lxy-cpc |
| PKCS11_migration_SEcert_removing | lxy-cpc |
| processCrash_split | crashupload |
| PRVMGR_ERR_File | lostandfound-cpc |
| PRVMGR_ERR_PollingTimeout | lostandfound-cpc |
| PRVMGR_ERR_XW3RegFail | lostandfound-cpc |
| PRVMGR_INFO_GotXBOId | lostandfound-cpc |
| PRVMGR_INFO_MobileDisconn | lostandfound-cpc |
| PRVMGR_INFO_MobilePaired | lostandfound-cpc |
| PRVMGR_INFO_Provisioned | lostandfound-cpc |
| PRVMGR_INFO_PRVSuccess | lostandfound-cpc |
| PRVMGR_INFO_StartBeacon | lostandfound-cpc |
| PRVMGR_INFO_WIFIAssfail | lostandfound-cpc |
| PRVMGR_INFO_WIFIAssOk | lostandfound-cpc |
| PRVMGR_INFO_XBOSuccess | lostandfound-cpc |
| PRVMGR_INFO_XW3RegOk | lostandfound-cpc |
| PRVMGR_split | lostandfound-cpc |
| RCU_FWver_split | sysint, sysint-cpc |
| RDM_ERR_package_failed | rdm-agent |
| RDM_ERR_package_notfound | rdm-agent |
| RDM_ERR_rdm_package_notfound | rdm |
| RDM_ERR_rdm_retry_fail | rdm, rdm-agent |
| RDM_ERR_rsa_signature_failed | rdm, rdm-agent |
| RDM_INFO_AppDownloadComplete | rdm-agent |
| RDM_INFO_AppDownloadSuccess | rdm-agent |
| RDM_INFO_DefaultURL | rdm-agent |
| RDM_INFO_DirectBlocked | rdm-agent |
| RDM_INFO_DownloadSSRURL | rdm-agent |
| RDM_INFO_extraction_complete | rdm-agent |
| RDM_INFO_package_download | rdm-agent |
| RDM_INFO_rsa_valid_signature | rdm, rdm-agent |
| RDM_INFO_rsa_verify_signature_failure | rdm-agent |
| RDMCAcert_split | rdk-ca-store-cpc |
| RDMwebuicert_split | sslcerts-cpc |
| RDMxPkicert_split | sslcerts-cpc |
| Router_Discovered | networkmanager |
| SCARD_INFO_emmc_noUpgd | sysint |
| SEFS_ssa_xpki_use_static_url | ssa-cpc |
| SEHAL_NO_KEY | ssa-cpc |
| SEHAL_NON_PROD_KEY | ssa-cpc |
| SEHAL_SSA_DAMON_FAILED | ssa-cpc |
| SESE_ssa_xpki_use_static_url | ssa-cpc |
| SHORTS_CONN_SUCCESS | sysint |
| SHORTS_DEVICE_TYPE_PROD | sysint |
| SHORTS_DEVICE_TYPE_TEST | sysint |
| SHORTS_DEVICE_TYPE_UNKNOWN | sysint |
| SHORTS_SSH_CLIENT_FAILURE | sysint |
| SHORTS_STUNNEL_CERT_FAILURE | sysint |
| SHORTS_STUNNEL_CLIENT_FAILURE | sysint |
| ssa_xpki_use_static_url | ssa-cpc |
| SYS_ERROR_S3CoreUpload_Failed | cpg-utils-cpc, crashupload |
| SYS_INFO_ActiveCredsMissing | rdkservices-cpc |
| SYS_INFO_CANARY_Update | rdkfwupdater |
| SYS_INFO_CodBPASS | rdkfwupdater, sysint, sysint-cpc |
| SYS_INFO_CrashedContainer | crashupload |
| SYS_INFO_DAC_Inject_Failed | ssa-cpc |
| SYS_INFO_DAC_Inject_Success | ssa-cpc |
| SYS_INFO_DEFER_CANARY_REBOOT | rdkfwupdater |
| SYS_INFO_DirectSuccess | rdkfwupdater |
| SYS_INFO_Invoke_batterymode | telemetry |
| SYS_INFO_Matter_DAC_Status | ssa-cpc |
| SYS_INFO_MTLS_enable | rdkfwupdater |
| SYS_INFO_S3CoreUploaded | cpg-utils-cpc, crashupload |
| SYS_INFO_swdltriggered | rdkfwupdater |
| SYS_INFO_TGZDUMP | crashupload |
| SYS_INFO_xPKI_Static_Fallback | sslcerts-cpc, sysint-cpc |
| SYS_SH_CMReset_PingFailed | telemetry |
| SYS_SH_lighttpdCrash | telemetry |
| SYST_ERR_ | sysint |
| SYST_ERR_10Times_reboot | reboot-manager, sysint, sysint-cpc |
| SYST_ERR_AUTHSERVICE_Read | rdkservices-cpc |
| SYST_ERR_AuthTokenExpiry | rdkservices-cpc |
| SYST_ERR_CCNotRepsonding_reboot | sysint |
| SYST_ERR_cdl_ssr | rdkfwupdater |
| SYST_ERR_CDLFail | rdkfwupdater, sysint, sysint-cpc |
| SYST_ERR_CECBusEx | hdmicec |
| SYST_ERR_CLIENTCERT_Fail | sysint-cpc |
| SYST_ERR_CompFail | crashupload |
| SYST_ERR_COREGZIP | sysint |
| SYST_ERR_Curl28 | dcm-agent, sysint |
| SYST_ERR_Cyclic_reboot | sysint |
| SYST_ERR_DiffFWCTN_FLdnld | rdkfwupdater |
| SYST_ERR_DNSFileEmpty | sysint |
| SYST_ERR_DSMGR_reboot | sysint |
| SYST_ERR_FailureAuthToken | rdkservices-cpc |
| SYST_ERR_FKPSError | rdkservices-cpc |
| SYST_ERR_FW_RFC_disabled | sysint, sysint-cpc |
| SYST_ERR_FWCTNFetch | rdkfwupdater |
| SYST_ERR_FWdnldFail | sysint, sysint-cpc |
| SYST_ERR_IARMDEMON_reboot | sysint |
| SYST_ERR_imageflsfail | rdkfwupdater |
| SYST_ERR_LogUpload_Failed | dcm-agent, sysint |
| SYST_ERR_MaintNetworkFail | entservices-softwareupdate |
| SYST_ERR_MINIDPZEROSIZE | crashupload |
| SYST_ERR_OPTFULL | sysint |
| SYST_ERR_OverflowMon_crash | sysint |
| SYST_ERR_PC_Conn169 | sysint |
| SYST_ERR_PC_MAF | sysint |
| SYST_ERR_PC_RBI | sysint |
| SYST_ERR_PC_Systemd | sysint |
| SYST_ERR_PC_TTSEngine | sysint |
| SYST_ERR_PDRI_VFail | sysint, sysint-cpc |
| SYST_ERR_PDRIUpg_failure | rdkfwupdater |
| SYST_ERR_PrevCDL_InProg | sysint |
| SYST_ERR_Process_Crash_accum | crashupload |
| SYST_ERR_ProcessCrash | crashupload, sysint |
| SYST_ERR_ProvisioningFail | rdkservices-cpc |
| SYST_ERR_RDMMISSING | rdm-agent |
| SYST_ERR_RedrecoveryCert | sysint-cpc |
| SYST_ERR_RFC | entservices-softwareupdate |
| SYST_ERR_Rmfstreamer_crash | sysint |
| SYST_ERR_Rmfstreamer_reboot | sysint |
| SYST_ERR_RunPod_reboot | sysint |
| SYST_ERR_S3signing_failed | cpg-utils-cpc |
| SYST_ERR_syslogng_crash | sysint |
| SYST_ERR_VodApp_restart | sysint |
| SYST_ERR_XACS401 | authservice-cpc |
| SYST_ERR_XCALDevice_crash | sysint |
| SYST_ERR_Xconf28 | sysint, sysint-cpc |
| SYST_ERR_xraudio_crash | sysint |
| SYST_ERROR_WAI_InitERR | entservices-softwareupdate |
| SYST_INFO_Act_split | rdkservices-cpc |
| SYST_INFO_ActivReady | rdkservices-cpc |
| SYST_INFO_ACTN_SUCCESS | rdkservices-cpc |
| SYST_INFO_AuthTokenSucc | rdkservices-cpc |
| SYST_INFO_C_CDL | rdkfwupdater |
| SYST_INFO_cb_xconf | rdkfwupdater, sysint, sysint-cpc |
| SYST_INFO_CDLSuccess | rdkfwupdater, sysint, sysint-cpc |
| SYST_INFO_Core_accum | sysint |
| SYST_INFO_CoreFull_accum | sysint |
| SYST_INFO_CoreIMP_accum | sysint |
| SYST_INFO_CoreNotProcessed | sysint |
| SYST_INFO_CoreProcessed | sysint |
| SYST_INFO_CoreProcessed_accum | sysint |
| SYST_INFO_CoreUpldSkipped | crashupload |
| SYST_INFO_CrashedProc_accum | sysint |
| SYST_INFO_CURL6 | cpg-utils-cpc, crashupload |
| SYST_INFO_DevicenotActivated | rdkservices-cpc |
| SYST_INFO_ETHConn | sysint |
| SYST_INFO_FetchFWCTN | rdkfwupdater |
| SYST_INFO_FWCOMPLETE | rdkfwupdater, sysint, sysint-cpc |
| SYST_INFO_FWUpgrade_Exit | rdkfwupdater, sysint |
| SYST_INFO_Healthcheck_split | rdkservices-cpc |
| SYST_INFO_Http302 | rdkfwupdater |
| SYST_INFO_ImgFlashOK | rdkfwupdater |
| SYST_INFO_JSPPShutdown | entservices-monitor |
| SYST_INFO_lu_success | dcm-agent, sysint |
| SYST_INFO_LUattempt | dcm-agent, sysint |
| SYST_INFO_MaintnceIncmpl | entservices-softwareupdate |
| SYST_INFO_MemAvailable_split | sysint |
| SYST_INFO_minidumpUpld | crashupload |
| SYST_INFO_mtls_xpki | dcm-agent, sysint |
| SYST_INFO_NewXactToken_success | authservice-cpc |
| SYST_INFO_NoConsentFlash | rdkfwupdater |
| SYST_INFO_PartnerId | sysint |
| SYST_INFO_PC_RF4CE | sysint |
| SYST_INFO_PDRILogUpload | dcm-agent, sysint |
| SYST_INFO_PDRIUpgSuccess | rdkfwupdater |
| SYST_INFO_ProvisioingSucc | rdkservices-cpc |
| SYST_INFO_PRXR_Ver_split | rdkfwupdater |
| SYST_INFO_Redrecovery | sysint-cpc |
| SYST_INFO_RedStateRecovery | rdkfwupdater |
| SYST_INFO_RedstateSet | rdkfwupdater |
| SYST_INFO_RTController_split | rdkservices-cpc |
| SYST_INFO_SAME_FWCTN | rdkfwupdater |
| SYST_INFO_SigDump_split | sysint |
| SYST_INFO_SOMT | entservices-softwareupdate |
| SYST_INFO_SwapCached_split | sysint |
| SYST_INFO_SwapFree_split | sysint |
| SYST_INFO_SwapTotal_split | sysint |
| SYST_INFO_swdlSameImg | rdkfwupdater, sysint, sysint-cpc |
| SYST_INFO_SwdlSameImg_Stndby | rdkfwupdater, sysint, sysint-cpc |
| SYST_INFO_SWUpgrdChck | rdkfwupdater |
| SYST_INFO_SYSBUILD | systemtimemgr |
| SYST_INFO_Thrtl_Enable | rdkfwupdater |
| SYST_INFO_TLS_xconf | rdkfwupdater |
| SYST_INFO_TVActivated | rdkservices-cpc |
| SYST_INFO_v2_fetchCalled | rdkservices-cpc |
| SYST_INFO_v2FKPS_Good | rdkservices-cpc |
| SYST_INFO_v2FKPS_NoFetch | rdkservices-cpc |
| SYST_INFO_v2FKPS_provCalled | rdkservices-cpc |
| SYST_INFO_v2FKPSSuccess | rdkservices-cpc |
| SYST_INFO_WIFIConn | sysint |
| SYST_INFO_Xconf200 | sysint, sysint-cpc |
| SYST_INFO_XCONFConnect | rdkfwupdater, sysint, sysint-cpc |
| SYST_SWDL_Retry_split | sysint, sysint-cpc |
| SYST_WARN_ClkNotSet | rdkservices-cpc |
| SYST_WARN_CompFail | crashupload |
| SYST_WARN_CoreNP_accum | sysint |
| SYST_WARN_dcm_curl28 | sysint |
| SYST_WARN_GW100PERC_PACKETLOSS | sysint |
| SYST_WARN_NoMinidump | crashupload |
| SYST_WARN_UPGD_SKIP | rdkfwupdater, sysint, sysint-cpc |
| TEST_EVENT_1 | telemetry |
| TEST_EVENT_2 | telemetry |
| TEST_lu_success | dcm-agent, sysint |
| Test_SWReset | reboot-manager, sysint, sysint-cpc |
| TimeZone_split | sysint |
| TR69HOSTIF_GET_1000_WITHIN_5MIN | tr69hostif |
| TR69HOSTIF_GET_200_WITHIN_1MIN | tr69hostif |
| TR69HOSTIF_GET_TIMEOUT_PARAM | tr69hostif |
| TR69HOSTIF_SET_1000_WITHIN_5MIN | tr69hostif |
| TR69HOSTIF_SET_200_WITHIN_1MIN | tr69hostif |
| TR69HOSTIF_SET_TIMEOUT_PARAM | tr69hostif |
| vmstats_split | sysint |
| WIFI_ERROR_PSM_GetRecordFail | telemetry |
| WIFI_INFO_MvdToPrvSSID | lostandfound-cpc |
| WIFIV_ERR_Lnf463 | lostandfound-cpc |
| WIFIV_ERR_lnf_461 | lostandfound-cpc |
| WIFIV_ERR_lnf_464 | lostandfound-cpc |
| WIFIV_ERR_lnf_466 | lostandfound-cpc |
| WIFIV_ERR_LnF_cred_xPKI | lostandfound-cpc |
| WIFIV_ERR_LnF_lfat_XPKI | lostandfound-cpc |
| WIFIV_ERR_LnF_XPKI_EAP-TLS | lostandfound-cpc |
| WIFIV_ERR_reassoc | sysint |
| WIFIV_INFO_HAL_RX_Bitrate | wifimetrics-cpc |
| WIFIV_INFO_HAL_TX_Bitrate | wifimetrics-cpc |
| WIFIV_INFO_HAL_WiFiChannelUtilization_split | wifimetrics-cpc |
| WIFIV_INFO_LnF_cred_xPKI | lostandfound-cpc |
| WIFIV_INFO_LnF_lfat_XPKI | lostandfound-cpc |
| WIFIV_INFO_LnfConnected | lostandfound-cpc |
| WIFIV_SET_BGSCAN_PARAMETERS | wifioptimizer-cpc |
| WIFIV_WARN_LnF_xPKI | lostandfound-cpc |
| WIFIV_WARN_PL_ | sysint |
| WIFIV_WARN_PL_10PERC | sysint |
| WPE_ERR_rtrmfplayer_crash | sysint |
| WPE_INFO_MigStatus_split | entservices-migration |
| xconf_couldnt_resolve | rdkfwupdater |
| Xi_wifiMAC_split | sysint |
| xr_fwdnld_split | rdkfwupdater |

## Detailed Marker Inventory
| Marker Name | Component | File Path | Line | API |
|-------------|-----------|-----------|------|-----|
| 5GclientMac_split | telemetry | source/testApp/testCommonLibApi.c | 73 | t2_event_s |
| ActStatus_split | rdkservices-cpc | DeviceProvisioning/DeviceProvisioningImplementation.cpp | 1225 | t2_event_s |
| ActStatus_split | rdkservices-cpc | DeviceProvisioning/DeviceProvisioningImplementation.cpp | 1265 | t2_event_s |
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
| AS_ERR_Corrupted_Credential | authservice-cpc | authservice.cpp | 431 | t2_event_s |
| AS_ERR_RSAInitFailed | authservice-cpc | authservice.cpp | 292 | t2_event_s |
| AS_WARN_EmptyPartnerID | rdkservices-cpc | DeviceProvisioning/rtcontroller.cpp | 359 | t2_event_s |
| Board_temperature_split | sysint | lib/rdk/temperature-telemetry.sh | 27 | t2ValNotify |
| btime_ipacqEth_split | sysint | lib/rdk/ipv6addressChange.sh | 62 | t2ValNotify |
| btime_ipacqWifi_split | sysint | lib/rdk/ipv6addressChange.sh | 65 | t2ValNotify |
| CA_Store_split | rdk-ca-store-cpc | scripts/check-ca-update.sh | 78 | t2ValNotify |
| CDL_INFO_inprogressExit ⚠️ | sysint | lib/rdk/userInitiatedFWDnld.sh | 755 | t2CountNotify |
| CDL_INFO_inprogressExit ⚠️ | sysint-cpc | lib/rdk/userInitiatedFWDnld.sh | 806 | t2CountNotify |
| CDLrdkportal_split ⚠️ | rdkfwupdater | src/device_status_helper.c | 377 | t2CountNotify→t2_event_d |
| CDLrdkportal_split ⚠️ | sysint | lib/rdk/userInitiatedFWDnld.sh | 178 | t2ValNotify |
| CDLrdkportal_split ⚠️ | sysint-cpc | lib/rdk/userInitiatedFWDnld.sh | 179 | t2ValNotify |
| CDLsuspended_split | rdkfwupdater | src/rdkv_upgrade.c | 146 | Upgradet2CountNotify→t2_event_d |
| cert_info_split | sslcerts-cpc | Scripts/cert-monitoring.sh | 65 | t2ValNotify |
| certerr_split ⚠️ | cpg-utils-cpc | uploadDumpsToS3.sh | 213 | t2ValNotify |
| certerr_split ⚠️ | cpg-utils-cpc | uploadDumpsToS3.sh | 271 | t2ValNotify |
| certerr_split ⚠️ | cpg-utils-cpc | uploadDumpsToS3.sh | 213 | t2ValNotify |
| certerr_split ⚠️ | cpg-utils-cpc | uploadDumpsToS3.sh | 271 | t2ValNotify |
| certerr_split ⚠️ | crashupload | c_sourcecode/src/upload/upload.c | 267 | t2ValNotify→t2_event_s |
| certerr_split ⚠️ | dcm-agent | uploadstblogs/src/path_handler.c | 458 | t2_val_notify→t2_event_s |
| certerr_split ⚠️ | rdkfwupdater | src/rdkv_upgrade.c | 284 | Upgradet2ValNotify→t2_event_s |
| certerr_split ⚠️ | rdm | scripts/downloadUtils.sh | 452 | t2ValNotify |
| certerr_split ⚠️ | rdm-agent | scripts/downloadUtils.sh | 417 | t2ValNotify |
| certerr_split ⚠️ | sysint | lib/rdk/uploadSTBLogs.sh | 307 | t2ValNotify |
| certerr_split ⚠️ | sysint | lib/rdk/xconfImageCheck.sh | 408 | t2ValNotify |
| certerr_split ⚠️ | sysint-cpc | lib/rdk/userInitiatedFWDnld.sh | 353 | t2ValNotify |
| certerr_split ⚠️ | sysint-cpc | lib/rdk/xconfImageCheck.sh | 435 | t2ValNotify |
| certerr_split ⚠️ | sysint-cpc | lib/rdk/xconfImageCheck.sh | 484 | t2ValNotify |
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
| Critical firmware upgrade in progress | lostandfound-cpc | ble/prvn_mgr/prvn_mgr.c | 1261 | t2_event_d |
| CurlRet_split ⚠️ | rdkfwupdater | src/rdkv_main.c | 1166 | t2CountNotify→t2_event_d |
| CurlRet_split ⚠️ | sysint | lib/rdk/xconfImageCheck.sh | 418 | t2ValNotify |
| CurlRet_split ⚠️ | sysint-cpc | lib/rdk/userInitiatedFWDnld.sh | 348 | t2ValNotify |
| CurlRet_split ⚠️ | sysint-cpc | lib/rdk/xconfImageCheck.sh | 442 | t2ValNotify |
| CurlRet_split ⚠️ | sysint-cpc | lib/rdk/xconfImageCheck.sh | 491 | t2ValNotify |
| CurrentActivationStatus_split | rdkservices-cpc | DeviceProvisioning/DeviceProvisioningImplementation.cpp | 1224 | t2_event_s |
| CurrentActivationStatus_split | rdkservices-cpc | DeviceProvisioning/DeviceProvisioningImplementation.cpp | 1264 | t2_event_s |
| DeviceCertUpdateFailure | ssa-cpc | ssa_top/ssa_cpc/ssa_common/providers/CA/SecureElement/scripts/rdkssaRunCertChecker.sh | 120 | t2CountNotify |
| DeviceCertUpdateFailure | ssa-cpc | ssa_top/ssa_cpc/ssa_common/providers/CA/SecureElement/scripts/rdk/rdkssaRunCertChecker.sh | 70 | t2CountNotify |
| emmcNoFile_split | sysint | lib/rdk/eMMC_Upgrade.sh | 131 | t2ValNotify |
| emmcVer_split | sysint | lib/rdk/eMMC_Upgrade.sh | 66 | t2ValNotify |
| FCR_split | rdkservices-cpc | DeviceProvisioning/DeviceProvisioningImplementation.cpp | 1174 | t2_event_s |
| Filesize_split | rdkfwupdater | src/rdkv_upgrade.c | 623 | Upgradet2CountNotify→t2_event_d |
| FREE_MEM_split | sysint | lib/rdk/system_info_collector.sh | 59 | t2ValNotify |
| FREE_MEM_split | sysint | lib/rdk/cpu-statistics.sh | 33 | t2ValNotify |
| FS_ssa_xpki_use_static_url | ssa-cpc | ssa_top/ssa_cpc/ssa_common/providers/CA/SecureElement/scripts/rdkssaRunFScertifier.sh | 240 | t2CountNotify |
| HDMI_DeviceInfo_split ⚠️ | entservices-hdmicecsink | plugin/HdmiCecSinkImplementation.cpp | 295 | t2_event_s |
| HDMI_DeviceInfo_split ⚠️ | entservices-hdmicecsource | plugin/HdmiCecSourceImplementation.cpp | 215 | t2_event_s |
| HDMI_INFO_PORT1connected | entservices-hdmicecsink | plugin/HdmiCecSinkImplementation.cpp | 1965 | t2_event_d |
| HDMI_INFO_PORT2connected | entservices-hdmicecsink | plugin/HdmiCecSinkImplementation.cpp | 1968 | t2_event_d |
| HDMI_INFO_PORT3connected | entservices-hdmicecsink | plugin/HdmiCecSinkImplementation.cpp | 1971 | t2_event_d |
| HDMI_WARN_CEC_InvalidParamExcptn | hdmicec | ccec/src/Bus.cpp | 346 | t2_event_s |
| HROT_NO_KEY | ssa-cpc | ssa_top/ssa_cpc/daemon/genericdaemon/Scripts/check_pph.sh | 18 | t2CountNotify |
| HROT_NO_KEY | ssa-cpc | ssa_top/ssa_cpc/ssa_common/providers/CA/Generic/scripts/rdkssacertcheck.sh | 325 | t2CountNotify |
| HROT_NON_PROD_KEY | ssa-cpc | ssa_top/ssa_cpc/ssa_common/providers/CA/Generic/scripts/rdkssacertcheck.sh | 328 | t2CountNotify |
| HROT_SSA_DAMON_FAILED | ssa-cpc | ssa_top/ssa_cpc/daemon/genericdaemon/Scripts/check_pph.sh | 23 | t2CountNotify |
| HROT_SSA_DAMON_FAILED | ssa-cpc | ssa_top/ssa_cpc/ssa_common/providers/CA/Generic/scripts/rdkssacertcheck.sh | 333 | t2CountNotify |
| HROT_ssa_xpki_use_static_url | ssa-cpc | ssa_top/ssa_cpc/ssa_common/providers/CA/Generic/scripts/rdkssacertcheck.sh | 239 | t2CountNotify |
| lnfErr_split | lostandfound-cpc | src/lost_and_found.c | 860 | laf_telemetry_event_d→t2_event_d |
| LUCurlErr_split ⚠️ | dcm-agent | uploadstblogs/src/path_handler.c | 214 | t2_val_notify→t2_event_s |
| LUCurlErr_split ⚠️ | dcm-agent | uploadstblogs/src/path_handler.c | 342 | t2_val_notify→t2_event_s |
| LUCurlErr_split ⚠️ | dcm-agent | uploadstblogs/src/path_handler.c | 431 | t2_val_notify→t2_event_s |
| LUCurlErr_split ⚠️ | dcm-agent | uploadstblogs/src/path_handler.c | 518 | t2_val_notify→t2_event_s |
| LUCurlErr_split ⚠️ | sysint | lib/rdk/uploadSTBLogs.sh | 338 | t2ValNotify |
| LUCurlErr_split ⚠️ | sysint | lib/rdk/uploadSTBLogs.sh | 614 | t2ValNotify |
| LUCurlErr_split ⚠️ | sysint | lib/rdk/uploadSTBLogs.sh | 645 | t2ValNotify |
| lxybundleversion_split | rdkfwupdater | src/json_process.c | 284 | t2ValNotify→t2_event_s |
| marker | rdkfwupdater | unittest/basic_rdkv_main_gtest.cpp | 246 | t2_event_s |
| marker | rdkfwupdater | unittest/basic_rdkv_main_gtest.cpp | 247 | t2ValNotify→t2_event_s |
| MFR_ERR_MFRSV_coredetected | sysint | lib/rdk/core_shell.sh | 85 | t2CountNotify |
| NF_ERR_rdm_filenotfound_extraction ⚠️ | rdm | scripts/downloadUtils.sh | 653 | t2CountNotify |
| NF_ERR_rdm_filenotfound_extraction ⚠️ | rdm-agent | scripts/downloadUtils.sh | 622 | t2CountNotify |
| NF_INFO_codedumped | sysint | lib/rdk/core_shell.sh | 121 | t2CountNotify |
| NF_INFO_rdm_package_failure | rdm-agent | src/rdm_downloadmgr.c | 279 | t2CountNotify→t2_event_d |
| NF_INFO_rdm_package_failure | rdm-agent | src/rdm_packagemgr.c | 193 | t2CountNotify→t2_event_d |
| NF_INFO_rdm_success ⚠️ | rdm | scripts/packagerMgr.sh | 333 | t2CountNotify |
| NF_INFO_rdm_success ⚠️ | rdm-agent | src/rdm_downloadmgr.c | 319 | t2ValNotify→t2_event_s |
| NF_INFO_rdm_success ⚠️ | rdm-agent | src/rdm_downloadmgr.c | 335 | t2ValNotify→t2_event_s |
| PCR_split | rdkservices-cpc | DeviceProvisioning/DeviceProvisioningImplementation.cpp | 1188 | t2_event_s |
| PDRI_Version_split ⚠️ | rdkfwupdater | src/deviceutils/device_api.c | 163 | t2ValNotify→t2_event_s |
| PDRI_Version_split ⚠️ | sysint | lib/rdk/xconfImageCheck.sh | 458 | t2ValNotify |
| PDRI_Version_split ⚠️ | sysint-cpc | lib/rdk/xconfImageCheck.sh | 525 | t2ValNotify |
| PKCS11_migration_NO_opcert | lxy-cpc | scripts/lxyinit.sh | 251 | t2CountNotify |
| PKCS11_migration_NO_opcert | lxy-cpc | scripts/lxyinit.sh | 251 | t2CountNotify |
| PKCS11_migration_removing_SEcert | lxy-cpc | scripts/lxyinit.sh | 90 | t2CountNotify |
| PKCS11_migration_removing_SEcert | lxy-cpc | scripts/lxyinit.sh | 90 | t2CountNotify |
| PKCS11_migration_SEcert_removing | lxy-cpc | scripts/lxyinit.sh | 99 | t2CountNotify |
| PKCS11_migration_SEcert_removing | lxy-cpc | scripts/lxyinit.sh | 99 | t2CountNotify |
| processCrash_split | crashupload | c_sourcecode/src/scanner/scanner.c | 367 | t2ValNotify→t2_event_s |
| processCrash_split | crashupload | runDumpUpload.sh | 759 | t2ValNotify |
| processCrash_split | crashupload | uploadDumps_TestCases.md | 1588 | t2ValNotify |
| PRVMGR_ERR_File | lostandfound-cpc | ble/prvn_mgr/prvn_mgr.c | 1398 | t2_event_d |
| PRVMGR_ERR_PollingTimeout | lostandfound-cpc | ble/prvn_mgr/prvn_mgr.c | 655 | t2_event_d |
| PRVMGR_ERR_XW3RegFail | lostandfound-cpc | ble/prvn_mgr/prvn_mgr.c | 1023 | t2_event_d |
| PRVMGR_INFO_GotXBOId | lostandfound-cpc | ble/prvn_mgr/prvn_mgr.c | 498 | t2_event_d |
| PRVMGR_INFO_MobileDisconn | lostandfound-cpc | ble/prvn_mgr/prvn_mgr.c | 1304 | t2_event_d |
| PRVMGR_INFO_MobilePaired | lostandfound-cpc | ble/prvn_mgr/prvn_mgr.c | 1015 | t2_event_d |
| PRVMGR_INFO_Provisioned | lostandfound-cpc | ble/prvn_mgr/prvn_mgr.c | 1470 | t2_event_d |
| PRVMGR_INFO_PRVSuccess | lostandfound-cpc | ble/prvn_mgr/prvn_mgr.c | 1232 | t2_event_d |
| PRVMGR_INFO_StartBeacon | lostandfound-cpc | ble/prvn_mgr/prvn_mgr.c | 988 | t2_event_d |
| PRVMGR_INFO_WIFIAssfail | lostandfound-cpc | ble/prvn_mgr/prvn_mgr.c | 1186 | t2_event_d |
| PRVMGR_INFO_WIFIAssOk | lostandfound-cpc | ble/prvn_mgr/prvn_mgr.c | 1180 | t2_event_d |
| PRVMGR_INFO_XBOSuccess | lostandfound-cpc | ble/prvn_mgr/prvn_mgr.c | 1055 | t2_event_d |
| PRVMGR_INFO_XW3RegOk | lostandfound-cpc | ble/prvn_mgr/prvn_mgr.c | 1027 | t2_event_d |
| PRVMGR_split | lostandfound-cpc | ble/prvn_mgr/prvn_mgr.c | 837 | t2_event_s |
| RCU_FWver_split ⚠️ | sysint | lib/rdk/xconfImageCheck.sh | 395 | t2ValNotify |
| RCU_FWver_split ⚠️ | sysint | lib/rdk/xconfImageCheck.sh | 541 | t2ValNotify |
| RCU_FWver_split ⚠️ | sysint-cpc | lib/rdk/xconfImageCheck.sh | 422 | t2ValNotify |
| RCU_FWver_split ⚠️ | sysint-cpc | lib/rdk/xconfImageCheck.sh | 608 | t2ValNotify |
| RDM_ERR_package_failed | rdm-agent | src/rdm_downloadutils.c | 296 | t2CountNotify→t2_event_d |
| RDM_ERR_package_notfound | rdm-agent | src/rdm_downloadmgr.c | 159 | t2CountNotify→t2_event_d |
| RDM_ERR_rdm_package_notfound | rdm | scripts/downloadMgr.sh | 404 | t2CountNotify |
| RDM_ERR_rdm_retry_fail ⚠️ | rdm | scripts/downloadUtils.sh | 416 | t2CountNotify |
| RDM_ERR_rdm_retry_fail ⚠️ | rdm-agent | scripts/downloadUtils.sh | 378 | t2CountNotify |
| RDM_ERR_rsa_signature_failed ⚠️ | rdm | scripts/opensslVerifier.sh | 121 | t2CountNotify |
| RDM_ERR_rsa_signature_failed ⚠️ | rdm | scripts/downloadMgr.sh | 437 | t2CountNotify |
| RDM_ERR_rsa_signature_failed ⚠️ | rdm-agent | src/rdm_downloadmgr.c | 308 | t2CountNotify→t2_event_d |
| RDM_ERR_rsa_signature_failed ⚠️ | rdm-agent | src/rdm_downloadmgr.c | 324 | t2CountNotify→t2_event_d |
| RDM_ERR_rsa_signature_failed ⚠️ | rdm-agent | src/rdm_packagemgr.c | 232 | t2CountNotify→t2_event_d |
| RDM_INFO_AppDownloadComplete | rdm-agent | rdm_main.c | 333 | t2ValNotify→t2_event_s |
| RDM_INFO_AppDownloadComplete | rdm-agent | rdm_main.c | 370 | t2ValNotify→t2_event_s |
| RDM_INFO_AppDownloadSuccess | rdm-agent | rdm_main.c | 378 | t2CountNotify→t2_event_d |
| RDM_INFO_DefaultURL | rdm-agent | src/rdm_downloadutils.c | 102 | t2ValNotify→t2_event_s |
| RDM_INFO_DirectBlocked | rdm-agent | src/rdm_downloadutils.c | 339 | t2CountNotify→t2_event_d |
| RDM_INFO_DownloadSSRURL | rdm-agent | src/rdm_downloadutils.c | 95 | t2ValNotify→t2_event_s |
| RDM_INFO_extraction_complete | rdm-agent | src/rdm_downloadmgr.c | 299 | t2CountNotify→t2_event_d |
| RDM_INFO_package_download | rdm-agent | src/rdm_downloadutils.c | 288 | t2ValNotify→t2_event_s |
| RDM_INFO_rsa_valid_signature ⚠️ | rdm | scripts/packagerMgr.sh | 263 | t2CountNotify |
| RDM_INFO_rsa_valid_signature ⚠️ | rdm-agent | src/rdm_openssl.c | 981 | t2CountNotify→t2_event_d |
| RDM_INFO_rsa_valid_signature ⚠️ | rdm-agent | src/rdm_downloadutils.c | 629 | t2CountNotify→t2_event_d |
| RDM_INFO_rsa_valid_signature ⚠️ | rdm-agent | src/rdm_packagemgr.c | 63 | t2CountNotify→t2_event_d |
| RDM_INFO_rsa_verify_signature_failure | rdm-agent | src/rdm_openssl.c | 985 | t2CountNotify→t2_event_d |
| RDM_INFO_rsa_verify_signature_failure | rdm-agent | src/rdm_downloadutils.c | 632 | t2CountNotify→t2_event_d |
| RDMCAcert_split | rdk-ca-store-cpc | scripts/post_cadl.sh | 126 | t2ValNotify |
| RDMwebuicert_split | sslcerts-cpc | webuicerts/scripts/post_webuicerts.sh | 47 | t2ValNotify |
| RDMxPkicert_split | sslcerts-cpc | RDM-xpki-certs/scripts/post_xpkicertsdl.sh | 53 | t2ValNotify |
| Router_Discovered | networkmanager | tools/upnp/UpnpDiscoveryManager.cpp | 97 | t2_event_s |
| SCARD_INFO_emmc_noUpgd | sysint | lib/rdk/eMMC_Upgrade.sh | 135 | t2CountNotify |
| SEFS_ssa_xpki_use_static_url | ssa-cpc | ssa_top/ssa_cpc/ssa_common/providers/CA/SecureElement/scripts/rdkssaRunSEFScertifier.sh | 243 | t2CountNotify |
| SEHAL_NO_KEY | ssa-cpc | ssa_top/ssa_cpc/daemon/se05x/Scripts/check_pph.sh | 14 | t2CountNotify |
| SEHAL_NO_KEY | ssa-cpc | ssa_top/ssa_cpc/daemon/ssaecckdf/Scripts/check_pph.sh | 15 | t2CountNotify |
| SEHAL_NO_KEY | ssa-cpc | ssa_top/ssa_cpc/daemon/a5000/Scripts/check_pph.sh | 15 | t2CountNotify |
| SEHAL_NO_KEY | ssa-cpc | ssa_top/ssa_cpc/ssa_common/providers/CA/SecureElement/scripts/rdk/rdkssaRunCertChecker.sh | 83 | t2CountNotify |
| SEHAL_NON_PROD_KEY | ssa-cpc | ssa_top/ssa_cpc/daemon/se05x/Scripts/check_pph.sh | 17 | t2CountNotify |
| SEHAL_NON_PROD_KEY | ssa-cpc | ssa_top/ssa_cpc/ssa_common/providers/CA/SecureElement/scripts/rdk/rdkssaRunCertChecker.sh | 86 | t2CountNotify |
| SEHAL_SSA_DAMON_FAILED | ssa-cpc | ssa_top/ssa_cpc/daemon/se05x/Scripts/check_pph.sh | 21 | t2CountNotify |
| SEHAL_SSA_DAMON_FAILED | ssa-cpc | ssa_top/ssa_cpc/daemon/ssaecckdf/Scripts/check_pph.sh | 19 | t2CountNotify |
| SEHAL_SSA_DAMON_FAILED | ssa-cpc | ssa_top/ssa_cpc/daemon/a5000/Scripts/check_pph.sh | 19 | t2CountNotify |
| SEHAL_SSA_DAMON_FAILED | ssa-cpc | ssa_top/ssa_cpc/ssa_common/providers/CA/SecureElement/scripts/rdk/rdkssaRunCertChecker.sh | 90 | t2CountNotify |
| SESE_ssa_xpki_use_static_url | ssa-cpc | ssa_top/ssa_cpc/ssa_common/providers/CA/SecureElement/scripts/rdkssaRunSESEcertifier.sh | 238 | t2CountNotify |
| SESE_ssa_xpki_use_static_url | ssa-cpc | ssa_top/ssa_cpc/ssa_common/providers/CA/SecureElement/scripts/rdk/rdkssaRunSESEcertifier.sh | 258 | t2CountNotify |
| SHORTS_CONN_SUCCESS | sysint | lib/rdk/startStunnel.sh | 181 | t2CountNotify |
| SHORTS_DEVICE_TYPE_PROD | sysint | lib/rdk/startStunnel.sh | 120 | t2CountNotify |
| SHORTS_DEVICE_TYPE_TEST | sysint | lib/rdk/startStunnel.sh | 115 | t2CountNotify |
| SHORTS_DEVICE_TYPE_UNKNOWN | sysint | lib/rdk/startStunnel.sh | 126 | t2CountNotify |
| SHORTS_SSH_CLIENT_FAILURE | sysint | lib/rdk/startStunnel.sh | 176 | t2CountNotify |
| SHORTS_STUNNEL_CERT_FAILURE | sysint | lib/rdk/startStunnel.sh | 101 | t2CountNotify |
| SHORTS_STUNNEL_CLIENT_FAILURE | sysint | lib/rdk/startStunnel.sh | 162 | t2CountNotify |
| ssa_xpki_use_static_url | ssa-cpc | ssa_top/ssa_cpc/ssa_common/providers/CA/xpkicertifier/scripts/rdkssaRunDeviceCertifier.sh | 282 | t2CountNotify |
| ssa_xpki_use_static_url | ssa-cpc | ssa_top/ssa_cpc/ssa_common/providers/CA/xpki/scripts/rdkssacertcheck.sh | 244 | t2CountNotify |
| SYS_ERROR_S3CoreUpload_Failed ⚠️ | cpg-utils-cpc | uploadDumpsToS3.sh | 281 | t2CountNotify |
| SYS_ERROR_S3CoreUpload_Failed ⚠️ | cpg-utils-cpc | uploadDumpsToS3.sh | 281 | t2CountNotify |
| SYS_ERROR_S3CoreUpload_Failed ⚠️ | crashupload | c_sourcecode/src/upload/upload.c | 275 | t2CountNotify→t2_event_d |
| SYS_INFO_ActiveCredsMissing | rdkservices-cpc | DeviceProvisioning/rtclient.cpp | 162 | t2_event_s |
| SYS_INFO_ActiveCredsMissing | rdkservices-cpc | DeviceProvisioning/rtclient.cpp | 625 | t2_event_s |
| SYS_INFO_ActiveCredsMissing | rdkservices-cpc | DeviceProvisioning/rtclient.cpp | 745 | t2_event_s |
| SYS_INFO_CANARY_Update | rdkfwupdater | src/flash.c | 396 | flashT2CountNotify→t2_event_d |
| SYS_INFO_CodBPASS ⚠️ | rdkfwupdater | src/rdkv_upgrade.c | 825 | Upgradet2CountNotify→t2_event_d |
| SYS_INFO_CodBPASS ⚠️ | sysint | lib/rdk/userInitiatedFWDnld.sh | 538 | t2CountNotify |
| SYS_INFO_CodBPASS ⚠️ | sysint | lib/rdk/userInitiatedFWDnld.sh | 620 | t2CountNotify |
| SYS_INFO_CodBPASS ⚠️ | sysint-cpc | lib/rdk/userInitiatedFWDnld.sh | 589 | t2CountNotify |
| SYS_INFO_CodBPASS ⚠️ | sysint-cpc | lib/rdk/userInitiatedFWDnld.sh | 671 | t2CountNotify |
| SYS_INFO_CrashedContainer | crashupload | c_sourcecode/src/scanner/scanner.c | 605 | t2CountNotify→t2_event_d |
| SYS_INFO_CrashedContainer | crashupload | runDumpUpload.sh | 819 | t2CountNotify |
| SYS_INFO_CrashedContainer | crashupload | uploadDumps_TestCases.md | 1617 | t2CountNotify |
| SYS_INFO_DAC_Inject_Failed | ssa-cpc | ssa_top/ssa_cpc/utils/DACTool/rdkssa_DAC_provisioning.c | 408 | t2ValNotify→t2_event_s |
| SYS_INFO_DAC_Inject_Success | ssa-cpc | ssa_top/ssa_cpc/utils/DACTool/rdkssa_DAC_provisioning.c | 413 | t2ValNotify→t2_event_s |
| SYS_INFO_DEFER_CANARY_REBOOT | rdkfwupdater | src/flash.c | 392 | flashT2CountNotify→t2_event_d |
| SYS_INFO_DirectSuccess | rdkfwupdater | src/rdkv_upgrade.c | 1156 | Upgradet2CountNotify→t2_event_d |
| SYS_INFO_DirectSuccess | rdkfwupdater | src/rdkv_upgrade.c | 1226 | Upgradet2CountNotify→t2_event_d |
| SYS_INFO_Invoke_batterymode | telemetry | source/testApp/testCommonLibApi.c | 75 | t2_event_s |
| SYS_INFO_Matter_DAC_Status | ssa-cpc | ssa_top/ssa_cpc/utils/DACTool/rdkssa_DAC_provisioning.c | 195 | t2ValNotify→t2_event_s |
| SYS_INFO_Matter_DAC_Status | ssa-cpc | ssa_top/ssa_cpc/utils/DACTool/rdkssa_DAC_provisioning.c | 206 | t2ValNotify→t2_event_s |
| SYS_INFO_Matter_DAC_Status | ssa-cpc | ssa_top/ssa_cpc/utils/DACTool/rdkssa_DAC_provisioning.c | 210 | t2ValNotify→t2_event_s |
| SYS_INFO_MTLS_enable | rdkfwupdater | src/rdkv_upgrade.c | 1028 | Upgradet2CountNotify→t2_event_d |
| SYS_INFO_MTLS_enable | rdkfwupdater | src/rdkv_upgrade.c | 1050 | Upgradet2CountNotify→t2_event_d |
| SYS_INFO_S3CoreUploaded ⚠️ | cpg-utils-cpc | uploadDumpsToS3.sh | 289 | t2CountNotify |
| SYS_INFO_S3CoreUploaded ⚠️ | cpg-utils-cpc | uploadDumpsToS3.sh | 289 | t2CountNotify |
| SYS_INFO_S3CoreUploaded ⚠️ | crashupload | c_sourcecode/src/upload/upload.c | 297 | t2CountNotify→t2_event_d |
| SYS_INFO_swdltriggered | rdkfwupdater | src/rdkv_upgrade.c | 434 | Upgradet2CountNotify→t2_event_d |
| SYS_INFO_swdltriggered | rdkfwupdater | src/rdkv_upgrade.c | 439 | Upgradet2CountNotify→t2_event_d |
| SYS_INFO_TGZDUMP | crashupload | c_sourcecode/src/scanner/scanner.c | 489 | t2CountNotify→t2_event_d |
| SYS_INFO_TGZDUMP | crashupload | runDumpUpload.sh | 792 | t2CountNotify |
| SYS_INFO_TGZDUMP | crashupload | uploadDumps_TestCases.md | 1641 | t2CountNotify |
| SYS_INFO_xPKI_Static_Fallback ⚠️ | sslcerts-cpc | xupnpcerts/idm_certs.sh | 65 | t2ValNotify |
| SYS_INFO_xPKI_Static_Fallback ⚠️ | sslcerts-cpc | xupnpcerts/hrot_idm_certs.sh | 101 | t2ValNotify |
| SYS_INFO_xPKI_Static_Fallback ⚠️ | sslcerts-cpc | xupnpcerts/dpcg.sh | 36 | t2ValNotify |
| SYS_INFO_xPKI_Static_Fallback ⚠️ | sysint-cpc | lib/rdk/exec_curl_mtls.sh | 73 | t2ValNotify |
| SYS_INFO_xPKI_Static_Fallback ⚠️ | sysint-cpc | lib/rdk/mtlsUtils.sh | 78 | t2ValNotify |
| SYS_SH_CMReset_PingFailed | telemetry | source/testApp/testCommonLibApi.c | 71 | t2_event_s |
| SYS_SH_lighttpdCrash | telemetry | source/testApp/testCommonLibApi.c | 77 | t2_event_d |
| SYST_ERR_ | sysint | lib/rdk/core_shell.sh | 132 | t2CountNotify |
| SYST_ERR_10Times_reboot ⚠️ | reboot-manager | src/reboot_reason_classify.c | 261 | t2CountNotify→t2_event_d |
| SYST_ERR_10Times_reboot ⚠️ | reboot-manager | src/reboot_reason_classify.c | 261 | t2CountNotify→t2_event_d |
| SYST_ERR_10Times_reboot ⚠️ | sysint | lib/rdk/update_previous_reboot_info.sh | 127 | t2CountNotify |
| SYST_ERR_10Times_reboot ⚠️ | sysint | lib/rdk/update_previous_reboot_info.sh | 140 | t2CountNotify |
| SYST_ERR_10Times_reboot ⚠️ | sysint-cpc | lib/rdk/update_previous_reboot_info.sh | 152 | t2CountNotify |
| SYST_ERR_10Times_reboot ⚠️ | sysint-cpc | lib/rdk/update_previous_reboot_info.sh | 165 | t2CountNotify |
| SYST_ERR_AUTHSERVICE_Read | rdkservices-cpc | AuthService/helpers.cpp | 443 | t2_event_s |
| SYST_ERR_AuthTokenExpiry | rdkservices-cpc | AuthService/AuthServiceImplementation.cpp | 1750 | t2_event_s |
| SYST_ERR_CCNotRepsonding_reboot | sysint | lib/rdk/rebootNow.sh | 140 | t2CountNotify |
| SYST_ERR_cdl_ssr | rdkfwupdater | src/rdkv_upgrade.c | 153 | Upgradet2CountNotify→t2_event_d |
| SYST_ERR_CDLFail ⚠️ | rdkfwupdater | src/rdkv_upgrade.c | 594 | Upgradet2CountNotify→t2_event_d |
| SYST_ERR_CDLFail ⚠️ | rdkfwupdater | src/rdkv_upgrade.c | 646 | Upgradet2CountNotify→t2_event_d |
| SYST_ERR_CDLFail ⚠️ | sysint | lib/rdk/userInitiatedFWDnld.sh | 661 | t2CountNotify |
| SYST_ERR_CDLFail ⚠️ | sysint-cpc | lib/rdk/userInitiatedFWDnld.sh | 712 | t2CountNotify |
| SYST_ERR_CECBusEx | hdmicec | ccec/src/MessageDecoder.cpp | 194 | t2_event_s |
| SYST_ERR_CLIENTCERT_Fail | sysint-cpc | lib/rdk/exec_curl_mtls.sh | 104 | t2ValNotify |
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
| SYST_ERR_FailureAuthToken | rdkservices-cpc | DeviceProvisioning/DeviceProvisioningImplementation.cpp | 1326 | t2_event_s |
| SYST_ERR_FKPSError | rdkservices-cpc | DeviceProvisioning/rtcontroller.cpp | 536 | t2_event_s |
| SYST_ERR_FW_RFC_disabled ⚠️ | sysint | lib/rdk/userInitiatedFWDnld.sh | 694 | t2CountNotify |
| SYST_ERR_FW_RFC_disabled ⚠️ | sysint-cpc | lib/rdk/userInitiatedFWDnld.sh | 745 | t2CountNotify |
| SYST_ERR_FWCTNFetch | rdkfwupdater | src/chunk.c | 114 | t2CountNotify→t2_event_d |
| SYST_ERR_FWdnldFail ⚠️ | sysint | lib/rdk/userInitiatedFWDnld.sh | 401 | t2ValNotify |
| SYST_ERR_FWdnldFail ⚠️ | sysint-cpc | lib/rdk/userInitiatedFWDnld.sh | 440 | t2ValNotify |
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
| SYST_ERR_PDRI_VFail ⚠️ | sysint | lib/rdk/xconfImageCheck.sh | 453 | t2CountNotify |
| SYST_ERR_PDRI_VFail ⚠️ | sysint-cpc | lib/rdk/xconfImageCheck.sh | 520 | t2CountNotify |
| SYST_ERR_PDRIUpg_failure | rdkfwupdater | src/rdkv_upgrade.c | 589 | Upgradet2CountNotify→t2_event_d |
| SYST_ERR_PrevCDL_InProg | sysint | lib/rdk/swupdate_utility.sh | 157 | t2CountNotify |
| SYST_ERR_Process_Crash_accum | crashupload | c_sourcecode/src/scanner/scanner.c | 368 | t2ValNotify→t2_event_s |
| SYST_ERR_Process_Crash_accum | crashupload | runDumpUpload.sh | 760 | t2ValNotify |
| SYST_ERR_Process_Crash_accum | crashupload | uploadDumps_TestCases.md | 1589 | t2ValNotify |
| SYST_ERR_ProcessCrash ⚠️ | crashupload | c_sourcecode/src/scanner/scanner.c | 369 | t2CountNotify→t2_event_d |
| SYST_ERR_ProcessCrash ⚠️ | crashupload | runDumpUpload.sh | 761 | t2CountNotify |
| SYST_ERR_ProcessCrash ⚠️ | crashupload | uploadDumps_TestCases.md | 1590 | t2CountNotify |
| SYST_ERR_ProcessCrash ⚠️ | sysint | lib/rdk/core_shell.sh | 81 | t2CountNotify |
| SYST_ERR_ProvisioningFail | rdkservices-cpc | DeviceProvisioning/DeviceProvisioningImplementation.cpp | 352 | t2_event_s |
| SYST_ERR_RDMMISSING | rdm-agent | src/rdm_downloadutils.c | 118 | t2ValNotify→t2_event_s |
| SYST_ERR_RedrecoveryCert | sysint-cpc | lib/rdk/xconfImageCheck.sh | 393 | t2CountNotify |
| SYST_ERR_RFC | entservices-softwareupdate | MaintenanceManager/MaintenanceManager.cpp | 1652 | t2_event_d |
| SYST_ERR_Rmfstreamer_crash | sysint | lib/rdk/core_shell.sh | 128 | t2CountNotify |
| SYST_ERR_Rmfstreamer_reboot | sysint | lib/rdk/rebootNow.sh | 158 | t2CountNotify |
| SYST_ERR_RunPod_reboot | sysint | lib/rdk/rebootNow.sh | 137 | t2CountNotify |
| SYST_ERR_RunPod_reboot | sysint | lib/rdk/rebootNow.sh | 161 | t2CountNotify |
| SYST_ERR_S3signing_failed | cpg-utils-cpc | uploadDumpsToS3.sh | 223 | t2CountNotify |
| SYST_ERR_S3signing_failed | cpg-utils-cpc | uploadDumpsToS3.sh | 223 | t2CountNotify |
| SYST_ERR_syslogng_crash | sysint | lib/rdk/core_shell.sh | 109 | t2CountNotify |
| SYST_ERR_VodApp_restart | sysint | lib/rdk/core_shell.sh | 106 | t2CountNotify |
| SYST_ERR_XACS401 | authservice-cpc | authservice.cpp | 553 | t2_event_s |
| SYST_ERR_XCALDevice_crash | sysint | lib/rdk/core_shell.sh | 124 | t2CountNotify |
| SYST_ERR_Xconf28 ⚠️ | sysint | lib/rdk/xconfImageCheck.sh | 512 | t2CountNotify |
| SYST_ERR_Xconf28 ⚠️ | sysint | lib/rdk/xconfImageCheck.sh | 561 | t2CountNotify |
| SYST_ERR_Xconf28 ⚠️ | sysint-cpc | lib/rdk/xconfImageCheck.sh | 579 | t2CountNotify |
| SYST_ERR_Xconf28 ⚠️ | sysint-cpc | lib/rdk/xconfImageCheck.sh | 628 | t2CountNotify |
| SYST_ERR_xraudio_crash | sysint | lib/rdk/core_shell.sh | 112 | t2CountNotify |
| SYST_ERROR_WAI_InitERR | entservices-softwareupdate | MaintenanceManager/MaintenanceManager.cpp | 648 | t2_event_d |
| SYST_INFO_Act_split | rdkservices-cpc | DeviceProvisioning/DeviceProvisioningImplementation.cpp | 1223 | t2_event_s |
| SYST_INFO_Act_split | rdkservices-cpc | DeviceProvisioning/DeviceProvisioningImplementation.cpp | 1263 | t2_event_s |
| SYST_INFO_ActivReady | rdkservices-cpc | DeviceProvisioning/DeviceProvisioningImplementation.cpp | 1312 | t2_event_s |
| SYST_INFO_ACTN_SUCCESS | rdkservices-cpc | DeviceProvisioning/DeviceProvisioningImplementation.cpp | 1228 | t2_event_s |
| SYST_INFO_ACTN_SUCCESS | rdkservices-cpc | DeviceProvisioning/DeviceProvisioningImplementation.cpp | 1268 | t2_event_s |
| SYST_INFO_AuthTokenSucc | rdkservices-cpc | DeviceProvisioning/rtcontroller.cpp | 106 | t2_event_s |
| SYST_INFO_AuthTokenSucc | rdkservices-cpc | DeviceProvisioning/DeviceProvisioningImplementation.cpp | 1290 | t2_event_s |
| SYST_INFO_C_CDL | rdkfwupdater | src/rdkFwupdateMgr.c | 1132 | t2CountNotify→t2_event_d |
| SYST_INFO_C_CDL | rdkfwupdater | src/rdkv_main.c | 1090 | t2CountNotify→t2_event_d |
| SYST_INFO_cb_xconf ⚠️ | rdkfwupdater | src/rdkv_upgrade.c | 733 | Upgradet2CountNotify→t2_event_d |
| SYST_INFO_cb_xconf ⚠️ | sysint | lib/rdk/xconfImageCheck.sh | 591 | t2CountNotify |
| SYST_INFO_cb_xconf ⚠️ | sysint | lib/rdk/xconfImageCheck.sh | 667 | t2CountNotify |
| SYST_INFO_cb_xconf ⚠️ | sysint-cpc | lib/rdk/xconfImageCheck.sh | 658 | t2CountNotify |
| SYST_INFO_cb_xconf ⚠️ | sysint-cpc | lib/rdk/xconfImageCheck.sh | 734 | t2CountNotify |
| SYST_INFO_CDLSuccess ⚠️ | rdkfwupdater | src/flash.c | 137 | flashT2CountNotify→t2_event_d |
| SYST_INFO_CDLSuccess ⚠️ | sysint | lib/rdk/userInitiatedFWDnld.sh | 664 | t2CountNotify |
| SYST_INFO_CDLSuccess ⚠️ | sysint-cpc | lib/rdk/userInitiatedFWDnld.sh | 715 | t2CountNotify |
| SYST_INFO_Core_accum | sysint | lib/rdk/core_shell.sh | 166 | t2ValNotify |
| SYST_INFO_CoreFull_accum | sysint | lib/rdk/core_shell.sh | 157 | t2ValNotify |
| SYST_INFO_CoreIMP_accum | sysint | lib/rdk/core_shell.sh | 158 | t2ValNotify |
| SYST_INFO_CoreNotProcessed | sysint | lib/rdk/core_shell.sh | 365 | t2CountNotify |
| SYST_INFO_CoreProcessed | sysint | lib/rdk/core_shell.sh | 184 | t2CountNotify |
| SYST_INFO_CoreProcessed_accum | sysint | lib/rdk/core_shell.sh | 185 | t2ValNotify |
| SYST_INFO_CoreUpldSkipped | crashupload | c_sourcecode/src/utils/system_utils.c | 145 | t2CountNotify→t2_event_d |
| SYST_INFO_CoreUpldSkipped | crashupload | runDumpUpload.sh | 662 | t2CountNotify |
| SYST_INFO_CrashedProc_accum | sysint | lib/rdk/core_shell.sh | 150 | t2ValNotify |
| SYST_INFO_CURL6 ⚠️ | cpg-utils-cpc | uploadDumpsToS3.sh | 283 | t2CountNotify |
| SYST_INFO_CURL6 ⚠️ | cpg-utils-cpc | uploadDumpsToS3.sh | 283 | t2CountNotify |
| SYST_INFO_CURL6 ⚠️ | crashupload | c_sourcecode/src/upload/upload.c | 278 | t2CountNotify→t2_event_d |
| SYST_INFO_DevicenotActivated | rdkservices-cpc | DeviceProvisioning/DeviceProvisioningImplementation.cpp | 1233 | t2_event_s |
| SYST_INFO_DevicenotActivated | rdkservices-cpc | DeviceProvisioning/DeviceProvisioningImplementation.cpp | 1273 | t2_event_s |
| SYST_INFO_ETHConn | sysint | lib/rdk/networkConnectionRecovery.sh | 162 | t2CountNotify |
| SYST_INFO_FetchFWCTN | rdkfwupdater | src/chunk.c | 95 | t2CountNotify→t2_event_d |
| SYST_INFO_FWCOMPLETE ⚠️ | rdkfwupdater | src/rdkv_upgrade.c | 605 | Upgradet2CountNotify→t2_event_d |
| SYST_INFO_FWCOMPLETE ⚠️ | sysint | lib/rdk/userInitiatedFWDnld.sh | 423 | t2CountNotify |
| SYST_INFO_FWCOMPLETE ⚠️ | sysint-cpc | lib/rdk/userInitiatedFWDnld.sh | 474 | t2CountNotify |
| SYST_INFO_FWUpgrade_Exit ⚠️ | rdkfwupdater | src/device_status_helper.c | 80 | t2CountNotify→t2_event_d |
| SYST_INFO_FWUpgrade_Exit ⚠️ | sysint | lib/rdk/swupdate_utility.sh | 130 | t2CountNotify |
| SYST_INFO_Healthcheck_split | rdkservices-cpc | DeviceProvisioning/DeviceProvisioningImplementation.cpp | 331 | t2_event_s |
| SYST_INFO_Healthcheck_split | rdkservices-cpc | DeviceProvisioning/DeviceProvisioningImplementation.cpp | 660 | t2_event_s |
| SYST_INFO_Healthcheck_split | rdkservices-cpc | DeviceProvisioning/DeviceProvisioningImplementation.cpp | 796 | t2_event_s |
| SYST_INFO_Healthcheck_split | rdkservices-cpc | DeviceProvisioning/DeviceProvisioningImplementation.cpp | 929 | t2_event_s |
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
| SYST_INFO_NewXactToken_success | authservice-cpc | authservice.cpp | 335 | t2_event_s |
| SYST_INFO_NewXactToken_success | authservice-cpc | authservice.cpp | 417 | t2_event_s |
| SYST_INFO_NoConsentFlash | rdkfwupdater | src/rdkFwupdateMgr.c | 670 | t2CountNotify→t2_event_d |
| SYST_INFO_NoConsentFlash | rdkfwupdater | src/rdkv_main.c | 619 | t2CountNotify→t2_event_d |
| SYST_INFO_PartnerId | sysint | lib/rdk/getDeviceId.sh | 77 | t2ValNotify |
| SYST_INFO_PC_RF4CE | sysint | lib/rdk/core_shell.sh | 115 | t2CountNotify |
| SYST_INFO_PDRILogUpload ⚠️ | dcm-agent | uploadstblogs/src/strategies.c | 953 | t2_count_notify→t2_event_d |
| SYST_INFO_PDRILogUpload ⚠️ | sysint | lib/rdk/uploadSTBLogs.sh | 883 | t2CountNotify |
| SYST_INFO_PDRILogUpload ⚠️ | sysint | lib/rdk/uploadSTBLogs.sh | 886 | t2CountNotify |
| SYST_INFO_PDRIUpgSuccess | rdkfwupdater | src/rdkv_upgrade.c | 629 | Upgradet2CountNotify→t2_event_d |
| SYST_INFO_ProvisioingSucc | rdkservices-cpc | DeviceProvisioning/DeviceProvisioningImplementation.cpp | 737 | t2_event_s |
| SYST_INFO_PRXR_Ver_split | rdkfwupdater | src/json_process.c | 281 | t2ValNotify→t2_event_s |
| SYST_INFO_Redrecovery | sysint-cpc | lib/rdk/xconfImageCheck.sh | 388 | t2CountNotify |
| SYST_INFO_RedStateRecovery | rdkfwupdater | src/rdkv_upgrade.c | 1058 | Upgradet2CountNotify→t2_event_d |
| SYST_INFO_RedstateSet | rdkfwupdater | src/device_status_helper.c | 370 | t2CountNotify→t2_event_d |
| SYST_INFO_RTController_split | rdkservices-cpc | DeviceProvisioning/DeviceProvisioningImplementation.cpp | 925 | t2_event_s |
| SYST_INFO_RTController_split | rdkservices-cpc | DeviceProvisioning/DeviceProvisioningImplementation.cpp | 1141 | t2_event_s |
| SYST_INFO_RTController_split | rdkservices-cpc | DeviceProvisioning/DeviceProvisioningImplementation.cpp | 1347 | t2_event_s |
| SYST_INFO_SAME_FWCTN | rdkfwupdater | src/chunk.c | 101 | t2CountNotify→t2_event_d |
| SYST_INFO_SigDump_split | sysint | lib/rdk/core_shell.sh | 152 | t2ValNotify |
| SYST_INFO_SOMT | entservices-softwareupdate | MaintenanceManager/MaintenanceManager.cpp | 477 | t2_event_d |
| SYST_INFO_SwapCached_split | sysint | lib/rdk/system_info_collector.sh | 80 | t2ValNotify |
| SYST_INFO_SwapFree_split | sysint | lib/rdk/system_info_collector.sh | 86 | t2ValNotify |
| SYST_INFO_SwapTotal_split | sysint | lib/rdk/system_info_collector.sh | 83 | t2ValNotify |
| SYST_INFO_swdlSameImg ⚠️ | rdkfwupdater | src/device_status_helper.c | 912 | t2CountNotify→t2_event_d |
| SYST_INFO_swdlSameImg ⚠️ | sysint | lib/rdk/userInitiatedFWDnld.sh | 515 | t2CountNotify |
| SYST_INFO_swdlSameImg ⚠️ | sysint-cpc | lib/rdk/userInitiatedFWDnld.sh | 566 | t2CountNotify |
| SYST_INFO_SwdlSameImg_Stndby ⚠️ | rdkfwupdater | src/device_status_helper.c | 905 | t2CountNotify→t2_event_d |
| SYST_INFO_SwdlSameImg_Stndby ⚠️ | sysint | lib/rdk/userInitiatedFWDnld.sh | 519 | t2CountNotify |
| SYST_INFO_SwdlSameImg_Stndby ⚠️ | sysint-cpc | lib/rdk/userInitiatedFWDnld.sh | 570 | t2CountNotify |
| SYST_INFO_SWUpgrdChck | rdkfwupdater | src/rdkFwupdateMgr.c | 1181 | t2CountNotify→t2_event_d |
| SYST_INFO_SWUpgrdChck | rdkfwupdater | src/rdkv_main.c | 1123 | t2CountNotify→t2_event_d |
| SYST_INFO_SYSBUILD | systemtimemgr | systimerfactory/rdkdefaulttimesync.cpp | 131 | t2CountNotify→t2_event_d |
| SYST_INFO_SYSBUILD | systemtimemgr | systimerfactory/rdkdefaulttimesync.cpp | 131 | t2CountNotify→t2_event_d |
| SYST_INFO_Thrtl_Enable | rdkfwupdater | src/rdkv_upgrade.c | 989 | Upgradet2CountNotify→t2_event_d |
| SYST_INFO_TLS_xconf | rdkfwupdater | src/rdkv_upgrade.c | 978 | Upgradet2CountNotify→t2_event_d |
| SYST_INFO_TVActivated | rdkservices-cpc | DeviceProvisioning/DeviceProvisioningImplementation.cpp | 1229 | t2_event_s |
| SYST_INFO_TVActivated | rdkservices-cpc | DeviceProvisioning/DeviceProvisioningImplementation.cpp | 1269 | t2_event_s |
| SYST_INFO_v2_fetchCalled | rdkservices-cpc | DeviceProvisioning/DeviceProvisioningImplementation.cpp | 1177 | t2_event_s |
| SYST_INFO_v2FKPS_Good | rdkservices-cpc | DeviceProvisioning/DeviceProvisioningImplementation.cpp | 1191 | t2_event_s |
| SYST_INFO_v2FKPS_NoFetch | rdkservices-cpc | DeviceProvisioning/DeviceProvisioningImplementation.cpp | 1167 | t2_event_s |
| SYST_INFO_v2FKPS_provCalled | rdkservices-cpc | DeviceProvisioning/DeviceProvisioningImplementation.cpp | 1184 | t2_event_s |
| SYST_INFO_v2FKPSSuccess | rdkservices-cpc | DeviceProvisioning/DeviceProvisioningImplementation.cpp | 1195 | t2_event_s |
| SYST_INFO_WIFIConn | sysint | lib/rdk/networkConnectionRecovery.sh | 144 | t2CountNotify |
| SYST_INFO_WIFIConn | sysint | lib/rdk/networkConnectionRecovery.sh | 155 | t2CountNotify |
| SYST_INFO_Xconf200 ⚠️ | sysint | lib/rdk/xconfImageCheck.sh | 514 | t2CountNotify |
| SYST_INFO_Xconf200 ⚠️ | sysint | lib/rdk/xconfImageCheck.sh | 563 | t2CountNotify |
| SYST_INFO_Xconf200 ⚠️ | sysint-cpc | lib/rdk/xconfImageCheck.sh | 581 | t2CountNotify |
| SYST_INFO_Xconf200 ⚠️ | sysint-cpc | lib/rdk/xconfImageCheck.sh | 630 | t2CountNotify |
| SYST_INFO_XCONFConnect ⚠️ | rdkfwupdater | src/rdkv_upgrade.c | 378 | Upgradet2CountNotify→t2_event_d |
| SYST_INFO_XCONFConnect ⚠️ | sysint | lib/rdk/xconfImageCheck.sh | 505 | t2CountNotify |
| SYST_INFO_XCONFConnect ⚠️ | sysint-cpc | lib/rdk/xconfImageCheck.sh | 572 | t2CountNotify |
| SYST_SWDL_Retry_split ⚠️ | sysint | lib/rdk/userInitiatedFWDnld.sh | 598 | t2ValNotify |
| SYST_SWDL_Retry_split ⚠️ | sysint-cpc | lib/rdk/userInitiatedFWDnld.sh | 649 | t2ValNotify |
| SYST_WARN_ClkNotSet | rdkservices-cpc | DeviceProvisioning/DeviceProvisioningImplementation.cpp | 590 | t2_event_s |
| SYST_WARN_ClkNotSet | rdkservices-cpc | DeviceProvisioning/DeviceProvisioningImplementation.cpp | 1119 | t2_event_s |
| SYST_WARN_CompFail | crashupload | c_sourcecode/src/archive/archive.c | 414 | t2ValNotify→t2_event_s |
| SYST_WARN_CompFail | crashupload | runDumpUpload.sh | 1012 | t2CountNotify |
| SYST_WARN_CoreNP_accum | sysint | lib/rdk/core_shell.sh | 366 | t2ValNotify |
| SYST_WARN_dcm_curl28 | sysint | lib/rdk/xconfImageCheck.sh | 416 | t2CountNotify |
| SYST_WARN_GW100PERC_PACKETLOSS | sysint | lib/rdk/networkConnectionRecovery.sh | 249 | t2CountNotify |
| SYST_WARN_NoMinidump | crashupload | c_sourcecode/src/utils/lock_manager.c | 42 | t2CountNotify→t2_event_d |
| SYST_WARN_NoMinidump | crashupload | runDumpUpload.sh | 231 | t2CountNotify |
| SYST_WARN_UPGD_SKIP ⚠️ | rdkfwupdater | src/deviceutils/device_api.c | 921 | t2ValNotify→t2_event_s |
| SYST_WARN_UPGD_SKIP ⚠️ | sysint | lib/rdk/xconfImageCheck.sh | 158 | t2ValNotify |
| SYST_WARN_UPGD_SKIP ⚠️ | sysint-cpc | lib/rdk/xconfImageCheck.sh | 164 | t2ValNotify |
| TEST_EVENT_1 | telemetry | source/testApp/testCommonLibApi.c | 79 | t2_event_d |
| TEST_EVENT_2 | telemetry | source/testApp/testCommonLibApi.c | 81 | t2_event_s |
| TEST_lu_success ⚠️ | dcm-agent | uploadstblogs/src/path_handler.c | 532 | t2_count_notify→t2_event_d |
| TEST_lu_success ⚠️ | sysint | lib/rdk/uploadSTBLogs.sh | 619 | t2CountNotify |
| TEST_lu_success ⚠️ | sysint | lib/rdk/uploadSTBLogs.sh | 648 | t2CountNotify |
| Test_SWReset ⚠️ | reboot-manager | src/reboot_reason_classify.c | 290 | t2CountNotify→t2_event_d |
| Test_SWReset ⚠️ | reboot-manager | src/reboot_reason_classify.c | 542 | t2CountNotify→t2_event_d |
| Test_SWReset ⚠️ | reboot-manager | src/reboot_reason_classify.c | 290 | t2CountNotify→t2_event_d |
| Test_SWReset ⚠️ | reboot-manager | src/reboot_reason_classify.c | 542 | t2CountNotify→t2_event_d |
| Test_SWReset ⚠️ | sysint | lib/rdk/update_previous_reboot_info.sh | 201 | t2CountNotify |
| Test_SWReset ⚠️ | sysint-cpc | lib/rdk/update_previous_reboot_info.sh | 220 | t2CountNotify |
| TimeZone_split | sysint | lib/rdk/getTimeZone.sh | 69 | t2ValNotify |
| TR69HOSTIF_GET_1000_WITHIN_5MIN | tr69hostif | src/hostif/handlers/src/hostIf_msgHandler.cpp | 171 | t2CountNotify→t2_event_d |
| TR69HOSTIF_GET_200_WITHIN_1MIN | tr69hostif | src/hostif/handlers/src/hostIf_msgHandler.cpp | 160 | t2CountNotify→t2_event_d |
| TR69HOSTIF_GET_TIMEOUT_PARAM | tr69hostif | src/hostif/handlers/src/hostIf_msgHandler.cpp | 205 | t2ValNotify→t2_event_s |
| TR69HOSTIF_SET_1000_WITHIN_5MIN | tr69hostif | src/hostif/handlers/src/hostIf_msgHandler.cpp | 260 | t2CountNotify→t2_event_d |
| TR69HOSTIF_SET_200_WITHIN_1MIN | tr69hostif | src/hostif/handlers/src/hostIf_msgHandler.cpp | 249 | t2CountNotify→t2_event_d |
| TR69HOSTIF_SET_TIMEOUT_PARAM | tr69hostif | src/hostif/handlers/src/hostIf_msgHandler.cpp | 291 | t2ValNotify→t2_event_s |
| vmstats_split | sysint | lib/rdk/vm-statistics.sh | 32 | t2ValNotify |
| WIFI_ERROR_PSM_GetRecordFail | telemetry | source/testApp/testCommonLibApi.c | 69 | t2_event_s |
| WIFI_INFO_MvdToPrvSSID | lostandfound-cpc | src/lost_and_found.c | 1491 | laf_telemetry_event_d→t2_event_d |
| WIFIV_ERR_Lnf463 | lostandfound-cpc | src/lost_and_found.c | 854 | laf_telemetry_event_d→t2_event_d |
| WIFIV_ERR_lnf_461 | lostandfound-cpc | src/lost_and_found.c | 852 | laf_telemetry_event_d→t2_event_d |
| WIFIV_ERR_lnf_464 | lostandfound-cpc | src/lost_and_found.c | 856 | laf_telemetry_event_d→t2_event_d |
| WIFIV_ERR_lnf_466 | lostandfound-cpc | src/lost_and_found.c | 858 | laf_telemetry_event_d→t2_event_d |
| WIFIV_ERR_LnF_cred_xPKI | lostandfound-cpc | src/lost_and_found.c | 1440 | laf_telemetry_event_d→t2_event_d |
| WIFIV_ERR_LnF_lfat_XPKI | lostandfound-cpc | src/lost_and_found.c | 1376 | laf_telemetry_event_d→t2_event_d |
| WIFIV_ERR_LnF_XPKI_EAP-TLS | lostandfound-cpc | src/lost_and_found.c | 1313 | laf_telemetry_event_d→t2_event_d |
| WIFIV_ERR_reassoc | sysint | lib/rdk/networkConnectionRecovery.sh | 183 | t2CountNotify |
| WIFIV_INFO_HAL_RX_Bitrate | wifimetrics-cpc | plugin/src/StaRateInfoReader.cpp | 75 | telemetry_event_s→t2_event_s |
| WIFIV_INFO_HAL_TX_Bitrate | wifimetrics-cpc | plugin/src/StaRateInfoReader.cpp | 74 | telemetry_event_s→t2_event_s |
| WIFIV_INFO_HAL_WiFiChannelUtilization_split | wifimetrics-cpc | plugin/src/ChannelUtilizationReader.cpp | 147 | telemetry_event_s→t2_event_s |
| WIFIV_INFO_LnF_cred_xPKI | lostandfound-cpc | src/lost_and_found.c | 1463 | laf_telemetry_event_d→t2_event_d |
| WIFIV_INFO_LnF_lfat_XPKI | lostandfound-cpc | src/lost_and_found.c | 1399 | laf_telemetry_event_d→t2_event_d |
| WIFIV_INFO_LnfConnected | lostandfound-cpc | src/lost_and_found.c | 1386 | laf_telemetry_event_d→t2_event_d |
| WIFIV_SET_BGSCAN_PARAMETERS | wifioptimizer-cpc | src/main.c | 253 | telemetry_event_s→t2_event_s |
| WIFIV_WARN_LnF_xPKI | lostandfound-cpc | src/lost_and_found.c | 1329 | laf_telemetry_event_d→t2_event_d |
| WIFIV_WARN_PL_ | sysint | lib/rdk/networkConnectionRecovery.sh | 271 | t2CountNotify |
| WIFIV_WARN_PL_10PERC | sysint | lib/rdk/networkConnectionRecovery.sh | 280 | t2CountNotify |
| WPE_ERR_rtrmfplayer_crash | sysint | lib/rdk/core_shell.sh | 118 | t2CountNotify |
| WPE_INFO_MigStatus_split | entservices-migration | plugin/MigrationImplementation.cpp | 74 | t2_event_s |
| WPE_INFO_MigStatus_split | entservices-migration | plugin/MigrationImplementation.cpp | 114 | t2_event_s |
| xconf_couldnt_resolve | rdkfwupdater | src/rdkv_upgrade.c | 591 | Upgradet2CountNotify→t2_event_d |
| Xi_wifiMAC_split | sysint | lib/rdk/NM_Dispatcher.sh | 120 | t2ValNotify |
| xr_fwdnld_split | rdkfwupdater | src/rdkFwupdateMgr.c | 576 | t2ValNotify→t2_event_s |
| xr_fwdnld_split | rdkfwupdater | src/rdkv_main.c | 526 | t2ValNotify→t2_event_s |

## Dynamic Markers
Markers containing shell variables (`$var`, `${var}`) that resolve at runtime.

| Marker Pattern | Component | File Path | Line | API |
|----------------|-----------|-----------|------|-----|
| SYST_ERR_$source | sysint | lib/rdk/rebootNow.sh | 143 | t2CountNotify |
| SYST_ERR_$source_reboot | sysint | lib/rdk/rebootNow.sh | 164 | t2CountNotify |
| SYST_ERR_CrashSig$2 | sysint | lib/rdk/core_shell.sh | 153 | t2CountNotify |
| WIFIV_INFO_NO${version}ROUTE | sysint | lib/rdk/networkConnectionRecovery.sh | 258 | t2CountNotify |

## Duplicate Markers
⚠️ **CDL_INFO_inprogressExit** - Found in 2 components:
- sysint: lib/rdk/userInitiatedFWDnld.sh:755 (`t2CountNotify`)
- sysint-cpc: lib/rdk/userInitiatedFWDnld.sh:806 (`t2CountNotify`)

⚠️ **CDLrdkportal_split** - Found in 3 components:
- rdkfwupdater: src/device_status_helper.c:377 (`t2CountNotify→t2_event_d`)
- sysint: lib/rdk/userInitiatedFWDnld.sh:178 (`t2ValNotify`)
- sysint-cpc: lib/rdk/userInitiatedFWDnld.sh:179 (`t2ValNotify`)

⚠️ **certerr_split** - Found in 8 components:
- cpg-utils-cpc: uploadDumpsToS3.sh:213 (`t2ValNotify`)
- cpg-utils-cpc: uploadDumpsToS3.sh:271 (`t2ValNotify`)
- cpg-utils-cpc: uploadDumpsToS3.sh:213 (`t2ValNotify`)
- cpg-utils-cpc: uploadDumpsToS3.sh:271 (`t2ValNotify`)
- crashupload: c_sourcecode/src/upload/upload.c:267 (`t2ValNotify→t2_event_s`)
- dcm-agent: uploadstblogs/src/path_handler.c:458 (`t2_val_notify→t2_event_s`)
- rdkfwupdater: src/rdkv_upgrade.c:284 (`Upgradet2ValNotify→t2_event_s`)
- rdm: scripts/downloadUtils.sh:452 (`t2ValNotify`)
- rdm-agent: scripts/downloadUtils.sh:417 (`t2ValNotify`)
- sysint: lib/rdk/uploadSTBLogs.sh:307 (`t2ValNotify`)
- sysint: lib/rdk/xconfImageCheck.sh:408 (`t2ValNotify`)
- sysint-cpc: lib/rdk/userInitiatedFWDnld.sh:353 (`t2ValNotify`)
- sysint-cpc: lib/rdk/xconfImageCheck.sh:435 (`t2ValNotify`)
- sysint-cpc: lib/rdk/xconfImageCheck.sh:484 (`t2ValNotify`)

⚠️ **CurlRet_split** - Found in 3 components:
- rdkfwupdater: src/rdkv_main.c:1166 (`t2CountNotify→t2_event_d`)
- sysint: lib/rdk/xconfImageCheck.sh:418 (`t2ValNotify`)
- sysint-cpc: lib/rdk/userInitiatedFWDnld.sh:348 (`t2ValNotify`)
- sysint-cpc: lib/rdk/xconfImageCheck.sh:442 (`t2ValNotify`)
- sysint-cpc: lib/rdk/xconfImageCheck.sh:491 (`t2ValNotify`)

⚠️ **HDMI_DeviceInfo_split** - Found in 2 components:
- entservices-hdmicecsink: plugin/HdmiCecSinkImplementation.cpp:295 (`t2_event_s`)
- entservices-hdmicecsource: plugin/HdmiCecSourceImplementation.cpp:215 (`t2_event_s`)

⚠️ **LUCurlErr_split** - Found in 2 components:
- dcm-agent: uploadstblogs/src/path_handler.c:214 (`t2_val_notify→t2_event_s`)
- dcm-agent: uploadstblogs/src/path_handler.c:342 (`t2_val_notify→t2_event_s`)
- dcm-agent: uploadstblogs/src/path_handler.c:431 (`t2_val_notify→t2_event_s`)
- dcm-agent: uploadstblogs/src/path_handler.c:518 (`t2_val_notify→t2_event_s`)
- sysint: lib/rdk/uploadSTBLogs.sh:338 (`t2ValNotify`)
- sysint: lib/rdk/uploadSTBLogs.sh:614 (`t2ValNotify`)
- sysint: lib/rdk/uploadSTBLogs.sh:645 (`t2ValNotify`)

⚠️ **NF_ERR_rdm_filenotfound_extraction** - Found in 2 components:
- rdm: scripts/downloadUtils.sh:653 (`t2CountNotify`)
- rdm-agent: scripts/downloadUtils.sh:622 (`t2CountNotify`)

⚠️ **NF_INFO_rdm_success** - Found in 2 components:
- rdm: scripts/packagerMgr.sh:333 (`t2CountNotify`)
- rdm-agent: src/rdm_downloadmgr.c:319 (`t2ValNotify→t2_event_s`)
- rdm-agent: src/rdm_downloadmgr.c:335 (`t2ValNotify→t2_event_s`)

⚠️ **PDRI_Version_split** - Found in 3 components:
- rdkfwupdater: src/deviceutils/device_api.c:163 (`t2ValNotify→t2_event_s`)
- sysint: lib/rdk/xconfImageCheck.sh:458 (`t2ValNotify`)
- sysint-cpc: lib/rdk/xconfImageCheck.sh:525 (`t2ValNotify`)

⚠️ **RCU_FWver_split** - Found in 2 components:
- sysint: lib/rdk/xconfImageCheck.sh:395 (`t2ValNotify`)
- sysint: lib/rdk/xconfImageCheck.sh:541 (`t2ValNotify`)
- sysint-cpc: lib/rdk/xconfImageCheck.sh:422 (`t2ValNotify`)
- sysint-cpc: lib/rdk/xconfImageCheck.sh:608 (`t2ValNotify`)

⚠️ **RDM_ERR_rdm_retry_fail** - Found in 2 components:
- rdm: scripts/downloadUtils.sh:416 (`t2CountNotify`)
- rdm-agent: scripts/downloadUtils.sh:378 (`t2CountNotify`)

⚠️ **RDM_ERR_rsa_signature_failed** - Found in 2 components:
- rdm: scripts/opensslVerifier.sh:121 (`t2CountNotify`)
- rdm: scripts/downloadMgr.sh:437 (`t2CountNotify`)
- rdm-agent: src/rdm_downloadmgr.c:308 (`t2CountNotify→t2_event_d`)
- rdm-agent: src/rdm_downloadmgr.c:324 (`t2CountNotify→t2_event_d`)
- rdm-agent: src/rdm_packagemgr.c:232 (`t2CountNotify→t2_event_d`)

⚠️ **RDM_INFO_rsa_valid_signature** - Found in 2 components:
- rdm: scripts/packagerMgr.sh:263 (`t2CountNotify`)
- rdm-agent: src/rdm_openssl.c:981 (`t2CountNotify→t2_event_d`)
- rdm-agent: src/rdm_downloadutils.c:629 (`t2CountNotify→t2_event_d`)
- rdm-agent: src/rdm_packagemgr.c:63 (`t2CountNotify→t2_event_d`)

⚠️ **SYS_ERROR_S3CoreUpload_Failed** - Found in 2 components:
- cpg-utils-cpc: uploadDumpsToS3.sh:281 (`t2CountNotify`)
- cpg-utils-cpc: uploadDumpsToS3.sh:281 (`t2CountNotify`)
- crashupload: c_sourcecode/src/upload/upload.c:275 (`t2CountNotify→t2_event_d`)

⚠️ **SYS_INFO_CodBPASS** - Found in 3 components:
- rdkfwupdater: src/rdkv_upgrade.c:825 (`Upgradet2CountNotify→t2_event_d`)
- sysint: lib/rdk/userInitiatedFWDnld.sh:538 (`t2CountNotify`)
- sysint: lib/rdk/userInitiatedFWDnld.sh:620 (`t2CountNotify`)
- sysint-cpc: lib/rdk/userInitiatedFWDnld.sh:589 (`t2CountNotify`)
- sysint-cpc: lib/rdk/userInitiatedFWDnld.sh:671 (`t2CountNotify`)

⚠️ **SYS_INFO_S3CoreUploaded** - Found in 2 components:
- cpg-utils-cpc: uploadDumpsToS3.sh:289 (`t2CountNotify`)
- cpg-utils-cpc: uploadDumpsToS3.sh:289 (`t2CountNotify`)
- crashupload: c_sourcecode/src/upload/upload.c:297 (`t2CountNotify→t2_event_d`)

⚠️ **SYS_INFO_xPKI_Static_Fallback** - Found in 2 components:
- sslcerts-cpc: xupnpcerts/idm_certs.sh:65 (`t2ValNotify`)
- sslcerts-cpc: xupnpcerts/hrot_idm_certs.sh:101 (`t2ValNotify`)
- sslcerts-cpc: xupnpcerts/dpcg.sh:36 (`t2ValNotify`)
- sysint-cpc: lib/rdk/exec_curl_mtls.sh:73 (`t2ValNotify`)
- sysint-cpc: lib/rdk/mtlsUtils.sh:78 (`t2ValNotify`)

⚠️ **SYST_ERR_10Times_reboot** - Found in 3 components:
- reboot-manager: src/reboot_reason_classify.c:261 (`t2CountNotify→t2_event_d`)
- reboot-manager: src/reboot_reason_classify.c:261 (`t2CountNotify→t2_event_d`)
- sysint: lib/rdk/update_previous_reboot_info.sh:127 (`t2CountNotify`)
- sysint: lib/rdk/update_previous_reboot_info.sh:140 (`t2CountNotify`)
- sysint-cpc: lib/rdk/update_previous_reboot_info.sh:152 (`t2CountNotify`)
- sysint-cpc: lib/rdk/update_previous_reboot_info.sh:165 (`t2CountNotify`)

⚠️ **SYST_ERR_CDLFail** - Found in 3 components:
- rdkfwupdater: src/rdkv_upgrade.c:594 (`Upgradet2CountNotify→t2_event_d`)
- rdkfwupdater: src/rdkv_upgrade.c:646 (`Upgradet2CountNotify→t2_event_d`)
- sysint: lib/rdk/userInitiatedFWDnld.sh:661 (`t2CountNotify`)
- sysint-cpc: lib/rdk/userInitiatedFWDnld.sh:712 (`t2CountNotify`)

⚠️ **SYST_ERR_Curl28** - Found in 2 components:
- dcm-agent: uploadstblogs/src/path_handler.c:216 (`t2_count_notify→t2_event_d`)
- dcm-agent: uploadstblogs/src/path_handler.c:344 (`t2_count_notify→t2_event_d`)
- dcm-agent: uploadstblogs/src/path_handler.c:433 (`t2_count_notify→t2_event_d`)
- dcm-agent: uploadstblogs/src/path_handler.c:520 (`t2_count_notify→t2_event_d`)
- sysint: lib/rdk/uploadSTBLogs.sh:370 (`t2CountNotify`)

⚠️ **SYST_ERR_FW_RFC_disabled** - Found in 2 components:
- sysint: lib/rdk/userInitiatedFWDnld.sh:694 (`t2CountNotify`)
- sysint-cpc: lib/rdk/userInitiatedFWDnld.sh:745 (`t2CountNotify`)

⚠️ **SYST_ERR_FWdnldFail** - Found in 2 components:
- sysint: lib/rdk/userInitiatedFWDnld.sh:401 (`t2ValNotify`)
- sysint-cpc: lib/rdk/userInitiatedFWDnld.sh:440 (`t2ValNotify`)

⚠️ **SYST_ERR_LogUpload_Failed** - Found in 2 components:
- dcm-agent: uploadstblogs/src/event_manager.c:166 (`t2_count_notify→t2_event_d`)
- dcm-agent: uploadstblogs/src/path_handler.c:552 (`t2_count_notify→t2_event_d`)
- sysint: lib/rdk/uploadSTBLogs.sh:657 (`t2CountNotify`)
- sysint: lib/rdk/uploadSTBLogs.sh:780 (`t2CountNotify`)
- sysint: lib/rdk/uploadSTBLogs.sh:871 (`t2CountNotify`)

⚠️ **SYST_ERR_PDRI_VFail** - Found in 2 components:
- sysint: lib/rdk/xconfImageCheck.sh:453 (`t2CountNotify`)
- sysint-cpc: lib/rdk/xconfImageCheck.sh:520 (`t2CountNotify`)

⚠️ **SYST_ERR_ProcessCrash** - Found in 2 components:
- crashupload: c_sourcecode/src/scanner/scanner.c:369 (`t2CountNotify→t2_event_d`)
- crashupload: runDumpUpload.sh:761 (`t2CountNotify`)
- crashupload: uploadDumps_TestCases.md:1590 (`t2CountNotify`)
- sysint: lib/rdk/core_shell.sh:81 (`t2CountNotify`)

⚠️ **SYST_ERR_Xconf28** - Found in 2 components:
- sysint: lib/rdk/xconfImageCheck.sh:512 (`t2CountNotify`)
- sysint: lib/rdk/xconfImageCheck.sh:561 (`t2CountNotify`)
- sysint-cpc: lib/rdk/xconfImageCheck.sh:579 (`t2CountNotify`)
- sysint-cpc: lib/rdk/xconfImageCheck.sh:628 (`t2CountNotify`)

⚠️ **SYST_INFO_cb_xconf** - Found in 3 components:
- rdkfwupdater: src/rdkv_upgrade.c:733 (`Upgradet2CountNotify→t2_event_d`)
- sysint: lib/rdk/xconfImageCheck.sh:591 (`t2CountNotify`)
- sysint: lib/rdk/xconfImageCheck.sh:667 (`t2CountNotify`)
- sysint-cpc: lib/rdk/xconfImageCheck.sh:658 (`t2CountNotify`)
- sysint-cpc: lib/rdk/xconfImageCheck.sh:734 (`t2CountNotify`)

⚠️ **SYST_INFO_CDLSuccess** - Found in 3 components:
- rdkfwupdater: src/flash.c:137 (`flashT2CountNotify→t2_event_d`)
- sysint: lib/rdk/userInitiatedFWDnld.sh:664 (`t2CountNotify`)
- sysint-cpc: lib/rdk/userInitiatedFWDnld.sh:715 (`t2CountNotify`)

⚠️ **SYST_INFO_CURL6** - Found in 2 components:
- cpg-utils-cpc: uploadDumpsToS3.sh:283 (`t2CountNotify`)
- cpg-utils-cpc: uploadDumpsToS3.sh:283 (`t2CountNotify`)
- crashupload: c_sourcecode/src/upload/upload.c:278 (`t2CountNotify→t2_event_d`)

⚠️ **SYST_INFO_FWCOMPLETE** - Found in 3 components:
- rdkfwupdater: src/rdkv_upgrade.c:605 (`Upgradet2CountNotify→t2_event_d`)
- sysint: lib/rdk/userInitiatedFWDnld.sh:423 (`t2CountNotify`)
- sysint-cpc: lib/rdk/userInitiatedFWDnld.sh:474 (`t2CountNotify`)

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

⚠️ **SYST_INFO_swdlSameImg** - Found in 3 components:
- rdkfwupdater: src/device_status_helper.c:912 (`t2CountNotify→t2_event_d`)
- sysint: lib/rdk/userInitiatedFWDnld.sh:515 (`t2CountNotify`)
- sysint-cpc: lib/rdk/userInitiatedFWDnld.sh:566 (`t2CountNotify`)

⚠️ **SYST_INFO_SwdlSameImg_Stndby** - Found in 3 components:
- rdkfwupdater: src/device_status_helper.c:905 (`t2CountNotify→t2_event_d`)
- sysint: lib/rdk/userInitiatedFWDnld.sh:519 (`t2CountNotify`)
- sysint-cpc: lib/rdk/userInitiatedFWDnld.sh:570 (`t2CountNotify`)

⚠️ **SYST_INFO_Xconf200** - Found in 2 components:
- sysint: lib/rdk/xconfImageCheck.sh:514 (`t2CountNotify`)
- sysint: lib/rdk/xconfImageCheck.sh:563 (`t2CountNotify`)
- sysint-cpc: lib/rdk/xconfImageCheck.sh:581 (`t2CountNotify`)
- sysint-cpc: lib/rdk/xconfImageCheck.sh:630 (`t2CountNotify`)

⚠️ **SYST_INFO_XCONFConnect** - Found in 3 components:
- rdkfwupdater: src/rdkv_upgrade.c:378 (`Upgradet2CountNotify→t2_event_d`)
- sysint: lib/rdk/xconfImageCheck.sh:505 (`t2CountNotify`)
- sysint-cpc: lib/rdk/xconfImageCheck.sh:572 (`t2CountNotify`)

⚠️ **SYST_SWDL_Retry_split** - Found in 2 components:
- sysint: lib/rdk/userInitiatedFWDnld.sh:598 (`t2ValNotify`)
- sysint-cpc: lib/rdk/userInitiatedFWDnld.sh:649 (`t2ValNotify`)

⚠️ **SYST_WARN_UPGD_SKIP** - Found in 3 components:
- rdkfwupdater: src/deviceutils/device_api.c:921 (`t2ValNotify→t2_event_s`)
- sysint: lib/rdk/xconfImageCheck.sh:158 (`t2ValNotify`)
- sysint-cpc: lib/rdk/xconfImageCheck.sh:164 (`t2ValNotify`)

⚠️ **TEST_lu_success** - Found in 2 components:
- dcm-agent: uploadstblogs/src/path_handler.c:532 (`t2_count_notify→t2_event_d`)
- sysint: lib/rdk/uploadSTBLogs.sh:619 (`t2CountNotify`)
- sysint: lib/rdk/uploadSTBLogs.sh:648 (`t2CountNotify`)

⚠️ **Test_SWReset** - Found in 3 components:
- reboot-manager: src/reboot_reason_classify.c:290 (`t2CountNotify→t2_event_d`)
- reboot-manager: src/reboot_reason_classify.c:542 (`t2CountNotify→t2_event_d`)
- reboot-manager: src/reboot_reason_classify.c:290 (`t2CountNotify→t2_event_d`)
- reboot-manager: src/reboot_reason_classify.c:542 (`t2CountNotify→t2_event_d`)
- sysint: lib/rdk/update_previous_reboot_info.sh:201 (`t2CountNotify`)
- sysint-cpc: lib/rdk/update_previous_reboot_info.sh:220 (`t2CountNotify`)

## Unresolved Components
Components from the input file that could not be scanned.

| Component | Version | Reason |
|-----------|---------|--------|
| airplay-application-cpc | 43c9d71147fa | Clone failed for rdk-e/airplay-application-cpc |
| nuance-eve | 163ec5ef1cfa | Clone failed for rdk-e/nuance-eve |
| ralf-utils | 2eda857fd887 | Clone failed for rdkcentral/ralf-utils |
