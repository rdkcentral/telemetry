---
name: quality-checker
description: Run comprehensive quality checks (static analysis, memory safety, thread safety, build verification, L1 unit tests, L2 integration tests) in the standard test container. Use when validating code changes or debugging before committing.
---

# Container-Based Quality Checker

## Purpose

Execute comprehensive quality checks on the codebase using the same containerized environment as CI/CD pipelines. Ensures consistency between local development and automated testing.

## Usage

Invoke this skill when:
- Validating changes before committing
- Debugging build or test failures
- Running quality checks locally
- Verifying memory safety of new code
- Checking for thread safety issues
- Performing static analysis
- Running unit tests (L1)
- Running integration tests (L2)
- Validating full system behavior

You can run all checks or select specific ones based on your needs.

## What It Does

This skill runs quality checks inside the official test container (`ghcr.io/rdkcentral/docker-device-mgt-service-test/native-platform:latest`), which includes:
- Build tools (gcc, autotools, make)
- Static analysis tools (cppcheck, shellcheck)
- Memory analysis tools (valgrind)
- Thread analysis tools (helgrind)
- Google Test/Mock frameworks

## Available Checks

### 1. Static Analysis
- **cppcheck**: Comprehensive C/C++ static code analyzer
- **shellcheck**: Shell script linter
- **Output**: XML report with findings

### 2. Memory Safety (Valgrind)
- **Memory leak detection**: Finds unreleased allocations
- **Use-after-free detection**: Catches dangling pointer usage
- **Invalid memory access**: Buffer overflows, uninitialized reads
- **Output**: XML and log files per test binary

### 3. Thread Safety (Helgrind)
- **Race condition detection**: Finds unsynchronized shared memory access
- **Deadlock detection**: Identifies lock ordering issues
- **Lock usage verification**: Validates proper synchronization
- **Output**: XML and log files per test binary

### 4. Build Verification
- **Strict compilation**: Builds with `-Wall -Wextra -Werror`
- **Test build**: Verifies tests compile
- **Binary analysis**: Reports size and dependencies
- **Output**: Build artifacts and size report

### 5. L1 Unit Tests
- **GTest suite execution**: Runs all unit tests via `test/run_ut.sh`
- **Component-level testing**: Tests individual modules in isolation
- **Fast feedback**: Typically completes in seconds to minutes
- **Output**: JSON test results in `/tmp/Gtest_Report`

### 6. L2 Integration Tests
- **End-to-end testing**: Runs integration tests via `test/run_l2.sh`
- **Mock service dependencies**: Uses mockxconf container for external services
- **Full build verification**: Builds complete binary before testing
- **Output**: Test results in `/tmp/l2_test_report`

## Execution Process

### Step 1: Setup Container Environment

Pull the latest test container (only if not present locally):
```bash
# Check if image exists locally, pull only if missing
if ! docker images ghcr.io/rdkcentral/docker-device-mgt-service-test/native-platform:latest --format "{{.Repository}}:{{.Tag}}" | grep -q "native-platform:latest"; then
  echo "Image not found locally, pulling..."
  docker pull ghcr.io/rdkcentral/docker-device-mgt-service-test/native-platform:latest
else
  echo "Using cached image"
fi
```

Start container with workspace mounted:
```bash
docker run -d --name native-platform \
  -v /path/to/workspace:/mnt/workspace \
  ghcr.io/rdkcentral/docker-device-mgt-service-test/native-platform:latest
```

### Step 2: Run Selected Checks

Execute the requested quality checks inside the container:

**Static Analysis:**
```bash
docker exec -i native-platform /bin/bash -c "
  cd /mnt/workspace && \
  cppcheck --enable=all \
           --inconclusive \
           --suppress=missingIncludeSystem \
           --suppress=unmatchedSuppression \
           --error-exitcode=0 \
           --xml \
           --xml-version=2 \
           source/ 2> cppcheck-report.xml
"
```

**Shell Script Checks:**
```bash
docker exec -i native-platform /bin/bash -c "
  cd /mnt/workspace && \
  find . -name '*.sh' -type f -exec shellcheck {} +
"
```

**Memory Safety:**
```bash
docker exec -i native-platform /bin/bash -c "
  cd /mnt/workspace && \
  autoreconf -fi && \
  ./configure --enable-gtest && \
  make -j\$(nproc) && \
  find source/test -type f -executable -name '*test*' | while read test_bin; do
    valgrind --leak-check=full \
             --show-leak-kinds=all \
             --track-origins=yes \
             --xml=yes \
             --xml-file=\"valgrind-\$(basename \$test_bin).xml\" \
             \"\$test_bin\" 2>&1 | tee \"valgrind-\$(basename \$test_bin).log\"
  done
"
```

**Thread Safety:**
```bash
docker exec -i native-platform /bin/bash -c "
  cd /mnt/workspace && \
  find source/test -type f -executable -name '*test*' | while read test_bin; do
    valgrind --tool=helgrind \
             --track-lockorders=yes \
             --xml=yes \
             --xml-file=\"helgrind-\$(basename \$test_bin).xml\" \
             \"\$test_bin\" 2>&1 | tee \"helgrind-\$(basename \$test_bin).log\"
  done
"
```

**Build Verification:**
```bash
docker exec -i native-platform /bin/bash -c "
  cd /mnt/workspace && \
  autoreconf -fi && \
  ./configure --enable-gtest CFLAGS='-Wall -Wextra -Werror' && \
  make -j\$(nproc) && \
  if [ -f 'telemetry2_0' ]; then
    ls -lh telemetry2_0
    file telemetry2_0
    size telemetry2_0
  fi
"
```

**L1 Unit Tests:**
```bash
docker exec -i native-platform /bin/bash -c "
  cd /mnt/workspace && \
  sh test/run_ut.sh
"
```

Copy test results from container:
```bash
docker cp native-platform:/tmp/Gtest_Report ./L1_test_results
ls -l ./L1_test_results
```

**L2 Integration Tests:**

First, pull mockxconf image if not present locally:
```bash
# Pull mockxconf image only if not present
if ! docker images ghcr.io/rdkcentral/docker-device-mgt-service-test/mockxconf:latest --format "{{.Repository}}:{{.Tag}}" | grep -q "mockxconf:latest"; then
  echo "Mockxconf image not found locally, pulling..."
  docker pull ghcr.io/rdkcentral/docker-device-mgt-service-test/mockxconf:latest
else
  echo "Using cached mockxconf image"
fi
```

Start the mock XCONF service container:
```bash
docker run -d --name mockxconf \
  -p 50050:50050 \
  -p 50051:50051 \
  -p 50052:50052 \
  -e ENABLE_MTLS=true \
  -v /path/to/workspace:/mnt/L2_CONTAINER_SHARED_VOLUME \
  ghcr.io/rdkcentral/docker-device-mgt-service-test/mockxconf:latest
```

Update the native-platform container to link with mockxconf:
```bash
docker stop native-platform
docker rm native-platform
docker run -d --name native-platform \
  --link mockxconf \
  -e ENABLE_MTLS=true \
  -v /path/to/workspace:/mnt/L2_CONTAINER_SHARED_VOLUME \
  ghcr.io/rdkcentral/docker-device-mgt-service-test/native-platform:latest
```

Build and run L2 tests:
```bash
docker exec -i native-platform /bin/bash -c "
  cd /mnt/L2_CONTAINER_SHARED_VOLUME && \
  sh build_inside_container.sh && \
  export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:/usr/lib/x86_64-linux-gnu:/lib/aarch64-linux-gnu:/usr/local/lib: && \
  sh test/run_l2.sh
"
```

Copy L2 test results from container:
```bash
docker cp native-platform:/tmp/l2_test_report ./L2_test_results
ls -l ./L2_test_results
```

Clean up mockxconf container:
```bash
docker stop mockxconf
docker rm mockxconf
```

### Step 3: Report Results

Parse and summarize results for the user:
- Number of issues found by category
- Critical issues requiring immediate attention
- Warnings that should be addressed
- Memory leaks with stack traces
- Race conditions or deadlock risks
- Build errors or warnings
- L1 test pass/fail counts and failures
- L2 test pass/fail counts and failures

### Step 4: Cleanup

Stop and remove the container(s):
```bash
docker stop native-platform
docker rm native-platform

# If L2 tests were run, also cleanup mockxconf
docker stop mockxconf 2>/dev/null || true
docker rm mockxconf 2>/dev/null || true
```

## Interpreting Results

### Static Analysis (cppcheck)
- **error**: Critical issues that must be fixed
- **warning**: Potential problems to review
- **style**: Code style improvements
- **performance**: Optimization opportunities

### Memory Safety (Valgrind)
- **definitely lost**: Memory leaks requiring fixes
- **indirectly lost**: Leaks from lost parent structures
- **possibly lost**: Potential leaks to investigate
- **still reachable**: Memory held at exit (usually OK)
- **Invalid read/write**: Buffer overflow (CRITICAL)
- **Use of uninitialized value**: Must initialize before use

### Thread Safety (Helgrind)
- **Possible data race**: Unsynchronized access to shared data
- **Lock order violation**: Potential deadlock scenario
- **Unlocking unlocked lock**: Synchronization bug
- **Thread still holds locks**: Resource leak

### Build Verification
- **Compilation errors**: Must fix before proceeding
- **Warnings**: Review and fix (builds with -Werror)
- **Binary size**: Monitor for embedded constraints

### L1 Unit Tests
- **JSON test results**: Detailed pass/fail status per test case
- **Failed tests**: Review test output for assertion failures
- **Crashes/segfaults**: Indicates memory corruption or logic errors
- **Test coverage**: Monitor overall test execution coverage

### L2 Integration Tests
- **End-to-end failures**: Indicates integration issues between components
- **Mock service errors**: Check mockxconf connectivity and configuration
- **Build failures**: Must complete build before L2 tests can run
- **Runtime errors**: Review full system behavior under realistic conditions

## User Interaction

When invoked, ask the user:

1. **Which checks to run?**
   - All checks (comprehensive - includes static analysis, memory, thread, build, L1, L2)
   - Static analysis only (fast)
   - Memory safety only
   - Thread safety only
   - Build verification only
   - L1 unit tests only
   - L2 integration tests only
   - Tests only (L1 + L2)
   - Quality only (static analysis, memory, thread safety)
   - Custom combination

2. **Scope:**
   - Full codebase
   - Specific directories
   - Recently changed files

3. **Report detail:**
   - Summary only (counts and critical issues)
   - Detailed (all findings)
   - Full raw output

## Example Invocations

**User**: "Run quality checks"
- Default: Run all checks on full codebase, provide summary

**User**: "Check memory safety"
- Run only valgrind checks, detailed report

**User**: "Quick static analysis"
- Run cppcheck and shellcheck, summary only

**User**: "Verify my changes build"
- Run build verification with strict warnings

**User**: "Full analysis on source/bulkdata"
- Run all checks scoped to bulkdata directory

**User**: "Run L1 tests"
- Execute unit test suite via run_ut.sh, report results

**User**: "Run L2 tests"
- Setup mockxconf, build, execute integration tests, report results

**User**: "Run all tests"
- Execute both L1 and L2 test suites

**User**: "Quick validation"
- Run static analysis + L1 tests (fast pre-commit check)

## Best Practices

1. **Run before committing**: Catch issues early
2. **Start with static analysis**: Fastest feedback
3. **Run L1 tests frequently**: Quick unit test feedback
4. **Run L2 tests before PR**: Validate integration behavior
5. **Run memory checks on test binaries**: Most effective
6. **Review thread safety for concurrent code**: Essential for multi-threaded components
7. **Monitor binary size**: Important for embedded targets
8. **Progressive validation**: Static → L1 → Build → L2 → Memory/Thread

## Integration with Development Workflow

1. **Pre-commit**: Quick static analysis + L1 tests
2. **Pre-push**: Full quality check suite + L2 tests
3. **Debugging**: Targeted memory/thread analysis
4. **Code review**: Validate reviewer feedback
5. **Refactoring**: Ensure no regressions with full test suite
6. **Feature development**: L1 tests during development, L2 before completion

## Advantages Over Manual Testing

- **Consistency**: Same environment as CI/CD
- **Completeness**: All tools in one command
- **Reproducibility**: Container ensures identical results
- **Efficiency**: No local tool installation needed
- **Confidence**: Pass locally = pass in CI

## Output Files Generated

- `cppcheck-report.xml`: Static analysis findings
- `valgrind-<testname>.xml`: Memory issues per test
- `valgrind-<testname>.log`: Detailed memory logs
- `helgrind-<testname>.xml`: Thread safety issues per test
- `helgrind-<testname>.log`: Detailed concurrency logs
- `L1_test_results/`: L1 unit test results (JSON format from /tmp/Gtest_Report)
- `L2_test_results/`: L2 integration test results (from /tmp/l2_test_report)

These files can be uploaded as artifacts or reviewed locally.

## Limitations

- Requires Docker with GitHub Container Registry access
- Container image pulls occur only on first run (then cached)
- Full suite can take several minutes depending on codebase size
- Valgrind slows execution significantly (expected)

## Tips for Faster Execution

1. Container images are auto-cached (pulled only when missing locally)
2. Run static analysis first (fastest)
3. Run L1 tests early for quick feedback (faster than L2)
4. Scope checks to changed directories
5. Run memory/thread checks only on affected tests
6. Use parallel execution where possible
7. Skip L2 tests during rapid development (run before PR)
8. Reuse mockxconf container for multiple L2 test runs

## Skill Execution Logic

When user invokes this skill:

1. **Authenticate with GitHub Container Registry**
   - Use github.actor and GITHUB_TOKEN if available
   - Otherwise prompt for credentials or skip private registries

2. **Pull container image(s)**
   - Check if image exists locally using `docker images`
   - Pull only if image is not present (saves time and bandwidth)
   - For L2 tests: also check and pull mockxconf container if needed
   - Skip pull with cached images to speed up execution

3. **Start container(s)**
   - Mount workspace at /mnt/workspace (or appropriate shared volume)
   - Use unique container name (quality-checker-<timestamp>)
   - Run in detached mode
   - For L2 tests: start mockxconf first, then link native-platform

4. **Execute requested checks**
   - Run checks in sequence (or selected subset)
   - Capture output
   - Continue on errors (collect all findings)
   - For L1 tests: execute test/run_ut.sh
   - For L2 tests: build first, then execute test/run_l2.sh

5. **Collect results**
   - Copy result files from container
   - Parse XML/log outputs
   - Categorize findings
   - For L1 tests: copy /tmp/Gtest_Report to local directory
   - For L2 tests: copy /tmp/l2_test_report to local directory

6. **Report to user**
   - Summary count (errors, warnings, test pass/fail)
   - Critical issues highlighted
   - L1/L2 test results summary
   - Link to detailed reports
   - Next steps recommendations

7. **Cleanup**
   - Stop container(s)
   - Remove container(s)
   - For L2 tests: also stop and remove mockxconf container
   - Optional: clean up result files

## Error Handling

- **Container pull fails**: Report error, suggest manual pull
- **Container start fails**: Check Docker daemon, ports, permissions
- **Build fails**: Report build errors, stop further checks
- **Tools missing**: Verify container version, report missing tools
- **Out of memory**: Suggest increasing Docker memory limit
- **L2 mockxconf connection fails**: Check container linking and port availability
- **Test failures**: Report which tests failed, suggest reviewing logs
