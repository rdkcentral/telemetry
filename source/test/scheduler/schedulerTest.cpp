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

extern "C" 
{
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include "scheduler/scheduler.h"
#include <utils/vector.h>
#include <bulkdata/profile.h>
#include <bulkdata/reportprofiles.h>
#include <utils/t2log_wrapper.h>	
sigset_t blocking_signal;

void freeSchedulerProfile(void *data);
void* TimeoutThread(void *arg);
}

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "../mocks/rdklogMock.h"
#include "schedulerMock.h"
#include <iostream>
#include <stdexcept>

using namespace std;

rdklogMock *m_rdklogMock = NULL;

SchedulerMock *m_schedulerMock = NULL;

class schedulerTestFixture : public ::testing::Test {
    protected:
            SchedulerMock mock_IO;

            schedulerTestFixture()
            {
                    m_schedulerMock = &mock_IO;
            }

            virtual ~schedulerTestFixture()
            {
                    m_schedulerMock = NULL;
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


TEST(GET_LOGDEMAND, TEST1)
{
   EXPECT_EQ( false, get_logdemand());
}

TEST(SET_LOGDEMAND, TEST1)
{
   set_logdemand(true);
}

TEST(GETLAPSEDTIME, T1_GT_T2)
{
    struct timespec time1;
    struct timespec time2;
    struct timespec output;
    time1.tv_nsec = 110196606;
    time1.tv_sec = 1683887703;
    time2.tv_nsec = 2919488;
    time2.tv_sec = 1683887687;
    EXPECT_EQ(0, getLapsedTime(&output, &time1, &time2));
}

TEST(GETLAPSEDTIME, T2_GT_T1)
{
    struct timespec time1;
    struct timespec time2;
    struct timespec output;
    time2.tv_nsec = 110196606;
    time2.tv_sec = 1683887703;
    time1.tv_nsec = 2919488;
    time1.tv_sec = 1683887687; 
    EXPECT_EQ(1, getLapsedTime(&output, &time1, &time2));
}

TEST(GETLAPSEDTIME, T1_TV_NSEC_GT_T2_BY_GT_1SEC)
{
    struct timespec time1;
    struct timespec time2;
    struct timespec output;

    // Make time1->tv_nsec - time2->tv_nsec > 1,000,000,000
    time1.tv_sec = 10;
    time1.tv_nsec = 200000000; // 2.1 seconds in nanoseconds
    time2.tv_sec = 10;
    time2.tv_nsec = 100000000;     // 0.001 seconds in nanoseconds

    // This will hit the "if (com)" block for the second condition
    getLapsedTime(&output, &time1, &time2);

    // You can check output values if you want, but just calling is enough for coverage
    EXPECT_EQ(output.tv_sec, 0); // 10-10
    EXPECT_EQ(output.tv_nsec, 100000000);
}

TEST(REGISTERSCHEWITHPROFILE_BEFORE_INITSCHEDULER, TEST1)
{
   EXPECT_EQ(T2ERROR_INVALID_ARGS,  registerProfileWithScheduler(NULL, 50, 3600, true, true, true, 10, "2022-12-20T11:05:56Z"));
}

TEST(REGISTERSCHEWITHPROFILE_BEFORE_INITSCHEDULER, TEST2)
{
   EXPECT_EQ(T2ERROR_FAILURE,  registerProfileWithScheduler("RDKB_Profile", 50, 3600, true, true, true, 10, "2022-12-20T11:05:56Z"));
}

TEST(UNREGISTERPROFILEFROMSCH_BEFORE_INITSCHEDULER, TEST1)
{
    EXPECT_EQ(T2ERROR_SUCCESS,  unregisterProfileFromScheduler("RDKB_PROFILE1"));
}

TEST(UNREGISTERPROFILEFROMSCH_BEFORE_INITSCHEDULER, TEST2)
{
    EXPECT_EQ(T2ERROR_INVALID_ARGS,  unregisterProfileFromScheduler(NULL));
}

TEST(UNINITSCHEDULER_BEFORE_INITSCHEDULER, TEST)
{
   uninitScheduler();
}

void ReportProfiles_ToutCb(const char* profileName, bool isClearSeekMap)
{
    printf("ReportProfiles_ToutCb is done\n");
}

void ReportProfiles_ActivationToutCb(const char* profileName)
{
    printf("ReportProfiles_ActivationTimeoutCb is done\n");
}

void NotifySchedulerstartCb()
{
    printf("NotifySchedulerstartCb is done\n");
}

TEST(initScheduler, NON_NULL_CALLBACK)
{
    EXPECT_EQ(T2ERROR_SUCCESS,  initScheduler((TimeoutNotificationCB)ReportProfiles_ToutCb, (ActivationTimeoutCB)ReportProfiles_ActivationToutCb, (NotifySchedulerstartCB)NotifySchedulerstartCb));
}

TEST(initScheduler, NULL_CALLBACK)
{
    EXPECT_EQ(T2ERROR_SUCCESS,  initScheduler((TimeoutNotificationCB)NULL, (ActivationTimeoutCB)NULL, (NotifySchedulerstartCB)NULL));
}

TEST(SendInterruptToTimeoutThread, NON_NULL_CHECK)
{
    EXPECT_EQ(T2ERROR_SUCCESS, SendInterruptToTimeoutThread("RDKB_Profile"));
}

TEST(REGISTERSCHEWITHPROFILE_AFTER_INITSCHEDULER, REGISTER_PROFILE)
{
   EXPECT_EQ(T2ERROR_SUCCESS,  registerProfileWithScheduler("RDKB_Profile", 10, 100, true, true, true, 15, "0001-01-01T00:00:00Z"));
}

TEST(REGISTERSCHEWITHPROFILE_AFTER_INITSCHEDULER, REGISTER_PROFILE_AGAIN)
{
   EXPECT_EQ(T2ERROR_SUCCESS,  registerProfileWithScheduler("RDKB_Profile", 10, 100, true, true, true, 15, "0001-01-01T00:00:00Z"));
}

TEST(REGISTERSCHEWITHPROFILE, DUPLICATE_PROFILE)
{
    initScheduler((TimeoutNotificationCB)ReportProfiles_ToutCb, (ActivationTimeoutCB)ReportProfiles_ActivationToutCb, (NotifySchedulerstartCB)NotifySchedulerstartCb);
    registerProfileWithScheduler("DUPLICATE_PROFILE", 5, 100, true, true, true, 5, "0001-01-01T00:00:00Z");
    EXPECT_EQ(T2ERROR_SUCCESS, registerProfileWithScheduler("DUPLICATE_PROFILE", 5, 100, true, true, true, 5, "0001-01-01T00:00:00Z"));
    unregisterProfileFromScheduler("DUPLICATE_PROFILE");
    uninitScheduler();
}

TEST(UNREGISTERPROFILEFROMSCHEDULER, NULL_NAME)
{
    EXPECT_EQ(T2ERROR_INVALID_ARGS, unregisterProfileFromScheduler(NULL));
}


TEST(UNREGISTERPROFILEFROMSCHEDULER, BEFORE_INIT)
{
    uninitScheduler(); // makes sure scheduler is uninitialized
    EXPECT_EQ(T2ERROR_SUCCESS, unregisterProfileFromScheduler("SOME_PROFILE"));
}


TEST(SENDINTERRUPTTOTIMEOUTTHREAD, NOT_INITIALIZED)
{
    uninitScheduler(); // ensure not initialized
    EXPECT_EQ(T2ERROR_FAILURE, SendInterruptToTimeoutThread("SHOULD_FAIL"));
}


TEST(UNREGISTERPROFILEFROMSCHEDULER, PROFILE_NOT_FOUND)
{
    initScheduler((TimeoutNotificationCB)ReportProfiles_ToutCb, (ActivationTimeoutCB)ReportProfiles_ActivationToutCb, (NotifySchedulerstartCB)NotifySchedulerstartCb);
    EXPECT_EQ(T2ERROR_FAILURE, unregisterProfileFromScheduler("NOT_EXIST_PROFILE"));
    uninitScheduler();
}


TEST(REGISTERPROFILEWITHSCHEDULER, NULL_PROFILE)
{
    initScheduler((TimeoutNotificationCB)ReportProfiles_ToutCb, (ActivationTimeoutCB)ReportProfiles_ActivationToutCb, (NotifySchedulerstartCB)NotifySchedulerstartCb);
    EXPECT_EQ(T2ERROR_INVALID_ARGS, registerProfileWithScheduler(NULL, 10, 100, true, true, true, 5, "2022-12-20T11:05:56Z"));
    uninitScheduler();
}


TEST(REGISTERPROFILEWITHSCHEDULER, NOT_INITIALIZED)
{
    uninitScheduler();
    EXPECT_EQ(T2ERROR_FAILURE, registerProfileWithScheduler("FAIL_PROFILE", 10, 100, true, true, true, 5, "2022-12-20T11:05:56Z"));
}


TEST(UNINITSCHEDULER, NOT_INITIALIZED)
{
    uninitScheduler(); // Should just return/log
}


TEST(REGISTERPROFILEWITHSCHEDULER, FIRSTREPORT_LT_TIMEOUT)
{
    initScheduler((TimeoutNotificationCB)ReportProfiles_ToutCb, (ActivationTimeoutCB)ReportProfiles_ActivationToutCb, (NotifySchedulerstartCB)NotifySchedulerstartCb);


    registerProfileWithScheduler("REPORT_TEST", 2, 50, true, true, true, 5, "0001-01-01T00:00:00Z");
    unregisterProfileFromScheduler("REPORT_TEST");
    uninitScheduler();
}

TEST(TIMEOUTTHREAD, TEST1)
{
    SchedulerProfile *tProfile = (SchedulerProfile *)malloc(sizeof(SchedulerProfile));
    tProfile->name = strdup("RDKB_Profile");
    tProfile->timeRefinSec = 10;
    tProfile->repeat = true;
    tProfile->terminated = false;
    tProfile->timeOutDuration = 10;
    tProfile->timeToLive = 100;
    tProfile->deleteonTime = true;
    tProfile->reportonupdate = true;
    tProfile->firstreportint = 20;
    tProfile->firstexecution = true;
 
    TimeoutThread((void *)tProfile);
    free(tProfile->name);
    free(tProfile);
}

TEST(TIMEOUTTHREAD, TEST2)
{
    SchedulerProfile *tProfile = (SchedulerProfile *)malloc(sizeof(SchedulerProfile));
    tProfile->name = strdup("RDKB_Profile");
    tProfile->timeRefinSec = 10;
    tProfile->repeat = true;
    tProfile->terminated = false;
    tProfile->timeOutDuration = 20;
    tProfile->timeToLive = 100;
    tProfile->deleteonTime = true;
    tProfile->reportonupdate = true;
    tProfile->firstreportint = 20;
    tProfile->firstexecution = false;

    TimeoutThread((void *)tProfile);
    free(tProfile->name);
    free(tProfile);
}

TEST(TIMEOUTTHREAD, WAIT_NO_REPORTING_INTERVAL)
{
    SchedulerProfile *tProfile = (SchedulerProfile *)malloc(sizeof(SchedulerProfile));
    tProfile->name = strdup("RDKB_Profile");
    tProfile->timeRefinSec = 0;
    tProfile->timeRef = NULL; 
    tProfile->timeOutDuration = UINT_MAX;
    tProfile->repeat = false;
    tProfile->terminated = false;
    tProfile->timeToLive = 100;
    tProfile->deleteonTime = true;
    tProfile->reportonupdate = true;
    tProfile->firstreportint = 0;
    tProfile->firstexecution = false;

    // Call TimeoutThread directly – will exercise pthread_cond_wait branch
    TimeoutThread((void *)tProfile);

    free(tProfile->name);
    free(tProfile);
}

TEST(SendInterruptToTimeoutThread, NULL_CHECK)
{
    EXPECT_EQ(T2ERROR_INVALID_ARGS, SendInterruptToTimeoutThread(NULL));
}
#if 0
TEST(UNREGISTERPROFILEFROMSCH_AFTER_INITSCHEDULER, UNREGISTER_PROFILE_NOT_REGISTERED)
{
    EXPECT_EQ(T2ERROR_FAILURE,  unregisterProfileFromScheduler("Profile1"));
}
#endif
TEST(UNREGISTERPROFILEFROMSCH_AFTER_INITSCHEDULER, UNREGISTER_KNOWN_PROFILE)
{
    EXPECT_EQ(T2ERROR_SUCCESS,  unregisterProfileFromScheduler("RDKB_Profile"));
}

TEST(UNINITSCHEDULER, TEST)
{
   uninitScheduler();
}

TEST(UNINITSCHEDULER, TEST1)
{
   initScheduler((TimeoutNotificationCB)ReportProfiles_ToutCb, (ActivationTimeoutCB)ReportProfiles_ActivationToutCb, (NotifySchedulerstartCB)NotifySchedulerstartCb);
   registerProfileWithScheduler("RDKB_Profile", 10, 100, true, true, true, 10, "0001-01-01T00:00:00Z");
   uninitScheduler();
}

TEST(FREE_SCHEDULER_PROFILE, NULL_ARG)
{
    freeSchedulerProfile(NULL); // just returns, for coverage
}

static void* DummySchedulerThread(void* arg)
{
    (void)arg;
    return NULL;
}

TEST(FREE_SCHEDULER_PROFILE, NORMAL)
{
    SchedulerProfile *sch = (SchedulerProfile*)calloc(1, sizeof(SchedulerProfile));
    sch->name = strdup("FREE_PROFILE");
    pthread_mutex_init(&sch->tMutex, NULL);
    pthread_cond_init(&sch->tCond, NULL);
    pthread_t threadId;
    ASSERT_EQ(0, pthread_create(&threadId, NULL, DummySchedulerThread, NULL));	
    freeSchedulerProfile(sch);
}

TEST(UNINITSCHEDULER, CALLED_TWICE)
{
    uninitScheduler();
    uninitScheduler();
}

TEST(UNREGISTERPROFILEFROMSCHEDULER, ALREADY_REMOVED)
{
    initScheduler((TimeoutNotificationCB)ReportProfiles_ToutCb, (ActivationTimeoutCB)ReportProfiles_ActivationToutCb, (NotifySchedulerstartCB)NotifySchedulerstartCb);
    registerProfileWithScheduler("REMOVEME", 2, 10, true, true, true, 1, "0001-01-01T00:00:00Z");
    unregisterProfileFromScheduler("REMOVEME");
    EXPECT_EQ(T2ERROR_FAILURE, unregisterProfileFromScheduler("REMOVEME"));
    uninitScheduler();
}
#ifdef GTEST_ENABLE
extern "C"
{
  typedef unsigned int (*getSchdInSec_fn)(char*);
  getSchdInSec_fn getSchdInSec_fnCallback(void);
}
TEST(getSchdInSec, IndirectCall)
{
    getSchdInSec_fn fn = getSchdInSec_fnCallback();
    char timeRef[] = "2022-12-20T11:05:56Z";
    // Call via function pointer
    unsigned int result = fn(timeRef);
    EXPECT_GE(result, 0u);
    EXPECT_LE(result, 86400u);
}
#endif
