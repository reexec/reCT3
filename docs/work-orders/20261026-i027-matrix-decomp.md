# 작업 지시: 운전 화면의 행렬 함수 세 개 디컴파일 (issue #27)

설계: [20261026-i027-matrix-decomp](../design/20261026-i027-matrix-decomp.md)

1. 작업 브랜치 `i027-matrix-decomp`. 커밋 제목 끝에 `(#27)`.
2. 분석: `docs/analysis/d3dx-matrix.md`에 세 함수의 인자, 연산 순서, 부수 효과, x87 정밀도 처리를 적는다.
3. `src/game/ct3/decomp/d3dx_matrix.c`: `sub_000E4B73`, `sub_000E4BB9`, `sub_000E4CB7`(설계 §3).
4. `tools/decomp_check/`(`rect3_decomp_check`, C 보조 파일 포함), `tools/CMakeLists.txt`, 최상위 `CMakeLists.txt`에서 등록, CTest.
5. `scripts/recomp/regenerate.py`로 재생성, Win32 Debug·Release 빌드, 검사, CTest. Linux x64(WSL2)에서도 검사.
6. 측정: 기준 빌드(v0.0.12)와 번갈아 운전 구간(`drive_west_coast.ps1`, profile과 sample), 프레임 덤프 비교.
7. 문서: 작업 로그, ARCHITECTURE(검사 도구), `decomp/README.md`, 가이드(검사 실행), 분석 문서.

---

# Work order: decompile the three matrix functions of the driving frame (issue #27)

Design: [20261026-i027-matrix-decomp](../design/20261026-i027-matrix-decomp.md)

(1) Task branch `i027-matrix-decomp`; commit titles end with `(#27)`. (2) Analysis: `docs/analysis/d3dx-matrix.md` with the three functions' arguments, operation order, side effects and x87 precision handling. (3) `src/game/ct3/decomp/d3dx_matrix.c`: `sub_000E4B73`, `sub_000E4BB9`, `sub_000E4CB7` (design section 3). (4) `tools/decomp_check/` (`rect3_decomp_check` with its C helper), `tools/CMakeLists.txt`, registered from the top-level `CMakeLists.txt`, CTest. (5) Regenerate with `scripts/recomp/regenerate.py`, build Win32 Debug and Release, check, CTest; check on Linux x64 (WSL2) too. (6) Measure: the driving span (`drive_west_coast.ps1`, profile and samples) alternating with the baseline build (v0.0.12), and compare frame dumps. (7) Docs: work log, ARCHITECTURE (the checker), `decomp/README.md`, the guide (running the checker), the analysis document.
