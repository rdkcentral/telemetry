from dataclasses import dataclass


@dataclass
class MarkerRecord:
    """A single discovered telemetry marker."""
    marker_name: str
    component: str
    file_path: str
    line: int
    api: str
    source_type: str  # "source" | "script" | "patch"
