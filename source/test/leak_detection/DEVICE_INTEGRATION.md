# Curl Memory Leak Test - Device Integration

## Overview
This memory leak detection tool is now integrated with the main telemetry build system and will be compiled as `curl_leak_test` alongside `telemetry2_0` for device deployment.

## Building with Telemetry

### 1. Standard Telemetry Build Process
```bash
# Navigate to telemetry root directory
cd c:\Users\yk185\docker\60476\telemetry

# Standard autotools build (this will now include curl_leak_test)
./configure [your_normal_telemetry_configure_flags]
make
make install
```

### 2. Build Output
After building, you'll have both binaries:
- `/usr/bin/telemetry2_0` - Main telemetry daemon
- `/usr/bin/curl_leak_test` - Memory leak detection tool

### 3. Configure Flags for curl_leak_test
The tool automatically inherits all telemetry build flags:

```bash
# Basic build
./configure

# With mTLS support
./configure --enable-rdkcertsel

# With RDKB support  
./configure --enable-rdkb-support

# With RDK logging
./configure --enable-rdklogger

# Full featured build
./configure --enable-rdkcertsel --enable-rdkb-support --enable-rdklogger
```

## Device Deployment

### 1. Install on Device
```bash
# Copy to device (same as telemetry2_0)
scp curl_leak_test root@device_ip:/usr/bin/
chmod +x /usr/bin/curl_leak_test
```

### 2. Run on Device
```bash
# Basic test (10 iterations, 2 second intervals)
curl_leak_test 10 2

# Continuous monitoring (infinite iterations)
curl_leak_test 0 5

# Quick test (5 iterations, 1 second intervals)
curl_leak_test 5 1

# Background monitoring with logging
nohup curl_leak_test 100 10 > /tmp/curl_leak_test.log 2>&1 &
```

### 3. Device-Specific Usage

#### Check Device Connectivity First
```bash
# Test basic connectivity before running curl_leak_test
ping -c 3 8.8.8.8
curl -v https://httpbin.org/get

# If external connectivity fails, modify test URLs in source
```

#### Monitor Device Resources
```bash
# Monitor while test runs
watch -n 5 "ps aux | grep curl_leak_test; echo; cat /proc/meminfo | head -5"

# Check for memory growth patterns
tail -f /proc/`pgrep curl_leak_test`/status | grep -E "(VmRSS|VmSize)"
```

## Integration Benefits

### 1. Same Build Environment
- Uses exact same compiler flags as telemetry2_0
- Links against same libraries (libhttp.la, libxconfclient.la)
- Inherits all conditional compilation flags
- No version mismatches

### 2. Real mTLS Testing
- When built with `--enable-rdkcertsel`, tests actual mTLS implementation
- Uses device certificates and configuration
- Tests real-world authentication scenarios

### 3. Device Network Environment
- Tests in actual device network conditions
- Uses device DNS, routing, firewall rules
- Tests with device-specific SSL/TLS configuration

## Memory Leak Analysis on Device

### 1. Expected Output on Device
```
=== Curl Memory Leak Test Application ===
Using max iterations: 10
Using interval: 2 seconds

Initial memory usage:
VmSize:     5432 kB  |  VmRSS:     2345 kB

[Iteration 1] Testing HTTP POST...
[DEBUG] http_pool_post ; POST url = https://httpbin.org/post
[Iteration 1] POST request successful

--- Memory Usage at iteration 5 ---
VmSize:     5438 kB  |  VmRSS:     2347 kB

=== Memory Usage Summary ===
Initial - VmSize:     5432 kB  |  VmRSS:     2345 kB
Peak    - VmSize:     5441 kB  |  VmRSS:     2350 kB
Final   - VmSize:     5439 kB  |  VmRSS:     2348 kB
Growth  - VmSize:       +7 kB  |  VmRSS:       +3 kB

✓ Memory usage appears stable (growth: 3 kB)
```

### 2. Interpreting Device Results
- **Growth < 50KB**: Normal for device environment
- **Growth 50-500KB**: Monitor across multiple test runs
- **Growth > 500KB**: Investigate potential leak
- **Consistent growth pattern**: Likely memory leak

### 3. Device Debugging Commands
```bash
# Real-time memory monitoring
watch -n 2 "grep -E 'VmRSS|VmSize' /proc/\`pgrep curl_leak_test\`/status"

# Check for memory fragments
cat /proc/`pgrep curl_leak_test`/smaps | grep -E "(Rss|Pss|Private)"

# Monitor network connections
netstat -tupln | grep curl_leak_test

# Check file descriptors
ls -la /proc/`pgrep curl_leak_test`/fd/ | wc -l
```

## Troubleshooting on Device

### 1. Build Issues
```bash
# If build fails, check autotools regeneration
autoreconf -fiv
./configure [flags]
make clean && make
```

### 2. Network Issues on Device
```bash
# Test basic device connectivity
ping -c 3 8.8.8.8
nslookup httpbin.org

# Check device firewall
iptables -L | grep OUTPUT

# Test with local URLs if external blocked
# Modify TEST_URL in source to use local server
```

### 3. Permission Issues
```bash
# Ensure proper permissions
chmod +x /usr/bin/curl_leak_test
chown root:root /usr/bin/curl_leak_test
```

## Customization for Device Environment

### 1. Modify Test URLs for Internal Testing
```c
// In curl_leak_test.c, change these for device-specific endpoints
#define TEST_URL "http://internal-server:8080/post"
#define TEST_GET_URL "http://internal-server:8080/get"
```

### 2. Device-Specific Configuration
```bash
# Set device-specific environment variables
export T2_CONNECTION_POOL_SIZE=1  # For resource-constrained devices
export CURL_CA_BUNDLE=/etc/ssl/certs/ca-bundle.crt
```

### 3. Integration with Device Scripts
```bash
#!/bin/bash
# Device monitoring script
ITERATIONS=50
INTERVAL=10
LOG_FILE="/tmp/memory_leak_$(date +%Y%m%d_%H%M%S).log"

echo "Starting curl memory leak test on device"
curl_leak_test $ITERATIONS $INTERVAL > $LOG_FILE 2>&1

# Analyze results
GROWTH=$(grep "Growth.*VmRSS:" $LOG_FILE | tail -1 | awk '{print $6}' | sed 's/[+kB]//g')
if [ "$GROWTH" -gt 100 ]; then
    echo "WARNING: Memory growth detected: ${GROWTH}kB"
    # Send alert or upload log
fi
```

## Continuous Integration

The tool is now part of the telemetry build pipeline, so:
- It builds automatically with telemetry CI/CD
- Uses same build flags and environment
- Can be deployed alongside telemetry updates
- Maintains version synchronization

This integration ensures your memory leak testing tool is always available on devices and stays in sync with the telemetry implementation it's testing.