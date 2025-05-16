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



//Insert node to the list

TEST(rdk_list_add_node, add_node_to_empty_list){
    rdkList_t *rdkListHndl = NULL;
    rdkList_t *node = (rdkList_t*) malloc (sizeof(rdkList_t));
    node->m_pUserData = (void*)"test";
    node->m_pForward = NULL;
    node->m_pBackward = NULL;
    rdkListHndl = rdk_list_add_node(rdkListHndl, node->m_pUserData);
    EXPECT_EQ(node->m_pUserData, rdkListHndl->m_pUserData);
    rdk_list_free_all_nodes(rdkListHndl);
}

// Insert node to the list which is not empty
TEST(rdk_list_add_node, add_node_to_non_empty_list){
    rdkList_t *rdkListHndl = NULL;
    rdkList_t *node1 = (rdkList_t*) malloc (sizeof(rdkList_t));
    node1->m_pUserData = (void*)"test1";
    node1->m_pForward = NULL;
    node1->m_pBackward = NULL;
    rdkListHndl = node1;
    rdkList_t *node2 = (rdkList_t*) malloc (sizeof(rdkList_t));
    node2->m_pUserData = (void*)"test2";
    node2->m_pForward = NULL;
    node2->m_pBackward = NULL;
    rdkListHndl = rdk_list_add_node(rdkListHndl, node2->m_pForward);
    EXPECT_EQ(node1->m_pForward, rdkListHndl->m_pForward);
    rdk_list_free_all_nodes(rdkListHndl);
}

//Prepend node to the list
TEST(rdk_list_prepend_node, prepend_node_to_empty_list){
    rdkList_t *rdkListHndl = NULL;
    rdkList_t *node = (rdkList_t*) malloc (sizeof(rdkList_t));
    node->m_pUserData = (void*)"test";
    node->m_pForward = NULL;
    node->m_pBackward = NULL;
    rdkListHndl = rdk_list_prepend_node(rdkListHndl, node->m_pUserData);
    EXPECT_EQ(node->m_pUserData, rdkListHndl->m_pUserData);
    rdk_list_free_all_nodes(rdkListHndl);
}

// Prepend node to the list which is not empty
TEST(rdk_list_prepend_node, prepend_node_to_non_empty_list){
    rdkList_t *rdkListHndl = NULL;
    rdkList_t *node1 = (rdkList_t*) malloc (sizeof(rdkList_t));
    node1->m_pUserData = (void*)"test1";
    node1->m_pForward = NULL;
    node1->m_pBackward = NULL;
    rdkListHndl = rdk_list_add_node(rdkListHndl, node1->m_pUserData);
    rdkList_t *node2 = (rdkList_t*) malloc (sizeof(rdkList_t));
    node2->m_pUserData = (void*)"test2";
    node2->m_pForward = NULL;
    node2->m_pBackward = NULL;
    rdkListHndl = rdk_list_prepend_node(rdkListHndl, node2->m_pUserData);
    EXPECT_EQ(node2->m_pUserData, rdkListHndl->m_pUserData);
    rdk_list_free_all_nodes(rdkListHndl);
}

// Add node before sibling node
TEST(rdk_list_add_node_before, add_node_before_sibling){
    rdkList_t *rdkListHndl = NULL;
    rdkList_t *node1 = (rdkList_t*) malloc (sizeof(rdkList_t));
    node1->m_pUserData = (void*)"test1";
    node1->m_pForward = NULL;
    node1->m_pBackward = NULL;
    rdkListHndl = rdk_list_add_node(rdkListHndl, node1->m_pUserData);
    rdkList_t *node2 = (rdkList_t*) malloc (sizeof(rdkList_t));
    node2->m_pUserData = (void*)"test2";
    node2->m_pForward = NULL;
    node2->m_pBackward = NULL;
    rdkListHndl = rdk_list_add_node_before(rdkListHndl, node1, node2->m_pUserData);
    EXPECT_EQ(node2->m_pUserData, rdkListHndl->m_pUserData);
    rdk_list_free_all_nodes(rdkListHndl);
}

TEST(rdk_list_add_node_before, add_node_to_empty_list_with_sibling){
    rdkList_t *rdkListHndl = NULL;
    rdkList_t *node1 = (rdkList_t*) malloc (sizeof(rdkList_t));
    node1->m_pUserData = (void*)"test1";
    node1->m_pForward = NULL;
    node1->m_pBackward = NULL;
    rdkList_t *node2 = (rdkList_t*) malloc (sizeof(rdkList_t));
    node2->m_pUserData = (void*)"test2";
    node2->m_pForward = NULL;
    node2->m_pBackward = NULL;
    rdkListHndl = rdk_list_add_node_before(rdkListHndl, node1, node2->m_pUserData);
    EXPECT_EQ(node2->m_pUserData, rdkListHndl->m_pUserData);
    rdk_list_free_all_nodes(rdkListHndl);
}

TEST(rdk_list_find_first_node, find_first_node)
{
    rdkList_t *rdkListHndl = NULL;
    rdkList_t *node1 = (rdkList_t*) malloc (sizeof(rdkList_t));
    node1->m_pUserData = (void*)"test1";
    node1->m_pForward = NULL;
    node1->m_pBackward = NULL;
    rdkListHndl = rdk_list_add_node(rdkListHndl, node1->m_pUserData);
    rdkList_t *node2 = (rdkList_t*) malloc (sizeof(rdkList_t));
    node2->m_pUserData = (void*)"test2";
    node2->m_pForward = NULL;
    node2->m_pBackward = NULL;
    rdkListHndl = rdk_list_add_node(rdkListHndl, node2->m_pUserData);
    EXPECT_EQ(node1->m_pUserData, rdk_list_find_first_node(rdkListHndl)->m_pUserData);
    rdk_list_free_all_nodes(rdkListHndl);
}

TEST(rdk_list_find_next_node, find_next_node)
{
    rdkList_t *rdkListHndl = NULL;
    rdkList_t *node1 = (rdkList_t*) malloc (sizeof(rdkList_t));
    node1->m_pUserData = (void*)"test1";
    node1->m_pForward = NULL;
    node1->m_pBackward = NULL;
    rdkListHndl = rdk_list_add_node(rdkListHndl, node1->m_pUserData);
    rdkList_t *node2 = (rdkList_t*) malloc (sizeof(rdkList_t));
    node2->m_pUserData = (void*)"test2";
    node2->m_pForward = NULL;
    node2->m_pBackward = NULL;
    rdkListHndl = rdk_list_add_node(rdkListHndl, node2->m_pUserData);
    EXPECT_EQ(NULL, rdk_list_find_next_node(rdkListHndl)->m_pForward);
    rdk_list_free_all_nodes(rdkListHndl);
}

TEST(rdk_list_reverse, reverse_list)
{
    rdkList_t *rdkListHndl = NULL;
    rdkList_t *node1 = (rdkList_t*) malloc (sizeof(rdkList_t));
    node1->m_pUserData = (void*)"test1";
    node1->m_pForward = NULL;
    node1->m_pBackward = NULL;
    rdkListHndl = rdk_list_add_node(rdkListHndl, node1->m_pUserData);
    rdkList_t *node2 = (rdkList_t*) malloc (sizeof(rdkList_t));
    node2->m_pUserData = (void*)"test2";
    node2->m_pForward = NULL;
    node2->m_pBackward = NULL;
    rdkListHndl = rdk_list_add_node(rdkListHndl, node2->m_pUserData);
    EXPECT_EQ(node2->m_pUserData, rdk_list_reverse(rdkListHndl)->m_pUserData);
    rdk_list_free_all_nodes(rdkListHndl);
}

TEST(rdk_list_remove_node, remove_node)
{
    rdkList_t *rdkListHndl = NULL;
    rdkList_t *node1 = (rdkList_t*) malloc (sizeof(rdkList_t));
    node1->m_pUserData = (void*)"test1";
    node1->m_pForward = NULL;
    node1->m_pBackward = NULL;
    rdkListHndl = rdk_list_add_node(rdkListHndl, node1->m_pUserData);
    rdkList_t *node2 = (rdkList_t*) malloc (sizeof(rdkList_t));
    node2->m_pUserData = (void*)"test2";
    node2->m_pForward = NULL;
    node2->m_pBackward = NULL;
    rdkListHndl = rdk_list_add_node(rdkListHndl, node2->m_pUserData);
    EXPECT_EQ(node1->m_pUserData, rdk_list_remove_node(rdkListHndl, node1)->m_pUserData);
    rdk_list_free_all_nodes(rdkListHndl);
}

TEST(rdk_list_remove, remove_list)
{
    rdkList_t *rdkListHndl = NULL;
    rdkList_t *node1 = (rdkList_t*) malloc (sizeof(rdkList_t));
    node1->m_pUserData = (void*)"test1";
    node1->m_pForward = NULL;
    node1->m_pBackward = NULL;
    rdkListHndl = rdk_list_add_node(rdkListHndl, node1->m_pUserData);
    rdkList_t *node2 = (rdkList_t*) malloc (sizeof(rdkList_t));
    node2->m_pUserData = (void*)"test2";
    node2->m_pForward = NULL;
    node2->m_pBackward = NULL;
    rdkListHndl = rdk_list_add_node(rdkListHndl, node2->m_pUserData);
    EXPECT_EQ(node1->m_pUserData, rdk_list_remove(rdkListHndl, node2->m_pUserData)->m_pUserData);
    rdk_list_free_all_nodes(rdkListHndl);
}

//dcalist.c

rdkList_t *pch = NULL;

TEST(insertPCNode, pattern_is_NULL) //when pattern is NULL
{
   EXPECT_EQ(0, insertPCNode(&pch, NULL, "SYS_INFO", STR, 0, "SYS_INFO_DATA", true, NULL));
}

TEST(insertPCNode, head_is_NULL) //when head is NULL
{
   EXPECT_EQ(0, insertPCNode(&pch, "info is", NULL, STR, 0, "SYS_INFO_DATA", true, NULL));
}

//When data is NULL
TEST(insertPCNode, data_is_NULL)
{
   EXPECT_EQ(0, insertPCNode(&pch, "info is", "SYS_INFO", STR, 0, NULL, false, NULL));
}
//When pattern, head and data are NULL
TEST(insertPCNode, all_NULL)
{
   EXPECT_EQ(0, insertPCNode(&pch, NULL, NULL, STR, 0, NULL, false, NULL));
}

TEST(insertPCNode, regex_not_NULL)
{
   EXPECT_EQ(0, insertPCNode(&pch,  NULL, NULL, STR, 0, NULL, false, "[0-9]+"));
   EXPECT_EQ(0, insertPCNode(&pch,  NULL, NULL, STR, 0, NULL, true, "[0-9]*"));
}

TEST(COMPAREPATTERN, NP_SP_NULL_CHECK)
{
   pcdata_t *sp = (pcdata_t*)"SEARCH_DATA_LIST";
   pcdata_t *np = (pcdata_t*)"POINTER TO BE SEARCHED";
   EXPECT_EQ(-1, comparePattern(NULL, (gpointer)sp));
   EXPECT_EQ(-1, comparePattern(np, NULL));
}

TEST(searchPCNode, list_pattern_NULL)
{
   char* pattern = "Memory Usage is";
   rdkList_t *node1 = NULL;
   EXPECT_EQ(NULL, searchPCNode(node1, pattern));
   rdkList_t *node = (rdkList_t*) malloc (sizeof(rdkList_t));
   node->m_pUserData = (void*)"test1";
   node->m_pForward = NULL;
   node->m_pBackward = NULL;
   EXPECT_EQ(NULL, searchPCNode(node, NULL));
   rdk_list_free_all_nodes(node);
}

//dcajson.c

TEST(addtosearchResult, searchresult_NULL)
{
   char* value = "Memory Usage is";
   char* key = "SYS_MEM_INFO";
   addToSearchResult(NULL, NULL);
   addToSearchResult(NULL, value);
   addToSearchResult(key, NULL);
}

TEST(clearSearchResultJson, check_json)
{
    cJSON* json = cJSON_CreateObject();
    clearSearchResultJson(&json);
}

//dcaproc.c

TEST(GETPROCUSAGE, GREPRESULTLIST_PROCESS_NULL)
{
   EXPECT_EQ(-1, getProcUsage("telemetry2_0", NULL, false, NULL));
   EXPECT_EQ(-1, getProcUsage("telemetry2_0", NULL, true, NULL));
   EXPECT_EQ(-1, getProcUsage("telemetry2_0", NULL, false, "[0-9]"));
   EXPECT_EQ(-1, getProcUsage("telemetry2_0", NULL, true, "[0-9]"));
}

TEST(GETPROCUSAGE, PROCESS_NULL)
{
   Vector* grepResultList = NULL;
   Vector_Create(&grepResultList);
   Vector_PushBack(grepResultList, (void*) strdup("SYS_INFO_BOOTUP"));
   Vector_PushBack(grepResultList, (void*) strdup("SYS_INFO_MEM"));
   EXPECT_EQ(-1, getProcUsage(NULL, grepResultList, false, NULL));
   EXPECT_EQ(-1, getProcUsage(NULL, grepResultList, true, NULL));
   EXPECT_EQ(-1, getProcUsage(NULL, grepResultList, false, "[0-9]"));
   EXPECT_EQ(-1, getProcUsage(NULL, grepResultList, true, "[0-9]"));
   Vector_Destroy(grepResultList, free);
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

TEST(GETGREPRESULTS, PROFILENAME_NULL)
{
   Vector* markerlist = NULL;
   Vector_Create(&markerlist);
   Vector_PushBack(markerlist, (void*) strdup(dcamarker1));
   Vector_PushBack(markerlist, (void*) strdup(dcamarker2));
   Vector* grepResultlist = NULL;
   Vector_Create(&grepResultlist);
   EXPECT_EQ(T2ERROR_FAILURE, getGrepResults(NULL, markerlist, &grepResultlist, false, false,"/opt/logs"));
   EXPECT_EQ(T2ERROR_FAILURE, getGrepResults("RDKB_Profile1", NULL, &grepResultlist, false, false,"/opt/logs"));
   EXPECT_EQ(T2ERROR_FAILURE, getGrepResults("RDKB_Profile1", markerlist, NULL, false, false,"/opt/logs"));
   EXPECT_EQ(T2ERROR_FAILURE, getGrepResults(NULL, markerlist, &grepResultlist, false, true,"/opt/logs"));
   EXPECT_EQ(T2ERROR_FAILURE, getGrepResults("RDKB_Profile1", NULL, &grepResultlist, false, true,"/opt/logs"));
   EXPECT_EQ(T2ERROR_FAILURE, getGrepResults("RDKB_Profile1", markerlist, NULL, false, true,"/opt/logs"));
   Vector_Destroy(markerlist, free);
   Vector_Destroy(grepResultlist, free);
   grepResultlist = NULL;
   markerlist = NULL;
}

#ifdef PERSIST_LOG_MON_REF
TEST(saveSeekConfigtoFile, profilename_NULL)
{
   EXPECT_EQ(T2ERROR_FAILURE, saveSeekConfigtoFile(NULL));
}
TEST(saveSeekConfigtoFile, profilename_NOT_NULL)
{
   EXPECT_EQ(T2ERROR_FAILURE, saveSeekConfigtoFile("RDKB_Profile1"));
}

TEST(loadSavedSeekConfig, profilename_NULL)
{
   EXPECT_EQ(T2ERROR_FAILURE, loadSavedSeekConfig(NULL));
}
TEST(loadSavedSeekConfig, profilename_NOT_NULL)
{
   EXPECT_EQ(T2ERROR_FAILURE, loadSavedSeekConfig("RDKB_Profile1"));
}
#endif

//legacyutils.c

TEST(ADDTOPROFILESEEK, PROFILENM_NULL)
{
    EXPECT_EQ(NULL, addToProfileSeekMap(NULL));
}

TEST(ADDTOPROFILESEEK, PROFILENM_INVALID)
{
   GrepSeekProfile* gp = NULL;
   EXPECT_NE(gp, addToProfileSeekMap("RDKB_INVALID_PROFILE1"));
}

TEST(GETLOGSEEKMAP, PROFILENM_NULL)
{
   EXPECT_EQ(NULL, getLogSeekMapForProfile(NULL));
}

TEST(GETLOGSEEKMAP, PROFILENM_INVALID)
{
   GrepSeekProfile *gp = NULL;
   EXPECT_NE(gp, getLogSeekMapForProfile("RDKB_INVALID_PROFILE1"));
}

TEST(UPDATELOGSEEK, LOGSEEKMAP_NULL)
{
   char* logFileName = "t2_log.txt";
   EXPECT_EQ(T2ERROR_FAILURE,  updateLogSeek(NULL, logFileName));
}

TEST(UPDATELOGSEEK, LOGSEEKMAP_NOT_NULL)
{
   char* logFileName = "t2_log.txt";
   hash_map_t* logseekmap = (hash_map_t *)malloc(sizeof(hash_map_t));
   logseekmap = hash_map_create();
   int *temp = (int *) malloc(sizeof(int));
   *temp = 1;
   hash_map_put(logseekmap, strdup(logFileName), (void*)temp, free);
   *temp = 2;
   hash_map_put(logseekmap, strdup("t2_log.txt.0"), (void*)temp, free);
   *temp = 3;
   hash_map_put(logseekmap, strdup("core_log.txt"), (void*)temp, free);
   EXPECT_EQ(T2ERROR_FAILURE,  updateLogSeek(logseekmap, NULL));
   EXPECT_EQ(T2ERROR_SUCCESS,  updateLogSeek(logseekmap, logFileName));
   hash_map_destroy(logseekmap, free);
   logseekmap = NULL;
}

TEST(GETLOGSEEKMAPFORPROFILE, PROFILE_NULL)
{
   EXPECT_EQ(NULL, getLogSeekMapForProfile(NULL));
}
TEST(GETLOGSEEKMAPFORPROFILE, PROFILE_NOT_NULL)
{
   GrepSeekProfile *gp = NULL;
   EXPECT_EQ(gp, getLogSeekMapForProfile("RDKB_PROFILE"));
}

TEST(GETLOADAVG, VECTOR_REGEX_NULL)
{
    EXPECT_EQ(0, getLoadAvg(NULL, false, NULL));
    EXPECT_EQ(0, getLoadAvg(NULL, true, NULL));
    EXPECT_EQ(0, getLoadAvg(NULL, false, "[0-9]"));
    EXPECT_EQ(0, getLoadAvg(NULL, true, "[0-9]"));
}

TEST(GETLOGLINE, NULL_CASES)
{
    char* logFileName = "t2_log.txt";
    hash_map_t* logseekmap = (hash_map_t *)malloc(sizeof(hash_map_t));
    logseekmap = hash_map_create();
    char *buf = (char*) malloc(512);
    char* name = "core_log.txt";
    int *temp = (int *) malloc(sizeof(int));
    *temp = 1;
    hash_map_put(logseekmap, strdup(logFileName), (void*)temp, free);
    *temp = 2;
    hash_map_put(logseekmap, strdup("t2_log.txt.0"), (void*)temp, free);
    *temp = 3;
    hash_map_put(logseekmap, strdup("core_log.txt"), (void*)temp, free);
    int seekFromEOF = 0;
    EXPECT_EQ(NULL, getLogLine(NULL, buf, 512, name, &seekFromEOF, false));
    EXPECT_EQ(NULL, getLogLine(logseekmap, NULL, 512, name, &seekFromEOF, false));
    EXPECT_EQ(NULL, getLogLine(logseekmap, buf, 512, NULL, &seekFromEOF, false));
    EXPECT_EQ(NULL, getLogLine(NULL, buf, 512, name, &seekFromEOF, true));
    EXPECT_EQ(NULL, getLogLine(logseekmap, NULL, 512, name, &seekFromEOF, true));
    EXPECT_EQ(NULL, getLogLine(logseekmap, buf, 512, NULL, &seekFromEOF, true));

    free(buf);
    buf = NULL;
    hash_map_destroy(logseekmap, free);
    logseekmap = NULL;
}

TEST(ISPROPSINITIALIZED, TESTING)
{
    EXPECT_EQ(false, isPropsInitialized());
}

//dca.c
TEST(PROCESSTOPPATTERN, VECTOR_NULL)
{
    rdkList_t *rdkListHndl = NULL;
    rdkList_t *tlist = (rdkList_t*) malloc (sizeof(rdkList_t));
    tlist->m_pUserData = (void*)"test1";
    tlist->m_pForward = NULL;
    tlist->m_pBackward = NULL;
    rdk_list_add_node(rdkListHndl, tlist->m_pUserData);
    Vector* grepResultlist = NULL;
    Vector_Create(&grepResultlist);
    Vector_PushBack(grepResultlist, (void*) strdup("SYS_INFO_BOOTUP"));
    Vector_PushBack(grepResultlist, (void*) strdup("SYS_INFO_MEM"));
    EXPECT_EQ(-1, processTopPattern(NULL, grepResultlist));
    EXPECT_EQ(-1, processTopPattern(rdkListHndl, NULL));
    EXPECT_EQ(-1, processTopPattern(rdkListHndl, grepResultlist));
    rdk_list_free_all_nodes(rdkListHndl);
    Vector_Destroy(grepResultlist, free);
    grepResultlist = NULL;
}

TEST(GETERRORCODE, STR_NOT_NULL)
{
    char* str = "telemetry-dcautil";
    EXPECT_EQ(0, getErrorCode(str, NULL));
}

TEST(GETERRORCODE, STR_NULL)
{
   EXPECT_EQ(-1, getErrorCode(NULL, NULL));
} 

TEST(strSplit, STR_NULL)
{
   EXPECT_EQ(NULL, strSplit(NULL, "<#>"));
}
/*
TEST(strSplit, STR_NOT_NULL)
{
  char* info = "info";
  EXPECT_EQ(info, strSplit("telemetry_<#>info","<#>"));
}
*/
TEST(getDCAResultsInJson, markerlist_NULL)
{
    char* profileName = "Profile1";
    cJSON* grepResultList = NULL;
    EXPECT_EQ(-1, getDCAResultsInJson(profileName, NULL, &grepResultList));
}

TEST(getDCAResultsInVector, markerlist_NULL)
{
    char* profileName = "Profile1";
    Vector* grepResultList = NULL;
    Vector* markerlist = NULL;
    Vector_Create(&markerlist);
    Vector_PushBack(markerlist, (void*) strdup("SYS_INFO_BOOTUP"));
    EXPECT_EQ(-1, getDCAResultsInVector(profileName, NULL, &grepResultList, true, "/opt/logs"));
}



//dcautil.c
TEST_F(dcaSystemTestFixture, firstBootstatus){
    EXPECT_CALL(*g_SystemMock, access(_,_))
            .Times(1)
            .WillOnce(Return(-1));

    firstBootStatus();
    g_SystemMock = nullptr;
}

TEST_F(dcaFileTestFixture, dcaFlagReportCompleation)
{
    FILE* fp = (FILE*)NULL;
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    dcaFlagReportCompleation();
    g_fileIOMock = nullptr;
}

TEST_F(dcaFileTestFixture, dcaFlagReportCompleation1)
{
    FILE* fp = (FILE*)0xffffffff;
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    EXPECT_CALL(*g_fileIOMock, fclose(_))
            .Times(1)
            .WillOnce(Return(0));
    dcaFlagReportCompleation();
    g_fileIOMock = nullptr;
}

#ifdef PERSIST_LOG_MON_RE
TEST_F(dcaFileTestFixture, loadSavedSeekConfig)
{
    FILE* fp = (FILE*)NULL;
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    EXPECT_EQ(T2ERROR_FAILURE, loadSavedSeekConfig("RDK_Profile"));
    g_fileIOMock = nullptr;
    
}

TEST_F(dcaFileTestFixture, loadSavedSeekConfig1)
{
    FILE* fp = (FILE*)0xffffffff;
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    EXPECT_CALL(*g_fileIOMock, fseek(_,_,_))
            .Times(1)
            .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, ftell(_))
            .Times(1)
            .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, fseek(_,_,_))
            .Times(1)
            .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, fread(_,_,_,_))
            .Times(1)
            .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, fclose(_))
            .Times(1)
            .WillOnce(Return(0));
    
    EXPECT_EQ(T2ERROR_SUCCESS, loadSavedSeekConfig("RDK_Profile"));
    g_fileIOMock = nullptr;
}
#endif

//dcautil.c
TEST_F(dcaFileTestFixture, getProcUsage)
{
    Vector* grepResultList = NULL;
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
    EXPECT_EQ(0, getProcUsage("telemetry2_0", grepResultList, true, "[0-9]"));
    g_fileIOMock = nullptr;
}

TEST_F(dcaFileTestFixture, getProcUsage1)
{
    Vector* grepResultList = NULL;
    Vector_Create(&grepResultList);
    Vector_PushBack(grepResultList, (void*) strdup("SYS_INFO_BOOTUP"));
    Vector_PushBack(grepResultList, (void*) strdup("SYS_INFO_MEM"));
    FILE* fp = (FILE*)0xffffffff;
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
    EXPECT_EQ(0, getProcUsage("telemetry2_0", grepResultList, true, "[0-9]"));
    g_fileIOMock = nullptr;
}

TEST_F(dcaFileTestFixture, getProcUsage2)
{
    Vector* grepResultList = NULL;
    Vector_Create(&grepResultList);
    Vector_PushBack(grepResultList, (void*) strdup("SYS_INFO_BOOTUP"));
    Vector_PushBack(grepResultList, (void*) strdup("SYS_INFO_MEM"));
    FILE* fp = (FILE*)0xffffffff;
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
    EXPECT_EQ(0, getProcUsage("telemetry2_0", grepResultList, true, "[0-9]"));
    g_fileIOMock = nullptr;
}

TEST_F(dcaFileTestFixture, getProcPidStat)
{
    procinfo pinfo;
    int fd = (int)0xffffffff;
    EXPECT_CALL(*g_fileIOMock, open(_,_))
            .Times(1)
            .WillOnce(Return(-1));
    ASSERT_EQ(0, getProcPidStat(123, &pinfo));
    g_fileIOMock = nullptr;
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
    g_fileIOMock = nullptr;
}

#if !defined(ENABLE_RDKC_SUPPORT) && !defined(ENABLE_RDKB_SUPPORT)
TEST_F(dcaSystemTestFixture, saveTopOutput)
{
    EXPECT_CALL(*g_SystemMock, access(_,_))
                .Times(1)
                .WillOnce(Return(0));
    saveTopOutput();
    g_SystemMock = nullptr;
}

TEST_F(dcaSystemTestFixture, saveTopOutput1)
{
    EXPECT_CALL(*g_SystemMock, access(_,_))
                .Times(1)
                .WillOnce(Return(-1));
    #ifdef LIBSYSWRAPPER_BUILD
       EXPECT_CALL(*g_SystemMock, v_secure_system(_))
                .Times(2)
                .WillOnce(Return(-1))
                .WillOnce(Return(-1));
    #else
       EXPECT_CALL(*g_SystemMock, system(_))
                .Times(2)
                .WillOnce(Return(-1))
                .WillOnce(Return(-1));
    #endif
    saveTopOutput();
    g_SystemMock = nullptr;
}

TEST_F(dcaSystemTestFixture, saveTopOutput2)
{
    EXPECT_CALL(*g_SystemMock, access(_,_))
                .Times(1)
                .WillOnce(Return(-1));
    #ifdef LIBSYSWRAPPER_BUILD
       EXPECT_CALL(*g_SystemMock, v_secure_system(_))
                .Times(2)
                .WillOnce(Return(0))
                .WillOnce(Return(-1));
    #else
       EXPECT_CALL(*g_SystemMock, system(_))
                .Times(2)
                .WillOnce(Return(0))
                .WillOnce(Return(-1));
    #endif
    saveTopOutput();
    g_SystemMock = nullptr;
}

TEST_F(dcaSystemTestFixture, removeTopOutput)
{
    #ifdef LIBSYSWRAPPER_BUILD
    EXPECT_CALL(*g_SystemMock, v_secure_system(_))
                .Times(1)
                .WillOnce(Return(-1));
    #else
    EXPECT_CALL(*g_SystemMock, system(_))
                .Times(1)
                .WillOnce(Return(-1));
    #endif
    removeTopOutput();
    g_SystemMock = nullptr;
}

TEST_F(dcaSystemTestFixture, removeTopOutput1)
{
    #ifdef LIBSYSWRAPPER_BUILD
    EXPECT_CALL(*g_SystemMock, v_secure_system(_))
                .Times(1)
                .WillOnce(Return(0));
    #else
    EXPECT_CALL(*g_SystemMock, system(_))
                .Times(1)
                .WillOnce(Return(0));
    #endif
    removeTopOutput();
    g_SystemMock = nullptr;
}

#else
TEST_F(dcaFileTestFixture, getTotalCpuTimes)
{
    FILE* mockfp = (FILE *)NULL;
    int *totaltime = 0;
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(mockfp));
    EXPECT_EQ(0, getTotalCpuTimes(totaltime));
    g_fileIOMock = nullptr;
}

TEST_F(dcaFileTestFixture, getTotalCpuTimes1)
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
    g_fileIOMock = nullptr;
}
#endif

TEST_F(dcaFileTestFixture,  getProcUsage4)
{
    Vector* gresulist = NULL;
    Vector_Create(&gresulist);
    char* processName = "telemetry2_0";
    FILE* fp = (FILE*)NULL;
    EXPECT_CALL(*g_fileIOMock, popen(_,_))
            .Times(1)
            .WillOnce(Return(fp));
    EXPECT_EQ(0, getProcUsage(processName, gresulist, true, "[0-9]+"));
    g_fileIOMock = nullptr;
}

TEST_F(dcaFileTestFixture,  getProcUsage3)
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
   g_fileIOMock = nullptr;
}

TEST_F(dcaFileTestFixture,  getProcUsage5)
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
   g_fileIOMock = nullptr;
}

TEST_F(dcaFileTestFixture, getLoadAvg)
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
    g_fileIOMock = nullptr;
}

TEST_F(dcaFileTestFixture, getLoadAvg1)
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
    Vector_Destroy(grepResultList, free);
    g_fileIOMock = nullptr;
}

TEST_F(dcaFileTestFixture, getLoadAvg2)
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
    Vector_Destroy(grepResultList, free);
    g_fileIOMock = nullptr;
}

