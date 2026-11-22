# 작업 지시: CRI safe thread 핸드셰이크와 ADXM 호스트 잠금의 교착 (issue #52)

설계: [20261123-i052](../design/20261123-i052-safe-thread-handshake.md)

작업 브랜치 `i052-safe-thread-handshake`, 커밋 제목 끝에 `(#52)`. 역할: 총괄·설계·리뷰·측정은 Claude, 구현은 Antigravity.

1. rexbox#87(rexbox 브랜치 `i087-guest-wait-c`): `rexbox_guest_wait_begin`/`end`와 kernel probe 확인.
2. `src/game/ct3/decomp/adxm_lock.c`: `sub_000B3060` 어댑터(설계 §4). 파일 머리 주석, `decomp/README.md`, ARCHITECTURE의 `rect3_game` 행, 분석 `docs/analysis/cri-middleware.md`(safe thread 핸드셰이크).
3. 재생성, Win32 x86 Release 빌드, CTest.
4. 검증(설계 §5, Claude): 숨긴 데모 8묶음 이상, 주행.
5. rexbox#87 머지 뒤 rexbox 고정을 올리고(Claude), 작업 로그와 릴리스 노트.

---

# Work order: the deadlock between CRI's safe-thread handshake and the ADXM host lock (issue #52)

Design: [20261123-i052](../design/20261123-i052-safe-thread-handshake.md)

Task branch `i052-safe-thread-handshake`; commit titles end with `(#52)`. Roles: Claude leads, designs, reviews and measures; Antigravity implements. (1) rexbox#87 (rexbox branch `i087-guest-wait-c`): `rexbox_guest_wait_begin`/`end` and a kernel probe check. (2) `src/game/ct3/decomp/adxm_lock.c`: the `sub_000B3060` adapter (design section 4); the file's header comment, `decomp/README.md`, ARCHITECTURE's `rect3_game` row, the analysis `docs/analysis/cri-middleware.md` (the safe-thread handshake). (3) Regeneration, Win32 x86 Release build, CTest. (4) Verification (design section 5, Claude): at least 8 hidden demo batches, a drive. (5) After rexbox#87 is merged, move the rexbox pin (Claude); work log and release notes.
