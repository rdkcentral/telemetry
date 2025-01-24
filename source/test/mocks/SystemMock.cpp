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

#include "test/mocks/SystemMock.h"

extern SystemMock * g_SystemMock;   /* This is just a declaration! The actual mock
                                     obj is defined globally in the test file. */

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