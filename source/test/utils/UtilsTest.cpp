/* Copyright 2020 Comcast Cable Communications Management, LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

extern "C" {
#include <utils/vector.h>
#include <utils/t2collection.h>
#include <utils/t2MtlsUtils.h>
#include <utils/t2log_wrapper.h>
#include <utils/persistence.h>
#include <utils/t2common.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <telemetry2_0.h>
#include <privacycontrol/rdkservices_privacyutils.h>
#include <stdarg.h>
}

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <iostream>
#include <stdexcept>
#include <sys/types.h>
#include <dirent.h>
#include <cstdio>
#include <cstring>
#define PROFILE_NAME "/opt/opt/opt/opt/opt/opt/logs/logs/logs/logs/logs/logs/opt/opt/opt/opt/opt/opt/logs/logs/logs/logs/logs/logs/opt/opt/opt/opt/opt/opt/logs/logs/logs/logs/logs/logs/opt/opt/opt/opt/opt/opt/logs/logs/logs/logs/logs/logs"
#include "test/mocks/SystemMock.h"
#include "test/mocks/FileioMock.h"
#include "test/mocks/rdklogMock.h"
#include "test/mocks/rdkconfigMock.h"
using namespace std;

using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;

//Testing t2MtlsUtils
TEST(GET_CERTS, MTLS_UTILS_NULL)
{
    int val = 1 ;
    EXPECT_EQ(T2ERROR_FAILURE, getMtlsCerts(NULL,NULL));
}


TEST(GET_CERTS, MTLS_UTILS_1ST_NULL)
{
    char* buff = NULL ;
    buff = (char*) malloc(128);
    memset(buff, '\0' , 128);
    if(buff) {
        EXPECT_EQ(T2ERROR_FAILURE, getMtlsCerts(&buff,NULL));
        free(buff);
    }
    buff = NULL;
}


TEST(GET_CERTS, MTLS_UTILS_2ND_NULL)
{
    char* buff = NULL ;
    buff = (char*) malloc(128);
    memset(buff, '\0' , 128);
    if(buff) {
        EXPECT_EQ(T2ERROR_FAILURE, getMtlsCerts(NULL,&buff));
        free(buff);
    }
    buff = NULL;
}

//Testing the vectors
//inputs for functions
Vector* config = NULL;
char *marker1  = "GTEST_TELEMETRY1";
char *marker2  = "GTEST_TELEMETRY2";


TEST(VECTOR_CREATE, VECTOR_NOT_NULL)
{
    EXPECT_EQ(T2ERROR_SUCCESS, Vector_Create(&config));
    Vector_Destroy(config, free); // Free the vector after the test
    config = NULL;
}

TEST(VECTOR_PUSHBACK, VECTOR_NULL)
{
   EXPECT_EQ(T2ERROR_INVALID_ARGS, Vector_PushBack(NULL, (void *) marker1));
}

TEST(VECTOR_PUSHBACK, VECTOR_ITEM_NULL)
{
    Vector_Create(&config);
     EXPECT_EQ(T2ERROR_INVALID_ARGS, Vector_PushBack(config, NULL));
     //Deletion of vector
     Vector_Destroy(config,free);
     config = NULL;
}

TEST(VECTOR_PUSHBACK, VECTOR_NOT_NULL)
{
     Vector_Create(&config);
     EXPECT_EQ(T2ERROR_SUCCESS, Vector_PushBack(config, (void *) strdup(marker1)));
     EXPECT_EQ(T2ERROR_SUCCESS, Vector_PushBack(config, (void *) strdup(marker2)));
     Vector_Destroy(config,free); // Free all elements and the vector
     config = NULL;
}

TEST(VECTOR_SIZE, VECTOR_NULL)
{
     EXPECT_EQ(0, Vector_Size(NULL));
}

TEST(VECTOR_SIZE, VECTOR_NOT_NULL)
{
    Vector_Create(&config);
    EXPECT_EQ(T2ERROR_SUCCESS, Vector_PushBack(config, (void *) strdup(marker1)));
    EXPECT_EQ(T2ERROR_SUCCESS, Vector_PushBack(config, (void *) strdup(marker2)));
    EXPECT_EQ(2, Vector_Size(config));
    //Deleting the vector
    Vector_Destroy(config, free);
    config = NULL;
}

TEST(VECTOR_AT,VECTOR_NULL)
{
    EXPECT_EQ(NULL, Vector_At(NULL, 1));
}

TEST(VECTOR_AT, VECTOR_OUT_OF_SIZE)
{
    Vector_Create(&config);
    Vector_PushBack(config, (void *) strdup(marker1));
    Vector_PushBack(config, (void *) strdup(marker2));
    EXPECT_EQ(NULL, Vector_At(config, 3));
    //Deleting the Vector
    Vector_Destroy(config, free);
    config = NULL;
}


TEST(VECTOR_CLEAR, VECTOR_NULL)
{
    Vector_Create(&config);
    EXPECT_EQ(T2ERROR_INVALID_ARGS, Vector_Clear(NULL, free));
    EXPECT_EQ(T2ERROR_INVALID_ARGS, Vector_Clear(config, NULL));
    //Deleting the vector
    Vector_Destroy(config, free);
    config = NULL;
}


TEST(VECTOR_CLEAR, VECTOR_NOT_NULL)
{
     Vector_Create(&config);
     Vector_PushBack(config, (void *) strdup(marker1));
     EXPECT_EQ(T2ERROR_SUCCESS, Vector_Clear(config, free));
     //Deleting the vector
    Vector_Destroy(config, free);
    config = NULL;
}

TEST(VECTOR_DESTROY, VECTOR_NULL)
{
    EXPECT_EQ(T2ERROR_INVALID_ARGS, Vector_Destroy(NULL, free));
}

TEST(VECTOR_DESTROY, VECTOR_NOT_NULL)
{
     Vector_Create(&config);
     Vector_PushBack(config, (void *) strdup(marker1));
     EXPECT_EQ(T2ERROR_SUCCESS, Vector_Destroy(config, free));
     config = NULL;
}

TEST(VECTOR_AT, VECTOR_NULL_AND_NOT_NULL)
{
     Vector_Create(&config);
     Vector_PushBack(config, (void *) strdup(marker1));
     Vector_PushBack(config, (void *) strdup(marker2));
     EXPECT_EQ(NULL, Vector_At(NULL, 1));
     EXPECT_EQ(NULL, Vector_At(config, 3));
     Vector_Destroy(config, free);
     config = NULL;
}

TEST(VECTOR_REMOVE_ITEM, VECTOR_FIRST_NULL)
{
   char *item = "GTEST_TELEMETRY1";
   EXPECT_EQ(T2ERROR_INVALID_ARGS, Vector_RemoveItem(NULL, (void *)item, free));
}

TEST(VECTOR_REMOVE_ITEM, VECTOR_SECOND_NULL)
{
     Vector_Create(&config);
     Vector_PushBack(config, (void *) strdup(marker1));
     EXPECT_EQ(T2ERROR_INVALID_ARGS, Vector_RemoveItem(config, NULL, free));
     //Deleting the vector
     Vector_Destroy(config, free);
     config = NULL;
}

TEST(VECTOR_REMOVE_ITEM, REMOVE_ITEM1)
{
     Vector_Create(&config);
     Vector_PushBack(config, (void *) strdup(marker1));
     Vector_PushBack(config, (void *) strdup(marker2));
     EXPECT_EQ(T2ERROR_SUCCESS, Vector_RemoveItem(config, marker1, free));// Free the removed item
     //Deleting the vector
     Vector_Destroy(config, free);
     config = NULL;
}

TEST(VECTOR_SORT, VECTOR_1_NULL)
{
    EXPECT_EQ(T2ERROR_INVALID_ARGS, Vector_Sort(NULL, sizeof(1), NULL));
}

TEST(VECTOR_SORT, VECTOR_3_NULL)
{
     Vector_Create(&config);
     Vector_PushBack(config, (void *) strdup(marker1));
     Vector_PushBack(config, (void *) strdup(marker2));
     EXPECT_EQ(T2ERROR_INVALID_ARGS, Vector_Sort(config, sizeof(config), NULL));
     //Deleting the vector
     Vector_Destroy(config, free);
     config = NULL;
}

//QUEUE TESTING
//inputs for queue

char* teststr1 = "SYS_TELEMETRY_TEST2";
char* teststr2 = "SYS_TELEMETRY_TEST3";
static queue_t *t2_test_queue = NULL;
void check_queue(queue_t* q1)
{
     if(q1 == NULL)
     {
	  throw std::overflow_error("queue is NULL");
     }
}


TEST(QUEUE_CREATE, check_queue)
{
  t2_test_queue = t2_queue_create();
  EXPECT_NO_THROW(check_queue(t2_test_queue));
}

TEST(QUEUE_PUSH, QUEUE_1_NULL)
{
    EXPECT_EQ(-1, t2_queue_push(NULL, (void *) teststr1));
}

TEST(QUEUE_PUSH, QUEUE_2_NULL)
{
     EXPECT_EQ(-1, t2_queue_push(t2_test_queue, NULL));
}

TEST(QUEUE_PUSH, QUEUE_PUSH_SUCCESS)
{
     EXPECT_EQ(0, t2_queue_push(t2_test_queue, (void *) strdup(teststr1)));
     EXPECT_EQ(0, t2_queue_push(t2_test_queue, (void *) strdup(teststr2)));
}

TEST(QUEUE_COUNT, QUEUE_NULL)
{
     EXPECT_EQ(0, t2_queue_count(NULL));
}

TEST(QUEUE_COUNT, QUEUE_NOT_NULL)
{
     EXPECT_EQ(2, t2_queue_count(t2_test_queue));
}

TEST(QUEUE_PEEK, QUEUE_NULL)
{
     EXPECT_EQ(NULL, t2_queue_peek(NULL, 1));
}

TEST(QUEUE_PEEK, QUEUE_OUT_OF_BOUNDS)
{
     EXPECT_EQ(NULL, t2_queue_peek(t2_test_queue, 3));
}

TEST(QUEUE_POP, QUEUE_NULL)
{
     EXPECT_EQ(NULL, t2_queue_pop(NULL));
}

TEST(QUEUE_POP, QUEUE_EMPTY)
{
      queue_t *t2_pop_queue = t2_queue_create();
      EXPECT_EQ(NULL,t2_queue_pop(t2_pop_queue));
      t2_queue_destroy(t2_pop_queue, free);
}

TEST(QUEUE_REMOVE, QUEUE_1_NULL)
{
    EXPECT_EQ(NULL, t2_queue_remove(NULL, 1));
}

TEST(QUEUE_REMOVE, QUEUE_OUT_OF_BOUNDS)
{
    EXPECT_EQ(NULL, t2_queue_remove(t2_test_queue, 4));
}

TEST(QUEUE_REMOVE, QUEUE_ELEMENT_NULL)
{
    queue_t *t2_remove_queue = t2_queue_create();
    EXPECT_EQ(NULL, t2_queue_remove(t2_remove_queue, 0));
    t2_queue_destroy(t2_remove_queue, free);
}

TEST(QUEUE_DELETE, QUEUE_1_NULL)
{
    queue_t* t2_destroy_queue = NULL;
    t2_queue_destroy(t2_destroy_queue, free);
    EXPECT_EQ(NULL, t2_destroy_queue);
}

TEST(QUEUE_DELETE, check_queue)
{
    queue_t* t2_destroy_queue = NULL;
    t2_destroy_queue = t2_queue_create();
    t2_queue_destroy(t2_destroy_queue, NULL);
    EXPECT_NO_THROW(check_queue(t2_destroy_queue));
    //Deleting the queue
    t2_queue_destroy(t2_destroy_queue, free);
}


//Hash map testing
//Inputs for hash map

void check_markercompmap(hash_map_t* markercomp)
{
    if( markercomp == NULL)
    {
	throw std::overflow_error("markercomp is NULL");
    }
}

char* markerh1 = "TEST_MARKER1";
char* markerh2 = "TEST_MARKER2";
char* markerh3 = "TEST_MARKER3";
static hash_map_t *markerCompMap = NULL;
char* teststring = "TEST_MARKER1";
char* teststring1 = "TEST_MARKER2";
char* teststring2 = "TEST_MARKER3";
static hash_map_t *markerCompMap1 = NULL;

TEST(HASH_MAP_TEST, check_markercompmap)
{
    markerCompMap = hash_map_create();
    EXPECT_NO_THROW(check_markercompmap(markerCompMap));
    hash_map_destroy(markerCompMap, free); // Free the hash map itself
    markerCompMap = NULL;
}

TEST(HASH_MAP_PUT, HASH_MAP_1_NULL)
{
    EXPECT_EQ(-1, hash_map_put(NULL, teststring, markerh1, NULL));
}

TEST(HASH_MAP_PUT, HASH_MAP_2_NULL)
{
    hash_map_t *markerCompMapTest = hash_map_create();
    EXPECT_EQ(-1, hash_map_put(markerCompMapTest, NULL, markerh1, NULL));
    hash_map_destroy(markerCompMapTest, free); // Free the hash map itself
    markerCompMapTest = NULL;
}

TEST(HASH_MAP_PUT, HASH_MAP_3_NULL)
{
    hash_map_t *markerCompMapTest = hash_map_create();
    EXPECT_EQ(-1, hash_map_put(markerCompMapTest, teststring, NULL, NULL));
    hash_map_destroy(markerCompMapTest, free); // Free the hash map itself
    markerCompMapTest = NULL;
}

TEST(HASH_MAP_COUNT, HASH_MAP_NULL)
{
    EXPECT_EQ(-1, hash_map_count(NULL));
}

TEST(HASH_MAP_LOOKUP, HASH_MAP_1_NULL)
{
    EXPECT_EQ(NULL, hash_map_lookup(NULL, 1));
}

TEST(HASH_MAP_LOOKUP, HASH_MAP_2_NULL)
{
   hash_map_t *markerCompMapTest = hash_map_create();
   EXPECT_EQ(NULL, hash_map_lookup(markerCompMapTest, 3));
   hash_map_destroy(markerCompMapTest, free); // Free the hash map itself
   markerCompMapTest = NULL;
}

TEST(HASH_MAP_LOOKUPKEY, HASH_MAP_1_NULL)
{
   EXPECT_EQ(NULL, hash_map_lookupKey(NULL, 1));
}

TEST(HASH_MAP_LOOKUPKEY, HASH_MAP_2_NULL)
{
    hash_map_t *markerCompMapTest = hash_map_create();
    EXPECT_EQ(NULL, hash_map_lookupKey(markerCompMapTest, 3));
    hash_map_destroy(markerCompMapTest, free); // Free the hash map itself
    markerCompMapTest = NULL;
}

TEST(HASH_MAP_GET, HASH_MAP_1_NULL)
{
    EXPECT_EQ(NULL, hash_map_get(NULL, "TEST_MARKER2"));
}

TEST(HASH_MAP_GET, HASH_MAP_2_NULL)
{
    hash_map_t *markerCompMapTest = hash_map_create();
    EXPECT_EQ(NULL, hash_map_get(markerCompMapTest, NULL));
    hash_map_destroy(markerCompMapTest, free); // Free the hash map itself
    markerCompMapTest = NULL;
}

TEST(HASH_MAP_GET, HASH_MAP_INVALID)
{
   hash_map_t *markerCompMapTest = hash_map_create();
   EXPECT_EQ(NULL, hash_map_get(markerCompMapTest, "TEST"));
   hash_map_destroy(markerCompMapTest, free); // Free the hash mapp itself
   markerCompMapTest = NULL;
}

TEST(HASH_MAP_GET_FIRST, HASH_MAP_NULL)
{
    EXPECT_EQ(NULL, hash_map_get_first(NULL));
}

TEST(HASH_MAP_GET_FIRST, HASH_MAP_VALUE_NULL)
{
    hash_map_t *markerCompMapTest = hash_map_create();
    EXPECT_EQ(NULL, hash_map_get_first(markerCompMapTest));
    hash_map_destroy(markerCompMapTest, free); // Free the hash map itself
    markerCompMapTest = NULL;
}

TEST(HASH_MAP_GET_NEXT, HASH_MAP_1_NULL)
{
   EXPECT_EQ(NULL, hash_map_get_next(NULL, (void *)teststring1));
}


TEST(HASH_MAP_GET_NEXT, HASH_MAP_2_NULL)
{
    hash_map_t *markerCompMapTest = hash_map_create();
    EXPECT_EQ(NULL, hash_map_get_next(markerCompMapTest, NULL));
    hash_map_destroy(markerCompMapTest, free); // Free the hash map itself
    markerCompMapTest = NULL;
}

TEST(HASH_MAP_GET_NEXT, HASH_ITEM_INVALID)
{
    hash_map_t *markerCompMapTest = hash_map_create();
    EXPECT_EQ(NULL, hash_map_get_next(markerCompMapTest, (void *)"TEST"));
    hash_map_destroy(markerCompMapTest, free); // Free the hash map itself
    markerCompMapTest = NULL;
}

TEST(HASH_MAP_REMOVE, HASH_MAP_NULL)
{
    EXPECT_EQ(NULL, hash_map_remove(NULL, teststring2));
}

TEST(HASH_MAP_REMOVE, HASH_ELEMENT_NULL)
{
    hash_map_t *markerCompMapTest = hash_map_create();
    EXPECT_EQ(NULL,hash_map_remove(markerCompMapTest, NULL));
    hash_map_destroy(markerCompMapTest, free); // Free the hash map itself
    markerCompMapTest = NULL;
}

TEST(HASH_MAP_REMOVE, HASH_INVALID_ELEMENT)
{
    hash_map_t *markerCompMapTemp = NULL;
    markerCompMapTemp = hash_map_create();
    EXPECT_EQ(NULL, hash_map_remove(markerCompMapTemp, "TEST"));
    hash_map_destroy(markerCompMapTemp, free); // Free the hash map itself
    markerCompMapTemp = NULL;
}

TEST(HASH_MAP_CLEAR1, check_markercompmap) {
    hash_map_t *markerCompMapTemp = NULL;
    markerCompMapTemp = hash_map_create();
    hash_map_clear(markerCompMapTemp, free); // Ensure elements are freed
    EXPECT_NO_THROW(check_markercompmap(markerCompMapTemp));
    markerCompMapTemp = NULL;
}

TEST(HASH_MAP_CLEAR2, check_markercompmap) {
    hash_map_t *markerCompMapTemp = NULL;
    markerCompMapTemp = hash_map_create();
    hash_map_clear(markerCompMapTemp, free); // Ensure elements are freed
    EXPECT_NO_THROW(check_markercompmap(markerCompMapTemp));
    // hash_map_destroy(markerCompMapTemp, free); // Free the hash map itself
    markerCompMapTemp = NULL;
}

TEST(HASH_MAP_DESTROY1, check_markercompmap) {
    hash_map_t *markerCompMapTemp = NULL;
    markerCompMapTemp = hash_map_create();
    hash_map_destroy(markerCompMapTemp, free); // Free the hash map and its elements
    markerCompMapTemp = NULL;
}

//t2common.c

TEST(GETDEVICEPROPERTYDATA, PARAM_NULL)
{
    char UseHWBasedCert[8] = {'\0'};
    EXPECT_EQ(false, getDevicePropertyData(NULL, UseHWBasedCert, sizeof(UseHWBasedCert)));
    EXPECT_EQ(false, getDevicePropertyData("UseSEBasedCert", NULL, sizeof(UseHWBasedCert)));
    EXPECT_EQ(false, getDevicePropertyData("UseSEBasedCert", UseHWBasedCert, 0));
    EXPECT_EQ(false, getDevicePropertyData("UseSEBasedCert", UseHWBasedCert, 90));
}

TEST(FETCHLOCALCONFIGS, PATH_NULL)
{
     Vector* configlist = NULL;
     Vector_Create(&configlist);
     EXPECT_EQ(T2ERROR_INVALID_ARGS, fetchLocalConfigs(NULL, configlist));
     Vector_Destroy(configlist, free);
}

TEST(FETCHLOCALCONFIGS, VECTOR_NULL)
{
     EXPECT_EQ(T2ERROR_INVALID_ARGS, fetchLocalConfigs("/nvram/path", NULL));
}

TEST(FETCHLOCALCONFIGS, ALL_NULL)
{
     EXPECT_EQ(T2ERROR_INVALID_ARGS, fetchLocalConfigs(NULL, NULL));
}

TEST(SAVECONFIGTOFILE, PATH_NULL)
{
     EXPECT_EQ(T2ERROR_INVALID_ARGS, saveConfigToFile(NULL, "Profile1", "{Config: Testing Conf}"));
}

TEST(SAVECONFIGTOFILE, PROFILENAME_NULL)
{
     EXPECT_EQ(T2ERROR_INVALID_ARGS, saveConfigToFile("/tmp/path", NULL, "{Config: Testing Conf}"));
}

TEST(SAVECONFIGTOFILE, CONFIG_NULL_ALL_NULL)
{
     EXPECT_EQ(T2ERROR_INVALID_ARGS, saveConfigToFile("/tmp/path", "Profile1", NULL));
     EXPECT_EQ(T2ERROR_INVALID_ARGS, saveConfigToFile(NULL, NULL, NULL));
}

TEST(SAVECONFIGTOFILE, PROFILENAME_SIZE)
{

     EXPECT_EQ(T2ERROR_FAILURE, saveConfigToFile("/tmp/path", PROFILE_NAME, "{Config: Testing Conf}"));

}


TEST(MSGPACKSAVECONFIG, ARGS_TEST)
{
     const char* path = "/nvram/.t2reportprofiles";
     const char* msgpack = "3wAAAAGocHJvZmlsZXPdAAAABN8AAAADpG5hbWW1UkRLQl9DQ1NQV2lmaV9Qcm9maWxlpGhhc2ilaGFzaDeldmFsdWXfAAAAC6ROYW1ltVJES0JfQ0";
     EXPECT_EQ(T2ERROR_INVALID_ARGS, MsgPackSaveConfig(NULL, NULL, msgpack, 45));
     EXPECT_EQ(T2ERROR_INVALID_ARGS, MsgPackSaveConfig(NULL, NULL, NULL, 0));

}

TEST(SAVECACREPTOPERSFOLD, ARGS_NULL)
{
     Vector* reportlist = NULL;
     Vector_Create(&reportlist);
     Vector_PushBack(reportlist, (void *)strdup("cJSON_Report: SYS_MEM_INFO:1"));
     EXPECT_EQ(T2ERROR_FAILURE, saveCachedReportToPersistenceFolder(NULL, NULL));
     EXPECT_EQ(T2ERROR_FAILURE, saveCachedReportToPersistenceFolder("PROFILE1", NULL));
     EXPECT_EQ(T2ERROR_FAILURE, saveCachedReportToPersistenceFolder(NULL, reportlist));
     Vector_Destroy(reportlist, free);
}

TEST(POPULATECACREPORT, ARGS_NULL)
{
     Vector* configlist = NULL;
     Vector_Create(&configlist);
     Vector_PushBack(configlist, (void *)strdup("marker1"));
     EXPECT_EQ(T2ERROR_FAILURE, populateCachedReportList(NULL, NULL));
     EXPECT_EQ(T2ERROR_FAILURE, populateCachedReportList("PROFILE1", NULL));
     EXPECT_EQ(T2ERROR_FAILURE, populateCachedReportList(NULL, configlist));
     Vector_Destroy(configlist, free);
}

//Privacymode module test cases
//getPrivacyMode
TEST(GETPRIVACYMODE, ARGS_NULL)
{
    char* privacyMode = NULL;
    getPrivacyMode(&privacyMode);
    EXPECT_STREQ("SHARE", privacyMode);
    free(privacyMode); // Free the allocated memory
    privacyMode = NULL;
}

//setPrivacyMode
TEST(SETPRIVACYMODE, ARGS_NULL)
{
    char* privacyMode = strdup("SHARE");
    EXPECT_EQ(T2ERROR_SUCCESS, setPrivacyMode(privacyMode));
    free(privacyMode);
}

rdkconfigMock *g_rdkconfigMock = nullptr;

class utilsTestFixture : public ::testing::Test {
protected:
    void SetUp() override
    {
        g_fileIOMock = new FileMock();
        g_systemMock = new SystemMock();
	g_rdkconfigMock = new rdkconfigMock();
    }

    void TearDown() override
    {
        delete g_fileIOMock;
        delete g_systemMock;
	delete g_rdkconfigMock;

        g_fileIOMock = nullptr;
        g_systemMock = nullptr;
	g_rdkconfigMock = nullptr;
    }
};

TEST_F(utilsTestFixture, FETCHLOCALCONFIGS_FUNC1)
{
    const char* path = "/tmp/t2reportprofiles/";
    DIR *dir = (DIR*)NULL ;
    Vector* configlist = NULL;
    Vector_Create(&configlist);
    Vector_PushBack(configlist, (void *)strdup("marker1"));
    EXPECT_CALL(*g_fileIOMock, opendir(_))
           .Times(1)
           .WillOnce(Return(dir));

    EXPECT_CALL(*g_fileIOMock, mkdir(_,_))
           .Times(1)
           .WillOnce(Return(-1));

    ASSERT_EQ(T2ERROR_FAILURE, fetchLocalConfigs(path, configlist));
    Vector_Destroy(configlist, free);
}

TEST_F(utilsTestFixture, FETCHLOCALCONFIGS_FUNC2)
{
    const char* path = "/tmp/t2reportprofiles/";
    DIR *dir = (DIR*)0xffffffff ;
    Vector* configlist = NULL;
    Vector_Create(&configlist);
    Vector_PushBack(configlist, (void *)strdup("marker1"));
    EXPECT_CALL(*g_fileIOMock, opendir(_))
           .Times(1)
           .WillOnce(Return(dir));
    EXPECT_CALL(*g_systemMock, system(_))
	   .Times(1)
           .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, closedir(_))
	   .Times(1)
	   .WillOnce(Return(0));
    
    ASSERT_EQ(T2ERROR_SUCCESS, fetchLocalConfigs(path, configlist));
    Vector_Destroy(configlist, free);
}

TEST_F(utilsTestFixture, FETCHLOCALCONFIGS_FUNC3)
{
    const char* path = "/opt/.t2persistentFolder/";
    DIR *dir = (DIR*)0xffffffff;
    Vector* configlist = NULL;
    Vector_Create(&configlist);
    Vector_PushBack(configlist, (void *)strdup("marker1"));
    Vector_PushBack(configlist, (void *)strdup("marker2"));
    struct dirent *entry = NULL;
    entry = (struct dirent *)malloc(sizeof(struct dirent));
    entry->d_type = DT_REG;
    std::strncpy(entry->d_name, "DCMresponse.txt", sizeof(entry->d_name));
    EXPECT_CALL(*g_fileIOMock, opendir(_))
           .Times(1)
           .WillOnce(Return(dir));
   /* EXPECT_CALL(*g_systemMock, system(_))
	   .Times(1)
	   .WillOnce(Return(0));*/ //v_secure_system need to be added
    EXPECT_CALL(*g_fileIOMock, readdir(_))
	   .Times(2)
	   .WillOnce(Return(entry))
	   .WillOnce(Return((struct dirent *)NULL));
    EXPECT_CALL(*g_fileIOMock, open(_,_))
	   .Times(1)
	   .WillOnce(Return(1));
    EXPECT_CALL(*g_fileIOMock, fstat(_,_))
	   .Times(1)
	   .WillOnce(Return(-1));
    EXPECT_CALL(*g_fileIOMock, close(_))                                                       
           .Times(1)
           .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, closedir(_))  
           .Times(1)
           .WillOnce(Return(0));    
    ASSERT_EQ(T2ERROR_SUCCESS, fetchLocalConfigs(path, configlist));
    Vector_Destroy(configlist, free);
}

TEST_F(utilsTestFixture, FETCHLOCALCONFIGS_FUNC4)
{
    const char* path = "/opt/.t2persistentFolder/";
    DIR *dir = (DIR*)0xffffffff;
    Vector* configlist = NULL;
    Vector_Create(&configlist);
    Vector_PushBack(configlist, (void *)strdup("marker1"));
    Vector_PushBack(configlist, (void *)strdup("marker2"));
    struct dirent *entry = NULL;
    entry = (struct dirent *)malloc(sizeof(struct dirent));
    entry->d_type = DT_REG;
    std::strncpy(entry->d_name, "DCMresponse.txt", sizeof(entry->d_name));
    EXPECT_CALL(*g_fileIOMock, opendir(_))
           .Times(1)
           .WillOnce(Return(dir));
   /* EXPECT_CALL(*g_systemMock, system(_))
           .Times(1)
           .WillOnce(Return(0));*/ //v_secure_system need to be added
    EXPECT_CALL(*g_fileIOMock, readdir(_))
           .Times(2)
           .WillOnce(Return(entry))
           .WillOnce(Return((struct dirent *)NULL));
    EXPECT_CALL(*g_fileIOMock, open(_,_))
           .Times(1)
           .WillOnce(Return(1));
    EXPECT_CALL(*g_fileIOMock, fstat(_,_))
           .Times(1)
           .WillOnce(Return(-1));
    EXPECT_CALL(*g_fileIOMock, close(_))
           .Times(1)
           .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, closedir(_))
           .Times(1)
           .WillOnce(Return(0));
    ASSERT_EQ(T2ERROR_SUCCESS, fetchLocalConfigs(path, configlist));
    Vector_Destroy(configlist, free);
}

#if defined (MTLS_FROM_ENV)
TEST(GetMtlsCertsTest, ReturnsDynamicCertsWhenEnvSet) {
    setenv("XPKI", "1", 1);
    setenv("XPKI_CERT", "dynamic_cert.pem", 1);
    setenv("XPKI_PASS", "dynamic_pass", 1);

    char* certName = nullptr;
    char* phrase = nullptr;
    EXPECT_EQ(getMtlsCerts(&certName, &phrase), T2ERROR_SUCCESS);
    EXPECT_STREQ(certName, "dynamic_cert.pem");
    EXPECT_STREQ(phrase, "dynamic_pass");

    free(certName);
    free(phrase);

    unsetenv("XPKI");
    unsetenv("XPKI_CERT");
    unsetenv("XPKI_PASS");
}

TEST(GetMtlsCertsTest, ReturnsStaticCertsWhenEnvSet) {
    unsetenv("XPKI");
    unsetenv("XPKI_CERT");
    unsetenv("XPKI_PASS");

    setenv("STATICXPKI", "1", 1);
    setenv("STATIC_XPKI_CERT", "static_cert.pem", 1);
    setenv("STATIC_XPKI_PASS", "static_pass", 1);

    char* certName = nullptr;
    char* phrase = nullptr;
    EXPECT_EQ(getMtlsCerts(&certName, &phrase), T2ERROR_SUCCESS);
    EXPECT_STREQ(certName, "static_cert.pem");
    EXPECT_STREQ(phrase, "static_pass");

    free(certName);
    free(phrase);
    unsetenv("STATICXPKI");
    unsetenv("STATIC_XPKI_CERT");
    unsetenv("STATIC_XPKI_PASS");
}

TEST(GetMtlsCertsTest, ReturnsFailureWhenNoEnvSet) {
    unsetenv("XPKI");
    unsetenv("XPKI_CERT");
    unsetenv("XPKI_PASS");
    unsetenv("STATICXPKI");
    unsetenv("STATIC_XPKI_CERT");
    unsetenv("STATIC_XPKI_PASS");

    char* certName = nullptr;
    char* phrase = nullptr;
    EXPECT_EQ(getMtlsCerts(&certName, &phrase), T2ERROR_FAILURE);
}
#else
TEST_F(utilsTestFixture, getMtlsCerts1)
{
    char* certName = nullptr;
    char* phrase = nullptr;
    EXPECT_CALL(*g_systemMock, access(_,_))
            .Times(1)
            .WillOnce(Return(0));
    #ifdef LIBRDKCONFIG_BUILD
    EXPECT_CALL(*g_rdkconfigMock, rdkconfig_get(_,_,_))
            .Times(1)
	    .WillOnce(Return(RDKCONFIG_OK));
    #endif
    EXPECT_EQ(T2ERROR_SUCCESS, getMtlsCerts(&certName, &phrase));
}

TEST_F(utilsTestFixture, getMtlsCerts2)
{
    char* certName = nullptr;
    char* phrase = nullptr;
    EXPECT_CALL(*g_systemMock, access(_,_))
            .Times(2)
            .WillOnce(Return(-1))
	    .WillOnce(Return(0));
    FILE* mockfp = (FILE *)0xffffffff;
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(mockfp));
    EXPECT_CALL(*g_fileIOMock, fscanf(_,_,_))
            .Times(1)
            .WillOnce(Return(1));
    EXPECT_CALL(*g_fileIOMock, fclose(_))
            .Times(1)
            .WillOnce(Return(0));
    #ifdef LIBRDKCONFIG_BUILD
    EXPECT_CALL(*g_rdkconfigMock, rdkconfig_get(_,_,_))
            .Times(1)
            .WillOnce(Return(RDKCONFIG_OK));
    #endif
    EXPECT_EQ(T2ERROR_SUCCESS, getMtlsCerts(&certName, &phrase));
}

TEST_F(utilsTestFixture, getMtlsCerts3)
{
    char* certName = nullptr;
    char* phrase = nullptr;
    EXPECT_CALL(*g_systemMock, access(_,_))
            .Times(2)
            .WillOnce(Return(-1))
            .WillOnce(Return(-1));
    EXPECT_EQ(T2ERROR_FAILURE, getMtlsCerts(&certName, &phrase));
}

#endif


TEST_F(utilsTestFixture, get_system_uptime)
{

    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(::testing::ReturnNull());
    EXPECT_EQ(0.0, get_system_uptime());
}

TEST_F(utilsTestFixture, get_system_uptime1)
{
    FILE* mockfp = (FILE *)0xffffffff;
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(mockfp));
    EXPECT_CALL(*g_fileIOMock, fscanf(_,_,_))
            .Times(1)
            .WillOnce(Return(1));
    EXPECT_CALL(*g_fileIOMock, fclose(_))
            .Times(1)
            .WillOnce(Return(0));
    EXPECT_EQ(0.0, get_system_uptime());
}

TEST_F(utilsTestFixture, SAVECONFITOFILE1)
{
     const char* filepath = "/nvram/.t2reportprofiles";
     const char* profilename = "Profile_1";
     const char* config = "This is a config string";
     FILE* mockfp = NULL;
     EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(mockfp));
     ASSERT_EQ(T2ERROR_FAILURE, saveConfigToFile(filepath, profilename, config));
}
/*
TEST_F(utilsTestFixture, SAVECONFITOFILE2)
{
     const char* filepath = "/nvram/.t2reportprofiles";
     const char* profilename = "Profile_1";
     const char* config = "This is a config string";
     FILE* mockfp = (FILE *)0xffffffff;

    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
	.Times(1)
        .WillOnce(Return(mockfp));
    EXPECT_CALL(*g_fileIOMock, fprintf(_,_,_))
        .Times(1)
        .WillOnce(Return(-1));
    EXPECT_CALL(*g_fileIOMock, fclose(_))
	.Times(1)
        .WillOnce(Return(0));

    EXPECT_EQ(saveConfigToFile(filepath, profilename, config), T2ERROR_SUCCESS);

}
*/
TEST_F(utilsTestFixture, getDevicePropertyData)
{

     FILE* mockfp = NULL;
     char UseHWBasedCert[8] = { '\0' };
     EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(mockfp));
     ASSERT_EQ(false, getDevicePropertyData("UseSEBasedCert", UseHWBasedCert, sizeof(UseHWBasedCert)));
}

TEST_F(utilsTestFixture, getDevicePropertyData1)
{
     FILE* mockfp = (FILE *)0xffffffff;
     char UseHWBasedCert[8] = { '\0' };
     EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(mockfp));
     EXPECT_CALL(*g_fileIOMock, fgets(_,_,_))
            .Times(1)
            .WillOnce(Return((char*)NULL));
     EXPECT_CALL(*g_fileIOMock, fclose(_))
            .Times(1)
            .WillOnce(Return(0));
     ASSERT_EQ(false, getDevicePropertyData("UseSEBasedCert", UseHWBasedCert, sizeof(UseHWBasedCert)));
}

TEST_F(utilsTestFixture, MSGPACKSAVECONFIG)
{
     const char* path = "/nvram/.t2reportprofiles";
     const char* fileName = "msgpack.profiles";
     const char* msgpack = "3wAAAAGocHJvZmlsZXPdAAAABN8AAAADpG5hbWW1UkRLQl9DQ1NQV2lmaV9Qcm9maWxlpGhhc2ilaGFzaDeldmFsdWXfAAAAC6ROYW1ltVJES0JfQ0";
     FILE* mockfp = NULL;
     EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(mockfp));
     ASSERT_EQ(T2ERROR_FAILURE,  MsgPackSaveConfig(path, fileName, msgpack, 45));
}

TEST_F(utilsTestFixture, MSGPACKSAVECONFIG1)
{
     const char* path = "/nvram/.t2reportprofiles";
     const char* fileName = "msgpack.profiles";
     const char* msgpack = "3wAAAAGocHJvZmlsZXPdAAAABN8AAAADpG5hbWW1UkRLQl9DQ1NQV2lmaV9Qcm9maWxlpGhhc2ilaGFzaDeldmFsdWXfAAAAC6ROYW1ltVJES0JfQ0";
     FILE* mockfp = (FILE *)0xffffffff;
     EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(mockfp));
     EXPECT_CALL(*g_fileIOMock, fwrite(_,_,_,_))
            .Times(1)
            .WillOnce(Return(45));
     EXPECT_CALL(*g_fileIOMock, fclose(_))
            .Times(1)
            .WillOnce(Return(0));
     ASSERT_EQ(T2ERROR_SUCCESS,  MsgPackSaveConfig(path, fileName, msgpack, 45));
}

TEST_F(utilsTestFixture, CLEARPERSISTENCEFOLDER)
{
        const char* path = "/tmp/profiles";
        #ifdef LIBSYSWRAPPER_BUILD
        EXPECT_CALL(*g_systemMock, v_secure_system(_))
                .Times(1)
                .WillOnce(Return(-1));
        #endif
        EXPECT_CALL(*g_systemMock, system(_))
                .Times(1)
                .WillOnce(Return(-1));
        clearPersistenceFolder(path);
}

TEST_F(utilsTestFixture, REMOVEPROFILEFROMDISK)
{
        const char* path = "/tmp/profiles";
        const char* fileName = "Profile1";
        EXPECT_CALL(*g_systemMock, unlink(_))
                .Times(1)
                .WillOnce(Return(-1));
        removeProfileFromDisk(path, fileName);
}

TEST_F(utilsTestFixture, SAVECACHEDREPORTTOPERSF)
{
    const char* profile = "FR2_US_TC2";
     DIR* mockfd = (DIR *)NULL;
     Vector* reportlist = NULL;
     Vector_Create(&reportlist);
     Vector_PushBack(reportlist, strdup("{FR2_US_TC2:[{UPTIME:NULL}]}"));
     EXPECT_CALL(*g_fileIOMock, opendir(_))
           .Times(1)
           .WillOnce(::testing::Return(mockfd));
     EXPECT_CALL(*g_fileIOMock, mkdir(_,_))
           .Times(1)
           .WillOnce(Return(-1));
     ASSERT_EQ(T2ERROR_FAILURE, saveCachedReportToPersistenceFolder(profile, reportlist));
     Vector_Destroy(reportlist, free);
}

TEST_F(utilsTestFixture, SAVECACHEDREPORTTOPERSF1)
{
    const char* profile = "FR2_US_TC2";
    FILE* mockfp = (FILE *)0xffffffff;
    DIR* mockfd = (DIR *)0xffffffff;
    Vector* reportlist = NULL;
    Vector_Create(&reportlist);
    Vector_PushBack(reportlist, strdup("{FR2_US_TC2:[{UPTIME:NULL}]}"));
    EXPECT_CALL(*g_fileIOMock, opendir(_))
            .Times(1)
            .WillOnce(Return(mockfd));
    EXPECT_CALL(*g_fileIOMock, closedir(_))
            .Times(1)
            .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(::testing::ReturnNull());
    ASSERT_EQ(T2ERROR_FAILURE, saveCachedReportToPersistenceFolder(profile, reportlist));
    Vector_Destroy(reportlist, free);
}

TEST_F(utilsTestFixture, POPULATECACHE_TEST1)
{
    const char* profile = "FR2_US_TC2";
    FILE* mockfp = (FILE *)0xffffffff;
    Vector* outReportlist = NULL;
    Vector_Create(&outReportlist);
    Vector_PushBack(outReportlist, strdup("{FR2_US_TC2:[{UPTIME:NULL}]}"));
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(mockfp));
    EXPECT_CALL(*g_fileIOMock, getline(_,_,_))
             .Times(1)
             .WillOnce(Return(-1));
    EXPECT_CALL(*g_fileIOMock, fclose(_))
            .Times(1)
            .WillOnce(Return(0));
    EXPECT_CALL(*g_systemMock, remove(_))
            .Times(1)
            .WillOnce(Return(-1));
    ASSERT_EQ(T2ERROR_FAILURE, populateCachedReportList(profile, outReportlist));
    Vector_Destroy(outReportlist, free);
}

TEST_F(utilsTestFixture, POPULATECACHE)
{
        const char* profile = "FR2_US_TC2";
        FILE* mockfp = (FILE *)0xffffffff;
        Vector* outReportlist = NULL;
        Vector_Create(&outReportlist);
        Vector_PushBack(outReportlist, strdup("{FR2_US_TC2:[{UPTIME:NULL}]}"));
        Vector_PushBack(outReportlist, strdup("{FR2_US_TC2:[{UPTIME:5688}]}"));
        EXPECT_CALL(*g_fileIOMock, fopen(_,_))
                .Times(1)
                .WillOnce(Return(mockfp));
        EXPECT_CALL(*g_fileIOMock, getline(_,_,_))
             .Times(2)
             .WillOnce(Return(0))
             .WillOnce(Return(-1));
        EXPECT_CALL(*g_fileIOMock, fclose(_))
            .Times(1)
            .WillOnce(Return(0));
        EXPECT_CALL(*g_systemMock, remove(_))
                .Times(1)
                .WillOnce(Return(0));
        ASSERT_EQ(T2ERROR_SUCCESS, populateCachedReportList(profile, outReportlist));
        Vector_Destroy(outReportlist, free);
}

TEST_F(utilsTestFixture, getPrivacyModeFromPersistentFolder)
{
   char *privacymode = NULL;
   FILE* mockfp = (FILE *)NULL;
   EXPECT_CALL(*g_fileIOMock, fopen(_,_))
           .Times(1)
           .WillOnce(Return(mockfp));
    EXPECT_EQ(T2ERROR_FAILURE, getPrivacyModeFromPersistentFolder(&privacymode));
}

TEST_F(utilsTestFixture, savePrivacyModeToPersistentFolder)
{
    char *privacymode = "SHARE";
    DIR* dir = (DIR *)0xffffffff;
    EXPECT_CALL(*g_fileIOMock, opendir(_))
            .Times(1)
            .WillOnce(::testing::ReturnNull());
    EXPECT_CALL(*g_fileIOMock, mkdir(_,_))
            .Times(1)
            .WillOnce(Return(-1));
    #ifdef LIBSYSWRAPPER_BUILD
    EXPECT_CALL(*g_systemMock, v_secure_system(_))
            .Times(1)
            .WillOnce(Return(0));
    #else
    EXPECT_CALL(*g_systemMock, system(_))
            .Times(1)
            .WillOnce(Return(0));
    #endif
    EXPECT_EQ(T2ERROR_FAILURE, savePrivacyModeToPersistentFolder(privacymode));
}

TEST_F(utilsTestFixture, savePrivacyModeToPersistentFolder1)
{
    char *privacymode = "SHARE";
    FILE* mockfp = (FILE *)0xffffffff;
    DIR* dir = (DIR *)0xffffffff;
    EXPECT_CALL(*g_fileIOMock, opendir(_))
            .Times(1)
            .WillOnce(Return(dir));
    EXPECT_CALL(*g_fileIOMock, closedir(_))
            .Times(1)
            .WillOnce(Return(0));
    #ifdef LIBSYSWRAPPER_BUILD
    EXPECT_CALL(*g_systemMock, v_secure_system(_))
            .Times(1)
	    .WillOnce(Return(0));
    #else
    EXPECT_CALL(*g_systemMock, system(_))
            .Times(1)
	    .WillOnce(Return(0));
    #endif
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(::testing::ReturnNull());
    EXPECT_EQ(T2ERROR_FAILURE, savePrivacyModeToPersistentFolder(privacymode));
}

TEST_F(utilsTestFixture, initMtls)
{
    FILE* mockfp = (FILE *)0xffffffff;
    char UseHWBasedCert[8] = { '\0' };
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(mockfp));
    EXPECT_CALL(*g_fileIOMock, fgets(_,_,_))
            .Times(1)
            .WillOnce(Return((char*)NULL));
    EXPECT_CALL(*g_fileIOMock, fclose(_))
            .Times(1)
            .WillOnce(Return(0));
    initMtls();
}
