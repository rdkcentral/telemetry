/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
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

/**
 * @file rdk_linkedlist.h
 * The header file provides RDK Linked List Utils APIs.
 */

#ifndef _RDK_LINKEDLIST_H_
#define _RDK_LINKEDLIST_H_

typedef struct rdkList_t {
    struct rdkList_t* m_pForward;
    struct rdkList_t* m_pBackward;
    void* m_pUserData;
} rdkList_t;

typedef int (*fnRDKListCustomCompare) (const void* pSrcData, const void* pDstData);
typedef void (*fnRDKListCustomFree) (void* pUserData);
typedef void (*fnRDKListCustomExecute) (void* pUserData, void* pUserActionData);

rdkList_t* rdk_list_add_node (rdkList_t* rdkListHndl, void* pData);
rdkList_t* rdk_list_prepend_node (rdkList_t* rdkListHndl, void* pData);
rdkList_t* rdk_list_add_node_before(rdkList_t* rdkListHndl, rdkList_t* sibling, void* pData);

void rdk_list_delete_node (rdkList_t* rdkListHndl);
rdkList_t* rdk_list_remove_node (rdkList_t* rdkListHndl, rdkList_t* node);
rdkList_t* rdk_list_remove (rdkList_t* rdkListHndl, const void *pData);

rdkList_t* rdk_list_find_first_node (rdkList_t* rdkListHndl);
rdkList_t* rdk_list_find_next_node (rdkList_t* rdkListHndl);
rdkList_t* rdk_list_find_previous_node (rdkList_t* rdkListHndl);
rdkList_t* rdk_list_find_node_custom (rdkList_t* rdkListHndl, void* pData, fnRDKListCustomCompare compareFunction);

void rdk_list_foreach (rdkList_t* rdkListHndl, fnRDKListCustomExecute executeFunction, void* pUserActionData);

void rdk_list_free_all_nodes (rdkList_t* rdkListHndl);
void rdk_list_free_all_nodes_custom (rdkList_t* rdkListHndl, fnRDKListCustomFree freeFunction);

rdkList_t* rdk_list_reverse(rdkList_t* rdkListHndl);

#endif /* _RDK_LINKEDLIST_H_ */
