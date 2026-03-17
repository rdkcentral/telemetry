"""Markdown report generator for telemetry marker discovery results."""

import logging
from collections import defaultdict
from datetime import datetime, timezone

from . import MarkerRecord

logger = logging.getLogger(__name__)


def _find_duplicates(markers):
    """Find markers that appear in more than one component.

    Returns dict: marker_name → list of MarkerRecord (only for duplicates).
    """
    by_name_and_component = defaultdict(set)
    by_name = defaultdict(list)

    for m in markers:
        by_name_and_component[m.marker_name].add(m.component)
        by_name[m.marker_name].append(m)

    duplicates = {}
    for name, components in by_name_and_component.items():
        if len(components) > 1:
            duplicates[name] = by_name[name]

    return duplicates


def _get_unique_components(markers):
    """Get set of unique component names from markers."""
    return {m.component for m in markers}


def generate_report(markers, branch, orgs, components_scanned):
    """Generate a markdown report from discovered markers.

    Args:
        markers: list of MarkerRecord
        branch: branch name that was scanned
        orgs: list of organization names
        components_scanned: total number of repos scanned

    Returns markdown string.
    """
    # Separate static and dynamic markers
    static_markers = [m for m in markers if m.source_type != "script_dynamic"]
    dynamic_markers = [m for m in markers if m.source_type == "script_dynamic"]

    # Sort by marker name, then component
    sorted_static = sorted(static_markers, key=lambda m: (m.marker_name.lower(), m.component.lower()))
    sorted_dynamic = sorted(dynamic_markers, key=lambda m: (m.marker_name.lower(), m.component.lower()))

    # Detect duplicates (among static markers only)
    duplicates = _find_duplicates(static_markers)
    duplicate_names = set(duplicates.keys())

    # Timestamp
    now = datetime.now(timezone.utc).strftime("%Y-%m-%d %H:%M:%S UTC")
    orgs_str = ", ".join(orgs)

    lines = []

    # Header
    lines.append("# Telemetry Marker Inventory")
    lines.append(f"**Branch**: {branch}")
    lines.append(f"**Organizations**: {orgs_str}")
    lines.append(f"**Generated**: {now}")
    lines.append("")

    # Summary
    lines.append("## Summary")
    lines.append(f"- **Total Markers**: {len(markers):,}")
    lines.append(f"- **Static Markers**: {len(sorted_static):,}")
    if sorted_dynamic:
        lines.append(f"- **Dynamic Markers**: {len(sorted_dynamic):,} (contain shell variables)")
    lines.append(f"- **Components Scanned**: {components_scanned}")
    if duplicate_names:
        lines.append(f"- **Duplicate Markers**: {len(duplicate_names)} ⚠️")
    else:
        lines.append(f"- **Duplicate Markers**: 0")
    lines.append("")

    # Marker Inventory table (static markers only)
    lines.append("## Marker Inventory")
    lines.append("| Marker Name | Component | File Path | Line | API |")
    lines.append("|-------------|-----------|-----------|------|-----|")

    for m in sorted_static:
        name_display = m.marker_name
        if m.marker_name in duplicate_names:
            name_display = f"{m.marker_name} ⚠️"
        lines.append(f"| {name_display} | {m.component} | {m.file_path} | {m.line} | {m.api} |")

    lines.append("")

    # Dynamic Markers section (markers containing shell variables)
    if sorted_dynamic:
        lines.append("## Dynamic Markers")
        lines.append("Markers containing shell variables (`$var`, `${var}`) that resolve at runtime.")
        lines.append("")
        lines.append("| Marker Pattern | Component | File Path | Line | API |")
        lines.append("|----------------|-----------|-----------|------|-----|")

        for m in sorted_dynamic:
            lines.append(f"| {m.marker_name} | {m.component} | {m.file_path} | {m.line} | {m.api} |")

        lines.append("")

    # Duplicate Markers section (only if duplicates exist)
    if duplicate_names:
        lines.append("## Duplicate Markers")
        for name in sorted(duplicate_names, key=str.lower):
            dup_records = duplicates[name]
            components = _get_unique_components(dup_records)
            lines.append(f"⚠️ **{name}** - Found in {len(components)} components:")
            for record in sorted(dup_records, key=lambda r: r.component.lower()):
                lines.append(f"- {record.component}: {record.file_path}:{record.line} (`{record.api}`)")
            lines.append("")

    return "\n".join(lines)