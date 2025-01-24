/*
* Copyright 2020 Comcast Cable Communications Management, LLC
** Licensed under the Apache License, Version 2.0 (the "License");
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
#ifndef SOURCE_TEST_MOCKS_SYSTEMMOCK_H_
#define SOURCE_TEST_MOCKS_SYSTEMMOCK_H_

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "telemetry2_0.h"

class SystemInterface {
public:
   virtual ~SystemInterface() {}
   virtual int system(const char *) = 0;
   virtual int unlink(const char *) = 0;
   virtual int access(const char *pathname , int mode) = 0;
   virtual int vsnprintf(char* str, size_t size, const char* format, va_list ap) = 0;
   virtual int remove(const char *pathname) = 0;
   virtual int v_secure_system(const char *arg) = 0;
   virtual FILE* v_secure_popen(const char *, const char *) = 0;
   virtual T2ERROR  getParameterValue(const char* paramName, char **paramValue) = 0;
   virtual T2ERROR publishEventsDCMSetConf(char *confPath) = 0;
   virtual int getRbusDCMEventStatus() = 0;
   virtual T2ERROR publishEventsDCMProcConf() = 0;
};

class SystemMock : public SystemInterface {
public:
   virtual ~SystemMock() {}
   MOCK_METHOD1(system, int(const char *));
   MOCK_METHOD1(unlink, int(const char *));
   MOCK_METHOD2(access, int(const char *, int));
   MOCK_METHOD4(vsnprintf, int(char*, size_t, const char*, va_list));
   MOCK_METHOD1(remove, int(const char *));
   MOCK_METHOD1(v_secure_system, int(const char *));
   MOCK_METHOD2(v_secure_popen, FILE*(const char*, const char*));
   MOCK_METHOD2(getParameterValue, T2ERROR(const char*, char **));
   MOCK_METHOD1(publishEventsDCMSetConf, T2ERROR(char *));
   MOCK_METHOD0(publishEventsDCMProcConf, T2ERROR());
   MOCK_METHOD0(getRbusDCMEventStatus, int());
};

#endif
