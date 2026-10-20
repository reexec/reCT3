#!/usr/bin/env python3
"""List CT3 functions reached only through pointers and not lifted (rexbox's script with this title's paths).

    python scripts/recomp/find_pointer_targets.py            # list candidates
    python scripts/recomp/find_pointer_targets.py --write    # also write the seed file, then regenerate
"""

import pathlib
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "third_party" / "rexbox" / "scripts" / "recomp"))
import find_pointer_targets  # noqa: E402  (rexbox's)

TITLE = ROOT / "src" / "game" / "ct3"

if __name__ == "__main__":
    sys.exit(find_pointer_targets.run(
        xbe=ROOT / "roms" / "default.xbe",
        variant="ct3_jp",
        variants=TITLE / "recomp" / "variants.json",
        dispatch=ROOT / "gen" / "ct3_jp" / "gen" / "recomp_dispatch.c",
        seed_file=TITLE / "recomp" / "seed_pointer_targets.json",
    ))
