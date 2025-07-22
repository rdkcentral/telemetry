/*
 * Copyright 2020 Comcast Cable Communications Management, LLC
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

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <iostream>
#include <stdexcept>
#include "test/mocks/SystemMock.h"
#include "test/mocks/FileioMock.h"
#include "test/mocks/rdklogMock.h"

extern "C" {
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <glib.h>
#include <glib/gi18n.h>
#include "utils/vector.h"
#include "dcautil/dca.h"
#include "dcautil/legacyutils.h"
#include "dcautil/dcautil.h"
#include "dcautil/dcalist.h"
#include "dcautil/rdk_linkedlist.h"
#include "telemetry2_0.h"
#include "utils/t2log_wrapper.h"
#include "utils/t2common.h"
#include "ccspinterface/busInterface.h"
}

using namespace std;

using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;

FileMock *g_fileIOMock = NULL;
SystemMock * g_systemMock = NULL;
rdklogMock *m_rdklogMock = NULL;

//dcaproc.c

TEST(GETPROCUSAGE, GREPRESULTLIST_NULL)
{
   EXPECT_EQ(-1, getProcUsage("telemetry2_0", NULL, false, NULL, NULL));
   EXPECT_EQ(-1, getProcUsage("telemetry2_0", NULL, true, NULL, NULL));
   EXPECT_EQ(-1, getProcUsage("telemetry2_0", NULL, false, "[0-9]", NULL));
   EXPECT_EQ(-1, getProcUsage("telemetry2_0", NULL, true, "[0-9]", NULL));
}

TEST(GETPROCUSAGE, PROCESS_NULL)
{
   Vector* grepResultList = NULL;
   char* filename = NULL;
   filename = strdup("top_log.txt");
   Vector_Create(&grepResultList);
   Vector_PushBack(grepResultList, (void*) strdup("SYS_INFO_BOOTUP"));
   Vector_PushBack(grepResultList, (void*) strdup("SYS_INFO_MEM"));
   EXPECT_EQ(-1, getProcUsage(NULL, grepResultList, false, NULL, filename));
   EXPECT_EQ(-1, getProcUsage(NULL, grepResultList, true, NULL, NULL));
   EXPECT_EQ(-1, getProcUsage(NULL, grepResultList, false, "[0-9]", NULL));
   EXPECT_EQ(-1, getProcUsage(NULL, grepResultList, true, "[0-9]", filename));
   Vector_Destroy(grepResultList, free);
   free(filename);
}

TEST(GETPROCPIDSTAT, PINFO_NULL)
{
   EXPECT_EQ(0, getProcPidStat(12345, NULL));
}

TEST(GETPROCINFO, PMINFO_NULL)
{
   char* filename = NULL;
   char* processName = NULL;
   processName = strdup("telemetry2_0");
   EXPECT_EQ(0, getProcInfo(NULL, filename));

   filename = strdup("top_log.txt");
   EXPECT_EQ(0, getProcInfo(NULL, filename));

   procMemCpuInfo pInfo;
   memset(&pInfo, '\0', sizeof(procMemCpuInfo));
   memcpy(pInfo.processName, processName, strlen(processName) + 1);
   pInfo.total_instance = 0;
   EXPECT_EQ(0,getProcInfo(&pInfo, NULL));
   free(filename);
}

TEST(GETMEMINFO, PMINFO_NULL)
{
   EXPECT_EQ(0, getMemInfo(NULL));
}

TEST(GETCPUINFO, PINFO_NULL)
{
   char* filename = NULL;
   EXPECT_EQ(0, getCPUInfo(NULL, filename));
   filename = strdup("top_log.txt");
   EXPECT_EQ(0, getCPUInfo(NULL, filename));


}
//dcautil.c
char* gmarker1 = "SYS_INFO_BOOTUP";
char* dcamarker1 = "SYS_INFO1";
char* dcamarker2 = "SYS_INFO2";

TEST(GETGREPRESULTS, PROFILENAME_NULL)
{
   Vector* markerlist = NULL;
   Vector_Create(&markerlist);
   Vector_PushBack(markerlist, (void*) strdup(dcamarker1));
   Vector_PushBack(markerlist, (void*) strdup(dcamarker2));
   Vector* grepResultlist = NULL;
   Vector_Create(&grepResultlist);
   GrepSeekProfile *gsProfile = (GrepSeekProfile *)malloc(sizeof(GrepSeekProfile));
   gsProfile->logFileSeekMap = hash_map_create();
   gsProfile->execCounter = 0;
   hash_map_put(gsProfile->logFileSeekMap, strdup("t2_log.txt"), (void*)1, free);
   EXPECT_EQ(T2ERROR_FAILURE, getGrepResults(NULL, markerlist, &grepResultlist, false, false,"/opt/logs"));
   EXPECT_EQ(T2ERROR_FAILURE, getGrepResults(&gsProfile, NULL, &grepResultlist, false, false,"/opt/logs"));
   EXPECT_EQ(T2ERROR_FAILURE, getGrepResults(&gsProfile, markerlist, NULL, false, false,"/opt/logs"));
   EXPECT_EQ(T2ERROR_FAILURE, getGrepResults(NULL, markerlist, &grepResultlist, false, true,"/opt/logs"));
   EXPECT_EQ(T2ERROR_FAILURE, getGrepResults(&gsProfile, NULL, &grepResultlist, false, true,"/opt/logs"));
   EXPECT_EQ(T2ERROR_FAILURE, getGrepResults(&gsProfile, markerlist, NULL, false, true,"/opt/logs"));
   Vector_Destroy(markerlist, free);
   Vector_Destroy(grepResultlist, free);
   if(gsProfile->logFileSeekMap)
   {
      hash_map_destroy(gsProfile->logFileSeekMap, free);
   }
   free(gsProfile);
   grepResultlist = NULL;
   markerlist = NULL;
}

#ifdef PERSIST_LOG_MON_REF
TEST(saveSeekConfigtoFile, profilename_NULL)
{
   GrepSeekProfile *gsProfile = (GrepSeekProfile *)malloc(sizeof(GrepSeekProfile));
   gsProfile->logFileSeekMap = hash_map_create();
   gsProfile->execCounter = 0;
   hash_map_put(gsProfile->logFileSeekMap, strdup("t2_log.txt"), (void*)1, free);
   EXPECT_EQ(T2ERROR_FAILURE, saveSeekConfigtoFile(NULL, gsProfile));
   EXPECT_EQ(T2ERROR_FAILURE, saveSeekConfigtoFile("RDKB_Profile1", NULL));
   if(gsProfile->logFileSeekMap)
   {
        hash_map_destroy(gsProfile->logFileSeekMap, free);
        gsProfile->logFileSeekMap = NULL;
   }
   EXPECT_EQ(T2ERROR_FAILURE, saveSeekConfigtoFile("RDKB_Profile1", gsProfile));
}

TEST(loadSavedSeekConfig, profilename_NULL)
{
   GrepSeekProfile *gsProfile = (GrepSeekProfile *)malloc(sizeof(GrepSeekProfile));
   gsProfile->logFileSeekMap = hash_map_create();
   gsProfile->execCounter = 0;
   EXPECT_EQ(T2ERROR_FAILURE, loadSavedSeekConfig(NULL, gsProfile));
   hash_map_destroy(gsProfile->logFileSeekMap, free);
   free(gsProfile);
}

#endif


TEST(GETLOADAVG, VECTOR_REGEX_NULL)
{
    EXPECT_EQ(0, getLoadAvg(NULL, false, NULL));
    EXPECT_EQ(0, getLoadAvg(NULL, true, NULL));
    EXPECT_EQ(0, getLoadAvg(NULL, false, "[0-9]"));
    EXPECT_EQ(0, getLoadAvg(NULL, true, "[0-9]"));
}

// Test createGrepResultObj with NULL parameters
TEST(CreateGrepResultObj, NullParameters)
{
    GrepResult* result = createGrepResultObj(NULL, "value", false, NULL);
    EXPECT_EQ(result, nullptr);
    
    result = createGrepResultObj("name", NULL, false, NULL);
    EXPECT_EQ(result, nullptr);
}

// Test createGrepResultObj with valid parameters
TEST(CreateGrepResultObj, ValidParameters)
{
    const char* name = "testMarker";
    const char* value = "testValue";
    const char* regex = "test.*";
    
    GrepResult* result = createGrepResultObj(name, value, true, regex);
    EXPECT_NE(result, nullptr);
    EXPECT_STREQ(result->markerName, name);
    EXPECT_STREQ(result->markerValue, value);
    EXPECT_STREQ(result->regexParameter, regex);
    EXPECT_TRUE(result->trimParameter);
    
    // Cleanup
    free(result->markerName);
    free(result->markerValue);
    free(result->regexParameter);
    free(result);
}

// Test strnstr optimizations
TEST(StrnStr, PatternMatchOptimization)
{
    // Test short pattern
    const char* haystack = "This is a test string for pattern matching";
    const char* shortNeedle = "test";
    const char* result = strnstr(haystack, shortNeedle, strlen(haystack));
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result - haystack, 10);  // "test" starts at index 10
    
    // Test long pattern (8+ chars)
    const char* longNeedle = "pattern matching";
    result = strnstr(haystack, longNeedle, strlen(haystack));
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result - haystack, 23);  // "pattern matching" starts at index 23
    
    // Test pattern not found
    const char* notFound = "nonexistent";
    result = strnstr(haystack, notFound, strlen(haystack));
    EXPECT_EQ(result, nullptr);
}

// Test chunked memory mapping
TEST_F(dcaTestFixture, ChunkedMemoryMapping)
{
    const size_t CHUNK_SIZE = 1024 * 1024; // 1MB
    const char* testFile = "test.log";
    const char* logPath = "/tmp";
    
    // Create a test file larger than CHUNK_SIZE
    FILE* fp = fopen("/tmp/test.log", "w");
    ASSERT_NE(fp, nullptr);
    for(int i = 0; i < CHUNK_SIZE + 1000; i++) {
        fputc('a', fp);
    }
    fclose(fp);
    
    int fd = open("/tmp/test.log", O_RDONLY);
    ASSERT_NE(fd, -1);
    
    FileDescriptor* desc = getFileDeltaInMemMapAndSearch(fd, 0, logPath, testFile, false);
    EXPECT_NE(desc, nullptr);
    if(desc) {
        EXPECT_NE(desc->baseAddr, nullptr);
        EXPECT_EQ(desc->rf_file_size, 0);  // No rotated file
        EXPECT_LE(desc->cf_file_size, CHUNK_SIZE + (PAGESIZE - 1));
        
        freeFileDescriptor(desc);
    }
    
    unlink("/tmp/test.log");
}

TEST(CREATEGREPSEEKPROFILE, SEEKMAPCREATE_CHECK)
{
    GrepSeekProfile *gsProfile = createGrepSeekProfile(0);
    EXPECT_NE(gsProfile, nullptr);
    EXPECT_NE(gsProfile->logFileSeekMap, nullptr);
    EXPECT_EQ(gsProfile->execCounter, 0);
    hash_map_destroy(gsProfile->logFileSeekMap, free);
    free(gsProfile);
}

TEST(FREEFREPSEEKPROFILE, SEEKMAPFREE_CHECK)
{
    GrepSeekProfile *gsProfile = createGrepSeekProfile(0);
    EXPECT_NE(gsProfile, nullptr);
    EXPECT_NE(gsProfile->logFileSeekMap, nullptr);
    EXPECT_EQ(gsProfile->execCounter, 0);
        
    freeGrepSeekProfile(gsProfile);
        
    // Check if the profile is freed correctly
    EXPECT_NE(gsProfile, nullptr);
}


//dca.c
TEST(PROCESSTOPPATTERN, VECTOR_NULL)
{
    Vector* topMarkerlist = NULL;
    Vector_Create(&topMarkerlist);
    Vector_PushBack(topMarkerlist, (void*) strdup("cpu_telemetry2_0"));
    Vector_PushBack(topMarkerlist, (void*) strdup("mem_telemetry2_0"));
    Vector* outgrepResultlist = NULL;
    Vector_Create(&outgrepResultlist);
    EXPECT_EQ(-1, processTopPattern("RDK_Profile", topMarkerlist, NULL, 1));
    EXPECT_EQ(-1, processTopPattern(NULL, topMarkerlist, outgrepResultlist, 1));
    EXPECT_EQ(-1, processTopPattern("RDK_Profile",NULL, outgrepResultlist, 1));
    Vector_Destroy(topMarkerlist, free);
    Vector_Destroy(outgrepResultlist, free);
    topMarkerlist = NULL;
    outgrepResultlist = NULL;
}

//int getDCAResultsInVector(GrepSeekProfile *gSeekProfile, Vector * vecMarkerList, Vector** out_grepResultList, bool check_rotated, char* customLogPath)
TEST(getDCAResultsInVector, markerlist_NULL)
{
    
    Vector* out_grepResultList = NULL;
    Vector_Create(&out_grepResultList);
    Vector* markerlist = NULL;
    Vector_Create(&markerlist);
    Vector_PushBack(markerlist, (void*) strdup("SYS_INFO_BOOTUP"));
    GrepSeekProfile *gsProfile = (GrepSeekProfile *)malloc(sizeof(GrepSeekProfile));
    gsProfile->logFileSeekMap = hash_map_create();
    gsProfile->execCounter = 0;
    hash_map_put(gsProfile->logFileSeekMap, strdup("t2_log.txt"), (void*)1, free);
    EXPECT_EQ(-1, getDCAResultsInVector(NULL, markerlist, &out_grepResultList, true, "/opt/logs/core_log.txt"));
    EXPECT_EQ(-1, getDCAResultsInVector(gsProfile, NULL, &out_grepResultList, true, "/opt/logs/core_log.txt"));
    EXPECT_EQ(-1, getDCAResultsInVector(gsProfile, markerlist, NULL, true, "/opt/logs/core_log.txt"));
    hash_map_destroy(gsProfile->logFileSeekMap, free);
    Vector_Destroy(markerlist, free);
    Vector_Destroy(out_grepResultList, free);
}


class dcaTestFixture : public ::testing::Test {
protected:
    void SetUp() override
    {
        g_fileIOMock = new FileMock();
	g_systemMock = new SystemMock();
    }

    void TearDown() override
    {
       delete g_fileIOMock;
       delete g_systemMock;

        g_fileIOMock = nullptr;
	g_systemMock = nullptr;
    }                                                                                                       
}; 

//dcautil.c
// Test memory mapping with various file sizes
TEST_F(dcaTestFixture, MemoryMappingWithDifferentSizes) {
    const char* testFile = "test.log";
    const char* logPath = "/tmp";
    const size_t sizes[] = {
        1024,                // 1KB - small file
        1024 * 1024,        // 1MB - chunk size
        1024 * 1024 + 100,  // Just over chunk size
        1024 * 1024 * 2     // 2MB - multiple chunks
    };

    for(size_t i = 0; i < sizeof(sizes)/sizeof(sizes[0]); i++) {
        // Create test file
        FILE* fp = fopen("/tmp/test.log", "w");
        ASSERT_NE(fp, nullptr);
        for(size_t j = 0; j < sizes[i]; j++) {
            fputc('a', fp);
        }
        fclose(fp);

        int fd = open("/tmp/test.log", O_RDONLY);
        ASSERT_NE(fd, -1);

        FileDescriptor* desc = getFileDeltaInMemMapAndSearch(fd, 0, logPath, testFile, false);
        EXPECT_NE(desc, nullptr);
        if(desc) {
            EXPECT_NE(desc->baseAddr, nullptr);
            EXPECT_EQ(desc->rf_file_size, 0);
            if(sizes[i] > 1024 * 1024) {
                EXPECT_LE(desc->cf_file_size, 1024 * 1024 + (PAGESIZE - 1));
            } else {
                EXPECT_EQ(desc->cf_file_size, sizes[i]);
            }
            freeFileDescriptor(desc);
        }
        unlink("/tmp/test.log");
    }
}

// Test pattern matching with various pattern lengths
TEST_F(dcaTestFixture, PatternMatchingOptimization) {
    // Create test data with known patterns
    const char* data = "This is a test string with multiple patterns: "
                      "short pat and longerpattern and verylongpatternhere";
    const size_t data_len = strlen(data);

    // Test cases with different pattern lengths
    struct {
        const char* pattern;
        bool should_find;
        size_t expected_pos;
    } test_cases[] = {
        {"short", true, 38},              // Short pattern
        {"longerpattern", true, 48},      // Medium pattern (>= 8 chars)
        {"verylongpatternhere", true, 63},// Long pattern
        {"nonexistent", false, 0},        // Not found
        {"", false, 0},                   // Empty pattern
        {"shortpat", false, 0}            // Almost matching
    };

    for(size_t i = 0; i < sizeof(test_cases)/sizeof(test_cases[0]); i++) {
        const char* result = strnstr(data, test_cases[i].pattern, data_len);
        if(test_cases[i].should_find) {
            EXPECT_NE(result, nullptr) << "Pattern: " << test_cases[i].pattern;
            if(result) {
                EXPECT_EQ(result - data, test_cases[i].expected_pos)
                    << "Pattern: " << test_cases[i].pattern;
            }
        } else {
            EXPECT_EQ(result, nullptr) << "Pattern: " << test_cases[i].pattern;
        }
    }
}

// Test rotated file handling
TEST_F(dcaTestFixture, RotatedFileHandling) {
    const char* testFile = "test.log.0";
    const char* logPath = "/tmp";
    
    // Create main and rotated test files
    FILE* fp = fopen("/tmp/test.log.0", "w");
    ASSERT_NE(fp, nullptr);
    fprintf(fp, "current log content\n");
    fclose(fp);
    
    fp = fopen("/tmp/test.log.1", "w");
    ASSERT_NE(fp, nullptr);
    fprintf(fp, "rotated log content\n");
    fclose(fp);

    int fd = open("/tmp/test.log.0", O_RDONLY);
    ASSERT_NE(fd, -1);

    FileDescriptor* desc = getFileDeltaInMemMapAndSearch(fd, 0, logPath, testFile, true);
    EXPECT_NE(desc, nullptr);
    if(desc) {
        EXPECT_NE(desc->baseAddr, nullptr);
        EXPECT_NE(desc->rotatedAddr, nullptr);
        EXPECT_GT(desc->rf_file_size, 0);
        
        // Verify we can read from both current and rotated files
        EXPECT_NE(strnstr((const char*)desc->cfaddr, "current", desc->cf_file_size), nullptr);
        EXPECT_NE(strnstr((const char*)desc->rfaddr, "rotated", desc->rf_file_size), nullptr);
        
        freeFileDescriptor(desc);
    }
    
    unlink("/tmp/test.log.0");
    unlink("/tmp/test.log.1");
}

TEST_F(dcaTestFixture, firstBootstatus){
    EXPECT_CALL(*g_systemMock, access(_,_))
            .Times(1)
            .WillOnce(Return(-1));

    firstBootStatus();
}


TEST_F(dcaTestFixture, dcaFlagReportCompleation)
{
    FILE* fp = (FILE*)NULL;
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    dcaFlagReportCompleation();
}

TEST_F(dcaTestFixture, dcaFlagReportCompleation1)
{
    FILE* fp = (FILE*)0xffffffff;
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    EXPECT_CALL(*g_fileIOMock, fclose(_))
            .Times(1)
            .WillOnce(Return(0));
    dcaFlagReportCompleation();
}

#ifdef PERSIST_LOG_MON_REF
TEST_F(dcaTestFixture, loadSavedSeekConfig)
{
    GrepSeekProfile *gsProfile = (GrepSeekProfile *)malloc(sizeof(GrepSeekProfile));
    gsProfile->logFileSeekMap = hash_map_create();
    gsProfile->execCounter = 0;
    FILE* fp = (FILE*)NULL;
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    EXPECT_EQ(T2ERROR_FAILURE, loadSavedSeekConfig("RDK_Profile", gsProfile));
    hash_map_destroy(gsProfile->logFileSeekMap, free);
    free(gsProfile);
}

TEST_F(dcaTestFixture, loadSavedSeekConfig1)
{
    GrepSeekProfile *gsProfile = (GrepSeekProfile *)malloc(sizeof(GrepSeekProfile));
    gsProfile->logFileSeekMap = hash_map_create();
    gsProfile->execCounter = 0;
    FILE* fp = (FILE*)0xffffffff;
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    EXPECT_CALL(*g_fileIOMock, fseek(_,_,_))
            .Times(2)
            .WillOnce(Return(0))
            .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, ftell(_))
            .Times(1)
            .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, fread(_,_,_,_))
            .Times(1)
            .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, fclose(_))
            .Times(1)
            .WillOnce(Return(0));
    
    EXPECT_EQ(T2ERROR_SUCCESS, loadSavedSeekConfig("RDK_Profile", gsProfile));
    hash_map_destroy(gsProfile->logFileSeekMap,free);
    free(gsProfile);
}

TEST_F(dcaTestFixture, saveSeekConfigtoFile)
{
    GrepSeekProfile *gsProfile = (GrepSeekProfile *)malloc(sizeof(GrepSeekProfile));
    gsProfile->logFileSeekMap = hash_map_create();
    gsProfile->execCounter = 0;
    long *tempnum;
    double val = 123456;
    tempnum = (long *)malloc(sizeof(long));
    *tempnum = (long)val;
    hash_map_put(gsProfile->logFileSeekMap, strdup("t2_log.txt"), (void*)tempnum, free);
    FILE* fp = (FILE*)NULL;
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    EXPECT_EQ(T2ERROR_FAILURE, saveSeekConfigtoFile("RDK_Profile", gsProfile));
    hash_map_destroy(gsProfile->logFileSeekMap, free);
    gsProfile->logFileSeekMap = NULL;
    free(gsProfile);
}
#endif

//dcaproc.c
TEST_F(dcaTestFixture, getProcUsage)
{
    Vector* grepResultList = NULL;
    char* filename = NULL;
    filename = strdup("top_log.txt");
    Vector_Create(&grepResultList);
    Vector_PushBack(grepResultList, (void*) strdup("SYS_INFO_BOOTUP"));
    Vector_PushBack(grepResultList, (void*) strdup("SYS_INFO_MEM"));
    FILE* fp = (FILE*)NULL;
    #ifdef LIBSYSWRAPPER_BUILD
    EXPECT_CALL(*g_fileIOMock, v_secure_popen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    #else
    EXPECT_CALL(*g_fileIOMock, popen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    #endif
    EXPECT_EQ(0, getProcUsage("telemetry2_0", grepResultList, true, "[0-9]", filename));
    Vector_Destroy(grepResultList, free);
    free(filename);
}   

TEST_F(dcaTestFixture, getProcUsage1)
{
    Vector* grepResultList = NULL;
    Vector_Create(&grepResultList);
    Vector_PushBack(grepResultList, (void*) strdup("SYS_INFO_BOOTUP"));
    Vector_PushBack(grepResultList, (void*) strdup("SYS_INFO_MEM"));
    FILE* fp = (FILE*)0xffffffff;
    char* filename = NULL;
    filename = strdup("top_log.txt");
    #ifdef LIBSYSWRAPPER_BUILD
    EXPECT_CALL(*g_fileIOMock, v_secure_popen(_,_))
            .Times(1)
            .WillOnce(Return(fp))
    #else
    EXPECT_CALL(*g_fileIOMock, popen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    #endif
    EXPECT_CALL(*g_fileIOMock, fscanf(_,_,_))
           .Times(1)
            .WillOnce(Return(0));
    #ifdef LIBSYSWRAPPER_BUILD
    EXPECT_CALL(*g_fileIOMock, v_secure_pclose(_))
            .Times(1)
            .WillOnce(Return(-1));
    #else
    EXPECT_CALL(*g_fileIOMock, pclose(_))
            .Times(1)
            .WillOnce(Return(-1));
    #endif
    EXPECT_EQ(0, getProcUsage("telemetry2_0", grepResultList, true, "[0-9]", filename));
    Vector_Destroy(grepResultList, free);
    free(filename);
}

TEST_F(dcaTestFixture, getProcUsage2)
{
    Vector* grepResultList = NULL;
    Vector_Create(&grepResultList);
    Vector_PushBack(grepResultList, (void*) strdup("SYS_INFO_BOOTUP"));
    Vector_PushBack(grepResultList, (void*) strdup("SYS_INFO_MEM"));
    FILE* fp = (FILE*)0xffffffff;
    char* filename = NULL;
    filename = strdup("top_log.txt");
    #ifdef LIBSYSWRAPPER_BUILD
    EXPECT_CALL(*g_fileIOMock, v_secure_popen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    #else
    EXPECT_CALL(*g_fileIOMock, popen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    #endif

    EXPECT_CALL(*g_fileIOMock, fscanf(_,_,_))
            .Times(2)
            .WillOnce(Return(1))
	    .WillOnce(Return(0));
	    
    #ifdef LIBSYSWRAPPER_BUILD
    EXPECT_CALL(*g_fileIOMock, v_secure_pclose(_))
            .Times(1)
	    .WillOnce(Return(-1));
    #else
    EXPECT_CALL(*g_fileIOMock, pclose(_))
            .Times(1)
	    .WillOnce(Return(-1));
    #endif
    EXPECT_EQ(0, getProcUsage("telemetry2_0", grepResultList, true, "[0-9]",filename));
    Vector_Destroy(grepResultList, free);
    free(filename);
}

TEST_F(dcaTestFixture, getProcPidStat)
{
    procinfo pinfo;
    int fd = (int)0xffffffff;
    EXPECT_CALL(*g_fileIOMock, open(_,_))
            .Times(1)
            .WillOnce(Return(-1));
    ASSERT_EQ(0, getProcPidStat(123, &pinfo));
}

TEST_F(dcaTestFixture, getProcPidStat1)
{
    procinfo pinfo;
    int fd = (int)0xffffffff;
    EXPECT_CALL(*g_fileIOMock, open(_,_))
            .Times(1)
            .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, read(_,_,_))
            .Times(1)
            .WillOnce(Return(-1));
    EXPECT_CALL(*g_fileIOMock, close(_))
            .Times(1)
            .WillOnce(Return(0));
    ASSERT_EQ(0, getProcPidStat(123, &pinfo));
}

#if !defined(ENABLE_RDKC_SUPPORT) && !defined(ENABLE_RDKB_SUPPORT)
TEST_F(dcaTestFixture, saveTopOutput)
{
    EXPECT_CALL(*g_systemMock, access(_,_))
                .Times(1)
                .WillOnce(Return(0));
   #ifdef LIBSYSWRAPPER_BUILD
       EXPECT_CALL(*g_systemMock, v_secure_system(_))
                .Times(3)
                .WillOnce(Return(0))
                .WillOnce(Return(-1))
                .WillOnce(Return(-1));
    #else
       EXPECT_CALL(*g_systemMock, system(_))
                .Times(3)
                .WillOnce(Return(0))
                .WillOnce(Return(-1))
                .WillOnce(Return(-1));
    #endif
    EXPECT_EQ(NULL, saveTopOutput("RDK_Profile")); 
}

TEST_F(dcaTestFixture, saveTopOutput1)
{
    EXPECT_CALL(*g_systemMock, access(_,_))
                .Times(1)
                .WillOnce(Return(-1));
    #ifdef LIBSYSWRAPPER_BUILD
       EXPECT_CALL(*g_systemMock, v_secure_system(_))
                .Times(2)
                .WillOnce(Return(-1))
                .WillOnce(Return(-1));
    #else
       EXPECT_CALL(*g_systemMock, system(_))
                .Times(2)
                .WillOnce(Return(-1))
                .WillOnce(Return(-1));
    #endif
   EXPECT_EQ(NULL, saveTopOutput("RDK_Profile"));
}

TEST_F(dcaTestFixture, saveTopOutput2)
{
    char* filename = NULL;
    filename = "/tmp/t2toplog_RDK_Profile";
    EXPECT_CALL(*g_systemMock, access(_,_))
                .Times(1)
                .WillOnce(Return(-1));
    #ifdef LIBSYSWRAPPER_BUILD
       EXPECT_CALL(*g_systemMock, v_secure_system(_))
                .Times(2)
                .WillOnce(Return(0))
                .WillOnce(Return(0));
    #else
       EXPECT_CALL(*g_systemMock, system(_))
                .Times(2)
                .WillOnce(Return(0))
                .WillOnce(Return(0));
    #endif
    EXPECT_STREQ(filename, saveTopOutput("RDK_Profile"));
}

TEST_F(dcaTestFixture, removeTopOutput)
{
   char* filename = NULL;
   filename = strdup("/tmp/t2toplog_RDK_Profile");
    #ifdef LIBSYSWRAPPER_BUILD
    EXPECT_CALL(*g_systemMock, v_secure_system(_))
                .Times(1)
                .WillOnce(Return(-1));
    #else
    EXPECT_CALL(*g_systemMock, system(_))
                .Times(1)
                .WillOnce(Return(-1));
    #endif
    removeTopOutput(filename);
}

TEST_F(dcaTestFixture, removeTopOutput1)
{
    char* filename = NULL;
    filename = strdup("/tmp/t2toplog_RDK_Profile");
    #ifdef LIBSYSWRAPPER_BUILD
    EXPECT_CALL(*g_systemMock, v_secure_system(_))
                .Times(1)
                .WillOnce(Return(0));
    #else
    EXPECT_CALL(*g_systemMock, system(_))
                .Times(1)
                .WillOnce(Return(0));
    #endif
    removeTopOutput(filename);
}

#else
TEST_F(dcaTestFixture, getTotalCpuTimes)
{
    FILE* mockfp = (FILE *)NULL;
    int *totaltime = 0;
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(mockfp));
    EXPECT_EQ(0, getTotalCpuTimes(totaltime));
}

TEST_F(dcaTestFixture, getTotalCpuTimes1)
{
    FILE* mockfp = (FILE *)0xFFFFFFFF;
    int *totaltime = 0;
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(mockfp));
    EXPECT_CALL(*g_fileIOMock, fscanf(_,_,_))
	    .Times(1)
	    .WillOnce(Return(-1));
    EXPECT_CALL(*g_fileIOMock, fclose(_))
            .Times(1)
	    .WillOnce(Return(0));
    EXPECT_EQ(1, getTotalCpuTimes(totaltime));
}
#endif
/*
TEST_F(dcaTestFixture,  getProcUsage4)
{
    Vector* gresulist = NULL;
    Vector_Create(&gresulist);
    char* processName = "telemetry2_0";
    FILE* fp = (FILE*)NULL;
    EXPECT_CALL(*g_fileIOMock, popen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    EXPECT_EQ(0, getProcUsage(processName, gresulist, true, "[0-9]+"));
}

TEST_F(dcaTestFixture,  getProcUsage3)
{
    Vector* grepResultList = NULL;
    Vector_Create(&grepResultList);
    Vector_PushBack(grepResultList, (void*) strdup("SYS_INFO_BOOTUP"));
    Vector_PushBack(grepResultList, (void*) strdup("SYS_INFO_MEM"));
    FILE* fp = (FILE*)0xffffffff;
    FILE* fv = (FILE*)NULL;
    #ifdef LIBSYSWRAPPER_BUILD
    EXPECT_CALL(*g_fileIOMock, v_secure_popen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    #else
    EXPECT_CALL(*g_fileIOMock, popen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    #endif

    EXPECT_CALL(*g_fileIOMock, fscanf(_,_,_))
            .Times(1)
            .WillOnce(Return(0));
    #ifdef LIBSYSWRAPPER_BUILD
    EXPECT_CALL(*g_fileIOMock, v_secure_pclose(_))
            .Times(1)
            .WillOnce(Return(0));
    #else
    EXPECT_CALL(*g_fileIOMock, pclose(_))
            .Times(1)
            .WillOnce(Return(0));
    #endif 
   EXPECT_EQ(0, getProcUsage("telemetry2_0", grepResultList, true, "[0-9]+"));
}

TEST_F(dcaTestFixture,  getProcUsage5)
{
   
	Vector* grepResultList = NULL;
    Vector_Create(&grepResultList);
    Vector_PushBack(grepResultList, (void*) strdup("SYS_INFO_BOOTUP"));
    Vector_PushBack(grepResultList, (void*) strdup("SYS_INFO_MEM"));
    FILE* fp = (FILE*)0xffffffff;
    FILE* fv = (FILE*)NULL;
    #ifdef LIBSYSWRAPPER_BUILD
    EXPECT_CALL(*g_fileIOMock, v_secure_popen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    #else
    EXPECT_CALL(*g_fileIOMock, popen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    #endif

    EXPECT_CALL(*g_fileIOMock, fscanf(_,_,_))
            .Times(1)
            .WillOnce(Return(0));
    #ifdef LIBSYSWRAPPER_BUILD
    EXPECT_CALL(*g_fileIOMock, v_secure_pclose(_))
            .Times(1)
            .WillOnce(Return(0));
    #else
    EXPECT_CALL(*g_fileIOMock, pclose(_))
            .Times(1)
            .WillOnce(Return(0));
    #endif 
   EXPECT_EQ(0, getProcUsage("telemetry2_0", grepResultList, true, "[0-9]+"));
}
*/
//legacyutils.c

TEST_F(dcaTestFixture, getLoadAvg)
{
    Vector* grepResultList;
    Vector_Create(&grepResultList);
    Vector_PushBack(grepResultList, (void*) strdup("SYS_INFO_BOOTUP"));
    Vector_PushBack(grepResultList, (void*) strdup("SYS_INFO_MEM"));
    FILE* fp = (FILE*)NULL;
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    EXPECT_EQ(0, getLoadAvg(grepResultList, false, NULL));
    Vector_Destroy(grepResultList, free);
}

TEST_F(dcaTestFixture, getLoadAvg1)
{
    Vector* grepResultList;
    Vector_Create(&grepResultList);
    GrepResult* loadAvg = (GrepResult*) malloc(sizeof(GrepResult));
    loadAvg->markerName = strdup("Load_Average");
    loadAvg->markerValue = strdup("2.15");
    loadAvg->trimParameter = true;
    loadAvg->regexParameter = "[0-9]+";
    Vector_PushBack(grepResultList, loadAvg);
    FILE* fp = (FILE*)0xffffffff;
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    EXPECT_CALL(*g_fileIOMock, fread(_,_,_,_))
            .Times(1)
            .WillOnce(Return(1));
    EXPECT_CALL(*g_fileIOMock, fclose(_))
	    .Times(1)
	    .WillOnce(Return(0));
    EXPECT_EQ(0, getLoadAvg(grepResultList, false, NULL));
    Vector_Destroy(grepResultList, freeGResult);
}

// Test memory management in GrepResult creation
TEST_F(dcaTestFixture, GrepResultMemoryManagement) {
    const char* testCases[][4] = {
        // markerName, markerValue, regex, should_succeed
        {"test1", "value1", "[0-9]+", "true"},
        {nullptr, "value2", nullptr, "false"},
        {"test3", nullptr, "[0-9]+", "false"},
        {"test4", "value4", nullptr, "true"},
        {"", "value5", "[0-9]+", "false"}
    };

    for(size_t i = 0; i < sizeof(testCases)/sizeof(testCases[0]); i++) {
        const bool should_succeed = strcmp(testCases[i][3], "true") == 0;
        GrepResult* result = createGrepResultObj(
            testCases[i][0], 
            testCases[i][1], 
            true, 
            (char*)testCases[i][2]
        );

        if(should_succeed) {
            EXPECT_NE(result, nullptr);
            if(result) {
                EXPECT_STREQ(result->markerName, testCases[i][0]);
                EXPECT_STREQ(result->markerValue, testCases[i][1]);
                if(testCases[i][2]) {
                    EXPECT_STREQ(result->regexParameter, testCases[i][2]);
                } else {
                    EXPECT_EQ(result->regexParameter, nullptr);
                }
                // Cleanup
                free(result->markerName);
                free(result->markerValue);
                free(result->regexParameter);
                free(result);
            }
        } else {
            EXPECT_EQ(result, nullptr);
        }
    }
}

// Test count pattern matching with different pattern sizes
TEST_F(dcaTestFixture, CountPatternMatching) {
    const char* testData = "This is a test with multiple patterns.\n"
                          "Pattern1 appears here and Pattern1 appears there.\n"
                          "ShortPat is here and ShortPat is there.\n"
                          "LongerPattern12345 shows up once.\n";
    
    const size_t dataLen = strlen(testData);
    
    // Create a temporary file with test data
    FILE* fp = fopen("/tmp/pattern_test.log", "w");
    ASSERT_NE(fp, nullptr);
    fwrite(testData, 1, dataLen, fp);
    fclose(fp);
    
    int fd = open("/tmp/pattern_test.log", O_RDONLY);
    ASSERT_NE(fd, -1);
    
    FileDescriptor* desc = (FileDescriptor*)malloc(sizeof(FileDescriptor));
    ASSERT_NE(desc, nullptr);
    memset(desc, 0, sizeof(FileDescriptor));
    
    desc->cfaddr = (char*)mmap(NULL, dataLen, PROT_READ, MAP_PRIVATE, fd, 0);
    ASSERT_NE(desc->cfaddr, MAP_FAILED);
    desc->cf_file_size = dataLen;
    
    // Test different pattern lengths
    struct {
        const char* pattern;
        int expected_count;
    } patterns[] = {
        {"Pattern1", 2},           // Appears twice
        {"ShortPat", 2},          // Short pattern appears twice
        {"LongerPattern12345", 1}, // Long pattern appears once
        {"NonExistent", 0},        // Doesn't appear
        {"test", 1}               // Appears once
    };
    
    for(size_t i = 0; i < sizeof(patterns)/sizeof(patterns[0]); i++) {
        int count = getCountPatternMatch(desc, patterns[i].pattern);
        EXPECT_EQ(count, patterns[i].expected_count) 
            << "Pattern: " << patterns[i].pattern;
    }
    
    munmap(desc->cfaddr, dataLen);
    close(fd);
    free(desc);
    unlink("/tmp/pattern_test.log");
}

TEST_F(dcaTestFixture, getLoadAvg2)
{
    Vector* grepResultList;
    Vector_Create(&grepResultList);
    GrepResult* loadAvg = (GrepResult*) malloc(sizeof(GrepResult));
    loadAvg->markerName = strdup("Load_Average");
    loadAvg->markerValue = strdup("2.15");
    loadAvg->trimParameter = true;
    loadAvg->regexParameter = "[0-9]+";
    Vector_PushBack(grepResultList, loadAvg);
    FILE* fp = (FILE*)0xffffffff;
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    EXPECT_CALL(*g_fileIOMock, fread(_,_,_,_))
            .Times(1)
            .WillOnce(Return(14));
    EXPECT_CALL(*g_fileIOMock, fclose(_))
            .Times(1)
            .WillOnce(Return(0));
    EXPECT_EQ(1, getLoadAvg(grepResultList, true, "[0-9]+"));
    Vector_Destroy(grepResultList, freeGResult);
}

// Test error handling in file operations
TEST_F(dcaTestFixture, FileOperationErrorHandling) {
    const char* testFile = "nonexistent.log";
    const char* logPath = "/nonexistent";
    
    // Test non-existent file
    int fd = open("/nonexistent/test.log", O_RDONLY);
    FileDescriptor* desc = getFileDeltaInMemMapAndSearch(fd, 0, logPath, testFile, false);
    EXPECT_EQ(desc, nullptr);
    
    // Test with invalid seek value
    fd = open("/tmp/test.log", O_CREAT | O_RDWR, 0644);
    ASSERT_NE(fd, -1);
    write(fd, "test", 4);
    close(fd);
    
    fd = open("/tmp/test.log", O_RDONLY);
    desc = getFileDeltaInMemMapAndSearch(fd, 1000000, logPath, testFile, false);
    EXPECT_EQ(desc, nullptr);
    
    // Test with zero-length file
    fd = open("/tmp/empty.log", O_CREAT | O_RDWR, 0644);
    ASSERT_NE(fd, -1);
    close(fd);
    
    fd = open("/tmp/empty.log", O_RDONLY);
    desc = getFileDeltaInMemMapAndSearch(fd, 0, logPath, testFile, false);
    EXPECT_EQ(desc, nullptr);
    
    unlink("/tmp/test.log");
    unlink("/tmp/empty.log");
}

// Test boundary conditions in pattern matching
TEST_F(dcaTestFixture, PatternMatchingBoundary) {
    // Test data at different buffer boundaries
    const char* patterns[] = {
        "pattern",           // Normal case
        "patternAtEnd",      // Pattern at end
        "StartPattern",      // Pattern at start
        "pat\ntern",         // Pattern across newline
        "pattern\0hidden"    // Pattern with null byte
    };
    
    for(size_t i = 0; i < sizeof(patterns)/sizeof(patterns[0]); i++) {
        // Create test data with pattern at different positions
        char* testData = (char*)malloc(1024);
        ASSERT_NE(testData, nullptr);
        
        // Fill with dummy data
        memset(testData, 'x', 1023);
        testData[1023] = '\0';
        
        // Insert pattern at start
        strcpy(testData, patterns[i]);
        
        // Insert pattern in middle
        strcpy(testData + 500, patterns[i]);
        
        // Insert pattern at end
        strcpy(testData + 1023 - strlen(patterns[i]), patterns[i]);
        
        // Test pattern matching at each position
        const char* result = strnstr(testData, patterns[i], 1023);
        EXPECT_NE(result, nullptr);
        if(result) {
            EXPECT_EQ(result, testData);
        }
        
        result = strnstr(testData + 500, patterns[i], 523);
        EXPECT_NE(result, nullptr);
        if(result) {
            EXPECT_EQ(result, testData + 500);
        }
        
        result = strnstr(testData + 1023 - strlen(patterns[i]), 
                        patterns[i], 
                        strlen(patterns[i]));
        EXPECT_NE(result, nullptr);
        
        free(testData);
    }
}

TEST_F(dcaTestFixture, initProperties)
{
    char* logpath = NULL;
    long int pagesize = 0;
    char* perspath = NULL;
    FILE* fp = (FILE*)0xffffffff;
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(2)
            .WillOnce(Return(fp))
            .WillOnce(Return(fp));
    EXPECT_CALL(*g_fileIOMock, fscanf(_,_,_))
            .Times(10)
            .WillOnce(Return(0))
            .WillOnce(Return(0))
            .WillOnce(Return(0))
            .WillOnce(Return(0))
            .WillOnce(Return(0))
            .WillOnce(Return(EOF))
            .WillOnce(Return(0))
            .WillOnce(Return(0))
            .WillOnce(Return(0))
            .WillOnce(Return(EOF));
    EXPECT_CALL(*g_fileIOMock, fclose(_))
            .Times(2)
            .WillOnce(Return(0))
            .WillOnce(Return(0));
    initProperties(&logpath, &perspath, &pagesize);
}

//dca.c
TEST_F(dcaTestFixture, processTopPattern)
{
    Vector* topMarkerlist = NULL;
    Vector_Create(&topMarkerlist);
    GrepMarker* topMarker = (GrepMarker*) malloc(sizeof(GrepMarker));
    topMarker->markerName = strdup("cpu_telemetry2_0");
    topMarker->searchString = strdup("telemetry2_0");
    topMarker->trimParam = false;
    topMarker->regexParam = NULL;
    topMarker->logFile = strdup("top_log.txt");
    topMarker->skipFreq = 0;
  
    Vector_PushBack(topMarkerlist, (void*) topMarker);
    Vector* outgrepResultlist = NULL;
    Vector_Create(&outgrepResultlist);
    EXPECT_CALL(*g_systemMock, access(_,_))
                .Times(1)
                .WillOnce(Return(-1));
    #ifdef LIBSYSWRAPPER_BUILD
       EXPECT_CALL(*g_systemMock, v_secure_system(_))
                .Times(3)
                .WillOnce(Return(0))
                .WillOnce(Return(0))
                .WillOnce(Return(-1));
    #else
       EXPECT_CALL(*g_systemMock, system(_))
                .Times(3)
                .WillOnce(Return(0))
                .WillOnce(Return(0))
                .WillOnce(Return(-1));
    #endif
    FILE* fp = (FILE*)NULL;
    #ifdef LIBSYSWRAPPER_BUILD
    EXPECT_CALL(*g_fileIOMock, v_secure_popen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    #else
    EXPECT_CALL(*g_fileIOMock, popen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    #endif
    EXPECT_EQ(0, processTopPattern("RDK_Profile", topMarkerlist, outgrepResultlist, 1));
    Vector_Destroy(topMarkerlist, freeGMarker);
    Vector_Destroy(outgrepResultlist, free);
}

TEST_F(dcaTestFixture, processTopPattern1)
{
    Vector* topMarkerlist = NULL;
    Vector_Create(&topMarkerlist);
    GrepMarker* topMarker = (GrepMarker*) malloc(sizeof(GrepMarker));
    topMarker->markerName = strdup("cpu_telemetry2_0");
    topMarker->searchString = strdup("telemetry2_0");
    topMarker->trimParam = true;
    topMarker->regexParam = strdup("[0-9]+");
    topMarker->logFile = strdup("top_log.txt");
    topMarker->skipFreq = 0;
    GrepMarker* topMarker1 = (GrepMarker*) malloc(sizeof(GrepMarker));
    topMarker1->markerName = strdup("Load_Average");
    topMarker1->searchString = strdup("telemetry2_0");
    topMarker1->trimParam = true;
    topMarker1->regexParam = strdup("[0-9]+");
    topMarker1->logFile = strdup("top_log.txt");
    topMarker1->skipFreq = 0;
    Vector_PushBack(topMarkerlist, (void*) topMarker1);
    Vector_PushBack(topMarkerlist, (void*) topMarker);
    Vector* outgrepResultlist = NULL;
    Vector_Create(&outgrepResultlist);
    EXPECT_CALL(*g_systemMock, access(_,_))
                .Times(1)
                .WillOnce(Return(-1));
    #ifdef LIBSYSWRAPPER_BUILD
       EXPECT_CALL(*g_systemMock, v_secure_system(_))
                .Times(3)
                .WillOnce(Return(0))
                .WillOnce(Return(0))
                .WillOnce(Return(-1));
    #else
       EXPECT_CALL(*g_systemMock, system(_))
                .Times(3)
                .WillOnce(Return(0))
                .WillOnce(Return(0))
                .WillOnce(Return(-1));
    #endif
    FILE* fp = (FILE*)NULL;
    #ifdef LIBSYSWRAPPER_BUILD
    EXPECT_CALL(*g_fileIOMock, v_secure_popen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    #else
    EXPECT_CALL(*g_fileIOMock, popen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    #endif
    FILE* fs= (FILE*)0xffffffff;
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(fs));
    EXPECT_CALL(*g_fileIOMock, fread(_,_,_,_))
            .Times(1)
            .WillOnce(Return(14));
    EXPECT_CALL(*g_fileIOMock, fclose(_))
            .Times(1)
            .WillOnce(Return(0));

    EXPECT_EQ(0, processTopPattern("RDK_Profile", topMarkerlist, outgrepResultlist, 1));
    Vector_Destroy(topMarkerlist, freeGMarker);
    Vector_Destroy(outgrepResultlist, free);
}

TEST_F(dcaTestFixture, getDCAResultsInVector)
{
   
    Vector* out_grepResultList = NULL;
    Vector_Create(&out_grepResultList);
    GrepSeekProfile *gsProfile = (GrepSeekProfile *)malloc(sizeof(GrepSeekProfile));
    gsProfile->logFileSeekMap = hash_map_create();
    gsProfile->execCounter = 0;
    long *tempnum;
    double val = 1234;
    tempnum = (long *)malloc(sizeof(long));
    *tempnum = (long)val;
    hash_map_put(gsProfile->logFileSeekMap, strdup("t2_log.txt"), (void*)tempnum, free);
    Vector* vecMarkerList = NULL;
    Vector_Create(&vecMarkerList);
    GrepMarker* marker = (GrepMarker*) malloc(sizeof(GrepMarker));
    marker->markerName = strdup("SYS_INFO_TEST");
    marker->searchString = strdup("Test Marker");
    marker->trimParam = true;
    marker->regexParam = strdup("[0-9]+");
    marker->logFile = strdup("Consolelog.txt.0");
    marker->skipFreq = 0;
    Vector_PushBack(vecMarkerList, (void*) marker);
    EXPECT_CALL(*g_fileIOMock, open(_,_))
            .Times(1)
            .WillOnce(Return(-1));
    EXPECT_EQ(0, getDCAResultsInVector(gsProfile, vecMarkerList, &out_grepResultList, true, "/opt/logs"));
    hash_map_destroy(gsProfile->logFileSeekMap, free);
    gsProfile->logFileSeekMap = NULL;
    free(gsProfile);
    Vector_Destroy(vecMarkerList, freeGMarker);
    Vector_Destroy(out_grepResultList, free);
}