# Telemetry 2.0 Source Code Feature Documentation

This folder contains BDD (Behavior Driven Development) feature files documenting the functionality implemented in the `source/` directory of the Telemetry 2.0 project.

## Feature Files Overview

| Feature File | Source Components | Description |
|--------------|-------------------|-------------|
| `telemetry_daemon.feature` | `telemetry2_0.c` | Core daemon functionality, signal handling, initialization |
| `xconf_client.feature` | `xconf-client/` | Remote configuration management via XConf server |
| `report_profiles.feature` | `bulkdata/reportprofiles.c` | Report profile management and processing |
| `profile_management.feature` | `bulkdata/profile.c` | Profile lifecycle, thread safety, CRUD operations |
| `scheduler.feature` | `scheduler/` | Report scheduling and timing management |
| `report_generation.feature` | `reportgen/` | Report encoding and generation |
| `dca_log_processing.feature` | `dcautil/` | Log file processing and pattern matching |
| `event_receiver.feature` | `bulkdata/t2eventreceiver.c` | Event queue and dispatch handling |
| `t2_parser.feature` | `t2parser/` | JSON and MessagePack configuration parsing |
| `protocol_http.feature` | `protocol/http/` | HTTP protocol for report transmission |
| `protocol_rbus.feature` | `protocol/rbusMethod/` | RBUS method protocol for report transmission |
| `rbus_interface.feature` | `ccspinterface/rbusInterface.c` | RBUS data model and event handling |
| `persistence.feature` | `utils/persistence.c` | Persistent storage management |
| `t2_markers.feature` | `bulkdata/t2markers.c` | Telemetry marker management |
| `utils.feature` | `utils/` | Utility functions (vector, hash map, logging) |

## Source Directory Mapping

Based on `source/Makefile.am`, the following directories are compiled:

### Always Compiled
- `utils/` - Utility functions and data structures
- `ccspinterface/` - CCSP/RBUS interface layer
- `t2parser/` - Configuration parsing (JSON/MessagePack)
- `xconf-client/` - XConf server communication
- `protocol/` - Report transmission protocols (HTTP, RBUS)
- `reportgen/` - Report generation and encoding
- `scheduler/` - Report scheduling
- `dcautil/` - Log processing and DCA utilities
- `commonlib/` - Common library functions
- `bulkdata/` - Profile and report management

### Conditionally Compiled
- `privacycontrol/` - Privacy mode control (if IS_PRIVACYCONTROL_ENABLED)
- `t2dm/` - T2 Data Model (if ENABLE_CCSP_SUPPORT)
- `test/` - Unit tests (if WITH_GTEST_SUPPORT)

### Not Documented (Not in Makefile.am SUBDIRS)
- `docs/` - Documentation
- `nativeProtocolSimulator/` - Development/testing tool
- `testApp/` - Test application
- `t2ssp/` - SSP integration (compiled directly into telemetry2_0)

## BDD Format

All feature files follow the Gherkin syntax used in the `test/functional-tests/features/` directory:

```gherkin
Feature: Feature Name

  Background:
    Given preconditions

  Scenario: Scenario Name
    Given initial context
    When action is performed
    Then expected outcome
```

## Usage

These feature files serve as:
1. **Documentation** - Describing expected behavior of each component
2. **Test Specification** - Basis for writing automated tests
3. **Requirements Traceability** - Linking code to functional requirements

## Generation Date

Generated: April 29, 2026

## Related Documentation

- `test/functional-tests/features/` - L2 functional test BDD documentation
- `test/functional-tests/GAP_ANALYSIS_REPORT.md` - Gap analysis between tests and documentation
