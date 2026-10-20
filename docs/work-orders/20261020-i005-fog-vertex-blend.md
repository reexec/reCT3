# 작업 지시: 포그와 정점 블렌딩 (issue #5)

설계: rexbox [20261020-i007-fog-vertex-blend](../../third_party/rexbox/docs/design/20261020-i007-fog-vertex-blend.md)(런타임 작업은 rexbox issue #7, 브랜치 `i007-fog-vertex-blend`)

1. 작업 브랜치 `i005-fog-vertex-blend`. 커밋 제목 끝에 `(#5)`.
2. 측정: CT3가 쓰는 포그 상태와 스킨 모드를 분석 문서 §4에 적는다(설계 §2의 값).
3. rexbox 구현이 들어가면 `third_party/rexbox`를 그 커밋으로 올리고, 재생성 없이 빌드한다(rexbox만 바뀜).
4. 검증: 데모 프레임(1900, 2100 무렵)을 바꾸기 전후로 덤프해 비교하고, probe와 CTest를 돌린다. 포그는 데모에서 계수 1이라 화면 변화가 없어야 한다.
5. 작업 로그 `docs/work-logs/20261020-i005-fog-vertex-blend.md`.

---

# Work order: fog and vertex blending (issue #5)

Design: rexbox [20261020-i007-fog-vertex-blend](../../third_party/rexbox/docs/design/20261020-i007-fog-vertex-blend.md) (the runtime work is rexbox issue #7 on branch `i007-fog-vertex-blend`).

(1) Task branch `i005-fog-vertex-blend`; commit titles end with `(#5)`. (2) Measurements: the fog states and skin modes CT3 uses go into analysis section 4 (the values of design section 2). (3) Once the rexbox implementation is in, move `third_party/rexbox` to that commit and build without regenerating (only rexbox changes). (4) Verify: dump demo frames (around 1900 and 2100) before and after and compare them, and run the probes and CTest; fog has factor 1 in the demo, so the picture must not change for it. (5) Work log `docs/work-logs/20261020-i005-fog-vertex-blend.md`.
