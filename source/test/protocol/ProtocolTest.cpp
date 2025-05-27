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
}

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "test/rbus/include/rbus.h"
#include "test/rbus/include/rbus_value.h"
using namespace std;

#include <iostream>
#include <stdexcept>
#include "test/mocks/SystemMock.h"
#include "test/mocks/FileioMock.h"
#include "test/mocks/rbusMock.h"
#include "test/mocks/rdklogMock.h"
#include "test/xconf-client/xconfclientMock.h"
#include "protocolMock.h"

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
        //m_protocolMock = new ProtocolMock();
        g_rbusMock = new rbusMock();
    }

    void TearDown() override
    {
        delete g_fileIOMock;
        delete g_systemMock;
        delete m_xconfclientMock;
//      delete m_protocolMock;
        delete g_rbusMock;

        g_fileIOMock = nullptr;
        g_systemMock = nullptr;
        m_xconfclientMock = nullptr;
//      m_protocolMock = nullptr;
        g_rbusMock = nullptr;
    }
};


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
      EXPECT_CALL(*m_xconfclientMock, getParameterValue(_,_)
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
      EXPECT_CALL(*m_xconfclientMock, getParameterValue(_,_)
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
/*
TEST_F(protocolTestFixture, SENDREPORTOVERHTTP4)
{
      char* httpURL = "https://mockxconf:50051/dataLakeMock";
      char* payload = strdup("This is a payload string");
      char *cm = (char*)0xFFFFFFFF;
      EXPECT_CALL(*g_fileIOMock, pipe(_))
              .Times(1)
              .WillOnce(Return(0));
      #if defined(ENABLE_RDKB_SUPPORT) && !defined(RDKB_EXTENDER)
      #if defined(WAN_FAILOVER_SUPPORTED) || defined(FEATURE_RDKB_CONFIGURABLE_WAN_INTERFACE)
      EXPECT_CALL(*m_xconfclientMock, getParameterValue(_,_)
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
              .WillOnce(Return(0));
      CURL* curl = (CURL*)nullptr;
      EXPECT_CALL(*g_fileIOMock, curl_easy_init())
              .Times(1)
              .WillOnce(Return(curl));
      EXPECT_CALL(*g_fileIOMock, close(_))
              .Times(2)
              .WillOnce(Return(-1))
              .WillOnce(Return(-1));
      EXPECT_CALL(*g_fileIOMock, write(_,_,_))
              .Times(1)
              .WillOnce(Return(-1));
      sendReportOverHTTP(httpURL, payload, NULL);
      free(payload);
}
*/
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
      EXPECT_CALL(*m_xconfclientMock, getParameterValue(_,_)
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
    Vector_Destroy(reportlist,free);
    Vector_Destroy(inputParams, free);
}
