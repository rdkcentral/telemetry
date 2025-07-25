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

#ifndef _DCAUTIL_H_
#define _DCAUTIL_H_

#include <stdbool.h>
#include "telemetry2_0.h"
#include "vector.h"
#include "dca.h"
#include <dirent.h>
#include <ctype.h>
#include "legacyutils.h"

#define TOPTEMP "/tmp/t2toplog"
#define DCADONEFLAG "/tmp/.dca_done"

#define PREVIOUS_LOG "PREVIOUS_LOG"
#define PREVIOUS_LOGS_VAL  "1"
#define PREVIOUS_LOGS_PATH "/opt/logs/PreviousLogs/"

typedef struct _GrepResult
{
    const char* markerName;
    const char* markerValue;
    bool trimParameter;
    char* regexParameter;
} GrepResult;

#if !defined(ENABLE_RDKC_SUPPORT) && !defined(ENABLE_RDKB_SUPPORT)
char* saveTopOutput(char* profilename);
void removeTopOutput(char* filename);
#endif

void removeGrepConfig(char* profileName, bool clearSeek, bool clearExec);
void freeGResult(void *data);
T2ERROR saveGrepConfig(char *name, Vector* grepMarkerList);
T2ERROR getGrepResults(GrepSeekProfile **GSP, Vector *markerList, Vector **grepResultList, bool isClearSeekMap, bool check_rotated, char *customLogPath);
#define PREFIX_SIZE 5
#define BUF_LEN 16

typedef struct proc_info
{
    int utime; /**< User mode jiffies */
    int stime; /**< Kernel mode jiffies */
    int cutime; /**< User mode jiffies with childs */
    int cstime; /**< Kernel mode jiffies with childs */
    unsigned int rss; /**< Resident Set Size */
} procinfo;

typedef struct _procMemCpuInfo
{
    pid_t *pid;
    char processName[BUF_LEN];
    char cpuUse[BUF_LEN];
    char memUse[BUF_LEN];
    int total_instance;
} procMemCpuInfo;

/* @} */ // End of group DCA_TYPES
int getProcInfo(procMemCpuInfo *pInfo, char* filename);
int getMemInfo(procMemCpuInfo *pmInfo);
int getCPUInfo(procMemCpuInfo *pInfo, char* filename);
int getProcPidStat(int pid, procinfo * pinfo);
int getTotalCpuTimes(int * totalTime);


#ifdef PERSIST_LOG_MON_REF
typedef void (*freeconfigdata)(void *data);
T2ERROR saveSeekConfigtoFile(char* profileName, GrepSeekProfile *ProfileSeekMap);
T2ERROR loadSavedSeekConfig(char *profileName, GrepSeekProfile *ProfileSeekMap);
bool firstBootStatus();
#endif

void dcaFlagReportCompleation();
#endif /* _DCAUTIL_H_ */
