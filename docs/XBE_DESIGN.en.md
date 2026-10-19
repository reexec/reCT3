# CT3 XBE Structure and Design (English)

Accumulates the structure confirmed by analysing the original XBE. Detailed evidence and confirmation status live in [`docs/analysis/`](analysis/README.md); only conclusions that affect the design are collected here. Korean version: [XBE_DESIGN.ko.md](XBE_DESIGN.ko.md).

## 1. Target build

* The primary target is the Japan build `default.xbe`. Its identity (title id, certificate time, image time, MD5) is registered with `scripts/recomp/register_xbe.py` once the XBE is in hand. → [ct3-xbe-overview §1](analysis/ct3-xbe-overview.md)
* Another regional build, if found, is recognized only and assumed to share no addresses (JSRF's two regional builds differed in `.text`; **inferred**).

## 2. Memory layout

To be filled once the XBE is read (`rexbox_xbe_info`).

## 3. Conclusions for the HLE design

* Once the XDK version is confirmed, the differences from rexbox's D3D8 HLE (measured against XDK 4134) are recorded here.

## 4. Start sequence and kernel structures

* XAPI's startup code and the main-thread creation order should match JSRF's if the XAPI library is the same (**inferred**); the boot-frontier analysis confirms it.
