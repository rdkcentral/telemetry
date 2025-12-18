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

extern "C" {
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <telemetry2_0.h>
#include <utils/vector.h>
#include <utils/t2MtlsUtils.h>
#include <signal.h>
#include <protocol/http/curlinterface.h>
#include <protocol/rbusMethod/rbusmethodinterface.h>
#include <bulkdata/reportprofiles.h>
#include <ccspinterface/busInterface.h>
#include <ccspinterface/rbusInterface.h>
#include <reportgen/reportgen.h>

typedef struct
{
    bool curlStatus;
    CURLcode curlResponse;
    CURLcode curlSetopCode;
    long http_code;
    int lineNumber;

} childResponse ;

    typedef size_t (*WriteToFileFunc)(void *, size_t, size_t, void *);
    WriteToFileFunc getWriteToFileCallback(void);
    typedef T2ERROR (*SetHeaderFunc)(CURL *, const char *, struct curl_slist **, childResponse *);
    SetHeaderFunc getSetHeaderCallback(void);
    typedef T2ERROR (*SetMtlsHeadersFunc)(CURL *, const char *, const char *, childResponse *);
    SetMtlsHeadersFunc getSetMtlsHeadersCallback(void);
    typedef T2ERROR (*SetPayloadFunc)(CURL *, const char *, childResponse *);
    SetPayloadFunc getSetPayloadCallback(void);

    // Global variables to control the mock
    namespace {
    static int mock_setopt_call = 0, fail_on_call = 0;
    static CURLcode fail_with_code = CURLE_OK;

    extern "C" CURLcode curl_easy_setopt_mock(CURL *curl, int option, ...) {
       mock_setopt_call++;
       if (fail_on_call > 0 && mock_setopt_call == fail_on_call) {
          return fail_with_code;
       }
       return CURLE_OK;
    }
    extern "C" struct curl_slist* curl_slist_append(struct curl_slist* list, const char* val) {
       return (struct curl_slist*)0x1; // fake address
    }
}

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <rbus/rbus.h>
#include <rbus/rbus_value.h>
using namespace std;

#include <iostream>
#include <stdexcept>
#define curl_easy_setopt curl_easy_setopt_mock
#define curl_easy_getinfo curl_easy_getinfo_mock
#include "test/mocks/SystemMock.h"
#include "test/mocks/FileioMock.h"
#undef curl_easy_setopt
#undef curl_easy_getinfo
#include "test/mocks/rbusMock.h"
#include "test/mocks/rdklogMock.h"
#include "test/xconf-client/xconfclientMock.h"
#include "protocolMock.h"
#include "test/mocks/rdkconfigMock.h"

extern FileMock *g_fileIOMock;
using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;


class protocolTestFixture : public ::testing::Test {
protected:
    void SetUp() override
    {
        g_fileIOMock = new FileMock();
        g_systemMock = new SystemMock();
        m_xconfclientMock = new XconfclientMock();
        g_rbusMock = new rbusMock();
        g_rdkconfigMock = new rdkconfigMock();

    }

    void TearDown() override
    {
        delete g_fileIOMock;
        delete g_systemMock;
        delete m_xconfclientMock;
        delete g_rbusMock;
	delete g_rdkconfigMock;

        g_fileIOMock = nullptr;
        g_systemMock = nullptr;
        m_xconfclientMock = nullptr;
        g_rbusMock = nullptr;
	g_rdkconfigMock = nullptr;
    }
};

#if 0
//typedef size_t (*WriteToFileFunc)(void *, size_t, size_t, void *);

// Declaration of the getter function to retrieve the function pointer
extern "C" {
    typedef size_t (*WriteToFileFunc)(void *, size_t, size_t, void *);
    WriteToFileFunc getWriteToFileCallback(void);
    typedef T2ERROR (*SetHeaderFunc)(CURL *, const char *, struct curl_slist **, childResponse *);
    SetHeaderFunc getSetHeaderCallback(void);
}
#endif
TEST(SENDREPORTOVERHTTP, 1_NULL_CHECK)
{
    char *payload = "This is a payload string";
    EXPECT_EQ(T2ERROR_FAILURE, sendReportOverHTTP(NULL, payload, NULL));
}

TEST(SENDREPORTOVERHTTP, 2_NULL_CHECK)
{
    char *url = "https://test.com";
    EXPECT_EQ(T2ERROR_FAILURE, sendReportOverHTTP(url, NULL, NULL));
}
TEST(SENDCACREPOVERHTTP, 1_NULL_CHECK)
{
    Vector* reportlist;
    Vector_Create(&reportlist);
    Vector_PushBack(reportlist, strdup("This is a payload string"));
    Vector_PushBack(reportlist, strdup("This is a output string"));
    EXPECT_EQ(T2ERROR_FAILURE, sendCachedReportsOverHTTP(NULL, reportlist));
    Vector_Destroy(reportlist, free);
}

TEST(SENDCACREPOVERHTTP, 2_NULL_CHECK)
{
    char *url = "https://test.com";
    EXPECT_EQ(T2ERROR_FAILURE, sendCachedReportsOverHTTP(url, NULL));
}

TEST(SENDRBUDREPORTOVERRBUS, 1_NULL_CHECK)
{
    Vector* inputParams = NULL;
    Vector_Create(&inputParams);
    char* payload = "This is a payload string";
    Vector_PushBack(inputParams, strdup(payload));
    EXPECT_EQ(T2ERROR_FAILURE, sendReportsOverRBUSMethod(NULL, inputParams, payload));
    Vector_Destroy(inputParams, free);
}

TEST(SENDRBUDREPORTOVERRBUS, 2_NULL_CHECK)
{
     char* method = "RBUS_METHOD";
     char* payload = "This is a payload string";
     EXPECT_EQ(T2ERROR_FAILURE, sendReportsOverRBUSMethod(method, NULL, payload));
}

TEST(SENDRBUDREPORTOVERRBUS,3_NULL_CHECK)
{
    char* method = "RBUS_METHOD";
    Vector* inputParams = NULL;
    Vector_Create(&inputParams);
    char* payload = "This is a payload string";
    Vector_PushBack(inputParams, strdup(payload));
    EXPECT_EQ(T2ERROR_FAILURE, sendReportsOverRBUSMethod(method, inputParams, NULL));
    EXPECT_EQ(T2ERROR_FAILURE, sendReportsOverRBUSMethod(NULL, NULL, NULL));
    Vector_Destroy(inputParams, free);
}

TEST(SENDRBUSCACHEREPORTOVERRBUS, NULL_CHECK)
{
    char* method = "RBUS_METHOD";
    Vector* inputParams = NULL;
    Vector_Create(&inputParams);
    Vector_PushBack(inputParams, strdup("This is a payload string"));
    Vector_PushBack(inputParams, strdup("This is a output string"));
    Vector* reportList = NULL;
    Vector_Create(&reportList);
    Vector_PushBack(reportList, strdup("This is a payload string"));
    Vector_PushBack(reportList, strdup("This is a output string"));
    EXPECT_EQ(T2ERROR_FAILURE, sendCachedReportsOverRBUSMethod(NULL, inputParams, reportList));
    EXPECT_EQ(T2ERROR_FAILURE, sendCachedReportsOverRBUSMethod(method, NULL, reportList));
    EXPECT_EQ(T2ERROR_FAILURE, sendCachedReportsOverRBUSMethod(method, inputParams, NULL));
    EXPECT_EQ(T2ERROR_FAILURE, sendCachedReportsOverRBUSMethod(NULL,NULL,NULL));
    Vector_Destroy(inputParams, free);
    Vector_Destroy(reportList, free);
}

TEST_F(protocolTestFixture, SENDREPORTOVERHTTP1)
{
      char* httpURL = "https://mockxconf:50051/dataLakeMock";
      char* payload = strdup("This is a payload string");
      EXPECT_CALL(*g_fileIOMock, pipe(_))
              .Times(1)
              .WillOnce(Return(-1));
      EXPECT_EQ(T2ERROR_FAILURE, sendReportOverHTTP(httpURL, payload, NULL));
      free(payload);
}

TEST_F(protocolTestFixture, SENDREPORTOVERHTTP2)
{
      char* httpURL = "https://mockxconf:50051/dataLakeMock";
      char* payload = strdup("This is a payload string");
      char *cm = (char*)0xFFFFFFFF;
      EXPECT_CALL(*g_fileIOMock, pipe(_))
              .Times(1)
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
              .WillOnce(Return(true));
      EXPECT_CALL(*g_systemMock, access(_,_))                                                                                      
             .Times(1)                                                                                                             
             .WillOnce(Return(0));                                                                                                 
      #ifdef LIBRDKCONFIG_BUILD                                                                                                    
      EXPECT_CALL(*g_rdkconfigMock, rdkconfig_get(_,_,_))                                                                          
             .Times(1)                                                                                                             
             .WillOnce(Return(RDKCONFIG_OK));                                                                                      
      #endif
      EXPECT_CALL(*g_fileIOMock, fork())
              .Times(1)
              .WillOnce(Return(-1));
      EXPECT_EQ(T2ERROR_FAILURE, sendReportOverHTTP(httpURL, payload, NULL));
      free(payload);
}

//parent process
TEST_F(protocolTestFixture, SENDREPORTOVERHTTP3)
{
      char* httpURL = "https://mockxconf:50051/dataLakeMock";
      char* payload = strdup("This is a payload string");
      char *cm = (char*)0xFFFFFFFF;
      EXPECT_CALL(*g_fileIOMock, pipe(_))
              .Times(1)
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
              .WillOnce(Return(true));
      EXPECT_CALL(*g_systemMock, access(_,_))
             .Times(1)
             .WillOnce(Return(0));
      #ifdef LIBRDKCONFIG_BUILD
      EXPECT_CALL(*g_rdkconfigMock, rdkconfig_get(_,_,_))
             .Times(1)
             .WillOnce(Return(RDKCONFIG_OK));
      #endif
      EXPECT_CALL(*g_fileIOMock, fork())
              .Times(1)
              .WillOnce(Return(1));
      EXPECT_CALL(*g_fileIOMock, close(_))
              .Times(2)
              .WillOnce(Return(-1))
              .WillOnce(Return(-1));
      EXPECT_CALL(*g_fileIOMock, read(_,_,_))
              .Times(1)
              .WillOnce(Return(-1));
      EXPECT_EQ(T2ERROR_SUCCESS, sendReportOverHTTP(httpURL, payload, NULL));
      free(payload);
}

TEST_F(protocolTestFixture, SENDCACHEDREPORTOVERHTTP)
{
      char* httpURL = "https://mockxconf:50051/dataLakeMock";
      char* payload = strdup("This is a payload string");
      Vector* reportlist = NULL;
      Vector_Create(&reportlist);
      Vector_PushBack(reportlist, payload);
      char *cm = (char*)0xFFFFFFFF;
      EXPECT_CALL(*g_fileIOMock, pipe(_))
              .Times(1)
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
              .WillOnce(Return(true));
      EXPECT_CALL(*g_systemMock, access(_,_))
             .Times(1)
             .WillOnce(Return(0));
      #ifdef LIBRDKCONFIG_BUILD
      EXPECT_CALL(*g_rdkconfigMock, rdkconfig_get(_,_,_))
             .Times(1)
             .WillOnce(Return(RDKCONFIG_OK));
      #endif
      EXPECT_CALL(*g_fileIOMock, fork())
              .Times(1)
              .WillOnce(Return(1));
      EXPECT_CALL(*g_fileIOMock, close(_))
              .Times(2)
              .WillOnce(Return(-1))
              .WillOnce(Return(-1));
      EXPECT_CALL(*g_fileIOMock, read(_,_,_))
              .Times(1)
              .WillOnce(Return(-1));
      EXPECT_EQ(T2ERROR_SUCCESS, sendCachedReportsOverHTTP(httpURL, reportlist));
      Vector_Destroy(reportlist, free);
}


TEST_F(protocolTestFixture, SENDREPORTSOVERRBUSMETHOD1)
{
    char* method = strdup("RBUS_METHOD");
    RBUSMethodParam *rbusMethodParam = (RBUSMethodParam *) malloc(sizeof(RBUSMethodParam));
    rbusMethodParam->name = "Device.X_RDK_Xmidt.SendData";
    rbusMethodParam->value = "This is a value string";
    Vector* inputParams = NULL;
    Vector_Create(&inputParams);
    Vector_PushBack(inputParams, rbusMethodParam);
    char* payload = strdup("This is a payload string");
    EXPECT_CALL(*g_rbusMock, rbusObject_Init(_,_))
            .Times(1)
            .WillOnce(Return((rbusObject_t)0xffffffff));
    EXPECT_CALL(*g_rbusMock, rbusValue_Init(_))
            .Times(3)
            .WillOnce(Return((rbusValue_t)0xffffffff))
            .WillOnce(Return((rbusValue_t)0xffffffff))
            .WillOnce(Return((rbusValue_t)0xffffffff));
    EXPECT_CALL(*g_rbusMock, rbusValue_SetString(_,_))
            .Times(2);
    EXPECT_CALL(*g_rbusMock, rbusObject_SetValue(_,_,_))
            .Times(3);
    EXPECT_CALL(*g_rbusMock, rbusValue_Release(_))
            .Times(3);
    EXPECT_CALL(*g_rbusMock, rbusValue_SetInt32(_,_))
            .Times(1);
    EXPECT_CALL(*g_rbusMock, rbusMethodCaller(_,_,_,_))
            .Times(1)
            .WillOnce(Return(T2ERROR_FAILURE));
    EXPECT_CALL(*g_rbusMock, rbusObject_Release(_))
            .Times(1);
    EXPECT_EQ(T2ERROR_FAILURE, sendReportsOverRBUSMethod(method, inputParams, payload));
    free(method);
    free(payload);
    Vector_Destroy(inputParams, free);
}

TEST_F(protocolTestFixture, SENDREPORTSOVERRBUSMETHOD2)
{
    char* method = strdup("RBUS_METHOD");
    RBUSMethodParam *rbusMethodParam = (RBUSMethodParam *) malloc(sizeof(RBUSMethodParam));
    rbusMethodParam->name = "Device.X_RDK_Xmidt.SendData";
    rbusMethodParam->value = "This is a value string";
    Vector* inputParams = NULL;
    Vector_Create(&inputParams);
    Vector_PushBack(inputParams, rbusMethodParam);
    char* payload = strdup("This is a payload string");
    EXPECT_CALL(*g_rbusMock, rbusObject_Init(_,_))
            .Times(1)
            .WillOnce(Return((rbusObject_t)0xffffffff));
    EXPECT_CALL(*g_rbusMock, rbusValue_Init(_))
            .Times(3)
            .WillOnce(Return((rbusValue_t)0xffffffff))
            .WillOnce(Return((rbusValue_t)0xffffffff))
            .WillOnce(Return((rbusValue_t)0xffffffff));
    EXPECT_CALL(*g_rbusMock, rbusValue_SetString(_,_))
            .Times(2);
    EXPECT_CALL(*g_rbusMock, rbusObject_SetValue(_,_,_))
            .Times(3);
    EXPECT_CALL(*g_rbusMock, rbusValue_Release(_))
            .Times(3);
    EXPECT_CALL(*g_rbusMock, rbusValue_SetInt32(_,_))
            .Times(1);
    EXPECT_CALL(*g_rbusMock, rbusMethodCaller(_,_,_,_))
            .Times(1)
            .WillOnce(Return(T2ERROR_SUCCESS));
    EXPECT_CALL(*g_rbusMock, rbusObject_Release(_))
            .Times(1);
    EXPECT_EQ(T2ERROR_NO_RBUS_METHOD_PROVIDER, sendReportsOverRBUSMethod(method, inputParams, payload));
    free(method);
    free(payload);
    Vector_Destroy(inputParams, free);
}

TEST_F(protocolTestFixture, sendCachedReportsOverRBUSMethod)
{
    char* method = strdup("RBUS_METHOD");
    RBUSMethodParam *rbusMethodParam = (RBUSMethodParam *) malloc(sizeof(RBUSMethodParam));
    rbusMethodParam->name = "Device.X_RDK_Xmidt.SendData";
    rbusMethodParam->value = "This is a value string";
    Vector* inputParams = NULL;
    Vector_Create(&inputParams);
    Vector_PushBack(inputParams, rbusMethodParam);
    char* payload = strdup("This is a payload string");
    Vector* reportlist = NULL;
    Vector_Create(&reportlist);
    Vector_PushBack(reportlist, payload);
    EXPECT_CALL(*g_rbusMock, rbusObject_Init(_,_))
            .Times(1)
            .WillOnce(Return((rbusObject_t)0xffffffff));
    EXPECT_CALL(*g_rbusMock, rbusValue_Init(_))
            .Times(3)
            .WillOnce(Return((rbusValue_t)0xffffffff))
            .WillOnce(Return((rbusValue_t)0xffffffff))
            .WillOnce(Return((rbusValue_t)0xffffffff));
    EXPECT_CALL(*g_rbusMock, rbusValue_SetString(_,_))
            .Times(2);
    EXPECT_CALL(*g_rbusMock, rbusObject_SetValue(_,_,_))
            .Times(3);
    EXPECT_CALL(*g_rbusMock, rbusValue_Release(_))
            .Times(3);
    EXPECT_CALL(*g_rbusMock, rbusValue_SetInt32(_,_))
            .Times(1);
    EXPECT_CALL(*g_rbusMock, rbusMethodCaller(_,_,_,_))
            .Times(1)
            .WillOnce(Return(T2ERROR_SUCCESS));
    EXPECT_CALL(*g_rbusMock, rbusObject_Release(_))
            .Times(1);
    EXPECT_EQ(T2ERROR_FAILURE, sendCachedReportsOverRBUSMethod(method, inputParams, reportlist));
    free(method);
    Vector_Destroy(inputParams, free);
    Vector_Destroy(reportlist,free);
}

//sendReportOverHTTP
TEST_F(protocolTestFixture, sendReportOverHTTP_6)
{
 char* httpURL = "https://mockxconf:50051/dataLakeMock";
      char* payload = strdup("This is a payload string");
      Vector* reportlist = NULL;
      Vector_Create(&reportlist);
      Vector_PushBack(reportlist, payload);
      char *cm = (char*)0xFFFFFFFF;
      EXPECT_CALL(*g_fileIOMock, pipe(_))
              .Times(1)
              .WillOnce(Return(0));
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
              .WillOnce(Return(true));
      EXPECT_CALL(*g_systemMock, access(_,_))
             .Times(1)
             .WillOnce(Return(0));
      #ifdef LIBRDKCONFIG_BUILD
      EXPECT_CALL(*g_rdkconfigMock, rdkconfig_get(_,_,_))
             .Times(1)
             .WillOnce(Return(RDKCONFIG_OK));
      #endif
      EXPECT_CALL(*g_fileIOMock, fork())
              .Times(1)
              .WillOnce(Return(1));
      EXPECT_CALL(*g_fileIOMock, close(_))
              .Times(2)
              .WillOnce(Return(-1))
              .WillOnce(Return(-1));
      EXPECT_CALL(*g_fileIOMock, read(_,_,_))
              .Times(1)
              .WillOnce([](int fd, void *buf, size_t count) {
                 childResponse* resp = (childResponse*)buf;
                 resp->curlStatus = true;
                 resp->curlResponse = CURLE_OK;
                 resp->curlSetopCode = CURLE_OK;
                 resp->http_code = 200;
                 resp->lineNumber = 123; // Set to test value
                 return sizeof(childResponse);
              });
     #ifdef LIBRDKCONFIG_BUILD
      EXPECT_CALL(*g_rdkconfigMock, rdkconfig_free(_, _))
             .Times(1)
             .WillOnce(Return(RDKCONFIG_OK));
      #endif
      EXPECT_EQ(T2ERROR_SUCCESS, sendReportOverHTTP(httpURL, payload,NULL));
      Vector_Destroy(reportlist, free);
}

// New test case to cover the failure handling in sendCachedReportsOverHTTP
TEST_F(protocolTestFixture, sendCachedReportsOverHTTP_FailureCase)
{
    char *httpURL = "https://mockxconf:50051/dataLakeMock";
    Vector* reportList = NULL;
    Vector_Create(&reportList);

    // Add two payloads to the report list
    char* payload1 = strdup("This is payload 1");
    char* payload2 = strdup("This is payload 2");
    Vector_PushBack(reportList, payload1);
    Vector_PushBack(reportList, payload2);

    // Mock failure for sendReportOverHTTP on the first payload
    EXPECT_CALL(*g_fileIOMock, pipe(_))
        .Times(1)
        .WillOnce(Return(0));

    EXPECT_CALL(*m_xconfclientMock, isMtlsEnabled())
        .Times(1)
        .WillOnce(Return(false));

    EXPECT_CALL(*g_fileIOMock, fork())
        .Times(1)
        .WillOnce(Return(-1));

    // Ensure that the function returns a failure due to the mocked failure
    EXPECT_EQ(T2ERROR_FAILURE, sendCachedReportsOverHTTP(httpURL, reportList));

    // Clean up
    Vector_Destroy(reportList, free);
}

 // Unit test for static writeToFile via its function pointer
 TEST(CURLINTERFACE_STATIC, WriteToFile)
 {
     WriteToFileFunc writeToFileCb = getWriteToFileCallback();
     ASSERT_NE(writeToFileCb, nullptr);

     // Prepare a buffer and write to a file
     const char msg[] = "test fwrite";
     char testFile[] = "/tmp/unittest_writeToFileXXXXXX";
     int fd = mkstemp(testFile);
     ASSERT_NE(fd, -1);
     FILE* fp = fdopen(fd, "wb ");
     ASSERT_NE(fp, nullptr);

     size_t n = writeToFileCb((void*)msg, sizeof(char), sizeof(msg), (void*)fp);
     ASSERT_EQ(n, sizeof(msg));
     fclose(fp);

     // Now read back and validate
     fp = fopen(testFile, "rb");
     ASSERT_NE(fp, nullptr);
     char buf[32];
     size_t bytes = fread(buf, sizeof(char), sizeof(msg), fp);
     ASSERT_EQ(bytes, sizeof(msg));
     ASSERT_EQ(memcmp(buf, msg, sizeof(msg)), 0);
     fclose(fp);
     remove(testFile);
 }

TEST(CURLINTERFACE_STATIC, SetHeader)
{
    SetHeaderFunc setHeaderCb = getSetHeaderCallback();
    ASSERT_NE(setHeaderCb, nullptr);
    CURL *curl = nullptr; // purposely NULL
    const char *destURL = "http://localhost";
    struct curl_slist *headerList = nullptr;
    childResponse resp;
    T2ERROR result = setHeaderCb(curl, destURL, &headerList, &resp);
    // According to implementation, curl==NULL returns T2ERROR_FAILURE
    EXPECT_EQ(result, T2ERROR_FAILURE);
}
#if 0
TEST(CURLINTERFACE_STATIC, SetHeader_NULL_destURL) {
    SetHeaderFunc setHeaderCb = getSetHeaderCallback();
    ASSERT_NE(setHeaderCb, nullptr);
    struct curl_slist *headerList = nullptr;
    childResponse resp;
    EXPECT_EQ(setHeaderCb((CURL*)0xDEADBEEF, nullptr, &headerList, &resp), T2ERROR_FAILURE);
}

// Simulate a failure on every call to curl_easy_setopt
TEST(CURLINTERFACE_STATIC, SetHeader_first_setopt_fails) {
    SetHeaderFunc setHeaderCb = getSetHeaderCallback();
    ASSERT_NE(setHeaderCb, nullptr);
    struct curl_slist *headerList = nullptr;
    childResponse resp;
    mock_setopt_call = 0; fail_on_call = 1; fail_with_code = CURLE_INTERFACE_FAILED;
    EXPECT_EQ(setHeaderCb((CURL*)0x1, "http://test", &headerList, &resp), T2ERROR_FAILURE);
}
TEST(CURLINTERFACE_STATIC, SetHeader_second_setopt_fails) {
    SetHeaderFunc setHeaderCb = getSetHeaderCallback();
    ASSERT_NE(setHeaderCb, nullptr);
    struct curl_slist *headerList = nullptr;
    childResponse resp;
    mock_setopt_call = 0; fail_on_call = 2; fail_with_code = CURLE_SSL_CACERT_BADFILE;
    EXPECT_EQ(setHeaderCb((CURL*)0x2, "http://test", &headerList, &resp), T2ERROR_FAILURE);
}
TEST(CURLINTERFACE_STATIC, SetHeader_third_setopt_fails) {
    SetHeaderFunc setHeaderCb = getSetHeaderCallback();
    ASSERT_NE(setHeaderCb, nullptr);
    struct curl_slist *headerList = nullptr;
    childResponse resp;
    mock_setopt_call = 0; fail_on_call = 3; fail_with_code = CURLE_SSL_CERTPROBLEM;
    EXPECT_EQ(setHeaderCb((CURL*)0x3, "http://test", &headerList, &resp), T2ERROR_FAILURE);
}
TEST(CURLINTERFACE_STATIC, SetHeader_fourth_setopt_fails) {
    SetHeaderFunc setHeaderCb = getSetHeaderCallback();
    ASSERT_NE(setHeaderCb, nullptr);
    struct curl_slist *headerList = nullptr;
    childResponse resp;
    mock_setopt_call = 0; fail_on_call = 4; fail_with_code = CURLE_SSL_CIPHER;
    EXPECT_EQ(setHeaderCb((CURL*)0x4, "http://test", &headerList, &resp), T2ERROR_FAILURE);
}
// For RDKB, WAN failover/config flags have more, else next step would be 5th setopt.

#if !defined(ENABLE_RDKB_SUPPORT) || defined(RDKB_EXTENDER)
// If not RDKB, next is interface setopt
TEST(CURLINTERFACE_STATIC, SetHeader_fifth_setopt_fails) {
    SetHeaderFunc setHeaderCb = getSetHeaderCallback();
    ASSERT_NE(setHeaderCb, nullptr);
    struct curl_slist *headerList = nullptr;
    childResponse resp;
    mock_setopt_call = 0; fail_on_call = 5; fail_with_code = CURLE_URL_MALFORMAT;
    EXPECT_EQ(setHeaderCb((CURL*)0x5, "http://test", &headerList, &resp), T2ERROR_FAILURE);
}
#endif

// Now simulate HTTPHEADER or WRITEFUNCTION failures (header is 6, writefunction is 7)
TEST(CURLINTERFACE_STATIC, SetHeader_HTTPHEADER_setopt_fails) {
    SetHeaderFunc setHeaderCb = getSetHeaderCallback();
    ASSERT_NE(setHeaderCb, nullptr);
    struct curl_slist *headerList = nullptr;
    childResponse resp;
    mock_setopt_call = 0; fail_on_call = 6; fail_with_code = CURLE_SSL_ISSUER_ERROR;
    EXPECT_EQ(setHeaderCb((CURL*)0x6, "http://test", &headerList, &resp), T2ERROR_FAILURE);
}
TEST(CURLINTERFACE_STATIC, SetHeader_WRITEFUNCTION_setopt_fails) {
    SetHeaderFunc setHeaderCb = getSetHeaderCallback();
    ASSERT_NE(setHeaderCb, nullptr);
    struct curl_slist *headerList = nullptr;
    childResponse resp;
    mock_setopt_call = 0; fail_on_call = 7; fail_with_code = CURLE_SSL_ENGINE_INITFAILED;
    EXPECT_EQ(setHeaderCb((CURL*)0x7, "http://test", &headerList, &resp), T2ERROR_FAILURE);
}

// Finally, successful case - all succeed
TEST(CURLINTERFACE_STATIC, SetHeader_all_success) {
    SetHeaderFunc setHeaderCb = getSetHeaderCallback();
    ASSERT_NE(setHeaderCb, nullptr);
    struct curl_slist *headerList = nullptr;
    childResponse resp;
    mock_setopt_call = 0; fail_on_call = 0; fail_with_code = CURLE_OK;
    EXPECT_EQ(setHeaderCb((CURL*)0x8, "http://test", &headerList, &resp), T2ERROR_SUCCESS);
}
#endif
TEST(CURLINTERFACE_STATIC, SetMtlsHeaders_NULL)
{
    SetMtlsHeadersFunc cb = getSetMtlsHeadersCallback();
    ASSERT_NE(cb, nullptr);
    childResponse resp;
    // NULL for CURL
    EXPECT_EQ(cb(nullptr, "cert", "pwd", &resp), T2ERROR_FAILURE);
    // NULL for certFile
    EXPECT_EQ(cb((CURL*)0x1, nullptr, "pwd", &resp), T2ERROR_FAILURE);
    // NULL for passwd
    EXPECT_EQ(cb((CURL*)0x1, "cert", nullptr, &resp), T2ERROR_FAILURE);
}

TEST(CURLINTERFACE_STATIC, SetPayload_NULL)
{
    SetPayloadFunc cb = getSetPayloadCallback();
    ASSERT_NE(cb, nullptr);
    childResponse resp;
    // NULL for CURL
    EXPECT_EQ(cb(nullptr, "payload", &resp), T2ERROR_FAILURE);
    // NULL for payload
    EXPECT_EQ(cb((CURL*)0x1, nullptr, &resp), T2ERROR_FAILURE);
}
