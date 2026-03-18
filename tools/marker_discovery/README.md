# Marker Discovery Tool

A command-line utility that scans repositories in GitHub organizations (e.g. `rdkcentral`) to identify all T2 telemetry marker instrumentations and generates a markdown inventory report.

## Features

- **C/C++ scanning** — Uses [tree-sitter](https://tree-sitter.github.io/) to parse `.c`, `.cpp`, and `.h` files, extracting markers from `t2_event_s()`, `t2_event_d()`, and `t2_event_f()` calls
- **Wrapper resolution** — Detects functions that wrap `t2_event_*` calls and traces call sites to recover the actual marker names
- **Script scanning** — Regex-based extraction of `t2ValNotify` and `t2CountNotify` calls from shell scripts and other text files
- **Dynamic marker tracking** — Markers containing shell variables (`$var`, `${var}`) are identified and listed separately; pure positional args like `$1` are resolved when possible
- **Patch scanning** — Detects marker calls in `.patch` files (added lines only)
- **Duplicate detection** — Flags markers that appear across multiple components
- **Branch fallback** — Tries the requested branch, falls back to `develop`, then `main`, skips if none exist

## Prerequisites

- Python 3.10+
- Git (for cloning repositories)
- GitHub credentials in `~/.netrc`:
  ```
  machine github.com login <username> password <token>
  ```

## Installation

```bash
pip install -r tools/marker_discovery/requirements.txt
```

Dependencies:
- `tree-sitter>=0.21.0`
- `tree-sitter-c>=0.21.0`
- `requests>=2.31.0`

## Usage

```bash
python3 -m tools.marker_discovery [OPTIONS]
```

### Options

| Option | Default | Description |
|--------|---------|-------------|
| `--branch` | `develop` | Git branch or tag to scan |
| `--org` | `rdkcentral` | GitHub organization to scan (repeatable) |
| `--output` | stdout | Output file path for the markdown report |
| `--verbose`, `-v` | off | Enable debug logging |

### Examples

```bash
# Scan rdkcentral on develop branch, print report to stdout
python3 -m tools.marker_discovery

# Scan multiple orgs on a release branch, save to file
python3 -m tools.marker_discovery \
  --org rdkcentral \
  --org rdk-e \
  --branch release-1.0 \
  --output markers_report.md \
  --verbose
```

## How It Works

### Scanning Strategy

The tool uses two strategies depending on the target branch:

- **Fast path** (default `develop` branch) — Uses the GitHub Search API to find only repos containing marker patterns, then clones just those repos
- **Full path** (other branches) — Lists all repos in the organization and clones each one, since the Search API only indexes the default branch

All clones are shallow (`--depth 1`) to minimize disk and network usage.

### Three-Pass C/C++ Scanning

1. **Direct calls** — Finds `t2_event_s("MARKER", ...)`, `t2_event_d(...)`, `t2_event_f(...)` where the first argument is a string literal
2. **Wrapper detection** — Identifies functions that call `t2_event_*` with a variable first argument and maps it to a function parameter
3. **Wrapper resolution** — Finds call sites of detected wrappers and extracts the string literal passed at the marker parameter position

Example:
```c
// Wrapper definition — detected in Pass 2
void log_telemetry(const char *marker, const char *value) {
    t2_event_s(marker, value);
}

// Call site — resolved in Pass 3, marker recorded as "BOOT_TIME"
log_telemetry("BOOT_TIME", boot_val);
```

In the report, resolved wrappers show the call chain: `log_telemetry→t2_event_s`.

### Script Scanning

Scans all non-C/non-binary files for:
- `t2ValNotify "MARKER_NAME" ...`
- `t2CountNotify "MARKER_NAME" ...`

Markers containing shell variables (e.g. `SYST_ERR_$source_reboot`, `${version}`) are classified as **dynamic** and listed in a separate report section.

For pure positional args like `$1`, the tool attempts resolution by finding the enclosing shell function and tracing its call sites within the same file.

### Patch Scanning

Scans `.patch` files for added lines (`+` prefix) containing any of the five marker API patterns. These appear in the report with a `(patch)` component suffix.

## Report Format

The generated markdown report includes:

1. **Header** — Branch, organizations, generation timestamp
2. **Summary** — Total markers, static/dynamic counts, components scanned, duplicate count
3. **Marker Inventory** — Table of all static markers sorted by name, with component, file path, line number, and API
4. **Dynamic Markers** — Separate table for markers containing shell variables (if any)
5. **Duplicate Markers** — Markers appearing in multiple components, flagged with ⚠️ (if any)

## Authentication

The tool reads credentials from `~/.netrc` at request time for both the GitHub API and `git clone`. No credentials are extracted or stored in memory — they are injected directly into HTTP headers per-request. The `.netrc` file is used as-is regardless of file permissions.

## Directory Structure

```
tools/marker_discovery/
├── __init__.py          # MarkerRecord dataclass
├── __main__.py          # Entry point
├── marker_scanner.py    # CLI arguments and orchestration
├── github_client.py     # GitHub API client and repo cloning
├── code_parser.py       # tree-sitter C/C++ scanner
├── script_parser.py     # Shell script scanner
├── patch_parser.py      # Patch file scanner
├── report_generator.py  # Markdown report output
├── requirements.txt     # Python dependencies
└── tests/               # Unit tests (pytest)
```

## Running Tests

```bash
python3 -m pytest tools/marker_discovery/tests/ -v
```
