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
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "telemetry2_0.h"
#include "t2log_wrapper.h"

// This is a placeholder file to implement the parvacy control feature, The default behaviour is to
// return SHARE when get is called and T2ERROR_SUCCESS when set is called.


// This Function is used to set the privacyMode and save the value to persistant storage

T2ERROR setPrivacyMode(char* data){
    T2Debug("%s ++in\n", __FUNCTION__);
    T2Warning("%s function is called at stub implementation please check the implementation \n ",__FUNCTION__);
    T2Debug("%s --out\n", __FUNCTION__);
    return T2ERROR_SUCCESS;

}

// This Function is used to get the privacyMode
// First Preference Local cache PRIVACY_STATE
// Next Preference contacts Rdkservices to fetch the value
// Finally we will try to get the data from the persistant storage it will return SHARE on ERROR

void getPrivacyMode(char** privMode){
    T2Debug("%s ++in\n", __FUNCTION__);
    T2Warning("%s function is called at stub implementation please check the implementation returning SHARE \n ",__FUNCTION__);
    *privMode = strdup("SHARE");
    T2Debug("%s --out\n", __FUNCTION__);
    return;
}

