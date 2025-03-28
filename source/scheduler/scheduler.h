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

#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include "telemetry2_0.h"
#include <pthread.h>
#include <stdbool.h>
#include <sys/time.h>

typedef struct _SchedulerProfile {
  char *name;
  char *timeRef;
  unsigned int timeOutDuration;
  unsigned int timeToLive;
  unsigned int timeRefinSec;
  bool repeat;
  bool terminated;
  bool deleteonTime;
  bool reportonupdate;
  unsigned int firstreportint;
  bool firstexecution;
  pthread_t tId;
  pthread_mutex_t tMutex;
  pthread_cond_t tCond;

} SchedulerProfile;

typedef void (*TimeoutNotificationCB)(const char *profileName,
                                      bool isClearSeekMap);
typedef void (*ActivationTimeoutCB)(const char *profileName);
typedef void (*NotifySchedulerstartCB)(char *profileName,
                                       bool isschedulerstarted);

int getLapsedTime(struct timespec *result, struct timespec *x,
                  struct timespec *y);

T2ERROR initScheduler(TimeoutNotificationCB notificationCb,
                      ActivationTimeoutCB activationCB,
                      NotifySchedulerstartCB notifyschedulerCB);

void uninitScheduler();

T2ERROR registerProfileWithScheduler(
    const char *profileName, unsigned int timeInterval,
    unsigned int activationTimeout, bool deleteonTimout, bool repeat,
    bool reportOnUpdate, unsigned int firstReportingInterval, char *timeRef);

T2ERROR unregisterProfileFromScheduler(const char *profileName);

T2ERROR SendInterruptToTimeoutThread(char *profileName);

bool get_logdemand();

void set_logdemand(bool value);

#endif /* _SCHEDULER_H_ */
