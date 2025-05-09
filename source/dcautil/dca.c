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
            dst->data[dst_len - 1] = ',';
            dst->data[dst_len] = '\0'; 
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

    while(NULL != tlist)
    {
        tmp = tlist->m_pUserData;
        if(NULL != tmp)
        {

            if(tmp->pattern && grepResultList != NULL )
            {
                if(tmp->d_type == OCCURENCE)
                {
                    if(tmp->count != 0)
                    {
                        char tmp_str[5] = { 0 };
                        if(tmp->count > 9999)
                        {
                            T2Debug("Count value is %d higher than limit of 9999 changing the value to %d to track buffer overflow", tmp->count, INVALID_COUNT);
                            tmp->count = INVALID_COUNT;
                        }
                        snprintf(tmp_str, sizeof(tmp_str), "%d", tmp->count);
                        GrepResult* grepResult = (GrepResult*) malloc(sizeof(GrepResult));
                        grepResult->markerName = strdup(tmp->header);
                        grepResult->markerValue = strdup(tmp_str);
                        grepResult->trimParameter = tmp->trimparam;
                        grepResult->regexParameter = tmp->regexparam;
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
                        GrepResult* grepResult = (GrepResult*) malloc(sizeof(GrepResult));
                        grepResult->markerName = strdup(tmp->header);
                        grepResult->markerValue = strdup(tmp->data);
                        grepResult->trimParameter = tmp->trimparam;
                        grepResult->regexParameter = tmp->regexparam;
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
            else
            {
                T2Debug("%s : grepResultList is NULL \n", __FUNCTION__);
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

    char *strFound = NULL;
    if(line == NULL || pcnode == NULL)
    {
        T2Error("Invalid arguments for %s\n", __FUNCTION__);
        return -1;
    }
    strFound = strstr(line, pcnode->pattern);

    if(strFound != NULL)
    {
        int tlen = 0, plen = 0, vlen = 0;
        tlen = (int) strlen(line);
        plen = (int) strlen(pcnode->pattern);
        strFound = strFound + plen;
        if(tlen > plen)
        {
            vlen = strlen(strFound);
            // If value is only single char make sure its not an empty space .
            // Ideally component should not print logs with empty values but we have to consider logs from OSS components
            if((1 == vlen) && isspace(strFound[plen]))
            {
                return 0;
            }

            if(vlen > 0)
            {
                if(NULL == pcnode->data)
                {
                    pcnode->data = (char *) malloc(MAXLINE);
                }

                if(NULL == pcnode->data)
                {
                    return (-1);
                }

                strncpy(pcnode->data, strFound, MAXLINE);
                pcnode->data[tlen - plen] = '\0'; //For Boundary Safety
            }
        }
    }

    return 0;
}

/**
 * @brief To get RDK error code.
 *
 * @param[in]  str    Source string.
 * @param[out] ec     Error code.
 *
 * @return Returns status of operation.
 * @retval Return 0 upon success.
 */
int getErrorCode(char *str, char *ec)
{

    T2Debug("%s ++in\n", __FUNCTION__);
    int i = 0, j = 0, len = 0;
// Solution: Check if str is NULL before dereferencing it.
    if(str == NULL)
    {
        T2Error("Str is NULL for %s\n", __FUNCTION__);
        return -1;
    }
    len = strlen(str);
    char tmpEC[LEN] = { 0 };
    while(str[i] != '\0')
    {
        if(len >= 4 && str[i] == 'R' && str[i + 1] == 'D' && str[i + 2] == 'K' && str[i + 3] == '-')
        {
            i += 4;
            j = 0;
            if(str[i] == '0' || str[i] == '1')
            {
                tmpEC[j] = str[i];
                i++;
                j++;
                if(str[i] == '0' || str[i] == '3')
                {
                    tmpEC[j] = str[i];
                    i++;
                    j++;
                    if(0 != isdigit(str[i]))
                    {
                        while(i <= len && 0 != isdigit(str[i]) && j < RDK_EC_MAXLEN)
                        {
                            tmpEC[j] = str[i];
                            i++;
                            j++;
                            ec[j] = '\0';
                            strncpy(ec, tmpEC, LEN);
                        }
                        break;
                    }
                }
            }
        }
        i++;
    }
    T2Debug("%s --out\n", __FUNCTION__);
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
    char temp[MAXLINE];
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
 * @param[in]  rdkec_head   RDK errorcode head
 * @param[in]  pchead       Node head
 * @param[in]  pcIndex      Node count
 *
 * @return Returns status on operation.
 * @retval Returns 0 upon success.
 */
static int processPattern(char **prev_file, char *logfile, rdkList_t **rdkec_head, rdkList_t *pchead, Vector *grepResultList, hash_map_t* logSeekMap, int *firstSeekFromEOF, bool check_rotated_logs)
{

    T2Debug("%s ++in\n", __FUNCTION__);
    if(NULL != logfile)
    {

        if((NULL == *prev_file) || (strcmp(*prev_file, logfile) != 0))
        {
            if(NULL == *prev_file)
            {
                *prev_file = strdup(logfile);
                if(*prev_file == NULL)
                {
                    T2Error("Insufficient memory available to allocate duplicate string %s\n", logfile);
                }
            }
            else
            {
                updateLogSeek(logSeekMap, *prev_file);
                free(*prev_file);
                *prev_file = strdup(logfile);
                if(*prev_file == NULL)
                {
                    T2Error("Insufficient memory available to allocate duplicate string %s\n", logfile);
                }
            }
        }
        // Process
        if(NULL != pchead)
        {
            if(0 == strcmp(logfile, "top_log.txt"))
            {
                if(grepResultList != NULL)
                {
                    processTopPattern(pchead, grepResultList);
                }
            }
            else if(0 == strcmp(logfile, "<message_bus>"))
            {
                processTr181Objects( pchead);
                if (grepResultList != NULL)
                {
                    addToVector(pchead, grepResultList);
                }
                else
                {
                    addToJson(pchead);
                }
            }
            else
            {
                processCountPattern(logSeekMap, logfile, pchead, rdkec_head, firstSeekFromEOF, check_rotated_logs);
                if (grepResultList != NULL)
                {
                    addToVector(pchead, grepResultList);
                }
                else
                {
                    addToJson(pchead);
                }
            }
        }
        clearPCNodes(&pchead);
    }
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
static int parseMarkerList(char* profileName, Vector* vMarkerList, Vector* grepResultList, bool check_rotated)
{
    T2Debug("%s ++in \n", __FUNCTION__);

    char *filename = NULL, *prevfile = NULL;
    rdkList_t *pchead = NULL, *rdkec_head = NULL;
    GrepSeekProfile* gsProfile = NULL;
    size_t var = 0;

    size_t vCount = Vector_Size(vMarkerList);
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
    if((gsProfile->execCounter == 1) && (firstreport_after_bootup == false))  //checking the execution count and first report after bootup because after config reload again execution count will get initialised and again reaches 1. check_rotated logs flag is to check the rotated log files even when seekvalue is less than filesize for the first time.
    {
        check_rotated_logs = check_rotated;
        firstreport_after_bootup = true;
    }
    else
    {
        check_rotated_logs = false;
    }

    // Traverse through marker list
    for( var = 0; var < vCount; ++var )
    {

        GrepMarker* markerList = (GrepMarker*) Vector_At(vMarkerList, var);
        int tmp_skip_interval, is_skip_param;

        char *temp_header = markerList->markerName;
        char *temp_pattern = markerList->searchString;
        char *temp_file = markerList->logFile;
        bool trim = markerList->trimParam;
        char *regex = markerList->regexParam;
        tmp_skip_interval = markerList->skipFreq;

        DType_t dtype;

        if(NULL == temp_file || NULL == temp_pattern || NULL == temp_header)
        {
            continue;
        }

        if((0 == strcmp(temp_pattern, "")) || (0 == strcmp(temp_file, "")))
        {
            continue;
        }

        if(0 == strcasecmp(temp_file, "snmp"))
        {
            continue;
        }

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

        if(NULL == filename)
        {
            filename = strdup(temp_file);
            if(filename == NULL)
            {
                T2Error("Insufficient memory available to allocate duplicate string %s\n", temp_file);
            }
        }
        else
        {
            if(0 != strcmp(filename, temp_file))
            {
                free(filename);
                filename = strdup(temp_file);
                if(filename == NULL)
                {
                    T2Error("Insufficient memory available to allocate duplicate string %s\n", temp_file);
                }
            }
        }

        // TODO optimize the list search in US
        if(is_skip_param == 0)
        {
            if(0 == insertPCNode(&pchead, temp_pattern, temp_header, dtype, 0, NULL, trim, regex))
            {
                processPattern(&prevfile, filename, &rdkec_head, pchead, grepResultList, gsProfile->logFileSeekMap, &(markerList->firstSeekFromEOF), check_rotated_logs);
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

