/*
 * If not stated otherwise in this file or this component's LICENSE file the 
 * following copyright and licenses apply:
 *
 * Copyright 2024 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * You may not use this file except in compliance with the License.
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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

extern "C" {
#include "reportprofiles.h"
#include "profile.h"
#include "t2eventreceiver.h"
#include "t2collection.h"
#include "t2log_wrapper.h"
#include "msgpack.h"
}

#include "../mocks/SystemMock.h"
#include "../mocks/FileioMock.h"
#include "../mocks/rdklogMock.h"
#include "../mocks/rbusMock.h"
#include "../mocks/VectorMock.h"
#include "SchedulerMock.h"

using namespace std;
using ::testing::_;
using ::testing::Return;

// Create global mocks if you need them just like t2markersTest.cpp
SystemMock* g_systemMock = nullptr;
FileMock* g_fileIOMock = nullptr;
rdklogMock* m_rdklogMock = nullptr;
rbusMock* g_rbusMock = nullptr;
VectorMock* g_vectorMock = nullptr;

// Test fixture for reportprofiles
class reportprofilesTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        g_systemMock = new SystemMock();
        g_fileIOMock = new FileMock();
        m_rdklogMock = new rdklogMock();
        g_rbusMock = new rbusMock();
        g_vectorMock = new VectorMock();
    }
    void TearDown() override {
        delete g_systemMock;
        delete g_fileIOMock;
        delete m_rdklogMock;
        delete g_rbusMock;
        delete g_vectorMock;
        g_systemMock = nullptr;
        g_fileIOMock = nullptr;
        m_rdklogMock = nullptr;
        g_rbusMock = nullptr;
        g_vectorMock = nullptr;
    }
};

TEST_F(reportprofilesTestFixture, InitReportProfilesSuccess) {
    EXPECT_EQ(initReportProfiles(), T2ERROR_SUCCESS);
}

TEST_F(reportprofilesTestFixture, ReportProfilesUninitSuccess) {
    EXPECT_EQ(ReportProfiles_uninit(), T2ERROR_SUCCESS);
}

TEST_F(reportprofilesTestFixture, DeleteAllReportProfiles) {
    EXPECT_EQ(deleteAllReportProfiles(), T2ERROR_SUCCESS);
}

TEST_F(reportprofilesTestFixture, ReportProfilesInterrupt) {
    // This function could just run, but you might want to check side effects
    ReportProfiles_Interrupt();
}

TEST_F(reportprofilesTestFixture, GenerateDcaReportNotDelayed) {
    generateDcaReport(false, false);
}

TEST_F(reportprofilesTestFixture, PrivacymodeDoNotShareSuccess) {
    EXPECT_EQ(privacymode_do_not_share(), T2ERROR_SUCCESS);
}

TEST_F(reportprofilesTestFixture, StoreMarkerEventNullProfile) {
    EXPECT_EQ(ReportProfiles_storeMarkerEvent(NULL, NULL), T2ERROR_SUCCESS);
}

TEST_F(reportprofilesTestFixture, ProfilememUsageApiWorks) {
    unsigned int value = 0;
    profilemem_usage(&value);
    EXPECT_GE(value, 0u);
}

TEST_F(reportprofilesTestFixture, T2totalmemCalculateApiWorks) {
    T2totalmem_calculate();
}

#if 0
// Example test for MsgPack blob processing
TEST_F(reportprofilesTestFixture, MsgpackBlobValidInput) {
    // Generate a valid msgpack blob, fill struct __msgpack__ and call the function
    // This is just a placeholder/template!
    struct __msgpack__ msg;
    msg.msgpack_blob = /* assign pointer to valid msgpack binary */;
    msg.msgpack_blob_size = /* assign size */;
    EXPECT_LE(__ReportProfiles_ProcessReportProfilesMsgPackBlob(&msg, false), T2ERROR_SUCCESS);
}
#endif
// Add more tests for edge cases and negative scenarios as needed

