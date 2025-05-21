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

#include <iostream>
#include "reportgenMock.h"
#include <curl/curl.h>
#include <cjson/cJSON.h>
using namespace std;

extern ReportgenMock *m_reportgenMock;

extern "C" cJSON* cJSON_CreateArray()
{
    if (!m_reportgenMock)
    {
         return NULL;
    }
    return m_reportgenMock->cJSON_CreateArray();
}

extern "C" cJSON* cJSON_CreateObject()
{
    if (!m_reportgenMock)
    {
         return NULL;
    }
    return m_reportgenMock->cJSON_CreateObject();
}

extern "C" cJSON* cJSON_Parse(const char* value)
{
    if(!m_reportgenMock)
    {
         return NULL;
    }
    return m_reportgenMock->cJSON_Parse(value);
}

extern "C" int cJSON_GetArraySize(const cJSON* array)
{
    if(!m_reportgenMock)
    {
         return 0;
    }
    return m_reportgenMock->cJSON_GetArraySize(array);
}

extern "C" cJSON* cJSON_GetArrayItem(const cJSON* array, int index)
{
    if(!m_reportgenMock)
    {
         return NULL;
    }
    return m_reportgenMock->cJSON_GetArrayItem(array, index);
}

extern "C" cJSON* cJSON_GetObjectItem(const cJSON * const array, const char* const value)
{
    if(!m_reportgenMock)
    {
         return NULL;
    }
    return m_reportgenMock->cJSON_GetObjectItem(array, value);
}

/*extern "C" cJSON_bool cJSON_AddItemToArray(cJSON *array, cJSON *item)
{
    if (!m_reportgenMock)
    {
         return false;
    }
    return m_reportgenMock->cJSON_AddItemToArray(array, item);
}

extern "C" cJSON_bool cJSON_AddItemToObject(cJSON *object, const char *string, cJSON *item)
{
    if (!m_reportgenMock)
    {
         return false;
    }
    return m_reportgenMock->cJSON_AddItemToObject(object, string, item);
}*/

extern "C" char* cJSON_PrintUnformatted(const cJSON *item)
{
    if (!m_reportgenMock)
    {
         return NULL;
    }
    return m_reportgenMock->cJSON_PrintUnformatted(item);
}

extern "C" cJSON*  cJSON_AddStringToObject(cJSON * const object, const char * const name, const char * const string)
{
    if (!m_reportgenMock)
    {
         return NULL;
    }
    return m_reportgenMock->cJSON_AddStringToObject(object, name, string);
}

extern "C" CURL* curl_easy_init()
{
        if(!m_reportgenMock)
        {
                return NULL;
        }
        return m_reportgenMock->curl_easy_init();
}

/*extern "C" char *curl_easy_escape( CURL *curl, const char *url, int length )
{
        if(!m_reportgenMock)
        {
                return NULL;
        }
        return m_reportgenMock->curl_easy_escape(curl, url, length);
}
*/
extern "C" void cJSON_Delete(cJSON * root)
{
        if(!m_reportgenMock)
        {
                return;
        }
        m_reportgenMock->cJSON_Delete(root);
}

extern "C" int regcomp(regex_t *preg, const char *pattern, int cflags)
{
        if(!m_reportgenMock)
        {
               return -1;
        }
        m_reportgenMock->regcomp(preg, pattern, cflags);
}

extern "C" int regexec(const regex_t *preg, const char *string, size_t nmatch, regmatch_t pmatch[], int eflags)
{
        if(!m_reportgenMock)
        {
               return -1;
        }
        m_reportgenMock->regexec(preg, string, nmatch, pmatch, eflags);
}

extern "C"  void regfree(regex_t *preg)
{
        if(!m_reportgenMock)
        {
               return;
        }
         m_reportgenMock->regfree(preg);
}

extern "C" T2ERROR getParameterValue(const char* paramName, char **paramValue)
{
	if(!m_reportgenMock)
        {
                return T2ERROR_FAILURE;
        }
        return m_reportgenMock->getParameterValue(paramName, paramValue);
}
