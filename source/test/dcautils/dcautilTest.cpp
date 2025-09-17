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

//testing the strnstr function
static const char *strnstr(const char *haystack, const char *needle, size_t len)
{
    if (haystack  == NULL || needle == NULL)
    {
        return NULL;
    }
    size_t needle_len = strlen(needle);
    if (needle_len == 0)
    {
        return haystack;
    }

    // Check if search is possible and prevent overflow
    if (len < needle_len || len - needle_len > len)
    {
        return NULL;
    }

    // Check minimum length requirements for optimized search
    if (needle_len < 4)
    {
        // Use simple search for short patterns
        for (size_t i = 0; i <= len - needle_len; i++)
        {
            if (memcmp(haystack + i, needle, needle_len) == 0)
            {
                return haystack + i;
            }
        }
        return NULL;
    }

    size_t skip[256];
    for (size_t i = 0; i < 256; ++i)
    {
        skip[i] = needle_len;
    }
    for (size_t i = 0; i < needle_len - 1; ++i)
    {
        skip[(unsigned char)needle[i]] = needle_len - i - 1;
    }

    size_t i = 0;
    while (i <= len - needle_len)
    {
        size_t j = needle_len - 1;
        while (j < needle_len && haystack[i + j] == needle[j])
        {
            j--;
        }
        if (j == (size_t) -1)
        {
            return haystack + i; // Match found
        }
        size_t s = skip[(unsigned char)haystack[i + needle_len - 1]];
        i += (s > 0) ? s : 1;
    }
    return NULL;
}

TEST(STRNSTR, SAMPLE1)
{
   char *haystack = "fdghfilikeflowershfjkh";
   char *needle = "ilikeflowers";
   EXPECT_STREQ(strnstr(haystack, needle, 22), "ilikeflowershfjkh");
}

TEST(STRNSTR, SAMPLE2)
{
   char *haystack = "fdghfilikehjowershfjkhilikeflowers";
   char *needle = "ilikeflowers";
   EXPECT_STREQ(strnstr(haystack, needle, 34), "ilikeflowers");
}

TEST(STRNSTR, SAMPLE3)
{
   char *haystack = "filikeflowershfjkhdghf";
   char *needle = "ilikeflowers";
   EXPECT_STREQ(strnstr(haystack, needle, 22), "ilikeflowershfjkhdghf");
}

TEST(STRNSTR, SAMPLE4)
{
   char *haystack = "abcabcabcde";
   char *needle = "abcd";
   EXPECT_STREQ(strnstr(haystack, needle, 11), "abcde");
}

TEST(STRNSTR, SAMPLE5)
{
   char *haystack = "abcabcabcabcabc";
   char *needle = "abcd";
   EXPECT_STREQ(strnstr(haystack, needle, 15), NULL);
}

TEST(STRNSTR, SAMPLE6)
{
   char *haystack = "abcdabcabcabcabc";
   char *needle = "abcd";
   EXPECT_STREQ(strnstr(haystack, needle, 15), "abcdabcabcabcabc");
}
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
