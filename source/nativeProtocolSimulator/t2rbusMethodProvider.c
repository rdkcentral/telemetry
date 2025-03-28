/*
 * If not stated otherwise in this file or this component's LICENSE file
 * the following copyright and licenses apply:
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

#include <getopt.h>
#include <pthread.h>
#include <rbus.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

int loopFor = 10;
rbusHandle_t handle;

static bool returnStatus = true;
static char *simulateMethodName = NULL;

typedef struct MethodData {
  rbusMethodAsyncHandle_t asyncHandle;
  rbusObject_t inParams;
} MethodData;

static void *asyncSimulatorImpl(void *p) {

  MethodData *data;
  rbusObject_t outParams;
  rbusValue_t value;
  rbusError_t err;

  printf("%s enter\n", __FUNCTION__);

  data = p;

  rbusValue_Init(&value);
  rbusValue_SetString(value, "Async method response from t2 simulator");

  rbusObject_Init(&outParams, NULL);
  rbusObject_SetValue(outParams, "value", value);
  rbusValue_Release(value);

  if (returnStatus) {
    printf("%s sending response as success \n", __FUNCTION__);
    err = rbusMethod_SendAsyncResponse(data->asyncHandle, RBUS_ERROR_SUCCESS,
                                       outParams);
  } else {
    printf("%s sending response as failure \n", __FUNCTION__);
    err = rbusMethod_SendAsyncResponse(
        data->asyncHandle, RBUS_ERROR_INVALID_RESPONSE_FROM_DESTINATION,
        outParams);
  }

  if (err != RBUS_ERROR_SUCCESS) {
    printf("%s rbusMethod_SendAsyncResponse failed err:%d\n", __FUNCTION__,
           err);
  }

  rbusObject_Release(data->inParams);
  rbusObject_Release(outParams);

  free(data);

  printf("%s exit\n", __FUNCTION__);

  return NULL;
}

static rbusError_t simulateHandler(rbusHandle_t handle, char const *methodName,
                                   rbusObject_t inParams,
                                   rbusObject_t outParams,
                                   rbusMethodAsyncHandle_t asyncHandle) {
  (void)handle;
  (void)outParams;
  printf("methodHandler called: %s\n", methodName);
  rbusObject_fwrite(inParams, 1, stdout);

  if (strcmp(methodName, simulateMethodName) == 0) {
    pthread_t pid;
    MethodData *data = malloc(sizeof(MethodData));
    data->asyncHandle = asyncHandle;
    data->inParams = inParams;

    rbusObject_Retain(inParams);

    if (pthread_create(&pid, NULL, asyncSimulatorImpl, data) ||
        pthread_detach(pid)) {
      printf("%s failed to spawn thread\n", __FUNCTION__);
      return RBUS_ERROR_BUS_ERROR;
    }
    return RBUS_ERROR_ASYNC_RESPONSE;
  } else {
    return RBUS_ERROR_BUS_ERROR;
  }
}

static void printHelp() {
  printf("Invalid arguments\n");
  printf("Invoke :\n t2rbusMethodSimulator <mehodName> <true/false> <keep "
         "alive in seconds> \n");
}

int main(int argc, char *argv[]) {

  if (argc < 4) {
    printHelp();
    return -1;
  }

  int rc = RBUS_ERROR_SUCCESS;

  char componentName[] = "t2NativeMethodSimulator";
  simulateMethodName = argv[1];
  if (strcmp(argv[2], "true") == 0) {
    printf("Setting return status to true \n");
    returnStatus = true;
  } else {
    printf("Setting return status to false \n");
    returnStatus = false;
  }
  loopFor = atoi(argv[3]);

  printf("Simulating %s method provider returning %s status for next %d "
         "seconds ... \n",
         simulateMethodName, returnStatus ? "SUCCESS" : "FAILURE", loopFor);

  rbusDataElement_t dataElements[1] = {
      {simulateMethodName,
       RBUS_ELEMENT_TYPE_METHOD,
       {NULL, NULL, NULL, NULL, NULL, simulateHandler}}};

  printf("provider: start\n");

  rc = rbus_open(&handle, componentName);
  if (rc != RBUS_ERROR_SUCCESS) {
    printf("provider: rbus_open failed: %d\n", rc);
    goto exit2;
  }

  rc = rbus_regDataElements(handle, 1, dataElements);
  if (rc != RBUS_ERROR_SUCCESS) {
    printf("provider: rbus_regDataElements failed: %d\n", rc);
    goto exit1;
  }

  while (loopFor != 0) {
    printf("provider: exiting in %d seconds\n", loopFor);
    sleep(1);
    loopFor--;
  }

  rbus_unregDataElements(handle, 1, dataElements);
exit1:
  rbus_close(handle);
exit2:
  printf("provider: exit\n");
  return rc;
}
