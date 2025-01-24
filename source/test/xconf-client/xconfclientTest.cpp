/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2024 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
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

extern "C" {
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <signal.h>
#include <xconf-client/xconfclient.h>
#include <utils/t2MtlsUtils.h>
#include <t2parser/t2parserxconf.h>
#include <utils/vector.h>
#include <utils/persistence.h>
#include <telemetry2_0.h>
#include <ccspinterface/busInterface.h>
sigset_t blocking_signal;
}

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "xconfclientMock.h"
#include "test/mocks/rdklogMock.h"
#include "test/mocks/rbusMock.h"
#include "test/mocks/SystemMock.h"
#include "test/mocks/FileioMock.h"

#include <iostream>
#include <stdexcept>
using namespace std;

using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;

rdklogMock *m_rdklogMock = NULL;
rbusMock *g_rbusMock = NULL;
SystemMock * g_SystemMock = NULL;
FileIOMock *g_fileIOMock = NULL;
XconfclientMock *m_xconfclientMock = NULL;

class rbusTestFixture : public ::testing::Test {
    protected:
            rbusMock rbusmock_IO;

            rbusTestFixture()
            {
                    g_rbusMock = &rbusmock_IO;
            }

            virtual ~rbusTestFixture()
            {
                    g_rbusMock = NULL;
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

class XconfcliTestFixture : public ::testing::Test {
    protected:
 
        XconfclientMock mockedxconf;

        XconfcliTestFixture()
        {
            m_xconfclientMock = &mockedxconf;

        }
        virtual ~XconfcliTestFixture()
        {
            m_xconfclientMock = NULL;
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

TEST(GETBUILDTYPE, NULL_CHECK)
{
    EXPECT_EQ(T2ERROR_FAILURE, getBuildType(NULL));
}

TEST(APPENDREQUEST, NULL_CHECK)
{
    char *buf = NULL;
    EXPECT_EQ(T2ERROR_FAILURE, appendRequestParams(buf,256));
    buf = (char*)malloc(256);
    memset(buf, '0', 256);
    EXPECT_EQ(T2ERROR_FAILURE, appendRequestParams(buf,0));
    free(buf);
    buf = NULL;
}


TEST(DOHTTPGET, HTTPURL_CHECK)
{
    char* data = NULL;
    EXPECT_EQ(T2ERROR_FAILURE,  doHttpGet(NULL, &data));
}

TEST(FETCHREMOTECONFIG, CONFIGURL_NULL_INVALID)
{
    char* configURL = "https://test.com";
    char* configData = NULL;
    EXPECT_EQ(T2ERROR_FAILURE, fetchRemoteConfiguration(configURL, &configData));
    EXPECT_EQ(T2ERROR_INVALID_ARGS, fetchRemoteConfiguration(NULL, &configData));
}

class XconfclientFSTestFixture : public ::testing::Test {
    protected:
        SystemMock mockedXsystem;
        FileIOMock mockedXfileIO;

        XconfclientFSTestFixture()
        {
            g_SystemMock = &mockedXsystem;
            g_fileIOMock = &mockedXfileIO;

        }
        virtual ~XconfclientFSTestFixture()
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

class XconfclientFileTestFixture : public ::testing::Test {
    protected:
        FileIOMock mockedXfileIO;

        XconfclientFileTestFixture()
        {
            g_fileIOMock = &mockedXfileIO;

        }
        virtual ~XconfclientFileTestFixture()
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

class XconfclientSystemTestFixture : public ::testing::Test {
    protected:
        SystemMock mockedXsystem;

        XconfclientSystemTestFixture()
        {
            g_SystemMock = &mockedXsystem;

        }
        virtual ~XconfclientSystemTestFixture()
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

TEST_F(XconfclientFileTestFixture, getBuildType)
{
     FILE* mockfp = NULL;
     char build_type[256] = {0};
     EXPECT_CALL(*g_fileIOMock, fopen(_,_))
            .Times(1)
            .WillOnce(Return(mockfp));
     ASSERT_EQ(T2ERROR_FAILURE, getBuildType(build_type));
}


TEST_F(XconfclientSystemTestFixture, appendRequestParams1)
{
     EXPECT_CALL(*g_SystemMock, getParameterValue(_,_))
            .Times(1)
            .WillOnce(Return(T2ERROR_FAILURE));
     char* urlWithParams = (char*) malloc(1024 * sizeof(char));
     memset(urlWithParams, '0', 1024 * sizeof(char));
     snprintf(urlWithParams, 1024, "%s?", "https://mockxconf:50050/loguploader/getT2DCMSettings?");
     EXPECT_EQ(T2ERROR_FAILURE, appendRequestParams(urlWithParams, 968));
     free(urlWithParams);
     urlWithParams = NULL;
}

TEST_F(XconfclientSystemTestFixture, appendRequestParams2)
{
     EXPECT_CALL(*g_SystemMock, getParameterValue(_,_))
            .Times(2)
	    .WillOnce(Return(T2ERROR_SUCCESS))
            .WillOnce(Return(T2ERROR_FAILURE));
     char* urlWithParams = (char*) malloc(1024 * sizeof(char));
     memset(urlWithParams, '0', 1024 * sizeof(char));
     snprintf(urlWithParams, 1024, "%s?", "https://mockxconf:50050/loguploader/getT2DCMSettings?");
     EXPECT_EQ(T2ERROR_FAILURE, appendRequestParams(urlWithParams, 968));
     free(urlWithParams);
     urlWithParams = NULL;
}

TEST_F(XconfclientSystemTestFixture, appendRequestParams3)
{
     EXPECT_CALL(*g_SystemMock, getParameterValue(_,_))
            .Times(3)
            .WillOnce(Return(T2ERROR_SUCCESS))
	    .WillOnce(Return(T2ERROR_SUCCESS))
            .WillOnce(Return(T2ERROR_FAILURE));
     char* urlWithParams = (char*) malloc(1024 * sizeof(char));
     memset(urlWithParams, '0', 1024 * sizeof(char));
     snprintf(urlWithParams, 1024, "%s?", "https://mockxconf:50050/loguploader/getT2DCMSettings?");
     EXPECT_EQ(T2ERROR_FAILURE, appendRequestParams(urlWithParams, 968));
     free(urlWithParams);
     urlWithParams = NULL;
}

TEST_F(XconfclientSystemTestFixture, appendRequestParams4)
{
     EXPECT_CALL(*g_SystemMock, getParameterValue(_,_))
            .Times(4)
            .WillOnce(Return(T2ERROR_SUCCESS))
            .WillOnce(Return(T2ERROR_SUCCESS))
            .WillOnce(Return(T2ERROR_SUCCESS))
            .WillOnce(Return(T2ERROR_FAILURE));
     char* urlWithParams = (char*) malloc(1024 * sizeof(char));
     memset(urlWithParams, '0', 1024 * sizeof(char));
     snprintf(urlWithParams, 1024, "%s?", "https://mockxconf:50050/loguploader/getT2DCMSettings?");
     EXPECT_EQ(T2ERROR_FAILURE, appendRequestParams(urlWithParams, 968));
     free(urlWithParams);
     urlWithParams = NULL;
}

TEST_F(XconfclientSystemTestFixture, appendRequestParams5)
{
     EXPECT_CALL(*g_SystemMock, getParameterValue(_,_))
            .Times(5)
            .WillOnce(Return(T2ERROR_SUCCESS))
            .WillOnce(Return(T2ERROR_SUCCESS))
            .WillOnce(Return(T2ERROR_SUCCESS))
            .WillOnce(Return(T2ERROR_SUCCESS))
            .WillOnce(Return(T2ERROR_FAILURE));
     char* urlWithParams = (char*) malloc(1024 * sizeof(char));
     memset(urlWithParams, '0', 1024 * sizeof(char));
     snprintf(urlWithParams, 1024, "%s?", "https://mockxconf:50050/loguploader/getT2DCMSettings?");
     EXPECT_EQ(T2ERROR_FAILURE, appendRequestParams(urlWithParams, 968));
     free(urlWithParams);
     urlWithParams = NULL;
}

TEST_F(XconfclientSystemTestFixture, appendRequestParams6)
{
     EXPECT_CALL(*g_SystemMock, getParameterValue(_,_))
            .Times(6)
            .WillOnce(Return(T2ERROR_SUCCESS))
            .WillOnce(Return(T2ERROR_SUCCESS))
            .WillOnce(Return(T2ERROR_SUCCESS))
            .WillOnce(Return(T2ERROR_SUCCESS))
            .WillOnce(Return(T2ERROR_SUCCESS))
            .WillOnce(Return(T2ERROR_FAILURE));
     char* urlWithParams = (char*) malloc(1024 * sizeof(char));
     memset(urlWithParams, '0', 1024 * sizeof(char));
     snprintf(urlWithParams, 1024, "%s?", "https://mockxconf:50050/loguploader/getT2DCMSettings?");
     EXPECT_EQ(T2ERROR_FAILURE, appendRequestParams(urlWithParams, 968));
     free(urlWithParams);
     urlWithParams = NULL;
}

TEST_F(XconfclientFileTestFixture, doHttpGet)
{
     char* data = NULL;
     EXPECT_CALL(*g_fileIOMock, pipe(_))
             .Times(1)
             .WillOnce(Return(-1));
     EXPECT_EQ(T2ERROR_FAILURE, doHttpGet("https://test.com", &data));
}

TEST_F(XconfclientSystemTestFixture,  getRemoteConfigURL)
{
     char* configURL = NULL;
     #if defined(ENABLE_RDKB_SUPPORT)
     EXPECT_CALL(*g_SystemMock, access(_,_))
                .Times(1)
                .WillOnce(Return(-1));
     #endif
     EXPECT_CALL(*g_SystemMock, getParameterValue(_,_))
                .Times(1)
                .WillOnce(Return(T2ERROR_FAILURE));
     EXPECT_EQ(T2ERROR_FAILURE, getRemoteConfigURL(&configURL));
}

TEST_F(XconfclientSystemTestFixture,  getRemoteConfigURL1)
{
     char* configURL = NULL;
     #if defined(ENABLE_RDKB_SUPPORT)
     EXPECT_CALL(*g_SystemMock, access(_,_))
                .Times(1)
                .WillOnce(Return(0));
     #endif
     EXPECT_CALL(*g_SystemMock, getParameterValue(_,_))
	        .Times(1)
		.WillOnce(Return(T2ERROR_FAILURE));
     EXPECT_EQ(T2ERROR_FAILURE, getRemoteConfigURL(&configURL));
}

TEST_F(XconfclientFileTestFixture, doHttpGet1)
{
     char* data = NULL;
     EXPECT_CALL(*g_fileIOMock, pipe(_))
             .Times(2)
             .WillOnce(Return(0))
	     .WillOnce(Return(-1));
     EXPECT_EQ(T2ERROR_FAILURE, doHttpGet("https://test.com", &data));
}
