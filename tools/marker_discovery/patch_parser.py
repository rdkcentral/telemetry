"""Patch file scanner for t2 telemetry markers in added lines."""

import logging
import os
import re

from . import MarkerRecord

logger = logging.getLogger(__name__)

# Patterns for C/C++ API calls in patches
C_PATTERNS = [
    (re.compile(r't2_event_s\s*\(\s*"([^"]+)"'), "t2_event_s"),
    (re.compile(r't2_event_d\s*\(\s*"([^"]+)"'), "t2_event_d"),
    (re.compile(r't2_event_f\s*\(\s*"([^"]+)"'), "t2_event_f"),
]

# Patterns for script API calls in patches
SCRIPT_PATTERNS = [
    (re.compile(r't2ValNotify\s+"([^"]+)"'), "t2ValNotify"),
    (re.compile(r't2CountNotify\s+"([^"]+)"'), "t2CountNotify"),
]

ALL_PATTERNS = C_PATTERNS + SCRIPT_PATTERNS


# Directory names to skip during scanning
SKIP_DIRS = {"test", "tests", ".git"}


def _find_patch_files(repo_path):
    """Recursively find all .patch files in a directory."""
    patch_files = []
    for root, dirs, files in os.walk(repo_path):
        dirs[:] = [d for d in dirs if d.lower() not in SKIP_DIRS]
        for fname in files:
            if fname.endswith(".patch"):
                patch_files.append(os.path.join(root, fname))
    return patch_files


def scan_repo_patches(repo_path, repo_name):
    """Scan all .patch files in a repo for t2 marker calls in added lines.

    Only lines starting with '+' (excluding '+++' headers) are scanned.
    Returns list of MarkerRecord with source_type="patch".
    """
    markers = []
    patch_files = _find_patch_files(repo_path)
    component = f"{repo_name} (patch)"

    for file_path in patch_files:
        rel_path = os.path.relpath(file_path, repo_path)

        try:
            with open(file_path, "r", errors="ignore") as f:
                for line_num, line in enumerate(f, start=1):
                    # Only scan added lines, exclude +++ header
                    if not line.startswith("+") or line.startswith("+++"):
                        continue

                    for pattern, api_name in ALL_PATTERNS:
                        for match in pattern.finditer(line):
                            marker_name = match.group(1)
                            markers.append(MarkerRecord(
                                marker_name=marker_name,
                                component=component,
                                file_path=rel_path,
                                line=line_num,
                                api=api_name,
                                source_type="patch",
                            ))
        except (IOError, OSError) as e:
            logger.warning("Could not read %s: %s", file_path, e)

    logger.info("Found %d patch markers in %s", len(markers), repo_name)
    return markers