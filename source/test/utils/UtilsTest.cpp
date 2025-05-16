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
#define PROFILE_NAME "/opt/opt/opt/opt/opt/opt/logs/logs/logs/logs/logs/logs/opt/opt/opt/opt/opt/opt/logs/logs/logs/logs/logs/logs/opt/opt/opt/opt/opt/opt/logs/logs/logs/logs/logs/logs/opt/opt/opt/opt/opt/opt/logs/logs/logs/logs/logs/logs"
#include "test/mocks/SystemMock.h"
#include "test/mocks/FileioMock.h"
#include "test/mocks/rdklogMock.h"
using namespace std;

using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;

SystemMock * g_SystemMock = NULL;  // This is the actual definition of the mock obj 
FileIOMock * g_fileIOMock = NULL;
rdklogMock *m_rdklogMock = NULL;
/*
#include <cstdio>

// Wrapper function for fopen
FILE* mock_fopen(const char* filename, const char* mode);

// Function pointer to switch between real and mock fopen
FILE* (*real_fopen)(const char* filename, const char* mode) = fopen;

// Mock implementation of fopen
/FILE* fopen(const char* filename, const char* mode) {
    return mock_fopen(filename, mode);
}

class MockFopen {
public:
    MOCK_METHOD(FILE*, fopen, (const char* filename, const char* mode));
    MOCK_METHOD(DIR*, opendir, (const char* filename));
    MOCK_METHOD(int, mkdir, (const char* filename, int mode));
};

// Singleton instance for testing
MockFopen* g_mock_fopen = nullptr;

// Mocked version of the wrapper function
 FILE* fopen_wrapper(const char* filename, const char* mode) {
    if (g_mock_fopen) {
        return g_mock_fopen->fopen(filename, mode);
    }
    return fopen(filename, mode);
}
DIR* opendir_wrapper(const char* filename) {
    if (g_mock_fopen) {
        return g_mock_fopen->opendir(filename);
    }
    return opendir(filename);
}
int mkdir_wrapper(const char* filename, int mode) {
    if (g_mock_fopen) {
        return g_mock_fopen->mkdir(filename, mode);
    }
    return mkdir(filename, mode);
}
*/
class rdklogTestFixture : public ::testing::Test {
    protected:
            rdklogMock rdklogmock_IO;

            rdklogTestFixture()
            {
                    m_rdklogMock = &rdklogmock_IO;
            }

            virtual ~rdklogTestFixture()
            {
                    m_rdklogMock = NULL;
            }
            virtual void SetUp()
            {
                    printf("%s\n", __func__);
            }

            virtual void TearDown()
            {
                    printf("%s\n", __func__);
            }
	    static void SetUpTestCase()
            {
                    printf("%s\n", __func__);
            }
            static void TearDownTestCase()
            {
                    printf("%s\n", __func__);
            }
};

class UtilsFileTestFixture : public ::testing::Test {
    protected:
        FileIOMock mockedFileIO;

        UtilsFileTestFixture()
        {
            g_fileIOMock = &mockedFileIO;

        }
        virtual ~UtilsFileTestFixture()
        {
            g_fileIOMock = NULL;
        }

        virtual void SetUp()
        {
            printf("%s\n", __func__);
        }

        virtual void TearDown()
        {
          //  printf("%s\n", __func__);
        }

        static void SetUpTestCase()
        {
            printf("%s\n", __func__);
        }

        static void TearDownTestCase()
        {
            //printf("%s\n", __func__);
        }
};

class UtilsSystemTestFixture : public ::testing::Test {
    protected:
        SystemMock mockedSystem;

        UtilsSystemTestFixture()
        {
            g_SystemMock = &mockedSystem;

        }
        virtual ~UtilsSystemTestFixture()
        {
            g_SystemMock = NULL;
        }

        virtual void SetUp()
        {
            printf("%s\n", __func__);
        }

        virtual void TearDown()
        {
            printf("%s\n", __func__);
        }

        static void SetUpTestCase()
        {
            printf("%s\n", __func__);
        }

        static void TearDownTestCase()
        {
            printf("%s\n", __func__);
        }
};

class UtilsTestFixture : public ::testing::Test {
    protected:
        SystemMock mockedSystem;
        FileIOMock mockedFileIO;

        UtilsTestFixture()
        {
            g_SystemMock = &mockedSystem;
            g_fileIOMock = &mockedFileIO;

        }
        virtual ~UtilsTestFixture()
        {
            g_SystemMock = NULL;
            g_fileIOMock = NULL;
        }

        virtual void SetUp()
        {
            printf("%s\n", __func__);
        }

        virtual void TearDown()
        {
            printf("%s\n", __func__);
        }

        static void SetUpTestCase()
        {
            printf("%s\n", __func__);
        }

        static void TearDownTestCase()
        {
            printf("%s\n", __func__);
        }
};

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
}

//Testing the vectors
//inputs for functions
Vector* config = NULL;
char *marker1  = "GTEST_TELEMETRY1";
char *marker2  = "GTEST_TELEMETRY2";


TEST(VECTOR_CREATE, VECTOR_NOT_NULL)
{
    EXPECT_EQ(T2ERROR_SUCCESS, Vector_Create(&config));
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
}

TEST(VECTOR_PUSHBACK, VECTOR_NOT_NULL)
{
     Vector_Create(&config);
     EXPECT_EQ(T2ERROR_SUCCESS, Vector_PushBack(config, (void *) strdup(marker1)));
     EXPECT_EQ(T2ERROR_SUCCESS, Vector_PushBack(config, (void *) strdup(marker2)));
     Vector_Destroy(config,free);
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
}


TEST(VECTOR_CLEAR, VECTOR_NULL)
{
    Vector_Create(&config);
    EXPECT_EQ(T2ERROR_INVALID_ARGS, Vector_Clear(NULL, free));
    EXPECT_EQ(T2ERROR_INVALID_ARGS, Vector_Clear(config, NULL));
    //Deleting the vector
    Vector_Destroy(config, free);
}


TEST(VECTOR_CLEAR, VECTOR_NOT_NULL)
{
     Vector_Create(&config);
     Vector_PushBack(config, (void *) strdup(marker1));
     EXPECT_EQ(T2ERROR_SUCCESS, Vector_Clear(config, free));
     //Deleting the vector
    Vector_Destroy(config, free);
}

TEST(VECTOR_DESTROY, VECTOR_NULL)
{
    Vector_Create(&config);
    EXPECT_EQ(T2ERROR_INVALID_ARGS, Vector_Destroy(NULL, free));
    //Deleting the vector
    Vector_Destroy(config, free);
}

TEST(VECTOR_DESTROY, VECTOR_NOT_NULL)
{
     Vector_Create(&config);
     Vector_PushBack(config, (void *) strdup(marker1));
     EXPECT_EQ(T2ERROR_SUCCESS, Vector_Destroy(config, free));
}

TEST(VECTOR_AT, VECTOR_NULL_AND_NOT_NULL)
{
     Vector_Create(&config);
     Vector_PushBack(config, (void *) strdup(marker1));
     Vector_PushBack(config, (void *) strdup(marker2));
     EXPECT_EQ(NULL, Vector_At(NULL, 1));
     EXPECT_EQ(NULL, Vector_At(config, 3));
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
}

TEST(VECTOR_REMOVE_ITEM, VECTOR_THIRD_NULL)
{
     Vector_Create(&config);
     Vector_PushBack(config, (void *) strdup(marker1));
     Vector_PushBack(config, (void *) strdup(marker2));
     EXPECT_EQ(T2ERROR_SUCCESS, Vector_RemoveItem(config, marker1, NULL));
     //Deleting the vector
     Vector_Destroy(config, free);
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
}

TEST(HASH_MAP_PUT, HASH_MAP_1_NULL)
{
    EXPECT_EQ(-1, hash_map_put(NULL, teststring, markerh1, NULL));
}

TEST(HASH_MAP_PUT, HASH_MAP_2_NULL)
{
    EXPECT_EQ(-1, hash_map_put(markerCompMap, NULL, markerh1, NULL));
}

TEST(HASH_MAP_PUT, HASH_MAP_3_NULL)
{
    EXPECT_EQ(-1, hash_map_put(markerCompMap, teststring, NULL, NULL));
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
   EXPECT_EQ(NULL, hash_map_lookup(markerCompMap, 3));
}

TEST(HASH_MAP_LOOKUPKEY, HASH_MAP_1_NULL)
{
   EXPECT_EQ(NULL, hash_map_lookupKey(NULL, 1));
}

TEST(HASH_MAP_LOOKUPKEY, HASH_MAP_2_NULL)
{
    EXPECT_EQ(NULL, hash_map_lookupKey(markerCompMap, 3));
}

TEST(HASH_MAP_GET, HASH_MAP_1_NULL)
{
    EXPECT_EQ(NULL, hash_map_get(NULL, "TEST_MARKER2"));
}

TEST(HASH_MAP_GET, HASH_MAP_2_NULL)
{
    EXPECT_EQ(NULL, hash_map_get(markerCompMap, NULL));
}

TEST(HASH_MAP_GET, HASH_MAP_INVALID)
{
   EXPECT_EQ(NULL, hash_map_get(markerCompMap, "TEST"));
}

TEST(HASH_MAP_GET_FIRST, HASH_MAP_NULL)
{
    EXPECT_EQ(NULL, hash_map_get_first(NULL));
}

TEST(HASH_MAP_GET_FIRST, HASH_MAP_VALUE_NULL)
{
    EXPECT_EQ(NULL, hash_map_get_first(markerCompMap1));
}

TEST(HASH_MAP_GET_NEXT, HASH_MAP_1_NULL)
{

    EXPECT_EQ(NULL, hash_map_get_next(NULL, (void *)teststring1));
}

TEST(HASH_MAP_GET_NEXT, HASH_MAP_2_NULL)
{
    EXPECT_EQ(NULL, hash_map_get_next(markerCompMap, NULL));
}

TEST(HASH_MAP_GET_NEXT, HASH_ITEM_INVALID)
{
    EXPECT_EQ(NULL, hash_map_get_next(markerCompMap, (void *)"TEST"));
}

TEST(HASH_MAP_REMOVE, HASH_MAP_NULL)
{
    EXPECT_EQ(NULL, hash_map_remove(NULL, teststring2));
}

TEST(HASH_MAP_REMOVE, HASH_ELEMENT_NULL)
{
    EXPECT_EQ(NULL,hash_map_remove(markerCompMap, NULL));
}

TEST(HASH_MAP_REMOVE, HASH_INVALID_ELEMENT)
{
    EXPECT_EQ(NULL, hash_map_remove(markerCompMap, "TEST"));
}

TEST(HASH_MAP_CLEAR1, check_markercompmap)
{
   hash_map_clear(markerCompMap, NULL);
   EXPECT_NO_THROW(check_markercompmap(markerCompMap));
}

TEST(HASH_MAP_CLEAR2, check_markercompmap)
{
   hash_map_clear(markerCompMap, free);
   EXPECT_NO_THROW(check_markercompmap(markerCompMap));
}

TEST(HASH_MAP_DESTROY1, check_markercompmap)
{
   hash_map_destroy(markerCompMap, NULL);
   EXPECT_NO_THROW(check_markercompmap(markerCompMap));
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
}

//setPrivacyMode
TEST(SETPRIVACYMODE, ARGS_NULL)
{
    char* privacyMode = strdup("SHARE");
    EXPECT_EQ(T2ERROR_SUCCESS, setPrivacyMode(privacyMode));
    free(privacyMode);
}


TEST_F(UtilsFileTestFixture, FETCHLOCALCONFIGS_FUNC1)
{
    const char* path = "/tmp/t2reportprofiles/";
    DIR *dir = (DIR*)NULL ;
    Vector* configlist = NULL;
    Vector_Create(&configlist);
    Vector_PushBack(configlist, (void *)strdup("marker1"));
  //  EXPECT_CALL(*g_fileIOMock, opendir(_))
    //       .Times(1)
      //     .WillOnce(Return(dir));

   // EXPECT_CALL(*g_fileIOMock, mkdir(_,_))
     //      .Times(1)
       //    .WillOnce(Return(-1));

    ASSERT_EQ(T2ERROR_FAILURE, fetchLocalConfigs(path, configlist));
    Vector_Destroy(configlist, free);
}

/*
TEST(UtilsFileTest, FETCHLOCALCONFIGS_FUNC)
{
    MockFopen mock;
    g_mock_fopen = &mock;
    const char* path = "/tmp/t2reportprofiles/";
    DIR *dir = NULL ;
    Vector* configlist = NULL;
    Vector_Create(&configlist);
    Vector_PushBack(configlist, (void *)strdup("marker1"));
    EXPECT_CALL(mock, opendir_wrapper(_))
           .Times(1)
           .WillOnce(Return(nullptr));

    EXPECT_CALL(mock, mkdir_wrapper(_,_))
           .Times(1)
           .WillOnce(Return(-1));

    ASSERT_EQ(T2ERROR_FAILURE, fetchLocalConfigs(path, configlist));
    Vector_Destroy(configlist, free);
    g_mock_fopen = nullptr;
}
#if defined (MTLS_FROM_ENV)
TEST_F(UtilsFileTestFixture, getMtlsCerts)
{ 
    char* cert = NULL;
    char* key = NULL;
    char* env =  (char*)0xffffffff;
    EXPECT_CALL(*g_fileIOMock, getenv(_))
            .Times(3)
            .WillOnce(Return(env))
            .WillOnce(Return(env))
            .WillOnce(Return(env));

    EXPECT_EQ(T2ERROR_SUCCESS, getMtlsCerts(&cert, &key));
   
}

TEST_F(UtilsFileTestFixture, getMtlsCerts1)
{
    char* cert = NULL;
    char* key = NULL;
    char* env =  (char*)0xffffffff;
    EXPECT_CALL(*g_fileIOMock, getenv(_))
            .Times(4)
            .WillOnce(ReturnNull())
            .WillOnce(Return(env))
            .WillOnce(Return(env))
            .WillOnce(Return(env));

    EXPECT_EQ(T2ERROR_SUCCESS, getMtlsCerts(&cert, &key));
   
}
TEST_F(UtilsFileTestFixture, getMtlsCerts2)
{
    char* cert = NULL;
    char* key = NULL;
    char* env =  (char*)0xffffffff;
    EXPECT_CALL(*g_fileIOMock, getenv(_))
            .Times(2)
            .WillOnce(ReturnNull())
            .WillOnce(ReturnNull());

    EXPECT_EQ(T2ERROR_FAILURE, getMtlsCerts(&cert, &key));
   
}
TEST_F(UtilsFileTestFixture, get_system_uptime)
{
    
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(ReturnNull());
    EXPECT_EQ(0.0, getSystemUptime());
}

TEST_F(UtilsFileTestFixture, get_system_uptime1)
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
    EXPECT_EQ(0.0, getSystemUptime());
}


*/    
/*
TEST(UtilsFileTest, SAVECONFITOFILE)
{
     MockFopen mock;                                                             
    g_mock_fopen = &mock; 
     const char* filepath = "/nvram/.t2reportprofiles";
     const char* profilename = "Profile_1";
     const char* config = "This is a config string";
     FILE* mockfp = NULL;
     EXPECT_CALL(mock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(mockfp));
     ASSERT_EQ(T2ERROR_FAILURE, saveConfigToFile(filepath, profilename, config));
     g_mock_fopen = nullptr;
}

TEST_F(UtilsFileTestFixture, SAVECONFITOFILE1)
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
     ASSERT_EQ(T2ERROR_SUCCESS, saveConfigToFile(filepath, profilename, config));
}



     
TEST_F(UtilsFileTestFixture, getDevicePropertyData)
{
     
     FILE* mockfp = NULL;
     char UseHWBasedCert[8] = { '\0' };
     EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(mockfp));
     ASSERT_EQ(false, getDevicePropertyData("UseSEBasedCert", UseHWBasedCert, sizeof(UseHWBasedCert)));
}

TEST_F(UtilsFileTestFixture, getDevicePropertyData1)
{
     FILE* mockfp = (FILE *)0xffffffff;
     char UseHWBasedCert[8] = { '\0' };
     EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(mockfp));
     EXPECT_CALL(*g_fileIOMock, fgets(_,_,_))
            .Times(1)
            .WillOnce(Return(NULL));
     EXPECT_CALL(*g_fileIOMock, fclose(_))
            .Times(1)
            .WillOnce(Return(0));       
     ASSERT_EQ(false, getDevicePropertyData("UseSEBasedCert", UseHWBasedCert, sizeof(UseHWBasedCert)));
}


TEST_F(UtilsFileTestFixture, MSGPACKSAVECONFIG)
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

TEST_F(UtilsFileTestFixture, MSGPACKSAVECONFIG1)
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
     ASSERT_EQ(T2ERROR_FAILURE,  MsgPackSaveConfig(path, fileName, msgpack, 45));
}

TEST_F(UtilsSystemTestFixture, CLEARPERSISTENCEFOLDER)
{
        const char* path = "/tmp/profiles";
        #ifdef LIBSYSWRAPPER_BUILD
        EXPECT_CALL(*g_SystemMock, v_secure_system(_))
                .Times(1)
                .WillOnce(Return(-1));
        #endif
        EXPECT_CALL(*g_SystemMock, system(_))
                .Times(1)
                .WillOnce(Return(-1));
        clearPersistenceFolder(path);
}

TEST_F(UtilsSystemTestFixture, REMOVEPROFILEFROMDISK)
{
	const char* path = "/tmp/profiles";
        const char* fileName = "Profile1";
        EXPECT_CALL(*g_SystemMock, unlink(_))
                .Times(1)
                .WillOnce(Return(-1));
	removeProfileFromDisk(path, fileName);
}

TEST_F(UtilsFileTestFixture, SAVECACHEDREPORTTOPERSF)
{
    const char* profile = "FR2_US_TC2";
     DIR* mockfd = (DIR *)NULL;
     Vector* reportlist = NULL;
     Vector_Create(&reportlist);
     Vector_PushBack(reportlist, strdup("{FR2_US_TC2:[{UPTIME:NULL}]}"));
     EXPECT_CALL(*g_fileIOMock, opendir(_))
           .Times(1)
           .WillOnce(::testing::Return(mockfd));
     EXPECT_CALL(*g_fileIOMock, mkdir(_, _))
           .Times(1)
           .WillOnce(Return(-1));
     ASSERT_EQ(T2ERROR_FAILURE, saveCachedReportToPersistenceFolder(profile, reportlist));
     Vector_Destroy(reportlist, free);
}

TEST_F(UtilsFileTestFixture, SAVECACHEDREPORTTOPERSF1)
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
            .WillOnce(ReturnNull());
    ASSERT_EQ(T2ERROR_FAILURE, saveCachedReportToPersistenceFolder(profile, reportlist));
    Vector_Destroy(reportlist, free);
}

TEST_F(UtilsFileTestFixture, POPULATECACHE_TEST)
{
    const char* profile = "FR2_US_TC2";
    FILE* mockfp = (FILE *)NULL;
    Vector* outReportlist = NULL;
    Vector_Create(&outReportlist);
    Vector_PushBack(outReportlist, strdup("{FR2_US_TC2:[{UPTIME:NULL}]}"));
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(mockfp));
    EXPECT_CALL(*g_fileIOMock, fclose(_))
            .Times(1)
            .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, remove(_))
            .Times(1)
            .WillOnce(Return(-1));
    ASSERT_EQ(T2ERROR_FAILURE, populateCachedReportList(profile, outReportlist));
    Vector_Destroy(outReportlist, free);
}

TEST_F(UtilsTestFixture, POPULATECACHE)
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
        EXPECT_CALL(*g_SystemMock, remove(_))
                .Times(1)
                .WillOnce(Return(0));
        ASSERT_EQ(T2ERROR_FAILURE, populateCachedReportList(profile, outReportlist));
        Vector_Destroy(outReportlist, free);
}

TEST_F(UtilsFileTestFixture, getPrivacyModeFromPersistentFolder)
{
   char *privacymode = NULL;
   FILE* mockfp = (FILE *)0xffffffff;
   EXPECT_CALL(*g_fileIOMock, fopen(_,_))
           .Times(1)
           .WillOnce(Return(mockfp));
   EXPECT_CALL(*g_fileIOMock, stat(_,_))
           .Times(1)
           .WillOnce(Return(0));
   EXPECT_CALL(*g_fileIOMock, fread(_,_,_,_))
           .Times(1)
           .WillOnce(Return(10));
    EXPECT_CALL(*g_fileIOMock, fclose(_))
           .Times(1)
           .WillOnce(Return(0));
    EXPECT_EQ(T2ERROR_FAILURE, getPrivacyModeFromPersistentFolder(&privacymode));
}

TEST_F(UtilsFileTestFixture, savePrivacyModeToPersistentFolder)
{
    char *privacymode = "SHARE";
    DIR* dir = (DIR *)0xffffffff;
    EXPECT_CALL(*g_fileIOMock, opendir(_))
            .Times(1)
            .WillOnce(ReturnNull());
    EXPECT_CALL(*g_fileIOMock, mkdir(_,_))
            .Times(1)
            .WillOnce(Return(-1));
    EXPECT_EQ(T2ERROR_FAILURE, savePrivacyModeToPersistentFolder(privacymode));
}

TEST_F(UtilsFileTestFixture, savePrivacyModeToPersistentFolder1)
{
    char *privacymode = "SHARE";
    FILE* mockfp = (FILE *)0xffffffff;
    DIR* dir = (DIR *)0xffffffff;
    EXPECT_CALL(*g_fileIOMock, opendir(_))
            .Times(1)
            .WillOnce(Return(dir));
    #if defined(DROP_ROOT_PRIV)
    #ifdef LIBSYSWRAPPER_BUILD
    EXPECT_CALL(*g_SystemMock, v_secure_system(_))
            .Times(1)
            .WillOnce(Return(0));
    #else
    EXPECT_CALL(*g_SystemMock, system(_))
            .Times(1)
            .WillOnce(Return(0));
    #endif
    #endif
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(mockfp));
    EXPECT_CALL(*g_fileIOMock, fprintf(_,_,_))
            .Times(1)
            .WillOnce(Return(-1));
    EXPECT_CALL(*g_fileIOMock, fclose(_))
            .Times(1)
            .WillOnce(Return(0));
    EXPECT_EQ(T2ERROR_FAILURE, savePrivacyModeToPersistentFolder(privacymode));
}

#if defined(PRIVACYMODES_CONTROL)
char* privacymode = "SHARE";

#if defined(DROP_ROOT_PRIV)
#ifdef LIBSYSWRAPPER_BUILD
TEST_F(UtilsTestFixture, SETPRIVACYMODE1)
{
    DIR* dir = (DIR *)0xffffffff;
    EXPECT_CALL(*g_fileIOMock, opendir(_))
                .Times(1)
                .WillOnce(Return(dir));
    EXPECT_CALL(*g_SystemMock, v_secure_system(_))
                .Times(1)
                .WillOnce(Return(-1));
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
                .Times(1)
                .WillOnce(testing::ReturnNull());
    ASSERT_EQ(T2ERROR_FAILURE, setPrivacyMode(privacymode));
}
#else
TEST_F(UtilsTestFixture, SETPRIVACYMODE1)
{
    DIR* dir = (DIR *)0xffffffff;
    EXPECT_CALL(*g_fileIOMock, opendir(_))
                .Times(1)
                .WillOnce(Return(dir));
    EXPECT_CALL(*g_SystemMock, system(_))
                .Times(1)
                .WillOnce(Return(-1));
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
                .Times(1)
                .WillOnce(testing::ReturnNull());
    ASSERT_EQ(T2ERROR_FAILURE, setPrivacyMode(privacymode));
}
#endif
#endif


TEST_F(UtilsFileTestFixture, SETPRIVACYMODE2)
{
    EXPECT_CALL(*g_fileIOMock, opendir(_))
                .Times(1)
                .WillOnce(testing::ReturnNull());
    EXPECT_CALL(*g_fileIOMock, mkdir(_,_))
                .Times(1)
                .WillOnce(Return(-1));

     ASSERT_EQ(T2ERROR_FAILURE, setPrivacyMode(privacymode));
}

#if defined(DROP_ROOT_PRIV)
#ifdef LIBSYSWRAPPER_BUILD
TEST_F(UtilsTestFixture, SETPRIVACYMODE3)
{
    DIR* dir = (DIR *)0xffffffff;
    FILE *fp = (FILE *) 0xffffffff;
    EXPECT_CALL(*g_fileIOMock, opendir(_))
                .Times(1)
                .WillOnce(Return(dir));
    EXPECT_CALL(*g_SystemMock, v_secure_system(_))
                .Times(1)
                .WillOnce(Return(-1));
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
                .Times(1)
                .WillOnce(Return(fp));
    EXPECT_CALL(*g_fileIOMock, fclose(_))
                .Times(1)
                .WillOnce(Return(-1));
    ASSERT_EQ(T2ERROR_FAILURE, setPrivacyMode(privacymode));
}
#else
TEST_F(UtilsTestFixture, SETPRIVACYMODE3)
{
    DIR* dir = (DIR *)0xffffffff;
    EXPECT_CALL(*g_fileIOMock, opendir(_))
                .Times(1)
                .WillOnce(Return(dir));
    EXPECT_CALL(*g_SystemMock, system(_))
                .Times(1)
                .WillOnce(Return(-1));
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
                .Times(1)
                .WillOnce(Return(fp));
    EXPECT_CALL(*g_fileIOMock, fclose(_))
                .Times(1)
                .WillOnce(Return(-1));
    ASSERT_EQ(T2ERROR_FAILURE, setPrivacyMode(privacymode));
}
#endif
#endif

char* privMode = NULL;
TEST_F(UtilsFileTestFixture, GETPRIVACYMODE1)
{
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
                .Times(1)
                .WillOnce(testing::ReturnNull());
    getPrivacyMode(&privMode);
    EXPECT_NE(*privMode, NULL);
}
#endif

*/
