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
#include <sys/stat.h>

#include <cjson/cJSON.h>

#include "dcalist.h"
#include "dcautil.h"
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
// safeclib is not compleately introduced in T2 but to be in sync with legacy
#define INVALID_COUNT -406

#define BUFFER_SIZE 4096  // TODO fine tune this value based on the size of the data
#define LARGE_FILE_THRESHOLD 1000000 // 1MB

/**
 * @addtogroup DCA_TYPES
 * @{
 */



cJSON *SEARCH_RESULT_JSON = NULL, *ROOT_JSON = NULL;

static char *logPath = NULL;
static char *persistentPath = NULL;
static pthread_mutex_t dcaMutex = PTHREAD_MUTEX_INITIALIZER;

#if !defined(ENABLE_RDKC_SUPPORT) && !defined(ENABLE_RDKB_SUPPORT)
static pthread_mutex_t topOutputMutex = PTHREAD_MUTEX_INITIALIZER;
#endif
/* @} */ // End of group DCA_TYPES
/**
 * @addtogroup DCA_APIS
 * @{
 */

/** @brief This API processes the top command log file patterns to retrieve load average and process usage.
 *
 *  @param[in] logfile  top_log file
 *  @param[in] pchead   Node  head
 *  @param[in] pcIndex  Node  count
 *
 *  @return  Returns the status of the operation.
 *  @retval  Returns zero on success, appropriate errorcode otherwise.
 */
int processTopPattern(rdkList_t *pchead, Vector* grepResultList)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if(pchead == NULL || grepResultList == NULL)
    {
        T2Error("Invalid arguments for %s\n", __FUNCTION__);
        return -1;
    }
    rdkList_t *tlist = pchead;
    pcdata_t *tmp = NULL;
    while(NULL != tlist)
    {
        tmp = tlist->m_pUserData;
        if(NULL != tmp)
        {
            if((NULL != tmp->header) && (NULL != strstr(tmp->header, "Load_Average")))
            {
                if(0 == getLoadAvg(grepResultList, tmp->trimparam, tmp->regexparam))
                {
                    T2Debug("getLoadAvg() Failed with error");
                }
            }
            else
            {
                if(NULL != tmp->pattern)
                {
                    // save top output
#if !defined(ENABLE_RDKC_SUPPORT) && !defined(ENABLE_RDKB_SUPPORT)
                    pthread_mutex_lock(&topOutputMutex);
                    saveTopOutput();
                    getProcUsage(tmp->pattern, grepResultList, tmp->trimparam, tmp->regexparam);
                    pthread_mutex_unlock(&topOutputMutex);
#else
                    getProcUsage(tmp->pattern, grepResultList, tmp->trimparam, tmp->regexparam);
#endif

                }
            }
        }
        tlist = rdk_list_find_next_node(tlist);
    }

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
 * @brief This API updates the filename if it is different from the current one.
 * @param currentFile The current filename.
 * @param newFile The new filename to update to.
 * @return The updated filename.
 */

static char* updateFilename(char* previousFile, const char* newFile)
{
    if (previousFile == NULL || strcmp(previousFile, newFile) != 0)
    {
        if (previousFile != NULL)
        {
            free(previousFile);
        }
        previousFile = strdup(newFile);
        if (previousFile == NULL)
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


#if 0
/**
 * @brief Function to process pattern if it has split text in the header
 *
 * @param[in] line    Log file matched line
 * @param[in] pcnode  Pattern to be verified.
 *
 * @return Returns status of operation.
 * @retval Return 0 on success, -1 on failure
 */
static int getSplitParameterValue(char *line, pcdata_t *pcnode)
{
    // Input validation
    if (!line || !pcnode || !pcnode->pattern)
    {
        T2Error("Invalid arguments for %s\n", __FUNCTION__);
        return -1;
    }

    // Use const for pattern to prevent accidental modifications
    const char *pattern = pcnode->pattern;
    const size_t pattern_len = strlen(pattern);

    // Find pattern in line
    const char *value_start = strstr(line, pattern);
    if (!value_start)
    {
        return 0;  // Pattern not found
    }

    // Calculate value position and length
    value_start += pattern_len;
    const size_t line_len = strlen(line);
    const size_t value_len = line_len - (value_start - line);

    // Skip if value is empty or just whitespace
    if (value_len <= 1 || (value_len == 1 && isspace(*value_start)))
    {
        return 0;
    }

    // Allocate memory only if needed
    if (!pcnode->data)
    {
        pcnode->data = malloc(MAXLINE);
        if (!pcnode->data)
        {
            T2Error("Memory allocation failed for split parameter value\n");
            return -1;
        }
    }

    // Copy value with bounds checking
    size_t copy_len = value_len < MAXLINE - 1 ? value_len : MAXLINE - 1;
    memcpy(pcnode->data, value_start, copy_len);
    pcnode->data[copy_len] = '\0';

    return 0;

}

/**
 * @brief Function to extract RDK error code from a string.
 *
 * @param[in]  str    Source string containing RDK error code
 * @param[out] ec     Buffer to store the extracted error code
 *
 * @return Returns 0 on success, -1 on failure
 */
int getErrorCode(const char *str, char *ec)
{
    T2Debug("%s ++in\n", __FUNCTION__);

    // Input validation
    if (!str || !ec)
    {
        T2Error("Invalid arguments: str=%p, ec=%p\n", (void*)str, (void*)ec);
        return -1;
    }

    // Use const for better optimization
    static const char RDK_PREFIX[] = "RDK-";
    const size_t PREFIX_LEN = sizeof(RDK_PREFIX) - 1;
    const char *ptr = str;

    // Find "RDK-" prefix
    while ((ptr = strstr(ptr, RDK_PREFIX)) != NULL)
    {
        ptr += PREFIX_LEN;  // Check if line has strings after "RDK-"
        if (*ptr != '0' && *ptr != '1')
        {
            ptr++;
            continue;
        }

        if (ptr[1] != '0' && ptr[1] != '3')
        {
            ptr++;
            continue;
        }

        // Validate remaining digits
        const char *digit_start = ptr + 2;
        size_t digit_count = 0;

        while (isdigit((unsigned char)digit_start[digit_count]) &&
                digit_count < RDK_EC_MAXLEN - 2)
        {
            digit_count++;
        }

        if (digit_count > 0)
        {
            // Copy the error code: first two digits + remaining digits
            memcpy(ec, ptr, 2 + digit_count);
            ec[2 + digit_count] = '\0';
            T2Debug("%s --out Success\n", __FUNCTION__);
            return 0;
        }

        ptr++;
    }

    // No valid error code found
    ec[0] = '\0';
    T2Debug("%s --out No valid error code found\n", __FUNCTION__);
    return 0;
}

/**
 * @brief Function to handle error codes received from the log file.
 *
 * @param[in]  rdkec_head  Node head.
 * @param[in]  line        Logfile matched line.
 *
 * @return Returns status of operation.
 * @retval Return 0 upon success, -1 on failure.
 * TODO: This was a special case for RDK-V. Need to check if this is still needed or atleast restrict to certain files.
 * This function should be moved to a more appropriate location if it is still required and exclude checking for each line.
 */
static int handleRDKErrCodes(rdkList_t **rdkec_head, char *line)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    char err_code[20] = { 0 }, rdkec[30] = { 0 };
    pcdata_t *tnode = NULL;

    getErrorCode(line, err_code);
    if(strcmp(err_code, "") != 0)
    {
        snprintf(rdkec, sizeof(rdkec), "RDK-%s", err_code);
        tnode = searchPCNode(*rdkec_head, rdkec);
        if(NULL != tnode)
        {
            tnode->count++;
        }
        else
        {
            /* Args:  rdkList_t **pch, char *pattern, char *header, DType_t dtype, int count, char *data, bool trim, char *regex */
            insertPCNode(rdkec_head, rdkec, rdkec, OCCURENCE, 1, NULL, false, NULL);
        }
        T2Debug("%s --out\n", __FUNCTION__);
        return 0;
    }
    T2Debug("%s --out Error .... \n", __FUNCTION__);
    return -1;
}

#endif

#if 0
static int processCountPatternOptimized(hash_map_t *logSeekMap, char *logfile,
                                        rdkList_t *pchead, rdkList_t **rdkec_head,
                                        int *firstSeekFromEOF, bool check_rotated_logs)
{

    size_t adaptive_buffer_size = BUFFER_SIZE;
    char line[MAXLINE];
    size_t line_pos = 0;
    FILE *fp;
    double cpuUsage = getSystemCPUUsage();
    double memoryUsage = getSystemMemoryUsage();


    T2Debug("System CPU Usage: %.2f%%\n", cpuUsage);
    T2Debug("System Memory Usage: %.2f%%\n", memoryUsage);

    if (memoryUsage > 80.0)
    {
        adaptive_buffer_size = BUFFER_SIZE / 2; // Reduce buffer size for high memory usage
        T2Info("High memory usage detected, using smaller buffer size %zu\n", adaptive_buffer_size);
    }


    if (memoryUsage < 50.0 && sb.st_size > LARGE_FILE_THRESHOLD)
    {
        return processWithMMapSafe(logfile, pchead);
    }
    else
    {
        return processWithBufferedIO(logfile, pchead);
    }

    char buffer[adaptive_buffer_size];
    // Logic for for rotated file and update log seek needs to be retained from getLogLine
    if ((fp = fopen(logfile, "r")) == NULL)
    {
        T2Error("Failed to open file %s\n", logfile);
        return -1;
    }


// Process file in chunks
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, fp)) > 0)
    {
        for (size_t i = 0; i < bytes_read; i++)
        {
            if (buffer[i] == '\n' || line_pos >= MAXLINE - 1)
            {
                line[line_pos] = '\0';

                // Pattern match check - Without any algorithms -
                // This is a simple linear search for the pattern in the line
                // This can be optimized further if needed
                pcdata_t *pc_node = searchPCNode(pchead, line);
                if (pc_node != NULL)
                {
                    if (pc_node->d_type == OCCURENCE)
                    {
                        pc_node->count++;
                    }
                    else if (pc_node->header != NULL)
                    {
                        getSplitParameterValue(line, pc_node);
                    }
                }
                else if (strstr(line, "RDK-"))
                {
                    handleRDKErrCodes(rdkec_head, line);
                }

                line_pos = 0;
            }
            else
            {
                line[line_pos++] = buffer[i];
            }
        }
    }

    fclose(fp);
    return 0;
}
#endif



static int get_count_of_matched_pattern(const char* buffer, const char* pattern) {

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
    
    printf("==============================\n");
    printf("Number of matches for string '%s': %d\n", pattern, count);
    printf("==============================\n");
    return count;

}

static char* get_value_of_matched_pattern(const char* buffer, const char* pattern) {
    char *found = strstr(buffer, pattern);
    char *last_found = NULL ;
    // Capture the last line that finds a match 
    while (found) {
        last_found = found ;
        found = strstr(found + 1, pattern);
    }

    printf("==============================\n");
    if ( last_found != NULL ){
        printf("Last line :\n %s \n", last_found );
    } else {
        printf("No matches were found for pattern : %s \n", pattern);
    }
    printf("==============================\n");
    return last_found;
}
 

static int processPatternWithOptimizedFunction(const GrepMarker* marker, Vector* out_grepResultList, const char* memmmapped_data) {
     // Sanitize the input
    if (!marker || !out_grepResultList || !memmmapped_data) {
        T2Error("Invalid arguments for %s\n", __FUNCTION__);
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

    if (mType == MTYPE_COUNTER) {
        // Count the number of occurrences of the pattern in the memory-mapped data
        count = get_count_of_matched_pattern(memmmapped_data, pattern);
        if (count > 0) {
            // If matches are found, process them accordingly
            char tmp_str[5] = { 0 };
            formatCount(tmp_str, sizeof(tmp_str), count);
            GrepResult* result = createGrepResultObj(header, tmp_str, trimParameter, regexParameter);
            if (result == NULL) {
                T2Error("Failed to create GrepResult\n");
                return -1;
            }
            Vector_PushBack(out_grepResultList, result);
        }
    } else {
        // Get the last occurrence of the pattern in the memory-mapped data
        last_found = get_value_of_matched_pattern(memmmapped_data, pattern);
        // TODO : If trimParameter is true, trim the pattern before adding to the result list
        if (last_found) {
            // If a match is found, process it accordingly
            GrepResult* result = createGrepResultObj(header,last_found, trimParameter, regexParameter);
            if (result == NULL) {
                T2Error("Failed to create GrepResult\n");
                return -1;
            }
            Vector_PushBack(out_grepResultList, result);
        } 
    }

    return 0;
}





/**
 * @brief Function like strstr but based on the string delimiter.
 *
 * @param[in] str    String.
 * @param[in] delim  Delimiter.
 *
 * @return Returns the output string.
 */
char *strSplit(char *str, char *delim)
{
    static char *next_str;
    char *last = NULL;
    if(str != NULL)
    {
        next_str = str;
    }

    if(NULL == next_str)
    {
        return next_str;
    }

    last = strstr(next_str, delim);
    if(NULL == last)
    {
        char *ret = next_str;
        next_str = NULL;
        return ret;
    }

    char *ret = next_str;
    *last = '\0';
    next_str = last + strlen(delim);
    return ret;
}

/**
 * @brief To get node data type based on pattern.
 *
 * @param[in]  filename   Conf filename.
 * @param[in]  header     node header.
 * @param[out] dtype      Data type
 *
 * @return Returns status of operation.
 */
void getDType(char *filename, MarkerType mType, DType_t *dtype)
{
    if (mType != MTYPE_COUNTER)
    {
        *dtype = STR;
    }
    else if(0 == strcmp(filename, "top_log.txt") || 0 == strcmp(filename, "<message_bus>"))
    {
        *dtype = STR;
    }
    else
    {
        *dtype = OCCURENCE;
    }
}


static int getLogFileDescriptor(GrepSeekProfile* gsProfile, const char* logFile, int old_fd, off_t* out_seek_value) {
    long seek_value_from_map = 0;
    getLogSeekValue(gsProfile->logFileSeekMap, logFile, &seek_value_from_map);
    updateLogSeek(gsProfile->logFileSeekMap, logFile);
    if (old_fd != -1) {
        close(old_fd);
    }
    int fd = open(logFile, O_RDONLY);
    if (fd == -1) {
        T2Error("Failed to open log file %s\n", logFile);
    }
    *out_seek_value = seek_value_from_map;
    return fd;
}

// Define a struct to hold the file descriptor and size
typedef struct {
    int fd;
    off_t file_size;
    char* addr;
} FileDescriptor;

// Caller should free the FileDescriptor struct after use
static void freeFileDescriptor(FileDescriptor* fileDescriptor) {
    if (fileDescriptor) {
        munmap(fileDescriptor->addr, fileDescriptor->file_size);
        close(fileDescriptor->fd);
        free(fileDescriptor);
    }
}

static FileDescriptor* getFileDeltaInMemMapAndSearch(const int fd , const off_t seek_value) {
       if (fd == -1) {
           perror("Error opening file");
           return NULL;
       }
       // Read the file contents using mmap
       struct stat sb;
       if(fstat(fd, &sb) == -1) {
           perror("Error getting file size");
           return NULL;
       }

       FileDescriptor* fileDescriptor = NULL;

       printf("File size: %ld bytes\n", sb.st_size);

       if (sb.st_size > seek_value) {
           printf("File has grown compared to previous lookup \n");
       } else {
           printf("File has not grown compared to previous lookup \n");
       }
       // Check if the file size is a multiple of the page size
   
       off_t file_size_offset ;
       int bytes_ignored = 0;

       // Find the nearest multiple of page size
       long page_size = sysconf(_SC_PAGESIZE);
       bytes_ignored = seek_value % page_size;
       if (bytes_ignored > 0) {
           printf("File size is not a multiple of page size. Ignoring %d bytes and Rounding up to nearest page size \n", bytes_ignored);
           file_size_offset = (seek_value / page_size) * page_size;
       } else {
           file_size_offset = sb.st_size;
       }
       printf("File size rounded to nearest page size: %ld bytes\n", file_size_offset);
   
       char *addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, file_size_offset);
       close(fd);

       if (addr == MAP_FAILED) {
           perror("Error mapping file");
           return NULL;
       }
       fileDescriptor = malloc(sizeof(FileDescriptor));
       if (!fileDescriptor) {
           perror("Error allocating memory");
           return NULL;
       }
       memset(fileDescriptor, 0, sizeof(FileDescriptor));
       fileDescriptor->addr = addr;
       fileDescriptor->fd = fd;
       fileDescriptor->file_size = sb.st_size;
   
       // addr needs to ignore the first bytes_ignored bytes
       addr += bytes_ignored;
       
       return fileDescriptor;
}

// Call 2
/** @description: Main logic function to parse sorted vector list and to process the pattern list
 *  @param filename
 *  @return -1 on failure, 0 on success
 */
static int parseMarkerListOptimized(char* profileName, Vector* ip_vMarkerList, Vector* out_grepResultList, bool check_rotated)
{
    T2Debug("%s ++in \n", __FUNCTION__);

    if(NULL == profileName || NULL == ip_vMarkerList || NULL == out_grepResultList)
    {
        T2Error("Invalid arguments for %s\n", __FUNCTION__);
        return -1;
    }

    char *prevfile = NULL;
    GrepSeekProfile* gsProfile = NULL;
    size_t var = 0;
    size_t vCount = Vector_Size(ip_vMarkerList);
    T2Debug("vMarkerList for profile %s is of count = %lu \n", profileName, (unsigned long )vCount);

    // Get logfile -> seek value map associated with the profile
    gsProfile = (GrepSeekProfile *) getLogSeekMapForProfile(profileName);
    if(NULL == gsProfile && (gsProfile = (GrepSeekProfile *) addToProfileSeekMap(profileName)) == NULL)
    {
        T2Error("%s Unable to retrieve/create logSeekMap for profile %s \n", __FUNCTION__, profileName);
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
    char *buffer = NULL;
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
            }

            if (fd != -1) {
                close(fd);
                fd = -1;
            }

            if (fileDescriptor != NULL) {
                freeFileDescriptor(fileDescriptor);
                fileDescriptor = NULL;
            }

            // Get a valid file descriptor for the current log file
            off_t seek_value = 0;
            fd = getLogFileDescriptor(gsProfile, log_file_for_this_iteration, fd, &seek_value);
            if (fd == -1) {
                continue;
            }
            prevfile = updateFilename(prevfile, log_file_for_this_iteration);
            fileDescriptor = getFileDeltaInMemMapAndSearch(fd, seek_value);
            if (fileDescriptor == NULL) {
                T2Error("Failed to get file descriptor for %s\n", log_file_for_this_iteration);
                continue;
            }
            buffer = fileDescriptor->addr;

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
            processPatternWithOptimizedFunction(grepMarkerObj, out_grepResultList, buffer);
        }

    }  // Loop of marker list ends here 


    gsProfile->execCounter += 1;
    T2Debug("Execution Count = %d\n", gsProfile->execCounter);

    if (prevfile != NULL)
    {
        free(prevfile);
    }

    if (fd != -1) {
        close(fd);
        fd = -1;
    }

    if (fileDescriptor != NULL) {
        freeFileDescriptor(fileDescriptor);
        fileDescriptor = NULL;
    }

    T2Debug("%s --out \n", __FUNCTION__);
    return 0;
}




// Call 1 
int getDCAResultsInVector(char* profileName, Vector* vecMarkerList, Vector** out_grepResultList, bool check_rotated, char* customLogPath)
{

    T2Debug("%s ++in \n", __FUNCTION__);
    int rc = -1;

    /*
     * Serializing grep result functionality,
     * to avoid synchronization issue on single processor device when
     * multiple profiles tries to get grep result.
     * TODO: Remove static variables from log files processing apis.
     */
    pthread_mutex_lock(&dcaMutex);
    if(NULL != vecMarkerList)
    {
        if (!isPropsInitialized())
        {
            initProperties(logPath, persistentPath);
        }

        // Set the log look up location as previous log folder
        if (customLogPath)
        {
            initProperties(customLogPath, persistentPath);
        }

        Vector_Create(out_grepResultList);

        // Go for looping through the marker list
        if( (rc = parseMarkerListOptimized(profileName, vecMarkerList, *out_grepResultList, check_rotated)) == -1 )
        {
            T2Debug("Error in fetching grep results\n");
        }
        // Reset the log look up directory to default
        if (customLogPath)
        {
            initProperties(logPath, persistentPath);
        }
    }
    pthread_mutex_unlock(&dcaMutex);
    T2Debug("%s --out \n", __FUNCTION__);
    return rc;
}

/** @} */

/** @} */
/** @} */


