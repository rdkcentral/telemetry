/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2026 RDK Management
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

/**
 * @file profileStub.c
 * @brief Stub implementation of profile functions for unit testing
 *
 * Provides minimal stubs for profile.c functions to avoid pulling in
 * the entire dependency tree. Used by t2parser and dcautil tests.
 */

#include <stdlib.h>

/**
 * @brief Stub for freeProfile - minimal cleanup for testing
 * 
 * In production, this function frees all memory in a Profile struct.
 * For unit tests, we provide a minimal stub that just frees the data pointer.
 * Tests that actually allocate Profile structs should provide their own cleanup.
 */
void freeProfile(void *data)
{
    if(data != NULL)
    {
        free(data);
    }
}
