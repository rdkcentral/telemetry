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

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>
#include <sys/select.h>
#include <sys/inotify.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "dca.h"
#include "dcautil.h"
#include "telemetry2_0.h"
#include "t2log_wrapper.h"
#include "t2common.h"
#include "legacyutils.h"
#include "persistence.h"

/**
 * @brief Wait for the backup_logs completion sentinel before grepping PreviousLogs.
 *
 * Strategy:
 * 1. Fast path: sentinel already present → return true immediately.
 * 2. Set up inotify on BACKUP_LOGS_DONE_DIR (/tmp) for IN_CREATE | IN_MOVED_TO.
 * 3. Re-check after watch is established to close the creation race window.
 * 4. select() loop with 2 s heartbeat; exit when sentinel appears or
 *    BACKUP_LOGS_SYNC_TIMEOUT_S total seconds have elapsed.
 *
 * This is a **soft gate**: if the function returns false (timeout or inotify
 * failure) the caller still proceeds — previous-log grep runs against whatever
 * files are present, and a warning is logged.
 *
 * @return true  sentinel detected within timeout
 * @return false timeout elapsed or inotify initialisation failed
 */
bool waitForBackupLogsDone(void)
{
    /* Fast path: sentinel already written by backup_logs */
    if (access(BACKUP_LOGS_DONE_FLAG, F_OK) == 0)
    {
        T2Info("backup_logs sentinel %s already present\n", BACKUP_LOGS_DONE_FLAG);
        return true;
    }

    int ifd = inotify_init1(IN_CLOEXEC);
    if (ifd < 0)
    {
        T2Error("inotify_init1 failed (errno=%d); proceeding without backup_logs sync\n", errno);
        return false;
    }

    int wd = inotify_add_watch(ifd, BACKUP_LOGS_DONE_DIR, IN_CREATE | IN_MOVED_TO);
    if (wd < 0)
    {
        T2Error("inotify_add_watch on %s failed (errno=%d); proceeding without backup_logs sync\n",
                BACKUP_LOGS_DONE_DIR, errno);
        close(ifd);
        return false;
    }

    /* Re-check after watch is set — closes the race window between the first
     * access() call and establishing the watch. */
    if (access(BACKUP_LOGS_DONE_FLAG, F_OK) == 0)
    {
        T2Info("backup_logs sentinel detected (race resolved): %s\n", BACKUP_LOGS_DONE_FLAG);
        inotify_rm_watch(ifd, wd);
        close(ifd);
        return true;
    }

    struct timespec deadline;
    if (clock_gettime(CLOCK_MONOTONIC, &deadline) != 0)
    {
        T2Error("clock_gettime failed (errno=%d); proceeding without backup_logs sync\n", errno);
        inotify_rm_watch(ifd, wd);
        close(ifd);
        return false;
    }
    deadline.tv_sec += BACKUP_LOGS_SYNC_TIMEOUT_S;

    T2Info("Waiting up to %ds for backup_logs sentinel: %s\n",
           BACKUP_LOGS_SYNC_TIMEOUT_S, BACKUP_LOGS_DONE_FLAG);

    bool found = false;
    char buf[sizeof(struct inotify_event) + NAME_MAX + 1];

    while (!found)
    {
        /* Check deadline */
        struct timespec now;
        if (clock_gettime(CLOCK_MONOTONIC, &now) != 0)
        {
            T2Error("clock_gettime failed (errno=%d) while waiting for backup_logs sentinel; proceeding without sync\n", errno);
            break;
        }
        if (now.tv_sec >= deadline.tv_sec)
        {
            T2Warning("backup_logs sentinel not present after %ds; proceeding without sync\n",
                      BACKUP_LOGS_SYNC_TIMEOUT_S);
            break;
        }

        struct timeval tv = {2, 0};
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(ifd, &fds);

        int ret = select(ifd + 1, &fds, NULL, NULL, &tv);
        if (ret < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            T2Error("select() failed (errno=%d) waiting for backup_logs sentinel\n", errno);
            break;
        }
        if (ret == 0)
        {
            continue;   /* 2 s heartbeat — loop back to check deadline */
        }

        ssize_t len = read(ifd, buf, sizeof(buf));
        if (len <= 0)
        {
            continue;
        }

        ssize_t offset = 0;
        while (offset < len)
        {
            struct inotify_event *ev = (struct inotify_event *)(buf + offset);
            if (ev->len > 0 && strcmp(ev->name, BACKUP_LOGS_DONE_FILENAME) == 0)
            {
                T2Info("backup_logs sentinel created: %s\n", BACKUP_LOGS_DONE_FLAG);
                found = true;
                break;
            }
            offset += (ssize_t)(sizeof(struct inotify_event) + ev->len);
        }
    }

    inotify_rm_watch(ifd, wd);
    close(ifd);
    return found;
}

/**
 * @brief Get the Grep Results object. Main function called by rest of the consumers.
 *
 * @param profileName
 * @param markerList
 * @param grepResultList
 * @param isClearSeekMap
 * @param check_rotated
 * @param customLogPath
 * @return T2ERROR
 */

T2ERROR
getGrepResults (GrepSeekProfile **GSP, Vector *markerList, bool isClearSeekMap, bool check_rotated, char *customLogPath)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if(GSP == NULL || markerList == NULL )
    {
        T2Error("Invalid Args or Args are NULL\n");
        return T2ERROR_FAILURE;
    }

    /* Synchronize with backup_logs before grepping the previous-log directory.
     * backup_logs populates PREVIOUS_LOGS_PATH; reading it before the write is
     * complete yields incomplete or empty results.  This is a soft gate — if the
     * sentinel does not appear within BACKUP_LOGS_SYNC_TIMEOUT_S seconds we
     * still proceed so telemetry is never permanently blocked. */
    if (customLogPath != NULL && strcmp(customLogPath, PREVIOUS_LOGS_PATH) == 0)
    {
        if (!waitForBackupLogsDone())
        {
            T2Warning("%s: backup_logs sentinel absent; grepping %s with potentially incomplete data\n",
                      __FUNCTION__, PREVIOUS_LOGS_PATH);
        }
    }

    getDCAResultsInVector(*GSP, markerList, check_rotated, customLogPath);
    if (isClearSeekMap)
    {
        int count = (*GSP)->execCounter;
        freeGrepSeekProfile(*GSP);
        *GSP = createGrepSeekProfile(count);
    }
     /* Signal that telemetry previous-log grep is complete.
     * Downstream consumers (e.g. uploadSTBLogs) wait for this sentinel
     * before starting the log upload to avoid incomplete telemetry data. */
    if (customLogPath != NULL && strcmp(customLogPath, PREVIOUS_LOGS_PATH) == 0)
    {
        int fd = open(TELEMETRY_PREVLOGS_DONE_FLAG, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (fd >= 0)
        {
            close(fd);
            T2Info("Created telemetry previous-logs sentinel: %s\n", TELEMETRY_PREVLOGS_DONE_FLAG);
        }
        else
        {
            T2Error("%s: Failed to create sentinel %s (errno=%d)\n",
                    __FUNCTION__, TELEMETRY_PREVLOGS_DONE_FLAG, errno);
        }
    }
    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

// dcaFlagReportCompleation this function is used to create legacy DCA Flag DCADONEFLAG
void dcaFlagReportCompleation()
{
    T2Debug("%s --in creating flag %s\n", __FUNCTION__, DCADONEFLAG);
    FILE *fileCheck = fopen(DCADONEFLAG, "w+");
    if (fileCheck == NULL )
    {
        T2Error(" Error in creating the Flag :  %s\n", DCADONEFLAG);
    }
    else
    {
        fclose(fileCheck);
    }
    T2Debug("%s --out\n", __FUNCTION__);
}

# ifdef PERSIST_LOG_MON_REF
T2ERROR saveSeekConfigtoFile(char* profileName, GrepSeekProfile *ProfileSeekMap)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if(profileName == NULL)
    {
        T2Error("Profile Name is not available\n");
        return T2ERROR_FAILURE;
    }
    if(ProfileSeekMap == NULL)
    {
        T2Error("ProfileSeekMap is NULL\n");
        return T2ERROR_FAILURE;
    }
    hash_map_t *logfileMap = ProfileSeekMap->logFileSeekMap;
    if(logfileMap == NULL)
    {
        T2Error("logfileMap is NULL\n");
        return T2ERROR_FAILURE;
    }

    unsigned int count = (unsigned int) hash_map_count(logfileMap);

    cJSON *valArray = cJSON_CreateArray();
    for (unsigned int i = 0; i < count ; i++)
    {
        char *logFileName = NULL;
        long *seekvalue = NULL;
        logFileName = hash_map_lookupKey(logfileMap, i);
        seekvalue = hash_map_lookup(logfileMap, i);
        cJSON *logFileObj = cJSON_CreateObject();
        cJSON_AddNumberToObject(logFileObj, logFileName, (double)*seekvalue);
        cJSON_AddItemToArray(valArray, logFileObj);
    }
    char *jsonReport = cJSON_PrintUnformatted(valArray);
    if(T2ERROR_SUCCESS != saveConfigToFile(SEEKFOLDER, profileName, jsonReport))
    {
        T2Error("Failed to save config to file\n");
        cJSON_Delete(valArray);
        free(jsonReport);
        return T2ERROR_FAILURE;
    }
    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

T2ERROR loadSavedSeekConfig(char *profileName, GrepSeekProfile *ProfileSeekMap)
{
    T2Debug("%s ++in\n", __FUNCTION__);

    if(profileName == NULL)
    {
        T2Error("Profile Name is not available\n");
        return T2ERROR_FAILURE;
    }
    int len = strlen(profileName) + strlen(SEEKFOLDER) + 2;
    char *seekFile = (char *)malloc(len);
    snprintf(seekFile, len, "%s/%s", SEEKFOLDER, profileName);
    FILE *file = fopen(seekFile, "rb");
    if(file == NULL)
    {
        T2Error("Failed to open file\n");
        free(seekFile);
        return T2ERROR_FAILURE;
    }
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *data = malloc(fileSize + 1);
    if (data == NULL)
    {
        T2Error("Memory allocation failed\n");
        fclose(file);
        return T2ERROR_FAILURE;
    }
    fread(data, 1, fileSize, file);
    fclose(file);
    data[fileSize] = '\0';
    cJSON *json = cJSON_Parse(data);
    cJSON *item = NULL;
    //GrepSeekProfile *ProfileSeekMap = NULL;
    cJSON_ArrayForEach(item, json)
    {
        // Each `item` is an object in the array
        if (item->child != NULL)
        {
            const char *key = item->child->string;
            cJSON *value = item->child;

            if (key != NULL)
            {
                // Check the value type and print it
                if (cJSON_IsNumber(value))
                {
                    long *tempnum;
                    double val = value->valuedouble;
                    tempnum = (long *)malloc(sizeof(long));
                    *tempnum = (long)val;
                    hash_map_put(ProfileSeekMap->logFileSeekMap, strdup(key), tempnum, NULL);
                    //printf("Key: %s, Value: %ld\n", key, *tempnum);
                }
            }

        }
    }
    cJSON_Delete(json);
    free(data);
    free(seekFile);
    return T2ERROR_SUCCESS;
    T2Debug("%s --out\n", __FUNCTION__);
}
#endif

bool firstBootStatus()
{
    T2Debug("%s ++in\n", __FUNCTION__);
    bool status = true;
    if(access(BOOTFLAG, F_OK) != -1)
    {
        status = false;
    }
    T2Debug("%s --out\n", __FUNCTION__);
    return status;
}
