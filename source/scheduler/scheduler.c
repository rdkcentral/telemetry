/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2019 RDK Management
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

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include "t2log_wrapper.h"
#include "scheduler.h"
#include "vector.h"
#include "profile.h"
#include "rdk_otlp_instrumentation.h"
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif
#define DEFAULT_TIME_REFERENCE "0001-01-01T00:00:00Z"
char *strptime(const char *s, const char *format, struct tm *tm);

static NotifySchedulerstartCB notifySchedulerstartcb;
static TimeoutNotificationCB timeoutNotificationCb;
static ActivationTimeoutCB activationTimeoutCb;
static Vector *profileList = NULL;
static pthread_mutex_t scMutex;
static bool sc_initialized = false;
static bool islogdemand = false;

static bool signalrecived_and_executing = true;
static bool is_activation_time_out = false;

bool get_logdemand ()
{
    T2Info(("get_logdemand ++in\n"));
    return islogdemand;
}

void set_logdemand (bool value)
{
    T2Info(("set_logdemand ++in\n"));
    islogdemand = value;
}

void freeSchedulerProfile(void *data)
{
    if(data != NULL)
    {
        SchedulerProfile *schProfile = (SchedulerProfile *)data;
        T2Info(" schProfile->name = %s schProfile->tId = %d\n", schProfile->name, (int)schProfile->tId);
        pthread_mutex_destroy(&schProfile->tMutex);
        pthread_cond_destroy(&schProfile->tCond);
        pthread_detach(schProfile->tId);
        free(schProfile->name);
        schProfile->name = NULL;
        free(schProfile);
    }
}

/*
 * Function:  getLapsedTime
 * --------------------
 * calculates the elapsed time between time2 and time1(in seconds and nano seconds)
 *
 * time2: start time
 * time1: finish time
 *
 * returns: output (calculated elapsed seconds and nano seconds)
 */
int getLapsedTime (struct timespec *output, struct timespec *time1, struct timespec *time2)
{
    rdk_otlp_start_child_span("scheduler", "get");
    /* handle the underflow condition, if time2 nsec has higher value */
    int com = time1->tv_nsec < time2->tv_nsec;
    if (com)
    {
        int nsec = (time2->tv_nsec - time1->tv_nsec) / 1000000000 + 1;

        time2->tv_nsec = time2->tv_nsec - 1000000000 * nsec;
        time2->tv_sec = time2->tv_sec + nsec;
    }

    com = time1->tv_nsec - time2->tv_nsec > 1000000000;
    if (com)
    {
        int nsec = (time1->tv_nsec - time2->tv_nsec) / 1000000000;

        time2->tv_nsec = time2->tv_nsec + 1000000000 * nsec;
        time2->tv_sec  = time2->tv_sec - nsec;
    }

    /* Calculate the elapsed time */
    output->tv_sec = time1->tv_sec - time2->tv_sec;

    output->tv_nsec = time1->tv_nsec - time2->tv_nsec;
    rdk_otlp_finish_child_span();
    if(time1->tv_sec < time2->tv_sec)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

// This function is used to calculate the difference between the currentUTC time and UTC time configured in the TimeReference.It takes hr:min:sec for the time calculation. When the timeRef value is missed the report will be generated at the same time ref after 24 hrs.
static unsigned int getSchdInSec(char* timeRef)
{
    struct tm timeptr, currtimeptr;
    time_t timeref = 0, timenow = 0, curtime;
    char * currtime = NULL;
    memset(&timeptr, '\0', sizeof(timeptr));
    memset(&currtimeptr, '\0', sizeof(currtimeptr));
    strptime(timeRef, "%Y-%m-%dT%H:%M:%SZ", &timeptr);
    timeref = (timeptr.tm_hour * 3600) + (timeptr.tm_min * 60) + timeptr.tm_sec;
    T2Debug("TimeReference given = %ld\n", (long) timeref);
    time(&curtime);
    currtime = ctime(&curtime);
    T2Debug("Current time = %s\n", currtime);
    strptime(currtime, "%a %b %d %H:%M:%S %Y", &currtimeptr);
    timenow = (currtimeptr.tm_hour * 3600) + (currtimeptr.tm_min * 60) + currtimeptr.tm_sec;
    T2Debug("timestamp_now = %ld\n", (long) timenow);
    if(timeref > timenow)
    {
        return (timeref - timenow);
    }
    else
    {
        return (timenow - timeref);
    }
}

void* TimeoutThread(void *arg)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    SchedulerProfile *tProfile = (SchedulerProfile*)arg;
    struct timespec _ts;
    struct timespec _now;
    struct timespec _MinThresholdTimeTs;
    struct timespec _MinThresholdTimeStart;
    unsigned int minThresholdTime = 0;
    int n;
    T2Debug("%s ++in\n", __FUNCTION__);
    registerTriggerConditionConsumer();
    T2Debug("TimeoutThread id is %d\n", (int)tProfile->tId);

    // 1. Initialize condition variable attributes
    pthread_condattr_t Profile_attr;
    if (pthread_condattr_init(&Profile_attr) != 0)
    {
        T2Error("pthread_condattr_init failed");
        return NULL;
    }

    //Set the clock source for the condition variable as CLOCK_MONOTONIC
    // This is important to prevent timer from drifting because of systemtime drift ( such as NTP sync)
    if (pthread_condattr_setclock(&Profile_attr, CLOCK_MONOTONIC) != 0)
    {
        T2Error("pthread_condattr_setclock failed \n");
        if (pthread_condattr_destroy(&Profile_attr) != 0)
        {
            T2Error("pthread_condattr_destroy failed \n");
        }
        return NULL;
    }

    //Initialize the condition variable with the attributes
    if (pthread_cond_init(&tProfile->tCond, &Profile_attr) != 0)
    {
        T2Error("pthread_cond_init failed\n");
        if (pthread_condattr_destroy(&Profile_attr) != 0)
        {
            T2Error("pthread_condattr_destroy failed \n");
        }
        return NULL;
    }

    if (pthread_condattr_destroy(&Profile_attr) != 0)
    {
        T2Error("pthread_condattr_destroy failed \n");
    }

    while(tProfile->repeat && !tProfile->terminated && tProfile->name)
    {
        memset(&_ts, 0, sizeof(struct timespec));
        memset(&_now, 0, sizeof(struct timespec));

        if(pthread_mutex_lock(&tProfile->tMutex) != 0)
        {
            T2Error("tProfile Mutex lock failed\n");
            return NULL;
        }

        if( clock_gettime(CLOCK_MONOTONIC, &_now) == -1 )
        {
            T2Error("clock_gettime failed\n");
        }
        else
        {
            //update the timevalues for profiles
            _ts.tv_sec = _now.tv_sec;
        }

        if(tProfile->timeRef && strcmp(tProfile->timeRef, DEFAULT_TIME_REFERENCE) != 0)
        {
            tProfile->timeRefinSec = getSchdInSec(tProfile->timeRef);
            T2Debug("TimeRefinSec is %d\n", tProfile->timeRefinSec);
        }

        if(tProfile->firstexecution == true)
        {
            T2Info("First Reporting Interval %d sec is given for profile %s\n", tProfile->firstreportint, tProfile->name);
            _ts.tv_sec += tProfile->firstreportint;
        }
        else
        {
            if(tProfile->timeRefinSec != 0)  // this loop is used to choose the minimum waiting value based on the comparison b/w TimeRef and Reporting Interval
            {
                if(tProfile->timeOutDuration <= tProfile->timeRefinSec)
                {
                    _ts.tv_sec += tProfile->timeOutDuration;
                    T2Info("Waiting for %d sec for next TIMEOUT for profile as reporting interval is taken - %s\n", tProfile->timeOutDuration, tProfile->name);
                }
                else if(tProfile->timeOutDuration > tProfile->timeRefinSec)
                {
                    _ts.tv_sec += tProfile->timeRefinSec;
                    T2Info("Waiting for %d sec for next TIMEOUT for profile as Time Reference is taken - %s\n", tProfile->timeRefinSec, tProfile->name);
                }
            }
            else
            {
                _ts.tv_sec += tProfile->timeOutDuration;
                T2Info("Waiting for %d sec for next TIMEOUT for profile as reporting interval is taken - %s\n", tProfile->timeOutDuration, tProfile->name);
            }
        }
        notifySchedulerstartcb(tProfile->name, true);
        //When first reporting interval is given waiting for first report int vale
        if(tProfile->firstreportint > 0 && tProfile->firstexecution == true )
        {
            T2Info("Waiting for %d sec for next TIMEOUT for profile as firstreporting interval is given - %s\n", tProfile->firstreportint, tProfile->name);
            n = pthread_cond_timedwait(&tProfile->tCond, &tProfile->tMutex, &_ts);
        }
        else
        {
            if(tProfile->timeOutDuration == UINT_MAX && tProfile->timeRefinSec == 0)
            {
                T2Info("Waiting for condition as reporting interval is not configured for profile - %s\n", tProfile->name);
                n = pthread_cond_wait(&tProfile->tCond, &tProfile->tMutex);
            }
            else
            {
                T2Info("Waiting for timeref or reporting interval for the profile - %s is started\n", tProfile->name);
                n = pthread_cond_timedwait(&tProfile->tCond, &tProfile->tMutex, &_ts);
            }
        }
        if(n == ETIMEDOUT)
        {
            T2Info("TIMEOUT for profile - %s\n", tProfile->name);
            timeoutNotificationCb(tProfile->name, false);
        }
        else if (n == 0)
        {
            /* CID 175316:- Value not atomically updated (ATOMICITY) */
            T2Info("Interrupted before TIMEOUT for profile : %s \n", tProfile->name);
            signalrecived_and_executing = false;
            if(minThresholdTime)
            {
                memset(&_MinThresholdTimeTs, 0, sizeof(struct timespec));
                clock_gettime(CLOCK_REALTIME, &_MinThresholdTimeTs);
                T2Debug("minThresholdTime left %ld -\n", (long int)(_MinThresholdTimeTs.tv_sec - _MinThresholdTimeStart.tv_sec));
                if(minThresholdTime < (unsigned int) (_MinThresholdTimeTs.tv_sec - _MinThresholdTimeStart.tv_sec))
                {
                    minThresholdTime = 0;
                    T2Debug("minThresholdTime reset done\n");
                }
            }

            if(minThresholdTime == 0)
            {
                if (get_logdemand() == true)
                {
                    set_logdemand(false);
                    timeoutNotificationCb(tProfile->name, false);
                }
                else
                {
                    timeoutNotificationCb(tProfile->name, true);
                }
                if(tProfile->terminated)
                {
                    T2Warning("Profile : %s is being removed from scheduler \n", tProfile->name);
                    if(pthread_mutex_unlock(&tProfile->tMutex) != 0)
                    {
                        T2Error("tProfile Mutex unlock failed\n");
                        return NULL;
                    }
                    break;
                }
                minThresholdTime = getMinThresholdDuration(tProfile->name);
                T2Info("minThresholdTime %u\n", minThresholdTime);

                if(minThresholdTime)
                {
                    memset(&_MinThresholdTimeStart, 0, sizeof(struct timespec));
                    clock_gettime(CLOCK_REALTIME, &_MinThresholdTimeStart);
                }
            }
        }
        else
        {
            T2Error("Profile : %s pthread_cond_timedwait ERROR!!!\n", tProfile->name);
        }
        //Update activation timeout
        if (tProfile->timeToLive != INFINITE_TIMEOUT && tProfile->firstexecution == true)
        {
            tProfile->timeToLive -= tProfile->firstreportint;
        }
        else if (tProfile->timeToLive != INFINITE_TIMEOUT)
        {
            tProfile->timeToLive -= tProfile->timeOutDuration;
        }
        tProfile->firstexecution = false;
        /*
         * If timeToLive < timeOutDuration,
         * invoke activationTimeout callback and
         * exit timeout thread.
         */
        if (tProfile->timeOutDuration == 0 || tProfile->timeToLive < tProfile->timeOutDuration)
        {
            T2Info("Profile activation timeout for %s \n", tProfile->name);
            is_activation_time_out = true;
            char *profileName = strdup(tProfile->name);
            tProfile->repeat = false;
            if(pthread_mutex_unlock(&tProfile->tMutex) != 0)
            {
                T2Error("tProfile Mutex unlock failed\n");
                free(profileName);
                return NULL;
            }
            activationTimeoutCb(profileName);
            is_activation_time_out = false;
            if(profileName != NULL)
            {
                free(profileName);
            }
            break;
        }
        if(pthread_mutex_unlock(&tProfile->tMutex) != 0)
        {
            T2Error("tProfile Mutex unlock failed\n");
            return NULL;
        }
    }
    T2Debug("%s --out\n", __FUNCTION__);
    return NULL;
}

T2ERROR SendInterruptToTimeoutThread(char* profileName)
{
    SchedulerProfile *tProfile = NULL;
    T2Debug("%s ++in\n", __FUNCTION__);
    if(profileName == NULL)
    {
        T2Error("profileName is NULL.. Invalid Args\n");
        return T2ERROR_INVALID_ARGS;
    }
    if(timeoutNotificationCb == NULL)
    {
        T2Error("Timerout Callback not set - Scheduler is not initialized yet\n");
        return T2ERROR_FAILURE;
    }
    size_t index = 0;
    if(pthread_mutex_lock(&scMutex) != 0)
    {
        T2Error("scMutex locking failed\n");
        return T2ERROR_FAILURE;
    }
    for (; index < profileList->count; ++index)
    {
        tProfile = (SchedulerProfile *)Vector_At(profileList, index);
        if(profileName == NULL || (strcmp(profileName, tProfile->name) == 0))
        {
            T2Info("Sending Interrupt signal to Timeout Thread of profile : %s\n", tProfile->name);
            int mutex_return = pthread_mutex_trylock(&tProfile->tMutex);
            if(mutex_return != 0)
            {
                T2Error("tProfile Mutex locking failed : %d \n", mutex_return);
                if(mutex_return == EBUSY)
                {
                    T2Error("tProfile Mutex is Busy, already the report generation might be in progress \n");
                }
                pthread_mutex_unlock(&scMutex);
                return T2ERROR_FAILURE;
            }
            pthread_cond_signal(&tProfile->tCond);
            if(pthread_mutex_unlock(&tProfile->tMutex) != 0)
            {
                T2Error("tProfile Mutex unlocking failed\n");
                pthread_mutex_unlock(&scMutex);
                return T2ERROR_FAILURE;
            }
        }
    }
    if(pthread_mutex_unlock(&scMutex) != 0)
    {
        T2Error("scMutex unlocking failed\n");
        return T2ERROR_FAILURE;
    }
    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

T2ERROR initScheduler(TimeoutNotificationCB notificationCb, ActivationTimeoutCB activationCB, NotifySchedulerstartCB notifyschedulerCB)
{
    T2Debug("%s ++in\n", __FUNCTION__);

    if (sc_initialized)
    {
        T2Info("Scheduler is already initialized \n");
        return T2ERROR_SUCCESS;
    }
    timeoutNotificationCb = notificationCb;
    activationTimeoutCb = activationCB;
    notifySchedulerstartcb = notifyschedulerCB;

    sc_initialized = true;
    if(pthread_mutex_init(&scMutex, NULL) != 0)
    {
        T2Error("%s Mutex init has failed\n", __FUNCTION__);
        return T2ERROR_FAILURE;
    }

    T2Debug("%s --out\n", __FUNCTION__);
    return Vector_Create(&profileList);
}

void uninitScheduler()
{
    size_t index = 0;
    SchedulerProfile *tProfile = NULL;

    T2Debug("%s ++in\n", __FUNCTION__);

    if (!sc_initialized)
    {
        T2Info("Scheduler is not initialized yet \n");
        return ;
    }
    sc_initialized = false;

    if(pthread_mutex_lock(&scMutex) != 0)
    {
        T2Error("scMutex lock failed\n");
    }
    for (; index < profileList->count; ++index)
    {
        tProfile = (SchedulerProfile *)Vector_At(profileList, index);
        if(pthread_mutex_lock(&tProfile->tMutex) != 0)
        {
            T2Error("tProfile Mutex locked failed\n");
        }
        tProfile->terminated = true;
        pthread_cond_signal(&tProfile->tCond);
        if(pthread_mutex_unlock(&tProfile->tMutex) != 0)
        {
            T2Error("tProfile Mutex unlocked failed\n");
        }
        T2Info(" tProfile->tId = %d tProfile->name = %s\n", (int)tProfile->tId, tProfile->name);
        //pthread_join(tProfile->tId, NULL); // pthread_detach in freeSchedulerProfile will detach the thread
        T2Info("Profile %s successfully removed from Scheduler\n", tProfile->name);
    }
    // Vector_Destroy(profileList, freeSchedulerProfile); This is not needed here as we will clear
    // profileList = NULL; the structure from unregisterProfileFromScheduler from TimeoutThread
    if(pthread_mutex_unlock(&scMutex) != 0)
    {
        T2Error("scMutex unlock failed\n");
    }
    if(pthread_mutex_destroy(&scMutex) != 0)
    {
        T2Error("scMutex destroy failed\n");
    }
    //timeoutNotificationCb = NULL; Not Needed as we are in the shutdown sequence

    T2Debug("%s --out\n", __FUNCTION__);
}

T2ERROR registerProfileWithScheduler(const char* profileName, unsigned int timeInterval, unsigned int activationTimeout, bool deleteonTimeout, bool repeat, bool reportOnUpdate, unsigned int firstReportingInterval, char *timeRef)
{
    T2ERROR ret;
    if(profileName == NULL)
    {
        T2Error("profileName is NULL\n");
        return T2ERROR_INVALID_ARGS;
    }
    T2Debug("%s ++in : profile - %s \n", __FUNCTION__, profileName);
    if(timeoutNotificationCb == NULL)
    {
        T2Error("Timerout Callback not set - Scheduler isn't initialized yet, Unable to register profile\n");
        return T2ERROR_FAILURE;
    }
    else
    {
        size_t index =  0 ;
        bool isSchedulerAssigned = false ;
        SchedulerProfile *tempSchProfile = NULL;
        if(sc_initialized)   // Check for existing scheduler to avoid duplicate scheduler entries
        {
            T2Debug("%s:%d scMutex is locked\n", __FUNCTION__, __LINE__);
            if(pthread_mutex_lock(&scMutex) != 0)
            {
                T2Error("scMutex lock failed\n");
                return T2ERROR_FAILURE;
            }
            for( ; index < profileList->count; ++index )
            {
                tempSchProfile = (SchedulerProfile *) Vector_At(profileList, index);
                if(strcmp(tempSchProfile->name, profileName) == 0)
                {
                    isSchedulerAssigned = true;
                    break ;
                }
            }
            T2Debug("%s:%d scMutex is unlocked\n", __FUNCTION__, __LINE__);
            if(pthread_mutex_unlock(&scMutex) != 0)
            {
                T2Error("scMutex unlock failed\n");
                return T2ERROR_FAILURE;
            }
            if (isSchedulerAssigned)
            {
                T2Info("Scheduler already assigned for profile %s , exiting .\n", profileName );
                T2Debug("%s --out\n", __FUNCTION__);
                return T2ERROR_SUCCESS ;
            }
        }

        SchedulerProfile *tProfile = (SchedulerProfile *)malloc(sizeof(SchedulerProfile));
        tProfile->name = strdup(profileName);
        tProfile->repeat = repeat;
        tProfile->timeOutDuration = timeInterval;
        tProfile->timeToLive = activationTimeout;
        tProfile->deleteonTime = deleteonTimeout;
        tProfile->terminated = false;
        tProfile->reportonupdate = reportOnUpdate;
        tProfile->firstreportint = firstReportingInterval;
        tProfile->firstexecution = false;
        tProfile->timeRef = timeRef;
        tProfile->timeRefinSec = 0;
        if(tProfile->timeOutDuration < tProfile->firstreportint)
        {
            tProfile->firstreportint = 0;
        }
        if(tProfile->firstreportint > 0 )
        {
            tProfile->firstexecution = true;
        }
        if(pthread_mutex_init(&tProfile->tMutex, NULL) != 0)
        {
            T2Error("%s Mutex init has failed\n",  __FUNCTION__);
            return T2ERROR_FAILURE;
        }
        pthread_cond_init(&tProfile->tCond, NULL);
        T2Info("Starting TimeoutThread for profile : %s\n", profileName);
        pthread_create(&tProfile->tId, NULL, TimeoutThread, (void*)tProfile);
        T2Debug("Thread id for profile %s is %d\n", tProfile->name, (int)tProfile->tId);
        T2Debug("%s --out\n", __FUNCTION__);
        T2Debug("%s:%d scMutex is locked\n", __FUNCTION__, __LINE__);
        if(pthread_mutex_lock(&scMutex) != 0)
        {
            T2Error("scMutex lock failed\n");
            return T2ERROR_FAILURE;
        }
        ret = Vector_PushBack(profileList, (void *)tProfile);
        T2Debug("%s:%d scMutex is unlocked\n", __FUNCTION__, __LINE__);
        if(pthread_mutex_unlock(&scMutex) != 0)
        {
            T2Error("scMutex unlock failed\n");
            return T2ERROR_FAILURE;
        }
        return ret;
    }
}

T2ERROR unregisterProfileFromScheduler(const char* profileName)
{
    if(profileName == NULL)
    {
        T2Error("profileName is NULL\n");
        return T2ERROR_INVALID_ARGS;
    }
    size_t index = 0;
    SchedulerProfile *tProfile = NULL;

    T2Debug("%s ++in\n", __FUNCTION__);

    if (!sc_initialized)
    {
        T2Error("scheduler not initialized \n");
        return T2ERROR_SUCCESS;
    }
    T2Debug("%s:%d scMutex is locked\n", __FUNCTION__, __LINE__);
    if(pthread_mutex_lock(&scMutex) != 0)
    {
        T2Error("scMutex lock failed\n");
        return T2ERROR_FAILURE;
    }
    for (; index < profileList->count; ++index)
    {
        tProfile = (SchedulerProfile *)Vector_At(profileList, index);

        if(strcmp(tProfile->name, profileName) == 0)
        {
            if(pthread_mutex_lock(&tProfile->tMutex) != 0)
            {
                T2Error("tProfile Mutex lock failed\n");
                return T2ERROR_FAILURE;
            }
            tProfile->terminated = true;
            signalrecived_and_executing = true;
            pthread_cond_signal(&tProfile->tCond);
            if(pthread_mutex_unlock(&tProfile->tMutex) != 0)
            {
                T2Error("tProfile Mutex unlock failed\n");
                pthread_mutex_unlock(&scMutex);
                return T2ERROR_FAILURE;
            }
            T2Info(" tProfile->tId = %d tProfile->name = %s\n", (int)tProfile->tId, tProfile->name);
            // pthread_join(tProfile->tId, NULL); // pthread_detach in freeSchedulerProfile will detach the thread
            sched_yield(); // This will give chance for the signal receiving thread to start
            int count = 0;
            while(signalrecived_and_executing && !is_activation_time_out)
            {
                if(count++ > 10)
                {
                    break;
                }
                sleep(1);
            }

            pthread_mutex_lock(&tProfile->tMutex);
            Vector_RemoveItem(profileList, tProfile, freeSchedulerProfile);
            pthread_mutex_unlock(&tProfile->tMutex);
            T2Debug("%s:%d scMutex is unlocked\n", __FUNCTION__, __LINE__);
            if(pthread_mutex_unlock(&scMutex) != 0)
            {
                T2Error("scMutex unlock failed\n");
                return T2ERROR_FAILURE;
            }

            T2Debug("%s --out\n", __FUNCTION__);
            return T2ERROR_SUCCESS;
        }
    }
    if(pthread_mutex_unlock(&scMutex) != 0)
    {
        T2Error("scMutex unlock failed\n");
        return T2ERROR_FAILURE;
    }
    T2Info("profile: %s, not found in scheduler. Already removed\n", profileName);
    return T2ERROR_FAILURE;
}
