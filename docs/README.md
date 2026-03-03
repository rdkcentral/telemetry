# Telemetry 2.0 Documentation

Welcome to the Telemetry 2.0 framework documentation.

## Quick Links

- [Architecture Overview](architecture/overview.md) - System design and components
- [API Reference](api/public-api.md) - Public API documentation
- [Developer Guide](integration/developer-guide.md) - Getting started for developers
- [Build Setup](integration/build-setup.md) - Build environment configuration
- [Testing Guide](integration/testing.md) - Test procedures and coverage

## What is Telemetry 2.0?

Telemetry 2.0 is a lightweight, efficient telemetry framework designed for embedded devices in the RDK (Reference Design Kit) ecosystem. It provides real-time monitoring, event collection, and reporting capabilities for resource-constrained devices such as set-top boxes, gateways, and IoT devices.

### Key Features

- **Lightweight**: Optimized for devices with <128MB RAM
- **Flexible**: Profile-based configuration via JSON or XConf
- **Efficient**: Connection pooling and batch reporting
- **Secure**: mTLS support for encrypted communication
- **Platform-Independent**: Supports multiple architectures and platforms

## Documentation Structure

```
docs/
├── architecture/          # System architecture and design
│   ├── overview.md       # High-level system architecture
│   ├── components.md     # Component relationships  
│   ├── threading-model.md # Threading and synchronization
│   └── data-flow.md      # Data flow and state machines
├── api/                  # API documentation
│   ├── public-api.md    # Public API reference
│   ├── internal-api.md  # Internal API reference
│   └── error-codes.md   # Error codes and meanings
├── integration/          # Integration and setup guides
│   ├── developer-guide.md # Getting started guide
│   ├── build-setup.md    # Build configuration
│   ├── platform-porting.md # Porting to new platforms
│   └── testing.md        # Testing procedures
└── troubleshooting/      # Debug and troubleshooting
    ├── memory-issues.md  # Memory debugging
    ├── threading-issues.md # Thread debugging
    └── common-errors.md  # Common error solutions
```

## Component Documentation

Detailed technical documentation for individual components is located in `source/docs/` following the source code structure:

- [Bulk Data](../source/docs/bulkdata/README.md) - Profile and marker management
- [Protocol](../source/docs/protocol/README.md) - HTTP communication layer
- [Scheduler](../source/docs/scheduler/README.md) - Report scheduling
- [XConf Client](../source/docs/xconf-client/README.md) - Configuration retrieval
- [DCA Utilities](../source/docs/dcautil/README.md) - Log marker collection

## Getting Started

### For Developers

1. **Read the [Architecture Overview](architecture/overview.md)** to understand the system design
2. **Follow the [Developer Guide](integration/developer-guide.md)** to set up your environment
3. **Review the [API Reference](api/public-api.md)** for available functions
4. **Check [Testing Guide](integration/testing.md)** for test procedures

### For Platform Vendors

1. **Review [Platform Porting Guide](integration/platform-porting.md)** for integration requirements
2. **Check [Build Setup](integration/build-setup.md)** for compilation options
3. **Review [Threading Model](architecture/threading-model.md)** for resource planning

### For Maintainers

1. **Review [Component Documentation](../source/docs/)** for implementation details
2. **Check [Troubleshooting Guides](troubleshooting/)** for common issues
3. **Refer to [Internal API](api/internal-api.md)** for module interfaces

## Project Resources

- **Repository**: https://github.com/rdkcentral/telemetry
- **Bug Reports**: GitHub Issues
- **Contributions**: See [CONTRIBUTING.md](../CONTRIBUTING.md)
- **License**: Apache 2.0 (see [LICENSE](../LICENSE))

## Documentation Conventions

### Code Formatting

- **Functions**: `function_name()`
- **Types**: `type_name_t`
- **Constants**: `CONSTANT_NAME`
- **Files**: [filename.c](../source/telemetry2_0.c)

### Platform Tags

- **[Linux]** - Linux-specific information
- **[RDKB]** - RDK-B specific information
- **[Embedded]** - General embedded platform info

### Importance Indicators

- ⚠️ **Warning** - Important information that could cause issues if ignored
- 💡 **Tip** - Helpful suggestion or best practice
- 📝 **Note** - Additional context or clarification
- ❌ **Deprecated** - Feature or API scheduled for removal

## Contributing to Documentation

Documentation improvements are welcome! When contributing:

1. Follow the [Documentation Style Guide](.github/skills/technical-documentation-writer/SKILL.md)
2. Test all code examples
3. Use Mermaid for diagrams
4. Add cross-references to related docs
5. Update the table of contents

## Need Help?

- **For usage questions**: Check [Troubleshooting](troubleshooting/)
- **For build issues**: See [Build Setup](integration/build-setup.md)
- **For bugs**: File a GitHub issue
- **For API questions**: See [API Reference](api/public-api.md)

---

**Last Updated**: March 2026  
**Version**: 2.0
