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
#include <stdbool.h>

// Supporting sub-module headers
#include <telemetry2_0.h>
#include <t2common.h>
#include <reportgen.h>

#include <busInterface.h>

static const char* TR181_PARAM = "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Telemetry.Version";
static const char* TR181_MODEL = "Device.DeviceInfo.ModelName";
static const char* TR181_SYND_SETTINGS = "Device.DeviceInfo.X_RDKCENTRAL-COM_Syndication.";

static const char* TR181_BAD_PARAM = "Device.DeviceInfo.X_RDKCENTRAL-COM" ;

void testGetProfileParameterValues( ) {
    // Vector* getProfileParameterValues(Vector *paramList)
    printf("\n ====> Inside %s \n\n", __FUNCTION__);
    Vector* paramList = NULL;
    Vector* pValueList = NULL;
    printf("Create vector for paramList \n");
    Vector_Create(&paramList);

    if(!paramList) {
        printf("paramList creation failed ... Ignoring tests !!!\n");
        return;
    }else {
        printf("Create vector for paramList success ... \n");
    }

    Param *paramModel = (Param *) malloc(sizeof(Param));
    if(paramModel) {
        memset(paramModel, 0, sizeof(Param));
        paramModel->paramType = strdup("dataModel");
        paramModel->name = TR181_MODEL;
        paramModel->alias = TR181_MODEL;
        //Vector_PushBack(paramList, paramModel); // Push a scalar param
    }else {
        printf("Unable to allocate memory \n");
    }
    printf("Param paramModel added \n");

    Param *paramSyndPartTable = (Param *) malloc(sizeof(Param));
    if(paramSyndPartTable) {
        memset(paramSyndPartTable, 0, sizeof(Param));
        paramSyndPartTable->paramType = strdup("dataModel");
        paramSyndPartTable->name = TR181_SYND_SETTINGS;
        paramSyndPartTable->alias = TR181_SYND_SETTINGS;
        Vector_PushBack(paramList, paramSyndPartTable); // Push a table param

    }else {
        printf("Unable to allocate memory \n");
    }
    printf("Param paramSyndPartTable added \n");

    Param *paramBad = (Param *) malloc(sizeof(Param));
    if(paramBad) {
        memset(paramBad, 0, sizeof(Param));
        paramSyndPartTable->paramType = strdup("dataModel");
        paramBad->name = TR181_BAD_PARAM;
        paramBad->alias = TR181_BAD_PARAM;
        Vector_PushBack(paramList, paramBad); // Push a bad tr181 param
    }else {
        printf("Unable to allocate memory \n");
    }
    printf("Param paramBad added \n");
    // Push a set of params for query

    printf("Calling getProfileParameterValues\n\n");
    pValueList = getProfileParameterValues(paramList);
    printf("Expect device model value, syndication partner settings value and again device model value\n\n");

    if(pValueList) {
        int i = 0;
        for( i = 0; i < Vector_Size(paramList); ++i ) {
            int iterate = 0;
            profileValues* retValObj = (profileValues*) Vector_At(pValueList, i);
            if(retValObj) {
                int paramValueCount = retValObj->paramValueCount;
                tr181ValStruct_t** valObjArr = retValObj->paramValues;
                if(paramValueCount == 0) {
                    printf("Value for param  %s = %s \n", (valObjArr[0])->parameterName, (valObjArr[0])->parameterValue);
                }else {
                    for( iterate = 0; iterate < paramValueCount; ++iterate ) {
                        printf("Value for param  %s = %s \n", (valObjArr[iterate])->parameterName, (valObjArr[iterate])->parameterValue);

                    }
                }
            }
        }
        // Vector_Destroy(&pValueList, freeProfileValues); // Ignore for testapp due to interdependency

    }else {
        printf("Return value list is null \n");
    }

    // Vector_Destroy(&paramList, freeParam); // Ignore for testapp due to interdependency
    printf("\n\n Exiting %s ====>\n", __FUNCTION__);
}

void testGetParameterValue( ) {

    printf("\n ====> Inside %s \n\n", __FUNCTION__);
    char *paramValue = NULL;
    T2ERROR retStatus = T2ERROR_FAILURE;

    retStatus = getParameterValue(TR181_PARAM, &paramValue);
    if(retStatus == T2ERROR_SUCCESS) {
        if(NULL != paramValue)
            printf("%s = %s \n\n ", TR181_PARAM, paramValue);
        else
            printf("Unable to get value from getParameterValue interface calls \n");
    }else {
        printf("Interface calls returns error status \n\n");
    }
    printf("\n\n Exiting %s ====>\n", __FUNCTION__);
}

/**
 * Performs tests related to sub module businterface .
 * Primary focus is on ensuring that CCSP vs rbus porting efforts to make sure nothing is broken
 *
 */
void testBusInterface( ) {

    testGetParameterValue();
    testGetProfileParameterValues();

}

