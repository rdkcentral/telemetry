---
name: l1-test-verifier
description: Verify an L1 test change for one telemetry module against git HEAD - registration, build, all tests pass, no tests removed, per-file coverage not lower, no new valgrind leaks, no new cppcheck findings - and confirm new tests can fail. Use after adding or changing L1 tests, before reporting a task done, or when asked to check coverage delta, leaks, or regressions.
---

# L1 Test Verifier

Runs the quality gates from `AGENTS.md` section 4 for one module by comparing the working tree with the last commit. Runs through Docker on a developer host, like `test/l1_gaps.sh`.

## Run

```bash
test/l1_check.sh <module>                 # full check, exit 0 only if every gate passes
test/l1_check.sh <module> --no-valgrind   # faster iteration; run the full check before reporting
test/l1_check.sh <module> --rebaseline    # remeasure HEAD (cached per commit otherwise)
```

- The baseline is `git HEAD`. Commit nothing yourself; uncommitted test changes are what gets checked.
- The first run per commit builds and measures twice (HEAD and working tree). Later runs reuse the HEAD result. A first full run of `scheduler` with valgrind took about 12 minutes under amd64 emulation on Apple Silicon.
- Valgrind under amd64 emulation on Apple Silicon is slow. Use `--no-valgrind` while iterating, then one full run.
- Results: `$L1_OUT/check/<module>/report.txt` (default `/tmp/l1`), plus `current/` and `base-<sha>/` with logs, coverage data, `vg.<binary>.log` and `cppcheck.txt`.
- Exit codes: 0 pass, 2 usage, 3 missing tool or baseline, 5 build failed, 6 gate failed.

## Gates

| Gate | Fails when |
|---|---|
| registration | a binary in the module `Makefile.am` is missing from `test/run_ut.sh` or `test/l1_modules.txt`, the test dir is missing from `SUBDIRS` in `source/test/Makefile.am`, or its `Makefile` is missing from `configure.ac` |
| tests | any binary fails, or a binary has fewer tests than at HEAD |
| coverage | line, branch or function coverage of any module source file is lower than at HEAD |
| valgrind | definitely + indirectly lost bytes of a binary grow |
| cppcheck | the count of any cppcheck id grows in module sources or the module test dir (skipped with a NOTE when cppcheck is not installed; the current CI image does not ship it) |

`NOTE` lines are informational: new files, baseline unavailable, valgrind error count changes (thread timing makes these vary).

## Handling failures

- Fix the cause in the test code. Never weaken, delete or disable an existing test, and never edit production code, to make a gate pass.
- Valgrind: compare `current/vg.<binary>.log` with `base-<sha>/vg.<binary>.log`; the new leak's stack points at the test that allocates without freeing.
- Pre-existing problems (leaks or findings that are present at HEAD) are not caused by the change and do not fail a gate. Mention them in the report; do not fix them unless asked.
- Registration failures: follow skill `l1-register-binary`.
- Coverage drop with only test changes usually means a test was removed or now exits early; check the test count line.

## Prove new tests can fail (mutation check)

Coverage only shows code ran. For each new test, once:

1. Invert or break its key assertion (for example change the expected `T2ERROR_*` value).
2. Run `test/l1_gaps.sh <module>`; it must exit 4 and the log in `/tmp/l1/<module>.<binary>.log` must show that test failing.
3. Restore the assertion and confirm `test/l1_gaps.sh <module>` passes again.

If a test still passes with a broken assertion, it does not test behavior; rewrite it.

## Report

Copy the gate lines and the coverage summary from the script output into your final message, state whether the mutation check was done for each new test, and list anything not run. Do not report coverage or pass/fail that the script did not print.
