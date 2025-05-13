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
 * @brief This API updates the filename if it is different from the current one.
 * @param currentFile The current filename.
 * @param newFile The new filename to update to.
 * @return The updated filename.
 */

static char* updateFilename(char* currentFile, const char* newFile)
{
    if (currentFile == NULL || strcmp(currentFile, newFile) != 0)
    {
        if (currentFile != NULL)
        {
            free(currentFile);
        }
        currentFile = strdup(newFile);
        if (currentFile == NULL)
        {
            T2Error("Insufficient memory to allocate string %s\n", newFile);
        }
    }
    return currentFile;
}

/** @brief This API appends tr181 object value to telemetry node.
 *
 *  @param[in] dst  Object node
 *  @param[in] src  Data value
 */
static void appendData(pcdata_t* dst, const char* src)
{

    T2Debug("%s ++in\n", __FUNCTION__);
    int dst_len, src_len = 0;

    if(NULL == dst || NULL == src)
    {
        return;
    }

    //Copy data
    if(NULL == dst->data)
    {
        src_len = strlen(src) + 1;
        dst->data = (char*) malloc(src_len);
        if(NULL != dst->data)
        {
            snprintf(dst->data, src_len, "%s", src);
        }
        else
        {
            T2Debug("Failed to allocate memory for telemetry node data\n");
        }
    }
    else    //Append data
    {
        dst_len = strlen(dst->data) + 1;
        src_len = strlen(src) + 1;
        dst->data = (char*) realloc(dst->data, dst_len + src_len);
        if(NULL != dst->data)
        {
            strcat(dst->data, ",");
            snprintf((dst->data) + dst_len, src_len, "%s", src);
        }
        else
        {
            T2Debug("Failed to re-allocate memory for telemetry node data\n");
        }
    }

    T2Debug("%s --out\n", __FUNCTION__);
}


/**
 *  @brief This API process tr181 objects through ccsp message bus
 *
 *  @param[in] logfile  DCA pattern file
 *  @param[in] pchead   Node head
 *  @param[in] pcIndex  Node count
 *
 *  @return Returns status of the operation.
 *  @retval Returns 1 on failure, 0 on success
 *  Retaining this for skip frequency param confined to XCONF profile
 */
static T2ERROR processTr181Objects(rdkList_t *pchead)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    T2ERROR ret_val = T2ERROR_FAILURE;
    int length, obj_count, i = 0;
    rdkList_t *tlist = NULL;
    pcdata_t *tmp = NULL;
    char tr181objBuff[TR181BUF_LENGTH + 15] = { '\0' };
    char *tck, *first_tck = NULL;
    if(pchead == NULL)
    {
        T2Error("pchead is NULL for %s\n", __FUNCTION__);
        return T2ERROR_FAILURE;
    }
    //Loop through the given list and fill the data field of each node
    for( tlist = pchead; tlist != NULL; tlist = rdk_list_find_next_node(tlist) )
    {
        char* tr181dataBuff = NULL;
        tmp = tlist->m_pUserData;
        if(NULL != tmp)
        {
            if(NULL != tmp->header && NULL != tmp->pattern && strlen(tmp->pattern) < TR181BUF_LENGTH && NULL == tmp->data)
            {

                //Check whether given object has multi-instance token, if no token found it will be treated as a single instance object
                //Or if more than one token found, skip the object as it is not valid/supported
                //Check for first multi-instance token
                tck = strstr(tmp->pattern, OBJ_DELIMITER);
                if(NULL == tck)   //Single instance check
                {
                    ret_val = getParameterValue(tmp->pattern, &tr181dataBuff);
                    if(T2ERROR_SUCCESS == ret_val)
                    {
                        appendData(tmp, tr181dataBuff);
                        free(tr181dataBuff);
                        tr181dataBuff = NULL;
                    }
                    else
                    {
                        T2Debug("Telemetry data source not found. Type = <message_bus>. Content string = %s\n", tmp->pattern);
                    }
                }
                else    //Multi-instance check
                {
                    first_tck = tck;
                    //Check for a next multi-instance token
                    tck = strstr(tck + DELIMITER_SIZE, OBJ_DELIMITER);
                    if(NULL == tck)
                    {
                        //Get NumberOfEntries of a multi-instance object
                        length = first_tck - tmp->pattern;
                        snprintf(tr181objBuff, sizeof(tr181objBuff), "%sNumberOfEntries", tmp->pattern);
                        ret_val = getParameterValue(tr181objBuff, &tr181dataBuff);
                        if(T2ERROR_SUCCESS == ret_val)
                        {
                            obj_count = atoi(tr181dataBuff);
                            free(tr181dataBuff);
                            tr181dataBuff = NULL ;
                            //Collect all all instance value of a object
                            if(obj_count > 0)
                            {
                                for( i = 1; i <= obj_count; i++ )
                                {
                                    //Replace multi-instance token with an object instance number
                                    snprintf(tr181objBuff, sizeof(tr181objBuff), "%s%d%s", tmp->pattern, i, (tmp->pattern + length + DELIMITER_SIZE));
                                    ret_val = getParameterValue(tr181objBuff, &tr181dataBuff);
                                    if(T2ERROR_SUCCESS == ret_val)
                                    {
                                        appendData(tmp, tr181dataBuff);
                                    }
                                    else
                                    {
                                        T2Debug("Telemetry data source not found. Type = <message_bus>. Content string = %s\n", tr181objBuff);
                                    }
                                    free(tr181dataBuff);
                                    tr181dataBuff = NULL;
                                } //End of for loop
                            }
                        }
                        else
                        {
                            T2Debug("Failed to get NumberOfEntries. Type = <message_bus>. Content string = %s\n", tr181objBuff);
                        }
                    }
                    else
                    {
                        T2Debug("Skipping Telemetry object due to invalid format. Type = <message_bus>. Content string = %s\n", tmp->pattern);
                    }
                } //End of Mult-instance check
            }
        }

    } //End of node loop through for loop

    T2Debug("%s --out\n", __FUNCTION__);
    return ret_val;
}

/**
 * @brief This function adds the value to the telemetry output json object.
 *
 * @param[in] pchead  Header field in the telemetry profile
 *
 * @return Returns status of operation.
 */
static void addToJson(rdkList_t *pchead)
{
    if(pchead == NULL)
    {
        T2Error("pchead is NULL for %s\n", __FUNCTION__);
        return;
    }
    T2Debug("%s ++in\n", __FUNCTION__);
    rdkList_t *tlist = pchead;
    pcdata_t *tmp = NULL;
    while(NULL != tlist)
    {
        tmp = tlist->m_pUserData;
        if(NULL != tmp)
        {
            if(tmp->pattern)
            {
                if(tmp->d_type == OCCURENCE)
                {
                    if(tmp->count != 0)
                    {
                        char tmp_str[5] = { 0 };
                        sprintf(tmp_str, "%d", tmp->count);
                        addToSearchResult(tmp->header, tmp_str);
                    }
                }
                else if(tmp->d_type == STR)
                {
                    if(NULL != tmp->data && (strcmp(tmp->data, "0") != 0))
                    {
                        addToSearchResult(tmp->header, tmp->data);
                    }
                }
            }
        }
        tlist = rdk_list_find_next_node(tlist);
    }
    T2Debug("%s --out\n", __FUNCTION__);
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

/**
 * @brief This function adds the value to the telemetry output vector object.
 *
 * @param[in] pchead  Header field in the telemetry profile
 *
 * @return Returns status of operation.
 */
static int addToVector(rdkList_t *pchead, Vector* grepResultList)
{

    T2Debug("%s ++in\n", __FUNCTION__);
    if(pchead == NULL || grepResultList == NULL)
    {
        T2Error("Inavlid arguments for %s\n", __FUNCTION__);
        return -1;
    }
    rdkList_t *tlist = pchead;
    pcdata_t *tmp = NULL;

    // Loop iterating through the results - JSON data converted back to vector
    while(NULL != tlist)
    {
        tmp = tlist->m_pUserData;
        if (NULL == tmp)
        {
            T2Debug("tmp is NULL for %s\n", __FUNCTION__);
            tlist = rdk_list_find_next_node(tlist);
            continue;

        }

        if(tmp->pattern)
        {
            if(tmp->d_type == OCCURENCE)
            {
                if(tmp->count != 0)
                {
                    // JSON respnse always expects message in string format
                    char tmp_str[5] = { 0 };
                    formatCount(tmp_str, sizeof(tmp_str), tmp->count);
                    GrepResult* grepResult = createGrepResultObj(tmp->header, tmp_str, tmp->trimparam, tmp->regexparam);
                    if (grepResult == NULL)
                    {
                        T2Error("Failed to create GrepResult\n");
                        return -1;
                    }
                    if(tmp->header)
                    {
                        free(tmp->header);
                        tmp->header = NULL;
                    }
                    T2Debug("Adding OCCURENCE to result list %s : %s \n", grepResult->markerName, grepResult->markerValue);
                    Vector_PushBack(grepResultList, grepResult);
                }
            }
            else if(tmp->d_type == STR)
            {
                if(NULL != tmp->data && (strcmp(tmp->data, "0") != 0))
                {
                    GrepResult* grepResult = createGrepResultObj(tmp->header, tmp->data, tmp->trimparam, tmp->regexparam);
                    if(grepResult == NULL)
                    {
                        T2Error("Failed to create GrepResult\n");
                        return -1;
                    }
                    if(tmp->header)
                    {
                        free(tmp->header);
                        tmp->header = NULL;
                    }
                    if(tmp->data)
                    {
                        free(tmp->data);
                        tmp->data = NULL;
                    }
                    T2Debug("Adding STR to result list %s : %s \n", grepResult->markerName, grepResult->markerValue);
                    Vector_PushBack(grepResultList, grepResult);

                }
            }
        }
        tlist = rdk_list_find_next_node(tlist);
    }
    T2Debug("%s --out\n", __FUNCTION__);
    return 0;
}

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



/**
 * @brief Function to process pattern count (loggrep)
 *
 * @param[in]  logfile     Current log file
 * @param[in]  pchead      Node head
 * @param[in]  pcIndex     Node count
 * @param[in]  rdkec_head  RDK errorcode head
 *
 * @return Returns status of operation.
 * @retval Return 0 upon success, -1 on failure.
 */
static int processCountPattern(hash_map_t *logSeekMap, char *logfile, rdkList_t *pchead, rdkList_t **rdkec_head, int *firstSeekFromEOF, bool check_rotated_logs)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    char temp[MAXLINE] = { 0 };
    T2Debug("Read from log file %s \n", logfile);
    while(getLogLine(logSeekMap, temp, MAXLINE, logfile, firstSeekFromEOF, check_rotated_logs) != NULL)
    {

        int len = strlen(temp);
        if(len > 0 && temp[len - 1] == '\n')
        {
            temp[--len] = '\0';
        }

        pcdata_t *pc_node = searchPCNode(pchead, temp);
        if(NULL != pc_node)
        {
            if(pc_node->d_type == OCCURENCE)
            {
                pc_node->count++;
            }
            else
            {
                if(NULL != pc_node->header)
                {
                    getSplitParameterValue(temp, pc_node);
                }
            }
        }
        else
        {
            // This is a RDK-V specific calls for reporting RDK error codes . Retaining for video porting
            // TODO: This should be moved to a more appropriate location if it is still required and exclude checking for each line.
            if(NULL != strstr(temp, "RDK-"))
            {
                handleRDKErrCodes(rdkec_head, temp);
            }
        }
    }
    T2Debug("%s --out\n", __FUNCTION__);
    return 0;
}

/**
 * @brief Generic pattern function based on pattern to call top/count or using ccsp message bus.
 *
 * @param[in]  prev_file    The previous log file.
 * @param[in]  logfile      The current log file.
 * @param[in]  rdk_error_code_head   RDK errorcode head
 * @param[in]  pchead       Node head
 * @param[in]  pcIndex      Node count
 *
 * @return Returns status on operation.
 * @retval Returns 0 upon success.
 */
static int processPattern(char **prev_file, char *logfile, rdkList_t **rdk_error_code_head, rdkList_t *pchead, Vector *grepResultList, hash_map_t* logSeekMap, int *firstSeekFromEOF, bool check_rotated_logs)
{

    T2Debug("%s ++in\n", __FUNCTION__);
    if(NULL == prev_file || NULL == logfile || NULL == rdk_error_code_head || NULL == pchead || NULL == grepResultList)
    {
        T2Error("Invalid arguments for %s\n", __FUNCTION__);
        return -1;
    }


    if((NULL == *prev_file) || (strcmp(*prev_file, logfile) != 0))
    {
        if(*prev_file != NULL)
        {
            updateLogSeek(logSeekMap, *prev_file);
            free(*prev_file);
        }
        *prev_file = strdup(logfile);
        if(*prev_file == NULL)
        {
            T2Error("Insufficient memory available to allocate duplicate string %s\n", logfile);
        }
    }
    // Based on the logfile name, processing varies.
    // Message bus still landing on the legacy utils is a case which has a non-zero skip frequency value
    if(0 == strcmp(logfile, "top_log.txt"))
    {
        processTopPattern(pchead, grepResultList);
    }
    else if(0 == strcmp(logfile, "<message_bus>"))
    {
        processTr181Objects(pchead);
        addToVector(pchead, grepResultList);
    }
    else
    {
        // This is the function which does actual loggrep for the pattern
        // It will also handle the rotated log files if the flag is set
        processCountPattern(logSeekMap, logfile, pchead, rdk_error_code_head, firstSeekFromEOF, check_rotated_logs);
        addToVector(pchead, grepResultList);
    }
    clearPCNodes(&pchead);
    T2Debug("%s --out\n", __FUNCTION__);
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

/** @description: Main logic function to parse sorted vector list and to process the pattern list
 *  @param filename
 *  @return -1 on failure, 0 on success
 */
static int parseMarkerList(char* profileName, Vector* ip_vMarkerList, Vector* op_grepResultList, bool check_rotated)
{
    T2Debug("%s ++in \n", __FUNCTION__);

    char *filename = NULL, *prevfile = NULL;
    rdkList_t *pchead = NULL, *rdkec_head = NULL;
    GrepSeekProfile* gsProfile = NULL;
    size_t var = 0;

    size_t vCount = Vector_Size(ip_vMarkerList);
    T2Debug("vMarkerList for profile %s is of count = %lu \n", profileName, (unsigned long )vCount);

    // Get logfile -> seek value map associated with the profile
    gsProfile = (GrepSeekProfile *) getLogSeekMapForProfile(profileName);
    if(NULL == gsProfile)
    {
        T2Debug("logSeekMap is null, add logSeekMap for %s \n", profileName);
        gsProfile = (GrepSeekProfile *) addToProfileSeekMap(profileName);
    }

    if(NULL == gsProfile)
    {
        T2Error("%s Unable to retrive / create logSeekMap for profile %s \n", __FUNCTION__, profileName);
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

    // Traverse through marker list
    for (var = 0; var < vCount; ++var)
    {
        GrepMarker* markerList = (GrepMarker*) Vector_At(ip_vMarkerList, var);
        if (!markerList || !markerList->logFile || !markerList->searchString || !markerList->markerName)
        {
            continue;
        }
        if (strcmp(markerList->searchString, "") == 0 || strcmp(markerList->logFile, "") == 0)
        {
            continue;
        }
        if (strcasecmp(markerList->logFile, "snmp") == 0)
        {
            continue;
        }

        int tmp_skip_interval, is_skip_param;

        char *temp_header = markerList->markerName;
        char *temp_pattern = markerList->searchString;
        char *temp_file = markerList->logFile;
        bool trim = markerList->trimParam;
        char *regex = markerList->regexParam;
        tmp_skip_interval = markerList->skipFreq;

        DType_t dtype;
        getDType(temp_file, markerList->mType, &dtype);

        if(tmp_skip_interval <= 0)
        {
            tmp_skip_interval = 0;
        }

        if(profileExecCounter % (tmp_skip_interval + 1) == 0)
        {
            is_skip_param = 0;
        }
        else
        {
            is_skip_param = 1;
        }

        filename = updateFilename(filename, temp_file);

        // TODO optimize the list search in US
        if(is_skip_param == 0)
        {
            if(0 == insertPCNode(&pchead, temp_pattern, temp_header, dtype, 0, NULL, trim, regex))
            {
                processPattern(&prevfile, filename, &rdkec_head, pchead, op_grepResultList, gsProfile->logFileSeekMap, &(markerList->firstSeekFromEOF), check_rotated_logs);
                pchead = NULL;
            }
        }
        else
        {
            T2Debug("Current iteration for this parameter needs to be excluded, but the seek values needs to be updated in case of logfile based marker\n");
            // TODO optimize seek update logic for skip intervals
            updateLastSeekval(gsProfile->logFileSeekMap, &prevfile, filename);
        }

    }  // End of adding list to node
#if !defined(ENABLE_RDKC_SUPPORT) && !defined(ENABLE_RDKB_SUPPORT)
    // remove the saved top information
    pthread_mutex_lock(&topOutputMutex);
    removeTopOutput();
    pthread_mutex_unlock(&topOutputMutex);
#endif


    if(filename)
    {
        updateLogSeek(gsProfile->logFileSeekMap, filename);
    }

    gsProfile->execCounter += 1;
    T2Debug("Execution Count = %d\n", gsProfile->execCounter);

    /* max limit not maintained for rdkec_head FIXME */
    if(NULL != rdkec_head)
    {
        addToJson(rdkec_head);
        // clear nodes memory after process
        clearPCNodes(&rdkec_head);
        rdkec_head = NULL;
    }

    if(NULL != filename)
    {
        free(filename);
    }

    if(NULL != prevfile)
    {
        free(prevfile);
    }

    T2Debug("%s --out \n", __FUNCTION__);
    return 0;
}

int getDCAResultsInJson(char* profileName, void* markerList, cJSON** grepResultList)
{

    T2Debug("%s ++in \n", __FUNCTION__);
    int rc = -1;

    /*
     * Keeping the lock here to be aligned with getDCAResultsInVector().
     * Although the lock here is redundant, coz for multiprocess devices
     * dcautil is having pInterChipLock for all inter-processor communications.
     */

    pthread_mutex_lock(&dcaMutex);
    if(NULL != markerList)
    {
        if (!isPropsInitialized())
        {
            initProperties(logPath, persistentPath);
        }

        initSearchResultJson(&ROOT_JSON, &SEARCH_RESULT_JSON);

        rc = parseMarkerList(profileName, markerList, NULL, false);
        *grepResultList = ROOT_JSON;
    }
    pthread_mutex_unlock(&dcaMutex);

    T2Debug("%s --out \n", __FUNCTION__);
    return rc;
}


int getDCAResultsInVector(char* profileName, Vector* vecMarkerList, Vector** grepResultList, bool check_rotated, char* customLogPath)
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

        if (customLogPath)
        {
            initProperties(customLogPath, persistentPath);
        }

        Vector_Create(grepResultList);
        if( (rc = parseMarkerList(profileName, vecMarkerList, *grepResultList, check_rotated)) == -1 )
        {
            T2Debug("Error in fetching grep results\n");
        }
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


