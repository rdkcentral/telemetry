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

#include <stdio.h>
#include <stdlib.h>

#include "rdk_linkedlist.h"

static void insertNode(rdkList_t* new, rdkList_t* tail)
{
    new->m_pForward = NULL;
    new->m_pBackward = NULL;
    if (tail)
    {
        tail->m_pForward = new;
        new->m_pBackward = tail;
    }
}

static void removeNode(rdkList_t* node)
{
    if(node->m_pForward == NULL && node->m_pBackward == NULL)
    {
        return;
    }
    else if(node->m_pForward == NULL)
    {
        node->m_pBackward->m_pForward = NULL;
    }
    else if(node->m_pBackward == NULL)
    {
        node->m_pForward->m_pBackward = NULL;
    }
    else
    {
        node->m_pBackward->m_pForward  = node->m_pForward;
        node->m_pForward->m_pBackward = node->m_pBackward;
    }
    node->m_pForward = NULL;
    node->m_pBackward = NULL;
}

rdkList_t* rdk_list_add_node (rdkList_t* rdkListHndl, void* pData)
{
    rdkList_t* tmpHndl;
    tmpHndl = (rdkList_t*) malloc (sizeof(rdkList_t));
    rdkList_t* tail = NULL;
    if (!tmpHndl)
    {
        printf ("Failed to allocate memory\n");
    }
    else
    {
        tmpHndl->m_pUserData = pData;
    }

    if (rdkListHndl)
    {
        tail = rdkListHndl;
        while (tail->m_pForward)
        {
            tail = tail->m_pForward;
        }
    }

    /* Add to List */
    insertNode(tmpHndl, tail);

    if (!rdkListHndl)
    {
        rdkListHndl = tmpHndl;
    }

    return rdkListHndl;
}

rdkList_t* rdk_list_prepend_node (rdkList_t* rdkListHndl, void* pData)
{
    rdkList_t* tmpHndl;
    tmpHndl = (rdkList_t*) malloc (sizeof(rdkList_t));
    if (!tmpHndl)
    {
        printf ("Failed to allocate memory\n");
        return rdkListHndl;
    }
    else
    {
        tmpHndl->m_pUserData = pData;
        tmpHndl->m_pForward = NULL;
        tmpHndl->m_pBackward = NULL;
    }

    if (rdkListHndl)
    {
        tmpHndl->m_pForward = rdkListHndl;
        tmpHndl->m_pBackward = rdkListHndl->m_pBackward;
        if (rdkListHndl->m_pBackward)
        {
            rdkListHndl->m_pBackward->m_pForward = tmpHndl;
        }
        rdkListHndl->m_pBackward = tmpHndl;
    }

    return tmpHndl;
}

rdkList_t* rdk_list_add_node_before(rdkList_t* rdkListHndl, rdkList_t* sibling, void* pData)
{
    rdkList_t* tmpHndl;
    tmpHndl = (rdkList_t*) malloc (sizeof(rdkList_t));
    if (!tmpHndl)
    {
        printf ("Failed to allocate memory\n");
        return rdkListHndl;
    }
    else
    {
        tmpHndl->m_pUserData = pData;
        tmpHndl->m_pForward = NULL;
        tmpHndl->m_pBackward = NULL;
    }

    if (!rdkListHndl)
    {
        rdkListHndl = tmpHndl;
        if (NULL != sibling)
        {
            printf("rdk_list_add_node_before(): sibling not NULL for an empty list\n");
        }
    }
    else if (sibling)
    {
        tmpHndl->m_pForward = sibling;
        tmpHndl->m_pBackward = sibling->m_pBackward;
        if (sibling->m_pBackward)
        {
            sibling->m_pBackward->m_pForward = tmpHndl;
        }
        sibling->m_pBackward = tmpHndl;

        if (!tmpHndl->m_pBackward)
        {
            rdkListHndl = tmpHndl;
        }
    }
    else
    {
        rdkList_t* tail = rdkListHndl;
        while (tail->m_pForward)
        {
            tail = tail->m_pForward;
        }
        insertNode(tmpHndl, tail);
    }

    return rdkListHndl;
}

void rdk_list_delete_node (rdkList_t* rdkListHndl)
{
    if (rdkListHndl)
    {
        removeNode(rdkListHndl);
        free (rdkListHndl);
    }
    else
    {
        printf("rdk_list_delete_node(): invalid pointer provided\n");
    }
}

rdkList_t* rdk_list_remove_node (rdkList_t* rdkListHndl, rdkList_t* node)
{
    if (!rdkListHndl)
    {
        return rdkListHndl;
    }

    removeNode(node);

    if (node == rdkListHndl)
    {
        rdkListHndl = rdkListHndl->m_pForward;
    }

    return rdkListHndl;
}

rdkList_t* rdk_list_remove (rdkList_t* rdkListHndl, const void *pData)
{
    rdkList_t* tmpHndl = rdkListHndl;

    while (tmpHndl)
    {
        if (tmpHndl->m_pUserData != pData)
        {
            tmpHndl = tmpHndl->m_pForward;
        }
        else
        {
            rdkListHndl = rdk_list_remove_node(rdkListHndl, tmpHndl);
            free(tmpHndl);
            break;
        }
    }

    return rdkListHndl;
}

rdkList_t* rdk_list_find_first_node (rdkList_t* rdkListHndl)
{
    if (rdkListHndl)
    {
        while (rdkListHndl->m_pBackward)
        {
            rdkListHndl = rdkListHndl->m_pBackward;
        }
    }
    return rdkListHndl;
}

rdkList_t* rdk_list_find_next_node (rdkList_t* rdkListHndl)
{
    if (rdkListHndl)
    {
        return rdkListHndl->m_pForward;
    }
    return rdkListHndl;
}

rdkList_t* rdk_list_find_previous_node (rdkList_t* rdkListHndl)
{
    if (rdkListHndl)
    {
        return rdkListHndl->m_pBackward;
    }
    return rdkListHndl;
}

rdkList_t* rdk_list_find_node_custom (rdkList_t* rdkListHndl, void* pData, fnRDKListCustomCompare compareFunction)
{
    if (rdkListHndl && compareFunction)
    {
        rdkList_t* tmpHndl = rdkListHndl;
        do
        {
            if (0 == compareFunction(tmpHndl->m_pUserData, pData))
            {
                return tmpHndl;
            }
            tmpHndl = tmpHndl->m_pBackward;
        }
        while(tmpHndl);
        rdkListHndl = rdkListHndl->m_pForward;
        while (rdkListHndl)
        {
            if (0 == compareFunction(rdkListHndl->m_pUserData, pData))
            {
                return rdkListHndl;
            }
            rdkListHndl = rdkListHndl->m_pForward;
        }
    }
    return NULL;
}

void rdk_list_free_all_nodes (rdkList_t* rdkListHndl)
{
    if (rdkListHndl)
    {
        rdkList_t* backTmpHndl = rdkListHndl;
        rdkListHndl = rdkListHndl->m_pForward;
        do
        {
            rdkList_t* tmpHndl = backTmpHndl->m_pBackward;
            removeNode(backTmpHndl);
            free(backTmpHndl);
            backTmpHndl = tmpHndl;

        }
        while (backTmpHndl);
        while (rdkListHndl)
        {
            rdkList_t* tmpHndl = rdkListHndl;
            rdkListHndl = rdkListHndl->m_pForward;
            removeNode(tmpHndl);
            free (tmpHndl);
        }
    }
}

void rdk_list_foreach (rdkList_t* rdkListHndl, fnRDKListCustomExecute executeFunction, void* pUserActionData)
{
    if (rdkListHndl && executeFunction)
    {
        rdkList_t* backTmpHndl = rdkListHndl;
        do
        {
            rdkList_t* tmpHndl = backTmpHndl->m_pBackward;
            executeFunction(backTmpHndl->m_pUserData, pUserActionData);
            backTmpHndl = tmpHndl;

        }
        while (backTmpHndl);
        rdkListHndl = rdkListHndl->m_pForward;
        while (rdkListHndl)
        {
            rdkList_t* tmpHndl = rdkListHndl->m_pForward;
            executeFunction(rdkListHndl->m_pUserData, pUserActionData);
            rdkListHndl = tmpHndl;
        }
    }
}

void rdk_list_free_all_nodes_custom (rdkList_t* rdkListHndl, fnRDKListCustomFree freeFunction)
{
    if (rdkListHndl && freeFunction)
    {
        rdkList_t* backTmpHndl = rdkListHndl;
        rdkListHndl = rdkListHndl->m_pForward;
        do
        {
            rdkList_t* tmpHndl = backTmpHndl->m_pBackward;
            freeFunction(backTmpHndl->m_pUserData);
            removeNode(backTmpHndl);
            free(backTmpHndl);
            backTmpHndl = tmpHndl;

        }
        while (backTmpHndl);
        while (rdkListHndl)
        {
            rdkList_t* tmpHndl = rdkListHndl->m_pForward;
            freeFunction(rdkListHndl->m_pUserData);
            removeNode(rdkListHndl);
            free (rdkListHndl);
            rdkListHndl = tmpHndl;
        }
    }
}

rdkList_t* rdk_list_reverse(rdkList_t* rdkListHndl)
{
    rdkList_t* prev = NULL;
    rdkList_t* curr = rdkListHndl;

    while (curr)
    {
        rdkList_t* next = curr->m_pForward;
        curr->m_pForward = prev;
        curr->m_pBackward = next;
        prev = curr;
        curr = next;
    }

    return prev;
}
