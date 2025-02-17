/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2019 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <stddef.h>
#include <limits.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "reportprofiles.h"

#include "xconfclient.h"
#include "t2collection.h"
#include "persistence.h"
#include "t2log_wrapper.h"
#include "profile.h"
#include "profilexconf.h"
#include "t2eventreceiver.h"
#if defined(CCSP_SUPPORT_ENABLED)
#include "t2_custom.h"
#endif
#include "scheduler.h"
#include "t2markers.h"
#include "datamodel.h"
#include "msgpack.h"
#include "busInterface.h"
#include "t2parser.h"
#include "telemetry2_0.h"
#include "t2MtlsUtils.h"
#include "persistence.h"

#if defined(PRIVACYMODES_CONTROL)
#include "rdkservices_privacyutils.h"
#endif

//Including Webconfig Framework For Telemetry 2.0 As part of RDKB-28897
#define SUBDOC_COUNT    1
#define SUBDOC_NAME "telemetry"
#if defined(ENABLE_RDKB_SUPPORT)
#define WEBCONFIG_BLOB_VERSION "/nvram/telemetry_webconfig_blob_version.txt"
#elif defined(DEVICE_EXTENDER)
#define WEBCONFIG_BLOB_VERSION "/usr/opensync/data/telemetry_webconfig_blob_version.txt"
#else
#define WEBCONFIG_BLOB_VERSION "/opt/telemetry_webconfig_blob_version.txt"
#endif


//Used in check_component_crash to inform Webconfig about telemetry component crash
#define TELEMETRY_INIT_FILE_BOOTUP "/tmp/telemetry_initialized_bootup"

#define MAX_PROFILENAMES_LENGTH 2048
#define T2_VERSION_DATAMODEL_PARAM  "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.Version"

#if defined(DROP_ROOT_PRIV)
#include "cap.h"

static cap_user appcaps;
#endif

static BulkData bulkdata;
static bool rpInitialized = false;
static char *t2Version = NULL;

pthread_mutex_t rpMutex = PTHREAD_MUTEX_INITIALIZER;
T2ERROR RemovePreRPfromDisk(const char* path , hash_map_t *map);
static bool isT2MtlsEnable = false;
static bool initT2MtlsEnable = false;
#if defined(PRIVACYMODES_CONTROL)
static char* paramValue = NULL;
#endif
struct rusage pusage;
unsigned int profilemem=0;

#if defined(DROP_ROOT_PRIV)
static void drop_root()
{
    appcaps.caps = NULL;
    appcaps.user_name = NULL;
    bool ret = false;
    ret = isBlocklisted();
    if(ret) 
    {
       T2Info("NonRoot feature is disabled\n");
    }
    else 
    {
       T2Info("NonRoot feature is enabled, dropping root privileges for Telemetry 2.0 Process\n");
       init_capability();
       drop_root_caps(&appcaps);
       if(update_process_caps(&appcaps) != -1)//CID 281096: Unchecked return value (CHECKED_RETURN)
       read_capability(&appcaps);
    }
}
#endif

#if defined(FEATURE_SUPPORT_WEBCONFIG)
uint32_t getTelemetryBlobVersion(char* subdoc)
{
    T2Debug("Inside getTelemetryBlobVersion subdoc %s \n",subdoc);
    uint32_t version = 0, ret =0;
    FILE *file = NULL;
    file = fopen(WEBCONFIG_BLOB_VERSION,  "r+");
    if(file == NULL)
    {
    T2Debug("Failed to read from /nvram/telemetry_webconfig_blob_version.txt \n");
    }
    else
    {
     /* CID 157387: Unchecked return value from library */
    if ((ret = fscanf(file,"%u",&version)) != 1)
    {
	T2Debug("Failed to read version from /nvram/telemetry_webconfig_blob_version.txt \n");
    }
    T2Debug("Version of Telemetry blob is %u\n",version);
    fclose(file);
    return version;
    }
    return 0;
}


int setTelemetryBlobVersion(char* subdoc,uint32_t version)
{
    T2Debug("Inside setTelemetryBlobVersion subdoc %s version %u \n",subdoc,version);
    FILE* file  = NULL;
    file = fopen(WEBCONFIG_BLOB_VERSION,"w+");
    if(file != NULL)
    {
    fprintf(file, "%u", version);
    T2Debug("New Version of Telemetry blob is %u\n",version);
    fclose(file);
    return 0;
    }
    else
    {
    T2Error("Failed to write into /nvram/telemetry_webconfig_blob_version \n");
    }
    return -1;
}


int tele_web_config_init()
{

    char *sub_docs[SUBDOC_COUNT+1]= {SUBDOC_NAME,(char *) 0 };
    blobRegInfo *blobData = NULL,*blobDataPointer = NULL;
    int i;

    blobData = (blobRegInfo*) malloc(SUBDOC_COUNT * sizeof(blobRegInfo));
    if (blobData == NULL) {
        T2Error("%s: Malloc error\n",__FUNCTION__);
        return -1;
    }
    memset(blobData, 0, SUBDOC_COUNT * sizeof(blobRegInfo));

    blobDataPointer = blobData;
    for (i=0 ;i < SUBDOC_COUNT; i++)
    {
        strncpy(blobDataPointer->subdoc_name, sub_docs[i], sizeof(blobDataPointer->subdoc_name)-1);
        blobDataPointer++;
    }
    blobDataPointer = blobData;

    getVersion versionGet = getTelemetryBlobVersion;
    setVersion versionSet = setTelemetryBlobVersion;
    T2Debug("Calling Call Back Function \n");
    register_sub_docs(blobData,SUBDOC_COUNT,versionGet,versionSet);
    T2Debug("Called register_sub_docs Succussfully \n");
    return 0;
}
#endif // CCSP_SUPPORT_ENABLED

void ReportProfiles_Interrupt()
{
    T2Debug("%s ++in\n", __FUNCTION__);
    char* xconfProfileName = NULL ;
    if (ProfileXConf_isSet()) {
        xconfProfileName = ProfileXconf_getName();
        if (xconfProfileName) {
            SendInterruptToTimeoutThread(xconfProfileName);
            free(xconfProfileName);
        }
    }

    sendLogUploadInterruptToScheduler();
    T2Debug("%s --out\n", __FUNCTION__);
}

void ReportProfiles_TimeoutCb(char* profileName, bool isClearSeekMap)
{
    T2Info("%s ++in\n", __FUNCTION__);

    if(ProfileXConf_isNameEqual(profileName)) {
        T2Debug("isclearSeekmap = %s \n", isClearSeekMap ? "true":"false");
        ProfileXConf_notifyTimeout(isClearSeekMap, false);
    }else {
        T2Debug("isclearSeekmap = %s \n", isClearSeekMap ? "true":"false");
        NotifyTimeout(profileName, isClearSeekMap);
    }

    T2Info("%s --out\n", __FUNCTION__);
}

void ReportProfiles_ActivationTimeoutCb(char* profileName)
{
    T2Info("%s ++in\n", __FUNCTION__);

    bool isDeleteRequired = false;
    if(ProfileXConf_isNameEqual(profileName)) {
        T2Error("ActivationTimeout received for Xconf profile. Ignoring!!!! \n");
    } else {
        if (T2ERROR_SUCCESS != disableProfile(profileName, &isDeleteRequired)) {
            T2Error("Failed to disable profile after timeout: %s \n", profileName);
            return;
        }

        if (isDeleteRequired) {
            removeProfileFromDisk(REPORTPROFILES_PERSISTENCE_PATH, profileName);
            if (T2ERROR_SUCCESS != deleteProfile(profileName)) {
                T2Error("Failed to delete profile after timeout: %s \n", profileName);
            }
        }

        T2ER_StopDispatchThread();
        clearT2MarkerComponentMap();

        if(ProfileXConf_isSet())
            ProfileXConf_updateMarkerComponentMap();
        updateMarkerComponentMap();

        /* Restart DispatchThread */
        if (ProfileXConf_isSet() || getProfileCount() > 0)
            T2ER_StartDispatchThread();
    }

    T2Info("%s --out\n", __FUNCTION__);
}

T2ERROR ReportProfiles_storeMarkerEvent(char *profileName, T2Event *eventInfo) {
    T2Debug("%s ++in\n", __FUNCTION__);

    if(ProfileXConf_isNameEqual(profileName)) {
        ProfileXConf_storeMarkerEvent(eventInfo);
    }else {
        Profile_storeMarkerEvent(profileName, eventInfo);
    }

    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

T2ERROR ReportProfiles_setProfileXConf(ProfileXConf *profile) {
    T2Debug("%s ++in\n", __FUNCTION__);
    if(T2ERROR_SUCCESS != ProfileXConf_set(profile)) {
        T2Error("Failed to set XConf profile\n");
        return T2ERROR_FAILURE;
    }

    T2ER_StopDispatchThread();
    T2ER_StartDispatchThread();

    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

T2ERROR ReportProfiles_deleteProfileXConf(ProfileXConf *profile) {
    T2Debug("%s ++in\n", __FUNCTION__);
    if(ProfileXConf_isSet()) {
        T2ER_StopDispatchThread();

        clearT2MarkerComponentMap();

        updateMarkerComponentMap();

        return ProfileXConf_delete(profile);
    }
    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

T2ERROR ReportProfiles_addReportProfile(Profile *profile) {
    T2Debug("%s ++in\n", __FUNCTION__);

    if(T2ERROR_SUCCESS != addProfile(profile)) {
        T2Error("Failed to create/add new report profile : %s\n", profile->name);
        return T2ERROR_FAILURE;
    }
    if(T2ERROR_SUCCESS != enableProfile(profile->name)) {
        T2Error("Failed to enable profile : %s\n", profile->name);
        return T2ERROR_FAILURE;
    }

    T2ER_StartDispatchThread(); //Error case can be ignored as Dispatch thread may be running already

    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

T2ERROR ReportProfiles_deleteProfile(const char* profileName) {
    
    bool is_profile_enable = false;
    T2Debug("%s ++in\n", __FUNCTION__);
    is_profile_enable = isProfileEnabled(profileName);

    if(T2ERROR_SUCCESS != deleteProfile(profileName))
    {
        T2Error("Failed to delete profile : %s\n", profileName);
        T2Debug("%s --out\n", __FUNCTION__);
        return T2ERROR_FAILURE;
    }
    if(is_profile_enable == true)
    {
        T2ER_StopDispatchThread();
        clearT2MarkerComponentMap();

        if(ProfileXConf_isSet())
            ProfileXConf_updateMarkerComponentMap();
        updateMarkerComponentMap();

        /* Restart DispatchThread */
        if (ProfileXConf_isSet() || getProfileCount() > 0)
            T2ER_StartDispatchThread();
     }

    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

static void createComponentDataElements() {
    T2Debug("%s ++in\n", __FUNCTION__);
    Vector* componentList = NULL ;
    FILE* cfgReadyFlag = NULL ;
    int i = 0;
    int length = 0 ;
    getComponentsWithEventMarkers(&componentList);
    length = Vector_Size(componentList);
    for (i = 0; i < length; ++i) {
        char *compName = (char*) Vector_At(componentList,i);
        if(compName)
            regDEforCompEventList(compName, getComponentMarkerList);
    }
    cfgReadyFlag = fopen(T2_CONFIG_READY, "w+");
    if(cfgReadyFlag){
        fclose(cfgReadyFlag);
    }
    setRbusParamValue(T2_CONFIG_READY);

    T2Debug("%s --out\n", __FUNCTION__);
}

void profilemem_usage(unsigned int *value) {
    T2Debug("%s ++in\n", __FUNCTION__);
    *value = profilemem;
    T2Debug("value is %u\n", *value);
    T2Debug("%s --out\n", __FUNCTION__);
}

void T2totalmem_calculate(){
    T2Debug("%s ++in\n", __FUNCTION__);
    getrusage(RUSAGE_SELF, &pusage);
    profilemem = (unsigned int)pusage.ru_maxrss;
    T2Debug("T2 memory = %u\n", profilemem);
    T2Debug("%s --out\n", __FUNCTION__);
}

static void* reportOnDemand(void *input) {
    T2Debug("%s ++in\n", __FUNCTION__);

    char* action = (char*) input;
    if(!input){
        T2Warning("Input is NULL, no action specified \n");
        return NULL ;
    }

    T2Debug("%s : action = %s \n", __FUNCTION__ , action);
    if(!strncmp(action, ON_DEMAND_ACTION_UPLOAD, MAX_PROFILENAMES_LENGTH)) {
        T2Info("Upload XCONF report on demand \n");
        set_logdemand(true);
        generateDcaReport(false, true);
    } else if(!strncmp(action, ON_DEMAND_ACTION_ABORT, MAX_PROFILENAMES_LENGTH)){
        T2Info("Abort report on demand \n");
        ProfileXConf_terminateReport();
    } else {
        T2Warning("Unkown action - %s \n", action);
    }

    return NULL;
    T2Debug("%s --out\n", __FUNCTION__);
}

T2ERROR privacymode_do_not_share ()
{
      T2Debug("%s ++in\n", __FUNCTION__);
     #ifndef DEVICE_EXTENDER
     stopXConfClient();
     if(T2ERROR_SUCCESS == startXConfClient()) {
         T2Info("XCONF Fetch with privacymode is enabled \n");
     }else {
         T2Info("XCONF Fetch - IN PROGRESS ... Ignore current reload request \n");
        return T2ERROR_FAILURE;
     }
     return T2ERROR_SUCCESS;
     #endif
     T2Debug("%s --out\n", __FUNCTION__);
     return T2ERROR_SUCCESS;
}

T2ERROR initReportProfiles()
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if(rpInitialized) {
        T2Error("%s ReportProfiles already initialized - ignoring\n", __FUNCTION__);
        return T2ERROR_FAILURE;
    }
    if(isMtlsEnabled() == true){
        initMtls();
    }
#if defined (PRIVACYMODES_CONTROL)
    DIR *dir = opendir(PRIVACYMODE_PATH);
    if(dir == NULL){
        T2Info("Persistence folder %s not present, creating folder\n", PRIVACYMODE_PATH);
        if(mkdir(PRIVACYMODE_PATH, S_IRWXU | S_IRWXG | S_IRWXO) != 0) {
            T2Error("%s,%d: Failed to make directory : %s  \n", __FUNCTION__, __LINE__, PRIVACYMODE_PATH);
        }
    }else {
        closedir(dir);
    }
#endif

    rpInitialized = true;

    bulkdata.enable = false;
    bulkdata.minReportInterval = 10;
    bulkdata.protocols = strdup("HTTP");
    bulkdata.encodingTypes = strdup("JSON");
    bulkdata.parameterWildcardSupported = true;
    bulkdata.maxNoOfParamReferences = MAX_PARAM_REFERENCES;
    bulkdata.maxReportSize = DEFAULT_MAX_REPORT_SIZE;

    initScheduler((TimeoutNotificationCB)ReportProfiles_TimeoutCb, (ActivationTimeoutCB)ReportProfiles_ActivationTimeoutCb, (NotifySchedulerstartCB)NotifySchedulerstart);
    initT2MarkerComponentMap();
    T2ER_Init();

    #if defined(DROP_ROOT_PRIV)
    // Drop root privileges for Telemetry 2.0, If NonRootSupport RFC is true
    drop_root();
    #endif

    t2Version = strdup("2.0.1"); // Setting the version to 2.0.1
    {
        T2Debug("T2 Version = %s\n", t2Version);
        //initProfileList();
        free(t2Version);
        // Init datamodel processing thread
        if (T2ERROR_SUCCESS == datamodel_init())
        {
#if defined(CCSP_SUPPORT_ENABLED)
            if(isRbusEnabled())
#endif
            {
                T2Debug("Enabling datamodel for report profiles in RBUS mode \n");
                callBackHandlers *interfaceListForBus = NULL;
                interfaceListForBus = (callBackHandlers*) malloc(sizeof(callBackHandlers));
                if(interfaceListForBus) {
                    interfaceListForBus->dmCallBack = datamodel_processProfile;
                    interfaceListForBus->dmMsgPckCallBackHandler = datamodel_MsgpackProcessProfile;
                    interfaceListForBus->dmSavedJsonCallBack = datamodel_getSavedJsonProfilesasString;
                    interfaceListForBus->dmSavedMsgPackCallBack = datamodel_getSavedMsgpackProfilesasString;
                    interfaceListForBus->pmCallBack = profilemem_usage;
                    interfaceListForBus->reportonDemand = reportOnDemand;
                    interfaceListForBus->privacyModesDoNotShare = privacymode_do_not_share;
                    interfaceListForBus->mprofilesdeleteDoNotShare =  deleteAllReportProfiles;
                    regDEforProfileDataModel(interfaceListForBus);

                    free(interfaceListForBus);
                } else{
                    T2Error("Unable to allocate memory for callback handler registry\n");
                }
            }
#if defined(CCSP_SUPPORT_ENABLED)
            else {
                // Register TR-181 DM for T2.0
                T2Debug("Enabling datamodel for report profiles in DBUS mode \n");
                if(0 != initTR181_dm()) {
                    T2Error("Unable to initialize TR181!!! \n");
                    datamodel_unInit();
                }
            }
#endif
            // Message pack format is supported only for reportprofile which is activated only if version is set to 2.0.1
            // Call webconfig init only when it is required and datamodel's have been initialized .
            #if defined(FEATURE_SUPPORT_WEBCONFIG)
            if(tele_web_config_init() !=0)
            {
                T2Error("Failed to intilize tele_web_config_init \n");
            }
            else
            {
                T2Debug("tele_web_config_init Successful\n");

                //Informing Webconfig about telemetry component crash
                check_component_crash(TELEMETRY_INIT_FILE_BOOTUP);

                //Touching TELEMETRY_INIT_FILE_BOOTUP during Bootup
                system("touch /tmp/telemetry_initialized_bootup");
                T2Debug(" %s Touched \n",TELEMETRY_INIT_FILE_BOOTUP);
            }
            #endif
            //Web Config Framework init ends

        }
        else
        {
            T2Error("Unable to start message processing thread!!! \n");
        }
        initProfileList();
    }
    #ifndef DEVICE_EXTENDER
    ProfileXConf_init();
    #endif
    if(ProfileXConf_isSet() || getProfileCount() > 0) {

        if(isRbusEnabled()){
            createComponentDataElements();
            getMarkerCompRbusSub(true);
        }
        T2ER_StartDispatchThread();

    }

    T2Debug("%s --out\n", __FUNCTION__);
    T2Info("Init ReportProfiles Successful\n");
    return T2ERROR_SUCCESS;
}


void generateDcaReport(bool isDelayed, bool isOnDemand) {

    if(ProfileXConf_isSet()) {
        /**
         * Field requirement - Generate the first report at early stage after around 2 mins of stabilization during boot
         * This is to make it at par with legacy dca reporting pattern
         */
        if(isDelayed) {
            T2Info("Triggering XCONF report generation during boot with delay \n");
            sleep(120);
        } else {
            T2Info("Triggering XCONF report generation \n");
        }

        ProfileXConf_notifyTimeout(false, isOnDemand);
    }
}


T2ERROR ReportProfiles_uninit( ) {
    T2Debug("%s ++in\n", __FUNCTION__);
    if(!rpInitialized) {
        T2Error("%s ReportProfiles is not initialized yet - ignoring\n", __FUNCTION__);
        return T2ERROR_FAILURE;
    }
    rpInitialized = false;
    if(isRbusEnabled())
        getMarkerCompRbusSub(false); // remove Rbus subscription
    uninitMtls();
    T2ER_Uninit();
    destroyT2MarkerComponentMap();
    uninitScheduler();

    if(t2Version && strcmp(t2Version, "2")) {
#if defined(CCSP_SUPPORT_ENABLED)
        // Unregister TR-181 DM
        unInitTR181_dm();
#endif

        // Stop datamodel processing thread;
        datamodel_unInit();

        uninitProfileList();
    }

    #ifndef DEVICE_EXTENDER
    ProfileXConf_uninit();
    #endif
    free(bulkdata.protocols);
    bulkdata.protocols = NULL ;
    free(bulkdata.encodingTypes);
    bulkdata.encodingTypes = NULL ;

    T2Debug("%s --out\n", __FUNCTION__);
    T2Info("Uninit ReportProfiles Successful\n");
    return T2ERROR_SUCCESS;
}

T2ERROR RemovePreRPfromDisk(const char* path , hash_map_t *map)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    struct dirent *entry;
    DIR *dir = opendir(path);
    if (dir == NULL) {
       T2Info("Failed to open persistence folder : %s, creating folder\n", path);
       return T2ERROR_FAILURE;
    }

    while ((entry = readdir(dir)) != NULL)
    {
       T2Info("Filename : %s \n", entry->d_name);
       if((entry->d_name[0] == '.') || (strcmp(entry->d_name, "..")==0))
           continue;

       if(NULL == hash_map_get(map, entry->d_name))
       {
          T2Debug("%s : Removed %s report profile from the disk due to coming new report profile \n", __FUNCTION__,entry->d_name);
          removeProfileFromDisk(REPORTPROFILES_PERSISTENCE_PATH, entry->d_name);
       }

    }
    closedir(dir);
    T2Debug("%s ++out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

static void freeProfilesHashMap(void *data) {
    T2Debug("%s ++in\n", __FUNCTION__);
    if(data != NULL) {
        hash_element_t *element = (hash_element_t *) data;
        if(element->key) {
            T2Debug("Freeing hash entry element for Profiles object Name:%s\n", element->key);
            free(element->key);
        }
        if (element->data)
            free(element->data);
        free(element);
    }
    T2Debug("%s --out\n", __FUNCTION__);
}

static void freeReportProfileHashMap(void *data) {
    T2Debug("%s ++in\n", __FUNCTION__);
    if (data != NULL) {
        hash_element_t *element = (hash_element_t *) data;

        if (element->key) {
            T2Debug("Freeing hash entry element for Profiles object Name:%s\n", element->key);
            free(element->key);
        }

        if (element->data) {
            T2Debug("Freeing element data\n");
            ReportProfile *entry = (ReportProfile *)element->data;

            if (entry->hash)
                free(entry->hash);

            if (entry->config)
                free(entry->config);

            free(entry);
        }

        free(element);
        element = NULL ;
    }
    T2Debug("%s --out\n", __FUNCTION__);
}

T2ERROR deleteAllReportProfiles() {
    T2Debug("%s ++in\n", __FUNCTION__);

    if (T2ERROR_SUCCESS != deleteAllProfiles(true)) {
        T2Error("Error while deleting all report profiles \n");
    }

    T2ER_StopDispatchThread();

    clearT2MarkerComponentMap();

    if(ProfileXConf_isSet())
        ProfileXConf_updateMarkerComponentMap();

    /* Restart DispatchThread */
    if (ProfileXConf_isSet()) {
        T2ER_StopDispatchThread();
        T2ER_StartDispatchThread();
    }

    T2Debug("%s --out\n", __FUNCTION__);

    return T2ERROR_SUCCESS;
}

void ReportProfiles_ProcessReportProfilesBlob(cJSON *profiles_root , bool rprofiletypes) {

    T2Debug("%s ++in\n", __FUNCTION__);
    if(profiles_root == NULL) {
        T2Error("Profile profiles_root is null . Unable to ReportProfiles_ProcessReportProfilesBlob \n");
        T2Debug("%s --out\n", __FUNCTION__);
        return;
    }
#if defined(PRIVACYMODES_CONTROL)
    getParameterValue(PRIVACYMODES_RFC, &paramValue);
    if(strcmp(paramValue, "DO_NOT_SHARE") == 0){
        T2Warning("Privacy Mode is DO_NOT_SHARE. Reportprofiles is not supported\n");
        free(paramValue);
        paramValue = NULL;
        return;
    }
#endif
    cJSON *profilesArray = cJSON_GetObjectItem(profiles_root, "profiles");
    uint32_t profiles_count = cJSON_GetArraySize(profilesArray);

    T2Info("Number of report profiles in current configuration is %u \n", profiles_count);
    if(profiles_count == 0) {
	if(rprofiletypes == T2_TEMP_RP) {
            T2Info("Empty report profiles are not valid configuration for temporary report profiles. \n");
	}else {
            T2Debug("Empty report profiles in configuration. Delete all active profiles. \n");
            if (T2ERROR_SUCCESS != deleteAllReportProfiles())
                T2Error("Failed to delete all profiles \n");
        }
        T2Debug("%s --out\n", __FUNCTION__);
        return;
    }

    char* profileName = NULL;
    uint32_t profileIndex = 0;

    hash_map_t *profileHashMap = getProfileHashMap();
    hash_map_t *receivedProfileHashMap = hash_map_create();
    // Rbus subscription of Tr181 datamodel events
    if(isRbusEnabled())
        getMarkerCompRbusSub(false);
    // Populate profile hash map for current configuration
    for( profileIndex = 0; profileIndex < profiles_count; profileIndex++ ) {
        cJSON* singleProfile = cJSON_GetArrayItem(profilesArray, profileIndex);
        if(singleProfile == NULL) {
            T2Error("Incomplete profile information, unable to create profile for index %u \n", profileIndex);
            continue;
        }

        cJSON* nameObj = cJSON_GetObjectItem(singleProfile, "name");
        cJSON* hashObj = cJSON_GetObjectItem(singleProfile, "hash");
        if(hashObj == NULL) {
            hashObj = cJSON_GetObjectItem(singleProfile, "versionHash");
        }
        cJSON* profileObj = cJSON_GetObjectItem(singleProfile, "value");

        if(nameObj == NULL || hashObj == NULL || profileObj == NULL || strcmp(nameObj->valuestring, "") == 0 || strcmp(hashObj->valuestring, "") == 0 ) {
            T2Error("Incomplete profile object information, unable to create profile\n");
            continue;
        }

        ReportProfile *profileEntry = (ReportProfile *)malloc(sizeof(ReportProfile));
        profileName = strdup(nameObj->valuestring);
        profileEntry->hash = strdup(hashObj->valuestring);
        profileEntry->config = cJSON_PrintUnformatted(profileObj);
        hash_map_put(receivedProfileHashMap, profileName, profileEntry, freeReportProfileHashMap);

    } // End of looping through report profiles

    // Delete profiles not present in the new profile list
    char *profileNameKey = NULL;
    int count = hash_map_count(profileHashMap) - 1;
    const char *DirPath = NULL;

    if (rprofiletypes == T2_RP){
        DirPath=REPORTPROFILES_PERSISTENCE_PATH;
    }else if (rprofiletypes == T2_TEMP_RP) {
        DirPath=SHORTLIVED_PROFILES_PATH;
    }

    bool rm_flag = false;
    if(rprofiletypes == T2_RP) {

        while(count >= 0) {
            profileNameKey = hash_map_lookupKey(profileHashMap, count--);
            T2Debug("%s Map content from disk = %s \n", __FUNCTION__ , profileNameKey);
            if(NULL == hash_map_get(receivedProfileHashMap, profileNameKey)) {
                T2Debug("%s Profile %s not present in current config . Remove profile from disk \n", __FUNCTION__, profileNameKey);
                removeProfileFromDisk(DirPath, profileNameKey);
                T2Debug("%s Terminate profile %s \n", __FUNCTION__, profileNameKey);
                ReportProfiles_deleteProfile(profileNameKey);
                rm_flag = true;
            }
        }

        if(T2ERROR_SUCCESS != RemovePreRPfromDisk(DirPath , receivedProfileHashMap)) {
            T2Error("Failed to remove previous report profile from the disk\n");
        }
    }

    if(isRbusEnabled())
        unregisterDEforCompEventList();

    for( profileIndex = 0; profileIndex < hash_map_count(receivedProfileHashMap); profileIndex++ ) {
        ReportProfile *profileEntry = (ReportProfile *)hash_map_lookup(receivedProfileHashMap, profileIndex);
        profileName = hash_map_lookupKey(receivedProfileHashMap, profileIndex);

        char *existingProfileHash = hash_map_remove(profileHashMap, profileName);
        if(existingProfileHash != NULL) {

            if(!strcmp(existingProfileHash, profileEntry->hash)) {
                T2Debug("%s Profile hash for %s is same as previous profile, ignore processing config\n", __FUNCTION__, profileName);
                free(existingProfileHash);
                continue;
            } else {
                Profile *profile = 0;
                free(existingProfileHash);

                if(T2ERROR_SUCCESS == processConfiguration(&(profileEntry->config), profileName, profileEntry->hash, &profile)) { //CHECK if process configuration should have locking mechanism
                    if (profile->reportOnUpdate){
                         T2Info("%s Profile %s present in current config and hash value is different. Generating  cjson report for the profile. \n", __FUNCTION__, profileName);
                         NotifyTimeout(profileName, true);
                    }
                    if(T2ERROR_SUCCESS != saveConfigToFile(DirPath, profile->name, profileEntry->config)) {
                        T2Error("Unable to save profile : %s to disk\n", profile->name);
                    }

                    if(T2ERROR_SUCCESS == ReportProfiles_deleteProfile(profile->name)) {
                        ReportProfiles_addReportProfile(profile);
                            if(rprofiletypes == T2_RP) {
                                rm_flag = true;
                            }
                    }
                }
                else
                {
                    T2Error("Unable to parse the profile: %s, invalid configuration\n", profileName);
                }
            }
        }
        else
        {
            T2Debug("%s Previous entry for profile %s not found . Adding new profile.\n", __FUNCTION__, profileName);
            Profile *profile = 0;

            if(T2ERROR_SUCCESS == processConfiguration(&(profileEntry->config), profileName, profileEntry->hash, &profile)) { //CHECK if process configuration should have locking mechanism

                if(T2ERROR_SUCCESS != saveConfigToFile(DirPath, profile->name, profileEntry->config)) {
                    T2Error("Unable to save profile : %s to disk\n", profile->name);
                }

                ReportProfiles_addReportProfile(profile);
                if(rprofiletypes == T2_RP) {
                    rm_flag = true;
                }
            } else {
                T2Error("Unable to parse the profile: %s, invalid configuration\n", profileName);
            }
        }
    }

    if (rm_flag) {
	removeProfileFromDisk(DirPath, MSGPACK_REPORTPROFILES_PERSISTENT_FILE);
	T2Info("%s is removed from disk \n", MSGPACK_REPORTPROFILES_PERSISTENT_FILE);
    }
    //To calculate the memory when the profiles are assigned
    T2totalmem_calculate(); 

    if(isRbusEnabled()) {
        createComponentDataElements();
        // Notify registered components that profile has received an update
        publishEventsProfileUpdates();
        getMarkerCompRbusSub(true);
    }
    hash_map_destroy(receivedProfileHashMap, freeReportProfileHashMap);
    hash_map_destroy(profileHashMap, freeProfilesHashMap);
    T2Debug("%s --out\n", __FUNCTION__);
    return;
}

static void __msgpack_free_blob(void *user_data)
{
    struct __msgpack__ *msgpack = (struct __msgpack__ *)user_data;
    free(msgpack->msgpack_blob);
    free(msgpack);
}

#if defined(FEATURE_SUPPORT_WEBCONFIG)
pErr Process_Telemetry_WebConfigRequest(void *Data)
{
     T2Info("FILE:%s\t FUNCTION:%s\t LINE:%d\n", __FILE__, __FUNCTION__, __LINE__);
     pErr execRetVal=NULL;
     execRetVal = (pErr ) malloc (sizeof(Err));
     memset(execRetVal,0,(sizeof(Err)));
     T2Info("FILE:%s\t FUNCTION:%s\t LINE:%d Execution in Handler, excuted \n", __FILE__, __FUNCTION__, __LINE__);
     int retval=__ReportProfiles_ProcessReportProfilesMsgPackBlob(Data);
     if(retval == T2ERROR_SUCCESS)
     {
     	execRetVal->ErrorCode=BLOB_EXEC_SUCCESS;
     	return execRetVal;
     }
     execRetVal->ErrorCode=TELE_BLOB_PROCESSES_FAILURE;
     return execRetVal;
}

void msgpack_free_blob(void *exec_data)
{
    execData *execDataPf = (execData *)exec_data;
    __msgpack_free_blob((void *)execDataPf->user_data);
    free(execDataPf);
    execDataPf = NULL;
}

#endif

void ReportProfiles_ProcessReportProfilesMsgPackBlob(char *msgpack_blob , int msgpack_blob_size)
{
#if defined(FEATURE_SUPPORT_WEBCONFIG)
    uint64_t subdoc_version=0;
    uint16_t transac_id=0;
    int entry_count=0;
#endif
    struct __msgpack__ *msgpack = malloc(sizeof(struct __msgpack__));
    if (NULL == msgpack) {
        T2Error("Insufficient memory at Line %d on %s \n", __LINE__, __FILE__);
        return;
    }
    msgpack->msgpack_blob = msgpack_blob;
    msgpack->msgpack_blob_size = msgpack_blob_size;

    T2Debug("%s ++in\n", __FUNCTION__);
    msgpack_unpacked result;
    size_t off = 0;
    msgpack_unpack_return ret;

    // int profiles_count;
    msgpack_object *profiles_root;
    // msgpack_object *profilesArray;

    msgpack_object *subdoc_name, *transaction_id, *version;

    msgpack_unpacked_init(&result);
    ret = msgpack_unpack_next(&result, msgpack_blob, msgpack_blob_size, &off);
    if (ret != MSGPACK_UNPACK_SUCCESS) {
        T2Error("The data in the buf is invalid format.\n");
        __msgpack_free_blob((void *)msgpack);
        msgpack_unpacked_destroy(&result);
        T2Debug("%s --out\n", __FUNCTION__);
        return;
    }
    profiles_root = &result.data;
    if(profiles_root == NULL) {
        T2Error("Profile profiles_root is null . Unable to ReportProfiles_ProcessReportProfilesBlob \n");
        __msgpack_free_blob((void *)msgpack);
        msgpack_unpacked_destroy(&result);
        T2Debug("%s --out\n", __FUNCTION__);
        return;
    }

    subdoc_name = msgpack_get_map_value(profiles_root, "subdoc_name");
    transaction_id = msgpack_get_map_value(profiles_root, "transaction_id");
    version = msgpack_get_map_value(profiles_root, "version");

    msgpack_print(subdoc_name, msgpack_get_obj_name(subdoc_name));
    msgpack_print(transaction_id, msgpack_get_obj_name(transaction_id));
    msgpack_print(version, msgpack_get_obj_name(version));

    // profilesArray = msgpack_get_map_value(profiles_root, "profiles");
    // MSGPACK_GET_ARRAY_SIZE(profilesArray, profiles_count);

    /*CID 158225 - Dereference after null check */
    if (NULL == subdoc_name || NULL == transaction_id || NULL == version) {
        /* dmcli flow */
        __ReportProfiles_ProcessReportProfilesMsgPackBlob((void *)msgpack);
        __msgpack_free_blob((void *)msgpack);
        msgpack_unpacked_destroy(&result);
        T2Debug("%s --out\n", __FUNCTION__);
        /* Return - further processing are only for webconfig framework which yet to be ported to generic layer from broadband */
        return;
    }

#if defined(FEATURE_SUPPORT_WEBCONFIG)
    /* webconfig flow */
    execData *execDataPf = NULL;
    subdoc_version=(uint64_t)version->via.u64;
    transac_id=(uint16_t)transaction_id->via.u64;
    T2Debug("subdocversion is %llu transac_id in integer is %u"
            " entry_count is %d \n",(long long unsigned int)subdoc_version,transac_id,entry_count);

    execDataPf = (execData*) malloc (sizeof(execData));
    if ( NULL == execDataPf ) {
        T2Error("execData memory allocation failed\n");
        __msgpack_free_blob((void *)msgpack);
        msgpack_unpacked_destroy(&result);
        T2Debug("%s --out\n", __FUNCTION__);
        return;
    }
    memset(execDataPf, 0, sizeof(execData));
    strncpy(execDataPf->subdoc_name,"telemetry",sizeof(execDataPf->subdoc_name)-1);
    execDataPf->txid = transac_id;
    execDataPf->version = (uint32_t)subdoc_version;
    execDataPf->numOfEntries = 1;
    execDataPf->user_data = (void*)msgpack;
    execDataPf->calcTimeout = NULL;
    execDataPf->executeBlobRequest = Process_Telemetry_WebConfigRequest;
    execDataPf->rollbackFunc = NULL;
    execDataPf->freeResources = msgpack_free_blob;
    T2Debug("subdocversion is %d transac_id in integer is %d entry_count is %lu subdoc_name is %s"
            " calcTimeout is %p\n",execDataPf->version,execDataPf->txid,(ulong) execDataPf->numOfEntries,
            execDataPf->subdoc_name,execDataPf->calcTimeout);

    PushBlobRequest(execDataPf);
    T2Debug("PushBlobRequest complete\n");
    msgpack_unpacked_destroy(&result);
#endif
    T2Debug("%s --out\n", __FUNCTION__);
    return;
}

int __ReportProfiles_ProcessReportProfilesMsgPackBlob(void *msgpack)
{
#if defined(PRIVACYMODES_CONTROL)
    getParameterValue(PRIVACYMODES_RFC, &paramValue);
    if(strcmp(paramValue, "DO_NOT_SHARE") == 0){
        T2Warning("Privacy Mode is DO_NOT_SHARE. Reportprofiles is not supported\n");
        free(paramValue);
        paramValue = NULL;
        return T2ERROR_SUCCESS;
    }
#endif
    char *msgpack_blob = ((struct __msgpack__ *)msgpack)->msgpack_blob;
    int msgpack_blob_size = ((struct __msgpack__ *)msgpack)->msgpack_blob_size;

    T2Debug("%s ++in\n", __FUNCTION__);
    msgpack_unpacked result;
    size_t off = 0;
    msgpack_unpack_return ret;

    int profiles_count = 0;
    msgpack_object *profiles_root;
    msgpack_object *profilesArray;

    msgpack_unpacked_init(&result);
    ret = msgpack_unpack_next(&result, msgpack_blob, msgpack_blob_size, &off);
    if (ret != MSGPACK_UNPACK_SUCCESS) {
        T2Error("The data in the buf is invalid format.\n");
        return T2ERROR_INVALID_ARGS;
    }
    profiles_root = &result.data;
    if(profiles_root == NULL) {
        T2Error("Profile profiles_root is null . Unable to ReportProfiles_ProcessReportProfilesBlob \n");
        T2Debug("%s --out\n", __FUNCTION__);
        return T2ERROR_INVALID_ARGS;
    }

    profilesArray = msgpack_get_map_value(profiles_root, "profiles");
    MSGPACK_GET_ARRAY_SIZE(profilesArray, profiles_count);

    T2Info("Number of report profiles in current configuration is %d \n", profiles_count);
    if(profiles_count == 0) {
        T2Debug("Empty report profiles in configuration. Delete all active profiles. \n");
        if (T2ERROR_SUCCESS != deleteAllReportProfiles())
        T2Error("Failed to delete all profiles \n");
        T2Debug("%s --out\n", __FUNCTION__);
        return T2ERROR_PROFILE_NOT_FOUND;
    }
    hash_map_t *profileHashMap;
    int count;
    char *profileNameKey = NULL;
    int profileIndex;
    msgpack_object *singleProfile;
    bool profile_found_flag = false;
    bool save_flag = false;

    profileHashMap = getProfileHashMap();

    // Unregister the Component Subscriptions
    if(isRbusEnabled())
        getMarkerCompRbusSub(false);

    /* Delete profiles not present in the new profile list */
    count = hash_map_count(profileHashMap) - 1;
    while(count >= 0) {
        profile_found_flag = false;
        profileNameKey = hash_map_lookupKey(profileHashMap, count--);
        for( profileIndex = 0; profileIndex < profiles_count; profileIndex++ ) {
            singleProfile = msgpack_get_array_element(profilesArray, profileIndex);
            msgpack_object* nameObj = msgpack_get_map_value(singleProfile, "name");
            if (0 == msgpack_strcmp(nameObj, profileNameKey)) {
                T2Info("%s is found \n",profileNameKey);
                profile_found_flag = true;
                break;
            }
        }
        if (false == profile_found_flag) {
            ReportProfiles_deleteProfile(profileNameKey);
            save_flag = true;
        }
    }
  
    // Unregister the Component Event List
    if(isRbusEnabled())
        unregisterDEforCompEventList();

    /* Populate profile hash map for current configuration */
    for( profileIndex = 0; profileIndex < profiles_count; profileIndex++ ) {
        singleProfile = msgpack_get_array_element(profilesArray, profileIndex);
        if(singleProfile == NULL) {
            T2Error("Incomplete profile information, unable to create profile for index %d \n", profileIndex);
            continue;
        }
        msgpack_object* nameObj = msgpack_get_map_value(singleProfile, "name");
        msgpack_object* hashObj = msgpack_get_map_value(singleProfile, "hash");
        if (hashObj == NULL){
             T2Debug("Hash value is null checking for versionHash value \n");
             hashObj = msgpack_get_map_value(singleProfile, "versionHash");
        }
        msgpack_object* profileObj = msgpack_get_map_value(singleProfile, "value");
        if(nameObj == NULL || hashObj == NULL || profileObj == NULL || msgpack_strcmp(nameObj, "") == 0 || msgpack_strcmp(hashObj, "") == 0 ) {
            T2Error("Incomplete profile object information, unable to create profile\n");
            continue;
        }

        char *profileName = NULL;
        char *existingProfileHash = NULL;
        Profile *profile = NULL;
        profileName = msgpack_strdup(nameObj);
        existingProfileHash = hash_map_remove(profileHashMap, profileName);
        if(NULL == existingProfileHash) {
            if(T2ERROR_SUCCESS == processMsgPackConfiguration(singleProfile, &profile)) {
                ReportProfiles_addReportProfile(profile);
                populateCachedReportList(profileName, profile->cachedReportList);
                save_flag = true;
            }
        }else {
            if(0 == msgpack_strcmp(hashObj, existingProfileHash)) {
                T2Info("Profile %s with %s hash already exist \n", profileName, existingProfileHash);
                free(profileName);
                free(existingProfileHash);
                continue;
            }else {
                if(T2ERROR_SUCCESS == processMsgPackConfiguration(singleProfile, &profile)) {
                    if(profile->reportOnUpdate) {
                        T2Info("%s Profile %s present in current config and hash value is different. Generating  cjson report for the profile. \n",
                                __FUNCTION__, profileName);
                        NotifyTimeout(profileName, true);
                    }
                    if(T2ERROR_SUCCESS == ReportProfiles_deleteProfile(profile->name)) {
                        ReportProfiles_addReportProfile(profile);
                        populateCachedReportList(profileName, profile->cachedReportList);
                        save_flag = true;
                    }
                }
            }
        }
        free(profileName);
        free(existingProfileHash);
    } /* End of looping through report profiles */
    if (save_flag) {
        clearPersistenceFolder(REPORTPROFILES_PERSISTENCE_PATH);
        T2Debug("Persistent folder is cleared\n");
        MsgPackSaveConfig(REPORTPROFILES_PERSISTENCE_PATH, MSGPACK_REPORTPROFILES_PERSISTENT_FILE,
                msgpack_blob , msgpack_blob_size);
        T2Debug("%s is saved on disk \n", MSGPACK_REPORTPROFILES_PERSISTENT_FILE);
    }
    T2totalmem_calculate();

    if(isRbusEnabled()) {
        createComponentDataElements();
        // Notify registered components that profile has received an update
        publishEventsProfileUpdates();
        getMarkerCompRbusSub(true);
    }
    msgpack_unpacked_destroy(&result);
    hash_map_destroy(profileHashMap, freeProfilesHashMap);
    clearPersistenceFolder(CACHED_MESSAGE_PATH);
    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

bool isMtlsEnabled(void) 
{
#if !defined (ENABLE_RDKC_SUPPORT)
    char *paramValue = NULL;

    if(initT2MtlsEnable == false) {
       if(T2ERROR_SUCCESS == getParameterValue(T2_MTLS_RFC, &paramValue))
       {
          if(paramValue != NULL && (strncasecmp(paramValue, "true", 4) == 0)) {
             T2Debug("mTLS support is Enabled\n");
             isT2MtlsEnable = true;
          }
          initT2MtlsEnable = true;
          free(paramValue);
          paramValue = NULL;
       }
       else{
              T2Error("getParameterValue failed\n");
       }
    }
    if(isT2MtlsEnable != true)
    {
       if(T2ERROR_SUCCESS == getParameterValue(TR181_DEVICE_PARTNER_ID, &paramValue))
       {
        if(paramValue != NULL && (strncasecmp(paramValue, "sky-uk", 6) == 0))
        {
          T2Debug("Enabling mTLS for sky-uk partner\n");
          isT2MtlsEnable = true;
          initT2MtlsEnable = true;
          free(paramValue);
          paramValue = NULL;
        }
        else
        {
          if(paramValue != NULL){
               free(paramValue);
          }
          T2Error("getParameterValue partner id failed\n");
        }
      }
    }
    return isT2MtlsEnable;
#else
    /* Enabling Mtls by default for RDKC */
    return true;
#endif
}
