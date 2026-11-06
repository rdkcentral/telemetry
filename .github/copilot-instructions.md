# Telemetry Embedded Systems Development Guidelines

This repository contains the Telemetry 2.0 framework for RDK (Reference Design Kit), designed for embedded devices with constrained resources.

## Project Context

- **Target Platform**: Embedded devices (STBs, gateways, IoT devices)
- **Resource Constraints**: Limited memory (often <512MB RAM), limited CPU
- **Languages**: C (primary), C++ (tests), Shell scripts
- **Build System**: Autotools (configure.ac, Makefile.am)
- **Testing**: Google Test (gtest) and Google Mock (gmock)
- **Platform Requirements**: Must be platform-independent, supporting multiple architectures

## Critical Requirements

1. **Memory Safety**: Every allocation must have a corresponding free
2. **Resource Constraints**: Minimize memory footprint and CPU usage
3. **Platform Independence**: Avoid platform-specific code; use abstraction layers
4. **Backward Compatibility**: Maintain API/ABI compatibility
5. **Zero Regression**: All changes must pass existing test suites
6. **Mandatory L1 Tests**: Every function, module, or bug fix ships with L1 unit tests under `source/test/` (see below)
7. **Thread Safety**: Use lightweight synchronization, prevent deadlocks, minimize memory fragmentation
8. **Production Quality**: Code ships to millions of devices

## Mandatory L1 Unit Tests (Non-Negotiable)

See [AGENTS.md](../AGENTS.md) for the full rules. Summary:

- Whenever you implement a function, module, behavior change, or bug fix, you MUST automatically generate or update the corresponding L1 gtest/gmock tests under `source/test/`, in the same change, without being asked.
- Code changes without accompanying L1 tests are **non-compliant**. Only docs-only, comment-only, and build-metadata-only changes are exempt; say so explicitly.
- Bug fixes require a regression test that fails before the fix and passes after.
- If code is untestable as written, add the smallest mockable seam instead of skipping the test. Keep API/ABI unchanged.
- Register new test directories in `source/test/Makefile.am` (`SUBDIRS`) and new `*_gtest.bin` binaries in `test/run_ut.sh`; otherwise they never run.

### Mocking and Test Patterns

Before writing tests, inspect and mirror existing code:

- `source/test/mocks/` (`SystemMock`, `FileioMock`, `VectorMock`, `rbusMock`, `rdkconfigMock`, `rdklogMock`, `protocolMock`, `profileStub.c`). Reuse before adding new mocks.
- The neighbouring suite for the module (for example `source/test/scheduler/schedulerTest.cpp`) for fixture class layout, `SetUp()`/`TearDown()`, global mock pointer assignment and reset, `EXPECT_CALL`/`ON_CALL` style, naming, and assertion style.
- Wrap C headers in `extern "C"`. Mock wrappers fall through to the real function via `dlsym(RTLD_NEXT, ...)` when the mock pointer is `NULL`. Enable `g_mockClockGettime`/`g_mockSelect` only within the test that needs them.
- Tests must be deterministic, independent, leak-free under valgrind, and leave no files behind.
- Name tests `TEST_F(<Component>TestFixture, <Function>_<Condition>_<ExpectedResult>)`.

### Coverage and Quality Gates

- L1 line and branch coverage of every touched file must stay the same or increase.
- Explicitly test, for each new or modified function: the success path, every `T2ERROR_*` return path, each `NULL` argument individually, empty and zero-size inputs, boundary values, allocation failures, failures of every checked external call, and cleanup behavior (NULL-safe, idempotent).
- Run `./test/run_ut.sh --enable-cov` and valgrind on affected binaries. Report what was run, pass/fail, and the coverage delta. Never claim success for commands that were not run.
- Never delete, disable, or weaken existing tests to make a change pass.

## Telemetry Codebase Conventions

### Defensive Programming

- Validate every pointer parameter at entry; return `T2ERROR_INVALID_ARGS` for `NULL` or invalid input.
- Check every allocation; return `T2ERROR_MEMALLOC_FAILED` on failure. Never assign `realloc` directly to the source pointer.
- Bounds-check all buffer writes. Use `snprintf` and explicitly terminated `strncpy`; never `strcpy`, `strcat`, `sprintf`, or `gets`.
- Validate sizes and counts from external input (JSON, rbus, XConf) before arithmetic or allocation.
- Check return values of all fallible calls. No silent failures.
- Release every resource on every exit path (single-exit `goto cleanup` for multiple resources). Set pointers to `NULL` after `free`; free functions accept `NULL`.

### Logging

- Use only `T2Error()`, `T2Warning()`, `T2Info()`, `T2Debug()` from `source/utils/t2log_wrapper.h`. No `printf` or direct `RDK_LOG`.
- `T2Error` for failures being returned, `T2Warning` for handled anomalies, `T2Info` for lifecycle events, `T2Debug` for tracing (`"%s ++in\n"` / `"%s --out\n"` with `__FUNCTION__`).
- Include context (function, key argument, cause). Never log secrets, tokens, or full device identifiers. Log once at the failure point, not again in each caller.

### Error Code Propagation

- Fallible functions return `T2ERROR` (`include/telemetry2_0.h`). Return the most specific code; use `T2ERROR_FAILURE` only as a last resort.
- Propagate callee errors; never turn a failure into `T2ERROR_SUCCESS`. Clean up, then return the original error.
- Telemetry event, marker, report generation, and report send paths must surface failures to the caller and never drop events silently.
- Do not add or renumber `T2ERROR_*` values without a compatibility review. Every returned error code must have an L1 test asserting it.

## Architecture Principles

- Use RAII (Resource Acquisition Is Initialization) patterns in C++ test code
- Prefer stack allocation over heap when possible
- Use memory pools for frequent allocations
- Create threads with explicit stack sizes to minimize memory usage
- Use lightweight synchronization primitives (atomic operations, simple mutexes)
- Prevent deadlocks through consistent lock ordering and timeouts
- Implement proper error handling (no silent failures)
- Follow defensive programming practices
- Design for testability from the start

## Code Review Standards

All code changes must:
- Pass static analysis (cppcheck, valgrind)
- Have zero memory leaks (verified by valgrind)
- Include L1 unit tests for all new or changed functionality (non-compliant otherwise)
- Maintain or improve L1 line and branch coverage
- Follow existing naming conventions
- Include clear commit messages explaining why, not just what

Refer to language-specific instruction files in `.github/instructions/` for detailed guidelines.

## Available Skills

Specialized skills for common development tasks:

- **quality-checker**: Run comprehensive quality checks (static analysis, memory safety, thread safety, build verification) in the standard test container. Use when validating code changes or debugging before committing.
- **memory-safety-analyzer**: Analyze C/C++ code for memory safety issues including leaks, use-after-free, buffer overflows, and provide fixes.
- **thread-safety-analyzer**: Analyze C/C++ code for thread safety issues including race conditions, deadlocks, and improper synchronization.
- **platform-portability-checker**: Verify C/C++ code is platform-independent and portable across embedded platforms.
- **technical-documentation-writer**: Create and maintain comprehensive technical documentation for embedded systems projects.
- **code-review**: Analyze PR changes for embedded C/C++ code and generate a REVIEW.md with impact assessment and regression risk analysis.
- **triage-logs**: Triage Telemetry 2.0 behavioral issues on RDK devices by correlating device log bundles with source code.
