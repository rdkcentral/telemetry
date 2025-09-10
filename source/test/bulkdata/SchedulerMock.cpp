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

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "test/bulkdata/SchedulerMock.h"

SchedulerMock* g_schedulerMock = nullptr;

extern "C" {

// Mock implementations of scheduler functions
T2ERROR initScheduler(TimeoutNotificationCB notificationCb, ActivationTimeoutCB activationCB, NotifySchedulerstartCB notifyschedulerCB)
{
    if (g_schedulerMock)
    {
        return g_schedulerMock->initScheduler(notificationCb, activationCB, notifyschedulerCB);
    }
    
    // Fallback implementation for when mock is not set
    return T2ERROR_SUCCESS;
}

void uninitScheduler()
{
    if (g_schedulerMock)
    {
        g_schedulerMock->uninitScheduler();
        return;
    }
    
    // Fallback implementation - do nothing
}

T2ERROR registerProfileWithScheduler(const char* profileName, unsigned int timeInterval, unsigned int activationTimeout, bool deleteonTimout, bool repeat, bool reportOnUpdate, unsigned int firstReportingInterval, char *timeRef)
{
    if (g_schedulerMock)
    {
        return g_schedulerMock->registerProfileWithScheduler(profileName, timeInterval, activationTimeout, deleteonTimout, repeat, reportOnUpdate, firstReportingInterval, timeRef);
    }
    
    // Fallback implementation for when mock is not set
    if (!profileName) return T2ERROR_INVALID_ARGS;
    return T2ERROR_SUCCESS;
}

T2ERROR unregisterProfileFromScheduler(const char* profileName)
{
    if (g_schedulerMock)
    {
        return g_schedulerMock->unregisterProfileFromScheduler(profileName);
    }
    
    // Fallback implementation for when mock is not set
    if (!profileName) return T2ERROR_INVALID_ARGS;
    return T2ERROR_SUCCESS;
}

T2ERROR SendInterruptToTimeoutThread(char* profileName)
{
    if (g_schedulerMock)
    {
        return g_schedulerMock->SendInterruptToTimeoutThread(profileName);
    }
    
    // Fallback implementation for when mock is not set
    if (!profileName) return T2ERROR_INVALID_ARGS;
    return T2ERROR_SUCCESS;
}

bool get_logdemand()
{
    if (g_schedulerMock)
    {
        return g_schedulerMock->get_logdemand();
    }
    
    // Fallback implementation for when mock is not set
    return false;
}

void set_logdemand(bool value)
{
    if (g_schedulerMock)
    {
        g_schedulerMock->set_logdemand(value);
        return;
    }
    
    // Fallback implementation - do nothing
}

int getLapsedTime(struct timespec *result, struct timespec *x, struct timespec *y)
{
    if (g_schedulerMock)
    {
        return g_schedulerMock->getLapsedTime(result, x, y);
    }
    
    // Fallback implementation for when mock is not set
    if (!result || !x || !y) return -1;
    
    // Simple fallback implementation
    result->tv_sec = y->tv_sec - x->tv_sec;
    result->tv_nsec = y->tv_nsec - x->tv_nsec;
    
    if (result->tv_nsec < 0) {
        result->tv_sec--;
        result->tv_nsec += 1000000000;
    }
    
    return 0;
}

} // extern "C"
