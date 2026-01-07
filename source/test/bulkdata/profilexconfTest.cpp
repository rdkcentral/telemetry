/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2024 RDK Management
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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include <thread>
#include <chrono>
extern "C" {
//#include "profilexconf.h"
#include "reportprofiles.h"
#include "t2eventreceiver.h"
#include "t2markers.h"
#include "t2log_wrapper.h"
#include "busInterface.h"
#include "curlinterface.h"
#include "scheduler.h"
#include "persistence.h"
#include "vector.h"
#include "dcautil.h"
#include "t2parserxconf.h"
//#include "legacyutils.h"
#include "t2common.h"

}
#include "../mocks/rbusMock.h"
#include "../mocks/SystemMock.h"
#include "../mocks/FileioMock.h"
#include "../mocks/rdklogMock.h"
#include "SchedulerMock.h"
#include "profilexconfMock.h"

using namespace std;

using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::Invoke;                                                                                                          
using ::testing::SetArgPointee;                                                                                                   
using ::testing::DoAll;     

rbusMock* g_rbusMock = nullptr;
SystemMock* g_systemMock = nullptr;
FileMock* g_fileIOMock = nullptr;
rdklogMock* m_rdklogMock = nullptr;
profilexconfMock *g_profileXConfMock = nullptr;


// Test fixture for profilexconf tests
class profileXconfTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        
	g_rbusMock = new rbusMock();
    g_systemMock = new SystemMock();
    g_fileIOMock = new FileMock();
    g_profileXConfMock = new profilexconfMock();
    g_schedulerMock = new SchedulerMock();
    }
    void TearDown() override {
       
	delete  g_rbusMock;
    delete  g_systemMock;
    delete  g_fileIOMock;
    delete  g_profileXConfMock;
    delete  g_schedulerMock;

    g_rbusMock = nullptr;
	g_systemMock = nullptr;
    g_fileIOMock = nullptr;
    g_profileXConfMock = nullptr;
    g_schedulerMock = nullptr;
    }
};


//Test whether the single profile is set or not in the device
TEST_F(profileXconfTestFixture, ProfileXConf_isSet_before_init)
{
    EXPECT_EQ(false, ProfileXConf_isSet());
}

//Test the marker component map update before init of single profiles, it should n't update the component map
TEST_F(profileXconfTestFixture, ProfileXConf_updateMarkerComponentMap_success_before_init)
{
    ProfileXConf_updateMarkerComponentMap();

}

//Test the termination  of report generation - if the profile is no set the terminate shouldn't happen

TEST_F(profileXconfTestFixture, ProfileXConf_terminateReport_failure)
{
    EXPECT_EQ(T2ERROR_FAILURE, ProfileXConf_terminateReport());
}
//Test the init of XConf profiles
TEST_F(profileXconfTestFixture, InitandUninit)  
{    // Covers ProfileXConf_init and ProfileXConf_uninit

    DIR *dir = (DIR*)0xffffffff ;
    EXPECT_CALL(*g_fileIOMock, opendir(_))
        .Times(1)
        .WillOnce(Return((DIR*)0xffffffff));
    EXPECT_CALL(*g_fileIOMock, readdir(_))
        .Times(2)
        .WillOnce([](DIR* dirp) {
            struct dirent* entry = (struct dirent*)malloc(sizeof(struct dirent));
            strcpy(entry->d_name, "RDK_reportprofiles");
            return entry;
        })
        .WillOnce(Return(nullptr));
    EXPECT_CALL(*g_fileIOMock, open(_, _))                                                                
        .Times(1)                                                                                         
        .WillOnce(Return(0));                                                                             
    EXPECT_CALL(*g_fileIOMock, fstat(_, _))                                                               
        .Times(1)                                                                                         
        .WillOnce([](int fd, struct stat* statbuf) {                                                      
            statbuf->st_size = 1500; // Example file size 1500 bytes                                      
            return 0;
            });                                                                                               
    EXPECT_CALL(*g_fileIOMock, read(_, _, _))                                                             
        .Times(1)                                                                                         
        .WillOnce([](int fd, void* buf, size_t count) {                                                   
            size_t len = 1376; // Length of testJson                                                      
            count = len;                                                                                  
            const char* testJson = 
"{"
"\"urn:settings:GroupName\":\"Generic\","
"\"urn:settings:CheckOnReboot\":false,"
"\"urn:settings:TimeZoneMode\":\"UTC\","
"\"urn:settings:CheckSchedule:cron\":null,"
"\"urn:settings:CheckSchedule:DurationMinutes\":300,"
"\"urn:settings:LogUploadSettings:Message\":null,"
"\"urn:settings:LogUploadSettings:Name\":\"Generic\","
"\"urn:settings:LogUploadSettings:NumberOfDays\":0,"
"\"urn:settings:LogUploadSettings:UploadRepositoryName\":\"S3\","
"\"urn:settings:LogUploadSettings:UploadRepository:URL\":\"https://secure.s3.bucket.test.url\","
"\"urn:settings:LogUploadSettings:UploadRepository:uploadProtocol\":\"HTTP\","
"\"urn:settings:LogUploadSettings:UploadOnReboot\":true,"
"\"urn:settings:LogUploadSettings:UploadImmediately\":false,"
"\"urn:settings:LogUploadSettings:upload\":true,"
"\"urn:settings:LogUploadSettings:UploadSchedule:cron\":null,"
"\"urn:settings:LogUploadSettings:UploadSchedule:levelone:cron\":null,"
"\"urn:settings:LogUploadSettings:UploadSchedule:leveltwo:cron\":null,"
"\"urn:settings:LogUploadSettings:UploadSchedule:levelthree:cron\":null,"
"\"urn:settings:LogUploadSettings:UploadSchedule:TimeZoneMode\":\"UTC\","
"\"urn:settings:LogUploadSettings:UploadSchedule:DurationMinutes\":240,"
"\"urn:settings:VODSettings:Name\":null,"
"\"urn:settings:VODSettings:LocationsURL\":null,"
"\"urn:settings:VODSettings:SRMIPList\":null,"
"\"urn:settings:TelemetryProfile\":{"
    "\"@type\":\"PermanentTelemetryProfile\","
    "\"id\":\"9e9fce57-8053-423e-a647-a537ca8b7899\","
    "\"telemetryProfile\":["
        "{"
            "\"header\":\"IUI_accum\","
            "\"content\":\"Device.DeviceInfo.X_RDKCENTRAL-COM.IUI.Version\","
            "\"type\":\"<message_bus>\","
            "\"pollingFrequency\":\"0\""
        "},"
        "{"
            "\"header\":\"SYS_GREP_TEST\","
            "\"content\":\"This log file is for previous logs\","
            "\"type\":\"session0.txt\","
            "\"pollingFrequency\":\"0\""
        "},"
        "{"
            "\"header\":\"SYS_GREP_TEST_2\","
            "\"content\":\"This log file is for previous logs\","
            "\"type\":\"session0.txt\","
            "\"pollingFrequency\":\"0\""
        "},"
        "{"
            "\"header\":\"SYS_TEST_ReportUpload\","
            "\"content\":\"uploading report\","
            "\"type\":\"core_log.txt\","
            "\"pollingFrequency\":\"0\""
        "},"
        "{"
            "\"header\":\"SYS_EVENT_TEST_accum\","
           "\"content\":\"generic\","
            "\"type\":\"<event>\","
            "\"pollingFrequency\":\"0\""
        "}"
    "],"
    "\"schedule\":\"*/1 * * * *\","
    "\"expires\":0,"
    "\"telemetryProfile:name\":\"NEW TEST PROFILE\","
    "\"uploadRepository:URL\":\"https://mock1xconf:50051/dataLakeMockXconf\","
    "\"uploadRepository:uploadProtocol\":\"HTTP\""
"}"
"}";
            memcpy(buf, testJson, len);                                                                            
            return len; // Return number of bytes read                                                    
        });                                                                                               
    EXPECT_CALL(*g_fileIOMock, close(_))                                                                  
        .Times(1);           
    EXPECT_CALL(*g_fileIOMock, closedir(_))                                                               
        .Times(1);                                                 
   
    EXPECT_CALL(*g_profileXConfMock, processConfigurationXConf(_, _))
        .Times(::testing::AtMost(1))
        .WillRepeatedly([](const char* jsonStr, ProfileXConf** profile) {
            *profile = (ProfileXConf*)malloc(sizeof(ProfileXConf));
            memset(*profile, 0, sizeof(ProfileXConf));
            (*profile)->name = strdup("RDK_Profile");
            Vector_Create(&(*profile)->eMarkerList);
            EventMarker *eMarker = (EventMarker*)malloc(sizeof (EventMarker));
            eMarker->markerName = strdup("sys_info_bootup");
            eMarker->alias = NULL;
            eMarker->compName = strdup("sysint");
            eMarker->skipFreq = 0;
            eMarker->regexParam = NULL;
            eMarker->trimParam = false;
            eMarker->paramType = NULL;
            eMarker->mType = (MarkerType)MTYPE_XCONF_COUNTER;
            Vector_PushBack((*profile)->eMarkerList, eMarker);

            EventMarker *eMarker1 = (EventMarker*)malloc(sizeof (EventMarker));
            eMarker1->markerName = strdup("sys_info_marker");
            eMarker1->alias = NULL;
            eMarker1->compName = strdup("sysint");
            eMarker1->u.markerValue = strdup("marker_value");
            eMarker1->skipFreq = 0;
            eMarker1->regexParam = NULL;
            eMarker1->trimParam = false;
            eMarker1->paramType = NULL;
            eMarker1->mType = (MarkerType)MTYPE_XCONF_ABSOLUTE;
            Vector_PushBack((*profile)->eMarkerList, eMarker1);

            EventMarker *eMarker2 = (EventMarker*)malloc(sizeof (EventMarker));
            eMarker2->markerName = strdup("USED_MEM_absolute_split");
            Vector_Create(&eMarker2->u.accumulatedValues);
            Vector_PushBack(eMarker2->u.accumulatedValues, strdup("1.34MB"));
            eMarker2->compName = strdup("sysint");
            eMarker2->skipFreq = 0;
            eMarker2->alias = NULL;
            eMarker2->paramType = NULL;
            eMarker2->mType = (MarkerType)MTYPE_XCONF_ACCUMULATE;
            eMarker2->regexParam = NULL;
            eMarker2->trimParam = false;
            Vector_PushBack((*profile)->eMarkerList, eMarker2);

            Vector_Create(&(*profile)->gMarkerList);
            GrepMarker *gMarker = (GrepMarker *)malloc(sizeof(GrepMarker));
            memset(gMarker, 0, sizeof(GrepMarker));
            gMarker->markerName = strdup("USED_MEM_split:");
            gMarker->searchString = strdup("Used Memory value is");
            gMarker->logFile = strdup("Consolelog.txt.0");
            gMarker->firstSeekFromEOF = 0;// memset will already set to 0 just a safeguard
            gMarker->regexParam = NULL;
            gMarker->mType = MTYPE_ABSOLUTE;
            gMarker->u.markerValue = NULL;
            gMarker->skipFreq = 0;
            gMarker->paramType = NULL;
            Vector_PushBack((*profile)->gMarkerList, gMarker);
            
            Vector_Create(&(*profile)->topMarkerList);
            GrepMarker *topMarker = (GrepMarker *)malloc(sizeof(GrepMarker));
            memset(topMarker, 0, sizeof(GrepMarker));
            topMarker->markerName = strdup("cpu_telemetry2_0");
            topMarker->searchString = strdup("telemetry2_0");
            topMarker->logFile = strdup("top_log.txt.0");
            topMarker->firstSeekFromEOF = 0;// memset will already set to 0 just a safeguard
            topMarker->regexParam = NULL;
            topMarker->mType = MTYPE_ABSOLUTE;
            topMarker->u.markerValue = NULL;
            topMarker->skipFreq = 0;
            topMarker->paramType = NULL;
            Vector_PushBack((*profile)->topMarkerList, topMarker);

            Vector_Create(&(*profile)->paramList);
            Param *param = (Param *)malloc(sizeof(Param));
            memset(param, 0, sizeof(Param));
            param->name = strdup("Uptime_split:");
            param->alias = strdup("Device.DeviceInfo.UpTime");
            param->regexParam = NULL;
            param->skipFreq = 0;
            param->trimParam = false;
            param->paramType = NULL;
            Vector_PushBack((*profile)->paramList, param);

            (*profile)->cachedReportList = nullptr;
            Vector_Create(&(*profile)->cachedReportList);
            Vector_PushBack((*profile)->cachedReportList, strdup("{\"searchResult\":["
    "{\"T2\":\"1.0\"},"
    "{\"Profile\":\"RDKV\"},"
    "{\"Time\":\"2025-09-30 06:18:53\"},"
    "{\"sys_info_bootup\":\"1953264994\"},"
    "{\"sys_info_marker\":\"info_marker_value\"},"
    "{\"USED_MEM_absolute_split\":[\"1.34MB\",\"134.56\"]}"
"]}"));
/*
            Vector_PushBack((*profile)->cachedReportList, strdup("{\"searchResult\":["
    "{\"T2\":\"1.0\"},"
    "{\"Profile\":\"RDKV\"},"
    "{\"Time\":\"2025-09-30 06:18:53\"},"
    "{\"sys_info_marker\":\"info_marker_value\"},"
    "{\"USED_MEM_absolute_split\":[\"1.4MB\",\"134.56\"]}"
"]}"));
            Vector_PushBack((*profile)->cachedReportList, strdup("{\"searchResult\":["
    "{\"T2\":\"1.0\"},"
    "{\"Profile\":\"RDKV\"},"
    "{\"Time\":\"2025-09-30 06:18:53\"},"
    "{\"sys_info_bootup\":\"1953264994\"},"
    "{\"sys_info_marker\":\"info_marker_value\"},"
    "{\"USED_MEM_absolute_split\":[\"1.3MB\",\"13.56\"]}"
"]}"));
            Vector_PushBack((*profile)->cachedReportList, strdup("{\"searchResult\":["
    "{\"T2\":\"1.0\"},"
    "{\"Profile\":\"RDKV\"},"
    "{\"Time\":\"2025-09-30 06:18:53\"},"
    "{\"sys_info_bootup\":\"1953264994\"},"
    "{\"USED_MEM_absolute_split\":[\"1.34MB\",\"134.56\"]}"
"]}"));
            Vector_PushBack((*profile)->cachedReportList, strdup("{\"searchResult\":["
    "{\"T2\":\"1.0\"},"
    "{\"Profile\":\"RDKV\"},"
    "{\"Time\":\"2025-09-30 06:18:53\"},"
    "{\"USED_MEM_absolute_split\":[\"1.34MB\",\"134.56\"]}"
"]}"));
*/

            (*profile)->protocol = strdup("HTTP");
            (*profile)->encodingType = strdup("JSON");

            (*profile)->t2HTTPDest = (T2HTTP *)malloc(sizeof(T2HTTP));
            (*profile)->t2HTTPDest->URL = strdup("https://mock1xconf:50051/dataLakeMockXconf");
            (*profile)->isUpdated = true;

            GrepSeekProfile *gsProfile = (GrepSeekProfile *)malloc(sizeof(GrepSeekProfile));
            if (gsProfile)
            {
                 gsProfile->logFileSeekMap = hash_map_create();
                 gsProfile->execCounter = 0;
            }
            (*profile)->grepSeekProfile = gsProfile;
            (*profile)->reportInProgress = false;
            (*profile)->isUpdated = false;
            return T2ERROR_SUCCESS;
        });
    
    EXPECT_CALL(*g_fileIOMock, fopen(_, _))                                                                
        .Times(1)                                                                                         
        .WillOnce(Return(nullptr));
    /*
    EXPECT_CALL(*g_fileIOMock, getline(_,_,_))
        .Times(6)
        .WillOnce([](char **lineptr, size_t *n, FILE *stream) {
            const char* line1 = "{\"searchResult\":["
    "{\"T2\":\"1.0\"},"
    "{\"Profile\":\"RDKV\"},"
    "{\"Time\":\"2025-09-30 06:18:53\"},"
    "{\"sys_info_bootup\":\"1953264994\"},"
    "{\"sys_info_marker\":\"info_marker_value\"},"
    "{\"USED_MEM_absolute_split\":[\"1.34MB\",\"134.56\"]}"
"]}";
            size_t len1 = strlen(line1);
            if (*lineptr == nullptr) {
                *lineptr = (char*)malloc(len1 + 1);
                *n = len1 + 1;
            }
            strcpy(*lineptr, line1);
            return len1;
        })
        .WillOnce([](char **lineptr, size_t *n, FILE *stream) {
            const char* line2 = "{\"searchResult\":["
    "{\"T2\":\"1.0\"},"
    "{\"Profile\":\"RDKV\"},"
    "{\"Time\":\"2025-09-30 06:18:53\"},"
    "{\"sys_info_bootup\":\"1953264994\"},"
    "{\"sys_info_marker\":\"info_marker_value\"},"
    "{\"USED_MEM_absolute_split\":[\"1.34MB\",\"134.56\"]}"
"]}";
            size_t len2 = strlen(line2);
            if (*lineptr == nullptr) {
                *lineptr = (char*)malloc(len2 + 1);
                *n = len2 + 1;
            }
            strcpy(*lineptr, line2);
            return len2;
        })
        .WillOnce([](char **lineptr, size_t *n, FILE *stream) {
            const char* line3 = "{\"searchResult\":["
    "{\"T2\":\"1.0\"},"
    "{\"Profile\":\"RDKV\"},"
    "{\"Time\":\"2025-09-30 06:18:53\"},"
    "{\"sys_info_bootup\":\"1953264994\"},"
    "{\"sys_info_marker\":\"info_marker_value\"},"
    "{\"USED_MEM_absolute_split\":[\"1.34MB\",\"134.56\"]}"
"]}";
            size_t len3 = strlen(line3);
            if (*lineptr == nullptr) {
                *lineptr = (char*)malloc(len3 + 1);
                *n = len3 + 1;
            }
            strcpy(*lineptr, line3);
            return len3;
        })
        .WillOnce([](char **lineptr, size_t *n, FILE *stream) {
            const char* line4 = "{\"searchResult\":["
    "{\"T2\":\"1.0\"},"
    "{\"Profile\":\"RDKV\"},"
    "{\"Time\":\"2025-09-30 06:18:53\"},"
    "{\"sys_info_bootup\":\"1953264994\"},"
    "{\"sys_info_marker\":\"info_marker_value\"},"
    "{\"USED_MEM_absolute_split\":[\"1.34MB\",\"134.56\"]}"
"]}";
            size_t len4 = strlen(line4);
            if (*lineptr == nullptr) {
                *lineptr = (char*)malloc(len4 + 1);
                *n = len4 + 1;
            }
            strcpy(*lineptr, line4);
            return len4;
        })
        .WillOnce([](char **lineptr, size_t *n, FILE *stream) {
            const char* line5 = "{\"searchResult\":["
    "{\"T2\":\"1.0\"},"
    "{\"Profile\":\"RDKV\"},"
    "{\"Time\":\"2025-09-30 06:18:53\"},"
    "{\"sys_info_bootup\":\"1953264994\"},"
    "{\"sys_info_marker\":\"info_marker_value\"},"
    "{\"USED_MEM_absolute_split\":[\"1.34MB\",\"134.56\"]}"
"]}";
            size_t len5 = strlen(line5);
            if (*lineptr == nullptr) {
                *lineptr = (char*)malloc(len5 + 1);
                *n = len5 + 1;
            }
            strcpy(*lineptr, line5);
            return len5;
        })
        .WillOnce(Return(-1));
    EXPECT_CALL(*g_fileIOMock, fclose(_))
        .Times(1)
        .WillOnce(Return(0));
    EXPECT_CALL(*g_systemMock, remove(_))
        .Times(1)
        .WillOnce(Return(0));
    */
    EXPECT_CALL(*g_schedulerMock, registerProfileWithScheduler(_, _, _, _, _, _, _, _))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(T2ERROR_SUCCESS));
    EXPECT_EQ(ProfileXConf_init(false), T2ERROR_SUCCESS);
    
}

//Test the marker component map update after init of single profiles, it should update the component map
TEST_F(profileXconfTestFixture, ProfileXConf_updateMarkerComponentMap_success_after_init)
{
    ProfileXConf_updateMarkerComponentMap();
}

//Test for not more than one single profiles is supported in T2

TEST_F(profileXconfTestFixture, ProfileXConf_set_for_second_profile)
{
    ProfileXConf *localProfile = (ProfileXConf*)malloc(sizeof(ProfileXConf));
    memset(localProfile, 0, sizeof(ProfileXConf));
    localProfile->name = strdup("RDK_Profile_2");
    EventMarker *eMarker = (EventMarker*)malloc(sizeof (EventMarker));
    eMarker->markerName = strdup("sys_info_bootup");
    eMarker->compName = strdup("sysint");
    eMarker->skipFreq = 0;
    eMarker->mType = (MarkerType)MTYPE_XCONF_COUNTER;

    Vector_PushBack(localProfile->eMarkerList, eMarker);
    localProfile->gMarkerList = nullptr;
    localProfile->topMarkerList = nullptr;
    localProfile->paramList = nullptr;
    localProfile->cachedReportList = nullptr;
    localProfile->protocol = strdup("HTTP");
    localProfile->encodingType = strdup("JSON");
    localProfile->t2HTTPDest = nullptr;
    localProfile->grepSeekProfile = nullptr;
    localProfile->reportInProgress = false;
    localProfile->isUpdated = false;

    EXPECT_EQ(ProfileXConf_set(localProfile), T2ERROR_FAILURE);   
}

//Test of  ProfileXConf_isNameEqual function which compares the given profile name with existing profile name when the xconf profile is updated or changed

TEST_F(profileXconfTestFixture, profilexconf_name_equal_check)
{
    EXPECT_EQ(ProfileXConf_isNameEqual("RDK_Profile"), true);
    EXPECT_EQ(ProfileXConf_isNameEqual("Xione_Profile"), false); 
}


TEST_F(profileXconfTestFixture, ProfileXConf_isSet_true)
{
    EXPECT_EQ(true, ProfileXConf_isSet());
}

//Test to get the name of the profile in the device if single profile is set
TEST_F(profileXconfTestFixture, ProfileXconf_getName)
{
    EXPECT_STREQ(ProfileXconf_getName(), "RDK_Profile");
}

//Test the termination  of report generation - if the profile report generation is not in progress the terminate shouldn't happen

TEST_F(profileXconfTestFixture, ProfileXConf_terminateReport)
{
    EXPECT_EQ(ProfileXConf_terminateReport(), T2ERROR_FAILURE);
}

//Test storing of marker events when the profile is set
//Count marker event
TEST_F(profileXconfTestFixture, ReportProfiles_storeMarkerEvent_success)
{
    T2Event *eventInfo = (T2Event*)malloc(sizeof(T2Event));
    eventInfo->name = strdup("sys_info_bootup");
    eventInfo->value = strdup("134.56");
    EXPECT_EQ(ProfileXConf_storeMarkerEvent(eventInfo), T2ERROR_SUCCESS);
    free(eventInfo->name);
    free(eventInfo->value);
    free(eventInfo);
}

//Absolute marker event
TEST_F(profileXconfTestFixture, ReportProfiles_storeMarkerEvent_success_1)
{
    T2Event *eventInfo = (T2Event*)malloc(sizeof(T2Event));
    eventInfo->name = strdup("sys_info_marker");
    eventInfo->value = strdup("info_marker_value");
    EXPECT_EQ(ProfileXConf_storeMarkerEvent(eventInfo), T2ERROR_SUCCESS);
    free(eventInfo->name);
    free(eventInfo->value);
    free(eventInfo);
}

//Accumulate marker event
TEST_F(profileXconfTestFixture, ReportProfiles_storeMarkerEvent_success_2)
{
    T2Event *eventInfo = (T2Event*)malloc(sizeof(T2Event));
    eventInfo->name = strdup("USED_MEM_absolute_split");
    eventInfo->value = strdup("134.56");
    EXPECT_EQ(ProfileXConf_storeMarkerEvent(eventInfo), T2ERROR_SUCCESS);
    free(eventInfo->name);
    free(eventInfo->value);
    free(eventInfo);
}

//ProfileXConf_notifyTimeout Test the timeout of the profile
/*
TEST_F(profileXconfTestFixture, ProfileXConf_notifyTimeout)
{
    EXPECT_CALL(*g_profileXConfMock, getProfileParameterValues(_, _))
        .Times(1)
        .WillOnce([](Vector *paramList, int count) {    
            Vector *outparamlist = nullptr;
            //Vector_Create(&outparamlist);
            return outparamlist;
        });
   
    EXPECT_CALL(*g_profileXConfMock, encodeParamResultInJSON(_, _))
        .Times(1)
        .WillOnce(Return(T2ERROR_SUCCESS));
   
    EXPECT_CALL(*g_profileXConfMock, processTopPattern(_,_,_,_))
        .Times(1)
        .WillOnce(Return(0));

    EXPECT_CALL(*g_profileXConfMock,  getGrepResults(_,_,_,_,_))
        .Times(1)
        .WillOnce([](GrepSeekProfile **GSP, Vector *markerList, Vector **grepResultList, bool isClearSeekMap, bool check_rotated, char *customLogPath) {
            return T2ERROR_SUCCESS;
        });
    
    EXPECT_CALL(*g_profileXConfMock, encodeGrepResultInJSON(_, _))
        .Times(1)
        .WillOnce(Return(T2ERROR_SUCCESS));
    
    
    EXPECT_CALL(*g_profileXConfMock, sendReportOverHTTP(_, _, _))
        .Times(1)
        .WillOnce(Return(T2ERROR_SUCCESS));
    EXPECT_CALL(*g_profileXConfMock, sendCachedReportsOverHTTP(_,_))
        .Times(1)
        .WillOnce(Return(T2ERROR_SUCCESS));
    EXPECT_CALL(*g_systemMock, unlink(_))
        .Times(1)
        .WillOnce(Return(0));
    EXPECT_CALL(*g_schedulerMock, getLapsedTime(_,_,_))
        .Times(1)
        .WillOnce(Return(30));
    EXPECT_CALL(*g_profileXConfMock, publishReportUploadStatus(_))
        .Times(1);
    ProfileXConf_notifyTimeout(false, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(20000));
}
*/
//Test the deletion of xconf profile
TEST_F(profileXconfTestFixture, ProfileXConf_deleteProfile)
{
    ProfileXConf *localProfile = (ProfileXConf*)malloc(sizeof(ProfileXConf));
    memset(localProfile, 0, sizeof(ProfileXConf));
    localProfile->name = strdup("RDK_Profile_2");
    EventMarker *eMarker = (EventMarker*)malloc(sizeof (EventMarker));
    eMarker->markerName = strdup("sys_info_bootup");
    eMarker->compName = strdup("sysint");
    eMarker->skipFreq = 0;
    eMarker->mType = (MarkerType)MTYPE_XCONF_COUNTER;

    Vector_PushBack(localProfile->eMarkerList, eMarker);
    localProfile->gMarkerList = nullptr;
    localProfile->topMarkerList = nullptr;
    localProfile->paramList = nullptr;
    localProfile->cachedReportList = nullptr;
    localProfile->protocol = strdup("HTTP");
    localProfile->encodingType = strdup("JSON");
    localProfile->t2HTTPDest = nullptr;
    localProfile->grepSeekProfile = nullptr;
    localProfile->reportInProgress = false;
    localProfile->isUpdated = false;

    EXPECT_CALL(*g_schedulerMock, unregisterProfileFromScheduler(_))
        .Times(1)
        .WillOnce(Return(T2ERROR_SUCCESS));
    EXPECT_EQ(ProfileXConf_delete(localProfile), T2ERROR_SUCCESS);
}

ProfileXConf* CreateProfile(const char* name, bool withCached, bool withEMarker, MarkerType mType, bool withGMarker, bool withReportInProgress) {
    ProfileXConf* localProfile = (ProfileXConf*)calloc(1, sizeof(ProfileXConf));
    localProfile->name = strdup(name);

    if (withEMarker) {
        EventMarker *eMarker = (EventMarker*)calloc(1, sizeof(EventMarker));
        eMarker->markerName = strdup("sys_info_bootup");
        eMarker->compName = strdup("sysint");
        eMarker->skipFreq = 0;
        eMarker->mType = mType;
        Vector_Create(&localProfile->eMarkerList);
        Vector_PushBack(localProfile->eMarkerList, eMarker);
    }

    if (withGMarker) {
        Vector_Create(&localProfile->gMarkerList);
        void* dummy = strdup("dummy");
        Vector_PushBack(localProfile->gMarkerList, dummy);
    }

    if (withCached) {
        Vector_Create(&localProfile->cachedReportList);
        void* dummy = strdup("cached");
        Vector_PushBack(localProfile->cachedReportList, dummy);
    }

    localProfile->reportInProgress = withReportInProgress;
    localProfile->protocol = strdup("HTTP");
    localProfile->encodingType = strdup("JSON");

    return localProfile;
}
TEST_F(profileXconfTestFixture, DeleteProfile_NotInitialized) {
    ProfileXConf* prof = CreateProfile("RDK_Profile_2", false, false, MTYPE_XCONF_COUNTER, false, false);
    EXPECT_EQ(ProfileXConf_delete(prof), T2ERROR_FAILURE);
    free(prof->name); free(prof->protocol); free(prof->encodingType); free(prof);
}

// Test: Profile not found path (singleProfile==nullptr)
TEST_F(profileXconfTestFixture, DeleteProfile_ProfileNotFound) {
    ProfileXConf* prof = CreateProfile("RDK_Profile_XYZ", false, false, MTYPE_XCONF_COUNTER, false, false);
    pthread_mutex_init(&plMutex, nullptr);
    EXPECT_EQ(ProfileXConf_delete(prof), T2ERROR_FAILURE);
    pthread_mutex_destroy(&plMutex);
    free(prof->name); free(prof->protocol); free(prof->encodingType); free(prof);
}
//Test the uninit of XConf profiles
TEST_F(profileXconfTestFixture, ProfileXConf_uninit)
{
    EXPECT_EQ(ProfileXConf_uninit(), T2ERROR_SUCCESS);
}

#if 1
extern "C" {
typedef char* (*getTimeStampFuncType)(void);
getTimeStampFuncType getTimeStampFuncCallback(void);

typedef T2ERROR (*initJSONReportXconfFuncType)(cJSON**, cJSON**);
initJSONReportXconfFuncType initJSONReportXconfCallback(void);

typedef void* (*CollectAndReportXconfFuncType)(void*);
CollectAndReportXconfFuncType CollectAndReportXconfCallback(void);
}
TEST(ProfileXconfStatic, GetTimeStampAllocatesProperString)
{
    getTimeStampFuncType cb = getTimeStampFuncCallback();
    ASSERT_NE(cb, nullptr);
    char *result = cb();
    ASSERT_NE(result, nullptr);
    // Optionally check format here with regex or string parsing
    free(result);
}

TEST_F(profileXconfTestFixture, InitJSONReportXConf_CreateObjectFails) {
    // Arrange: Set expectation for cJSON_CreateObject to simulate failure
    EXPECT_CALL(*g_profileXConfMock, cJSON_CreateObject())
        .WillOnce(::testing::Return(nullptr));

    cJSON* jsonObj = reinterpret_cast<cJSON*>(0xDEADBEEF);
    cJSON* valArray = reinterpret_cast<cJSON*>(0xDEADC0DE);

    // Act: Call the static function via function pointer
    auto initJSONReportXconfFP = initJSONReportXconfCallback();
    T2ERROR ret = initJSONReportXconfFP(&jsonObj, &valArray);

    // Assert: Should return failure and output pointer should be set to nullptr
    EXPECT_EQ(ret, T2ERROR_FAILURE);
    EXPECT_EQ(jsonObj, nullptr);
}

TEST_F(profileXconfTestFixture, InitJSONReportXConf_Success) {
    // Dummy objects for mocking
    cJSON dummyObj1, dummyArr, dummyItem1, dummyItem2, dummyItem3;

    // cJSON mocks
    EXPECT_CALL(*g_profileXConfMock, cJSON_CreateObject())
        .WillOnce(::testing::Return(&dummyObj1))    // *jsonObj
        .WillOnce(::testing::Return(&dummyItem1))   // arrayItem 1 (T2)
        .WillOnce(::testing::Return(&dummyItem2))   // arrayItem 2 (Profile)
        .WillOnce(::testing::Return(&dummyItem3));  // arrayItem 3 (Time)

    EXPECT_CALL(*g_profileXConfMock, cJSON_CreateArray())
        .WillOnce(::testing::Return(&dummyArr));

    EXPECT_CALL(*g_profileXConfMock, cJSON_AddItemToObject(&dummyObj1, ::testing::StrEq("searchResult"), &dummyArr))
        .WillOnce(::testing::Return(1)); // cJSON_True

    // First array item (T2 entry)
    EXPECT_CALL(*g_profileXConfMock, cJSON_AddStringToObject(&dummyItem1, ::testing::StrEq("T2"), ::testing::StrEq("1.0")))
        .WillOnce(::testing::Return(&dummyItem1));
    EXPECT_CALL(*g_profileXConfMock, cJSON_AddItemToArray(&dummyArr, &dummyItem1))
        .WillOnce(::testing::Return(1));
    // Second array item (Profile)
    EXPECT_CALL(*g_profileXConfMock, cJSON_AddStringToObject(&dummyItem2, ::testing::StrEq("Profile"), ::testing::_))
        .WillOnce(::testing::Return(&dummyItem2));
    EXPECT_CALL(*g_profileXConfMock, cJSON_AddItemToArray(&dummyArr, &dummyItem2))
        .WillOnce(::testing::Return(1));
    // Third array item (Time)
    EXPECT_CALL(*g_profileXConfMock, cJSON_AddStringToObject(&dummyItem3, ::testing::StrEq("Time"), ::testing::_))
        .WillOnce(::testing::Return(&dummyItem3));
    EXPECT_CALL(*g_profileXConfMock, cJSON_AddItemToArray(&dummyArr, &dummyItem3))
        .WillOnce(::testing::Return(1));

    // Now run the function
    cJSON* jsonObj = nullptr;
    cJSON* valArray = nullptr;
    auto func = initJSONReportXconfCallback();
    T2ERROR ret = func(&jsonObj, &valArray);

    // Check result and output pointers
    EXPECT_EQ(ret, T2ERROR_SUCCESS);
    EXPECT_EQ(jsonObj, &dummyObj1);
    EXPECT_EQ(valArray, &dummyArr);
}

TEST_F(profileXconfTestFixture, Cover_CollectAndReportXconf)
{
    CollectAndReportXconfFuncType fn = CollectAndReportXconfCallback();
    ASSERT_NE(fn, nullptr);
    void* result = fn(nullptr);
    // Optionally, check result
}

#endif

