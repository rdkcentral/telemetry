"""Allow running as: python3 -m tools.marker_discovery [--org ORG] [--branch BRANCH] [--output FILE]"""

import sys
from .marker_scanner import main

sys.exit(main())
