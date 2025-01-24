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

#ifndef _T2COLLECTION_H_
#define _T2COLLECTION_H_

#include <stdint.h>
#include <stdbool.h>

#define MAX_KEY_LEN 512

typedef struct element {
    void *data;
    struct element *next;
} element_t;

typedef struct {
    void    *data;
    char    *key;
} hash_element_t;

typedef struct {
    element_t   *head;
} queue_t;

typedef struct{
    queue_t *queue;
} hash_map_t;


typedef void (*queue_cleanup)(void *);
typedef void (*hashelement_data_cleanup)(void *);
// queue operations
queue_t *t2_queue_create(void);
void t2_queue_destroy(queue_t *q, queue_cleanup freeItem);
int8_t t2_queue_push(queue_t *q, void *data);
void *t2_queue_pop(queue_t *q);
void *t2_queue_remove(queue_t *q, uint32_t n);
void *t2_queue_peek(queue_t *q, uint32_t n);
uint32_t t2_queue_count(queue_t *q);


// hash map operations, currently hash map is flat there are no buckets
hash_map_t *hash_map_create(void);
void hash_map_destroy(hash_map_t *map, queue_cleanup freeItem);
int8_t hash_map_put(hash_map_t *map, char *key, void *data, hashelement_data_cleanup freeItem);
void *hash_map_get(hash_map_t *map, const char *key);
void *hash_map_remove(hash_map_t *map, const char *key);
void *hash_map_lookup(hash_map_t *map, uint32_t n);
void *hash_map_lookupKey(hash_map_t *map, uint32_t n);
uint32_t hash_map_count(hash_map_t *map);
void hash_map_clear(hash_map_t *map, queue_cleanup freeItem);

void *hash_map_get_first(hash_map_t *map);
void *hash_map_get_next(hash_map_t *map, void *data);

#endif // _T2COLLECTION_H_
