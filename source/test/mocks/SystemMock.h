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
//#ifndef SOURCE_TEST_MOCKS_SYSTEMMOCK_H_
//#define SOURCE_TEST_MOCKS_SYSTEMMOCK_H_

#include <gtest/gtest.h>
#include <gmock/gmock.h>


#include "telemetry2_0.h"

class SystemMock
{
public:
    MOCK_METHOD(int, system, (const char* cmd), ());
    MOCK_METHOD(int, unlink, (const char *str), ());
    MOCK_METHOD(int, access, (const char *pathname, int mode), ());
    MOCK_METHOD(int, remove, (const char *pathname), ());
};

extern SystemMock* g_systemMock;

extern "C" int system(const char* cmd);
extern "C" int unlink(const char* str);
extern "C" int access(const char* pathname, int mode);
extern "C" int remove(const char* pathname);
