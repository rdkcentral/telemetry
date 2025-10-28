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
#include <unistd.h>

#include "t2log_wrapper.h"
#include "legacyutils.h"
#include "vector.h"
#include "dcautil.h"

#define EC_BUF_LEN 20

static char* PERSISTENT_PATH = NULL;
static char* LOG_PATH        = NULL;
static char* DEVICE_TYPE     = NULL;

/**
 * Start of functions dealing with log seek values
 */

static void freeLogFileSeekMap(void *data)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if (data)
    {
        hash_element_t *element = (hash_element_t *)data;

        if (element->key)
        {
            free(element->key);
            element->key = NULL;
        }
        if (element->data)
        {
            free(element->data);
            element->data = NULL;
        }
        free(element);
        element = NULL;
    }
    T2Debug("%s --out\n", __FUNCTION__);
}

void freeGrepSeekProfile(GrepSeekProfile *gsProfile)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if (gsProfile)
    {
        hash_map_destroy(gsProfile->logFileSeekMap, freeLogFileSeekMap);
        free(gsProfile);
    }
    T2Debug("%s --out\n", __FUNCTION__);
}

GrepSeekProfile *createGrepSeekProfile(int execCounter)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    GrepSeekProfile *gsProfile = (GrepSeekProfile *)malloc(sizeof(GrepSeekProfile));
    if (gsProfile)
    {
        gsProfile->logFileSeekMap = hash_map_create();
        gsProfile->execCounter = execCounter;
    }
    else
    {
        T2Error("Failed to allocate memory for GrepSeekProfile\n");
    }
    T2Debug("%s --out\n", __FUNCTION__);
    return gsProfile;
}
/* This is used to free the hash map for profile seek map but it is no more needed
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
int getLoadAvg(TopMarker* marker)
{
    T2Debug("%s ++in \n", __FUNCTION__);
    FILE *fp;
    char str[LEN + 1];
    if(marker == NULL)
    {
        T2Debug("marker is NULL\n");
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


    if(marker->loadAverage)
    {
        free(marker->loadAverage);
    }
    marker->loadAverage = strndup(str, LEN);

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
void updateIncludeConfVal(char **logpath, char **perspath)
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
    if(LOG_PATH == NULL)
    {
        LOG_PATH = DEFAULT_LOG_PATH;
    }
    if (perspath)
    {
        *perspath = PERSISTENT_PATH;
    }
    if (logpath)
    {
        *logpath = LOG_PATH;
    }
    T2Debug("%s --out \n", __FUNCTION__);
}

/**
 *  @brief Function to update the configuration values from device.properties file.
 *
 *  @param[in] logpath   Log file path
 *  @param[in] perspath  Persistent path
 */
void initProperties(char **logpath, char **perspath, long* pagesize)
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

    *pagesize = sysconf(_SC_PAGESIZE);

    T2Debug("%s --out \n", __FUNCTION__);
}
