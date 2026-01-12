# RDK Telemetry 2.0

A comprehensive telemetry framework for RDK devices that collects, processes, and reports device metrics and events.

## Architecture Overview

```mermaid
graph TB
    subgraph "External Interfaces"
        TR181[TR-181 Data Model]
        XCONF[XConf Server]
        WEBCONFIG[WebConfig]
        RBUS[RBUS/CCSP Bus]
    end
    
    subgraph "Telemetry Core"
        T2MAIN[Telemetry 2.0 Main]
        PROFILES[Report Profiles]
        BULKDATA[Bulk Data Collection]
        SCHEDULER[Scheduler]
        EVENTS[Event Receiver]
    end
    
    subgraph "Data Processing"
        PARSER[T2 Parser]
        REPORTGEN[Report Generator]
        PROTOCOL[Protocol Handler]
        UTILS[Utilities]
    end
    
    subgraph "Interfaces"
        CCSP[CCSP Interface]
        DCA[DCA Utils]
        HTTP[HTTP Client]
    end
    
    TR181 --> T2MAIN
    XCONF --> PROFILES
    WEBCONFIG --> PROFILES
    RBUS --> EVENTS
    
    T2MAIN --> PROFILES
    PROFILES --> BULKDATA
    BULKDATA --> SCHEDULER
    EVENTS --> BULKDATA
    
    BULKDATA --> PARSER
    PARSER --> REPORTGEN
    REPORTGEN --> PROTOCOL
    PROTOCOL --> HTTP
    
    T2MAIN --> CCSP
    BULKDATA --> DCA
    SCHEDULER --> UTILS
```

## Component Flow

```mermaid
sequenceDiagram
    participant Config as Configuration
    participant T2Main as Telemetry Main
    participant Profiles as Report Profiles
    participant Events as Event System
    participant Reports as Report Generator
    participant Upload as Upload Service
    
    Config->>T2Main: Initialize
    T2Main->>Profiles: Load Profiles
    Profiles->>Events: Register Markers
    
    loop Data Collection
        Events->>Profiles: Collect Events
        Profiles->>Reports: Generate Report
        Reports->>Upload: Send to Backend
    end
```

## Key Components

- **Telemetry 2.0 Main** (`source/telemetry2_0.c`) - Core daemon and signal handling
- **Bulk Data** (`source/bulkdata/`) - Profile management and data collection
- **Report Generator** (`source/reportgen/`) - Format and generate telemetry reports
- **Event Receiver** (`source/bulkdata/t2eventreceiver.c`) - Asynchronous event processing
- **Scheduler** (`source/scheduler/`) - Timed report generation
- **CCSP Interface** (`source/ccspinterface/`) - CCSP/RBUS communication
- **Protocol Handler** (`source/protocol/`) - HTTP/HTTPS upload protocols

## Testing

### Container-Based Development Environment

Two pre-built containers are available for faster development and testing:

#### Available Containers
- **Native Platform Container** (`ghcr.io/rdkcentral/docker-device-mgt-service-test/native-platform`) - Complete RDK environment with telemetry dependencies
- **Mock XConf Container** (`ghcr.io/rdkcentral/docker-device-mgt-service-test/mockxconf`) - Mock XConf services for testing

#### Quick Container Setup

**Option 1: Use Pre-built Containers**
```bash
# Pull and start pre-built containers
docker-compose up
```

**Option 2: Build Containers Locally**
```bash
# Clone the container source repository
git clone https://github.com/rdkcentral/docker-device-mgt-service-test.git

# Build containers locally from source
cd containers
./build.sh

# Start development environment with locally built containers
docker-compose up

# Access native platform container
docker exec -it native-platform /bin/bash

# Access mock XConf container  
docker exec -it mockxconf /bin/bash
```

#### Mock Services Available
- **XConf DCA Settings**: `https://mockxconf:50050/loguploader/getT2DCMSettings`
- **Data Lake Upload**: `https://mockxconf:50051/dataLakeMock`
- **Report Profile Setup**: Use `rbuscli setvalues Device.X_RDKCENTRAL-COM_T2.ReportProfiles string ''`

### L1 Tests (Unit Tests)
Located in `source/test/` directory with individual component tests.

```bash
# Run all unit tests
./test/run_ut.sh

# Run with coverage
./test/run_ut.sh --enable-cov

# In container environment
docker exec -it native-platform /bin/bash
cd /mnt/L2_CONTAINER_SHARED_VOLUME
./test/run_ut.sh
```

**Test Coverage:**
- Bulk data operations
- Data model processing  
- Profile management
- Report generation
- Scheduler functionality
- DCA utilities
- CCSP interface

### L2 Tests (Integration Tests)
Located in `test/functional-tests/` directory with end-to-end scenarios.

```bash
# Run integration tests
./test/run_l2.sh

# In container environment with mock services
docker exec -it native-platform /bin/bash
cd /mnt/L2_CONTAINER_SHARED_VOLUME
./test/run_l2.sh
```

**Test Scenarios:**
- Daemon lifecycle and process management
- Bootup sequence validation
- XConf server communication
- Multi-profile message packet handling

## Quick Start

### Build
```bash
autoreconf --install
./configure
make
```

### Run
```bash
# Start telemetry daemon
./source/telemetry2_0

# Set report profile
./test/set_report_profile.sh
```

### Container Support
```bash
# Build and run in container
./containers/build.sh
docker-compose -f containers/compose.yaml up
```

## License

Licensed under Apache License 2.0. See `LICENSE` file for details.
