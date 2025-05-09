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
    gMarker1->logFile = strdup("sky-messages.log");
    gMarker1->skipFreq = 0 ;

    //2nd Split parameter
    // 1169 occurence in control log file
    GrepMarker *gMarker2 = (GrepMarker *)malloc(sizeof(GrepMarker));
    memset(gMarker2, 0, sizeof(GrepMarker));
    gMarker2->markerName = strdup("FailedToConnect_split");
    gMarker2->searchString = strdup("failed to reconnect to ws");
    gMarker2->logFile = strdup("sky-messages.log");
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
    gMarker4->logFile = strdup("sky-messages.log");
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

    Vector_Destroy(markerlist, destroyer_func);
    markerlist = NULL ;


#if 0
    printf("\n\n\nWaiting for 2nd test suite exec\n\n\n" ) ;
    printf("Simulating log rotation scenario by renaming the log file ...\n");

    if (system("cp /opt/logs/sky-messages.log /opt/logs/sky-messages.log.original") != 0) {
        perror("Error creating a backup of the original log file");
    } else {
        printf("Original log file successfully backed up as /opt/logs/sky-messages.log.original\n");
    }

    if (rename("/opt/logs/sky-messages.log", "/opt/logs/sky-messages.log.1") != 0) {
        perror("Error renaming log file");
    } else {
        printf("Log file successfully moved to /opt/logs/sky-messages.log.1\n");
    }
    FILE *logFile = fopen("/opt/logs/sky-messages.log", "w");
    if (logFile != NULL) {
        fprintf(logFile, "Starting 2nd test suite execution...\n");
        fclose(logFile);
    } else {
        printf("Error opening log file for appending.\n");
    }

    sleep(5); // Buffer time for tracing
    printf("\n\n Start of test set ################## \n\n\n");
    printf("Start time %s \n", ctime(&now));
    ret = getGrepResults("profile1", markerlist, &grepResultList, false, false, NULL);
    printf("!!! Results should be absolutely nothing else !!! \n");
    if (ret == T2ERROR_SUCCESS)
    {
        int resultSize = Vector_Size(grepResultList);
        printf("%s 2nd set of tests with continuous exec Got results .. loop and print data of size = %d\n", __FUNCTION__, resultSize);
        int var = 0 ;
        for (var = 0; var < resultSize; ++var)
        {
            GrepResult* result = (GrepResult *) Vector_At(grepResultList, var);
            printf(" Marker = %s , Value = %s \n", result->markerName, result->markerValue);
        }

    }
    else
    {
        printf("%s \n", "Something quite not right .... Debug now !!!!");
    }
    printf("End of test set ################## \n\n\n");

    if (grepResultList)
    {
        // TODO Free up the nodes
        free(grepResultList);
        grepResultList = NULL ;
    }

    printf("\n\n\nWaiting for another 3rd test suite exec for skip frequency check\n\n\n" ) ;
    sleep(20); // Buffer time for tracing
    printf("\n\n Start of test set ################## \n\n\n");
    printf("!!! Results should contain only message bus parameters . Nothing else !!! \n");
    printf("Start time %s \n", ctime(&now));
    ret = getGrepResults("profile1", markerlist, &grepResultList, false, false, NULL);
    printf("End time %s \n", ctime(&now));
    if (ret == T2ERROR_SUCCESS)
    {
        int resultSize = Vector_Size(grepResultList);
        printf("%s 3nd set of tests with continuous exec Got results. Result size = %d\n", __FUNCTION__, resultSize);
        int var = 0 ;
        for (var = 0; var < resultSize; ++var)
        {
            GrepResult* result = (GrepResult *) Vector_At(grepResultList, var);
            printf(" Marker = %s , Value = %s \n", result->markerName, result->markerValue);
        }
    }
    else
    {
        printf("%s \n", "Something quite not right .... Debug now !!!!");
    }
#endif

    printf("%s ++out \n", __FUNCTION__);
    return ;
}

#if 0
/**
 * Run same tests repeated for a different profile name
 */
void logGrepTest2()
{
    printf("%s ++in \n", __FUNCTION__ );

    T2ERROR ret = T2ERROR_FAILURE ;
    // Split parameter

    GrepMarker *gMarker1 = (GrepMarker *)malloc(sizeof(GrepMarker));
    gMarker1->markerName = strdup("EXPECTED_WIFI_VAP_split");
    gMarker1->searchString = strdup("WIFI_VAP_PERCENT_UP");
    gMarker1->logFile = strdup("wifihealth.txt");
    gMarker1->skipFreq = 0 ;

    //Split parameter
    GrepMarker *gMarker2 = (GrepMarker *)malloc(sizeof(GrepMarker));
    gMarker2->markerName = strdup("EXPECTED_WIFI_COUNTRY_CODE_split");
    gMarker2->searchString = strdup("WIFI_COUNTRY_CODE_1");
    gMarker2->logFile = strdup("wifihealth.txt");
    gMarker2->skipFreq = 0 ;

    // TODO Add 1 counter marker to file 1
    // TODO Add 1 message_bus marker to file 1


    /**
     * Intensional different file name in between to make sure legacy utils handles the sorting internally
     */

    GrepMarker *gMarker3 = (GrepMarker *)malloc(sizeof(GrepMarker));
    gMarker3->markerName = strdup("DO_NOT_REPORT_split");
    gMarker3->searchString = strdup("no matching pattern");
    gMarker3->logFile = strdup("console.log");
    gMarker3->skipFreq = 0 ;

    // TODO Add 1 counter marker to file 2
    GrepMarker *gMarker4 = (GrepMarker *)malloc(sizeof(GrepMarker));
    gMarker4->markerName = strdup("EXPECTED_MARKER_AFTER_SORTING");
    gMarker4->searchString = strdup("WIFI_CHANNEL_1");
    gMarker4->logFile = strdup("wifihealth.txt");
    gMarker4->skipFreq = 0 ;

    // TODO Add 1 message_bus marker to file 2
    GrepMarker *gMarker5 = (GrepMarker *)malloc(sizeof(GrepMarker));
    gMarker5->markerName = strdup("EXPECTED_TR181_MESH_ENABLE_STATUS");
    gMarker5->searchString = strdup("Device.DeviceInfo.X_RDKCENTRAL-COM_xOpsDeviceMgmt.Mesh.Enable");
    gMarker5->logFile = strdup("<message_bus>");
    gMarker5->skipFreq = 0 ;

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


    if (T2ERROR_FAILURE == Vector_PushBack(markerlist, gMarker5))
    {
        printf("%s %d %s \n", __FUNCTION__, __LINE__, "Error pushing entry5 to vector markerlist");
    }

    if (T2ERROR_FAILURE == Vector_PushBack(markerlist, gMarker3))
    {
        printf("%s %d %s \n", __FUNCTION__, __LINE__, "Error pushing entry3 to vector markerlist");
    }


    time_t now;
    printf("\n\n Start of test set ################## \n\n\n");
    printf("Start time %s \n", ctime(&now));
    ret = getGrepResults("profile2", markerlist, &grepResultList, false, false, NULL);
    printf("End time %s \n", ctime(&now));

    if (ret == T2ERROR_SUCCESS)
    {
        int resultSize = Vector_Size(grepResultList);
        printf("\n\n\n %s Got results .. loop and print data of size = %d \n\n\n", __FUNCTION__, resultSize);
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
    printf("\n\n End of test set ################## \n\n\n");
    if (grepResultList)
    {
        // TODO Free up the nodes
        free(grepResultList);
        grepResultList = NULL ;
    }


    printf("\n\n\nWaiting for another test suite exec\n\n\n" ) ;
    sleep(20); // Buffer time for tracing
    printf("\n\n Start of test set ################## \n\n\n");
    printf("!!! Results should be everything - for profile 3 !!! \n");
    printf("Start time %s \n", ctime(&now));
    ret = getGrepResults("profile3", markerlist, &grepResultList, false, false, NULL);
    printf("End time %s \n", ctime(&now));
    if (ret == T2ERROR_SUCCESS)
    {
        int resultSize = Vector_Size(grepResultList);
        printf("%s 2nd set of tests with continuous exec Got results .. loop and print data of size = %d\n", __FUNCTION__, resultSize);
        int var = 0 ;
        for (var = 0; var < resultSize; ++var)
        {
            GrepResult* result = (GrepResult *) Vector_At(grepResultList, var);
            printf(" Marker = %s , Value = %s \n", result->markerName, result->markerValue);
        }

    }
    else
    {
        printf("%s \n", "Something quite not right .... Debug now !!!!");
    }
    printf("End of test set ################## \n\n\n");
    if (grepResultList)
    {
        // TODO Free up the nodes
        free(grepResultList);
        grepResultList = NULL ;
    }

    printf("\n\n\nWaiting for another test suite exec for skip frequency check\n\n\n" ) ;
    sleep(20); // Buffer time for tracing
    printf("\n\n Start of test set ################## \n\n\n");
    printf("!!! Results should be everything - for profile 4 !!! \n");
    printf("Start time %s \n", ctime(&now));
    ret = getGrepResults("profile4", markerlist, &grepResultList, false, false , NULL);
    printf("End time %s \n", ctime(&now));
    if (ret == T2ERROR_SUCCESS)
    {
        int resultSize = Vector_Size(grepResultList);
        printf("%s 3rd set of tests with continuous exec Got results. Result size = %d\n", __FUNCTION__, resultSize);
        int var = 0 ;
        for (var = 0; var < resultSize; ++var)
        {
            GrepResult* result = (GrepResult *) Vector_At(grepResultList, var);
            printf(" Marker = %s , Value = %s \n", result->markerName, result->markerValue);
        }
    }
    else
    {
        printf("%s \n", "Something quite not right .... Debug now !!!!");
    }
    printf("End of test set ################## \n\n\n");
    printf("%s %d ++out \n", __FUNCTION__, __LINE__);
    return ;
}

static void logGrepTests()
{

    printf("Validating dcautils test cases .... \n \n");

    logGrepTest1();
    sleep(2);
    logGrepTest2();

    printf("End of dcautils test cases .... \n \n");

}


#endif 

int main()
{

    LOGInit();
    logGrepTest1();
    return 0;
}
