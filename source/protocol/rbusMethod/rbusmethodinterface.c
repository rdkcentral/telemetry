/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2021 RDK Management
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
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <net/if.h>
#include <netdb.h>
#include <string.h>
#include <ifaddrs.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>

#include "reportgen.h"
#include "rbusInterface.h"
#include "rbusmethodinterface.h"
#include "reportprofiles.h"
#include "t2log_wrapper.h"
#define MAX_RETRY_ATTEMPTS 5

static pthread_once_t rbusMethodMutexOnce = PTHREAD_ONCE_INIT;
static pthread_mutex_t rbusMethodMutex;
static pthread_cond_t rbusMethodCond;
static bool rbusMethodCallbackDone = false;
static bool isRbusMethod = false ;

static void sendOverRBUSMethodInit()
{
    pthread_mutex_init(&rbusMethodMutex, NULL);
    pthread_cond_init(&rbusMethodCond, NULL);
}

static void asyncMethodHandler(rbusHandle_t handle, char const* methodName, rbusError_t retStatus, rbusObject_t params)
{
    (void) handle;
    (void) params;

    T2Info("T2 asyncMethodHandler called: %s with return error code  = %s \n", methodName, rbusError_ToString(retStatus));

    /* Lock the mutex, update result, signal the waiting caller, then unlock.
     * This ensures no cross-thread unlock (which is UB for default mutexes)
     * and provides proper memory visibility for isRbusMethod. */
    pthread_mutex_lock(&rbusMethodMutex);
    if(retStatus == RBUS_ERROR_SUCCESS)
    {
        isRbusMethod = true ;
    }
    else
    {
        T2Error("Unable to send data over method %s \n", methodName) ;
        isRbusMethod = false ;
    }
    rbusMethodCallbackDone = true;
    pthread_cond_signal(&rbusMethodCond);
    pthread_mutex_unlock(&rbusMethodMutex);
}

T2ERROR sendReportsOverRBUSMethod(char *methodName, Vector* inputParams, char* payload)
{
    if(methodName == NULL || inputParams == NULL || payload == NULL)
    {
        return T2ERROR_FAILURE;
    }
    T2Debug("%s ++in\n", __FUNCTION__);
    T2ERROR ret = T2ERROR_FAILURE;
    T2Info("methodName = %s payload = %s \n", methodName, payload);

    rbusObject_t inParams;
    rbusValue_t value;
    size_t i = 0;

    rbusObject_Init(&inParams, NULL);

    for( i = 0; i < Vector_Size(inputParams); i++ )
    {
        rbusValue_Init(&value);
        RBUSMethodParam *rbusMethodParam = (RBUSMethodParam *) Vector_At(inputParams, i);
        if(rbusMethodParam)
        {
            if(rbusMethodParam->name && rbusMethodParam->value)
            {
                rbusValue_SetString(value, rbusMethodParam->value);
                rbusObject_SetValue(inParams, rbusMethodParam->name, value);
            }
        }
        rbusValue_Release(value);
    }

    // Default parameters associated with a data payload
    rbusValue_Init(&value);
    rbusValue_SetInt32(value, (strlen(payload) + 1));
    rbusObject_SetValue(inParams, "payloadlen", value);
    rbusValue_Release(value);

    rbusValue_Init(&value);
    rbusValue_SetString(value, payload);
    rbusObject_SetValue(inParams, "payload", value);
    rbusValue_Release(value);

    pthread_once(&rbusMethodMutexOnce, sendOverRBUSMethodInit);

    /* Initialize state under lock, then release before the async call.
     * The callback will lock, set results, signal, and unlock — so every
     * thread only unlocks a mutex it locked itself (no cross-thread unlock). */
    pthread_mutex_lock(&rbusMethodMutex);
    isRbusMethod = false ;
    rbusMethodCallbackDone = false;
    pthread_mutex_unlock(&rbusMethodMutex);

    if ( T2ERROR_SUCCESS == rbusMethodCaller(methodName, &inParams, payload, &asyncMethodHandler))
    {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += MAX_RETRY_ATTEMPTS * 2;  /* Total timeout: 10 seconds */

        pthread_mutex_lock(&rbusMethodMutex);
        while (!rbusMethodCallbackDone)
        {
            int rc = pthread_cond_timedwait(&rbusMethodCond, &rbusMethodMutex, &ts);
            if(rc == ETIMEDOUT)
            {
                T2Error("Timed out waiting for rbus method callback. Giving up\n");
                break;
            }
        }
        if (rbusMethodCallbackDone && isRbusMethod)
        {
            T2Info("Return status of send via rbusMethod is success\n");
            ret = T2ERROR_SUCCESS ;
        }
        else if (rbusMethodCallbackDone)
        {
            T2Info("Return status of send via rbusMethod is failure\n");
            ret = T2ERROR_NO_RBUS_METHOD_PROVIDER;
        }
        else
        {
            /* Timed out — callback never fired */
            ret = T2ERROR_NO_RBUS_METHOD_PROVIDER;
        }
        pthread_mutex_unlock(&rbusMethodMutex);
    }
    rbusObject_Release(inParams);

    T2Debug("%s --out\n", __FUNCTION__);
    return ret;
}

T2ERROR sendCachedReportsOverRBUSMethod(char *methodName, Vector* inputParams, Vector* reportList)
{
    if(methodName == NULL || inputParams == NULL || reportList == NULL)
    {
        return T2ERROR_FAILURE;
    }
    T2Debug("%s ++in\n", __FUNCTION__);
    while(Vector_Size(reportList) > 0)
    {
        char* payload = (char *) Vector_At(reportList, 0);
        T2ERROR ret = sendReportsOverRBUSMethod(methodName, inputParams, payload);
        if(ret == T2ERROR_FAILURE || ret == T2ERROR_NO_RBUS_METHOD_PROVIDER)
        {
            T2Error("Failed to send cached report, left with %zu reports in cache \n", Vector_Size(reportList));
            return T2ERROR_FAILURE;
        }
        Vector_RemoveItem(reportList, payload, NULL);
        free(payload);
    }
    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;
}

#ifdef GTEST_ENABLE
pthread_mutex_t* getRbusMethodMutex(void)
{
    return &rbusMethodMutex;
}

typedef void (*asyncMethodHandlerFunc)(rbusHandle_t, char const*, rbusError_t, rbusObject_t);
asyncMethodHandlerFunc asyncMethodHandlerFuncCallback(void)
{
    return asyncMethodHandler;
}
#endif
