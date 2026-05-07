"""Script file scanner for t2ValNotify and t2CountNotify marker calls."""

import logging
import os
import re

from . import MarkerRecord

logger = logging.getLogger(__name__)

# Extensions to exclude (handled by code_parser or patch_parser)
EXCLUDE_EXTENSIONS = {".c", ".cpp", ".h", ".patch", ".o", ".so", ".a", ".bin",
                      ".png", ".jpg", ".jpeg", ".gif", ".ico", ".pdf",
                      ".tar", ".gz", ".zip", ".bz2", ".xz"}

# Patterns for script-level telemetry calls
# Matches: t2ValNotify "MARKER_NAME" ...
#          t2CountNotify "MARKER_NAME" ...
SCRIPT_PATTERNS = [
    (re.compile(r't2ValNotify\s+"([^"]+)"'), "t2ValNotify"),
    (re.compile(r't2CountNotify\s+"([^"]+)"'), "t2CountNotify"),
]

# Pattern to detect shell variables in marker names
SHELL_VAR_PATTERN = re.compile(r'\$\{?\w+\}?')

# Pattern to detect markers that are ENTIRELY a positional arg (e.g. "$1", "$2")
PURE_POSITIONAL_PATTERN = re.compile(r'^\$\d+$')

# Directory names to skip during scanning
SKIP_DIRS = {"test", "tests", ".git"}


def _is_dynamic_marker(marker_name):
    """Check if a marker name contains shell variables."""
    return bool(SHELL_VAR_PATTERN.search(marker_name))


def _is_script_file(file_path):
    """Check if a file should be scanned as a script (not C/C++, not binary)."""
    ext = os.path.splitext(file_path)[1].lower()
    if ext in EXCLUDE_EXTENSIONS:
        return False
    # Skip files without extensions that look binary
    try:
        with open(file_path, "rb") as f:
            chunk = f.read(512)
            if b"\x00" in chunk:
                return False
    except (IOError, OSError):
        return False
    return True


def _resolve_function_calls(file_content, func_name, arg_position):
    """Try to resolve a shell function's argument by finding its call sites in the same file.

    Looks for: func_name "LITERAL" ... where LITERAL is at arg_position (0-based).
    Returns list of resolved string literals.
    """
    resolved = []
    # Pattern: function_name followed by arguments
    # Shell function call: func_name arg1 arg2 ...
    # Arguments can be quoted or unquoted
    pattern = re.compile(
        r'(?:^|\s)' + re.escape(func_name) + r'\s+(.*)', re.MULTILINE
    )
    for match in pattern.finditer(file_content):
        args_str = match.group(1).strip()
        # Parse arguments (handle quoted strings)
        args = []
        i = 0
        while i < len(args_str):
            if args_str[i] == '"':
                # Quoted argument
                end = args_str.find('"', i + 1)
                if end != -1:
                    args.append(args_str[i + 1:end])
                    i = end + 1
                else:
                    break
            elif args_str[i] in (' ', '\t'):
                i += 1
                continue
            else:
                # Unquoted argument
                end = i
                while end < len(args_str) and args_str[end] not in (' ', '\t', ';', '#', '\n'):
                    end += 1
                args.append(args_str[i:end])
                i = end

        if len(args) > arg_position:
            val = args[arg_position]
            # Only accept if it looks like a static marker (no variables)
            if not _is_dynamic_marker(val):
                resolved.append(val)

    return resolved


def _find_shell_function_for_line(file_content, line_num):
    """Find what shell function a given line belongs to.

    Returns (function_name, positional_arg_index) if the marker at line_num
    uses a positional parameter like $1, $2, or None if not in a function.
    """
    lines = file_content.split('\n')
    if line_num < 1 or line_num > len(lines):
        return None

    # Walk backwards to find function definition
    func_pattern = re.compile(r'^\s*(?:function\s+)?(\w+)\s*\(\s*\)')
    for i in range(line_num - 1, -1, -1):
        line = lines[i]
        m = func_pattern.match(line)
        if m:
            return m.group(1)
        # If we hit another closing brace at column 0, we've left the function
        if line.strip() == '}' and i < line_num - 1:
            return None

    return None


def scan_repo_scripts(repo_path, repo_name):
    """Scan all script files in a repo for t2ValNotify/t2CountNotify calls.

    Returns list of MarkerRecord with source_type="script" or "script_dynamic".
    """
    markers = []

    for root, dirs, files in os.walk(repo_path):
        # Skip test and .git directories
        dirs[:] = [d for d in dirs if d.lower() not in SKIP_DIRS]
        for fname in files:
            file_path = os.path.join(root, fname)
            if not _is_script_file(file_path):
                continue

            rel_path = os.path.relpath(file_path, repo_path)

            try:
                with open(file_path, "r", errors="ignore") as f:
                    file_content = f.read()

                for line_num, line in enumerate(file_content.split('\n'), start=1):
                    for pattern, api_name in SCRIPT_PATTERNS:
                        for match in pattern.finditer(line):
                            marker_name = match.group(1)
                            is_dynamic = _is_dynamic_marker(marker_name)

                            if is_dynamic and PURE_POSITIONAL_PATTERN.match(marker_name):
                                # Pure positional arg (e.g. "$1") — try to resolve
                                # by finding the enclosing function and its call sites
                                arg_idx = int(marker_name[1:]) - 1  # $1 → index 0
                                func_name = _find_shell_function_for_line(file_content, line_num)
                                if func_name:
                                    resolved = _resolve_function_calls(file_content, func_name, arg_idx)
                                    if resolved:
                                        for resolved_name in resolved:
                                            markers.append(MarkerRecord(
                                                marker_name=resolved_name,
                                                component=repo_name,
                                                file_path=rel_path,
                                                line=line_num,
                                                api=f"{func_name}→{api_name}",
                                                source_type="script",
                                            ))
                                        continue  # skip adding the unresolved $1 entry

                            source_type = "script_dynamic" if is_dynamic else "script"
                            markers.append(MarkerRecord(
                                marker_name=marker_name,
                                component=repo_name,
                                file_path=rel_path,
                                line=line_num,
                                api=api_name,
                                source_type=source_type,
                            ))
            except (IOError, OSError) as e:
                logger.warning("Could not read %s: %s", file_path, e)

    logger.info("Found %d script markers in %s", len(markers), repo_name)
    return markers