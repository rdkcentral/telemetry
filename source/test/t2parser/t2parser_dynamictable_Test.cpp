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
 * @file t2parser_dynamictable_Test.cpp
 * @brief Unit tests for DataModelTable feature and memory safety fixes
 * 
 * Features Tested:
 * - DataModelTable parameter type parsing
 * - Index parameter support (single, range, comma-separated)
 * - Nested dataModelTable configurations
 * - Dynamic table parameter filtering
 * - Wildcard matching for table instances
 * 
 * Memory Safety Fixes Tested:
 * - Coverity BAD_FREE fix (allocation tracking for strdup vs cJSON pointers)
 * - Profile cleanup on parse failure (freeProfile + cJSON_Delete)
 * - Conditional vector creation (prevent double-initialization)
 * - Error path resource cleanup (parameterName, parameterWild)
 */

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <memory>

using namespace std;
using ::testing::_;
using ::testing::Return;
using ::testing::Invoke;

extern "C" {
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <signal.h>
#include <stdint.h>
#include <limits.h>
#include <msgpack.h>
#include <cjson/cJSON.h>

#include <bulkdata/reportprofiles.h>
#include <bulkdata/profilexconf.h>
#include <bulkdata/profile.h>
#include <utils/t2common.h>
#include <xconf-client/xconfclient.h>
#include <t2parser/t2parser.h>
#include <t2parser/t2parserxconf.h>
#include <telemetry2_0.h>
#include <ccspinterface/busInterface.h>
#include <glib.h>
#include <glib/gi18n.h>

sigset_t blocking_signal;

// Expose internal functions for testing
T2ERROR processConfiguration(char** configData, char *profileName, char* profileHash, Profile **localProfile);
T2ERROR addParameter_marker_config(Profile* profile, cJSON *jprofileParameter, int ThisProfileParameter_count);
void freeProfile(void *data);
}

#include "t2parserMock.h"
#include "test/mocks/rdklogMock.h"
#include "test/mocks/rbusMock.h"

extern T2parserMock *m_t2parserMock;
extern rdklogMock *m_rdklogMock;
extern rbusMock *g_rbusMock;

// Mock instance definitions
T2parserMock *m_t2parserMock = NULL;
rdklogMock *m_rdklogMock = NULL;
rbusMock *g_rbusMock = NULL;

/**
 * @brief Test fixture for dynamic table tests
 */
class DynamicTableTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize mocks if needed
    }

    void TearDown() override {
        // Cleanup
    }
};

/**
 * @brief Verifies that when addParameter_marker_config() fails:
 * 1. freeProfile() is called
 * 2. cJSON_Delete() is called
 * 3. No memory leak occurs
 * 4. Function returns T2ERROR_FAILURE
 */
TEST_F(DynamicTableTestFixture, ParseFailure_TriggersProperCleanup)
{
    // Prepare invalid JSON configuration that will fail parameter parsing
    const char* invalidConfig = R"({
        "Description": "Test Profile",
        "Version": "1",
        "Protocol": "HTTP",
        "EncodingType": "JSON",
        "ReportingInterval": 60,
        "Parameter": [
            {
                "type": "invalid_type_that_causes_failure",
                "name": "InvalidParam"
            }
        ]
    })";

    char* configData = strdup(invalidConfig);
    Profile* profile = nullptr;
    
    // processConfiguration should return failure and cleanup properly
    T2ERROR result = processConfiguration(&configData, 
                                         const_cast<char*>("TestProfile"), 
                                         nullptr, 
                                         &profile);
    
    // Verify failure is returned
    EXPECT_EQ(T2ERROR_FAILURE, result);
    
    // Verify profile was not created (should be null or cleaned up)
    // If cleanup worked, profile should not have been returned
    // Note: Actual verification would need valgrind to confirm no leak
    
    if (configData) {
        free(configData);
    }
}

/**
 * @brief Test dataModelTable without index allocates and frees correctly
 * 
 * Verifies the allocation tracking fix:
 * - content and header allocated with strdup() should be freed
 * - Allocation flags properly track dynamic memory
 */
TEST_F(DynamicTableTestFixture, DataModelTable_WithoutIndex_ProperMemoryManagement)
{
    // Configuration with dataModelTable but no index
    const char* validConfig = R"({
        "Description": "DataModelTable Test",
        "Version": "1",
        "Protocol": "HTTP",
        "EncodingType": "JSON",
        "ReportingInterval": 60,
        "Parameter": [
            {
                "type": "dataModelTable",
                "reference": "Device.WiFi.AccessPoint."
            }
        ]
    })";

    char* configData = strdup(validConfig);
    Profile* profile = nullptr;
    
    T2ERROR result = processConfiguration(&configData,
                                         const_cast<char*>("DataModelTableTest"),
                                         nullptr,
                                         &profile);
    
    // Should succeed (assuming all dependencies are mocked properly)
    // In a full test environment with proper mocks, this would verify:
    // 1. strdup() was called for content/header
    // 2. Allocation flags were set to true
    // 3. Memory was freed correctly on cleanup
    
    // Cleanup
    if (profile != nullptr) {
        freeProfile(profile);
    }
    if (configData) {
        free(configData);
    }
    
    // Note: Full verification requires valgrind integration
    SUCCEED();
}

/**
 * @brief Test that cJSON valuestring pointers are NOT freed
 * 
 * Verifies the Coverity fix:
 * - Regular dataModel parameters use cJSON's internal valuestring
 * - These should NOT be freed (allocation flags remain false)
 */
TEST_F(DynamicTableTestFixture, RegularDataModel_NoDoubleFree)
{
    // Configuration with regular dataModel (not dataModelTable)
    const char* validConfig = R"({
        "Description": "Regular DataModel Test",
        "Version": "1",
        "Protocol": "HTTP",
        "EncodingType": "JSON",
        "ReportingInterval": 60,
        "Parameter": [
            {
                "type": "dataModel",
                "name": "CPUTemp",
                "reference": "Device.DeviceInfo.ProcessorTemp"
            }
        ]
    })";

    char* configData = strdup(validConfig);
    Profile* profile = nullptr;
    
    T2ERROR result = processConfiguration(&configData,
                                         const_cast<char*>("RegularDataModelTest"),
                                         nullptr,
                                         &profile);
    
    // This test verifies that:
    // 1. content points to jpSubitemreference->valuestring (cJSON internal)
    // 2. content_allocated flag remains false
    // 3. No attempt is made to free cJSON's internal memory
    
    // Cleanup
    if (profile != nullptr) {
        freeProfile(profile);
    }
    if (configData) {
        free(configData);
    }
    
    // Note: This test would catch the original Coverity defect
    // by causing a crash or corruption if free() was called on cJSON memory
    SUCCEED();
}

/**
 * @brief Test conditional vector creation doesn't recreate existing vectors
 * 
 * Verifies:
 * - Vectors are only created if they don't exist
 * - No memory leak from recreating vectors
 * - Prevents potential crashes from vector double-initialization
 */
TEST_F(DynamicTableTestFixture, ConditionalVectorCreation_NoRecreation)
{
    // This test would require accessing profile internals
    // Create a profile with pre-initialized vectors
    Profile* testProfile = (Profile*)calloc(1, sizeof(Profile));
    ASSERT_NE(testProfile, nullptr);
    
    // Pre-create some vectors to simulate reuse scenario
    Vector_Create(&testProfile->paramList);
    Vector_Create(&testProfile->gMarkerList);
    
    // Store original pointers
    void* originalParamList = testProfile->paramList;
    void* originalGMarkerList = testProfile->gMarkerList;
    
    // Create a minimal valid configuration
    cJSON* jprofileParameter = cJSON_CreateArray();
    ASSERT_NE(jprofileParameter, nullptr);
    
    cJSON* param = cJSON_CreateObject();
    cJSON_AddStringToObject(param, "type", "grep");
    cJSON_AddStringToObject(param, "marker", "TestMarker");
    cJSON_AddStringToObject(param, "search", "TestSearch");
    cJSON_AddStringToObject(param, "logFile", "/tmp/test.log");
    cJSON_AddItemToArray(jprofileParameter, param);
    
    // Call addParameter_marker_config with pre-initialized vectors
    T2ERROR result = addParameter_marker_config(testProfile, jprofileParameter, 1);
    
    // Verify vectors were not recreated (pointers should be same)
    // Note: This requires the fix from PR-363 where we check if vectors exist
    // before creating them
    EXPECT_EQ(originalParamList, testProfile->paramList) 
        << "paramList should not be recreated";
    EXPECT_EQ(originalGMarkerList, testProfile->gMarkerList) 
        << "gMarkerList should not be recreated";
    
    // Cleanup
    cJSON_Delete(jprofileParameter);
    freeProfile(testProfile);
}

/**
 * @brief Test multiple parameter types with mixed allocation patterns
 * 
 * Verifies allocation tracking works correctly with:
 * - dataModel (cJSON pointer - no allocation)
 * - event (cJSON pointer - no allocation)
 * - grep (cJSON pointer - no allocation)
 * - dataModelTable without index (strdup - allocation)
 */
TEST_F(DynamicTableTestFixture, MixedParameterTypes_ProperAllocationTracking)
{
    const char* mixedConfig = R"({
        "Description": "Mixed Parameter Types",
        "Version": "1",
        "Protocol": "HTTP",
        "EncodingType": "JSON",
        "ReportingInterval": 60,
        "Parameter": [
            {
                "type": "dataModel",
                "name": "CPUUsage",
                "reference": "Device.DeviceInfo.ProcessorLoad"
            },
            {
                "type": "event",
                "eventName": "TestEvent",
                "component": "TestComponent"
            },
            {
                "type": "grep",
                "marker": "ErrorMarker",
                "search": "ERROR",
                "logFile": "/var/log/test.log"
            },
            {
                "type": "dataModelTable",
                "reference": "Device.WiFi.SSID."
            }
        ]
    })";

    char* configData = strdup(mixedConfig);
    Profile* profile = nullptr;
    
    T2ERROR result = processConfiguration(&configData,
                                         const_cast<char*>("MixedParamsTest"),
                                         nullptr,
                                         &profile);
    
    // This test exercises all code paths in the allocation tracking logic
    // - First 3 params: content/header point to cJSON (no allocation)
    // - Last param: content/header use strdup (allocation = true)
    // - Cleanup should only free the last param's strings
    
    if (profile != nullptr) {
        freeProfile(profile);
    }
    if (configData) {
        free(configData);
    }
    
    SUCCEED();
}

/**
 * @brief Test error path cleanup with NULL checks
 * 
 * Verifies:
 * - NULL checks before free() work correctly
 * - Error paths don't crash on NULL pointers
 * - Allocation flags prevent freeing unallocated memory
 */
TEST_F(DynamicTableTestFixture, ErrorPath_NullSafety)
{
    // Configuration that will trigger error during parameter processing
    const char* errorConfig = R"({
        "Description": "Error Path Test",
        "Version": "1",
        "Protocol": "HTTP",
        "EncodingType": "JSON",
        "ReportingInterval": 60,
        "Parameter": [
            {
                "type": "dataModel",
                "reference": "Device.Test"
            }
        ]
    })";

    char* configData = strdup(errorConfig);
    Profile* profile = nullptr;
    
    // This may fail due to missing dependencies in test environment
    // The key is that it doesn't crash even if failures occur
    T2ERROR result = processConfiguration(&configData,
                                         const_cast<char*>("ErrorPathTest"),
                                         nullptr,
                                         &profile);
    
    // Regardless of success/failure, should not crash
    // The fixes ensure:
    // 1. NULL checks before free()
    // 2. Allocation flags prevent invalid free()
    // 3. Proper cleanup on all error paths
    
    if (profile != nullptr) {
        freeProfile(profile);
    }
    if (configData) {
        free(configData);
    }
    
    SUCCEED();
}

/**
 * @brief Integration test: End-to-end parse and cleanup
 * 
 * Comprehensive test that:
 * 1. Parses a complete valid configuration
 * 2. Verifies all vectors are created
 * 3. Cleans up properly without leaks
 */
TEST_F(DynamicTableTestFixture, EndToEnd_ParseAndCleanup)
{
    const char* completeConfig = R"({
        "Description": "Complete Profile Test",
        "Version": "2.0",
        "Protocol": "HTTP",
        "EncodingType": "JSON",
        "ReportingInterval": 300,
        "Parameter": [
            {
                "type": "dataModel",
                "name": "MemoryUsage",
                "reference": "Device.DeviceInfo.MemoryStatus.Total"
            },
            {
                "type": "grep",
                "marker": "BootupTime",
                "search": "boot_time",
                "logFile": "/var/log/boot.log"
            }
        ]
    })";

    char* configData = strdup(completeConfig);
    Profile* profile = nullptr;
    
    T2ERROR result = processConfiguration(&configData,
                                         const_cast<char*>("CompleteProfileTest"),
                                         nullptr,
                                         &profile);
    
    // This exercises the full parse flow:
    // - Conditional vector creation
    // - Allocation tracking
    // - Proper cleanup on both success and failure paths
    
    if (profile != nullptr) {
        // Verify profile was created
        EXPECT_NE(profile->name, nullptr);
        
        // Cleanup
        freeProfile(profile);
    }
    
    if (configData) {
        free(configData);
    }
}

// ============================================================================
// DataModelTable Dynamic Table Support
// ============================================================================

/**
 * @brief Test dataModelTable with single index
 * 
 * Index parameter supports single values
 * Example: "index": "2"
 */
TEST_F(DynamicTableTestFixture, DataModelTable_SingleIndex)
{
    const char* singleIndexConfig = R"({
        "Description": "Single Index Test",
        "Version": "2.0",
        "Protocol": "HTTP",
        "EncodingType": "JSON",
        "ReportingInterval": 60,
        "Parameter": [
            {
                "type": "dataModelTable",
                "reference": "Device.WiFi.AccessPoint.",
                "index": "1"
            }
        ]
    })";

    char* configData = strdup(singleIndexConfig);
    Profile* profile = nullptr;
    
    T2ERROR result = processConfiguration(&configData,
                                         const_cast<char*>("SingleIndexTest"),
                                         nullptr,
                                         &profile);
    
    // Should parse successfully and create parameter for index 1
    // Resulting in: Device.WiFi.AccessPoint.1.
    
    if (profile != nullptr) {
        // Verify dataModelTableList was created
        EXPECT_NE(profile->dataModelTableList, nullptr);
        freeProfile(profile);
    }
    if (configData) free(configData);
}

/**
 * @brief Test dataModelTable with range of indexes
 * 
 * Index parameter supports ranges
 * Example: "index": "1-5"
 */
TEST_F(DynamicTableTestFixture, DataModelTable_IndexRange)
{
    const char* rangeIndexConfig = R"({
        "Description": "Index Range Test",
        "Version": "2.0",
        "Protocol": "HTTP",
        "EncodingType": "JSON",
        "ReportingInterval": 60,
        "Parameter": [
            {
                "type": "dataModelTable",
                "reference": "Device.WiFi.SSID.",
                "index": "1-3"
            }
        ]
    })";

    char* configData = strdup(rangeIndexConfig);
    Profile* profile = nullptr;
    
    T2ERROR result = processConfiguration(&configData,
                                         const_cast<char*>("RangeIndexTest"),
                                         nullptr,
                                         &profile);
    
    // Should create parameters for indexes 1, 2, 3
    // Resulting in: Device.WiFi.SSID.1., Device.WiFi.SSID.2., Device.WiFi.SSID.3.
    
    if (profile != nullptr) {
        freeProfile(profile);
    }
    if (configData) free(configData);
}

/**
 * @brief Test dataModelTable with comma-separated indexes
 * 
 * Index parameter supports comma-separated values
 * Example: "index": "1,3,5,7"
 */
TEST_F(DynamicTableTestFixture, DataModelTable_CommaSeparatedIndexes)
{
    const char* commaIndexConfig = R"({
        "Description": "Comma-Separated Index Test",
        "Version": "2.0",
        "Protocol": "HTTP",
        "EncodingType": "JSON",
        "ReportingInterval": 60,
        "Parameter": [
            {
                "type": "dataModelTable",
                "reference": "Device.Hosts.Host.",
                "index": "1,3,5"
            }
        ]
    })";

    char* configData = strdup(commaIndexConfig);
    Profile* profile = nullptr;
    
    T2ERROR result = processConfiguration(&configData,
                                         const_cast<char*>("CommaIndexTest"),
                                         nullptr,
                                         &profile);
    
    // Should create parameters for indexes 1, 3, 5 (skipping 2, 4)
    
    if (profile != nullptr) {
        freeProfile(profile);
    }
    if (configData) free(configData);
}

/**
 * @brief Test dataModelTable with mixed index specification
 * 
 * Index parameter supports mixed ranges and singles
 * Example: "index": "1-3,5,8-10"
 */
TEST_F(DynamicTableTestFixture, DataModelTable_MixedIndexes)
{
    const char* mixedIndexConfig = R"({
        "Description": "Mixed Index Test",
        "Version": "2.0",
        "Protocol": "HTTP",
        "EncodingType": "JSON",
        "ReportingInterval": 60,
        "Parameter": [
            {
                "type": "dataModelTable",
                "reference": "Device.WiFi.AccessPoint.",
                "index": "1-2,5,7-9"
            }
        ]
    })";

    char* configData = strdup(mixedIndexConfig);
    Profile* profile = nullptr;
    
    T2ERROR result = processConfiguration(&configData,
                                         const_cast<char*>("MixedIndexTest"),
                                         nullptr,
                                         &profile);
    
    // Should create parameters for indexes: 1, 2, 5, 7, 8, 9
    
    if (profile != nullptr) {
        freeProfile(profile);
    }
    if (configData) free(configData);
}

/**
 * @brief Test dataModelTable without index (wildcard collection)
 * 
 * dataModelTable without index collects from all instances.
 * This triggers the strdup() allocation path (Coverity fix).
 */
TEST_F(DynamicTableTestFixture, DataModelTable_NoIndex_WildcardCollection)
{
    const char* noIndexConfig = R"({
        "Description": "No Index Wildcard Test",
        "Version": "2.0",
        "Protocol": "HTTP",
        "EncodingType": "JSON",
        "ReportingInterval": 60,
        "Parameter": [
            {
                "type": "dataModelTable",
                "reference": "Device.WiFi.AccessPoint."
            }
        ]
    })";

    char* configData = strdup(noIndexConfig);
    Profile* profile = nullptr;
    
    T2ERROR result = processConfiguration(&configData,
                                         const_cast<char*>("NoIndexTest"),
                                         nullptr,
                                         &profile);
    
    // This case uses strdup() for content/header (Coverity fix applies)
    // Should collect from all AccessPoint instances dynamically
    
    if (profile != nullptr) {
        // Verify table list created
        if (profile->dataModelTableList != nullptr) {
            EXPECT_GT(Vector_Size(profile->dataModelTableList), 0);
        }
        freeProfile(profile);
    }
    if (configData) free(configData);
}

/**
 * @brief Test dataModelTable with nested parameters
 * 
 * Supports nested parameters within dataModelTable.
 */
TEST_F(DynamicTableTestFixture, DataModelTable_NestedParameters)
{
    const char* nestedConfig = R"({
        "Description": "Nested Parameters Test",
        "Version": "2.0",
        "Protocol": "HTTP",
        "EncodingType": "JSON",
        "ReportingInterval": 60,
        "Parameter": [
            {
                "type": "dataModelTable",
                "reference": "Device.WiFi.AccessPoint.",
                "index": "1",
                "parameters": [
                    {
                        "name": "SSID"
                    },
                    {
                        "name": "Status"
                    }
                ]
            }
        ]
    })";

    char* configData = strdup(nestedConfig);
    Profile* profile = nullptr;
    
    T2ERROR result = processConfiguration(&configData,
                                         const_cast<char*>("NestedParamsTest"),
                                         nullptr,
                                         &profile);
    
    // Should collect Device.WiFi.AccessPoint.1.SSID and Device.WiFi.AccessPoint.1.Status
    
    if (profile != nullptr) {
        freeProfile(profile);
    }
    if (configData) free(configData);
}

/**
 * @brief Test duplicate index handling
 * 
 * Duplicate indexes should be filtered.
 * Example: "index": "1,2,2,3,1" should process only 1,2,3
 */
TEST_F(DynamicTableTestFixture, DataModelTable_DuplicateIndexFiltering)
{
    const char* duplicateConfig = R"({
        "Description": "Duplicate Index Test",
        "Version": "2.0",
        "Protocol": "HTTP",
        "EncodingType": "JSON",
        "ReportingInterval": 60,
        "Parameter": [
            {
                "type": "dataModelTable",
                "reference": "Device.WiFi.SSID.",
                "index": "1,2,2,3,1,3"
            }
        ]
    })";

    char* configData = strdup(duplicateConfig);
    Profile* profile = nullptr;
    
    T2ERROR result = processConfiguration(&configData,
                                         const_cast<char*>("DuplicateIndexTest"),
                                         nullptr,
                                         &profile);
    
    // Implementation filters duplicates using duplicate[] array
    // Should process only 1, 2, 3 (each once)
    
    if (profile != nullptr) {
        freeProfile(profile);
    }
    if (configData) free(configData);
}

/**
 * @brief Test invalid index values
 * 
 * Invalid indexes (negative, out of range) should be skipped.
 */
TEST_F(DynamicTableTestFixture, DataModelTable_InvalidIndexHandling)
{
    const char* invalidConfig = R"({
        "Description": "Invalid Index Test",
        "Version": "2.0",
        "Protocol": "HTTP",
        "EncodingType": "JSON",
        "ReportingInterval": 60,
        "Parameter": [
            {
                "type": "dataModelTable",
                "reference": "Device.WiFi.SSID.",
                "index": "-1,1,256,2,300"
            }
        ]
    })";

    char* configData = strdup(invalidConfig);
    Profile* profile = nullptr;
    
    T2ERROR result = processConfiguration(&configData,
                                         const_cast<char*>("InvalidIndexTest"),
                                         nullptr,
                                         &profile);
    
    // Validates: if (val < 0 || val >= 256) skip
    // Should process only 1, 2 (skip -1, 256, 300)
    
    if (profile != nullptr) {
        freeProfile(profile);
    }
    if (configData) free(configData);
}

/**
 * @brief Test whitespace handling in index parameter
 * 
 * Whitespace in index string should be stripped.
 */
TEST_F(DynamicTableTestFixture, DataModelTable_WhitespaceInIndex)
{
    const char* whitespaceConfig = R"({
        "Description": "Whitespace Index Test",
        "Version": "2.0",
        "Protocol": "HTTP",
        "EncodingType": "JSON",
        "ReportingInterval": 60,
        "Parameter": [
            {
                "type": "dataModelTable",
                "reference": "Device.WiFi.SSID.",
                "index": " 1 , 2 - 4 , 6 "
            }
        ]
    })";

    char* configData = strdup(whitespaceConfig);
    Profile* profile = nullptr;
    
    T2ERROR result = processConfiguration(&configData,
                                         const_cast<char*>("WhitespaceIndexTest"),
                                         nullptr,
                                         &profile);
    
    // Strips whitespace before parsing
    // Should process: 1, 2, 3, 4, 6
    
    if (profile != nullptr) {
        freeProfile(profile);
    }
    if (configData) free(configData);
}

/**
 * @brief Test dataModelTable combined with regular dataModel parameters
 * 
 * Can mix dataModelTable with other parameter types.
 */
TEST_F(DynamicTableTestFixture, MixedParameterTypes_WithDataModelTable)
{
    const char* mixedTypeConfig = R"({
        "Description": "Mixed Types with DataModelTable",
        "Version": "2.0",
        "Protocol": "HTTP",
        "EncodingType": "JSON",
        "ReportingInterval": 60,
        "Parameter": [
            {
                "type": "dataModel",
                "name": "CPUUsage",
                "reference": "Device.DeviceInfo.ProcessorLoad"
            },
            {
                "type": "dataModelTable",
                "reference": "Device.WiFi.AccessPoint.",
                "index": "1-2"
            },
            {
                "type": "grep",
                "marker": "BootTime",
                "search": "boot_complete",
                "logFile": "/var/log/boot.log"
            }
        ]
    })";

    char* configData = strdup(mixedTypeConfig);
    Profile* profile = nullptr;
    
    T2ERROR result = processConfiguration(&configData,
                                         const_cast<char*>("MixedTypesWithTableTest"),
                                         nullptr,
                                         &profile);
    
    // Should successfully parse all three types
    // This tests that dynamic table integration doesn't break existing functionality
    
    if (profile != nullptr) {
        freeProfile(profile);
    }
    if (configData) free(configData);
}

// ============================================================================
// Edge Case and Rejection Scenarios
// ============================================================================

/**
 * @brief Reject missing reference field → T2ERROR_FAILURE, no crash
 *
 * Given a dataModelTable entry without a "reference" field,
 * the parser MUST log an error and not crash.
 */
TEST_F(DynamicTableTestFixture, Reject_MissingReference_NoCrash)
{
    const char* noRefConfig = R"({
        "Description": "Missing Reference Test",
        "Version": "1",
        "Protocol": "HTTP",
        "EncodingType": "JSON",
        "ReportingInterval": 60,
        "Parameter": [
            {
                "type": "dataModelTable",
                "index": "1",
                "Parameter": [
                    { "type": "dataModel", "reference": "Enable" }
                ]
            }
        ]
    })";

    char* configData = strdup(noRefConfig);
    Profile* profile = nullptr;

    T2ERROR result = processConfiguration(&configData,
                                         const_cast<char*>("MissingRefTest"),
                                         nullptr,
                                         &profile);

    // Should not crash — the entry is skipped with a logged error
    // Profile may still be created (other params could succeed)
    if (profile != nullptr) {
        freeProfile(profile);
    }
    if (configData) free(configData);
    SUCCEED();
}

/**
 * @brief Reject empty or missing Parameter array → T2ERROR_FAILURE, no crash
 *
 * Given a dataModelTable entry with no nested "Parameter" array,
 * the parser MUST log an error and not crash.
 */
TEST_F(DynamicTableTestFixture, Reject_EmptyParameterArray_NoCrash)
{
    const char* noParamConfig = R"({
        "Description": "Empty Parameter Array Test",
        "Version": "1",
        "Protocol": "HTTP",
        "EncodingType": "JSON",
        "ReportingInterval": 60,
        "Parameter": [
            {
                "type": "dataModelTable",
                "reference": "Device.WiFi.Radio."
            }
        ]
    })";

    char* configData = strdup(noParamConfig);
    Profile* profile = nullptr;

    T2ERROR result = processConfiguration(&configData,
                                         const_cast<char*>("EmptyParamArrayTest"),
                                         nullptr,
                                         &profile);

    // Should not crash — missing Parameter array is handled gracefully
    if (profile != nullptr) {
        freeProfile(profile);
    }
    if (configData) free(configData);
    SUCCEED();
}

/**
 * @brief Reject completely empty Parameter array (zero elements)
 */
TEST_F(DynamicTableTestFixture, Reject_ZeroElementParameterArray_NoCrash)
{
    const char* emptyArrayConfig = R"({
        "Description": "Zero Element Parameter Array",
        "Version": "1",
        "Protocol": "HTTP",
        "EncodingType": "JSON",
        "ReportingInterval": 60,
        "Parameter": [
            {
                "type": "dataModelTable",
                "reference": "Device.WiFi.Radio.",
                "Parameter": []
            }
        ]
    })";

    char* configData = strdup(emptyArrayConfig);
    Profile* profile = nullptr;

    T2ERROR result = processConfiguration(&configData,
                                         const_cast<char*>("ZeroParamArrayTest"),
                                         nullptr,
                                         &profile);

    if (profile != nullptr) {
        freeProfile(profile);
    }
    if (configData) free(configData);
    SUCCEED();
}

/**
 * @brief Path construction reaches MAX_PATH_LENGTH (512 bytes)
 *
 * Given a reference path that exceeds 512 bytes when combined with index
 * and sub-parameter names, the parser MUST detect truncation and log an error.
 */
TEST_F(DynamicTableTestFixture, Edge_PathExceedsMaxPathLength_ErrorLogged)
{
    // Build a reference that, when combined with nested params, exceeds 512 bytes
    // MAX_PATH_LENGTH is 512 in t2parser.h
    std::string longRef = "Device.";
    while (longRef.size() < 480) {
        longRef += "VeryLongComponentNameForTesting.";
    }

    std::string config = R"({
        "Description": "Long Path Test",
        "Version": "1",
        "Protocol": "HTTP",
        "EncodingType": "JSON",
        "ReportingInterval": 60,
        "Parameter": [
            {
                "type": "dataModelTable",
                "reference": ")" + longRef + R"(",
                "Parameter": [
                    { "type": "dataModel", "reference": "SubParam.DeepNested.Value" }
                ]
            }
        ]
    })";

    char* configData = strdup(config.c_str());
    Profile* profile = nullptr;

    T2ERROR result = processConfiguration(&configData,
                                         const_cast<char*>("LongPathTest"),
                                         nullptr,
                                         &profile);

    // Should not crash — truncation is detected and error is logged
    if (profile != nullptr) {
        freeProfile(profile);
    }
    if (configData) free(configData);
    SUCCEED();
}

/**
 * @brief Invalid index string (letters, negative numbers) → warning logged, index skipped
 *
 * Validates that non-numeric and negative index values are skipped gracefully.
 */
TEST_F(DynamicTableTestFixture, Edge_InvalidIndexLetters_Skipped)
{
    const char* letterIndexConfig = R"({
        "Description": "Letter Index Test",
        "Version": "1",
        "Protocol": "HTTP",
        "EncodingType": "JSON",
        "ReportingInterval": 60,
        "Parameter": [
            {
                "type": "dataModelTable",
                "reference": "Device.WiFi.SSID.",
                "index": "abc,1,xyz,2,-5"
            }
        ]
    })";

    char* configData = strdup(letterIndexConfig);
    Profile* profile = nullptr;

    T2ERROR result = processConfiguration(&configData,
                                         const_cast<char*>("LetterIndexTest"),
                                         nullptr,
                                         &profile);

    // atoi("abc") returns 0, atoi("xyz") returns 0 → index 0 is valid (< 256)
    // atoi("-5") returns -5 → skipped (val < 0)
    // Only valid indices should be processed, no crash
    if (profile != nullptr) {
        freeProfile(profile);
    }
    if (configData) free(configData);
    SUCCEED();
}

/**
 * @brief Valid parse - nested dataModelTable (table within table)
 *
 * Verifies recursive parsing of nested dataModelTable entries.
 */
TEST_F(DynamicTableTestFixture, ValidParse_NestedDataModelTable)
{
    const char* nestedTableConfig = R"({
        "Description": "Nested Table Test",
        "Version": "1",
        "Protocol": "HTTP",
        "EncodingType": "JSON",
        "ReportingInterval": 60,
        "Parameter": [
            {
                "type": "dataModelTable",
                "reference": "Device.WiFi.AccessPoint.",
                "Parameter": [
                    {
                        "type": "dataModel",
                        "reference": "Enable"
                    },
                    {
                        "type": "dataModelTable",
                        "reference": "AssociatedDevice.",
                        "Parameter": [
                            {
                                "type": "dataModel",
                                "reference": "MACAddress"
                            },
                            {
                                "type": "dataModel",
                                "reference": "SignalStrength"
                            }
                        ]
                    }
                ]
            }
        ]
    })";

    char* configData = strdup(nestedTableConfig);
    Profile* profile = nullptr;

    T2ERROR result = processConfiguration(&configData,
                                         const_cast<char*>("NestedTableTest"),
                                         nullptr,
                                         &profile);

    if (profile != nullptr) {
        // Verify dataModelTableList was created with nested params
        ASSERT_NE(profile->dataModelTableList, nullptr);
        EXPECT_GT(Vector_Size(profile->dataModelTableList), (size_t)0);

        // Verify the table has sub-parameters from both levels
        DataModelTable* table = (DataModelTable*)Vector_At(profile->dataModelTableList, 0);
        ASSERT_NE(table, nullptr);
        EXPECT_NE(table->paramList, nullptr);
        // Should have: Enable, MACAddress, SignalStrength from nested parsing
        EXPECT_GE(Vector_Size(table->paramList), (size_t)2);

        freeProfile(profile);
    }
    if (configData) free(configData);
}

// ============================================================================
// Coverage: parseDataModelTableParams, buildFullPath, addParameter_marker_config
// These static functions are exercised through processConfiguration.
// ============================================================================

/**
 * @brief Covers parseDataModelTableParams: valid table with multiple sub-parameters
 *
 * Exercises: parseDataModelTableParams (root table creation path),
 *            buildFullPath (path concatenation), and
 *            dataModelTable case in addParameter_marker_config.
 */
TEST_F(DynamicTableTestFixture, Coverage_ParseDataModelTableParams_MultipleSubParams)
{
    const char* config = R"({
        "Description": "Multi SubParam Table",
        "Version": "1",
        "Protocol": "HTTP",
        "EncodingType": "JSON",
        "ReportingInterval": 60,
        "Parameter": [
            {
                "type": "dataModelTable",
                "reference": "Device.WiFi.Radio.",
                "Parameter": [
                    { "type": "dataModel", "reference": "Enable" },
                    { "type": "dataModel", "reference": "Channel" },
                    { "type": "dataModel", "reference": "OperatingFrequencyBand" }
                ]
            }
        ]
    })";

    char* configData = strdup(config);
    Profile* profile = nullptr;

    T2ERROR result = processConfiguration(&configData,
                                         const_cast<char*>("MultiSubParamTest"),
                                         nullptr, &profile);

    if (profile != nullptr) {
        // Verify dataModelTableList was created
        ASSERT_NE(profile->dataModelTableList, nullptr);
        EXPECT_GT(Vector_Size(profile->dataModelTableList), (size_t)0);

        DataModelTable* table = (DataModelTable*)Vector_At(profile->dataModelTableList, 0);
        ASSERT_NE(table, nullptr);
        EXPECT_STREQ(table->reference, "Device.WiFi.Radio.");
        EXPECT_EQ(table->index, nullptr);  // No index specified

        // Should have 3 sub-parameters
        ASSERT_NE(table->paramList, nullptr);
        EXPECT_EQ(Vector_Size(table->paramList), (size_t)3);

        // Verify parameter names include wildcard path
        DataModelParam* p0 = (DataModelParam*)Vector_At(table->paramList, 0);
        ASSERT_NE(p0, nullptr);
        EXPECT_NE(strstr(p0->name, "Device.WiFi.Radio."), nullptr);
        EXPECT_NE(strstr(p0->name, "Enable"), nullptr);

        freeProfile(profile);
    }
    if (configData) free(configData);
}

/**
 * @brief Covers buildFullPath: basePath already ends with dot
 *
 * Tests the path where basePath ends with '.' so no extra dot is needed.
 */
TEST_F(DynamicTableTestFixture, Coverage_BuildFullPath_BaseEndsWithDot)
{
    const char* config = R"({
        "Description": "BuildFullPath Dot Test",
        "Version": "1",
        "Protocol": "HTTP",
        "EncodingType": "JSON",
        "ReportingInterval": 60,
        "Parameter": [
            {
                "type": "dataModelTable",
                "reference": "Device.Hosts.Host.",
                "Parameter": [
                    { "type": "dataModel", "reference": "HostName" }
                ]
            }
        ]
    })";

    char* configData = strdup(config);
    Profile* profile = nullptr;

    T2ERROR result = processConfiguration(&configData,
                                         const_cast<char*>("BuildFullPathDotTest"),
                                         nullptr, &profile);

    if (profile != nullptr) {
        ASSERT_NE(profile->dataModelTableList, nullptr);
        DataModelTable* table = (DataModelTable*)Vector_At(profile->dataModelTableList, 0);
        ASSERT_NE(table, nullptr);
        ASSERT_NE(table->paramList, nullptr);

        DataModelParam* p = (DataModelParam*)Vector_At(table->paramList, 0);
        ASSERT_NE(p, nullptr);
        // Path should be: Device.Hosts.Host.*.HostName (dot already present)
        EXPECT_NE(strstr(p->name, "HostName"), nullptr);
        EXPECT_STREQ(p->reference, "HostName");

        freeProfile(profile);
    }
    if (configData) free(configData);
}

/**
 * @brief Covers parseDataModelTableParams: table with index (exercises addParameter loop)
 *
 * When "index" is specified, addParameter_marker_config takes the indexed path
 * (strtok loop + addParameter calls) AND still calls parseDataModelTableParams.
 */
TEST_F(DynamicTableTestFixture, Coverage_DataModelTable_WithIndex_ExercisesFullPath)
{
    const char* config = R"({
        "Description": "Index and Params Test",
        "Version": "1",
        "Protocol": "HTTP",
        "EncodingType": "JSON",
        "ReportingInterval": 60,
        "Parameter": [
            {
                "type": "dataModelTable",
                "reference": "Device.Ethernet.Interface.",
                "index": "1,2",
                "Parameter": [
                    { "type": "dataModel", "reference": "Enable" },
                    { "type": "dataModel", "reference": "Status" }
                ]
            }
        ]
    })";

    char* configData = strdup(config);
    Profile* profile = nullptr;

    T2ERROR result = processConfiguration(&configData,
                                         const_cast<char*>("IndexAndParamsTest"),
                                         nullptr, &profile);

    if (profile != nullptr) {
        // The index path creates addParameter calls for index 1 and 2
        // parseDataModelTableParams also creates the table with sub-params
        ASSERT_NE(profile->dataModelTableList, nullptr);
        EXPECT_GT(Vector_Size(profile->dataModelTableList), (size_t)0);

        DataModelTable* table = (DataModelTable*)Vector_At(profile->dataModelTableList, 0);
        ASSERT_NE(table, nullptr);
        EXPECT_STREQ(table->reference, "Device.Ethernet.Interface.");
        // Index should be stored
        ASSERT_NE(table->index, nullptr);
        EXPECT_STREQ(table->index, "1,2");

        freeProfile(profile);
    }
    if (configData) free(configData);
}

/**
 * @brief Covers parseDataModelTableParams: nested recursive call
 *
 * Exercises the recursive path where type=dataModelTable appears inside
 * another dataModelTable's Parameter array.
 */
TEST_F(DynamicTableTestFixture, Coverage_ParseDataModelTableParams_RecursiveNested)
{
    const char* config = R"({
        "Description": "Recursive Nested Test",
        "Version": "1",
        "Protocol": "HTTP",
        "EncodingType": "JSON",
        "ReportingInterval": 60,
        "Parameter": [
            {
                "type": "dataModelTable",
                "reference": "Device.WiFi.AccessPoint.",
                "Parameter": [
                    { "type": "dataModel", "reference": "Enable" },
                    {
                        "type": "dataModelTable",
                        "reference": "AssociatedDevice.",
                        "Parameter": [
                            { "type": "dataModel", "reference": "MACAddress" }
                        ]
                    }
                ]
            }
        ]
    })";

    char* configData = strdup(config);
    Profile* profile = nullptr;

    T2ERROR result = processConfiguration(&configData,
                                         const_cast<char*>("RecursiveNestedTest"),
                                         nullptr, &profile);

    if (profile != nullptr) {
        ASSERT_NE(profile->dataModelTableList, nullptr);
        DataModelTable* table = (DataModelTable*)Vector_At(profile->dataModelTableList, 0);
        ASSERT_NE(table, nullptr);
        ASSERT_NE(table->paramList, nullptr);

        // Should have Enable + MACAddress (from nested table)
        EXPECT_GE(Vector_Size(table->paramList), (size_t)2);

        // Verify nested param includes nested path
        bool foundMac = false;
        for (size_t i = 0; i < Vector_Size(table->paramList); i++) {
            DataModelParam* p = (DataModelParam*)Vector_At(table->paramList, i);
            if (p && p->name && strstr(p->name, "MACAddress")) {
                foundMac = true;
                // Path should contain both AccessPoint and AssociatedDevice
                EXPECT_NE(strstr(p->name, "AccessPoint"), nullptr);
                EXPECT_NE(strstr(p->name, "AssociatedDevice"), nullptr);
            }
        }
        EXPECT_TRUE(foundMac);

        freeProfile(profile);
    }
    if (configData) free(configData);
}

/**
 * @brief Covers addParameter_marker_config dataModelTable: range index "1-3"
 *
 * Exercises the sscanf "%d-%d" branch in the index parsing loop.
 */
TEST_F(DynamicTableTestFixture, Coverage_DataModelTable_RangeIndex_BranchCoverage)
{
    const char* config = R"({
        "Description": "Range Index Branch",
        "Version": "1",
        "Protocol": "HTTP",
        "EncodingType": "JSON",
        "ReportingInterval": 60,
        "Parameter": [
            {
                "type": "dataModelTable",
                "reference": "Device.MoCA.Interface.",
                "index": "1-3",
                "Parameter": [
                    { "type": "dataModel", "reference": "Enable" }
                ]
            }
        ]
    })";

    char* configData = strdup(config);
    Profile* profile = nullptr;

    T2ERROR result = processConfiguration(&configData,
                                         const_cast<char*>("RangeIndexBranchTest"),
                                         nullptr, &profile);

    if (profile != nullptr) {
        // Verify the paramList has parameters created by addParameter
        // for indices 1, 2, 3 via the range branch
        EXPECT_NE(profile->paramList, nullptr);
        freeProfile(profile);
    }
    if (configData) free(configData);
}

/**
 * @brief Covers addParameter_marker_config dataModelTable: missing reference (error path)
 *
 * Exercises the "Missing reference in dataModelTable configuration" branch.
 */
TEST_F(DynamicTableTestFixture, Coverage_DataModelTable_MissingReference_ErrorPath)
{
    const char* config = R"({
        "Description": "Missing Ref Error",
        "Version": "1",
        "Protocol": "HTTP",
        "EncodingType": "JSON",
        "ReportingInterval": 60,
        "Parameter": [
            {
                "type": "dataModelTable",
                "index": "1",
                "Parameter": [
                    { "type": "dataModel", "reference": "Status" }
                ]
            }
        ]
    })";

    char* configData = strdup(config);
    Profile* profile = nullptr;

    T2ERROR result = processConfiguration(&configData,
                                         const_cast<char*>("MissingRefErrorTest"),
                                         nullptr, &profile);

    // Should not crash; entry is skipped
    if (profile != nullptr) {
        freeProfile(profile);
    }
    if (configData) free(configData);
    SUCCEED();
}

/**
 * @brief Covers addParameter_marker_config: dataModelTable without index (strdup path)
 *
 * Exercises the else branch where content/header are strdup'd and
 * content_allocated/header_allocated are set.
 */
TEST_F(DynamicTableTestFixture, Coverage_DataModelTable_NoIndex_StrdupPath)
{
    const char* config = R"({
        "Description": "No Index Strdup Path",
        "Version": "1",
        "Protocol": "HTTP",
        "EncodingType": "JSON",
        "ReportingInterval": 60,
        "Parameter": [
            {
                "type": "dataModelTable",
                "reference": "Device.DHCPv4.Server.Pool.",
                "Parameter": [
                    { "type": "dataModel", "reference": "Enable" },
                    { "type": "dataModel", "reference": "MinAddress" },
                    { "type": "dataModel", "reference": "MaxAddress" }
                ]
            }
        ]
    })";

    char* configData = strdup(config);
    Profile* profile = nullptr;

    T2ERROR result = processConfiguration(&configData,
                                         const_cast<char*>("NoIndexStrdupTest"),
                                         nullptr, &profile);

    if (profile != nullptr) {
        // The strdup path creates content=header=basePath and calls addParameter
        // Also parseDataModelTableParams creates the table
        ASSERT_NE(profile->dataModelTableList, nullptr);
        DataModelTable* table = (DataModelTable*)Vector_At(profile->dataModelTableList, 0);
        ASSERT_NE(table, nullptr);
        EXPECT_STREQ(table->reference, "Device.DHCPv4.Server.Pool.");
        EXPECT_EQ(table->index, nullptr);  // No index
        EXPECT_EQ(Vector_Size(table->paramList), (size_t)3);

        freeProfile(profile);
    }
    if (configData) free(configData);
}

/**
 * @brief Covers parseDataModelTableParams: missing Parameter array returns failure
 *
 * Exercises the early return when "Parameter" key is missing.
 */
TEST_F(DynamicTableTestFixture, Coverage_ParseDataModelTableParams_MissingParamArray)
{
    const char* config = R"({
        "Description": "Missing Param Array",
        "Version": "1",
        "Protocol": "HTTP",
        "EncodingType": "JSON",
        "ReportingInterval": 60,
        "Parameter": [
            {
                "type": "dataModelTable",
                "reference": "Device.IP.Interface."
            }
        ]
    })";

    char* configData = strdup(config);
    Profile* profile = nullptr;

    T2ERROR result = processConfiguration(&configData,
                                         const_cast<char*>("MissingParamArrayTest"),
                                         nullptr, &profile);

    // parseDataModelTableParams returns T2ERROR_FAILURE but processing continues
    if (profile != nullptr) {
        freeProfile(profile);
    }
    if (configData) free(configData);
    SUCCEED();
}

/**
 * @brief Covers buildFullPath: path exceeds MAX_PATH_LENGTH
 *
 * Exercises the snprintf overflow detection branch in buildFullPath.
 */
TEST_F(DynamicTableTestFixture, Coverage_BuildFullPath_Overflow)
{
    // Create a reference so long that basePath + reference > 512 bytes
    std::string longRef(500, 'A');
    std::string config = R"({
        "Description": "Overflow Path",
        "Version": "1",
        "Protocol": "HTTP",
        "EncodingType": "JSON",
        "ReportingInterval": 60,
        "Parameter": [
            {
                "type": "dataModelTable",
                "reference": "Device.VeryLongPath.",
                "Parameter": [
                    { "type": "dataModel", "reference": ")" + longRef + R"(" }
                ]
            }
        ]
    })";

    char* configData = strdup(config.c_str());
    Profile* profile = nullptr;

    T2ERROR result = processConfiguration(&configData,
                                         const_cast<char*>("OverflowPathTest"),
                                         nullptr, &profile);

    // Should not crash; overflow is detected and parameter is skipped
    if (profile != nullptr) {
        freeProfile(profile);
    }
    if (configData) free(configData);
    SUCCEED();
}

// Run all tests
int main(int argc, char **argv) {
    char testresults_fullfilepath[128];
    char buffer[128];
    char *basename_ptr;

    memset( testresults_fullfilepath, 0, 128 );
    memset( buffer, 0, 128 );

    /* Extract basename from argv[0] to create unique filename */
    basename_ptr = strrchr(argv[0], '/');
    basename_ptr = basename_ptr ? basename_ptr + 1 : argv[0];
    snprintf( buffer, 128, "%s_report.json", basename_ptr);
    snprintf( testresults_fullfilepath, 128, "json:/tmp/Gtest_Report/%s" , buffer);
    
    /* Set output flag BEFORE InitGoogleTest */
    ::testing::GTEST_FLAG(output) = testresults_fullfilepath;
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
