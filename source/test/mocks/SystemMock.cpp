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


typedef int (*system_ptr) (const char * cmd);
typedef int (*unlink_ptr) (const char * str);
typedef int (*access_ptr) (const char * pathname, int mode);

system_ptr system_func = (system_ptr) dlsym(RTLD_NEXT, "system");
unlink_ptr unlink_func = (unlink_ptr) dlsym(RTLD_NEXT, "unlink");
access_ptr access_func = (access_ptr) dlsym(RTLD_NEXT, "access");

// Mock Method
extern "C" int system(const char * cmd)
{
    if (g_systemMock)
    {
        return g_systemMock->system(cmd);
    }
    return system_func(cmd);
}

extern "C" int unlink(const char * str)
{
    if (g_systemMock)
    {
        return g_systemMock->unlink(str);
    }
    return unlink_func(str);
}

extern "C" int access(const char * pathname, int mode)
{
    if (!g_systemMock)
    {
        return access_func(pathname,mode);
    }
    return g_systemMock->access(pathname, mode);
}
