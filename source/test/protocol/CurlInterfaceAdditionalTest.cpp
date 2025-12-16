/*
 Additional tests for protocol/http (curlinterface) to improve coverage.

 Mirrors the mocking pattern used in ProtocolTest.cpp.
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

}

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <rbus/rbus.h>
#include <rbus/rbus_value.h>
using namespace std;

#include <iostream>
#include <stdexcept>

// Keep same curl mocks macro pattern as existing tests so curl_easy_* map to mocks
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
extern SystemMock *g_systemMock;
extern XconfclientMock *m_xconfclientMock;
extern rbusMock *g_rbusMock;
extern rdkconfigMock *g_rdkconfigMock;

using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::DoAll;
using ::testing::SetArrayArgument;
using ::testing::Invoke;
using ::testing::WithArgs;
using ::testing::SaveArgPointee;
using ::testing::InvokeWithoutArgs;
using ::testing::ReturnArg;

class CurlInterfaceAdditionalTestFixture : public ::testing::Test {
protected:
    void SetUp() override
    {
        // reuse same injection pattern as other tests
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

/*
 Test: when the child reports HTTP 500 the parent should return failure.
 This covers the branch where curl returns OK but HTTP response != 200.
*/
TEST_F(CurlInterfaceAdditionalTestFixture, sendReportOverHTTP_http_500_results_in_failure)
{
    char* httpURL = (char*)"https://mockxconf:50051/dataLakeMock";
    char* payload = strdup("This is a payload string");

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

    // Simulate mTLS disabled to exercise branch where we don't try to fetch certs
    EXPECT_CALL(*m_xconfclientMock, isMtlsEnabled())
            .Times(1)
            .WillOnce(Return(false));

    EXPECT_CALL(*g_systemMock, access(_,_))
           .Times(0); // shouldn't be called when mTLS disabled in this config

#ifdef LIBRDKCONFIG_BUILD
    EXPECT_CALL(*g_rdkconfigMock, rdkconfig_get(_,_,_))
           .Times(0);
#endif

    // Parent branch
    EXPECT_CALL(*g_fileIOMock, fork())
            .Times(1)
            .WillOnce(Return(1)); // parent

    EXPECT_CALL(*g_fileIOMock, close(_))
            .Times(2)
            .WillOnce(Return(-1))
            .WillOnce(Return(-1));

    // Provide childResponse indicating HTTP 500
    EXPECT_CALL(*g_fileIOMock, read(_,_,_))
            .Times(1)
            .WillOnce([](int fd, void *buf, size_t count) {
                 childResponse* resp = (childResponse*)buf;
                 resp->curlStatus = true;
                 resp->curlResponse = CURLE_OK;
                 resp->curlSetopCode = CURLE_OK;
                 resp->http_code = 500;
                 resp->lineNumber = 200;
                 return sizeof(childResponse);
            });

#ifdef LIBRDKCONFIG_BUILD
    EXPECT_CALL(*g_rdkconfigMock, rdkconfig_free(_, _))
           .Times(0);
#endif

    EXPECT_EQ(T2ERROR_FAILURE, sendReportOverHTTP(httpURL, payload, NULL));
    free(payload);
}

/*
 Test: when the child reports a curl error (e.g., CURLE_COULDNT_CONNECT), parent should return failure.
 Covers branch where curlResponse != CURLE_OK.
*/
TEST_F(CurlInterfaceAdditionalTestFixture, sendReportOverHTTP_curl_error_results_in_failure)
{
    char* httpURL = (char*)"https://mockxconf:50051/dataLakeMock";
    char* payload = strdup("This is a payload string");

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

    // simulate mtls disabled
    EXPECT_CALL(*m_xconfclientMock, isMtlsEnabled())
            .Times(1)
            .WillOnce(Return(false));

    EXPECT_CALL(*g_fileIOMock, fork())
            .Times(1)
            .WillOnce(Return(1)); // parent

    EXPECT_CALL(*g_fileIOMock, close(_))
            .Times(2)
            .WillOnce(Return(-1))
            .WillOnce(Return(-1));

    // Provide childResponse indicating curl error
    EXPECT_CALL(*g_fileIOMock, read(_,_,_))
            .Times(1)
            .WillOnce([](int fd, void *buf, size_t count) {
                 childResponse* resp = (childResponse*)buf;
                 resp->curlStatus = true;
                 resp->curlResponse = CURLE_COULDNT_CONNECT;
                 resp->curlSetopCode = CURLE_OK;
                 resp->http_code = 0;
                 resp->lineNumber = 300;
                 return sizeof(childResponse);
            });

    EXPECT_EQ(T2ERROR_FAILURE, sendReportOverHTTP(httpURL, payload, NULL));
    free(payload);
}

/*
 Test: sendCachedReportsOverHTTP when multiple cached reports exist and the mocked child
 returns success - exercise loop behavior.
*/
TEST_F(CurlInterfaceAdditionalTestFixture, sendCachedReportsOverHTTP_multiple_reports_success)
{
    char* httpURL = (char*)"https://mockxconf:50051/dataLakeMock";
    Vector* reportlist = NULL;
    Vector_Create(&reportlist);
    // Two payloads to exercise loop iterating twice
    Vector_PushBack(reportlist, strdup("payload 1"));
    Vector_PushBack(reportlist, strdup("payload 2"));

    // We'll allow sendReportOverHTTP to succeed twice by returning success childResponse twice.
    // The code creates a pipe and forks for every sendReportOverHTTP call; our mock expectations
    // will be called twice in order.

    // First sendReportOverHTTP call's pipe
    EXPECT_CALL(*g_fileIOMock, pipe(_))
            .Times(2)
            .WillRepeatedly(Return(0));

#if defined(ENABLE_RDKB_SUPPORT) && !defined(RDKB_EXTENDER)
  #if defined(WAN_FAILOVER_SUPPORTED) || defined(FEATURE_RDKB_CONFIGURABLE_WAN_INTERFACE)
    EXPECT_CALL(*m_xconfclientMock, getParameterValue(_,_))
            .Times(2)
            .WillRepeatedly(Return(T2ERROR_SUCCESS));
  #endif
#endif

    // Simulate mtls disabled for simplicity
    EXPECT_CALL(*m_xconfclientMock, isMtlsEnabled())
            .Times(2)
            .WillRepeatedly(Return(false));

    // fork called twice, always return parent
    EXPECT_CALL(*g_fileIOMock, fork())
            .Times(2)
            .WillRepeatedly(Return(1)); // parent

    // Each parent call closes 2 fds; set expectations for both calls (2 * 2)
    EXPECT_CALL(*g_fileIOMock, close(_))
            .Times(4)
            .WillRepeatedly(Return(-1));

    // Provide two successful childResponses (CURLE_OK + http_code 200)
    EXPECT_CALL(*g_fileIOMock, read(_,_,_))
            .Times(2)
            .WillOnce([](int fd, void *buf, size_t count) {
                 childResponse* resp = (childResponse*)buf;
                 resp->curlStatus = true;
                 resp->curlResponse = CURLE_OK;
                 resp->curlSetopCode = CURLE_OK;
                 resp->http_code = 200;
                 resp->lineNumber = 400;
                 return sizeof(childResponse);
            })
            .WillOnce([](int fd, void *buf, size_t count) {
                 childResponse* resp = (childResponse*)buf;
                 resp->curlStatus = true;
                 resp->curlResponse = CURLE_OK;
                 resp->curlSetopCode = CURLE_OK;
                 resp->http_code = 200;
                 resp->lineNumber = 401;
                 return sizeof(childResponse);
            });

    EXPECT_EQ(T2ERROR_SUCCESS, sendCachedReportsOverHTTP(httpURL, reportlist));
    // sendCachedReportsOverHTTP frees payloads itself; reportlist should be empty/destroyable
    Vector_Destroy(reportlist, free);
}

