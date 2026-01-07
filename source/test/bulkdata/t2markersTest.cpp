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
#include <thread>                                                                                         
#include <chrono> 

extern "C" {
#include "t2markers.h"
#include "t2collection.h"
#include "t2eventreceiver.h"
#include "t2log_wrapper.h"
#include "rbusInterface.h"
#include "t2eventreceiver.h"
#include "telemetry2_0.h"
#include "profile.h"
#include "busInterface.h"
#include "dca.h"
}
#include "../mocks/rbusMock.h"
#include "../mocks/SystemMock.h"
#include "../mocks/FileioMock.h"
#include "../mocks/rdklogMock.h"
#include "t2markersMock.h"


using namespace std;

using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::Invoke;                                                                                                          
using ::testing::SetArgPointee;                                                                                                   
using ::testing::DoAll;     

rbusMock* g_rbusMock = nullptr;
SystemMock* g_systemMock = nullptr;
FileMock* g_fileIOMock = nullptr;
rdklogMock* m_rdklogMock = nullptr;
t2markersMock* g_t2markersMock = nullptr;

// Test fixture for telemetry_busmessage_sender
class t2markersTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
    g_t2markersMock = new t2markersMock();
	g_rbusMock = new rbusMock();
     g_systemMock = new SystemMock();
     g_fileIOMock = new FileMock();



    }
    void TearDown() override {
       
	delete  g_rbusMock;
    delete  g_systemMock;
    delete  g_fileIOMock;
    delete  g_t2markersMock;

    g_rbusMock = nullptr;
	g_systemMock = nullptr;
    g_fileIOMock = nullptr;
    g_t2markersMock = nullptr;
    }
};

//Test the initialization of t2markers
TEST_F(t2markersTestFixture, test_t2markers_init) 
{
    EXPECT_EQ(T2ERROR_SUCCESS, initT2MarkerComponentMap());
}

TEST_F(t2markersTestFixture, clearT2MarkerComponentMap_test) 
{
    EXPECT_EQ(T2ERROR_SUCCESS, clearT2MarkerComponentMap());
}

TEST_F(t2markersTestFixture, test_t2markersdestroyComponentMap)
{
    EXPECT_EQ(T2ERROR_SUCCESS, destroyT2MarkerComponentMap());
} 

TEST_F(t2markersTestFixture, UpdateEventMap_sucess)
{
    T2Marker *t2Marker = (T2Marker *)malloc(sizeof(T2Marker));
    memset(t2Marker, '\0', sizeof(T2Marker));
    t2Marker->markerName = strdup("sys_test_marker");
    t2Marker->componentName = strdup("sysint");
    Vector_Create(&t2Marker->profileList);
    Vector_PushBack(t2Marker->profileList, (void *)strdup("testProfile"));
    EXPECT_EQ(T2ERROR_SUCCESS, updateEventMap("sys_test_marker", t2Marker));
}

TEST_F(t2markersTestFixture, UpdateEventMap_sucess_1)
{
    T2Marker *t2Marker = (T2Marker *)malloc(sizeof(T2Marker));
    memset(t2Marker, '\0', sizeof(T2Marker));
    t2Marker->markerName = strdup("dac_test_marker");
    t2Marker->componentName = strdup("dca");
    Vector_Create(&t2Marker->profileList);
    Vector_PushBack(t2Marker->profileList, (void *)strdup("testProfile1"));
    EXPECT_EQ(T2ERROR_SUCCESS, updateEventMap("dca_test_marker", t2Marker));
}

TEST_F(t2markersTestFixture, addt2EventMarker_match_found) 
{
    EXPECT_EQ(T2ERROR_SUCCESS, addT2EventMarker("sys_test_marker", "sysint", "testProfile", 0));
}

TEST_F(t2markersTestFixture, addt2EventMarker_no_match) 
{
    EXPECT_EQ(T2ERROR_SUCCESS, addT2EventMarker("sys_test_marker", "sysint", "testProfile_new", 0));
}

TEST_F(t2markersTestFixture, addt2EventMarker_new_marker) 
{
    EXPECT_EQ(T2ERROR_SUCCESS, addT2EventMarker("New_marker1", "sysint", "RDK_Profile1", 0));
}

TEST_F(t2markersTestFixture, addt2EventMarker_new_marker_1) 
{
    EXPECT_EQ(T2ERROR_SUCCESS, addT2EventMarker("New_marker2", "sysint", "RDK_Profile2", 0));
}

TEST_F(t2markersTestFixture, addt2EventMarker_new_marker_2) 
{
    EXPECT_EQ(T2ERROR_SUCCESS, addT2EventMarker("New_marker3", "wifi", "RDK_Profile2", 0));
}

TEST_F(t2markersTestFixture,  getComponentsWithEventMarkers_success) 
{
    Vector* eventComponentList = NULL;
    Vector_Create(&eventComponentList);
    getComponentsWithEventMarkers(&eventComponentList);
    EXPECT_NE(eventComponentList, nullptr);
    Vector_Destroy(eventComponentList, free);
}

TEST_F(t2markersTestFixture,  getComponentMarkerList_success) 
{
    Vector* markerList = NULL;
    Vector_Create(&markerList);
    getComponentMarkerList("sysint", (void**)&markerList);
    Vector_Destroy(markerList, free);
}

TEST_F(t2markersTestFixture,  getMarkerProfileList_success) 
{
    Vector* profileList = NULL;
    Vector_Create(&profileList); 
    getMarkerProfileList("sys_test_marker", &profileList);
    Vector_Destroy(profileList, free);
}

/*TEST_F(t2markersTestFixture,  createcompDE_success) 
{
    addT2EventMarker("New_marker3", "ccsp", "RDK_Profile3", 0);
    createComponentDataElements();
    destroyT2MarkerComponentMap();
}*/


//t2eventreceiver.c

//Uninit is ignored when event receiver is not initialized
TEST_F(t2markersTestFixture, T2ER_Uninit_ignored_when_not_initialized)
{
   T2ER_Uninit();
}

//StopDispatchThread is ignored when event receiver is not initialized
TEST_F(t2markersTestFixture, T2ER_StopDispatchThread_ignored_when_not_initialized)
{
   EXPECT_EQ(T2ERROR_FAILURE, T2ER_StopDispatchThread());
}

//StartDispatchThread is ignored when event receiver is not initialized
TEST_F(t2markersTestFixture, T2ER_StartDispatchThread_ignored_when_not_initialized)
{
   EXPECT_EQ(T2ERROR_FAILURE, T2ER_StartDispatchThread());
}

//PushEvent is ignored when event receiver is not initialized
TEST_F(t2markersTestFixture, T2ER_PushEvent_ignored_when_not_initialized)
{
    char *eventname = strdup("test_event");
    char *eventValue = strdup("test_value");
    T2ER_Push(eventname, eventValue);
    free(eventname);
    free(eventValue);
}

//PushDataWithDelim is ignored when event receiver is not initialized
TEST_F(t2markersTestFixture, T2ER_Pushwithdelimiter_ignored_when_not_initialized)
{
    char *eventname = strdup("test_event");
    char *eventValue = strdup("test_value");
    T2ER_PushDataWithDelim(eventname, eventValue);
    free(eventname);
    free(eventValue);
}

//T2ER_Init success when rbus is disabled
TEST_F(t2markersTestFixture, T2_ER_init_success_rbusdisabled) 
{
    EXPECT_CALL(*g_t2markersMock, isRbusEnabled())
        .Times(1)
        .WillOnce(Return(false));
    EXPECT_CALL(*g_t2markersMock, registerForTelemetryEvents(_))
        .Times(1)
        .WillOnce(Return(T2ERROR_SUCCESS));
    EXPECT_EQ(T2ERROR_SUCCESS, T2ER_Init());
}

//T2ER_Uninit success after init
TEST_F(t2markersTestFixture, T2ER_Uninit)
{
    T2ER_Uninit();
}

//T2ER_Init success when rbus is enabled
TEST_F(t2markersTestFixture, T2_ER_init_success_rbusenabled) 
{
    EXPECT_CALL(*g_t2markersMock, isRbusEnabled())
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*g_t2markersMock, registerForTelemetryEvents(_))
        .Times(1)
        .WillOnce(Return(T2ERROR_SUCCESS));
    EXPECT_EQ(T2ERROR_SUCCESS, T2ER_Init());
}

//start dispatch thread success after init
TEST_F(t2markersTestFixture, T2ER_StartDispatchThread_success_after_init)
{
   EXPECT_EQ(T2ERROR_SUCCESS, T2ER_StartDispatchThread());
}

TEST_F(t2markersTestFixture, T2ER_PushEvent_success_after_init)
{
    char *eventname = strdup("test_event");
    char *eventValue = strdup("test_value");
    T2ER_Push(eventname, eventValue);
}

TEST_F(t2markersTestFixture, T2ER_Push_success_after_init_1)
{
    char *eventname = strdup("New_marker2");
    char *eventValue = strdup("value2");

    EXPECT_CALL(*g_t2markersMock, ReportProfiles_storeMarkerEvent(_,_))
        .Times(1)
        .WillOnce(Return(T2ERROR_SUCCESS));
    addT2EventMarker("New_marker2", "sysint", "RDK_Profile2", 0);
    T2ER_Push(eventname, eventValue);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

}

TEST_F(t2markersTestFixture, T2ER_Pushwithdelimiter_success_after_init)
{
    char *eventname = strdup("test_event");
    char *eventValue = strdup("test_value");
    T2ER_PushDataWithDelim(eventname, eventValue);
}

//Stop dispatch thread success after init
TEST_F(t2markersTestFixture, T2ER_StopDispatchThread_success_after_init)
{
  /* EXPECT_CALL(*g_fileIOMock, fopen(_, _))
        .Times(1)
        .WillOnce(Return((FILE*)0XFFFFFFFF));
   EXPECT_CALL(*g_fileIOMock, fgets(_, _, _))
        .Times(2)
        .WillOnce([]( char* buf, int size, FILE* stream) {
            strncpy(buf, "New_marker3", size);
            return buf;
        })
        .WillOnce(Return((char*)NULL));
   EXPECT_CALL(*g_fileIOMock, fclose(_))
        .Times(1)
        .WillOnce(Return(0));
   EXPECT_CALL(*g_systemMock, remove(_))
        .Times(1)
        .WillOnce(Return(0));*/
   EXPECT_EQ(T2ERROR_SUCCESS, T2ER_StopDispatchThread());
}

//T2ER_Uninit success after stop dispatch thread
TEST_F(t2markersTestFixture, T2ER_Uninit_after_stop_dispatch_thread)
{
    T2ER_Uninit();
}

#if 1
extern "C" {
typedef void (*freeT2ComponentListFunc) (void *data);
freeT2ComponentListFunc freeT2ComponentListFuncCallback(void);

typedef void (*updateComponentListFunc) (const char* componentName);
updateComponentListFunc updateComponentListFuncCallback(void);


typedef void (*freeT2MarkerFunc) (void *data);
freeT2MarkerFunc freeT2MarkerFuncCallback(void);
}
TEST(t2markersTest, freeT2ComponentNull) {
    freeT2ComponentListFunc freeFunc = freeT2ComponentListFuncCallback();
    ASSERT_NE(freeFunc, nullptr);
    freeFunc(nullptr);
}

TEST(t2markerTest, updateComponentListNull)
{
	updateComponentListFunc updateFunc = updateComponentListFuncCallback();
	ASSERT_NE(updateFunc, nullptr);
        updateFunc(nullptr);
}

TEST(t2markerTest , freeT2ComponentListNull)
{
     freeT2MarkerFunc  freeT2Func = freeT2MarkerFuncCallback();
     ASSERT_NE(freeT2Func, nullptr);
}
#endif
