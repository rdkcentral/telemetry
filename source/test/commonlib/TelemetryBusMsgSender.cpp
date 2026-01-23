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
extern "C" {
#include "telemetry_busmessage_sender.h"

T2ERROR getParamValue(const char* paramName, char **paramValue);
}
#include "../mocks/rbusMock.h"
#include "../mocks/SystemMock.h"
#include "../mocks/FileioMock.h"

using namespace std;

using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::Invoke;                                                                                                          
using ::testing::SetArgPointee;                                                                                                   
using ::testing::DoAll;     


// Test fixture for telemetry_busmessage_sender
class TelemetryBusmessageSenderTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Always start with a clean state
        t2_uninit();
	 g_rbusMock = new rbusMock();
     g_systemMock = new SystemMock();
     g_fileIOMock = new FileMock();

	 EXPECT_CALL(*g_rbusMock, rbus_close(_))
            .WillRepeatedly(Return(RBUS_ERROR_SUCCESS));
    }
    void TearDown() override {
        t2_uninit();
	delete  g_rbusMock;
    delete  g_systemMock;
    delete  g_fileIOMock;

    g_rbusMock = nullptr;
	g_systemMock = nullptr;
    g_fileIOMock = nullptr;
    }
};

#ifdef GTEST_ENABLE
extern "C" {
    typedef T2ERROR (*doPopulateEventMarkerListFunc)(void);
    doPopulateEventMarkerListFunc getDoPopulateEventMarkerListCallback(void);
    bool* test_get_isRbusEnabled_ptr(void);
    bool* test_get_isT2Ready_ptr(void);
    void** test_get_bus_handle_ptr(void);
}
#endif

// Positive test: Init and Uninit
TEST_F(TelemetryBusmessageSenderTest, InitAndUninit) {
    t2_init((char*)"test_component");
    // No direct state verification, just ensure no crash
    t2_uninit();
}

// Negative test: t2_event_s with NULL component (should return T2ERROR_COMPONENT_NULL)
TEST_F(TelemetryBusmessageSenderTest, SendStringEvent_NullComponent) {
    t2_uninit();
    EXPECT_CALL(*g_systemMock, access(_,_))
            .Times(2)
            .WillOnce(Return(-1))
            .WillOnce(Return(-1));
    EXPECT_CALL(*g_fileIOMock, getpid())
            .Times(1)
            .WillOnce(Return(1234));//Dummy pid
    T2ERROR err = t2_event_s("marker", "value");
    EXPECT_EQ(err, T2ERROR_COMPONENT_NULL);
}
// Negative test: t2_event_s with NULL marker

/*TEST_F(TelemetryBusmessageSenderTest, SendStringEvent_NullMarker) {
    t2_init((char*)"test_component");
    EXPECT_CALL(*g_systemMock, access(_,_))
            .Times(2)
            .WillOnce(Return(-1))
            .WillOnce(Return(-1));
    T2ERROR err = t2_event_s(NULL, "value");
    EXPECT_EQ(err, T2ERROR_FAILURE);
}

// Negative test: t2_event_s with NULL value
TEST_F(TelemetryBusmessageSenderTest, SendStringEvent_NullValue) {
    t2_init((char*)"test_component");
    EXPECT_CALL(*g_systemMock, access(_,_))
            .Times(2)
            .WillOnce(Return(-1))
            .WillOnce(Return(-1));
    T2ERROR err = t2_event_s("marker", NULL);
    EXPECT_EQ(err, T2ERROR_FAILURE);
}
*/

// Negative test: t2_event_s with empty string value (should not send, returns success)
TEST_F(TelemetryBusmessageSenderTest, SendStringEvent_EmptyValue) {
    t2_init((char*)"test_component");
    EXPECT_CALL(*g_systemMock, access(_,_))
           .WillRepeatedly(Return(-1));
    T2ERROR err = t2_event_s("marker", "");
    EXPECT_EQ(err, T2ERROR_SUCCESS);
}

// Negative test: t2_event_s with value "0" (should not send, returns success)
TEST_F(TelemetryBusmessageSenderTest, SendStringEvent_ZeroValue) {
    t2_init((char*)"test_component");
    EXPECT_CALL(*g_systemMock, access(_,_))
           .WillRepeatedly(Return(-1));
    T2ERROR err = t2_event_s("marker", "0");
    EXPECT_EQ(err, T2ERROR_SUCCESS);
}

// Negative test: t2_event_f with NULL marker
/*
TEST_F(TelemetryBusmessageSenderTest, SendDoubleEvent_NullMarker) {
    t2_init((char*)"test_component");
    EXPECT_CALL(*g_systemMock, access(_,_))
            .Times(2)
            .WillOnce(Return(-1))
            .WillOnce(Return(-1));
    T2ERROR err = t2_event_f(NULL, 3.14);
    EXPECT_EQ(err, T2ERROR_FAILURE);
}
*/
// Negative test: t2_event_f with NULL component
TEST_F(TelemetryBusmessageSenderTest, SendDoubleEvent_NullComponent) {
    t2_uninit();
    EXPECT_CALL(*g_systemMock, access(_,_))
            .Times(2)
            .WillOnce(Return(-1))
            .WillOnce(Return(-1));
    EXPECT_CALL(*g_fileIOMock, getpid())
            .Times(1)
            .WillOnce(Return(1234));
    T2ERROR err = t2_event_f("marker", 3.14);
    EXPECT_EQ(err, T2ERROR_COMPONENT_NULL);
}

// Negative test: t2_event_d with NULL marker

/*TEST_F(TelemetryBusmessageSenderTest, SendIntEvent_NullMarker) {
    t2_init((char*)"test_component");
    EXPECT_CALL(*g_systemMock, access(_, _))
        .WillRepeatedly(Return(-1)); // Allow unlimited calls to access()
    T2ERROR err = t2_event_d(NULL, 123);
    EXPECT_EQ(err, T2ERROR_FAILURE);
}*/

// Negative test: t2_event_d with value 0 (should not send, returns success)
TEST_F(TelemetryBusmessageSenderTest, SendIntEvent_Zero) {
    t2_init((char*)"test_component");
    EXPECT_CALL(*g_systemMock, access(_,_))
           .WillRepeatedly(Return(-1));
    T2ERROR err = t2_event_d("marker", 0);
    EXPECT_EQ(err, T2ERROR_SUCCESS);
}

// Negative test: t2_event_d with NULL component
TEST_F(TelemetryBusmessageSenderTest, SendIntEvent_NullComponent) {
    t2_uninit();
    EXPECT_CALL(*g_systemMock, access(_,_))
            .Times(2)
            .WillOnce(Return(-1))
            .WillOnce(Return(-1));
    EXPECT_CALL(*g_fileIOMock, getpid())
            .Times(1)
            .WillOnce(Return(1234));
    T2ERROR err = t2_event_d("marker", 13);
    EXPECT_EQ(err, T2ERROR_COMPONENT_NULL);
}

/*TEST_F(TelemetryBusmessageSenderTest, SendIntEvent_Component) {
    t2_uninit();
    EXPECT_CALL(*g_systemMock, access(_,_))
           .WillRepeatedly(Return(-1));
    T2ERROR err = t2_event_d("marker", 13);
    EXPECT_EQ(err, T2ERROR_SUCCESS);
}*/

TEST_F(TelemetryBusmessageSenderTest, getParamValue_failure)
{
    char* paramValue = NULL;
    t2_init((char*)"test_component");

    EXPECT_EQ(T2ERROR_FAILURE, getParamValue("Device.DeviceInfo.Dummy", &paramValue));
}

TEST_F(TelemetryBusmessageSenderTest, t2_event_s_iscachingenabled_false)
{
    t2_init((char*)"sysint");
    EXPECT_CALL(*g_systemMock, access(_,_))
            .Times(4)
            .WillOnce(Return(-1))
            .WillOnce(Return(-1))
            .WillOnce(Return(-1))
            .WillOnce(Return(-1));
   
    EXPECT_CALL(*g_rbusMock, rbus_checkStatus())
            .Times(1)
            .WillOnce(Return(RBUS_ENABLED));
    EXPECT_CALL(*g_rbusMock, rbus_open(_, _))
            .Times(1)
            .WillOnce([](rbusHandle_t* handle, const char* componentName) {
                *handle = (rbusHandle_t)0xdeadbeef;
                return RBUS_ERROR_SUCCESS;
            });

    EXPECT_CALL(*g_rbusMock, rbus_getUint(_, _, _))
            .Times(1)
            .WillOnce([](rbusHandle_t handle, const char* name, uint32_t* value) {
                *value = 0;
                return RBUS_ERROR_BUS_ERROR;
            });
    
    int ret;
    ret = t2_event_s((char*)"Test_marker:", (char*)"test_data");
    EXPECT_EQ(ret, T2ERROR_SUCCESS);
}


TEST_F(TelemetryBusmessageSenderTest, t2_event_d_iscachingenabled_false)
{
    t2_init((char*)"sysint");
    EXPECT_CALL(*g_systemMock, access(_,_))
        .WillRepeatedly(Return(-1));
    EXPECT_CALL(*g_rbusMock, rbus_getUint(_, _, _))
            .Times(1)
            .WillOnce([](rbusHandle_t handle, const char* name, uint32_t* value) {
                *value = 0;
                return RBUS_ERROR_BUS_ERROR;
            });

    int ret;
    ret = t2_event_d("marker", 13);
    EXPECT_EQ(ret, T2ERROR_SUCCESS);
}

TEST_F(TelemetryBusmessageSenderTest, t2_event_f_iscachingenabled_false)
{
    t2_init((char*)"sysint");
    EXPECT_CALL(*g_systemMock, access(_,_))
        .WillRepeatedly(Return(-1));
    EXPECT_CALL(*g_rbusMock, rbus_getUint(_, _, _))
            .Times(1)
            .WillOnce([](rbusHandle_t handle, const char* name, uint32_t* value) {
                *value = 0;
                return RBUS_ERROR_BUS_ERROR;
            });
    int ret;
    ret = t2_event_f("marker", 123.456);
    EXPECT_EQ(ret, T2ERROR_SUCCESS);
}

TEST_F(TelemetryBusmessageSenderTest, t2_event_d_iscachingenabled_true)
{
    t2_init((char*)"sysint");

    EXPECT_CALL(*g_systemMock, access(_,_))
        .WillRepeatedly(Return(-1)); // Accept any number of calls

    EXPECT_CALL(*g_rbusMock, rbus_getUint(_, _, _))
        .Times(1)
        .WillOnce([](rbusHandle_t handle, const char* name, uint32_t* value) {
            *value = 0;
            return RBUS_ERROR_SUCCESS; // <-- Simulate SUCCESS
        });

    int ret = t2_event_d("marker", 13);
    EXPECT_EQ(ret, T2ERROR_SUCCESS);
}

TEST_F(TelemetryBusmessageSenderTest, t2_event_d_iscachingenabled_true_1)
{
    t2_init((char*)"sysinit");

    EXPECT_CALL(*g_systemMock, access(_,_))
        .WillRepeatedly(Return(-1)); // Accept any number of calls
    EXPECT_CALL(*g_rbusMock, rbus_getUint(_, _, _))
        .Times(1)
        .WillOnce([](rbusHandle_t handle, const char* name, uint32_t* value) {
            *value = 1;
            return RBUS_ERROR_SUCCESS; // <-- Simulate SUCCESS
        });
#if 1
    *test_get_isRbusEnabled_ptr() = false;
    *test_get_isT2Ready_ptr() = true;
#endif 
    int ret = t2_event_d("marker", 13);

    printf("################## ret = %d \n",ret);
    printf("###### T2ERROR_SUCESS = %d\n",T2ERROR_SUCCESS);
    *test_get_isRbusEnabled_ptr() = true;
    EXPECT_EQ(ret, T2ERROR_SUCCESS);
}

TEST_F(TelemetryBusmessageSenderTest, getParameterValue_success)
{
    char* paramValue = NULL;
    t2_init((char*)"test_component");
    
    EXPECT_CALL(*g_rbusMock, rbus_get(_, _, _))
        .Times(1)
        .WillOnce(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(*g_rbusMock, rbusValue_GetType(_))
        .Times(1)
        .WillOnce(Return(RBUS_STRING));
    EXPECT_CALL(*g_rbusMock, rbusValue_ToString(_,_,_))
        .Times(1)
        .WillOnce(Return((char*)"test_value"));
    EXPECT_CALL(*g_rbusMock, rbusValue_Release(_))
        .Times(1);
  
    EXPECT_EQ(T2ERROR_SUCCESS, getParamValue("Device.DeviceInfo.SerialNumber", &paramValue));
}

TEST_F(TelemetryBusmessageSenderTest, getParameterValue_success_boolean)
{
    char* paramValue = NULL;
    t2_init((char*)"test_component");
    
    EXPECT_CALL(*g_rbusMock, rbus_get(_, _, _))
        .Times(1)
        .WillOnce(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(*g_rbusMock, rbusValue_GetType(_))
        .Times(1)
        .WillOnce(Return(RBUS_BOOLEAN));
    EXPECT_CALL(*g_rbusMock, rbusValue_GetBoolean(_))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*g_rbusMock, rbusValue_Release(_))
        .Times(1);
  
    EXPECT_EQ(T2ERROR_SUCCESS, getParamValue("Device.DeviceInfo.SerialNumber", &paramValue));
}

TEST_F(TelemetryBusmessageSenderTest, getParameterValue_failure_boolean)
{
    char* paramValue = NULL;
    t2_init((char*)"test_component");

    EXPECT_CALL(*g_rbusMock, rbus_get(_, _, _))
        .Times(1)
        .WillOnce(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(*g_rbusMock, rbusValue_GetType(_))
        .Times(1)
        .WillOnce(Return(RBUS_BOOLEAN));
    EXPECT_CALL(*g_rbusMock, rbusValue_GetBoolean(_))
        .Times(1)
        .WillOnce(Return(false));
    EXPECT_CALL(*g_rbusMock, rbusValue_Release(_))
        .Times(1);

    EXPECT_EQ(T2ERROR_SUCCESS, getParamValue("Device.DeviceInfo.SerialNumber", &paramValue));
}


/*TEST_F(TelemetryBusmessageSenderTest, filtered_event_send_1)
{
    t2_init((char*)"telemetry_client");

    EXPECT_CALL(*g_rbusMock,  rbusValue_Init(_))
            .Times(2)
            .WillOnce(Return((rbusValue_t)0xffffffff))
            .WillOnce(Return((rbusValue_t)0xffffffff));
    EXPECT_CALL(*g_rbusMock,  rbusValue_SetString(_, _))
            .Times(1);
    EXPECT_CALL(*g_rbusMock,  rbusProperty_Init(_,_,_))
            .Times(1);
    EXPECT_CALL(*g_rbusMock,  rbusValue_SetProperty (_, _))
            .Times(1);
    EXPECT_CALL(*g_rbusMock,  rbusValue_Release(_))
            .Times(2);
    EXPECT_CALL(*g_rbusMock,  rbus_set(_,_,_,_))
            .Times(1)
            .WillOnce(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(*g_rbusMock,  rbusProperty_Release(_))
            .Times(1);
    int ret;
    ret = filtered_event_send((char*)"test_data", (char*)"Test_marker:");
    EXPECT_EQ(ret, 0);

}

TEST_F(TelemetryBusmessageSenderTest, filtered_event_send_2)
{
    t2_init((char*)"sysint");
    int ret;
    ret = filtered_event_send((char*)"test_data", (char*)"Test_marker:");
    EXPECT_EQ(ret, 0);
}*/

// Positive test: Send string event with valid input

TEST_F(TelemetryBusmessageSenderTest, SendStringEvent_Valid) {
    t2_init((char*)"test_component");
    EXPECT_CALL(*g_systemMock, access(_,_))
            .WillRepeatedly(Return(-1));
    
    EXPECT_CALL(*g_rbusMock, rbus_getUint(_, _, _))
            .Times(1)
            .WillOnce([](rbusHandle_t handle, const char* name, uint32_t* value) {
                *value = 1;
                return RBUS_ERROR_BUS_ERROR;
            });
        
    T2ERROR err = t2_event_s("test_marker", "test_value");
    EXPECT_EQ(err, T2ERROR_SUCCESS);
    t2_uninit();
}

TEST_F(TelemetryBusmessageSenderTest, filtered_event_send_null_bus_handle) {
    t2_init((char*)"test_component");
    // Force bus_handle to NULL
    *test_get_bus_handle_ptr() = nullptr; // Use your GTEST helper
    int ret = filtered_event_send("somedata", "somemarker");
    EXPECT_EQ(ret, 0);  // ret is initialized to 0
    t2_uninit();
}
#if 0
// Cover: isRbusEnabled, marker NOT in eventMarkerMap (event filtering disables send)
TEST_F(TelemetryBusmessageSenderTest, filtered_event_send_marker_not_in_map) {
    t2_init((char*)"not_telemetry_client");
    // Setup for bus_handle, rbus enabled
    static int fake_handle = 42;
    *test_get_bus_handle_ptr() = (void*)&fake_handle;
    *test_get_isRbusEnabled_ptr() = true;

    // eventMarkerMap exists but does NOT contain marker
    extern hash_map_t *eventMarkerMap;
    extern "C" hash_map_t* hash_map_create();
    extern "C" void hash_map_destroy(hash_map_t*, void(*)(void*));
    if(eventMarkerMap) hash_map_destroy(eventMarkerMap, free);
    eventMarkerMap = hash_map_create();

    int ret = filtered_event_send("somedata", "not_present_marker");
    EXPECT_EQ(ret, 0);
    t2_uninit();
}

// Cover: isRbusEnabled, script event (name is telemetry_client; bypass filtering, rbus_set OK)
TEST_F(TelemetryBusmessageSenderTest, filtered_event_send_script_event_success) {
    t2_init((char*)T2_SCRIPT_EVENT_COMPONENT);
    static int fake_handle = 43;
    *test_get_bus_handle_ptr() = (void*)&fake_handle;
    *test_get_isRbusEnabled_ptr() = true;

    // Mocks for rbus
    EXPECT_CALL(*g_rbusMock, rbusValue_Init(_))
        .Times(2);
    EXPECT_CALL(*g_rbusMock, rbusValue_SetString(_, _)).Times(1);
    EXPECT_CALL(*g_rbusMock, rbusProperty_Init(_, _, _)).Times(1);
    EXPECT_CALL(*g_rbusMock, rbusValue_SetProperty(_, _)).Times(1);
    EXPECT_CALL(*g_rbusMock, rbus_set(_,_,_,_)).WillOnce(Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(*g_rbusMock, rbusValue_Release(_)).Times(2);
    EXPECT_CALL(*g_rbusMock, rbusProperty_Release(_)).Times(1);

    int ret = filtered_event_send("somedata", "somescriptmarker");
    EXPECT_EQ(ret, 0);
    t2_uninit();
}

// Cover: isRbusEnabled, rbus_set returns error branch
TEST_F(TelemetryBusmessageSenderTest, filtered_event_send_script_event_rbus_set_fail) {
    t2_init((char*)T2_SCRIPT_EVENT_COMPONENT);
    static int fake_handle = 44;
    *test_get_bus_handle_ptr() = (void*)&fake_handle;
    *test_get_isRbusEnabled_ptr() = true;

    EXPECT_CALL(*g_rbusMock, rbusValue_Init(_))
        .Times(2);
    EXPECT_CALL(*g_rbusMock, rbusValue_SetString(_, _)).Times(1);
    EXPECT_CALL(*g_rbusMock, rbusProperty_Init(_, _, _)).Times(1);
    EXPECT_CALL(*g_rbusMock, rbusValue_SetProperty(_, _)).Times(1);
    EXPECT_CALL(*g_rbusMock, rbus_set(_,_,_,_)).WillOnce(Return(RBUS_ERROR_BUS_ERROR));
    EXPECT_CALL(*g_rbusMock, rbusValue_Release(_)).Times(2);
    EXPECT_CALL(*g_rbusMock, rbusProperty_Release(_)).Times(1);

    int ret = filtered_event_send("somedata", "failmarker");
    EXPECT_EQ(ret, -1);
    t2_uninit();
}
#endif
// ==================== Additional Tests for Coverage (Copilot Suggestion) ====================

// Positive path for t2_event_f (double, non-zero, valid)
TEST_F(TelemetryBusmessageSenderTest, SendDoubleEvent_Positive)
{
    t2_init((char*)"component_f");
    EXPECT_CALL(*g_systemMock, access(_,_)).WillRepeatedly(Return(-1));
    EXPECT_CALL(*g_rbusMock, rbus_getUint(_, _, _)).WillOnce([](rbusHandle_t, const char*, uint32_t* out) {
        *out = 1; return RBUS_ERROR_SUCCESS;
    });
    T2ERROR err = t2_event_f("float_marker", 7.89);
    EXPECT_EQ(err, T2ERROR_SUCCESS);
    t2_uninit();
}

// Positive path for t2_event_d (int, non-zero, valid)
TEST_F(TelemetryBusmessageSenderTest, SendIntEvent_Positive)
{
    t2_init((char*)"component_d");
    EXPECT_CALL(*g_systemMock, access(_,_)).WillRepeatedly(Return(-1));
    EXPECT_CALL(*g_rbusMock, rbus_getUint(_, _, _)).WillOnce([](rbusHandle_t, const char*, uint32_t* out) {
        *out = 1; return RBUS_ERROR_SUCCESS;
    });
    T2ERROR err = t2_event_d("int_marker", 55);
    EXPECT_EQ(err, T2ERROR_SUCCESS);
    t2_uninit();
}
#if 0
// Test: t2_event_f malloc failure coverage (simulate out-of-memory)
TEST_F(TelemetryBusmessageSenderTest, SendDoubleEvent_MallocFail)
{
    t2_init((char*)"component_f_mallocfail");
    EXPECT_CALL(*g_systemMock, access(_,_)).WillRepeatedly(Return(-1));
    // Inject malloc failure (if malloc can be mocked; otherwise, document for manual/integration fail testing)
    // Example - skip malloc and rely on code branch for error coverage
    // This branch is only reached if malloc fails, so coverage tool will reflect invocation if forcibly triggered
    // Not implementable with Google Mock directly - source might need to use wrapper you can mock
    // For illustration only:
    // EXPECT_CALL(*g_customMock, my_malloc(_)).WillOnce(Return(nullptr));
    // T2ERROR err = t2_event_f("marker", 1.23);
    // EXPECT_EQ(err, T2ERROR_FAILURE);
    t2_uninit();
}
// Test: t2_event_d malloc failure coverage
TEST_F(TelemetryBusmessageSenderTest, SendIntEvent_MallocFail)
{
    t2_init((char*)"component_i_mallocfail");
    EXPECT_CALL(*g_systemMock, access(_,_)).WillRepeatedly(Return(-1));
    // See note above for malloc mocking.
    // T2ERROR err = t2_event_d("marker", 42);
    // EXPECT_EQ(err, T2ERROR_FAILURE);
    t2_uninit();
}

// t2_uninit() idempotency (called multiple times for double-free protection)
TEST_F(TelemetryBusmessageSenderTest, UninitTwice_Idempotency)
{
    t2_init((char*)"component_uninit");
    t2_uninit();
    // Second call should not crash, even if everything was already cleaned up
    t2_uninit();
}

// Test: Extreme case, calling init/uninit in sequence (mutex logic and memory edge)
TEST_F(TelemetryBusmessageSenderTest, MultipleInitUninitSequence)
{
    for (int i = 0; i < 3; ++i) {
        t2_init((char*)"component_seq");
        t2_uninit();
    }
}

#endif
// (Optional) If you refactor or add a wrapper for malloc for testability,
// then uncomment the malloc-failure tests and implement the needed mocks.

#if 0
TEST_F(TelemetryBusmessageSenderTest, getParameterValue_failure_rbus_get)
{
    t2_init((char*)"test_component");	
    char* paramValue = NULL;
    t2_init((char*)"test_component");
    *test_get_isRbusEnabled_ptr() = true; 
    EXPECT_CALL(*g_rbusMock, rbus_get(_, _, _))
        .Times(1)
        .WillOnce(Return(RBUS_ERROR_BUS_ERROR));
    *test_get_isRbusEnabled_ptr() = true;
    EXPECT_EQ(T2ERROR_FAILURE, getParamValue("Device.DeviceInfo.SerialNumber", &paramValue));
    t2_uninit();
}
#endif
#if 0
#ifdef GTEST_ENABLE
extern "C" {
    typedef T2ERROR (*doPopulateEventMarkerListFunc)(void);
    doPopulateEventMarkerListFunc getDoPopulateEventMarkerListCallback(void);
    bool* test_get_isRbusEnabled_ptr(void);
    void** test_get_bus_handle_ptr(void);
}
#endif
#if 0
TEST_F(TelemetryBusmessageSenderTest, doPopulateEventMarkerList_RbusDisabled)
{
    *test_get_isRbusEnabled_ptr() = false;
    *test_get_bus_handle_ptr() = (void*)0xdeadbeef;
    auto cb = getDoPopulateEventMarkerListCallback();
    EXPECT_EQ(cb(), T2ERROR_SUCCESS);
}

TEST_F(TelemetryBusmessageSenderTest, doPopulateEventMarkerList_RbusGetFails)
{
    *test_get_isRbusEnabled_ptr() = true;
    *test_get_bus_handle_ptr() = (void*)0xcafef00d;

    EXPECT_CALL(*g_rbusMock, rbus_get(_, _, _))
        .Times(1)
        .WillOnce(Return(RBUS_ERROR_BUS_ERROR));

    auto cb = getDoPopulateEventMarkerListCallback();
    EXPECT_EQ(cb(), T2ERROR_SUCCESS);
}

TEST_F(TelemetryBusmessageSenderTest, doPopulateEventMarkerList_ValueTypeNotObject)
{
    *test_get_isRbusEnabled_ptr() = true;
    *test_get_bus_handle_ptr() = (void*)0xcafef00d;
    rbusValue_t dummyValue = (rbusValue_t)0xf00BAAA;

    EXPECT_CALL(*g_rbusMock, rbus_get(_, _, _))
        .Times(1)
        .WillOnce(DoAll(SetArgPointee<2>(dummyValue), Return(RBUS_ERROR_SUCCESS)));

    EXPECT_CALL(*g_rbusMock, rbusValue_GetType(dummyValue))
        .Times(1)
        .WillOnce(Return(RBUS_STRING));

    EXPECT_CALL(*g_rbusMock, rbusValue_Release(dummyValue)).Times(1);

    auto cb = getDoPopulateEventMarkerListCallback();
    EXPECT_EQ(cb(), T2ERROR_FAILURE);
}

TEST_F(TelemetryBusmessageSenderTest, doPopulateEventMarkerList_ObjectValueNull)
{
    *test_get_isRbusEnabled_ptr() = true;
    *test_get_bus_handle_ptr() = (void*)0xcafef00d;
    rbusValue_t dummyValue = (rbusValue_t)0xbabe123;

    EXPECT_CALL(*g_rbusMock, rbus_get(_, _, _))
        .Times(1)
        .WillOnce(DoAll(SetArgPointee<2>(dummyValue), Return(RBUS_ERROR_SUCCESS)));

    EXPECT_CALL(*g_rbusMock, rbusValue_GetType(dummyValue))
        .Times(1)
        .WillOnce(Return(RBUS_OBJECT));

    EXPECT_CALL(*g_rbusMock, rbusValue_GetObject(dummyValue))
        .Times(1)
        .WillOnce(Return(nullptr));

    EXPECT_CALL(*g_rbusMock, rbusValue_Release(dummyValue)).Times(1);

    auto cb = getDoPopulateEventMarkerListCallback();
    EXPECT_EQ(cb(), T2ERROR_SUCCESS);
}

TEST_F(TelemetryBusmessageSenderTest, doPopulateEventMarkerList_ObjectWithOneEvent)
{
    *test_get_isRbusEnabled_ptr() = true;
    *test_get_bus_handle_ptr() = (void*)0x7777;
    rbusValue_t dummyValue = (rbusValue_t)0xaaaa;
    rbusObject_t dummyObj = (rbusObject_t)0xbbbb;
    rbusProperty_t dummyProp = (rbusProperty_t)0xcccc;
    const char* dummyEventName = "TelemetryEvent.Test";

    EXPECT_CALL(*g_rbusMock, rbus_get(_, _, _))
        .Times(1)
        .WillOnce(DoAll(SetArgPointee<2>(dummyValue), Return(RBUS_ERROR_SUCCESS)));
    EXPECT_CALL(*g_rbusMock, rbusValue_GetType(dummyValue))
        .Times(1)
        .WillOnce(Return(RBUS_OBJECT));
    EXPECT_CALL(*g_rbusMock, rbusValue_GetObject(dummyValue))
        .Times(1)
        .WillOnce(Return(dummyObj));
    EXPECT_CALL(*g_rbusMock, rbusObject_GetProperties(dummyObj))
        .Times(1)
        .WillOnce(Return(dummyProp));
    EXPECT_CALL(*g_rbusMock, rbusProperty_GetName(dummyProp))
        .Times(1)
        .WillOnce(Return(dummyEventName));
    EXPECT_CALL(*g_rbusMock, rbusProperty_GetNext(dummyProp))
        .Times(1)
        .WillOnce(Return(nullptr));
    EXPECT_CALL(*g_rbusMock, rbusValue_Release(dummyValue)).Times(1);

    auto cb = getDoPopulateEventMarkerListCallback();
    EXPECT_EQ(cb(), T2ERROR_SUCCESS);
}

#endif
extern "C" {
typedef void (*rbusEventReceiveHandlerFunc)(rbusHandle_t, rbusEvent_t const*, rbusEventSubscription_t*);
rbusEventReceiveHandlerFunc getRbusEventReceiveHandlerCallback(void);
// Also ensure that struct definitions for rbusEvent_t etc. are visible!
}
TEST_F(TelemetryBusmessageSenderTest, rbusEventReceiveHandler_static_coverage) {
    rbusEventReceiveHandlerFunc handler = getRbusEventReceiveHandlerCallback();
    ASSERT_NE(handler, nullptr);

    rbusEventSubscription_t sub = {0};
    rbusEvent_t event = {0};

    // Case 1: correct notify name
    event.name = "test";
    handler(h, &event, &sub);

}
#endif
