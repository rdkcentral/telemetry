---
name: 'l1-coverage'
description: 'Add L1 unit tests to one telemetry module to close coverage gaps, verified against git HEAD'
agent: 'L1 Test Engineer'
argument-hint: 'MODULE=<scheduler|bulkdata|...> [MAX_FUNCTIONS=3] [FOCUS=<function or file>]'
---

Improve L1 coverage for the module given in the input.

Inputs:
- `MODULE` (required): a module name from [test/l1_modules.txt](../../test/l1_modules.txt). Run `test/l1_gaps.sh --list` if unsure.
- `MAX_FUNCTIONS` (default 3): stop after this many functions have new tests.
- `FOCUS` (optional): restrict targets to this function or file.

Follow your workflow and output format. Start with `test/l1_gaps.sh <MODULE>` and finish with a full `test/l1_check.sh <MODULE>`.
