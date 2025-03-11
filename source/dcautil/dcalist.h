/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2016 RDK Management
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



/**
 * @defgroup dca
 * @{
 **/




/**
 * @defgroup dca
 * @{
 * @defgroup src
 * @{
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "rdk_linkedlist.h"
#include "vector.h"

typedef enum {
  OCCURENCE,
  STR
} DType_t;

typedef struct pclist {
  char *header;
  char *pattern;
  DType_t d_type;
  union {
    int count;
    char *data;
  };
  bool trimparam;
  char* regexparam;
} pcdata_t;

extern rdkList_t *pchead;

int insertPCNode(rdkList_t **pch, char *pattern, char *header, DType_t dtype, int count, char *data, bool trim, char *regex);
pcdata_t* searchPCNode(rdkList_t *pch, char *pattern);
void printPCNodes(rdkList_t *pch);
void clearPCNodes(rdkList_t **pch);
int comparePattern(const void *np, const void *sp);
void print_pc_node(void *data, void *user_data);
int processTopPattern(rdkList_t *pchead, Vector* grepResultList);

/** @} */


/** @} */
/** @} */
/** @} */


/** @} */
/** @} */
