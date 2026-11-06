---
name: l1-register-binary
description: Register new L1 test sources, test binaries, or test module directories in this repository's Autotools build and test runner so they are built and executed in CI. Use when adding a new test file, a new *_gtest.bin, or a new source/test/<module> directory, or when l1_check.sh reports a registration failure.
---

# L1 Register Binary

A test that is not registered compiles locally but never runs in CI. Pick the smallest case that applies. Follow `.github/instructions/build-system.instructions.md` for Makefile style.

## Case 1: new test file in an existing binary (most common)

Add tests to the module's existing test `.cpp` whenever possible; then nothing needs registering. If a new `.cpp` is required:

1. Add it to `<binary>_gtest_bin_SOURCES` in `source/test/<module>/Makefile.am`.
2. If it needs a mock or production source that the binary does not compile yet, add those to the same `_SOURCES` (for example `../mocks/FileioMock.cpp`, `../../utils/persistence.c`).
3. If it needs another library, add it to `<binary>_gtest_bin_LDFLAGS`.

## Case 2: new binary in an existing module directory

1. `source/test/<module>/Makefile.am`:
   - append `<name>_gtest.bin` to `bin_PROGRAMS`
   - add `<name>_gtest_bin_SOURCES`, `<name>_gtest_bin_CPPFLAGS`, `<name>_gtest_bin_LDFLAGS` by copying an existing binary in the same file and trimming what is not needed
   - include `gtest_main.cpp` (or a copy with its own JSON report name, see below)
2. `test/run_ut.sh`: add `./source/test/<module>/<name>_gtest.bin` to the `tests` list.
3. `test/l1_modules.txt`: append `<name>_gtest.bin` to the module's binary list.

## Case 3: new module test directory

1. Create `source/test/<module>/` with `Makefile.am`, `gtest_main.cpp` and the test `.cpp`. Copy `source/test/scheduler/` as the template.
2. In `gtest_main.cpp`, set a unique `GTEST_DEFAULT_RESULT_FILENAME` (for example `<module>_gtest_report.json`); CI collects these from `/tmp/Gtest_Report`.
3. `source/test/Makefile.am`: add `<module>` to `SUBDIRS`.
4. `configure.ac`: add `source/test/<module>/Makefile` to the `AC_CONFIG_FILES` list inside the `--enable-gtestapp` block.
5. `test/run_ut.sh`: add each binary to the `tests` list.
6. `test/l1_modules.txt`: add `<module>|source/test/<module>|<bin>[,<bin>]|source/<prod dir>[,...]`.

Note: production `source/dcautil/` is tested in `source/test/dcautils/`. Check existing names before creating a directory.

## Verify

```bash
test/l1_check.sh <module> --no-valgrind
```

All `registration` lines must be `PASS`, the new binary must appear in the `tests` lines, and its tests must run (test count above 0). Do not change unrelated targets, flags, or the order of existing entries.
