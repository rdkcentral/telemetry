## How to invoke

In VS Code Copilot Chat, type `/` and pick the prompt file, then paste the input block:

```
/l1-agent-bootstrap
```

Everything after it is your input block. Always supply the block — the agent treats unsupplied values as defaults, and unsupplied *commands* as "derive" (greenfield only).

---

## Input block template

```text
SOURCE_FOLDER=
UNIT_TEST_FOLDER=
BUILD_COMMAND=
TEST_COMMAND=
COVERAGE_COMMAND=
LINE_COVERAGE_THRESHOLD=90
FUNCTION_COVERAGE_THRESHOLD=95
ALLOW_BOOTSTRAP=true
```

---

## Greenfield — this repo is a real example

lostandfound-cpc has no `test/` folder and no GTest wiring, so it will classify as GREENFIELD.

```text
/l1-agent-bootstrap

SOURCE_FOLDER=plugin/refactorImpl/src
UNIT_TEST_FOLDER=test/l1
BUILD_COMMAND=
TEST_COMMAND=
COVERAGE_COMMAND=
LINE_COVERAGE_THRESHOLD=90
FUNCTION_COVERAGE_THRESHOLD=95
ALLOW_BOOTSTRAP=true

Run ITERATION 0 bootstrap only. Stop after the bootstrap deliverable report.
Build inside the existing Thunder dev container.

BUILD_COMMAND=./test/l1/build_l1.sh
TEST_COMMAND=bash test/l1/run_l1.sh --gtest_filter=LnfUtils*
COVERAGE_COMMAND=./test/l1/coverage.sh --clean --src plugin/refactorImpl/src
```

Leaving the three commands blank is deliberate — the agent derives them and reports them back.

**What you should see, in order:**
1. Environment report (compiler, cmake, GTest availability, lcov).
2. `Mode: GREENFIELD` + evidence.
3. Testability assessment of `src` — likely flagging `LnfHttpTransport`, `LnfBackendClient`, and the Thunder/`WPEFramework` dependency as the things needing stubs, and `LnfUtils` as the low-link-complexity first target.
4. Files created: `test/l1/CMakeLists.txt`, `test/l1/LnfUtilsTest.cpp`, maybe `test/l1/mocks/`.
5. One guarded `if(BUILD_UNIT_TESTS)` line in the top-level `CMakeLists.txt`.
6. Build → pass → fail-check → coverage → production-build-unchanged proof.
7. Derived commands echoed back.

**Then, in a follow-up message:**

```text
Bootstrap approved. Continue with iterations 1-3 using the derived commands.
```

---

## Brownfield — after the harness exists

Now the commands are known, so supply them and they become authoritative:

```text
/l1-agent-bootstrap

SOURCE_FOLDER=plugin/refactorImpl/src
UNIT_TEST_FOLDER=test/l1
BUILD_COMMAND=cmake -S . -B build -DBUILD_UNIT_TESTS=ON -DENABLE_COVERAGE=ON && cmake --build build -j
TEST_COMMAND=ctest --test-dir build --output-on-failure
COVERAGE_COMMAND=lcov --capture --directory build --output-file build/cov.info && lcov --extract build/cov.info '*/plugin/refactorImpl/src/*' --output-file build/cov-src.info && lcov --list build/cov-src.info
LINE_COVERAGE_THRESHOLD=90
FUNCTION_COVERAGE_THRESHOLD=95

Run 2 iterations. Focus on error paths in LnfHttpTransport.
```

Confirm the first line of its response says `Mode: BROWNFIELD` and `Bootstrap required: no`. If it says GREENFIELD, your `UNIT_TEST_FOLDER` path is wrong — stop and fix it before letting it write anything.

---

## Step-by-step operating procedure

**Greenfield**

| # | You do | Agent does |
|---|--------|-----------|
| 1 | Attach/open the dev container | — |
| 2 | Send input block, ask for Iteration 0 only | Env + mode + testability report |
| 3 | **Review the file list before approving** | Waits |
| 4 | "Bootstrap approved, continue iterations 1-3" | Adds real tests |
| 5 | `git diff` — verify nothing under `SOURCE_FOLDER` changed | — |
| 6 | Ask for the final L1 report | Summary + follow-ups |

**Brownfield**

| # | You do | Agent does |
|---|--------|-----------|
| 1 | Send input block with all 3 commands | Validates, confirms BROWNFIELD |
| 2 | — | Runs baseline build/test/coverage |
| 3 | — | Picks cluster, adds ≤3 tests, re-measures |
| 4 | "Next iteration" per cycle | Repeats |
| 5 | `git status` — only `UNIT_TEST_FOLDER` should be dirty | — |

---

## Useful follow-up prompts

- `Show the mode detection evidence again — I want to confirm it found the existing tests.`
- `Iteration 2: target the null/error branches in LnfDataAccessor only.`
- `That test has no meaningful assertion. Rewrite it to assert the actual return value and side effect.`
- `Classify the remaining gaps as A/B/C/D and stop — do not add more tests.`
- `You want a production seam. Explain the exact diff, the risk, and the coverage gain before I approve.`
- `Confirm the production build still succeeds with -DBUILD_UNIT_TESTS=OFF.`

---

## Guardrails worth watching

- **Wrong-mode detection is the main failure risk.** Always read the `Mode:` line first.
- Set `ALLOW_BOOTSTRAP=false` when you want a hard guarantee no infrastructure is created.
- If you supply a `BUILD_COMMAND` that is wrong, the agent will **stop and report** rather than guess — that's intended, just fix the input.
- After any run, `git diff --stat` is the fastest check that the test-only boundary held.
