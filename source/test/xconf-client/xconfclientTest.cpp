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
#include "test/mocks/rbusMock.h"
#include "xconfclientMock.h"

extern "C" {
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include "telemetry2_0.h"
#include "xconf-client/xconfclient.h"
#include "utils/t2MtlsUtils.h"
#include "t2parser/t2parserxconf.h"
#include "utils/vector.h"
#include "utils/persistence.h"
#include "telemetry2_0.h"
#include "ccspinterface/busInterface.h"
sigset_t blocking_signal;
}

using namespace std;

using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::SetArgPointee;
using ::testing::DoAll;

XconfclientMock *m_xconfclientMock = NULL;
rbusMock *g_rbusMock = NULL;

TEST(GETBUILDTYPE, NULL_CHECK)
{
    EXPECT_EQ(T2ERROR_FAILURE, getBuildType(NULL));
}
/*
TEST(APPENDREQUEST, NULL_CHECK)
{
    char *buf = NULL;
    EXPECT_EQ(T2ERROR_FAILURE, appendRequestParams(buf,256));
    char* urlWithParams = (char*) malloc(1024 * sizeof(char));
     memset(urlWithParams, '0', 1024 * sizeof(char));
     snprintf(urlWithParams, 1024, "%s?", "https://mockxconf:50050/loguploader/getT2DCMSettings?");
    EXPECT_EQ(T2ERROR_FAILURE, appendRequestParams(urlWithParams,970));
    free(urlWithParams);
    urlWithParams = NULL;
}
*/

TEST(DOHTTPGET, HTTPURL_CHECK)
{
    char* data = NULL;
    EXPECT_EQ(T2ERROR_FAILURE,  doHttpGet(NULL, &data));
}

TEST(FETCHREMOTECONFIG, CONFIGUURL_NULL)
{
   char *configdata = NULL;
   EXPECT_EQ(T2ERROR_INVALID_ARGS, fetchRemoteConfiguration(NULL, &configdata));
}

class xconfclientTestFixture : public ::testing::Test {
protected:
    void SetUp() override
    {
        g_fileIOMock = new FileMock();
	g_systemMock = new SystemMock();
    m_xconfclientMock = new XconfclientMock();
    }

    void TearDown() override
    {
        delete g_fileIOMock;
	delete g_systemMock;
    delete m_xconfclientMock;

        g_fileIOMock = nullptr;
	g_systemMock = nullptr;
    m_xconfclientMock = nullptr;
    }
};


TEST_F(xconfclientTestFixture, getBuildType)
{
     FILE* mockfp = (FILE*)NULL;
     char build_type[BUILD_TYPE_MAX_LENGTH] = { 0 };
     EXPECT_CALL(*g_fileIOMock,  fopen(_,_))
            .Times(1)
            .WillOnce(Return(mockfp));
     EXPECT_EQ(T2ERROR_FAILURE, getBuildType(build_type));
}

TEST_F(xconfclientTestFixture, getBuildType1)
{
     FILE* fakeFile = reinterpret_cast<FILE*>(0x1234);
    char fileContent[] = "BUILD_TYPE=PROD";
    char build_type[256] = {0};

    // Expect fopen called with DEVICE_PROPERTIES and returns fakeFile
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
        .WillOnce(Return(fakeFile));

    EXPECT_CALL(*g_fileIOMock, fscanf(_, _, _))
     .WillOnce(::testing::Return(EOF));
    EXPECT_CALL(*g_fileIOMock, fclose(_))
            .Times(1)
            .WillOnce(Return(0));
     EXPECT_EQ(T2ERROR_FAILURE, getBuildType(build_type));
}

TEST_F(xconfclientTestFixture, appendRequestParams1)
{
     EXPECT_CALL(*m_xconfclientMock, getParameterValue(_,_))
            .Times(1)
            .WillOnce(Return(T2ERROR_FAILURE));
     //char* urlWithParams = (char*) malloc(1024 * sizeof(char));
     //memset(urlWithParams, '0', 1024 * sizeof(char));
     CURLU *requestURL = curl_url();
     curl_url_set(requestURL, CURLUPART_URL, "https://mockxconf:50050/loguploader/getT2DCMSettings", 0);
     EXPECT_EQ(T2ERROR_FAILURE, appendRequestParams(requestURL));
     curl_free(requestURL);
     requestURL = NULL;
}

TEST_F(xconfclientTestFixture, appendRequestParams2)
{
     EXPECT_CALL(*m_xconfclientMock, getParameterValue(_,_))
            .Times(2)
	    .WillOnce(::testing::Return(T2ERROR_SUCCESS))
            .WillOnce(::testing::Return(T2ERROR_FAILURE));
     CURLU *requestURL = curl_url();
     curl_url_set(requestURL, CURLUPART_URL, "https://mockxconf:50050/loguploader/getT2DCMSettings", 0);
     EXPECT_EQ(T2ERROR_FAILURE, appendRequestParams(requestURL));
     curl_free(requestURL);
     requestURL = NULL;
}

TEST_F(xconfclientTestFixture, appendRequestParams3)
{
     EXPECT_CALL(*m_xconfclientMock, getParameterValue(_,_))
            .Times(3)
            .WillOnce(::testing::Return(T2ERROR_SUCCESS))
	    .WillOnce(::testing::Return(T2ERROR_SUCCESS))
            .WillOnce(::testing::Return(T2ERROR_FAILURE));
     CURLU *requestURL = curl_url();
     curl_url_set(requestURL, CURLUPART_URL, "https://mockxconf:50050/loguploader/getT2DCMSettings", 0);
     EXPECT_EQ(T2ERROR_FAILURE, appendRequestParams(requestURL));
     curl_free(requestURL);
     requestURL = NULL;
}

TEST_F(xconfclientTestFixture, appendRequestParams4)
{
     EXPECT_CALL(*m_xconfclientMock, getParameterValue(_,_))
            .Times(4)
            .WillOnce(::testing::Return(T2ERROR_SUCCESS))
            .WillOnce(::testing::Return(T2ERROR_SUCCESS))
            .WillOnce(::testing::Return(T2ERROR_SUCCESS))
            .WillOnce(::testing::Return(T2ERROR_FAILURE));
     CURLU *requestURL = curl_url();
     curl_url_set(requestURL, CURLUPART_URL, "https://mockxconf:50050/loguploader/getT2DCMSettings", 0);
     EXPECT_EQ(T2ERROR_FAILURE, appendRequestParams(requestURL));
     curl_free(requestURL);
     requestURL = NULL;
}

TEST_F(xconfclientTestFixture, appendRequestParams5)
{
     EXPECT_CALL(*m_xconfclientMock, getParameterValue(_,_))
            .Times(5)
            .WillOnce(::testing::Return(T2ERROR_SUCCESS))
            .WillOnce(::testing::Return(T2ERROR_SUCCESS))
            .WillOnce(::testing::Return(T2ERROR_SUCCESS))
            .WillOnce(::testing::Return(T2ERROR_SUCCESS))
            .WillOnce(::testing::Return(T2ERROR_FAILURE));
     CURLU *requestURL = curl_url();
     curl_url_set(requestURL, CURLUPART_URL, "https://mockxconf:50050/loguploader/getT2DCMSettings", 0);
     EXPECT_EQ(T2ERROR_FAILURE, appendRequestParams(requestURL));
     curl_free(requestURL);
     requestURL = NULL;
}

TEST_F(xconfclientTestFixture, appendRequestParams6)
{
     EXPECT_CALL(*m_xconfclientMock, getParameterValue(_,_))
            .Times(6)
            .WillOnce(::testing::Return(T2ERROR_SUCCESS))
            .WillOnce(::testing::Return(T2ERROR_SUCCESS))
            .WillOnce(::testing::Return(T2ERROR_SUCCESS))
            .WillOnce(::testing::Return(T2ERROR_SUCCESS))
            .WillOnce(::testing::Return(T2ERROR_SUCCESS))
            .WillOnce(::testing::Return(T2ERROR_FAILURE));
     CURLU *requestURL = curl_url();
     curl_url_set(requestURL, CURLUPART_URL, "https://mockxconf:50050/loguploader/getT2DCMSettings", 0);
     EXPECT_EQ(T2ERROR_FAILURE, appendRequestParams(requestURL));
     curl_free(requestURL);
     requestURL = NULL;
}

TEST_F(xconfclientTestFixture, appendRequestParams7)
{
     char build_type[256] = {0};
     EXPECT_CALL(*m_xconfclientMock, getParameterValue(_,_))
            .Times(6)
            .WillOnce(Return(T2ERROR_SUCCESS))
            .WillOnce(Return(T2ERROR_SUCCESS))
            .WillOnce(Return(T2ERROR_SUCCESS))
            .WillOnce(Return(T2ERROR_SUCCESS))
            .WillOnce(Return(T2ERROR_SUCCESS))
            .WillOnce(Return(T2ERROR_SUCCESS));
    
     FILE* mockfp = (FILE*)NULL;
     FILE* fp = (FILE*)0xffffffff;
     EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(mockfp));
     CURLU *requestURL = curl_url();
     curl_url_set(requestURL, CURLUPART_URL, "https://mockxconf:50050/loguploader/getT2DCMSettings", 0);
     EXPECT_EQ(T2ERROR_FAILURE, appendRequestParams(requestURL));
     curl_free(requestURL);
     requestURL = NULL;
}
TEST_F(xconfclientTestFixture, fetchRemoteConfiguration)
{
    char* configURL = "https://mockxconf:50050/loguploader/getT2DCMSettings";
    char* configData = NULL;
    EXPECT_CALL(*m_xconfclientMock, getParameterValue(_,_))
            .Times(1)
            .WillOnce(Return(T2ERROR_FAILURE));
    EXPECT_EQ(T2ERROR_FAILURE, fetchRemoteConfiguration(configURL, &configData));
}

TEST_F(xconfclientTestFixture, doHttpGet)
{
     char* data = NULL;
     EXPECT_CALL(*g_fileIOMock, pipe(_))
             .Times(1)
             .WillOnce(Return(-1));
     EXPECT_EQ(T2ERROR_FAILURE, doHttpGet("https://test.com", &data));
}

TEST_F(xconfclientTestFixture,  getRemoteConfigURL)
{
     char* configURL = NULL;
     #if defined(ENABLE_RDKB_SUPPORT)
     EXPECT_CALL(*g_systemMock, access(_,_))
                .Times(1)
                .WillOnce(Return(-1));
     #endif
     EXPECT_CALL(*m_xconfclientMock, getParameterValue(_,_))
                .Times(1)
                .WillOnce(Return(T2ERROR_FAILURE));
     EXPECT_EQ(T2ERROR_FAILURE, getRemoteConfigURL(&configURL));
}

TEST_F(xconfclientTestFixture,  getRemoteConfigURL1)
{
     char* configURL = NULL;
     #if defined(ENABLE_RDKB_SUPPORT)
     EXPECT_CALL(*g_systemMock, access(_,_))
                .Times(1)
                .WillOnce(Return(0));
     #endif
     EXPECT_CALL(*m_xconfclientMock, getParameterValue(_,_))
	        .Times(1)
		.WillOnce(Return(T2ERROR_FAILURE));
     EXPECT_EQ(T2ERROR_FAILURE, getRemoteConfigURL(&configURL));
}

TEST_F(xconfclientTestFixture,  getRemoteConfigURL2)
{
     char* configURL = NULL;
     #if defined(ENABLE_RDKB_SUPPORT)
     EXPECT_CALL(*g_systemMock, access(_,_))
                .Times(1)
                .WillOnce(Return(0));
     #endif
     EXPECT_CALL(*m_xconfclientMock, getParameterValue(_,_))
            .Times(1)
        .WillOnce(Return(T2ERROR_SUCCESS));

     EXPECT_EQ(T2ERROR_FAILURE, getRemoteConfigURL(&configURL));
}

TEST_F(xconfclientTestFixture, doHttpGet1)
{
     char* data = NULL;
     EXPECT_CALL(*g_fileIOMock, pipe(_))
             .Times(2)
             .WillOnce(Return(0))
	     .WillOnce(Return(-1));
     EXPECT_EQ(T2ERROR_FAILURE, doHttpGet("https://test.com", &data));
}

TEST(STOPXCONFCLIENT, success_check)
{
     EXPECT_EQ(T2ERROR_SUCCESS, stopXConfClient());
}

TEST(STARTXCONFCLIENT, success_check)
{
    EXPECT_EQ(T2ERROR_SUCCESS, startXConfClient());
}

TEST_F(xconfclientTestFixture, doHttpGet2)
{
     char* data = NULL;
     EXPECT_CALL(*g_fileIOMock, pipe(_))
             .Times(2)
             .WillOnce(Return(0))
	     .WillOnce(Return(0));
#if defined(ENABLE_RDKB_SUPPORT) && !defined(RDKB_EXTENDER)
#if defined(WAN_FAILOVER_SUPPORTED) || defined(FEATURE_RDKB_CONFIGURABLE_WAN_INTERFACE)
    EXPECT_CALL(*m_xconfclientMock, getParameterValue(_,_))
            .Times(1)
            .WillOnce(Return(T2ERROR_SUCCESS));
#endif
#endif
    EXPECT_CALL(*m_xconfclientMock, isMtlsEnabled())
            .Times(1)
            .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, fork())
            .Times(1)
            .WillOnce(Return(-1));
     EXPECT_EQ(T2ERROR_FAILURE, doHttpGet("https://test.com", &data));
}

TEST_F(xconfclientTestFixture, doHttpGet3)
{
     char* data = NULL;
     EXPECT_CALL(*g_fileIOMock, pipe(_))
             .Times(2)
             .WillOnce(Return(0))
             .WillOnce(Return(0));
#if defined(ENABLE_RDKB_SUPPORT) && !defined(RDKB_EXTENDER)
#if defined(WAN_FAILOVER_SUPPORTED) || defined(FEATURE_RDKB_CONFIGURABLE_WAN_INTERFACE)
    EXPECT_CALL(*m_xconfclientMock, getParameterValue(_,_))
            .Times(1)
            .WillOnce(Return(T2ERROR_SUCCESS));
#endif
#endif
    EXPECT_CALL(*m_xconfclientMock, isMtlsEnabled())
            .Times(1)
            .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, fork())
            .Times(1)
            .WillOnce(Return(1));
    EXPECT_CALL(*g_fileIOMock, close(_))
            .Times(1)
            .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, read(_,_,_))
            .Times(1)
            .WillOnce(Return(-1));
     EXPECT_EQ(T2ERROR_FAILURE, doHttpGet("https://test.com", &data));
}

TEST_F(xconfclientTestFixture, doHttpGet4)
{
     char* data = NULL;
     EXPECT_CALL(*g_fileIOMock, pipe(_))
             .Times(2)
             .WillOnce(Return(0))
             .WillOnce(Return(0));
    #if defined(ENABLE_RDKB_SUPPORT) && !defined(RDKB_EXTENDER)
#if defined(WAN_FAILOVER_SUPPORTED) || defined(FEATURE_RDKB_CONFIGURABLE_WAN_INTERFACE)
    EXPECT_CALL(*m_xconfclientMock, getParameterValue(_,_))
            .Times(1)
            .WillOnce(Return(T2ERROR_SUCCESS));
#endif
#endif
    EXPECT_CALL(*m_xconfclientMock, isMtlsEnabled())
            .Times(1)
            .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, fork())
            .Times(1)
            .WillOnce(Return(1));
    EXPECT_CALL(*g_fileIOMock, close(_))
            .Times(2)
            .WillOnce(Return(0))
            .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, read(_,_,_))
            .Times(1)
            .WillOnce(Return(0));

     EXPECT_EQ(T2ERROR_FAILURE, doHttpGet("https://test.com", &data));  
}
