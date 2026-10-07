import sys
sys.path.insert(0, "/mnt/L2_CONTAINER_SHARED_VOLUME/59001")
from tools.marker_discovery.code_parser import detect_wrappers
ws = detect_wrappers("/tmp/test_telemetry")
for w in ws:
    print(w)
