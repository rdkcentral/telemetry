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

## Building and Running Tests

### Prerequisites
```bash
# Install Docker and GHCR authentication (if needed)
# Ensure you have access to ghcr.io/rdkcentral/docker-device-mgt-service-test/native-platform:latest
docker login ghcr.io -u <username> -p <personal-access-token>
docker pull ghcr.io/rdkcentral/docker-device-mgt-service-test/native-platform:latest
# Run container with workspace mounted
docker run -it --rm -v $(pwd):/mnt/workspace ghcr.io/rdkcentral/docker-device-mgt-service-test/native-platform:latest bash

```

### Build Tests
```bash
# From repository root
cd /mnt/workspace
autoreconf -i
./configure --enable-tests
make check
```

### Run Specific PR-363 Tests
```bash
# Run t2parser PR-363 tests
./source/test/t2parser/t2parser_dynamictable_Test

# Run reportgen PR-363 tests
./source/test/reportgen/reportgen_dynamictable_Test

# Run profile PR-363 tests
./source/test/bulkdata/profile_dynamictable_Test

# Run all tests with verbose output
./source/test/t2parser/t2parser_dynamictable_Test --gtest_verbose
```

### Memory Leak Detection (Recommended)
```bash
# Run under valgrind to detect memory leaks
valgrind --leak-check=full --show-leak-kinds=all \
    ./source/test/t2parser/t2parser_dynamictable_Test

valgrind --leak-check=full --show-leak-kinds=all \
    ./source/test/reportgen/reportgen_dynamictable_Test

valgrind --leak-check=full --show-leak-kinds=all \
    ./source/test/bulkdata/profile_dynamictable_Test
```

## Test Execution Checklist

Before merging PR-363, ensure:

- [ ] All PR-363 tests pass (100% pass rate)
- [ ] No memory leaks detected by valgrind
- [ ] No buffer overflows detected (ASan/valgrind)
- [ ] Existing regression tests still pass
- [ ] Code coverage for new code paths >= 80%

## Continuous Integration

These tests are integrated into the CI/CD pipeline:

1. **L1 Unit Tests**: Run all PR-363 tests in test container
2. **Valgrind Checks**: Detect memory leaks in critical paths
3. **Coverage Report**: Ensure new code is adequately tested

## Test Maintenance

### Adding New Tests

When adding new memory safety features:

1. Add test cases to the appropriate PR-363 test file
2. Follow existing naming convention: `Feature_Scenario_ExpectedBehavior`
3. Document what PR issue/line numbers the test validates
4. Include both positive and negative test cases
5. Add valgrind verification for memory-related tests

### Updating Tests

When modifying PR-363 code:

1. Update corresponding tests to reflect changes
2. Ensure backward compatibility tests still pass
3. Add regression tests if bugs are found
4. Update this README with new test coverage

## Known Limitations

1. Random failures when executed from github environment (potentially due to resource constraints or timing issues)
2. Some edge cases may not be fully covered 
3. Tests may need updates if underlying implementation changes significantly
   

## References

- **PR-161**: TELEMETRY-2: Dynamic telemetry markers for Dynamic Tables in Telemetry
  - Introduced DataModelTable type
  - Added index parameter support
  - Implemented dynamic JSON encoding
  - Enhanced parameter filtering with wildcards
  
- **PR-363**: Fix memory safety issues blocking PR-161 L1 tests
  - Fixed Coverity BAD_FREE defect (CWE-590)
  - Added buffer overflow protection (concatenatedKey)
  - Implemented proper error path cleanup
  - Added NULL safety checks

- **Original Issue**: PR-161 L1 test crashes with exit code 134 (SIGABRT)
- **Root Cause**: Memory corruption from freeing cJSON internal pointers and buffer overflows

