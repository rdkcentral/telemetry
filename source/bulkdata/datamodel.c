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

#include <stdlib.h>
#include <cjson/cJSON.h>
#include <pthread.h>
#include <string.h>
#include "datamodel.h"
#include "persistence.h"
#include "t2collection.h"
#include "reportprofiles.h"
#include "t2log_wrapper.h"
#if defined(CCSP_SUPPORT_ENABLED)
#include "t2_custom.h"
#endif

static bool               stopProcessing = true;
static queue_t            *rpQueue = NULL;
static queue_t            *tmpRpQueue = NULL;
static pthread_t          rpThread;
static pthread_t          tmpRpThread;
static pthread_mutex_t    rpMutex;
static pthread_mutex_t    tmpRpMutex;
static pthread_cond_t     rpCond;
static pthread_cond_t     tmpRpCond;
static queue_t            *rpMsgPkgQueue = NULL;
static pthread_t          rpMsgThread;
static pthread_mutex_t    rpMsgMutex;
static pthread_cond_t     msg_Cond;
/**
 * Thread function to receive report profiles Json object
 */
static void *process_rp_thread(void *data)
{
    (void) data;//To fix compiler warning
    cJSON *reportProfiles = NULL;

    T2Debug("%s ++in\n", __FUNCTION__);

    while(!stopProcessing)
    {
        pthread_mutex_lock(&rpMutex);
        T2Info("%s: Waiting for event from tr-181 \n", __FUNCTION__);
        pthread_cond_wait(&rpCond, &rpMutex);

        T2Debug("%s: Received wake up signal \n", __FUNCTION__);
        if(t2_queue_count(rpQueue) > 0)
        {
            reportProfiles = (cJSON *)t2_queue_pop(rpQueue);
            if (reportProfiles)
            {
                ReportProfiles_ProcessReportProfilesBlob(reportProfiles, T2_RP);
                cJSON_Delete(reportProfiles);
                // Unused value reportProfiles
            }
        }
        pthread_mutex_unlock(&rpMutex);
    }
    T2Debug("%s --out\n", __FUNCTION__);
    return NULL;
}

static void *process_tmprp_thread(void *data)
{
    (void) data;//To fix compiler warning
    cJSON *tmpReportProfiles = NULL;

    T2Debug("%s ++in\n", __FUNCTION__);

    while(!stopProcessing)
    {
        pthread_mutex_lock(&tmpRpMutex);
        T2Info("%s: Waiting for event from tr-181 \n", __FUNCTION__);
        pthread_cond_wait(&tmpRpCond, &tmpRpMutex);

        T2Debug("%s: Received wake up signal \n", __FUNCTION__);
        if(t2_queue_count(tmpRpQueue) > 0)
        {
            tmpReportProfiles = (cJSON *)t2_queue_pop(tmpRpQueue);
            if (tmpReportProfiles)
            {
                ReportProfiles_ProcessReportProfilesBlob(tmpReportProfiles, T2_TEMP_RP);
                cJSON_Delete(tmpReportProfiles);
                // Unused value tmpReportProfiles
            }
        }
        pthread_mutex_unlock(&tmpRpMutex);
    }
    T2Debug("%s --out\n", __FUNCTION__);
    return NULL;
}

static void *process_msg_thread(void *data)
{
    (void) data;//To fix compiler warning
    struct __msgpack__ *msgpack;
    while(!stopProcessing)
    {
        pthread_mutex_lock(&rpMsgMutex);
        pthread_cond_wait(&msg_Cond, &rpMsgMutex);
        if(t2_queue_count(rpMsgPkgQueue) > 0)
        {
            msgpack = (struct __msgpack__ *)t2_queue_pop(rpMsgPkgQueue);
            if (msgpack)
            {
                ReportProfiles_ProcessReportProfilesMsgPackBlob(msgpack->msgpack_blob, msgpack->msgpack_blob_size);
                free(msgpack);
            }
        }
        pthread_mutex_unlock(&rpMsgMutex);
    }
    return NULL;
}

/* Description:
 *      The API validate JSON format and check if 'profiles' field is present in JSON data.
 *      It saves the received json data in temporary file and
 *      notify process_rp_thread() for incoming data.
 * Arguments:
 *      char *JsonBlob         List of active profiles.
 */
T2ERROR datamodel_processProfile(char *JsonBlob, bool rprofiletypes)
{
    cJSON *rootObj = NULL;
    cJSON *profiles = NULL;

    if((rootObj = cJSON_Parse(JsonBlob)) == NULL)
    {
        T2Error("%s: JSON parsing failure\n", __FUNCTION__);
        return T2ERROR_FAILURE;
    }
    if((profiles = cJSON_GetObjectItem(rootObj, "profiles")) == NULL)
    {
        T2Error("%s: Missing required param 'profiles' \n", __FUNCTION__);
        cJSON_Delete(rootObj);
        return T2ERROR_FAILURE;
    }

    T2Info("Number of report profiles in configuration is %d \n", cJSON_GetArraySize(profiles));

    pthread_mutex_lock(&rpMutex);
    if (!stopProcessing)
    {
        if(rprofiletypes == T2_RP)
        {
            t2_queue_push(rpQueue, (void *)rootObj);
            pthread_cond_signal(&rpCond);
        }
        else if(rprofiletypes == T2_TEMP_RP)
        {
            t2_queue_push(tmpRpQueue, (void *)rootObj);
            pthread_cond_signal(&tmpRpCond);
        }
    }
    else
    {
        T2Error("Datamodel not initialized, dropping request \n");
        cJSON_Delete(rootObj);
    }
    pthread_mutex_unlock(&rpMutex);
    return T2ERROR_SUCCESS;
}

/**
 * Caller is responsible for freeing the allocated memory in passed reference
 */
void datamodel_getSavedJsonProfilesasString(char** SavedProfiles)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    size_t configIndex = 0;
    Vector *configList = NULL;
    Config *config = NULL;
    Vector_Create(&configList);
    fetchLocalConfigs(REPORTPROFILES_PERSISTENCE_PATH, configList);
    if (Vector_Size(configList) > 0)
    {
        cJSON *valArray = NULL;
        cJSON *jsonObj = cJSON_CreateObject();
        cJSON_AddItemToObject(jsonObj, "Profiles", valArray = cJSON_CreateArray());
        for(; configIndex < Vector_Size(configList); configIndex++)
        {
            config = Vector_At(configList, configIndex);
            T2Debug("Processing config with name : %s\n", config->name);
            cJSON *temparrayItem = cJSON_CreateObject();
            cJSON_AddStringToObject(temparrayItem, "name", config->name);
            cJSON *tempObject = cJSON_Parse(config->configData);
            cJSON *temp = cJSON_GetObjectItem(tempObject, "Hash");
            cJSON_AddStringToObject(temparrayItem, "Hash", temp->valuestring);
            cJSON_DeleteItemFromObject(tempObject, "Hash");
            cJSON_AddItemToObject(temparrayItem, "value", tempObject);
            cJSON_AddItemToArray(valArray, temparrayItem);
        }
        *SavedProfiles = cJSON_PrintUnformatted(jsonObj);
        cJSON_Delete(jsonObj);
    }
    Vector_Destroy(configList, free);
    T2Debug("%s --out\n", __FUNCTION__);
}

/**
 * Caller is responsible for freeing the allocated memory in passed reference
 */
int datamodel_getSavedMsgpackProfilesasString(char** SavedProfiles)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    //#if defined(FEATURE_SUPPORT_WEBCONFIG) get should work without the restriction of webconfig
    char filePath[REPORTPROFILES_FILE_PATH_SIZE] = {'\0'};
    snprintf(filePath, sizeof(filePath), "%s%s", REPORTPROFILES_PERSISTENCE_PATH, MSGPACK_REPORTPROFILES_PERSISTENT_FILE);
    FILE *fp;
    fp = fopen (filePath, "rb");
    if(fp != NULL)
    {
        T2Info("Msgpack: loadReportProfilesFromDisk \n");
        struct __msgpack__ msgpack;
        fseek(fp, 0L, SEEK_END);
        msgpack.msgpack_blob_size = ftell(fp);
        if(msgpack.msgpack_blob_size < 0)
        {
            T2Error("Unable to detect the file pointer position for file %s\n", filePath);
            fclose(fp);
            return 0;
        }
        msgpack.msgpack_blob = malloc(sizeof(char) * msgpack.msgpack_blob_size);
        if (NULL == msgpack.msgpack_blob)
        {
            T2Error("Unable to allocate %d bytes of memory at Line %d on %s \n",
                    msgpack.msgpack_blob_size, __LINE__, __FILE__);
            fclose (fp);
            return 0;
        }
        fseek(fp, 0L, SEEK_SET);
        if(fread(msgpack.msgpack_blob, sizeof(char), msgpack.msgpack_blob_size, fp) < (size_t)msgpack.msgpack_blob_size)
        {
            T2Error("fread is returning fewer bytes than expected from the file %s\n", filePath);
            free(msgpack.msgpack_blob);
            fclose(fp);
            return 0;
        }
        fclose (fp);
        T2Debug("%s --out\n", __FUNCTION__);
        *SavedProfiles = msgpack.msgpack_blob;
        return msgpack.msgpack_blob_size;
    }
    //#endif
    T2Debug("%s --out\n", __FUNCTION__);
    return 0;
}

T2ERROR datamodel_MsgpackProcessProfile(char *str, int strSize)
{
    struct __msgpack__ *msgpack;
    msgpack = (struct __msgpack__ *)malloc(sizeof(struct __msgpack__));

    if (msgpack == NULL)
    {
        return T2ERROR_FAILURE;
    }

    msgpack->msgpack_blob = str;
    msgpack->msgpack_blob_size = strSize;
    pthread_mutex_lock(&rpMsgMutex);
    if (!stopProcessing)
    {
        t2_queue_push(rpMsgPkgQueue, (void *)msgpack);
        pthread_cond_signal(&msg_Cond);
    }
    else
    {
        free(msgpack->msgpack_blob);
        free(msgpack);
        T2Error("Datamodel not initialized, dropping request \n");
    }
    pthread_mutex_unlock(&rpMsgMutex);
    return T2ERROR_SUCCESS;
}

/* Description:
 *      This API initializes message queue.
 * Arguments:
 *      NULL
 */

T2ERROR datamodel_init(void)
{
    T2Debug("%s ++in\n", __FUNCTION__);
    printf("%s : %d \n", __func__, __LINE__);
    rpQueue = t2_queue_create();
    printf("%s : %d \n", __func__, __LINE__);
    if (rpQueue == NULL)
    {
        T2Error("Failed to create report profile Queue\n");
        return T2ERROR_FAILURE;
    }
    rpMsgPkgQueue = t2_queue_create();
    printf("%s : %d \n", __func__, __LINE__);
    if (rpMsgPkgQueue == NULL)
    {
        T2Error("Failed to create Msg Pck report profile Queue\n");
        return T2ERROR_FAILURE;
    }
    tmpRpQueue = t2_queue_create();
    printf("%s : %d \n", __func__, __LINE__);
    if (tmpRpQueue == NULL)
    {
        T2Error("Failed to create report profile Queue\n");
        return T2ERROR_FAILURE;
    }

    pthread_mutex_init(&rpMutex, NULL);
    pthread_cond_init(&rpCond, NULL);
    pthread_mutex_init(&rpMsgMutex, NULL);
    pthread_cond_init(&msg_Cond, NULL);
    pthread_mutex_init(&tmpRpMutex, NULL);
    pthread_cond_init(&tmpRpCond, NULL);

    pthread_mutex_lock(&rpMutex);
    stopProcessing = false;
    pthread_mutex_unlock(&rpMutex);
    printf("%s : %d \n", __func__, __LINE__);
    pthread_create(&rpThread, NULL, process_rp_thread, (void *)NULL);
    printf("%s : %d \n", __func__, __LINE__);
    pthread_create(&rpMsgThread, NULL, process_msg_thread, (void *)NULL);
    printf("%s : %d \n", __func__, __LINE__);
    pthread_create(&tmpRpThread, NULL, process_tmprp_thread, (void *)NULL);
    printf("%s : %d \n", __func__, __LINE__);

    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

void datamodel_unInit(void)
{
    T2Debug("%s ++in\n", __FUNCTION__);

    pthread_mutex_lock(&rpMutex);
    stopProcessing = true;
    pthread_cond_signal(&rpCond);
    pthread_mutex_unlock(&rpMutex);
    pthread_mutex_lock(&rpMsgMutex);
    pthread_cond_signal(&msg_Cond);
    pthread_mutex_unlock(&rpMsgMutex);
    pthread_join(rpThread, NULL);
    pthread_mutex_destroy(&rpMutex);
    pthread_cond_destroy(&rpCond);
    pthread_join(rpMsgThread, NULL);
    pthread_mutex_destroy(&rpMsgMutex);
    pthread_cond_destroy(&msg_Cond);
    pthread_join(tmpRpThread, NULL);
    pthread_mutex_destroy(&tmpRpMutex);
    pthread_cond_destroy(&tmpRpCond);

    T2Debug("%s --out\n", __FUNCTION__);
}
