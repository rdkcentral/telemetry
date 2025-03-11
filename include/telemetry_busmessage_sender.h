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

#ifndef MESSAGESENDER_H_
#define MESSAGESENDER_H_

#include <stdio.h>
#include <stdbool.h>

#include <unistd.h>
#include <fcntl.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "telemetry2_0.h"

/*
 * NAME        : t2_init
 * DESCRIPTION : Initializes the event sending module by registering
 *               with a unique name over the entire system
 * ARGUMENTS   : copmponent
 *               Name of the component value in string
*/
void t2_init(char *component);

/*
 * NAME        : t2_event_s
 * DESCRIPTION : Creates Telemetry information signal string format
 * ARGUMENTS   : marker name in string
 *               value in string
 * RETURN      : 0 on successful sending of the message
 */
T2ERROR t2_event_s(const char* marker, const char* value);

/*
 * NAME        : t2_event_f
 * DESCRIPTION : Creates Telemetry information signal string format
 * ARGUMENTS   : marker name in string
 *               value - floating point value
 * RETURN      : 0 on successful sending of the message
 */
T2ERROR t2_event_f(const char* marker, double value);


/*
 * NAME        : t2_log_d
 * DESCRIPTION : Creates Telemetry information signal string format
 * ARGUMENTS   : marker name in string
 *               value - integer value
 * RETURN      : 0 on successful sending of the message
*/
T2ERROR t2_event_d(const char* marker, int value);

/*
 * NAME        : t2_uninit
 * DESCRIPTION : Uninitializes the event sending module
 *               
 */
void t2_uninit(void);

#ifdef __cplusplus
}
#endif

#endif
