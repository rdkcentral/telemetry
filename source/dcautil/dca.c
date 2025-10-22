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
#include <sys/sendfile.h>
#include <sys/file.h>
#include <errno.h>
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

// Define a struct to hold the file descriptor and size with mapping metadata
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
    // Mapping metadata for safer cleanup
    size_t cf_mapped_size;  // Actual size used for mmap
    size_t rf_mapped_size;  // Actual size used for mmap
    off_t cf_original_size; // Original file size at mapping time
    off_t rf_original_size; // Original rotated file size at mapping time
    int lock_fd;            // File descriptor for locking
    bool is_locked;         // Lock status
} FileDescriptor;

/**
 * Portable implementation of strnstr (BSD function).
 * Searches for the first occurrence of the substring 'needle' in the
 * first 'len' bytes of 'haystack'.
 * Returns pointer to the beginning of the match, or NULL if not found.
 */
static const char *strnstr(const char *haystack, const char *needle, size_t len)
{
    if (haystack  == NULL || needle == NULL)
    {
        return NULL;
    }
    size_t needle_len = strlen(needle);
    if (needle_len == 0)
    {
        return haystack;
    }

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

    size_t skip[256];
    for (size_t i = 0; i < 256; ++i)
    {
        skip[i] = needle_len;
    }
    for (size_t i = 0; i < needle_len - 1; ++i)
    {
        skip[(unsigned char)needle[i]] = needle_len - i - 1;
    }

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
    return NULL;
}

cJSON *SEARCH_RESULT_JSON = NULL, *ROOT_JSON = NULL;

static char *LOGPATH = NULL;
static char *PERSISTENTPATH = NULL;
static long PAGESIZE;
static pthread_mutex_t dcaMutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Atomic file operations helper functions
 */

/**
 * Acquire file lock for atomic operations
 * @param fd File descriptor to lock
 * @return 0 on success, -1 on failure
 */
static int acquireFileLock(int fd)
{
    if (fd < 0) return -1;
    
    struct flock lock;
    lock.l_type = F_RDLCK;    // Read lock for shared access
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;           // Lock entire file
    
    int result = fcntl(fd, F_SETLK, &lock);
    if (result == -1) {
        T2Debug("Failed to acquire file lock: %s\n", strerror(errno));
        return -1;
    }
    
    T2Debug("File lock acquired successfully\n");
    return 0;
}

/**
 * Release file lock
 * @param fd File descriptor to unlock
 * @return 0 on success, -1 on failure
 */
static int releaseFileLock(int fd)
{
    if (fd < 0) return -1;
    
    struct flock lock;
    lock.l_type = F_UNLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    
    int result = fcntl(fd, F_SETLK, &lock);
    if (result == -1) {
        T2Debug("Failed to release file lock: %s\n", strerror(errno));
        return -1;
    }
    
    T2Debug("File lock released successfully\n");
    return 0;
}

/**
 * Bounds checking helper function
 * @param addr Base address of mapped memory
 * @param offset Offset to check
 * @param mapped_size Size of mapped memory
 * @param access_size Size of data to access
 * @return true if access is safe, false otherwise
 */
static bool isMemoryAccessSafe(const void* addr, size_t offset, size_t mapped_size, size_t access_size)
{
    if (!addr || mapped_size == 0) {
        T2Error("Invalid memory parameters for bounds check\n");
        return false;
    }
    
    // Check for overflow
    if (offset > mapped_size || access_size > mapped_size) {
        T2Error("Memory access parameters exceed mapped size\n");
        return false;
    }
    
    // Check if offset + access_size would exceed mapped memory
    if (offset + access_size > mapped_size) {
        T2Error("Memory access would exceed mapped region: offset=%zu, access_size=%zu, mapped_size=%zu\n", 
                offset, access_size, mapped_size);
        return false;
    }
    
    return true;
}

/**
 * Safe memory access wrapper with bounds checking
 * @param fileDescriptor File descriptor structure
 * @param offset Offset from base address
 * @param access_size Size of data to access
 * @param is_rotated Whether accessing rotated file data
 * @return Pointer to safe memory location or NULL if unsafe
 */
static const char* safeMemoryAccess(FileDescriptor* fileDescriptor, size_t offset, size_t access_size, bool is_rotated)
{
    if (!fileDescriptor) {
        T2Error("Invalid file descriptor for memory access\n");
        return NULL;
    }
    
    if (is_rotated) {
        if (!fileDescriptor->rfaddr || fileDescriptor->rf_mapped_size == 0) {
            T2Error("No rotated file data available\n");
            return NULL;
        }
        
        if (!isMemoryAccessSafe(fileDescriptor->rfaddr, offset, fileDescriptor->rf_mapped_size, access_size)) {
            return NULL;
        }
        
        return fileDescriptor->rfaddr + offset;
    } else {
        if (!fileDescriptor->cfaddr || fileDescriptor->cf_mapped_size == 0) {
            T2Error("No current file data available\n");
            return NULL;
        }
        
        if (!isMemoryAccessSafe(fileDescriptor->cfaddr, offset, fileDescriptor->cf_mapped_size, access_size)) {
            return NULL;
        }
        
        return fileDescriptor->cfaddr + offset;
    }
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
        if(tmp_skip_interval <= 0)
        {
            tmp_skip_interval = 0;
        }
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
        if(tmp_skip_interval <= 0)
        {
            tmp_skip_interval = 0;
        }
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

static int getCountPatternMatch(FileDescriptor* fileDescriptor, const char* pattern)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if (!fileDescriptor || !pattern || !*pattern)
    {
        T2Error("Invalid arguments for pattern match\n");
        return -1;
    }

    size_t patlen = strlen(pattern);
    int count = 0;

    // Process both current and rotated files with bounds checking
    for(int i = 0; i < 2; i++)
    {
        bool is_rotated = (i == 1);
        const char* buffer = safeMemoryAccess(fileDescriptor, 0, patlen, is_rotated);
        size_t buflen = is_rotated ? fileDescriptor->rf_mapped_size : fileDescriptor->cf_mapped_size;
        
        if (!buffer || buflen == 0)
        {
            T2Debug("No data available for %s file\n", is_rotated ? "rotated" : "current");
            continue;
        }
        
        if (patlen == 0 || buflen < patlen)
        {
            T2Info("File size is less than pattern length, ignoring file\n");
            continue;
        }

        size_t bytes_left = buflen;
        size_t current_offset = 0;

        while (bytes_left >= patlen)
        {
            // Ensure we have safe access for the remaining search
            const char *safe_cur = safeMemoryAccess(fileDescriptor, current_offset, bytes_left, is_rotated);
            if (!safe_cur) {
                T2Error("Unsafe memory access detected, stopping search\n");
                break;
            }
            
            const char *found = strnstr(safe_cur, pattern, bytes_left);
            if (!found)
            {
                break;
            }
            
            count++;
            size_t advance = (size_t)(found - safe_cur) + patlen;
            current_offset += advance;
            
            if (bytes_left < advance || current_offset >= buflen)
            {
                break;
            }
            bytes_left -= advance;
        }
    }
    
    T2Debug("Pattern count: %d\n", count);
    T2Debug("%s --out\n", __FUNCTION__);
    return count;
}

static char* getAbsolutePatternMatch(FileDescriptor* fileDescriptor, const char* pattern)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if (!fileDescriptor || !pattern || !*pattern)
    {
        T2Error("Invalid arguments for absolute pattern match\n");
        return NULL;
    }

    size_t patlen = strlen(pattern);
    const char *last_found = NULL;
    const char *last_buffer = NULL;
    size_t last_buflen = 0;
    bool last_is_rotated = false;

    // Process both current and rotated files with bounds checking
    for (int i = 0; i < 2; i++)
    {
        bool is_rotated = (i == 1);
        const char* buffer = safeMemoryAccess(fileDescriptor, 0, patlen, is_rotated);
        size_t buflen = is_rotated ? fileDescriptor->rf_mapped_size : fileDescriptor->cf_mapped_size;
        
        if (!buffer || buflen == 0)
        {
            T2Debug("No data available for %s file\n", is_rotated ? "rotated" : "current");
            continue;
        }
        
        if (patlen == 0 || buflen < patlen)
        {
            T2Info("File size is less than pattern length, ignoring file\n");
            continue;
        }

        size_t bytes_left = buflen;
        size_t current_offset = 0;

        while (bytes_left >= patlen)
        {
            // Ensure we have safe access for the remaining search
            const char *safe_cur = safeMemoryAccess(fileDescriptor, current_offset, bytes_left, is_rotated);
            if (!safe_cur) {
                T2Error("Unsafe memory access detected, stopping search\n");
                break;
            }
            
            const char *found = strnstr(safe_cur, pattern, bytes_left);
            if (!found)
            {
                break;
            }
            
            last_found = found;
            last_buffer = buffer;
            last_buflen = buflen;
            last_is_rotated = is_rotated;
            
            size_t advance = (size_t)(found - safe_cur) + patlen;
            current_offset += advance;
            
            if (bytes_left < advance || current_offset >= buflen)
            {
                break;
            }
            bytes_left -= advance;
        }

        // If we found a match in current file, don't check rotated file
        if (last_found && i == 0)
        {
            break;
        }
    }

    if (!last_found)
    {
        return NULL;
    }
    
    // Move pointer just after the pattern with bounds checking
    const char *start = last_found + patlen;
    size_t offset_from_base = start - last_buffer;
    
    if (offset_from_base >= last_buflen)
    {
        T2Error("Pattern match extends beyond buffer bounds\n");
        return NULL;
    }
    
    size_t chars_left = last_buflen - offset_from_base;
    
    // Verify safe access for the remaining data
    if (!safeMemoryAccess(fileDescriptor, offset_from_base, chars_left, last_is_rotated))
    {
        T2Error("Unsafe access to remaining data after pattern\n");
        return NULL;
    }

    // Find next newline or end of buffer
    const char *end = memchr(start, '\n', chars_left);
    size_t length = end ? (size_t)(end - start) : chars_left;

    char *result = (char*)malloc(length + 1);
    if (!result)
    {
        T2Error("Failed to allocate memory for result\n");
        return NULL;
    }
    
    memcpy(result, start, length);
    result[length] = '\0';
    T2Debug("%s --out\n", __FUNCTION__);
    return result;
}

static int processPatternWithOptimizedFunction(const GrepMarker* marker, Vector* out_grepResultList, FileDescriptor* filedescriptor)
{
    // Sanitize the input with bounds checking
    if (!marker || !out_grepResultList || !filedescriptor)
    {
        T2Error("Invalid arguments for %s\n", __FUNCTION__);
        return -1;
    }
    
    // Verify we have valid mapped data with bounds checking
    const char* pattern = marker->searchString;
    if (!pattern || !*pattern)
    {
        T2Error("Invalid pattern for processing\n");
        return -1;
    }
    
    size_t pattern_len = strlen(pattern);
    const char* safe_data = safeMemoryAccess(filedescriptor, 0, pattern_len, false);
    if (!safe_data)
    {
        T2Debug("No safe memory access available for pattern matching\n");
        return 0; // Not an error, just no data to process
    }
    // Extract parameters from the marker
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
        T2Debug("The logfile size matches the seek value (%ld) for %s\n", seek_value_from_map, logFile);
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
// Uses safer cleanup with original mapping sizes to prevent corruption
static void freeFileDescriptor(FileDescriptor* fileDescriptor)
{
    if (!fileDescriptor)
    {
        return;
    }
    
    T2Debug("Starting safe cleanup of FileDescriptor\n");
    
    // Release file lock if held
    if (fileDescriptor->is_locked && fileDescriptor->lock_fd >= 0)
    {
        if (releaseFileLock(fileDescriptor->lock_fd) == 0)
        {
            fileDescriptor->is_locked = false;
            T2Debug("File lock released during cleanup\n");
        }
        else
        {
            T2Error("Failed to release file lock during cleanup\n");
        }
    }
    
    // Close lock file descriptor if different from main fd
    if (fileDescriptor->lock_fd >= 0 && fileDescriptor->lock_fd != fileDescriptor->fd)
    {
        close(fileDescriptor->lock_fd);
        fileDescriptor->lock_fd = -1;
    }
    
    // Safely unmap current file using original mapped size
    if (fileDescriptor->baseAddr && fileDescriptor->cf_mapped_size > 0)
    {
        if (munmap(fileDescriptor->baseAddr, fileDescriptor->cf_mapped_size) == -1)
        {
            T2Error("Failed to unmap current file: %s\n", strerror(errno));
        }
        else
        {
            T2Debug("Successfully unmapped current file (%zu bytes)\n", fileDescriptor->cf_mapped_size);
        }
        fileDescriptor->baseAddr = NULL;
        fileDescriptor->cfaddr = NULL;
    }
    
    // Safely unmap rotated file using original mapped size
    if (fileDescriptor->rotatedAddr && fileDescriptor->rf_mapped_size > 0)
    {
        if (munmap(fileDescriptor->rotatedAddr, fileDescriptor->rf_mapped_size) == -1)
        {
            T2Error("Failed to unmap rotated file: %s\n", strerror(errno));
        }
        else
        {
            T2Debug("Successfully unmapped rotated file (%zu bytes)\n", fileDescriptor->rf_mapped_size);
        }
        fileDescriptor->rotatedAddr = NULL;
        fileDescriptor->rfaddr = NULL;
    }
    
    // Close main file descriptor
    if (fileDescriptor->fd >= 0)
    {
        close(fileDescriptor->fd);
        fileDescriptor->fd = -1;
    }
    
    // Clear all fields for safety
    memset(fileDescriptor, 0, sizeof(FileDescriptor));
    fileDescriptor->fd = -1;
    fileDescriptor->lock_fd = -1;
    
    free(fileDescriptor);
    T2Debug("FileDescriptor cleanup completed\n");
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
    
    // Initialize all fields to safe defaults
    memset(fileDescriptor, 0, sizeof(FileDescriptor));
    fileDescriptor->fd = -1;
    fileDescriptor->lock_fd = -1;
    fileDescriptor->is_locked = false;
    
    // Set up atomic file operations with locking
    // Use a duplicate fd for locking to avoid conflicts
    int lock_fd = dup(fd);
    if (lock_fd >= 0)
    {
        if (acquireFileLock(lock_fd) == 0)
        {
            fileDescriptor->lock_fd = lock_fd;
            fileDescriptor->is_locked = true;
            T2Debug("File lock acquired for atomic operations\n");
        }
        else
        {
            T2Debug("Failed to acquire file lock, proceeding without lock\n");
            close(lock_fd);
            fileDescriptor->lock_fd = -1;
        }
    }
    else
    {
        T2Debug("Failed to duplicate fd for locking\n");
        fileDescriptor->lock_fd = -1;
    }
    
    // Set up memory mapping pointers
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
    
    // Set legacy size fields
    fileDescriptor->cf_map_size = main_fsize;
    fileDescriptor->cf_file_size = sb.st_size;
    
    // Set new mapping metadata for safer cleanup
    fileDescriptor->cf_mapped_size = (size_t)sb.st_size;
    fileDescriptor->cf_original_size = sb.st_size;
    
    if(fileDescriptor->rfaddr != NULL)
    {
        fileDescriptor->rf_map_size = rotated_fsize;
        fileDescriptor->rf_file_size = rb.st_size;
        fileDescriptor->rf_mapped_size = (size_t)rb.st_size;
        fileDescriptor->rf_original_size = rb.st_size;
    }
    else
    {
        fileDescriptor->rf_map_size = 0;
        fileDescriptor->rf_file_size = 0;
        fileDescriptor->rf_mapped_size = 0;
        fileDescriptor->rf_original_size = 0;
    }
    
    T2Debug("FileDescriptor initialized with atomic operations and mapping metadata\n");
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
            processPatternWithOptimizedFunction(grepMarkerObj, out_grepResultList, fileDescriptor);
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
