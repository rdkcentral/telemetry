"""Quick integration test against a real cloned repo."""
import sys
sys.path.insert(0, "/mnt/L2_CONTAINER_SHARED_VOLUME/59001")

from tools.marker_discovery.code_parser import scan_repo
from tools.marker_discovery.script_parser import scan_repo_scripts
from tools.marker_discovery.patch_parser import scan_repo_patches
from tools.marker_discovery.report_generator import generate_report

repo_path = "/tmp/test_telemetry"
repo_name = "telemetry"

print("=== Code Parser ===")
code_markers = scan_repo(repo_path, repo_name)
for m in code_markers[:10]:
    print(f"  {m.marker_name:40s} | {m.file_path:50s} | {m.line:5d} | {m.api}")
print(f"  ... Total code markers: {len(code_markers)}")

print("\n=== Script Parser ===")
script_markers = scan_repo_scripts(repo_path, repo_name)
for m in script_markers[:10]:
    print(f"  {m.marker_name:40s} | {m.file_path:50s} | {m.line:5d} | {m.api}")
print(f"  ... Total script markers: {len(script_markers)}")

print("\n=== Patch Parser ===")
patch_markers = scan_repo_patches(repo_path, repo_name)
for m in patch_markers[:10]:
    print(f"  {m.marker_name:40s} | {m.file_path:50s} | {m.line:5d} | {m.api}")
print(f"  ... Total patch markers: {len(patch_markers)}")

all_markers = code_markers + script_markers + patch_markers
print(f"\n=== TOTAL: {len(all_markers)} markers ===")

print("\n=== Report Preview (first 30 lines) ===")
report = generate_report(all_markers, "develop", ["rdkcentral"], 1)
for line in report.split("\n")[:30]:
    print(line)
