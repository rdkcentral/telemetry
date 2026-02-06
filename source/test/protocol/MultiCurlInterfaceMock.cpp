/*
 * Copyright 2026 Comcast Cable Communications Management, LLC
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

#include "MultiCurlInterfaceMock.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

MultiCurlInterfaceMock* g_multiCurlMock = nullptr;

extern "C" {

// pthread function wrappers
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
    if (g_multiCurlMock != nullptr) {
        return g_multiCurlMock->pthread_mutex_init_mock(mutex, attr);
    }
    return 0;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
    if (g_multiCurlMock != nullptr) {
        return g_multiCurlMock->pthread_mutex_destroy_mock(mutex);
    }
    return 0;
}

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
    if (g_multiCurlMock != nullptr) {
        return g_multiCurlMock->pthread_mutex_lock_mock(mutex);
    }
    return 0;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    if (g_multiCurlMock != nullptr) {
        return g_multiCurlMock->pthread_mutex_unlock_mock(mutex);
    }
    return 0;
}

int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr)
{
    if (g_multiCurlMock != nullptr) {
        return g_multiCurlMock->pthread_cond_init_mock(cond, attr);
    }
    return 0;
}

int pthread_cond_destroy(pthread_cond_t *cond)
{
    if (g_multiCurlMock != nullptr) {
        return g_multiCurlMock->pthread_cond_destroy_mock(cond);
    }
    return 0;
}

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    if (g_multiCurlMock != nullptr) {
        return g_multiCurlMock->pthread_cond_wait_mock(cond, mutex);
    }
    return 0;
}

int pthread_cond_signal(pthread_cond_t *cond)
{
    if (g_multiCurlMock != nullptr) {
        return g_multiCurlMock->pthread_cond_signal_mock(cond);
    }
    return 0;
}

int pthread_cond_broadcast(pthread_cond_t *cond)
{
    if (g_multiCurlMock != nullptr) {
        return g_multiCurlMock->pthread_cond_broadcast_mock(cond);
    }
    return 0;
}

// Memory function wrappers
void* malloc(size_t size)
{
    if (g_multiCurlMock != nullptr) {
        return g_multiCurlMock->malloc_mock(size);
    }
    return ::malloc(size);
}

void free(void* ptr)
{
    if (g_multiCurlMock != nullptr) {
        g_multiCurlMock->free_mock(ptr);
        return;
    }
    ::free(ptr);
}

// File function wrappers
FILE* fdopen(int fd, const char* mode)
{
    if (g_multiCurlMock != nullptr) {
        return g_multiCurlMock->fdopen_mock(fd, mode);
    }
    return ::fdopen(fd, mode);
}

int fclose(FILE* stream)
{
    if (g_multiCurlMock != nullptr) {
        return g_multiCurlMock->fclose_mock(stream);
    }
    return ::fclose(stream);
}

int fputs(const char* s, FILE* stream)
{
    if (g_multiCurlMock != nullptr) {
        return g_multiCurlMock->fputs_mock(s, stream);
    }
    return ::fputs(s, stream);
}

#ifdef LIBRDKCERTSEL_BUILD
// Certificate selector function wrappers
typedef struct rdkcertselector_s rdkcertselector_t;
typedef rdkcertselector_t *rdkcertselector_h;

rdkcertselectorStatus_t rdkcertselector_getCert(rdkcertselector_h selector, char** certUri, char** password)
{
    if (g_multiCurlMock != nullptr) {
        return (rdkcertselectorStatus_t)g_multiCurlMock->rdkcertselector_getCert_mock((void*)selector, certUri, password);
    }
    return certselectorFail;
}

rdkcertselectorRetry_t rdkcertselector_setCurlStatus(rdkcertselector_h selector, CURLcode curlCode, const char* url)
{
    if (g_multiCurlMock != nullptr) {
        return (rdkcertselectorRetry_t)g_multiCurlMock->rdkcertselector_setCurlStatus_mock((void*)selector, (int)curlCode, url);
    }
    return STOP_TRYING;
}
#endif

} // extern "C"
