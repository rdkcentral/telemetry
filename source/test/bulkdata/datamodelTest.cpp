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
#include "telemetry_busmessage_sender.h"
#include "utils/t2collection.h"
#include "datamodel.h"
#include "persistence.h"
#include "t2collection.h"
#include "reportprofiles.h"
#include "t2log_wrapper.h"
}
#include "../mocks/rbusMock.h"
#include "../mocks/SystemMock.h"
#include "../mocks/FileioMock.h"
#include "../mocks/rdklogMock.h"
#include "datamodelMock.h"

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
datamodelMock *g_datamodelMock = nullptr;

// Test fixture for telemetry_busmessage_sender
class datamodelTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        
	g_rbusMock = new rbusMock();
     g_systemMock = new SystemMock();
     g_fileIOMock = new FileMock();
     g_datamodelMock = new datamodelMock();


    }
    void TearDown() override {
       
	delete  g_rbusMock;
    delete  g_systemMock;
    delete  g_fileIOMock;
    delete  g_datamodelMock;

    g_rbusMock = nullptr;
	g_systemMock = nullptr;
    g_fileIOMock = nullptr;
    g_datamodelMock = nullptr;
    }
};

//Testing the scenario of setting the reportprofiles before datamodel_init

TEST_F(datamodelTestFixture, datamodel_processProfile_before_init)
{
    const char* testJson =
    "{\"profiles\":[{\"name\":\"FIRST_REP_INTERVAL\",\"hash\":\"Hash7\",\"value\":{"
    "\"Name\":\"FIRST_REP_INTL\",\"Description\":\"FirstReportingInterval\",\"Version\":\"0.1\","
    "\"Protocol\":\"HTTP\",\"EncodingType\":\"JSON\",\"ReportingInterval\":30,\"ActivationTimeout\":3000,"
    "\"RootName\":\"Profile_Datamodel_event_epoch\",\"TimeReference\":\"0001-01-01T00:00:00Z\","
    "\"Parameter\":[{\"type\":\"dataModel\",\"name\":\"UPTIME\",\"reference\":\"Device.DeviceInfo.UpTime\","
    "\"use\":\"absolute\",\"method\":\"subscribe\",\"reportTimestamp\":\"Unix-Epoch\"},"
    "{\"type\":\"event\",\"eventName\":\"USED_MEM_absolute_split\",\"component\":\"sysint\",\"use\":\"absolute\",\"reportTimestamp\":\"Unix-Epoch\"},"
    "{\"type\":\"event\",\"eventName\":\"USED_MEM_count\",\"component\":\"sysint\",\"use\":\"absolute\",\"reportTimestamp\":\"Unix-Epoch\"},"
    "{\"type\":\"event\",\"eventName\":\"USED_MEM_accum_split\",\"component\":\"sysint\",\"use\":\"accumulate\",\"reportTimestamp\":\"Unix-Epoch\"},"
    "{\"type\":\"grep\",\"content\":\"cujo-agent\",\"logFile\":\"top_log.txt\",\"use\":\"absolute\"}],"
    "\"ReportingAdjustments\":{\"ReportOnUpdate\":true,\"FirstReportingInterval\":15,\"MaxUploadLatency\":20000},"
    "\"HTTP\":{\"URL\":\"https://stbrtl.r53.xcal.tv/\",\"Compression\":\"None\",\"Method\":\"POST\","
    "\"RequestURIParameter\":[{\"Name\":\"reportName\",\"Reference\":\"Profile.Name\"}]},"
    "\"JSONEncoding\":{\"ReportFormat\":\"NameValuePair\",\"ReportTimestamp\":\"None\"}}}]}";
    
    EXPECT_EQ(T2ERROR_SUCCESS, datamodel_processProfile((char*)testJson, T2_RP));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

//Testing the scenario of setting the msgpack reportprofiles before datamodel_init

TEST_F(datamodelTestFixture, datamodel_MsgpackProcessProfile_before_init)
{
    char *str = strdup("gahwcm9maWxlc5GDpG5hbWWyRklSU1RfUkVQX0lOVEVSVkFMpGhhc2ilSGFzaDeldmFsdWWNpE5hbWWuRklSU1RfUkVQX0lOVEyrRGVzY3JpcHRpb262Rmlyc3RSZXBvcnRpbmdJbnRlcnZhbKdWZXJzaW9uozAuMahQcm90b2NvbKRIVFRQrEVuY29kaW5nVHlwZaRKU09OsVJlcG9ydGluZ0ludGVydmFsHrFBY3RpdmF0aW9uVGltZW91dM0LuKhSb290TmFtZb1Qcm9maWxlX0RhdGFtb2RlbF9ldmVudF9lcG9jaK1UaW1lUmVmZXJlbmNltDAwMDEtMDEtMDFUMDA6MDA6MDBaqVBhcmFtZXRlcpWGpHR5cGWpZGF0YU1vZGVspG5hbWWmVVBUSU1FqXJlZmVyZW5jZbhEZXZpY2UuRGV2aWNlSW5mby5VcFRpbWWjdXNlqGFic29sdXRlpm1ldGhvZKlzdWJzY3JpYmWvcmVwb3J0VGltZXN0YW1wqlVuaXgtRXBvY2iFpHR5cGWlZXZlbnSpZXZlbnROYW1lt1VTRURfTUVNX2Fic29sdXRlX3NwbGl0qWNvbXBvbmVudKZzeXNpbnSjdXNlqGFic29sdXRlr3JlcG9ydFRpbWVzdGFtcKpVbml4LUVwb2NohaR0eXBlpWV2ZW50qWV2ZW50TmFtZa5VU0VEX01FTV9jb3VudKljb21wb25lbnSmc3lzaW50o3VzZahhYnNvbHV0Za9yZXBvcnRUaW1lc3RhbXCqVW5peC1FcG9jaIWkdHlwZaVldmVudKlldmVudE5hbWW0VVNFRF9NRU1fYWNjdW1fc3BsaXSpY29tcG9uZW50pnN5c2ludKN1c2WqYWNjdW11bGF0Za9yZXBvcnRUaW1lc3RhbXCqVW5peC1FcG9jaISkdHlwZaRncmVwp2NvbnRlbnSqY3Vqby1hZ2VudKdsb2dGaWxlq3RvcF9sb2cudHh0o3VzZahhYnNvbHV0ZbRSZXBvcnRpbmdBZGp1c3RtZW50c4OuUmVwb3J0T25VcGRhdGXDtkZpcnN0UmVwb3J0aW5nSW50ZXJ2YWwPsE1heFVwbG9hZExhdGVuY3nNTiCkSFRUUISjVVJMu2h0dHBzOi8vc3RicnRsLnI1My54Y2FsLnR2L6tDb21wcmVzc2lvbqROb25lpk1ldGhvZKRQT1NUs1JlcXVlc3RVUklQYXJhbWV0ZXKRgqROYW1lqnJlcG9ydE5hbWWpUmVmZXJlbmNlrFByb2ZpbGUuTmFtZaxKU09ORW5jb2RpbmeCrFJlcG9ydEZvcm1hdK1OYW1lVmFsdWVQYWlyr1JlcG9ydFRpbWVzdGFtcKROb25l");
    int str_size = 1376; //assigning the size of the above string
    
    EXPECT_EQ(T2ERROR_SUCCESS,  datamodel_MsgpackProcessProfile(str, str_size));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
}

//Initialisation of datamodel threads

TEST_F(datamodelTestFixture, datamodel_init)
{
    EXPECT_EQ(T2ERROR_SUCCESS, datamodel_init());
}

//datamodel_processProfile success test
//Testing the Processing of reportprofiles json blob

TEST_F(datamodelTestFixture, datamodel_processProfile_RP)
{
    const char* testJson =
    "{\"profiles\":[{\"name\":\"FIRST_REP_INTERVAL\",\"hash\":\"Hash7\",\"value\":{"
    "\"Name\":\"FIRST_REP_INTL\",\"Description\":\"FirstReportingInterval\",\"Version\":\"0.1\","
    "\"Protocol\":\"HTTP\",\"EncodingType\":\"JSON\",\"ReportingInterval\":30,\"ActivationTimeout\":3000,"
    "\"RootName\":\"Profile_Datamodel_event_epoch\",\"TimeReference\":\"0001-01-01T00:00:00Z\","
    "\"Parameter\":[{\"type\":\"dataModel\",\"name\":\"UPTIME\",\"reference\":\"Device.DeviceInfo.UpTime\","
    "\"use\":\"absolute\",\"method\":\"subscribe\",\"reportTimestamp\":\"Unix-Epoch\"},"
    "{\"type\":\"event\",\"eventName\":\"USED_MEM_absolute_split\",\"component\":\"sysint\",\"use\":\"absolute\",\"reportTimestamp\":\"Unix-Epoch\"},"
    "{\"type\":\"event\",\"eventName\":\"USED_MEM_count\",\"component\":\"sysint\",\"use\":\"absolute\",\"reportTimestamp\":\"Unix-Epoch\"},"
    "{\"type\":\"event\",\"eventName\":\"USED_MEM_accum_split\",\"component\":\"sysint\",\"use\":\"accumulate\",\"reportTimestamp\":\"Unix-Epoch\"},"
    "{\"type\":\"grep\",\"content\":\"cujo-agent\",\"logFile\":\"top_log.txt\",\"use\":\"absolute\"}],"
    "\"ReportingAdjustments\":{\"ReportOnUpdate\":true,\"FirstReportingInterval\":15,\"MaxUploadLatency\":20000},"
    "\"HTTP\":{\"URL\":\"https://stbrtl.r53.xcal.tv/\",\"Compression\":\"None\",\"Method\":\"POST\","
    "\"RequestURIParameter\":[{\"Name\":\"reportName\",\"Reference\":\"Profile.Name\"}]},"
    "\"JSONEncoding\":{\"ReportFormat\":\"NameValuePair\",\"ReportTimestamp\":\"None\"}}}]}";
    
    EXPECT_CALL(*g_datamodelMock, ReportProfiles_ProcessReportProfilesBlob(_, _))
        .Times(1);
    EXPECT_EQ(T2ERROR_SUCCESS, datamodel_processProfile((char*)testJson, T2_RP));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

//datamodel_processProfile success test
//Testing the Processing of temp reportprofiles json blob
TEST_F(datamodelTestFixture, datamodel_processProfile_TRP)
{
    const char* testJson =
    "{\"profiles\":[{\"name\":\"FIRST_REP_INTERVAL\",\"hash\":\"Hash7\",\"value\":{"
    "\"Name\":\"FIRST_REP_INTL\",\"Description\":\"FirstReportingInterval\",\"Version\":\"0.1\","
    "\"Protocol\":\"HTTP\",\"EncodingType\":\"JSON\",\"ReportingInterval\":30,\"ActivationTimeout\":3000,"
    "\"RootName\":\"Profile_Datamodel_event_epoch\",\"TimeReference\":\"0001-01-01T00:00:00Z\","
    "\"Parameter\":[{\"type\":\"dataModel\",\"name\":\"UPTIME\",\"reference\":\"Device.DeviceInfo.UpTime\","
    "\"use\":\"absolute\",\"method\":\"subscribe\",\"reportTimestamp\":\"Unix-Epoch\"},"
    "{\"type\":\"event\",\"eventName\":\"USED_MEM_absolute_split\",\"component\":\"sysint\",\"use\":\"absolute\",\"reportTimestamp\":\"Unix-Epoch\"},"
    "{\"type\":\"event\",\"eventName\":\"USED_MEM_count\",\"component\":\"sysint\",\"use\":\"absolute\",\"reportTimestamp\":\"Unix-Epoch\"},"
    "{\"type\":\"event\",\"eventName\":\"USED_MEM_accum_split\",\"component\":\"sysint\",\"use\":\"accumulate\",\"reportTimestamp\":\"Unix-Epoch\"},"
    "{\"type\":\"grep\",\"content\":\"cujo-agent\",\"logFile\":\"top_log.txt\",\"use\":\"absolute\"}],"
    "\"ReportingAdjustments\":{\"ReportOnUpdate\":true,\"FirstReportingInterval\":15,\"MaxUploadLatency\":20000},"
    "\"HTTP\":{\"URL\":\"https://stbrtl.r53.xcal.tv/\",\"Compression\":\"None\",\"Method\":\"POST\","
    "\"RequestURIParameter\":[{\"Name\":\"reportName\",\"Reference\":\"Profile.Name\"}]},"
    "\"JSONEncoding\":{\"ReportFormat\":\"NameValuePair\",\"ReportTimestamp\":\"None\"}}}]}";
    
    EXPECT_CALL(*g_datamodelMock, ReportProfiles_ProcessReportProfilesBlob(_, _))
        .Times(1);
    EXPECT_EQ(T2ERROR_SUCCESS, datamodel_processProfile((char*)testJson, T2_TEMP_RP));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

//datamodel_processProfile failure test

//Passing invalid reportprofiles boolean options Processing of datamodel is skipped
TEST_F(datamodelTestFixture, datamodel_processProfile_invalid)
{
    const char* testJson =
    "{\"profiles\":[{\"name\":\"FIRST_REP_INTERVAL\",\"hash\":\"Hash7\",\"value\":{"
    "\"Name\":\"FIRST_REP_INTL\",\"Description\":\"FirstReportingInterval\",\"Version\":\"0.1\","
    "\"Protocol\":\"HTTP\",\"EncodingType\":\"JSON\",\"ReportingInterval\":30,\"ActivationTimeout\":3000,"
    "\"RootName\":\"Profile_Datamodel_event_epoch\",\"TimeReference\":\"0001-01-01T00:00:00Z\","
    "\"Parameter\":[{\"type\":\"dataModel\",\"name\":\"UPTIME\",\"reference\":\"Device.DeviceInfo.UpTime\","
    "\"use\":\"absolute\",\"method\":\"subscribe\",\"reportTimestamp\":\"Unix-Epoch\"},"
    "{\"type\":\"event\",\"eventName\":\"USED_MEM_absolute_split\",\"component\":\"sysint\",\"use\":\"absolute\",\"reportTimestamp\":\"Unix-Epoch\"},"
    "{\"type\":\"event\",\"eventName\":\"USED_MEM_count\",\"component\":\"sysint\",\"use\":\"absolute\",\"reportTimestamp\":\"Unix-Epoch\"},"
    "{\"type\":\"event\",\"eventName\":\"USED_MEM_accum_split\",\"component\":\"sysint\",\"use\":\"accumulate\",\"reportTimestamp\":\"Unix-Epoch\"},"
    "{\"type\":\"grep\",\"content\":\"cujo-agent\",\"logFile\":\"top_log.txt\",\"use\":\"absolute\"}],"
    "\"ReportingAdjustments\":{\"ReportOnUpdate\":true,\"FirstReportingInterval\":15,\"MaxUploadLatency\":20000},"
    "\"HTTP\":{\"URL\":\"https://stbrtl.r53.xcal.tv/\",\"Compression\":\"None\",\"Method\":\"POST\","
    "\"RequestURIParameter\":[{\"Name\":\"reportName\",\"Reference\":\"Profile.Name\"}]},"
    "\"JSONEncoding\":{\"ReportFormat\":\"NameValuePair\",\"ReportTimestamp\":\"None\"}}}]}";
    
    EXPECT_EQ(T2ERROR_SUCCESS, datamodel_processProfile((char*)testJson, 3));
     std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

//Processing of NULL reportprofiles json blob should fail
TEST_F(datamodelTestFixture, datamodel_processProfile_null)
{
    EXPECT_EQ(T2ERROR_FAILURE, datamodel_processProfile(NULL, T2_RP));
}

//Processing of empty reportprofiles json blob should fail
TEST_F(datamodelTestFixture, datamodel_processProfile_empty)
{
    const char* testJson = "{}";
    
    EXPECT_EQ(T2ERROR_FAILURE, datamodel_processProfile((char*)testJson, T2_RP));
}

//Processing of invalid reportprofiles json blob without profiles key should fail
TEST_F(datamodelTestFixture, datamodel_processProfile_no_profiles)
{
    const char* testJson = "{\"profilez\":[{\"name\":\"FIRST_REP_INTERVAL\",\"hash\":\"Hash7\",\"value\":{"
    "\"Name\":\"FIRST_REP_INTL\",\"Description\":\"FirstReportingInterval\",\"Version\":\"0.1\","
    "\"Protocol\":\"HTTP\",\"EncodingType\":\"JSON\",\"ReportingInterval\":30,\"ActivationTimeout\":3000,"
    "\"RootName\":\"Profile_Datamodel_event_epoch\",\"TimeReference\":\"0001-01-01T00:00:00Z\","
    "\"Parameter\":[{\"type\":\"dataModel\",\"name\":\"UPTIME\",\"reference\":\"Device.DeviceInfo.UpTime\","
    "\"use\":\"absolute\",\"method\":\"subscribe\",\"reportTimestamp\":\"Unix-Epoch\"},"
    "{\"type\":\"event\",\"eventName\":\"USED_MEM_absolute_split\",\"component\":\"sysint\",\"use\":\"absolute\",\"reportTimestamp\":\"Unix-Epoch\"},"
    "{\"type\":\"event\",\"eventName\":\"USED_MEM_count\",\"component\":\"sysint\",\"use\":\"absolute\",\"reportTimestamp\":\"Unix-Epoch\"},"
    "{\"type\":\"event\",\"eventName\":\"USED_MEM_accum_split\",\"component\":\"sysint\",\"use\":\"accumulate\",\"reportTimestamp\":\"Unix-Epoch\"},"
    "{\"type\":\"grep\",\"content\":\"cujo-agent\",\"logFile\":\"top_log.txt\",\"use\":\"absolute\"}],"
    "\"ReportingAdjustments\":{\"ReportOnUpdate\":true,\"FirstReportingInterval\":15,\"MaxUploadLatency\":20000},"
    "\"HTTP\":{\"URL\":\"https://stbrtl.r53.xcal.tv/\",\"Compression\":\"None\",\"Method\":\"POST\","
    "\"RequestURIParameter\":[{\"Name\":\"reportName\",\"Reference\":\"Profile.Name\"}]},"
    "\"JSONEncoding\":{\"ReportFormat\":\"NameValuePair\",\"ReportTimestamp\":\"None\"}}}]}";
    
    EXPECT_EQ(T2ERROR_FAILURE, datamodel_processProfile((char*)testJson, T2_RP));
}

//datamodel_getSavedJsonProfilesasString success test
//Testing the retrieval of saved reportprofiles json blob as string

TEST_F(datamodelTestFixture, datamodel_getSavedJsonProfilesasString)
{
    char* profiles = NULL;

    EXPECT_CALL(*g_fileIOMock, opendir(_))
        .Times(1)
        .WillOnce(Return((DIR*)NULL));
    EXPECT_CALL(*g_fileIOMock, mkdir(_,_))
        .Times(1)
        .WillOnce(Return(0));
    /*EXPECT_CALL(*g_fileIOMock, readdir(_))
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
            buf = NULL;
            return len; // Return number of bytes read
        });
    EXPECT_CALL(*g_fileIOMock, close(_))
        .Times(1);
    EXPECT_CALL(*g_fileIOMock, closedir(_))
        .Times(1);*/
    datamodel_getSavedJsonProfilesasString(&profiles);
    EXPECT_EQ(profiles, nullptr);
}

//datamodel_MsgPack processing success case
//Processing the msgpack formatted profile data

TEST_F(datamodelTestFixture, datamodel_MsgpackProcessProfile_success)
{
    char *str = "gahwcm9maWxlc5GDpG5hbWWyRklSU1RfUkVQX0lOVEVSVkFMpGhhc2ilSGFzaDeldmFsdWWNpE5hbWWuRklSU1RfUkVQX0lOVEyrRGVzY3JpcHRpb262Rmlyc3RSZXBvcnRpbmdJbnRlcnZhbKdWZXJzaW9uozAuMahQcm90b2NvbKRIVFRQrEVuY29kaW5nVHlwZaRKU09OsVJlcG9ydGluZ0ludGVydmFsHrFBY3RpdmF0aW9uVGltZW91dM0LuKhSb290TmFtZb1Qcm9maWxlX0RhdGFtb2RlbF9ldmVudF9lcG9jaK1UaW1lUmVmZXJlbmNltDAwMDEtMDEtMDFUMDA6MDA6MDBaqVBhcmFtZXRlcpWGpHR5cGWpZGF0YU1vZGVspG5hbWWmVVBUSU1FqXJlZmVyZW5jZbhEZXZpY2UuRGV2aWNlSW5mby5VcFRpbWWjdXNlqGFic29sdXRlpm1ldGhvZKlzdWJzY3JpYmWvcmVwb3J0VGltZXN0YW1wqlVuaXgtRXBvY2iFpHR5cGWlZXZlbnSpZXZlbnROYW1lt1VTRURfTUVNX2Fic29sdXRlX3NwbGl0qWNvbXBvbmVudKZzeXNpbnSjdXNlqGFic29sdXRlr3JlcG9ydFRpbWVzdGFtcKpVbml4LUVwb2NohaR0eXBlpWV2ZW50qWV2ZW50TmFtZa5VU0VEX01FTV9jb3VudKljb21wb25lbnSmc3lzaW50o3VzZahhYnNvbHV0Za9yZXBvcnRUaW1lc3RhbXCqVW5peC1FcG9jaIWkdHlwZaVldmVudKlldmVudE5hbWW0VVNFRF9NRU1fYWNjdW1fc3BsaXSpY29tcG9uZW50pnN5c2ludKN1c2WqYWNjdW11bGF0Za9yZXBvcnRUaW1lc3RhbXCqVW5peC1FcG9jaISkdHlwZaRncmVwp2NvbnRlbnSqY3Vqby1hZ2VudKdsb2dGaWxlq3RvcF9sb2cudHh0o3VzZahhYnNvbHV0ZbRSZXBvcnRpbmdBZGp1c3RtZW50c4OuUmVwb3J0T25VcGRhdGXDtkZpcnN0UmVwb3J0aW5nSW50ZXJ2YWwPsE1heFVwbG9hZExhdGVuY3nNTiCkSFRUUISjVVJMu2h0dHBzOi8vc3RicnRsLnI1My54Y2FsLnR2L6tDb21wcmVzc2lvbqROb25lpk1ldGhvZKRQT1NUs1JlcXVlc3RVUklQYXJhbWV0ZXKRgqROYW1lqnJlcG9ydE5hbWWpUmVmZXJlbmNlrFByb2ZpbGUuTmFtZaxKU09ORW5jb2RpbmeCrFJlcG9ydEZvcm1hdK1OYW1lVmFsdWVQYWlyr1JlcG9ydFRpbWVzdGFtcKROb25l";
    int str_size = 1376; //assigning the size of the above string
    EXPECT_CALL(*g_datamodelMock, ReportProfiles_ProcessReportProfilesMsgPackBlob(_, _))
        .Times(1);

    EXPECT_EQ(T2ERROR_SUCCESS,  datamodel_MsgpackProcessProfile(str, str_size));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
}

//datamodel_getSavedMsgpackProfilesasString success test
//Testing the retrieval of saved msgpack formatted reportprofiles as string

TEST_F(datamodelTestFixture, datamodel_getSavedMsgpackProfilesasString_sucess)
{
    char* savedProfiles = NULL;

    EXPECT_CALL(*g_fileIOMock, fopen(_, _))
        .Times(1)
        .WillOnce(Return((FILE*)0xdeadbeef));
    EXPECT_CALL(*g_fileIOMock, fseek(_, _, _))
        .Times(2)
        .WillOnce(Return(0))
        .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, ftell(_))
        .Times(1)
        .WillOnce(Return(1500)); // Example file size 1500 bytes
    EXPECT_CALL(*g_fileIOMock, fread(_, _, _, _))
        .Times(1)
        .WillOnce([](void* ptr, size_t size, size_t nmemb, FILE* stream) {
            const char* testMsgpack = "gahwcm9maWxlc5GDpG5hbWWyRklSU1RfUkVQX0lOVEVSVkFMpGhhc2ilSGFzaDeldmFsdWWNpE5hbWWuRklSU1RfUkVQX0lOVEyrRGVzY3JpcHRpb262Rmlyc3RSZXBvcnRpbmdJbnRlcnZhbKdWZXJzaW9uozAuMahQcm90b2NvbKRIVFRQrEVuY29kaW5nVHlwZaRKU09OsVJlcG9ydGluZ0ludGVydmFsHrFBY3RpdmF0aW9uVGltZW91dM0LuKhSb290TmFtZb1Qcm9maWxlX0RhdGFtb2RlbF9ldmVudF9lcG9jaK1UaW1lUmVmZXJlbmNltDAwMDEtMDEtMDFUMDA6MDA6MDBaqVBhcmFtZXRlcpWGpHR5cGWpZGF0YU1vZGVspG5hbWWmVVBUSU1FqXJlZmVyZW5jZbhEZXZpY2UuRGV2aWNlSW5mby5VcFRpbWWjdXNlqGFic29sdXRlpm1ldGhvZKlzdWJzY3JpYmWvcmVwb3J0VGltZXN0YW1wqlVuaXgtRXBvY2iFpHR5cGWlZXZlbnSpZXZlbnROYW1lt1VTRURfTUVNX2Fic29sdXRlX3NwbGl0qWNvbXBvbmVudKZzeXNpbnSjdXNlqGFic29sdXRlr3JlcG9ydFRpbWVzdGFtcKpVbml4LUVwb2NohaR0eXBlpWV2ZW50qWV2ZW50NmFtZa5VU0VEX01FTV9jb3";
            memcpy(ptr, testMsgpack, strlen(testMsgpack)); // Copy testMsgpack into ptr
            return strlen(testMsgpack); // Return number of bytes read
        });
    EXPECT_CALL(*g_fileIOMock, fclose(_))
        .Times(1);
    EXPECT_EQ(0, datamodel_getSavedMsgpackProfilesasString(&savedProfiles));
    if (savedProfiles) {
        free(savedProfiles);
    }
}

//datamodel_getSavedMsgpackProfilesasString failure test
//Testing the retrieval of saved msgpack formatted reportprofiles as string when file open fails

TEST_F(datamodelTestFixture, datamodel_getSavedMsgpackProfilesasString_failure)
{
    char* savedProfiles = NULL;

    EXPECT_CALL(*g_fileIOMock, fopen(_, _))
        .Times(1)
        .WillOnce(Return(nullptr)); // Simulate file open failure

    EXPECT_EQ(0, datamodel_getSavedMsgpackProfilesasString(&savedProfiles));
    EXPECT_EQ(savedProfiles, nullptr); // savedProfiles should remain NULL
}

//Testing the retrieval of saved msgpack formatted reportprofiles as string when file size is zero
TEST_F(datamodelTestFixture, datamodel_getSavedMsgpackProfilesasString_zero_size)
{
    char* savedProfiles = NULL;

    EXPECT_CALL(*g_fileIOMock, fopen(_, _))
        .Times(1)
        .WillOnce(Return((FILE*)0xdeadbeef));
    EXPECT_CALL(*g_fileIOMock, fseek(_, _, _))
        .Times(1)
        .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, ftell(_))
        .Times(1)
        .WillOnce(Return(-1)); // Simulate zero file size
    EXPECT_CALL(*g_fileIOMock, fclose(_))
        .Times(1);

    EXPECT_EQ(0, datamodel_getSavedMsgpackProfilesasString(&savedProfiles));
    EXPECT_EQ(savedProfiles, nullptr); // savedProfiles should remain NULL
}

//Testing the retrieval of saved msgpack formatted reportprofiles as string when file read fails
TEST_F(datamodelTestFixture, datamodel_getSavedMsgpackProfilesasString_read_failure)
{
    char* savedProfiles = NULL;

    EXPECT_CALL(*g_fileIOMock, fopen(_, _))
        .Times(1)
        .WillOnce(Return((FILE*)0xdeadbeef));
    EXPECT_CALL(*g_fileIOMock, fseek(_, _, _))
        .Times(2)
        .WillOnce(Return(0))
        .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, ftell(_))
        .Times(1)
        .WillOnce(Return(1500)); // Example file size 1500 bytes
    EXPECT_CALL(*g_fileIOMock, fread(_, _, _, _))
        .Times(1)
        .WillOnce(Return(0)); // Simulate read failure
    EXPECT_CALL(*g_fileIOMock, fclose(_))
        .Times(1);

    EXPECT_EQ(0, datamodel_getSavedMsgpackProfilesasString(&savedProfiles));
    EXPECT_EQ(savedProfiles, nullptr); // savedProfiles should remain NULL
}
