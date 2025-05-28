/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2016 RDK Management
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
 * @defgroup dca
 * @{
 **/

/**
 * @defgroup dca
 * @{
 * @defgroup src
 * @{
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>           /* Definition of AT_* constants */
#include <unistd.h>

#define LEN 14
#define CMD_LEN 256
#define MAXLEN 512
#define PID_SIZE 10
#define PIDOF_SIZE 50
#define MEM_STRING_SIZE 20
#define PROC_PATH_SIZE 50
#define TOPITERATION 3

#include "vector.h"
#include "dcautil.h"

#include "legacyutils.h"
#include "t2log_wrapper.h"
#ifdef LIBSYSWRAPPER_BUILD
#include "secure_wrapper.h"
#endif

#ifdef LIBSYSWRAPPER_BUILD
#include "secure_wrapper.h"
#endif

/**
 * @addtogroup DCA_TYPES
 * @{
 */

/**
 * @addtogroup DCA_APIS
 * @{
 */



ProcessSnapshot* createProcessSnapshot() {
    DIR *procDir = opendir("/proc");
    if (!procDir) {
        T2Error("Failed to open /proc directory\n");
        return NULL;
    }

    ProcessSnapshot *snapshot = malloc(sizeof(ProcessSnapshot));
    if (!snapshot) {
        closedir(procDir);
        T2Error("Failed to allocate memory for snapshot\n");
        return NULL;
    }

    snapshot->processList = Vector_Create();
    struct dirent *entry;

    while ((entry = readdir(procDir)) != NULL) {
        if (!isdigit(entry->d_name[0])) {
            continue; // Skip non-numeric directories
        }

        pid_t pid = atoi(entry->d_name);
        char cmdPath[256];
        snprintf(cmdPath, sizeof(cmdPath), "/proc/%s/comm", entry->d_name);

        FILE *cmdFile = fopen(cmdPath, "r");
        if (!cmdFile) {
            continue; 
        }

        char processName[256];
        if (fgets(processName, sizeof(processName), cmdFile)) {
            processName[strcspn(processName, "\n")] = '\0'; // Remove newline

            ProcessInfo *info = malloc(sizeof(ProcessInfo));
            if (!info) {
                fclose(cmdFile);
                continue;
            }

            info->pid = pid;
            strncpy(info->processName, processName, sizeof(info->processName) - 1);

            // Optionally, collect memory and CPU usage
            // TODO: Implement memory and CPU usage collection
            snprintf(info->memUsage, sizeof(info->memUsage), "N/A"); 
            snprintf(info->cpuUsage, sizeof(info->cpuUsage), "N/A"); 

            Vector_PushBack(snapshot->processList, info);
        }
        fclose(cmdFile);
    }

    closedir(procDir);
    return snapshot;
}

void freeProcessSnapshot(ProcessSnapshot *snapshot) {
    if (!snapshot) {
        return;
    }

    for (size_t i = 0; i < Vector_Size(snapshot->processList); i++) {
        free(Vector_At(snapshot->processList, i));
    }
    Vector_Destroy(snapshot->processList);
    free(snapshot);
}

ProcessInfo* lookupProcess(ProcessSnapshot *snapshot, const char *processName) {
    if (!snapshot || !processName) {
        return NULL;
    }

    for (size_t i = 0; i < Vector_Size(snapshot->processList); i++) {
        ProcessInfo *info = Vector_At(snapshot->processList, i);
        if (strcmp(info->processName, processName) == 0) {
            return info; // Return the matching process info
        }
    }
    return NULL; // Process not found
}

/**
 * @brief To get process usage.
 *
 * @param[in] processName   Process name.
 *
 * @return  Returns status of operation.
 * @retval  0 on sucess, appropiate errorcode otherwise.
 */

int getProcUsage(char *processName, Vector* grepResultList, bool trim, char* regex)
{
    T2Debug("%s ++in \n", __FUNCTION__);
    if(grepResultList == NULL || processName == NULL)
    {
        T2Error("Invalid arguments for getProcUsage\n");
        return -1;
    }
    if(processName != NULL)
    {
        T2Debug("Process name is %s \n", processName);
        procMemCpuInfo pInfo;
        char pidofCommand[PIDOF_SIZE];
#if defined (ENABLE_PS_PROCESS_SEARCH)
        char psCommand[CMD_LEN];
#endif
        FILE *cmdPid;
        char *mem_key = NULL, *cpu_key = NULL;
        int ret = 0, pclose_ret = 0;
        int index = 0;
        pid_t *pid = NULL;
        pid_t *temp = NULL;
        int pname_prefix_len = strlen(processName) + PREFIX_SIZE + 1;
        memset(&pInfo, '\0', sizeof(procMemCpuInfo));
        memcpy(pInfo.processName, processName, strlen(processName) + 1);

        T2Debug("Command for collecting process info : \n pidof %s", processName);
        snprintf(pidofCommand, sizeof(pidofCommand), "pidof %s", processName);
        cmdPid = popen(pidofCommand, "r");
        if(!cmdPid)
        {
            T2Debug("Failed to execute %s", pidofCommand);
            return 0;
        }
        pid = (int *) malloc(sizeof(pid_t));
        if(NULL == pid)
        {
            pclose_ret = pclose(cmdPid);
            if(pclose_ret != 0)
            {
                T2Debug("failed in closing pipe! ret %d\n", pclose_ret);
            }
            return 0;
        }
        *pid = 0;
        while(fscanf(cmdPid, "%d", (pid + index)) == 1)
        {
            if((*(pid + index)) <= 0)
            {
                continue;
            }
            index++;
            temp = (pid_t *) realloc(pid, ((index + 1) * sizeof(pid_t)));
            if(NULL == temp)
            {
                if(pid)
                {
                    free(pid);
                }
                pclose_ret = pclose(cmdPid);
                if(pclose_ret != 0)
                {
                    T2Debug("failed in closing pipe! ret %d\n", pclose_ret);
                }
                return 0;
            }
            pid = temp;
        }


        pclose_ret = pclose(cmdPid);
        if(pclose_ret != 0)
        {
            T2Debug("failed in closing pipe! ret %d\n", pclose_ret);
        }

#if defined (ENABLE_PS_PROCESS_SEARCH)
        // Pidof command output is empty
        if ((*pid) <= 0)
        {
            // pidof was empty, see if we can grab the pid via ps
            sprintf(psCommand, "busybox ps | grep %s | grep -v grep | awk '{ print $1 }' | tail -n1", processName);

            if (!(cmdPid = popen(psCommand, "r")))
            {
                free(pid);//CID 172839:Resource leak (RESOURCE_LEAK)
                return 0;
            }

            *pid = 0;
            index = 0;
            while(fscanf(cmdPid, "%d", (pid + index)) == 1)
            {
                if ((*(pid + index)) <= 0)
                {
                    continue;
                }
                index++;
                temp = (pid_t *) realloc (pid, ((index + 1) * sizeof(pid_t)) );
                if ( NULL == temp )
                {
                    free(pid);
                    pclose(cmdPid);
                    return 0;
                }
                pid = temp;
            }

            pclose(cmdPid);

            // If pidof command output is empty
            if ((*pid) <= 0)
            {
                free(pid);
                return 0;
            }
        }
#else
        // If pidof command output is empty
        if ((*pid) <= 0)
        {
            free(pid);
            return 0;
        }
#endif

        pInfo.total_instance = index;
        pInfo.pid = pid;
        if(0 != getProcInfo(&pInfo))
        {
            mem_key = malloc(pname_prefix_len);
            cpu_key = malloc(pname_prefix_len);
            if(NULL != mem_key && NULL != cpu_key)
            {
                snprintf(cpu_key, pname_prefix_len, "cpu_%s", processName);
                snprintf(mem_key, pname_prefix_len, "mem_%s", processName);

                T2Debug("Add to search result %s , value = %s , %s \n", cpu_key,  pInfo.cpuUse, pInfo.memUse);
                GrepResult* cpuInfo = (GrepResult*) malloc(sizeof(GrepResult));
                if(cpuInfo)
                {
                    cpuInfo->markerName = strdup(cpu_key);
                    cpuInfo->markerValue = strdup(pInfo.cpuUse);
                    cpuInfo->trimParameter = trim;
                    cpuInfo->regexParameter = regex;
                    Vector_PushBack(grepResultList, cpuInfo);
                }

                GrepResult* memInfo = (GrepResult*) malloc(sizeof(GrepResult));
                if(memInfo)
                {
                    memInfo->markerName = strdup(mem_key);
                    memInfo->markerValue = strdup(pInfo.memUse);
                    memInfo->trimParameter = trim;
                    memInfo->regexParameter = regex;
                    Vector_PushBack(grepResultList, memInfo);
                }

                ret = 1;
            }

            if(mem_key)
            {
                free(mem_key);
            }

            if(cpu_key)
            {
                free(cpu_key);
            }

            if(pid)
            {
                free(pid);
            }

            return ret;
        }
        if(pid)
        {
            free(pid);
        }
    }
    T2Debug("%s --out \n", __FUNCTION__);
    return 0;
}

/**
 * @brief To get status of a process from its process ID.
 *
 * This will return information such as process priority, virtual memory size, signals etc.
 *
 * @param[in] pid      PID value of the  process.
 * @param[in] pinfo    Process info.
 *
 * @return  Returns status of operation.
 * @retval  Return 1 on success, appropiate errorcode otherwise.
 */
int getProcPidStat(int pid, procinfo * pinfo)
{
    T2Debug("%s ++in \n", __FUNCTION__);
    char szFileName[CMD_LEN], szStatStr[2048], *s, *t;
    int ppid, fd, pgrp, session, tty, tpgid, counter, priority, starttime, signal, blocked, sigignore, sigcatch, j;
    char exName[CMD_LEN], state;
    //unsigned euid, egid;
    unsigned int flags, minflt, cminflt, majflt, cmajflt, timeout, itrealvalue, vsize, rlim, startcode, endcode, startstack, kstkesp, kstkeip, wchan;

    if(NULL == pinfo)
    {
        T2Debug("Invalid input(pinfo=NULL) to get process info");
        return 0;
    }

    sprintf(szFileName, "/proc/%u/stat", (unsigned) pid);
    if((fd = open(szFileName, O_RDONLY)) == -1)
    {
        T2Debug("Failed to open file in get process info");
        return 0;
    }

    /* if(-1 != fstat(fd, &st)) {
        euid = st.st_uid;
        egid = st.st_gid;
    }else {
        euid = egid = -1;
    } */


    if((j = read(fd, szStatStr, 2047)) == -1)
    {
        close(fd);
        return 0;
    }
    szStatStr[j++] = '\0';
    /** pid **/
    s = strchr(szStatStr, '(') + 1;
    t = strchr(szStatStr, ')');
    strncpy(exName, s, t - s);
    exName[t - s] = '\0';

    sscanf(t + 2, "%c %d %d %d %d %d %u %u %u %u %u %d %d %d %d %d %d %u %u %d %u %u %u %u %u %u %u %u %d %d %d %d %u",
           /*       1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33*/
           & (state), &(ppid), &(pgrp), &(session), &(tty), &(tpgid), &(flags), &(minflt), &(cminflt), &(majflt), &(cmajflt), &(pinfo->utime),
           &(pinfo->stime), &(pinfo->cutime), &(pinfo->cstime), &(counter), &(priority), &(timeout), &(itrealvalue), &(starttime), &(vsize),
           &(pinfo->rss), &(rlim), &(startcode), &(endcode), &(startstack), &(kstkesp), &(kstkeip), &(signal), &(blocked), &(sigignore), &(sigcatch),
           &(wchan));

    close(fd);

    T2Debug("%s --out \n", __FUNCTION__);

    return 1;
}
/**
 * @brief To get CPU and mem info.
 *
 * @param[out] pmInfo  Memory/CPU Info.
 *
 * @return  Returns status of operation.
 * @retval  Return 1 on success.
 */
int getProcInfo(procMemCpuInfo *pmInfo)
{
    T2Debug("%s ++in \n", __FUNCTION__);
    if(0 == getMemInfo(pmInfo))
    {
        return 0;
    }

    if(0 == getCPUInfo(pmInfo))
    {
        return 0;
    }

    return 1;
}

/**
 * @brief To get the reserve memory of a given process.
 *
 * @param[out] pmInfo  Memory  Info.
 *
 * @return  Returns status of operation.
 * @retval  Return 1 on success.
 */
int getMemInfo(procMemCpuInfo *pmInfo)
{
    T2Debug("%s ++in \n", __FUNCTION__);
    static char retMem[MEM_STRING_SIZE];
    int intStr = 0, intValue = 0;
    double residentMemory = 0.0;
    procinfo pinfo;
    long pageSizeInKb = sysconf(_SC_PAGE_SIZE) / 1024; /* x86-64 is configured to use 2MB pages */
    unsigned int total_memory = 0;
    int index = 0;
    if(pmInfo == NULL)
    {
        T2Error("Invalid arguments or NULL arguments\n");
        return 0;
    }
    for( index = 0; index < (pmInfo->total_instance); index++ )
    {
        memset(&pinfo, 0, sizeof(procinfo));
        if(0 == getProcPidStat(pmInfo->pid[index], &pinfo))
        {
            return 0;
        }
        total_memory += pinfo.rss;
    }

    residentMemory = total_memory * pageSizeInKb;
    intStr = (int) residentMemory;
    intValue = intStr;
    if(intValue >= 1024)
    {
        intStr = intStr / 1024;
    }
    snprintf(retMem, sizeof(retMem), "%d%c", intStr, (intValue >= 1024) ? 'm' : 'k');

    strncpy(pmInfo->memUse, retMem, sizeof(pmInfo->memUse) - 1);
    pmInfo->memUse[sizeof(pmInfo->memUse) - 1] = '\0';
    T2Debug("%s --out \n", __FUNCTION__);
    return 1;
}


#if !defined(ENABLE_RDKC_SUPPORT) && !defined(ENABLE_RDKB_SUPPORT)

void saveTopOutput()
{
    T2Debug("%s ++in \n", __FUNCTION__);
    if(access(TOPTEMP, F_OK) == 0)
    {
        T2Debug("%s --out \n", __FUNCTION__);
        return;
    }
    int ret = 0;
    char command[CMD_LEN] = { '\0' };
    int cmd_option = 0;
    /* Check Whether -c option is supported */
    sprintf(command, "top -b -n 1 -c > %s ", TOPTEMP);
#ifdef LIBSYSWRAPPER_BUILD
    ret = v_secure_system(command);
#else
    ret = system(command);
#endif
    // clear command
    memset(command, '\0', sizeof(command));
    if(0 == ret)
    {
        cmd_option = 1;
    }
    T2Debug("top cmd -c arg status %d, ret value %d \n", cmd_option, ret);
#ifdef INTEL
    /* Format Use:  `top n 1 | grep Receiver` */
    if ( 1 == cmd_option )
    {
        sprintf(command, "COLUMNS=512 top -n %d -c > %s ", TOPITERATION, TOPTEMP);
    }
    else
    {
        sprintf(command, "top -n %d > %s", TOPITERATION, TOPTEMP);
    }
#else
    /* ps -C Receiver -o %cpu -o %mem */
    //sprintf(command, "ps -C '%s' -o %%cpu -o %%mem | sed 1d", pInfo->processName);
    snprintf(command, CMD_LEN, "%s top -b -n %d %s > %s", (cmd_option == 1) ? "COLUMNS=512" : "", TOPITERATION, (cmd_option == 1) ? "-c" : "", TOPTEMP);

#endif

#ifdef LIBSYSWRAPPER_BUILD
    ret = v_secure_system(command);
#else
    ret = system(command);
#endif
    if(ret != 0)
    {
        T2Debug("return value of system command to create %s is success with code %d\n", TOPTEMP, ret);
    }
    else
    {
        T2Error("return value of system command to create %s is not successful with code \n", TOPTEMP);
        return;
    }
    T2Debug("%s --out \n", __FUNCTION__);

}

void removeTopOutput()
{
    T2Debug("%s ++in \n", __FUNCTION__);
    int ret = 0;
    char command[256] = {'\0'};
    snprintf(command, sizeof(command), "rm -rf %s", TOPTEMP);
#ifdef LIBSYSWRAPPER_BUILD
    ret = v_secure_system(command);
#else
    ret = system(command);
#endif
    if(ret == 0)
    {
        T2Debug("return value of system command to remove %s is success with code %d \n", TOPTEMP, ret);
    }
    else
    {
        T2Error("return value of system command to remove %s is not successful with code %d \n", TOPTEMP, ret);
    }
    T2Debug("%s --out \n", __FUNCTION__);
}

//#if !defined(ENABLE_RDKC_SUPPORT) && !defined(ENABLE_RDKB_SUPPORT)
/**
 * @brief To get CPU info.
 *
 * @param[out] pInfo  CPU info.
 *
 * @return  Returns status of operation.
 * @retval  Return 1 on success,appropiate errorcode otherwise.
 */
int getCPUInfo(procMemCpuInfo *pInfo)
{
    int ret = 0, pclose_ret = 0;
    FILE *inFp = NULL;
    char command[CMD_LEN] = { '\0' };
    char var1[BUF_LEN] = { '\0' };
    char var2[BUF_LEN] = { '\0' };
    char var3[BUF_LEN] = { '\0' };
    char var4[BUF_LEN] = { '\0' };
    char var5[BUF_LEN] = { '\0' };
    char var6[BUF_LEN] = { '\0' };
    char var7[BUF_LEN] = { '\0' };
    char var8[512] = { '\0' };
    char var9[512] = { '\0' };
    char var10[512] = { '\0' };
    float total_cpu_usage = 0;
    char top_op[2048] = { '\0' };
    int cmd_option = 0;
    int normalize = 1;

    if(pInfo == NULL)
    {

        return 0;
    }
    if(access(TOPTEMP, F_OK) != 0)
    {
        T2Debug("%s ++in the savad temp log %s is not available \n", __FUNCTION__, TOPTEMP);
        /* Check Whether -c option is supported */
#ifdef LIBSYSWRAPPER_BUILD
        ret = v_secure_system(" top -c -n 1 2> /dev/null 1> /dev/null");
#else
        ret = system(" top -c -n 1 2> /dev/null 1> /dev/null");
#endif
        if(0 == ret)
        {
            cmd_option = 1;
        }

#ifdef INTEL
        /* Format Use:  `top n 1 | grep Receiver` */
        if ( 1 == cmd_option )
        {
#ifdef LIBSYSWRAPPER_BUILD
            inFp = v_secure_popen("r", "top -n 1 -c | grep -v grep |grep -i '%s'", pInfo->processName);
#else
            sprintf(command, "top -n 1 -c | grep -v grep |grep -i '%s'", pInfo->processName);
            inFp = popen(command, "r");
#endif
        }
        else
        {
#ifdef LIBSYSWRAPPER_BUILD
            inFp = v_secure_popen("r", "top -n 1 | grep -i '%s'", pInfo->processName);
#else
            sprintf(command, "top -n 1 | grep -i '%s'", pInfo->processName);
            inFp = popen(command, "r");
#endif
        }
#else
        /* ps -C Receiver -o %cpu -o %mem */
        //sprintf(command, "ps -C '%s' -o %%cpu -o %%mem | sed 1d", pInfo->processName);
#ifdef LIBSYSWRAPPER_BUILD
        inFp = v_secure_popen("r", "top -b -n 1 %s | grep -v grep | grep -i '%s'", (cmd_option == 1) ? "-c" : "", pInfo->processName);
#else
        snprintf(command, CMD_LEN, "top -b -n 1 %s | grep -v grep | grep -i '%s'", (cmd_option == 1) ? "-c" : "", pInfo->processName);
        inFp = popen(command, "r");
#endif

#endif
    }
    else
    {
        T2Debug("%s ++in the savad temp log %s is available \n", __FUNCTION__, TOPTEMP);
#ifdef LIBSYSWRAPPER_BUILD
        inFp = v_secure_popen("r", "cat %s |grep -i '%s'", TOPTEMP, pInfo->processName);
#else
        sprintf(command, "cat %s |grep -i '%s'", TOPTEMP, pInfo->processName);
        inFp = popen(command, "r");
#endif
        normalize = TOPITERATION;

    }

    if(!(inFp))
    {
        T2Debug("failed in open v_scure_popen pipe! ret %d\n", pclose_ret);
        return 0;
    }

    //  2268 root      20   0  831m  66m  20m S   27 13.1 491:06.82 Receiver
#ifdef INTEL
    while(fgets(top_op, 2048, inFp) != NULL)
    {
        if(sscanf(top_op, "%s %s %s %s %s %s %s %s", var1, var2, var3, var4, var5, var6, var7, var8) == 8)
        {
            total_cpu_usage += atof(var7);
            ret = 1;
        }
    }
    //#endif
#else
    while(fgets(top_op, 2048, inFp) != NULL)
    {
        if(sscanf(top_op, "%16s %16s %16s %16s %16s %16s %16s %512s %512s %512s", var1, var2, var3, var4, var5, var6, var7, var8, var9, var10) == 10)
        {
            total_cpu_usage += atof(var9);
            ret = 1;
        }
    }
#endif

    snprintf(pInfo->cpuUse, sizeof(pInfo->cpuUse), "%.1lf", (float)(total_cpu_usage / normalize));
    T2Debug("calculated CPU total value : %f Normalized value : %.1lf\n", total_cpu_usage, (float)(total_cpu_usage / normalize));
#ifdef LIBSYSWRAPPER_BUILD
    pclose_ret = v_secure_pclose(inFp);
#else
    pclose_ret = pclose(inFp);
#endif

    if(pclose_ret != 0)
    {
        T2Debug("failed in closing pipe! ret %d\n", pclose_ret);
    }
    return ret;
    T2Debug("--out %s", __FUNCTION__);

}

#else //ENABLE_RDKC_SUPPORT & ENABLE_RDKB_SUPPORT

/**
 * @brief To get total CPU time of the device.
 *
 * @param[out] totalTime   Total time of device.
 *
 * @return  Returns status of operation.
 * @retval  Return 1 on success, appropiate errorcode otherwise.
 */
int getTotalCpuTimes(int * totalTime)
{
    FILE *fp;
    long double a[10];
    int total, ret = 0;

    fp = fopen("/proc/stat", "r");

    if(!fp)
    {
        return 0;
    }

    ret = fscanf(fp, "%*s %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf",
                 &a[0], &a[1], &a[2], &a[3], &a[4], &a[5], &a[6], &a[7], &a[8], &a[9]);
    if(ret == -1)
    {
        T2Debug("%s --read error from /proc/stat \n", __FUNCTION__);
    }

    fclose(fp);
    total = (a[0] + a[1] + a[2] + a[3] + a[4] + a[5] + a[6] + a[7] + a[8] + a[9]);
    *totalTime = total;

    return 1;
}

/**
 * @brief To get process CPU utilization of the process.
 *
 * @param[in]  pid            Process id.
 * @param[out] procCpuUtil    CPU utilization of process.
 *
 * @return  Returns status of operation.
 * @retval  Return 1 on success, appropiate errorcode otherwise.
 */
int getProcessCpuUtilization(int pid, float *procCpuUtil)
{
    procinfo pinfo1;
    float total_time_process[2], time[2];
    int t[2];
    float sub1;
    float time1;
    float util = 0;

    if( 0 == getProcPidStat(pid, &pinfo1))
    {
        return 0;
    }

    total_time_process[0] = pinfo1.utime +
                            pinfo1.stime +
                            pinfo1.cutime +
                            pinfo1.cstime;
    //start=pinfo1.starttime;

    if( !getTotalCpuTimes(&t[0]) )
    {
        return 0;
    }

    time[0] = t[0];
    sleep(2);

    if( 0 == getProcPidStat(pid, &pinfo1))
    {
        return 0;
    }

    total_time_process[1] = pinfo1.utime +
                            pinfo1.stime +
                            pinfo1.cutime +
                            pinfo1.cstime;

    if( 0 == getTotalCpuTimes(&t[1]) )
    {
        return 0;
    }

    time[1] = t[1];
    sub1 = total_time_process[1] - total_time_process[0];
    time1 = time[1] - time[0];
    util = (sub1 / time1) * 100;

    if(procCpuUtil)
    {
        *procCpuUtil = util;
    }
    else
    {
        return 0;
    }

    return 1;
}

int getCPUInfo(procMemCpuInfo *pmInfo)
{
    float cpu = 0;
    float total_cpu = 0;
    int index = 0;
    if(pmInfo == NULL)
    {
        T2Info("Invalid or NULL arguments\n");
        return 0;
    }
    for(index = 0; index < (pmInfo->total_instance); index++)
    {
        if (0 == getProcessCpuUtilization(pmInfo->pid[index], &cpu))
        {
            continue;
        }
        total_cpu += cpu;
    }

    snprintf(pmInfo->cpuUse, sizeof(pmInfo->cpuUse), "%.1f", (float)total_cpu);
    return 1;
}

#endif //ENABLE_RDKC_SUPPORT & ENABLE_RDKB_SUPPORT

/** @} */  //END OF GROUP DCA_APIS
/** @} */

/** @} */
/** @} */
