"""Tests for code_parser module — direct call extraction and wrapper resolution."""

import os
import tempfile
import pytest

from tools.marker_discovery.code_parser import scan_direct_calls, detect_wrappers, resolve_wrapper_calls, scan_repo


@pytest.fixture
def repo_with_direct_calls(tmp_path):
    """Create a fake repo with direct t2_event_* calls."""
    src = tmp_path / "src"
    src.mkdir()

    (src / "telemetry.c").write_text('''
#include <stdio.h>
#include "t2_api.h"

void report_metrics() {
    t2_event_s("WIFI_CONNECT_OK", "success");
    t2_event_d("CPU_USAGE", cpu_pct);
    t2_event_f("MEM_FREE_PCT", mem_free);
}
''')

    (src / "other.h").write_text('''
#ifndef OTHER_H
#define OTHER_H
// No markers here
void do_stuff();
#endif
''')

    return tmp_path


@pytest.fixture
def repo_with_multiline_call(tmp_path):
    """Repo with a multi-line t2_event call."""
    src = tmp_path / "src"
    src.mkdir()

    (src / "multi.c").write_text('''
void foo() {
    t2_event_s(
        "LONG_MARKER_NAME",
        some_variable
    );
}
''')
    return tmp_path


@pytest.fixture
def repo_with_wrapper(tmp_path):
    """Repo with a wrapper function and call sites."""
    src = tmp_path / "src"
    src.mkdir()

    # File with wrapper definition
    (src / "wrapper.c").write_text('''
#include "t2_api.h"

void t2CountNotify(const char *marker, int val) {
    t2_event_d(marker, val);
}
''')

    # File with wrapper call sites
    (src / "caller.c").write_text('''
#include "wrapper.h"

void check_status() {
    t2CountNotify("STATUS_CHECK_OK", 1);
    t2CountNotify("STATUS_CHECK_FAIL", 0);
}
''')

    return tmp_path


@pytest.fixture
def repo_with_variable_marker(tmp_path):
    """Repo with a t2_event_* call using a variable (not resolvable without wrapper)."""
    src = tmp_path / "src"
    src.mkdir()

    (src / "dynamic.c").write_text('''
void report(const char *name) {
    t2_event_s(name, "1");
}
''')
    return tmp_path


class TestDirectCalls:
    def test_finds_all_three_api_types(self, repo_with_direct_calls):
        markers = scan_direct_calls(repo_with_direct_calls, "test-repo")

        assert len(markers) == 3
        names = {m.marker_name for m in markers}
        assert names == {"WIFI_CONNECT_OK", "CPU_USAGE", "MEM_FREE_PCT"}

        apis = {m.api for m in markers}
        assert apis == {"t2_event_s", "t2_event_d", "t2_event_f"}

    def test_correct_metadata(self, repo_with_direct_calls):
        markers = scan_direct_calls(repo_with_direct_calls, "my-component")

        for m in markers:
            assert m.component == "my-component"
            assert m.source_type == "source"
            assert m.file_path.startswith("src/")
            assert m.line > 0

    def test_multiline_call(self, repo_with_multiline_call):
        markers = scan_direct_calls(repo_with_multiline_call, "test")

        assert len(markers) == 1
        assert markers[0].marker_name == "LONG_MARKER_NAME"
        assert markers[0].api == "t2_event_s"

    def test_skips_variable_marker(self, repo_with_variable_marker):
        """Direct scan should NOT pick up calls with variable first args."""
        markers = scan_direct_calls(repo_with_variable_marker, "test")
        assert len(markers) == 0

    def test_empty_repo(self, tmp_path):
        markers = scan_direct_calls(tmp_path, "empty")
        assert markers == []


class TestWrapperDetection:
    def test_detects_wrapper(self, repo_with_wrapper):
        wrappers = detect_wrappers(repo_with_wrapper)

        assert len(wrappers) == 1
        w = wrappers[0]
        assert w["wrapper_name"] == "t2CountNotify"
        assert w["marker_param_index"] == 0
        assert w["api"] == "t2_event_d"

    def test_no_wrappers_in_direct_only(self, repo_with_direct_calls):
        wrappers = detect_wrappers(repo_with_direct_calls)
        assert len(wrappers) == 0


class TestWrapperResolution:
    def test_resolves_call_sites(self, repo_with_wrapper):
        wrappers = detect_wrappers(repo_with_wrapper)
        markers = resolve_wrapper_calls(repo_with_wrapper, "test-repo", wrappers)

        assert len(markers) == 2
        names = {m.marker_name for m in markers}
        assert names == {"STATUS_CHECK_OK", "STATUS_CHECK_FAIL"}

        for m in markers:
            assert m.api == "t2CountNotify→t2_event_d"
            assert m.source_type == "source"


class TestScanRepo:
    def test_full_scan_combines_direct_and_wrapper(self, repo_with_wrapper):
        # Add a direct call file too
        src = repo_with_wrapper / "src"
        (src / "direct.c").write_text('''
void init() {
    t2_event_s("INIT_OK", "1");
}
''')

        markers = scan_repo(repo_with_wrapper, "combo-repo")

        names = {m.marker_name for m in markers}
        assert "INIT_OK" in names
        assert "STATUS_CHECK_OK" in names
        assert "STATUS_CHECK_FAIL" in names
