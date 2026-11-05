# 작업 지시: ADXT 정지와 파일 서버 사이의 스트림 필드 경쟁 (issue #44)

설계: [20261106-i044](../design/20261106-i044-adxt-stop-lock.md)

1. 브랜치 `i044-adxt-stop-lock`(rexbox v0.0.23 커밋 `7211b8e` 위). 커밋 제목 끝에 `(#44)`.
2. 1단계: `sub_000B1700`과 `sub_000B2730`에 임시 계측(설계 §4)을 넣고, 숨긴 데모 묶음으로 원인을 확인한다. 계측은 커밋하지 않는다.
3. 2단계: `src/game/ct3/decomp/adxm_lock.c`에 `sub_000B1700`을 둔다(설계 §4). `decomp/README.md`, ARCHITECTURE의 `rect3_game` 행, 분석 `docs/analysis/cri-middleware.md`(정지 경로, §6 미해결)도 고친다.
4. 재생성, Win32 x86 Release 빌드, CTest.
5. 검증: 최종 빌드로 숨긴 데모 4묶음 이상, 운전 진입.
6. 작업 로그, 릴리스 노트.

---

# Work order: the stream-field race between ADXT's stop and the file server (issue #44)

Design: [20261106-i044](../design/20261106-i044-adxt-stop-lock.md)

(1) Branch `i044-adxt-stop-lock`, on the rexbox v0.0.23 commit `7211b8e`; commit titles end with `(#44)`. (2) Step 1: temporary instrumentation in `sub_000B1700` and `sub_000B2730` (design section 4); confirm the cause with batches of hidden demos; do not commit the instrumentation. (3) Step 2: `sub_000B1700` in `src/game/ct3/decomp/adxm_lock.c` (design section 4); `decomp/README.md`, ARCHITECTURE's `rect3_game` row, the analysis `docs/analysis/cri-middleware.md` (the stop path, section 6 Unresolved). (4) Regeneration, Win32 x86 Release build, CTest. (5) Verify: at least 4 batches of hidden demos with the final build, a drive into gameplay. (6) Work log, release notes.
