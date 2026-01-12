# Unit Test Coverage Expansion - AI Assistant Prompt

## Document Information

| Field | Value |
|-------|-------|
| **Purpose** | Reusable prompt for AI assistants to expand unit test coverage |
| **Version** | 1.0 |
| **Date** | January 2026 |
| **Audience** | AI Coding Assistants (Cascade, Copilot, etc.) |

---

## Objective

Expand unit test coverage for a specified source file or module by adding new test cases that are **simple, reliable, and regression-free**. Tests must execute successfully without errors and provide measurable coverage improvement.

---

## Core Principles

### 1. **Simplicity First**
- Write straightforward tests that are easy to understand and maintain
- Avoid complex mock setups that are fragile or hard to debug
- Prefer testing through public APIs rather than internal implementation details
- Keep each test focused on a single behavior or code path

### 2. **Reliability**
- Tests must pass consistently without flakiness
- No segmentation faults, memory leaks, or undefined behavior
- Proper mock expectations that match actual code execution
- Clean setup and teardown for each test

### 3. **No Regressions**
- Never modify or weaken existing tests without explicit approval
- Ensure new tests don't interfere with existing test suite
- Verify all tests pass before and after changes
- Maintain existing code style and patterns

### 4. **Measurable Impact**
- Always verify coverage improvement with actual metrics
- Document baseline and post-implementation coverage percentages
- Focus on functions/lines with 0% coverage first
- Target meaningful coverage gains (not just trivial lines)

---

## Step-by-Step Process

### Phase 1: Analysis (REQUIRED)

1. **Identify Current Coverage**
   ```bash
   # Run existing tests with coverage
   sh test/run_ut.sh --enable-cov
   
   # Check coverage report
   cat coverage_report/[module]/[file].func.html
   ```

2. **Analyze Uncovered Code**
   - List all functions with 0 hits
   - Identify uncovered branches and error paths
   - Note functions with partial coverage (<50%)
   - Document any static/private functions

3. **Assess Testability**
   - **Easy**: Public functions with simple dependencies
   - **Medium**: Functions requiring mock setup
   - **Hard**: Static functions, complex state machines, threading
   - **Skip**: Functions requiring source code refactoring

4. **Create Implementation Plan**
   - Prioritize by: testability (easy first) → impact (high coverage gain)
   - Group related test cases together
   - Estimate coverage improvement per phase
   - Document any blockers or challenges

### Phase 2: Test Design

1. **Study Existing Test Patterns**
   ```cpp
   // Review existing test file structure
   // - How are mocks initialized?
   // - What's the typical test fixture setup?
   // - How are assertions structured?
   // - What naming conventions are used?
   ```

2. **Design Test Cases**
   For each target function, create tests for:
   - **Happy path**: Normal execution with valid inputs
   - **Null/invalid inputs**: Boundary conditions
   - **Error conditions**: Expected failure modes
   - **Edge cases**: Empty lists, zero values, limits

3. **Mock Strategy**
   - Use existing mocks whenever possible
   - Keep mock expectations minimal and flexible
   - Use `::testing::AtLeast(1)` for non-critical call counts
   - Use `::testing::AtMost(N)` to prevent over-mocking
   - Avoid `EXPECT_CALL(...).Times(::testing::Exactly(N))` unless critical

### Phase 3: Implementation

1. **Follow Existing Patterns**
   ```cpp
   // Match existing test structure
   TEST_F(ExistingFixture, NewTestName_Scenario) {
       // Setup - use existing mock patterns
       EXPECT_CALL(*g_mockObj, method(_))
           .Times(::testing::AtLeast(1))
           .WillRepeatedly(Return(expected_value));
       
       // Execute
       T2ERROR result = functionUnderTest(params);
       
       // Verify
       EXPECT_EQ(result, T2ERROR_SUCCESS);
   }
   ```

2. **Start Simple**
   - Begin with the easiest test case
   - Verify it compiles and passes
   - Incrementally add complexity
   - Test after each addition

3. **Handle Static Functions**
   - Test indirectly through public APIs that call them
   - Document if direct testing is not feasible
   - Consider if the coverage gain justifies complexity
   - **Do NOT modify source code** to make functions testable

4. **Avoid Common Pitfalls**
   - ❌ Don't create tests that only mock without executing real code
   - ❌ Don't use overly specific mock expectations that break easily
   - ❌ Don't test implementation details instead of behavior
   - ✅ Do suggest tests that becomes easier to write with only minor source code changes
   - ✅ Do verify tests actually execute the target code path
   - ✅ Do use flexible mock expectations
   - ✅ Do test observable behavior
   - ✅ Do work within existing architecture

### Phase 4: Verification

1. **Compile and Run**
   ```bash
   # Build tests
   cd source/test/[module] && make clean && make [test_binary]
   
   # Run specific tests
   ./[test_binary] --gtest_filter="*NewTest*"
   
   # Run full suite with coverage
   sh test/run_ut.sh --enable-cov
   ```

2. **Verify Coverage Improvement**
   ```bash
   # Check function coverage
   grep "target_function" coverage_report/[module]/[file].func.html
   
   # Compare line coverage percentages
   # Before: X.X%
   # After: Y.Y%
   # Improvement: +Z.Z%
   ```

3. **Check for Regressions**
   - All existing tests still pass
   - No new warnings or errors
   - No memory leaks (if using valgrind)
   - Coverage didn't decrease for any file

4. **Document Results**
   - List functions now covered
   - Show before/after coverage metrics
   - Note any functions that remain uncovered and why
   - Provide recommendations for next phase

---

## Test Quality Checklist

Before submitting tests, verify:

- [ ] Tests compile without errors or warnings
- [ ] All new tests pass consistently
- [ ] All existing tests still pass
- [ ] Coverage has measurably improved
- [ ] Test names clearly describe what they test
- [ ] Tests follow existing code style and patterns
- [ ] Mock expectations are reasonable and flexible
- [ ] No source code modifications were made
- [ ] Tests are independent (no test order dependencies)
- [ ] Setup and teardown are properly handled
- [ ] No memory leaks or resource leaks
- [ ] Tests execute in reasonable time (<1s each typically)

---

## Example Prompt Usage

### For AI Assistant:

```
Task: Improve unit test coverage for source/bulkdata/profile.c

Current Status:
- Line coverage: 24.6% (240/976 lines)
- Function coverage: 79.3% (23/29 functions)
- Target: 80%+ line coverage

Instructions:
1. Follow the process in .github/instructions/unit-test-coverage-expansion-prompt.md
2. Start with Phase 1 analysis
3. Focus on functions with 0 hits first
4. Create simple, reliable tests
5. Verify coverage improvement after each phase
6. Do not modify source code. Suggest only minor source code changes to make tests easier to write.

Constraints:
- Use existing test infrastructure in source/test/bulkdata/profileTest.cpp
- Follow existing mock patterns (FileMock, SystemMock, VectorMock, etc.)
- Tests must pass without errors
- No source code changes allowed
```

---

## Common Patterns

### Pattern 1: Testing Public Functions with Mocks

```cpp
TEST_F(ModuleTest, FunctionName_Scenario) {
    // Setup mocks
    EXPECT_CALL(*g_mock, dependency_call(_))
        .Times(::testing::AtLeast(1))
        .WillRepeatedly(Return(SUCCESS));
    
    // Execute
    result = functionUnderTest(valid_input);
    
    // Verify
    EXPECT_EQ(result, EXPECTED_VALUE);
}
```

### Pattern 2: Testing Error Paths

```cpp
TEST_F(ModuleTest, FunctionName_NullInput) {
    result = functionUnderTest(nullptr);
    EXPECT_EQ(result, ERROR_INVALID_ARGS);
}

TEST_F(ModuleTest, FunctionName_NotInitialized) {
    // Don't call init
    result = functionUnderTest(valid_input);
    EXPECT_EQ(result, ERROR_NOT_INITIALIZED);
}
```

### Pattern 3: Testing Static Functions Indirectly

```cpp
// Static function: static void cleanupResource(void* data)
// Called by: publicFunction() -> Vector_Destroy(list, cleanupResource)

TEST_F(ModuleTest, PublicFunction_TriggersCleanup) {
    // Setup: Create data that will be cleaned up
    setupDataStructure();
    
    // Mock Vector_Destroy to actually call cleanup callback
    EXPECT_CALL(*g_vectorMock, Vector_Destroy(_, _))
        .WillOnce(::testing::Invoke([](Vector* v, void (*cleanup)(void*)) {
            if (cleanup) {
                cleanup(test_data); // This calls static cleanupResource
            }
            return SUCCESS;
        }));
    
    // Execute public function that triggers cleanup
    publicFunction();
    
    // Verify cleanup occurred (check side effects)
}
```

### Pattern 4: Testing with Real Operations

```cpp
TEST_F(ModuleTest, FunctionName_Simple) {
    // Minimal mocking - let real code execute
    EXPECT_CALL(*g_mock, unavoidable_dependency())
        .Times(::testing::AtMost(1))
        .WillRepeatedly(Return(0));
    
    // Execute - most code runs for real
    result = functionUnderTest();
    
    // Verify
    EXPECT_NE(result, nullptr);
}
```

---

## Troubleshooting Guide

### Issue: Tests compile but don't improve coverage

**Cause**: Mocks are intercepting calls before real code executes

**Solution**:
- Review mock expectations - are they too broad?
- Check if you're mocking the function you're trying to test
- Verify test actually calls the target function
- Use coverage report to see which lines executed

### Issue: Segmentation faults

**Cause**: Uninitialized pointers, mock mismatches, or missing setup

**Solution**:
- Initialize all pointers and structures
- Ensure mock expectations match actual calls
- Check existing tests for proper initialization patterns
- Use valgrind to identify the exact issue

### Issue: Tests fail with mock expectation errors

**Cause**: Overly specific or incorrect mock expectations

**Solution**:
- Use `::testing::AtLeast(1)` instead of exact counts
- Review actual code to see what it really calls
- Check if function behavior changed
- Simplify mock setup

### Issue: Can't test static function

**Cause**: Function is not exposed outside the file

**Solution**:
- Test indirectly through public APIs
- Document as untestable without refactoring
- Focus on other functions with better testability
- Consider if coverage gain justifies complexity

---

## Success Metrics

A successful coverage expansion should achieve:

- ✅ **Measurable improvement**: +5% or more line coverage
- ✅ **Function coverage**: At least 2-3 new functions covered
- ✅ **Zero regressions**: All existing tests pass
- ✅ **Clean execution**: No errors, warnings, or crashes
- ✅ **Maintainable tests**: Clear, simple, following existing patterns
- ✅ **Documented**: Before/after metrics and remaining gaps noted

---

## Anti-Patterns to Avoid

### ❌ Mock-Only Tests
```cpp
// BAD: This doesn't test real code
TEST_F(Test, Bad) {
    EXPECT_CALL(*mock, everything()).WillRepeatedly(Return(0));
    function(); // Real code never executes
}
```

### ❌ Overly Specific Expectations
```cpp
// BAD: Brittle, breaks easily
EXPECT_CALL(*mock, func(_)).Times(::testing::Exactly(3));
```

### ❌ Testing Implementation Details
```cpp
// BAD: Tests how, not what
TEST_F(Test, Bad) {
    EXPECT_CALL(*mock, internal_helper()).Times(1);
    publicFunction();
}
```

---

## Best Practices

### ✅ Test Behavior, Not Implementation
```cpp
// GOOD: Tests observable outcome
TEST_F(Test, Good) {
    result = processData(input);
    EXPECT_EQ(result->status, SUCCESS);
    EXPECT_GT(result->count, 0);
}
```

### ✅ Flexible Mock Expectations
```cpp
// GOOD: Resilient to minor changes
EXPECT_CALL(*mock, func(_))
    .Times(::testing::AtLeast(1))
    .WillRepeatedly(Return(SUCCESS));
```

### ✅ Clear Test Names
```cpp
// GOOD: Name describes scenario
TEST_F(ProfileTest, AddProfile_WithValidData_ReturnsSuccess)
TEST_F(ProfileTest, AddProfile_WhenNotInitialized_ReturnsError)
TEST_F(ProfileTest, AddProfile_WithNullName_ReturnsInvalidArgs)
```

### ✅ Independent Tests
```cpp
// GOOD: Each test is self-contained
TEST_F(Test, Scenario1) {
    setup();
    test();
    verify();
    cleanup();
}
```

---

## Reporting Template

After completing coverage expansion, report using this template:

```markdown
## Coverage Improvement Report

### Module: [module_name]
### File: [source_file]

### Coverage Metrics

| Metric | Before | After | Change |
|--------|--------|-------|--------|
| Line Coverage | X.X% (A/B) | Y.Y% (C/D) | +Z.Z% |
| Function Coverage | X.X% (E/F) | Y.Y% (G/H) | +Z.Z% |

### Newly Covered Functions

✅ `function1` - Description
✅ `function2` - Description
✅ `function3` - Description

### Remaining Uncovered Functions

❌ `function4` - Reason (e.g., static, complex threading, requires refactoring)
❌ `function5` - Reason

### Tests Added

- Test file: [test_file]
- Number of new tests: N
- Lines of test code added: M

### Recommendations

- Next phase should target: [functions or areas]
- Consider: [any suggestions for improving testability]
- Blockers: [any issues preventing further coverage]
```

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | January 2026 | Initial version |

---

**Document maintained by**: Development Team  
**Last reviewed**: January 12, 2026  
**Status**: Active
