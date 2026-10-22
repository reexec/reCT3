# 작업 지시: 운전 화면 — D3D8 그리기 하나의 CPU 비용 줄이기 (issue #20)

설계: rexbox [20261021-i023-draw-cpu-cost](../../third_party/rexbox/docs/design/20261021-i023-draw-cpu-cost.md)(런타임 작업은 rexbox issue #23, 브랜치 `i023-draw-cpu-cost`) · 앞 작업: #17

1. 작업 브랜치 `i020-draw-cpu-cost`. 커밋 제목 끝에 `(#20)`. 커밋마다 push한다. 머지는 사용자가 요청할 때만 한다.
2. rexbox의 단계가 커밋될 때마다 `third_party/rexbox`를 그 커밋으로 올리고 재생성 없이 빌드한다.
3. 측정: [가이드](../guides/driving-measurement.md)대로 기준 빌드(v0.0.9 exe 사본)와 새 빌드를 번갈아 바로 이어 실행하고(다른 빌드가 돌고 있으면 기다린다), 운전 구간(draws 약 800)의 `REXBOX_D3D8_PROFILE` fps, CPU, 단계 시간을 비교한다.
4. 그림: 어트랙트 데모의 같은 프레임(`REXBOX_FRAME_DUMP=1500,2100`)을 전후 비교한다.
5. 분석 문서 §7, 작업 로그 `docs/work-logs/20261022-i020-draw-cpu-cost.md`.

---

# Work order: driving — cutting the CPU cost of one D3D8 draw (issue #20)

Design: rexbox [20261021-i023-draw-cpu-cost](../../third_party/rexbox/docs/design/20261021-i023-draw-cpu-cost.md) (the runtime work is rexbox issue #23 on branch `i023-draw-cpu-cost`) · Previous task: #17

(1) Task branch `i020-draw-cpu-cost`; commit titles end with `(#20)`; every commit is pushed; merging only when the user asks. (2) Each time a rexbox step is committed, move `third_party/rexbox` to it and build without regenerating. (3) Measure: following the [guide](../guides/driving-measurement.md), run the baseline (a copy of the v0.0.9 exe) and the new build alternately back to back (waiting while another build runs) and compare `REXBOX_D3D8_PROFILE` fps, CPU and phase times over the driving span (about 800 draws). (4) Pictures: compare the same attract-demo frames (`REXBOX_FRAME_DUMP=1500,2100`) before and after. (5) Analysis section 7 and the work log `docs/work-logs/20261022-i020-draw-cpu-cost.md`.
