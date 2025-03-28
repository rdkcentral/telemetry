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

#include <stdio.h>
#include <time.h>

#include "../dcautil/dca.h"
#include "../dcautil/dcautil.h"
#include "dcautil.h"
#include "profile.h"
#include "t2log_wrapper.h"
#include "telemetry2_0.h"
#include "vector.h"

#include "busInterfaceTests.h"

/**
 * This is to primarily unit test api getGrepResults
 * primarily used for getting log grep results
 *
 * Create Vector with 2 marker vals
 * Push expected match pattern to logs and check for return pattern
 * Push similar entries and verify counts are appropriate
 */

// TODO add asserts for future and plugin fixed sample log files
void logGrepTest1() {
  printf("%s ++in \n", __FUNCTION__);

  T2ERROR ret = T2ERROR_FAILURE;
  // Split parameter

  GrepMarker *gMarker1 = (GrepMarker *)malloc(sizeof(GrepMarker));
  gMarker1->markerName = strdup("EXPECTED_WIFI_VAP_split");
  gMarker1->searchString = strdup("WIFI_VAP_PERCENT_UP");
  gMarker1->logFile = strdup("wifihealth.txt");
  gMarker1->skipFreq = 0;

  // Split parameter
  GrepMarker *gMarker2 = (GrepMarker *)malloc(sizeof(GrepMarker));
  gMarker2->markerName = strdup("EXPECTED_WIFI_COUNTRY_CODE_split");
  gMarker2->searchString = strdup("WIFI_COUNTRY_CODE_1");
  gMarker2->logFile = strdup("wifihealth.txt");
  gMarker2->skipFreq = 0;

  // TODO Add 1 counter marker to file 1
  // TODO Add 1 message_bus marker to file 1

  /**
   * Intensional different file name in between to make sure legacy utils
   * handles the sorting internally
   */

  GrepMarker *gMarker3 = (GrepMarker *)malloc(sizeof(GrepMarker));
  gMarker3->markerName = strdup("DO_NOT_REPORT_split");
  gMarker3->searchString = strdup("no matching pattern");
  gMarker3->logFile = strdup("console.log");
  gMarker3->skipFreq = 0;

  // TODO Add 1 counter marker to file 2
  GrepMarker *gMarker4 = (GrepMarker *)malloc(sizeof(GrepMarker));
  gMarker4->markerName = strdup("EXPECTED_MARKER_AFTER_SORTING");
  gMarker4->searchString = strdup("WIFI_CHANNEL_1");
  gMarker4->logFile = strdup("wifihealth.txt");
  gMarker4->skipFreq = 0;

  // TODO Add 1 message_bus marker to file 2
  GrepMarker *gMarker5 = (GrepMarker *)malloc(sizeof(GrepMarker));
  gMarker5->markerName = strdup("EXPECTED_TR181_MESH_ENABLE_STATUS");
  gMarker5->searchString =
      strdup("Device.DeviceInfo.X_RDKCENTRAL-COM_xOpsDeviceMgmt.Mesh.Enable");
  gMarker5->logFile = strdup("<message_bus>");
  gMarker5->skipFreq = 1;

  Vector *markerlist = NULL;
  Vector *grepResultList = NULL;

  if (T2ERROR_FAILURE == Vector_Create(&markerlist))
    printf("%s %d %s \n", __FUNCTION__, __LINE__,
           "Error creating markerlist<GrepMarker> vector");

  if (T2ERROR_FAILURE == Vector_Create(&grepResultList))
    printf("%s %d %s \n", __FUNCTION__, __LINE__,
           "Error creating grepResultList<GrepResult> vector");

  if (T2ERROR_FAILURE == Vector_PushBack(markerlist, gMarker1))
    printf("%s %d %s \n", __FUNCTION__, __LINE__,
           "Error pushing entry1 to vector markerlist");

  if (T2ERROR_FAILURE == Vector_PushBack(markerlist, gMarker2))
    printf("%s %d %s \n", __FUNCTION__, __LINE__,
           "Error pushing entry2 to vector markerlist");

  if (T2ERROR_FAILURE == Vector_PushBack(markerlist, gMarker4))
    printf("%s %d %s \n", __FUNCTION__, __LINE__,
           "Error pushing entry4 to vector markerlist");

  if (T2ERROR_FAILURE == Vector_PushBack(markerlist, gMarker5))
    printf("%s %d %s \n", __FUNCTION__, __LINE__,
           "Error pushing entry5 to vector markerlist");

  if (T2ERROR_FAILURE == Vector_PushBack(markerlist, gMarker3))
    printf("%s %d %s \n", __FUNCTION__, __LINE__,
           "Error pushing entry3 to vector markerlist");

  time_t now;

  printf("\n\n Start of test set ################## \n\n\n");
  printf("Start time %s \n", ctime(&now));
  ret = getGrepResults("profile1", markerlist, &grepResultList, false);
  printf("End time %s \n", ctime(&now));

  if (ret == T2ERROR_SUCCESS) {
    int resultSize = Vector_Size(grepResultList);
    int var = 0;
    for (var = 0; var < resultSize; ++var) {
      GrepResult *result = (GrepResult *)Vector_At(grepResultList, var);
      printf(" Marker = %s , Value = %s \n", result->markerName,
             result->markerValue);
    }

  } else {
    printf("\n\n\n  %s %d %s \n\n\n", __FUNCTION__, __LINE__,
           "Something quite not right .... Debug now !!!!");
  }

  printf("End of test set ################## \n\n\n");
  if (grepResultList) {
    // TODO Free up the nodes
    free(grepResultList);
    grepResultList = NULL;
  }

  printf("\n\n\nWaiting for 2nd test suite exec\n\n\n");
  sleep(20); // Buffer time for tracing
  printf("\n\n Start of test set ################## \n\n\n");
  printf("Start time %s \n", ctime(&now));
  ret = getGrepResults("profile1", markerlist, &grepResultList, false);
  printf("End time %s \n", ctime(&now));
  printf("!!! Results should be absolutely nothing else !!! \n");
  if (ret == T2ERROR_SUCCESS) {
    int resultSize = Vector_Size(grepResultList);
    printf("%s 2nd set of tests with continuous exec Got results .. loop and "
           "print data of size = %d\n",
           __FUNCTION__, resultSize);
    int var = 0;
    for (var = 0; var < resultSize; ++var) {
      GrepResult *result = (GrepResult *)Vector_At(grepResultList, var);
      printf(" Marker = %s , Value = %s \n", result->markerName,
             result->markerValue);
    }

  } else {
    printf("%s \n", "Something quite not right .... Debug now !!!!");
  }
  printf("End of test set ################## \n\n\n");

  if (grepResultList) {
    // TODO Free up the nodes
    free(grepResultList);
    grepResultList = NULL;
  }

  printf("\n\n\nWaiting for another 3rd test suite exec for skip frequency "
         "check\n\n\n");
  sleep(20); // Buffer time for tracing
  printf("\n\n Start of test set ################## \n\n\n");
  printf("!!! Results should contain only message bus parameters . Nothing "
         "else !!! \n");
  printf("Start time %s \n", ctime(&now));
  ret = getGrepResults("profile1", markerlist, &grepResultList, false);
  printf("End time %s \n", ctime(&now));
  if (ret == T2ERROR_SUCCESS) {
    int resultSize = Vector_Size(grepResultList);
    printf("%s 3nd set of tests with continuous exec Got results. Result size "
           "= %d\n",
           __FUNCTION__, resultSize);
    int var = 0;
    for (var = 0; var < resultSize; ++var) {
      GrepResult *result = (GrepResult *)Vector_At(grepResultList, var);
      printf(" Marker = %s , Value = %s \n", result->markerName,
             result->markerValue);
    }
  } else {
    printf("%s \n", "Something quite not right .... Debug now !!!!");
  }

  printf("%s ++out \n", __FUNCTION__);
  return;
}

/**
 * Run same tests repeated for a different profile name
 */
void logGrepTest2() {
  printf("%s %d ++in \n", __FUNCTION__);

  T2ERROR ret = T2ERROR_FAILURE;
  // Split parameter

  GrepMarker *gMarker1 = (GrepMarker *)malloc(sizeof(GrepMarker));
  gMarker1->markerName = strdup("EXPECTED_WIFI_VAP_split");
  gMarker1->searchString = strdup("WIFI_VAP_PERCENT_UP");
  gMarker1->logFile = strdup("wifihealth.txt");
  gMarker1->skipFreq = 0;

  // Split parameter
  GrepMarker *gMarker2 = (GrepMarker *)malloc(sizeof(GrepMarker));
  gMarker2->markerName = strdup("EXPECTED_WIFI_COUNTRY_CODE_split");
  gMarker2->searchString = strdup("WIFI_COUNTRY_CODE_1");
  gMarker2->logFile = strdup("wifihealth.txt");
  gMarker2->skipFreq = 0;

  // TODO Add 1 counter marker to file 1
  // TODO Add 1 message_bus marker to file 1

  /**
   * Intensional different file name in between to make sure legacy utils
   * handles the sorting internally
   */

  GrepMarker *gMarker3 = (GrepMarker *)malloc(sizeof(GrepMarker));
  gMarker3->markerName = strdup("DO_NOT_REPORT_split");
  gMarker3->searchString = strdup("no matching pattern");
  gMarker3->logFile = strdup("console.log");
  gMarker3->skipFreq = 0;

  // TODO Add 1 counter marker to file 2
  GrepMarker *gMarker4 = (GrepMarker *)malloc(sizeof(GrepMarker));
  gMarker4->markerName = strdup("EXPECTED_MARKER_AFTER_SORTING");
  gMarker4->searchString = strdup("WIFI_CHANNEL_1");
  gMarker4->logFile = strdup("wifihealth.txt");
  gMarker4->skipFreq = 0;

  // TODO Add 1 message_bus marker to file 2
  GrepMarker *gMarker5 = (GrepMarker *)malloc(sizeof(GrepMarker));
  gMarker5->markerName = strdup("EXPECTED_TR181_MESH_ENABLE_STATUS");
  gMarker5->searchString =
      strdup("Device.DeviceInfo.X_RDKCENTRAL-COM_xOpsDeviceMgmt.Mesh.Enable");
  gMarker5->logFile = strdup("<message_bus>");
  gMarker5->skipFreq = 0;

  Vector *markerlist = NULL;
  Vector *grepResultList = NULL;

  if (T2ERROR_FAILURE == Vector_Create(&markerlist))
    printf("%s %d %s \n", __FUNCTION__, __LINE__,
           "Error creating markerlist<GrepMarker> vector");

  if (T2ERROR_FAILURE == Vector_Create(&grepResultList))
    printf("%s %d %s \n", __FUNCTION__, __LINE__,
           "Error creating grepResultList<GrepResult> vector");

  if (T2ERROR_FAILURE == Vector_PushBack(markerlist, gMarker1))
    printf("%s %d %s \n", __FUNCTION__, __LINE__,
           "Error pushing entry1 to vector markerlist");

  if (T2ERROR_FAILURE == Vector_PushBack(markerlist, gMarker2))
    printf("%s %d %s \n", __FUNCTION__, __LINE__,
           "Error pushing entry2 to vector markerlist");

  if (T2ERROR_FAILURE == Vector_PushBack(markerlist, gMarker4))
    printf("%s %d %s \n", __FUNCTION__, __LINE__,
           "Error pushing entry4 to vector markerlist");

  if (T2ERROR_FAILURE == Vector_PushBack(markerlist, gMarker5))
    printf("%s %d %s \n", __FUNCTION__, __LINE__,
           "Error pushing entry5 to vector markerlist");

  if (T2ERROR_FAILURE == Vector_PushBack(markerlist, gMarker3))
    printf("%s %d %s \n", __FUNCTION__, __LINE__,
           "Error pushing entry3 to vector markerlist");

  time_t now;
  printf("\n\n Start of test set ################## \n\n\n");
  printf("Start time %s \n", ctime(&now));
  ret = getGrepResults("profile2", markerlist, &grepResultList, false);
  printf("End time %s \n", ctime(&now));

  if (ret == T2ERROR_SUCCESS) {
    int resultSize = Vector_Size(grepResultList);
    printf("\n\n\n %s Got results .. loop and print data of size = %d \n\n\n",
           __FUNCTION__, resultSize);
    int var = 0;
    for (var = 0; var < resultSize; ++var) {
      GrepResult *result = (GrepResult *)Vector_At(grepResultList, var);
      printf(" Marker = %s , Value = %s \n", result->markerName,
             result->markerValue);
    }

  } else {
    printf("\n\n\n  %s %d %s \n\n\n", __FUNCTION__, __LINE__,
           "Something quite not right .... Debug now !!!!");
  }
  printf("\n\n End of test set ################## \n\n\n");
  if (grepResultList) {
    // TODO Free up the nodes
    free(grepResultList);
    grepResultList = NULL;
  }

  printf("\n\n\nWaiting for another test suite exec\n\n\n");
  sleep(20); // Buffer time for tracing
  printf("\n\n Start of test set ################## \n\n\n");
  printf("!!! Results should be everything - for profile 3 !!! \n");
  printf("Start time %s \n", ctime(&now));
  ret = getGrepResults("profile3", markerlist, &grepResultList, false);
  printf("End time %s \n", ctime(&now));
  if (ret == T2ERROR_SUCCESS) {
    int resultSize = Vector_Size(grepResultList);
    printf("%s 2nd set of tests with continuous exec Got results .. loop and "
           "print data of size = %d\n",
           __FUNCTION__, resultSize);
    int var = 0;
    for (var = 0; var < resultSize; ++var) {
      GrepResult *result = (GrepResult *)Vector_At(grepResultList, var);
      printf(" Marker = %s , Value = %s \n", result->markerName,
             result->markerValue);
    }

  } else {
    printf("%s \n", "Something quite not right .... Debug now !!!!");
  }
  printf("End of test set ################## \n\n\n");
  if (grepResultList) {
    // TODO Free up the nodes
    free(grepResultList);
    grepResultList = NULL;
  }

  printf("\n\n\nWaiting for another test suite exec for skip frequency "
         "check\n\n\n");
  sleep(20); // Buffer time for tracing
  printf("\n\n Start of test set ################## \n\n\n");
  printf("!!! Results should be everything - for profile 4 !!! \n");
  printf("Start time %s \n", ctime(&now));
  ret = getGrepResults("profile4", markerlist, &grepResultList, false);
  printf("End time %s \n", ctime(&now));
  if (ret == T2ERROR_SUCCESS) {
    int resultSize = Vector_Size(grepResultList);
    printf("%s 3rd set of tests with continuous exec Got results. Result size "
           "= %d\n",
           __FUNCTION__, resultSize);
    int var = 0;
    for (var = 0; var < resultSize; ++var) {
      GrepResult *result = (GrepResult *)Vector_At(grepResultList, var);
      printf(" Marker = %s , Value = %s \n", result->markerName,
             result->markerValue);
    }
  } else {
    printf("%s \n", "Something quite not right .... Debug now !!!!");
  }
  printf("End of test set ################## \n\n\n");
  printf("%s %d ++out \n", __FUNCTION__, __LINE__);
  return;
}

static void logGrepTests() {

  printf("Validating dcautils test cases .... \n \n");

  logGrepTest1();
  sleep(2);
  logGrepTest2();

  printf("End of dcautils test cases .... \n \n");
}

int main(int argc, char *argv[]) {

  LOGInit();

  // logGrepTests() ;

  testBusInterface();

  return 0;
}
