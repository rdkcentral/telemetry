# Telemetry Embedded Systems Development Guidelines

This repository contains the Telemetry 2.0 framework for RDK (Reference Design Kit), designed for embedded devices with constrained resources.

## Project Context

- **Target Platform**: Embedded devices (STBs, gateways, IoT devices)
- **Resource Constraints**: Limited memory (often <128MB RAM), limited CPU
- **Languages**: C (primary), C++ (tests), Shell scripts
- **Build System**: Autotools (configure.ac, Makefile.am)
- **Testing**: Google Test (gtest) and Google Mock (gmock)
- **Platform Requirements**: Must be platform-independent, supporting multiple architectures

## Critical Requirements

1. **Memory Safety**: Every allocation must have a corresponding free
2. **Resource Constraints**: Minimize memory footprint and CPU usage
3. **Platform Independence**: Avoid platform-specific code; use abstraction layers
4. **Backward Compatibility**: Maintain API/ABI compatibility
5. **Zero Regression**: All changes must pass existing test suites
6. **Thread Safety**: Use lightweight synchronization, prevent deadlocks, minimize memory fragmentation
7. **Production Quality**: Code ships to millions of devices

## Architecture Principles

- Use RAII patterns in C++ test code
- Prefer stack allocation over heap when possible
- Use memory pools for frequent allocations
- Create threads with explicit stack sizes to minimize memory usage
- Use lightweight synchronization primitives (atomic operations, simple mutexes)
- Prevent deadlocks through consistent lock ordering and timeouts
- Implement proper error handling (no silent failures)
- Follow defensive programming practices
- Design for testability from the start

## Code Review Standards

All code changes must:
- Pass static analysis (cppcheck, valgrind)
- Have zero memory leaks (verified by valgrind)
- Include unit tests for new functionality
- Maintain or improve code coverage
- Follow existing naming conventions
- Include clear commit messages explaining why, not just what

Refer to language-specific instruction files in `.github/instructions/` for detailed guidelines.
