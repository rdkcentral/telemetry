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
typedef int (*remove_ptr) (const char * pathname);
typedef int (*inotify_init1_ptr) (int flags);
typedef int (*inotify_add_watch_ptr) (int fd, const char *pathname, uint32_t mask);
typedef int (*inotify_rm_watch_ptr) (int fd, int wd);
typedef int (*clock_gettime_ptr) (clockid_t clk_id, struct timespec *tp);
typedef int (*select_ptr) (int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);

system_ptr system_func = (system_ptr) dlsym(RTLD_NEXT, "system");
unlink_ptr unlink_func = (unlink_ptr) dlsym(RTLD_NEXT, "unlink");
access_ptr access_func = (access_ptr) dlsym(RTLD_NEXT, "access");
remove_ptr remove_func = (remove_ptr) dlsym(RTLD_NEXT, "remove");
inotify_init1_ptr inotify_init1_func = (inotify_init1_ptr) dlsym(RTLD_NEXT, "inotify_init1");
inotify_add_watch_ptr inotify_add_watch_func = (inotify_add_watch_ptr) dlsym(RTLD_NEXT, "inotify_add_watch");
inotify_rm_watch_ptr inotify_rm_watch_func = (inotify_rm_watch_ptr) dlsym(RTLD_NEXT, "inotify_rm_watch");
clock_gettime_ptr clock_gettime_func = (clock_gettime_ptr) dlsym(RTLD_NEXT, "clock_gettime");
select_ptr select_func = (select_ptr) dlsym(RTLD_NEXT, "select");

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

extern "C" int remove(const char *pathname)
{
    if(!g_systemMock)                                                                                                             
    {                                                                                                                              
        return remove_func(pathname);                                                                                         
    }                                                                                                                              
    return g_systemMock->remove(pathname);                                                                                   
}

extern "C" int inotify_init1(int flags)
{
    if (!g_systemMock)
    {
        return inotify_init1_func(flags);
    }
    return g_systemMock->inotify_init1(flags);
}

extern "C" int inotify_add_watch(int fd, const char *pathname, uint32_t mask)
{
    if (!g_systemMock)
    {
        return inotify_add_watch_func(fd, pathname, mask);
    }
    return g_systemMock->inotify_add_watch(fd, pathname, mask);
}

extern "C" int inotify_rm_watch(int fd, int wd)
{
    if (!g_systemMock)
    {
        return inotify_rm_watch_func(fd, wd);
    }
    return g_systemMock->inotify_rm_watch(fd, wd);
}

extern "C" int clock_gettime(clockid_t clk_id, struct timespec *tp)
{
    if (!g_systemMock)
    {
        return clock_gettime_func(clk_id, tp);
    }
    return g_systemMock->clock_gettime(clk_id, tp);
}

extern "C" int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
    if (!g_systemMock)
    {
        return select_func(nfds, readfds, writefds, exceptfds, timeout);
    }
    return g_systemMock->select(nfds, readfds, writefds, exceptfds, timeout);
}                                              
