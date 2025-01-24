/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2019 RDK Management
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
#ifndef  _CCSP_T2LOG_WRPPER_H_
#define  _CCSP_T2LOG_WRPPER_H_

#define DEBUG_INI_NAME  "/etc/debug.ini"

#if defined(RDK_LOGGER)
    #include "rdk_debug.h"
#else
typedef enum
{
    ENUM_RDK_LOG_BEGIN = 0, /**< Used as array index. */

    RDK_LOG_FATAL = ENUM_RDK_LOG_BEGIN,
    RDK_LOG_ERROR,
    RDK_LOG_WARN,
    RDK_LOG_NOTICE,
    RDK_LOG_INFO,
    RDK_LOG_DEBUG,

    RDK_LOG_TRACE1,
    RDK_LOG_TRACE2,
    RDK_LOG_TRACE3,
    RDK_LOG_TRACE4,
    RDK_LOG_TRACE5,
    RDK_LOG_TRACE6,
    RDK_LOG_TRACE7,
    RDK_LOG_TRACE8,
    RDK_LOG_TRACE9,

    ENUM_RDK_LOG_COUNT
} rdk_LogLevel;

#endif


#if defined(ENABLE_RDKB_SUPPORT)
#define ENABLE_DEBUG_FLAG "/nvram/enable_t2_debug"
#elif defined(DEVICE_EXTENDER)
#define ENABLE_DEBUG_FLAG "/usr/opensync/data/enable_t2_debug"
#else
#define ENABLE_DEBUG_FLAG "/opt/enable_t2_debug"
#endif

#define T2Error(...)                   T2Log(RDK_LOG_ERROR, __VA_ARGS__)
#define T2Info(...)                    T2Log(RDK_LOG_INFO, __VA_ARGS__)
#define T2Warning(...)                 T2Log(RDK_LOG_WARN, __VA_ARGS__)
#define T2Debug(...)                   T2Log(RDK_LOG_DEBUG, __VA_ARGS__)

void LOGInit();

void T2Log(unsigned int level, const char *msg, ...)
    __attribute__((format (printf, 2, 3)));

#endif

