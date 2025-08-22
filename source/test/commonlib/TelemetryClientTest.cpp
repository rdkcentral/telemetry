/*
 * Copyright 2025 RDK Management
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
//#include <telemetry_busmessage_sender.h>
}

// Google Test/Mock headers
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::_;
using ::testing::Return;

// Mock definitions for telemetry_busmessage_sender.h
class TelemetryBusMock {
public:
    MOCK_METHOD(int, t2_init, (const char*), ());
    MOCK_METHOD(int, t2_event_s, (const char*, const char*), ());
    MOCK_METHOD(void, t2_uninit, (), ());
};

// Global instance for linkage
TelemetryBusMock* g_telemetryBusMock = nullptr;
// C wrappers for mocks
extern "C" {
    int t2_init(const char* cname) {
        return g_telemetryBusMock ? g_telemetryBusMock->t2_init(cname) : 0;
    }
    int t2_event_s(const char* name, const char* value) {
        return g_telemetryBusMock ? g_telemetryBusMock->t2_event_s(name, value) : 0;
    }
    void t2_uninit() {
        if (g_telemetryBusMock) g_telemetryBusMock->t2_uninit();
    }
}

// Test fixture
class TelemetryClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        g_telemetryBusMock = new TelemetryBusMock();
    }
    void TearDown() override {
        delete g_telemetryBusMock;
        g_telemetryBusMock = nullptr;
    }
};

// Argument validity tests
TEST_F(TelemetryClientTest, Main_NullArguments) {
    char* argv[] = { (char*)"telemetry_client", nullptr, nullptr };
    EXPECT_CALL(*g_telemetryBusMock, t2_init(_)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_telemetryBusMock, t2_event_s(_, _)).Times(1).WillOnce(Return(-1)); // Should fail
    EXPECT_CALL(*g_telemetryBusMock, t2_uninit()).Times(1);

    // main() expects argv[1] and argv[2] to be non-null; simulate missing args
    int rv = main(1, argv); // argc=1, argv[1]=nullptr
    EXPECT_EQ(rv, 0); // Still returns 0 since main() doesn't check for nullptr
}

TEST_F(TelemetryClientTest, Main_NormalFlow) {
    char* argv[] = { (char*)"telemetry_client", (char*)"EventName", (char*)"EventValue" };
    EXPECT_CALL(*g_telemetryBusMock, t2_init(_)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_telemetryBusMock, t2_event_s(StrEq("EventName"), StrEq("EventValue"))).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_telemetryBusMock, t2_uninit()).Times(1);

    int rv = main(3, argv);
    EXPECT_EQ(rv, 0);
}

TEST_F(TelemetryClientTest, Main_EventFailure) {
    char* argv[] = { (char*)"telemetry_client", (char*)"EventName", (char*)"EventValue" };
    EXPECT_CALL(*g_telemetryBusMock, t2_init(_)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_telemetryBusMock, t2_event_s(_, _)).Times(1).WillOnce(Return(-1)); // Simulate failure
    EXPECT_CALL(*g_telemetryBusMock, t2_uninit()).Times(1);

    int rv = main(3, argv);
    EXPECT_EQ(rv, 0); // main() always returns 0, test that event_s fails
}

TEST_F(TelemetryClientTest, Main_InitFailure) {
    char* argv[] = { (char*)"telemetry_client", (char*)"EventName", (char*)"EventValue" };
    EXPECT_CALL(*g_telemetryBusMock, t2_init(_)).Times(1).WillOnce(Return(-1));
    EXPECT_CALL(*g_telemetryBusMock, t2_event_s(_, _)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_telemetryBusMock, t2_uninit()).Times(1);

    int rv = main(3, argv);
    EXPECT_EQ(rv, 0);
}

TEST_F(TelemetryClientTest, Main_UninitCalled) {
    char* argv[] = { (char*)"telemetry_client", (char*)"EventName", (char*)"EventValue" };
    EXPECT_CALL(*g_telemetryBusMock, t2_init(_)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_telemetryBusMock, t2_event_s(_, _)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*g_telemetryBusMock, t2_uninit()).Times(1);

    int rv = main(3, argv);
    EXPECT_EQ(rv, 0);
}


