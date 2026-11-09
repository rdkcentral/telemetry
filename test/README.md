
# Testing Instructions

All L1 (Unit tests) and L2 (Integration tests) tests for the application are included in this container.
Tests are built and run inside the container using the existing build and test scripts. 
Test results are printed to the console and can also be saved to a file for further analysis.

## Running L1 Tests

To run the tests, follow these steps:

````bash
# Clone the repository and navigate to the test directory
git clone <repository_url>
cd telemetry
# Build and run tests inside the container using the provided script

docker run -d --name l1-final-check --platform linux/amd64 -v "$PWD:/mnt/workspace" ghcr.io/rdkcentral/docker-device-mgt-service-test/native-platform:latest
docker exec -i l1-final-check /bin/bash -c "cd /mnt/workspace && sh test/run_ut.sh 2>&1" | tee /tmp/l1_test_output.log

# Once tests are complete, you can stop and remove the container
docker stop l1-final-check
docker rm l1-final-check

````

This will run all the unit tests and print the results to the console. The test results will also be saved to `/tmp/l1_test_output.log` for further analysis.

## Running L2 Tests

To run the integration tests, follow these steps:

````bash
# Start the mock XCONF container
docker run -d --name mockxconf --platform linux/amd64 ghcr.io/rdkcentral/docker-device-mgt-service-test/mockxconf:latest
# Start the main test container and link it to the mock XCONF container
docker run -d --name l2-final-check --platform linux/amd64 --link mockxconf:mockxconf -v "$PWD:/mnt/workspace" ghcr.io/rdkcentral/docker-device-mgt-service-test/native-platform:latest
# Run the integration tests inside the main test container
docker exec -i l2-final-check /bin/bash -c "cd /mnt/workspace && sh test/run_it.sh 2>&1" | tee /tmp/l2_test_output.log
# Once tests are complete, you can stop and remove the containers
docker stop l2-final-check mockxconf
docker rm l2-final-check mockxconf
````

This will run all the integration tests and print the results to the console. The test results will also be saved to `/tmp/l2_test_output.log` for further analysis.

## L1 Development Utilities

In addition to `run_ut.sh`, this directory has scripts for day-to-day L1 work: finding coverage
gaps, checking a module's current state, and confirming new tests are properly registered.
They all understand the module map in `test/l1_modules.txt` and, on a host without a build
toolchain (e.g. macOS), run themselves inside a persistent Docker container/volume instead of
requiring `--local`/a manual container.

### `test/l1_modules.txt`

The registry of L1 modules: `name|test_dir|bin1,bin2,...|src_dir1,src_dir2,...`. Every module
used by the scripts below must have an entry here, matching the `bin_PROGRAMS` built by its
`Makefile.am` and the binaries listed in `test/run_ut.sh`.

### `test/l1_gaps.sh` — find coverage gaps

Builds one module with coverage instrumentation, runs its gtest binaries, and ranks the
least-covered functions/lines.

```bash
sh test/l1_gaps.sh --list                       # list known modules
sh test/l1_gaps.sh scheduler --build --top 20    # top 20 coverage gaps for scheduler
```

Flags: `--build` (rebuild first), `--top N` (how many gaps to print, `0` to just measure),
`--local` (run directly instead of via Docker; needs `lcov`/autotools on the host).
Env: `L1_OUT` (default `/tmp/l1`), `L1_IMAGE`, `L1_DOCKER_PLATFORM`, `LCOV_EXTRA`.

### `test/l1_check.sh` — check a module's current L1 state

Builds and tests one module's working tree and reports its current state: registration gates
(binary wired into `Makefile.am`/`configure.ac`/`run_ut.sh`/`l1_modules.txt`), whether the
build and all tests pass, and informational coverage/cppcheck notes. Valgrind (if enabled) is
a hard gate — any leak or error fails the check.

```bash
sh test/l1_check.sh scheduler --no-valgrind      # quick check, skip valgrind
sh test/l1_check.sh scheduler                    # full check including valgrind
```

Exit code `0` means every gate passed (`RESULT: PASS`); non-zero means a gate failed or the
module/tooling could not be checked. Env: same as `l1_gaps.sh`.

### `test/l1_docker_cache.sh` — shared Docker helpers

Not run directly; sourced by `l1_gaps.sh` and `l1_check.sh` to manage a persistent container
(`l1-check-runner`) and volume (`l1-check-vol`) that are reused across invocations instead of
being recreated every run, with the working tree synced in incrementally via `rsync` so
previously built objects (`.o`/`.gcno`/`.gcda`) and `make`'s incremental state survive between
runs. To force a clean rebuild:

```bash
docker rm -f l1-check-runner; docker volume rm l1-check-vol
```

## Agentic L1 Skills

These repository skills (under `.github/skills/`) wrap the utilities above into guided,
autonomous workflows for an AI coding agent. Invoke them by describing the task; the agent
picks the matching skill.

| Skill | Use when |
|---|---|
| `l1-gap-analyzer` | Finding and ranking L1 coverage gaps for a module, picking the next function to test. |
| `l1-test-writer` | Writing new L1 gtest/gmock tests for a function, following the module's existing fixtures/mocks. |
| `l1-test-verifier` | Verifying an L1 change before calling it done: registration, build, tests pass, no regressions. |
| `l1-register-binary` | Wiring up a new test source/binary/module directory into the Autotools build and `run_ut.sh`. |

See `AGENTS.md` and `.github/copilot-instructions.md` for the mandatory L1-testing rules these
skills enforce (every function/module/bug-fix change ships with L1 tests, coverage must not
regress, etc.).
