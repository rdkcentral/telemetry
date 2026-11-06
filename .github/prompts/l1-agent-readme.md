# L1 test agent

Autonomous L1 unit-test development for this repository. Rules live in `AGENTS.md`; procedures live in skills; measurement lives in scripts.

| Piece | Path | Role |
|---|---|---|
| Prompt | `.github/prompts/l1-coverage.prompt.md` | Entry point (`/l1-coverage`) |
| Agent | `.github/agents/l1-test-engineer.agent.md` | Workflow, tool limits, output format |
| Skills | `.github/skills/l1-gap-analyzer`, `l1-test-writer`, `l1-register-binary`, `l1-test-verifier` | How to find gaps, write tests, register binaries, verify |
| Scripts | `test/l1_gaps.sh`, `test/l1_check.sh`, `test/l1_modules.txt` | Build, run, coverage, gates (in the CI container) |

The previous bootstrap prompt is kept for reference in `archive/l1-agent-bootstrap.md` and is no longer a slash command.

## Prerequisites

- Docker running on the host. Both scripts start the CI image (`native-platform`) themselves; the host tree is mounted read-only.
- The repository is a git checkout; the verifier compares the working tree with `HEAD`.
- On Apple Silicon the current image runs under amd64 emulation. A first full `l1_check.sh` run on `scheduler` took about 12 minutes.

## How to invoke

In Copilot Chat (Agent mode), type:

```text
/l1-coverage MODULE=scheduler MAX_FUNCTIONS=2
```

Optional: `FOCUS=registerProfileWithScheduler` to restrict targets. Modules: `test/l1_gaps.sh --list`.

You can also select the **L1 Test Engineer** agent in the agent picker and describe the task in plain words.

## What you should see

1. Gap list from `test/l1_gaps.sh` with the coverage summary.
2. For each target: the chosen function, its class (A/B/C/D), the lines it aims to reach.
3. Tests added to the module's existing test file.
4. `test/l1_check.sh` gate lines after each function, and a mutation check per new test.
5. A final table: functions, tests, coverage before and after, skipped gaps with reasons.

## Run the scripts yourself

```bash
test/l1_gaps.sh scheduler --top 10        # ranked gaps
test/l1_check.sh scheduler --no-valgrind  # quick gates vs HEAD
test/l1_check.sh scheduler                # full gates incl. valgrind
```

Results are in `/tmp/l1` (`L1_OUT` to change). See the skills for details and exit codes.

## Review checklist

| # | Check |
|---|---|
| 1 | `git status`: only files under `source/test/` (and registration files, if a binary was added) changed |
| 2 | `test/l1_check.sh <module>` prints `RESULT: PASS` |
| 3 | Each new test asserts behavior and has a meaningful name |
| 4 | Reported coverage numbers match the script output |

---

## Useful follow-up prompts

- `Only target the NULL and error-return branches in unregisterProfileFromScheduler.`
- `That test has no meaningful assertion. Rewrite it to assert the return value and the mock interaction.`
- `Classify the remaining gaps as A/B/C/D and stop; do not add more tests.`
- `You want a production seam. Show the exact diff, the risk, and the coverage gain before I approve.`
- `Run the full test/l1_check.sh with valgrind and paste the gate lines.`

---

## Guardrails worth watching

- The agent must not edit production code. `git diff --stat -- source ':!source/test'` should be empty.
- Pre-existing valgrind leaks at HEAD do not fail the gate; only growth does.
- cppcheck is not in the current CI image, so that gate is skipped with a NOTE.
- Coverage thresholds are not a per-run target; each run should leave touched files at equal or higher coverage.
