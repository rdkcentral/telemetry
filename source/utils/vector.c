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
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "vector.h"

#define RTVECT_BLOCKSIZE 16

T2ERROR
Vector_Create(Vector** v)
{
  (*v) = (Vector *) malloc(sizeof(Vector));
  if (!(*v))
    return T2ERROR_MEMALLOC_FAILED;

  memset((*v), 0 ,sizeof(Vector));
  (*v)->data = NULL;
  (*v)->capacity = 0;
  (*v)->count = 0;
  return T2ERROR_SUCCESS;
}

T2ERROR
Vector_Destroy(Vector *v, Vector_Cleanup destroyer)
{
  size_t i;

  if (!v || !destroyer)
    return T2ERROR_INVALID_ARGS;
  if (v->data)
  {
    if (destroyer)
    {
      for (i = 0; i < v->count; ++i){
	if(v->data[i] != NULL){      
           destroyer(v->data[i]);
	}
      }
    }
    free(v->data);
    v->data = NULL;
  }
  free(v);
  return T2ERROR_SUCCESS;
}

T2ERROR
Vector_Clear(Vector *v, Vector_Cleanup destroyer)
{ 
  if (!v || !destroyer)
    return T2ERROR_INVALID_ARGS;
  size_t i;
  if (v->data)
  {
      for (i = 0; i < v->count; ++i)
          destroyer(v->data[i]);
      free(v->data);
  }
  v->data = NULL;
  v->capacity = 0;
  v->count = 0;
  return T2ERROR_SUCCESS;
}

T2ERROR
Vector_PushBack(Vector *v, void* item)
{
  if ((!v) || item == NULL){
    return T2ERROR_INVALID_ARGS;
  }

  if (!v->data)
  {
    v->data = calloc(RTVECT_BLOCKSIZE, sizeof(void *));
    v->capacity = RTVECT_BLOCKSIZE;
  }
  else if (v->count + 1 >= v->capacity)
  {
    v->data = realloc(v->data, (v->capacity + RTVECT_BLOCKSIZE) * sizeof(void *));
    v->capacity += RTVECT_BLOCKSIZE;
  }

  if (!v->data)
    return T2ERROR_MEMALLOC_FAILED;

  v->data[v->count++] = item;
  return T2ERROR_SUCCESS;
}

T2ERROR
Vector_RemoveItem(Vector *v, void* item, Vector_Cleanup destroyer)
{
  if(!v || !item){
      return T2ERROR_INVALID_ARGS;
  }
  size_t i;
  for (i = 0; i < v->count; ++i)
  {
    if (v->data[i] == item)
    {
      if (destroyer)
        destroyer(v->data[i]);
      break;
    }
  }

  while (i < v->count)
  {
    v->data[i] = v->data[i+1];
    i++;
  }

  v->count -= 1;

  while (i < v->capacity)
  {
    v->data[i] = NULL;
    i++;
  }

  return T2ERROR_SUCCESS;
}

void*
Vector_At(Vector *v, size_t index)
{
  if (!v) return NULL;
  if (index >= v->count) return NULL;
  return v->data[index];
}

size_t
Vector_Size(Vector *v)
{
  if (!v) return 0;
  return v->count;
}


T2ERROR
Vector_Sort(Vector* v, size_t size, Vecor_Comparator comparator)
{
    if (!v || !comparator)
      return T2ERROR_INVALID_ARGS;

    if (comparator) {
        if (v->data) {
            if ( v->count > 1 )
                qsort(v->data, v->count, size, comparator);
        }
    }

    return T2ERROR_SUCCESS;
}
