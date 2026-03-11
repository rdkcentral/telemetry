# Memory Verifier

**Standalone tool to verify your curl/OpenSSL configuration has no memory leaks.**

## Quick Start

```bash
cd memory-verifier
make run
```

## Purpose

This is a **completely separate test application** that verifies your curl 7.81.0 + OpenSSL 3.0.2 setup has no memory leaks. It's independent from the main telemetry leak detection tests.


## What It Does

Runs multiple independent verification tests, matching production and edge-case usage:

### Standard Tests

- **Version Check**: Validates your libcurl and OpenSSL versions for known issues.
- **Connection Pool Pattern**: Simulates production connection pool usage with detailed per-iteration memory tracking. Pool size is configurable via `T2_CONNECTION_POOL_SIZE`.
- **POST Memory Leak Test**: Runs 100 POST requests to detect memory growth in typical POST usage.

### Valgrind-Integrated Tests

- **Valgrind Pool Test**: Runs the connection pool pattern under Valgrind to detect leaks and profile operational memory usage.
- **Valgrind SSL/OpenSSL Investigation**: Deep-dive memory profiling of SSL/OpenSSL usage, focusing on known memory growth patterns.

### Logging & Analysis

- Per-iteration logs: `connection_pool_iterations.log` for detailed memory and operation tracking.
- Valgrind and Massif outputs: `valgrind_pool_baseline.log`, `massif_pool_baseline.out`, `massif_ssl_investigation.out`, and human-readable reports if `ms_print` is available.

### Environment Variables

- `T2_CONNECTION_POOL_SIZE`: Set the connection pool size (default: 2, min: 1, max: 5).
- `T2_TEST_ITERATIONS`: Set the number of pool test iterations (default: 50, max: 200).
- `VALGRIND_FAST_MODE`: If set, reduces iterations and delays for faster Valgrind runs.

### CLI Options (see `--help`)

```
	--version, -v         Run version-specific assessment
	--pool, -p            Run connection pool pattern test
	--post, -o            Run POST memory leak test
	--valgrind-pool       Valgrind analysis of pool without reset
	--valgrind-ssl        Investigate SSL/OpenSSL memory patterns
	--all, -a             Run all standard tests (default)
	--all-valgrind        Run all valgrind-integrated tests
	--help                Show help message with all options
```

See the source or run `./memory_verifier --help` for the full list of options and examples.


## Expected Results

✅ **Success (No Leaks):**
```
��� FINAL VERIFICATION RESULT
Tests Run: <N>
Passed: <N>
Failed: 0

✅ VERDICT: NO MEMORY LEAKS DETECTED
��� YOUR HTTP CLIENT IS PRODUCTION-READY!
```

⚠️ **Acceptable (Monitor):**
- Some warnings but no failures
- Memory growth within acceptable thresholds (see logs for details)

❌ **Issues Found:**
- Test failures indicate potential memory problems
- Requires investigation (see Valgrind logs and summary)

## Directory Structure

```
memory-verifier/           # <- You are here
├── memory_verifier.c      # Standalone verification tool
├── Makefile              # Independent build system  
└── README.md             # This file

../leak_detection/         # <- Your main test suite (separate)
├── curl_leak_test.c       # Main telemetry tests
├── multicurlinterface.c   # Your fixed HTTP client
└── Makefile              # Main test build system
```


## Build & Run

```bash
make           # Build only
make run       # Build and run (recommended)
make clean     # Clean build artifacts
make help      # Show detailed help


# Run with custom options
./memory_verifier --pool                # Run connection pool test
T2_CONNECTION_POOL_SIZE=3 ./memory_verifier --pool   # Pool size 3
T2_TEST_ITERATIONS=100 ./memory_verifier --pool      # 100 iterations
./memory_verifier --valgrind-pool       # Valgrind pool analysis
./memory_verifier --valgrind-ssl        # Deep SSL/OpenSSL investigation

# Automated Dynamic Analysis (GDB/SSL investigation)
./run_dynamic_analysis.sh               # Run automated dynamic SSL/OpenSSL memory analysis
./run_dynamic_analysis.sh --help        # Show script options and usage
```


## Integration & Analysis


This tool is **completely independent** from your main telemetry code.

- **For production verification**: Use this tool
- **For telemetry testing**: Use `../leak_detection/`
- **For detailed analysis**: Use `cd ../leak_detection && make valgrind`
- **For automated dynamic SSL/OpenSSL analysis**: Use `run_dynamic_analysis.sh` (runs GDB-based tracing, logging, and summary for SSL memory investigation)


### Log Files & Outputs

- `connection_pool_iterations.log`: Per-iteration memory and operation log
- `valgrind_pool_baseline.log`: Valgrind leak report for pool test
- `massif_pool_baseline.out`: Massif memory profile for pool test
- `massif_ssl_investigation.out`: Massif memory profile for SSL investigation
- `massif_ssl_report.txt`: Human-readable SSL memory profile (if `ms_print` is available)
- `dynamic_analysis.log`: Main log for run_dynamic_analysis.sh (GDB/SSL trace summary)
- `dynamic_analysis_summary.log`: High-level summary of dynamic analysis results
## run_dynamic_analysis.sh

This script automates dynamic SSL/OpenSSL memory analysis using GDB. It runs the memory_verifier in a controlled environment, traces SSL/OpenSSL allocation functions, and summarizes memory growth and leak patterns.

**Usage:**

```bash
./run_dynamic_analysis.sh           # Run full dynamic analysis suite
./run_dynamic_analysis.sh --help    # Show all options and usage
```

**Features:**
- Automated GDB tracing of SSL/OpenSSL allocation and cleanup functions
- Per-iteration and summary logging
- Timeout and iteration controls (via environment variables)
- Correlates memory growth with SSL connection pool operations
- Produces logs for management and engineering review

See the script header or run with `--help` for full details and advanced options.


## Requirements

- Linux environment (WSL2 supported)
- libcurl development headers
- Valgrind (for valgrind-integrated tests)
- Internet connection (tests use httpbin.org)

## Troubleshooting & Tips

- If you see memory growth, check the logs and Valgrind output for SSL/OpenSSL or curl-related leaks.
- Use environment variables to tune pool size and iteration count for your environment.
- For deep SSL/OpenSSL memory analysis, use the `--valgrind-ssl` option and review `massif_ssl_report.txt`.

For full CLI usage and test descriptions, run:

```bash
./memory_verifier --help
```
