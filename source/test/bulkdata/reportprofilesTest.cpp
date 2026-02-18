/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2026 RDK Management
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

extern "C" {
#include "reportprofiles.h"
#include "profile.h"
#include "t2eventreceiver.h"
#include "t2collection.h"
#include "t2log_wrapper.h"
#include "msgpack.h"
#include <glib.h>
#include <glib/gi18n.h>
}

#include "test/mocks/SystemMock.h"
#include "test/mocks/FileioMock.h"
#include "test/mocks/rdklogMock.h"
#include "test/mocks/rbusMock.h"
#include "test/mocks/rdkconfigMock.h"
#include "test/mocks/VectorMock.h"
#include "test/bulkdata/SchedulerMock.h"
#include "reportprofileMock.h"

using namespace std;
using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::Invoke;
// Create global mocks if you need them just like t2markersTest.cpp
FileMock *g_fileIOMock = NULL;
SystemMock * g_systemMock = NULL;
rdklogMock *m_rdklogMock = NULL;
rbusMock *g_rbusMock = NULL;
rdkconfigMock *g_rdkconfigMock = nullptr;
extern VectorMock *g_vectorMock;
extern SchedulerMock *g_schedulerMock;
reportprofileMock* g_reportprofileMock = nullptr;
// Test fixture for reportprofiles
class reportprofilesTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
	g_reportprofileMock = new reportprofileMock();    
        g_systemMock = new SystemMock();
        g_fileIOMock = new FileMock();
        m_rdklogMock = new rdklogMock();
        g_rbusMock = new rbusMock();
        g_vectorMock = new VectorMock();
    }
    void TearDown() override {
	delete g_reportprofileMock;    
        delete g_systemMock;
        delete g_fileIOMock;
        delete m_rdklogMock;
        delete g_rbusMock;
        delete g_vectorMock;
	g_reportprofileMock = nullptr;
        g_systemMock = nullptr;
        g_fileIOMock = nullptr;
        m_rdklogMock = nullptr;
        g_rbusMock = nullptr;
        g_vectorMock = nullptr;
    }
};


TEST_F(reportprofilesTestFixture, ProcessMsgPackBlob_InvalidFormat) {
    struct __msgpack__ msg;
    msg.msgpack_blob = nullptr;
    msg.msgpack_blob_size = 0;
    int ret = __ReportProfiles_ProcessReportProfilesMsgPackBlob(&msg, false);
    EXPECT_EQ(ret, T2ERROR_INVALID_ARGS);
}

TEST_F(reportprofilesTestFixture, ProcessMsgPackBlob_Test1) {
   const char *data = "AQ==";
   gsize decodedDataLen = 0;
   guchar *webConfigString = g_base64_decode(data, &decodedDataLen);

   struct __msgpack__ *msg = (struct __msgpack__*)malloc(sizeof(struct __msgpack__));
   msg->msgpack_blob = (char*)webConfigString;
   msg->msgpack_blob_size = (int)decodedDataLen;

   EXPECT_CALL(*g_vectorMock, Vector_Size(_))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(0)); 
   EXPECT_CALL(*g_vectorMock, Vector_Destroy(_, _)).Times(::testing::AtMost(1))
        .WillRepeatedly(Return(T2ERROR_SUCCESS));

   ReportProfiles_ProcessReportProfilesMsgPackBlob(msg->msgpack_blob, msg->msgpack_blob_size);
}

TEST_F(reportprofilesTestFixture, ProcessReportProfilesBlob_EmptyProfile_T2_TEMP_RP) {
    cJSON *root = cJSON_CreateObject();
    cJSON *profiles = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "profiles", profiles);
    ReportProfiles_ProcessReportProfilesBlob(root, T2_TEMP_RP);
    cJSON_Delete(root);
}

TEST_F(reportprofilesTestFixture, ProcessReportProfilesBlob_EmptyProfile_Normal) {
    cJSON *root = cJSON_CreateObject();
    cJSON *profiles = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "profiles", profiles);
    EXPECT_CALL(*g_vectorMock, Vector_Size(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_vectorMock, Vector_Destroy(_, _)).WillRepeatedly(Return(T2ERROR_SUCCESS));
    ReportProfiles_ProcessReportProfilesBlob(root, T2_RP); // normal, triggers deleteAllReportProfiles
    cJSON_Delete(root);
}

TEST_F(reportprofilesTestFixture, ProcessReportProfilesBlob_AddNewProfile) {
    cJSON *root = cJSON_CreateObject();
    cJSON *profiles = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "profiles", profiles);
    cJSON *profile = cJSON_CreateObject();
    cJSON_AddStringToObject(profile, "name", "newprofile");
    cJSON_AddStringToObject(profile, "hash", "newhash");
    cJSON *value = cJSON_CreateObject();
    cJSON_AddStringToObject(value, "param", "value");
    cJSON_AddItemToObject(profile, "value", value);
    cJSON_AddItemToArray(profiles, profile);

    EXPECT_CALL(*g_reportprofileMock, isRbusEnabled())
     .WillRepeatedly(Return(false));
    EXPECT_CALL(*g_vectorMock, Vector_Size(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_vectorMock, Vector_Destroy(_, _)).WillRepeatedly(Return(T2ERROR_SUCCESS));

    DIR *dir = reinterpret_cast<DIR*>(1);

    EXPECT_CALL(*g_fileIOMock, opendir(_))
           .Times(::testing::AtMost(1))
           .WillRepeatedly(Return(dir));
    EXPECT_CALL(*g_fileIOMock, readdir(_))
           .Times(::testing::AtMost(1))
           .WillRepeatedly(Return((struct dirent *)NULL));
    EXPECT_CALL(*g_fileIOMock, closedir(_))
           .Times(::testing::AtMost(1))
           .WillRepeatedly(Return(0));
    ReportProfiles_ProcessReportProfilesBlob(root, T2_RP);
    cJSON_Delete(root);
}
TEST_F(reportprofilesTestFixture, ProcessReportProfilesMsgPackBlob_NullBlob) {
    ReportProfiles_ProcessReportProfilesMsgPackBlob(NULL,0);
}

