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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include "telemetry_busmessage_sender.h"
//#include "../mocks/rbusMock.cpp"

// Test fixture for telemetry_busmessage_sender
class TelemetryBusmessageSenderTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Always start with a clean state
        t2_uninit();
    }
    void TearDown() override {
        t2_uninit();
    }
};

// Positive test: Init and Uninit
TEST_F(TelemetryBusmessageSenderTest, InitAndUninit) {
    t2_init((char*)"test_component");
    // No direct state verification, just ensure no crash
    t2_uninit();
}

// Positive test: Send string event with valid input
TEST_F(TelemetryBusmessageSenderTest, SendStringEvent_Valid) {
    t2_init((char*)"test_component");
    T2ERROR err = t2_event_s("test_marker", "test_value");
    EXPECT_EQ(err, T2ERROR_SUCCESS);
    t2_uninit();
}

// Positive test: Send double event with valid input
TEST_F(TelemetryBusmessageSenderTest, SendDoubleEvent_Valid) {
    t2_init((char*)"test_component");
    T2ERROR err = t2_event_f("test_marker", 1.23);
    EXPECT_EQ(err, T2ERROR_SUCCESS);
    t2_uninit();
}

// Positive test: Send int event with valid input
TEST_F(TelemetryBusmessageSenderTest, SendIntEvent_Valid) {
    t2_init((char*)"test_component");
    T2ERROR err = t2_event_d("test_marker", 42);
    EXPECT_EQ(err, T2ERROR_SUCCESS);
    t2_uninit();
}

// Negative test: t2_event_s with NULL component (should return T2ERROR_COMPONENT_NULL)
TEST_F(TelemetryBusmessageSenderTest, SendStringEvent_NullComponent) {
    t2_uninit();
    T2ERROR err = t2_event_s("marker", "value");
    EXPECT_EQ(err, T2ERROR_COMPONENT_NULL);
}

// Negative test: t2_event_s with NULL marker
TEST_F(TelemetryBusmessageSenderTest, SendStringEvent_NullMarker) {
    t2_init((char*)"test_component");
    T2ERROR err = t2_event_s(NULL, "value");
    EXPECT_EQ(err, T2ERROR_FAILURE);
}

// Negative test: t2_event_s with NULL value
TEST_F(TelemetryBusmessageSenderTest, SendStringEvent_NullValue) {
    t2_init((char*)"test_component");
    T2ERROR err = t2_event_s("marker", NULL);
    EXPECT_EQ(err, T2ERROR_FAILURE);
}

// Negative test: t2_event_s with empty string value (should not send, returns success)
TEST_F(TelemetryBusmessageSenderTest, SendStringEvent_EmptyValue) {
    t2_init((char*)"test_component");
    T2ERROR err = t2_event_s("marker", "");
    EXPECT_EQ(err, T2ERROR_SUCCESS);
}

// Negative test: t2_event_s with value "0" (should not send, returns success)
TEST_F(TelemetryBusmessageSenderTest, SendStringEvent_ZeroValue) {
    t2_init((char*)"test_component");
    T2ERROR err = t2_event_s("marker", "0");
    EXPECT_EQ(err, T2ERROR_SUCCESS);
}

// Negative test: t2_event_f with NULL marker
TEST_F(TelemetryBusmessageSenderTest, SendDoubleEvent_NullMarker) {
    t2_init((char*)"test_component");
    T2ERROR err = t2_event_f(NULL, 3.14);
    EXPECT_EQ(err, T2ERROR_FAILURE);
}

// Negative test: t2_event_f with NULL component
TEST_F(TelemetryBusmessageSenderTest, SendDoubleEvent_NullComponent) {
    t2_uninit();
    T2ERROR err = t2_event_f("marker", 3.14);
    EXPECT_EQ(err, T2ERROR_COMPONENT_NULL);
}

// Negative test: t2_event_d with NULL marker
TEST_F(TelemetryBusmessageSenderTest, SendIntEvent_NullMarker) {
    t2_init((char*)"test_component");
    T2ERROR err = t2_event_d(NULL, 123);
    EXPECT_EQ(err, T2ERROR_FAILURE);
}

// Negative test: t2_event_d with value 0 (should not send, returns success)
TEST_F(TelemetryBusmessageSenderTest, SendIntEvent_Zero) {
    //extern uint32_t t2ReadyStatus;
    //t2ReadyStatus = T2_STATE_COMPONENT_READY;
    t2_init((char*)"test_component");
    T2ERROR err = t2_event_d("marker", 0);
    EXPECT_EQ(err, T2ERROR_SUCCESS);
}

// Negative test: t2_event_d with NULL component
TEST_F(TelemetryBusmessageSenderTest, SendIntEvent_NullComponent) {
    t2_uninit();
    T2ERROR err = t2_event_d("marker", 13);
    EXPECT_EQ(err, T2ERROR_COMPONENT_NULL);
}

// 1. Bus handle null
TEST_F(TelemetryBusmessageSenderTest, ReturnsErrorIfBusHandleNull) {
    int ret;
    //int ret = initMessageBus();
    //EXPECT_NE(ret, 0);
    ret = filtered_event_send("data", "marker");
    EXPECT_NE(ret, 0); // Should return error (RBUS_ERROR_SUCCESS is 0)
}

/*
// 2. Rbus mode: event filtering - markerName not in eventMarkerMap
TEST_F(TelemetryBusmessageSenderTest, RbusMode_MarkerNotInEventMap_NotSent) {
    eventMarkerMap = reinterpret_cast<void*>(0xdeadbeef); // fake non-null
    // hash_map_get returns nullptr by default mock
    int ret = filtered_event_send("data", "markerNotPresent");
    EXPECT_EQ(ret, 0); // Not sent, returns 0
}

// 3. Rbus mode: event filtering - markerName in eventMarkerMap
TEST_F(TelemetryBusmessageSenderTest, RbusMode_MarkerInEventMap_SentSuccess) {
    static bool markerFound = false;
    eventMarkerMap = reinterpret_cast<void*>(0xbeef);
    // Patch hash_map_get to simulate found marker
    struct {
        static void* get(void*, const char*) { markerFound = true; return (void*)1; }
    } patch;
    auto orig = hash_map_get;
    hash_map_get = patch.get;
    int ret = filtered_event_send("data", "marker");
    EXPECT_EQ(ret, 0);
    EXPECT_TRUE(markerFound);
    hash_map_get = orig;
}

// 4. Rbus mode: event filtering - componentName is script client (no filter)
TEST_F(TelemetryBusmessageSenderTest, RbusMode_ScriptComponent_SentNoFilter) {
    setComponentName("telemetry_client");
    int ret = filtered_event_send("data", "marker");
    EXPECT_EQ(ret, 0);
}

// 5. Rbus mode: rbus_set returns error
TEST_F(TelemetryBusmessageSenderTest, RbusMode_RbusSetFails) {
    setComponentName("not_script");
    static bool called = false;
    eventMarkerMap = reinterpret_cast<void*>(0xbeef);
    struct {
        static void* get(void*, const char*) { return (void*)1; }
        static rbusError_t set(void*, const char*, void*, void*) { called = true; return (rbusError_t)99; }
    } patch;
    auto orig_get = hash_map_get;
    auto orig_set = rbus_set;
    hash_map_get = patch.get;
    rbus_set = patch.set;
    int ret = filtered_event_send("data", "marker");
    EXPECT_NE(ret, 0);
    EXPECT_TRUE(called);
    hash_map_get = orig_get;
    rbus_set = orig_set;
}

// 6. CCSP mode: buffer allocation fails
TEST_F(TelemetryBusmessageSenderTest, CCSPMode_BufferAllocationFails) {
    // Patch malloc to always fail
    struct {
        static void* malloc(size_t) { return nullptr; }
    } patch;
    // Not possible to patch malloc directly in portable C++, so skip actual override.
    // But this test is a placeholder for when using a malloc shim in real test infra.
    // int ret = filtered_event_send("data", "marker");
    // EXPECT_EQ(ret, -1);
    SUCCEED() << "Can't patch malloc in portable C++, test is a placeholder.";
}

// 7. CCSP mode: SendTelemetryDataSignal fails
TEST_F(TelemetryBusmessageSenderTest, CCSPMode_SendFails) {
    struct {
        static int send(void*, const char*) { return 1; }
    } patch;
    auto orig = CcspBaseIf_SendTelemetryDataSignal;
    CcspBaseIf_SendTelemetryDataSignal = patch.send;
    int ret = filtered_event_send("data", "marker");
    EXPECT_EQ(ret, -1);
    CcspBaseIf_SendTelemetryDataSignal = orig;
}

// 8. CCSP mode: SendTelemetryDataSignal succeeds
TEST_F(TelemetryBusmessageSenderTest, CCSPMode_SendSucceeds) {
    struct {
        static int send(void*, const char*) { return 0; }
    } patch;
    auto orig = CcspBaseIf_SendTelemetryDataSignal;
    CcspBaseIf_SendTelemetryDataSignal = patch.send;
    int ret = filtered_event_send("data", "marker");
    EXPECT_EQ(ret, 0);
    CcspBaseIf_SendTelemetryDataSignal = orig;
}

// 9. MarkerName is nullptr
TEST_F(TelemetryBusmessageSenderTest, NullMarkerName) {
    int ret = filtered_event_send("data", nullptr);
    // Should handle gracefully for both rbus and ccsp
    EXPECT_EQ(ret, 0);
}

// 10. Data is nullptr
TEST_F(TelemetryBusmessageSenderTest, NullData) {
    int ret = filtered_event_send(nullptr, "marker");
    // Should not crash, behavior as per implementation
    EXPECT_EQ(ret, 0);
}

// 11. Both arguments null
TEST_F(TelemetryBusmessageSenderTest, NullArgs) {
    int ret = filtered_event_send(nullptr, nullptr);
    EXPECT_EQ(ret, 0);
}

*/
