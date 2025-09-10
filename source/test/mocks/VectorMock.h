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

#ifndef SOURCE_TEST_MOCKS_VECTORMOCK_H_
#define SOURCE_TEST_MOCKS_VECTORMOCK_H_

#include <gtest/gtest.h>
#include <gmock/gmock.h>

extern "C" {
#include "vector.h"
#include "telemetry2_0.h"
}

class VectorMock
{
public:
    MOCK_METHOD(T2ERROR, Vector_Create, (Vector** v), ());
    MOCK_METHOD(T2ERROR, Vector_Destroy, (Vector* v, Vector_Cleanup destroyer), ());
    MOCK_METHOD(T2ERROR, Vector_Clear, (Vector* v, Vector_Cleanup destroyer), ());
    MOCK_METHOD(T2ERROR, Vector_PushBack, (Vector* v, void* item), ());
    MOCK_METHOD(T2ERROR, Vector_RemoveItem, (Vector* v, void* item, Vector_Cleanup destroyer), ());
    MOCK_METHOD(T2ERROR, Vector_Sort, (Vector* v, size_t size, Vecor_Comparator comparator), ());
    MOCK_METHOD(void*, Vector_At, (Vector* v, size_t index), ());
    MOCK_METHOD(size_t, Vector_Size, (Vector* v), ());
};

extern VectorMock* g_vectorMock;

#endif // SOURCE_TEST_MOCKS_VECTORMOCK_H_
