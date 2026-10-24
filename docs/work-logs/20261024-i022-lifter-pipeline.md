# 작업 로그: lifter 파이프라인을 이 저장소로 옮김 (issue #22)

작업 지시: [20261024-i022-lifter-pipeline](../work-orders/20261024-i022-lifter-pipeline.md) · 설계: [20261024-i022-lifter-pipeline](../design/20261024-i022-lifter-pipeline.md) · rexbox: issue #26([설계](../../third_party/rexbox/docs/design/20261024-i026-drop-xboxrecomp.md))

## 한 일

1. `third_party/rexbox`를 rexbox #26 커밋(`bac8e80`, 브랜치 `i026-drop-xboxrecomp`)으로 올렸다.
2. `scripts/recomp/lifter.py`를 새로 만들었다. xboxrecomp 파이프라인(parse, disasm, func_id, abi, lift; `third_party/xboxrecomp`, upstream v0.15.0), 그 출력 읽기(`disasm/functions.json`, `/* ret N */`, `recomp_dispatch.c`), 이름 규칙(`sub_%08X`, `sub_%08X_gen`)을 가진다. rexbox의 `hle_overrides`, `xbe_variant`를 import한다.
3. `regenerate.py`는 `lifter.run`을 부른다. `check_hle_pops.py`, `find_pointer_targets.py`는 `lifter.py`와 rexbox의 `hle_pops`·`pointer_targets`·`xbe_variant`를 쓴다. `register_xbe.py`는 그대로다.
4. `CMakeLists.txt`: `REXBOX_XBOXRECOMP_DIR`를 지웠다. `third_party/xboxrecomp` 존재 검사는 경로로 남겼다. `rexbox_check_license` 호출을 지웠다.
5. 라이선스: 타이틀 쪽 가드 코드를 두지 않고 `AGENTS.md` 규칙(xboxrecomp의 LGPL 파일과 `include/xbox/xboxrecomp.h`를 소스 목록에 넣지 않는다)으로 바꿨다.
6. 저장소 사고: rexbox 쪽에서 rexbox의 `third_party/xboxrecomp` 체크아웃을 지우자 이 저장소의 `.git/modules/third_party/xboxrecomp`가 깨졌다. `objects/info/alternates`로 그 체크아웃의 객체를 빌려 쓰고 있었다. 복구: alternates를 잠시 같은 커밋(`6376eb8`)을 가진 로컬 clone `E:/MYWORK/Projects/xboxrecomp`로 돌리고 `git repack -a -d` 뒤 alternates를 지웠다. `git fsck`는 깨끗하다. rexbox submodule 자신의 alternates(`E:/MYWORK/Projects/rexbox/.git/objects`)는 남아 있다.

## 검증

| 항목 | 결과 |
|---|---|
| `ct3_jp` 재생성, 바꾸기 전 기준(같은 xboxrecomp, 같은 XBE)과 비교 | 45개 파일 바이트 단위로 같음. 5개는 예상한 차이뿐: `hle_overrides.c`와 `gen/rexbox_hle_overrides.c`의 머리 주석, `overrides.c`의 머리 주석과 들어 있는 경로, `recomp-summary/summary.json`의 출력 루트 경로, `disasm/.disasm_cache.json`의 시각과 경과 시간. lift된 C는 같음 |
| `check_hle_pops --all`, `find_pointer_targets` | 출력이 기준과 바이트 단위로 같음(불일치 0, 후보 6) |
| Win32 x86 Release 빌드 | `recomp_abi.c`가 `rect3_recomp_abi`에서 upstream v0.15.0 헤더로 `/W4` 경고 없이 컴파일됨. `rect3.exe` 링크됨 |
| `scripts/drive_west_coast.ps1`(100 s, `REXBOX_DISPLAY=hidden`, `REXBOX_D3D8_PROFILE=120`) | West Coast 운전에 도달. 프레임당 draws 약 790, 59 fps. 멈춤, fault, ABI 보고 없음 |

* 사용자는 재생성할 필요가 없다.

## 남은 것

* rexbox #26은 v0.0.11로 머지되었고, 생성 코드 없는 트리(이 저장소의 CI)에서 앱이 링크되지 않던 그 문제를 v0.0.12(`a018038`)가 고쳤다. `third_party/rexbox`를 v0.0.12의 머지 커밋으로 올렸다. ARCHITECTURE와 THIRD_PARTY_NOTICES의 rexbox 버전 표기도 맞췄다.

---

# Work log: moving the lifter pipeline into this repository (issue #22)

Work order: [20261024-i022-lifter-pipeline](../work-orders/20261024-i022-lifter-pipeline.md) · Design: [20261024-i022-lifter-pipeline](../design/20261024-i022-lifter-pipeline.md) · rexbox: issue #26 ([design](../../third_party/rexbox/docs/design/20261024-i026-drop-xboxrecomp.md))

Done:

1. `third_party/rexbox` moved to the rexbox #26 commit (`bac8e80`, branch `i026-drop-xboxrecomp`).
2. New `scripts/recomp/lifter.py`: the xboxrecomp pipeline (parse, disasm, func_id, abi, lift; `third_party/xboxrecomp`, upstream v0.15.0), the readers of its output (`disasm/functions.json`, `/* ret N */`, `recomp_dispatch.c`) and its naming (`sub_%08X`, `sub_%08X_gen`); it imports rexbox's `hle_overrides` and `xbe_variant`.
3. `regenerate.py` calls `lifter.run`; `check_hle_pops.py` and `find_pointer_targets.py` use `lifter.py` and rexbox's `hle_pops`, `pointer_targets` and `xbe_variant`; `register_xbe.py` is unchanged.
4. `CMakeLists.txt`: `REXBOX_XBOXRECOMP_DIR` removed; the existence check of `third_party/xboxrecomp` stays, by path; the `rexbox_check_license` call removed.
5. License: no title-side guard code; the `AGENTS.md` rule now says xboxrecomp's LGPL files and `include/xbox/xboxrecomp.h` are never added to a source list.
6. Repository incident: deleting rexbox's own `third_party/xboxrecomp` checkout on the rexbox side broke this repository's `.git/modules/third_party/xboxrecomp`, which borrowed objects from it through `objects/info/alternates`. Repair: alternates pointed for a moment at the local clone `E:/MYWORK/Projects/xboxrecomp` (same commit `6376eb8`), `git repack -a -d`, alternates removed; `git fsck` is clean. The rexbox submodule's own alternates (`E:/MYWORK/Projects/rexbox/.git/objects`) remain.

Verification:

| Item | Result |
|---|---|
| `ct3_jp` regenerated, compared with a baseline made before the change (same xboxrecomp, same XBE) | 45 files byte-identical; 5 differ only as expected: header comments of `hle_overrides.c` and `gen/rexbox_hle_overrides.c`, the header and embedded paths of `overrides.c`, the output-root paths in `recomp-summary/summary.json`, the timestamp and elapsed time in `disasm/.disasm_cache.json`. The lifted C is identical |
| `check_hle_pops --all`, `find_pointer_targets` | output byte-identical to the baseline (0 mismatches, 6 candidates) |
| Win32 x86 Release build | `recomp_abi.c` compiles in `rect3_recomp_abi` at `/W4` with no warnings against upstream v0.15.0's header; `rect3.exe` links |
| `scripts/drive_west_coast.ps1` (100 s, `REXBOX_DISPLAY=hidden`, `REXBOX_D3D8_PROFILE=120`) | reaches West Coast driving, about 790 draws per frame at 59 fps; no stop, fault or ABI report |

* Users need not regenerate.

Left over:

* rexbox #26 was merged as v0.0.11, and v0.0.12 (`a018038`) fixed its link failure for an application in a tree with no generated code (this repository's CI); `third_party/rexbox` points at v0.0.12's merge commit, and the rexbox version in ARCHITECTURE and THIRD_PARTY_NOTICES matches it.
