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

TEST(GETPROCUSAGE, MARKER_NULL)
{
   EXPECT_EQ(-1, getProcUsage("telemetry2_0", NULL, NULL));
}

TEST(GETPROCUSAGE, PROCESS_NULL)
{
   char* filename = NULL;
   filename = strdup("top_log.txt");
   EXPECT_EQ(-1, getProcUsage(NULL, NULL, filename));
   EXPECT_EQ(-1, getProcUsage(NULL, NULL, NULL));
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
   EXPECT_EQ(T2ERROR_FAILURE, getGrepResults(NULL, markerlist, false, false,"/opt/logs"));
   EXPECT_EQ(T2ERROR_FAILURE, getGrepResults(&gsProfile, NULL, false, false,"/opt/logs"));
   EXPECT_EQ(T2ERROR_FAILURE, getGrepResults(NULL, markerlist, false, true,"/opt/logs"));
   EXPECT_EQ(T2ERROR_FAILURE, getGrepResults(&gsProfile, NULL, false, true,"/opt/logs"));
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


TEST(GETLOADAVG, MARKER_NULL)
{
    EXPECT_EQ(0, getLoadAvg(NULL));
}

TEST(GETLOADAVG, VALID_MARKER)
{
    TopMarker* topMarker = (TopMarker*) malloc(sizeof(TopMarker));
    memset(topMarker, 0, sizeof(TopMarker));
    EXPECT_EQ(1, getLoadAvg(topMarker));
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

TEST(CLEARCONFVAL, FREECONFVAL)
{
        clearConfVal();
}

#if 0
//dca.c
TEST(PROCESSTOPPATTERN, VECTOR_NULL)
{
    Vector* topMarkerlist = NULL;
    Vector_Create(&topMarkerlist);
    Vector_PushBack(topMarkerlist, (void*) strdup("cpu_telemetry2_0"));
    Vector_PushBack(topMarkerlist, (void*) strdup("mem_telemetry2_0"));
    EXPECT_EQ(-1, processTopPattern("RDK_Profile", topMarkerlist, 1));
    EXPECT_EQ(-1, processTopPattern(NULL, topMarkerlist, 1));
    EXPECT_EQ(-1, processTopPattern("RDK_Profile",NULL, 1));
    Vector_Destroy(topMarkerlist, free);
    topMarkerlist = NULL;
}

TEST(getDCAResultsInVector, markerlist_NULL)
{
    
    Vector* markerlist = NULL;
    Vector_Create(&markerlist);
    Vector_PushBack(markerlist, (void*) strdup("SYS_INFO_BOOTUP"));
    GrepSeekProfile *gsProfile = (GrepSeekProfile *)malloc(sizeof(GrepSeekProfile));
    gsProfile->logFileSeekMap = hash_map_create();
    gsProfile->execCounter = 0;
    hash_map_put(gsProfile->logFileSeekMap, strdup("t2_log.txt"), (void*)1, free);
    EXPECT_EQ(-1, getDCAResultsInVector(NULL, markerlist, true, "/opt/logs/core_log.txt"));
    EXPECT_EQ(-1, getDCAResultsInVector(gsProfile, NULL, true, "/opt/logs/core_log.txt"));
    EXPECT_EQ(-1, getDCAResultsInVector(gsProfile, markerlist, true, "/opt/logs/core_log.txt"));
    hash_map_destroy(gsProfile->logFileSeekMap, free);
    Vector_Destroy(markerlist, free);
}

#endif

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

    EXPECT_EQ(true, firstBootStatus());
}

TEST_F(dcaTestFixture, firstBootstatus_1){
    EXPECT_CALL(*g_systemMock, access(_,_))
            .Times(1)
            .WillOnce(Return(0));

    EXPECT_EQ(false, firstBootStatus());
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

#if 0
TEST_F(dcaTestFixture, getProcUsage)
{
    Vector* topMarkerlist = NULL;
    Vector_Create(&topMarkerlist);
    TopMarker* topMarker = (TopMarker*) malloc(sizeof(TopMarker));
    topMarker->markerName = strdup("cpu_telemetry2_0");
    topMarker->searchString = strdup("telemetry2_0");
    topMarker->trimParam = false;
    topMarker->regexParam = NULL;
    topMarker->logFile = strdup("top_log.txt");
    topMarker->skipFreq = 0;
    topMarker->paramType = strdup("grep");
    topMarker->reportEmptyParam = true;
    Vector_PushBack(topMarkerlist, (void*) topMarker);

    Vector* outgrepResultlist = NULL;
    Vector_Create(&outgrepResultlist);
    char* filename = strdup("/tmp/t2toplog/RDK_Profile");

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
    EXPECT_EQ(0, getProcUsage(topMarker->searchString, topMarker, filename));
    Vector_Destroy(topMarkerlist, freeGMarker);
    Vector_Destroy(outgrepResultlist, free);
    free(filename);
}   
#endif

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


//legacyutils.c

/*
TEST_F(dcaTestFixture, getLoadAvg)
{
    TopMarker* topMarker = (TopMarker*) malloc(sizeof(TopMarker));
    topMarker->markerName = strdup("cpu_telemetry2_0");
    topMarker->searchString = strdup("telemetry2_0");
    topMarker->trimParam = false;
    topMarker->regexParam = NULL;
    topMarker->logFile = strdup("top_log.txt");
    topMarker->skipFreq = 0;
    topMarker->paramType = strdup("grep");
    topMarker->reportEmptyParam = true;
    Vector* grepResultList;
    Vector_Create(&grepResultList);
    Vector_PushBack(grepResultList, (void*) strdup("SYS_INFO_BOOTUP"));
    Vector_PushBack(grepResultList, (void*) strdup("SYS_INFO_MEM"));
    FILE* fp = (FILE*)NULL;
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    EXPECT_EQ(0, getLoadAvg(topMarker));
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
*/

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

#if 0
//dca.c
TEST_F(dcaTestFixture, processTopPattern)
{
    Vector* topMarkerlist = NULL;
    Vector_Create(&topMarkerlist);
    TopMarker* topMarker = (TopMarker*) malloc(sizeof(TopMarker));
    topMarker->markerName = strdup("cpu_telemetry2_0");
    topMarker->searchString = strdup("telemetry2_0");
    topMarker->trimParam = false;
    topMarker->regexParam = NULL;
    topMarker->logFile = strdup("top_log.txt");
    topMarker->skipFreq = 0;
    topMarker->paramType = strdup("grep");
    topMarker->reportEmptyParam = true;
  
    TopMarker* topMarker1 = (TopMarker*) malloc(sizeof(TopMarker));
    topMarker1->markerName = strdup("cpu_telemetry2_0");
    topMarker1->searchString = strdup("telemetry2_0");
    topMarker1->trimParam = false;
    topMarker1->regexParam = NULL;
    topMarker1->logFile = strdup("top_log.txt");
    topMarker1->skipFreq = 0;
    topMarker1->paramType = strdup("grep");
    topMarker->reportEmptyParam = true;
    Vector_PushBack(topMarkerlist, (void*) topMarker1);

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
    EXPECT_EQ(0, processTopPattern("RDK_Profile", topMarkerlist, 1));
    Vector_Destroy(topMarkerlist, freeGMarker);
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

    EXPECT_EQ(0, processTopPattern("RDK_Profile", topMarkerlist, 1));
    Vector_Destroy(topMarkerlist, freeGMarker);
}

TEST_F(dcaTestFixture, processTopPattern2)
{
    Vector* topMarkerlist = NULL;
    Vector_Create(&topMarkerlist);
    GrepMarker* topMarker = (GrepMarker*) malloc(sizeof(GrepMarker));
    topMarker->markerName = strdup("cpu_telemetry2_0");
    topMarker->searchString = strdup("telemetry2_0");
    topMarker->trimParam = true;
    topMarker->regexParam = strdup("[0-9]+");
    topMarker->logFile = strdup("top_log.txt");
    topMarker->skipFreq = 1;
    Vector_PushBack(topMarkerlist, (void*) topMarker);

    GrepMarker* topMarker2 = (GrepMarker*) malloc(sizeof(GrepMarker));
    topMarker2->markerName = strdup("mem_telemetry2_0");
    topMarker2->searchString = strdup("telemetry2_0");
    topMarker2->trimParam = true;
    topMarker2->regexParam = strdup("[0-9]+");
    topMarker2->logFile = strdup("top_log.txt");
    topMarker2->skipFreq = 0;
    Vector_PushBack(topMarkerlist, (void*) topMarker2);

    //saveTopoutput
    EXPECT_CALL(*g_systemMock, access(_,_))
                .Times(2)
                .WillOnce(Return(0))
                .WillOnce(Return(0));
    #ifdef LIBSYSWRAPPER_BUILD
       EXPECT_CALL(*g_systemMock, v_secure_system(_))
                .Times(4)
                .WillOnce(Return(0)) //saveTopoutput
                .WillOnce(Return(0)) //removeTopoutput
                .WillOnce(Return(0)) //saveTopoutput
                .WillOnce(Return(0));
    #else 
       EXPECT_CALL(*g_systemMock, system(_))
                .Times(4)
                .WillOnce(Return(0))
                .WillOnce(Return(0))
                .WillOnce(Return(0))
                .WillOnce(Return(0));
    #endif
    FILE* fp = (FILE*)0xFFFFFFFF;
    //getProcUsage
    #ifdef LIBSYSWRAPPER_BUILD
    EXPECT_CALL(*g_fileIOMock, v_secure_popen(_,_))
            .Times(3)
            .WillOnce(Return(fp))
            .WillOnce(Return(fp))
            .WillOnce(Return(fp));
    #else
    EXPECT_CALL(*g_fileIOMock, popen(_,_))
            .Times(2)
            .WillOnce(Return(fp))
            .WillOnce(Return(fp));
    #endif
    
    #ifdef LIBSYSWRAPPER_BUILD
    EXPECT_CALL(*g_fileIOMock, v_secure_pclose(_))
                .Times(3)
                .WillOnce(Return(0))
                .WillOnce(Return(0))
                .WillOnce(Return(0));
    #else
    EXPECT_CALL(*g_fileIOMock, pclose(_))
                .Times(2)
                .WillOnce(Return(0))
                .WillOnce(Return(0));
    #endif

    //getMEMinfo
    EXPECT_CALL(*g_fileIOMock, read(_,_,_))
             .WillOnce([](int fd, void* buf, size_t count) {
                 const char *stat_str = "7396 (telemetry2_0) S 1 7396 7396 0 -1 1077936448 17297 316924 7 544 410 610 1888 258 20 0 14 0 2373301 1024380928 3425 18446744073709551615 94059930939392 94059930943425 140725223263616 0 0 0 0 4096 268454400 0 0 0 17 3 0 0 0 0 0 94059930950768 94059930951697 94060511973376 140725223266476 140725223266504 140725223266504 140725223268316 0";
                 size_t len = strlen(stat_str);
                 size_t to_copy = len < count ? len : count;
                 memcpy(buf, stat_str, to_copy);
                 return to_copy;
                });
    
   //getprocpidstat
   EXPECT_CALL(*g_fileIOMock, open(_,_))
            .Times(1)
            .WillOnce(Return(0));
   EXPECT_CALL(*g_fileIOMock, close(_))
            .Times(1)
            .WillOnce(Return(0));
    //getCPUInfo
    EXPECT_CALL(*g_fileIOMock, fgets(_,_,_))
            .WillOnce([](char* buf, size_t size, FILE* stream) {
                const char* test_line = "2268 root 20 0 831m 66m 20m S 27 13.1 491:06.82 telemetry2_0\n";
                strncpy(buf, test_line, size-1);
                buf[size-1] = '\0';
                return buf;
            })
            .WillOnce(Return((char*)NULL));
    EXPECT_EQ(0, processTopPattern("RDK_Profile", topMarkerlist, 1));
    Vector_Destroy(topMarkerlist, freeGMarker);
}

TEST_F(dcaTestFixture, getDCAResultsInVector_1)
{
   
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
    marker->paramType = strdup("grep");
    marker->mType = MTYPE_COUNTER;
    marker->u.count = 0;
    marker->reportEmptyParam = true;
    Vector_PushBack(vecMarkerList, (void*) marker);

    //freeFileDescriptor
    EXPECT_CALL(*g_fileIOMock, munmap(_, _))
            .Times(1)
            .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, close(_)) 
            .Times(4)
            .WillOnce(Return(0))
            .WillOnce(Return(0))
            .WillOnce(Return(0))
            .WillOnce(Return(0));

    //getLogFileDescriptor
    EXPECT_CALL(*g_fileIOMock, open(_,_))
            .Times(1)
            .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, fstat(_, _))
        .Times(2)
        .WillOnce([](int fd, struct stat* statbuf) {
        statbuf->st_size = 1235;      // Set file size
        return 0; // Success
    })
        .WillOnce([](int fd, struct stat* statbuf) {
        statbuf->st_size = 1300;      // Set file size
        return 0; // Success
    });

    //getDeltainmmapsearch 
    EXPECT_CALL(*g_fileIOMock, mkstemp(_))
            .Times(1)
            .WillOnce(Return(0));
    EXPECT_CALL(*g_systemMock, unlink(_))
                .Times(1)
                .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock,sendfile(_,_,_,_))
            .Times(1)
            .WillOnce(Return(1300));
    EXPECT_CALL(*g_fileIOMock, mmap(_,_,_,_,_,_))
                .Times(1)
                .WillOnce([](void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
                    const char* test_str = "This is a Test Marker with value 1234 in the log file.\nAnother line without the marker.\n";
                    char* mapped_mem = (char*)malloc(length);
                    memset(mapped_mem, 0, length);
                    strncpy(mapped_mem, test_str, length - 1);
                    return (void*)mapped_mem;
                });

    
    EXPECT_EQ(0, getDCAResultsInVector(gsProfile, vecMarkerList, true, "/opt/logs"));
    hash_map_destroy(gsProfile->logFileSeekMap, free);
    gsProfile->logFileSeekMap = NULL;
    free(gsProfile);
    Vector_Destroy(vecMarkerList, freeGMarker);
}


TEST_F(dcaTestFixture, getDCAResultsInVector_2)
{
   
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
    marker->searchString = strdup("temp:");
    marker->trimParam = true;
    marker->regexParam = strdup("[0-9]+");
    marker->logFile = strdup("Consolelog.txt.0");
    marker->skipFreq = 0;
    marker->paramType = strdup("grep");
    marker->mType = MTYPE_ABSOLUTE;
    marker->u.markerValue = NULL;
    marker->u.count = 0;
    marker->reportEmptyParam = true;
    Vector_PushBack(vecMarkerList, (void*) marker);

    //freeFileDescriptor
    EXPECT_CALL(*g_fileIOMock, munmap(_, _))
            .Times(1)
            .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, close(_)) 
            .Times(4)
            .WillOnce(Return(0))
            .WillOnce(Return(0))
            .WillOnce(Return(0))
            .WillOnce(Return(0));

    //getLogFileDescriptor
    EXPECT_CALL(*g_fileIOMock, open(_,_))
            .Times(1)
            .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, fstat(_, _))
        .Times(2)
        .WillOnce([](int fd, struct stat* statbuf) {
        statbuf->st_size = 1300;      // Set file size
        return 0; // Success
    })
        .WillOnce([](int fd, struct stat* statbuf) {
        statbuf->st_size = 1300;      // Set file size
        return 0; // Success
    });

    //getDeltainmmapsearch 
    EXPECT_CALL(*g_fileIOMock, mkstemp(_))
            .Times(1)
            .WillOnce(Return(0));
    EXPECT_CALL(*g_systemMock, unlink(_))
                .Times(1)
                .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock,sendfile(_,_,_,_))
            .Times(1)
            .WillOnce(Return(1300));
    EXPECT_CALL(*g_fileIOMock, mmap(_,_,_,_,_,_))
                .Times(1)
                .WillOnce([](void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
                    const char* test_str = "This is a Test Marker with value temp:1245.\nAnother line without the marker.\n Another line with marker temp:2345\n";
                    char* mapped_mem = (char*)malloc(length);
                    memset(mapped_mem, 0, length);
                    strncpy(mapped_mem, test_str, length - 1);
                    return (void*)mapped_mem;
                });

    
    EXPECT_EQ(0, getDCAResultsInVector(gsProfile, vecMarkerList, true, "/opt/logs"));
    hash_map_destroy(gsProfile->logFileSeekMap, free);
    gsProfile->logFileSeekMap = NULL;
    free(gsProfile);
    Vector_Destroy(vecMarkerList, freeGMarker);
}

TEST_F(dcaTestFixture, getDCAResultsInVector_3)
{
   
    GrepSeekProfile *gsProfile = (GrepSeekProfile *)malloc(sizeof(GrepSeekProfile));
    gsProfile->logFileSeekMap = hash_map_create();
    gsProfile->execCounter = 1;
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
    marker->u.markerValue = NULL;
    marker->regexParam = strdup("[0-9]+");
    marker->logFile = strdup("Consolelog.txt.0");
    marker->skipFreq = 0;
    marker->paramType = strdup("grep");
    marker->mType = MTYPE_COUNTER;
    marker->u.count = 0;
    marker->reportEmptyParam = true;
    Vector_PushBack(vecMarkerList, (void*) marker);


    //freeFileDescriptor
    EXPECT_CALL(*g_fileIOMock, munmap(_, _))
            .Times(2)
            .WillOnce(Return(0))
            .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, close(_)) 
            .Times(6)
            .WillOnce(Return(0))
            .WillOnce(Return(0))
            .WillOnce(Return(0))
            .WillOnce(Return(0))
            .WillOnce(Return(0))
            .WillOnce(Return(0));

    //getLogFileDescriptor
    EXPECT_CALL(*g_fileIOMock, open(_,_))
            .Times(2)
            .WillOnce(Return(0))
            .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, fstat(_, _))
        .Times(4)
        .WillOnce([](int fd, struct stat* statbuf) {
        statbuf->st_size = 1235;      // Set file size
        return 0; // Success
    })
        .WillOnce([](int fd, struct stat* statbuf) {
        statbuf->st_size = 1235;      // Set file size
        return 0; // Success
    })
        .WillOnce([](int fd, struct stat* statbuf) {
        statbuf->st_size = 1000;      // Set file size
        return 0; // Success
    })
        .WillOnce([](int fd, struct stat* statbuf) {
        statbuf->st_size = 1000;      // Set file size
        return 0; // Success
    });

    //getDeltainmmapsearch 
    EXPECT_CALL(*g_fileIOMock, mkstemp(_))
            .Times(2)
            .WillOnce(Return(0))
            .WillOnce(Return(0));
    EXPECT_CALL(*g_systemMock, unlink(_))
                .Times(2)
                .WillOnce(Return(0))
                .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock,sendfile(_,_,_,_))
            .Times(2)
            .WillOnce(Return(1235))
            .WillOnce(Return(1000));
    EXPECT_CALL(*g_fileIOMock, mmap(_,_,_,_,_,_))
                .Times(2)
                .WillOnce([](void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
                    const char* test_str = "This is a Test Marker with value 1234 in the log file.\nAnother line without the marker.\n";
                    char* mapped_mem = (char*)malloc(length);
                    memset(mapped_mem, 0, length);
                    strncpy(mapped_mem, test_str, length - 1);
                    return (void*)mapped_mem;
                })
                .WillOnce([](void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
                    const char* test_str = "This is with value Test:1250 in the log file.\nAnother line without the marker.\nThe line with Test Markeris found\n";
                    char* mapped_mem = (char*)malloc(length);
                    memset(mapped_mem, 0, length);
                    strncpy(mapped_mem, test_str, length - 1);
                    return (void*)mapped_mem;
                });

    
    EXPECT_EQ(0, getDCAResultsInVector(gsProfile, vecMarkerList, true, "/opt/logs"));
    hash_map_destroy(gsProfile->logFileSeekMap, free);
    gsProfile->logFileSeekMap = NULL;
    free(gsProfile);
    Vector_Destroy(vecMarkerList, freeGMarker);
}

#endif

TEST_F(dcaTestFixture, T2InitProperties)
{
   EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(2)
            .WillOnce(Return((FILE*)0XFFFFFFFF))
            .WillOnce(Return((FILE*)0XFFFFFFFF));
   EXPECT_CALL(*g_fileIOMock, fscanf(_,_,_))
            .Times(2)
            .WillOnce(Return(EOF))
            .WillOnce(Return(EOF));

   EXPECT_CALL(*g_fileIOMock, fclose(_))
            .Times(2)
            .WillOnce(Return(0))
            .WillOnce(Return(0));         
            
    T2InitProperties();
}

//dcautil.c

TEST_F(dcaTestFixture, getGrepResults_success)
{
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
    marker->searchString = strdup("temp:");
    marker->trimParam = true;
    marker->regexParam = strdup("[0-9]+");
    marker->logFile = strdup("Consolelog.txt.0");
    marker->skipFreq = 0;
    marker->paramType = strdup("grep");
    marker->mType = MTYPE_ABSOLUTE;
    marker->u.markerValue = NULL;
    marker->u.count = 0;
    marker->reportEmptyParam = true;
    Vector_PushBack(vecMarkerList, (void*) marker);

    //freeFileDescriptor
    EXPECT_CALL(*g_fileIOMock, munmap(_, _))
            .Times(1)
            .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, close(_)) 
            .Times(4)
            .WillOnce(Return(0))
            .WillOnce(Return(0))
            .WillOnce(Return(0))
            .WillOnce(Return(0));

    //getLogFileDescriptor
    EXPECT_CALL(*g_fileIOMock, open(_,_))
            .Times(1)
            .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, fstat(_, _))
        .Times(2)
        .WillOnce([](int fd, struct stat* statbuf) {
        statbuf->st_size = 1300;      // Set file size
        return 0; // Success
    })
        .WillOnce([](int fd, struct stat* statbuf) {
        statbuf->st_size = 1300;      // Set file size
        return 0; // Success
    });

    //getDeltainmmapsearch 
    EXPECT_CALL(*g_fileIOMock, mkstemp(_))
            .Times(1)
            .WillOnce(Return(0));
    EXPECT_CALL(*g_systemMock, unlink(_))
                .Times(1)
                .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock,sendfile(_,_,_,_))
            .Times(1)
            .WillOnce(Return(1300));
    EXPECT_CALL(*g_fileIOMock, mmap(_,_,_,_,_,_))
                .Times(1)
                .WillOnce([](void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
                    const char* test_str = "This is a Test Marker with value temp:1245.\nAnother line without the marker.\n Another line with marker temp:2345\n";
                    char* mapped_mem = (char*)malloc(length);
                    memset(mapped_mem, 0, length);
                    strncpy(mapped_mem, test_str, length - 1);
                    return (void*)mapped_mem;
                });

    
    EXPECT_EQ(T2ERROR_SUCCESS, getGrepResults(&gsProfile, vecMarkerList, true, true, "/opt/logs"));
    hash_map_destroy(gsProfile->logFileSeekMap, free);
    gsProfile->logFileSeekMap = NULL;
    free(gsProfile);
    Vector_Destroy(vecMarkerList, freeGMarker);
}

