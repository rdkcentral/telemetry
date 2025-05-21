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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "telemetry2_0.h"
#include "reportgen.h"
#include "vector.h"
#include <cjson/cJSON.h>

typedef struct _ProfilexConf
{
    bool isUpdated;
    bool reportInProgress;
    bool bClearSeekMap;
    char* name;
    char* protocol;
    char* encodingType;
    unsigned int reportingInterval;
    unsigned int timeRef;
    unsigned int paramNumOfEntries;
    Vector *paramList;
    T2HTTP *t2HTTPDest;
    Vector *eMarkerList;
    Vector *gMarkerList;
    Vector *cachedReportList;
    cJSON *jsonReportObj;
    pthread_t reportThread;
} ProfilexConf;
/*
class XconfclientInterface
{
public:
    virtual ~XconfclientInterface() {}
    virtual bool isMtlsEnabled() = 0;
    virtual bool ProfileXConf_isSet() = 0;
    virtual T2ERROR processConfigurationXConf(char*, ProfilexConf **) = 0;
    virtual bool ProfileXConf_isNameEqual(char*) = 0;
    virtual T2ERROR ReportProfiles_deleteProfileXConf(ProfilexConf *) = 0;
    virtual T2ERROR ReportProfiles_setProfileXConf(ProfilexConf *) = 0;
    virtual T2ERROR getParameterValue(const char* paramName, char **paramValue) = 0;
};

class XconfclientMock: public XconfclientInterface
{
public:
    virtual ~XconfclientMock() {}
    MOCK_METHOD0(isMtlsEnabled, bool());
    MOCK_METHOD0(ProfileXConf_isSet, bool());
    MOCK_METHOD2(processConfigurationXConf, T2ERROR(char*, ProfilexConf **));
    MOCK_METHOD1(ProfileXConf_isNameEqual, bool(char*));
    MOCK_METHOD1(ReportProfiles_deleteProfileXConf, T2ERROR(ProfilexConf *));
    MOCK_METHOD1(ReportProfiles_setProfileXConf, T2ERROR(ProfilexConf *));
    MOCK_METHOD2(getParameterValue, T2ERROR(const char* paramName, char **paramValue));
};
*/

class XconfclientMock {
public:
    MOCK_METHOD(bool, isMtlsEnabled, (), ());
    MOCK_METHOD(bool, ProfileXConf_isSet, (), ());
    MOCK_METHOD(T2ERROR, processConfigurationXConf, (char*, ProfilexConf **), ());
    MOCK_METHOD(bool, ProfileXConf_isNameEqual, (char*), ());
    MOCK_METHOD(T2ERROR, ReportProfiles_deleteProfileXConf, (ProfilexConf *), ());
    MOCK_METHOD(T2ERROR, ReportProfiles_setProfileXConf, (ProfilexConf *), ());
    MOCK_METHOD(T2ERROR, getParameterValue, (const char* paramName, char **paramValue), ());
};

extern XconfclientMock* m_xconfclientMock;
