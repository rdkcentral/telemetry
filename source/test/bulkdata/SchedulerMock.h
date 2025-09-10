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

#ifndef SOURCE_TEST_BULKDATA_SCHEDULERMOCK_H_
#define SOURCE_TEST_BULKDATA_SCHEDULERMOCK_H_

#include <gtest/gtest.h>
#include <gmock/gmock.h>

extern "C" {
#include "scheduler.h"
#include "telemetry2_0.h"
}

class SchedulerMock
{
public:
    MOCK_METHOD(T2ERROR, initScheduler, (TimeoutNotificationCB notificationCb, ActivationTimeoutCB activationCB, NotifySchedulerstartCB notifyschedulerCB), ());
    MOCK_METHOD(void, uninitScheduler, (), ());
    MOCK_METHOD(T2ERROR, registerProfileWithScheduler, (const char* profileName, unsigned int timeInterval, unsigned int activationTimeout, bool deleteonTimout, bool repeat, bool reportOnUpdate, unsigned int firstReportingInterval, char *timeRef), ());
    MOCK_METHOD(T2ERROR, unregisterProfileFromScheduler, (const char* profileName), ());
    MOCK_METHOD(T2ERROR, SendInterruptToTimeoutThread, (char* profileName), ());
    MOCK_METHOD(bool, get_logdemand, (), ());
    MOCK_METHOD(void, set_logdemand, (bool value), ());
    MOCK_METHOD(int, getLapsedTime, (struct timespec *result, struct timespec *x, struct timespec *y), ());
};

extern SchedulerMock* g_schedulerMock;

#endif // SOURCE_TEST_BULKDATA_SCHEDULERMOCK_H_
