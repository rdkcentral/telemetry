---
name: l1-gap-analyzer
description: Find and rank L1 unit-test coverage gaps for one telemetry module by running its gtest binaries with coverage inside the CI test container. Use when asked to improve L1 coverage, find untested functions, pick the next function to test, or measure coverage before and after adding tests.
---

# L1 Gap Analyzer

Measures L1 coverage for one module and ranks the functions that still have uncovered lines or branches. Runs on a developer host (macOS or Linux) through Docker, using the same image and coverage flags as `.github/workflows/L1-tests.yml`.

## Run

From the repository root:

```bash
test/l1_gaps.sh --list                     # known modules (test/l1_modules.txt)
test/l1_gaps.sh scheduler --top 10         # build + run + coverage + ranked gaps
```

- Docker must be running. The workspace is copied into a throwaway container; the host tree is not modified.
- A run builds the module from scratch, so expect minutes, not seconds. Run it once per iteration, not per edit.
- Output files in `/tmp/l1` (override with `L1_OUT`): `<module>.info` (lcov data), `<module>.gaps.tsv` (full ranked list), `build.log`, `lcov.log`, one log per test binary.
- Exit codes: 2 usage, 3 missing tool or lcov failure, 4 a test binary failed, 5 build failed. Fix the cause first; never analyze gaps from a failing suite.

## Read the output

```
== Coverage summary ==      lcov --list: lines, functions, branches for the module's files
== Top N gaps ==
PARTIAL   source/scheduler/scheduler.c:574   registerProfileWithScheduler   uncovered_lines=26 uncovered_branches=8
UNCALLED  <file>:<line>                      <function>                     (function never executed)
```

- `UNCALLED`: no test reaches the function. Highest value if it is production logic.
- `PARTIAL`: called, but some lines or branches are untested. These are usually error paths.
- Exact uncovered lines for a file:

```bash
awk '/^SF:/{f=$0} /^DA:[0-9]+,0$/{split($0,a,/[:,]/); print f":"a[2]}' /tmp/l1/<module>.info
```

## Choose the next target

1. Take the top entries and read each function plus its callers in the source.
2. Classify every candidate:

| Class | Meaning | Action |
|---|---|---|
| A | Reachable with the existing fixture and mocks | Write tests (skill `l1-test-writer`) |
| B | Needs a mock from `source/test/mocks/` that the module test does not use yet | Add it to the module build, then write tests |
| C | Needs a production seam (hard-wired global, direct syscall, static state) | Stop and report the seam; do not edit production code unless authorized |
| D | Not unit-testable (infinite thread loop, `main`, signal handler) or already covered by L2 | Record and skip |

3. Prefer, in order: `UNCALLED` production logic, then `PARTIAL` functions whose uncovered lines are error returns (`T2ERROR_*`, NULL checks, allocation or I/O failure), then everything else. Do not choose a function only because it has the most uncovered lines.
4. State the chosen function, its class, the uncovered lines you intend to reach, and why, before writing tests.

## After tests are added

Re-run `test/l1_gaps.sh <module>` and compare with the earlier output: the target function must drop out of the list or lose uncovered lines, file coverage must not decrease, and all binaries must pass. Report the before and after numbers from the tool output. Do not estimate coverage.

## Limits

- Per-module scope only; the module map lives in `test/l1_modules.txt` and must match `source/test/Makefile.am` and `test/run_ut.sh`.
- lcov 1.14 is what the container ships. If it is upgraded and lcov errors appear, set `LCOV_EXTRA="--ignore-errors mismatch"`.
