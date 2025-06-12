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

TEST(REGISTERSCHEWITHPROFILE_AFTER_INITSCHEDULER, TEST3)
{
   EXPECT_EQ(T2ERROR_SUCCESS,  registerProfileWithScheduler("RDKB_Profile", 50, 3600, true, true, true, 10, "2022-12-20T11:05:56Z"));
}

TEST(SendInterruptToTimeoutThread, NULL_CHECK)
{
    EXPECT_EQ(T2ERROR_INVALID_ARGS, SendInterruptToTimeoutThread(NULL));
}

TEST(SendInterruptToTimeoutThread, NON_NULL_CHECK)
{
    EXPECT_EQ(T2ERROR_SUCCESS, SendInterruptToTimeoutThread("RDKB_Profile"));
}

TEST(UNREGISTERPROFILEFROMSCH_AFTER_INITSCHEDULER, TEST1)
{
    EXPECT_EQ(T2ERROR_SUCCESS,  unregisterProfileFromScheduler("RDKB_Profile"));
}

TEST(UNINITSCHEDULER, TEST)
{
   uninitScheduler();
}
