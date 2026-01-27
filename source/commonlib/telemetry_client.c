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
#include <telemetry_busmessage_sender.h>

#define COMP_NAME "client_one"

// int main(int argc, char *argv[])
// {
//     (void) argc;// To fix compiler warning
//     (void) argv;// To fix compiler warning
//     t2_init(COMP_NAME);
//     t2_event_s("c1Test2_bootup", "test_value_1");
//     t2_event_s("c1Test2_bootup", "test_value_2");
//     //sleep(10); // Sleep for 100ms
//     t2_uninit();

//     return 0;
// }
int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    
    t2_init(COMP_NAME);
    
    printf("1. Sending: c1Test1_split = test_value_1\n");
    t2_event_s("c1Test1_split", "test_value_1");
    
    printf("2. Sending: c1Test2_bootup = test_value_2\n");
    t2_event_s("c1Test2_bootup", "test_value_2");
    
    printf("3. Sending: c1Test2_bootup = test_value_3\n");
    t2_event_s("c1Test2_bootup", "test_value_3");
    
    printf("4. Sending: c1Test3_network = test_value_4\n");
    t2_event_s("c1Test3_network", "test_value_4");
    
    printf("5. Sending: c1Test4_wifi = test_value_5\n");
    t2_event_s("c1Test4_wifi", "test_value_5");
    
    
    sleep(10); // Sleep for 10 seconds
    
    t2_uninit();
    
    return 0;
}