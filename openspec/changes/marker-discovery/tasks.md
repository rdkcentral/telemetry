# Tasks: Marker Discovery Tool

## Setup

- [x] **Task 1: Project scaffolding**
  Create `tools/marker_discovery/` directory structure with empty module files (`__init__.py`, `marker_scanner.py`, `github_client.py`, `code_parser.py`, `script_parser.py`, `patch_parser.py`, `report_generator.py`) and `requirements.txt` with dependencies: `tree-sitter`, `tree-sitter-c`, `requests`.

## Core modules

- [x] **Task 2: GitHub client — repo enumeration**
  Implement `github_client.py` with:
  - Read credentials from `~/.netrc`
  - `list_org_repos(org)`: paginated `GET /orgs/{org}/repos`, return list of repo names + clone URLs
  - `search_code_in_org(org, query)`: paginated code search API, return set of repo names with matches
  - Rate limit handling with backoff for search API (10 req/min)

- [x] **Task 3: GitHub client — branch check and cloning**
  Add to `github_client.py`:
  - `check_branch_exists(org, repo, branch)`: check via API, return bool
  - `clone_repo(org, repo, branch, target_dir)`: shallow clone with fallback chain (specified → develop → main → skip)
  - `clone_matching_repos(org, repos, branch, temp_dir)`: orchestrate cloning for a list of repos, return list of cloned repo paths with metadata
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
  - Format: header with metadata → summary stats → marker inventory table (⚠️ on duplicates) → duplicate markers section (if any)
  - Write to file or stdout

## CLI + integration

- [x] **Task 9: CLI entry point and orchestration**
  Implement `marker_scanner.py`:
  - Argument parsing: `--branch` (default: `develop`), `--org` (default: `rdkcentral`, repeatable), `--output` (default: stdout)
  - Orchestration flow:
    - If branch is `develop` or unset → fast path (search API then clone matches)
    - If branch is other → full path (list all repos, clone all)
  - For each cloned repo: run `code_parser.scan_repo()`, `script_parser.scan_repo_scripts()`, and `patch_parser.scan_repo_patches()`
  - Collect all results, pass to `report_generator`
  - Cleanup temp directory
  - Exit 0 on success, non-zero on error

## Testing

- [x] **Task 10: Unit tests**
  Create `tools/marker_discovery/tests/`:
  - `test_code_parser.py`: test direct call extraction and wrapper resolution against sample C files
  - `test_script_parser.py`: test `t2ValNotify`/`t2CountNotify` extraction from sample scripts
  - `test_patch_parser.py`: test patch line filtering and marker extraction against sample .patch content (both C and script APIs)
  - `test_report_generator.py`: test markdown output format, duplicate detection, sorting
  - Sample fixtures: small `.c` files, scripts with known markers, `.patch` files with added lines

- [x] **Task 11: Integration test with real repo**
  Test end-to-end against a single known `rdkcentral` repo (e.g., one with known markers) to validate:
  - Clone works with `.netrc` auth
  - Branch fallback works
  - Markers are discovered correctly
  - Report output matches expected format
