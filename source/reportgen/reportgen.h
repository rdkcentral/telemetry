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

#ifndef _REPORTGEN_H_
#define _REPORTGEN_H_

#include <cjson/cJSON.h>

#include "vector.h"

typedef struct _HTTPReqParam
{
    char* HttpName;
    char* HttpRef;
    char* HttpValue;
} HTTPReqParam;

typedef struct _RBUSMethodParam
{
    char* name;
    char* value;
} RBUSMethodParam;

typedef enum
{
    HTTP_PUT,
    HTTP_POST
} HTTPMethod;

typedef enum
{
    COMP_NONE
} HTTPComp;

typedef struct _T2HTTP
{
    char *URL;
    HTTPComp Compression;
    HTTPMethod Method;
    Vector *RequestURIparamList;
} T2HTTP;


typedef struct _T2RBUS
{
    char *rbusMethodName;
    Vector *rbusMethodParamList;
} T2RBUS;

void freeProfileValues(void* data);

T2ERROR destroyJSONReport(cJSON *jsonObj);

bool isDataModelTable(const char *paramName);

T2ERROR encodeParamResultInJSON(cJSON *valArray, Vector *paramNameList, Vector *paramValueList, Vector *dataModelTableList);

T2ERROR encodeStaticParamsInJSON(cJSON *valArray, Vector *staticParamList);

T2ERROR encodeGrepResultInJSON(cJSON *valArray, Vector *grepMarkerList);

T2ERROR encodeTopResultInJSON(cJSON *valArray, Vector *topMarkerList);

T2ERROR encodeEventMarkersInJSON(cJSON *valArray, Vector *eventMarkerList);

T2ERROR prepareJSONReport(cJSON* jsonObj, char** reportBuff);

char *prepareHttpUrl(T2HTTP *http);

void tagReportAsCached(char **jsonReport);

#endif /* _REPORTGEN_H_ */
