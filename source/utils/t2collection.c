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
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "t2collection.h"

queue_t *t2_queue_create(void)
{
	queue_t *q;
	
	q = (queue_t *)malloc(sizeof(queue_t));
	if (q == NULL) {
		return NULL;
	}

	memset(q, 0, sizeof(queue_t));
	return q;
}

int8_t t2_queue_push(queue_t *q, void *data)
{
	element_t *e, *tmp;
        if(data == NULL || q == NULL){
              return -1;
        }
	e = (element_t *)malloc(sizeof(element_t));
	if (e == NULL) {
		return -1;
	}

	memset(e, 0, sizeof(element_t));
	e->data = data;
	if (q->head == NULL) {
		q->head = e;
	} else {
		tmp = q->head;
		q->head = e;
		e->next = tmp;	
	}
	return 0;	
}

void *t2_queue_pop(queue_t *q)
{
    if(q == NULL)
    {
	 return NULL;
    }
    element_t *e, *tmp = NULL;
	void *data;

	e = q->head;
	if (e == NULL) {
		return NULL;
	}

    while (e->next != NULL) {
        tmp = e;
        e = e->next;
    }
		
	data = e->data;
    if (tmp != NULL) {
        tmp->next = NULL;
    } else {
        q->head = NULL;
    }
	free(e);
	return data;
}

void *t2_queue_remove(queue_t *q, uint32_t index)
{
	element_t	*e, *tmp = NULL;
	void *data;
	uint32_t i = 0;
    if(q == NULL)
	    return NULL;

    if (index > (t2_queue_count(q) - 1)) {
        return NULL;
    }

	e = q->head;
	if (e == NULL) {
		return NULL;
	}

	while (i < index) {
		tmp = e;
		e = e->next;	
		i++;	
	}

	if (tmp == NULL) {
		q->head = e->next;
	} else {
		tmp->next = e->next;
	}

	data = e->data;
	free(e);
	return data;
}

void    *t2_queue_peek(queue_t *q, uint32_t index)
{
	element_t	*e;
	uint32_t i = 0;
        if(q == NULL){
             return NULL;
        }
    
    if (index > (t2_queue_count(q) - 1)) {
        return NULL;
    }

	e = q->head;
	if (e == NULL) {
		return NULL;
	}

	while (i < index) {
		e = e->next;	
		i++;	
	}
	return e->data;

}

uint32_t t2_queue_count(queue_t *q)
{
        if(q == NULL){
             return 0;
	}
	uint32_t i = 0;
	element_t	*e;

	e = q->head;

	while (e != NULL) {
		i++;
		e = e->next;
	}
	return i;
}

void t2_queue_destroy(queue_t *q, queue_cleanup freeItem)
{
	element_t	*e, *tmp;
        if(q == NULL || freeItem == NULL)
             return;
	e = q->head;

	while (e != NULL) {
		tmp = e->next;
		if (e->data)
		    freeItem(e->data);
		free(e);
		e = tmp;
	}

	free(q);
}

void *hash_map_get(hash_map_t *map, const char *key)
{
	uint32_t count, i;
	hash_element_t *e = NULL;
	bool found = false;
        if ((map == NULL) || (key == NULL)) {
                return NULL;
        }
	count = t2_queue_count(map->queue);
	if (count == 0) {
		return NULL;
	}

	for (i = 0; i < count; i++) {
		e = t2_queue_peek(map->queue, i);
		if ((e != NULL) && (strncmp(e->key, key, MAX_KEY_LEN) == 0)) {
			found = true;
			break;
		}
	}

	if (found == false) {
		return NULL;
	}


	return e->data;
}

void *hash_map_remove(hash_map_t *map, const char *key)
{
    uint32_t count, i;
    hash_element_t *e = NULL, *tmp = NULL;
    bool found = false;
    void *data;

    if ((map == NULL) || (key == NULL)) {
        return NULL;
    }

    count = t2_queue_count(map->queue);
    if (count == 0) {
        return NULL;
    }

    for (i = 0; i < count; i++) {
        e = t2_queue_peek(map->queue, i);
        if ((e != NULL) && (strncmp(e->key, key, MAX_KEY_LEN) == 0)) {
            found = true;
            break;
        }
    }

    if (found == false) {
        return NULL;
    }

    tmp = t2_queue_remove(map->queue, i);
    assert(tmp == e);

    data = e->data;
    free(e->key);
    free(e);

    return data;
}

int8_t hash_map_put(hash_map_t *map, char *key, void *data, hashelement_data_cleanup freeItem) 
{
    hash_element_t *e;
  
    if ((map == NULL) || (key == NULL) || (data == NULL)) {
        return -1;
    }

    // Hash map should support only unique keys. If previous entry exists, replace it.
    if(hash_map_get(map,key)) {
        void* dataelement = hash_map_remove(map, key);
        if(dataelement != NULL && freeItem != NULL){
            freeItem(dataelement);
	    dataelement = NULL;
        }
    }
    e = (hash_element_t *)malloc(sizeof(hash_element_t));
    if (e == NULL) {
        return -1;
    }

    memset(e, 0, sizeof(hash_element_t));

    e->key = key;
    e->data = data;
    return t2_queue_push(map->queue, e);
}


void *hash_map_get_first(hash_map_t *map)
{
    hash_element_t *e;

    if (map == NULL) {
        return NULL;
    }

    e = t2_queue_peek(map->queue, 0);
    if (e == NULL) {
        return NULL;
    }

    return e->data;
}

void *hash_map_lookup(hash_map_t *map, uint32_t n)
{
    hash_element_t *e;

    if (map == NULL) {
        return NULL;
    }

    e = t2_queue_peek(map->queue, n);
    if (e == NULL) {
        return NULL;
    }

    return e->data;
}

void *hash_map_lookupKey(hash_map_t *map, uint32_t n)
{
    hash_element_t *e;

    if (map == NULL) {
        return NULL;
    }

    e = t2_queue_peek(map->queue, n);
    if (e == NULL) {
        return NULL;
    }

    return e->key;
}

void *hash_map_get_next(hash_map_t *map, void *data)
{
    uint32_t count, i;
    hash_element_t *e;
    bool found = false;

    if ((map == NULL)|| (data == NULL)) {
        return NULL;
    }

    count = hash_map_count(map);
    for (i = 0; i < count; i++) {
        e = t2_queue_peek(map->queue, i);
        if ((e != NULL) && (e->data == data)) {
            found = true;
            break;
        }
    }

    if (found == false) {
        return NULL;
    }

    e = t2_queue_peek(map->queue, i + 1);
    if (e == NULL) {
        return NULL;
    }

    return e->data;
}

uint32_t hash_map_count(hash_map_t *map)
{
    if (map == NULL) {
        return -1;
    }
    return t2_queue_count(map->queue);
}

hash_map_t *hash_map_create()
{
	hash_map_t	*map;

	map = (hash_map_t *)malloc(sizeof(hash_map_t));
	if (map == NULL) {
		return NULL;
	}
	
	memset(map, 0, sizeof(hash_map_t));
	map->queue = t2_queue_create();

	return map;
}

void  hash_map_destroy(hash_map_t *map, queue_cleanup freeItem)
{
    if(map == NULL)
    {
        return;
    }
	t2_queue_destroy(map->queue, freeItem);
	free(map);
}

void hash_map_clear(hash_map_t *map, queue_cleanup freeItem)
{
    if(map == NULL)
    {
        return;
    }
    t2_queue_destroy(map->queue, freeItem);

    map = realloc(map, sizeof(hash_map_t));
    map->queue = t2_queue_create();
}
