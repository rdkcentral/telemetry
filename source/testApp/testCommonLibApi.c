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

#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#include <telemetry_busmessage_sender.h>

#include "t2log_wrapper.h"

unsigned int rdkLogLevel = RDK_LOG_INFO;

#define COMPONENT_NAME_1 "CcspWifiAgent"

void LOGInit() {
  rdk_logger_init(DEBUG_INI_NAME);
  if (access(ENABLE_DEBUG_FLAG, F_OK) != -1)
    rdkLogLevel = RDK_LOG_DEBUG;
}

static void eventApiTests() {

  printf("%s Test Start ... \n\n ", __FUNCTION__);
  printf("====== Sends following events repeatedly : ====== \n");

  printf("\t ccsp-wifi-agent \n");
  printf("WIFI_ERROR_PSM_GetRecordFail \n"
         "5GclientMac_split \n ");

  printf("\t test-and-diagnostic \n");
  printf("SYS_SH_CMReset_PingFailed \n"
         "SYS_INFO_Invoke_batterymode \n"
         "SYS_SH_lighttpdCrash ");

  printf("\t TEST_COMP \n");
  printf("TEST_EVENT_1 \n"
         "TEST_EVENT_2 \n");

  printf("================================================ \n");

  sleep(5);

  t2_event_s("WIFI_ERROR_PSM_GetRecordFail", "WIFI_PSM");
  sleep(5);
  t2_event_s("SYS_SH_CMReset_PingFailed", "SelfHealTest1");
  sleep(5);
  t2_event_s("5GclientMac_split", "Yes_From_Test");
  sleep(5);
  t2_event_s("SYS_INFO_Invoke_batterymode", "SelfHealBatteryTest");
  sleep(5);
  t2_event_d("SYS_SH_lighttpdCrash", 1);
  sleep(5);
  t2_event_d("TEST_EVENT_1", 1);
  sleep(5);
  t2_event_s("TEST_EVENT_2", "Test Value 2");
  sleep(5);

  printf("\n\n %s Test End ... \n\n ", __FUNCTION__);
}

/**
 *  Tests are configured to send events from two comoponenets .
 *  Test app is mimicking the eventing for below 4 parameters :
 *  1] CcspWifiAgent   (WIFI_ERROR_PSM_GetRecordFail, WIFI_ERROR_NvramCorrupt)
 *  2] SELF_HEAL       (SYS_SH_CMReset_PingFailed, SYS_INFO_Invoke_batterymode)
 *
 *  More tests to be included to check for parameters :
 *  1] Subscribed updates on parameters dynamically included events from report
 * profiles 2] Negative test cases like all 1 parameter getting removed,
 *     Multiple params removed and new added
 *     All parameters removed (ie. Module excluded from monitoring )
 *  3] Aggressive tests for memory and cpu evaluation
 *
 */
int main(int argc, char *argv[]) {

  LOGInit();
  if (argc > 1) {
    t2_init(argv[1]);
    while (1) {
      printf("Initiatized component %s \n ", argv[1]);
      eventApiTests();
      sleep(10);
      eventApiTests();
    }
  } else {
    printf("Invoke %s with component name ccsp-wifi-agent or "
           "test-and-diagnostic or TEST_COMP \n ",
           argv[0]);
  }
  return 0;
}
