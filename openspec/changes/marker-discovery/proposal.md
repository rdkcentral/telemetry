# Proposal: Marker Discovery Tool

## What

A Python CLI tool that scans all repositories in the `rdkcentral` (and later `rdk-e`) GitHub organizations to discover T2 telemetry marker instrumentation calls (`t2_event_s`, `t2_event_d`, `t2_event_f`), and generates a comprehensive markdown report of all markers found.

## Why

- Developers and system administrators have no centralized way to know which T2 telemetry markers are instrumented across the RDK ecosystem
- Manual searching across dozens of repositories is error-prone and time-consuming
- Marker naming conflicts across components go undetected
- Support engineers analyzing telemetry data lack documentation of what markers exist and where they originate

## Scope

### In scope (v1)

- **Repo enumeration**: Dynamically list all repositories in the `rdkcentral` GitHub organization via the GitHub API
- **Branch strategy**: Accept a release branch name as input
  - If branch is `develop` or not specified → use GitHub Search API to find repos with markers, then clone only those (fast path)
  - If branch is anything else → clone all repos and scan locally with grep/tree-sitter (full path)
- **Branch fallback**: For each repo: specified branch → `develop` → `main` → skip
- **Source file scanning**: Parse `.c`, `.cpp`, `.h` files using tree-sitter C/C++ AST to extract `t2_event_s`, `t2_event_d`, `t2_event_f` calls
- **Wrapper resolution (level 1)**: Detect functions that wrap `t2_event_*` calls with a variable argument, then resolve call sites to extract the actual marker name string literals
- **Script file scanning**: Scan all script files (any type) for `t2ValNotify` and `t2CountNotify` calls. These APIs are NOT searched in C/C++ files (they appear there as wrapper definitions, handled by wrapper resolution). Report API as `t2ValNotify` or `t2CountNotify` directly.
- **Patch file scanning**: Scan `.patch` files for added lines (`+` prefix) containing `t2_event_*`, `t2ValNotify`, and `t2CountNotify` calls
- **Duplicate detection**: Identify markers with the same name appearing in different components
- **Markdown report**: Generate a structured report with summary stats, full marker inventory table, and duplicate marker section
- **Authentication**: Use `~/.netrc` credentials for GitHub access

### Out of scope (v1)

- `rdk-e` organization scanning (deferred — auth not yet available)
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
**Organizations**: rdkcentral
**Generated**: 2026-03-13 10:30:00 UTC

## Summary
- **Total Markers**: 1,234
- **Components Scanned**: 67
- **Duplicate Markers**: 12 ⚠️

## Marker Inventory
| Marker Name | Component | File Path | Line | API |
|-------------|-----------|-----------|------|-----|
| BOOT_TIME | dcm-agent | src/dcm.c | 234 | t2_event_d |
| CPU_USAGE | telemetry | src/metrics.c | 89 | t2_event_f |
| CPU_USAGE ⚠️ | common_utilities | utils/system.c | 156 | t2_event_d |

## Duplicate Markers
⚠️ **CPU_USAGE** - Found in 2 components:
- telemetry: src/metrics.c:89 (`t2_event_f`)
- common_utilities: utils/system.c:156 (`t2_event_d`)
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
├── github_client.py       — GitHub API: org enumeration, branch check, clone
├── code_parser.py         — tree-sitter AST parsing for .c/.cpp/.h files
├── script_parser.py       — Script file scanning for t2ValNotify/t2CountNotify
├── patch_parser.py        — .patch file scanning (added lines only)
├── report_generator.py    — Markdown report generation
└── requirements.txt       — Python dependencies
```
