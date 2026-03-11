# Telemetry Agentic Development System

This directory contains the agentic-centric development setup for the Telemetry embedded systems project, designed to assist with C/C++/shell development on resource-constrained embedded devices.

## Overview

The agentic system provides specialized AI agents, skills, and instructions to help developers:

- **Write memory-safe code** for embedded devices with limited resources
- **Refactor legacy code** without introducing regressions
- **Maintain platform independence** across different architectures
- **Prevent memory fragmentation** while optimizing performance
- **Implement thread-safe code** with lightweight synchronization
- **Avoid deadlocks** through proper lock ordering and timeouts
- **Ensure zero regressions** through comprehensive testing

## Structure

```
.github/
├── README.md                          # This file
├── copilot-instructions.md            # Main project guidelines
├── agents/                            # Specialized AI agents
│   ├── embedded-c-expert.agent.md
│   └── legacy-refactor-specialist.agent.md
├── skills/                            # Reusable analysis skills
│   ├── memory-safety-analyzer/
│   │   └── SKILL.md
│   ├── thread-safety-analyzer/
│   │   └── SKILL.md
│   └── platform-portability-checker/
│       └── SKILL.md
├── instructions/                      # Language-specific guidelines
│   ├── c-embedded.instructions.md
│   ├── cpp-testing.instructions.md
│   ├── shell-scripts.instructions.md
│   └── build-system.instructions.md
└── workflows/                         # CI/CD automation
    └── copilot-quality-checks.yml
```

## Components

### 1. Main Instructions

**[copilot-instructions.md](copilot-instructions.md)**

- Project context and constraints
- Critical requirements for embedded development
- Architecture principles
- Code review standards

### 2. Agents

#### **Embedded C Expert** ([embedded-c-expert.agent.md](agents/embedded-c-expert.agent.md))

Expert in embedded C development focusing on:

- Memory management without garbage collection
- Thread safety with lightweight synchronization
- Resource optimization for constrained environments
- Platform-independent code patterns
- Memory leak detection and prevention
- Deadlock prevention and race condition detection

**Use when:**

- Writing new C code
- Reviewing memory-critical changes
- Optimizing for embedded devices
- Debugging memory issues

#### **Legacy Refactor Specialist** ([legacy-refactor-specialist.agent.md](agents/legacy-refactor-specialist.agent.md))

Specialist in safely refactoring legacy code while:

- Maintaining zero regressions
- Preserving API compatibility
- Reducing memory footprint
- Improving maintainability

**Use when:**

- Refactoring existing code
- Improving code quality
- Reducing technical debt
- Updating legacy implementations

### 3. Skills

#### **Memory Safety Analyzer** ([skills/memory-safety-analyzer/SKILL.md](skills/memory-safety-analyzer/SKILL.md))

Systematic analysis of C/C++ code for:

- Memory leaks
- Use-after-free errors
- Buffer overflows
- Double-free issues
- Unchecked allocations

**Usage:** `@memory-safety-analyzer analyze [file]`

#### **Thread Safety Analyzer** ([skills/thread-safety-analyzer/SKILL.md](skills/thread-safety-analyzer/SKILL.md))

Analysis of concurrent code for:

- Race conditions
- Deadlock potential
- Lock ordering violations
- Improper synchronization
- Atomic usage issues

**Usage:** `@thread-safety-analyzer check [file]`

#### **Platform Portability Checker** ([skills/platform-portability-checker/SKILL.md](skills/platform-portability-checker/SKILL.md))

Verification of platform independence:

- Integer type portability
- Endianness handling
- Structure packing
- Platform-specific syscalls

**Usage:** `@platform-portability-checker check [file]`

### 4. Language-Specific Instructions

#### **C Embedded** ([instructions/c-embedded.instructions.md](instructions/c-embedded.instructions.md))

Applies to: `**/*.c`, `**/*.h`

- Memory management best practices
- Thread safety and concurrency patterns
- Resource constraint optimization
- Platform independence patterns
- Error handling conventions

#### **C++ Testing** ([instructions/cpp-testing.instructions.md](instructions/cpp-testing.instructions.md))

Applies to: `source/test/**/*.cpp`, `source/test/**/*.h`

- Google Test/Mock patterns
- Testing C code from C++
- Memory leak testing
- RAII( Resource Acquisition Is Initialization ) wrappers for C resources

#### **Shell Scripts** ([instructions/shell-scripts.instructions.md](instructions/shell-scripts.instructions.md))
Applies to: `**/*.sh`

- POSIX compliance
- Resource-aware scripting
- Error handling
- Platform independence

#### **Build System** ([instructions/build-system.instructions.md](instructions/build-system.instructions.md))

Applies to: `**/Makefile.am`, `**/configure.ac`

- Autotools best practices
- Cross-compilation support
- Dependency management
- Testing integration

### 5. CI/CD Workflow

**[workflows/copilot-quality-checks.yml](workflows/copilot-quality-checks.yml)**

Automated quality checks:
- Build verification with strict warnings
- Unit test execution
- Static analysis (cppcheck)
- Memory leak detection (valgrind)
- Thread safety checks (helgrind)
- Shell script validation (shellcheck)

**[workflows/copilot-setup-steps.yml](workflows/copilot-setup-steps.yml)**

Automated quality checks:

- Build verification
- Unit test execution
- Static analysis (cppcheck)
- Memory leak detection (valgrind)
- Shell script validation (shellcheck)

## How to Use

### For New Development

1. **Start a new feature:**
   ```
   @workspace /new Create a new telemetry event handler for network statistics
   ```
   The system will automatically apply C embedded standards.

2. **Use specific agent:**
   ```
   @embedded-c-expert Implement a memory pool for frequent event allocations
   ```

3. **Run analysis:**
   ```
   @memory-safety-analyzer Review source/telemetry/events.c for memory issues
   ```

4. **Thread safety implementation:**
   ```
   @embedded-c-expert Add thread-safe event queue with minimal locking
   ```

### For Refactoring

1. **Invoke refactoring specialist:**
   ```
   @legacy-refactor-specialist Refactor source/protocol/http_client.c to reduce memory usage
   ```

2. **Check platform portability:**
   ```
   @platform-portability-checker Verify source/utils/endian.c is platform-independent
   ```

### For Code Review

1. **Request comprehensive review:**
   ```
   @workspace Review this PR for memory safety and platform independence
   ```

2. **Specific analysis:**
   ```
   @memory-safety-analyzer Check for leaks in the error paths
   ```

## Best Practices

### Memory Management

✅ **DO:**
- Check all `malloc()` return values
- Use single exit point with cleanup labels
- Free resources in reverse order of allocation
- Run valgrind on all tests
- Prefer stack allocation for small data

❌ **DON'T:**
- Assume `malloc()` always succeeds
- Create memory leaks in error paths
- Use unchecked `strcpy()` or `sprintf()`
- Ignore compiler warnings
- Mix different allocation strategies

### Platform Independence

✅ **DO:**
- Use `stdint.h` types (`uint32_t`, etc.)
- Handle endianness explicitly (`htonl`/`ntohl`)
- Use `stdbool.h` for booleans
- Test on multiple architectures
- Use autoconf for platform detection

❌ **DON'T:**
- Assume pointer sizes
- Use platform-specific headers without checks
- Hard-code byte order
- Use non-standard compiler extensions
- Assume structure packing

### Thread Safety

✅ **DO:**
- Create threads with explicit stack sizes (pthread_attr_setstacksize)
- Use atomic operations for simple counters
- Establish consistent lock ordering
- Use timeouts with pthread_mutex_timedlock
- Document thread safety of all functions
- Prefer lock-free patterns when possible
- Keep critical sections minimal

❌ **DON'T:**
- Create threads with default attributes (wastes memory)
- Use heavy locks (rwlocks) for simple operations
- Acquire locks in different orders (causes deadlocks)
- Hold locks during expensive operations
- Use reader-writer locks for counters
- Create threads dynamically for each task
- Ignore thread sanitizer warnings

### Testing

✅ **DO:**
- Write tests for all new code
- Test error paths
- Run tests under valgrind
- Use mocks for external dependencies
- Aim for >80% coverage

❌ **DON'T:**
- Skip testing "simple" functions
- Ignore memory leaks in tests
- Test only happy paths
- Commit code that fails tests
- Remove tests to make builds pass

## CI Integration

The workflow runs automatically on:
- Push to any branch
- Pull requests
- Manual trigger via workflow_dispatch

### Workflow Steps

1. **Build** - Compile with autotools
2. **Test** - Run unit tests with gtest
3. **Static Analysis** - cppcheck for code issues
4. **Memory Check** - valgrind for leaks
5. **Concurrency Check** - helgrind for race conditions and deadlocks
6. **Shell Check** - shellcheck for scripts

### Viewing Results

Check the Actions tab in GitHub for:
- Build logs
- Test results
- Static analysis warnings
- Memory leak reports
- Artifact downloads

## Metrics and Goals

### Code Quality Metrics

- **Memory Leaks:** Zero tolerance (valgrind must pass)
- **Race Conditions:** Zero data races (helgrind/thread sanitizer)
- **Deadlocks:** No deadlock potential detected
- **Test Coverage:** Minimum 80% for new code
- **Static Analysis:** Zero critical/high severity issues
- **Build Warnings:** Zero warnings with -Wall -Wextra
- **Platform Support:** Linux (x86_64, ARM, MIPS)

### Performance Targets
Thread Stack Size:** 64KB per thread (not default 8MB)
- **Binary Size:** Minimize code size
- **CPU Usage:** <5% average on target devices
- **Fragmentation:** <10% heap fragmentation after 24h
- **Lock Contention:** <1% time spent waiting on locks
- **CPU Usage:** <5% average on target devices
- **Fragmentation:** <10% heap fragmentation after 24h

## Troubleshooting

### Agent Not Responding

- Ensure agent filename ends with `.agent.md`
- Check YAML frontmatter is valid
- Verify agent is committed to repository

### Instructions Not Applied

- Check `applyTo` glob pattern matches your files
- Ensure YAML frontmatter is properly formatted
- Verify file is in workspace

### Workflow Failing

- Check dependencies are available
- Verify configure.ac syntax
- Review build logs in Actions tab
- Test locally with same commands

## Resources

### Internal

- [RDK Coding Standards](../CONTRIBUTING.md)
- [Telemetry API Documentation](../include/telemetry2_0.h)
- [Test Examples](../source/test/)

### External

- [Autotools Manual](https://www.gnu.org/software/automake/manual/)
- [Valgrind Documentation](https://valgrind.org/docs/manual/)
- [Google Test Guide](https://google.github.io/googletest/)
- [Working Effectively with Legacy Code](https://www.oreilly.com/library/view/working-effectively-with/0131177052/)

## Contributing

To improve the agentic system:

1. **Add new agents** - Create `.agent.md` in `agents/`
2. **Add new skills** - Create `SKILL.md` in `skills/<name>/`
3. **Extend instructions** - Update language files in `instructions/`
4. **Improve workflow** - Enhance `workflows/copilot-setup-steps.yml`

Follow the patterns in existing files and test thoroughly.

## Support

For questions or issues:
1. Check this README
2. Review existing agents/skills
3. Consult the awesome-copilot repository
4. Ask in team channels

---

**Last Updated:** February 27, 2026  
**Version:** 1.0.0  
**Maintainers:** RDK Telemetry Team
