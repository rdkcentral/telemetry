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

#ifndef _T2MARKERS_H_
#define _T2MARKERS_H_

#include "telemetry2_0.h"
#include "profile.h"
#include "vector.h"


#define MAX_EVENT_MARKER_NAME_LEN 150

typedef struct _T2Marker
{
    char* markerName;
    char* componentName;
    Vector *profileList;
} T2Marker;

T2ERROR initT2MarkerComponentMap();

T2ERROR destroyT2MarkerComponentMap();

T2ERROR clearT2MarkerComponentMap();

T2ERROR addT2EventMarker(const char* markerName, const char* compName, const char *profileName, unsigned int skipFreq);

void getComponentMarkerList(const char* compName, void **markerList);

void getComponentsWithEventMarkers(Vector **eventComponentList);

T2ERROR getMarkerProfileList(const char* markerName, Vector **profileList);

void createComponentDataElements();

#endif /* _T2MARKERS_H_ */
