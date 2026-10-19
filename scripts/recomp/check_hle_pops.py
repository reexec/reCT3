#!/usr/bin/env python3
"""Compare the HLE exports' pop counts with the lifted CT3 originals (rexbox's script with this title's paths).

    python scripts/recomp/check_hle_pops.py          # exit 1 on any mismatch
    python scripts/recomp/check_hle_pops.py --all    # also list the matches
"""

import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
SCRIPT = ROOT / "third_party" / "rexbox" / "scripts" / "recomp" / "check_hle_pops.py"

if __name__ == "__main__":
    sys.exit(subprocess.call([sys.executable, str(SCRIPT),
                              "--gen", str(ROOT / "gen" / "ct3_jp"),
                              "--extra-symbols", str(ROOT / "src" / "game" / "ct3" / "recomp" / "xdk_symbols_extra.json")]
                             + sys.argv[1:]))
