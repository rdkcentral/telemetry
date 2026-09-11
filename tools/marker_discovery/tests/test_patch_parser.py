"""Tests for patch_parser module — .patch file scanning."""

import pytest

from tools.marker_discovery.patch_parser import scan_repo_patches


SAMPLE_PATCH = '''diff --git a/src/telemetry.c b/src/telemetry.c
--- a/src/telemetry.c
+++ b/src/telemetry.c
@@ -10,6 +10,8 @@
 context line
+    t2_event_s("NEW_MARKER", "value");
+    t2_event_d("ADDED_COUNT", count);
-    t2_event_s("OLD_MARKER", "removed");
 more context
'''

SAMPLE_PATCH_SCRIPTS = '''diff --git a/scripts/monitor.sh b/scripts/monitor.sh
--- a/scripts/monitor.sh
+++ b/scripts/monitor.sh
@@ -5,3 +5,5 @@
 echo "starting"
+t2ValNotify "SCRIPT_NEW_MARKER" "$val"
+t2CountNotify "SCRIPT_COUNT" 1
 echo "done"
'''

SAMPLE_PATCH_MIXED = '''diff --git a/src/foo.c b/src/foo.c
+++ b/src/foo.c
@@ -1,3 +1,5 @@
+    t2_event_f("FLOAT_MARKER", fval);
+t2ValNotify "MIXED_VAL" "test"
'''


@pytest.fixture
def repo_with_patches(tmp_path):
    patches = tmp_path / "patches"
    patches.mkdir()
    (patches / "telemetry.patch").write_text(SAMPLE_PATCH)
    return tmp_path


@pytest.fixture
def repo_with_script_patches(tmp_path):
    patches = tmp_path / "patches"
    patches.mkdir()
    (patches / "scripts.patch").write_text(SAMPLE_PATCH_SCRIPTS)
    return tmp_path


@pytest.fixture
def repo_with_mixed_patches(tmp_path):
    patches = tmp_path / "patches"
    patches.mkdir()
    (patches / "mixed.patch").write_text(SAMPLE_PATCH_MIXED)
    return tmp_path


class TestPatchParser:
    def test_finds_added_c_markers(self, repo_with_patches):
        markers = scan_repo_patches(repo_with_patches, "test-repo")

        names = {m.marker_name for m in markers}
        assert "NEW_MARKER" in names
        assert "ADDED_COUNT" in names
        # Removed lines should NOT be found
        assert "OLD_MARKER" not in names

    def test_correct_api_types(self, repo_with_patches):
        markers = scan_repo_patches(repo_with_patches, "test")
        api_map = {m.marker_name: m.api for m in markers}
        assert api_map["NEW_MARKER"] == "t2_event_s"
        assert api_map["ADDED_COUNT"] == "t2_event_d"

    def test_component_has_patch_suffix(self, repo_with_patches):
        markers = scan_repo_patches(repo_with_patches, "meta-rdk")
        for m in markers:
            assert m.component == "meta-rdk (patch)"

    def test_source_type_is_patch(self, repo_with_patches):
        markers = scan_repo_patches(repo_with_patches, "test")
        for m in markers:
            assert m.source_type == "patch"

    def test_finds_script_apis_in_patches(self, repo_with_script_patches):
        markers = scan_repo_patches(repo_with_script_patches, "test")

        names = {m.marker_name for m in markers}
        assert "SCRIPT_NEW_MARKER" in names
        assert "SCRIPT_COUNT" in names

        api_map = {m.marker_name: m.api for m in markers}
        assert api_map["SCRIPT_NEW_MARKER"] == "t2ValNotify"
        assert api_map["SCRIPT_COUNT"] == "t2CountNotify"

    def test_mixed_c_and_script_in_patch(self, repo_with_mixed_patches):
        markers = scan_repo_patches(repo_with_mixed_patches, "test")

        names = {m.marker_name for m in markers}
        assert "FLOAT_MARKER" in names
        assert "MIXED_VAL" in names

    def test_empty_repo(self, tmp_path):
        markers = scan_repo_patches(tmp_path, "empty")
        assert markers == []

    def test_line_numbers_positive(self, repo_with_patches):
        markers = scan_repo_patches(repo_with_patches, "test")
        for m in markers:
            assert m.line > 0
