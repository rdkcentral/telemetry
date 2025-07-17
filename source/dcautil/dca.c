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

/**
 * Portable implementation of strnstr (BSD function).
 * Searches for the first occurrence of the substring 'needle' in the
 * first 'len' bytes of 'haystack'.
 * Returns pointer to the beginning of the match, or NULL if not found.
 */
static const char *strnstr(const char *haystack, const char *needle, size_t len)
{
    size_t needle_len;

    if (*needle == '\0')
    {
        return haystack;
    }

    needle_len = strlen(needle);

    if (needle_len == 0)
    {
        return haystack;
    }

    for (size_t i = 0; i + needle_len <= len; i++)
    {
        if (memcmp(haystack + i, needle, needle_len) == 0)
        {
            return haystack + i;
        }
        if (haystack[i] == '\0')
        {
            break;
        }
    }
    return NULL;
}

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

    for (var = 0; var < vCount; ++var){
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
        else{
            
            //ProcessSnapshot *snapshot = createProcessSnapshot();
            #if !defined(ENABLE_RDKC_SUPPORT) && !defined(ENABLE_RDKB_SUPPORT)
            filename = saveTopOutput(profileName);
            #endif
            break;
        }
        T2Debug("topMarkerList[%lu] markerName = %s, logFile = %s, searchString = %s \n", (unsigned long)var, grepMarkerObj->markerName, grepMarkerObj->logFile, grepMarkerObj->searchString);
    }
    // TODO Generate the top output file - should be profile specific and thread safe
    
    // If the header contains "Load_Average", it calls getLoadAvg() to retrieve load average data.
    // If the header does not contain "Load_Average", it checks if the pattern field is present in the top out put snapshot.
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
            // This block is for process level usage
            // TODO - Move this to a separate function which adds the results to the out_grepResultList
            /*ProcessInfo *info = lookupProcess(snapshot, grepMarkerObj->markerName);
            if (info) {
                printf("Process found: PID=%d, Name=%s, Mem=%s, CPU=%s\n",
                    info->pid, info->processName, info->memUsage, info->cpuUsage);
            } else {
                printf("Process %s not found\n", grepMarkerObj->markerName);
            }*/

            getProcUsage(grepMarkerObj->searchString, out_grepResultList, grepMarkerObj->trimParam, grepMarkerObj->regexParam, filename);
        }

    }

    // TODO Clear the top output file
    //freeProcessSnapshot(snapshot);
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
        free(previousFile);
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
    grepResult->markerName = strdup(markerName);
    grepResult->markerValue = strdup(markerValue);
    grepResult->trimParameter = trimParameter;
    if (regexParameter != NULL)
    {
        grepResult->regexParameter = strdup(regexParameter);
    }
    else
    {
        grepResult->regexParameter = NULL;
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
/*
static int getCountPatternMatch(const char* buffer, const char* pattern) {

    if (!buffer || !pattern) {
        return -1; // Invalid arguments
    }

   // Search for number of matches of specific string in the file
   // const char *search_str = "Induced log entries from test";
   char *found = strstr(buffer, pattern);
   // Capture the last line that finds a match
   int count = 0;
   while (found) {
       count++;
       found = strstr(found + 1, pattern);
    }
    return count;

}*/

static int getCountPatternMatch(FileDescriptor* fileDescriptor, const char* pattern)
{
    if (!fileDescriptor || !fileDescriptor->cfaddr || !pattern || !*pattern || fileDescriptor->cf_file_size <= 0)
    {
        T2Error("Invalid file descriptor arguments pattern match\n");
        return -1; // Invalid arguments
    }

    const char* buffer;
    size_t buflen = 0;
    size_t patlen = strlen(pattern);
    int count = 0;

    for(int i = 0; i < 2; i++)
    {
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
    T2Info(" count is %d\n", count);
    return count;
}

/*

static char* getAbsolutePatternMatch(const char* buffer, const char* pattern) {

    if (!buffer || !pattern) {
        return NULL; // Invalid arguments
    }

   char *found = strstr(buffer, pattern);
   char *last_found = NULL ;
   // Capture the last line that finds a match
   while (found)
   {
       last_found = found;
       found = strstr(found + 1, pattern);
  }

    if (last_found)
    {
        // Exclude the pattern from the results
        last_found = last_found + strlen(pattern);
        char *end_of_line = strchr(last_found, '\n');
        if (end_of_line)
        {
            size_t length = end_of_line - last_found;
            char *result = (char*)malloc(length + 1);
            if (result)
            {
                strncpy(result, last_found, length);
                result[length] = '\0';
                return result;
            }
        }
    } else {
        T2Error("Pattern not found in the buffer\n");
        last_found = NULL;
    }


   return last_found;
}
 */

static char* getAbsolutePatternMatch(FileDescriptor* fileDescriptor, const char* pattern)
{
    if (!fileDescriptor || !fileDescriptor->cfaddr || fileDescriptor->cf_file_size <= 0 || !pattern || !*pattern)
    {
        T2Error("Invalid file descriptor arguments absolute\n");
        return NULL;
    }

    const char* buffer;
    size_t buflen = 0;
    size_t patlen = strlen(pattern);
    const char *last_found = NULL;

    for ( int i = 0; i < 2; i++ )
    {
        if (i == 0)
        {
            buffer = fileDescriptor->cfaddr;
            T2Info("current file for absolute\n");
            buflen = (size_t)fileDescriptor->cf_file_size;
        }
        else
        {
            buffer = fileDescriptor->rfaddr;
            T2Info("Rotated file for absolute\n");
            buflen = (size_t)fileDescriptor->rf_file_size;
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
        T2Info("NULL value for absolute\n");
        return NULL;
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
        return NULL;
    }
    memcpy(result, start, length);
    result[length] = '\0';
    T2Info("result is %s\n", result);
    return result;
}

static int processPatternWithOptimizedFunction(const GrepMarker* marker, Vector* out_grepResultList, FileDescriptor* filedescriptor)
{
    // Sanitize the input

    const char* memmmapped_data_cf = filedescriptor->cfaddr;
    const char* memmmapped_data_rf = filedescriptor->rfaddr;
    if (!marker || !out_grepResultList || !memmmapped_data_cf)
    {
        T2Error("Invalid arguments for %s\n", __FUNCTION__);
        return -1;
    }
    if( !memmmapped_data_cf && !memmmapped_data_rf)
    {
        return -1;
    }
    // Extract the pattern and other parameters from the marker
    const char* pattern = marker->searchString;
    bool trimParameter = marker->trimParam;
    char* regexParameter = marker->regexParam;
    char* header = marker->markerName;
    int count = 0;
    char* last_found = NULL;
    MarkerType mType = marker->mType;

    if (mType == MTYPE_COUNTER)
    {
        // Count the number of occurrences of the pattern in the memory-mapped data
        count = getCountPatternMatch(filedescriptor, pattern);
        if (count > 0)
        {
            // If matches are found, process them accordingly
            char tmp_str[5] = { 0 };
            formatCount(tmp_str, sizeof(tmp_str), count);
            GrepResult* result = createGrepResultObj(header, tmp_str, trimParameter, regexParameter);
            if (result == NULL)
            {
                T2Error("Failed to create GrepResult\n");
                return -1;
            }
            Vector_PushBack(out_grepResultList, result);
        }
    }
    else
    {
        // Get the last occurrence of the pattern in the memory-mapped data
        last_found = getAbsolutePatternMatch(filedescriptor, pattern);
        // TODO : If trimParameter is true, trim the pattern before adding to the result list
        if (last_found)
        {
            // If a match is found, process it accordingly
            GrepResult* result = createGrepResultObj(header, last_found, trimParameter, regexParameter);
            if(last_found)
            {
                free(last_found);
                last_found = NULL;
            }
            if (result == NULL)
            {
                T2Error("Failed to create GrepResult\n");
                return -1;
            }
            Vector_PushBack(out_grepResultList, result);
        }
    }
    return 0;
}


static int getLogFileDescriptor(GrepSeekProfile* gsProfile, const char* logPath, const char* logFile, int old_fd, off_t* out_seek_value)
{
    long seek_value_from_map = 0;
    printf("address of logFileSeekMap: %p\n", gsProfile->logFileSeekMap);
    getLogSeekValue(gsProfile->logFileSeekMap, logFile, &seek_value_from_map);
    if (old_fd != -1)
    {
        close(old_fd);
    }

    char logFilePath[PATH_MAX];
    if(logFile[0]=='/'){
        // If the logFile is an absolute path, use it directly
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
    if(name_len > 2 && logFilePath[name_len - 2] == '.' && logFilePath[name_len - 1] == '0')
    {
        snprintf(rotatedlogFilePath, sizeof(rotatedlogFilePath), "%s/%s", logPath, logFile);
        rotatedlogFilePath[name_len - 1] = '1';
        T2Info("Log file name seems to be having .0 extension hence Rotated log file name is %s\n", rotatedlogFilePath);
    }
    else
    {
        snprintf(rotatedlogFilePath, sizeof(rotatedlogFilePath), "%s/%s%s", logPath, logFile, fileExtn);
        T2Info("Rotated log file name is %s\n", rotatedlogFilePath);
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
        munmap(fileDescriptor->baseAddr, fileDescriptor->cf_file_size);
        if(fileDescriptor->rotatedAddr)
        {
            munmap(fileDescriptor->rotatedAddr, fileDescriptor->rf_file_size);
        }
        close(fileDescriptor->fd);
        free(fileDescriptor);
    }
}
/*
static size_t align_up(size_t x, size_t page_size)
{
    return ((x + page_size - 1) / page_size) * page_size;
}

static void *map_files_rotated(int fd_main, int fd_rotated, size_t size_main, size_t size_rotated, off_t rotated_offset)
{
    size_t total_size = size_main + size_rotated;
    T2Info("Before conversion fd_main = %d, fd_rotated = %d, size_main = %zu, size_rotated = %zu, rotated_offset = %ld\n",
           fd_main, fd_rotated, size_main, size_rotated, rotated_offset);
    size_main = align_up(size_main, PAGESIZE);
    size_rotated = align_up(size_rotated, PAGESIZE);
    total_size = align_up(total_size, PAGESIZE);

    T2Info("fd_main = %d, fd_rotated = %d, size_main = %zu, size_rotated = %zu, rotated_offset = %ld\n",
           fd_main, fd_rotated, size_main, size_rotated, rotated_offset);
    void *base_addr = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (base_addr == MAP_FAILED)
    {
        T2Error("Error allocating memory region");
        return NULL;
    }

    // Map the first file into the allocated region
    void *addr1 = mmap(base_addr, size_rotated, PROT_READ, MAP_PRIVATE | MAP_FIXED, fd_rotated, rotated_offset);
    if (addr1 == MAP_FAILED)
    {
        T2Error("Error mapping rotated file");
        munmap(base_addr, total_size); // Clean up
        return NULL;
    }

    // Map the second file into the allocated region, right after the first file
    void *addr2 = mmap(base_addr + size_rotated, size_main, PROT_READ, MAP_PRIVATE | MAP_FIXED, fd_main, 0);
    if (addr2 == MAP_FAILED)
    {
        T2Error("Error mapping file main: %s\n", strerror(errno));
        munmap(base_addr, total_size); // Clean up
        return NULL;
    }
    T2Info("loaded the files in address map\n");
    close(fd_rotated);
    return base_addr;
}
*/

static FileDescriptor* getFileDeltaInMemMapAndSearch(const int fd, const off_t seek_value, const char* logPath, const char* logFile, bool check_rotated )
{
    char *addrcf = NULL;
    char *addrrf = NULL;
    if (fd == -1)
    {
        T2Error("Error opening file\n");
        return NULL;
    }
    // Read the file contents using mmap
    struct stat sb;
    struct stat rb;
    if(fstat(fd, &sb) == -1)
    {
        T2Error("Error getting file size\n");
        return NULL;
    }

    if(sb.st_size == 0)
    {
        T2Error("The Size of the logfile is 0\n");
        return NULL;
    }

    FileDescriptor* fileDescriptor = NULL;
    off_t offset_in_page_size_multiple ;
    unsigned int bytes_ignored = 0, bytes_ignored_main = 0, bytes_ignored_rotated = 0;
    size_t size_main = 0, size_rotated = 0;
    size_main = sb.st_size;
    // Find the nearest multiple of page size
    if (seek_value > 0)
    {
        offset_in_page_size_multiple = (seek_value / PAGESIZE) * PAGESIZE;
        bytes_ignored = seek_value - offset_in_page_size_multiple;
    }
    else
    {
        offset_in_page_size_multiple = 0;
        bytes_ignored = 0;
    }
    if(seek_value < sb.st_size)
    {
	T2Info("check rotated %d\n",check_rotated);
        if(check_rotated == true)
        {
            int rd = getRotatedLogFileDescriptor(logPath, logFile);
            if (rd == -1)
            {
                T2Error("Error opening rotated file. Start search in current file\n");
                T2Info("File size rounded to nearest page size used for offset read: %ld bytes\n", offset_in_page_size_multiple);
                // addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, offset_in_page_size_multiple);
                addrcf = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, offset_in_page_size_multiple);
                bytes_ignored_main = bytes_ignored;
            }
            else
            {
                int fs = 0;
                fs = fstat(rd, &rb);
                if(fs == -1)
                {
                    T2Error("Error getting file size\n");
                }
                else
                {
                    if(rb.st_size == 0)
                    {
                        T2Error("The Size of the logfile is 0\n");
                    }
                }

                if(rb.st_size > 0)
                {
                    size_main = sb.st_size;
                    size_rotated = rb.st_size - seek_value;
                    T2Info("main = %zu, rotated = %zu, seek_value = %lu\n", size_main, size_rotated, seek_value);
                    //addr =  map_files_rotated(fd, rd, size_main, size_rotated, offset_in_page_size_multiple);
                    addrcf = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
                    addrrf = mmap(NULL, rb.st_size, PROT_READ, MAP_PRIVATE, rd, offset_in_page_size_multiple);
                    bytes_ignored_rotated = bytes_ignored;
                    close(rd);
                }

                if(rb.st_size == 0 || fs == -1)
                {
                    T2Info("File size rounded to nearest page size used for offset read: %ld bytes\n", offset_in_page_size_multiple);
                    // addr =  mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, offset_in_page_size_multiple);
                    addrcf = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, offset_in_page_size_multiple);
                    bytes_ignored_main = bytes_ignored;
                }
            }
        }
        else
        {
            T2Info("File size rounded to nearest page size used for offset read: %ld bytes\n", offset_in_page_size_multiple);
            //addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, offset_in_page_size_multiple);
            addrcf = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, offset_in_page_size_multiple);
            bytes_ignored_main = bytes_ignored;
        }
    }

    if(seek_value > sb.st_size)
    {
        int rd = getRotatedLogFileDescriptor(logPath, logFile);
        if (rd == -1)
        {
            T2Error("Error opening rotated file. Start search in current file\n");
            T2Info("File size rounded to nearest page size used for offset read: %ld bytes\n", offset_in_page_size_multiple);
            //addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, offset_in_page_size_multiple);
            addrcf = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, offset_in_page_size_multiple);
            bytes_ignored_main = bytes_ignored;
        }
        else
        {
            int fs = 0;
            fs = fstat(rd, &rb);
            if(fs == -1)
            {
                T2Error("Error getting file size\n");
            }
            else
            {
                if(rb.st_size == 0)
                {
                    T2Error("The Size of the logfile is 0\n");
                }
            }

            if(rb.st_size > 0)
            {
                size_main = sb.st_size;
                size_rotated = rb.st_size - seek_value;
                T2Info("main = %zu, rotated = %zu, seek_value = %lu\n", size_main, size_rotated, seek_value);
                //addr =  map_files_rotated(fd, rd, size_main, size_rotated, offset_in_page_size_multiple);
                addrcf = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
                addrrf = mmap(NULL, rb.st_size, PROT_READ, MAP_PRIVATE, rd, offset_in_page_size_multiple);
                bytes_ignored_rotated = bytes_ignored;
                close(rd);

            }

            if(rb.st_size == 0 && fs == -1)
            {
                T2Debug("No contents in rotated log file. File size rounded to nearest page size used for offset read: %ld bytes\n", offset_in_page_size_multiple);
                addrcf = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, offset_in_page_size_multiple);
                bytes_ignored_main = bytes_ignored;
            }
        }
    }

    close(fd);

    if (addrcf == MAP_FAILED)
    {
        T2Error("Error in memory mapping file %d: %s\n", fd, strerror(errno));
        return NULL;
    }
    if (addrrf == MAP_FAILED)
    {
        T2Error("Error in memory mapping file %d: %s\n", fd, strerror(errno));
        return NULL;
    }
    fileDescriptor = malloc(sizeof(FileDescriptor));
    if (!fileDescriptor)
    {
        T2Error("Error allocating memory for FileDescriptor\n");
        return NULL;
    }
    memset(fileDescriptor, 0, sizeof(FileDescriptor));
    // addr needs to ignore the first bytes_ignored bytes
    /* fileDescriptor->baseAddr = (void *)addr;
     addr += bytes_ignored;
     fileDescriptor->addr = addr;
     */
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
    fileDescriptor->cf_file_size = sb.st_size;
    if(fileDescriptor->rfaddr != NULL)
    {
        fileDescriptor->rf_file_size = rb.st_size;
    }
    else
    {
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
    //GrepSeekProfile* gsProfile = NULL;
    size_t var = 0;
    size_t vCount = Vector_Size(ip_vMarkerList);
    //T2Debug("vMarkerList for profile %s is of count = %lu \n", profileName, (unsigned long )vCount);

    // Get logfile -> seek value map associated with the profile
    //gsProfile = (GrepSeekProfile *) getLogSeekMapForProfile(profileName);
    if(NULL == gsProfile)
    {
        T2Error("%s Unable to retrieve/create logSeekMap for profile \n", __FUNCTION__);
        return -1;
    }
    printf("address of logFileSeekMap: %p\n", gsProfile->logFileSeekMap);
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
            if (fd == -1)
            {
                T2Error("Error opening file %s\n", log_file_for_this_iteration);
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
            prevfile = updateFilename(prevfile, log_file_for_this_iteration);

        }

        if(tmp_skip_interval <= 0)
        {
            tmp_skip_interval = 0;
        }
        is_skip_param = (profileExecCounter % (tmp_skip_interval + 1) == 0) ? 0 : 1;
        // If skip param is 0, then process the pattern with optimized function
        if (is_skip_param == 0)
        {
            // Call the optimized function to process the pattern
            processPatternWithOptimizedFunction(grepMarkerObj, out_grepResultList, fileDescriptor);
        }

    }  // Loop of marker list ends here


    gsProfile->execCounter += 1;
    T2Debug("Execution Count = %d\n", gsProfile->execCounter);

    if (prevfile != NULL)
    {
        free(prevfile);
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
