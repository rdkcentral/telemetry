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

#ifndef _T2PARSER_H_

#define _T2PARSER_H_

#include <stdio.h>
#include <stdlib.h>
#include <cjson/cJSON.h>
#include "telemetry2_0.h"
#include "profile.h"
#include "msgpack.h"
#define MAX_PATH_LENGTH 512

#define T2REPORTCOMPONENT "RBUS_SUBSCRIPTION" //TR-181 event's component name

T2ERROR processConfiguration(char** configData, char* profileName, char* profileHash, Profile **localProfile);

msgpack_object *msgpack_get_map_value(msgpack_object *obj, char *key);

msgpack_object *msgpack_get_array_element(msgpack_object *obj, int index);

char *msgpack_strdup(msgpack_object *obj);

void msgpack_print(msgpack_object *obj, char *obj_name);

int msgpack_strcmp(msgpack_object *obj, char *str);

T2ERROR processMsgPackConfiguration(msgpack_object *profiles_array_map, Profile **profile_dp);

#endif /* _T2PARSER_H_ */
