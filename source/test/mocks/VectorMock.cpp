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
#include <stdlib.h>
#include <string.h>
#include "test/mocks/VectorMock.h"

VectorMock* g_vectorMock = nullptr;

extern "C" {
// Mock implementations of Vector functions
T2ERROR Vector_Create(Vector** v)
{
    if (g_vectorMock)
    {
        return g_vectorMock->Vector_Create(v);
    }
    
    // Fallback implementation for when mock is not set
    if (!v) return T2ERROR_FAILURE;
    
    *v = (Vector*)malloc(sizeof(Vector));
    if (!*v) return T2ERROR_FAILURE;
    
    (*v)->data = nullptr;
    (*v)->capacity = 0;
    (*v)->count = 0;
    
    return T2ERROR_SUCCESS;
}

T2ERROR Vector_Destroy(Vector* v, Vector_Cleanup destroyer)
{
    if (g_vectorMock)
    {
        return g_vectorMock->Vector_Destroy(v, destroyer);
    }
    
    // Fallback implementation
    if (!v) return T2ERROR_FAILURE;
    
    if (v->data && destroyer)
    {
        for (size_t i = 0; i < v->count; i++)
        {
            if (v->data[i])
            {
                destroyer(v->data[i]);
            }
        }
    }
    
    if (v->data) free(v->data);
    free(v);
    
    return T2ERROR_SUCCESS;
}

T2ERROR Vector_Clear(Vector* v, Vector_Cleanup destroyer)
{
    if (g_vectorMock)
    {
        return g_vectorMock->Vector_Clear(v, destroyer);
    }
    
    // Fallback implementation
    if (!v) return T2ERROR_FAILURE;
    
    if (v->data && destroyer)
    {
        for (size_t i = 0; i < v->count; i++)
        {
            if (v->data[i])
            {
                destroyer(v->data[i]);
            }
        }
    }
    
    v->count = 0;
    return T2ERROR_SUCCESS;
}

T2ERROR Vector_PushBack(Vector* v, void* item)
{
    if (g_vectorMock)
    {
        return g_vectorMock->Vector_PushBack(v, item);
    }
    
    // Fallback implementation
    if (!v) return T2ERROR_FAILURE;
    
    if (v->count >= v->capacity)
    {
        size_t new_capacity = (v->capacity == 0) ? 4 : v->capacity * 2;
        void** new_data = (void**)realloc(v->data, new_capacity * sizeof(void*));
        if (!new_data) return T2ERROR_FAILURE;
        
        v->data = new_data;
        v->capacity = new_capacity;
    }
    
    v->data[v->count] = item;
    v->count++;
    
    return T2ERROR_SUCCESS;
}

T2ERROR Vector_RemoveItem(Vector* v, void* item, Vector_Cleanup destroyer)
{
    if (g_vectorMock)
    {
        return g_vectorMock->Vector_RemoveItem(v, item, destroyer);
    }
    
    // Fallback implementation
    if (!v || !v->data) return T2ERROR_FAILURE;
    
    for (size_t i = 0; i < v->count; i++)
    {
        if (v->data[i] == item)
        {
            if (destroyer && v->data[i])
            {
                destroyer(v->data[i]);
            }
            
            // Shift remaining elements
            for (size_t j = i; j < v->count - 1; j++)
            {
                v->data[j] = v->data[j + 1];
            }
            v->count--;
            return T2ERROR_SUCCESS;
        }
    }
    
    return T2ERROR_FAILURE;
}

T2ERROR Vector_Sort(Vector* v, size_t size, Vecor_Comparator comparator)
{
    if (g_vectorMock)
    {
        return g_vectorMock->Vector_Sort(v, size, comparator);
    }
    
    // Fallback implementation - simple stub
    if (!v || !comparator) return T2ERROR_FAILURE;
    
    // For test purposes, just return success without actual sorting
    return T2ERROR_SUCCESS;
}

void* Vector_At(Vector* v, size_t index)
{
    if (g_vectorMock)
    {
        return g_vectorMock->Vector_At(v, index);
    }
    
    // Fallback implementation
    if (!v || !v->data || index >= v->count)
    {
        return nullptr;
    }
    
    return v->data[index];
}

size_t Vector_Size(Vector* v)
{
    if (g_vectorMock)
    {
        return g_vectorMock->Vector_Size(v);
    }
    
    // Fallback implementation
    if (!v) return 0;
    
    return v->count;
}

} // extern "C"
