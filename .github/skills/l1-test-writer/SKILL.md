---
name: l1-test-writer
description: Write L1 gtest/gmock unit tests for a telemetry C function in this repository, following the module's existing fixtures and the mocks in source/test/mocks. Use when adding L1 tests, covering NULL, error and boundary paths, writing a regression test for a bug fix, or extending a module test suite.
---

# L1 Test Writer

Writes tests for one function at a time, mirroring the repository's existing test code. Rules in `AGENTS.md` (section 3 and 4) apply; this skill is the procedure.

## Before writing

1. Read the production function and its header. List every branch, `T2ERROR_*` return, NULL check, allocation, and external call.
2. Find the module's test directory in `test/l1_modules.txt` and read the existing suite for that module (fixture class, mock pointers, includes, helper functions, naming).
3. Read the mock headers the suite already includes. Mock catalog and recipes: [mock-recipes.md](mock-recipes.md).
4. Check what the target already covers (`test/l1_gaps.sh`, skill `l1-gap-analyzer`) so you only add what is missing.

## Where to put the test

- Add to the module's existing test file and fixture. Create a new file or binary only for a new source file under test.
- A new file or `*_gtest.bin` is not built or run until it is registered: follow skill `l1-register-binary`.

## Fixture pattern (mirror the module's own)

Global mock pointers are defined in the test file and set in `SetUp()`:

```cpp
extern "C" {
#include "scheduler/scheduler.h"
}
#include "../mocks/SystemMock.h"
#include "../mocks/rdklogMock.h"

SystemMock* g_systemMock = nullptr;
rdklogMock* m_rdklogMock = nullptr;

class schedulerTestFixture : public ::testing::Test {
protected:
    void SetUp() override    { g_systemMock = new SystemMock(); }
    void TearDown() override { delete g_systemMock; g_systemMock = nullptr; }
};

TEST_F(schedulerTestFixture, registerProfileWithScheduler_NullName_ReturnsInvalidArgs)
{
    EXPECT_EQ(T2ERROR_INVALID_ARGS,
              registerProfileWithScheduler(NULL, 10, 100, true, true, true, 5, (char*)"0001-01-01T00:00:00Z"));
}
```

Use the module's own style if it differs (for example a mock object as a fixture member with the pointer set in the constructor, as in `schedulerTest.cpp`). Do not define a pointer that a mock `.cpp` already defines (for example `g_vectorMock` is defined in `VectorMock.cpp`).

## What to cover for the target function

Write only cases that map to real code in the function:

- success path, with assertions on return value and on state or output
- each distinct `T2ERROR_*` return, one test each
- each pointer argument as NULL, individually
- empty string, zero length, zero count
- boundary values (max, min, off by one, buffer full)
- allocation failure, when the allocation goes through a mockable call
- failure of each checked external call (file I/O, system, rbus, vector, parser)
- cleanup behavior: NULL-safe, idempotent, no leak
- threaded code: start, stop, double stop, lock-held and no-contention paths

## Rules

- Name tests `TEST_F(<Module>TestFixture, <Function>_<Condition>_<ExpectedResult>)`. Match the file's fixture name.
- Every test asserts behavior: return code plus at least one of output, state, or mock interaction. A test that only calls the function is not acceptable.
- `ASSERT_*` when the next line would dereference the value; otherwise `EXPECT_*`.
- Wrap C headers in `extern "C"`. Do not use `printf` in new tests.
- Deterministic: no real network, no sleeps beyond a short bound, no dependence on test order, files only under `/tmp` and removed in `TearDown()`.
- Enable `g_mockClockGettime` or `g_mockSelect` only inside the test that needs them and reset them before the test ends.
- Free everything the test allocates; the binary must be clean under valgrind.
- Never change or disable an existing test. If an existing assertion is wrong because behavior changed on purpose, say so.
- Do not edit production code. If the function cannot be reached without a seam, stop and report the seam (class C in the gap analyzer).

## Finish

1. Run `test/l1_check.sh <module>` (skill `l1-test-verifier`); every gate must pass, and do the mutation check for each new test.
2. Report the tests added (name and behavior), the gate lines and coverage numbers printed by the script, and anything left uncovered with the reason.
