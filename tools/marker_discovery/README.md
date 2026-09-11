# Marker Discovery Tool

A command-line utility that scans repositories in GitHub organizations to identify all T2 telemetry marker instrumentations and generates a markdown inventory report. By default scans the 4 RDK organizations (`rdkcentral`, `rdk-e`, `rdk-common`, `rdk-gdcs`).

## Features

- **C/C++ scanning** — Uses [tree-sitter](https://tree-sitter.github.io/) to parse `.c`, `.cpp`, and `.h` files, extracting markers from `t2_event_s()`, `t2_event_d()`, and `t2_event_f()` calls. Handles cast-wrapped arguments like `(char *) "MARKER"`. When a marker name cannot be resolved (variables, macros), the raw argument text is captured so no occurrence is missed.
- **Wrapper resolution** — Detects functions that wrap `t2_event_*` calls and traces call sites to recover the actual marker names. Wrapper-internal forwarding calls are excluded to avoid spurious entries.
- **Script scanning** — Regex-based extraction of `t2ValNotify` and `t2CountNotify` calls from shell scripts and other text files
- **Dynamic marker tracking** — Markers containing shell variables (`$var`, `${var}`) are identified and listed separately; pure positional args like `$1` are resolved when possible
- **Patch scanning** — Detects marker calls in `.patch` files (added lines only)
- **Configured component name detection** — Finds `t2_init()` calls in each component and extracts the configured component name argument, including cast-wrapped forms like `(char *) "name"`. Falls back to raw argument text if not a string literal.
- **Duplicate detection** — Flags markers that appear across multiple components
- **Version manifest input** — Accept a `versions.txt` file with exact commit SHAs for reproducible scans. Includes all GitHub repos regardless of org. Supports HTTPS and SSH GitHub URLs.
- **Single attempt cloning** — No branch fallback retries; 5-minute timeout per clone

## Prerequisites

- Python 3.10+
- Git (for cloning repositories)
- GitHub credentials:
  - **API calls**: `~/.netrc` with a GitHub PAT:
    ```
    machine api.github.com
      login <username>
      password <token>
    ```
  - **Git clones**: `~/.gitconfig` per-org credential helpers (supports multiple accounts):
    ```ini
    [credential "https://github.com"]
        helper = "!f() { echo username=DEFAULT_USER; echo password=DEFAULT_TOKEN; }; f"

    [credential "https://github.com/rdkcentral"]
        helper = "!f() { echo username=OTHER_USER; echo password=OTHER_TOKEN; }; f"
    ```
    Git uses the longest matching URL prefix, so the rdkcentral block overrides the default for that org. All other orgs use the default credential.

## Installation

Dependencies are **auto-installed** on first run if missing. To install manually:

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
| `--branch` | `main` | Git branch or tag to scan (ignored with `--input-file`) |
| `--org` | All 4 RDK orgs | GitHub organization(s) to scan. Example: `--org rdkcentral rdk-e` |
| `--repo` | None | One or more `org/repo` pairs to scan. Example: `--repo rdkcentral/telemetry rdk-e/rdkservices-cpc` |
| `--input-file` | None | Version manifest file (`versions.txt` format) |
| `--output` | stdout | Output file path for the markdown report |
| `--verbose`, `-v` | off | Enable debug logging |

### Examples

```bash
# Scan all 4 RDK orgs on main branch, print report to stdout
python3 -m tools.marker_discovery

# Scan specific orgs on a release branch, save to file
python3 -m tools.marker_discovery \
  --org rdkcentral rdk-e \
  --branch release-1.0 \
  --output markers_report.md \
  --verbose

# Scan specific repos (org/repo format)
python3 -m tools.marker_discovery \
  --repo rdkcentral/telemetry rdk-e/rdkservices-cpc \
  --output markers_report.md

# Scan from a version manifest file (exact commits)
python3 -m tools.marker_discovery \
  --input-file versions.txt \
  --output markers_report.md
```

## How It Works

### Scanning Modes

**Default mode** (no `--input-file`):
- Lists all repos in each org via GitHub API
- Shallow clones each repo on the target branch (single attempt, 5-minute timeout)
- Scans all cloned repos

**Repo mode** (`--repo org/repo`):
- Clones one or more specific repos by `org/repo` pairs
- Uses the `--branch` flag (default: `main`), single attempt

**Input-file mode** (`--input-file versions.txt`):
- Parses a version manifest listing GitHub repos with commit SHAs
- Clones at exact commit SHA (single attempt, no fallback to branch)
- Includes ALL GitHub repos in the file regardless of org; skips non-GitHub URLs

### Version Manifest Format

The `--input-file` accepts a `versions.txt` with one repo per line:

```
https://github.com/rdkcentral/telemetry@develop : a1b2c3d4e5f6...
https://github.com/rdkcentral/rbus.git@develop : 4a25e92112e8...
b'https://github.com/rdkcentral/meta-rdk-iot'@sha : sha
ssh://github.com/rdk-e/rdkservices-cpc@ : 1dff01bd7714...
b'ssh://git@github.com/rdk-e/meta-rdk-tools'@sha : sha
```

Supported URL schemes: `https://`, `http://`, `ssh://`, `ssh://git@`. SSH URLs are automatically converted to HTTPS for cloning. Non-GitHub URLs (gerrit, kernel.org, tarballs) and `md5sum` entries are skipped.

### Three-Pass C/C++ Scanning

1. **Direct calls** — Finds `t2_event_s("MARKER", ...)`, `t2_event_d(...)`, `t2_event_f(...)`. Extracts the first argument as the marker name. Handles cast-wrapped args like `(const char *) "MARKER"`. If the argument is a variable or macro, the raw text is captured (e.g. `MY_MACRO`, `some_var`).
2. **Wrapper detection** — Identifies functions that call `t2_event_*` with a variable first argument and maps it to a function parameter. Records the line of the internal `t2_event_*` call.
3. **Wrapper resolution** — Finds call sites of detected wrappers and extracts the string literal (or raw text) at the marker parameter position. Wrapper-internal `t2_event_*` calls (from Pass 2) are excluded from Pass 1 results to avoid spurious entries.

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
2. **Summary** — Total markers, static/dynamic counts, components scanned, unresolved count, duplicate count
3. **Configured Component Names** — Table mapping each component to its `t2_init()` configured name (NA if absent, all occurrences if multiple)
4. **Unique Marker Inventory** — One row per unique marker name with the list of components it appears in
5. **Detailed Marker Inventory** — Table of all static markers sorted by name, with component, file path, line number, and API
6. **Dynamic Markers** — Separate table for markers containing shell variables (if any)
7. **Duplicate Markers** — Markers appearing in multiple components, flagged with ⚠️ (if any)
8. **Unresolved Components** — Components from the input file that could not be cloned (if any)

## Authentication

**GitHub API calls** use `~/.netrc` via a custom `_NetrcAuth` class. Credentials are read per-request and never stored in memory. The `.netrc` file is used as-is regardless of file permissions. Subdomain matching (`api.github.com` → `github.com`) is handled automatically.

**Git clone operations** use `~/.gitconfig` credential helpers. Per-org helpers allow using different GitHub accounts for different organizations (e.g., one PAT for rdkcentral, another for rdk-e). Git uses longest-prefix URL matching, so a default `[credential "https://github.com"]` block covers all orgs, while specific blocks like `[credential "https://github.com/rdkcentral"]` override for that org.

## Directory Structure

```
tools/marker_discovery/
├── __init__.py              # MarkerRecord dataclass
├── __main__.py              # Entry point
├── marker_scanner.py        # CLI arguments and orchestration
├── github_client.py         # GitHub API client and repo cloning
├── component_file_parser.py # Version manifest (versions.txt) parser
├── code_parser.py           # tree-sitter C/C++ scanner
├── script_parser.py         # Shell script scanner
├── patch_parser.py          # Patch file scanner
├── report_generator.py      # Markdown report output
├── requirements.txt         # Python dependencies
└── tests/                   # Unit tests (pytest)
```

## Running Tests

```bash
python3 -m pytest tools/marker_discovery/tests/ -v
```
