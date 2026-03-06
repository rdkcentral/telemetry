# CURL Usage Architecture Documentation

## Overview

This document describes the architecture of the CURL-based HTTP communication subsystem in the Telemetry component. The system implements a connection pooling mechanism for efficient HTTP operations with thread-safe access and mTLS support.

## Components

### 1. **multicurlinterface.c** - Connection Pool Layer
The core component implementing a thread-safe connection pool for CURL handles.

**Key Features:**
- Connection pooling with configurable pool size (1-5 connections)
- Thread-safe handle acquisition and release
- Support for both GET and POST operations
- mTLS certificate management integration
- Keep-alive and connection reuse optimization

**Key Data Structures:**
```c
typedef struct {
    CURL *easy_handle;               // Single CURL handle
    bool handle_available;           // Availability flag
    rdkcertselector_h cert_selector; // Certificate selector (LIBRDKCERTSEL_BUILD)
    rdkcertselector_h rcvry_cert_selector; // Recovery cert selector
} http_pool_entry_t;
```

**Global State:**
- `pool_entries`: Array of pool entries
- `pool_mutex`: Protects pool state (statically initialized)
- `pool_cond`: Condition variable for handle availability
- `pool_initialized`: Initialization flag
- `active_requests`: Counter for in-use handles
- `pool_size`: Configured pool size

### 2. **curlinterface.c** - Wrapper Layer
Simple wrapper providing backward compatibility and convenience functions.

**Key Functions:**
- `sendReportOverHTTP()`: Sends telemetry reports via HTTP POST
- `sendCachedReportsOverHTTP()`: Sends multiple cached reports sequentially

### 3. **xconfclient.c** - Configuration Client
Manages fetching configuration from XConf server with threading and retry logic.

**Key Features:**
- Background thread for configuration fetching
- Retry logic with exponential backoff
- Multiple condition variables for flow control
- DCM integration (when DCMAGENT is defined)

**Synchronization Primitives:**
- `xcMutex` & `xcCond`: Controls retry timing
- `xcThreadMutex` & `xcThreadCond`: Controls thread lifecycle
- `xcrThread`: Configuration fetching thread
- `dcmThread`: DCM notification thread (DCMAGENT)

## Architecture Diagrams

### Component Architecture

### HTTP GET Flow

```mermaid
sequenceDiagram
    participant XC as XConf Client
    participant MCI as multicurlinterface
    participant POOL as Connection Pool
    participant CURL as libcurl
    participant CERT as Cert Selector
    
    XC->>MCI: http_pool_get(url, response_data)
    MCI->>MCI: acquire_pool_handle(&easy, &idx)
    
    alt Handle Available
        MCI->>POOL: Lock pool_mutex
        POOL-->>MCI: Return handle & index
    else All Handles Busy
        MCI->>MCI: pthread_cond_wait(pool_cond)
        Note over MCI: Wait until handle available
        POOL-->>MCI: Return handle & index
    end
    
    MCI->>MCI: Clear POST options
    MCI->>MCI: Set GET options (URL, callback)
    
    alt mTLS Enabled
        MCI->>CERT: rdkcertselector_getCertificate()
        CERT-->>MCI: Certificate URI
        MCI->>CURL: Set CURLOPT_SSLCERT
        
        loop Retry on Certificate Error
            MCI->>CURL: curl_easy_perform()
            CURL-->>MCI: CURLcode result
            alt Certificate Error
                MCI->>CERT: rdkcertselector_setCurlStatus(TRY_ANOTHER)
                CERT-->>MCI: Next certificate
            else Success or Other Error
                MCI->>MCI: Break loop
            end
        end
    else mTLS Disabled
        MCI->>CURL: curl_easy_perform()
        CURL-->>MCI: CURLcode result
    end
    
    MCI->>MCI: release_pool_handle(idx)
    MCI->>POOL: Mark handle available
    MCI->>POOL: pthread_cond_signal(pool_cond)
    MCI-->>XC: Return T2ERROR status
```

### HTTP POST Flow

```mermaid
sequenceDiagram
    participant APP as Application
    participant CI as curlinterface
    participant MCI as multicurlinterface
    participant POOL as Connection Pool
    participant CURL as libcurl
    
    APP->>CI: sendReportOverHTTP(url, payload)
    CI->>MCI: http_pool_post(url, payload)
    
    MCI->>MCI: acquire_pool_handle(&easy, &idx)
    
    rect rgb(200, 220, 240)
        Note over MCI,POOL: Critical Section: pool_mutex locked
        POOL-->>MCI: Handle acquired
    end
    
    MCI->>MCI: Set POST options
    MCI->>CURL: CURLOPT_POSTFIELDS = payload
    MCI->>CURL: CURLOPT_HTTPHEADER = post_headers
    
    alt mTLS with Certificates
        MCI->>MCI: Configure SSL certificates
        MCI->>CURL: curl_easy_perform()
    else Standard POST
        MCI->>CURL: curl_easy_perform()
    end
    
    CURL-->>MCI: HTTP Response Code
    
    MCI->>MCI: release_pool_handle(idx)
    
    rect rgb(200, 220, 240)
        Note over MCI,POOL: Critical Section: pool_mutex locked
        MCI->>POOL: Mark handle available
        MCI->>POOL: pthread_cond_signal(pool_cond)
    end
    
    MCI-->>CI: T2ERROR status
    CI-->>APP: T2ERROR status
```

### XConf Client Thread Flow

```mermaid
sequenceDiagram
    participant MAIN as Main Thread
    participant XCT as XConf Thread
    participant MCI as multicurlinterface
    participant MUTEX as xcThreadMutex
    
    MAIN->>XCT: initXConfClient()
    activate XCT
    
    XCT->>MUTEX: pthread_mutex_lock(&xcThreadMutex)
    
    loop While isXconfInit
        XCT->>XCT: getRemoteConfigURL(&configURL)
        
        alt URL Not Ready
            XCT->>XCT: Lock xcMutex
            XCT->>XCT: pthread_cond_timedwait(xcCond, RFC_RETRY_TIMEOUT)
            Note over XCT: Wait 60 seconds
            XCT->>XCT: Unlock xcMutex
        else URL Ready
            XCT->>XCT: Break loop
        end
        
        XCT->>MCI: fetchRemoteConfiguration(url, &data)
        MCI-->>XCT: Configuration data
        
        alt Config Fetch Success
            XCT->>XCT: Process configuration
            XCT->>XCT: stopFetchRemoteConfiguration = true
        else Config Fetch Failed
            XCT->>XCT: Increment retry count
            XCT->>XCT: pthread_cond_timedwait(xcCond, XCONF_RETRY_TIMEOUT)
            Note over XCT: Wait 180 seconds
        end
        
        alt Done Fetching
            XCT->>XCT: pthread_cond_wait(xcThreadCond)
            Note over XCT: Wait for restart signal
        end
    end
    
    MAIN->>XCT: uninitXConfClient()
    MAIN->>MUTEX: pthread_cond_signal(xcThreadCond)
    XCT->>MUTEX: pthread_mutex_unlock(&xcThreadMutex)
    deactivate XCT
    MAIN->>XCT: pthread_join(xcrThread)
```

### Connection Pool Initialization

```mermaid
stateDiagram-v2
    [*] --> Uninitialized: pool_initialized = false
    
    Uninitialized --> Initializing: init_connection_pool()
    
    Initializing --> CheckInitialized: Lock pool_mutex
    CheckInitialized --> AlreadyInitialized: pool_initialized == true
    CheckInitialized --> ConfigureSize: pool_initialized == false
    
    ConfigureSize --> AllocatePool: Read T2_CONNECTION_POOL_SIZE
    AllocatePool --> AllocateHandles: calloc(pool_size)
    
    AllocateHandles --> ConfigureHandle1: Create CURL handles
    ConfigureHandle1 --> ConfigureHandle2: Set CURLOPT_* options
    ConfigureHandle2 --> InitCertSelector: Configure keepalive, SSL
    
    InitCertSelector --> SetupHeaders: rdkcertselector_init()
    SetupHeaders --> MarkInitialized: curl_slist_append()
    
    MarkInitialized --> Ready: pool_initialized = true
    AlreadyInitialized --> Ready
    
    Ready --> [*]: Unlock pool_mutex
    
    Ready --> Cleanup: http_pool_cleanup()
    Cleanup --> CleanupWait: Lock pool_mutex
    CleanupWait --> WaitForActive: active_requests > 0
    WaitForActive --> BroadcastShutdown: pthread_cond_broadcast()
    BroadcastShutdown --> FreeResources: Free handles & cert selectors
    FreeResources --> [*]: pool_initialized = false
```

### Handle Acquisition & Release Flow

```mermaid
flowchart TD
    Start([acquire_pool_handle]) --> Lock[Lock pool_mutex]
    Lock --> CheckInit{pool_initialized?}
    
    CheckInit -->|No| InitPool[Call init_connection_pool]
    InitPool --> CheckInitFail{Init Success?}
    CheckInitFail -->|No| ReturnError1[Return T2ERROR_FAILURE]
    CheckInitFail -->|Yes| WaitLoop
    
    CheckInit -->|Yes| WaitLoop[Enter while loop]
    
    WaitLoop --> CheckAvail{Handle available?}
    
    CheckAvail -->|Yes| FindHandle[Search for available handle]
    FindHandle --> MarkBusy[Mark handle busy]
    MarkBusy --> IncrActive[active_requests++]
    IncrActive --> Unlock1[Unlock pool_mutex]
    Unlock1 --> ReturnSuccess[Return T2ERROR_SUCCESS]
    
    CheckAvail -->|No| CheckShutdown{pool_initialized?}
    CheckShutdown -->|No| Unlock2[Unlock pool_mutex]
    Unlock2 --> ReturnError2[Return T2ERROR_FAILURE]
    
    CheckShutdown -->|Yes| CondWait[pthread_cond_wait]
    CondWait --> WaitLoop
    
    ReturnSuccess --> End1([End])
    ReturnError1 --> End2([End])
    ReturnError2 --> End3([End])
    
    Release([release_pool_handle]) --> LockR[Lock pool_mutex]
    LockR --> CheckIdx{Valid index?}
    CheckIdx -->|Yes| MarkAvail[Mark handle available]
    MarkAvail --> DecrActive[active_requests--]
    DecrActive --> Signal[pthread_cond_signal]
    Signal --> UnlockR[Unlock pool_mutex]
    
    CheckIdx -->|No| LogError[Log error]
    LogError --> UnlockR
    
    UnlockR --> EndR([End])
```

## Synchronization Mechanisms

## Critical Race Conditions & Deadlock Risks

### 🔴 HIGH RISK: Pool Cleanup During Active Requests

**Location:** `http_pool_cleanup()` in multicurlinterface.c

**Issue:**
```c
while(active_requests > 0) {
    pthread_cond_wait(&pool_cond, &pool_mutex);
}
```

**Risk:** If a thread is blocked in `acquire_pool_handle()` waiting for an available handle when cleanup starts:
1. Cleanup sets `pool_initialized = false`
2. Waiting thread wakes up, sees `!pool_initialized`, exits with error
3. If the thread had incremented `active_requests` before waiting, the counter may be incorrect
4. Cleanup may wait forever if signals are missed

**Mitigation Required:**
- Use `pthread_cond_broadcast()` instead of waiting
- Add shutdown flag checked in acquire path
- Ensure proper signal on all return paths



### 🟡 MEDIUM RISK: Static Initialization Race

**Location:** `init_connection_pool()` in multicurlinterface.c

**Issue:**
```c
static pthread_mutex_t pool_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool pool_initialized = false;

// Later in code:
pthread_mutex_lock(&pool_mutex);
if(pool_initialized) {
    pthread_mutex_unlock(&pool_mutex);
    return T2ERROR_SUCCESS;
}
```

**Risk:** Although `PTHREAD_MUTEX_INITIALIZER` provides static initialization, there's a check-then-act pattern that could theoretically have issues if multiple threads call `init_connection_pool()` simultaneously before `pool_initialized` is set.

**Current Protection:** The lock is acquired before checking, so this is actually safe. However, the `acquire_pool_handle()` function calls `init_connection_pool()` while holding `pool_mutex`, which is correct but creates nested critical sections.

**Status:** Currently protected, but recursive pattern is fragile.

### 🟡 MEDIUM RISK: Condition Variable Missed Signals

**Location:** Various locations using `pthread_cond_wait()`


### 🟢 LOW RISK: Active Request Counter

**Location:** `acquire_pool_handle()` and `release_pool_handle()` in multicurlinterface.c

**Issue:**
```c
active_requests++;  // In acquire
active_requests--;  // In release
```

**Risk:** Counter could become inaccurate if:
- Release called without acquire
- Error path doesn't decrement
- Signal handling interrupts

**Current Protection:** 
- All paths protected by `pool_mutex`
- index validation in release prevents double-release

**Status:** Well-protected but no assertions to detect corruption.


## Thread Safety Analysis

### Thread-Safe Operations
✅ `http_pool_get()` - Protected by pool_mutex
✅ `http_pool_post()` - Protected by pool_mutex  
✅ `init_connection_pool()` - Double-check locking with mutex
✅ `http_pool_cleanup()` - Synchronized with broadcast
✅ Handle acquisition/release - Mutex + condition variable

### Non-Thread-Safe (By Design)
⚠️ Individual CURL handle operations - Assumes single thread per handle (correct)
⚠️ Certificate selector callbacks - Called within handle's execution context (correct)


## Recommended Improvements

### 1. Add Timeout to Acquire - Prevent deadlocks

```c
T2ERROR acquire_pool_handle_timeout(CURL **easy, int *idx, int timeout_ms);
```

### 2. Separate Cert Selectors from Pool (Not needed now for pool sie 1)

Consider managing cert selectors independently to reduce coupling and simplify state management.



## Testing Recommendations

### Unit Tests Needed
1. ✅ Pool initialization and cleanup
2. ✅ Concurrent acquire/release from multiple threads
3. ✅ Pool exhaustion behavior (all handles busy)
4. ⚠️ Cleanup during active requests
5. ⚠️ Signal/wait patterns with multiple threads
6. ⚠️ Lock ordering validation
7. ⚠️ Environment variable edge cases

### Integration Tests Needed
1. ✅ Full GET/POST operations
2. ⚠️ Certificate rotation scenarios
3. ⚠️ Network failures and retries
4. ⚠️ Graceful shutdown with pending requests
5. ⚠️ Pool size scaling under load

### Stress Tests Needed
1. High-frequency requests (pool saturation)
2. Long-running requests (blocking pool) simulated by long responding connection
3. Rapid init/cleanup cycles
4. Mixed GET/POST workloads

---

**Document Version:** 1.0  
**Last Updated:** February 24, 2026  
**Author:** Architecture Analysis  
**Status:** Initial Analysis
