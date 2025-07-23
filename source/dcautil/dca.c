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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <limits.h>

#include <cjson/cJSON.h>

#include "dcautil.h"
#include "dca.h"
#include "legacyutils.h"
#include "rdk_linkedlist.h"

#define TR181BUF_LENGTH 512
#define OBJ_DELIMITER "{i}"
#define DELIMITER_SIZE 3

#include "t2log_wrapper.h"
#include "t2common.h"
#include "busInterface.h"


static bool check_rotated_logs = false; // using this variable to indicate whether it needs to check the rotated logs or not . Initialising it with false.
static bool firstreport_after_bootup = false; // the rotated logs check should run only for the first time.

// Using same error code used by safeclib for buffer overflow for easy metrics collection
// safeclib is not completely introduced in T2 but to be in sync with legacy
#define INVALID_COUNT -406

#define BUFFER_SIZE 4096  // TODO fine tune this value based on the size of the data    
#define LARGE_FILE_THRESHOLD 1000000 // 1MB

/**
 * @addtogroup DCA_TYPES
 * @{
 */

// Define a struct to hold the file descriptor and size
typedef struct
{
    int fd;
    off_t cf_file_size;
    off_t rf_file_size;
    char* cfaddr;
    char* rfaddr;
    void* baseAddr;
    void* rotatedAddr;
} FileDescriptor;



cJSON *SEARCH_RESULT_JSON = NULL, *ROOT_JSON = NULL;

static char *LOGPATH = NULL;
static char *PERSISTENTPATH = NULL;
static long PAGESIZE;
static pthread_mutex_t dcaMutex = PTHREAD_MUTEX_INITIALIZER;




/* @} */ // End of group DCA_TYPES
/**
 * @addtogroup DCA_APIS
 * @{
 */

/** @brief This API processes the top command log file patterns to retrieve load average and process usage.
 *
 *  @param[in] logfile  top_log file
 *
 *  @return  Returns the status of the operation.
 *  @retval  Returns zero on success, appropriate errorcode otherwise.
 */
int processTopPattern(char* profileName,  Vector* topMarkerList, Vector* out_grepResultList, int profileExecCounter)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if(profileName == NULL || topMarkerList == NULL || out_grepResultList == NULL)
    {
        T2Error("Invalid arguments for %s\n", __FUNCTION__);
        return -1;
    }
    size_t var = 0;
    size_t vCount = Vector_Size(topMarkerList);
    T2Debug("topMarkerList for profile %s is of count = %lu \n", profileName, (unsigned long )vCount);
    // Get logfile -> seek value map associated with the profile

    // We are getting the exec count directly from the profileExecCounter parameter
    //int profileExecCounter = gsProfile->execCounter;
    char* filename = NULL;

    for (var = 0; var < vCount; ++var)
    {
        GrepMarker* grepMarkerObj = (GrepMarker*) Vector_At(topMarkerList, var);
        if (!grepMarkerObj || !grepMarkerObj->logFile || !grepMarkerObj->searchString || !grepMarkerObj->markerName)
        {
            continue;
        }
        int tmp_skip_interval, is_skip_param;
        tmp_skip_interval = grepMarkerObj->skipFreq;
        is_skip_param = (profileExecCounter % (tmp_skip_interval + 1) == 0) ? 0 : 1;
        if (is_skip_param != 0)
        {

            T2Debug("Skipping marker %s for profile %s as per skip frequency %d \n", grepMarkerObj->markerName, profileName, tmp_skip_interval);
            continue;
        }
        else
        {

#if !defined(ENABLE_RDKC_SUPPORT) && !defined(ENABLE_RDKB_SUPPORT)
            filename = saveTopOutput(profileName);
#endif
            break;
        }
        T2Debug("topMarkerList[%lu] markerName = %s, logFile = %s, searchString = %s \n", (unsigned long)var, grepMarkerObj->markerName, grepMarkerObj->logFile, grepMarkerObj->searchString);
    }

    for (; var < vCount; ++var) // Loop of marker list starts here
    {
        GrepMarker* grepMarkerObj = (GrepMarker*) Vector_At(topMarkerList, var);
        if (!grepMarkerObj || !grepMarkerObj->logFile || !grepMarkerObj->searchString || !grepMarkerObj->markerName)
        {
            continue;
        }
        if (strcmp(grepMarkerObj->searchString, "") == 0 || strcmp(grepMarkerObj->logFile, "") == 0)
        {
            continue;
        }


        // If the skip frequency is set, skip the marker processing for this interval
        int tmp_skip_interval, is_skip_param;
        tmp_skip_interval = grepMarkerObj->skipFreq;
        is_skip_param = (profileExecCounter % (tmp_skip_interval + 1) == 0) ? 0 : 1;
        if (is_skip_param != 0)
        {

            T2Debug("Skipping marker %s for profile %s as per skip frequency %d \n", grepMarkerObj->markerName, profileName, tmp_skip_interval);
            continue;
        }


        if (strcmp(grepMarkerObj->markerName, "Load_Average") == 0)   // This block is for device level load average
        {
            if (0 == getLoadAvg(out_grepResultList, grepMarkerObj->trimParam, grepMarkerObj->regexParam))
            {
                T2Debug("getLoadAvg() Failed with error");
            }
        }
        else
        {
            getProcUsage(grepMarkerObj->searchString, out_grepResultList, grepMarkerObj->trimParam, grepMarkerObj->regexParam, filename);
        }

    }

#if !defined(ENABLE_RDKC_SUPPORT) && !defined(ENABLE_RDKB_SUPPORT)
    removeTopOutput(filename);
#endif
    T2Debug("%s --out\n", __FUNCTION__);
    return 0;
}

/**
 *  @brief Function to read the rotated Log file.
 *
 *  @param[in] name        Log file name.
 *  @param[in] seek_value  Position to seek.
 *
 *  @return Returns the status of the operation.
 *  @retval Returns -1 on failure, appropriate errorcode otherwise.
 */
static int getLogSeekValue(hash_map_t *logSeekMap, const char *name, long *seek_value)
{

    T2Debug("%s ++in for file %s \n", __FUNCTION__, name);
    int rc = 0;
    if (logSeekMap)
    {
        long *data = (long*) hash_map_get(logSeekMap, name) ;
        if (data)
        {
            *seek_value = *data ;
        }
        else
        {
            T2Debug("data is null .. Setting seek value to 0 from getLogSeekValue \n");
            *seek_value = 0 ;
        }
    }
    else
    {
        T2Debug("logSeekMap is null .. Setting seek value to 0 \n");
        *seek_value = 0 ;
    }

    T2Debug("%s --out \n", __FUNCTION__);
    return rc;
}

/**
 *  @brief Function to write the rotated Log file.
 *
 *  @param[in] name        Log file name.
 *
 *  @return Returns the status of the operation.
 *  @retval Returns -1 on failure, appropriate errorcode otherwise.
 */
static T2ERROR updateLogSeek(hash_map_t *logSeekMap, const char* logFileName, const long logfileSize)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if(logSeekMap == NULL || logFileName == NULL)
    {
        T2Error("Invalid or NULL arguments\n");
        return T2ERROR_FAILURE;
    }
    T2Debug("Adding seekvalue of %ld for %s to logSeekMap \n", logfileSize, logFileName);
    long* val = (long *) malloc(sizeof(long));
    if(NULL != val)
    {
        memset(val, 0, sizeof(long));
        *val = logfileSize;
        hash_map_put(logSeekMap, strdup(logFileName), (void *)val, free);
    }
    else
    {
        T2Warning("Unable to allocate memory for seek value pointer \n");
    }

    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}



/**
 * @brief This API updates the filename if it is different from the current one.
 * @param currentFile The current filename.
 * @param newFile The new filename to update to.
 * @return The updated filename.
 */
static char* updateFilename(char* previousFile, const char* newFile)
{
    if (!newFile)
    {
        return NULL;
    }

    if (!previousFile || strcmp(previousFile, newFile) != 0)
    {
        if(previousFile)
        {
            free(previousFile);
        }
        previousFile = strdup(newFile);
        if (!previousFile)
        {
            T2Error("Insufficient memory to allocate string %s\n", newFile);
        }
    }
    return previousFile;
}

static GrepResult* createGrepResultObj(const char* markerName, const char* markerValue, bool trimParameter, char* regexParameter)
{
    GrepResult* grepResult = (GrepResult*) malloc(sizeof(GrepResult));
    if (grepResult == NULL)
    {
        T2Error("Failed to allocate memory for GrepResult\n");
        return NULL;
    }

    // Initialize pointers to NULL for safer cleanup
    grepResult->markerName = NULL;
    grepResult->markerValue = NULL;
    grepResult->regexParameter = NULL;

    if ((grepResult->markerName = strdup(markerName)) == NULL)
    {
        T2Error("Failed to duplicate markerName\n");
        free((void*)grepResult);
        return NULL;
    }

    if ((grepResult->markerValue = strdup(markerValue)) == NULL)
    {
        T2Error("Failed to duplicate markerValue\n");
        free((void*)grepResult->markerName);
        free((void*)grepResult);
        return NULL;
    }

    grepResult->trimParameter = trimParameter;

    if (regexParameter != NULL)
    {
        if ((grepResult->regexParameter = strdup(regexParameter)) == NULL)
        {
            T2Error("Failed to duplicate regexParameter\n");
            free((void*)grepResult->markerName);
            free((void*)grepResult->markerValue);
            free((void*)grepResult);
            return NULL;
        }
    }
    return grepResult;
}

/**
 * @brief This function formats the count value to a string.
 *     Formatting is to handle the case where the count of error occurence in an interval exceeds 9999.
 *
 * @param[out] buffer  The buffer to store the formatted string.
 * @param[in]  size    The size of the buffer.
 * @param[in]  count   The count value to format.
 */
static inline void formatCount(char* buffer, size_t size, int count)
{
    if (count > 9999)
    {
        count = INVALID_COUNT;
    }
    snprintf(buffer, size, "%d", count);
}


// Simple substring count for patterns <8 chars
static int getCountPatternMatch_std(FileDescriptor* fileDescriptor, const char* pattern) {
    if (!fileDescriptor || !fileDescriptor->cfaddr || !pattern || !*pattern || fileDescriptor->cf_file_size <= 0)
        return -1;

    int count = 0;
    size_t patlen = strlen(pattern);
    for (int i = 0; i < 2; i++) {
        const char* buf = (const char*)(i == 0 ? fileDescriptor->cfaddr : fileDescriptor->rfaddr);
        size_t buflen = (size_t)(i == 0 ? fileDescriptor->cf_file_size : fileDescriptor->rf_file_size);
        if (!buf || buflen < patlen)
            continue;
        for (size_t pos = 0; pos + patlen <= buflen; pos++) {
            if (memcmp(buf + pos, pattern, patlen) == 0) {
                count++;
                pos += patlen - 1; // skip overlapping
            }
        }
    }
    return count;
}

static int getCountPatternMatch(FileDescriptor* fileDescriptor, const char* pattern)
{
    if (!fileDescriptor || !fileDescriptor->cfaddr || !pattern || !*pattern || fileDescriptor->cf_file_size <= 0)
    {
        T2Error("Invalid file descriptor arguments pattern match\n");
        return -1;
    }

    const size_t patlen = strlen(pattern);
    if (patlen < 15)
    {
        T2Debug("Pattern length < 15, using standard search\n");
        return getCountPatternMatch_std(fileDescriptor, pattern); // Call to the new function
    }

    int count = 0;
    const unsigned char first = (unsigned char)pattern[0];
    const unsigned char last = (unsigned char)pattern[patlen - 1];

    for (int i = 0; i < 2; i++)
    {
        const unsigned char* buf = (const unsigned char*)(i == 0 ? fileDescriptor->cfaddr : fileDescriptor->rfaddr);
        const size_t buflen = (size_t)(i == 0 ? fileDescriptor->cf_file_size : fileDescriptor->rf_file_size);

        if (!buf || buflen < patlen)
        {
            continue;
        }

        const unsigned char* end = buf + buflen - patlen;
        const unsigned char* cur = buf;

        while (cur <= end)
        {
            // Quick check first and last chars before full comparison
            if (cur[0] == first && cur[patlen - 1] == last &&
                    memcmp(cur + 1, pattern + 1, patlen - 2) == 0)
            {
                count++;
                cur += patlen; // Skip the whole pattern length
            }
            else
            {
                // Jump to next potential match using first char
                const unsigned char* next = (const unsigned char*)memchr(cur + 1, first, end - cur);
                if (!next)
                {
                    break;
                }
                cur = next;
            }
        }
    }
    return count;
}

static char* getAbsolutePatternMatch(FileDescriptor* fileDescriptor, const char* pattern)
{
    if (!fileDescriptor || !fileDescriptor->cfaddr || fileDescriptor->cf_file_size <= 0 || !pattern || !*pattern)
    {
        T2Error("Invalid file descriptor arguments absolute\n");
        return NULL;
    }

    const size_t patlen = strlen(pattern);
    const unsigned char first = (unsigned char)pattern[0];
    const unsigned char last = (unsigned char)pattern[patlen - 1];
    const char* last_found = NULL;
    size_t current_buflen = 0;

    // Process both current and rotated files
    for (int i = 0; i < 2; i++)
    {
        const unsigned char* buf = (const unsigned char*)(i == 0 ? fileDescriptor->cfaddr : fileDescriptor->rfaddr);
        const size_t buflen = (size_t)(i == 0 ? fileDescriptor->cf_file_size : fileDescriptor->rf_file_size);

        if (!buf || buflen < patlen)
        {
            continue;
        }

        // Start from end of buffer for faster last occurrence finding
        const unsigned char* cur = buf + buflen - patlen;

        while (cur >= buf)
        {
            // Quick check of first and last chars
            if (cur[0] == first && cur[patlen - 1] == last)
            {
                // Full pattern check only if boundary chars match
                if (memcmp(cur + 1, pattern + 1, patlen - 2) == 0)
                {
                    last_found = (const char*)cur;
                    current_buflen = buflen;
                    // For current file (i==0), we can stop at first match from end
                    if (i == 0)
                    {
                        goto found;
                    }
                    break;
                }
            }
            cur--;
        }
    }

found:
    if (!last_found)
    {
        return NULL;
    }

    // Extract the value after pattern until newline
    const char* start = last_found + patlen;
    const char* end = memchr(start, '\n', current_buflen - (start - (const char*)last_found));
    size_t length = end ? (size_t)(end - start) : current_buflen - (start - (const char*)last_found);

    // Allocate only what's needed
    char* result = (char*)malloc(length + 1);
    if (!result)
    {
        T2Error("Memory allocation failed for pattern match result\n");
        return NULL;
    }

    memcpy(result, start, length);
    result[length] = '\0';
    return result;
}

static int processPatternWithOptimizedFunction(const GrepMarker* marker, Vector* out_grepResultList, FileDescriptor* filedescriptor)
{
    if (!marker || !out_grepResultList || !filedescriptor || !filedescriptor->cfaddr)
    {
        T2Error("Invalid arguments for pattern processing\n");
        return -1;
    }

    // Pre-validate pattern
    const char* pattern = marker->searchString;
    if (!pattern || !*pattern)
    {
        T2Error("Empty pattern specified\n");
        return -1;
    }

    // Stack allocation for small strings
    char result_buffer[16] = {0}; // For count results
    GrepResult* result = NULL;

    if (marker->mType == MTYPE_COUNTER)
    {
        int count = getCountPatternMatch(filedescriptor, pattern);
        if (count > 0)
        {
            formatCount(result_buffer, sizeof(result_buffer), count);
            result = createGrepResultObj(marker->markerName, result_buffer,
                                         marker->trimParam, marker->regexParam);
        }
    }
    else
    {
        char* match = getAbsolutePatternMatch(filedescriptor, pattern);
        if (match)
        {
            result = createGrepResultObj(marker->markerName, match,
                                         marker->trimParam, marker->regexParam);
            free(match);
        }
    }

    if (result)
    {
        Vector_PushBack(out_grepResultList, result);
        return 0;
    }

    return 1; // No match found
}


static int getLogFileDescriptor(GrepSeekProfile* gsProfile, const char* logPath, const char* logFile, int old_fd, off_t* out_seek_value)
{
    long seek_value_from_map = 0;
    getLogSeekValue(gsProfile->logFileSeekMap, logFile, &seek_value_from_map);
    if (old_fd != -1)
    {
        close(old_fd);
    }

    char logFilePath[PATH_MAX];
    if(logFile[0] == '/')
    {
        // If the logFile is an absolute path, use it directly
        T2Info("Log file is an absolute path not prefix in the directory: %s\n", logFile);
        snprintf(logFilePath, sizeof(logFilePath), "%s", logFile);
    }
    else
    {
        snprintf(logFilePath, sizeof(logFilePath), "%s/%s", logPath, logFile);
    }

    T2Debug("Opening log file %s\n", logFilePath);
    int fd = open(logFilePath, O_RDONLY);
    if (fd == -1)
    {
        T2Error("Failed to open log file %s\n", logFilePath);
        return -1;
    }

    // Calculate the file size
    struct stat sb;
    if (fstat(fd, &sb) == -1)
    {
        T2Error("Error getting file size for %s\n", logFile);
        close(fd);
        return -1;
    }

    // Check if the file size is 0
    if (sb.st_size == 0)
    {
        T2Error("The size of the logfile is 0 for %s\n", logFile);
        close(fd);
        return -1; // Consistent error return value
    }

    // Check if the file size matches the seek value from the map
    if (sb.st_size == seek_value_from_map)
    {
        T2Error("The logfile size matches the seek value (%ld) for %s\n", seek_value_from_map, logFile);
        close(fd);
        return -1; // Consistent error return value
    }
    updateLogSeek(gsProfile->logFileSeekMap, logFile, sb.st_size);
    *out_seek_value = seek_value_from_map;
    return fd;
}

static int getRotatedLogFileDescriptor(const char* logPath, const char* logFile)
{
    char logFilePath[PATH_MAX];
    snprintf(logFilePath, sizeof(logFilePath), "%s/%s", logPath, logFile);
    //get the rotated filename
    char *fileExtn = ".1";
    char rotatedlogFilePath[PATH_MAX];
    size_t name_len = strlen(logFilePath);
    if(logFile[0] == '/')
    {
        // If the logFile is an absolute path, use it directly
        T2Debug("RotatedLog file is an absolute path not prefix in the directory: %s\n", logFile);
        snprintf(rotatedlogFilePath, sizeof(logFilePath), "%s", logFile);
    }
    else
    {
        // If the logFile is not an absolute path, prefix it with the logPath
        T2Debug("RotatedLog file is not an absolute path, prefixing with directory: %s\n", logPath);
        snprintf(rotatedlogFilePath, sizeof(rotatedlogFilePath), "%s/%s", logPath, logFile);
    }
    if(name_len > 2 && logFilePath[name_len - 2] == '.' && logFilePath[name_len - 1] == '0')
    {
        rotatedlogFilePath[name_len - 1] = '1';
        T2Debug("Log file name seems to be having .0 extension hence Rotated log file name is %s\n", rotatedlogFilePath);
    }
    else
    {
        strncat(rotatedlogFilePath, fileExtn, sizeof(rotatedlogFilePath) - strlen(rotatedlogFilePath) - 1);
        T2Debug("Rotated log file name is %s\n", rotatedlogFilePath);
    }

    int rd = open(rotatedlogFilePath, O_RDONLY);
    if (rd == -1)
    {
        T2Error("Failed to open log file %s\n", rotatedlogFilePath);
        return -1;
    }

    // Calculate the file size
    struct stat rb;
    if (fstat(rd, &rb) == -1)
    {
        T2Error("Error getting file size for %s\n", rotatedlogFilePath);
        close(rd);
        return -1;
    }

    // Check if the file size is 0
    if (rb.st_size == 0)
    {
        T2Error("The size of the logfile is 0 for %s\n", rotatedlogFilePath);
        close(rd);
        return -1; // Consistent error return value
    }
    return rd;
}

// Caller should free the FileDescriptor struct after use
static void freeFileDescriptor(FileDescriptor* fileDescriptor)
{
    if (fileDescriptor)
    {
        if(fileDescriptor->baseAddr)
        {
            munmap(fileDescriptor->baseAddr, fileDescriptor->cf_file_size);
        }
        if(fileDescriptor->rotatedAddr)
        {
            munmap(fileDescriptor->rotatedAddr, fileDescriptor->rf_file_size);
        }
        fileDescriptor->cfaddr = NULL;
        fileDescriptor->rfaddr = NULL;
        if(fileDescriptor->fd != -1)
        {
            close(fileDescriptor->fd);
            fileDescriptor->fd = -1;
        }
        free(fileDescriptor);
    }
}

static FileDescriptor* getFileDeltaInMemMapAndSearch(const int fd, const off_t seek_value, const char* logPath, const char* logFile, bool check_rotated )
{
    // Size threshold for chunked reading (1MB)
    const size_t CHUNK_SIZE = 1024 * 1024;

    // Portable, memory-efficient mmap logic for embedded devices
    if (fd == -1)
    {
        T2Error("Error opening file\n");
        return NULL;
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1 || sb.st_size == 0)
    {
        T2Error("Error getting file size or file is empty\n");
        close(fd);
        return NULL;
    }

    off_t offset = (seek_value > 0) ? ((seek_value / PAGESIZE) * PAGESIZE) : 0;
    size_t bytes_ignored = (seek_value > 0) ? (seek_value - offset) : 0;

    char *main_addr = NULL, *rot_addr = NULL;
    size_t main_size = sb.st_size;
    size_t rot_size = 0;
    size_t map_size = main_size;

    // For large files, only map the portion we need
    if (main_size > CHUNK_SIZE)
    {
        map_size = CHUNK_SIZE + (seek_value % CHUNK_SIZE);
        if (map_size > main_size)
        {
            map_size = main_size;
        }
    }

    int rd = -1;
    struct stat rb;
    bool use_rotated = (seek_value > sb.st_size || check_rotated);
    if (use_rotated)
    {
        rd = getRotatedLogFileDescriptor(logPath, logFile);
        if (rd != -1 && fstat(rd, &rb) == 0 && rb.st_size > 0)
        {
            size_t rot_map_size = (size_t)rb.st_size > CHUNK_SIZE ? CHUNK_SIZE : (size_t)rb.st_size;
            main_addr = mmap(NULL, map_size, PROT_READ, MAP_PRIVATE, fd, 0);
            rot_addr = mmap(NULL, rot_map_size, PROT_READ, MAP_PRIVATE, rd, offset);
            rot_size = rb.st_size;
            close(rd);
        }
        else
        {
            main_addr = mmap(NULL, main_size, PROT_READ, MAP_PRIVATE, fd, offset);
            rot_addr = NULL;
            rot_size = 0;
            if (rd != -1)
            {
                close(rd);
            }
        }
    }
    else
    {
        main_addr = mmap(NULL, main_size, PROT_READ, MAP_PRIVATE, fd, offset);
        rot_addr = NULL;
        rot_size = 0;
    }
    close(fd);

    // Check main file mapping first
    if (main_addr == MAP_FAILED)
    {
        if (rot_addr && rot_addr != MAP_FAILED)
        {
            munmap(rot_addr, rot_size);
        }
        T2Error("Error in memory mapping main file\n");
        return NULL;
    }

    // Check rotated file mapping if it was attempted
    if (rot_addr && rot_addr == MAP_FAILED)
    {
        munmap(main_addr, main_size);
        T2Error("Error in memory mapping rotated file\n");
        return NULL;
    }

    FileDescriptor* fileDescriptor = (FileDescriptor*)malloc(sizeof(FileDescriptor));
    if (!fileDescriptor)
    {
        if (main_addr)
        {
            munmap(main_addr, main_size);
        }
        if (rot_addr)
        {
            munmap(rot_addr, rot_size);
        }
        T2Error("Error allocating memory for FileDescriptor\n");
        return NULL;
    }
    memset(fileDescriptor, 0, sizeof(FileDescriptor));
    fileDescriptor->baseAddr = (void *)main_addr;
    fileDescriptor->cfaddr = main_addr + bytes_ignored;
    fileDescriptor->cf_file_size = main_size;
    fileDescriptor->fd = -1;
    if (rot_addr)
    {
        fileDescriptor->rotatedAddr = (void *)rot_addr;
        fileDescriptor->rfaddr = rot_addr + bytes_ignored;
        fileDescriptor->rf_file_size = rot_size;
    }
    else
    {
        fileDescriptor->rotatedAddr = NULL;
        fileDescriptor->rfaddr = NULL;
        fileDescriptor->rf_file_size = 0;
    }
    return fileDescriptor;
}

// Call 2
/** @description: Main logic function to parse sorted vector list and to process the pattern list
 *  @param filename
 *  @return -1 on failure, 0 on success
 */
static int parseMarkerListOptimized(GrepSeekProfile *gsProfile, Vector * ip_vMarkerList, Vector * out_grepResultList, bool check_rotated, char* logPath)
{
    T2Debug("%s ++in \n", __FUNCTION__);

    if(NULL == gsProfile || NULL == ip_vMarkerList || NULL == out_grepResultList)
    {
        T2Error("Invalid arguments for %s\n", __FUNCTION__);
        return -1;
    }
    char *prevfile = NULL;
    int fd = -1;
    FileDescriptor* fileDescriptor = NULL;
    size_t vCount = Vector_Size(ip_vMarkerList);
    int profileExecCounter = gsProfile ? gsProfile->execCounter : 0;

    if (!gsProfile || !ip_vMarkerList || !out_grepResultList)
    {
        T2Error("Invalid arguments for %s\n", __FUNCTION__);
        return -1;
    }

    if ((gsProfile->execCounter == 1) && (firstreport_after_bootup == false))
    {
        check_rotated_logs = check_rotated;
        firstreport_after_bootup = true;
    }
    else
    {
        check_rotated_logs = false;
    }

    for (size_t var = 0; var < vCount; ++var)
    {
        GrepMarker* marker = (GrepMarker*) Vector_At(ip_vMarkerList, var);
        if (!marker || !marker->logFile || !marker->searchString || !marker->markerName ||
                strcmp(marker->searchString, "") == 0 || strcmp(marker->logFile, "") == 0)
        {
            continue;
        }

        int skipFreq = marker->skipFreq > 0 ? marker->skipFreq : 0;
        int is_skip = (profileExecCounter % (skipFreq + 1) == 0) ? 0 : 1;

        // Only update file resources if log file changes
        if (!prevfile || strcmp(marker->logFile, prevfile) != 0)
        {
            if (prevfile)
            {
                free(prevfile);
                prevfile = NULL;
            }
            if (fd != -1)
            {
                close(fd);
                fd = -1;
            }
            if (fileDescriptor)
            {
                freeFileDescriptor(fileDescriptor);
                fileDescriptor = NULL;
            }

            off_t seek_value = 0;
            fd = getLogFileDescriptor(gsProfile, logPath, marker->logFile, fd, &seek_value);
            prevfile = updateFilename(prevfile, marker->logFile);
            if (fd == -1)
            {
                T2Error("Error opening file %s\n", marker->logFile);
                continue;
            }
            fileDescriptor = getFileDeltaInMemMapAndSearch(fd, seek_value, logPath, marker->logFile, check_rotated_logs);
            if (!fileDescriptor)
            {
                T2Error("Failed to get file descriptor for %s\n", marker->logFile);
                continue;
            }
        }

        if (is_skip == 0 && fileDescriptor)
        {
            processPatternWithOptimizedFunction(marker, out_grepResultList, fileDescriptor);
        }
    }

    gsProfile->execCounter += 1;
    T2Debug("Execution Count = %d\n", gsProfile->execCounter);

    if (prevfile)
    {
        free(prevfile);
        prevfile = NULL;
    }
    if (fd != -1)
    {
        close(fd);
        fd = -1;
    }
    if (fileDescriptor)
    {
        freeFileDescriptor(fileDescriptor);
        fileDescriptor = NULL;
    }

    T2Debug("%s --out \n", __FUNCTION__);
    return 0;
}

void T2InitProperties()
{
    initProperties(&LOGPATH, &PERSISTENTPATH, &PAGESIZE);
}


// Call 1
int getDCAResultsInVector(GrepSeekProfile *gSeekProfile, Vector * vecMarkerList, Vector** out_grepResultList, bool check_rotated, char* customLogPath)
{

    T2Debug("%s ++in \n", __FUNCTION__);
    int rc = -1;
    if(NULL == gSeekProfile || NULL == vecMarkerList || NULL == out_grepResultList)
    {
        T2Error("Invalid arguments for %s\n", __FUNCTION__);
        return rc;
    }
    pthread_mutex_lock(&dcaMutex);
    if(NULL != vecMarkerList)
    {
        char* logPath = customLogPath ? customLogPath : LOGPATH;

        Vector_Create(out_grepResultList);

        // Go for looping through the marker list
        if( (rc = parseMarkerListOptimized(gSeekProfile, vecMarkerList, *out_grepResultList, check_rotated, logPath)) == -1 )
        {
            T2Debug("Error in fetching grep results\n");
        }
    }
    pthread_mutex_unlock(&dcaMutex);
    T2Debug("%s --out \n", __FUNCTION__);
    return rc;
}
