# AGENTS.md - Telemetry 2.0 (rdkcentral/telemetry)

Rules for AI coding agents working in this repository. They are binding, not advisory.
Tool-specific details live in `.github/copilot-instructions.md` and `.github/instructions/*.instructions.md`.

## 1. Project Context

- Telemetry 2.0 framework for RDK (Reference Design Kit), designed for embedded devices with constrained resources.
- **Target platform**: embedded devices (STBs, gateways, IoT devices), shipped to millions of units.
- **Resource constraints**: limited memory (often <512MB RAM) and limited CPU.
- **Languages**: C (primary, production code), C++11 (tests), POSIX shell.
- **Build system**: Autotools (`configure.ac`, `Makefile.am`).
- **Testing**: Google Test (gtest) and Google Mock (gmock). **L1** = unit tests in `source/test/`; **L2** = functional tests in `test/functional-tests/`.
- **Platform requirements**: platform-independent code supporting multiple architectures.

### Critical Requirements

1. **Memory Safety**: every allocation must have a corresponding free.
2. **Resource Constraints**: minimize memory footprint and CPU usage.
3. **Platform Independence**: avoid platform-specific code; use abstraction layers.
4. **Backward Compatibility**: maintain API/ABI compatibility.
5. **Zero Regression**: all changes must pass existing test suites.
6. **Mandatory L1 Tests**: every function, module, or bug fix ships with L1 unit tests under `source/test/` (section 2).
7. **Thread Safety**: use lightweight synchronization, prevent deadlocks, minimize memory fragmentation.
8. **Production Quality**: code ships to millions of devices.

## 2. Mandatory L1 Unit Tests (Non-Negotiable)

1. Every new function, new module, behavior change, or bug fix MUST ship with new or updated L1 tests under `source/test/` in the same change.
2. The agent MUST generate or update these tests itself, without being asked, and MUST NOT leave them as a follow-up.
3. A change without accompanying L1 tests is **non-compliant** and must not be presented as complete.
4. A bug fix MUST include a regression test that fails before the fix and passes after it.
5. Refactors must keep existing tests passing and add tests for any previously uncovered path they touch.
6. Documentation-only, comment-only, and pure build-metadata changes are the only exemptions. State the exemption explicitly in the summary.
7. If a function is untestable as written (for example, hard-wired globals or direct system calls), make the minimal seam change needed (an injectable dependency or a mockable wrapper) rather than skipping the test. Preserve public API/ABI.

### Where tests go

| Production code | L1 test location |
|---|---|
| `source/<module>/foo.c` | `source/test/<module>/<module>Test.cpp` or a `fooTest.cpp` alongside it (follow the module's existing file) |
| New module directory | New `source/test/<module>/` with its own `Makefile.am`, `gtest_main.cpp`, and mock files if needed |

Note that `source/dcautil/` is tested under `source/test/dcautils/`. Check the existing directory name before creating a new one.

### Registering new test binaries

- Add new test subdirectories to `SUBDIRS` in `source/test/Makefile.am`.
- Add each new `*_gtest.bin` to the `tests` list in `test/run_ut.sh`. Otherwise it is built but never run.
- Add new source files under test to the module's `*_gtest_bin_SOURCES`, and any required libraries to `*_gtest_bin_LDFLAGS`.
- Follow `.github/instructions/build-system.instructions.md` for Makefile changes.

## 3. Mocking and Test Patterns

Before writing any test, the agent MUST inspect, and mirror, the existing code:

1. **`source/test/mocks/`**: reuse existing mocks before creating new ones.
   - `SystemMock` (`system`, `unlink`, `access`, `remove`, `inotify_*`, `clock_gettime`, `select`)
   - `FileioMock`, `VectorMock`, `rbusMock`, `rdkconfigMock`, `rdklogMock`, `protocolMock`, `profileStub.c`
2. **The sibling test suite for the module being changed** (for example `source/test/scheduler/schedulerTest.cpp`) for:
   - fixture structure (`class <module>TestFixture : public ::testing::Test`)
   - `SetUp()`/`TearDown()` and how global mock pointers are assigned and reset to `NULL`
   - `EXPECT_CALL` / `ON_CALL` / `WillOnce(Return(...))` style
   - naming, assertion style, and `extern "C"` include blocks
3. Mock conventions:
   - Mocks intercept C symbols via `extern "C"` wrappers backed by a global `g_*Mock` / `m_*Mock` pointer. When the pointer is `NULL`, the wrapper must fall through to the real function (`dlsym(RTLD_NEXT, ...)`).
   - Mocks of `clock_gettime` and `select` are gated by explicit flags (`g_mockClockGettime`, `g_mockSelect`). Enable them only for the duration of the test that needs them, because gtest itself calls these.
   - Add a new mock only when no existing one covers the dependency. Place it in `source/test/mocks/` if reusable across modules, otherwise in the module's test directory.
4. Test rules:
   - Wrap all C headers in `extern "C" { ... }`.
   - Use fixtures; allocate in `SetUp()`, release in `TearDown()`.
   - Tests must be deterministic and independent: no real network, no real sleeps beyond a minimal bound, no dependence on test order, and no files left behind (use `/tmp` and clean up).
   - Every test must be leak-free under valgrind.
   - Test names: `TEST_F(<Component>TestFixture, <Function>_<Condition>_<ExpectedResult>)`.
   - Use `ASSERT_*` where the test cannot continue after failure (for example, a NULL pointer that is dereferenced next). Use `EXPECT_*` otherwise. Add `<<` messages when they aid diagnosis.

## 4. Coverage and Quality Gates

A contribution is acceptable only if all of the following hold:

1. **Coverage must not decrease.** L1 line coverage and branch coverage for every touched file must stay the same or increase. New code should target full line and branch coverage of what it adds.
2. **All of these must be explicitly tested** for each new or modified function:
   - the success path
   - every distinct `T2ERROR_*` return path
   - every `NULL` pointer argument, individually
   - empty strings and zero-length or zero-count inputs
   - boundary values (max and min sizes, off-by-one, buffer-full, `INT_MAX`/`UINT_MAX` where relevant)
   - allocation failure (`malloc`/`calloc`/`strdup` returning `NULL`), when the allocation is mockable
   - failure of every external call the function checks (system, file I/O, rbus, vector, parser)
   - cleanup/destructor behavior, including idempotence and NULL-safety
   - concurrency-sensitive code: both the lock-held and no-contention paths, plus start/stop and double-stop
3. **Verification commands** (run and report results; do not claim success without running them):

   ```bash
   ./test/run_ut.sh --enable-cov     # build, run all L1 binaries, produce lcov report
   ```
   
   Individual binary: `./source/test/<module>/<name>_gtest.bin`, optionally with `--gtest_filter=<Fixture>.<Test>`.
   Leak check: `valgrind --leak-check=full --show-leak-kinds=all <binary>`.
4. Static analysis (`cppcheck`) and valgrind must be clean. Use the `quality-checker` skill to run the full suite in the standard test container.
5. All pre-existing tests must still pass (zero regression). Never delete, disable, or weaken an existing test or assertion to make a change pass. If a test must change because behavior intentionally changed, say so explicitly.
6. Before declaring a task done, report: the files and tests added or changed, the commands run, pass/fail results, and coverage delta for touched files. If something could not be run, say so; do not imply it passed.

## 5. Telemetry Codebase Conventions

### Defensive programming (C production code)

- Validate every pointer parameter at function entry. Return `T2ERROR_INVALID_ARGS` (or the function's documented failure value) for `NULL` or invalid input.
- Check every allocation (`malloc`, `calloc`, `realloc`, `strdup`). Return `T2ERROR_MEMALLOC_FAILED` on failure. Never assign `realloc` directly to the original pointer.
- Bounds-check all buffer writes. Use `snprintf`, `strncpy` with explicit termination, and size-aware APIs. Never use `strcpy`, `strcat`, `sprintf`, or `gets`.
- Validate sizes and counts from external input (JSON, rbus, XConf responses) before using them in arithmetic or allocations.
- Check return values of all calls that can fail. No silent failures.
- Every acquired resource (memory, FILE*, fd, mutex, thread, rbus handle, cJSON object) has exactly one clear release on every exit path, including error paths. Use a single-exit `goto cleanup` pattern where there are multiple resources.
- Set pointers to `NULL` after `free`. Free functions must accept `NULL`.
- Keep stack usage small; prefer stack buffers with fixed bounds over heap for short-lived data. Create threads with explicit stack sizes.
- Lock ordering must be consistent. Never call back into user code or log at length while holding a lock unless the existing code does so. Always unlock on every path.
- Do not introduce platform-specific code in shared sources. Use the existing abstraction layers.
- Maintain public API/ABI compatibility (`include/`, exported symbols).

### Logging

- Use only the project macros from `source/utils/t2log_wrapper.h`: `T2Error()`, `T2Warning()`, `T2Info()`, `T2Debug()`. Do not use `printf`, `fprintf(stderr, ...)`, or direct `RDK_LOG`.
- Level usage:
  - `T2Error`: operation failed and the error is being returned or propagated.
  - `T2Warning`: recoverable or unexpected-but-handled condition.
  - `T2Info`: significant lifecycle events (profile enable/disable, report sent).
  - `T2Debug`: detailed tracing. Begin and end entry/exit traces of non-trivial functions with `T2Debug("%s ++in\n", __FUNCTION__)` / `T2Debug("%s --out\n", __FUNCTION__)`, matching neighbouring code.
- Log the failing condition with enough context (function, key argument, error cause), but never log secrets, tokens, credentials, or full device identifiers.
- Log once at the point of failure; callers that propagate the error should not re-log the same failure at `T2Error`.
- Tests that exercise logging use `rdklogMock`; do not assert on exact log text unless the log is part of the contract.

### Error code propagation

- Functions that can fail return `T2ERROR` (defined in `include/telemetry2_0.h`):
  `T2ERROR_SUCCESS`, `T2ERROR_FAILURE`, `T2ERROR_INVALID_PROFILE`, `T2ERROR_PROFILE_NOT_FOUND`, `T2ERROR_PROFILE_NOT_SET`, `T2ERROR_MAX_PROFILES_REACHED`, `T2ERROR_MEMALLOC_FAILED`, `T2ERROR_INVALID_ARGS`, `T2ERROR_INVALID_RESPONSE`, `T2ERROR_INTERNAL_ERROR`, `T2ERROR_NO_RBUS_METHOD_PROVIDER`, `T2ERROR_COMPONENT_NULL`.
- Return the most specific code available. Use `T2ERROR_FAILURE` only when no specific code applies.
- Propagate errors from callees; never convert a failure to `T2ERROR_SUCCESS`. After cleanup, return the original error.
- Telemetry event paths (`t2_event_*`, marker handling, report generation, report send) must surface failures to the caller and must not drop events silently. A failed send must follow the existing retry or cache behavior.
- Do not add new `T2ERROR_*` values or change numeric values of existing ones without a compatibility review.
- Every returned error code must have a corresponding L1 test asserting it.

### Architecture principles

- Use RAII patterns in C++ test code.
- Prefer stack allocation over heap when possible; use memory pools for frequent allocations.
- Create threads with explicit stack sizes to minimize memory usage.
- Use lightweight synchronization primitives (atomic operations, simple mutexes).
- Prevent deadlocks through consistent lock ordering and timeouts.
- Implement proper error handling (no silent failures) and follow defensive programming practices.
- Design for testability from the start.

## 6. Agent Workflow

1. Read the target source file, its header, and its existing test file(s) first.
2. Inspect `source/test/mocks/` and the neighbouring test suite for patterns (section 3).
3. Implement the change following section 5.
4. Write or update L1 tests covering section 4.2 items; register new binaries (section 2).
5. Build and run the L1 suite with coverage; run valgrind on affected binaries; fix failures.
6. Report results per section 4.6.

Do not: add unrequested features or refactors; modify unrelated files; commit, push, or rewrite history without being asked; use `--no-verify` or similar bypasses.

## 7. Related Instructions and Skills

- `.github/instructions/c-embedded.instructions.md` - C coding rules
- `.github/instructions/cpp-testing.instructions.md` - gtest/gmock standards
- `.github/instructions/build-system.instructions.md` - Autotools rules
- `.github/instructions/shell-scripts.instructions.md` - shell rules
- Skills: `quality-checker`, `memory-safety-analyzer`, `thread-safety-analyzer`, `platform-portability-checker`, `code-review`, `technical-documentation-writer`, `triage-logs`

## 8. Code Review Standards

All code changes must:

- Pass static analysis (cppcheck, valgrind).
- Have zero memory leaks (verified by valgrind).
- Include L1 unit tests for all new or changed functionality (non-compliant otherwise).
- Maintain or improve L1 line and branch coverage.
- Follow existing naming conventions.
- Include clear commit messages explaining why, not just what.
