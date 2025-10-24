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

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <stdbool.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <limits.h>
#include <sys/sendfile.h>
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
    off_t cf_map_size;
    off_t rf_map_size;
    off_t cf_file_size;
    off_t rf_file_size;
    char* cfaddr;
    char* rfaddr;
    void* baseAddr;
    void* rotatedAddr;
} FileDescriptor;

/**
 * Portable implementation of strnstr (BSD function).
 * Searches for the first occurrence of the substring 'needle' in the
 * first 'len' bytes of 'haystack'.
 * Returns pointer to the beginning of the match, or NULL if not found.
 */
static const char *strnstr(const char *haystack, const char *needle, size_t len)
{
    T2Info("%s %d \n", __FUNCTION__, __LINE__);
    if (haystack  == NULL || needle == NULL)
    {
        return NULL;
    }
    T2Info("%s %d \n", __FUNCTION__, __LINE__);

    size_t needle_len = strlen(needle);
    if (needle_len == 0)
    {
        return haystack;
    }
    T2Info("%s %d \n", __FUNCTION__, __LINE__);

    // Check if search is possible and prevent overflow
    if (len < needle_len || len - needle_len > len)
    {
        return NULL;
    }

    // Check minimum length requirements for optimized search
    if (needle_len < 4)
    {
        // Use simple search for short patterns
        for (size_t i = 0; i <= len - needle_len; i++)
        {
            if (memcmp(haystack + i, needle, needle_len) == 0)
            {
                return haystack + i;
            }
        }
        return NULL;
    }

    T2Info("%s %d \n", __FUNCTION__, __LINE__);
    size_t skip[256];
    for (size_t i = 0; i < 256; ++i)
    {
        skip[i] = needle_len;
    }
    T2Info("%s %d \n", __FUNCTION__, __LINE__);

    for (size_t i = 0; i < needle_len - 1; ++i)
    {
        skip[(unsigned char)needle[i]] = needle_len - i - 1;
    }
    T2Info("%s %d \n", __FUNCTION__, __LINE__);

    size_t i = 0;
    while (i <= len - needle_len)
    {
        size_t j = needle_len - 1;
        while (j < needle_len && haystack[i + j] == needle[j])
        {
            j--;
        }
        if (j == (size_t) -1)
        {
            return haystack + i; // Match found
        }
        size_t s = skip[(unsigned char)haystack[i + needle_len - 1]];
        i += (s > 0) ? s : 1;
    }
    T2Info("%s %d \n", __FUNCTION__, __LINE__);

    return NULL;
}

cJSON *SEARCH_RESULT_JSON = NULL, *ROOT_JSON = NULL;

static char *LOGPATH = NULL;
static char *PERSISTENTPATH = NULL;
static long PAGESIZE;
static pthread_mutex_t dcaMutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief Extract Unix timestamp from ISO 8601 format timestamp at the beginning of a line.
 *
 * @param line_start Pointer to the beginning of the line containing the timestamp
 * @return Unix timestamp on success, 0 on failure or if no valid timestamp found
 */
static time_t extractUnixTimestamp(const char* line_start)
{
    if (!line_start)
    {
        T2Warning("extractUnixTimestamp: line_start is NULL\n");
        return 0;
    }

    // Check if we have enough characters for ISO 8601 format: YYYY-MM-DDTHH:MM:SS.mmm (23 chars)
    // We'll do a basic length check by looking for the expected format structure
    if (strlen(line_start) < 23)
    {
        T2Debug("extractUnixTimestamp: Line too short for timestamp\n");
        return 0;
    }

    // Extract the timestamp portion (first 23 characters)
    char timestamp_str[24] = {0};
    strncpy(timestamp_str, line_start, 23);
    timestamp_str[23] = '\0';

    T2Debug("extractUnixTimestamp: Attempting to parse timestamp: %s\n", timestamp_str);

    struct tm tm_time = {0};

    // Parse using strptime - format: "YYYY-MM-DDTHH:MM:SS"
    // Note: strptime doesn't handle milliseconds, so we parse up to seconds
    char* result = strptime(timestamp_str, "%Y-%m-%dT%H:%M:%S", &tm_time);
    if (result != NULL)
    {
        tm_time.tm_isdst = -1; // Let system determine DST
        time_t unix_timestamp = mktime(&tm_time);
        if (unix_timestamp != -1)
        {
            T2Debug("extractUnixTimestamp: Successfully parsed timestamp: %s -> Unix: %ld\n",
                    timestamp_str, unix_timestamp);
            return unix_timestamp;
        }
        else
        {
            T2Warning("extractUnixTimestamp: mktime() failed for timestamp: %s\n", timestamp_str);
        }
    }
    else
    {
        T2Debug("extractUnixTimestamp: strptime() failed to parse timestamp: %s\n", timestamp_str);
    }

    return 0;
}


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
int processTopPattern(char* profileName,  Vector* topMarkerList, int profileExecCounter)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if(profileName == NULL || topMarkerList == NULL)
    {
        T2Error("Invalid arguments for %s\n", __FUNCTION__);
        return -1;
    }
    size_t var = 0;
    size_t vCount = Vector_Size(topMarkerList);
    T2Debug("topMarkerList for profile %s is of count = %lu \n", profileName, (unsigned long )vCount);

    char* filename = NULL;

    for (var = 0; var < vCount; ++var)
    {
        TopMarker* topMarkerObj = (TopMarker*) Vector_At(topMarkerList, var);
        if (!topMarkerObj || !topMarkerObj->logFile || !topMarkerObj->searchString || !topMarkerObj->markerName)
        {
            continue;
        }
        int tmp_skip_interval, is_skip_param;
        tmp_skip_interval = topMarkerObj->skipFreq;
        if(tmp_skip_interval <= 0)
        {
            tmp_skip_interval = 0;
        }
        is_skip_param = (profileExecCounter % (tmp_skip_interval + 1) == 0) ? 0 : 1;
        if (is_skip_param != 0)
        {
            T2Debug("Skipping marker %s for profile %s as per skip frequency %d \n", topMarkerObj->markerName, profileName, tmp_skip_interval);
            continue;
        }
        else
        {

#if !defined(ENABLE_RDKC_SUPPORT) && !defined(ENABLE_RDKB_SUPPORT)
            filename = saveTopOutput(profileName);
#endif
            break;
        }
        T2Debug("topMarkerList[%lu] markerName = %s, logFile = %s, searchString = %s \n", (unsigned long)var, topMarkerObj->markerName, topMarkerObj->logFile, topMarkerObj->searchString);
    }

    for (; var < vCount; ++var) // Loop of marker list starts here
    {
        TopMarker* topMarkerObj = (TopMarker*) Vector_At(topMarkerList, var);
        if (!topMarkerObj || !topMarkerObj->logFile || !topMarkerObj->searchString || !topMarkerObj->markerName)
        {
            continue;
        }
        if (strcmp(topMarkerObj->searchString, "") == 0 || strcmp(topMarkerObj->logFile, "") == 0)
        {
            continue;
        }


        // If the skip frequency is set, skip the marker processing for this interval
        int tmp_skip_interval, is_skip_param;
        tmp_skip_interval = topMarkerObj->skipFreq;
        if(tmp_skip_interval <= 0)
        {
            tmp_skip_interval = 0;
        }
        is_skip_param = (profileExecCounter % (tmp_skip_interval + 1) == 0) ? 0 : 1;
        if (is_skip_param != 0)
        {

            T2Debug("Skipping marker %s for profile %s as per skip frequency %d \n", topMarkerObj->markerName, profileName, tmp_skip_interval);
            continue;
        }


        if (strcmp(topMarkerObj->markerName, "Load_Average") == 0)   // This block is for device level load average
        {
            if (0 == getLoadAvg(topMarkerObj, topMarkerObj->trimParam, topMarkerObj->regexParam))
            {
                T2Debug("getLoadAvg() Failed with error");
            }
        }
        else
        {
            getProcUsage(topMarkerObj->searchString, topMarkerObj, topMarkerObj->trimParam, topMarkerObj->regexParam, filename);
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

static int getCountPatternMatch(FileDescriptor* fileDescriptor, GrepMarker* marker)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if (!fileDescriptor || !fileDescriptor->cfaddr || !marker || !marker->searchString || fileDescriptor->cf_map_size <= 0)
    {
        T2Error("Invalid file descriptor arguments pattern match\n");
        return -1; // Invalid arguments
    }

    const char* pattern = marker->searchString;
    const char* buffer;
    size_t buflen = 0;
    size_t patlen = strlen(pattern);
    int count = 0;

    for(int i = 0; i < 2; i++)
    {
        if (i == 0)
        {
            buffer = fileDescriptor->cfaddr;
            buflen = (size_t)fileDescriptor->cf_map_size;
        }
        else
        {
            buffer = fileDescriptor->rfaddr;
            buflen = (size_t)fileDescriptor->rf_map_size;
        }
        if(buffer == NULL)
        {
            T2Debug("Invalid file descriptor arguments pattern match\n");
            continue;
        }
        if (patlen == 0 || buflen < patlen)
        {
            T2Info("File size is less than pattern length so ignoring the file\n");
            continue;
        }

        const char *cur = buffer;
        size_t bytes_left = buflen;

        while (bytes_left >= patlen)
        {
            const char *found = strnstr(cur, pattern, bytes_left);
            if (!found)
            {
                break;
            }
            count++;
            size_t advance = (size_t)(found - cur) + patlen;
            cur = found + patlen;
            if (bytes_left < advance)
            {
                break;
            }
            bytes_left -= advance;
        }

    }

    // Using the union instead of the previous out list.
    marker->u.count = count;
    T2Debug("%s --out\n", __FUNCTION__);
    return 0;
    return 0;
}

static int getAbsolutePatternMatch(FileDescriptor* fileDescriptor, GrepMarker* marker)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if (!fileDescriptor || !fileDescriptor->cfaddr || fileDescriptor->cf_map_size <= 0 || !marker || !marker->searchString)
    {
        T2Error("Invalid file descriptor arguments absolute\n");
        return -1;
        return -1;
    }

    const char* pattern = marker->searchString;
    const char* buffer;
    size_t buflen = 0;
    size_t patlen = strlen(pattern);
    const char *last_found = NULL;

    for ( int i = 0; i < 2; i++ )
    {
        if (i == 0)
        {
            buffer = fileDescriptor->cfaddr;
            buflen = (size_t)fileDescriptor->cf_map_size;
        }
        else
        {
            buffer = fileDescriptor->rfaddr;
            buflen = (size_t)fileDescriptor->rf_map_size;
        }

        if(buffer == NULL)
        {
            T2Info("Invalid file descriptor arguments absolute match\n");
            continue;
        }
        const char *cur = buffer;
        size_t bytes_left = buflen;

        while (bytes_left >= patlen)
        {
            const char *found = strnstr(cur, pattern, bytes_left);
            if (!found)
            {
                break;
            }
            last_found = found;
            size_t advance = (size_t)(found - cur) + patlen;
            cur = found + patlen;
            if (bytes_left < advance)
            {
                break;
            }
            bytes_left -= advance;
        }

        if (!last_found)
        {
            continue;
        }
        if(last_found && i == 0)
        {
            break;
        }
    }

    if(!last_found)
    {
        marker->u.markerValue = NULL;
        return 0;
    }


    // Move pointer just after the pattern
    const char *start = last_found + patlen;
    size_t chars_left = buflen - (start - buffer);

    // Find next newline or end of buffer
    const char *end = memchr(start, '\n', chars_left);
    size_t length = end ? (size_t)(end - start) : chars_left;

    char *result = (char*)malloc(length + 1);
    if (!result)
    {
        marker->u.markerValue = NULL;
        return -1;
    }
    memcpy(result, start, length);
    result[length] = '\0';

    marker->u.markerValue = result;
    T2Debug("%s --out\n", __FUNCTION__);
    return 0;
}

static int getAccumulatePatternMatch(FileDescriptor* fileDescriptor, GrepMarker* marker)
{
    T2Info("%s ++in", __FUNCTION__);
    if (!fileDescriptor || !fileDescriptor->cfaddr || fileDescriptor->cf_file_size <= 0 || !marker || !marker->searchString || !*marker->searchString )
    {
        T2Error("Invalid file descriptor arguments accumulate\n");
        return -1;
    }


    const char* pattern = marker->searchString;
    const char* buffer;
    size_t buflen = 0;
    size_t patlen = strlen(pattern);

    // Using the existing accumulatedValues Vector from marker's union
    Vector* accumulatedValues = marker->u.accumulatedValues;
    if (!accumulatedValues)
    {
        T2Error("accumulatedValues vector is NULL in marker\n");
        return -1;
    }

    for (int i = 0; i < 2; i++)
    {
        T2Info("%s %d \n", __FUNCTION__, __LINE__);
        if (i == 0)
        {
            buffer = fileDescriptor->cfaddr;
            buflen = (size_t)fileDescriptor->cf_file_size;
        }
        else
        {
            buffer = fileDescriptor->rfaddr;
            buflen = (size_t)fileDescriptor->rf_file_size;
        }

        if (buffer == NULL)
        {
            T2Info("Invalid file descriptor arguments accumulate match\n");
            continue;
        }

        const char *cur = buffer;
        size_t bytes_left = buflen;
        const char *buffer_end = buffer + buflen;

        while (bytes_left >= patlen)
        {
            T2Info("%s %d \n", __FUNCTION__, __LINE__);

            // Check MAX_ACCUMULATE limit before processing this match
            int arraySize = Vector_Size(accumulatedValues);
            T2Info("Current array size : %d \n", arraySize);

            if (arraySize >= MAX_ACCUMULATE)
            {
                T2Info("%s %d \n", __FUNCTION__, __LINE__);
                if (arraySize == MAX_ACCUMULATE)
                {
                    T2Warning("Max size of the array has been reached appending warning message : %s\n", MAX_ACCUMULATE_MSG);
                    Vector_PushBack(accumulatedValues, strdup(MAX_ACCUMULATE_MSG));
                    T2Debug("Successfully added warning message into vector New Size : %d\n", arraySize + 1);
                }
                else
                {
                    T2Warning("Max size of the array has been reached Ignore New Value\n");
                }
                break;
            }
            T2Info("%s %d \n", __FUNCTION__, __LINE__);

            const char *found = strnstr(cur, pattern, bytes_left);
            if (!found)
            {
                T2Info("%s %d \n", __FUNCTION__, __LINE__);
                break;
            }
            T2Info("%s %d \n", __FUNCTION__, __LINE__);

            // Find the beginning of the line containing the pattern
            const char *line_start = found;
            while (line_start > buffer && *(line_start - 1) != '\n')
            {
                line_start--;
            }

            time_t unix_timestamp = extractUnixTimestamp (line_start);

            T2Info("%s %d \n", __FUNCTION__, __LINE__);

            // Move pointer just after the pattern
            const char *start = found + patlen;
            size_t chars_left = buflen - (start - buffer);

            // Find next newline or end of buffer
            const char *end = memchr(start, '\n', chars_left);
            size_t length = end ? (size_t)(end - start) : chars_left;
            T2Info("%s %d \n", __FUNCTION__, __LINE__);

            // Create result string for this occurrence
            char *result = (char*)malloc(length + 1);
            if (result)
            {
                T2Info("%s %d \n", __FUNCTION__, __LINE__);

                memcpy(result, start, length);
                result[length] = '\0';
                T2Info("%s %d : result = %s\n", __FUNCTION__, __LINE__, result);
                Vector_PushBack(accumulatedValues, result);

                if (unix_timestamp > 0 && marker->accumulatedTimestamp)
                {
                    char *timestamp_str_epoch = (char*)malloc(32);
                    if (timestamp_str_epoch)
                    {
                        snprintf(timestamp_str_epoch, 32, "%ld", unix_timestamp);
                        Vector_PushBack(marker->accumulatedTimestamp, timestamp_str_epoch);
                        T2Info("Stored timestamp: %s\n", timestamp_str_epoch);
                    }
                }
            }

            size_t advance = (size_t)(found - cur) + patlen;
            cur = found + patlen;
            if (bytes_left < advance)
            {
                T2Info("%s %d \n", __FUNCTION__, __LINE__);
                break;
            }
            bytes_left -= advance;
            T2Info("%s %d, current position: %p , buffer_end: %p \n", __FUNCTION__, __LINE__, cur, buffer_end);
            if (cur >= buffer_end)
            {
                T2Info("Reached end of buffer\n");
                break;
            }
            T2Info("%s %d: bytes_left = %d, advance= %d\n", __FUNCTION__, __LINE__, (int)bytes_left, (int)advance);
        }
        T2Info("%s %d --out\n", __FUNCTION__, __LINE__);
    }

    T2Debug("%s --out\n", __FUNCTION__);
    return 0;
}

static int processPatternWithOptimizedFunction(GrepMarker* marker, FileDescriptor* filedescriptor)
{
    // Sanitize the input
    const char* memmmapped_data_cf = filedescriptor->cfaddr;
    if (!marker || !memmmapped_data_cf)
    {
        T2Error("Invalid arguments for %s\n", __FUNCTION__);
        return -1;
    }
    // Extract the pattern and other parameters from the marker
    MarkerType mType = marker->mType;

    if (mType == MTYPE_COUNTER)
    {
        // Count the number of occurrences of the pattern in the memory-mapped data
        getCountPatternMatch(filedescriptor, marker);
    }
    else if (mType == MTYPE_ACCUMULATE)
    {
        //Get MAX_ACCUMULATE number of occurrences of the pattern in the memory-mapped data
        T2Info("%s %d : Accumulate is called\n", __FUNCTION__, __LINE__);
        getAccumulatePatternMatch(filedescriptor, marker);
        T2Info("%s %d : Accumulate is complete\n", __FUNCTION__, __LINE__);
    }
    else /* MTYPE_ABSOLUTE */
    {
        // Get the last occurrence of the pattern in the memory-mapped data
        getAbsolutePatternMatch(filedescriptor, marker);
    }
    return 0;
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
        T2Debug("Failed to open log file %s\n", logFilePath);
        return -1;
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1)
    {
        T2Debug("Error getting file size for %s\n", logFile);
        close(fd);
        return -1;
    }

    if (sb.st_size == 0)
    {
        T2Debug("The size of the logfile is 0 for %s\n", logFile);
        close(fd);
        return -1; // Consistent error return value
    }

    // Check if the file size matches the seek value from the map
    if (sb.st_size == seek_value_from_map)
    {
        T2Info("The logfile size matches the seek value (%ld) for %s\n", seek_value_from_map, logFile);
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
        T2Debug("Failed to open log file %s\n", rotatedlogFilePath);
        return -1;
    }

    struct stat rb;
    if (fstat(rd, &rb) == -1)
    {
        T2Debug("Error getting file size for %s\n", rotatedlogFilePath);
        close(rd);
        return -1;
    }

    // Check if the file size is 0
    if (rb.st_size == 0)
    {
        T2Debug("The size of the logfile is 0 for %s\n", rotatedlogFilePath);
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
            fileDescriptor->baseAddr = NULL;
        }
        if(fileDescriptor->rotatedAddr)
        {
            munmap(fileDescriptor->rotatedAddr, fileDescriptor->rf_file_size);
            fileDescriptor->rotatedAddr = NULL;
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
    char *addrcf = NULL;
    char *addrrf = NULL;
    if (fd == -1)
    {
        T2Debug("Error opening file\n");
        return NULL;
    }
    // Read the file contents using mmap
    struct stat sb;
    struct stat rb;
    if(fstat(fd, &sb) == -1)
    {
        T2Debug("Error getting file size\n");
        return NULL;
    }

    if(sb.st_size == 0)
    {
        T2Debug("The Size of the logfile is 0\n");
        return NULL;
    }

    FileDescriptor* fileDescriptor = NULL;
    off_t offset_in_page_size_multiple ;
    unsigned int bytes_ignored = 0, bytes_ignored_main = 0, bytes_ignored_rotated = 0;
    off_t main_fsize = 0, rotated_fsize = 0;
    // Find the nearest multiple of page size
    if (seek_value > 0 && PAGESIZE > 0)
    {
        offset_in_page_size_multiple = (seek_value / PAGESIZE) * PAGESIZE;
        bytes_ignored = seek_value - offset_in_page_size_multiple;
    }
    else
    {
        offset_in_page_size_multiple = 0;
        bytes_ignored = 0;
    }

    //create a tmp file for main file fd
    char tmp_fdmain[] = "/tmp/dca_tmpfile_fdmainXXXXXX";
    int tmp_fd = mkstemp(tmp_fdmain);
    if (tmp_fd == -1)
    {
        T2Error("Failed to create temp file: %s\n", strerror(errno));
        return NULL;
    }
    if(unlink(tmp_fdmain) == -1)
    {
        T2Error("unlink failed\n");
        close(tmp_fd);
        return NULL;
    }
    off_t offset = 0;
    ssize_t sent = sendfile(tmp_fd, fd, &offset, sb.st_size);
    if (sent != sb.st_size)
    {
        T2Error("sendfile failed: %s\n", strerror(errno));
        close(tmp_fd);
        return NULL;
    }

    if(seek_value > sb.st_size || check_rotated == true)
    {
        int rd = getRotatedLogFileDescriptor(logPath, logFile);
        if (rd != -1 && fstat(rd, &rb) == 0 && rb.st_size > 0)
        {
            char tmp_fdrotated[] = "/tmp/dca_tmpfile_fdrotatedXXXXXX";
            int tmp_rd = mkstemp(tmp_fdrotated);
            if (tmp_rd == -1)
            {
                T2Error("Failed to create temp file: %s\n", strerror(errno));
                close(tmp_fd);
                if(rd != -1)
                {
                    close(rd);
                    rd = -1;
                }
                return NULL;
            }
            if(unlink(tmp_fdrotated) == -1)
            {
                T2Error("unlink failed\n");
                close(tmp_fd);
                close(tmp_rd);
                if(rd != -1)
                {
                    close(rd);
                    rd = -1;
                }
                return NULL;
            }

            offset = 0;
            sent = sendfile(tmp_rd, rd, &offset, rb.st_size);
            if (sent != rb.st_size)
            {
                T2Error("sendfile failed: %s\n", strerror(errno));
                close(tmp_rd);
                close(tmp_fd);
                if(rd != -1)
                {
                    close(rd);
                    rd = -1;
                }
                return NULL;
            }

            if(rb.st_size > seek_value)
            {
                rotated_fsize = rb.st_size - seek_value;
                main_fsize = sb.st_size;
                bytes_ignored_rotated = bytes_ignored;
                addrcf = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, tmp_fd, 0);
                addrrf = mmap(NULL, rb.st_size, PROT_READ, MAP_PRIVATE, tmp_rd, offset_in_page_size_multiple);
            }
            else
            {
                rotated_fsize = rb.st_size;
                main_fsize = sb.st_size - seek_value;
                bytes_ignored_main = bytes_ignored;
                addrcf = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, tmp_fd, offset_in_page_size_multiple);
                addrrf = mmap(NULL, rb.st_size, PROT_READ, MAP_PRIVATE, tmp_rd, 0);
            }
            close(tmp_rd);
            close(rd);
            rd = -1;
        }
        else
        {
            T2Error("Error opening rotated file. Start search in current file\n");
            T2Debug("File size rounded to nearest page size used for offset read: %jd bytes\n", (intmax_t)offset_in_page_size_multiple);
            if(seek_value < sb.st_size)
            {
                addrcf = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, tmp_fd, offset_in_page_size_multiple);
                bytes_ignored_main = bytes_ignored;
                main_fsize = sb.st_size - seek_value;
            }
            else
            {

                T2Debug("Log file got rotated. Ignoring invalid mapping\n");
                close(tmp_fd);
                close(fd);
                if(rd != -1)
                {
                    close(rd);
                    rd = -1;
                }
                return NULL;
            }
        }
        if(rd != -1)
        {
            close(rd);
            rd = -1;
        }
    }
    else
    {
        T2Debug("File size rounded to nearest page size used for offset read: %jd bytes\n", (intmax_t)offset_in_page_size_multiple);
        if(seek_value < sb.st_size)
        {
            addrcf = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, tmp_fd, offset_in_page_size_multiple);
            bytes_ignored_main = bytes_ignored;
            main_fsize = sb.st_size - seek_value;
        }
        else
        {
            T2Debug("Log file got rotated. Ignoring invalid mapping\n");
            close(tmp_fd);
            close(fd);
            return NULL;
        }
        addrrf = NULL;
    }
    close(tmp_fd);
    close(fd);

    if (addrcf == MAP_FAILED)
    {
        if(addrrf != NULL)
        {
            munmap(addrrf, rb.st_size);
        }
        T2Error("Error in memory mapping file %d: %s\n", fd, strerror(errno));
        return NULL;
    }
    if (addrrf == MAP_FAILED)
    {
        munmap(addrcf, sb.st_size);
        T2Error("Error in memory mapping file %d: %s\n", fd, strerror(errno));
        return NULL;
    }
    fileDescriptor = (FileDescriptor*)malloc(sizeof(FileDescriptor));
    if (!fileDescriptor)
    {
        T2Error("Error allocating memory for FileDescriptor\n");
        munmap(addrcf, sb.st_size);
        if(addrrf != NULL)
        {
            munmap(addrrf, rb.st_size);
        }
        return NULL;
    }
    memset(fileDescriptor, 0, sizeof(FileDescriptor));
    fileDescriptor->baseAddr = (void *)addrcf;
    addrcf += bytes_ignored_main;
    if(addrrf != NULL)
    {
        fileDescriptor->rotatedAddr = (void *)addrrf;
        addrrf += bytes_ignored_rotated;
        fileDescriptor->rfaddr = addrrf;
    }
    else
    {
        fileDescriptor->rotatedAddr = NULL;
        fileDescriptor->rfaddr = NULL;
    }
    fileDescriptor->cfaddr = addrcf;
    fileDescriptor->fd = fd;
    fileDescriptor->cf_map_size = main_fsize;
    fileDescriptor->cf_file_size = sb.st_size;
    if(fileDescriptor->rfaddr != NULL)
    {
        fileDescriptor->rf_map_size = rotated_fsize;
        fileDescriptor->rf_file_size = rb.st_size;
    }
    else
    {
        fileDescriptor->rf_map_size = 0;
        fileDescriptor->rf_file_size = 0;
    }
    return fileDescriptor;
}

// Call 2
/** @description: Main logic function to parse sorted vector list and to process the pattern list
 *  @param filename
 *  @return -1 on failure, 0 on success
 */
static int parseMarkerListOptimized(GrepSeekProfile *gsProfile, Vector * ip_vMarkerList, bool check_rotated, char* logPath)
{
    T2Debug("%s ++in \n", __FUNCTION__);

    if(NULL == gsProfile || NULL == ip_vMarkerList )
    {
        T2Error("Invalid arguments for %s\n", __FUNCTION__);
        return -1;
    }

    char *prevfile = NULL;

    size_t var = 0;
    size_t vCount = Vector_Size(ip_vMarkerList);

    if(NULL == gsProfile)
    {
        T2Error("%s Unable to retrieve/create logSeekMap for profile \n", __FUNCTION__);
        return -1;
    }
    int profileExecCounter = gsProfile->execCounter;
    // checking the execution count and first report after bootup because after config reload again execution count will get initialised and again reaches 1.
    // check_rotated logs flag is to check the rotated log files even when seekvalue is less than filesize for the first time.
    if((gsProfile->execCounter == 1) && (firstreport_after_bootup == false))
    {
        check_rotated_logs = check_rotated;
        firstreport_after_bootup = true;
    }
    else
    {
        check_rotated_logs = false;
    }

    // Loops start here - This should be completed here
    // Traverse through sorted ip_vMarkerList marker list
    // Reuse the file descriptor or memmory mapped I/O when the log file is same between iterations

    int fd = -1;
    FileDescriptor* fileDescriptor = NULL;

    for (var = 0; var < vCount; ++var) // Loop of marker list starts here
    {
        GrepMarker* grepMarkerObj = (GrepMarker*) Vector_At(ip_vMarkerList, var);
        if (!grepMarkerObj || !grepMarkerObj->logFile || !grepMarkerObj->searchString || !grepMarkerObj->markerName)
        {
            continue;
        }
        if (strcmp(grepMarkerObj->searchString, "") == 0 || strcmp(grepMarkerObj->logFile, "") == 0)
        {
            continue;
        }

        int tmp_skip_interval, is_skip_param;
        tmp_skip_interval = grepMarkerObj->skipFreq;

        char *log_file_for_this_iteration = grepMarkerObj->logFile;

        // For first iteration and when the log file changes
        if (NULL == prevfile || strcmp(log_file_for_this_iteration, prevfile) != 0)
        {
            if (prevfile != NULL)
            {
                free(prevfile);
                prevfile = NULL;
            }

            if (fd != -1)
            {
                close(fd);
                fd = -1;
            }

            if (fileDescriptor != NULL)
            {
                freeFileDescriptor(fileDescriptor);
                fileDescriptor = NULL;
            }

            // Get a valid file descriptor for the current log file
            off_t seek_value = 0;
            fd = getLogFileDescriptor(gsProfile, logPath, log_file_for_this_iteration, fd, &seek_value);
            prevfile = updateFilename(prevfile, log_file_for_this_iteration);
            if (fd == -1)
            {
                T2Debug("Error in creating file descriptor for file %s\n", log_file_for_this_iteration);
                continue;
            }

            fileDescriptor = getFileDeltaInMemMapAndSearch(fd, seek_value, logPath, log_file_for_this_iteration, check_rotated_logs);
            if (fileDescriptor == NULL)
            {
                T2Error("Failed to get file descriptor for %s\n", log_file_for_this_iteration);
                if (fd != -1)
                {
                    close(fd);
                    fd = -1;
                }
                continue;
            }
        }

        if(tmp_skip_interval <= 0)
        {
            tmp_skip_interval = 0;
        }
        is_skip_param = (profileExecCounter % (tmp_skip_interval + 1) == 0) ? 0 : 1;
        // If skip param is 0, then process the pattern with optimized function
        if (is_skip_param == 0 && fileDescriptor != NULL)
        {

            // Call the optimized function to process the pattern
            processPatternWithOptimizedFunction(grepMarkerObj, fileDescriptor);
            //T2Info("Outer Time Array size = %ld\n", Vector_Size(grepMarkerObj->accumulatedTimestamp));
        }

    }  // Loop of marker list ends here

    if (prevfile != NULL)
    {
        free(prevfile);
        prevfile = NULL;
    }

    if (fd != -1)
    {
        close(fd);
        fd = -1;
    }

    if (fileDescriptor != NULL)
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
int getDCAResultsInVector(GrepSeekProfile *gSeekProfile, Vector * vecMarkerList, bool check_rotated, char* customLogPath)
{
    T2Debug("%s ++in \n", __FUNCTION__);
    int rc = -1;
    if(NULL == gSeekProfile || NULL == vecMarkerList )
    {
        T2Error("Invalid arguments for %s\n", __FUNCTION__);
        return rc;
    }
    pthread_mutex_lock(&dcaMutex);
    if(NULL != vecMarkerList)
    {
        char* logPath = customLogPath ? customLogPath : LOGPATH;

        // Go for looping through the marker list
        if( (rc = parseMarkerListOptimized(gSeekProfile, vecMarkerList, check_rotated, logPath)) == -1 )
        {
            T2Debug("Error in fetching grep results\n");
        }
    }
    pthread_mutex_unlock(&dcaMutex);
    T2Debug("%s --out \n", __FUNCTION__);
    return rc;
}
