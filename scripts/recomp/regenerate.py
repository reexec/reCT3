#!/usr/bin/env python3
"""Regenerate the lifted C for the Crazy Taxi 3 Japan build (gen/ct3_jp/).

A thin wrapper over this project's pipeline (lifter.py: XDK identification,
parse, disasm, func_id, abi, HLE and decompiled overrides, lift) that fills
in this title's paths: the XBE under roms/, the recomp configuration in
src/game/ct3/recomp/, the decompiled sources in src/game/ct3/decomp/ and
this project's own xboxrecomp pin (third_party/xboxrecomp). Any of them can
still be given on the command line (--help).

    python scripts/recomp/regenerate.py
    python scripts/recomp/regenerate.py --xbe <path> --variant ct3_jp
"""

import os
import pathlib
import sys

import lifter

ROOT = lifter.ROOT
TITLE = ROOT / "src" / "game" / "ct3"
# Linux trees: build/<preset>, or $RECT3_BUILD_DIR/<preset> (scripts/build_linux.sh).
LINUX_TREES = [pathlib.Path(base) / f"linux-{arch}-{config}"
               for base in filter(None, [os.environ.get("RECT3_BUILD_DIR"), ROOT / "build"])
               for arch in ("x64", "x86") for config in ("release", "debug")]

if __name__ == "__main__":
    sys.exit(lifter.run(
        xbe=ROOT / "roms" / "default.xbe",
        variant="ct3_jp",
        title_dir=TITLE / "recomp",
        override_dirs=[TITLE / "decomp"],
        gen_root=ROOT / "gen",
        xboxrecomp=ROOT / "third_party" / "xboxrecomp",
        symbol_tool_candidates=[
            ROOT / "build" / "win32_x86" / "third_party" / "rexbox" / "tools" / "Release" / "rexbox_xdk_symbols.exe",
            ROOT / "build" / "win32_x86" / "third_party" / "rexbox" / "tools" / "Debug" / "rexbox_xdk_symbols.exe",
            ROOT / "build" / "third_party" / "rexbox" / "tools" / "rexbox_xdk_symbols",
        ] + [tree / "third_party" / "rexbox" / "tools" / "rexbox_xdk_symbols" for tree in LINUX_TREES],
        root=ROOT,
    ))
