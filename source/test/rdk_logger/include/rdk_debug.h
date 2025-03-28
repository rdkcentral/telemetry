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

typedef unsigned int uint32_t;
typedef uint32_t rdk_Error;
#define RDK_SUCCESS 0
#define DEBUG_INI_NAME "/etc/debug.ini"

typedef enum {
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

rdk_Error rdk_logger_init(const char *debugConfigFile);
