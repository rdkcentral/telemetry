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
#include <string.h>

#include "t2eventreceiver.h"

#include "t2collection.h"
#include "t2markers.h"
#include "telemetry2_0.h"
#include "profile.h"
#include "t2log_wrapper.h"
#include "busInterface.h"
#include "dca.h"

#define T2EVENTQUEUE_MAX_LIMIT 200
#define MESSAGE_DELIMITER "<#=#>"

static queue_t *eQueue = NULL;
static bool EREnabled = false;
static bool stopDispatchThread = true;

static pthread_t erThread;
static pthread_mutex_t erMutex;
static pthread_cond_t erCond;
static pthread_mutex_t sTDMutex;

T2ERROR ReportProfiles_storeMarkerEvent(char *profileName, T2Event *eventInfo);

void freeT2Event(void *data)
{
    if(data != NULL)
    {
        T2Event *event = (T2Event *)data;
        free(event->name);
        free(event->value);
        free(event);
    }
}

void T2ER_PushDataWithDelim(char* eventInfo, char* user_data) {
    (void) user_data; //To fic compiler warning.
    T2Debug("%s ++in\n", __FUNCTION__);
    if(EREnabled) {
        if(!eventInfo) {
            T2Error("EventInfo is NULL, ignoring the notification\n");
        }else {
            pthread_mutex_lock(&erMutex);
            T2Debug("Received eventInfo : %s\n", eventInfo);
            char* token = strSplit(eventInfo, MESSAGE_DELIMITER);
            if(token != NULL) {
                T2Event *event = (T2Event *) malloc(sizeof(T2Event));
                if(event) {
                    event->name = strdup(token);
                    token = strSplit(NULL, MESSAGE_DELIMITER);
                    if(token != NULL) {
                        event->value = strdup(token);
                        if(t2_queue_count(eQueue) > T2EVENTQUEUE_MAX_LIMIT) {
                            T2Warning("T2EventQueue max limit : %d reached, dropping packet for  eventName : %s eventValue : %s\n",
                                    T2EVENTQUEUE_MAX_LIMIT, event->name, event->value);
                            free(event->value);
                            free(event->name);
                            free(event);
                        }else {
                            T2Debug("Adding eventName : %s eventValue : %s to t2event queue\n", event->name, event->value);
                            t2_queue_push(eQueue, (void *) event);
                            if(!stopDispatchThread)
                                pthread_cond_signal(&erCond);
                        }
                    }else {
                        free(event->name);
                        free(event);
                        T2Error("Missing event value\n");
                    }
                }
            }else {
                T2Error("Missing delimiter in the event received\n");
            }
            pthread_mutex_unlock(&erMutex);
        }
    }else {
        T2Warning("ER is not initialized, ignoring telemetry events for now\n");
    }
    T2Debug("%s --out\n", __FUNCTION__);
}

void T2ER_Push(char* eventName, char* eventValue) {
    T2Debug("%s ++in\n", __FUNCTION__);
    if(EREnabled) {
        if(!eventName || !eventValue) {
            T2Error("EventName or EventValue is NULL, ignoring the notification\n");
        }else {
            pthread_mutex_lock(&erMutex);
            if(t2_queue_count(eQueue) > T2EVENTQUEUE_MAX_LIMIT) {
                T2Warning("T2EventQueue max limit : %d reached, dropping packet for eventName : %s eventValue : %s\n", 
			   T2EVENTQUEUE_MAX_LIMIT, eventName, eventValue);
            }else {
                T2Debug("Received eventInfo : %s value : %s\n", eventName, (char* ) eventValue);
                T2Event *event = (T2Event *) malloc(sizeof(T2Event));
                if(event != NULL) {
                    event->name = strdup(eventName);
                    event->value = strdup(eventValue);
                    T2Debug("Adding eventName : %s eventValue : %s to t2event queue\n", event->name, event->value);
                    t2_queue_push(eQueue, (void *) event);
                    if(!stopDispatchThread)
                        pthread_cond_signal(&erCond);
                }

            }
            pthread_mutex_unlock(&erMutex);
            free(eventName);
            free(eventValue);
        }
    }else {
        T2Warning("ER is not initialized, ignoring telemetry events for now\n");
    }
    T2Debug("%s --out\n", __FUNCTION__);
}

void* T2ER_EventDispatchThread(void *arg)
{
    (void) arg; // To avoid compiler warning
    T2Debug("%s ++in\n", __FUNCTION__);
    Vector *profileList = NULL;
    while(!stopDispatchThread)
    {
        pthread_mutex_lock(&erMutex);
        T2Debug("Checking for events in event queue , event count = %d\n", t2_queue_count(eQueue));
        if(t2_queue_count(eQueue) > 0)
        {
            T2Event *event = (T2Event *)t2_queue_pop(eQueue);
            if(event == NULL)
            {
                T2Error("event data in queue points to NULL\n");
                pthread_mutex_unlock(&erMutex);
                continue;
            }
            pthread_mutex_unlock(&erMutex);
            Vector_Create(&profileList);
            if(T2ERROR_SUCCESS == getMarkerProfileList(event->name, &profileList))
            {
                T2Debug("Found matching profileIDs for event with markerName : %s value : %s\n", event->name, event->value);
                size_t index = 0;
                for(; index < Vector_Size(profileList); index++)
                {
                    T2Debug("Storing in profile : %s\n", (char *)Vector_At(profileList, index));
                    ReportProfiles_storeMarkerEvent((char *)Vector_At(profileList, index), event);
                }
            }
            else
            {
                T2Debug("No Matching Profiles for event with MarkerName : %s Value : %s - Ignoring\n", event->name, event->value);
            }
            Vector_Destroy(profileList, free);
            freeT2Event(event);
        }
        else
        {
            T2Debug("Event Queue size is 0, Waiting events from T2ER_Push\n");
            pthread_cond_wait(&erCond, &erMutex);
            pthread_mutex_unlock(&erMutex);
            T2Debug("Received signal from T2ER_Push\n");
        }
    }

    T2Debug("%s --out\n", __FUNCTION__);
    return NULL;
}

T2ERROR T2ER_Init()
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if(EREnabled)
    {
        T2Debug("T2ER already initialized, ignoring\n");
        return T2ERROR_SUCCESS;
    }
    eQueue = t2_queue_create();
    if(eQueue == NULL)
    {
        T2Error("Failed to create Event Receiver Queue\n");
        return T2ERROR_FAILURE;
    }

    pthread_mutex_init(&sTDMutex, NULL);
    pthread_mutex_init(&erMutex, NULL);
    pthread_cond_init(&erCond, NULL);

    EREnabled = true;
    if(isRbusEnabled()) {
       T2Debug("Register event call back function T2ER_Push \n");
       registerForTelemetryEvents(T2ER_Push);
    }else{
        T2Debug("Register event call back function T2ER_PushDataWithDelim \n");
       registerForTelemetryEvents(T2ER_PushDataWithDelim);
    }

    system("touch /tmp/.t2ReadyToReceiveEvents");
    setT2EventReceiveState(T2_STATE_COMPONENT_READY);
    T2Info("T2 is now Ready to Recieve Events\n");

    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

T2ERROR T2ER_StartDispatchThread()
{
    T2Debug("%s ++in\n", __FUNCTION__);
    pthread_mutex_lock(&sTDMutex); 
    if(!EREnabled || !stopDispatchThread)
    {
        T2Info("T2ER isn't initialized or dispatch thread is already running\n");
        pthread_mutex_unlock(&sTDMutex);
        return T2ERROR_FAILURE;
    }
    stopDispatchThread = false;
    pthread_create(&erThread, NULL, T2ER_EventDispatchThread, NULL);
 
    pthread_mutex_unlock(&sTDMutex);
    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

static T2ERROR flushCacheFromFile(void)
{
        T2Debug("%s ++in\n",__FUNCTION__);
        FILE *fp;
        char telemetry_data[255]="";
        size_t data_len = 0 ;

        fp = fopen(T2_CACHE_FILE, "r");
        if(fp){
                while(fgets(telemetry_data, 255, (FILE*)fp) != NULL)
                {
                        data_len = strlen(telemetry_data);
                        if(data_len > 0 && telemetry_data[data_len-1] == '\n' )
                            telemetry_data[data_len-1] = '\0' ;

                        T2Debug("T2: Sending cache event : %s\n", telemetry_data);
                        T2ER_PushDataWithDelim(telemetry_data, NULL);
                        memset(telemetry_data, 0, sizeof(telemetry_data));
                }
                fclose(fp);
                if(remove(T2_CACHE_FILE) != 0){
                        T2Error("Failed to remove the file %s\n",T2_CACHE_FILE);
                }
        }
        else{
                T2Debug("fopen failed for %s\n", T2_CACHE_FILE);
        }

        T2Debug("%s --out\n",__FUNCTION__);
        return T2ERROR_SUCCESS;
}

T2ERROR T2ER_StopDispatchThread()
{
    T2Debug("%s ++in\n", __FUNCTION__);
    pthread_mutex_lock(&sTDMutex);
    if(!EREnabled || stopDispatchThread)
    {
        T2Info("T2ER isn't initialized or dispatch thread isn't running\n");
        pthread_mutex_unlock(&sTDMutex);
        return T2ERROR_FAILURE;
    }
    stopDispatchThread = true;
    

    pthread_mutex_lock(&erMutex);
    pthread_cond_signal(&erCond);
    pthread_mutex_unlock(&erMutex);

    pthread_detach(erThread);
    flushCacheFromFile();
    pthread_mutex_unlock(&sTDMutex);
    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

void T2ER_Uninit()
{
    T2Debug("%s ++in\n", __FUNCTION__);
    if(!EREnabled)
    {
        T2Debug("T2ER isn't initialized, ignoring\n");
        return;
    }
    EREnabled = false;

    if(!stopDispatchThread)
    {
        pthread_mutex_lock(&sTDMutex);
        stopDispatchThread = true;
        pthread_mutex_unlock(&sTDMutex);
    
        pthread_mutex_lock(&erMutex);
        pthread_cond_signal(&erCond);
        pthread_mutex_unlock(&erMutex);

        pthread_join(erThread, NULL);

        pthread_mutex_destroy(&erMutex);
        pthread_mutex_destroy(&sTDMutex);
        pthread_cond_destroy(&erCond);
    }
    T2Debug("T2ER Event Dispatch Thread successfully terminated\n");
    t2_queue_destroy(eQueue, freeT2Event);
    eQueue = NULL;
    T2Debug("%s --out\n", __FUNCTION__);
}
