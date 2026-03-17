# Design: Marker Discovery Tool

## Architecture overview

```
┌──────────────────────────────────────────────────────────────────┐
│                      marker_scanner.py (CLI)                     │
│  --branch <name> --org rdkcentral [--output report.md]           │
└──────────────────────────┬───────────────────────────────────────┘
                           │
                           ▼
┌──────────────────────────────────────────────────────────────────┐
│                      github_client.py                            │
│                                                                  │
│  ┌────────────────────────────────────────────────────────────┐  │
│  │  Branch = "develop" or unset        Branch = other         │  │
│  │  ─────────────────────────          ───────────────        │  │
│  │  1. Search API: "t2_event"          1. List all repos      │  │
│  │     in org (default branch)            in org (paginated)  │  │
│  │  2. Collect matching repos          2. For each repo:      │  │
│  │  3. Shallow clone each:                shallow clone:      │  │
│  │     branch → develop → main            branch → develop    │  │
│  │     → skip                             → main → skip       │  │
│  └────────────────────────────────────────────────────────────┘  │
└──────────────────────────┬───────────────────────────────────────┘
                           │ list of cloned repo paths
                           ▼
              ┌────────────┼────────────┐
              ▼            ▼            ▼
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
│     - Marker inventory table (with ⚠️ on duplicates)             │
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
    source_type: str       # "source" | "script" | "patch"
```

## Module details

### marker_scanner.py — CLI + orchestration

**Arguments:**
| Arg | Required | Default | Description |
|-----|----------|---------|-------------|
| `--branch` | No | `develop` | Branch/tag to scan |
| `--org` | No | `rdkcentral` | GitHub org (repeatable) |
| `--output` | No | `stdout` | Output file path for markdown report |

**Flow:**
1. Parse arguments
2. Call `github_client` to enumerate and clone repos
3. For each cloned repo, run `code_parser`, `script_parser`, and `patch_parser`
4. Collect all `MarkerRecord` results
5. Pass to `report_generator`
6. Clean up cloned repos (temp directory)
7. Exit 0 on success, non-zero on failure

### github_client.py — GitHub API + cloning

**Authentication:** Read credentials from `~/.netrc` for `github.com`.

**Repo enumeration:**
- `GET /orgs/{org}/repos` — paginated (100 per page)
- Collect all repo names and clone URLs

**Branch checking:**
- `GET /repos/{org}/{repo}/branches/{branch}` — 200 = exists, 404 = not found
- Fallback chain: specified → `develop` → `main` → skip

**Search API (fast path):**
- `GET /search/code?q=t2_event+org:{org}` — search for C/C++ API calls
- `GET /search/code?q=t2ValNotify+org:{org}` — search for script API calls
- `GET /search/code?q=t2CountNotify+org:{org}` — search for script API calls
- Merge unique repo names from all search results
- Rate limit: 10 req/min for code search — implement delay/retry

**Cloning:**
- `git clone --depth 1 --branch {branch} {url} {temp_dir}`
- Use `~/.netrc` for auth (git respects this automatically)
- All clones into a temp directory, cleaned up on exit

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
2. Summary: total markers, components scanned, duplicate count
3. Marker inventory table (sorted, duplicates flagged)
4. Duplicate markers section (only if duplicates exist)

## Error handling

| Scenario | Behavior |
|----------|----------|
| Repo clone fails | Log warning, skip repo, continue |
| Branch not found, develop not found, main not found | Skip repo |
| File parse error (tree-sitter) | Log warning, skip file, continue |
| GitHub API rate limit | Wait and retry with backoff |
| No markers found in any repo | Generate report with zero counts |
| Network failure mid-run | Fail with non-zero exit, clean up temp dirs |

## Performance considerations

- Shallow clones (`--depth 1`) minimize data transfer
- Fast path (search API) avoids cloning repos without markers
- Repos can be cloned concurrently (thread pool, ~5 parallel clones)
- tree-sitter parsing is fast (native C library)
- Temp directory cleanup in `finally` block to avoid disk leak
