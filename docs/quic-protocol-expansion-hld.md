# High-Level Design: QUIC Protocol Support for RDK Telemetry 2.0

## Document Information
- **Version**: 1.0
- **Date**: November 2024
- **Author**: Senior Developer
- **Status**: POC Design

## Executive Summary

This document outlines the high-level design for expanding RDK Telemetry 2.0 reporting protocols to include QUIC support alongside the existing HTTP and RBUS_METHOD protocols. The implementation follows a POC approach using a Go binary system call mechanism for QUIC transport.

### QUIC Integration Overview

```mermaid
graph LR
    A[RDK Telemetry 2.0] --> B{Protocol Support}
    B --> C[HTTP - Existing]
    B --> D[RBUS_METHOD - Existing]
    B --> E[QUIC - New]
    
    E --> F[Go Binary]
    F --> G[QUIC Endpoint]
    
    style E fill:#e8f5e8,stroke:#4caf50,stroke-width:3px
    style F fill:#e8f5e8,stroke:#4caf50,stroke-width:2px
    style G fill:#e8f5e8,stroke:#4caf50,stroke-width:2px
```

## Table of Contents
1. [Current Architecture Analysis](#current-architecture-analysis)
2. [QUIC Protocol Integration Design](#quic-protocol-integration-design)
3. [System Architecture](#system-architecture)
4. [Schema Extensions](#schema-extensions)
5. [Implementation Strategy](#implementation-strategy)
6. [Security Considerations](#security-considerations)
7. [Performance Considerations](#performance-considerations)
8. [Future Enhancements](#future-enhancements)

## Current Architecture Analysis

### Existing Protocol Support

The current Telemetry 2.0 system supports two protocols:

1. **HTTP Protocol** (`source/protocol/http/`)
   - Uses libcurl for HTTP/HTTPS transport
   - Supports POST/PUT methods
   - Handles compression (gzip)
   - Manages SSL/TLS certificates

2. **RBUS_METHOD Protocol** (`source/protocol/rbusMethod/`)
   - Uses RBUS framework for inter-component communication
   - Method-based parameter passing
   - Local system transport

### Current Protocol Flow

```mermaid
graph TD
    A[Telemetry Report Generator] --> B{Protocol Selection}
    B -->|HTTP| C[curlinterface.c]
    B -->|RBUS_METHOD| D[rbusmethodinterface.c]
    C --> E[libcurl]
    D --> F[RBUS Framework]
    E --> G[Remote HTTP Endpoint]
    F --> H[Local RBUS Method]
```

## QUIC Protocol Integration Design

### Design Principles

1. **Minimal Core Changes**: Leverage existing protocol abstraction
2. **External Binary Approach**: Use Go binary for QUIC implementation
3. **Consistent Interface**: Maintain same API pattern as existing protocols
4. **Error Handling**: Robust error reporting and fallback mechanisms

### QUIC Protocol Architecture

```mermaid
graph TD
    A[Telemetry Report Generator] --> B{Protocol Selection}
    B -->|HTTP| C[curlinterface.c]
    B -->|RBUS_METHOD| D[rbusmethodinterface.c]
    B -->|QUIC| E[quicinterface.c]
    
    C --> F[libcurl]
    D --> G[RBUS Framework]
    E --> H[System Call]
    
    F --> I[Remote HTTP Endpoint]
    G --> J[Local RBUS Method]
    H --> K[Go QUIC Binary]
    K --> L[Remote QUIC Endpoint]
    
    style E fill:#e1f5fe
    style H fill:#e1f5fe
    style K fill:#e1f5fe
    style L fill:#e1f5fe
```

### QUIC Implementation Components

```mermaid
classDiagram
    class QuicInterface {
        +sendReportOverQUIC(url, payload, pid) T2ERROR
        +sendCachedReportsOverQUIC(url, reportList) T2ERROR
        -executeQuicBinary(args) int
        -validateQuicResponse(response) bool
    }
    
    class QuicBinary {
        +main(args) int
        +establishConnection(endpoint) error
        +sendPayload(data) error
        +handleResponse() error
    }
    
    class QuicConfig {
        +endpoint string
        +timeout int
        +retryCount int
        +compression string
        +tlsConfig TLSConfig
    }
    
    QuicInterface --> QuicBinary : "system call"
    QuicInterface --> QuicConfig : "uses"
```

## System Architecture

### Protocol Selection Flow

```mermaid
sequenceDiagram
    participant TG as Telemetry Generator
    participant PS as Protocol Selector
    participant QI as QUIC Interface
    participant QB as QUIC Binary
    participant QE as QUIC Endpoint
    
    TG->>PS: Send Report Request
    PS->>PS: Parse Protocol Config
    alt Protocol == "QUIC"
        PS->>QI: sendReportOverQUIC()
        QI->>QI: Validate Config
        QI->>QB: Execute Go Binary
        QB->>QE: Establish QUIC Connection
        QE-->>QB: Connection Established
        QB->>QE: Send Payload
        QE-->>QB: Response/ACK
        QB-->>QI: Exit Code + Output
        QI-->>PS: T2ERROR Status
        PS-->>TG: Report Status
    end
```

### Error Handling Flow

```mermaid
graph TD
    A[QUIC Send Request] --> B{Go Binary Execution}
    B -->|Success| C[Parse Response]
    B -->|Failure| D[Log Error]
    C -->|Valid Response| E[Return Success]
    C -->|Invalid Response| F[Return Error]
    D --> G{Retry Logic}
    F --> G
    G -->|Retries Available| H[Increment Counter]
    G -->|Max Retries| I[Return Final Error]
    H --> A
```

## Schema Extensions

### JSON Schema Updates

The `t2_reportProfileSchema.schema.json` needs to be extended to support QUIC protocol:

#### Protocol Enum Extension
```json
{
  "Protocol": {
    "type": "string",
    "enum": ["HTTP", "RBUS_METHOD", "QUIC"],
    "description": "The protocol to be used for the upload of report generated by this profile."
  }
}
```

#### QUIC Protocol Definition
```json
{
  "protocolDefinitions": {
    "properties": {
      "QUIC": {
        "title": "QUIC Definition",
        "type": "object",
        "properties": {
          "Endpoint": {
            "type": "string",
            "description": "QUIC endpoint URL (quic://host:port)"
          },
          "Timeout": {
            "type": "integer",
            "default": 30,
            "description": "Connection timeout in seconds"
          },
          "Compression": {
            "type": "string",
            "enum": ["None"],
            "default": "None",
            "description": "Compression method for payload"
          },
          "RetryCount": {
            "type": "integer",
            "default": 3,
            "description": "Number of retry attempts"
          },
          "BinaryPath": {
            "type": "string",
            "default": "/usr/bin/quic-sender",
            "description": "Path to QUIC Go binary"
          }
        },
        "required": ["Endpoint"],
        "description": "QUIC Protocol details for report transmission"
      }
    }
  }
}
```

#### Schema Validation Rules
```json
{
  "anyOf": [
    {
      "properties": {
        "Protocol": { "const": "HTTP" }
      },
      "required": ["HTTP"]
    },
    {
      "properties": {
        "Protocol": { "const": "RBUS_METHOD" }
      },
      "required": ["RBUS_METHOD"]
    },
    {
      "properties": {
        "Protocol": { "const": "QUIC" }
      },
      "required": ["QUIC"]
    }
  ]
}
```

## Implementation Strategy

### Phase 1: Core Infrastructure
1. **QUIC Interface Module** (`source/protocol/quic/quicinterface.c`)
   - Implement system call wrapper
   - Error handling and logging
   - Configuration parsing

2. **Go Binary Development**
   - QUIC client will be implemented in a separate Go binary
   - The binary will be called using system call

### Phase 2: Integration
1. **Protocol Factory Updates**
   - Extend protocol selection logic
   - Add QUIC protocol instantiation

2. **Configuration Validation**
   - Schema validation for QUIC parameters
   - Runtime configuration checks

### Phase 3: Testing & Validation
1. **Unit Testing**
   - QUIC interface module tests
   - Go binary functionality tests

2. **Integration Testing**
   - End-to-end protocol testing
   - Error scenario validation

### Directory Structure
```
telemetry/
├── source/
│   └── protocol/
│       ├── http/
│       ├── rbusMethod/
│       └── quic/              # New QUIC protocol implementation
│           ├── Makefile.am
│           ├── quicinterface.c
│           ├── quicinterface.h

├── schemas/
│   └── t2_reportProfileSchema.schema.json  # Updated schema
└── docs/
    └── quic-protocol-expansion-hld.md      # This document
```

## Security Considerations

### Transport Security
- Security is handled by the QUIC Go binary

### System Security
- **Binary Validation**: Relaxed for POC
- **Input Sanitization**: Validate all configuration parameters

### Network Security
```mermaid
graph TD
    A[Telemetry Data] --> B[JSON Payload]
    B -->  F[QUIC]
    F --> G[Remote Endpoint]
```

## Performance Considerations


### Performance Metrics
```mermaid
graph LR
    A[Performance Metrics] 
    A --> C[Throughput]
    A --> D[CPU Usage]
    A --> E[Memory Usage]
    
  
    C --> G[Payload Size Impact]
    D --> H[Go Binary Overhead]
    E --> I[Memory Footprint]
```

### Optimization Strategies
1. **Connection Pooling**: Reuse QUIC connections when possible
2. **Batch Processing**: Send multiple reports in single connection
3. **Compression**: Optimize payload size with gzip compression

### Complete QUIC Data Flow

```mermaid
flowchart TD
    A[Telemetry Data Collection] --> B[Report Generation]
    B --> C[Protocol Configuration Parse]
    C --> D{Protocol Type?}
    
    D -->|HTTP| E[HTTP Handler]
    D -->|RBUS_METHOD| F[RBUS Handler]
    D -->|QUIC| G[QUIC Handler]
    
    G --> H[Validate QUIC Config]
    H --> I[Prepare Payload]
    I --> J[Execute Go Binary]
    J --> K[QUIC Connection]
    K --> L[Send Data]
    L --> M[Receive Response]
    M --> N[Parse Result]
    N --> O[Return Status]
    
    E --> P[HTTP Response]
    F --> Q[RBUS Response]
    O --> R[QUIC Response]
    
    P --> S[Log Result]
    Q --> S
    R --> S
    
    style G fill:#e1f5fe,stroke:#01579b,stroke-width:2px
    style H fill:#e1f5fe,stroke:#01579b,stroke-width:2px
    style I fill:#e1f5fe,stroke:#01579b,stroke-width:2px
    style J fill:#e1f5fe,stroke:#01579b,stroke-width:2px
    style K fill:#e1f5fe,stroke:#01579b,stroke-width:2px
    style L fill:#e1f5fe,stroke:#01579b,stroke-width:2px
    style M fill:#e1f5fe,stroke:#01579b,stroke-width:2px
    style N fill:#e1f5fe,stroke:#01579b,stroke-width:2px
    style O fill:#e1f5fe,stroke:#01579b,stroke-width:2px
    style R fill:#e1f5fe,stroke:#01579b,stroke-width:2px
```

## Future Enhancements

### Phase 2 Enhancements
1. **Native QUIC Library**: Replace Go binary with native C implementation


## Conclusion

This HLD provides a comprehensive approach to integrating QUIC protocol support into RDK Telemetry 2.0. The design maintains backward compatibility while introducing modern transport capabilities through a well-architected POC implementation using Go binary system calls.

The modular approach ensures minimal impact on existing functionality while providing a foundation for future enhancements and native QUIC implementation.

---

**Next Steps:**
1. Review and approve this HLD
2. Begin Phase 1 implementation
3. Develop comprehensive test suite
