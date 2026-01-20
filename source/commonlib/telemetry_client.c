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
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <telemetry_busmessage_sender.h>

#define COMP_NAME "telemetry_client"
/*
int main(int argc, char *argv[])
{
    (void) argc;// To fix compiler warning
    t2_init(COMP_NAME);
    t2_event_s(argv[1], argv[2]);

    t2_uninit();

    return 0;
}
*/
/*
int main(int argc, char *argv[])
{
    int i = 0, n;
    n = (argc < 2) ? 100 : atoi(argv[1]);
    // Initialize Telemetry2.0
    t2_init("telemetry_client");

    while(i <= n)
    {
        t2_event_d("T2_INFO_Test", i);
        i++;
    }
    t2_uninit();
    printf("Sent %d t2_event_d events.\n", n);
    return 0;
}
    */


// Thread argument structure
typedef struct {
    int value;
} thread_arg_t;

// Thread function that sends telemetry event
void* send_event_thread(void* arg)
{
    thread_arg_t* t_arg = (thread_arg_t*)arg;
    int value = t_arg->value;
    free(t_arg);
    
    printf("Thread %ld: Sending T2_INFO_Test=%d\n", (long)pthread_self(), value);
    t2_event_d("T2_INFO_Test", value);
    printf("Thread %ld: Event sent successfully\n", (long)pthread_self());
    
    return NULL;
}

int main(int argc, char *argv[])
{
    int i = 0, n;
    n = (argc < 2) ? 100 : atoi(argv[1]);
    
    // Initialize Telemetry2.0
    t2_init("telemetry_client");
    
    printf("Starting multi-threaded telemetry test with %d events\n", n);
    printf("Each event will be sent from a separate thread\n\n");
    
    // Array to store thread IDs
    pthread_t* threads = (pthread_t*)malloc((n + 1) * sizeof(pthread_t));
    if (!threads) {
        printf("Failed to allocate memory for threads\n");
        t2_uninit();
        return 1;
    }
    
    // Create a thread for each event
    while(i <= n)
    {
        thread_arg_t* arg = (thread_arg_t*)malloc(sizeof(thread_arg_t));
        if (!arg) {
            printf("Failed to allocate memory for thread argument\n");
            break;
        }
        arg->value = i;
        
        if (pthread_create(&threads[i], NULL, send_event_thread, (void*)arg) != 0) {
            printf("Failed to create thread %d\n", i);
            free(arg);
            break;
        }
        
        i++;
       usleep(100);
    }
    printf("\nAll %d threads created, waiting for completion...\n\n", i);
    
    // Wait for all threads to complete
    for (int j = 0; j < i; j++) {
        pthread_join(threads[j], NULL);
    }
    
    free(threads);
    
    printf("\n===========================================\n");
    printf("All threads completed!\n");
    printf("Sent %d t2_event_d events from %d threads.\n", i, i);
    printf("===========================================\n");
    
    t2_uninit();
    return 0;
}
