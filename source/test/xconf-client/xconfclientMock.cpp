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
#include <dlfcn.h>

typedef bool (*isMtlsEnabled_ptr)();
typedef bool (*ProfileXConf_isSet_ptr)();
typedef T2ERROR (*processConfigurationXConf_ptr)(char* configData, ProfilexConf **localProfile);
typedef bool (*ProfileXConf_isNameEqual_ptr)(char* profileName);
typedef T2ERROR (*ReportProfiles_deleteProfileXConf_ptr)(ProfilexConf *profile);
typedef T2ERROR (*ReportProfiles_setProfileXConf_ptr)(ProfilexConf *profile);
typedef T2ERROR (*getParameterValue_ptr)(const char* paramName, char **paramValue);
typedef int (*getRbusDCMEventStatus_ptr)();
typedef T2ERROR (*publishEventsDCMSetConf_ptr)(char *confPath);
typedef T2ERROR (*publishEventsDCMProcConf_ptr)();
//pedef T2ERROR (*getMtlsCerts_ptr)(char **certName, char **phrase);


isMtlsEnabled_ptr isMtlsEnabled_func = (isMtlsEnabled_ptr) dlsym(RTLD_NEXT, "isMtlsEnabled");
ProfileXConf_isSet_ptr ProfileXConf_isSet_func = (ProfileXConf_isSet_ptr) dlsym(RTLD_NEXT, "ProfileXConf_isSet");
processConfigurationXConf_ptr processConfigurationXConf_func = (processConfigurationXConf_ptr) dlsym(RTLD_NEXT, "processConfigurationXConf");
ProfileXConf_isNameEqual_ptr ProfileXConf_isNameEqual_func = (ProfileXConf_isNameEqual_ptr) dlsym(RTLD_NEXT, "ProfileXConf_isNameEqual");
ReportProfiles_deleteProfileXConf_ptr ReportProfiles_deleteProfileXConf_func = (ReportProfiles_deleteProfileXConf_ptr) dlsym(RTLD_NEXT, "ReportProfiles_deleteProfileXConf");
ReportProfiles_setProfileXConf_ptr ReportProfiles_setProfileXConf_func = (ReportProfiles_setProfileXConf_ptr) dlsym(RTLD_NEXT, "ReportProfiles_setProfileXConf");
getParameterValue_ptr getParameterValue_func = (getParameterValue_ptr) dlsym(RTLD_NEXT, "getParameterValue");
getRbusDCMEventStatus_ptr getRbusDCMEventStatus_func = (getRbusDCMEventStatus_ptr) dlsym(RTLD_NEXT, "getRbusDCMEventStatus");
publishEventsDCMSetConf_ptr publishEventsDCMSetConf_func = (publishEventsDCMSetConf_ptr) dlsym(RTLD_NEXT, "publishEventsDCMSetConf");
publishEventsDCMProcConf_ptr publishEventsDCMProcConf_func = (publishEventsDCMProcConf_ptr) dlsym(RTLD_NEXT, "publishEventsDCMProcConf");
//tMtlsCerts_ptr getMtlsCerts_func = (getMtlsCerts_ptr) dlsym(RTLD_NEXT, "getMtlsCerts");


extern "C" bool isMtlsEnabled()
{
    if (!m_xconfclientMock)
    {
        return isMtlsEnabled_func();
    }
    return m_xconfclientMock->isMtlsEnabled();
}

extern "C" bool ProfileXConf_isSet()
{
    if (!m_xconfclientMock)
    {
         return ProfileXConf_isSet_func();
    }
    return m_xconfclientMock->ProfileXConf_isSet();
}

extern "C" T2ERROR processConfigurationXConf(char* configData, ProfilexConf **localProfile)
{
    if (!m_xconfclientMock)
    {
         return processConfigurationXConf_func(configData, localProfile);
    }
    return m_xconfclientMock->processConfigurationXConf(configData, localProfile);
}

extern "C" bool ProfileXConf_isNameEqual(char* profileName)
{
    if (!m_xconfclientMock)
    {
         return ProfileXConf_isNameEqual_func(profileName);
    }
    return m_xconfclientMock->ProfileXConf_isNameEqual(profileName);
}

extern "C" T2ERROR ReportProfiles_deleteProfileXConf(ProfilexConf *profile)
{
    if (!m_xconfclientMock)
    { 
         return ReportProfiles_deleteProfileXConf_func(profile);
    }
    return m_xconfclientMock->ReportProfiles_deleteProfileXConf(profile);
}

extern "C" T2ERROR ReportProfiles_setProfileXConf(ProfilexConf *profile)
{
    if (!m_xconfclientMock)
    {
         return ReportProfiles_setProfileXConf_func(profile);
    }
    return m_xconfclientMock->ReportProfiles_setProfileXConf(profile);
}

extern "C" T2ERROR getParameterValue(const char* paramName, char **paramValue)
{
    if (!m_xconfclientMock)
    {
         return getParameterValue_func(paramName, paramValue);
    }
    return m_xconfclientMock->getParameterValue(paramName, paramValue);
}

extern "C" int getRbusDCMEventStatus()
{
    if (!m_xconfclientMock)
    {
         return getRbusDCMEventStatus_func();
    }
    return m_xconfclientMock->getRbusDCMEventStatus();
}

extern "C" T2ERROR publishEventsDCMSetConf(char *confPath)
{
    if (!m_xconfclientMock)
    {
         return publishEventsDCMSetConf_func(confPath);
    }
    return m_xconfclientMock->publishEventsDCMSetConf(confPath);
}

extern "C" T2ERROR publishEventsDCMProcConf()
{
    if (!m_xconfclientMock)
    {
         return publishEventsDCMProcConf_func();
    }
    return m_xconfclientMock->publishEventsDCMProcConf();
}