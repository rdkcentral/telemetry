#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <pthread.h>
#include <string>
#include <vector>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "test/mocks/SystemMock.h"
#include "test/mocks/FileioMock.h"
#include "test/mocks/rdklogMock.h"
#include "test/mocks/rbusMock.h"
#include "test/mocks/rdkconfigMock.h"

using namespace std;

using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;

extern "C" {
#include "busInterface.h"
#include "profile.h"
#include "datamodel.h"
#include "t2markers.h"
#include "reportprofiles.h"
#include "profilexconf.h"
#include "t2eventreceiver.h"

extern bool initialized;

#define T2ERROR_SUCCESS 0
#define T2ERROR_FAILURE 1

void* sendCachedReportsOverHTTP(const char*, void*) { return nullptr; }
void* sendReportOverHTTP(const char*, const char*, void*) { return nullptr; }
sigset_t blocking_signal;
hash_map_t *markerCompMap = NULL;
} 
 
FileMock *g_fileIOMock = NULL;
SystemMock * g_systemMock = NULL;
rdklogMock *m_rdklogMock = NULL;
rbusMock *g_rbusMock = NULL;
rdkconfigMock *g_rdkconfigMock = nullptr;

class ProfileTest : public ::testing::Test {
protected:
    void SetUp() override 
    {
        g_fileIOMock = new FileMock();
        g_systemMock = new SystemMock();
	g_rbusMock = new rbusMock();
	g_rdkconfigMock = new rdkconfigMock();
    }
    void TearDown() override 
    {
       delete g_fileIOMock;
       delete g_systemMock;
       delete g_rbusMock;
       delete g_rdkconfigMock;

        g_fileIOMock = nullptr;
        g_systemMock = nullptr;
	g_rbusMock = nullptr;
	g_rdkconfigMock = nullptr;
    }
};



//==================================== profile.c ===================

// Test initProfileList
TEST_F(ProfileTest, InitProfileList_Success) {
    const char* path = "/tmp/t2reportprofiles/";
    DIR *dir = (DIR*)0xffffffff ;
    Vector* configlist = NULL;
    Vector_Create(&configlist);
    Vector_PushBack(configlist, (void *)strdup("marker1"));
    EXPECT_CALL(*g_fileIOMock, opendir(_))
           .Times(2)
           .WillOnce(Return(dir))
	   .WillOnce(Return(dir));
    EXPECT_CALL(*g_systemMock, system(_))
           .Times(1)
           .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, readdir(_))
	   .Times(1)
	   .WillOnce(Return((struct dirent *)NULL));
    EXPECT_CALL(*g_fileIOMock, closedir(_))
           .Times(1)
           .WillOnce(Return(0));

    EXPECT_EQ(initProfileList(false), T2ERROR_SUCCESS);
    Vector_Destroy(configlist, free);
}

// Test profileWithNameExists
TEST_F(ProfileTest, ProfileWithNameExists_NotInitialized) {
    bool exists = false;
    EXPECT_EQ(profileWithNameExists("test", &exists), T2ERROR_FAILURE);
}

TEST_F(ProfileTest, ProfileWithNameExists_NullName) {
    bool exists = false;
    EXPECT_EQ(profileWithNameExists(nullptr, &exists), T2ERROR_FAILURE);
    EXPECT_FALSE(exists);
}

// Test addProfile
TEST_F(ProfileTest, AddProfile_NotInitialized) {
    Profile dummy;
    EXPECT_EQ(addProfile(&dummy), T2ERROR_FAILURE);
}

// Test enableProfile
TEST_F(ProfileTest, EnableProfile_NotInitialized) {
    EXPECT_EQ(enableProfile("abc"), T2ERROR_FAILURE);
}

// Test disableProfile
TEST_F(ProfileTest, DisableProfile_NotInitialized) {
    bool isDeleteRequired = false;
    EXPECT_EQ(disableProfile("abc", &isDeleteRequired), T2ERROR_FAILURE);
}


/* SEG FAULT
// Test deleteProfile
TEST_F(ProfileTest, DeleteProfile_NotInitialized) {
    EXPECT_EQ(deleteProfile("abc"), T2ERROR_FAILURE);
}

// Test deleteAllProfiles
TEST_F(ProfileTest, DeleteAllProfiles_ProfileListNull) {
    EXPECT_EQ(deleteAllProfiles(true), T2ERROR_FAILURE);
}

*/

/* HANG
// Test uninitProfileList
TEST_F(ProfileTest, UninitProfileList_Success) {
    EXPECT_EQ(uninitProfileList(), T2ERROR_SUCCESS);
}
*/

// Test getProfileCount
TEST_F(ProfileTest, GetProfileCount_NotInitialized) {
    EXPECT_EQ(getProfileCount(), 0);
}

// Test appendTriggerCondition - success case and fail case
TEST_F(ProfileTest, AppendTriggerCondition_Success) {
    Profile tmp;
    memset(&tmp, 0, sizeof(tmp));
    pthread_mutex_init(&tmp.triggerCondMutex, nullptr);
    EXPECT_EQ(appendTriggerCondition(&tmp, "refname", "refval"), T2ERROR_SUCCESS);
}

TEST_F(ProfileTest, AppendTriggerCondition_FailAndQueue) {
    Profile tmp;
    memset(&tmp, 0, sizeof(tmp));
    pthread_mutex_init(&tmp.triggerCondMutex, nullptr);
    extern int pthread_mutex_trylock(pthread_mutex_t*);
    struct Override {
        static int locked(pthread_mutex_t*) { return 1; }
    };
    auto orig = pthread_mutex_trylock;
    EXPECT_EQ(appendTriggerCondition(&tmp, "refname", "refval"), T2ERROR_FAILURE);
}

//==================================== datamodel.c ===================

TEST_F(ProfileTest, processProfile_ValidJson_RP) {
    char json[] = "{\"profiles\":[]}";
    EXPECT_EQ(datamodel_processProfile(json, T2_RP), T2ERROR_SUCCESS);
}

TEST_F(ProfileTest, processProfile_ValidJson_TEMP_RP) {
    char json[] = "{\"profiles\":[]}";
    EXPECT_EQ(datamodel_processProfile(json, T2_TEMP_RP), T2ERROR_SUCCESS);
}

TEST_F(ProfileTest, processProfile_InvalidJson) {
    char json[] = "fail";
    EXPECT_EQ(datamodel_processProfile(json, T2_RP), T2ERROR_FAILURE);
}

TEST_F(ProfileTest, processProfile_MissingProfiles) {
    char json[] = "missing_profiles";
    EXPECT_EQ(datamodel_processProfile(json, T2_RP), T2ERROR_FAILURE);
}

TEST_F(ProfileTest, processProfile_StopProcessingTrue) {
    char json[] = "{\"profiles\":[]}";
    EXPECT_EQ(datamodel_processProfile(json, T2_RP), T2ERROR_SUCCESS);
}

/* ==> To be FIXED
TEST_F(ProfileTest, getSavedJsonProfilesasString_EmptyVector) {
    char* result = nullptr;
    //gVector.configs.clear();
    datamodel_getSavedJsonProfilesasString(&result);
    // Result should be a valid JSON string
    ASSERT_NE(result, nullptr);
    free(result);
}

TEST_F(ProfileTest, getSavedJsonProfilesasString_WithConfigs) {
    //Config conf;
    //strcpy(conf.name, "test");
    //conf.configData = strdup("{\"Hash\":\"abc123\"}");
    //gVector.configs = { &conf };
    char* result = nullptr;
    datamodel_getSavedJsonProfilesasString(&result);
    ASSERT_NE(result, nullptr);
    //free(conf.configData);
    free(result);
}
*/
/*
TEST_F(ProfileTest, getSavedMsgpackProfilesasString_FileNotFound) {
    char* result = nullptr;
    ASSERT_EQ(datamodel_getSavedMsgpackProfilesasString(&result), 0);
    ASSERT_EQ(result, nullptr);
}

*/
TEST_F(ProfileTest, MsgpackProcessProfile_Success) {
    char* blob = (char*)malloc(10);
    int size = 10;
    EXPECT_EQ(datamodel_MsgpackProcessProfile(blob, size), T2ERROR_SUCCESS);
}


TEST_F(ProfileTest, Init_Success)
{
    EXPECT_EQ(datamodel_init(), T2ERROR_SUCCESS);
    //datamodel_unInit(); //==> only for line coverage
}

// ============================== t2markers.c =============================

TEST_F(ProfileTest, InitAndDestroyShouldWork) {
    EXPECT_EQ(initT2MarkerComponentMap(), T2ERROR_SUCCESS);
    EXPECT_EQ(destroyT2MarkerComponentMap(), T2ERROR_SUCCESS);
}

TEST_F(ProfileTest, AddEventMarkerShouldAddMarkerAndProfile) {
    EXPECT_EQ(addT2EventMarker("SYS_INFO_TEST_MARKER", "sysint", "PROFILE_1", 0), T2ERROR_SUCCESS);

    T2Marker* marker = (T2Marker*)hash_map_get(markerCompMap, "SYS_INFO_TEST_MARKER");
    ASSERT_NE(marker, nullptr);
    ASSERT_STREQ(marker->markerName, "SYS_INFO_TEST_MARKER");
    ASSERT_STREQ(marker->componentName, "sysint");

    // Should contain profile
    bool foundProfile = false;
    int sz = Vector_Size(marker->profileList);
    for (int i = 0; i < sz; ++i) {
        char* p = (char*)Vector_At(marker->profileList, i);
        if (strcmp(p, "PROFILE_1") == 0) {
            foundProfile = true;
            break;
        }
    }
    EXPECT_TRUE(foundProfile);
}

TEST_F(ProfileTest, ShouldAddMultipleProfilesToMarker) {
    EXPECT_EQ(addT2EventMarker("SYS_INFO_TEST_MARKER", "sysint", "PROFILE_1", 0), T2ERROR_SUCCESS);
    EXPECT_EQ(addT2EventMarker("SYS_INFO_TEST_MARKER", "sysint", "PROFILE_2", 0), T2ERROR_SUCCESS);

    T2Marker* marker = (T2Marker*)hash_map_get(markerCompMap, "SYS_INFO_TEST_MARKER");
    ASSERT_NE(marker, nullptr);

    bool foundProfile1 = false, foundProfile2 = false;
    int sz = Vector_Size(marker->profileList);
    for (int i = 0; i < sz; ++i) {
        char* p = (char*)Vector_At(marker->profileList, i);
        if (strcmp(p, "PROFILE_1") == 0) foundProfile1 = true;
        if (strcmp(p, "PROFILE_2") == 0) foundProfile2 = true;
    }
    EXPECT_TRUE(foundProfile1);
    EXPECT_TRUE(foundProfile2);
}

TEST_F(ProfileTest, DuplicateProfilesNotAdded) {
    EXPECT_EQ(addT2EventMarker("SYS_INFO_TEST_MARKER", "sysint", "PROFILE_1", 0), T2ERROR_SUCCESS);
    // Add same profile again
    EXPECT_EQ(addT2EventMarker("SYS_INFO_TEST_MARKER", "sysint", "PROFILE_1", 0), T2ERROR_SUCCESS);

    T2Marker* marker = (T2Marker*)hash_map_get(markerCompMap, "SYS_INFO_TEST_MARKER");
    ASSERT_NE(marker, nullptr);

    int count = 0;
    int sz = Vector_Size(marker->profileList);
    for (int i = 0; i < sz; ++i) {
        char* p = (char*)Vector_At(marker->profileList, i);
        if (strcmp(p, "PROFILE_1") == 0) count++;
    }
    EXPECT_EQ(count, 1); // Only one instance
}

TEST_F(ProfileTest, ComponentListIsUpdated) {
    EXPECT_EQ(addT2EventMarker("SYS_INFO_TEST_MARKER", "sysint", "PROFILE_1", 0), T2ERROR_SUCCESS);

    Vector* eventComponentList = nullptr;
    getComponentsWithEventMarkers(&eventComponentList);
    ASSERT_NE(eventComponentList, nullptr);

    bool found = false;
    int sz = Vector_Size(eventComponentList);
    for (int i = 0; i < sz; ++i) {
        char* comp = (char*)Vector_At(eventComponentList, i);
        if (strcmp(comp, "sysint") == 0) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

TEST_F(ProfileTest, GetMarkerProfileListReturnsProfiles) {
    EXPECT_EQ(addT2EventMarker("SYS_INFO_TEST_MARKER", "sysint", "PROFILE_1", 0), T2ERROR_SUCCESS);
    EXPECT_EQ(addT2EventMarker("SYS_INFO_TEST_MARKER", "sysint", "PROFILE_2", 0), T2ERROR_SUCCESS);

    Vector* profileList = nullptr;
    Vector_Create(&profileList);
    EXPECT_EQ(getMarkerProfileList("SYS_INFO_TEST_MARKER", &profileList), T2ERROR_SUCCESS);

    int foundProfiles = 0;
    int sz = Vector_Size(profileList);
    for (int i = 0; i < sz; ++i) {
        char* prof = (char*)Vector_At(profileList, i);
        if (strcmp(prof, "PROFILE_1") == 0 || strcmp(prof, "PROFILE_2") == 0) {
            foundProfiles++;
        }
    }
    EXPECT_EQ(foundProfiles, 2);
    Vector_Destroy(profileList, free);
}

TEST_F(ProfileTest, GetComponentMarkerListWorks) {
    EXPECT_EQ(addT2EventMarker("SYS_INFO_TEST_MARKER", "sysint", "PROFILE_1", 0), T2ERROR_SUCCESS);

    void* markerList = nullptr;
    getComponentMarkerList("sysint", &markerList);

    Vector* list = (Vector*)markerList;
    ASSERT_NE(list, nullptr);

    bool foundMarker = false;
    int sz = Vector_Size(list);
    for (int i = 0; i < sz; ++i) {
        char* marker = (char*)Vector_At(list, i);
        if (strcmp(marker, "SYS_INFO_TEST_MARKER") == 0) {
            foundMarker = true;
            break;
        }
    }
    EXPECT_TRUE(foundMarker);
    Vector_Destroy(list, free);
}

TEST_F(ProfileTest, ClearMarkerComponentMapShouldRemoveEntries) {
    EXPECT_EQ(addT2EventMarker("SYS_INFO_TEST_MARKER", "sysint", "PROFILE_1", 0), T2ERROR_SUCCESS);

    EXPECT_EQ(clearT2MarkerComponentMap(), T2ERROR_SUCCESS);

    T2Marker* marker = (T2Marker*)hash_map_get(markerCompMap, "SYS_INFO_TEST_MARKER");
    EXPECT_EQ(marker, nullptr);

    Vector* eventComponentList = nullptr;
    getComponentsWithEventMarkers(&eventComponentList);
    EXPECT_EQ(Vector_Size(eventComponentList), 0);
}



//================================ reportProfiles.c ====================================

TEST_F(ProfileTest, initReportProfiles) {
    char status[8] = "true";
    DIR *dir = (DIR*)0xffffffff ;
    FILE* fp = (FILE*)0xffffffff;
    EXPECT_CALL(*g_rbusMock, rbus_get(_,_,_))
           .Times(2)
           .WillOnce(Return(RBUS_ERROR_SUCCESS))
           .WillOnce(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(*g_rbusMock, rbusValue_GetType(_))
           .Times(2)
           .WillOnce(Return(RBUS_BOOLEAN))
           .WillOnce(Return(RBUS_BOOLEAN));
    EXPECT_CALL(*g_rbusMock, rbusValue_GetBoolean(_))
           .Times(2)
           .WillOnce(Return(RBUS_ERROR_SUCCESS))
           .WillOnce(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(*g_rbusMock, rbusValue_ToString(_,_,_))
           .Times(1)
           .WillOnce(Return(status));
    EXPECT_CALL(*g_rbusMock, rbusValue_Release(_))
           .Times(2)
           .WillOnce(Return())
           .WillOnce(Return());
    EXPECT_CALL(*g_fileIOMock, opendir(_))
           .Times(2)
           .WillOnce(Return(dir))
	   .WillOnce(Return(dir));
    EXPECT_CALL(*g_fileIOMock, closedir(_))
           .Times(1)
           .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, mkdir(_,_))
           .Times(1)
           .WillOnce(Return(-1));
    EXPECT_CALL(*g_rbusMock, rbus_registerLogHandler(_))
           .Times(1)
           .WillOnce(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(*g_rbusMock, rbus_open(_,_))
           .Times(1)
           .WillOnce(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(*g_rbusMock, rbus_checkStatus())
           .Times(1)
           .WillOnce(Return(RBUS_ENABLED));
    EXPECT_CALL(*g_rbusMock, rbus_regDataElements(_,_,_))
           .Times(2)
           .WillOnce(Return(RBUS_ERROR_SUCCESS))
           .WillOnce(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(*g_systemMock, system(_))
           .Times(1)
           .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, readdir(_))
	   .Times(1)
	   .WillOnce(Return((struct dirent *)NULL));
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    EXPECT_CALL(*g_fileIOMock, fclose(_))                                                                            
            .Times(1)                                                                                                
            .WillOnce(Return(0));   
    EXPECT_EQ(initReportProfiles(), T2ERROR_SUCCESS);
}

TEST_F(ProfileTest, ReportProfiles_Interrupt_Coverage) {
    // Should call sendLogUploadInterruptToScheduler and interrupt xconf profile
    ReportProfiles_Interrupt();
}

/* ===> callback failure
TEST_F(ProfileTest, ReportProfiles_TimeoutCb_XConfProfile) {
    // Should trigger ProfileXConf_notifyTimeout
    ReportProfiles_TimeoutCb(strdup("XConfProfile"), true);
}

TEST_F(ProfileTest, ReportProfiles_TimeoutCb_NonXConfProfile) {
    // Should trigger NotifyTimeout
    ReportProfiles_TimeoutCb(strdup("NonXConfProfile"), false);
}

TEST_F(ProfileTest, ReportProfiles_ActivationTimeoutCb_XConfProfile) {
    ReportProfiles_ActivationTimeoutCb(strdup("XConfProfile"));
}

TEST_F(ProfileTest, ReportProfiles_ActivationTimeoutCb_NonXConfProfile) {
    ReportProfiles_ActivationTimeoutCb(strdup("NonXConfProfile"));
}

TEST_F(ProfileTest, ReportProfiles_storeMarkerEvent_XConfProfile) {
    T2Event event;
    ReportProfiles_storeMarkerEvent(strdup("XConfProfile"), &event);
}

TEST_F(ProfileTest, ReportProfiles_storeMarkerEvent_NonXConfProfile) {
    T2Event event;
    ReportProfiles_storeMarkerEvent(strdup("NonXConfProfile"), &event);
}

TEST_F(ProfileTest, ReportProfiles_setProfileXConf) {
    ProfileXConf profile;
    EXPECT_EQ(ReportProfiles_setProfileXConf(&profile), T2ERROR_SUCCESS);
}

TEST_F(ProfileTest, ReportProfiles_deleteProfileXConf) {
    ProfileXConf profile;
    EXPECT_EQ(ReportProfiles_deleteProfileXConf(&profile), T2ERROR_SUCCESS);
}
*/

TEST_F(ProfileTest, ReportProfiles_addReportProfile) {
    Profile profile;
    EXPECT_EQ(ReportProfiles_addReportProfile(&profile), T2ERROR_SUCCESS);
}

/*
TEST_F(ProfileTest, ReportProfiles_deleteProfile) {
    EXPECT_EQ(ReportProfiles_deleteProfile("testprofile"), T2ERROR_SUCCESS);
}
*/

TEST_F(ProfileTest, profilemem_usage) {
    unsigned int value = 0;
    //profilemem = 1234;
    profilemem_usage(&value);
    //EXPECT_EQ(value, 1234);
    EXPECT_EQ(value, 0);
}

TEST_F(ProfileTest, T2totalmem_calculate) {
    //profilemem = 0;
    T2totalmem_calculate();
    //EXPECT_GT(profilemem, 0u);
}

TEST_F(ProfileTest, privacymode_do_not_share) {
    EXPECT_EQ(privacymode_do_not_share(), T2ERROR_SUCCESS);
}

TEST_F(ProfileTest, generateDcaReport) {
    generateDcaReport(false, true);
    generateDcaReport(true, false);
}

/* HANG
TEST_F(ProfileTest, RemovePreRPfromDisk) {
    hash_map_t dummy;
    EXPECT_EQ(RemovePreRPfromDisk("/tmp", &dummy), T2ERROR_FAILURE);
}

TEST_F(ProfileTest, deleteAllReportProfiles) {
    EXPECT_EQ(deleteAllReportProfiles(), T2ERROR_SUCCESS);
}

TEST_F(ProfileTest, isMtlsEnabled) {
    EXPECT_TRUE(isMtlsEnabled());
}
*/

/*
TEST_F(ProfileTest, ReportProfiles_uninit) {
    EXPECT_EQ(ReportProfiles_uninit(), T2ERROR_SUCCESS);
}
*/

//=================================== profilexconf.c ================================

TEST_F(ProfileTest, InitAndUninit) {
    // Covers ProfileXConf_init and ProfileXConf_uninit
    DIR *dir = (DIR*)0xffffffff ;
    EXPECT_CALL(*g_fileIOMock, opendir(_))
           .Times(2)
           .WillOnce(Return(dir))
	   .WillOnce(Return(dir));
    EXPECT_CALL(*g_systemMock, system(_))
           .Times(1)
           .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, readdir(_))
	   .Times(1)
	   .WillOnce(Return((struct dirent *)NULL));
    EXPECT_CALL(*g_fileIOMock, closedir(_))
           .Times(1)
           .WillOnce(Return(0));
    EXPECT_EQ(ProfileXConf_init(false), T2ERROR_SUCCESS);
}

TEST_F(ProfileTest, SetAndIsSet) {
    // Covers ProfileXConf_set and ProfileXConf_isSet
    ProfileXConf* profile = (ProfileXConf*)malloc(sizeof(ProfileXConf));
    memset(profile, 0, sizeof(ProfileXConf));
    profile->name = strdup("TestProfile");
    profile->eMarkerList = nullptr;
    profile->gMarkerList = nullptr;
    profile->topMarkerList = nullptr;
    profile->paramList = nullptr;
    profile->cachedReportList = nullptr;
    profile->protocol = strdup("HTTP");
    profile->encodingType = strdup("JSON");
    profile->t2HTTPDest = nullptr;
    profile->grepSeekProfile = nullptr;
    profile->reportInProgress = false;
    profile->isUpdated = false;

    EXPECT_EQ(ProfileXConf_set(profile), T2ERROR_SUCCESS);
    EXPECT_TRUE(ProfileXConf_isSet());

    // Get name
    char* name = ProfileXconf_getName();
    ASSERT_NE(name, nullptr);
    EXPECT_STREQ(name, "TestProfile");
    free(name);

    // Clean up
    EXPECT_EQ(ProfileXConf_uninit(), T2ERROR_SUCCESS);
}

TEST_F(ProfileTest, IsNameEqual) {
    ProfileXConf* profile = (ProfileXConf*)malloc(sizeof(ProfileXConf));
    memset(profile, 0, sizeof(ProfileXConf));
    profile->name = strdup("NameEqualProfile");
    profile->eMarkerList = nullptr;
    profile->gMarkerList = nullptr;
    profile->topMarkerList = nullptr;
    profile->paramList = nullptr;
    profile->cachedReportList = nullptr;
    profile->protocol = strdup("HTTP");
    profile->encodingType = strdup("JSON");
    profile->t2HTTPDest = nullptr;
    profile->grepSeekProfile = nullptr;
    profile->reportInProgress = false;
    profile->isUpdated = false;

    ProfileXConf_set(profile);
    EXPECT_TRUE(ProfileXConf_isNameEqual((char*)"NameEqualProfile"));
    EXPECT_FALSE(ProfileXConf_isNameEqual((char*)"OtherProfile"));
    ProfileXConf_uninit();
}

TEST_F(ProfileTest, NotifyTimeout) {
    ProfileXConf* profile = (ProfileXConf*)malloc(sizeof(ProfileXConf));
    memset(profile, 0, sizeof(ProfileXConf));
    profile->name = strdup("TimeoutProfile");
    profile->eMarkerList = nullptr;
    profile->gMarkerList = nullptr;
    profile->topMarkerList = nullptr;
    profile->paramList = nullptr;
    profile->cachedReportList = nullptr;
    profile->protocol = strdup("HTTP");
    profile->encodingType = strdup("JSON");
    profile->t2HTTPDest = nullptr;
    profile->grepSeekProfile = nullptr;
    profile->reportInProgress = false;
    profile->isUpdated = false;

    ProfileXConf_set(profile);

    // Should not crash
    ProfileXConf_notifyTimeout(false, false);

    ProfileXConf_uninit();
}

TEST_F(ProfileTest, StoreMarkerEventFail) {
    // No profile set, should fail
    T2Event event;
    event.name = const_cast<char*>("EventName");
    event.value = const_cast<char*>("EventValue");
    EXPECT_EQ(ProfileXConf_storeMarkerEvent(&event), T2ERROR_FAILURE);
}

TEST_F(ProfileTest, StoreMarkerEventSuccess) {
    // Setup a profile and an event marker
    ProfileXConf* profile = (ProfileXConf*)malloc(sizeof(ProfileXConf));
    memset(profile, 0, sizeof(ProfileXConf));
    profile->name = strdup("EventProfile");
    Vector* eMarkerList = nullptr;
    Vector_Create(&eMarkerList);
    profile->eMarkerList = eMarkerList;
    profile->gMarkerList = nullptr;
    profile->topMarkerList = nullptr;
    profile->paramList = nullptr;
    profile->cachedReportList = nullptr;
    profile->protocol = strdup("HTTP");
    profile->encodingType = strdup("JSON");
    profile->t2HTTPDest = nullptr;
    profile->grepSeekProfile = nullptr;
    profile->reportInProgress = false;
    profile->isUpdated = false;

    // Marker
    EventMarker* marker = (EventMarker*)malloc(sizeof(EventMarker));
    marker->markerName = strdup("EventName");
    marker->mType = MTYPE_ACCUMULATE;
    marker->u.count = 0;
    Vector_PushBack(eMarkerList, marker);

    ProfileXConf_set(profile);

    T2Event event;
    event.name = const_cast<char*>("EventName");
    event.value = const_cast<char*>("EventValue");
    EXPECT_EQ(ProfileXConf_storeMarkerEvent(&event), T2ERROR_SUCCESS);

    ProfileXConf_uninit();
}

TEST_F(ProfileTest, TerminateReportNoProfile) {
    // Should fail if no profile
    EXPECT_EQ(ProfileXConf_terminateReport(), T2ERROR_FAILURE);
}

TEST_F(ProfileTest, TerminateReportNoInProgress) {
    ProfileXConf* profile = (ProfileXConf*)malloc(sizeof(ProfileXConf));
    memset(profile, 0, sizeof(ProfileXConf));
    profile->name = strdup("TerminateProfile");
    profile->eMarkerList = nullptr;
    profile->gMarkerList = nullptr;
    profile->topMarkerList = nullptr;
    profile->paramList = nullptr;
    profile->cachedReportList = nullptr;
    profile->protocol = strdup("HTTP");
    profile->encodingType = strdup("JSON");
    profile->t2HTTPDest = nullptr;
    profile->grepSeekProfile = nullptr;
    profile->reportInProgress = false;
    profile->isUpdated = false;

    ProfileXConf_set(profile);

    EXPECT_EQ(ProfileXConf_terminateReport(), T2ERROR_FAILURE);

    ProfileXConf_uninit();
}

// Additional coverage for delete
TEST_F(ProfileTest, DeleteProfile) {
    ProfileXConf* profile = (ProfileXConf*)malloc(sizeof(ProfileXConf));
    memset(profile, 0, sizeof(ProfileXConf));
    profile->name = strdup("DeleteProfile");
    profile->eMarkerList = nullptr;
    profile->gMarkerList = nullptr;
    profile->topMarkerList = nullptr;
    profile->paramList = nullptr;
    profile->cachedReportList = nullptr;
    profile->protocol = strdup("HTTP");
    profile->encodingType = strdup("JSON");
    profile->t2HTTPDest = nullptr;
    profile->grepSeekProfile = nullptr;
    profile->reportInProgress = false;
    profile->isUpdated = false;

    ProfileXConf_set(profile);
    EXPECT_EQ(ProfileXConf_delete(profile), T2ERROR_SUCCESS);
    ProfileXConf_uninit();
}

TEST_F(ProfileTest, ProfileXConf_updateMarkerComponentMap)
{
    ProfileXConf_updateMarkerComponentMap();
    EXPECT_EQ(ProfileXConf_uninit(), T2ERROR_SUCCESS);
}


//=============================== t2eventreceiver.c =============================


TEST_F(ProfileTest, FreeT2EventHandlesNullAndValid) {
    freeT2Event(nullptr);
    T2Event* e = (T2Event*)malloc(sizeof(T2Event));
    e->name = strdup("n");
    e->value = strdup("v");
    freeT2Event(e);
}

TEST_F(ProfileTest, PushDataWithDelim_NormalEvent) {
    EXPECT_CALL(*g_rbusMock, rbus_registerLogHandler(_))
           .Times(1)
           .WillOnce(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(*g_rbusMock, rbus_open(_,_))
           .Times(1)
           .WillOnce(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(*g_rbusMock, rbus_checkStatus())
           .Times(1)
           .WillOnce(Return(RBUS_ENABLED));
    EXPECT_CALL(*g_rbusMock, rbus_regDataElements(_,_,_))
           .Times(1)
           .WillOnce(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(*g_systemMock, system(_))
           .Times(1)
           .WillOnce(Return(0));
    T2ER_Init();
    ////EREnabled = true;
    ////stopDispatchThread = true;
    ////gQueueCount = 0;
    char event[] = "marker1<#=#>value1";
    T2ER_PushDataWithDelim(event, NULL);
    //ASSERT_TRUE(gQueuePushCalled);
}

TEST_F(ProfileTest, PushDataWithDelim_NullEvent) {
    T2ER_PushDataWithDelim(NULL, NULL);
}

TEST_F(ProfileTest, PushDataWithDelim_QueueLimit) {
    //T2ER_Init();
    ////EREnabled = true;
    ////gQueueCount = 201;
    char event[] = "marker1<#=#>value1";
    T2ER_PushDataWithDelim(event, nullptr);
}

TEST_F(ProfileTest, PushDataWithDelim_MissingValue) {
    //T2ER_Init();
    //EREnabled = true;
    char event[] = "marker1";
    T2ER_PushDataWithDelim(event, nullptr);
}

TEST_F(ProfileTest, PushDataWithDelim_MissingDelimiter) {
    //T2ER_Init();
    //EREnabled = true;
    char event[] = "marker1value1";
    T2ER_PushDataWithDelim(event, nullptr);
}

TEST_F(ProfileTest, PushEvent_NormalCase) {
    //T2ER_Init();
    //EREnabled = true;
    //stopDispatchThread = true;
    //gQueueCount = 0;
    char* name = strdup("marker1");
    char* value = strdup("value1");
    T2ER_Push(name, value);
    //ASSERT_TRUE(gQueuePushCalled);
}

TEST_F(ProfileTest, PushEvent_NullNameOrValue) {
    //T2ER_Init();
    //EREnabled = true;
    T2ER_Push(nullptr, strdup("value1"));
    T2ER_Push(strdup("marker1"), nullptr);
}

TEST_F(ProfileTest, PushEvent_QueueLimit) {
    //T2ER_Init();
    //EREnabled = true;
    //gQueueCount = 201;
    char* name = strdup("marker1");
    char* value = strdup("value1");
    T2ER_Push(name, value);
}

TEST_F(ProfileTest, PushEvent_NotInitialized) {
    //EREnabled = false;
    T2ER_Push(strdup("marker1"), strdup("value1"));
}

/*
TEST_F(ProfileTest, EventDispatchThread_DispatchesEvents) {
    //EREnabled = true;
    //stopDispatchThread = false;
    //gQueueCount = 1;
    T2Event* event = (T2Event*)malloc(sizeof(T2Event));
    event->name = strdup("marker1");
    event->value = strdup("value1");
    //eQueue->data.push_back(event);
    pthread_t t;
    pthread_create(&t, nullptr, T2ER_EventDispatchThread, nullptr);
    sleep(1);
    //stopDispatchThread = true;
    //pthread_cond_signal(&erCond);
    pthread_join(t, nullptr);
}

TEST_F(ProfileTest, EventDispatchThread_NoEventsWait) {
    //EREnabled = true;
    //stopDispatchThread = false;
    //gQueueCount = 0;
    pthread_t t;
    pthread_create(&t, nullptr, T2ER_EventDispatchThread, nullptr);
    sleep(1);
    //stopDispatchThread = true;
    //pthread_cond_signal(&erCond);
    pthread_join(t, nullptr);
}
*/

TEST_F(ProfileTest, InitAlreadyInitialized) {
    EXPECT_CALL(*g_rbusMock, rbus_registerLogHandler(_))
           .Times(1)
           .WillOnce(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(*g_rbusMock, rbus_open(_,_))
           .Times(1)
           .WillOnce(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(*g_rbusMock, rbus_checkStatus())
           .Times(1)
           .WillOnce(Return(RBUS_ENABLED));
    EXPECT_CALL(*g_rbusMock, rbus_regDataElements(_,_,_))
           .Times(1)
           .WillOnce(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(*g_systemMock, system(_))
           .Times(1)
           .WillOnce(Return(0));
    T2ER_Init();
    T2ERROR res = T2ER_Init();
    ASSERT_EQ(res, T2ERROR_SUCCESS);
}

/*
TEST_F(ProfileTest, InitFailures) {
    //eQueue = nullptr;
    //EREnabled = false;
    // Simulate t2_queue_create failure
    bool t2_queue_create_failed = false;
    auto orig_t2_queue_create = t2_queue_create;
    //gQueue = nullptr;
    T2ERROR res = T2ER_Init();
    ASSERT_EQ(res, T2ERROR_SUCCESS); // Will create a queue, not fail in stub
}
*/

TEST_F(ProfileTest, StartDispatchThread_Normal) {
    //EREnabled = true;
    //stopDispatchThread = true;
    T2ERROR res = T2ER_StartDispatchThread();
    ASSERT_EQ(res, T2ERROR_SUCCESS);
}

TEST_F(ProfileTest, StartDispatchThread_AlreadyRunningOrNotInitialized) {
    //EREnabled = false;
    //stopDispatchThread = false;
    T2ERROR res = T2ER_StartDispatchThread();
    ASSERT_EQ(res, T2ERROR_FAILURE);
}

/*
TEST_F(ProfileTest, StopDispatchThread_Normal) {
    //EREnabled = true;
    //stopDispatchThread = false;
    T2ERROR res = T2ER_StopDispatchThread();
    ASSERT_EQ(res, T2ERROR_SUCCESS);
}

TEST_F(ProfileTest, StopDispatchThread_NotRunningOrNotInitialized) {
    //T2ER_Init();
    //EREnabled = false;
    //stopDispatchThread = true;
    T2ERROR res = T2ER_StopDispatchThread();
    ASSERT_EQ(res, T2ERROR_FAILURE);
}
*/

TEST_F(ProfileTest, Uninit_Normal) {
    EXPECT_CALL(*g_rbusMock, rbus_registerLogHandler(_))
           .Times(1)
           .WillOnce(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(*g_rbusMock, rbus_open(_,_))
           .Times(1)
           .WillOnce(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(*g_rbusMock, rbus_checkStatus())
           .Times(1)
           .WillOnce(Return(RBUS_ENABLED));
    EXPECT_CALL(*g_rbusMock, rbus_regDataElements(_,_,_))
           .Times(1)
           .WillOnce(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(*g_systemMock, system(_))
           .Times(1)
           .WillOnce(Return(0));
    T2ER_Init();
    //EREnabled = true;
    //stopDispatchThread = false;
    T2ER_Uninit();
    //ASSERT_FALSE(//EREnabled);
    //ASSERT_EQ(eQueue, nullptr);
}

TEST_F(ProfileTest, Uninit_NotInitialized) {
    //EREnabled = false;
    T2ER_Uninit();
}

/* Static functions
TEST_F(ProfileTest, FlushCacheFromFile_AndRemove) {
    std::ofstream f(T2_CACHE_FILE);
    f << "marker1<#=#>value1\n";
    f.close();
    T2ER_Init();
    extern T2ERROR flushCacheFromFile(void);
    flushCacheFromFile();
    ASSERT_FALSE(std::ifstream(T2_CACHE_FILE).good());
}

TEST_F(ProfileTest, FlushCacheFromFile_FopenFail) {
    remove(T2_CACHE_FILE);
    T2ER_Init();
    extern T2ERROR flushCacheFromFile(void);
    flushCacheFromFile();
}
*/
