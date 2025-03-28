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

#include <curl/curl.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

class ProtocolInterface
{
public:
    virtual ~ProtocolInterface() {}
    virtual bool isMtlsEnabled() = 0;
};

class ProtocolMock: public ProtocolInterface
{
public:
    virtual ~ProtocolMock() {}
    MOCK_METHOD0(isMtlsEnabled, bool());
};

