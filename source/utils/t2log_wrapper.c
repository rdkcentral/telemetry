

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


#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <pthread.h>
#ifdef GTEST_ENABLE
#include "test/rdk_logger/include/rdk_debug.h"
#else
#include "t2log_wrapper.h"
#endif

unsigned int rdkLogLevel = RDK_LOG_INFO;
static pthread_mutex_t loggerMutex = PTHREAD_MUTEX_INITIALIZER;

/* Convert log level to string */
static const char* getLogLevelString(unsigned int level)
{
    switch(level)
    {
        case RDK_LOG_FATAL:  return "FATAL";
        case RDK_LOG_ERROR:  return "ERROR";
        case RDK_LOG_WARN:   return "WARN";
        case RDK_LOG_NOTICE: return "NOTICE";
        case RDK_LOG_INFO:   return "INFO";
        case RDK_LOG_DEBUG:  return "DEBUG";
        default:             return "UNKNOWN";
    }
}

void  LOGInit()
{
#ifdef RDK_LOGGER
    rdk_logger_init(DEBUG_INI_NAME);
#else
    printf("LOG.RDK.T2: LOGInit\n");
#endif

}

#if 0
void T2Log(unsigned int level, const char *msg, ...)
{
    va_list arg;
    char *pTempChar = NULL;
    int ret = 0;

    if (NULL == msg)
    {
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

    if (messageLen < 1)
    {
#ifdef RDK_LOGGER
        RDK_LOG(level, "LOG.RDK.T2", "Failed [%d] to compose a message [%s].", messageLen, msg);
#else
        printf("LOG.RDK.T2 [lvl=%u]: Failed [%d] to compose a message [%s].", level, messageLen, msg);
#endif
        return;
    }

    messageLen++;
    pTempChar = (char *)malloc(messageLen);
    memset(pTempChar, '\0', messageLen);
    if(pTempChar)
    {
        va_start(arg, msg);
        ret = vsnprintf(pTempChar, messageLen, msg, arg);
        if(ret < 0)
        {
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
#endif

void T2Log(unsigned int level, const char *msg, ...)
{
    va_list arg;
    char *pTempChar = NULL;
    int ret = 0;
    FILE *logHandle = NULL;

    if (NULL == msg)
    {
        return;
    }

    /* Check if debug logging is enabled for RDK_LOG_DEBUG (level 5) */
    if (level == RDK_LOG_DEBUG)
    {
        if (access(ENABLE_DEBUG_FLAG, F_OK) == -1)
        {
            return;
        }
    }

    /* Compose the log message */
    va_start(arg, msg);
    int messageLen = vsnprintf(NULL, 0, msg, arg);
    va_end(arg);

    if (messageLen < 1)
    {
        return;
    }

    messageLen++;
    pTempChar = (char *)malloc(messageLen);
    if (!pTempChar)
    {
        return;
    }

    memset(pTempChar, '\0', messageLen);
    va_start(arg, msg);
    ret = vsnprintf(pTempChar, messageLen, msg, arg);
    va_end(arg);

    if (ret < 0)
    {
        free(pTempChar);
        return;
    }

    /* Write to log file with timestamp */
    pthread_mutex_lock(&loggerMutex);
    logHandle = fopen("/opt/logs/t2.log", "a+");
    if (logHandle)
    {
        struct timespec ts;
        struct tm timeinfo;

        if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
        {
            fclose(logHandle);
            pthread_mutex_unlock(&loggerMutex);
            free(pTempChar);
            return;
        }

        char timeBuffer[24] = { '\0' };
        long msecs;

        localtime_r(&ts.tv_sec, &timeinfo);
        msecs = ts.tv_nsec / 1000000;
        strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
        snprintf(timeBuffer + strlen(timeBuffer), sizeof(timeBuffer) - strlen(timeBuffer), ".%03ld", msecs);
        fprintf(logHandle, "%s [%s] : %s", timeBuffer, getLogLevelString(level), pTempChar);
        fclose(logHandle);
    }
    pthread_mutex_unlock(&loggerMutex);

    free(pTempChar);
}
