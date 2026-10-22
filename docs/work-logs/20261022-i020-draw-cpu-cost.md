# 작업 로그: 운전 화면 — D3D8 그리기 하나의 CPU 비용 — 1차 (issue #20)

작업 지시: [20261022-i020-draw-cpu-cost](../work-orders/20261022-i020-draw-cpu-cost.md) · rexbox: issue #23([작업 로그](../../third_party/rexbox/docs/work-logs/20261021-i023-draw-cpu-cost.md))

## 한 일

1. rexbox #23의 단계를 셋으로 나눠 넣고, 그때마다 submodule을 올렸다. 브랜치 `i023-draw-cpu-cost`이고 모두 push했다.
   * 상태 읽기·프로그램 조회·인덱스·uniform
   * 범위별 VAO
   * combiner 상수·sampler
2. 측정: v0.0.9 exe 사본과 새 빌드를 번갈아 바로 이어 실행했다. 실행마다 다른 빌드(`cl.exe`)가 없을 때까지 기다렸다.
3. GL 호출 수: v0.0.9 운전에서 프레임당 약 8000번이었다(`glVertexAttribPointer` 약 3000, `glBufferSubData`·`glDrawElements`·`glBindBuffer` 각 약 800). uniform 행렬은 약 350번뿐이라 constants의 비용은 CPU 계산이다.

## 검증

| 상태 | 빌드 | fps | CPU ms | D3D8 단계 ms |
|---|---|---|---|---|
| 느린 기계 | v0.0.9 → VAO까지 | 18.5 → 20.7 | 32.0 → 28.0 | 22.7 → 17.2 |
| 빠른 기계 | v0.0.9 → 이번 끝 | 28.2 → 27.8 | 23.5 → 21.7 | 13.7 → 11.5 |

* 빠른 상태에서 fps가 그대로인 것은 vblank 양자화 때문이다. 바쁜 시간이 약 25 ms에서 vblank 둘(33.3 ms) 근처를 오가서, 줄어든 시간이 대기로 바뀌었다.
* 그림: 어트랙트 데모 frame 1500과 운전 frame 2700을 덤프했다. 같은 장면에서 움직이는 물체만 다르다.
* CTest reCT3 5/5, rexbox 3/3.

## 남은 것

* 30 fps 고정과 게임 속도 0.5배에 이르려면 바쁜 시간을 몇 ms 더 줄여야 한다. rexbox 작업 로그의 다음 후보를 보라.
* 측정이 바깥 부하(다른 프로젝트 빌드, VS Code 색인·업데이트)에 크게 흔들렸다. 그런 구간의 실행은 버렸다.

---

# Work log: driving — the CPU cost of one D3D8 draw, first pass (issue #20)

Work order: [20261022-i020-draw-cpu-cost](../work-orders/20261022-i020-draw-cpu-cost.md) · rexbox: issue #23 ([work log](../../third_party/rexbox/docs/work-logs/20261021-i023-draw-cpu-cost.md))

Done:

1. rexbox #23 landed in three steps, each moving the submodule: state reads, program lookup, indices and uniforms; a VAO per range; combiner constants and samplers. All are on branch `i023-draw-cpu-cost`, pushed.
2. Measurement: a copy of the v0.0.9 exe and the new build in alternating back-to-back runs, each run waiting until no other build (`cl.exe`) was running.
3. GL calls while driving on v0.0.9: about 8000 per frame (about 3000 `glVertexAttribPointer`; `glBufferSubData`, `glDrawElements` and `glBindBuffer` about 800 each). Only about 350 are uniform matrices, so the constants cost is CPU work.

Verification:

| State | Build | fps | CPU ms | D3D8 phases ms |
|---|---|---|---|---|
| slow machine | v0.0.9 → up to the VAOs | 18.5 → 20.7 | 32.0 → 28.0 | 22.7 → 17.2 |
| fast machine | v0.0.9 → end of this pass | 28.2 → 27.8 | 23.5 → 21.7 | 13.7 → 11.5 |

* fps stays level in the fast state because of vblank quantization: a busy time of about 25 ms hovers around two vblanks (33.3 ms), so the saved time becomes waiting.
* Pictures: attract-demo frame 1500 and driving frame 2700 dumped; matching scenes differ only in moving objects.
* CTest reCT3 5/5, rexbox 3/3.

Left over:

* A steady 30 fps (game at 0.5×) needs a few more ms off the busy time; see the rexbox work log for the next candidates.
* Measurements swung widely with outside load (another project's build, VS Code indexing and an update); runs from those spans were discarded.
