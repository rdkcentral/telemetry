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
    if (data)
    {
        hash_element_t *element = (hash_element_t *)data;

        if (element->key) {
            free(element->key);
            element->key = NULL;
        }
        if (element->data) {
            free(element->data);
            element->data = NULL;
        }
        free(element);
        element = NULL;
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
 *  @brief Function to write the rotated Log file.
 *
 *  @param[in] name        Log file name.
 *
 *  @return Returns the status of the operation.
 *  @retval Returns -1 on failure, appropriate errorcode otherwise.
 */
T2ERROR updateLogSeek(hash_map_t *logSeekMap, const char* logFileName,const long logfileSize)
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
            loadAvg->markerName = strndup("Load_Average", (strlen("Load_Average") + 1));
            loadAvg->markerValue = strndup(str, LEN);
            loadAvg->trimParameter = trim;
            loadAvg->regexParameter = regex;
            Vector_PushBack(grepResultList, loadAvg);
        }
    }
    T2Debug("%s --out \n", __FUNCTION__);
    return 1;
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