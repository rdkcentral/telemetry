"""Allow running as: python3 -m tools.marker_discovery [--org ORG] [--branch BRANCH] [--output FILE]"""

import importlib
import subprocess
import sys

_REQUIRED = ["tree_sitter", "tree_sitter_c", "requests"]


def _ensure_deps():
    missing = []
    for mod in _REQUIRED:
        try:
            importlib.import_module(mod)
        except ImportError:
            missing.append(mod)
    if missing:
        req = str(__import__("pathlib").Path(__file__).parent / "requirements.txt")
        print(f"Installing missing dependencies: {', '.join(missing)}", file=sys.stderr)
        subprocess.check_call([sys.executable, "-m", "pip", "install", "-r", req])


_ensure_deps()

from .marker_scanner import main

sys.exit(main())
