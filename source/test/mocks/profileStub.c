/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2026 RDK Management
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
 * @file profileStub.c
 * @brief Stub implementation of profile functions for unit testing
 *
 * Provides minimal stubs for profile.c functions to avoid pulling in
 * the entire dependency tree. Used by t2parser, dcautil, and bulkdata tests.
 */

#include <stdlib.h>
#include <string.h>
#include "profile.h"
#include "vector.h"

/**
 * @brief Stub for freeProfile - cleans up Profile struct for testing
 * 
 * In production, this function frees all memory in a Profile struct including
 * complex dependencies. For unit tests, we provide cleanup for basic fields
 * that tests typically allocate (name, hash, vectors, etc.) but stub out
 * complex subsystems like schedulers, protocols, and report generation.
 */
void freeProfile(void *data)
{
    if(data == NULL) {
        return;
    }
    
    Profile *profile = (Profile *)data;
    
    // Free simple string fields that tests commonly allocate
    if(profile->name) {
        free(profile->name);
    }
    if(profile->hash) {
        free(profile->hash);
    }
    if(profile->protocol) {
        free(profile->protocol);
    }
    if(profile->encodingType) {
        free(profile->encodingType);
    }
    if(profile->RootName) {
        free(profile->RootName);
    }
    if(profile->Description) {
        free(profile->Description);
    }
    if(profile->version) {
        free(profile->version);
    }
    if(profile->jsonEncoding) {
        free(profile->jsonEncoding);
    }
    if(profile->timeRef) {
        free(profile->timeRef);
    }
    
    // Free vectors that tests commonly create
    // Note: Using NULL cleanup function since tests typically don't
    // allocate complex objects in these vectors during basic testing
    if(profile->paramList) {
        Vector_Destroy(profile->paramList, NULL);
    }
    if(profile->staticParamList) {
        Vector_Destroy(profile->staticParamList, NULL);
    }
    if(profile->eMarkerList) {
        Vector_Destroy(profile->eMarkerList, NULL);
    }
    if(profile->gMarkerList) {
        Vector_Destroy(profile->gMarkerList, NULL);
    }
    if(profile->topMarkerList) {
        Vector_Destroy(profile->topMarkerList, NULL);
    }
    if(profile->dataModelTableList) {
        Vector_Destroy(profile->dataModelTableList, NULL);
    }
    if(profile->triggerConditionList) {
        Vector_Destroy(profile->triggerConditionList, NULL);
    }
    
    // Free destination structures if allocated
    if(profile->t2HTTPDest) {
        if(profile->t2HTTPDest->URL) {
            free(profile->t2HTTPDest->URL);
        }
        if(profile->t2HTTPDest->RequestURIparamList) {
            Vector_Destroy(profile->t2HTTPDest->RequestURIparamList, NULL);
        }
        free(profile->t2HTTPDest);
    }
    
    if(profile->t2RBUSDest) {
        if(profile->t2RBUSDest->rbusMethodName) {
            memset(profile->t2RBUSDest->rbusMethodName, 0, strlen(profile->t2RBUSDest->rbusMethodName));
            free(profile->t2RBUSDest->rbusMethodName);
        }
        if(profile->t2RBUSDest->rbusMethodParamList) {
            Vector_Destroy(profile->t2RBUSDest->rbusMethodParamList, NULL);
        }
        free(profile->t2RBUSDest);
    }
    
    // Finally free the profile struct itself
    free(profile);
}
