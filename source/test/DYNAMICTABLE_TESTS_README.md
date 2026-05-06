# PR-161 & PR-363 Comprehensive Test Suite

This directory contains comprehensive unit tests for **PR-161 (DataModelTable feature)** and **PR-363 (memory safety fixes)**.

## Background

- **PR-161**: Introduced DataModelTable type for dynamic telemetry collection from multi-instance tables
- **PR-363**: Fixed critical memory safety issues (Coverity defects, buffer overflows, memory leaks) introduced by PR-161

These tests validate both the new functionality AND the safety fixes that make it production-ready.

## Test Coverage

### 1. t2parser_dynamictable_Test.cpp
Tests for **PR-161 DataModelTable parsing** and **PR-363 memory safety fixes**.

**PR-161 Features Tested:**
- ✅ **DataModelTable parsing**: Single index, range, comma-separated, mixed formats
- ✅ **Index parameter support**: "1", "1-5", "1,3,5", "1-2,5,7-9"
- ✅ **Duplicate filtering**: "1,2,2,3,1" → processes only 1,2,3
- ✅ **Invalid index handling**: Skips negative and out-of-range values (>= 256)
- ✅ **Whitespace stripping**: " 1 , 2 - 4 " properly parsed
- ✅ **Nested parameters**: Parameters within dataModelTable
- ✅ **Wildcard collection**: DataModelTable without index collects all instances
- ✅ **Mixed parameter types**: DataModelTable with dataModel, event, grep types

**PR-363 Memory Safety Fixes Tested:**
- ✅ **Allocation tracking**: content_allocated/header_allocated flags prevent BAD_FREE
- ✅ **Parse failure cleanup**: freeProfile() + cJSON_Delete() on errors
- ✅ **Conditional vector creation**: Prevents vector double-initialization
- ✅ **Error path cleanup**: NULL checks and proper resource freeing
- ✅ **Coverity CWE-590 fix**: cJSON valuestring NOT freed, strdup allocations freed

**Critical Tests:**
- `PR161_DataModelTable_SingleIndex`: Tests index="1"
- `PR161_DataModelTable_IndexRange`: Tests index="1-5"  
- `PR161_DataModelTable_MixedIndexes`: Tests index="1-2,5,7-9"
- `PR161_DataModelTable_NoIndex_WildcardCollection`: Tests wildcard + strdup path
- `DataModelTable_WithoutIndex_ProperMemoryManagement`: Validates PR-363 strdup tracking
- `RegularDataModel_NoDoubleFree`: Validates cJSON valuestring safety (Coverity fix)

### 2. reportgen_dynamictable_Test.cpp
Tests for **PR-161 dynamic JSON encoding** and **PR-363 buffer safety**.

**PR-161 Features Tested:**
- ✅ **Nested JSON creation**: Device.WiFi.SSID.1.Name → {"WiFi":{"SSID":[{"Name":"val"}]}}
- ✅ **Array creation**: Multiple instances create JSON arrays
- ✅ **Deep nesting**: Multi-level table structures (AccessPoint.1.AssociatedDevice.2.MAC)
- ✅ **Token parsing**: strtok() splits path by '.' delimiter
- ✅ **Dynamic key building**: concatenatedKey built from tokens
- ✅ **Array index detection**: isdigit() determines numeric vs object keys
- ✅ **Wildcard matching**: matchesParameter() filters configured parameters

**PR-363 Memory Safety Fixes Tested:**
- ✅ **Buffer overflow prevention**: concatenatedKey bounds checking (256-byte limit)
- ✅ **Maximum-length parameters**: Paths >= 256 chars rejected safely
- ✅ **Boundary conditions**: 255-char paths handled correctly
- ✅ **Error path cleanup**: parameterName and parameterWild freed on errors
- ✅ **Safe strncat usage**: strncat with proper size: sizeof(buf) - strlen(buf) - 1

**Critical Tests:**
- `PR161_NestedJSONCreation_SimpleTable`: Tests basic nested object creation
- `PR161_ArrayCreation_MultipleInstances`: Tests JSON array for multiple instances
- `PR161_DeeplyNested_TableStructures`: Tests multi-level nesting without overflow
- `PR161_ConcatenatedKey_DynamicBuilding`: Tests dynamic path building with safety
- `MaxLengthParameterName_PreventBufferOverflow`: Validates PR-363 fix (>= 256 chars rejected)

### 3. profile_dynamictable_Test.cpp
Tests for **PR-161 NULL safety** and **PR-363 profile cleanup**.

**PR-161/PR-363 Integration Tested:**
- ✅ **NULL dataModelTableList handling**: Prevents crash when list is NULL
- ✅ **Empty dataModelTableList**: Validates Vector_Size(0) behavior  
- ✅ **freeProfile() public API**: Safe cleanup for parse failures
- ✅ **Thread safety contract**: Documents caller responsibilities
- ✅ **Defensive patterns**: NULL checks before Vector_Size()

**Critical Tests:**
- `NullDataModelTableList_NoCrash`: Validates PR-363 fix at profile.c:512
- `FreeProfile_ValidProfile_CleansUp`: Validates cleanup includes dataModelTableList
- `ParseFailure_ProperCleanup_Integration`: Documents expected error path behavior
