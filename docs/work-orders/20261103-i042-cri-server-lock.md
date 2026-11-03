# 작업 지시: 로딩 화면 멈춤 — CRI 레벨 2 서버 실행과 SJ 링 리셋 (issue #42)

설계: [20261103-i042](../design/20261103-i042-cri-server-lock.md)

1. 브랜치 `i042-loading-hang`. 커밋 제목 끝에 `(#42)`.
2. `src/game/ct3/decomp/adxm_lock.c`: `sub_000B9540`, `sub_000B7EE0`(설계 §4). `decomp/README.md`, ARCHITECTURE의 `rect3_game` 행, 분석 `docs/analysis/cri-middleware.md`(SJ 리셋, 오류 트랩, 서버와 정지 경로).
3. 재생성, Win32 x86 Release 빌드, CTest.
4. 검증: 숨긴 어트랙트 데모 6개 동시 실행 묶음(계측 빌드와 최종 빌드), 운전 진입.
5. 작업 로그.

---

# Work order: the loading-screen hang — CRI's level-2 server pass and the SJ ring reset (issue #42)

Design: [20261103-i042](../design/20261103-i042-cri-server-lock.md)

(1) Branch `i042-loading-hang`; commit titles end with `(#42)`. (2) `src/game/ct3/decomp/adxm_lock.c`: `sub_000B9540` and `sub_000B7EE0` (design section 4); `decomp/README.md`, ARCHITECTURE's `rect3_game` row, the analysis `docs/analysis/cri-middleware.md` (the SJ reset, the error traps, the server and stop paths). (3) Regeneration, Win32 x86 Release build, CTest. (4) Verify: batches of six hidden attract demos in parallel (instrumented and final builds), a drive into gameplay. (5) Work log.
