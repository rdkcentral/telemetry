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

#ifndef SOURCE_COMMONLIB_TELEMETRY_BUSMESSAGE_INTERNAL_H_
#define SOURCE_COMMONLIB_TELEMETRY_BUSMESSAGE_INTERNAL_H_

#define MAX_DATA_LEN 256

extern const char destCompName[64];
extern const char destCompPath[64];


#define EVENT_ERROR(format, ...) \
    fprintf(stderr, "T2ERROR:%s %s:%d: ", __func__ , __FILE__, __LINE__ ); \
    fprintf(stderr, (format), ##__VA_ARGS__ ); \
    fprintf(stderr, "\n" );

#endif /* SOURCE_COMMONLIB_TELEMETRY_BUSMESSAGE_INTERNAL_H_ */
