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

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef GTEST_ENABLE
#include "test/rdk_logger/include/rdk_debug.h"
#else
#include "t2log_wrapper.h"
#endif

unsigned int rdkLogLevel = RDK_LOG_INFO;

void LOGInit() {
#ifdef RDK_LOGGER
  rdk_logger_init(DEBUG_INI_NAME);
#else
  printf("LOG.RDK.T2: LOGInit\n");
#endif
}

void T2Log(unsigned int level, const char *msg, ...) {
  va_list arg;
  char *pTempChar = NULL;
  int ret = 0;

  if (NULL == msg) {
#ifdef RDK_LOGGER
    RDK_LOG(level, "LOG.RDK.T2", "NULL message passed to T2Log");
#else
    printf("LOG.RDK.T2 [lvl=%u]: NULL message passed to T2Log", level);
#endif
    return;
  }

  va_start(arg, msg);
  int messageLen = vsnprintf(NULL, 0, msg, arg);
  va_end(arg);

  if (messageLen < 1) {
#ifdef RDK_LOGGER
    RDK_LOG(level, "LOG.RDK.T2", "Failed [%d] to compose a message [%s].",
            messageLen, msg);
#else
    printf("LOG.RDK.T2 [lvl=%u]: Failed [%d] to compose a message [%s].", level,
           messageLen, msg);
#endif
    return;
  }

  messageLen++;
  pTempChar = (char *)malloc(messageLen);
  memset(pTempChar, '\0', messageLen);
  if (pTempChar) {
    va_start(arg, msg);
    ret = vsnprintf(pTempChar, messageLen, msg, arg);
    if (ret < 0) {
      perror(pTempChar);
    }
    va_end(arg);
#ifdef RDK_LOGGER
    RDK_LOG(level, "LOG.RDK.T2", "%s", pTempChar);
#else
    printf("LOG.RDK.T2 [lvl=%u]: %s", level, pTempChar);
#endif
    free(pTempChar);
  }
}
