/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2019 RDK Management
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
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#ifdef LIBSYSWRAPPER_BUILD
#include "secure_wrapper.h"
#endif

#include "persistence.h"
#include "t2log_wrapper.h"
#include "telemetry2_0.h"

#define MAX_FILENAME_LENGTH 128

static pthread_once_t persistCahedReporMutexOnce = PTHREAD_ONCE_INIT;
static pthread_mutex_t persistCachedReportMutex;

static void persistReportMethodInit( )
{
    if(pthread_mutex_init(&persistCachedReportMutex, NULL) != 0)
    {
        return;
    }
}


T2ERROR fetchLocalConfigs(const char* path, Vector *configList)
{
    if(path == NULL || ((strcmp(path, SHORTLIVED_PROFILES_PATH) != 0) && configList == NULL))
    {
        T2Error("Path is NULL or Configlist is NULL.. Invalid argument\n");
        return T2ERROR_INVALID_ARGS;
    }
    struct dirent *entry;
    T2Debug("%s ++in\n", __FUNCTION__);
    DIR *dir = opendir(path);
    if (dir == NULL)
    {
        T2Info("Failed to open persistence folder : %s, creating folder\n", path);
        if (mkdir(path, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) != 0)
        {
            T2Error("%s,%d: Failed to make directory : %s  \n", __FUNCTION__, __LINE__, path);
        }

        return T2ERROR_FAILURE;
    }
    if(strcmp(path, SHORTLIVED_PROFILES_PATH) == 0)
    {
        T2Debug("%s alreay created : \n", SHORTLIVED_PROFILES_PATH);
        T2Debug("clearing short lived profile from the disk \n");
        clearPersistenceFolder(SHORTLIVED_PROFILES_PATH);
        if(closedir(dir) != 0)
        {
            T2Error("%s,%d: Failed to close persistent folder\n", __FUNCTION__, __LINE__);
            return T2ERROR_FAILURE;
        }
        return T2ERROR_SUCCESS;
    }
#if defined(DROP_ROOT_PRIV)
#ifdef LIBSYSWRAPPER_BUILD
    v_secure_system("chmod 755 %s", path);
#else
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "chmod 755 %s", path);
    system(cmd);
#endif
#endif

    while ((entry = readdir(dir)) != NULL)
    {
        struct stat filestat;
        int         status;
        char absfilepath[256] = {'\0'};

        if(entry->d_name[0] == '.' || (strcmp(entry->d_name, MSGPACK_REPORTPROFILES_PERSISTENT_FILE) == 0))
        {
            continue;
        }

        snprintf(absfilepath, sizeof(absfilepath), "%s%s", path, entry->d_name);
        T2Debug("Config file : %s\n", absfilepath);
        int fp = open(absfilepath, O_RDONLY);
        if(fp == -1)
        {
            T2Error("Failed to open file : %s\n", entry->d_name);
            continue;
        }

        status = fstat(fp, &filestat);
        if(status == 0)
        {
            T2Info("Filename : %s Size : %ld\n", entry->d_name, (long int)filestat.st_size);

            Config *config = (Config *)malloc(sizeof(Config));
            memset(config, 0, sizeof(Config));
            config->name = strdup(entry->d_name);
            config->configData = (char *)malloc((filestat.st_size + 1) * sizeof(char));
            memset( config->configData, 0, (filestat.st_size + 1 ));
            int read_size = read(fp, config->configData, filestat.st_size);
            config->configData[filestat.st_size] = '\0';

            if(read_size != filestat.st_size)
            {
                T2Error("read size = %d filestat.st_size = %lu\n", read_size, (unsigned long)filestat.st_size);
            }
            close(fp);
            Vector_PushBack(configList, config);

            T2Debug("Config data size = %lu\n", (unsigned long)strlen(config->configData));
            T2Debug("Config data = %s\n", config->configData);
        }
        else
        {
            T2Error("Unable to stat, Invalid file : %s\n", entry->d_name);
            close(fp);
            continue;
        }
    }

    T2Info("Returning %lu local configurations \n", (unsigned long)Vector_Size(configList));
    closedir(dir);
    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

T2ERROR saveConfigToFile(const char* path, const char *profileName, const char* configuration)
{
    if(path == NULL || profileName == NULL || configuration == NULL)
    {
        return T2ERROR_INVALID_ARGS;
    }
    FILE *fp = NULL;
    char filePath[256] = {'\0'};
    T2Debug("%s ++in\n", __FUNCTION__);

    if(strlen(profileName) > MAX_FILENAME_LENGTH)
    {
        T2Error("profileID exceeds max limit of %d chars\n", MAX_FILENAME_LENGTH);
        return T2ERROR_FAILURE;
    }
    snprintf(filePath, sizeof(filePath), "%s%s", path, profileName);

    fp = fopen(filePath, "w");
    if(fp == NULL)
    {
        T2Error("Unable to write to file : %s\n", filePath);
        return T2ERROR_FAILURE;
    }
    fprintf(fp, "%s", configuration);
    if(fclose(fp) != 0)
    {
        T2Error("Unable to close file : %s\n", filePath);
        return T2ERROR_FAILURE;
    }
    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

T2ERROR MsgPackSaveConfig(const char* path, const char *fileName, const char *msgpack_blob, size_t blob_size)
{
    if(path == NULL || fileName == NULL || msgpack_blob == NULL)
    {
        return T2ERROR_INVALID_ARGS;
    }
    FILE *fp;
    char filePath[256] = {'\0'};

    if(strlen(fileName) > MAX_FILENAME_LENGTH)
    {
        T2Error("fileName exceeds max limit of %d chars\n", MAX_FILENAME_LENGTH);
        return T2ERROR_FAILURE;
    }
    snprintf(filePath, sizeof(filePath), "%s%s", path, fileName);
    fp = fopen(filePath, "wb");
    if (NULL == fp)
    {
        T2Error("%s file open is failed \n", filePath);
        return T2ERROR_FAILURE;
    }
    fwrite(msgpack_blob, sizeof(char), blob_size, fp);
    if(fclose(fp) != 0)
    {
        T2Error("%s file close is failed \n", filePath);
        return T2ERROR_FAILURE;
    }
    return T2ERROR_SUCCESS;
}

void clearPersistenceFolder(const char* path)
{

    T2Debug("%s ++in\n", __FUNCTION__);
    if(path == NULL)
    {
        return;
    }
#ifdef LIBSYSWRAPPER_BUILD
    T2Debug("Executing command : rm -f %s* \n", path);
    if (v_secure_system("sh -c 'rm -rf %s*'", path) != 0)
    {
        T2Error("%s,%d:command failed\n", __FUNCTION__, __LINE__);
        return;
    }
#else
    char command[256] = {'\0'};
    snprintf(command, sizeof(command), "rm -f %s*", path);
    T2Debug("Executing command : %s\n", command);
    if (system(command) != 0)
    {
        T2Error("%s,%d: %s command failed\n", __FUNCTION__, __LINE__, command);
        return;
    }
#endif

    T2Debug("%s --out\n", __FUNCTION__);

}

void removeProfileFromDisk(const char* path, const char* fileName)
{
    if(path == NULL || fileName == NULL)
    {
        return;
    }
    size_t len = strlen(path) + strlen(fileName) + 1;
    char *str = malloc(len);
    if (! str)
    {
        T2Error("%s,%d: memory allocation failed\n", __FUNCTION__, __LINE__);
        return;
    }
    snprintf(str, len, "%s%s", path, fileName);
    if (unlink(str) != 0)
    {
        T2Error("%s,%d: command failed\n", __FUNCTION__, __LINE__);
    }
    free(str);

    T2Debug("%s --out\n", __FUNCTION__);
}

T2ERROR saveCachedReportToPersistenceFolder(const char *profileName, Vector *reportList)
{
    T2ERROR ret = T2ERROR_FAILURE ;

    T2Debug("%s ++in\n", __FUNCTION__);

    if( NULL == profileName || NULL == reportList )
    {
        T2Error("%s : %d Either of input arguments are NULL \n", __FUNCTION__, __LINE__);
        return ret ;
    }

    // Lock a mutex to write to common file
    pthread_once(&persistCahedReporMutexOnce, persistReportMethodInit);


    // Create directory at CACHED_MESSAGE_PATH if not present
    DIR *dir = opendir(CACHED_MESSAGE_PATH);
    FILE *filePtr = NULL ;
    char absFilePath[MAX_FILENAME_LENGTH] = {'0'};

    pthread_mutex_lock(&persistCachedReportMutex);
    if(dir == NULL)
    {
        T2Info("Persistence folder %s not present, creating folder\n", CACHED_MESSAGE_PATH);
        if(mkdir(CACHED_MESSAGE_PATH, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) != 0)
        {
            T2Error("%s,%d: Failed to make directory : %s  \n", __FUNCTION__, __LINE__, CACHED_MESSAGE_PATH);
            pthread_mutex_unlock(&persistCachedReportMutex);
            T2Debug("%s --out\n", __FUNCTION__);
            return T2ERROR_FAILURE;
        }
    }
    else
    {
        closedir(dir);
    }

    snprintf(absFilePath, (MAX_FILENAME_LENGTH - 1), "%s/%s", CACHED_MESSAGE_PATH, profileName );
    filePtr = fopen(absFilePath, "w+");
    if(NULL != filePtr)
    {
        // if absFilePath present, wipe it off
        int vectorSize = Vector_Size(reportList);
        int loop = 0 ;
        T2Debug("Writing %d data to file %s \n", vectorSize, absFilePath);
        if(vectorSize > 0)
        {
            char *payload = (char*) Vector_At(reportList, loop);
            fprintf(filePtr, "%s", payload);
            for(loop = 1; loop < vectorSize; loop++ )
            {
                char *payload = (char*) Vector_At(reportList, loop);
                fprintf(filePtr, "\n%s", payload);
            }
        }
        fclose(filePtr);
        ret = T2ERROR_SUCCESS ;
    }
    else
    {
        T2Error("Unable to open file %s for caching unsent reports \n", absFilePath);
    }
    // Release the mutex
    pthread_mutex_unlock(&persistCachedReportMutex);
    T2Debug("%s --out\n", __FUNCTION__);
    return ret;
}


T2ERROR populateCachedReportList(const char *profileName, Vector *outReportList)
{
    T2ERROR ret = T2ERROR_FAILURE ;

    T2Debug("%s ++in\n", __FUNCTION__);

    if( NULL == profileName || NULL == outReportList )
    {
        T2Error("%s : %d Either of input arguments are NULL \n", __FUNCTION__, __LINE__);
        T2Debug("%s --out\n", __FUNCTION__);
        return ret ;
    }

    FILE *filePtr = NULL ;
    char absFilePath[MAX_FILENAME_LENGTH] = {'0'};

    pthread_once(&persistCahedReporMutexOnce, persistReportMethodInit);
    pthread_mutex_lock(&persistCachedReportMutex);

    snprintf(absFilePath, (MAX_FILENAME_LENGTH - 1), "%s/%s", CACHED_MESSAGE_PATH, profileName );
    filePtr = fopen(absFilePath, "r+");
    if(NULL != filePtr)
    {
        char *payload = NULL ;
        size_t dataLength = 1 ;
        ssize_t linelength; // Use ssize_t to handle return value from getline
        payload = (char *) malloc(1);
        T2Info("Reading data from file %s \n", absFilePath);
        while((linelength = getline(&payload, &dataLength, filePtr)) != -1)
        {
            T2Debug("Payload Value = %s\n", payload);
            if (linelength < 2)
            {
                continue;
            }
            Vector_PushBack(outReportList, (void *)strdup(payload));
            T2Debug("vector size = %lu\n", (unsigned long )Vector_Size(outReportList));
            if(payload)
            {
                free(payload);
                payload = NULL ;
            }
        }
        if(payload)
        {
            free(payload);
        }
        fclose(filePtr);
        if (remove(absFilePath) == 0)
        {
            T2Info("Remove cached report file - %s \n", absFilePath);
        }
        else
        {
            T2Error("Unable to remove cached report file - %s \n", absFilePath);
            pthread_mutex_unlock(&persistCachedReportMutex);
            return T2ERROR_FAILURE;
        }
        ret = T2ERROR_SUCCESS ;
    }
    else
    {
        T2Debug("Unable to open file %s. \n", absFilePath);
    }
    pthread_mutex_unlock(&persistCachedReportMutex);

    T2Debug("%s --out\n", __FUNCTION__);
    return ret;
}

//Privacy mode functions
T2ERROR savePrivacyModeToPersistentFolder(char *data)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    clearPersistenceFolder(PRIVACYMODE_PATH);
    DIR *dir = opendir(PRIVACYMODE_PATH);
    FILE *fp = NULL;
    char filePath[256] = {'\0'};
    if(dir == NULL)
    {
        T2Info("Persistence folder %s not present, creating folder\n", PRIVACYMODE_PATH);
        if(mkdir(PRIVACYMODE_PATH, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) != 0)
        {
            T2Error("%s,%d: Failed to make directory : %s  \n", __FUNCTION__, __LINE__, PRIVACYMODE_PATH);
            return T2ERROR_FAILURE;
        }
    }
    else
    {
        closedir(dir);
    }
#if defined(DROP_ROOT_PRIV)
#ifdef LIBSYSWRAPPER_BUILD
    v_secure_system("chmod 777 %s", PRIVACYMODE_PATH);
#else
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "chmod 777 %s", PRIVACYMODE_PATH);
    system(cmd);
#endif
#endif

    snprintf(filePath, sizeof(filePath), "%s/%s", PRIVACYMODE_PATH, "privacymodes.txt");
    fp = fopen(filePath, "w+");
    if(fp == NULL)
    {
        T2Error("Unable to write to file : %s\n", filePath);
        return T2ERROR_FAILURE;
    }
    fprintf(fp, "%s", data);
    if(fclose(fp) != 0)
    {
        T2Error("Unable to close file : %s\n", filePath);
        return T2ERROR_FAILURE;
    }

    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

T2ERROR getPrivacyModeFromPersistentFolder(char **privMode)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    FILE *fp = NULL;
    char filePath[256] = {'\0'};
    char data[20] = {'\0'};
    struct stat filestat;
    snprintf(filePath, sizeof(filePath), "%s/%s", PRIVACYMODE_PATH, "privacymodes.txt");
    fp = fopen(filePath, "r+");
    if(fp == NULL)
    {
        T2Error("Unable to open the file : %s\n", filePath);
        return T2ERROR_FAILURE;
    }
    if (stat(filePath, &filestat) != 0)
    {
        T2Error("Unable to stat file : %s\n", filePath);
        fclose(fp);
        return T2ERROR_FAILURE;
    }
    if (fread(data, sizeof(char), filestat.st_size, fp) != (size_t)filestat.st_size)
    {
        T2Error("Failed to read complete data from file : %s\n", filePath);
        fclose(fp);
        return T2ERROR_FAILURE;
    }
    *privMode = strdup(data);
    if(fclose(fp) != 0)
    {
        T2Error("Unable to close file : %s\n", filePath);
        return T2ERROR_INTERNAL_ERROR;
    }
    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}






