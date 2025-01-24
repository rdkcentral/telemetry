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

#include "xconfclientMock.h"

extern XconfclientMock *m_xconfclientMock;

extern "C" bool isMtlsEnabled()
{
    if (!m_xconfclientMock)
    {
         return false;
    }
    return m_xconfclientMock->isMtlsEnabled();
}

extern "C" bool ProfileXConf_isSet()
{
    if (!m_xconfclientMock)
    {
         return false;
    }
    return m_xconfclientMock->ProfileXConf_isSet();
}

extern "C" T2ERROR processConfigurationXConf(char* configData, ProfilexConf **localProfile)
{
    if (!m_xconfclientMock)
    {
         return T2ERROR_FAILURE;
    }
    return m_xconfclientMock->processConfigurationXConf(configData, localProfile);
}

extern "C" bool ProfileXConf_isNameEqual(char* profileName)
{
    if (!m_xconfclientMock)
    {
         return false;
    }
    return m_xconfclientMock->ProfileXConf_isNameEqual(profileName);
}

extern "C" T2ERROR ReportProfiles_deleteProfileXConf(ProfilexConf *profile)
{
    if (!m_xconfclientMock)
    { 
         return T2ERROR_FAILURE;
    }
    return m_xconfclientMock->ReportProfiles_deleteProfileXConf(profile);
}

extern "C" T2ERROR ReportProfiles_setProfileXConf(ProfilexConf *profile)
{
    if (!m_xconfclientMock)
    {
         return T2ERROR_FAILURE;
    }
    return m_xconfclientMock->ReportProfiles_setProfileXConf(profile);
}
