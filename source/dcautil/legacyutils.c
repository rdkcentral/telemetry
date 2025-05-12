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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#include "t2log_wrapper.h"
#include "legacyutils.h"
#include "vector.h"
#include "dcautil.h"

#define EC_BUF_LEN 20

static char* PERSISTENT_PATH = NULL;
static char* LOG_PATH        = NULL;
static char* DEVICE_TYPE     = NULL;
static bool  isPropsIntialized = false ;
static long  LAST_SEEK_VALUE = 0;

// Map holding profile name to Map ( logfile -> seek value) ]
static hash_map_t *profileSeekMap = NULL;
static pthread_mutex_t pSeekLock = PTHREAD_MUTEX_INITIALIZER;

// Map holding profile name to Exec Count
static hash_map_t *profileExecCountMap = NULL;
static pthread_mutex_t pExecCountLock = PTHREAD_MUTEX_INITIALIZER;

/**
 * Start of functions dealing with log seek values
 */

static void freeLogFileSeekMap(void *data)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if (data != NULL)
    {
        hash_element_t *element = (hash_element_t *) data;

        if (element->key)
        {
            T2Debug("Freeing hash entry element for Log file Name:%s\n", element->key);
            free(element->key);
        }
        if (element->data)
        {
            T2Debug("Freeing element data\n");
            free(element->data);
        }

        free(element);
    }
    T2Debug("%s --out\n", __FUNCTION__);
}

static void freeGrepSeekProfile(GrepSeekProfile *gsProfile)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if (gsProfile)
    {
        hash_map_destroy(gsProfile->logFileSeekMap, freeLogFileSeekMap);
        free(gsProfile);
    }
    T2Debug("%s --out\n", __FUNCTION__);
}

static void freeProfileSeekHashMap(void *data)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if (data != NULL)
    {
        hash_element_t *element = (hash_element_t *) data;

        if (element->key)
        {
            T2Debug("Freeing hash entry element for Profiles object Name:%s\n", element->key);
            free(element->key);
        }

        if (element->data)
        {
            T2Debug("Freeing GrepSeekProfile data\n");
            GrepSeekProfile* gsProfile = (GrepSeekProfile *)element->data;
            freeGrepSeekProfile(gsProfile);
            gsProfile = NULL;
        }

        free(element);
    }
    T2Debug("%s --out\n", __FUNCTION__);
}

GrepSeekProfile *addToProfileSeekMap(char* profileName)
{
    if(profileName == NULL)
    {
        T2Error("profileName is NULL\n");
        return NULL;
    }
    T2Debug("%s ++in for profileName = %s \n", __FUNCTION__, profileName);

    pthread_mutex_lock(&pSeekLock);
    GrepSeekProfile *gsProfile = NULL;
    if (profileSeekMap)
    {
        T2Debug("Adding GrepSeekProfile for profile %s in profileSeekMap\n", profileName);
        gsProfile = malloc(sizeof(GrepSeekProfile));
        gsProfile->logFileSeekMap = hash_map_create();

        pthread_mutex_lock(&pExecCountLock);
        if(profileExecCountMap == NULL)
        {
            T2Debug("ExecCount map doesnot exist, creatinag a new hash map\n");
            profileExecCountMap = hash_map_create();
            gsProfile->execCounter = 0;
        }
        else
        {
            int *execCount = (int *) hash_map_get(profileExecCountMap, profileName);
            if(execCount)
            {
                gsProfile->execCounter = *execCount;
                T2Info("Update the Exec Count value to %d\n", gsProfile->execCounter);
            }
        }
        pthread_mutex_unlock(&pExecCountLock);

        hash_map_put(profileSeekMap, strdup(profileName), (void*)gsProfile, freeProfileSeekHashMap);
    }
    else
    {
        T2Debug("profileSeekMap exists .. \n");
    }
    pthread_mutex_unlock(&pSeekLock);
    T2Debug("%s --out\n", __FUNCTION__);
    return gsProfile;
}

void removeProfileFromSeekMap(char *profileName)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    pthread_mutex_lock(&pSeekLock);
    if (profileSeekMap)
    {
        GrepSeekProfile* gsProfile = (GrepSeekProfile *)hash_map_remove(profileSeekMap, profileName);
        if (NULL != gsProfile)
        {

            if(gsProfile->execCounter != 0)
            {
                int *temp = (int *) malloc(sizeof(int));
                T2Debug("Saving the ExecCount : %d for thr profile : %s\n", gsProfile->execCounter, profileName);
                pthread_mutex_lock(&pExecCountLock);
                if (profileExecCountMap == NULL)
                {
                    T2Debug("ExecCount map doesnot exist, creatinag a new hash map\n");
                    profileExecCountMap = hash_map_create();
                }
                *temp = gsProfile->execCounter;
                hash_map_put(profileExecCountMap, strdup(profileName), (void *) temp, free);
                pthread_mutex_unlock(&pExecCountLock);
            }
            freeGrepSeekProfile(gsProfile);

        }
        else
        {
            T2Debug("%s: Matching grep profile not found for %s\n", __FUNCTION__, profileName);
        }
    }
    else
    {
        T2Debug("%s: profileSeekMap is empty \n", __FUNCTION__);
    }
    pthread_mutex_unlock(&pSeekLock);
    T2Debug("%s --out\n", __FUNCTION__);
}

void removeProfileFromExecMap(char *profileName)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    pthread_mutex_lock(&pExecCountLock);
    if (profileExecCountMap)
    {
        int* value = (int *)hash_map_remove(profileExecCountMap, profileName);
        if (NULL != value)
        {
            free(value);
            T2Debug("%s : Released the exec count for profile %s \n", __FUNCTION__, profileName);
        }
        else
        {
            T2Debug("%s: Matching grep profile not found for %s\n", __FUNCTION__, profileName);
        }
    }

    pthread_mutex_unlock(&pExecCountLock);
    // Adding this logic if profile name is same then we will not remove the seekMap in this case
    // we need to check if the seek map exists, and assign the execCounter to 0
    pthread_mutex_lock(&pSeekLock);
    if (profileSeekMap)
    {
        GrepSeekProfile* gsProfile = (GrepSeekProfile *)hash_map_get(profileSeekMap, profileName);
        if (NULL != gsProfile)
        {
            gsProfile->execCounter = 0;
        }
    }
    pthread_mutex_unlock(&pSeekLock);
    T2Debug("%s --out\n", __FUNCTION__);
}

// Returns map if found else NULL
GrepSeekProfile *getLogSeekMapForProfile(char* profileName)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if(profileName == NULL)
    {
        T2Error("profileName is NULL for getLogSeekMap\n");
        return NULL;
    }
    GrepSeekProfile* gsProfile = NULL ;
    T2Debug("Get profileseek map for %s \n", profileName);
    pthread_mutex_lock(&pSeekLock);

    if(profileSeekMap == NULL)
    {
        T2Debug("Profile seek map doesn't exist, creating one ... \n");
        profileSeekMap = hash_map_create();
    }

    T2Debug("profileSeekMap count %d \n", hash_map_count(profileSeekMap));

    if(profileSeekMap)
    {
        gsProfile = hash_map_get(profileSeekMap, profileName);
    }
    else
    {
        T2Debug("Profile seek map is NULL from  getProfileSeekMap \n");
    }

    pthread_mutex_unlock(&pSeekLock);
    T2Debug("%s --out\n", __FUNCTION__);

    return gsProfile;
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
static int getLogSeekValue(hash_map_t *logSeekMap, char *name, long *seek_value)
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
T2ERROR updateLogSeek(hash_map_t *logSeekMap, char* logFileName)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if(logSeekMap == NULL || logFileName == NULL)
    {
        T2Error("Invalid or NULL arguments\n");
        return T2ERROR_FAILURE;
    }

    pthread_mutex_lock(&pSeekLock);
    if(NULL != logSeekMap)
    {
        T2Debug("Adding seekvalue of %ld for %s to logSeekMap \n", LAST_SEEK_VALUE, logFileName);
        long* val = (long *) malloc(sizeof(long));
        if(NULL != val)
        {
            *val = LAST_SEEK_VALUE ;
            hash_map_put(logSeekMap, strdup(logFileName), (void *)val, free);
        }
        else
        {
            T2Warning("Unable to allocate memory for seek value pointer \n");
        }
    }
    else
    {
        T2Debug("logSeekMap is null, unable to update logSeekMap \n");
    }
    pthread_mutex_unlock(&pSeekLock);
    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

/**
 * Start of functions dealing with log seek values
 */

/**
 * @addtogroup DCA_APIS
 * @{
 */

/**
 * @brief This API is to find the load average of system and add it to the SearchResult JSON.
 *
 * @return  Returns status of operation.
 * @retval  Return 1 on success.
 */
int getLoadAvg(Vector* grepResultList, bool trim, char* regex)
{
    T2Debug("%s ++in \n", __FUNCTION__);
    FILE *fp;
    char str[LEN + 1];
    if(grepResultList == NULL)
    {
        T2Debug("grepResultList is NULL\n");
        return 0;
    }

    if(NULL == (fp = fopen("/proc/loadavg", "r")))
    {
        T2Debug("Error in opening /proc/loadavg file");
        return 0;
    }
    if(fread(str, 1, LEN, fp) != LEN)
    {
        T2Debug("Error in reading loadavg");
        fclose(fp);
        return 0;
    }
    fclose(fp);
    str[LEN] = '\0';

    if(grepResultList != NULL)
    {
        GrepResult* loadAvg = (GrepResult*) malloc(sizeof(GrepResult));
        if(loadAvg)
        {
            loadAvg->markerName = strdup("Load_Average");
            loadAvg->markerValue = strdup(str);
            loadAvg->trimParameter = trim;
            loadAvg->regexParameter = regex;
            Vector_PushBack(grepResultList, loadAvg);
        }
    }
    T2Debug("%s --out \n", __FUNCTION__);
    return 1;
}

/**
 * @brief This function returns file size.
 *
 * @param[in] fp    File name
 *
 * @return  Returns size of file.
 */
static long fsize(FILE *fp)
{
    struct stat st;
    if (fstat(fileno(fp), &st) == 0) {
        return st.st_size;
    }
    return -1;
}

/**
 * @brief This function is to clear/free the global paths.
 */
void clearConfVal(void)
{
    T2Debug("%s ++in \n", __FUNCTION__);
    if(PERSISTENT_PATH)
    {
        free(PERSISTENT_PATH);
    }

    if(LOG_PATH)
    {
        free(LOG_PATH);
    }

    if(DEVICE_TYPE)
    {
        free(DEVICE_TYPE);
    }
    T2Debug("%s --out \n", __FUNCTION__);
}

void updateLastSeekval(hash_map_t *logSeekMap, char **prev_file, char* filename)
{

    FILE *pcurrentLogFile = NULL;
    char* currentLogFile = NULL;
    long fileSize = 0;

    if((NULL == PERSISTENT_PATH) || (NULL == LOG_PATH) || (NULL == filename) || (NULL == logSeekMap))
    {
        T2Debug("Path variables are empty");
        return;
    }

    if((NULL == *prev_file) || (strcmp(*prev_file, filename) != 0))
    {
        if(NULL == *prev_file)
        {
            *prev_file = strdup(filename);
            if(*prev_file == NULL)
            {
                T2Error("Insufficient memory available to allocate duplicate string %s\n", filename);
            }
        }
        else
        {
            updateLogSeek(logSeekMap, *prev_file);
            free(*prev_file);
            *prev_file = strdup(filename);
            if(*prev_file == NULL)
            {
                T2Error("Insufficient memory available to allocate duplicate string %s\n", filename);
            }
        }
    }

    int logname_len = strlen(LOG_PATH) + strlen(filename) + 1;
    currentLogFile = malloc(logname_len);
    if(currentLogFile)
    {
        snprintf(currentLogFile, logname_len, "%s%s", LOG_PATH, filename);
        pcurrentLogFile = fopen(currentLogFile, "rb");
        if(pcurrentLogFile)
        {
            fileSize = fsize(pcurrentLogFile);
            LAST_SEEK_VALUE = fileSize;
            fclose(pcurrentLogFile);
        }
        free(currentLogFile);
    }
}

/**
 *  @brief Function to return rotated log file.
 *
 *  @param[in] buf       Buffer
 *  @param[in] buflen    Maximum buffer length
 *  @param[in] name      Current Log file
 *
 *  @return Returns Seek Log file.
 */
char *getLogLine(hash_map_t *logSeekMap, char *buf, int buflen, char *name, int *firstSeekFromEOF, bool check_rotated_logs)
{
    static FILE *pcurrentLogFile = NULL;
    static int is_rotated_log = 0;
    char *rval = NULL;

    if((NULL == PERSISTENT_PATH) || (NULL == LOG_PATH) || (NULL == name))
    {
        T2Debug("Path variables are empty");
        return NULL;
    }
    if(logSeekMap == NULL || buf == NULL)
    {
        T2Debug("Invalid arguments or NULL arguments\n");
        return NULL;
    }
    char *logpath = LOG_PATH;

    /* If name is already an absolute path then prepend empty string instead of LOG_PATH */
    if (name[0] == '/')
    {
        logpath = "";
    }

    int logname_len = strlen(logpath) + strlen(name) + 1;
    char *fileExtn = ".1";
    int fileExtn_len = logname_len + strlen(fileExtn);
    if(NULL == pcurrentLogFile)
    {
        char *currentLogFile = NULL;
        long seek_value = 0;

        currentLogFile = malloc(logname_len);
        if(NULL != currentLogFile)
        {
            snprintf(currentLogFile, logname_len, "%s%s", logpath, name);
            if(0 != getLogSeekValue(logSeekMap, name, &seek_value))
            {
                pcurrentLogFile = fopen(currentLogFile, "rb");
                free(currentLogFile);
                if(NULL == pcurrentLogFile)
                {
                    return NULL;
                }
            }
            else
            {
                long fileSize = 0;
                pcurrentLogFile = fopen(currentLogFile, "rb");

                if(NULL == pcurrentLogFile)
                {
                    LAST_SEEK_VALUE = 0;
                    free(currentLogFile);
                    return NULL;
                }
                fileSize = fsize(pcurrentLogFile);

                // if the seek value is given in the profile use that value to calculate the seek value from the END OF FILE
                // the seek value becomes filesize - firstSeekFromEOF since it cant be negetive set it zero if negative
                if (*firstSeekFromEOF != 0)
                {
                    seek_value = (fileSize > (*firstSeekFromEOF)) ? (fileSize - (*firstSeekFromEOF)) : 0 ;
                    T2Debug("First Seek from the EOF is given  value : %d; the size of file is %ld; current seek after calculations %ld\n", *firstSeekFromEOF, fileSize, seek_value);
                    *firstSeekFromEOF = 0; // update this to zero as this initial seek update should only run for the first time
                }
                if(seek_value <= fileSize)
                {
                    if(check_rotated_logs)  // considering the rotated log file  within few minutes after bootup, it should check only for the first time
                    {
                        char * rotatedLog;
                        size_t name_len = strlen(currentLogFile);
                        if(name_len > 2 && currentLogFile[name_len - 2] == '.' && currentLogFile[name_len - 1] == '0')
                        {
                            rotatedLog = strdup(currentLogFile);
                            if(NULL != rotatedLog)
                            {
                                rotatedLog[name_len - 1] = '1';
                                T2Debug("Log file name seems to be having .0 extension hence Rotated log file name is %s\n", rotatedLog);
                            }
                        }
                        else
                        {
                            rotatedLog = malloc(fileExtn_len);
                            if(NULL != rotatedLog)
                            {
                                snprintf(rotatedLog, fileExtn_len, "%s%s%s", logpath, name, fileExtn);
                                T2Debug("Rotated log file name is %s\n", rotatedLog);
                            }
                        }
                        if(NULL != rotatedLog)
                        {
                            fclose(pcurrentLogFile);
                            pcurrentLogFile = NULL;
                            pcurrentLogFile = fopen(rotatedLog, "rb");
                            is_rotated_log = 1;
                            if(NULL == pcurrentLogFile)
                            {
                                T2Debug("Error in opening file %s\n", rotatedLog);
                                is_rotated_log = 0;
                                pcurrentLogFile = fopen(currentLogFile, "rb");
                                if(pcurrentLogFile == NULL)
                                {
                                    T2Debug("Error in opening file %s\n", currentLogFile);
                                    free(currentLogFile);
                                    free(rotatedLog);
                                    return NULL;
                                }
                            }
                            free(currentLogFile);
                            free(rotatedLog);
                            if(fseek(pcurrentLogFile, seek_value, 0) != 0)
                            {
                                T2Error("Cannot set the file position indicator for the stream pointed to by stream\n");
                            }
                        }
                        else
                        {
                            T2Error("Malloc failure for rotated log\n");
                            free(currentLogFile);
                            fclose(pcurrentLogFile);
                            return NULL;
                        }
                    }
                    else
                    {
                        free(currentLogFile);
                        if( fseek(pcurrentLogFile, seek_value, 0) != 0)
                        {
                            T2Error("Cannot set the file position indicator for the stream pointed to by stream\n");
                        }
                    }
                }
                else
                {
                    if((NULL != DEVICE_TYPE) && (0 == strcmp("broadband", DEVICE_TYPE)))
                    {
                        T2Debug("Telemetry file pointer corrupted");
                        if(fseek(pcurrentLogFile, 0, 0) != 0)
                        {
                            T2Error("Cannot set the file position indicator for the stream pointed to by stream\n");
                        }
                        if(currentLogFile != NULL)
                        {
                            free(currentLogFile);
                        }
                    }
                    else
                    {
                        char * rotatedLog;
                        size_t name_len = strlen(currentLogFile);
                        if(name_len > 2 && currentLogFile[name_len - 2] == '.' && currentLogFile[name_len - 1] == '0')
                        {
                            rotatedLog = strdup(currentLogFile);
                            if(NULL != rotatedLog)
                            {
                                rotatedLog[name_len - 1] = '1';
                                T2Debug("Log file name seems to be having .0 extension hence Rotated log file name is %s\n", rotatedLog);
                            }
                        }
                        else
                        {
                            rotatedLog = malloc(fileExtn_len);
                            if(NULL != rotatedLog)
                            {
                                snprintf(rotatedLog, fileExtn_len, "%s%s%s", logpath, name, fileExtn);
                                T2Debug("Rotated log file name is %s\n", rotatedLog);
                            }
                        }
                        if(currentLogFile != NULL)
                        {
                            free(currentLogFile);
                        }

                        if(NULL != rotatedLog)
                        {
                            fclose(pcurrentLogFile);
                            pcurrentLogFile = NULL;
                            pcurrentLogFile = fopen(rotatedLog, "rb");

                            if(NULL == pcurrentLogFile)
                            {
                                T2Debug("Error in opening file %s", rotatedLog);
                                LAST_SEEK_VALUE = 0;
                                free(rotatedLog);
                                return NULL;
                            }
                            free(rotatedLog);

                            if(fseek(pcurrentLogFile, seek_value, 0) != 0)
                            {
                                T2Error("Cannot set the file position indicator for the stream pointed to by stream\n");
                            }
                            is_rotated_log = 1;
                        }
                    }
                }
            }
        }
    }
    if(NULL != pcurrentLogFile)
    {
        rval = fgets(buf, buflen, pcurrentLogFile);
        if(NULL == rval)
        {
            long seek_value = ftell(pcurrentLogFile);
            LAST_SEEK_VALUE = seek_value;

            fclose(pcurrentLogFile);
            pcurrentLogFile = NULL;

            if(is_rotated_log == 1)
            {
                char *curLog = NULL;
                is_rotated_log = 0;
                curLog = malloc(logname_len);
                if(NULL != curLog)
                {
                    snprintf(curLog, logname_len, "%s%s", logpath, name);
                    pcurrentLogFile = fopen(curLog, "rb");

                    if(NULL == pcurrentLogFile)
                    {
                        T2Debug("Error in opening file %s", curLog);
                        LAST_SEEK_VALUE = 0;
                        free(curLog);
                        return NULL;
                    }

                    free(curLog);
                    rval = fgets(buf, buflen, pcurrentLogFile);
                    if(NULL == rval)
                    {
                        seek_value = ftell(pcurrentLogFile);

                        LAST_SEEK_VALUE = seek_value;
                        fclose(pcurrentLogFile);
                        pcurrentLogFile = NULL;
                    }
                }
            }
        }
    }
    return rval;
}
/**
 *  @brief Function to update the global paths like PERSISTENT_PATH,LOG_PATH from include.properties file.
 *
 *  @param[in] logpath   Log file path
 *  @param[in] perspath  Persistent path
 */
void updateIncludeConfVal(char *logpath, char *perspath)
{

    T2Debug("%s ++in \n", __FUNCTION__);

    FILE *file = fopen( INCLUDE_PROPERTIES, "r");
    if(NULL != file)
    {
        char props[255] = { "" };
        while(fscanf(file, "%254s", props) != EOF)
        {
            char *property = NULL;
            if((property = strstr(props, "PERSISTENT_PATH=")))
            {
                property = property + strlen("PERSISTENT_PATH=");
                if(PERSISTENT_PATH != NULL)
                {
                    free(PERSISTENT_PATH);
                }
                PERSISTENT_PATH = strdup(property);
            }
            else if((property = strstr(props, "LOG_PATH=")))
            {
                if(0 == strncmp(props, "LOG_PATH=", strlen("LOG_PATH=")))
                {
                    property = property + strlen("LOG_PATH=");
                    if(LOG_PATH != NULL)
                    {
                        free(LOG_PATH);
                    }
                    LOG_PATH = strdup(property);
                }
            }
        }
        fclose(file);
    }


    if(NULL != logpath && strcmp(logpath, "") != 0)
    {
        char *tmp = NULL;
        int logpath_len = strlen(logpath) + 1;
        tmp = realloc(LOG_PATH, logpath_len);
        if(NULL != tmp)
        {
            LOG_PATH = tmp;
            strncpy(LOG_PATH, logpath, logpath_len);
        }
        else
        {
            free(LOG_PATH);
            LOG_PATH = NULL;
        }
    }

    if(NULL != perspath && strcmp(perspath, "") != 0)
    {
        char *tmp = NULL;
        int perspath_len = strlen(perspath) + 1;
        tmp = realloc(PERSISTENT_PATH, perspath_len);
        if(NULL != tmp)
        {
            PERSISTENT_PATH = tmp;
            strncpy(PERSISTENT_PATH, perspath, perspath_len);
        }
        else
        {
            free(PERSISTENT_PATH);
            PERSISTENT_PATH = NULL;
        }
    }

    T2Debug("%s --out \n", __FUNCTION__);

}
/**
 *  @brief Function to update the configuration values from device.properties file.
 *
 *  @param[in] logpath   Log file path
 *  @param[in] perspath  Persistent path
 */
void initProperties(char *logpath, char *perspath)
{


    T2Debug("%s ++in \n", __FUNCTION__);

    FILE *file = NULL;

    file = fopen( DEVICE_PROPERTIES, "r");
    if(NULL != file)
    {
        char props[255] = { "" };
        while(fscanf(file, "%254s", props) != EOF)
        {
            char *property = NULL;
            if((property = strstr(props, "DEVICE_TYPE=")))
            {
                property = property + strlen("DEVICE_TYPE=");
                if(DEVICE_TYPE != NULL)
                {
                    free(DEVICE_TYPE);
                }
                DEVICE_TYPE = strdup(property);
                break;
            }
        }
        fclose(file);
    }

    updateIncludeConfVal(logpath, perspath);

    if(NULL != DEVICE_TYPE && NULL != PERSISTENT_PATH && NULL != LOG_PATH)
    {
        int logpath_len = strlen(LOG_PATH);
        int perspath_len = strlen(PERSISTENT_PATH);
        if(0 == strcmp("broadband", DEVICE_TYPE))   // Update config for broadband
        {
            char *tmp_seek_file = "/.telemetry/tmp/rtl_";
            char *tmp_log_file = "/";
            char *tmp = NULL;
            int tmp_seek_len = strlen(tmp_seek_file) + 1;
            int tmp_log_len = strlen(tmp_log_file) + 1;
            if(NULL == perspath || strcmp(perspath, "") == 0)
            {
                tmp = realloc(PERSISTENT_PATH, perspath_len + tmp_seek_len);
                if(NULL != tmp)
                {
                    PERSISTENT_PATH = tmp;
                    strncat(PERSISTENT_PATH, tmp_seek_file, tmp_seek_len);
                }
                else
                {
                    free(PERSISTENT_PATH);
                    PERSISTENT_PATH = NULL;
                }
            }

            if(NULL == logpath || strcmp(logpath, "") == 0)
            {
                tmp = realloc(LOG_PATH, logpath_len + tmp_log_len);;
                if(NULL != tmp)
                {
                    LOG_PATH = tmp;
                    strncat(LOG_PATH, tmp_log_file, tmp_log_len);
                }
                else
                {
                    free(LOG_PATH);
                    LOG_PATH = NULL;
                }
            }
        }
        else
        {
            /* FIXME */
            char *tmp_seek_file = DEFAULT_SEEK_PREFIX;
            char *tmp_log_file = DEFAULT_LOG_PATH;
            char *tmp = NULL;
            int tmp_seek_len = strlen(tmp_seek_file) + 1;
            int tmp_log_len = strlen(tmp_log_file) + 1;
            if(NULL == perspath || strcmp(perspath, "") == 0)
            {
                tmp = realloc(PERSISTENT_PATH, tmp_seek_len);
                if(NULL != tmp)
                {
                    PERSISTENT_PATH = tmp;
                    strncpy(PERSISTENT_PATH, tmp_seek_file, tmp_seek_len);
                }
                else
                {
                    free(PERSISTENT_PATH);
                    PERSISTENT_PATH = NULL;
                }
            }

            if(NULL == logpath || strcmp(logpath, "") == 0)
            {
                tmp = realloc(LOG_PATH, tmp_log_len);
                if(NULL != tmp)
                {
                    LOG_PATH = tmp;
                    strncpy(LOG_PATH, tmp_log_file, tmp_log_len);
                }
                else
                {
                    free(LOG_PATH);
                    LOG_PATH = NULL;
                }
            }
        }
    }

    isPropsIntialized = true ;
    T2Debug("%s --out \n", __FUNCTION__);
}

bool isPropsInitialized()
{

    return isPropsIntialized ;
}