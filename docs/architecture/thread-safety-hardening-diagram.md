# Telemetry Thread Safety Hardening - Architecture Diagram

## User Story
**[T2] [RDKB] Harden Telemetry Thread Safety Under Concurrent Load**

Harden critical synchronization paths across telemetry modules to eliminate deadlocks and race conditions under concurrent load scenarios (15+ profiles with extended offline periods).

---

## 1. High-Level Component Architecture with Threading

```mermaid
graph TB
    subgraph "External Systems"
        APPS[Applications<br/>t2_event_s/d/f calls]
        XCONF[XConf Server<br/>Configuration Source]
        COLLECTOR[Collection Server<br/>HTTPS/RBUS]
    end
    
    subgraph "Telemetry Core Process"
        subgraph "Main Thread"
            MAIN[Main Thread<br/>Initialization & Cleanup]
        end
        
        subgraph "Event Collection Thread"
            ER[Event Receiver Thread<br/>🔴 Queue processing<br/>⚠️ High cyclomatic complexity]
            EQ[(Event Queue<br/>Max: 200 events<br/>🔴 Lock contention)]
        end
        
        subgraph "Configuration Thread"
            XC[XConf Client Thread<br/>🔴 Config update races<br/>Periodic fetch]
        end
        
        subgraph "Scheduling Thread"
            SCHED[Scheduler Thread<br/>Timer-based triggers]
        end
        
        subgraph "Per-Profile Report Threads (1-15+)"
            RT1[Report Thread 1<br/>🔴 Deadlock risk<br/>plMutex + pool_mutex]
            RT2[Report Thread 2<br/>...]
            RTN[Report Thread N<br/>🔴 Connection pool blocking]
        end
        
        subgraph "Data Model Threads"
            DM[Data Model Thread<br/>TR-181/RBUS queries]
        end
        
        subgraph "Shared Resources"
            PROF[(Profile List<br/>🔴 plMutex contention<br/>⚠️ Lock ordering issues)]
            POOL[(Connection Pool<br/>🔴 pool_mutex deadlock<br/>Size: 1-5 handles<br/>⚠️ No timeout!)]
            MARKERS[(Marker Cache<br/>Hash map lookup)]
        end
    end
    
    APPS -->|t2_event_*| ER
    ER --> EQ
    EQ --> MARKERS
    MARKERS --> PROF
    
    XCONF -->|HTTPS| XC
    XC -->|🔴 Write lock| PROF
    
    SCHED -->|Trigger| PROF
    PROF --> RT1
    PROF --> RT2
    PROF --> RTN
    
    RT1 -->|Acquire| POOL
    RT2 -->|Acquire| POOL
    RTN -->|🔴 Blocks forever| POOL
    
    RT1 --> DM
    POOL -->|HTTPS| COLLECTOR
    
    style ER fill:#FFE6E6
    style RT1 fill:#FFE6E6
    style RTN fill:#FFE6E6
    style POOL fill:#FFE6E6
    style PROF fill:#FFE6E6
    style XC fill:#FFE6E6
    style EQ fill:#FFE6E6
```

**Legend:**
- 🔴 **Current Critical Issues** - Deadlocks, race conditions, or blocking problems
- ⚠️ **High Complexity Areas** - Cyclomatic complexity or maintainability concerns
- 🟢 **Hardened Solutions** - Applied in hardening effort (shown in later diagrams)

---

## 2. Thread Interaction & Synchronization Points

```mermaid
sequenceDiagram
    participant App as Application<br/>(External)
    participant ER as Event Receiver<br/>Thread
    participant XC as XConf Client<br/>Thread
    participant Sched as Scheduler<br/>Thread
    participant RT1 as Report Thread 1
    participant RT2 as Report Thread 2
    participant Pool as Connection Pool<br/>(Shared Resource)
    participant Prof as Profile List<br/>(plMutex)
    
    Note over App,Pool: 🔴 Problem Scenario: Report Generation Deadlock
    
    App->>ER: t2_event_s("WIFI_ERROR")
    activate ER
    ER->>ER: Lock erMutex
    ER->>Prof: Lock plMutex
    Note right of Prof: 🔴 DEADLOCK RISK:<br/>Lock order violation
    
    par Configuration Update (Concurrent)
        XC->>Prof: Lock plMutex<br/>🔴 Already locked!
        Note right of XC: ⏳ Blocks waiting...
    and Report Thread 1 (Concurrent)
        Sched->>RT1: Trigger report
        activate RT1
        RT1->>Prof: Lock plMutex<br/>🔴 Already locked!
        Note right of RT1: ⏳ Blocks waiting...
    and Report Thread 2 (Concurrent)
        Sched->>RT2: Trigger report
        activate RT2
        RT2->>Pool: Acquire connection
        Note right of Pool: 🔴 All handles busy
        RT2->>Pool: ⏳ Spin-wait<br/>NO TIMEOUT!
        Note right of RT2: 🔴 Can block forever<br/>if RT1 holds handle
    end
    
    ER->>Prof: Unlock plMutex
    ER->>ER: Unlock erMutex
    deactivate ER
    
    RT1->>Prof: Lock acquired
    RT1->>Pool: Acquire connection
    RT1->>Pool: ⏳ Spin-wait
    Note over RT1,RT2: 🔴 DEADLOCK:<br/>RT1 waits for pool<br/>RT2 holds pool, waits for plMutex<br/>plMutex held by XC
    
    deactivate RT1
    deactivate RT2
```

---

## 3. Critical Synchronization Mechanisms (Current State)

### Current Mutex Inventory

```mermaid
graph LR
    subgraph "Global Mutexes"
        PM[plMutex<br/>🔴 Profile List<br/>High contention]
        POOLM[pool_mutex<br/>🔴 Connection Pool<br/>Deadlock risk]
        ERM[erMutex<br/>Event Queue]
        SCM[scMutex<br/>Scheduler]
        XCM[xcMutex<br/>XConf Client]
    end
    
    subgraph "Per-Profile Mutexes"
        RIPM[reportInProgressMutex<br/>Per profile]
        TCM[triggerCondMutex<br/>Per profile]
        EM[eventMutex<br/>Per profile]
        RM[reportMutex<br/>Per profile]
    end
    
    subgraph "Condition Variables"
        RIPC[reportInProgressCond]
        RC[reportcond]
        ERC[erCond]
        SCC[xcCond]
    end
    
    PM ---|🔴 Lock order<br/>violation risk| RIPM
    POOLM ---|🔴 Circular<br/>dependency| PM
    PM ---|Used by| ERM
    
    RIPM -.Signal.-> RIPC
    RM -.Signal.-> RC
    ERM -.Signal.-> ERC
    XCM -.Signal.-> SCC
    
    style PM fill:#FFE6E6
    style POOLM fill:#FFE6E6
    style RIPM fill:#FFE6E6
```

### 🔴 Current Lock Ordering Issues

**No documented lock ordering!** Current code exhibits these patterns:

```c
// Pattern 1: Event Receiver -> Profile List
pthread_mutex_lock(&erMutex);
pthread_mutex_lock(&plMutex);    // ← Lock order A→B

// Pattern 2: Report Thread -> Pool
pthread_mutex_lock(&plMutex);     
acquire_pool_handle();             // Acquires pool_mutex internally
// ← Lock order A→C

// Pattern 3: XConf Update -> Profile
pthread_mutex_lock(&plMutex);     // ← Can block report threads
// Long-running configuration update
pthread_mutex_unlock(&plMutex);

// Pattern 4: reportInProgress flag access
// 🔴 RACE CONDITION: Accessed without consistent protection!
if (!profile->reportInProgress) {  // ← Read without lock in some paths
    profile->reportInProgress = true;
}
```

---

## 4. Critical Data Flow: Report Generation with Concurrent Load

```mermaid
sequenceDiagram
    participant Sched as Scheduler
    participant Prof as Profile<br/>(plMutex)
    participant RT as Report Thread
    participant Pool as Connection Pool<br/>(pool_mutex)
    participant DM as Data Model<br/>Client
    participant Srv as Collection<br/>Server
    
    Note over Sched,Srv: 🔴 Problematic Flow: 15+ Profiles Under Load
    
    loop For each of 15+ profiles
        Sched->>Prof: Lock plMutex
        Sched->>Prof: Check reportInProgress
        
        alt Report NOT in progress
            Prof->>Prof: Set reportInProgress = true
            Prof->>RT: Create/signal thread
            Prof->>Prof: Unlock plMutex
            
            activate RT
            RT->>Prof: Lock plMutex<br/>🔴 Re-acquire lock!
            RT->>Prof: Get profile data
            RT->>Prof: Unlock plMutex
            
            RT->>Pool: Acquire handle<br/>Lock pool_mutex
            Note right of Pool: 🔴 BLOCKING POINT<br/>If pool exhausted,<br/>spin-wait with NO timeout
            
            alt Pool handle available
                Pool-->>RT: Return handle
                RT->>DM: Get TR-181 params
                DM-->>RT: Parameter values
                RT->>RT: Build JSON report
                RT->>Srv: HTTP POST (via CURL)
                Srv-->>RT: 200 OK
                RT->>Pool: Release handle<br/>Unlock pool_mutex
            else 🔴 All handles busy (>35s)
                Pool-->>RT: TIMEOUT (new)
                RT->>RT: Fail report
                RT->>Prof: reportInProgress = false
                Note right of RT: 🟢 HARDENED:<br/>Timeout prevents<br/>indefinite blocking
            end
            
            RT->>Prof: Lock reportInProgressMutex
            RT->>Prof: Set reportInProgress = false
            RT->>Prof: Signal reportInProgressCond
            RT->>Prof: Unlock reportInProgressMutex
            deactivate RT
            
        else 🔴 Report already in progress
            Note right of Prof: ⚠️ Skip this cycle<br/>Can accumulate delays<br/>under sustained load
            Prof->>Prof: Unlock plMutex
        end
    end
```

**Critical Path Issues:**
1. **plMutex held during thread creation** - Blocks all profile operations
2. **No pool acquisition timeout** - Can block indefinitely if pool exhausted
3. **reportInProgress flag** - Pattern allows race between check and set
4. **Profile count scales badly** - 15+ profiles = 15+ lock cycles per scheduler tick

---

## 5. Problem Areas: Annotated Critical Sections

```mermaid
graph TB
    subgraph "🔴 Problem Area 1: Report Generation Deadlock"
        P1A[Profile Update<br/>Holds plMutex]
        P1B[Report Thread<br/>Waits for plMutex]
        P1C[Connection Pool<br/>Held by another thread]
        
        P1A -->|Blocks| P1B
        P1B -->|Waits for| P1C
        P1C -->|Held by blocked thread| P1A
        
        P1Note[🔴 Circular wait:<br/>A→B→C→A]
    end
    
    subgraph "🔴 Problem Area 2: Connection Pool Exhaustion"
        P2A[15+ profiles trigger<br/>simultaneously]
        P2B[Pool size: 1-5 handles]
        P2C[No timeout on acquire]
        P2D[Threads spin-wait forever]
        
        P2A --> P2B
        P2B --> P2C
        P2C --> P2D
        
        P2Note[🔴 Starvation:<br/>Threads blocked indefinitely<br/>No backpressure mechanism]
    end
    
    subgraph "🔴 Problem Area 3: Configuration Update Race"
        P3A[XConf receives update]
        P3B[Lock plMutex]
        P3C[Delete old profiles]
        P3D[Create new profiles]
        P3E[Unlock plMutex]
        
        P3A --> P3B
        P3B --> P3C
        P3C --> P3D
        P3D --> P3E
        
        P3RC[🔴 Race condition:<br/>Report threads may access<br/>deleted profile memory<br/>Use-after-free risk]
        
        P3D -.Race.-> P3RC
    end
    
    subgraph "🔴 Problem Area 4: reportInProgress Flag Sync"
        P4A[Check: !reportInProgress]
        P4B[Set: reportInProgress = true]
        P4C[Thread 2 checks same flag]
        
        P4A -.Window.-> P4C
        P4C -.Race.-> P4B
        
        P4Note[🔴 TOCTOU Race:<br/>Time-of-check to<br/>time-of-use vulnerability<br/>Multiple threads enter<br/>critical section]
    end
    
    style P1A fill:#FFE6E6
    style P1B fill:#FFE6E6
    style P1C fill:#FFE6E6
    style P2A fill:#FFE6E6
    style P2D fill:#FFE6E6
    style P3C fill:#FFE6E6
    style P3RC fill:#FFE6E6
    style P4A fill:#FFE6E6
    style P4B fill:#FFE6E6
```

---

## 6. Hardened Architecture: Solutions Applied

### Solution 1: Documented Lock Ordering
```mermaid
graph LR
    S1[Strict Lock Hierarchy:<br/>1. plMutex global profile list<br/>2. profile mutexes instance<br/>3. pool_mutex connection pool<br/>4. erMutex event queue]
    S1A[Validation: Static analysis<br/>enforces at compile-time]
    S1B[Runtime: Lock tracking<br/>with debug assertions]
    
    S1 --> S1A
    S1 --> S1B
    
    style S1 fill:#E6FFE6
    style S1A fill:#E6FFE6
    style S1B fill:#E6FFE6
```

### Solution 2: Pool Acquisition Timeout
```mermaid
graph LR
    S2[Timeout: 35 seconds<br/>on pool acquisition]
    S2A[Fail fast: Return error<br/>instead of infinite wait]
    S2B[Backpressure: Scheduler<br/>backs off on failures]
    S2C[Metrics: Track pool<br/>contention and timeouts]
    
    S2 --> S2A
    S2 --> S2B
    S2 --> S2C
    
    style S2 fill:#E6FFE6
    style S2A fill:#E6FFE6
    style S2B fill:#E6FFE6
    style S2C fill:#E6FFE6
```

### Solution 3: Reference-Counted Profiles
```mermaid
graph LR
    S3[Profile Refcount:<br/>Atomic increment/decrement]
    S3A[Safe deletion:<br/>Wait for refcount = 0]
    S3B[Use-after-free:<br/>Prevented by refcount]
    
    S3 --> S3A
    S3 --> S3B
    
    style S3 fill:#E6FFE6
    style S3A fill:#E6FFE6
    style S3B fill:#E6FFE6
```

### Solution 4: Atomic reportInProgress
```mermaid
graph LR
    S4[Atomic flag:<br/>Compare-and-swap]
    S4A[Race-free:<br/>Only one thread succeeds]
    S4B[No mutex needed:<br/>Reduced contention]
    
    S4 --> S4A
    S4 --> S4B
    
    style S4 fill:#E6FFE6
    style S4A fill:#E6FFE6
    style S4B fill:#E6FFE6
```

### Solution 5: Fine-Grained Locking
```mermaid
graph LR
    S5[Per-profile locks:<br/>Replace coarse plMutex]
    S5A[Concurrent profiles:<br/>Different profiles do not block]
    S5B[Reduced contention:<br/>15+ profiles scale better]
    
    S5 --> S5A
    S5 --> S5B
    
    style S5 fill:#E6FFE6
    style S5A fill:#E6FFE6
    style S5B fill:#E6FFE6
```

### Solution 6: ThreadSanitizer Integration
```mermaid
graph LR
    S6[TSan enabled:<br/>Detect races at runtime]
    S6A[CI/CD integration:<br/>Automated testing]
    S6B[Production monitoring:<br/>Detect edge cases]
    
    S6 --> S6A
    S6 --> S6B
    
    style S6 fill:#E6FFE6
    style S6A fill:#E6FFE6
    style S6B fill:#E6FFE6
```

---

## 7. Hardened Report Generation Flow (After Fixes)

```mermaid
sequenceDiagram
    participant Sched as Scheduler
    participant Prof as Profile<br/>(Fine-grained lock)
    participant RT as Report Thread
    participant Pool as Connection Pool<br/>(With timeout)
    participant Srv as Server
    
    Note over Sched,Srv: 🟢 Hardened Flow: Safe Under 15+ Concurrent Profiles
    
    Sched->>Prof: Lock profile→scheduleMutex<br/>🟢 Fine-grained, not global
    Sched->>Prof: Atomic CAS reportInProgress<br/>🟢 Race-free
    
    alt CAS succeeded
        Prof->>Prof: Increment refcount<br/>🟢 Prevent deletion
        Prof-->>Sched: Success
        Sched->>Prof: Unlock scheduleMutex
        
        Sched->>RT: Signal thread
        activate RT
        
        RT->>Prof: Lock profile→dataMutex<br/>🟢 Independent of schedule lock
        RT->>Prof: Read profile config
        RT->>Prof: Unlock dataMutex
        
        RT->>Pool: acquire_pool_handle()<br/>with 35s timeout
        
        alt Pool handle available
            Pool-->>RT: Handle acquired
            RT->>Srv: HTTP POST
            Srv-->>RT: 200 OK
            RT->>Pool: Release handle
            
        else 🟢 Timeout after 35s
            Pool-->>RT: T2ERROR_FAILURE
            RT->>RT: Log pool timeout
            RT->>Sched: Signal backoff
            Note right of Sched: 🟢 Scheduler adjusts<br/>retry interval
        end
        
        RT->>Prof: Atomic store reportInProgress = false
        RT->>Prof: Decrement refcount<br/>🟢 Safe to delete if 0
        deactivate RT
        
    else CAS failed (already in progress)
        Note right of Prof: 🟢 Expected behavior<br/>No contention/blocking
        Prof-->>Sched: Skip this cycle
        Sched->>Prof: Unlock scheduleMutex
    end
```

**Improvements:**
- ✅ Fine-grained per-profile locks eliminate global contention
- ✅ Atomic CAS eliminates reportInProgress races
- ✅ Reference counting prevents use-after-free
- ✅ Pool timeout prevents indefinite blocking
- ✅ Backpressure mechanism handles load spikes

---

## 8. Lock Ordering Hierarchy (Hardened)

```mermaid
graph TD
    L1[Level 1: Profile List Lock<br/>profileListMutex<br/>🟢 Short critical sections only]
    L2[Level 2: Profile Instance Locks<br/>profile→scheduleMutex<br/>profile→dataMutex<br/>profile→eventMutex<br/>🟢 Independent per profile]
    L3[Level 3: Connection Pool<br/>pool_mutex<br/>🟢 Timeout-protected]
    L4[Level 4: Event Queue<br/>erMutex<br/>🟢 Lowest priority]
    
    L1 -->|May acquire| L2
    L2 -->|May acquire| L3
    L2 -->|May acquire| L4
    
    L1 -.Never.-> L3
    L1 -.Never.-> L4
    L3 -.Never.-> L1
    L3 -.Never.-> L2
    L4 -.Never.-> L1
    
    RULE1[🟢 Rule: Always acquire<br/>in descending order<br/>Never hold L2+ while acquiring L1]
    RULE2[🟢 Rule: Pool operations<br/>must not hold profile locks<br/>Release before acquire_pool_handle]
    RULE3[🟢 Validation: Static analyzer<br/>enforces at compile time<br/>ThreadSanitizer checks at runtime]
    
    style L1 fill:#E6FFE6
    style L2 fill:#E6FFE6
    style L3 fill:#E6FFE6
    style L4 fill:#E6FFE6
```

---

## 9. Validation Strategy

```mermaid
graph LR
    subgraph "🔍 Static Analysis"
        SA1[Clang Thread Safety<br/>Annotations]
        SA2[Lock Order Checker]
        SA3[Cyclomatic Complexity<br/>Analysis]
    end
    
    subgraph "🧪 Dynamic Testing"
        DT1[ThreadSanitizer TSan<br/>Race detection]
        DT2[Deadlock Detector<br/>Lock cycle detection]
        DT3[Load Testing<br/>15+ concurrent profiles]
    end
    
    subgraph "📊 Production Monitoring"
        PM1[Lock contention metrics]
        PM2[Pool timeout counters]
        PM3[Report failure rates]
    end
    
    SA1 --> CODE[Codebase]
    SA2 --> CODE
    SA3 --> CODE
    
    CODE --> DT1
    CODE --> DT2
    CODE --> DT3
    
    DT1 --> PASS{All checks<br/>pass?}
    DT2 --> PASS
    DT3 --> PASS
    
    PASS -->|Yes| DEPLOY[Deploy]
    PASS -->|No| FIX[Fix Issues]
    FIX --> CODE
    
    DEPLOY --> PM1
    DEPLOY --> PM2
    DEPLOY --> PM3
    
    style SA1 fill:#E6F3FF
    style DT1 fill:#FFF9E6
    style PM1 fill:#F0E6FF
```

---

## 10. Summary: Before vs. After Hardening

| Aspect | 🔴 Before Hardening | 🟢 After Hardening |
|--------|---------------------|-------------------|
| **Lock Ordering** | Undocumented, ad-hoc | Strict hierarchy enforced by static analysis |
| **Pool Blocking** | Infinite spin-wait | 35s timeout with backpressure |
| **Profile Deletion** | Use-after-free risk | Reference-counted, safe deletion |
| **reportInProgress** | TOCTOU race condition | Atomic compare-and-swap |
| **Concurrency** | Global plMutex bottleneck | Per-profile fine-grained locks |
| **Validation** | Manual testing only | TSan + static analysis + load tests |

---

## Acceptance Criteria Coverage

✅ **Report generation/connection deadlocks eliminated** - Pool timeout + lock ordering  
✅ **Configuration client synchronization hardened** - Reference counting + fine-grained locks  
✅ **Profile lifecycle race conditions resolved** - Atomic flags + proper synchronization  
✅ **ThreadSanitizer integration complete** - CI/CD automated testing  
✅ **Cyclomatic complexity reduced** - Refactored critical paths  
✅ **Production-grade reliability verified** - Load tested with 15+ profiles under prolonged offline periods  

---

## References

- Main implementation: [source/bulkdata/profile.c](../../source/bulkdata/profile.c)
- Connection pool: [source/protocol/http/multicurlinterface.c](../../source/protocol/http/multicurlinterface.c)
- Configuration client: [source/xconf-client/xconfclient.c](../../source/xconf-client/xconfclient.c)
- Event receiver: [source/bulkdata/t2eventreceiver.c](../../source/bulkdata/t2eventreceiver.c)
- Architecture overview: [overview.md](./overview.md)

---

