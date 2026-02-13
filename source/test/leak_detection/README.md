# Curl Memory Leak Detection Test

## Overview
This standalone application tests the multicurlinterface.c implementation for memory leaks by making periodic HTTP requests and monitoring memory usage.

## Quick Start

### 1. Build and Run Basic Test
```bash
cd /c/Users/yk185/docker/60476/telemetry/source/test/leak_detection

# Make the test script executable
chmod +x run_leak_test.sh

# Build and run basic test
make clean && make
./curl_leak_test 5 2
```

### 2. Memory Leak Detection with Valgrind
```bash
# Install valgrind if needed
sudo apt-get install valgrind

# Run leak detection
make valgrind
```

### 3. Use the Comprehensive Test Script
```bash
# Quick test
./run_leak_test.sh --quick

# Full leak detection with mTLS
./run_leak_test.sh --mtls --valgrind

# Run all tests
./run_leak_test.sh --all-tests
```

## Files

- **curl_leak_test.c** - Main test application
- **Makefile** - Build configuration with multiple targets
- **run_leak_test.sh** - Comprehensive test script
- **monitor_memory.sh** - Real-time memory monitoring (existing)

## Build Options

| Option | Description |
|--------|-------------|
| `make` | Basic build |
| `make build-mtls` | Build with mTLS support |
| `make build-rdkb` | Build with RDKB support |
| `make build-full` | Build with all features |

## Test Types

| Test | Command | Purpose |
|------|---------|---------|
| Basic | `make run` | 20 iterations, check for basic leaks |
| Quick | `make quick` | 5 iterations, fast verification |
| Valgrind | `make valgrind` | Detailed leak detection |
| Massif | `make massif` | Memory profiling |
| Stress | `make stress` | 100 iterations, long-term growth |

## Interpreting Results

### Memory Growth Analysis
- **< 100KB growth**: Normal, likely not a leak
- **100KB - 1MB growth**: Monitor for continued growth
- **> 1MB growth**: Potential leak, investigate

### Valgrind Output
- **"definitely lost"**: Confirmed memory leak - **FIX REQUIRED**
- **"indirectly lost"**: Related to definitely lost leaks
- **"still reachable"**: Memory not freed but accessible (may be acceptable)
- **"possibly lost"**: Unclear ownership, investigate

### Memory Usage Patterns
```
Normal:     Memory stabilizes after initial allocations
Leak:       Memory continuously grows with iterations
Burst:      Temporary growth that levels off
```

## Troubleshooting

### Build Errors
```bash
# Check dependencies
pkg-config --exists libcurl && echo "libcurl OK" || echo "libcurl MISSING"

# Install dependencies (Ubuntu/Debian)
sudo apt-get install gcc make libcurl4-openssl-dev valgrind

# For RDKB builds, ensure rbus library is available
```

### Network Issues
```bash
# Test connectivity
curl -v https://httpbin.org/get

# Use local test server if needed (modify TEST_URL in curl_leak_test.c)
```

### Permission Issues
```bash
# Make scripts executable
chmod +x run_leak_test.sh monitor_memory.sh
```

## Example Usage Session

```bash
# 1. Quick verification
make clean && make quick
# Expected: Should run 5 iterations successfully

# 2. Check for leaks
make valgrind
# Expected: "All heap blocks were freed" or specific leak details

# 3. Analyze memory patterns
make massif
# Expected: massif_report.txt showing allocation patterns

# 4. Stress test
make stress
# Expected: Memory should stabilize, not grow continuously
```

## Integration with CI/CD

```bash
# Add to your CI pipeline
./run_leak_test.sh --quick --valgrind
if [ $? -eq 0 ]; then
    echo "Memory leak tests passed"
else
    echo "Memory leak tests failed"
    exit 1
fi
```

## Expected Normal Output

```
=== Memory Usage Summary ===
Initial - VmSize:   12345 kB  |  VmRSS:   8765 kB
Peak    - VmSize:   12567 kB  |  VmRSS:   8890 kB  
Final   - VmSize:   12456 kB  |  VmRSS:   8801 kB
Growth  - VmSize:    +111 kB  |  VmRSS:    +36 kB

✓ Memory usage appears stable (growth: 36 kB)
```

## When to Investigate

🔴 **Investigate immediately:**
- Valgrind reports "definitely lost" > 0 bytes
- Memory growth > 1MB per test run
- Consistent growth pattern across multiple runs

🟡 **Monitor closely:**
- Memory growth 100KB - 1MB
- Increasing "still reachable" memory
- Intermittent growth patterns

✅ **Likely normal:**
- Memory growth < 100KB
- Growth only in first few iterations
- Stable memory after initial allocations