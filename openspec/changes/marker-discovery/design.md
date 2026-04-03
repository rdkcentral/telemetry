# Design: Marker Discovery Tool

## Architecture overview

```
┌──────────────────────────────────────────────────────────────────┐
│                      marker_scanner.py (CLI)                     │
│  --org | --repo org/repo | --input-file  [--branch] [--output]   │
└──────────────────────────┬───────────────────────────────────────┘
                           │
          ┌────────────────┼────────────────┐
          ▼                ▼                ▼
┌──────────────────┐ ┌──────────────┐ ┌──────────────────────────┐
│  Default mode    │ │  Repo mode   │ │  Input-file mode         │
│  (--org)         │ │  (--repo)    │ │  (--input-file)          │
│                  │ │              │ │                          │
│  1. List repos   │ │ Parse        │ │ component_file_parser.py │
│     in org (API) │ │ org/repo     │ │ parse versions.txt       │
│  2. Shallow clone│ │ pairs        │ │ all GitHub orgs included │
│     on --branch  │ │ Clone each   │ │                          │
│     (single try, │ │ on --branch  │ │ github_client.py         │
│      5min timeout│ │ (single try) │ │ clone at exact commit    │
│      no retry)   │ │              │ │ (single try, no fallback)│
└────────┬─────────┘ └──────┬───────┘ └────────────┬─────────────┘
         │                  │                      │
         └──────────────────┼──────────────────────┘
                            │ list of cloned repo paths
                            ▼
              ┌─────────────┼────────────┐
              ▼             ▼            ▼
┌──────────────────┐ ┌──────────────┐ ┌──────────────────┐
│ code_parser.py   │ │script_parser │ │ patch_parser.py  │
│                  │ │          .py │ │                  │
│ .c / .cpp / .h   │ │ All scripts  │ │ .patch files     │
│                  │ │              │ │                  │
│ t2_event_s/d/f   │ │ t2ValNotify  │ │ All APIs (source │
│ + wrapper resolve│ │ t2CountNotify│ │ + script)        │
│                  │ │              │ │ added lines only │
│ NOT t2ValNotify/ │ │ NOT t2_event │ │                  │
│ t2CountNotify    │ │              │ │                  │
└────────┬─────────┘ └──────┬───────┘ └────────┬─────────┘
         │                  │                  │
         └──────────────────┼──────────────────┘
                      │  list of MarkerRecord
                      ▼
┌──────────────────────────────────────────────────────────────────┐
│                    report_generator.py                            │
│                                                                  │
│  1. Sort all markers by name                                     │
│  2. Detect duplicates (same marker name, different component)    │
│  3. Generate markdown:                                           │
│     - Summary stats                                              │
│     - Unique marker inventory (marker + components list)         │
│     - Detailed marker inventory table (with ⚠️ on duplicates)    │
│     - Duplicate markers section                                  │
│  4. Write to output file                                         │
└──────────────────────────────────────────────────────────────────┘
```

## Data model

All scanners produce the same record type:

```python
@dataclass
class MarkerRecord:
    marker_name: str       # "BOOT_TIME"
    component: str         # "dcm-agent" (repo name)
    file_path: str         # "src/dcm.c" (relative to repo root)
    line: int              # 234
    api: str               # "t2_event_d" or "t2CountNotify→t2_event_d" or "t2ValNotify"
    source_type: str       # "source" | "script" | "script_dynamic" | "patch"
```

## Module details

### marker_scanner.py — CLI + orchestration

**Arguments:**
| Arg | Required | Default | Description |
|-----|----------|---------|-------------|
| `--branch` | No | `main` | Branch/tag to scan (ignored with `--input-file`) |
| `--org` | No | All 4 RDK orgs | GitHub org(s) to scan. Example: `--org rdkcentral rdk-e` |
| `--repo` | No | None | One or more `org/repo` pairs. Example: `--repo rdkcentral/telemetry rdk-e/rdkservices-cpc` |
| `--input-file` | No | None | Version manifest file (versions.txt format) |
| `--output` | No | `stdout` | Output file path for markdown report |
| `--verbose` / `-v` | No | off | Enable debug logging |

**Flow (default mode — no input file):**
1. Parse arguments
2. For each org, call `github_client.list_org_repos()` and clone all on target branch (single attempt, 5-minute timeout)
3. For each cloned repo, run all three scanners
4. Pass results + empty unresolved list to `report_generator`

**Flow (repo mode — `--repo org/repo`):**
1. Parse `org/repo` pairs from arguments
2. Clone each repo on the target branch (single attempt)
3. For each cloned repo, run all three scanners
4. Pass results to `report_generator`

**Flow (input-file mode):**
1. Parse version manifest via `component_file_parser.parse_component_file()` — extracts org, repo, commit SHA, branch from each line. All GitHub repos included regardless of org.
2. For each component, clone at exact commit SHA via `github_client.clone_components_from_file()` (single attempt, no fallback)
3. Track unresolved components (clone failures)
4. Pass results + unresolved list to `report_generator`

### github_client.py — GitHub API + cloning

**Authentication:**
- API calls: Read credentials from `~/.netrc` for `api.github.com` per-request via `_NetrcAuth` class (never stored in memory)
- Git clones: Use `~/.gitconfig` per-org credential helpers for HTTPS clones (supports multiple GitHub accounts)

**Default organizations:** `rdkcentral`, `rdk-e`, `rdk-common`, `rdk-gdcs`

**Repo enumeration:**
- `GET /orgs/{org}/repos` — paginated (100 per page)
- Collect all repo names and clone URLs

**Search API (fast path):**
- `GET /search/code?q=t2_event+org:{org}` — search for C/C++ API calls
- `GET /search/code?q=t2ValNotify+org:{org}` — search for script API calls
- `GET /search/code?q=t2CountNotify+org:{org}` — search for script API calls
- Merge unique repo names from all search results
- Rate limit: 10 req/min for code search — implement delay/retry

**Branch fallback (default mode):**
- `clone_repo(org, repo, branch, target_dir)`: single attempt on specified branch, 5-minute timeout, no fallback retries

**Commit-SHA cloning (input-file mode):**
- `clone_components_from_file(components, temp_dir)`: for each component from the parser:
  1. `_clone_at_commit(url, path, sha)`: `git init` → `git fetch --depth 1 origin <sha>` → `git checkout FETCH_HEAD`
  2. No fallback — if commit clone fails, component is marked unresolved

**Robust cleanup:**
- `_force_rmtree()`: handles stubborn git-lfs directories via `onerror` handler with `chmod`
- All clones into a temp directory, cleaned up on exit

### component_file_parser.py — Version manifest parsing

Parses `versions.txt` files listing GitHub repos with branches and commit SHAs.

**Supported URL formats:**
```
https://github.com/rdkcentral/telemetry@develop : a1b2c3d4...
https://github.com/rdkcentral/rbus.git@develop : e5f6a7b8...
b'https://github.com/rdkcentral/meta-rdk-iot'@sha : sha
ssh://github.com/rdk-e/rdkservices-cpc@ : 1dff01bd...
ssh://github.com/rdk-e/airplay-application-cpc@ : 43c9d71147...
b'ssh://git@github.com/rdk-e/meta-rdk-tools'@sha : sha
```

**Filtering:**
- Only GitHub URLs (both HTTPS and SSH) are matched
- All GitHub repos are included regardless of org
- Non-GitHub URLs (gerrit, kernel.org, tarballs) are skipped
- Lines with `md5sum` checksums are skipped

**SSH → HTTPS conversion:** All SSH URLs are converted to `https://github.com/{org}/{repo}.git` for clone compatibility with `.gitconfig` credential helpers.

**Output:** List of `{name, org, commit, branch, url}`

### code_parser.py — tree-sitter AST scanning

**Dependencies:** `tree-sitter`, `tree-sitter-c` (C grammar covers most `.cpp` in RDK repos; add `tree-sitter-cpp` if needed)

**Pass 1 — Direct calls:**
- Walk AST for `call_expression` nodes where function name is `t2_event_s`, `t2_event_d`, or `t2_event_f`
- First argument is a `string_literal` → extract marker name
- Record: marker name, file, line, API variant

**Pass 2 — Wrapper detection:**
- Walk AST for `function_definition` nodes whose body contains a `call_expression` to `t2_event_*`
- If the first argument to `t2_event_*` is an `identifier` (not a string literal) → this function is a wrapper
- Record: wrapper function name, which parameter position carries the marker, which `t2_event_*` variant

**Pass 3 — Wrapper call site resolution:**
- For each detected wrapper, walk AST again to find all `call_expression` nodes calling that wrapper
- Extract the string literal at the marker argument position
- Record as: marker name, file, line, API = `wrapperName→t2_event_*`

**Limitation:** Only one level of wrapping. If a wrapper calls another wrapper, the inner wrapper is not resolved.

### script_parser.py — Script file scanning

Scans all non-C/C++ files for `t2ValNotify` and `t2CountNotify` calls.

**File selection:**
- Include any file that is NOT `.c`, `.cpp`, `.h`, or `.patch`
- This covers `.sh`, `.py`, `.lua`, `.pl`, `.rb`, and any other script type

**Pattern matching (regex):**
```
t2ValNotify\s+"([^"]+)"
t2CountNotify\s+"([^"]+)"
```

**Call format:**
```bash
t2ValNotify "MARKER_NAME" "value"
t2CountNotify "MARKER_NAME" 1
```

- First argument (quoted string) is the marker name
- API column reports `t2ValNotify` or `t2CountNotify` directly (not mapped to underlying `t2_event_*`)
- `source_type` = `"script"`

**Important:** `t2ValNotify` and `t2CountNotify` are NOT searched in `.c/.cpp/.h` files. In C/C++ these functions are wrapper definitions — the wrapper resolver in `code_parser.py` handles those naturally by tracing their call sites.

### patch_parser.py — .patch file scanning

- Recursively find all `.patch` files in each cloned repo
- For each `.patch` file:
  - Read line by line
  - Include only lines starting with `+` (exclude `+++` header lines)
  - Apply regex patterns to match:
    - C/C++ APIs: `t2_event_s(`, `t2_event_d(`, `t2_event_f(`
    - Script APIs: `t2ValNotify "`, `t2CountNotify "`
  - Extract marker name from first argument (string literal only)
  - Line number = line number within the `.patch` file
- Component name = repo name + `(patch)`

**Note:** Wrapper resolution within patches is limited — patches lack full function context. Direct calls only for v1.

### report_generator.py — Markdown output

**Input:** List of `MarkerRecord`, metadata (branch, org, timestamp)

**Processing:**
1. Sort records alphabetically by `marker_name`, then by `component`
2. Group by `marker_name` to detect duplicates (same name, different component)
3. Flag duplicate markers with ⚠️

**Output sections:**
1. Header with branch, org, generation timestamp
2. Summary: total markers (static/dynamic split), components scanned, unresolved count, duplicate count
3. Marker inventory table (static markers, sorted, duplicates flagged)
4. Dynamic markers table (markers with shell variables)
5. Duplicate markers section (if any)
6. Unresolved components table (if input-file mode, components not found)

## Error handling

| Scenario | Behavior |
|----------|----------|
| Repo clone fails | Log warning, skip repo, continue |
| Commit SHA not fetchable | Fall back to branch, then default branch |
| Component clone fails entirely | Add to unresolved list, continue |
| File parse error (tree-sitter) | Log warning, skip file, continue |
| GitHub API rate limit | Wait and retry with backoff |
| No markers found in any repo | Generate report with zero counts |
| Network failure mid-run | Fail with non-zero exit, clean up temp dirs |
| Git LFS / stubborn directories | `_force_rmtree` with `chmod` + retry |

## Performance considerations

- Shallow clones (`--depth 1`) minimize data transfer
- Fast path (search API) avoids cloning repos without markers
- Repos can be cloned concurrently (thread pool, ~5 parallel clones)
- tree-sitter parsing is fast (native C library)
- Temp directory cleanup in `finally` block to avoid disk leak
