"""Tests for script_parser module — t2ValNotify/t2CountNotify extraction."""

import pytest

from tools.marker_discovery.script_parser import (
    scan_repo_scripts,
    _is_dynamic_marker,
    _resolve_function_calls,
)


@pytest.fixture
def repo_with_scripts(tmp_path):
    """Create a fake repo with script files containing marker calls."""
    scripts = tmp_path / "scripts"
    scripts.mkdir()

    (scripts / "monitor.sh").write_text('''#!/bin/bash
# Monitor script
CPU=$(top -bn1 | grep "Cpu(s)")
t2ValNotify "CPU_USAGE_HIGH" "$CPU"
t2CountNotify "MONITOR_RUN_COUNT" 1
echo "Done"
''')

    (scripts / "setup.py").write_text('''#!/usr/bin/env python3
import os
# No markers in this file
print("setup done")
''')

    (scripts / "telemetry.sh").write_text('''#!/bin/bash
t2ValNotify "WIFI_STATUS" "connected"
t2ValNotify "BOOT_TIME" "$boot_secs"
''')

    return tmp_path


@pytest.fixture
def repo_with_c_files_only(tmp_path):
    """Repo with only C files — script parser should find nothing."""
    src = tmp_path / "src"
    src.mkdir()

    # This file has t2ValNotify but it's a .c file — should be skipped
    (src / "wrapper.c").write_text('''
void t2ValNotify(const char *marker, const char *val) {
    t2_event_s(marker, val);
}
''')
    return tmp_path


@pytest.fixture
def repo_with_mixed(tmp_path):
    """Repo with both scripts and C files."""
    (tmp_path / "run.sh").write_text('t2CountNotify "SCRIPT_MARKER" 1\n')
    src = tmp_path / "src"
    src.mkdir()
    (src / "main.c").write_text('t2_event_s("C_MARKER", "val");\n')
    return tmp_path


@pytest.fixture
def repo_with_dynamic_markers(tmp_path):
    """Repo with dynamic markers containing shell variables."""
    scripts = tmp_path / "scripts"
    scripts.mkdir()

    (scripts / "dynamic.sh").write_text('''#!/bin/bash
t2ValNotify "SYST_ERR_CrashSig$2" "crash"
t2ValNotify "WIFIV_INFO_NO${version}ROUTE" "1"
t2CountNotify "SYST_ERR_$source_reboot" 1
t2ValNotify "CLEAN_MARKER" "ok"
''')

    return tmp_path


@pytest.fixture
def repo_with_positional_resolution(tmp_path):
    """Repo where a function uses $1 and is called with a literal."""
    scripts = tmp_path / "scripts"
    scripts.mkdir()

    (scripts / "log.sh").write_text('''#!/bin/bash

log_marker() {
    t2ValNotify "$1" "$2"
}

log_marker "RESOLVED_MEMORY" "512"
log_marker "RESOLVED_CPU" "75"
''')

    return tmp_path


class TestScriptParser:
    def test_finds_val_and_count_notify(self, repo_with_scripts):
        markers = scan_repo_scripts(repo_with_scripts, "test-repo")

        names = {m.marker_name for m in markers}
        assert "CPU_USAGE_HIGH" in names
        assert "MONITOR_RUN_COUNT" in names
        assert "WIFI_STATUS" in names
        assert "BOOT_TIME" in names

    def test_correct_api_names(self, repo_with_scripts):
        markers = scan_repo_scripts(repo_with_scripts, "test-repo")

        api_map = {m.marker_name: m.api for m in markers}
        assert api_map["CPU_USAGE_HIGH"] == "t2ValNotify"
        assert api_map["MONITOR_RUN_COUNT"] == "t2CountNotify"

    def test_source_type_for_static_markers(self, repo_with_scripts):
        markers = scan_repo_scripts(repo_with_scripts, "test")
        static = [m for m in markers if m.source_type == "script"]
        assert len(static) > 0
        for m in static:
            assert "$" not in m.marker_name

    def test_skips_c_files(self, repo_with_c_files_only):
        markers = scan_repo_scripts(repo_with_c_files_only, "test")
        assert len(markers) == 0

    def test_only_scans_non_c_files(self, repo_with_mixed):
        markers = scan_repo_scripts(repo_with_mixed, "test")
        names = {m.marker_name for m in markers}
        assert "SCRIPT_MARKER" in names
        assert "C_MARKER" not in names

    def test_empty_repo(self, tmp_path):
        markers = scan_repo_scripts(tmp_path, "empty")
        assert markers == []

    def test_line_numbers(self, repo_with_scripts):
        markers = scan_repo_scripts(repo_with_scripts, "test")
        for m in markers:
            assert m.line > 0


class TestDynamicMarkers:
    def test_is_dynamic_marker(self):
        assert _is_dynamic_marker("SYST_ERR_CrashSig$2")
        assert _is_dynamic_marker("WIFIV_INFO_NO${version}ROUTE")
        assert _is_dynamic_marker("$1")
        assert _is_dynamic_marker("$source_reboot")
        assert not _is_dynamic_marker("CLEAN_MARKER")
        assert not _is_dynamic_marker("CPU_USAGE_HIGH")

    def test_dynamic_markers_flagged(self, repo_with_dynamic_markers):
        markers = scan_repo_scripts(repo_with_dynamic_markers, "test")

        dynamic = [m for m in markers if m.source_type == "script_dynamic"]
        static = [m for m in markers if m.source_type == "script"]

        dynamic_names = {m.marker_name for m in dynamic}
        static_names = {m.marker_name for m in static}

        assert "SYST_ERR_CrashSig$2" in dynamic_names
        assert "WIFIV_INFO_NO${version}ROUTE" in dynamic_names
        assert "SYST_ERR_$source_reboot" in dynamic_names
        assert "CLEAN_MARKER" in static_names

    def test_positional_arg_resolution(self, repo_with_positional_resolution):
        markers = scan_repo_scripts(repo_with_positional_resolution, "test")

        names = {m.marker_name for m in markers}
        # $1 should be resolved to the literals from call sites
        assert "RESOLVED_MEMORY" in names
        assert "RESOLVED_CPU" in names
        # The raw $1 should NOT appear since it was resolved
        assert "$1" not in names

    def test_resolved_markers_have_wrapper_api(self, repo_with_positional_resolution):
        markers = scan_repo_scripts(repo_with_positional_resolution, "test")

        for m in markers:
            # Resolved markers should show function→api chain
            assert "log_marker→t2ValNotify" in m.api

    def test_unresolvable_positional_stays_dynamic(self, tmp_path):
        """If a function using $1 is never called with a literal, keep it as dynamic."""
        scripts = tmp_path / "scripts"
        scripts.mkdir()

        (scripts / "orphan.sh").write_text('''#!/bin/bash
orphan_func() {
    t2ValNotify "$1" "val"
}
''')

        markers = scan_repo_scripts(tmp_path, "test")
        assert len(markers) == 1
        assert markers[0].marker_name == "$1"
        assert markers[0].source_type == "script_dynamic"


class TestFunctionResolution:
    def test_resolve_function_calls_basic(self):
        content = '''#!/bin/bash
log_marker "HELLO" "world"
log_marker "GOODBYE" "!"
'''
        resolved = _resolve_function_calls(content, "log_marker", 0)
        assert "HELLO" in resolved
        assert "GOODBYE" in resolved

    def test_resolve_function_calls_second_arg(self):
        content = '''#!/bin/bash
send "prefix" "MARKER_TWO"
'''
        resolved = _resolve_function_calls(content, "send", 1)
        assert "MARKER_TWO" in resolved

    def test_resolve_skips_dynamic_values(self):
        content = '''#!/bin/bash
log_marker "$VAR" "data"
log_marker "STATIC" "data"
'''
        resolved = _resolve_function_calls(content, "log_marker", 0)
        assert "STATIC" in resolved
        assert "$VAR" not in resolved
