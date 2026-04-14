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
char* configURL = NULL;
}

using namespace std;

using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::SetArgPointee;
using ::testing::DoAll;

extern FileMock *g_fileIOMock;
XconfclientMock *m_xconfclientMock = NULL;
rbusMock *g_rbusMock = NULL;

// Helper macro to prevent deadlock from mocked fwrite and fputs calls in the protocol code when GTest tries to log output, which can cause a deadlock if the logging functions are mocked without allowing real calls to them.
#define PREVENT_GTEST_LOGGING_DEADLOCK() \
    EXPECT_CALL(*g_fileIOMock, fwrite(::testing::_, ::testing::_, ::testing::_, ::testing::_)) \
        .Times(::testing::AnyNumber()) \
        .WillRepeatedly(::testing::Invoke( \
            [](const void* ptr, size_t size, size_t nitems, FILE* stream) { \
                return ::fwrite(ptr, size, nitems, stream); \
            }))

TEST(GETBUILDTYPE, NULL_CHECK)
{
    EXPECT_EQ(T2ERROR_FAILURE, getBuildType(NULL));
}

TEST(DOHTTPGET, HTTPURL_CHECK)
{
    char* data = NULL;
    FileMock fileMock;
    g_fileIOMock = &fileMock;
    PREVENT_GTEST_LOGGING_DEADLOCK();
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

        // Set default behaviors for curl functions to prevent them from being called
        EXPECT_CALL(*g_fileIOMock, curl_easy_init())
            .Times(::testing::AnyNumber())
            .WillRepeatedly(::testing::Return(nullptr));
        
        EXPECT_CALL(*g_fileIOMock, curl_easy_perform(::testing::_))
            .Times(::testing::AnyNumber())
            .WillRepeatedly(::testing::Return(CURLE_FAILED_INIT));

        EXPECT_CALL(*g_fileIOMock, curl_easy_cleanup(::testing::_))
            .Times(::testing::AnyNumber());

        EXPECT_CALL(*g_fileIOMock, curl_easy_setopt(::testing::_, ::testing::_, ::testing::_))
            .Times(::testing::AnyNumber())
            .WillRepeatedly(::testing::Return(CURLE_OK));

        // Add curl_slist functions to prevent deadlock during pool initialization
        EXPECT_CALL(*g_fileIOMock, curl_slist_append(::testing::_, ::testing::_))
            .Times(::testing::AnyNumber())
            .WillRepeatedly(::testing::Invoke([](struct curl_slist* list, const char* str) {
                // Return a fake non-null pointer to simulate successful append
                static int counter = 1;
                return (struct curl_slist*)(uintptr_t)(0x2000 + counter++);
            }));
        
        EXPECT_CALL(*g_fileIOMock, curl_slist_free_all(::testing::_))
            .Times(::testing::AnyNumber());
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

TEST_F(xconfclientTestFixture, getBuildType2)
{
    FILE* fakeFile = reinterpret_cast<FILE*>(0x1234);
    char fileContent[] = "BUILD_TYPE=PROD";
    char build_type[256] = {0};

    // Expect fopen called with DEVICE_PROPERTIES and returns fakeFile
    EXPECT_CALL(*g_fileIOMock, fopen(_,_))
        .WillOnce(Return(fakeFile));

    EXPECT_CALL(*g_fileIOMock, fscanf(fakeFile, _, _))
            .WillOnce(Return(255))
            .WillOnce(Return(EOF));
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
     EXPECT_CALL(*m_xconfclientMock, getParameterValue(_,_))
            .Times(7)
            .WillOnce(::testing::Return(T2ERROR_SUCCESS))
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

TEST_F(xconfclientTestFixture, appendRequestParams8)
{
     char build_type[256] = {0};
     EXPECT_CALL(*m_xconfclientMock, getParameterValue(_,_))
            .Times(7)
            .WillOnce(Return(T2ERROR_SUCCESS))
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

     EXPECT_EQ(T2ERROR_FAILURE, doHttpGet("https://test.com", &data));
}

TEST_F(xconfclientTestFixture,  getRemoteConfigURL)
{
     char* configURL = NULL;
     #if defined(ENABLE_RDKB_SUPPORT)
     EXPECT_CALL(*g_systemMock, access(_,_))
                .Times(2)
                .WillOnce(Return(-1))
                .WillOnce(Return(0));
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

     EXPECT_CALL(*m_xconfclientMock, getParameterValue(_, _))
        .WillOnce([](const char* paramName, char** paramValue) {
            if (strcmp(paramName, "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.ConfigURL") == 0)
                *paramValue = strdup("https://mockxconf:50050/loguploader/getT2DCMSettings");
            else
                *paramValue = strdup("UNKNOWN");
            return T2ERROR_SUCCESS;
    });
     
     EXPECT_EQ(T2ERROR_SUCCESS, getRemoteConfigURL(&configURL));
}

TEST_F(xconfclientTestFixture,  getRemoteConfigURL3)
{
     char* configURL = NULL;

     #if defined(ENABLE_RDKB_SUPPORT)
     EXPECT_CALL(*g_systemMock, access(_,_))
                .Times(1)
                .WillOnce(Return(0));
     #endif

     EXPECT_CALL(*m_xconfclientMock, getParameterValue(_, _))
        .Times(2)
        .WillOnce([](const char* paramName, char** paramValue) {
            if (strcmp(paramName, "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.ConfigURL") == 0)
                *paramValue = strdup("www.//mockxconf:50050/loguploader/getT2DCMSettings");
            else
                *paramValue = strdup("UNKNOWN");
            return T2ERROR_SUCCESS;
    })
        .WillOnce([](const char* paramName, char** paramValue) {
            if (strcmp(paramName, "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.ConfigURL") == 0)
                *paramValue = strdup("https://mockxconf:50050/loguploader/getT2DCMSettings");
            else
                *paramValue = strdup("UNKNOWN");
            return T2ERROR_SUCCESS;
    });
     EXPECT_EQ(T2ERROR_SUCCESS, getRemoteConfigURL(&configURL));
}

TEST_F(xconfclientTestFixture, doHttpGet1)
{
     char* data = NULL;

        // Set up curl_easy_init to return valid handles for pool initialization
        // The pool will call curl_easy_init() based on pool size (default 2)
        EXPECT_CALL(*g_fileIOMock, curl_easy_init())
            .Times(::testing::AnyNumber())
            .WillRepeatedly(::testing::Invoke([]() {
                // Return unique fake handles for each call
                static int handle_counter = 1;
                return (CURL*)(uintptr_t)(0x1000 + handle_counter++);
            }));
        EXPECT_CALL(*m_xconfclientMock, isMtlsEnabled())
            .Times(1)
            .WillOnce(Return(0));
     EXPECT_EQ(T2ERROR_FAILURE, doHttpGet("https://test.com", &data));
}

TEST_F(xconfclientTestFixture, doHttpGet2)
{
     char* data = NULL;

     EXPECT_CALL(*g_fileIOMock, curl_easy_init())
     .Times(::testing::AnyNumber())
     .WillRepeatedly(::testing::Invoke([]() {
         // Return unique fake handles for each call
         static int handle_counter = 1;
         return (CURL*)(uintptr_t)(0x1000 + handle_counter++);
     }));

#if defined(ENABLE_RDKB_SUPPORT) && !defined(RDKB_EXTENDER)
#if defined(WAN_FAILOVER_SUPPORTED) || defined(FEATURE_RDKB_CONFIGURABLE_WAN_INTERFACE)
    EXPECT_CALL(*m_xconfclientMock, getParameterValue(_,_))
            .WillOnce([](const char* paramName, char** paramValue) {
            if (strcmp(paramName, "Device.X_RDK_WanManager.CurrentActiveInterface") == 0)
                *paramValue = strdup("erouter0");
            else
                *paramValue = strdup("unknown");
            return T2ERROR_SUCCESS;
    });
#endif
#endif

    EXPECT_CALL(*m_xconfclientMock, isMtlsEnabled())
            .Times(1)
            .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, curl_easy_perform(_))
            .Times(1)
            .WillOnce(Return(CURLE_OK));
    EXPECT_CALL(*g_fileIOMock, curl_easy_getinfo_mock(_,_,_))
            .Times(::testing::AtLeast(1))
            .WillRepeatedly([](CURL* curl, CURLINFO info, void* response_code) {
                if (info == CURLINFO_RESPONSE_CODE) {
                    *(long*)response_code = 200;
                }
                return CURLE_OK;
            });
    
    //Add curl_easy_cleanup expectation
    EXPECT_CALL(*g_fileIOMock, curl_easy_cleanup(_))
            .Times(::testing::AnyNumber());
    EXPECT_EQ(T2ERROR_SUCCESS, doHttpGet("https://test.com", &data));
}


TEST_F(xconfclientTestFixture, doHttpGet3)
{
     char* data = NULL;

     EXPECT_CALL(*g_fileIOMock, curl_easy_init())
     .Times(::testing::AnyNumber())
     .WillRepeatedly(::testing::Invoke([]() {
         // Return unique fake handles for each call
         static int handle_counter = 1;
         return (CURL*)(uintptr_t)(0x1000 + handle_counter++);
     }));

#if defined(ENABLE_RDKB_SUPPORT) && !defined(RDKB_EXTENDER)
#if defined(WAN_FAILOVER_SUPPORTED) || defined(FEATURE_RDKB_CONFIGURABLE_WAN_INTERFACE)
    EXPECT_CALL(*m_xconfclientMock, getParameterValue(_,_))
            .WillOnce([](const char* paramName, char** paramValue) {
            if (strcmp(paramName, "Device.X_RDK_WanManager.CurrentActiveInterface") == 0)
                *paramValue = strdup("erouter0");
            else
                *paramValue = strdup("unknown");
            return T2ERROR_SUCCESS;
    });
#endif
#endif

    EXPECT_CALL(*m_xconfclientMock, isMtlsEnabled())
            .Times(1)
            .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, curl_easy_perform(_))
            .Times(1)
            .WillOnce(Return(CURLE_OK));
            EXPECT_CALL(*g_fileIOMock, curl_easy_getinfo_mock(_,_,_))
            .Times(::testing::AtLeast(1))
            .WillRepeatedly([](CURL* curl, CURLINFO info, void* response_code) {
                if (info == CURLINFO_RESPONSE_CODE) {
                    *(long*)response_code = 404;
                }
                return CURLE_OK;
            });
    
    //Add curl_easy_cleanup expectation
    EXPECT_CALL(*g_fileIOMock, curl_easy_cleanup(_))
            .Times(::testing::AnyNumber());
    EXPECT_EQ(T2ERROR_PROFILE_NOT_SET, doHttpGet("https://test.com", &data));
}

TEST_F(xconfclientTestFixture, doHttpGet4)
{
     char* data = NULL;

     EXPECT_CALL(*g_fileIOMock, curl_easy_init())
     .Times(::testing::AnyNumber())
     .WillRepeatedly(::testing::Invoke([]() {
         // Return unique fake handles for each call
         static int handle_counter = 1;
         return (CURL*)(uintptr_t)(0x1000 + handle_counter++);
     }));

#if defined(ENABLE_RDKB_SUPPORT) && !defined(RDKB_EXTENDER)
#if defined(WAN_FAILOVER_SUPPORTED) || defined(FEATURE_RDKB_CONFIGURABLE_WAN_INTERFACE)
    EXPECT_CALL(*m_xconfclientMock, getParameterValue(_,_))
            .WillOnce([](const char* paramName, char** paramValue) {
            if (strcmp(paramName, "Device.X_RDK_WanManager.CurrentActiveInterface") == 0)
                *paramValue = strdup("erouter0");
            else
                *paramValue = strdup("unknown");
            return T2ERROR_SUCCESS;
    });
#endif
#endif

    EXPECT_CALL(*m_xconfclientMock, isMtlsEnabled())
            .Times(1)
            .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, curl_easy_perform(_))
            .Times(1)
            .WillOnce(Return(CURLE_OK));
    EXPECT_CALL(*g_fileIOMock, curl_easy_getinfo_mock(_,_,_))
            .Times(::testing::AtLeast(1))
            .WillRepeatedly([](CURL* curl, CURLINFO info, void* response_code) {
                if (info == CURLINFO_RESPONSE_CODE) {
                    *(long*)response_code = 302;
                }
                return CURLE_OK;
            });
    
    //Add curl_easy_cleanup expectation
    EXPECT_CALL(*g_fileIOMock, curl_easy_cleanup(_))
            .Times(::testing::AnyNumber());
    EXPECT_EQ(T2ERROR_FAILURE, doHttpGet("https://test.com", &data));
}

TEST_F(xconfclientTestFixture, doHttpGet5)
{
     char* data = NULL;

     EXPECT_CALL(*g_fileIOMock, curl_easy_init())
     .Times(::testing::AnyNumber())
     .WillRepeatedly(::testing::Invoke([]() {
         // Return unique fake handles for each call
         static int handle_counter = 1;
         return (CURL*)(uintptr_t)(0x1000 + handle_counter++);
     }));

#if defined(ENABLE_RDKB_SUPPORT) && !defined(RDKB_EXTENDER)
#if defined(WAN_FAILOVER_SUPPORTED) || defined(FEATURE_RDKB_CONFIGURABLE_WAN_INTERFACE)
    EXPECT_CALL(*m_xconfclientMock, getParameterValue(_,_))
            .WillOnce([](const char* paramName, char** paramValue) {
            if (strcmp(paramName, "Device.X_RDK_WanManager.CurrentActiveInterface") == 0)
                *paramValue = strdup("erouter0");
            else
                *paramValue = strdup("unknown");
            return T2ERROR_SUCCESS;
    });
#endif
#endif

    EXPECT_CALL(*m_xconfclientMock, isMtlsEnabled())
            .Times(1)
            .WillOnce(Return(0));
    // Mock curl_easy_perform to simulate receiving data
    EXPECT_CALL(*g_fileIOMock, curl_easy_perform(_))
            .Times(1)
            .WillOnce(::testing::Invoke([](CURL* handle) {
                // Simulate curl calling the write callback with response data
                const char* test_response = "{\"status\":\"success\",\"data\":\"test response data\"}";
                
                // In a real scenario, curl would call the callback set via CURLOPT_WRITEFUNCTION
                // For testing, we need to directly populate the response structure
                // This is tricky because we need access to the response pointer
                
                return CURLE_OK;
            }));
    EXPECT_CALL(*g_fileIOMock, curl_easy_getinfo_mock(_,_,_))
            .Times(::testing::AtLeast(1))
            .WillRepeatedly([](CURL* curl, CURLINFO info, void* response_code) {
                if (info == CURLINFO_RESPONSE_CODE) {
                    *(long*)response_code = 200;
                }
                return CURLE_OK;
            });
    
    //Add curl_easy_cleanup expectation
    EXPECT_CALL(*g_fileIOMock, curl_easy_cleanup(_))
            .Times(::testing::AnyNumber());
    EXPECT_EQ(T2ERROR_SUCCESS, doHttpGet("https://test.com", &data));
}

TEST_F(xconfclientTestFixture, doHttpGet7)
{
     char* data = NULL;

     EXPECT_CALL(*g_fileIOMock, curl_easy_init())
     .Times(::testing::AnyNumber())
     .WillRepeatedly(::testing::Invoke([]() {
         // Return unique fake handles for each call
         static int handle_counter = 1;
         return (CURL*)(uintptr_t)(0x1000 + handle_counter++);
     }));
    #if defined(ENABLE_RDKB_SUPPORT) && !defined(RDKB_EXTENDER)
#if defined(WAN_FAILOVER_SUPPORTED) || defined(FEATURE_RDKB_CONFIGURABLE_WAN_INTERFACE)
    EXPECT_CALL(*m_xconfclientMock, getParameterValue(_,_))
            .Times(1)
            .WillOnce([](const char* paramName, char** paramValue) {
            if (strcmp(paramName, "Device.X_RDK_WanManager.CurrentActiveInterface") == 0)
                *paramValue = strdup("erouter0");
            else
                *paramValue = strdup("unknown");
            return ;
    });
#endif
#endif
    EXPECT_CALL(*m_xconfclientMock, isMtlsEnabled())
            .Times(1)
            .WillOnce(Return(1));
    EXPECT_CALL(*g_systemMock, access(_,_))
            .Times(1)
            .WillOnce(Return(1));

    EXPECT_CALL(*g_fileIOMock, curl_easy_perform(_))
            .Times(1)
            .WillOnce(Return(CURLE_OK));
    EXPECT_CALL(*g_fileIOMock, curl_easy_getinfo_mock(_,_,_))
            .Times(1)
            .WillOnce([](CURL* curl, CURLINFO info, void* response_code) {
                if (info == CURLINFO_RESPONSE_CODE) {
                    *(long*)response_code = 200;
                    return CURLE_OK;
                }
                return CURLE_OK;
            });

    EXPECT_EQ(doHttpGet("https://test.com", &data), T2ERROR_SUCCESS);
}

TEST_F(xconfclientTestFixture, doHttpGet8)
{
     char* data = NULL;

     EXPECT_CALL(*g_fileIOMock, curl_easy_init())
     .Times(::testing::AnyNumber())
     .WillRepeatedly(::testing::Invoke([]() {
         // Return unique fake handles for each call
         static int handle_counter = 1;
         return (CURL*)(uintptr_t)(0x1000 + handle_counter++);
     }));
    #if defined(ENABLE_RDKB_SUPPORT) && !defined(RDKB_EXTENDER)
#if defined(WAN_FAILOVER_SUPPORTED) || defined(FEATURE_RDKB_CONFIGURABLE_WAN_INTERFACE)
    EXPECT_CALL(*m_xconfclientMock, getParameterValue(_,_))
            .Times(1)
            .WillOnce([](const char* paramName, char** paramValue) {
            if (strcmp(paramName, "Device.X_RDK_WanManager.CurrentActiveInterface") == 0)
                *paramValue = strdup("erouter0");
            else
                *paramValue = strdup("unknown");
            return ;
    });
#endif
#endif
    EXPECT_CALL(*m_xconfclientMock, isMtlsEnabled())
            .Times(1)
            .WillOnce(Return(1));
    EXPECT_CALL(*g_systemMock, access(_,_))
            .Times(1)
            .WillOnce(Return(0));

    EXPECT_CALL(*g_fileIOMock, curl_easy_perform(_))
            .Times(1)
            .WillOnce(Return(CURLE_OK));
    EXPECT_CALL(*g_fileIOMock, curl_easy_getinfo_mock(_,_,_))
            .Times(1)
            .WillOnce([](CURL* curl, CURLINFO info, void* response_code) {
                if (info == CURLINFO_RESPONSE_CODE) {
                    *(long*)response_code = 200;
                    return CURLE_OK;
                }
                return CURLE_OK;
            });

    EXPECT_EQ(doHttpGet("https://test.com", &data), T2ERROR_SUCCESS);
}

TEST_F(xconfclientTestFixture, initXConfClient_failure)
{
     #if defined(ENABLE_RDKB_SUPPORT)
     EXPECT_CALL(*g_systemMock, access(_,_))
                .Times(1)
                .WillOnce(Return(0));
     #endif

     EXPECT_CALL(*m_xconfclientMock, getParameterValue(_, _))
        .WillOnce([](const char* paramName, char** paramValue) {
            if (strcmp(paramName, "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.ConfigURL") == 0)
                *paramValue = strdup("www.mockxconf:50050/loguploader/getT2DCMSettings");
            else
                *paramValue = strdup("UNKNOWN");
            return T2ERROR_SUCCESS;
     })
        .WillOnce([](const char* paramName, char** paramValue) {
            if (strcmp(paramName, "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.ConfigURL") == 0)
                *paramValue = strdup("www.mockxconf:50050/loguploader/getT2DCMSettings");
            else
                *paramValue = strdup("UNKNOWN");
            return T2ERROR_SUCCESS;
     })
        .WillOnce([](const char* paramName, char** paramValue) {
            if (strcmp(paramName, "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.ConfigURL") == 0)
                *paramValue = strdup("www.mockxconf:50050/loguploader/getT2DCMSettings");
            else
                *paramValue = strdup("UNKNOWN");
            return T2ERROR_SUCCESS;
     });
     EXPECT_CALL(*m_xconfclientMock, ProfileXConf_isSet())
            .Times(1)
            .WillOnce(Return(false));
     EXPECT_EQ(T2ERROR_SUCCESS, initXConfClient());
     sleep(20);
}

TEST(STOPXCONFCLIENT, success_check)
{
     EXPECT_EQ(T2ERROR_SUCCESS, stopXConfClient());
}

/*
 * Tests for the getTimezone() timeZoneDST fallback path.
 *
 * getTimezone() is a static function exercised indirectly via appendRequestParams().
 * The fallback to /opt/persistent/timeZoneDST is reached after the JSON-based
 * primary path exhausts its 10 retries without finding a timezone.
 *
 * Preconditions for each test:
 *   - All getParameterValue calls succeed (6 calls with dummy values).
 *   - getBuildType succeeds: fopen(DEVICE_PROPERTIES) returns a fake handle, fscanf
 *     fills "BUILD_TYPE=PROD" and then returns EOF.
 *   - getTimezone CPU_ARCH read: fopen(DEVICE_PROPERTIES) returns NULL (no CPU_ARCH).
 *   - getTimezone JSON loop: fopen("/opt/output.json") returns NULL (10 retries fail).
 * After these preconditions the timeZoneDST fallback is exercised.
 */
#if !defined(ENABLE_RDKB_SUPPORT) && !defined(ENABLE_RDKC_SUPPORT)

// Helper to set up the mocks needed to reach the timeZoneDST fallback in getTimezone().
// Requires g_fileIOMock and m_xconfclientMock to be set.
static void SetupReachTimeZoneDSTFallback(FILE* devicePropsHandle)
{
    using namespace testing;

    // All 6 getParameterValue calls succeed with dummy values.
    EXPECT_CALL(*m_xconfclientMock, getParameterValue(_, _))
        .Times(AtLeast(6))
        .WillRepeatedly(Invoke([](const char*, char** val) -> T2ERROR {
            *val = strdup("dummy");
            return T2ERROR_SUCCESS;
        }));

    // getBuildType: fopen(DEVICE_PROPERTIES) -> devicePropsHandle, fscanf fills
    // "BUILD_TYPE=PROD" then EOF, fclose returns 0.
    EXPECT_CALL(*g_fileIOMock, fopen(StrEq("/etc/device.properties"), _))
        .WillOnce(Return(devicePropsHandle))  // getBuildType
        .WillOnce(Return(nullptr));            // getTimezone CPU_ARCH read

    EXPECT_CALL(*g_fileIOMock, fscanf(devicePropsHandle, _, _))
        .WillOnce(Invoke([](FILE*, const char*, va_list args) -> int {
            char* buf = va_arg(args, char*);
            strncpy(buf, "BUILD_TYPE=PROD", 254);
            buf[254] = '\0';
            return 1;
        }))
        .WillOnce(Return(EOF));

    EXPECT_CALL(*g_fileIOMock, fclose(devicePropsHandle))
        .WillOnce(Return(0));

    // getTimezone JSON loop: fopen("/opt/output.json") returns NULL all 10 retries.
    EXPECT_CALL(*g_fileIOMock, fopen(StrEq("/opt/output.json"), _))
        .Times(10)
        .WillRepeatedly(Return(nullptr));
}

// Test 1: fopen("/opt/persistent/timeZoneDST") returns NULL.
// The fallback cannot open the file; timezone stays NULL; appendRequestParams returns FAILURE.
TEST_F(xconfclientTestFixture, getTimezone_timeZoneDST_fopen_null)
{
    FILE* devicePropsHandle = reinterpret_cast<FILE*>(0x1234);
    PREVENT_GTEST_LOGGING_DEADLOCK();
    SetupReachTimeZoneDSTFallback(devicePropsHandle);

    EXPECT_CALL(*g_fileIOMock, fopen(StrEq("/opt/persistent/timeZoneDST"), _))
        .WillOnce(Return(nullptr));

    CURLU* requestURL = curl_url();
    curl_url_set(requestURL, CURLUPART_URL,
                 "https://mockxconf:50050/loguploader/getT2DCMSettings", 0);
    EXPECT_EQ(T2ERROR_FAILURE, appendRequestParams(requestURL));
    curl_free(requestURL);
    requestURL = NULL;
}

// Test 2: ftell returns -1 (unreadable/error).
// The fallback opens the file but ftell fails; file is closed; timezone stays NULL.
TEST_F(xconfclientTestFixture, getTimezone_timeZoneDST_ftell_negative)
{
    FILE* devicePropsHandle = reinterpret_cast<FILE*>(0x1234);
    FILE* tzFile = reinterpret_cast<FILE*>(0x5678);
    PREVENT_GTEST_LOGGING_DEADLOCK();
    SetupReachTimeZoneDSTFallback(devicePropsHandle);

    EXPECT_CALL(*g_fileIOMock, fopen(StrEq("/opt/persistent/timeZoneDST"), _))
        .WillOnce(Return(tzFile));
    EXPECT_CALL(*g_fileIOMock, fseek(tzFile, 0, SEEK_END))
        .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, ftell(tzFile))
        .WillOnce(Return(-1L));
    EXPECT_CALL(*g_fileIOMock, fclose(tzFile))
        .WillOnce(Return(0));

    CURLU* requestURL = curl_url();
    curl_url_set(requestURL, CURLUPART_URL,
                 "https://mockxconf:50050/loguploader/getT2DCMSettings", 0);
    EXPECT_EQ(T2ERROR_FAILURE, appendRequestParams(requestURL));
    curl_free(requestURL);
    requestURL = NULL;
}

// Test 3: ftell returns 0 (empty file).
// The fallback opens the file but it is empty; file is closed; timezone stays NULL.
TEST_F(xconfclientTestFixture, getTimezone_timeZoneDST_empty_file)
{
    FILE* devicePropsHandle = reinterpret_cast<FILE*>(0x1234);
    FILE* tzFile = reinterpret_cast<FILE*>(0x5678);
    PREVENT_GTEST_LOGGING_DEADLOCK();
    SetupReachTimeZoneDSTFallback(devicePropsHandle);

    EXPECT_CALL(*g_fileIOMock, fopen(StrEq("/opt/persistent/timeZoneDST"), _))
        .WillOnce(Return(tzFile));
    EXPECT_CALL(*g_fileIOMock, fseek(tzFile, 0, SEEK_END))
        .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, ftell(tzFile))
        .WillOnce(Return(0L));
    EXPECT_CALL(*g_fileIOMock, fclose(tzFile))
        .WillOnce(Return(0));

    CURLU* requestURL = curl_url();
    curl_url_set(requestURL, CURLUPART_URL,
                 "https://mockxconf:50050/loguploader/getT2DCMSettings", 0);
    EXPECT_EQ(T2ERROR_FAILURE, appendRequestParams(requestURL));
    curl_free(requestURL);
    requestURL = NULL;
}

// Test 4: ftell returns a value exceeding the 256-byte limit.
// The fallback rejects oversized files; file is closed; timezone stays NULL.
TEST_F(xconfclientTestFixture, getTimezone_timeZoneDST_file_too_large)
{
    FILE* devicePropsHandle = reinterpret_cast<FILE*>(0x1234);
    FILE* tzFile = reinterpret_cast<FILE*>(0x5678);
    PREVENT_GTEST_LOGGING_DEADLOCK();
    SetupReachTimeZoneDSTFallback(devicePropsHandle);

    EXPECT_CALL(*g_fileIOMock, fopen(StrEq("/opt/persistent/timeZoneDST"), _))
        .WillOnce(Return(tzFile));
    EXPECT_CALL(*g_fileIOMock, fseek(tzFile, 0, SEEK_END))
        .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, ftell(tzFile))
        .WillOnce(Return(512L));
    EXPECT_CALL(*g_fileIOMock, fclose(tzFile))
        .WillOnce(Return(0));

    CURLU* requestURL = curl_url();
    curl_url_set(requestURL, CURLUPART_URL,
                 "https://mockxconf:50050/loguploader/getT2DCMSettings", 0);
    EXPECT_EQ(T2ERROR_FAILURE, appendRequestParams(requestURL));
    curl_free(requestURL);
    requestURL = NULL;
}

// Test 5: Valid timezone string read from /opt/persistent/timeZoneDST.
// The fallback reads a valid timezone; appendRequestParams returns SUCCESS.
TEST_F(xconfclientTestFixture, getTimezone_timeZoneDST_valid_timezone)
{
    FILE* devicePropsHandle = reinterpret_cast<FILE*>(0x1234);
    FILE* tzFile = reinterpret_cast<FILE*>(0x5678);
    PREVENT_GTEST_LOGGING_DEADLOCK();
    SetupReachTimeZoneDSTFallback(devicePropsHandle);

    EXPECT_CALL(*g_fileIOMock, fopen(StrEq("/opt/persistent/timeZoneDST"), _))
        .WillOnce(Return(tzFile));
    EXPECT_CALL(*g_fileIOMock, fseek(tzFile, 0, SEEK_END))
        .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, ftell(tzFile))
        .WillOnce(Return(10L));
    EXPECT_CALL(*g_fileIOMock, fseek(tzFile, 0, SEEK_SET))
        .WillOnce(Return(0));
    // fscanf fills the zone buffer with "US/Eastern" and then signals end-of-file.
    EXPECT_CALL(*g_fileIOMock, fscanf(tzFile, _, _))
        .WillOnce(Invoke([](FILE*, const char*, va_list args) -> int {
            char* buf = va_arg(args, char*);
            strncpy(buf, "US/Eastern", 10);
            buf[10] = '\0';
            return 1;
        }))
        .WillOnce(Return(0));
    EXPECT_CALL(*g_fileIOMock, fclose(tzFile))
        .WillOnce(Return(0));

    CURLU* requestURL = curl_url();
    curl_url_set(requestURL, CURLUPART_URL,
                 "https://mockxconf:50050/loguploader/getT2DCMSettings", 0);
    EXPECT_EQ(T2ERROR_SUCCESS, appendRequestParams(requestURL));
    curl_free(requestURL);
    requestURL = NULL;
}

#endif /* !defined(ENABLE_RDKB_SUPPORT) && !defined(ENABLE_RDKC_SUPPORT) */
