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

#include "telemetry2_0.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

class SchedulerInterface {
public:
  virtual ~SchedulerInterface() {}
  virtual T2ERROR deleteProfile(const char *profileName) = 0;
  virtual unsigned int getMinThresholdDuration(char *profileName) = 0;
  virtual T2ERROR registerTriggerConditionConsumer() = 0;
};

class SchedulerMock : public SchedulerInterface {
public:
  virtual ~SchedulerMock() {}
  MOCK_METHOD0(registerTriggerConditionConsumer, T2ERROR());
  MOCK_METHOD1(getMinThresholdDuration, unsigned int(char *));
  MOCK_METHOD1(deleteProfile, T2ERROR(const char *));
};
