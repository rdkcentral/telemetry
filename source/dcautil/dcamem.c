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

//cpu and free memory
#include <stdio.h>
#include "t2log_wrapper.h"

#define MAXLEN 512

int getMemoryUsage(char *memoryUtilization);

/**
 * @addtogroup DCA_APIS
 * @{
 */

int main( ) {

    char memoryUtilization[MAXLEN] = { '\0' };

    if(getMemoryUsage(memoryUtilization)) {
        printf("USED_MEM:%s\n", memoryUtilization);
    }else {
        printf("USED_MEM:NOT AVAILABLE\n");
    }
    return 0;
}

/**
 * @brief To get memory usage of the device.
 *
 * @param[out] memoryUtilization    Memory usage of the device.
 *
 * @return  Returns status of operation.
 * @retval  Return 1 on success.
 */
int getMemoryUsage(char *memoryUtilization) {
    FILE *memoryinfo;
    char line[MAXLEN];
    char tmp[MAXLEN];
    long long memTotal = 0;
    long long memFree = 0;
    long long memoryInUse = 0;
    int Total_flag = 0;
    int Free_flag = 0;

    /* Open /proc/cpuinfo file*/
    if((memoryinfo = fopen("/proc/meminfo", "r")) == NULL) {
        printf("Failed to get Memory Utilization mode\n");
        return 0;
    }

    /* Search until the "MemTotal" entry is found*/
    while(fgets(line, MAXLEN, memoryinfo)) {
        sscanf(line, "%s", tmp);
        if((strcmp(tmp, "MemTotal:") == 0)) {
            sscanf(line, "%*s %s", tmp);
            memTotal = atoll(tmp);
            Total_flag = 1;
        }else if((strcmp(tmp, "MemFree:") == 0)) {
            sscanf(line, "%*s %s", tmp);
            memFree = atoll(tmp);
            Free_flag = 1;
        }
        if(Total_flag == 1 && Free_flag == 1) {
            break;
        }
    }

    fclose(memoryinfo);
    memoryInUse = (memTotal - memFree);
    if(memoryUtilization) {
        sprintf(memoryUtilization, "%lld", memoryInUse);
    }else {
        printf("Exit from get Memory Utilization due to NULL pointer");
        return 0;
    }
    return 1;
}

/** @} */  //END OF GROUP DCA_APIS
/** @} */

/** @} */
/** @} */
