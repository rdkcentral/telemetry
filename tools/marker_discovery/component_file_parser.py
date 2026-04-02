"""Parser for component version input files.

Parses version manifest files that list repo URLs, branches, and commit SHAs.
Expected format (one per line):
    https://github.com/rdkcentral/telemetry@develop : a1b2c3d4...
    b'https://github.com/rdkcentral/rbus.git'@develop : e5f6a7b8...
    ssh://github.com/rdk-e/rdkservices-cpc@ : 1dff01bd...
    b'ssh://git@github.com/rdk-e/meta-rdk-tools'@sha : sha

Only GitHub repos belonging to the specified orgs are included.
Non-GitHub URLs (gerrit, kernel.org, tarballs, etc.) are skipped.
SSH URLs are converted to HTTPS clone URLs for compatibility with credential helpers.
"""

import logging
import re

logger = logging.getLogger(__name__)

# Known orgs to filter for
DEFAULT_FILTER_ORGS = {"rdkcentral", "rdk-e", "rdk-common", "rdk-gdcs"}

# Matches GitHub URLs in various formats found in version files:
#   https://github.com/org/repo@ref : sha
#   https://github.com/org/repo.git@ref : sha
#   b'https://github.com/org/repo'@sha : sha
#   ssh://github.com/org/repo@ref : sha
#   ssh://git@github.com/org/repo.git@ : sha
#   b'ssh://git@github.com/org/repo'@sha : sha
_GITHUB_LINE_PATTERN = re.compile(
    r"^(?:b')?"                          # optional b' prefix
    r"(?:"
        r"https?://github\.com/"         # HTTPS URL
        r"|"
        r"ssh://(?:git@)?github\.com/"   # SSH URL (with optional git@ user)
    r")"
    r"([^/]+)/"                          # org
    r"([^@'.\s]+)"                       # repo name
    r"(?:\.git)?"                        # optional .git suffix
    r"'?"                                # optional closing quote
    r"@(\S*)"                            # @ref (branch/tag/sha, may be empty)
    r"\s*:\s*"                           # separator
    r"(\S+)"                             # commit SHA or checksum
)


def parse_component_file(file_path, filter_orgs=None):
    """Parse a version manifest file and return list of component entries.

    Only includes repos from GitHub belonging to the specified orgs.

    Args:
        file_path: Path to the versions file.
        filter_orgs: Set of org names to include (default: 4 RDK orgs).

    Returns:
        List of dicts: {
            'name': str,       # repo name
            'org': str,        # GitHub org
            'commit': str,     # exact commit SHA
            'branch': str,     # branch/ref from the file (may be empty)
            'url': str,        # clone URL
        }
    """
    if filter_orgs is None:
        filter_orgs = DEFAULT_FILTER_ORGS

    components = []
    skipped = 0

    with open(file_path, 'r') as f:
        for line in f:
            line = line.strip()
            if not line:
                continue

            match = _GITHUB_LINE_PATTERN.match(line)
            if not match:
                skipped += 1
                continue

            org = match.group(1)
            repo = match.group(2)
            ref = match.group(3)
            commit = match.group(4)

            # Only include repos from our target orgs
            if org not in filter_orgs:
                skipped += 1
                continue

            # Skip entries that are tarball downloads (sha is md5sum, not a commit)
            if commit.startswith("md5sum"):
                skipped += 1
                continue

            components.append({
                'name': repo,
                'org': org,
                'commit': commit,
                'branch': ref if ref else '',
                'url': f"https://github.com/{org}/{repo}.git",
            })

    logger.info("Parsed %d components from %s (%d non-GitHub/non-RDK lines skipped)",
                len(components), file_path, skipped)
    return components
