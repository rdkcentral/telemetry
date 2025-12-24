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

extern "C"
{
  void convertVectorToJson(cJSON *output, Vector *input);
}

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
void FreeString(void* item) { free(item); }

TEST(ConvertVectorToJson, OutputNotNull_VectorEmpty)
{
    cJSON *output = cJSON_CreateArray();
    Vector *input = nullptr;
    ASSERT_EQ(Vector_Create(&input), T2ERROR_SUCCESS);

    convertVectorToJson(output, input);
    EXPECT_EQ(cJSON_GetArraySize(output), 0);

    Vector_Destroy(input, FreeString);
    cJSON_Delete(output);
}

TEST(DESTROY_JSONREPORT, CHECK_JSON)
{
    EXPECT_EQ(T2ERROR_INVALID_ARGS, destroyJSONReport(NULL));
}

TEST(ENCODEPARAMRESINJSON, NULL_CHECK)
{
    Vector *paramNlist = NULL;
    Vector *paramVlist = NULL;
    cJSON *valarray = (cJSON*)malloc(sizeof(cJSON));
    Vector_Create(&paramNlist);
    Vector_Create(&paramVlist);
    Vector_PushBack(paramNlist, (void*)strdup("param1"));
    Vector_PushBack(paramVlist, (void*)strdup("value1"));
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
    cJSON* valarray = (cJSON*)malloc(sizeof(cJSON));
    Vector *staticparamlist = NULL;
    Vector_Create(&staticparamlist);
    Vector_PushBack(staticparamlist, (void*)strdup("param1"));
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
    cJSON* valarray = (cJSON*)malloc(sizeof(cJSON));
    Vector *greplist = NULL;
    Vector_Create(&greplist);
    Vector_PushBack(greplist, (void*)strdup("grep1"));
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
    cJSON* valarray = (cJSON*)malloc(sizeof(cJSON));
    Vector *eventlist = NULL;
    Vector_Create(&eventlist);
    Vector_PushBack(eventlist, (void*)strdup("event1"));
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
	Vector_Create(&data->RequestURIparamList);
	HTTPReqParam *httpreqparam = (HTTPReqParam *) malloc(sizeof(HTTPReqParam));
	httpreqparam->HttpRef = strdup("reportName");
        httpreqparam->HttpName = strdup("Profile.Name");
	httpreqparam->HttpValue = strdup("RDK_Profile");
	Vector_PushBack(data->RequestURIparamList, httpreqparam);
	CURL* curl = (CURL*) 0xffffffff;
	char* profile = "RDK_Profile";
	EXPECT_CALL(*m_reportgenMock, curl_easy_init())
		.Times(1)
		.WillOnce(Return(curl));
        EXPECT_CALL(*m_reportgenMock, curl_easy_escape(_,_,_))
		.Times(1)
		.WillOnce(Return(profile));
	EXPECT_CALL(*m_reportgenMock, curl_free(_))
		.Times(1);
	EXPECT_CALL(*m_reportgenMock, curl_easy_cleanup(_))
	        .Times(1);

	EXPECT_STREQ("https://mockxconf:50051/dataLakeMock/?Profile.Name=RDK_Profile", prepareHttpUrl(data));
	free(data->URL);
	free(httpreqparam->HttpRef);
	free(httpreqparam->HttpName);
	free(httpreqparam->HttpValue);
	Vector_Destroy(data->RequestURIparamList, free);
	free(data);
}

TEST_F(reportgenTestFixture, PrepareJSONReport1)
{
      cJSON* jsonobj = (cJSON*)malloc(sizeof(cJSON));
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

TEST_F(reportgenTestFixture, PrepareJSONReport2)
{
      cJSON* jsonobj = (cJSON*)malloc(sizeof(cJSON));
      char* rb = NULL;
      EXPECT_CALL(*m_reportgenMock, cJSON_PrintUnformatted(_))
                .Times(1)
                .WillOnce(Return((char*)0xffffffff));
      EXPECT_EQ(T2ERROR_SUCCESS, prepareJSONReport(jsonobj, &rb));
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
      if(jsonobj != NULL)
      {
          free(jsonobj);
          jsonobj = NULL;
      }
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
      valArray = (cJSON*)malloc(sizeof(cJSON));
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

TEST_F(reportgenTestFixture, encodeStaticParamsInJSON2)
{
      Vector* staticParamList = NULL;
      Vector_Create(&staticParamList);
      StaticParam *sparam = (StaticParam *) malloc(sizeof(StaticParam));
      sparam->paramType = strdup("datamodel");
      sparam->name = strdup("test1");
      sparam->value = strdup("value1");
      Vector_PushBack(staticParamList, sparam);
      cJSON *valArray = NULL;
      valArray = (cJSON*)malloc(sizeof(cJSON));
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

TEST_F(reportgenTestFixture, encodeStaticParamsInJSON3)
{
      Vector* staticParamList = NULL;
      Vector_Create(&staticParamList);
      StaticParam *sparam = (StaticParam *) malloc(sizeof(StaticParam));
      sparam->paramType = strdup("datamodel");
      sparam->name = strdup("test1");
      sparam->value = strdup("value1");
      Vector_PushBack(staticParamList, sparam);
      cJSON *valArray = NULL;
      valArray = (cJSON*)malloc(sizeof(cJSON));
      cJSON* mockobj = (cJSON *)0xffffffff;
      EXPECT_CALL(*m_reportgenMock,  cJSON_CreateObject())
                 .Times(1)
                 .WillOnce(Return(mockobj));
      EXPECT_CALL(*m_reportgenMock, cJSON_AddStringToObject(_,_,_))
               .Times(1)
               .WillOnce(Return(mockobj));
      EXPECT_CALL(*m_reportgenMock, cJSON_AddItemToArray(_, _))
               .Times(1)
               .WillOnce(Return(true));
      EXPECT_EQ(T2ERROR_SUCCESS, encodeStaticParamsInJSON(valArray, staticParamList));
      cJSON_Delete(valArray);
      if(valArray != NULL)
      {
          free(valArray);
          valArray = NULL;
      }
      Vector_Destroy(staticParamList, freeStaticParam);
}

//Testcases to be altered as per new prototype
TEST_F(reportgenTestFixture, encodeGrepResultInJSON1)
{
     Vector* grepResult = NULL;
     Vector_Create(&grepResult);
      GrepMarker* gparam = (GrepMarker *) malloc(sizeof(GrepMarker));
      memset(gparam, 0, sizeof(GrepMarker));
      gparam->markerName = strdup("TEST_MARKER1");
      gparam->u.markerValue = strdup("TEST_STRING1");
      gparam->mType = MTYPE_ABSOLUTE;
      gparam->trimParam = false;
      gparam->regexParam = NULL;
      Vector_PushBack(grepResult, gparam);
      cJSON *valArray = NULL;
      valArray = (cJSON*)malloc(sizeof(cJSON));
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

TEST_F(reportgenTestFixture, encodeGrepResultInJSON2)
{
      Vector* grepResult = NULL;
       Vector_Create(&grepResult);
      GrepMarker* gparam = (GrepMarker *) malloc(sizeof(GrepMarker));
      memset(gparam, 0, sizeof(GrepMarker));
      gparam->markerName = strdup("TEST_MARKER1");
      gparam->u.count = 5;
      gparam->mType = MTYPE_COUNTER;
      gparam->trimParam = false;
      gparam->regexParam = NULL;
      Vector_PushBack(grepResult, gparam);
      cJSON *valArray = NULL;
      valArray = (cJSON*)malloc(sizeof(cJSON));
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

TEST_F(reportgenTestFixture, encodeGrepResultInJSON3)
{
      Vector* grepResult = NULL;
       Vector_Create(&grepResult);
      GrepMarker* gparam = (GrepMarker *) malloc(sizeof(GrepMarker));
      memset(gparam, 0, sizeof(GrepMarker));
      gparam->markerName = strdup("TEST_MARKER1");
      //gparam->u.accumulatedValues ;
      gparam->mType = MTYPE_ACCUMULATE;
      gparam->trimParam = false;
      gparam->regexParam = NULL;
      //gparam->trimParam = true;
      //gparam->regexParam = "[A-Z]+";
      Vector_PushBack(grepResult, gparam);
      cJSON *valArray = NULL;
      valArray = (cJSON*)malloc(sizeof(cJSON));
      cJSON* mockobj = (cJSON *)0xffffffff;
      /*EXPECT_CALL(*m_reportgenMock,  cJSON_CreateObject())
               .Times(1)
                 .WillOnce(Return(mockobj));
      EXPECT_CALL(*m_reportgenMock, regcomp(_,_,_))
               .Times(1)
               .WillOnce(Return(-1));
      EXPECT_CALL(*m_reportgenMock, regexec(_,_,_,_,_))
               .Times(1)
               .WillOnce(Return(0));
      EXPECT_CALL(*m_reportgenMock, regfree(_))
               .Times(1);
      EXPECT_CALL(*m_reportgenMock, cJSON_AddStringToObject(_,_,_))
               .Times(1)
               .WillOnce(Return(mockobj));
      EXPECT_CALL(*m_reportgenMock, cJSON_AddItemToArray(_, _))
               .Times(1)
               .WillOnce(Return(true));*/
      EXPECT_EQ(T2ERROR_SUCCESS, encodeGrepResultInJSON(valArray, grepResult));
      cJSON_Delete(valArray);
      if(valArray != NULL)
      {
          free(valArray);
          valArray = NULL;
      }
      Vector_Destroy(grepResult, freeGResult);
}

TEST_F(reportgenTestFixture, encodeGrepResultInJSON4)
{
    // Case 1: both NULL
    EXPECT_EQ(T2ERROR_INVALID_ARGS, encodeGrepResultInJSON(NULL, NULL));

    // Case 2: valArray is NULL, grepMarkerList non-NULL
    Vector* grepMarkerList = nullptr;
    ASSERT_EQ(Vector_Create(&grepMarkerList), T2ERROR_SUCCESS);
    EXPECT_EQ(T2ERROR_INVALID_ARGS, encodeGrepResultInJSON(NULL, grepMarkerList));

    // Case 3: valArray non-NULL, grepMarkerList is NULL
    cJSON* valArray = cJSON_CreateArray();
    EXPECT_EQ(T2ERROR_INVALID_ARGS, encodeGrepResultInJSON(valArray, NULL));

    // cleanup
    Vector_Destroy(grepMarkerList, free); // assuming it only contains malloc'd pointers
    cJSON_Delete(valArray);
}
TEST_F(reportgenTestFixture, encodeGrepResultInJSON_CreateObjectFails)
{
    Vector* grepList = nullptr;
    ASSERT_EQ(Vector_Create(&grepList), T2ERROR_SUCCESS);

    GrepMarker* marker = (GrepMarker*)malloc(sizeof(GrepMarker));
    memset(marker, 0, sizeof(GrepMarker));
    marker->mType = MTYPE_ACCUMULATE;
    Vector_Create(&marker->u.accumulatedValues);
    Vector_PushBack(marker->u.accumulatedValues, strdup("ANYVAL"));
    Vector_PushBack(grepList, marker);

    cJSON* valArray = (cJSON*)0xdeadbeef; // any fake handle

    // Failure: CreateObject returns NULL triggers return at 507
    EXPECT_CALL(*m_reportgenMock, cJSON_CreateObject())
        .Times(1)
        .WillOnce(::testing::Return(nullptr));

    EXPECT_EQ(T2ERROR_FAILURE, encodeGrepResultInJSON(valArray, grepList));

    free(Vector_At(marker->u.accumulatedValues, 0));
    Vector_Destroy(marker->u.accumulatedValues, nullptr);
    free(marker);
    Vector_Destroy(grepList, nullptr);
}

TEST_F(reportgenTestFixture, encodeGrepResultInJSON_CreateArrayFails)
{
    Vector* grepList = nullptr;
    ASSERT_EQ(Vector_Create(&grepList), T2ERROR_SUCCESS);

    GrepMarker* marker = (GrepMarker*)malloc(sizeof(GrepMarker));
    memset(marker, 0, sizeof(GrepMarker));
    marker->mType = MTYPE_ACCUMULATE;
    Vector_Create(&marker->u.accumulatedValues);
    Vector_PushBack(marker->u.accumulatedValues, strdup("ANYVAL"));
    Vector_PushBack(grepList, marker);

    cJSON* valArray = (cJSON*)0xdeadbeef;
    cJSON* mockArrayObj = (cJSON*)0x1234;

    EXPECT_CALL(*m_reportgenMock, cJSON_CreateObject())
        .Times(1)
        .WillOnce(::testing::Return(mockArrayObj));
    EXPECT_CALL(*m_reportgenMock, cJSON_CreateArray())
        .Times(1)
        .WillOnce(::testing::Return(nullptr));
    EXPECT_CALL(*m_reportgenMock, cJSON_Delete(mockArrayObj))
        .Times(1);

    EXPECT_EQ(T2ERROR_FAILURE, encodeGrepResultInJSON(valArray, grepList));

    free(Vector_At(marker->u.accumulatedValues, 0));
    Vector_Destroy(marker->u.accumulatedValues, nullptr);
    free(marker);
    Vector_Destroy(grepList, nullptr);
}
TEST_F(reportgenTestFixture, encodeGrepResultInJSON_markerValueBranch_success)
{
    // Setup grepMarker
    Vector* grepList = nullptr;
    ASSERT_EQ(Vector_Create(&grepList), T2ERROR_SUCCESS);

    GrepMarker* marker = (GrepMarker*)malloc(sizeof(GrepMarker));
    memset(marker, 0, sizeof(GrepMarker));
    marker->markerName = strdup("TEST_GREP");
    marker->mType = MTYPE_ABSOLUTE;
    marker->trimParam = true;             // will hit trimLeadingAndTrailingws
    marker->regexParam = strdup("[A-Z]+"); // will hit applyRegexToValue

    marker->u.markerValue = strdup(" foo "); // leading/trailing ws

    Vector_PushBack(grepList, marker);

    cJSON* valArray = (cJSON*)malloc(sizeof(cJSON));
    cJSON* mockArrayObj = (cJSON*)0x1234;

    EXPECT_CALL(*m_reportgenMock, cJSON_CreateObject())
        .Times(1)
        .WillOnce(Return(mockArrayObj));
        // cJSON_AddStringToObject called and succeed
    EXPECT_CALL(*m_reportgenMock, cJSON_AddStringToObject(mockArrayObj, StrEq("TEST_GREP"), _))
        .Times(1)
        .WillOnce(Return((cJSON*)0x5678));

    // Add to array
    EXPECT_CALL(*m_reportgenMock, cJSON_AddItemToArray(valArray, mockArrayObj))
        .Times(1)
        .WillOnce(Return(true));

    // Do not set EXPECT_CALL for applyRegexToValue since it is not called in this code path!

    EXPECT_EQ(T2ERROR_SUCCESS, encodeGrepResultInJSON(valArray, grepList));

     // Clean up
     free(marker->markerName);
     free(marker->regexParam);
     free(marker);
     Vector_Destroy(grepList, nullptr);
     cJSON_Delete(valArray);
     if(valArray) free(valArray);
}
#if 0
TEST_F(reportgenTestFixture, encodeGrepResultInJSON_RegcompFails)
{
    Vector* grepList = nullptr;
    ASSERT_EQ(Vector_Create(&grepList), T2ERROR_SUCCESS);

    GrepMarker* marker = (GrepMarker*)malloc(sizeof(GrepMarker));
    memset(marker, 0, sizeof(GrepMarker));
    marker->mType = MTYPE_ACCUMULATE;
    marker->regexParam = strdup("invalid[");
    Vector_Create(&marker->u.accumulatedValues);
    Vector_PushBack(marker->u.accumulatedValues, strdup("value"));
    Vector_PushBack(grepList, marker);

    cJSON* valArray = (cJSON*)0xdeadbeef;
    cJSON* mockArrayObj = (cJSON*)0x1234;
    cJSON* mockVectorArray = (cJSON*)0x4321;

    EXPECT_CALL(*m_reportgenMock, cJSON_CreateObject())
        .Times(1)
        .WillOnce(::testing::Return(mockArrayObj));
    EXPECT_CALL(*m_reportgenMock, cJSON_CreateArray())
        .Times(1)
        .WillOnce(::testing::Return(mockVectorArray));
    EXPECT_CALL(*m_reportgenMock, regcomp(_, _, _))
        .Times(1)
        .WillOnce(Return(1));
    // Let any cJSON_AddItemToArray go, regardless of parameters
    EXPECT_CALL(*m_reportgenMock, cJSON_AddItemToArray(_, _))
        .Times(::testing::AnyNumber())
        .WillRepeatedly(Return(true));
    // Expect cJSON_AddItemToObject to add vectorArray to arrayObj
    EXPECT_CALL(*m_reportgenMock, cJSON_AddItemToObject(mockArrayObj, _, mockVectorArray))
        .Times(1)
        .WillOnce(Return(true));
        EXPECT_EQ(T2ERROR_SUCCESS, encodeGrepResultInJSON(valArray, grepList));

    free(Vector_At(marker->u.accumulatedValues, 0));
    Vector_Destroy(marker->u.accumulatedValues, nullptr);
    free(marker->regexParam);
    free(marker);
    Vector_Destroy(grepList, nullptr);
}
#endif
//When ParamValueCount is 0
TEST_F(reportgenTestFixture, encodeParamResultInJSON)
{
    Vector *paramNameList = NULL;
    Vector *paramValueList = NULL;
    Vector_Create(&paramNameList);
    Vector_Create(&paramValueList);
    cJSON *valArray = NULL;
    valArray = (cJSON*)malloc(sizeof(cJSON));
    Param* param = (Param *) malloc(sizeof(Param));
    param->reportEmptyParam = true;
    param->paramType = strdup("event");
    param->name = strdup("Event1");
    param->alias = strdup("EventMarker1");
    param->trimParam = true;
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

//When paramValcount is 1
TEST_F(reportgenTestFixture, encodeParamResultInJSON1)
{
    Vector *paramNameList = NULL;
    Vector *paramValueList = NULL;
    Vector_Create(&paramNameList);
    Vector_Create(&paramValueList);
    cJSON* valArray = NULL;
    valArray = (cJSON*)malloc(sizeof(cJSON));
    Param* param = (Param *) malloc(sizeof(Param));
    param->reportEmptyParam = true;
    param->paramType = strdup("event");
    param->name = strdup("Event1");
    param->alias = strdup("EventMarker1");
    param->trimParam = true;
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

TEST_F(reportgenTestFixture, encodeParamResultInJSON2)
{
    Vector *paramNameList = NULL;
    Vector *paramValueList = NULL;
    Vector_Create(&paramNameList);
    Vector_Create(&paramValueList);
     cJSON* valArray = NULL;
    valArray = (cJSON*)malloc(sizeof(cJSON));
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

TEST_F(reportgenTestFixture, encodeParamResultInJSON3)
{
    Vector *paramNameList = NULL;
    Vector *paramValueList = NULL;
    Vector_Create(&paramNameList);
    Vector_Create(&paramValueList);
     cJSON* valArray = NULL;
    valArray = (cJSON*)malloc(sizeof(cJSON));
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

TEST_F(reportgenTestFixture, encodeParamResultInJSON4)
{
    Vector *paramNameList = NULL;
    Vector *paramValueList = NULL;
    Vector_Create(&paramNameList);
    Vector_Create(&paramValueList);
    cJSON* valArray = NULL;
    valArray = (cJSON*)malloc(sizeof(cJSON));
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

TEST_F(reportgenTestFixture, encodeParamResultInJSON5)
{
    Vector *paramNameList = NULL;
    Vector *paramValueList = NULL;
    Vector_Create(&paramNameList);
    Vector_Create(&paramValueList);
     cJSON* valArray = NULL;
    valArray = (cJSON*)malloc(sizeof(cJSON));
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

TEST_F(reportgenTestFixture, encodeParamResultInJSON6)
{
    Vector *paramNameList = NULL;
    Vector *paramValueList = NULL;
    Vector_Create(&paramNameList);
    Vector_Create(&paramValueList);
    cJSON* valArray = NULL;
    valArray = (cJSON*)malloc(sizeof(cJSON));
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

TEST_F(reportgenTestFixture, encodeParamResultInJSON7)
{
    Vector *paramNameList = NULL;
    Vector *paramValueList = NULL;
    Vector_Create(&paramNameList);
    Vector_Create(&paramValueList);
    cJSON *valArray = NULL;
    valArray = (cJSON*)malloc(sizeof(cJSON));
    Param* param = (Param *) malloc(sizeof(Param));
    param->reportEmptyParam = true;
    param->paramType = strdup("event");
    param->name = strdup("Event1");
    param->alias = strdup("EventMarker1");
    param->trimParam = true;
    param->regexParam = strdup("[a-z]+");
    Vector_PushBack(paramNameList, param);
    profileValues *profVals = (profileValues *) malloc(sizeof(profileValues));
    profVals->paramValues = (tr181ValStruct_t**) malloc(sizeof(tr181ValStruct_t*));
    profVals->paramValues[0] = (tr181ValStruct_t*) malloc(sizeof(tr181ValStruct_t));
    profVals->paramValues[0]->parameterName = strdup("Event1");
    profVals->paramValues[0]->parameterValue = strdup("NULL");
    profVals->paramValueCount = 0;
    Vector_PushBack(paramValueList, profVals);
    cJSON* mockobj = (cJSON*)0xffffffff;
    EXPECT_CALL(*m_reportgenMock, cJSON_CreateObject())
                 .Times(1)
                 .WillOnce(Return(mockobj));
    EXPECT_CALL(*m_reportgenMock, cJSON_AddStringToObject(_,_,_))
	         .Times(1)
		 .WillOnce(Return(mockobj));
    EXPECT_CALL(*m_reportgenMock, cJSON_AddItemToArray(_,_))
	         .Times(1)
		 .WillOnce(Return(true));
    EXPECT_EQ(T2ERROR_SUCCESS, encodeParamResultInJSON(valArray,paramNameList,paramValueList));
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
    valArray = (cJSON*)malloc(sizeof(cJSON));
    Param* param = (Param *) malloc(sizeof(Param));
    param->reportEmptyParam = true;
    param->paramType = strdup("event");
    param->name = strdup("Event1");
    param->alias = strdup("EventMarker1");
    param->trimParam = true;
    param->regexParam = strdup("[a+");
    Vector_PushBack(paramNameList, param);
    profileValues *profVals = (profileValues *) malloc(sizeof(profileValues));
    profVals->paramValues = (tr181ValStruct_t**) malloc(sizeof(tr181ValStruct_t*));
    profVals->paramValues[0] = (tr181ValStruct_t*) malloc(sizeof(tr181ValStruct_t));
    profVals->paramValues[0]->parameterName = strdup("Event1");
    profVals->paramValues[0]->parameterValue = strdup("EventMarker1");
    profVals->paramValueCount = 1;
    Vector_PushBack(paramValueList, profVals);
    cJSON* mockobj = (cJSON*)0xffffffff;
    EXPECT_CALL(*m_reportgenMock, cJSON_CreateObject())
                 .Times(1)
                 .WillOnce(Return(mockobj));
    EXPECT_CALL(*m_reportgenMock, regcomp(_,_,_))
	         .Times(1)
		 .WillOnce(Return(-1));
    EXPECT_CALL(*m_reportgenMock, cJSON_AddStringToObject(_,_,_))
                 .Times(1)
                 .WillOnce(Return(mockobj));
    EXPECT_CALL(*m_reportgenMock, cJSON_AddItemToArray(_,_))
                 .Times(1)
                 .WillOnce(Return(true));
    EXPECT_EQ(T2ERROR_SUCCESS, encodeParamResultInJSON(valArray,paramNameList,paramValueList));
    cJSON_Delete(valArray);
    if(valArray != NULL)
    {
        free(valArray);
        valArray = NULL;
    }
    Vector_Destroy(paramNameList, freeParam);
    Vector_Destroy(paramValueList, freeProfileValues);
}
/*
TEST_F(reportgenTestFixture, encodeParamResultInJSON10)
{
    Vector *paramNameList = NULL;
    Vector *paramValueList = NULL;
    Vector_Create(&paramNameList);
    Vector_Create(&paramValueList);
    cJSON* valArray = NULL;
    valArray = (cJSON*)malloc(sizeof(cJSON));
    Param* param = (Param *) malloc(sizeof(Param));
    param->reportEmptyParam = true;
    param->paramType = strdup("event");
    param->name = strdup("Event1");
    param->alias = strdup("EventMarker1");
    param->trimParam = true;
    param->regexParam = strdup("[A-Z]+");
    Vector_PushBack(paramNameList, param);
    Param* param1 = (Param *) malloc(sizeof(Param));
    param1->reportEmptyParam = true;
    param1->paramType = strdup("event");
    param1->name = strdup("Event2");
    param1->alias = strdup("EventMarker2");
    param1->trimParam = true;
    param1->regexParam = strdup("[A-Z]+");
    Vector_PushBack(paramNameList, param1);
    Param* param2 = (Param *) malloc(sizeof(Param));
    param2->reportEmptyParam = true;
    param2->paramType = strdup("event");
    param2->name = strdup("Event3");
    param2->alias = strdup("EventMarker3");
    param2->trimParam = true;
    param2->regexParam = strdup("[A-Z]+");
    Vector_PushBack(paramNameList, param2);
    profileValues *profVals = (profileValues *) malloc(sizeof(profileValues));
    profVals->paramValues = (tr181ValStruct_t**) malloc(3 * sizeof(tr181ValStruct_t*));
    profVals->paramValues[0] = (tr181ValStruct_t*) malloc(sizeof(tr181ValStruct_t));
    profVals->paramValues[0]->parameterName = strdup("Event1");
    profVals->paramValues[0]->parameterValue = strdup("Event_Marker1");
    profVals->paramValues[1] = (tr181ValStruct_t*) malloc(sizeof(tr181ValStruct_t));
    profVals->paramValues[1]->parameterName = strdup("Event2");
    profVals->paramValues[1]->parameterValue = strdup("Event_Marker2");
    profVals->paramValues[2] = (tr181ValStruct_t*) malloc(sizeof(tr181ValStruct_t));
    profVals->paramValues[2]->parameterName = strdup("Event3");
    profVals->paramValues[2]->parameterValue = strdup("Event_Marker3"); 
    profVals->paramValueCount = 2;
    Vector_PushBack(paramValueList, profVals);
    cJSON* mockobj = (cJSON*)0xffffffff;
    EXPECT_CALL(*m_reportgenMock, cJSON_CreateObject())
                 .Times(3)
                 .WillOnce(Return(mockobj))
		 .WillOnce(Return(mockobj))
		 .WillOnce(Return(mockobj));
    EXPECT_CALL(*m_reportgenMock, cJSON_AddItemToObject)
	         .Times(1)
		 .WillOnce(Return(true));
    EXPECT_CALL(*m_reportgenMock, cJSON_CreateArray())
                 .Times(1)
                 .WillOnce(Return(mockobj));
    EXPECT_CALL(*m_reportgenMock, regcomp(_,_,_))
                 .Times(2)
                 .WillOnce(Return(-1))
		 .WillOnce(Return(-1));
    EXPECT_CALL(*m_reportgenMock, cJSON_AddStringToObject(_,_,_))
	         .Times(2)
		 .WillOnce(Return(mockobj))
		 .WillOnce(Return(mockobj));

    EXPECT_CALL(*m_reportgenMock, cJSON_AddItemToArray(_,_))
	         .Times(3)
		 .WillOnce(Return(true))
		 .WillOnce(Return(true))
		 .WillOnce(Return(true));
    EXPECT_EQ(T2ERROR_SUCCESS, encodeParamResultInJSON(valArray,paramNameList,paramValueList));
    cJSON_Delete(valArray);
    if(valArray != NULL)
    {
        free(valArray);
        valArray = NULL;
    }
    Vector_Destroy(paramNameList, freeParam);
    Vector_Destroy(paramValueList, freeProfileValues);
}
*/
TEST_F(reportgenTestFixture,  encodeEventMarkersInJSON)
{
     cJSON* valArray = NULL;
    valArray = (cJSON*)malloc(sizeof(cJSON));
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
    valArray = (cJSON*)malloc(sizeof(cJSON));
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
    cJSON *valArray = (cJSON *)malloc(sizeof(cJSON));
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

TEST_F(reportgenTestFixture,  encodeEventMarkersInJSON3)
{
     cJSON* valArray = NULL;
    valArray = (cJSON*)malloc(sizeof(cJSON));
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

TEST_F(reportgenTestFixture,  encodeEventMarkersInJSON4)
{
     cJSON* valArray = NULL;
    valArray = (cJSON*)malloc(sizeof(cJSON));
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

TEST_F(reportgenTestFixture,  encodeEventMarkersInJSON5)
{
     cJSON* valArray = NULL;
    valArray = (cJSON*)malloc(sizeof(cJSON));
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

TEST_F(reportgenTestFixture,  encodeEventMarkersInJSON6)
{
     cJSON* valArray = NULL;
    valArray = (cJSON*)malloc(sizeof(cJSON));
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

TEST_F(reportgenTestFixture,  encodeEventMarkersInJSON7)
{
     cJSON* valArray = NULL;
    valArray = (cJSON*)malloc(sizeof(cJSON));
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

TEST_F(reportgenTestFixture,  encodeEventMarkersInJSON8)
{
    cJSON *valArray = (cJSON *)malloc(sizeof(cJSON));
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
    eMarker->trimParam = true;
    eMarker->regexParam = strdup("[A-Z]+");
    Vector_PushBack(eventMarkerList, eMarker);
    cJSON* mockobj = (cJSON *) 0Xffffffff;
    EXPECT_CALL(*m_reportgenMock,  cJSON_CreateObject())
            .Times(1)
            .WillOnce(Return(mockobj));
    EXPECT_CALL(*m_reportgenMock, regcomp(_,_,_))
               .Times(1)
               .WillOnce(Return(-1));
    
    EXPECT_CALL(*m_reportgenMock, cJSON_AddStringToObject(_,_,_))
            .Times(2)
	    .WillOnce(Return(mockobj))
            .WillOnce(Return(mockobj));
    EXPECT_CALL(*m_reportgenMock, cJSON_AddItemToArray(_,_))
	    .Times(1)
	    .WillOnce(Return(true));
    EXPECT_EQ(T2ERROR_SUCCESS, encodeEventMarkersInJSON(valArray, eventMarkerList));
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
    valArray = (cJSON*)malloc(sizeof(cJSON));
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
    eMarker->trimParam = true;
    eMarker->regexParam = strdup("[A-Z]+");
    Vector_PushBack(eventMarkerList, eMarker);
    cJSON* mockobj = (cJSON *) 0Xffffffff;
    EXPECT_CALL(*m_reportgenMock,  cJSON_CreateObject())
            .Times(1)
            .WillOnce(Return(mockobj));
    EXPECT_CALL(*m_reportgenMock, regcomp(_,_,_))
               .Times(1)
               .WillOnce(Return(-1));

    EXPECT_CALL(*m_reportgenMock, cJSON_AddStringToObject(_,_,_))
            .Times(2)
            .WillOnce(Return(mockobj))
            .WillOnce(Return(mockobj));
    EXPECT_CALL(*m_reportgenMock, cJSON_AddItemToArray(_,_))
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_EQ(T2ERROR_SUCCESS, encodeEventMarkersInJSON(valArray, eventMarkerList));
    cJSON_Delete(valArray);
    if(valArray != NULL)
    {
        free(valArray);
        valArray = NULL;
    }
    Vector_Destroy(eventMarkerList, freeEMarker);
}

TEST_F(reportgenTestFixture,  encodeEventMarkersInJSON10)
{
     cJSON* valArray = NULL;
    valArray = (cJSON*)malloc(sizeof(cJSON));
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
    eMarker->trimParam = true;
    eMarker->regexParam = strdup("[A-Z]+");
    eMarker->reportTimestampParam = REPORTTIMESTAMP_UNIXEPOCH;
    Vector_Create(&eMarker->u.accumulatedValues);
    Vector_Create(&eMarker->accumulatedTimestamp);
    Vector_PushBack(eMarker->u.accumulatedValues, strdup("eventmissing"));
    Vector_PushBack(eMarker->u.accumulatedValues, strdup("eventfound"));
    Vector_PushBack(eMarker->accumulatedTimestamp, strdup("1678613787878"));
    Vector_PushBack(eMarker->accumulatedTimestamp, strdup("167861378164"));
    Vector_PushBack(eventMarkerList, eMarker);
    cJSON* mockobj = (cJSON*) 0Xfffffff;
    EXPECT_CALL(*m_reportgenMock,  cJSON_CreateObject())
            .Times(1)
            .WillOnce(Return(mockobj));
    EXPECT_CALL(*m_reportgenMock, cJSON_CreateArray())
	    .Times(2)
	    .WillOnce(Return(mockobj))
	    .WillOnce(Return(mockobj));
    EXPECT_CALL(*m_reportgenMock, regcomp(_,_,_))
               .Times(1)
               .WillOnce(Return(-1));
    EXPECT_CALL(*m_reportgenMock, cJSON_AddItemToArray(_,_))
	    .Times(5)
            .WillOnce(Return(true))
	    .WillOnce(Return(true))
	    .WillOnce(Return(true))
	    .WillOnce(Return(true))
	    .WillOnce(Return(true));
    EXPECT_CALL(*m_reportgenMock, cJSON_CreateString(_))
	    .Times(4)
	    .WillOnce(Return((cJSON*)0xffffffff))
	    .WillOnce(Return((cJSON*)0xffffffff))
	    .WillOnce(Return((cJSON*)0xffffffff))
	    .WillOnce(Return((cJSON*)0xffffffff));
    EXPECT_CALL(*m_reportgenMock, cJSON_AddItemToObject(_,_,_))
	    .Times(2)
	    .WillOnce(Return(true))
	    .WillOnce(Return(true));
    EXPECT_CALL(*m_reportgenMock, cJSON_Print(_))
	    .Times(1)
	    .WillOnce(::testing::ReturnNull());

    EXPECT_EQ(T2ERROR_SUCCESS, encodeEventMarkersInJSON(valArray, eventMarkerList));
    cJSON_Delete(valArray);
    if(valArray != NULL)
    {
        free(valArray);
        valArray = NULL;
    }
    Vector_Destroy(eventMarkerList, freeEMarker);
}

#ifdef GTEST_ENABLE
extern "C" {
typedef bool (*checkForEmptyStringFunc)(char *);
checkForEmptyStringFunc checkForEmptyStringCallback(void);
}

TEST(CheckForEmptyString, AllBranchesAreCovered)
{
    checkForEmptyStringFunc cb = checkForEmptyStringCallback();

    // 1. Null pointer: triggers else at line 48
    EXPECT_TRUE(cb(NULL));

    // 2. Empty string: triggers strlen < 1 at line 42
    char empty[] = "";
    EXPECT_TRUE(cb(empty));

    // 3. Single space: triggers !strncmp(valueString, " ", 1) at line 42
    char space[] = " ";
    EXPECT_TRUE(cb(space));

    // 4. "NULL" string: triggers !strncmp(valueString, "NULL", 4) at line 42
    char nulllit[] = "NULL";
    EXPECT_TRUE(cb(nulllit));

    // 5. Non-empty, not space, not NULL: should return false
    char nonempty[] = "VALUE";
    EXPECT_FALSE(cb(nonempty));

    // 6. "NUL" (should NOT match "NULL") - check that only "NULL" triggers true
    char almostnull[] = "NUL";
    EXPECT_FALSE(cb(almostnull));
}
#endif
