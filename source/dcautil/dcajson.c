/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2016 RDK Management
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
 * @defgroup dca
 * @{
 **/

/**
 * @defgroup dca
 * @{
 * @defgroup src
 * @{
 **/

// To provide telemetry data in a JSON format
#include "legacyutils.h"
#include "t2log_wrapper.h"

/**
 * @addtogroup DCA_APIS
 * @{
 */

/**
 * @brief This API creates "searchResult" JSON array.
 *
 * The search result list contains collection of telemetry marker headers with
 * their value.
 *
 * Eg: {"searchResult":[{"MOCA_INFO_pnc_enabled":"1"},{"samv2_boardver_split":"
 * V3.0 ##"},{"RF_ERR_DS_lockfail":"1"},{"RF_ERR_T3_timeout":"2"}]}
 *
 * @param[out] root  JSON object
 * @param[in]  sr    Search result JSON array
 */

// {"searchResult":[{}]}  Telemetry 2.0 is not bound to any such data type - Get
// rid of this !!!
void initSearchResultJson(cJSON **root, cJSON **sr) {
  T2Debug("%s ++in \n", __FUNCTION__);
  *root = cJSON_CreateObject();
  if (NULL != *root) {
    cJSON_AddItemToObject(*root, "searchResult", *sr = cJSON_CreateArray());
  }
  T2Debug("%s --out \n", __FUNCTION__);
}

/**
 * @brief This API is to append the key/value pair to the SearchResult JSON
 * array .
 *
 * @param[in]  key     marker name
 * @param[in]  value   metric count
 */
void addToSearchResult(char *key, char *value) {
  T2Debug("%s ++in \n", __FUNCTION__);
  if (key == NULL || value == NULL) {
    T2Error("Key or Value is NULL\n");
    return;
  }
  if (NULL != SEARCH_RESULT_JSON) {
    cJSON *obj = cJSON_CreateObject();
    if (NULL != obj) {
      cJSON_AddStringToObject(obj, key, value);
      cJSON_AddItemToArray(SEARCH_RESULT_JSON, obj);
    }
  }
  T2Debug("%s --out \n", __FUNCTION__);
}

/**
 * @brief This API deletes the result JSON object.
 *
 * @param[in]  root JSON object to be deleted.
 */
void clearSearchResultJson(cJSON **root) {
  T2Debug("%s ++in \n", __FUNCTION__);
  if (root == NULL) {
    T2Error("root is NULL, can't be deleted\n");
    return;
  }
  cJSON_Delete(*root);
  T2Debug("%s --out \n", __FUNCTION__);
}

/** @} */ // END OF GROUP DCA_APIS
/** @} */

/** @} */
/** @} */
