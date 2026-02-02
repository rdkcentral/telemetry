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

#ifndef MULTICURL_INTERFACE_MOCK_H
#define MULTICURL_INTERFACE_MOCK_H

#include <pthread.h>
#include <gmock/gmock.h>

#ifdef LIBRDKCERTSEL_BUILD
typedef enum
{
    certselectorOk = 0,
    certselectorFail = 1
} rdkcertselectorStatus_t;

typedef enum
{
    TRY_ANOTHER = 1,
    STOP_TRYING = 0
} rdkcertselectorRetry_t;
#endif

class MultiCurlInterfaceMock
{
public:
    // pthread functions
    MOCK_METHOD(int, pthread_mutex_init_mock, (pthread_mutex_t *mutex, const pthread_mutexattr_t *attr), ());
    MOCK_METHOD(int, pthread_mutex_destroy_mock, (pthread_mutex_t *mutex), ());
    MOCK_METHOD(int, pthread_mutex_lock_mock, (pthread_mutex_t *mutex), ());
    MOCK_METHOD(int, pthread_mutex_unlock_mock, (pthread_mutex_t *mutex), ());
    MOCK_METHOD(int, pthread_cond_init_mock, (pthread_cond_t *cond, const pthread_condattr_t *attr), ());
    MOCK_METHOD(int, pthread_cond_destroy_mock, (pthread_cond_t *cond), ());
    MOCK_METHOD(int, pthread_cond_wait_mock, (pthread_cond_t *cond, pthread_mutex_t *mutex), ());
    MOCK_METHOD(int, pthread_cond_signal_mock, (pthread_cond_t *cond), ());
    MOCK_METHOD(int, pthread_cond_broadcast_mock, (pthread_cond_t *cond), ());

    // Memory functions
    MOCK_METHOD(void*, malloc_mock, (size_t size), ());
    MOCK_METHOD(void, free_mock, (void* ptr), ());

    // File functions
    MOCK_METHOD(FILE*, fdopen_mock, (int fd, const char* mode), ());
    MOCK_METHOD(int, fclose_mock, (FILE* stream), ());
    MOCK_METHOD(int, fputs_mock, (const char* s, FILE* stream), ());

#ifdef LIBRDKCERTSEL_BUILD
    // Certificate selector functions
    MOCK_METHOD(rdkcertselectorStatus_t, rdkcertselector_getCert_mock,
                (void* selector, char** certUri, char** password), ());
    MOCK_METHOD(rdkcertselectorRetry_t, rdkcertselector_setCurlStatus_mock,
                (void* selector, int curlCode, const char* url), ());
#endif
};

extern MultiCurlInterfaceMock* g_multiCurlMock;

#endif // MULTICURL_INTERFACE_MOCK_H
