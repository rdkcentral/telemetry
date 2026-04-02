"""Tests for report_generator module — markdown output, sorting, duplicates."""

import pytest

from tools.marker_discovery import MarkerRecord
from tools.marker_discovery.report_generator import generate_report, _find_duplicates


def _make_marker(name, component, file_path="src/test.c", line=1, api="t2_event_s", source_type="source"):
    return MarkerRecord(name, component, file_path, line, api, source_type)


class TestDuplicateDetection:
    def test_no_duplicates(self):
        markers = [
            _make_marker("A", "repo1"),
            _make_marker("B", "repo2"),
        ]
        dupes = _find_duplicates(markers)
        assert len(dupes) == 0

    def test_same_name_same_component_not_duplicate(self):
        markers = [
            _make_marker("A", "repo1", "file1.c", 10),
            _make_marker("A", "repo1", "file2.c", 20),
        ]
        dupes = _find_duplicates(markers)
        assert len(dupes) == 0

    def test_same_name_different_component_is_duplicate(self):
        markers = [
            _make_marker("CPU_USAGE", "repo1", api="t2_event_f"),
            _make_marker("CPU_USAGE", "repo2", api="t2_event_d"),
        ]
        dupes = _find_duplicates(markers)
        assert "CPU_USAGE" in dupes
        assert len(dupes["CPU_USAGE"]) == 2


class TestReportGeneration:
    def test_basic_report_structure(self):
        markers = [_make_marker("BOOT_TIME", "dcm-agent", "src/dcm.c", 234, "t2_event_d")]
        report = generate_report(markers, "develop", ["rdkcentral"], 10)

        assert "# Telemetry Marker Inventory" in report
        assert "**Branch**: develop" in report
        assert "**Organizations**: rdkcentral" in report
        assert "## Summary" in report
        assert "**Total Markers**: 1" in report
        assert "## Unique Marker Inventory" in report
        assert "## Detailed Marker Inventory" in report
        assert "BOOT_TIME" in report
        assert "dcm-agent" in report

    def test_sorted_output(self):
        markers = [
            _make_marker("ZEBRA", "repo1"),
            _make_marker("ALPHA", "repo2"),
            _make_marker("MIDDLE", "repo1"),
        ]
        report = generate_report(markers, "main", ["rdkcentral"], 5)

        alpha_pos = report.index("ALPHA")
        middle_pos = report.index("MIDDLE")
        zebra_pos = report.index("ZEBRA")
        assert alpha_pos < middle_pos < zebra_pos

    def test_duplicates_flagged(self):
        markers = [
            _make_marker("CPU_USAGE", "repo1", api="t2_event_f"),
            _make_marker("CPU_USAGE", "repo2", api="t2_event_d"),
            _make_marker("UNIQUE", "repo1"),
        ]
        report = generate_report(markers, "develop", ["rdkcentral"], 10)

        assert "CPU_USAGE ⚠️" in report
        assert "## Duplicate Markers" in report
        assert "Found in 2 components" in report

    def test_no_duplicate_section_when_none(self):
        markers = [_make_marker("A", "repo1"), _make_marker("B", "repo2")]
        report = generate_report(markers, "develop", ["rdkcentral"], 10)

        assert "## Duplicate Markers" not in report

    def test_empty_markers(self):
        report = generate_report([], "develop", ["rdkcentral"], 0)

        assert "**Total Markers**: 0" in report
        assert "## Duplicate Markers" not in report

    def test_multiple_orgs(self):
        markers = [_make_marker("M", "repo1")]
        report = generate_report(markers, "develop", ["rdkcentral", "rdk-e"], 20)

        assert "rdkcentral, rdk-e" in report

    def test_script_markers_in_report(self):
        markers = [
            _make_marker("SCRIPT_M", "repo1", "scripts/run.sh", 5, "t2ValNotify", "script"),
        ]
        report = generate_report(markers, "develop", ["rdkcentral"], 10)

        assert "t2ValNotify" in report
        assert "SCRIPT_M" in report

    def test_patch_markers_in_report(self):
        markers = [
            _make_marker("PATCH_M", "repo1 (patch)", "patches/fix.patch", 12, "t2_event_s", "patch"),
        ]
        report = generate_report(markers, "develop", ["rdkcentral"], 10)

        assert "repo1 (patch)" in report
        assert "PATCH_M" in report

    def test_dynamic_markers_separate_section(self):
        markers = [
            _make_marker("CLEAN", "repo1", "src/main.sh", 1, "t2ValNotify", "script"),
            _make_marker("SYST_ERR_$var", "repo2", "scripts/err.sh", 5, "t2ValNotify", "script_dynamic"),
        ]
        report = generate_report(markers, "develop", ["rdkcentral"], 10)

        assert "## Dynamic Markers" in report
        assert "SYST_ERR_$var" in report
        # Static markers should be in inventory, dynamic should NOT
        inventory_section = report.split("## Dynamic Markers")[0]
        assert "CLEAN" in inventory_section

    def test_no_dynamic_section_when_none(self):
        markers = [_make_marker("A", "repo1", source_type="script")]
        report = generate_report(markers, "develop", ["rdkcentral"], 10)

        assert "## Dynamic Markers" not in report

    def test_summary_shows_static_and_dynamic_counts(self):
        markers = [
            _make_marker("STATIC1", "repo1", source_type="script"),
            _make_marker("DYN_$x", "repo1", source_type="script_dynamic"),
            _make_marker("DYN_$y", "repo2", source_type="script_dynamic"),
        ]
        report = generate_report(markers, "develop", ["rdkcentral"], 10)

        assert "**Total Markers**: 3" in report
        assert "**Static Markers**: 1" in report
        assert "**Dynamic Markers**: 2" in report

    def test_duplicates_only_among_static(self):
        markers = [
            _make_marker("SHARED", "repo1", source_type="source"),
            _make_marker("SHARED", "repo2", source_type="source"),
            _make_marker("DYN_$x", "repo3", source_type="script_dynamic"),
            _make_marker("DYN_$x", "repo4", source_type="script_dynamic"),
        ]
        report = generate_report(markers, "develop", ["rdkcentral"], 10)

        assert "## Duplicate Markers" in report
        assert "SHARED" in report.split("## Duplicate Markers")[1]
        # DYN_$x appears in 2 repos but should NOT be flagged as duplicate
        # since dynamic markers are excluded from duplicate detection
        dup_section = report.split("## Duplicate Markers")[1]
        assert "DYN_$x" not in dup_section

    def test_unresolved_components_section(self):
        markers = [_make_marker("A", "repo1")]
        unresolved = [
            {"name": "missing-repo", "version": "1.0.0-r0", "reason": "Not found in any organization"},
            {"name": "broken-repo", "version": "2.0.0-r1", "reason": "Clone failed in rdkcentral"},
        ]
        report = generate_report(markers, "develop", ["rdkcentral"], 10, unresolved_components=unresolved)

        assert "## Unresolved Components" in report
        assert "missing-repo" in report
        assert "broken-repo" in report
        assert "Not found in any organization" in report

    def test_no_unresolved_section_when_empty(self):
        markers = [_make_marker("A", "repo1")]
        report = generate_report(markers, "develop", ["rdkcentral"], 10)

        assert "## Unresolved Components" not in report

    def test_unresolved_count_in_summary(self):
        markers = [_make_marker("A", "repo1")]
        unresolved = [
            {"name": "missing", "version": "1.0", "reason": "Not found"},
        ]
        report = generate_report(markers, "develop", ["rdkcentral"], 10, unresolved_components=unresolved)

        assert "**Unresolved Components**: 1" in report
