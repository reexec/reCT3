# CT3 XBE Structure and Design (English)

Accumulates the structure confirmed by analysing the original XBE. Detailed evidence and confirmation status live in [`docs/analysis/`](analysis/README.md); only conclusions that affect the design are collected here. Korean version: [XBE_DESIGN.ko.md](XBE_DESIGN.ko.md).

## 1. Target build

* The primary target is the Japan build `default.xbe` (region 2, title id `53450004`, MD5 `18a88739…`, XDK 4531). → [ct3-xbe-overview §1](analysis/ct3-xbe-overview.md)
* Another regional build, if found, is recognized only and assumed to share no addresses (**inferred**).

## 2. Memory layout

The diagram in the Korean version applies: headers at `0x00010000`, `.text` at `0x00011000`, the `PSGSFD*` sections from `0x000EF480`, `D3D` at `0x000F3500`, `XGRPH`/`DSOUND`/`XPP` from `0x00105400`, `.rdata` at `0x0012A260` (kernel thunks, 121 slots), `.data` at `0x001BB660`, `DOLBY` at `0x00351E20`, then `$$XTIMAGE`.

* The image ends at `0x0035B7A0` (base + `0x34B7A0`), inside the 64 MiB of guest RAM.
* The title turns a D3D resource's physical address into a CPU pointer with `| 0x80000000` or `| 0xF0000000`, so reCT3 turns rexbox's physical window on (`GuestMemory::Options::physical_window`, rexbox design i008).

## 3. Conclusions for the HLE design

* XDK 4531: rexbox's D3D8 HLE (render-state numbering, caps, the default-state routine) is measured against 4134 and must take the 4531 layout (rexbox #7); the identified `D3DRS_*` variables show the deferred and complex blocks one slot above 4134's.
* Boot checks the disc through the DVD drive's SCSI pass-through (MODE SENSE page 0x3E) and returns to the dashboard otherwise (answered in rexbox #6).
* DSOUND needs 4531's public functions (`XAudioDownloadEffectsImage`, `IDirectSound_SetMixBinHeadroom`, `IDirectSoundBuffer_SetPitch`, ...; rexbox #9).
* 17 of the 121 kernel imports have no HLE export (analysis §5); they are implemented when the boot reaches them.

## 4. Start sequence and kernel structures

* The entry point `0xE0934` is XAPI's startup (the same shape as JSRF's). The main thread `0xDDF9D` runs the disc check, the drive links (D:, T:, U:, Z: formatted), the cache-file lookups, the D3D device, shaders and textures, then DirectSound. → [ct3-xbe-overview §7](analysis/ct3-xbe-overview.md)
