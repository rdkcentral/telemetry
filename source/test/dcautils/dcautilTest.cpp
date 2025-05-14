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
#include "telemetry2_0.h"
#include "utils/t2log_wrapper.h"
#include "utils/t2common.h"
#include "ccspinterface/busInterface.h"
}
using namespace std;

using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;

FileIOMock * g_fileIOMock = NULL;
SystemMock * g_SystemMock = NULL;
rdklogMock *m_rdklogMock = NULL;

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

class dcaFileTestFixture : public ::testing::Test {
    protected:
        FileIOMock mockeddcaFileIO;

        dcaFileTestFixture()
        {
            g_fileIOMock = &mockeddcaFileIO;

        }
        virtual ~dcaFileTestFixture()
        {
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

class dcaSystemTestFixture : public ::testing::Test {
    protected:
        SystemMock mockedSSystem;

        dcaSystemTestFixture()
        {
            g_SystemMock = &mockedSSystem;

        }
        virtual ~dcaSystemTestFixture()
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

class dcaTestFixture : public ::testing::Test {
    protected:
        FileIOMock mockeddcaFileIO;
        SystemMock mockedSSystem;

        dcaTestFixture()
        {
            g_fileIOMock = &mockeddcaFileIO;
            g_SystemMock = &mockedSSystem;

        }
        virtual ~dcaTestFixture()
        {
            g_fileIOMock = NULL;
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


//dcalist.c

GList *pch = NULL;

TEST(INSERTPCNODE, PATT_NULL)
{
   EXPECT_EQ(0, insertPCNode(&pch, NULL, "SYS_INFO", STR, 0, "SYS_INFO_DATA", true, NULL));
}

TEST(INSERTPCNODE, HEAD_NULL)
{
   EXPECT_EQ(0, insertPCNode(&pch, "info is", NULL, STR, 0, "SYS_INFO_DATA", true, NULL));
}

TEST(INSERTPCNODE, DATA_NULL)
{
   EXPECT_EQ(0, insertPCNode(&pch, "info is", "SYS_INFO", STR, 0, NULL, false, NULL));
}

TEST(INSERTPCNODE, APPEND_CHECK)
{
   EXPECT_EQ(0, insertPCNode(&pch,  NULL, NULL, STR, 0, NULL, false, NULL));
}

TEST(COMPAREPATTERN, NP_SP_NULL_CHECK)
{
   pcdata_t *sp = (pcdata_t*)"SEARCH_DATA_LIST";
   pcdata_t *np = (pcdata_t*)"POINTER TO BE SEARCHED";
   EXPECT_EQ(-1, comparePattern(NULL, (gpointer)sp));
   EXPECT_EQ(-1, comparePattern(np, NULL));
}

TEST(SEARCHPCNODE, NULL_CHECK)
{
   GList *fnode = NULL;
   EXPECT_EQ(NULL, searchPCNode(NULL, "info is"));
   fnode = g_list_append(fnode, (gpointer)"Hello world!");
   EXPECT_EQ(NULL, searchPCNode(fnode, NULL));
   rdk_list_free_all_nodes(fnode);
}


//dcaproc.c

TEST(GETPROCUSAGE, GREPRESULTLIST_NULL)
{
   EXPECT_EQ(-1, getProcUsage("telemetry2_0", NULL, false, NULL));
}

TEST(GETPROCUSAGE, GREPRESULTLIST_NULL_1)
{
   EXPECT_EQ(-1, getProcUsage("telemetry2_0", NULL, true, NULL));
}

TEST(GETPROCUSAGE, PROCESS_NULL)
{
   Vector* gresulist = NULL;
   Vector_Create(&gresulist);
   EXPECT_EQ(-1, getProcUsage(NULL, gresulist, false, NULL));
   Vector_Destroy(gresulist, free);
   gresulist = NULL;
}

TEST(GETPROCUSAGE, PROCESS_NULL_1)
{
   Vector* gresulist = NULL;
   Vector_Create(&gresulist);
   EXPECT_EQ(-1, getProcUsage(NULL, gresulist, true, NULL));
   Vector_Destroy(gresulist, free);
   gresulist = NULL;
}


TEST(GETPROCPIDSTAT, PINFO_NULL)
{
   EXPECT_EQ(0, getProcPidStat(12345, NULL));
}

TEST(GETPROCINFO, PMINFO_NULL)
{
   EXPECT_EQ(0, getProcInfo(NULL));
}

TEST(GETMEMINFO, PMINFO_NULL)
{
   EXPECT_EQ(0, getMemInfo(NULL));
}

TEST(GETCPUINFO, PINFO_NULL)
{
   EXPECT_EQ(0, getCPUInfo(NULL));
}
//dcautil.c
char* gmarker1 = "SYS_INFO_BOOTUP";
char* dcamarker1 = "SYS_INFO1";
char* dcamarker2 = "SYS_INFO2";


/*TEST(SAVEGREPCONFIG, ARGS_NULL)
{
   Vector* greplist = NULL;
   Vector_Create(&greplist);
   Vector_PushBack(greplist, (void*) strdup(gmarker1));
   EXPECT_EQ(T2ERROR_FAILURE, saveGrepConfig(NULL, greplist));
   EXPECT_EQ(T2ERROR_FAILURE, saveGrepConfig("RDKB_Profile1", NULL));
   Vector_Destroy(greplist, free);
}*/

TEST(GETGREPRESULTS, PROFILENAME_NULL)
{
   Vector* markerlist = NULL;
   Vector_Create(&markerlist);
   Vector_PushBack(markerlist, (void*) strdup(dcamarker1));
   Vector_PushBack(markerlist, (void*) strdup(dcamarker2));
   Vector* grepResultlist = NULL;
   Vector_Create(&grepResultlist);
   EXPECT_EQ(T2ERROR_FAILURE, getGrepResults(NULL, markerlist, &grepResultlist, false, false));
   EXPECT_EQ(T2ERROR_FAILURE, getGrepResults("RDKB_Profile1", NULL, &grepResultlist, false, false));
   EXPECT_EQ(T2ERROR_FAILURE, getGrepResults("RDKB_Profile1", markerlist, NULL, false, false));
   EXPECT_EQ(T2ERROR_FAILURE, getGrepResults(NULL, markerlist, &grepResultlist, false, true));
   EXPECT_EQ(T2ERROR_FAILURE, getGrepResults("RDKB_Profile1", NULL, &grepResultlist, false, true));
   EXPECT_EQ(T2ERROR_FAILURE, getGrepResults("RDKB_Profile1", markerlist, NULL, false, true));
   Vector_Destroy(markerlist, free);
   Vector_Destroy(grepResultlist, free);
   grepResultlist = NULL;
   markerlist = NULL;
}

//legacyutils.c

TEST(ADDTOPROFILESEEK, PROFILENM_NULL)
{
    EXPECT_EQ(NULL, addToProfileSeekMap(NULL));
}

TEST(ADDTOPROFILESEEK, PROFILENM_INVALID)
{
   EXPECT_EQ(NULL, addToProfileSeekMap("RDKB_INVALID_PROFILE1"));
}

TEST(GETLOGSEEKMAP, PROFILENM_NULL)
{
   EXPECT_EQ(NULL, getLogSeekMapForProfile(NULL));
}

TEST(GETLOGSEEKMAP, PROFILENM_INVALID)
{
   EXPECT_EQ(NULL, getLogSeekMapForProfile("RDKB_INVALID_PROFILE1"));
}

TEST(UPDATELOGSEEK, LOGSEEKMAP_NULL)
{
   char* logFileName = "t2_log.txt";
   hash_map_t* logseekmap = (hash_map_t *)malloc(512);
   EXPECT_EQ(T2ERROR_FAILURE,  updateLogSeek(NULL, logFileName));
   EXPECT_EQ(T2ERROR_FAILURE,  updateLogSeek(logseekmap, NULL));
   free(logseekmap);
   logseekmap = NULL;
}

TEST(GETLOADAVG, VECTOR_NULL)
{
    EXPECT_EQ(0, getLoadAvg(NULL, false, NULL));
}

TEST(GETLOGLINE, NAME_NULL)
{
    hash_map_t* logSeekMap = (hash_map_t *)malloc(512);
    char* buf = (char *)malloc(512);
    char* name = "Consolelog.txt.0";
    int seekFromEOF = 0;
    EXPECT_EQ(NULL, getLogLine(NULL, buf, 512, name, &seekFromEOF, false));
    EXPECT_EQ(NULL, getLogLine(logSeekMap, NULL, 512, name, &seekFromEOF, false));
    EXPECT_EQ(NULL, getLogLine(logSeekMap, buf, 512, NULL, &seekFromEOF, false));
    EXPECT_EQ(NULL, getLogLine(NULL, buf, 512, name, &seekFromEOF, true));
    EXPECT_EQ(NULL, getLogLine(logSeekMap, NULL, 512, name, &seekFromEOF, true));
    EXPECT_EQ(NULL, getLogLine(logSeekMap, buf, 512, NULL, &seekFromEOF, true));

    free(buf);
    buf = NULL;
    free(logSeekMap);
    logSeekMap = NULL;
}

TEST(ISPROPSINITIALIZED, TESTING)
{
    EXPECT_EQ(false, isPropsInitialized());
}

//dca.c

TEST(PROCESSTOPPATTERN, VECTOR_NULL)
{
    char* logfile = "Consolelog.txt.0";
    GList *tlist = NULL;
    tlist =  g_list_append(tlist, (gpointer)"Hello world!");
    Vector* grepResultlist = NULL;
    Vector_Create(&grepResultlist);
    EXPECT_EQ(-1, processTopPattern(tlist, grepResultlist));
    EXPECT_EQ(-1, processTopPattern(NULL, grepResultlist));
    EXPECT_EQ(-1, processTopPattern(tlist, NULL));
    rdk_list_free_all_nodes(tlist);
    Vector_Destroy(grepResultlist, free);
    grepResultlist = NULL;
}

TEST(GETERRORCODE, STR_NULL)
{
    char* str = "telemetry-dcautil";
    EXPECT_EQ(0, getErrorCode(str, NULL));
}

//dcaproc.c

TEST_F(dcaFileTestFixture, getProcPidStat)
{
    procinfo pinfo;
    int fd = (int)0xffffffff;
    EXPECT_CALL(*g_fileIOMock, open(_,_))
            .Times(1)
            .WillOnce(Return(-1));
    ASSERT_EQ(0, getProcPidStat(123, &pinfo));
}

TEST_F(dcaFileTestFixture, getProcPidStat1)
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

TEST_F(dcaFileTestFixture,  getProcUsage)
{
    Vector* gresulist = NULL;
    Vector_Create(&gresulist);
    char* processName = "telemetry2_0";
    FILE* fp = (FILE*)NULL;
    #ifdef LIBSYSWRAPPER_BUILD
    EXPECT_CALL(*g_SystemMock, v_secure_popen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    #else
    EXPECT_CALL(*g_fileIOMock, popen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    #endif
    EXPECT_EQ(0, getProcUsage(processName, gresulist, false, NULL));
}


#if !defined(ENABLE_RDKC_SUPPORT) && !defined(ENABLE_RDKB_SUPPORT)
TEST_F(dcaTestFixture, getCPUInfo1)
{
    FILE *inFp = (FILE*)NULL;
    EXPECT_CALL(*g_SystemMock, access(_,_))
                .Times(1)
                .WillOnce(Return(-1));

    #ifdef LIBSYSWRAPPER_BUILD
       EXPECT_CALL(*g_SystemMock, v_secure_system(_))
                .Times(1)
                .WillOnce(Return(-1));
    #else
       EXPECT_CALL(*g_SystemMock, system(_))
                .Times(1)
                .WillOnce(Return(-1));
    #endif
    #ifdef INTEL
       #ifdef LIBSYSWRAPPER_BUILD

           EXPECT_CALL(*g_fileIOMock, v_secure_popen(_,_))
                .Times(1)
                .WillOnce(Return(inFp));
       #else
           EXPECT_CALL(*g_fileIOMock, popen(_,_))
                .Times(1)
                .WillOnce(Return(inFp));
       #endif
   #else
       #ifdef LIBSYSWRAPPER_BUILD

           EXPECT_CALL(*g_fileIOMock, v_secure_popen(_,_))
                .Times(1)
                .WillOnce(Return(inFp));
       #else
           EXPECT_CALL(*g_fileIOMock, popen(_,_))
                .Times(1)
                .WillOnce(Return(inFp));
       #endif
   #endif
      
         procMemCpuInfo pinfo;
         char* processName = "telemetry";
         memset(&pinfo, '\0', sizeof(procMemCpuInfo));
         memcpy(pinfo.processName, processName, strlen(processName) + 1);
         EXPECT_EQ(0,  getCPUInfo(&pinfo));
}


#else
TEST_F(dcaFileTestFixture, getTotalCpuTimes)
{
    FILE* mockfp = (FILE *)NULL;
    int a[2];
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
     EXPECT_CALL(*g_ffileIOMock, pclose(_))
             .Times(1)
             .WillOnce(Return(-1));
    EXPECT_EQ(0, getProcUsage(processName, gresulist, false));
}

TEST_F(dcaFileTestFixture,  getProcUsage2)
{
    Vector* gresulist = NULL;
    Vector_Create(&gresulist);
    char* processName = "telemetry2_0";
    FILE* fp = (FILE*)NULL;
    EXPECT_CALL(*g_ffileIOMock, popen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    EXPECT_EQ(0, getProcUsage(processName, gresulist, true));
}

TEST_F(dcaFileTestFixture,  getProcUsage3)
{
    Vector* gresulist = NULL;
    Vector_Create(&gresulist);
    char* processName = "telemetry2_0";
    FILE* fp = (FILE*)0xffffffff;
     EXPECT_CALL(*g_ffileIOMock, popen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
     EXPECT_CALL(*g_ffileIOMock, pclose(_))
             .Times(1)
             .WillOnce(Return(-1));
    EXPECT_EQ(0, getProcUsage(processName, gresulist, true));
}
#endif


//legacyutils.c
TEST_F(dcaFileTestFixture, getLoadAvg)
{
    Vector* grepResultList;
    Vector_Create(&grepResultList);
    FILE* fp = (FILE*)NULL;
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    EXPECT_EQ(0, getLoadAvg(grepResultList, false, NULL));
}

#if 0
TEST_F(dcaFileTestFixture, getLoadAvg1)
{
    Vector* grepResultList;
    Vector_Create(&grepResultList);
    FILE* fp = (FILE*)0xffffffff;
    EXPECT_CALL(*g_ffileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    EXPECT_CALL(*g_ffileIOMock, fread(_,_,_,_))
            .Times(1)
            .WillOnce(Return(-1));
    EXPECT_CALL(*g_ffileIOMock, fclose(_))
            .Times(1)
            .WillOnce(Return(0));
    EXPECT_EQ(0, getLoadAvg(grepResultList, false, NULL));
}

TEST_F(dcaFileTestFixture, getLoadAvg2)
{
    Vector* grepResultList;
    Vector_Create(&grepResultList);
    FILE* fp = (FILE*)NULL;
    EXPECT_CALL(*g_ffileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    EXPECT_EQ(0, getLoadAvg(grepResultList, true, "[0-9]*"));
}

TEST_F(dcaFileTestFixture, getLoadAvg3)
{
    Vector* grepResultList;
    Vector_Create(&grepResultList);
    FILE* fp = (FILE*)0xffffffff;
    EXPECT_CALL(*g_ffileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    EXPECT_CALL(*g_ffileIOMock, fread(_,_,_,_))
            .Times(1)
            .WillOnce(Return(-1));
    EXPECT_CALL(*g_ffileIOMock, fclose(_))
            .Times(1)
            .WillOnce(Return(0));
    EXPECT_EQ(0, getLoadAvg(grepResultList, true, "[0-9]+"));
}


TEST_F(dcaFileTestFixture, getLogLine)
{
    hash_map_t* logSeekMap = (hash_map_t *)malloc(512);
    char* buf = (char *)malloc(512);
    char* name = "Consolelog.txt.0";
    FILE* fp = (FILE*)NULL;
    int seekFromEOF =0;
    EXPECT_CALL(*g_ffileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    EXPECT_EQ(NULL, getLogLine(logSeekMap, buf, 10, name, &seekFromEOF, 1, false));
}

TEST_F(dcaFileTestFixture, getLogLine2)
{
    hash_map_t* logSeekMap = (hash_map_t *)malloc(512);
    char* buf = (char *)malloc(512);
    char* name = "Consolelog.txt.0";
    FILE* fp = (FILE*)NULL;
    int seekFromEOF =0;
    EXPECT_CALL(*g_ffileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    EXPECT_EQ(NULL, getLogLine(logSeekMap, buf, 10, name, &seekFromEOF, 1, true));
}


TEST_F(dcaFileTestFixture, updateLastSeekval)
{
    hash_map_t *logSeekMap = ( hash_map_t*)malloc(512);
    char* prev_file = NULL;
    char* filename = "Consolelog.txt.0";
    FILE* fp = (FILE*)NULL;
    EXPECT_CALL(*g_ffileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    updateLastSeekval(logSeekMap, &prev_file, filename);
}

TEST_F(dcaFileTestFixture, updateLastSeekval1)
{
    hash_map_t *logSeekMap = ( hash_map_t*)malloc(512);
    char* prev_file = NULL;
    char* filename = "Consolelog.txt.0";
    FILE* fp = (FILE*)0xffffffff;
    EXPECT_CALL(*g_ffileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    EXPECT_CALL(*g_ffileIOMock, fclose(_))
            .Times(1)
            .WillOnce(Return(0));
    updateLastSeekval(logSeekMap, &prev_file, filename);
}

TEST_F(dcaFileTestFixture, getLogLine1)
{
    hash_map_t* logSeekMap = (hash_map_t *)malloc(512);
    char* buf = (char *)malloc(512);
    char* name = "core_log.txt";
     FILE* fp = (FILE*)0xffffffff;
     int seekFromEOF =0;
     EXPECT_CALL(*g_ffileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    EXPECT_CALL(*g_ffileIOMock, fseek(_,_,_))
            .Times(1)
            .WillOnce(Return(-1));
     EXPECT_EQ(NULL, getLogLine(logSeekMap, buf, 10, name, &seekFromEOF, 1, false));

}
#endif

