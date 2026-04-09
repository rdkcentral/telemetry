# Telemetry 2.0 Public API Reference

## Overview

This document describes the public API for the Telemetry 2.0 framework. These functions provide the interface for applications to interact with the telemetry system.

## Header Files

```c
#include "telemetry2_0.h"
#include "telemetry_busmessage_sender.h"
```

## Initialization and Cleanup

### t2_init()

Initialize the telemetry system.

**Signature:**
```c
void t2_init(char *component);
```

**Parameters:**
- `component` - Mandatory name of the component, used for marker to component mapping in profiles. Must be a non-NULL string.

**Returns:** Nothing (`void`).

**Thread Safety:** Not thread-safe. Must be called once during application startup before any other telemetry functions.

**Example:**
```c
#include "telemetry2_0.h"
#include <stdio.h>

int main(void) {
    // Initialize with component name (must not be NULL)
    t2_init("MyComponent");
    
    // ... application code ...
    
    t2_uninit();
    return 0;
}
```


### t2_uninit()

Uninitialize and cleanup telemetry system.

**Signature:**
```c
void t2_uninit(void);
```

**Thread Safety:** Not thread-safe. Must be called only after all telemetry operations complete, typically during application shutdown.

**Notes:**
- Stops all background threads
- Sends any pending reports (best effort)
- Frees all allocated resources
- Safe to call even if t2_init() failed

**Example:**
```c
// Cleanup on exit
void cleanup(void) {
    t2_uninit();
    // Other cleanup...
}

int main(void) {
    atexit(cleanup);
    
    // component name must be a valid non-NULL string
    t2_init("MyComponent");
    
    // Application runs...
    
    return 0;
}
```

## Event Reporting

### t2_event_s()

Send a string event marker.

**Signature:**
```c
T2ERROR t2_event_s(const char* marker, const char* value);
```

**Parameters:**
- `marker` - Event marker name (non-NULL)
- `value` - Event value string (non-NULL); empty string `""` and `"0"` are silently ignored

**Returns:**
- `T2ERROR_SUCCESS` - Event sent or queued
- `T2ERROR_FAILURE` - Send failed or event filtered

**Thread Safety:** Thread-safe. Can be called from any thread.

**Notes:**
- Event matched against active profile markers
- Empty value (`""`) or value `"0"` is ignored without sending
- Returns `T2ERROR_FAILURE` if `t2_init()` has not been called

**Examples:**
```c
// Simple event
t2_event_s("Device_Boot_Complete", "success");

// Component status
t2_event_s("WIFI_Status", "connected");

// Error event
t2_event_s("Storage_Error", "disk_full");

// Formatted message
char msg[256];
snprintf(msg, sizeof(msg), "Connection failed: %s", strerror(errno));
t2_event_s("Network_Error", msg);
```

### t2_event_d()

Send an integer or metric event marker.

**Signature:**
```c
T2ERROR t2_event_d(const char* marker, int value);
```

**Parameters:**
- `marker` - Event marker name (non-NULL)
- `value` - Integer value; value `0` is silently ignored

**Returns:**
- `T2ERROR_SUCCESS` - Event sent or queued
- `T2ERROR_FAILURE` - Send failed or event filtered

**Thread Safety:** Thread-safe.

**Notes:**
- Value `0` is ignored without sending (by design)

**Examples:**
```c
// Signal strength
t2_event_d("WIFI_RSSI", -65);

// CPU usage percentage
t2_event_d("SYS_ERROR_LOGUPLOAD_FAILED", 1);

// Temperature reading
t2_event_d("SYS_INFO_NTP_SYNC_ERROR", 1);

// Uptime in seconds
t2_event_d("System_Uptime", 3600);

// Bandwidth in Mbps
t2_event_d("Download_Speed", 125);
```

### t2_event_f()

Send a floating-point event marker.

**Signature:**
```c
T2ERROR t2_event_f(const char* marker, double value);
```

**Parameters:**
- `marker` - Event marker name (non-NULL)
- `value` - Floating-point (double) value

**Returns:**
- `T2ERROR_SUCCESS` - Event sent or queued
- `T2ERROR_FAILURE` - Send failed or event filtered

**Thread Safety:** Thread-safe.

**Examples:**
```c
// Latency measurement
t2_event_f("Connection_Latency_ms", 123.45);

// Memory usage as percentage
t2_event_f("Memory_Usage_Pct", 67.8);

// Query performance in milliseconds
t2_event_f("Query_Performance_ms", 45.2);
```


## Usage Patterns

### Basic Telemetry Integration

```c
#include "telemetry2_0.h"
#include <stdio.h>

int main(void) {
    // Initialize (returns void; component name must not be NULL)
    t2_init("MyComponent");
    
    // Send boot event
    t2_event_s("Application_Start", "v1.0.0");
    
    // Application runs...
    
    // Send shutdown event
    t2_event_s("Application_Stop", "clean_shutdown");
    
    // Cleanup
    t2_uninit();
    return 0;
}
```

### Error Reporting

```c
void handle_error(const char* component, int error_code) {
    char marker[128];
    char details[256];
    
    // Construct marker name
    snprintf(marker, sizeof(marker), "%s_Error", component);
    
    // Construct error details
    snprintf(details, sizeof(details), 
             "code=%d,message=%s", 
             error_code, strerror(error_code));
    
    // Send event
    t2_event_s(marker, details);
    
    // Also log locally
    syslog(LOG_ERR, "%s: %s", marker, details);
}
```

### Performance Monitoring

```c
void monitor_operation(const char* operation_name) {
    struct timespec start, end;
    double duration_ms;
    char marker[128];
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    // Perform operation...
    do_operation();
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    // Calculate duration
    duration_ms = (end.tv_sec - start.tv_sec) * 1000.0 +
                  (end.tv_nsec - start.tv_nsec) / 1000000.0;
    
    // Report if slow: use t2_event_f for floating-point values
    if (duration_ms > 100.0) {
        snprintf(marker, sizeof(marker), "%s_Duration_ms", operation_name);
        t2_event_f(marker, duration_ms);
    }
}
```

### State Transitions

```c
typedef enum {
    STATE_IDLE,
    STATE_CONNECTING,
    STATE_CONNECTED,
    STATE_DISCONNECTED
} connection_state_t;

void report_state_change(connection_state_t old_state, 
                          connection_state_t new_state) {
    const char* state_names[] = {
        "IDLE", "CONNECTING", "CONNECTED", "DISCONNECTED"
    };
    
    t2_event_s("Connection_State_Change", state_names[new_state]);
}
```

## Best Practices

### 1. Marker Naming Conventions

Use clear, hierarchical naming:

```c
// GOOD
t2_event_s("WIFI_Connection_Success", "5GHz");
t2_event_s("Storage_Disk_Full", "/var");
t2_event_s("Process_Crash", "watchdog");

// AVOID
t2_event_s("event1", "data");
t2_event_s("err", "err");
```

**Recommended format**: `Component_SubComponent_Event`

### 2. Value Format

Provide structured, parseable values:

```c
// GOOD - Structured
t2_event_s("Connection_Info", "ssid=MyNetwork,channel=6,rssi=-65");

// GOOD - Simple
t2_event_s("Boot_Status", "success");

// AVOID - Unstructured prose
t2_event_s("Event", "The connection was established after 3 retries");
```

### 3. Error Handling

Telemetry should never cause application failure:

```c
// GOOD - Defensive guard before sending
if (marker_name && value) {
    t2_event_s(marker_name, value);
}

// GOOD - Check return value of event functions
if (t2_event_s("Boot_Status", "up") != T2ERROR_SUCCESS) {
    syslog(LOG_WARNING, "Telemetry event send failed");
    // Application continues
}

// NOTE: t2_init() returns void; it cannot fail with a return code.
// Always pass a valid non-NULL component name.
t2_init("MyComponent");
```

### 4. Resource Efficiency

Don't spam events:

```c
// GOOD - Throttled
static time_t last_report = 0;
time_t now = time(NULL);

if (now - last_report >= 60) {  // Max once per minute
    t2_event_d("CPU_Usage", get_cpu_usage());
    last_report = now;
}

// AVOID - Too frequent
while (1) {
    t2_event_d("CPU_Usage", get_cpu_usage());  // Don't!
    usleep(1000);
}
```

### 5. Lifecycle Management

```c
// GOOD - Proper lifecycle
void init_application(void) {
    t2_init("MyComponent");  // Must pass valid non-NULL name
    // Other init...
}

void cleanup_application(void) {
    // Other cleanup...
    t2_uninit();  // Call last
}

// In main
atexit(cleanup_application);
init_application();
```

## Performance Considerations

### Event Queue

- **Queue Size**: 200 events
- **Behavior**: Newest events dropped when full
- **Recommendation**: Don't send >10 events/sec sustained

### Memory

- **Per Event**: ~100 bytes
- **Total Overhead**: ~1MB typical
- **Recommendation**: Limit marker count in profiles

### CPU

- **Event Processing**: <1ms per event
- **Report Generation**: ~10ms per report
- **Recommendation**: Events should be fire-and-forget

## Common Patterns

### Application Monitoring

```c
// Track application lifecycle
void app_lifecycle_monitoring(void) {
    t2_event_s("App_Start", VERSION);
    
    // ... application runs ...
    
    t2_event_s("App_Stop", "normal");
}
```


### Health Monitoring

```c
void health_check(void) {
    // Memory check (integer KB value)
    long mem_free = get_free_memory_kb();
    if (mem_free < 1024) {  // Low memory
        t2_event_d("Memory_Low_KB", (int)mem_free);
    }
    
    // Disk check (integer MB value)
    long disk_free = get_free_disk_mb();
    if (disk_free < 100) {  // Low disk
        t2_event_d("Disk_Low_MB", (int)disk_free);
    }
    
    // CPU check (floating point - use t2_event_f for double)
    double cpu = get_cpu_usage();
    if (cpu > 90.0) {  // High CPU
        t2_event_f("CPU_High_Pct", cpu);
    }
}
```

## Error Codes

Event functions return `T2ERROR` (defined in `telemetry2_0.h`). `t2_init()` and `t2_uninit()` return `void`.

| Code | Value | Meaning |
|------|-------|---------|
| `T2ERROR_SUCCESS` | `0` | Operation succeeded |
| `T2ERROR_FAILURE` | `1` | Generic failure |
| `T2ERROR_INVALID_ARGS` | — | NULL or invalid parameter passed |
| `T2ERROR_MEMALLOC_FAILED` | — | Memory allocation failed |
| `T2ERROR_COMPONENT_NULL` | — | `t2_init()` not called before event send |
| `T2ERROR_INVALID_PROFILE` | — | Profile data is malformed |
| `T2ERROR_PROFILE_NOT_FOUND` | — | Referenced profile does not exist |

## Thread Safety Summary

| Function | Return Type | Thread Safety |
|----------|-------------|---------------|
| `t2_init()` | `void` | ❌ Not thread-safe |
| `t2_uninit()` | `void` | ❌ Not thread-safe |
| `t2_event_s()` | `T2ERROR` | ✅ Thread-safe |
| `t2_event_d()` | `T2ERROR` | ✅ Thread-safe |
| `t2_event_f()` | `T2ERROR` | ✅ Thread-safe |

## See Also

- [Architecture Overview](../architecture/overview.md) - System design
- [Component Documentation](../../source/docs/) - Component details
- [Integration Guide](../integration/developer-guide.md) - Developer guide
- [Troubleshooting](../troubleshooting/common-errors.md) - Common issues

---

**API Version**: 2.0  
**Last Updated**: March 2026
