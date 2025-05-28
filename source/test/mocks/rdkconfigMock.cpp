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

#include <iostream>
#include "rdkconfigMock.h"
using namespace std;


extern "C" int rdkconfig_get( uint8_t **sbuff, size_t *sbuffsz, const char *refname )
{
    if(!g_rdkconfigMock)
    {
        return RDKCONFIG_FAIL ;
    }
    return g_rdkconfigMock->rdkconfig_get(sbuff, sbuffsz, refname);
}

extern "C" int rdkconfig_set( const char *refname, uint8_t *sbuff, size_t sbuffsz )
{   if(!g_rdkconfigMock)
    {
        return RDKCONFIG_FAIL ;
    }
    return g_rdkconfigMock->rdkconfig_set(refname, sbuff, sbuffsz);
}

extern "C" int rdkconfig_free( uint8_t **sbuff, size_t sbuffsz )
{
    if(!g_rdkconfigMock)
    {
        return RDKCONFIG_FAIL ;
    }
    return g_rdkconfigMock->rdkconfig_free(sbuff, sbuffsz);
}
