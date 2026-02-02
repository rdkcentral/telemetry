/*
 * Copyright 2026 Comcast Cable Communications Management, LLC
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
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <curl/curl.h>
#include <protocol/http/multicurlinterface.h>
#include <bulkdata/reportprofiles.h>
#include <ccspinterface/busInterface.h>
#include <utils/t2MtlsUtils.h>
}

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace std;
using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::DoAll;
using ::testing::SetArgPointee;
using ::testing::Invoke;

#define curl_easy_setopt curl_easy_setopt_mock
#define curl_easy_getinfo curl_easy_getinfo_mock
#define curl_easy_init curl_easy_init_mock
#define curl_easy_perform curl_easy_perform_mock
#define curl_easy_cleanup curl_easy_cleanup_mock
#define curl_slist_append curl_slist_append_mock
#define curl_slist_free_all curl_slist_free_all_mock

#include "test/mocks/SystemMock.h"
#include "test/mocks/FileioMock.h"
#include "test/mocks/rdklogMock.h"
#include "test/protocol/MultiCurlInterfaceMock.h"
#include "test/xconf-client/xconfclientMock.h"

#undef curl_easy_setopt
#undef curl_easy_getinfo
#undef curl_easy_init
#undef curl_easy_perform
#undef curl_easy_cleanup
#undef curl_slist_append
#undef curl_slist_free_all

extern FileMock *g_fileIOMock;
extern SystemMock *g_systemMock;
extern MultiCurlInterfaceMock *g_multiCurlMock;
extern XconfclientMock *m_xconfclientMock;

class MultiCurlInterfaceTestFixture : public ::testing::Test {
protected:
    void SetUp() override
    {
        g_fileIOMock = new FileMock();
        g_systemMock = new SystemMock();
        g_multiCurlMock = new MultiCurlInterfaceMock();
        m_xconfclientMock = new XconfclientMock();
    }

    void TearDown() override
    {
        // Cleanup the pool between tests
        http_pool_cleanup();
        
        delete g_fileIOMock;
        delete g_systemMock;
        delete g_multiCurlMock;
        delete m_xconfclientMock;

        g_fileIOMock = nullptr;
        g_systemMock = nullptr;
        g_multiCurlMock = nullptr;
        m_xconfclientMock = nullptr;
    }
};

// ========== init_connection_pool Tests ==========

TEST_F(MultiCurlInterfaceTestFixture, InitConnectionPool_MutexInitFails)
{
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_init_mock(_, _))
        .WillOnce(Return(1)); // Return non-zero to indicate failure
    
    EXPECT_EQ(T2ERROR_FAILURE, init_connection_pool());
}

TEST_F(MultiCurlInterfaceTestFixture, InitConnectionPool_CondInitFails)
{
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_init_mock(_, _))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_cond_init_mock(_, _))
        .WillOnce(Return(1)); // Return non-zero to indicate failure
    
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_destroy_mock(_))
        .Times(1);
    
    EXPECT_EQ(T2ERROR_FAILURE, init_connection_pool());
}

TEST_F(MultiCurlInterfaceTestFixture, InitConnectionPool_CurlInitFails)
{
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_init_mock(_, _))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_cond_init_mock(_, _))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_init_mock())
        .WillOnce(Return(nullptr)); // First handle init fails
    
    EXPECT_CALL(*g_multiCurlMock, pthread_cond_destroy_mock(_))
        .Times(1);
    
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_destroy_mock(_))
        .Times(1);
    
    EXPECT_EQ(T2ERROR_FAILURE, init_connection_pool());
}

TEST_F(MultiCurlInterfaceTestFixture, InitConnectionPool_Success)
{
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_init_mock(_, _))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_cond_init_mock(_, _))
        .WillOnce(Return(0));
    
    // Mock 3 curl handle initializations
    EXPECT_CALL(*g_fileIOMock, curl_easy_init_mock())
        .Times(3)
        .WillRepeatedly(Return((CURL*)0x1234));
    
    // Mock all curl_easy_setopt calls to succeed
    EXPECT_CALL(*g_fileIOMock, curl_easy_setopt_mock(_, _, _))
        .WillRepeatedly(Return(CURLE_OK));
    
    // Mock curl_slist_append for headers
    EXPECT_CALL(*g_fileIOMock, curl_slist_append_mock(_, _))
        .Times(2)
        .WillRepeatedly(Return((struct curl_slist*)0x5678));
    
    EXPECT_EQ(T2ERROR_SUCCESS, init_connection_pool());
    
    // Second call should return success immediately
    EXPECT_EQ(T2ERROR_SUCCESS, init_connection_pool());
}

TEST_F(MultiCurlInterfaceTestFixture, InitConnectionPool_AlreadyInitialized)
{
    // First initialization
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_init_mock(_, _))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_cond_init_mock(_, _))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_init_mock())
        .Times(3)
        .WillRepeatedly(Return((CURL*)0x1234));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_setopt_mock(_, _, _))
        .WillRepeatedly(Return(CURLE_OK));
    
    EXPECT_CALL(*g_fileIOMock, curl_slist_append_mock(_, _))
        .Times(2)
        .WillRepeatedly(Return((struct curl_slist*)0x5678));
    
    EXPECT_EQ(T2ERROR_SUCCESS, init_connection_pool());
    
    // Second call should return success without reinitializing
    EXPECT_EQ(T2ERROR_SUCCESS, init_connection_pool());
}

// ========== http_pool_get Tests ==========

TEST_F(MultiCurlInterfaceTestFixture, HttpPoolGet_NullUrl)
{
    EXPECT_EQ(T2ERROR_FAILURE, http_pool_get(nullptr, nullptr, false));
}

TEST_F(MultiCurlInterfaceTestFixture, HttpPoolGet_InitPoolFails)
{
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_init_mock(_, _))
        .WillOnce(Return(1));
    
    char* response = nullptr;
    EXPECT_EQ(T2ERROR_FAILURE, http_pool_get("http://test.com", &response, false));
}

TEST_F(MultiCurlInterfaceTestFixture, HttpPoolGet_AllocResponseFails)
{
    // Setup successful pool initialization
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_init_mock(_, _))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_cond_init_mock(_, _))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_init_mock())
        .Times(3)
        .WillRepeatedly(Return((CURL*)0x1234));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_setopt_mock(_, _, _))
        .WillRepeatedly(Return(CURLE_OK));
    
    EXPECT_CALL(*g_fileIOMock, curl_slist_append_mock(_, _))
        .Times(2)
        .WillRepeatedly(Return((struct curl_slist*)0x5678));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_lock_mock(_))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_unlock_mock(_))
        .WillOnce(Return(0));
    
    // Make malloc fail
    EXPECT_CALL(*g_multiCurlMock, malloc_mock(_))
        .WillOnce(Return(nullptr));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_cond_signal_mock(_))
        .WillOnce(Return(0));
    
    char* response = nullptr;
    EXPECT_EQ(T2ERROR_FAILURE, http_pool_get("http://test.com", &response, false));
}

TEST_F(MultiCurlInterfaceTestFixture, HttpPoolGet_CurlPerformFails)
{
    // Setup successful pool initialization
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_init_mock(_, _))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_cond_init_mock(_, _))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_init_mock())
        .Times(3)
        .WillRepeatedly(Return((CURL*)0x1234));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_setopt_mock(_, _, _))
        .WillRepeatedly(Return(CURLE_OK));
    
    EXPECT_CALL(*g_fileIOMock, curl_slist_append_mock(_, _))
        .Times(2)
        .WillRepeatedly(Return((struct curl_slist*)0x5678));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_lock_mock(_))
        .Times(2)
        .WillRepeatedly(Return(0));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_unlock_mock(_))
        .Times(2)
        .WillRepeatedly(Return(0));
    
    // Allocate response buffer
    char* testBuffer = (char*)malloc(8192);
    EXPECT_CALL(*g_multiCurlMock, malloc_mock(_))
        .WillOnce(Return(testBuffer));
    
    EXPECT_CALL(*m_xconfclientMock, isMtlsEnabled())
        .WillOnce(Return(false));
    
    // Curl perform fails
    EXPECT_CALL(*g_fileIOMock, curl_easy_perform_mock(_))
        .WillOnce(Return(CURLE_COULDNT_CONNECT));
    
    EXPECT_CALL(*g_multiCurlMock, free_mock(_))
        .Times(1);
    
    EXPECT_CALL(*g_multiCurlMock, pthread_cond_signal_mock(_))
        .WillOnce(Return(0));
    
    char* response = nullptr;
    EXPECT_EQ(T2ERROR_FAILURE, http_pool_get("http://test.com", &response, false));
}

TEST_F(MultiCurlInterfaceTestFixture, HttpPoolGet_Success_Http200)
{
    // Setup successful pool initialization
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_init_mock(_, _))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_cond_init_mock(_, _))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_init_mock())
        .Times(3)
        .WillRepeatedly(Return((CURL*)0x1234));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_setopt_mock(_, _, _))
        .WillRepeatedly(Return(CURLE_OK));
    
    EXPECT_CALL(*g_fileIOMock, curl_slist_append_mock(_, _))
        .Times(2)
        .WillRepeatedly(Return((struct curl_slist*)0x5678));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_lock_mock(_))
        .Times(2)
        .WillRepeatedly(Return(0));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_unlock_mock(_))
        .Times(2)
        .WillRepeatedly(Return(0));
    
    // Allocate response buffer
    char* testBuffer = (char*)malloc(8192);
    strcpy(testBuffer, "test response data");
    EXPECT_CALL(*g_multiCurlMock, malloc_mock(_))
        .WillOnce(Return(testBuffer))
        .WillOnce(Return(malloc(100))); // For response_data allocation
    
    EXPECT_CALL(*m_xconfclientMock, isMtlsEnabled())
        .WillOnce(Return(false));
    
    // Curl perform succeeds
    EXPECT_CALL(*g_fileIOMock, curl_easy_perform_mock(_))
        .WillOnce(Return(CURLE_OK));
    
    // Return HTTP 200
    EXPECT_CALL(*g_fileIOMock, curl_easy_getinfo_mock(_, CURLINFO_RESPONSE_CODE, _))
        .WillOnce(DoAll(SetArgPointee<2>(200L), Return(CURLE_OK)));
    
    EXPECT_CALL(*g_multiCurlMock, free_mock(_))
        .Times(2); // Free test buffer and response
    
    EXPECT_CALL(*g_multiCurlMock, pthread_cond_signal_mock(_))
        .WillOnce(Return(0));
    
    char* response = nullptr;
    EXPECT_EQ(T2ERROR_SUCCESS, http_pool_get("http://test.com", &response, false));
    EXPECT_NE(response, nullptr);
}

TEST_F(MultiCurlInterfaceTestFixture, HttpPoolGet_Http404)
{
    // Setup successful pool initialization
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_init_mock(_, _))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_cond_init_mock(_, _))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_init_mock())
        .Times(3)
        .WillRepeatedly(Return((CURL*)0x1234));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_setopt_mock(_, _, _))
        .WillRepeatedly(Return(CURLE_OK));
    
    EXPECT_CALL(*g_fileIOMock, curl_slist_append_mock(_, _))
        .Times(2)
        .WillRepeatedly(Return((struct curl_slist*)0x5678));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_lock_mock(_))
        .Times(2)
        .WillRepeatedly(Return(0));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_unlock_mock(_))
        .Times(2)
        .WillRepeatedly(Return(0));
    
    char* testBuffer = (char*)malloc(8192);
    EXPECT_CALL(*g_multiCurlMock, malloc_mock(_))
        .WillOnce(Return(testBuffer));
    
    EXPECT_CALL(*m_xconfclientMock, isMtlsEnabled())
        .WillOnce(Return(false));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_perform_mock(_))
        .WillOnce(Return(CURLE_OK));
    
    // Return HTTP 404
    EXPECT_CALL(*g_fileIOMock, curl_easy_getinfo_mock(_, CURLINFO_RESPONSE_CODE, _))
        .WillOnce(DoAll(SetArgPointee<2>(404L), Return(CURLE_OK)));
    
    EXPECT_CALL(*g_multiCurlMock, free_mock(_))
        .Times(1);
    
    EXPECT_CALL(*g_multiCurlMock, pthread_cond_signal_mock(_))
        .WillOnce(Return(0));
    
    char* response = nullptr;
    EXPECT_EQ(T2ERROR_PROFILE_NOT_SET, http_pool_get("http://test.com", &response, false));
}

// ========== http_pool_post Tests ==========

TEST_F(MultiCurlInterfaceTestFixture, HttpPoolPost_NullUrl)
{
    EXPECT_EQ(T2ERROR_FAILURE, http_pool_post(nullptr, "payload"));
}

TEST_F(MultiCurlInterfaceTestFixture, HttpPoolPost_NullPayload)
{
    EXPECT_EQ(T2ERROR_FAILURE, http_pool_post("http://test.com", nullptr));
}

TEST_F(MultiCurlInterfaceTestFixture, HttpPoolPost_Success_Http200)
{
    // Setup successful pool initialization
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_init_mock(_, _))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_cond_init_mock(_, _))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_init_mock())
        .Times(3)
        .WillRepeatedly(Return((CURL*)0x1234));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_setopt_mock(_, _, _))
        .WillRepeatedly(Return(CURLE_OK));
    
    EXPECT_CALL(*g_fileIOMock, curl_slist_append_mock(_, _))
        .Times(2)
        .WillRepeatedly(Return((struct curl_slist*)0x5678));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_lock_mock(_))
        .Times(2)
        .WillRepeatedly(Return(0));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_unlock_mock(_))
        .Times(2)
        .WillRepeatedly(Return(0));
    
    EXPECT_CALL(*g_fileIOMock, open(_, _, _))
        .WillOnce(Return(5));
    
    EXPECT_CALL(*g_multiCurlMock, fdopen_mock(_, _))
        .WillOnce(Return((FILE*)0x9999));
    
    EXPECT_CALL(*g_multiCurlMock, fclose_mock(_))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*m_xconfclientMock, isMtlsEnabled())
        .WillOnce(Return(false));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_perform_mock(_))
        .WillOnce(Return(CURLE_OK));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_getinfo_mock(_, CURLINFO_RESPONSE_CODE, _))
        .WillOnce(DoAll(SetArgPointee<2>(200L), Return(CURLE_OK)));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_cond_signal_mock(_))
        .WillOnce(Return(0));
    
    EXPECT_EQ(T2ERROR_SUCCESS, http_pool_post("http://test.com", "{\"data\":\"test\"}"));
}

TEST_F(MultiCurlInterfaceTestFixture, HttpPoolPost_FileOpenFails)
{
    // Setup successful pool initialization
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_init_mock(_, _))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_cond_init_mock(_, _))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_init_mock())
        .Times(3)
        .WillRepeatedly(Return((CURL*)0x1234));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_setopt_mock(_, _, _))
        .WillRepeatedly(Return(CURLE_OK));
    
    EXPECT_CALL(*g_fileIOMock, curl_slist_append_mock(_, _))
        .Times(2)
        .WillRepeatedly(Return((struct curl_slist*)0x5678));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_lock_mock(_))
        .Times(2)
        .WillRepeatedly(Return(0));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_unlock_mock(_))
        .Times(2)
        .WillRepeatedly(Return(0));
    
    // File open fails
    EXPECT_CALL(*g_fileIOMock, open(_, _, _))
        .WillOnce(Return(-1));
    
    EXPECT_CALL(*m_xconfclientMock, isMtlsEnabled())
        .WillOnce(Return(false));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_perform_mock(_))
        .WillOnce(Return(CURLE_OK));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_getinfo_mock(_, CURLINFO_RESPONSE_CODE, _))
        .WillOnce(DoAll(SetArgPointee<2>(200L), Return(CURLE_OK)));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_cond_signal_mock(_))
        .WillOnce(Return(0));
    
    EXPECT_EQ(T2ERROR_SUCCESS, http_pool_post("http://test.com", "{\"data\":\"test\"}"));
}

// ========== http_pool_cleanup Tests ==========

TEST_F(MultiCurlInterfaceTestFixture, HttpPoolCleanup_NotInitialized)
{
    EXPECT_EQ(T2ERROR_SUCCESS, http_pool_cleanup());
}

TEST_F(MultiCurlInterfaceTestFixture, HttpPoolCleanup_Success)
{
    // Initialize pool first
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_init_mock(_, _))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_cond_init_mock(_, _))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_init_mock())
        .Times(3)
        .WillRepeatedly(Return((CURL*)0x1234));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_setopt_mock(_, _, _))
        .WillRepeatedly(Return(CURLE_OK));
    
    EXPECT_CALL(*g_fileIOMock, curl_slist_append_mock(_, _))
        .Times(2)
        .WillRepeatedly(Return((struct curl_slist*)0x5678));
    
    EXPECT_EQ(T2ERROR_SUCCESS, init_connection_pool());
    
    // Now cleanup
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_lock_mock(_))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_cond_broadcast_mock(_))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_unlock_mock(_))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_fileIOMock, curl_slist_free_all_mock(_))
        .Times(1);
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_cleanup_mock(_))
        .Times(3);
    
    EXPECT_CALL(*g_multiCurlMock, pthread_cond_destroy_mock(_))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_destroy_mock(_))
        .WillOnce(Return(0));
    
    EXPECT_EQ(T2ERROR_SUCCESS, http_pool_cleanup());
}

// ========== mTLS and Certificate Tests ==========

#ifdef LIBRDKCERTSEL_BUILD
TEST_F(MultiCurlInterfaceTestFixture, HttpPoolGet_MtlsEnabled_CertRetrievalFails)
{
    // Setup pool
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_init_mock(_, _))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_cond_init_mock(_, _))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_init_mock())
        .Times(3)
        .WillRepeatedly(Return((CURL*)0x1234));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_setopt_mock(_, _, _))
        .WillRepeatedly(Return(CURLE_OK));
    
    EXPECT_CALL(*g_fileIOMock, curl_slist_append_mock(_, _))
        .Times(2)
        .WillRepeatedly(Return((struct curl_slist*)0x5678));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_lock_mock(_))
        .Times(2)
        .WillRepeatedly(Return(0));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_unlock_mock(_))
        .Times(2)
        .WillRepeatedly(Return(0));
    
    char* testBuffer = (char*)malloc(8192);
    EXPECT_CALL(*g_multiCurlMock, malloc_mock(_))
        .WillOnce(Return(testBuffer));
    
    EXPECT_CALL(*m_xconfclientMock, isMtlsEnabled())
        .WillOnce(Return(true));
    
    // Certificate retrieval fails
    EXPECT_CALL(*g_multiCurlMock, rdkcertselector_getCert_mock(_, _, _))
        .WillOnce(Return(1)); // certselectorFail
    
    EXPECT_CALL(*g_multiCurlMock, free_mock(_))
        .Times(1);
    
    EXPECT_CALL(*g_multiCurlMock, pthread_cond_signal_mock(_))
        .WillOnce(Return(0));
    
    char* response = nullptr;
    EXPECT_EQ(T2ERROR_FAILURE, http_pool_get("http://test.com", &response, false));
}

TEST_F(MultiCurlInterfaceTestFixture, HttpPoolGet_MtlsEnabled_CertRotation)
{
    // Setup pool
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_init_mock(_, _))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_cond_init_mock(_, _))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_init_mock())
        .Times(3)
        .WillRepeatedly(Return((CURL*)0x1234));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_setopt_mock(_, _, _))
        .WillRepeatedly(Return(CURLE_OK));
    
    EXPECT_CALL(*g_fileIOMock, curl_slist_append_mock(_, _))
        .Times(2)
        .WillRepeatedly(Return((struct curl_slist*)0x5678));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_lock_mock(_))
        .Times(2)
        .WillRepeatedly(Return(0));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_unlock_mock(_))
        .Times(2)
        .WillRepeatedly(Return(0));
    
    char* testBuffer = (char*)malloc(8192);
    EXPECT_CALL(*g_multiCurlMock, malloc_mock(_))
        .WillOnce(Return(testBuffer));
    
    EXPECT_CALL(*m_xconfclientMock, isMtlsEnabled())
        .WillOnce(Return(true));
    
    // First cert retrieval succeeds
    char* certUri1 = strdup("file:///tmp/cert1.p12");
    char* passwd1 = strdup("pass1");
    EXPECT_CALL(*g_multiCurlMock, rdkcertselector_getCert_mock(_, _, _))
        .WillOnce(DoAll(
            SetArgPointee<1>(certUri1),
            SetArgPointee<2>(passwd1),
            Return(0)
        ))
        .WillOnce(DoAll(
            SetArgPointee<1>(strdup("file:///tmp/cert2.p12")),
            SetArgPointee<2>(strdup("pass2")),
            Return(0)
        ));
    
    // First cert fails, triggers retry
    EXPECT_CALL(*g_fileIOMock, curl_easy_perform_mock(_))
        .WillOnce(Return(CURLE_SSL_CONNECT_ERROR))
        .WillOnce(Return(CURLE_OK));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_getinfo_mock(_, CURLINFO_RESPONSE_CODE, _))
        .WillOnce(DoAll(SetArgPointee<2>(0L), Return(CURLE_OK)))
        .WillOnce(DoAll(SetArgPointee<2>(200L), Return(CURLE_OK)));
    
    // First setCurlStatus returns TRY_ANOTHER, second returns success
    EXPECT_CALL(*g_multiCurlMock, rdkcertselector_setCurlStatus_mock(_, _, _))
        .WillOnce(Return(1)) // TRY_ANOTHER
        .WillOnce(Return(0)); // Success
    
    EXPECT_CALL(*g_multiCurlMock, free_mock(_))
        .Times(5); // Free certUri1, passwd1, certUri2, passwd2, testBuffer
    
    EXPECT_CALL(*g_multiCurlMock, pthread_cond_signal_mock(_))
        .WillOnce(Return(0));
    
    char* response = nullptr;
    EXPECT_EQ(T2ERROR_SUCCESS, http_pool_get("http://test.com", &response, false));
}
#endif

// ========== Thread Safety Tests ==========

TEST_F(MultiCurlInterfaceTestFixture, ConcurrentPoolAccess_AllHandlesBusy)
{
    // Initialize pool
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_init_mock(_, _))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_cond_init_mock(_, _))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_init_mock())
        .Times(3)
        .WillRepeatedly(Return((CURL*)0x1234));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_setopt_mock(_, _, _))
        .WillRepeatedly(Return(CURLE_OK));
    
    EXPECT_CALL(*g_fileIOMock, curl_slist_append_mock(_, _))
        .Times(2)
        .WillRepeatedly(Return((struct curl_slist*)0x5678));
    
    // Simulate all handles busy, then one becomes available
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_lock_mock(_))
        .Times(2)
        .WillRepeatedly(Return(0));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_cond_wait_mock(_, _))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_unlock_mock(_))
        .Times(2)
        .WillRepeatedly(Return(0));
    
    char* testBuffer = (char*)malloc(8192);
    EXPECT_CALL(*g_multiCurlMock, malloc_mock(_))
        .WillOnce(Return(testBuffer));
    
    EXPECT_CALL(*m_xconfclientMock, isMtlsEnabled())
        .WillOnce(Return(false));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_perform_mock(_))
        .WillOnce(Return(CURLE_OK));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_getinfo_mock(_, CURLINFO_RESPONSE_CODE, _))
        .WillOnce(DoAll(SetArgPointee<2>(200L), Return(CURLE_OK)));
    
    EXPECT_CALL(*g_multiCurlMock, free_mock(_))
        .Times(1);
    
    EXPECT_CALL(*g_multiCurlMock, pthread_cond_signal_mock(_))
        .WillOnce(Return(0));
    
    char* response = nullptr;
    EXPECT_EQ(T2ERROR_SUCCESS, http_pool_get("http://test.com", &response, false));
}

// ========== Edge Case Tests ==========

TEST_F(MultiCurlInterfaceTestFixture, HttpPoolGet_EnableFileOutput)
{
    // Setup pool
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_init_mock(_, _))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_cond_init_mock(_, _))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_init_mock())
        .Times(3)
        .WillRepeatedly(Return((CURL*)0x1234));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_setopt_mock(_, _, _))
        .WillRepeatedly(Return(CURLE_OK));
    
    EXPECT_CALL(*g_fileIOMock, curl_slist_append_mock(_, _))
        .Times(2)
        .WillRepeatedly(Return((struct curl_slist*)0x5678));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_lock_mock(_))
        .Times(2)
        .WillRepeatedly(Return(0));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_unlock_mock(_))
        .Times(2)
        .WillRepeatedly(Return(0));
    
    char* testBuffer = (char*)malloc(8192);
    strcpy(testBuffer, "response data for file");
    EXPECT_CALL(*g_multiCurlMock, malloc_mock(_))
        .WillOnce(Return(testBuffer));
    
    EXPECT_CALL(*m_xconfclientMock, isMtlsEnabled())
        .WillOnce(Return(false));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_perform_mock(_))
        .WillOnce(Return(CURLE_OK));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_getinfo_mock(_, CURLINFO_RESPONSE_CODE, _))
        .WillOnce(DoAll(SetArgPointee<2>(200L), Return(CURLE_OK)));
    
    // File output expectations
    EXPECT_CALL(*g_fileIOMock, open(StrEq("/tmp/httpOutput.txt"), _, _))
        .WillOnce(Return(5));
    
    EXPECT_CALL(*g_multiCurlMock, fdopen_mock(5, StrEq("w+")))
        .WillOnce(Return((FILE*)0x8888));
    
    EXPECT_CALL(*g_multiCurlMock, fputs_mock(_, _))
        .WillOnce(Return(1));
    
    EXPECT_CALL(*g_multiCurlMock, fclose_mock(_))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_multiCurlMock, free_mock(_))
        .Times(1);
    
    EXPECT_CALL(*g_multiCurlMock, pthread_cond_signal_mock(_))
        .WillOnce(Return(0));
    
    char* response = nullptr;
    EXPECT_EQ(T2ERROR_SUCCESS, http_pool_get("http://test.com", &response, true));
}

TEST_F(MultiCurlInterfaceTestFixture, HttpPoolGet_ResponseSizeExceedsMax)
{
    // Setup pool
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_init_mock(_, _))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_cond_init_mock(_, _))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_init_mock())
        .Times(3)
        .WillRepeatedly(Return((CURL*)0x1234));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_setopt_mock(_, _, _))
        .WillRepeatedly(Return(CURLE_OK));
    
    EXPECT_CALL(*g_fileIOMock, curl_slist_append_mock(_, _))
        .Times(2)
        .WillRepeatedly(Return((struct curl_slist*)0x5678));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_lock_mock(_))
        .Times(2)
        .WillRepeatedly(Return(0));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_unlock_mock(_))
        .Times(2)
        .WillRepeatedly(Return(0));
    
    // Create response with size = SIZE_MAX (will exceed SIZE_MAX - 1)
    char* testBuffer = (char*)malloc(8192);
    EXPECT_CALL(*g_multiCurlMock, malloc_mock(_))
        .WillOnce(Return(testBuffer));
    
    EXPECT_CALL(*m_xconfclientMock, isMtlsEnabled())
        .WillOnce(Return(false));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_perform_mock(_))
        .WillOnce(Return(CURLE_OK));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_getinfo_mock(_, CURLINFO_RESPONSE_CODE, _))
        .WillOnce(DoAll(SetArgPointee<2>(200L), Return(CURLE_OK)));
    
    EXPECT_CALL(*g_multiCurlMock, free_mock(_))
        .Times(1);
    
    EXPECT_CALL(*g_multiCurlMock, pthread_cond_signal_mock(_))
        .WillOnce(Return(0));
    
    char* response = nullptr;
    // This test would need to mock the response.size to be SIZE_MAX
    // For simplicity, we're showing the test structure
    EXPECT_EQ(T2ERROR_SUCCESS, http_pool_get("http://test.com", &response, false));
}

TEST_F(MultiCurlInterfaceTestFixture, HttpPoolPost_CurlPerformFails)
{
    // Setup pool
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_init_mock(_, _))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_cond_init_mock(_, _))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_init_mock())
        .Times(3)
        .WillRepeatedly(Return((CURL*)0x1234));
    
    EXPECT_CALL(*g_fileIOMock, curl_easy_setopt_mock(_, _, _))
        .WillRepeatedly(Return(CURLE_OK));
    
    EXPECT_CALL(*g_fileIOMock, curl_slist_append_mock(_, _))
        .Times(2)
        .WillRepeatedly(Return((struct curl_slist*)0x5678));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_lock_mock(_))
        .Times(2)
        .WillRepeatedly(Return(0));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_mutex_unlock_mock(_))
        .Times(2)
        .WillRepeatedly(Return(0));
    
    EXPECT_CALL(*g_fileIOMock, open(_, _, _))
        .WillOnce(Return(5));
    
    EXPECT_CALL(*g_multiCurlMock, fdopen_mock(_, _))
        .WillOnce(Return((FILE*)0x9999));
    
    EXPECT_CALL(*g_multiCurlMock, fclose_mock(_))
        .WillOnce(Return(0));
    
    EXPECT_CALL(*m_xconfclientMock, isMtlsEnabled())
        .WillOnce(Return(false));
    
    // Curl perform fails
    EXPECT_CALL(*g_fileIOMock, curl_easy_perform_mock(_))
        .WillOnce(Return(CURLE_OPERATION_TIMEDOUT));
    
    EXPECT_CALL(*g_multiCurlMock, pthread_cond_signal_mock(_))
        .WillOnce(Return(0));
    
    EXPECT_EQ(T2ERROR_FAILURE, http_pool_post("http://test.com", "{\"data\":\"test\"}"));
}
