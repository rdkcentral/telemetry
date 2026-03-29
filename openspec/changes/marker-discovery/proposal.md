# Proposal: Marker Discovery Tool

## What

A Python CLI tool that scans repositories in the `rdkcentral`, `rdk-e`, `rdk-common`, and `rdk-gdcs` GitHub organizations to discover T2 telemetry marker instrumentation calls (`t2_event_s`, `t2_event_d`, `t2_event_f`, `t2ValNotify`, `t2CountNotify`), and generates a comprehensive markdown report of all markers found.

Supports two modes:
1. **Default mode**: Scan the `main` branch of all repos in all 4 organizations
2. **Input-file mode**: Accept a `versions.txt` manifest file listing GitHub repos with exact commit SHAs, clone at those commits, and scan

## Why

- Developers and system administrators have no centralized way to know which T2 telemetry markers are instrumented across the RDK ecosystem
- Manual searching across dozens of repositories is error-prone and time-consuming
- Marker naming conflicts across components go undetected
- Support engineers analyzing telemetry data lack documentation of what markers exist and where they originate

## Scope

### In scope (v1)

- **Repo enumeration**: Dynamically list all repositories in 4 GitHub organizations (`rdkcentral`, `rdk-e`, `rdk-common`, `rdk-gdcs`) via the GitHub API
- **Version manifest input**: Accept a `versions.txt` file (`--input-file`) listing GitHub repos with branches and commit SHAs in the format `URL@ref : commit_sha`. Supports HTTPS and SSH URLs. SSH URLs are converted to HTTPS for cloning.
- **Branch strategy**:
  - Default (no input file): scan `main` branch of all repos in all 4 orgs, fallback: specified → main → develop → default branch
  - With input file: clone at exact commit SHA, fallback: commit → branch → default branch
- **Org filtering**: Only repos from the 4 default RDK orgs are included from version manifest files. Non-RDK orgs, non-GitHub URLs (gerrit, kernel.org, tarballs), and md5sum entries are skipped.
- **Unresolved component tracking**: Components that fail to clone are listed in a dedicated report section
- **Source file scanning**: Parse `.c`, `.cpp`, `.h` files using tree-sitter C/C++ AST to extract `t2_event_s`, `t2_event_d`, `t2_event_f` calls
- **Wrapper resolution (level 1)**: Detect functions that wrap `t2_event_*` calls with a variable argument, then resolve call sites to extract the actual marker name string literals
- **Script file scanning**: Scan all script files (any type) for `t2ValNotify` and `t2CountNotify` calls. These APIs are NOT searched in C/C++ files (they appear there as wrapper definitions, handled by wrapper resolution). Report API as `t2ValNotify` or `t2CountNotify` directly.
- **Dynamic marker classification**: Markers containing shell variables (`$var`, `${var}`) are classified as "script_dynamic" and reported in a separate section. Pure positional args (`$1`) are resolved when possible by tracing shell function call sites.
- **Patch file scanning**: Scan `.patch` files for added lines (`+` prefix) containing `t2_event_*`, `t2ValNotify`, and `t2CountNotify` calls
- **Duplicate detection**: Identify markers with the same name appearing in different components
- **Markdown report**: Generate a structured report with summary stats, full marker inventory table, dynamic markers section, duplicate marker section, and unresolved components section
- **Authentication**: Use `~/.netrc` credentials for GitHub API calls. Use `~/.gitconfig` per-org credential helpers for git clone operations (supports multiple GitHub accounts).

### Out of scope (v1)

- Macro-wrapped marker calls (e.g., `#define REPORT(m,v) t2_event_d(m,v)`)
- Wrappers of wrappers (more than one level of indirection)
- JSON/CSV/HTML output formats
- Cross-referencing against telemetry profile configurations
- Log file scanning
- CI/CD integration

### Non-goals

- This tool does not modify any source code
- This tool does not validate marker values or telemetry payloads
- This tool does not replace runtime telemetry monitoring

## Output format

```markdown
# Telemetry Marker Inventory
**Branch**: main
**Organizations**: rdkcentral, rdk-e, rdk-common, rdk-gdcs
**Generated**: 2026-03-13 10:30:00 UTC

## Summary
- **Total Markers**: 1,234
- **Static Markers**: 1,200
- **Dynamic Markers**: 34 (contain shell variables)
- **Components Scanned**: 258
- **Unresolved Components**: 5 ⚠️
- **Duplicate Markers**: 12 ⚠️

## Marker Inventory
| Marker Name | Component | File Path | Line | API |
|-------------|-----------|-----------|------|-----|
| BOOT_TIME | dcm-agent | src/dcm.c | 234 | t2_event_d |
| CPU_USAGE ⚠️ | telemetry | src/metrics.c | 89 | t2_event_f |

## Dynamic Markers
Markers containing shell variables (`$var`, `${var}`) that resolve at runtime.

| Marker Pattern | Component | File Path | Line | API |
|----------------|-----------|-----------|------|-----|
| SYST_ERR_$source_reboot | sysint | scripts/reboot.sh | 45 | t2ValNotify |

## Duplicate Markers
⚠️ **CPU_USAGE** - Found in 2 components:
- telemetry: src/metrics.c:89 (`t2_event_f`)
- common_utilities: utils/system.c:156 (`t2_event_d`)

## Unresolved Components
Components from the input file that could not be scanned.

| Component | Version | Reason |
|-----------|---------|--------|
| sky-jspp | 1.0.0-r4 | Not found in any organization |
```

## Key decisions

1. **tree-sitter over regex** for source parsing — enables reliable multi-line call extraction and natural wrapper function traversal via AST walking
2. **Dual scanning strategy** based on branch — search API for default branch (fast), full clone for release branches (complete)
3. **Shallow clones** (`--depth 1`) to minimize network and disk usage
4. **Patch files scanned in all cloned repos** — not limited to meta-* repos

## Location

```
tools/marker_discovery/
├── marker_scanner.py      — CLI entry point + orchestration
├── github_client.py       — GitHub API: org enumeration, org discovery, branch check, clone
├── component_file_parser.py — Parse component version input files
├── code_parser.py         — tree-sitter AST parsing for .c/.cpp/.h files
├── script_parser.py       — Script file scanning for t2ValNotify/t2CountNotify
├── patch_parser.py        — .patch file scanning (added lines only)
├── report_generator.py    — Markdown report generation
└── requirements.txt       — Python dependencies
```
