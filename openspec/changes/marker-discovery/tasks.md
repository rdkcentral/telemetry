# Tasks: Marker Discovery Tool

## Setup

- [x] **Task 1: Project scaffolding**
  Create `tools/marker_discovery/` directory structure with empty module files (`__init__.py`, `marker_scanner.py`, `github_client.py`, `code_parser.py`, `script_parser.py`, `patch_parser.py`, `report_generator.py`) and `requirements.txt` with dependencies: `tree-sitter`, `tree-sitter-c`, `requests`.

## Core modules

- [x] **Task 2: GitHub client — repo enumeration**
  Implement `github_client.py` with:
  - `_NetrcAuth` class that reads `~/.netrc` per-request without storing credentials
  - `list_org_repos(org)`: paginated `GET /orgs/{org}/repos`, return list of repo names + clone URLs
  - `search_code_in_org(org, query)`: paginated code search API, return set of repo names with matches
  - `find_repo_org(name, orgs)`: discover which org a repo belongs to by checking all orgs
  - `DEFAULT_ORGS`: `["rdkcentral", "rdk-e", "rdk-common", "rdk-gdcs"]`
  - Rate limit handling with backoff for search API (10 req/min)

- [x] **Task 3: GitHub client — branch check and cloning**
  Add to `github_client.py`:
  - `clone_repo(org, repo, branch, target_dir)`: shallow clone on specified branch, single attempt, 5-minute timeout, no fallback retries
  - `clone_matching_repos(org, repos, branch, temp_dir)`: orchestrate cloning for a list of repos
  - `clone_components_from_file(components, temp_dir)`: clone from parsed version manifest at exact commit SHAs. Single attempt via `_clone_at_commit` (git init + fetch --depth 1 + checkout FETCH_HEAD). No fallback to branch. Returns (cloned, unresolved).
  - `_clone_at_commit(url, path, sha)`: shallow clone at exact commit SHA, 5-minute timeout
  - `_force_rmtree(path)`: robust directory cleanup handling git-lfs stubborn files
  - Temp directory management (create on start, cleanup on exit)

- [x] **Task 4: Code parser — direct call extraction**
  Implement `code_parser.py`:
  - `scan_repo(repo_path, repo_name)`: find all `.c`, `.cpp`, `.h` files recursively
  - Use tree-sitter with C grammar to parse each file
  - Walk AST for `call_expression` nodes matching `t2_event_s`, `t2_event_d`, `t2_event_f`
  - Extract marker name from first argument (must be `string_literal`)
  - Return list of `MarkerRecord`

- [x] **Task 5: Code parser — wrapper detection and resolution**
  Add to `code_parser.py`:
  - Pass 2: walk AST for `function_definition` nodes containing `t2_event_*` calls where first arg is an `identifier` (not literal). Record wrapper name, parameter index, API variant.
  - Pass 3: for each detected wrapper, find all `call_expression` nodes calling that wrapper. Extract string literal at marker argument position.
  - Return additional `MarkerRecord` entries with API as `wrapperName→t2_event_*`

- [x] **Task 6: Script parser**
  Implement `script_parser.py`:
  - `scan_repo_scripts(repo_path, repo_name)`: find all files that are NOT `.c`, `.cpp`, `.h`, or `.patch`
  - Regex match for `t2ValNotify "MARKER"` and `t2CountNotify "MARKER"` patterns
  - Extract marker name from first quoted argument
  - API column = `t2ValNotify` or `t2CountNotify` (not mapped to underlying t2_event_*)
  - Return list of `MarkerRecord` with `source_type="script"`
  - Do NOT search for `t2ValNotify`/`t2CountNotify` in C/C++ files (wrapper resolver handles those)

- [x] **Task 7: Patch parser**
  Implement `patch_parser.py`:
  - `scan_repo_patches(repo_path, repo_name)`: find all `.patch` files recursively
  - Read each `.patch` file line by line
  - Filter to `+` lines only (exclude `+++` headers)
  - Regex match for `t2_event_s(`, `t2_event_d(`, `t2_event_f(` AND `t2ValNotify "`, `t2CountNotify "` patterns
  - Extract marker name from first argument (string literal)
  - Return list of `MarkerRecord` with `source_type="patch"` and component as `repo_name (patch)`

- [x] **Task 8: Report generator**
  Implement `report_generator.py`:
  - `generate_report(markers, branch, orgs, components_scanned)`: produce markdown string
  - Sort markers alphabetically by name, then component
  - Detect duplicates (same marker name in different components)
  - Format: header with metadata → summary stats → unique marker inventory (marker + components list) → detailed marker inventory table (⚠️ on duplicates) → dynamic markers section (if any) → duplicate markers section (if any)
  - Write to file or stdout

## CLI + integration

- [x] **Task 9: CLI entry point and orchestration**
  Implement `marker_scanner.py`:
  - Argument parsing: `--branch` (default: `main`), `--org` (default: all 4 RDK orgs, accepts multiple values), `--repo` (one or more `org/repo` pairs), `--input-file` (versions.txt), `--output` (default: stdout), `--verbose`/`-v`
  - Three modes:
    - Default mode (no input file, no repo): list all repos in all orgs, clone on target branch, scan
    - Repo mode (`--repo`): clone specific `org/repo` pairs on target branch
    - Input-file mode: parse version manifest via `component_file_parser`, clone at exact commits via `clone_components_from_file`, track unresolved
  - For each cloned repo: run `code_parser.scan_repo()`, `script_parser.scan_repo_scripts()`, and `patch_parser.scan_repo_patches()`
  - Pass all results + unresolved list to `report_generator`
  - Cleanup temp directory
  - Exit 0 on success, non-zero on error

## Testing

- [x] **Task 10: Unit tests**
  Create `tools/marker_discovery/tests/`:
  - `test_code_parser.py`: test direct call extraction and wrapper resolution against sample C files
  - `test_script_parser.py`: test `t2ValNotify`/`t2CountNotify` extraction, dynamic marker detection, positional arg resolution
  - `test_patch_parser.py`: test patch line filtering and marker extraction against sample .patch content
  - `test_report_generator.py`: test markdown output format, duplicate detection, sorting, dynamic markers section, unresolved components section
  - `test_component_file_parser.py`: test versions.txt parsing — HTTPS URLs, SSH URLs (with and without `git@`), byte-string prefix (`b'...'`), commit/branch extraction, SSH-to-HTTPS URL conversion, gerrit/tarball skipping, empty file
  - Sample fixtures: small `.c` files, scripts with known markers, `.patch` files with added lines

- [x] **Task 11: Integration test with real repo**
  Test end-to-end against a single known `rdkcentral` repo (e.g., one with known markers) to validate:
  - Clone works with `.netrc` auth
  - Branch fallback works
  - Markers are discovered correctly
  - Report output matches expected format

- [x] **Task 12: Configured component name detection (t2_init)**
  Add `scan_t2_init(repo_path)` to `code_parser.py`:
  - Walk tree-sitter AST for `t2_init()` call expressions
  - Extract first string literal argument as configured component name
  - Return list of names (empty if none found, multiple if multiple calls)
  Update `marker_scanner.py` to collect t2_init results per component via `scan_cloned_repos()`.
  Update `report_generator.py` to render a "Configured Component Names" table after Summary (NA for components without t2_init, comma-separated for multiple). Only show components that have markers.
  Add 6 tests in `test_code_parser.py` (TestScanT2Init) and 5 tests in `test_report_generator.py`.

- [x] **Task 13: Cast-wrapped argument handling and raw text fallback**
  Update all extraction points in `code_parser.py`:
  - Add `_extract_string_from_arg()` helper that handles cast expressions like `(char *) "MARKER"` and `(const char *) "name"`
  - All three passes (direct calls, wrapper resolution, t2_init) use cast-aware extraction with raw text fallback
  - If the first argument is not a string literal or cast-wrapped literal, capture the raw argument text (e.g. `MY_MACRO`, `some_var`, `get_name()`)
  - No occurrence is missed — the developer can decode raw text by inspecting the file/line
  Update `detect_wrappers()` to record the `inner_line` of wrapper-internal `t2_event_*` calls.
  Update `scan_repo()` to exclude wrapper-internal calls from Pass 1 results using `(file, line)` matching, preventing spurious raw-variable-name entries.
  Add 7 new tests: cast-wrapped markers, macro markers, cast-wrapped wrapper call sites, variable wrapper call sites, wrapper-internal exclusion, cast-wrapped t2_init, header file t2_init. Total: 80 tests passing.
