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

## 2차 (2026-11-03, 브랜치 `i020-draw-cpu-cost-2`, rexbox v0.0.20)

rexbox [작업 로그 i023 2차](../../third_party/rexbox/docs/work-logs/20261021-i023-draw-cpu-cost.md)와 [설계 i023 미리 올리기](../../third_party/rexbox/docs/design/20261103-i023-program-warmup.md). 기계: Ryzen 5 5600X, RTX 4090.

* 이 기계에서 꾸준한 운전은 60 fps다(기준 v0.0.12와 같음, `other` 2.4~2.7 ms). SJ 잠금(#29)은 영향이 없다. 처음에 30 fps로 보였던 것은 rexbox의 긴 프레임 진단이 매 프레임 stderr에 써서 생긴 것이었다. 진단을 고쳤다.
* 재생성을 잊으면 decomp 대체가 링크되지 않는다. 10-29의 `gen/`으로 빌드하면 `sj_lock.c`가 빠진 채 링크됐다(map 파일로 확인). main으로 옮길 때는 늘 재생성한다.
* 끊김은 운전이 시작되는 몇 초에 몰린다. rexbox v0.0.20의 프로그램 미리 올리기로 장면 시작의 캐시 프로그램 74개가 40~42 ms에서 4~5 ms로 줄었다. 최악 프레임은 58~61 ms에서 50~51 ms가 됐다(켬/끔 번갈아 3쌍). 부팅 구간 fps는 같다.
* 검증: Win32 x86 Release 경고 0(생성 코드의 기존 D9025 제외), CTest 7/7.
* 남은 것: 운전 시작의 텍스처 업로드(112개 15 ms, 가끔 56 ms), 카메라 도입부의 그리기 1000~1500개 프레임. 미리 올리기를 표시 스레드에서 파일을 읽게 했던 첫 시험 실행이 `000C967E`의 null 간접 호출로 한 번 멈췄다(부팅 타이밍이 흔들린 실행, #18 계열로 **추정**).

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

Second pass (2026-11-03, branch `i020-draw-cpu-cost-2`, rexbox v0.0.20; see rexbox's [i023 work log](../../third_party/rexbox/docs/work-logs/20261021-i023-draw-cpu-cost.md) and the [warming design](../../third_party/rexbox/docs/design/20261103-i023-program-warmup.md); Ryzen 5 5600X, RTX 4090): steady driving runs at 60 fps on this machine, as v0.0.12 does (`other` 2.4–2.7 ms), and the SJ lock (#29) costs nothing; the 30 fps first seen came from rexbox's long-frame diagnostic writing to stderr each frame, now fixed. A build without regeneration leaves the decomp replacements out (built on the 10-29 `gen/`, `sj_lock.c` was not linked, as the map file showed), so moving to a new main always regenerates. Hitches gather in the first seconds of driving; with v0.0.20's program warming the 74 cached programs at the scene start went from 40–42 ms to 4–5 ms and the worst frame from 58–61 ms to 50–51 ms (three alternating on/off pairs), boot fps unchanged. Verification: Win32 x86 Release with 0 warnings (apart from the generated code's existing D9025), CTest 7/7. Left over: the texture uploads at the start of driving (112 in 15 ms, sometimes 56 ms) and the camera-intro frames of 1000–1500 draws; a first trial run in which warming read files on the presenting thread stopped once on a null indirect call at `000C967E` (boot timing disturbed; **inferred** to be of #18's family).
