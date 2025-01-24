/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2024 RDK Management
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

extern "C" 
{
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <reportgen/reportgen.h>
#include <utils/vector.h>
#include <telemetry2_0.h>
#include <utils/t2common.h>
#include <bulkdata/profile.h>
#include <bulkdata/profilexconf.h>
#include <dcautil/dcautil.h>
#include <ccspinterface/busInterface.h>
sigset_t blocking_signal;
}
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "reportgenMock.h"
#include "../mocks/rdklogMock.h"
#include <iostream>
#include <stdexcept>

using namespace std;
using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;

rdklogMock *m_rdklogMock = NULL;

class rdklogTestFixture : public ::testing::Test {
    protected:
            rdklogMock rdklogmock_IO;

            rdklogTestFixture()
            {
                    m_rdklogMock = &rdklogmock_IO;
            }

            virtual ~rdklogTestFixture()
            {
                    m_rdklogMock = NULL;
            }
            virtual void SetUp()
            {
                    printf("%s\n", __func__);
            }

            virtual void TearDown()
            {
                    printf("%s\n", __func__);
            }
	    static void SetUpTestCase()
            {
                    printf("%s\n", __func__);
            }
            static void TearDownTestCase()
            {
                    printf("%s\n", __func__);
            }
};

TEST(DESTROY_JSONREPORT, CHECK_JSON)
{
    EXPECT_EQ(T2ERROR_INVALID_ARGS, destroyJSONReport(NULL));
}

cJSON *valarray = NULL;
Vector *paramNlist = NULL;
Vector *paramVlist = NULL;

TEST(ENCODEPARAMRESINJSON, NULL_CHECK)
{
    Vector_Create(&paramNlist);
    Vector_Create(&paramVlist);
    valarray = (cJSON*)malloc(256);
    EXPECT_EQ(T2ERROR_INVALID_ARGS, encodeParamResultInJSON(NULL, paramNlist, paramVlist));
    EXPECT_EQ(T2ERROR_INVALID_ARGS,  encodeParamResultInJSON(valarray, NULL, paramVlist));
    EXPECT_EQ(T2ERROR_INVALID_ARGS,  encodeParamResultInJSON(valarray, paramNlist, NULL));
    Vector_Destroy(paramNlist, free);
    Vector_Destroy(paramVlist, free);
    cJSON_Delete(valarray);
    if(valarray != NULL)
    {
        free(valarray);
        valarray = NULL;
    }
}

TEST(ENCODESTATICINJSON, NULL_CHECK)
{
    valarray = (cJSON*)malloc(256);
    Vector *staticparamlist = NULL;
    Vector_Create(&staticparamlist);
    EXPECT_EQ(T2ERROR_INVALID_ARGS, encodeStaticParamsInJSON(NULL, staticparamlist));
    EXPECT_EQ(T2ERROR_INVALID_ARGS, encodeStaticParamsInJSON(valarray, NULL));
    cJSON_Delete(valarray);
    if(valarray != NULL)
    {
        free(valarray);
        valarray = NULL;
    }
    Vector_Destroy(staticparamlist, free);
}

TEST(ENCODEGREPRESINJSON, NULL_CHECK)
{
    valarray = (cJSON*)malloc(256);
    Vector *greplist = NULL;
    Vector_Create(&greplist);
    EXPECT_EQ(T2ERROR_INVALID_ARGS, encodeStaticParamsInJSON(NULL, greplist));
    EXPECT_EQ(T2ERROR_INVALID_ARGS, encodeStaticParamsInJSON(valarray, NULL));
    cJSON_Delete(valarray);
    if(valarray != NULL)
    {
        free(valarray);
        valarray = NULL;
    }
    Vector_Destroy(greplist, free);
}


TEST(ENCODEEVENTRESINJSON, NULL_CHECK)
{
    valarray = (cJSON*)malloc(256);
    Vector *eventlist = NULL;
    Vector_Create(&eventlist);
    EXPECT_EQ(T2ERROR_INVALID_ARGS, encodeStaticParamsInJSON(NULL, eventlist));
    EXPECT_EQ(T2ERROR_INVALID_ARGS, encodeStaticParamsInJSON(valarray, NULL));
    cJSON_Delete(valarray);
    if(valarray != NULL)
    {
        free(valarray);
        valarray = NULL;
    }
    Vector_Destroy(eventlist, free);
}

TEST(PREPAREJSONREPORT, NULL_CHECK)
{
    char* report = NULL;
    EXPECT_EQ(T2ERROR_INVALID_ARGS,  prepareJSONReport(NULL, &report));
}

TEST(PREPAREHHTPURL, NULL_CHECK)
{
    EXPECT_EQ(NULL, prepareHttpUrl(NULL));
}

ReportgenMock *m_reportgenMock = NULL;

class reportgenTestFixture : public ::testing::Test {
    protected:
            ReportgenMock mock_IO;

            reportgenTestFixture()
            {
                    m_reportgenMock = &mock_IO;
            }

            virtual ~reportgenTestFixture()
            {
                    m_reportgenMock = NULL;
            }


            virtual void SetUp()
            {
                    printf("%s\n", __func__);
            }

            virtual void TearDown()
            {
                    printf("%s\n", __func__);
            }

            static void SetUpTestCase()
            {
                    printf("%s\n", __func__);
            }

            static void TearDownTestCase()
            {
                    printf("%s\n", __func__);
            }

};

TEST_F(reportgenTestFixture, PrepareHttpUrl)
{
        T2HTTP* data =  (T2HTTP *) malloc(sizeof(T2HTTP));
	data->URL = strdup("https://mockxconf:50051/dataLakeMock/");
	data->Compression  = COMP_NONE;
	data->Method = HTTP_POST;
	CURL* curl = (CURL*) NULL;
	EXPECT_CALL(*m_reportgenMock, curl_easy_init())
		.Times(1)
		.WillOnce(Return(curl));
	EXPECT_EQ(NULL, prepareHttpUrl(data));
	free(data->URL);
	free(data);
}

TEST_F(reportgenTestFixture, PrepareJSONReport)
{
      cJSON* jsonobj = (cJSON*)malloc(320);
      char* rb = NULL;
      EXPECT_CALL(*m_reportgenMock, cJSON_PrintUnformatted(_))
                .Times(1)
                .WillOnce(::testing::ReturnNull());
      EXPECT_EQ(T2ERROR_FAILURE, prepareJSONReport(jsonobj, &rb));
      cJSON_Delete(jsonobj);
      if(jsonobj != NULL)
      {
          free(jsonobj);
          jsonobj = NULL;
      }
}

TEST_F(reportgenTestFixture, destroyJSONReport)
{
      cJSON* jsonobj = (cJSON*)malloc(320);
      EXPECT_CALL(*m_reportgenMock, cJSON_Delete(_))
	      .Times(1);
      EXPECT_EQ(T2ERROR_SUCCESS, destroyJSONReport(jsonobj));
      // cJSON_Delete(jsonobj);
      if(jsonobj != NULL)
      {
          free(jsonobj);
          jsonobj = NULL;
      }
}

TEST_F(reportgenTestFixture, encodeStaticParamsInJSON)
{
      Vector* staticParamList = NULL;
      Vector_Create(&staticParamList);
      StaticParam *sparam = (StaticParam *) malloc(sizeof(StaticParam));
      sparam->paramType = strdup("datamodel");
      sparam->name = strdup("test1");
      sparam->value = strdup("value1");
      Vector_PushBack(staticParamList, sparam);
      cJSON *valArray = NULL;
      valArray = (cJSON*)malloc(320);
      cJSON* mockobj = NULL;
      EXPECT_CALL(*m_reportgenMock,  cJSON_CreateObject())
                 .Times(1)
                 .WillOnce(Return(mockobj));
      EXPECT_EQ(T2ERROR_FAILURE, encodeStaticParamsInJSON(valArray, staticParamList));
      cJSON_Delete(valArray);
      if(valArray != NULL)
      {
          free(valArray);
          valArray = NULL;
      }
      Vector_Destroy(staticParamList, freeStaticParam);
}

TEST_F(reportgenTestFixture, encodeStaticParamsInJSON1)
{
      Vector* staticParamList = NULL;
      Vector_Create(&staticParamList);
      StaticParam *sparam = (StaticParam *) malloc(sizeof(StaticParam));
      sparam->paramType = strdup("datamodel");
      sparam->name = strdup("test1");
      sparam->value = strdup("value1");
      Vector_PushBack(staticParamList, sparam);
      cJSON *valArray = NULL;
      valArray = (cJSON*)malloc(320);
      cJSON* mockobj = (cJSON *)0xffffffff;
      EXPECT_CALL(*m_reportgenMock,  cJSON_CreateObject())
                 .Times(1)
                 .WillOnce(Return(mockobj));
      EXPECT_CALL(*m_reportgenMock, cJSON_AddStringToObject(_,_,_))
               .Times(1)
               .WillOnce(::testing::ReturnNull());
      EXPECT_EQ(T2ERROR_FAILURE, encodeStaticParamsInJSON(valArray, staticParamList));
      cJSON_Delete(valArray);
      if(valArray != NULL)
      {
          free(valArray);
          valArray = NULL;
      }
      Vector_Destroy(staticParamList, freeStaticParam);
}

TEST_F(reportgenTestFixture, encodeGrepResultInJSON)
{
     Vector* grepResult = NULL;
     Vector_Create(&grepResult);
      GrepResult* gparam = (GrepResult *) malloc(sizeof(GrepResult));
      gparam->markerName = strdup("TEST_MARKER1");
      gparam->markerValue = strdup("TEST_STRING1");
      gparam->trimParameter = false;
      gparam->regexParameter = NULL;
      Vector_PushBack(grepResult, gparam);
      cJSON *valArray = NULL;
      valArray = (cJSON*)malloc(320);
      cJSON* mockobj = NULL;
      EXPECT_CALL(*m_reportgenMock,  cJSON_CreateObject())
                 .Times(1)
                 .WillOnce(Return(mockobj));
      EXPECT_EQ(T2ERROR_FAILURE, encodeGrepResultInJSON(valArray, grepResult));
      cJSON_Delete(valArray);
      if(valArray != NULL)
      {
          free(valArray);
          valArray = NULL;
      }
      Vector_Destroy(grepResult, freeGResult);
}

TEST_F(reportgenTestFixture, encodeGrepResultInJSON1)
{
      Vector* grepResult = NULL;
       Vector_Create(&grepResult);
      GrepResult* gparam = (GrepResult *) malloc(sizeof(GrepResult));
      gparam->markerName = strdup("TEST_MARKER1");
      gparam->markerValue = strdup("TEST_STRING1");
      gparam->trimParameter = false;
      gparam->regexParameter = NULL;
      Vector_PushBack(grepResult, gparam);
      cJSON *valArray = NULL;
      valArray = (cJSON*)malloc(320);
      cJSON* mockobj = (cJSON *)0xffffffff;
      EXPECT_CALL(*m_reportgenMock,  cJSON_CreateObject())
                 .Times(1)
                 .WillOnce(Return(mockobj));
      EXPECT_CALL(*m_reportgenMock, cJSON_AddStringToObject(_,_,_))
               .Times(1)
               .WillOnce(::testing::ReturnNull());
      EXPECT_EQ(T2ERROR_FAILURE, encodeGrepResultInJSON(valArray, grepResult));
      cJSON_Delete(valArray);
      if(valArray != NULL)
      {
          free(valArray);
          valArray = NULL;
      }
      Vector_Destroy(grepResult, freeGResult);
}

TEST_F(reportgenTestFixture, encodeParamResultInJSON)
{
    Vector *paramNameList = NULL;
    Vector *paramValueList = NULL;
    Vector_Create(&paramNameList);
    Vector_Create(&paramValueList);
    cJSON *valArray = NULL;
    valArray = (cJSON*)malloc(320);
    Param* param = (Param *) malloc(sizeof(Param));
    param->reportEmptyParam = true;
    param->paramType = strdup("event");
    param->name = strdup("Event1");
    param->alias = strdup("EventMarker1");
    param->trimParam = false;
    param->regexParam = NULL;
    Vector_PushBack(paramNameList, param);
    profileValues *profVals = (profileValues *) malloc(sizeof(profileValues));
    profVals->paramValues = (tr181ValStruct_t**) malloc(sizeof(tr181ValStruct_t*));
    profVals->paramValues[0] = (tr181ValStruct_t*) malloc(sizeof(tr181ValStruct_t));
    profVals->paramValues[0]->parameterName = strdup("Event1");
    profVals->paramValues[0]->parameterValue = strdup("NULL");
    profVals->paramValueCount = 0;
    Vector_PushBack(paramValueList, profVals);
    cJSON* mockobj = NULL;
    EXPECT_CALL(*m_reportgenMock, cJSON_CreateObject())
                 .Times(1)
                 .WillOnce(Return(mockobj));
    EXPECT_EQ(T2ERROR_FAILURE, encodeParamResultInJSON(valArray,paramNameList,paramValueList));
    cJSON_Delete(valArray);
    if(valArray != NULL)
    {
        free(valArray);
        valArray = NULL;
    }
    Vector_Destroy(paramNameList, freeParam);
    Vector_Destroy(paramValueList, freeProfileValues);
}

TEST_F(reportgenTestFixture, encodeParamResultInJSON1)
{
    Vector *paramNameList = NULL;
    Vector *paramValueList = NULL;
    Vector_Create(&paramNameList);
    Vector_Create(&paramValueList);
    cJSON* valArray = NULL;
    valArray = (cJSON*)malloc(320);
    Param* param = (Param *) malloc(sizeof(Param));
    param->reportEmptyParam = true;
    param->paramType = strdup("event");
    param->name = strdup("Event1");
    param->alias = strdup("EventMarker1");
    param->trimParam = false;
    param->regexParam = NULL;
    Vector_PushBack(paramNameList, param);
    profileValues *profVals = (profileValues *) malloc(sizeof(profileValues));
    profVals->paramValues = (tr181ValStruct_t**) malloc(sizeof(tr181ValStruct_t*));
    profVals->paramValues[0] = (tr181ValStruct_t*) malloc(sizeof(tr181ValStruct_t));
    profVals->paramValues[0]->parameterName = strdup("Event1");
    profVals->paramValues[0]->parameterValue = strdup("EventMarker1");
    profVals->paramValueCount = 1;
    Vector_PushBack(paramValueList, profVals);
    cJSON* mockobj = NULL;
    EXPECT_CALL(*m_reportgenMock, cJSON_CreateObject())
                 .Times(1)
                 .WillOnce(Return(mockobj));
    EXPECT_EQ(T2ERROR_FAILURE, encodeParamResultInJSON(valArray,paramNameList,paramValueList));
    cJSON_Delete(valArray);
    if(valArray != NULL)
    {
        free(valArray);
        valArray = NULL;
    }
    Vector_Destroy(paramNameList, freeParam);
    Vector_Destroy(paramValueList, freeProfileValues);
}

TEST_F(reportgenTestFixture, encodeParamResultInJSON5)
{
    Vector *paramNameList = NULL;
    Vector *paramValueList = NULL;
    Vector_Create(&paramNameList);
    Vector_Create(&paramValueList);
     cJSON* valArray = NULL;
    valArray = (cJSON*)malloc(320);
    Param* param = (Param *) malloc(sizeof(Param));
    param->reportEmptyParam = true;
    param->paramType = strdup("event");
    param->name = strdup("Event1");
    param->alias = strdup("EventMarker1");
    param->trimParam = false;
    param->regexParam = NULL;
    Vector_PushBack(paramNameList, param);
    Param* param1 = (Param *) malloc(sizeof(Param));
    param1->reportEmptyParam = true;
    param1->paramType = strdup("event");
    param1->name = strdup("Event2");
    param1->alias = strdup("EventMarker2");
    param1->trimParam = false;
    param1->regexParam = NULL;
    Vector_PushBack(paramNameList, param1);
    profileValues *profVals = (profileValues *) malloc(sizeof(profileValues));
    profVals->paramValues = (tr181ValStruct_t**) malloc(2 * sizeof(tr181ValStruct_t*));
    profVals->paramValues[0] = (tr181ValStruct_t*) malloc(sizeof(tr181ValStruct_t));
    profVals->paramValues[0]->parameterName = strdup("Event1");
    profVals->paramValues[0]->parameterValue = strdup("Event_Marker1");
    profVals->paramValues[1] = (tr181ValStruct_t*) malloc(sizeof(tr181ValStruct_t));
    profVals->paramValues[1]->parameterName = strdup("Event2");
    profVals->paramValues[1]->parameterValue = strdup("Event_Marker2");
    profVals->paramValueCount = 2;
    Vector_PushBack(paramValueList, profVals);
    EXPECT_CALL(*m_reportgenMock, cJSON_CreateObject())
                 .Times(1)
		 .WillOnce(::testing::ReturnNull());
    EXPECT_EQ(T2ERROR_FAILURE, encodeParamResultInJSON(valArray,paramNameList,paramValueList));
    cJSON_Delete(valArray);
    if(valArray != NULL)
    {
        free(valArray);
        valArray = NULL;
    }
    Vector_Destroy(paramNameList, freeParam);
    Vector_Destroy(paramValueList, freeProfileValues);
}

TEST_F(reportgenTestFixture, encodeParamResultInJSON6)
{
    Vector *paramNameList = NULL;
    Vector *paramValueList = NULL;
    Vector_Create(&paramNameList);
    Vector_Create(&paramValueList);
     cJSON* valArray = NULL;
    valArray = (cJSON*)malloc(320);
    Param* param = (Param *) malloc(sizeof(Param));
    param->reportEmptyParam = true;
    param->paramType = strdup("event");
    param->name = strdup("Event1");
    param->alias = strdup("EventMarker1");
    param->trimParam = false;
    param->regexParam = NULL;
    Vector_PushBack(paramNameList, param);
    Param* param1 = (Param *) malloc(sizeof(Param));
    param1->reportEmptyParam = true;
    param1->paramType = strdup("event");
    param1->name = strdup("Event2");
    param1->alias = strdup("EventMarker2");
    param1->trimParam = false;
    param1->regexParam = NULL;
    Vector_PushBack(paramNameList, param1);
    profileValues *profVals = (profileValues *) malloc(sizeof(profileValues));
    profVals->paramValues = (tr181ValStruct_t**) malloc(2 * sizeof(tr181ValStruct_t*));
    profVals->paramValues[0] = (tr181ValStruct_t*) malloc(sizeof(tr181ValStruct_t));
    profVals->paramValues[0]->parameterName = strdup("Event1");
    profVals->paramValues[0]->parameterValue = strdup("Event_Marker1");
    profVals->paramValues[1] = (tr181ValStruct_t*) malloc(sizeof(tr181ValStruct_t));
    profVals->paramValues[1]->parameterName = strdup("Event2");
    profVals->paramValues[1]->parameterValue = strdup("Event_Marker2");
    profVals->paramValueCount = 2;
    Vector_PushBack(paramValueList, profVals);
    cJSON* mockobj = (cJSON*)0xffffffff;
    EXPECT_CALL(*m_reportgenMock, cJSON_CreateObject())
                 .Times(2)
                 .WillOnce(Return(mockobj))
		 .WillOnce(::testing::ReturnNull());
    EXPECT_CALL(*m_reportgenMock, cJSON_CreateArray())
            .Times(1)
            .WillOnce(Return(mockobj));
    EXPECT_EQ(T2ERROR_FAILURE, encodeParamResultInJSON(valArray,paramNameList,paramValueList));
    cJSON_Delete(valArray);
    if(valArray != NULL)
    {
        free(valArray);
        valArray = NULL;
    }
    Vector_Destroy(paramNameList, freeParam);
    Vector_Destroy(paramValueList, freeProfileValues);
}

TEST_F(reportgenTestFixture, encodeParamResultInJSON7)
{
    Vector *paramNameList = NULL;
    Vector *paramValueList = NULL;
    Vector_Create(&paramNameList);
    Vector_Create(&paramValueList);
    cJSON* valArray = NULL;
    valArray = (cJSON*)malloc(320);
    Param* param = (Param *) malloc(sizeof(Param));
    param->reportEmptyParam = true;
    param->paramType = strdup("event");
    param->name = strdup("Event1");
    param->alias = strdup("EventMarker1");
    param->trimParam = false;
    param->regexParam = NULL;
    Vector_PushBack(paramNameList, param);
    profileValues *profVals = (profileValues *) malloc(sizeof(profileValues));
    profVals->paramValues = (tr181ValStruct_t**) malloc(sizeof(tr181ValStruct_t*));
    profVals->paramValues[0] = (tr181ValStruct_t*) malloc(sizeof(tr181ValStruct_t));
    profVals->paramValues[0]->parameterName = strdup("Event1");
    profVals->paramValues[0]->parameterValue = strdup("NULL");
    profVals->paramValueCount = 0;
    Vector_PushBack(paramValueList, profVals);
    cJSON* mockobj = (cJSON *)0xffffffff;
    EXPECT_CALL(*m_reportgenMock, cJSON_CreateObject())
                 .Times(1)
                 .WillOnce(Return(mockobj));
	EXPECT_CALL(*m_reportgenMock, cJSON_AddStringToObject(_,_,_))
               .Times(1)
               .WillOnce(::testing::ReturnNull());

    EXPECT_EQ(T2ERROR_FAILURE, encodeParamResultInJSON(valArray,paramNameList,paramValueList));
    cJSON_Delete(valArray);
    if(valArray != NULL)
    {
        free(valArray);
        valArray = NULL;
    }
    Vector_Destroy(paramNameList, freeParam);
    Vector_Destroy(paramValueList, freeProfileValues);
}

TEST_F(reportgenTestFixture, encodeParamResultInJSON8)
{
    Vector *paramNameList = NULL;
    Vector *paramValueList = NULL;
    Vector_Create(&paramNameList);
    Vector_Create(&paramValueList);
     cJSON* valArray = NULL;
    valArray = (cJSON*)malloc(320);
    Param* param = (Param *) malloc(sizeof(Param));
    param->reportEmptyParam = true;
    param->paramType = strdup("event");
    param->name = strdup("Event1");
    param->alias = strdup("EventMarker1");
    param->trimParam = false;
    param->regexParam = NULL;
    Vector_PushBack(paramNameList, param);
    profileValues *profVals = (profileValues *) malloc(sizeof(profileValues));
    profVals->paramValues = (tr181ValStruct_t**) malloc(sizeof(tr181ValStruct_t*));
    profVals->paramValues[0] = (tr181ValStruct_t*) malloc(sizeof(tr181ValStruct_t));
    profVals->paramValues[0]->parameterName = strdup("Event1");
    profVals->paramValues[0]->parameterValue = strdup("EventMarker1");
    profVals->paramValueCount = 1;
    Vector_PushBack(paramValueList, profVals);
	cJSON* mockobj = (cJSON *)0xffffffff;
    EXPECT_CALL(*m_reportgenMock, cJSON_CreateObject())
                 .Times(1)
                 .WillOnce(Return(mockobj));
	EXPECT_CALL(*m_reportgenMock, cJSON_AddStringToObject(_,_,_))
               .Times(1)
               .WillOnce(::testing::ReturnNull());
    EXPECT_EQ(T2ERROR_FAILURE, encodeParamResultInJSON(valArray,paramNameList,paramValueList));
    cJSON_Delete(valArray);
    if(valArray != NULL)
    {
        free(valArray);
        valArray = NULL;
    }
    Vector_Destroy(paramNameList, freeParam);
    Vector_Destroy(paramValueList, freeProfileValues);
}

TEST_F(reportgenTestFixture, encodeParamResultInJSON9)
{
    Vector *paramNameList = NULL;
    Vector *paramValueList = NULL;
    Vector_Create(&paramNameList);
    Vector_Create(&paramValueList);
    cJSON* valArray = NULL;
    valArray = (cJSON*)malloc(320);
    Param* param = (Param *) malloc(sizeof(Param));
    param->reportEmptyParam = true;
    param->paramType = strdup("event");
    param->name = strdup("Event1");
    param->alias = strdup("EventMarker1");
    param->trimParam = false;
    param->regexParam = NULL;
    Vector_PushBack(paramNameList, param);
    Param* param1 = (Param *) malloc(sizeof(Param));
    param1->reportEmptyParam = true;
    param1->paramType = strdup("event");
    param1->name = strdup("Event2");
    param1->alias = strdup("EventMarker2");
    param1->trimParam = false;
    param1->regexParam = NULL;

    Vector_PushBack(paramNameList, param1);
    profileValues *profVals = (profileValues *) malloc(sizeof(profileValues));
    profVals->paramValues = (tr181ValStruct_t**) malloc(2 * sizeof(tr181ValStruct_t*));
    profVals->paramValues[0] = (tr181ValStruct_t*) malloc(sizeof(tr181ValStruct_t));
    profVals->paramValues[0]->parameterName = strdup("Event1");
    profVals->paramValues[0]->parameterValue = strdup("Event_Marker1");
    profVals->paramValues[1] = (tr181ValStruct_t*) malloc(sizeof(tr181ValStruct_t));
    profVals->paramValues[1]->parameterName = strdup("Event2");
    profVals->paramValues[1]->parameterValue = strdup("Event_Marker2");

    profVals->paramValueCount = 2;
    Vector_PushBack(paramValueList, profVals);
    cJSON* mockobj = (cJSON*)0xffffffff;
    EXPECT_CALL(*m_reportgenMock, cJSON_CreateObject())
                 .Times(2)
                 .WillOnce(Return(mockobj))
                 .WillOnce(Return(mockobj));
    EXPECT_CALL(*m_reportgenMock, cJSON_CreateArray())
            .Times(1)
            .WillOnce(Return(mockobj));
    EXPECT_CALL(*m_reportgenMock, cJSON_AddStringToObject(_,_,_))
            .Times(1)
            .WillOnce(::testing::ReturnNull());
    EXPECT_EQ(T2ERROR_FAILURE, encodeParamResultInJSON(valArray,paramNameList,paramValueList));
    cJSON_Delete(valArray);
    if(valArray != NULL)
    {
        free(valArray);
        valArray = NULL;
    }
    Vector_Destroy(paramNameList, freeParam);
    Vector_Destroy(paramValueList, freeProfileValues);
}

TEST_F(reportgenTestFixture,  encodeEventMarkersInJSON)
{
     cJSON* valArray = NULL;
    valArray = (cJSON*)malloc(320);
    Vector *eventMarkerList = NULL;
    Vector_Create(&eventMarkerList);
    EventMarker *eMarker = (EventMarker *) malloc(sizeof(EventMarker));
    eMarker->markerName = strdup("Event1");
    eMarker->compName = strdup("sysint");
    eMarker->alias = strdup("EventMarker1");
    eMarker->paramType = strdup("event");
    eMarker->markerName_CT = strdup("Event1_CT");
    eMarker->timestamp = strdup("162716381732");
    eMarker->mType = MTYPE_COUNTER;
    eMarker->reportTimestampParam = REPORTTIMESTAMP_UNIXEPOCH;
    eMarker->u.count = 1;
    eMarker->trimParam = false;
    eMarker->regexParam = NULL;
    Vector_PushBack(eventMarkerList, eMarker);
    EXPECT_CALL(*m_reportgenMock,  cJSON_CreateObject())
            .Times(1)
            .WillOnce(::testing::ReturnNull());
    EXPECT_EQ(T2ERROR_FAILURE, encodeEventMarkersInJSON(valArray, eventMarkerList));
    cJSON_Delete(valArray);
    if(valArray != NULL)
    {
        free(valArray);
        valArray = NULL;
    }
    Vector_Destroy(eventMarkerList, freeEMarker);
}

TEST_F(reportgenTestFixture,  encodeEventMarkersInJSON1)
{
     cJSON* valArray = NULL;
    valArray = (cJSON*)malloc(320);
    Vector *eventMarkerList = NULL;
    Vector_Create(&eventMarkerList);
    EventMarker *eMarker = (EventMarker *) malloc(sizeof(EventMarker));
    eMarker->markerName = strdup("Event1");
    eMarker->compName = strdup("sysint");
    eMarker->alias = strdup("EventMarker1");
    eMarker->paramType = strdup("event");
    eMarker->markerName_CT = strdup("Event1_CT");
    eMarker->timestamp = strdup("162716381732");
    eMarker->trimParam = false;
    eMarker->regexParam = NULL;
    eMarker->mType = MTYPE_COUNTER;
    eMarker->reportTimestampParam = REPORTTIMESTAMP_UNIXEPOCH;
    eMarker->u.count = 1;
    Vector_PushBack(eventMarkerList, eMarker);
    cJSON* mockobj = (cJSON *) 0Xffffffff;
    EXPECT_CALL(*m_reportgenMock,  cJSON_CreateObject())
            .Times(1)
            .WillOnce(Return(mockobj));
    EXPECT_CALL(*m_reportgenMock, cJSON_AddStringToObject(_,_,_))
	    .Times(1)
	    .WillOnce(::testing::ReturnNull());
    EXPECT_EQ(T2ERROR_FAILURE, encodeEventMarkersInJSON(valArray, eventMarkerList));
    cJSON_Delete(valArray);
    if(valArray != NULL)
    {
        free(valArray);
        valArray = NULL;
    }
    Vector_Destroy(eventMarkerList, freeEMarker);

}

TEST_F(reportgenTestFixture,  encodeEventMarkersInJSON2)
{
    cJSON *valArray = (cJSON *)malloc(320);
    Vector *eventMarkerList = NULL;
    Vector_Create(&eventMarkerList);
    EventMarker *eMarker = (EventMarker *) malloc(sizeof(EventMarker));
    eMarker->markerName = strdup("Event1");
    eMarker->compName = strdup("sysint");
    eMarker->alias = strdup("EventMarker1");
    eMarker->paramType = strdup("event");
    eMarker->markerName_CT = strdup("Event1_CT");
    eMarker->timestamp = strdup("162716381732");
    eMarker->mType = MTYPE_COUNTER;
    eMarker->reportTimestampParam = REPORTTIMESTAMP_UNIXEPOCH;
    eMarker->u.count = 1;
    eMarker->trimParam = false;
    eMarker->regexParam = NULL;
    Vector_PushBack(eventMarkerList, eMarker);
    cJSON* mockobj = (cJSON *) 0Xffffffff;
    EXPECT_CALL(*m_reportgenMock,  cJSON_CreateObject())
            .Times(1)
            .WillOnce(Return(mockobj));
    EXPECT_CALL(*m_reportgenMock, cJSON_AddStringToObject(_,_,_))
            .Times(2)
	    .WillOnce(Return(mockobj))
            .WillOnce(::testing::ReturnNull());
    EXPECT_EQ(T2ERROR_FAILURE, encodeEventMarkersInJSON(valArray, eventMarkerList));
    cJSON_Delete(valArray);
    if(valArray != NULL)
    {
        free(valArray);
        valArray = NULL;
    }
    Vector_Destroy(eventMarkerList, freeEMarker);

}

TEST_F(reportgenTestFixture,  encodeEventMarkersInJSON4)
{
     cJSON* valArray = NULL;
    valArray = (cJSON*)malloc(320);
    Vector *eventMarkerList = NULL;
    Vector_Create(&eventMarkerList);
    EventMarker *eMarker = (EventMarker *) malloc(sizeof(EventMarker));
    eMarker->markerName = strdup("Event1");
    eMarker->compName = strdup("sysint");
    eMarker->paramType = strdup("event");
    eMarker->alias = strdup("EventMarker1");
    eMarker->markerName_CT = strdup("Event1_CT");
    eMarker->timestamp = strdup("162716381732");
    eMarker->mType = MTYPE_ABSOLUTE;
    eMarker->reportTimestampParam = REPORTTIMESTAMP_UNIXEPOCH;
    eMarker->u.markerValue = strdup("eventmissing");
    eMarker->trimParam = false;
    eMarker->regexParam = NULL;
    Vector_PushBack(eventMarkerList, eMarker);
    EXPECT_CALL(*m_reportgenMock,  cJSON_CreateObject())
            .Times(1)
            .WillOnce(::testing::ReturnNull());
    EXPECT_EQ(T2ERROR_FAILURE, encodeEventMarkersInJSON(valArray, eventMarkerList));
    cJSON_Delete(valArray);
    if(valArray != NULL)
    {
        free(valArray);
        valArray = NULL;
    }   
    Vector_Destroy(eventMarkerList, freeEMarker);

}

TEST_F(reportgenTestFixture,  encodeEventMarkersInJSON5)
{
     cJSON* valArray = NULL;
    valArray = (cJSON*)malloc(320);
    Vector *eventMarkerList = NULL;
    Vector_Create(&eventMarkerList);
    EventMarker *eMarker = (EventMarker *) malloc(sizeof(EventMarker));
    eMarker->markerName = strdup("Event1");
    eMarker->compName = strdup("sysint");
    eMarker->paramType = strdup("event");
    eMarker->alias = strdup("EventMarker1");
    eMarker->markerName_CT = strdup("Event1_CT");
    eMarker->timestamp = strdup("162716381732");
    eMarker->mType = MTYPE_ABSOLUTE;
    eMarker->reportTimestampParam = REPORTTIMESTAMP_UNIXEPOCH;
    eMarker->u.markerValue = strdup("eventmissing");
    eMarker->trimParam = false;
    eMarker->regexParam = NULL;
    Vector_PushBack(eventMarkerList, eMarker);
    cJSON* mockobj = (cJSON *) 0Xffffffff;
    EXPECT_CALL(*m_reportgenMock,  cJSON_CreateObject())
            .Times(1)
            .WillOnce(Return(mockobj));
    EXPECT_CALL(*m_reportgenMock, cJSON_AddStringToObject(_,_,_))
            .Times(1)
            .WillOnce(::testing::ReturnNull());
    EXPECT_EQ(T2ERROR_FAILURE, encodeEventMarkersInJSON(valArray, eventMarkerList));
    cJSON_Delete(valArray);
    if(valArray != NULL)
    {
        free(valArray);
        valArray = NULL;
    }
    Vector_Destroy(eventMarkerList, freeEMarker);

}

TEST_F(reportgenTestFixture,  encodeEventMarkersInJSON6)
{
     cJSON* valArray = NULL;
    valArray = (cJSON*)malloc(320);
    Vector *eventMarkerList = NULL;
    Vector_Create(&eventMarkerList);
    EventMarker *eMarker = (EventMarker *) malloc(sizeof(EventMarker));
    eMarker->markerName = strdup("Event1");
    eMarker->compName = strdup("sysint");
    eMarker->alias = strdup("EventMarker1");
    eMarker->paramType = strdup("event");
    eMarker->markerName_CT = strdup("Event1_CT");
    eMarker->timestamp = strdup("162716381732");
    eMarker->mType = MTYPE_ABSOLUTE;
    eMarker->reportTimestampParam = REPORTTIMESTAMP_UNIXEPOCH;
    eMarker->u.markerValue = strdup("eventmissing");
    eMarker->trimParam = false;
    eMarker->regexParam = NULL;
    Vector_PushBack(eventMarkerList, eMarker);
    cJSON* mockobj = (cJSON *) 0Xffffffff;
    EXPECT_CALL(*m_reportgenMock,  cJSON_CreateObject())
            .Times(1)
            .WillOnce(Return(mockobj));
    EXPECT_CALL(*m_reportgenMock, cJSON_AddStringToObject(_,_,_))
            .Times(2)
            .WillOnce(Return(mockobj))
            .WillOnce(::testing::ReturnNull());
    EXPECT_EQ(T2ERROR_FAILURE, encodeEventMarkersInJSON(valArray, eventMarkerList));
    cJSON_Delete(valArray);
    if(valArray != NULL)
    {
        free(valArray);
        valArray = NULL;
    }
    Vector_Destroy(eventMarkerList, freeEMarker);

}

TEST_F(reportgenTestFixture,  encodeEventMarkersInJSON8)
{
     cJSON* valArray = NULL;
    valArray = (cJSON*)malloc(320);
    Vector *eventMarkerList = NULL;
    Vector_Create(&eventMarkerList);
    EventMarker *eMarker = (EventMarker *) malloc(sizeof(EventMarker));
    eMarker->markerName = strdup("Event1");
    eMarker->compName = strdup("sysint");
    eMarker->alias = strdup("EventMarker1");
    eMarker->paramType = strdup("event");
    eMarker->timestamp = NULL;
    eMarker->markerName_CT = NULL;
    eMarker->mType = MTYPE_ACCUMULATE;
    eMarker->trimParam = false;
    eMarker->regexParam = NULL;
    eMarker->reportTimestampParam = REPORTTIMESTAMP_UNIXEPOCH;
    Vector_Create(&eMarker->u.accumulatedValues);
    Vector_Create(&eMarker->accumulatedTimestamp);
    Vector_PushBack(eMarker->u.accumulatedValues, strdup("125"));
    Vector_PushBack(eMarker->u.accumulatedValues, strdup("124"));
    Vector_PushBack(eMarker->accumulatedTimestamp, strdup("1678613787878"));
    Vector_PushBack(eMarker->accumulatedTimestamp, strdup("167861378164"));
    Vector_PushBack(eventMarkerList, eMarker);
    EXPECT_CALL(*m_reportgenMock,  cJSON_CreateObject())
            .Times(1)
            .WillOnce(::testing::ReturnNull());
    EXPECT_EQ(T2ERROR_FAILURE, encodeEventMarkersInJSON(valArray, eventMarkerList));
    cJSON_Delete(valArray);
    if(valArray != NULL)
    {
        free(valArray);
        valArray = NULL;
    }
    Vector_Destroy(eventMarkerList, freeEMarker);

}

TEST_F(reportgenTestFixture,  encodeEventMarkersInJSON9)
{
     cJSON* valArray = NULL;
    valArray = (cJSON*)malloc(320);
    Vector *eventMarkerList = NULL;
    Vector_Create(&eventMarkerList);
    EventMarker *eMarker = (EventMarker *) malloc(sizeof(EventMarker));
    eMarker->markerName = strdup("Event1");
    eMarker->compName = strdup("sysint");
    eMarker->paramType = strdup("event");
    eMarker->alias = strdup("EventMarker1");
    eMarker->timestamp = NULL;
    eMarker->markerName_CT = NULL;
    eMarker->mType = MTYPE_ACCUMULATE;
    eMarker->trimParam = false;
    eMarker->regexParam = NULL;
    eMarker->reportTimestampParam = REPORTTIMESTAMP_UNIXEPOCH;
    Vector_Create(&eMarker->u.accumulatedValues);
    Vector_Create(&eMarker->accumulatedTimestamp);
    Vector_PushBack(eMarker->u.accumulatedValues, strdup("125"));
    Vector_PushBack(eMarker->u.accumulatedValues, strdup("124"));
    Vector_PushBack(eMarker->accumulatedTimestamp, strdup("1678613787878"));
    Vector_PushBack(eMarker->accumulatedTimestamp, strdup("167861378164"));
    Vector_PushBack(eventMarkerList, eMarker);
    cJSON* mockobj = (cJSON*) 0Xfffffff;
    EXPECT_CALL(*m_reportgenMock,  cJSON_CreateObject())
            .Times(1)
            .WillOnce(Return(mockobj));
    EXPECT_CALL(*m_reportgenMock, cJSON_CreateArray())
	    .Times(1)
	    .WillOnce(::testing::ReturnNull());
    EXPECT_EQ(T2ERROR_FAILURE, encodeEventMarkersInJSON(valArray, eventMarkerList));
    cJSON_Delete(valArray);
    if(valArray != NULL)
    {
        free(valArray);
        valArray = NULL;
    }
    Vector_Destroy(eventMarkerList, freeEMarker);
}

