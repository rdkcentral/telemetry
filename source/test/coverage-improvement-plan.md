# Unit Test Coverage Improvement Plan

## Current Status

**Overall Coverage**: 56.6% lines (5370/9490), 84.0% functions (274/326)  
**Target**: 80%+ line coverage  
**Date**: January 12, 2026

## Coverage Analysis by Module

### Critical Priority (Below 50% Line Coverage)

#### 1. Protocol/HTTP Module
- **Current**: 28.8% lines (59/205), 28.6% functions (2/7)
- **File**: `protocol/http/curlinterface.c`
- **Gap**: 146 uncovered lines, 5 uncovered functions
- **Impact**: High - critical for telemetry data upload

#### 2. XConf Client Module  
- **Current**: 21.2% lines (124/585), 41.7% functions (5/12)
- **File**: `xconf-client/xconfclient.c`
- **Gap**: 461 uncovered lines, 7 uncovered functions
- **Impact**: High - essential for configuration management

#### 3. Bulkdata Profile Module
- **Current**: 24.6% lines (240/976), 79.3% functions (23/29)
- **File**: `bulkdata/profile.c`
- **Gap**: 736 uncovered lines, 6 uncovered functions
- **Impact**: High - core profile management functionality

#### 4. Bulkdata Report Profiles Module
- **Current**: 24.1% lines (141/586), 54.2% functions (13/24)
- **File**: `bulkdata/reportprofiles.c`
- **Gap**: 445 uncovered lines, 11 uncovered functions
- **Impact**: High - central report coordination

#### 5. Report Generator Module
- **Current**: 46.4% lines (331/714), 73.3% functions (11/15)
- **Files**: Multiple files in `reportgen/`
- **Gap**: 383 uncovered lines, 4 uncovered functions
- **Impact**: Medium - report formatting and generation

#### 6. Common Library Module
- **Current**: 41.1% lines (158/384), 84.2% functions (16/19)
- **Gap**: 226 uncovered lines, 3 uncovered functions
- **Impact**: Medium - shared utilities

#### 7. Profile XConf Module
- **Current**: 40.0% lines (207/517), 75.0% functions (12/16)
- **File**: `bulkdata/profilexconf.c`
- **Gap**: 310 uncovered lines, 4 uncovered functions
- **Impact**: Medium - legacy XConf profile support

## Implementation Strategy

### Phase 1: Quick Wins (Target: +15% coverage)

**Focus**: Functions with 0 hits that have simple test patterns

#### Bulkdata Profile Module (`profile.c`)
**Uncovered Functions (0 hits)**:
- `CollectAndReport` - Core reporting function
- `freeProfile` - Memory cleanup
- `freeReportProfileConfig` - Config cleanup  
- `freeRequestURIparam` - URI parameter cleanup
- `getMarkerCompRbusSub` - RBUS subscription
- `initJSONReportProfile` - JSON profile initialization

**Test Approach**:
- Use existing `ProfileTest` fixture in `source/test/bulkdata/profileTest.cpp`
- Mock dependencies: FileMock, SystemMock, VectorMock, RbusMock
- Test error paths and null input validation
- Test memory cleanup functions indirectly through profile lifecycle

#### Bulkdata Report Profiles Module (`reportprofiles.c`)
**Uncovered Functions (0 hits)**:
- `ReportProfiles_ProcessReportProfilesBlob` - JSON blob processing
- `ReportProfiles_ProcessReportProfilesMsgPackBlob` - MessagePack processing
- `ReportProfiles_deleteProfileXConf` - XConf profile deletion
- `ReportProfiles_setProfileXConf` - XConf profile setting
- `ReportProfiles_uninit` - Module cleanup
- `deleteAllReportProfiles` - Bulk deletion
- `freeProfilesHashMap` - HashMap cleanup
- `freeReportProfileHashMap` - Report HashMap cleanup
- `reportOnDemand` - On-demand reporting
- `__ReportProfiles_ProcessReportProfilesMsgPackBlob` - Internal MessagePack processing
- `__msgpack_free_blob` - MessagePack memory cleanup

**Test Approach**:
- Extend existing tests in `source/test/bulkdata/`
- Mock JSON/MessagePack parsing
- Test blob processing with valid/invalid data
- Test cleanup functions through initialization/shutdown cycles

### Phase 2: Protocol and XConf Modules (Target: +20% coverage)

#### Protocol HTTP Module (`curlinterface.c`)
**Uncovered Functions**: 5 out of 7 functions (71% uncovered)

**Test Approach**:
- Create new test file: `source/test/protocol/http/curlinterfaceTest.cpp`
- Mock CURL operations
- Test HTTP upload scenarios
- Test error handling and retry logic
- Test MTLS certificate handling

#### XConf Client Module (`xconfclient.c`)
**Uncovered Functions**: 7 out of 12 functions (58% uncovered)

**Test Approach**:
- Extend existing tests in `source/test/xconf-client/`
- Mock HTTP client operations
- Test configuration retrieval
- Test error scenarios and timeouts
- Test configuration parsing and validation

### Phase 3: Report Generation and Utilities (Target: +10% coverage)

#### Report Generator Module
**Test Approach**:
- Extend existing tests in `source/test/reportgen/`
- Test different output formats (JSON, MessagePack, XML, CSV)
- Test report assembly and formatting
- Test large data handling

#### Common Library Module
**Test Approach**:
- Add tests for utility functions
- Test string manipulation and validation
- Test file operations and error handling

## Test Implementation Guidelines

### Mock Strategy
- **Reuse existing mocks**: FileMock, SystemMock, VectorMock, RbusMock, CurlMock
- **Flexible expectations**: Use `::testing::AtLeast(1)` instead of exact counts
- **Error injection**: Test failure paths by mocking dependency failures

### Test Patterns
```cpp
// Pattern 1: Basic function test
TEST_F(ModuleTest, FunctionName_ValidInput_ReturnsSuccess) {
    EXPECT_CALL(*g_mock, dependency(_))
        .Times(::testing::AtLeast(1))
        .WillRepeatedly(Return(SUCCESS));
    
    T2ERROR result = functionUnderTest(valid_input);
    EXPECT_EQ(result, T2ERROR_SUCCESS);
}

// Pattern 2: Error path test
TEST_F(ModuleTest, FunctionName_NullInput_ReturnsError) {
    T2ERROR result = functionUnderTest(nullptr);
    EXPECT_EQ(result, T2ERROR_INVALID_ARGS);
}

// Pattern 3: Cleanup function test (indirect)
TEST_F(ModuleTest, InitUninit_ProperCleanup_NoMemoryLeaks) {
    initFunction();
    // Verify initialization
    uninitFunction(); // This should call cleanup functions
    // Verify cleanup occurred
}
```

### Test Organization
- **File Structure**: Mirror source directory structure in `source/test/`
- **Naming Convention**: `[module]Test.cpp` for test files
- **Test Names**: `FunctionName_Scenario_ExpectedResult`
- **Fixtures**: Reuse existing test fixtures where possible

## Success Metrics

### Phase 1 Target (4-6 weeks)
- **Bulkdata modules**: 50%+ line coverage (currently 37.9%)
- **New functions covered**: 15-20 functions
- **Line coverage improvement**: +8-10%

### Phase 2 Target (6-8 weeks)
- **Protocol/HTTP**: 60%+ line coverage (currently 28.8%)
- **XConf Client**: 50%+ line coverage (currently 21.2%)
- **Line coverage improvement**: +15-20%

### Phase 3 Target (8-10 weeks)
- **Overall project**: 80%+ line coverage (currently 56.6%)
- **All modules**: Minimum 60% line coverage
- **Function coverage**: Maintain 85%+ (currently 84.0%)

## Risk Assesment

### High-Risk Areas
1. **Threading code**: Profile report threads, event dispatcher
2. **Static functions**: Test indirectly through public APIs
3. **Complex state machines**: Profile lifecycle management
4. **External dependencies**: CURL, RBUS, file system operations

### Mitigation Strategies
- **Threading**: Use synchronization mocks and controlled test environments
- **Static functions**: Document as tested indirectly, focus on observable behavior
- **State machines**: Test state transitions through public API calls
- **Dependencies**: Comprehensive mocking with error injection

## Challenges

### Identified Challenges
1. **Complex mock setups**: Some functions require extensive dependency mocking
2. **Static function access**: Several utility functions are static
3. **Threading complexity**: Event dispatcher and report generation threads
4. **Legacy code patterns**: Some modules use older C patterns

### Recommended Solutions
1. **Incremental approach**: Start with simplest functions, build complexity gradually
2. **Indirect testing**: Test static functions through public API calls
3. **Mock threading**: Use controlled threading mocks for predictable tests
4. **Pattern consistency**: Follow existing test patterns in the codebase

## Next Steps

1. **Phase 1 Implementation**: Start with uncovered functions
2. **Test Infrastructure**: Ensure all required mocks are available
3. **Baseline Measurement**: Run coverage before starting each phase
4. **Continuous Validation**: Run full test suite after each function addition
5. **Documentation**: Update this plan based on implementation findings

---

**Document Status**: Active  
**Last Updated**: January 12, 2026  
**Next Review**: After Phase 1 completion
