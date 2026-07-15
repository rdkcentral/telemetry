####################################################################################
# If not stated otherwise in this file or this component's Licenses file the
# following copyright and licenses apply:
#
# Copyright 2024 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
####################################################################################

"""
L2 functional tests for the backup_logs synchronisation gate
(waitForBackupLogsDone) added to dcautil.c.

The function uses inotify to wait for /tmp/.backup_logs_done before
grepping PreviousLogs.  These tests exercise:
  1. Fast path  – sentinel already present at grep time.
  2. Inotify    – sentinel created while the daemon is waiting.
  3. Timeout    – sentinel never appears; daemon proceeds after 60 s.
  4. Completion – /tmp/.telemetry_prevlogs_done created after grep.
"""

import json
import os
import subprocess
import threading
from datetime import datetime as dt
from time import sleep

import base64
import msgpack
import pytest

from basic_constants import *
from helper_functions import *

# ── Sentinel paths (must match dcautil.h) ────────────────────────────
BACKUP_LOGS_DONE_FLAG = "/tmp/.backup_logs_done"
TELEMETRY_PREVLOGS_DONE_FLAG = "/tmp/.telemetry_prevlogs_done"
PREVIOUS_LOGS_DIR = "/opt/logs/PreviousLogs"
PREVIOUS_LOGS_FILE = os.path.join(PREVIOUS_LOGS_DIR, "backup_sync_test.txt")
PREVIOUS_LOGS_CONTENT = "backup_sync_test_marker_value_12345"

# ── Persistence paths (must match source/utils/persistence.h) ────────
SEEKFOLDER = "/opt/.t2seekmap"
REPORTPROFILES_PATH = "/opt/.t2reportprofiles"
MSGPACK_FILE = "profiles.msgpack"
T2_BOOTFLAG = "/tmp/.t2bootup"

# ── Profile that greps a PreviousLogs file ───────────────────────────
# GenerateNow must be false for the checkPreviousSeek path to activate
# when the daemon loads this profile from disk at startup.
_PROFILE_BACKUP_SYNC = json.dumps({
    "profiles": [{
        "name": "BACKUP_SYNC_TEST",
        "hash": "hash_backup_sync_001",
        "value": {
            "Name": "BACKUP_SYNC_TEST",
            "Description": "L2 test for backup_logs synchronisation gate",
            "Version": "0.1",
            "Protocol": "HTTP",
            "EncodingType": "JSON",
            "ActivationTimeOut": 180,
            "ReportingInterval": 300,
            "RootName": "backup_sync_report",
            "Parameter": [
                {
                    "type": "grep",
                    "marker": "PREVLOG_SYNC_MARKER",
                    "search": "backup_sync_test_marker_value",
                    "logFile": "backup_sync_test.txt",
                    "use": "absolute"
                }
            ]
        }
    }]
})

LOG_PROFILE_ENABLE = "Successfully enabled profile :"

def _tomsgpack(json_string):
    data = json.loads(json_string)
    return base64.b64encode(msgpack.packb(data)).decode("utf-8")

def _clean_sentinels():
    """Remove both sentinel files so each test starts clean."""
    for path in (BACKUP_LOGS_DONE_FLAG, TELEMETRY_PREVLOGS_DONE_FLAG):
        try:
            os.remove(path)
        except FileNotFoundError:
            pass

def _create_previous_logs():
    """Populate the PreviousLogs directory with a known test file."""
    os.makedirs(PREVIOUS_LOGS_DIR, exist_ok=True)
    with open(PREVIOUS_LOGS_FILE, "w") as f:
        f.write(PREVIOUS_LOGS_CONTENT + "\n")
        f.write("second line in backup sync test log\n")

def _seed_persistence():
    """Create persistence state (seek config + profile on disk) so the
    daemon loads the profile with checkPreviousSeek=true on next startup.

    Requires:
    * SEEKFOLDER with a valid seek config for the profile name.
    * Profile msgpack saved to REPORTPROFILES_PATH.
    * BOOTFLAG absent so firstBootStatus() returns true.
    """
    os.makedirs(SEEKFOLDER, exist_ok=True)
    seek_config = json.dumps([{"backup_sync_test.txt": 0}])
    with open(os.path.join(SEEKFOLDER, "BACKUP_SYNC_TEST"), "w") as f:
        f.write(seek_config)

    os.makedirs(REPORTPROFILES_PATH, exist_ok=True)
    data = json.loads(_PROFILE_BACKUP_SYNC)
    raw_msgpack = msgpack.packb(data)
    with open(os.path.join(REPORTPROFILES_PATH, MSGPACK_FILE), "wb") as f:
        f.write(raw_msgpack)

    try:
        os.remove(T2_BOOTFLAG)
    except FileNotFoundError:
        pass

def _full_reset():
    """Kill daemon, remove flags/sentinels, recreate PreviousLogs."""
    kill_telemetry(9)
    sleep(1)
    remove_T2bootup_flag()
    clear_persistant_files()
    _clean_sentinels()
    _create_previous_logs()
    clear_T2logs()
    try:
        os.remove(T2_BOOTFLAG)
    except FileNotFoundError:
        pass


# ── Test 1: fast path – sentinel already present ─────────────────────
@pytest.mark.run(order=1)
def test_backup_sync_sentinel_preexists():
    """When /tmp/.backup_logs_done exists before PreviousLogs grep,
    the daemon should log 'already present' and harvest data immediately."""
    _full_reset()

    # Pre-create the sentinel BEFORE starting the daemon
    open(BACKUP_LOGS_DONE_FLAG, "w").close()

    # Seed persistence so profile loads from disk with checkPreviousSeek=true.
    # On startup the daemon will call NotifyTimeout which triggers a report
    # with customLogPath=PREVIOUS_LOGS_PATH, invoking waitForBackupLogsDone().
    _seed_persistence()

    run_telemetry()
    sleep(5)

    # Verify fast-path log message
    assert "already present" in grep_T2logs("backup_logs sentinel")

    # Verify PreviousLogs data was harvested in the report
    assert "PREVLOG_SYNC_MARKER" in grep_T2logs("cJSON Report")

    # Verify telemetry completion sentinel was created
    assert os.path.exists(TELEMETRY_PREVLOGS_DONE_FLAG), \
        "Telemetry previous-logs completion sentinel not created"


# ── Test 2: inotify path – sentinel arrives during wait ──────────────
@pytest.mark.run(order=2)
def test_backup_sync_sentinel_via_inotify():
    """When the sentinel is absent at grep time but appears within the
    timeout window, inotify should detect it and the daemon proceeds."""
    _full_reset()

    # Seed persistence so profile loads from disk with checkPreviousSeek=true.
    # The daemon will call waitForBackupLogsDone() immediately at startup;
    # since the sentinel doesn't exist yet it enters the inotify wait loop.
    _seed_persistence()

    # Create the sentinel after a short delay (while daemon is waiting)
    def _create_sentinel():
        sleep(5)
        open(BACKUP_LOGS_DONE_FLAG, "w").close()

    t = threading.Thread(target=_create_sentinel, daemon=True)
    t.start()

    run_telemetry()
    sleep(15)  # Allow time for daemon start + inotify detection + grep + report

    # Verify inotify detection log (either "sentinel created" or "race resolved")
    inotify_log = grep_T2logs("backup_logs sentinel")
    assert ("sentinel created" in inotify_log or
            "race resolved" in inotify_log or
            "already present" in inotify_log), \
        f"Expected inotify sentinel detection log, got: {inotify_log}"

    # Verify PreviousLogs data was harvested
    assert "PREVLOG_SYNC_MARKER" in grep_T2logs("cJSON Report")

    # Verify telemetry completion sentinel was created
    assert os.path.exists(TELEMETRY_PREVLOGS_DONE_FLAG), \
        "Telemetry previous-logs completion sentinel not created"


# ── Test 3: timeout path – sentinel never appears ────────────────────
@pytest.mark.run(order=3)
def test_backup_sync_timeout():
    """When the sentinel never appears the daemon should time out after
    BACKUP_LOGS_SYNC_TIMEOUT_S (60 s) and proceed with the grep anyway."""
    _full_reset()

    # Do NOT create the sentinel — force the timeout path.
    # Seed persistence so profile loads from disk with checkPreviousSeek=true.
    _seed_persistence()

    run_telemetry()

    # Wait for the 60 s timeout + report generation
    sleep(70)

    # Verify timeout warning was logged
    assert "not present after" in grep_T2logs("backup_logs sentinel") or \
           "proceeding without" in grep_T2logs("backup_logs"), \
        "Expected timeout/proceeding-without log message"

    # Verify PreviousLogs data was still harvested (best-effort)
    assert "PREVLOG_SYNC_MARKER" in grep_T2logs("cJSON Report")

    # Verify telemetry completion sentinel was created even after timeout
    assert os.path.exists(TELEMETRY_PREVLOGS_DONE_FLAG), \
        "Telemetry previous-logs completion sentinel not created after timeout"
