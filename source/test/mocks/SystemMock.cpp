/*
* Copyright 2020 Comcast Cable Communications Management, LLC
*
* Licensed under the Apache License, Version 2.0 (the "License");
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
#include <stdarg.h>
#include <dlfcn.h>
#include "test/mocks/SystemMock.h"

extern SystemMock * g_SystemMock;   /* This is just a declaration! The actual mock
                                     obj is defined globally in the test file. */

typedef int (*system_ptr) (const char * cmd);
typedef int (*unlink_ptr) (const char * str);
typedef int (*access_ptr) (const char * pathname, int mode);
typedef int (*remove_ptr) (const char *pathname);
typedef int (*vsnprintf_ptr)(char* str, size_t size, const char* format, va_list ap);
typedef int (*v_secure_system_ptr) (const char * cmd);
typedef FILE* (*v_secure_popen_ptr) (const char *direction, const char *command, ...);
typedef int (*getParameterValue_ptr) (const char* paramName, char **paramValue);
typedef int (*publishEventsDCMSetConf_ptr) (char *eventInfo);
typedef int (*getRbusDCMEventStatus_ptr) ();
typedef int (*publishEventsDCMProcConf_ptr) ();

system_ptr system_func = (system_ptr) dlsym(RTLD_NEXT, "system");
unlink_ptr unlink_func = (unlink_ptr) dlsym(RTLD_NEXT, "unlink");
access_ptr access_func = (access_ptr) dlsym(RTLD_NEXT, "access");
remove_ptr remove_func = (remove_ptr) dlsym(RTLD_NEXT, "remove");
vsnprintf_ptr vsnprintf_func = (vsnprintf_ptr) dlsym(RTLD_NEXT, "vsnprintf");
v_secure_system_ptr v_secure_system_func = (v_secure_system_ptr) dlsym(RTLD_NEXT, "v_secure_system");
v_secure_popen_ptr v_secure_popen_func = (v_secure_popen_ptr) dlsym(RTLD_NEXT, "v_secure_popen");
getParameterValue_ptr getParameterValue_func = (getParameterValue_ptr) dlsym(RTLD_NEXT, "getParameterValue");
publishEventsDCMSetConf_ptr publishEventsDCMSetConf_func = (publishEventsDCMSetConf_ptr) dlsym(RTLD_NEXT, "publishEventsDCMSetConf");
getRbusDCMEventStatus_ptr getRbusDCMEventStatus_func = (getRbusDCMEventStatus_ptr) dlsym(RTLD_NEXT, "getRbusDCMEventStatus");
publishEventsDCMProcConf_ptr publishEventsDCMProcConf_func = (publishEventsDCMProcConf_ptr) dlsym(RTLD_NEXT, "publishEventsDCMProcConf");

// Mock Method
extern "C" int system(const char * cmd)
{
    if (!g_SystemMock)
    {
        return 0;
    }
    return g_SystemMock->system(cmd);
}

extern "C" int unlink(const char * str)
{
    if (!g_SystemMock)
    {
        return 0;
    }
    return g_SystemMock->unlink(str);
}

extern "C" int access(const char * pathname, int mode)
{
    if (!g_SystemMock)
    {
        return -1;
    }
    return g_SystemMock->access(pathname, mode);
}

extern "C" int vsnprintf(char* str, size_t size, const char* format, va_list ap)
{
     if(!g_SystemMock)
     {
	 return -1;
     }
     return  g_SystemMock->vsnprintf(str, size, format,ap);
}

extern "C" int remove(const char *pathname)
{
   if (!g_SystemMock)
   {
        return 0;
   }
   return g_SystemMock->remove(pathname);
}

extern "C" int v_secure_system(const char * cmd, ...)
{
    if (!g_SystemMock)
    {
        return 0;
    }

    char format[250] = { 0 };

    va_list argptr;
    va_start(argptr, cmd);
    vsnprintf(format, sizeof(format), cmd, argptr);
    va_end(argptr);

    return g_SystemMock->v_secure_system(format);
}

extern "C" FILE* v_secure_popen(const char *direction, const char *command, ...)
{
        if (!g_SystemMock)
        {
        return NULL;
        }
        char format[250] = { 0 };

        va_list argptr;
        va_start(argptr, command);
        vsnprintf(format, sizeof(format), command, argptr);
        va_end(argptr);
        return g_SystemMock->v_secure_popen(direction, format);
}

extern "C" T2ERROR getParameterValue(const char* paramName, char **paramValue)
{
    if(!g_SystemMock)
    {
        return T2ERROR_FAILURE;
    }
    return g_SystemMock->getParameterValue(paramName, paramValue);
}

extern "C" T2ERROR publishEventsDCMSetConf(char *eventInfo)
{
    if(!g_SystemMock)
    {
        return T2ERROR_FAILURE;
    }
    return g_SystemMock->publishEventsDCMSetConf(eventInfo);
}

extern "C" int getRbusDCMEventStatus()
{
    if(!g_SystemMock)
    {
        return -1;
    }
    return g_SystemMock->getRbusDCMEventStatus();
}

extern "C" T2ERROR publishEventsDCMProcConf()
{
    if(!g_SystemMock)
    {
        return T2ERROR_FAILURE;
    }
    return g_SystemMock->publishEventsDCMProcConf();
}
