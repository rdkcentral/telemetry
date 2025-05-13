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
#include <stdlib.h>
#include <string.h>

#include "dcautil.h"
#include "profile.h"
#include "vector.h"
#include "t2common.h"
#include "telemetry2_0.h"
#include "t2log_wrapper.h"


#include "busInterfaceTests.h"


void destroyer_func(void *data)
{
    if (data)
    {
        GrepMarker *gMarker = (GrepMarker *)data;
        if (gMarker->markerName)
        {
            free(gMarker->markerName);
            gMarker->markerName = NULL ;
        }
        if (gMarker->searchString)
        {
            free(gMarker->searchString);
            gMarker->searchString = NULL ;
        }
        if (gMarker->logFile)
        {
            free(gMarker->logFile);
            gMarker->logFile = NULL ;
        }
        free(gMarker);
    }
}

void backupAndAppendLogFile(const char *filePath)
{
    if (!filePath)
    {
        printf("Invalid file path provided.\n");
        return;
    }

    // Create backup file path
    char backupFilePath[256];
    snprintf(backupFilePath, sizeof(backupFilePath), "%s.bak", filePath);

    // Open the original file for reading
    FILE *originalFile = fopen(filePath, "r");
    if (!originalFile)
    {
        printf("Failed to open the original file: %s\n", filePath);
        return;
    }

    // Open the backup file for writing
    FILE *backupFile = fopen(backupFilePath, "w");
    if (!backupFile)
    {
        printf("Failed to create the backup file: %s\n", backupFilePath);
        fclose(originalFile);
        return;
    }

    // Copy content from the original file to the backup file
    char buffer[1024];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), originalFile)) > 0)
    {
        fwrite(buffer, 1, bytesRead, backupFile);
    }

    fclose(originalFile);
    fclose(backupFile);

    // Open the original file for appending
    FILE *file = fopen(filePath, "a");
    if (!file)
    {
        printf("Failed to open the file for appending: %s\n", filePath);
        return;
    }

    // Append default lines to the file
    fprintf(file, "\n### Induced log entries from test #### \n");
    fprintf(file, "2025-04-08T14:17:46.053Z com.sky.as.apps_com.bskyb.epgui[3320]:  GlobalTimeoutManager.log: On Wake - systemState: ACTIVE  - wakeReason: DOCKER WAKEUP\n");
    fprintf(file, "2025-04-08T14:58:24.897Z appsserviced[9543]:  failed to reconnect to ws 'ws://DOCKER_EMULATION/jsonrpc/org.rdk.SystemMode' on attempt 601 - The server did not accept the WebSocket handshake\n");
    fclose(file);

    printf("Backup created and default lines appended to the file: %s\n", filePath);
}

/**
 * This is to primarily unit test api getGrepResults
 * primarily used for getting log grep results
 *
 * Create Vector with 2 marker vals
 * Push expected match pattern to logs and check for return pattern
 * Push similar entries and verify counts are appropriate
 */

// TODO add asserts for future and plugin fixed sample log files
void logGrepTest1()
{
    printf("%s ++in \n", __FUNCTION__ );

    T2ERROR ret = T2ERROR_FAILURE ;
    // 1st Split parameter from rotated files
    GrepMarker *gMarker1 = (GrepMarker *)malloc(sizeof(GrepMarker));
    memset(gMarker1, 0, sizeof(GrepMarker));
    gMarker1->markerName = strdup("wakeReason_split");
    gMarker1->searchString = strdup("wakeReason:");
    gMarker1->logFile = strdup("messages.log");
    gMarker1->skipFreq = 0 ;

    //2nd Split parameter
    // 1169 occurence in control log file
    GrepMarker *gMarker2 = (GrepMarker *)malloc(sizeof(GrepMarker));
    memset(gMarker2, 0, sizeof(GrepMarker));
    gMarker2->markerName = strdup("FailedToConnect_split");
    gMarker2->searchString = strdup("failed to reconnect to ws");
    gMarker2->logFile = strdup("messages.log");
    gMarker2->skipFreq = 0 ;

    /**
     * Intensional different file name in between to make sure legacy utils handles the sorting internally
     */
    GrepMarker *gMarker3 = (GrepMarker *)malloc(sizeof(GrepMarker));
    memset(gMarker3, 0, sizeof(GrepMarker));
    gMarker3->markerName = strdup("DO_NOT_REPORT_split");
    gMarker3->searchString = strdup("no matching pattern");
    gMarker3->logFile = strdup("non-existing.log");
    gMarker3->skipFreq = 0 ;

    // 1st Counter marker
    GrepMarker *gMarker4 = (GrepMarker *)malloc(sizeof(GrepMarker));
    memset(gMarker4, 0, sizeof(GrepMarker));
    gMarker4->markerName = strdup("FailedToConnect");
    gMarker4->mType = MTYPE_COUNTER;
    gMarker4->u.count = 0;
    gMarker4->searchString = strdup("failed to reconnect to ws");
    gMarker4->logFile = strdup("messages.log");
    gMarker4->skipFreq = 0 ;

    Vector* markerlist = NULL ;
    Vector* grepResultList = NULL ;

    if (T2ERROR_FAILURE == Vector_Create(&markerlist))
    {
        printf("%s %d %s \n", __FUNCTION__, __LINE__, "Error creating markerlist<GrepMarker> vector");
    }

    if (T2ERROR_FAILURE == Vector_Create(&grepResultList))
    {
        printf("%s %d %s \n", __FUNCTION__, __LINE__, "Error creating grepResultList<GrepResult> vector");
    }

    /**
    * Marker addition starts here
    */
    if (T2ERROR_FAILURE == Vector_PushBack(markerlist, gMarker1))
    {
        printf("%s %d %s \n", __FUNCTION__, __LINE__, "Error pushing entry1 to vector markerlist");
    }

    if (T2ERROR_FAILURE == Vector_PushBack(markerlist, gMarker2))
    {
        printf("%s %d %s \n", __FUNCTION__, __LINE__, "Error pushing entry2 to vector markerlist");
    }


    if (T2ERROR_FAILURE == Vector_PushBack(markerlist, gMarker4))
    {
        printf("%s %d %s \n", __FUNCTION__, __LINE__, "Error pushing entry4 to vector markerlist");
    }

    if (T2ERROR_FAILURE == Vector_PushBack(markerlist, gMarker3))
    {
        printf("%s %d %s \n", __FUNCTION__, __LINE__, "Error pushing entry3 to vector markerlist");
    }



    printf("\n\n################## Start of test set ################## \n\n\n");
    clock_t start_time = clock(); // Start profiling
    ret = getGrepResults("profile1", markerlist, &grepResultList, false, false, NULL);
    clock_t end_time = clock(); // End profiling

    double time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC; // Calculate elapsed time
    printf("\ngetGrepResults execution time: %f seconds\n", time_taken);

    // Print markerlist
    printf("============== Marker list contents: ==============\n");
    int markerListSize = Vector_Size(markerlist);
    for (int i = 0; i < markerListSize; ++i)
    {
        GrepMarker* marker = (GrepMarker*)Vector_At(markerlist, i);
        printf(" Marker Name: %s, Search String: %s, Log File: %s, Skip Frequency: %d\n",
               marker->markerName, marker->searchString, marker->logFile, marker->skipFreq);
    }
    printf("====================================================\n");

    printf("============== Data collected from logs : ==============\n");
    if (ret == T2ERROR_SUCCESS)
    {
        int resultSize = Vector_Size(grepResultList);
        int var = 0 ;
        for (var = 0; var < resultSize; ++var)
        {
            GrepResult* result = (GrepResult *) Vector_At(grepResultList, var);
            printf(" Marker = %s , Value = %s \n", result->markerName, result->markerValue);
        }

    }
    else
    {
        printf("\n\n\n  %s %d %s \n\n\n", __FUNCTION__, __LINE__, "Something quite not right .... Debug now !!!!");
    }
    printf("====================================================\n");

    printf("\n##################End of test set ################## \n\n\n");
    if (grepResultList)
    {
        free(grepResultList);
        grepResultList = NULL ;
    }

    backupAndAppendLogFile("/opt/logs/messages.log");
    if (T2ERROR_FAILURE == Vector_Create(&grepResultList))
    {
        printf("%s %d %s \n", __FUNCTION__, __LINE__, "Error creating grepResultList<GrepResult> vector");
    }
    printf("\n\n################## Start of 2nd test set ################## \n\n\n");
    start_time = clock(); // Start profiling
    ret = getGrepResults("profile1", markerlist, &grepResultList, false, false, NULL);
    end_time = clock(); // End profiling

    time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC; // Calculate elapsed time
    printf("\ngetGrepResults execution time: %f seconds\n", time_taken);

    printf("============== Data collected from logs : ==============\n");
    if (ret == T2ERROR_SUCCESS)
    {
        int resultSize = Vector_Size(grepResultList);
        int var = 0 ;
        for (var = 0; var < resultSize; ++var)
        {
            GrepResult* result = (GrepResult *) Vector_At(grepResultList, var);
            printf(" Marker = %s , Value = %s \n", result->markerName, result->markerValue);
        }

    }
    else
    {
        printf("\n\n\n  %s %d %s \n\n\n", __FUNCTION__, __LINE__, "Something quite not right .... Debug now !!!!");
    }
    printf("====================================================\n");

    printf("\n##################End of test set ################## \n\n\n");
    if (grepResultList)
    {
        free(grepResultList);
        grepResultList = NULL ;
    }

    Vector_Destroy(markerlist, destroyer_func);
    markerlist = NULL ;

    printf("%s ++out \n", __FUNCTION__);
    return ;
}



int main()
{

    LOGInit();
    logGrepTest1();
    return 0;
}
