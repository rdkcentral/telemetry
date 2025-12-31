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

#ifndef REPORTGEN_MOCK
#define REPORTGEN_MOCK
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pthread.h>
#include <cjson/cJSON.h>
#include <curl/curl.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <regex.h>
#include <telemetry2_0.h>

class ReportgenInterface
{
public:
    virtual ~ReportgenInterface() {}
    virtual cJSON* cJSON_CreateArray() = 0;
    virtual cJSON* cJSON_CreateObject() = 0;
    virtual cJSON* cJSON_Parse(const char* value) = 0;
    virtual cJSON* cJSON_GetObjectItem(const cJSON * const object, const char * const string) = 0;
    virtual int cJSON_GetArraySize(const cJSON *array) = 0;
    virtual cJSON* cJSON_GetArrayItem(const cJSON *array, int index) = 0;
    virtual cJSON_bool cJSON_AddItemToArray(cJSON *array, cJSON *item) = 0;
    virtual cJSON*  cJSON_AddStringToObject(cJSON * const object, const char * const name, const char * const string) = 0;
    virtual cJSON_bool cJSON_AddItemToObject(cJSON *object, const char *string, cJSON *item) = 0;
    virtual char* cJSON_PrintUnformatted(const cJSON *item) = 0;
    virtual void cJSON_Delete(cJSON *root) = 0;
    virtual void cJSON_free(void* root) = 0;
    virtual cJSON* cJSON_CreateString(const char *string) = 0;
    virtual char* cJSON_Print(const cJSON *) = 0;
    virtual CURL* curl_easy_init() = 0;
    virtual int regcomp(regex_t *preg, const char *pattern, int cflags) = 0;
    virtual int regexec(const regex_t *preg, const char *string, size_t nmatch, regmatch_t pmatch[], int eflags) = 0;
    virtual void regfree(regex_t *preg) = 0;
    virtual char* curl_easy_escape(CURL *c, const char *string, int len) = 0;
    virtual void curl_free(void *ptr) = 0;
    virtual void curl_easy_cleanup(CURL *curl) = 0;
     virtual T2ERROR applyRegexToValue(char **value, const char *regex) = 0;
    virtual int cJSON_IsArray(const cJSON* arr) = 0;
    virtual cJSON_bool cJSON_InsertItemInArray(cJSON* array, int which, cJSON* item) = 0;
    virtual cJSON* cJSON_GetObjectItemCaseSensitive(const cJSON * const object, const char * const string) = 0;
};

class ReportgenMock: public ReportgenInterface
{
public:
    virtual ~ReportgenMock() {}
    MOCK_METHOD0(cJSON_CreateArray,  cJSON * ());
    MOCK_METHOD0(cJSON_CreateObject, cJSON * ());
    MOCK_METHOD1(cJSON_Parse, cJSON * (const char*));
    MOCK_METHOD1(cJSON_GetArraySize, int(const cJSON*));
    MOCK_METHOD2(cJSON_GetArrayItem, cJSON * (const cJSON *, int));
    MOCK_METHOD2(cJSON_GetObjectItem, cJSON * (const cJSON * const, const char * const));
    MOCK_METHOD2(cJSON_AddItemToArray, cJSON_bool(cJSON *, cJSON *));
    MOCK_METHOD3(cJSON_AddStringToObject, cJSON * (cJSON * const, const char * const, const char * const));
    MOCK_METHOD3(cJSON_AddItemToObject, cJSON_bool(cJSON *, const char *, cJSON *));
    MOCK_METHOD1(cJSON_PrintUnformatted, char*(const cJSON *));
    MOCK_METHOD1(cJSON_Delete, void(cJSON *));
    MOCK_METHOD1(cJSON_free, void(void*));
    MOCK_METHOD1(cJSON_CreateString, cJSON * (const char*));
    MOCK_METHOD0(curl_easy_init, CURL * ());
    MOCK_METHOD3(regcomp, int(regex_t*,  const char *, int));
    MOCK_METHOD5(regexec, int(const regex_t *,  const char *, size_t, regmatch_t*, int));
    MOCK_METHOD1(regfree, void(regex_t*));
    MOCK_METHOD1(cJSON_Print, char*(const cJSON *));
    //MOCK_METHOD3(curl_easy_escape, char*(CURL *, const char *, int));
    MOCK_METHOD2(getParameterValue, T2ERROR(const char*, char **));
    MOCK_METHOD3(curl_easy_escape, char*(CURL *, const char *, int));
    MOCK_METHOD1(curl_free, void(void *));
    MOCK_METHOD1(curl_easy_cleanup, void(CURL *));
        MOCK_METHOD2(applyRegexToValue, T2ERROR(char **, const char *));
    MOCK_METHOD1(cJSON_IsArray, int(const cJSON*));
    MOCK_METHOD3(cJSON_InsertItemInArray, cJSON_bool(cJSON*, int, cJSON*));
    MOCK_METHOD2(cJSON_GetObjectItemCaseSensitive, cJSON* (const cJSON * const, const char * const));
};

#endif


