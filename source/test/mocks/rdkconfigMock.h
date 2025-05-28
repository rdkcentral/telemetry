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

#define RDKCONFIG_OK 0
#define RDKCONFIG_FAIL 1

#include <gtest/gtest.h>
#include <gmock/gmock.h>

class rdkconfigMock
{
public:
    MOCK_METHOD(int, rdkconfig_get, (uint8_t **sbuff, size_t *sbuffsz, const char *refname ), ());
    MOCK_METHOD(int, rdkconfig_set, (const char *refname, uint8_t *sbuff, size_t sbuffsz), ());
    MOCK_METHOD(int, rdkconfig_free, (uint8_t **sbuff, size_t sbuffsz ), ());
};

extern rdkconfigMock* g_rdkconfigMock;

extern "C" int rdkconfig_get( uint8_t **sbuff, size_t *sbuffsz, const char *refname );
extern "C" int rdkconfig_set( const char *refname, uint8_t *sbuff, size_t sbuffsz );
extern "C" int rdkconfig_free( uint8_t **sbuff, size_t sbuffsz );
