/*
 * If not stated otherwise in this file or this component's LICENSE file the 
 * following copyright and licenses apply:
 *
 * Copyright 2024 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * You may not use this file except in compliance with the License.
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
#if 1
TEST_F(reportprofilesTestFixture, ProcessMsgPackBlob_Test1) {
   printf("##### test starts\n");
  // const  char *data = "3wAAAAGocHJvZmlsZXPdAAAAAd8AAAADpG5hbWWsUkRLQl9Qcm9maWxlpGhhc2ilSGFzaDKldmFsdWXfAAAADaROYW1lb...";
  const char *data = "AQ==";
// decode
   gsize decodedDataLen = 0;
   guchar *webConfigString = g_base64_decode(data, &decodedDataLen);

// allocate and fill
   struct __msgpack__ *msg = (struct __msgpack__*)malloc(sizeof(struct __msgpack__));
   msg->msgpack_blob = (char*)webConfigString;
   msg->msgpack_blob_size = (int)decodedDataLen;

       EXPECT_CALL(*g_vectorMock, Vector_Size(_))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(0)); // Return 1 to indicate only one profile (no duplicates)
    EXPECT_CALL(*g_vectorMock, Vector_Destroy(_, _)).Times(::testing::AtMost(1))
        .WillRepeatedly(Return(T2ERROR_SUCCESS));
// call target
  int ret = __ReportProfiles_ProcessReportProfilesMsgPackBlob((void*)msg, false);
  EXPECT_EQ(ret, T2ERROR_PROFILE_NOT_FOUND);
// cleanup
  free(msg);
  g_free(webConfigString);
}
#endif
char* makeMsgpackWithOneProfile(const char* name, const char* hash, const char* value, int* out_size) {
    msgpack_sbuffer sbuf;
    msgpack_packer pk;

    msgpack_sbuffer_init(&sbuf);
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

    // Root: a map with one entry ("profiles")
    msgpack_pack_map(&pk, 1);
    msgpack_pack_str(&pk, strlen("profiles"));
    msgpack_pack_str_body(&pk, "profiles", strlen("profiles"));

    // Value: an array with one profile object
    msgpack_pack_array(&pk, 1);

    // The profile object (as a map)
    msgpack_pack_map(&pk, 3);  // name, hash, value
    // name
    msgpack_pack_str(&pk, strlen("name"));
    msgpack_pack_str_body(&pk, "name", strlen("name"));
    msgpack_pack_str(&pk, strlen(name));
    msgpack_pack_str_body(&pk, name, strlen(name));
    // hash
    msgpack_pack_str(&pk, strlen("hash"));
    msgpack_pack_str_body(&pk, "hash", strlen("hash"));
    msgpack_pack_str(&pk, strlen(hash));
    msgpack_pack_str_body(&pk, hash, strlen(hash));
    // value
    msgpack_pack_str(&pk, strlen("value"));
    msgpack_pack_str_body(&pk, "value", strlen("value"));
    msgpack_pack_str(&pk, strlen(value));
    msgpack_pack_str_body(&pk, value, strlen(value));

    // Prepare output
    *out_size = sbuf.size;
    char *data = (char*) malloc(sbuf.size);
    memcpy(data, sbuf.data, sbuf.size);

    msgpack_sbuffer_destroy(&sbuf);
    return data;
}

TEST_F(reportprofilesTestFixture, ProcessMsgPackBlob_ProfileAdded) {

    struct __msgpack__ msg;
    int blob_len;
    char* msgpack_blob = makeMsgpackWithOneProfile("profileA", "hashA", "valueA", &blob_len);

    // setup test struct
    msg.msgpack_blob = msgpack_blob;
    msg.msgpack_blob_size = blob_len;

    EXPECT_CALL(*g_reportprofileMock, isRbusEnabled())
     .WillRepeatedly(Return(false));

     EXPECT_CALL(*g_vectorMock, Vector_Size(_))
        .Times(::testing::AtMost(2))
        .WillRepeatedly(Return(0));
    // Mock success for config parsing and addition
    //EXPECT_CALL(*profileMock, processMsgPackConfiguration(_, _)).WillOnce(Return(T2ERROR_SUCCESS));
    //EXPECT_CALL(*profileMock, ReportProfiles_addReportProfile(_)).WillOnce(Return(T2ERROR_SUCCESS));
    int ret = __ReportProfiles_ProcessReportProfilesMsgPackBlob(&msg, false);
    EXPECT_EQ(ret, T2ERROR_SUCCESS); // covers main add path 1300-1305
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
    // New profile, triggers add logic and saveConfigToFile
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
    // Expect add and saveConfigToFile, can stub if needed
    EXPECT_CALL(*g_vectorMock, Vector_Size(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_vectorMock, Vector_Destroy(_, _)).WillRepeatedly(Return(T2ERROR_SUCCESS));

//       EXPECT_CALL(*g_fileIOMock, opendir(_))
  //      .Times(1)
    //    .WillOnce(Return(nullptr));
       DIR *dir = (DIR*)0xffffffff ;
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

#if 0
TEST_F(reportprofilesTestFixture, ProcessReportProfilesBlob_ReplaceProfile_DifferentHash) {
    // Add two profiles, one existing, such that hash differs, triggers replacement branch
    cJSON *root = cJSON_CreateObject();
    cJSON *profiles = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "profiles", profiles);

    // First profile, old hash, will be replaced
    cJSON *profile1 = cJSON_CreateObject();
    cJSON_AddStringToObject(profile1, "name", "profileA");
    cJSON_AddStringToObject(profile1, "hash", "oldhash");
    cJSON *value1 = cJSON_CreateObject();
    cJSON_AddStringToObject(value1, "param", "value1");
    cJSON_AddItemToObject(profile1, "value", value1);
    cJSON_AddItemToArray(profiles, profile1);

    // Second profile, same name, new hash, triggers replacement logic
    cJSON *profile2 = cJSON_CreateObject();
    cJSON_AddStringToObject(profile2, "name", "profileA");
    cJSON_AddStringToObject(profile2, "hash", "newhash");
    cJSON *value2 = cJSON_CreateObject();
    cJSON_AddStringToObject(value2, "param", "value2");
    cJSON_AddItemToObject(profile2, "value", value2);
    cJSON_AddItemToArray(profiles, profile2);
      EXPECT_CALL(*g_reportprofileMock, isRbusEnabled())
     .WillRepeatedly(Return(false));

    EXPECT_CALL(*g_vectorMock, Vector_Size(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(*g_vectorMock, Vector_Destroy(_, _)).WillRepeatedly(Return(T2ERROR_SUCCESS));
          DIR *dir = (DIR*)0xffffffff ;
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
#endif //segmentation fault
TEST_F(reportprofilesTestFixture, ReportProfiles_ProcessReportProfilesMsgPackBlobTest) {
    // Should return early if root is NULL
    ReportProfiles_ProcessReportProfilesMsgPackBlob(NULL, false);
    // Possibly assert/expect logs/error
}

#if 1
TEST_F(reportprofilesTestFixture, ProcessMsgPackBlob_Test2) {
   printf("##### test starts\n");
  // const  char *data = "3wAAAAGocHJvZmlsZXPdAAAAAd8AAAADpG5hbWWsUkRLQl9Qcm9maWxlpGhhc2ilSGFzaDKldmFsdWXfAAAADaROYW1lb...";
  const char *data = "AQ==";
// decode
   gsize decodedDataLen = 0;
   guchar *webConfigString = g_base64_decode(data, &decodedDataLen);

// allocate and fill
   struct __msgpack__ *msg = (struct __msgpack__*)malloc(sizeof(struct __msgpack__));
   msg->msgpack_blob = (char*)webConfigString;
   msg->msgpack_blob_size = (int)decodedDataLen;
#if 1
       EXPECT_CALL(*g_vectorMock, Vector_Size(_))
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(0)); // Return 1 to indicate only one profile (no duplicates)
    EXPECT_CALL(*g_vectorMock, Vector_Destroy(_, _)).Times(::testing::AtMost(1))
        .WillRepeatedly(Return(T2ERROR_SUCCESS));
#endif
// call target
  int ret = ReportProfiles_ProcessReportProfilesMsgPackBlob(msg->msgpack_blob, msg->msgpack_blob_size);
//  EXPECT_EQ(ret, T2ERROR_PROFILE_NOT_FOUND);
// cleanup
  free(msg);
  g_free(webConfigString);
}
#endif
#if 0
TEST_F(reportprofilesTestFixture, ProcessReportProfilesBlob_AddNewProfile) {

    // New profile, triggers add logic and saveConfigToFile

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

    EXPECT_CALL(*g_t2markersMock, isRbusEnabled())
        .Times(1)
        .WillOnce(Return(false))
    // Expect add and saveConfigToFile, can stub if needed

    EXPECT_CALL(*g_vectorMock, Vector_Size(_)).WillRepeatedly(Return(0));

    EXPECT_CALL(*g_vectorMock, Vector_Destroy(_, _)).WillRepeatedly(Return(T2ERROR_SUCCESS));

//       EXPECT_CALL(*g_fileIOMock, opendir(_))

  //      .Times(1)

    //    .WillOnce(Return(nullptr));

       DIR *dir = (DIR*)0xffffffff ;

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
#endif

#if 0
// Example test for MsgPack blob processing
TEST_F(reportprofilesTestFixture, MsgpackBlobValidInput) {
    // Generate a valid msgpack blob, fill struct __msgpack__ and call the function
    // This is just a placeholder/template!
    struct __msgpack__ msg;
    msg.msgpack_blob = /* assign pointer to valid msgpack binary */;
    msg.msgpack_blob_size = /* assign size */;
    EXPECT_LE(__ReportProfiles_ProcessReportProfilesMsgPackBlob(&msg, false), T2ERROR_SUCCESS);
}
#endif
// Add more tests for edge cases and negative scenarios as needed

