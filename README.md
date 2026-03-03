# Telemetry 2.0

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](LICENSE)
[![C](https://img.shields.io/badge/Language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Platform](https://img.shields.io/badge/Platform-Embedded%20Linux-orange.svg)](https://www.yoctoproject.org/)

A lightweight, efficient telemetry framework for RDK (Reference Design Kit) embedded devices.

## Overview

Telemetry 2.0 provides real-time monitoring, event collection, and reporting capabilities optimized for resource-constrained embedded devices such as set-top boxes, gateways, and IoT devices.

### Key Features

- 🚀 **Lightweight**: Minimal memory footprint (<1MB typical)
- ⚡ **Efficient**: Connection pooling and batch reporting
- 🔒 **Secure**: mTLS support for encrypted communication
- 📊 **Flexible**: Profile-based configuration (JSON/XConf)
- 🔧 **Platform-Independent**: Multiple architecture support
- 🧵 **Thread-Safe**: Robust concurrent operation

### Architecture Highlights

```mermaid
graph TB
    A[Telemetry Events/Markers] --> B[Profile Matcher]
    B --> C[Report Generator]
    C --> D[HTTP Connection Pool]
    D --> E[XConf Server / Data Collector]
    F[XConf Client] -.->|Config| B
    G[Scheduler] -.->|Triggers| C
```

## Quick Start

### Prerequisites

- GCC 4.8+ or Clang 3.5+
- pthread library
- libcurl 7.65.0+
- cJSON library
- OpenSSL 1.1.1+ (for mTLS)

### Build

```bash
# Clone repository
git clone https://github.com/rdkcentral/telemetry.git
cd telemetry

# Configure
./configure --prefix=/usr --enable-rbus

# Build
make

# Run tests
make check

# Install
sudo make install
```

### Docker Development

```bash
# Build and start development container
cd containers
docker compose up -d

# Run tests inside container
docker compose exec telemetry bash
./configure && make check
```

See [Build Setup Guide](docs/integration/build-setup.md) for detailed build options.

### Basic Usage

```c
#include "telemetry2_0.h"

int main(void) {
    // Initialize telemetry
    if (t2_init() != 0) {
        fprintf(stderr, "Failed to initialize telemetry\n");
        return -1;
    }
    
    // Send a marker event
    t2_event_s("SYS_INFO_DeviceBootup", "Device started successfully");
    
    // Cleanup
    t2_uninit();
    return 0;
}
```

Compile: `gcc -o myapp myapp.c -ltelemetry`

## Documentation

📚 **[Complete Documentation](docs/README.md)**

### Key Documents

- **[Architecture Overview](docs/architecture/overview.md)** - System design and components
- **[API Reference](docs/api/public-api.md)** - Public API documentation
- **[Developer Guide](docs/integration/developer-guide.md)** - Getting started
- **[Build Setup](docs/integration/build-setup.md)** - Build configuration
- **[Testing Guide](docs/integration/testing.md)** - Test procedures

### Component Documentation

Individual component documentation is in [`source/docs/`](source/docs/):

- [Bulk Data System](source/docs/bulkdata/README.md) - Profile and marker management
- [HTTP Protocol](source/docs/protocol/README.md) - Communication layer
- [Scheduler](source/docs/scheduler/README.md) - Report scheduling
- [XConf Client](source/docs/xconf-client/README.md) - Configuration retrieval

## Project Structure

```
telemetry/
├── source/              # Source code
│   ├── bulkdata/       # Profile and marker management
│   ├── protocol/       # HTTP/RBUS communication
│   ├── scheduler/      # Report scheduling
│   ├── xconf-client/   # Configuration retrieval
│   ├── dcautil/        # Log marker utilities
│   └── test/           # Unit tests (gtest/gmock)
├── include/            # Public headers
├── config/             # Configuration files
├── docs/               # Documentation
├── containers/         # Docker development environment
└── test/               # Functional tests
```

## Configuration

### Profile Configuration

Telemetry uses JSON profiles to define what data to collect:

```json
{
  "Profile": "RDKB_BasicProfile",
  "Version": "1.0.0",
  "Protocol": "HTTP",
  "EncodingType": "JSON",
  "ReportingInterval": 300,
  "Parameters": [
    {
      "type": "dataModel",
      "name": "Device.DeviceInfo.Manufacturer"
    },
    {
      "type": "event",
      "eventName": "bootup_complete"
    }
  ]
}
```

See [Profile Configuration Guide](docs/integration/profile-configuration.md) for details.

### Environment Variables

| Variable | Description | Default |
|----------|-------------|---------|
| `T2_ENABLE_DEBUG` | Enable debug logging | `0` |
| `T2_PROFILE_PATH` | Profile directory | `/etc/telemetry` |
| `T2_XCONF_URL` | XConf server URL | - |
| `T2_REPORT_URL` | Report upload URL | - |

## Development

### Running Tests

```bash
# Unit tests
make check

# Functional tests
cd test
./run_ut.sh

# Code coverage
./cov_build.sh
```

### Code Quality

```bash
# Static analysis
cppcheck --enable=all source/

# Memory leak check
valgrind --leak-check=full ./test/telemetry_test

# Thread safety check
valgrind --tool=helgrind ./test/telemetry_test
```

### Development Container

Use the provided Docker container for consistent development:

```bash
cd containers
docker compose up -d
docker compose exec telemetry bash
```

See [Docker Development Guide](containers/README.md) for more details.

## Platform Support

Telemetry 2.0 is designed to be platform-independent and has been tested on:

- **RDK-B** (Broadband devices)
- **RDK-V** (Video devices)
- **Linux** (x86_64, ARM, ARM64)
- **Yocto Project** builds

See [Platform Porting Guide](docs/integration/platform-porting.md) for porting to new platforms.

## Performance

Typical resource usage on embedded devices:

| Metric | Value |
|--------|-------|
| Memory (RSS) | ~1.5 MB |
| Threads | 3-5 |
| CPU (idle) | <1% |
| CPU (active reporting) | ~5% |

See [Performance Tuning Guide](docs/troubleshooting/performance.md) for optimization.

## Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

### Development Workflow

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Add tests for new functionality
5. Ensure all tests pass (`make check`)
6. Commit your changes (`git commit -m 'Add amazing feature'`)
7. Push to the branch (`git push origin feature/amazing-feature`)
8. Open a Pull Request

### Code Style

- Follow existing C code style (K&R-like)
- Maximum line length: 120 characters
- Use descriptive variable names
- Document all public APIs
- Add unit tests for new functions

See [Coding Guidelines](.github/instructions/c-embedded.instructions.md) for details.

## Troubleshooting

### Common Issues

**Q: Telemetry not sending reports**
- Check network connectivity
- Verify XConf URL configuration
- Review logs in `/var/log/telemetry/`

**Q: High memory usage**
- Reduce number of active profiles
- Decrease reporting intervals
- Check for memory leaks with valgrind

**Q: Build errors**
- Ensure all dependencies installed
- Check compiler version (GCC 4.8+)
- Review build logs for missing libraries

See [Troubleshooting Guide](docs/troubleshooting/common-errors.md) for more solutions.

## License

This project is licensed under the Apache License 2.0 - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- RDK Management LLC
- RDK Community Contributors
- Open Source Community

## Contact

- **Repository**: https://github.com/rdkcentral/telemetry
- **Issues**: https://github.com/rdkcentral/telemetry/issues
- **RDK Central**: https://rdkcentral.com

## Changelog

See [CHANGELOG.md](CHANGELOG.md) for version history and release notes.

---

**Built for the RDK Community**
