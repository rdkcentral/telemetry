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

#include "dcautil.h"
#include "dca.h"
#include "legacyutils.h"
#include "t2common.h"
#include "t2log_wrapper.h"
#include "telemetry2_0.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

T2ERROR
getGrepResults(char *profileName, Vector *markerList, Vector **grepResultList,
               bool isClearSeekMap, bool check_rotated) {
  T2Debug("%s ++in\n", __FUNCTION__);
  if (profileName == NULL || markerList == NULL || grepResultList == NULL) {
    T2Error("Invalid Args or Args are NULL\n");
    return T2ERROR_FAILURE;
  }

  getDCAResultsInVector(profileName, markerList, grepResultList, check_rotated);
  if (isClearSeekMap) {
    removeProfileFromSeekMap(profileName);
  }

  T2Debug("%s --out\n", __FUNCTION__);
  return T2ERROR_SUCCESS;
}

void removeGrepConfig(char *profileName, bool clearSeekMap, bool clearExecMap) {
  T2Debug("%s ++in\n", __FUNCTION__);

  if (clearSeekMap)
    removeProfileFromSeekMap(profileName);

  if (clearExecMap)
    removeProfileFromExecMap(profileName);
  T2Debug("%s ++out\n", __FUNCTION__);
}

// dcaFlagReportCompleation this function is used to create legacy DCA Flag
// DCADONEFLAG
void dcaFlagReportCompleation() {
  T2Debug("%s --in creating flag %s\n", __FUNCTION__, DCADONEFLAG);
  FILE *fileCheck = fopen(DCADONEFLAG, "w+");
  if (fileCheck == NULL) {
    T2Error(" Error in creating the Flag :  %s\n", DCADONEFLAG);
  } else {
    fclose(fileCheck);
  }
  T2Debug("%s --out\n", __FUNCTION__);
}
