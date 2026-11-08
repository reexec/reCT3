# 작업 로그: 프레임 페이싱 — 1차, 밀집 장면의 GL 비용 (issue #41)

issue: [#41](https://github.com/reexec/reCT3/issues/41) · rexbox 설계: [i076 GL 시간](../../third_party/rexbox/docs/design/20261107-i076-gl-call-time.md), [i078 VAO 비용](../../third_party/rexbox/docs/design/20261107-i078-vao-cost.md)

## 한 일

1. rexbox를 v0.0.21에서 v0.0.23으로 올렸다(`7211b8e`, v0.0.22로 머지). 그 뒤 스트레스 실행에서 드러난 CRI 경쟁은 #44로 따로 고쳤다.
2. 밀집 장면의 HLE 시간 가운데 GL 드라이버가 차지하는 몫을 재는 진단을 rexbox #76으로 더했다(rexbox v0.0.27, `REXBOX_GL_CALLS=time`).
   * West Coast 주행에서 1584 draw 창의 GL 시간은 프레임당 7.44 ms였다. 그중 6.66 ms가 `glBindVertexArray`였다.
   * 바인드 한 번의 비용은 draw 수와 함께 1 µs에서 4.4 µs로 커졌다.
   * 그래서 렌더 스레드 대신 VAO 사용 방식을 먼저 고치기로 했다.
3. rexbox #78(v0.0.28)에서 캐시된 정점 구간마다 두던 VAO(rexbox #23)를 걷어내고, 모든 draw가 VAO 하나를 쓰게 했다. 실험 스위치로 두 방식을 같은 주행에서 비교한 뒤 결정했다.
4. 역할: rexbox 코드는 Antigravity 코더가 썼다. 설계, 검토, 측정, 문서는 Claude가 했다.

## 검증

reCT3 West Coast 주행(`scripts/drive_west_coast.ps1`, 21:9, 렌더 배율 4, MSAA 16x, RTX 4090), 밀집 구간:

| rexbox | draw/프레임 | fps | cpu ms/프레임 | GL ms/프레임 |
|---|---|---|---|---|
| v0.0.27(구간별 VAO) | 1315~1586 | 54.2~56.7 | 12.7~14.3 | 4.1~4.6 |
| v0.0.28(VAO 하나) | 1229~1297 | 59.2~59.3 | 8.3~8.9 | 약 0.9 |

* 주행마다 지나는 장면의 밀도가 달라, draw 수가 비슷한 구간끼리 비교했다.
* Win32 x86 Release 빌드 경고 0, CTest 7/7. 최종 빌드 주행에서 스트림 멈춤·null 호출·잠금 시한·stall·GL 오류가 없었다.

## 남은 것

* 사용자 확인으로는 프레임이 아직 고르지 않다. issue 본문의 1·2(가벼운 장면의 간격 흔들림, 가상 수직 귀선 시계와 모니터의 어긋남)와 밀집 장면의 나머지 cpu(타이틀 코드 약 4 ms, HLE 변환)가 남았다. #41은 열어 둔다.

---

# Work log: frame pacing — part 1, the GL cost of dense scenes (issue #41)

Issue: [#41](https://github.com/reexec/reCT3/issues/41) · rexbox designs: [i076 GL time](../../third_party/rexbox/docs/design/20261107-i076-gl-call-time.md), [i078 VAO cost](../../third_party/rexbox/docs/design/20261107-i078-vao-cost.md)

## Done

1. Moved rexbox from v0.0.21 to v0.0.23 (`7211b8e`, merged with v0.0.22). The CRI race that the stress runs exposed afterwards was fixed separately as #44.
2. Added a diagnostic for the GL driver's share of the HLE time in dense scenes, as rexbox #76 (rexbox v0.0.27, `REXBOX_GL_CALLS=time`).
   * In a West Coast drive, a 1584-draw window spent 7.44 ms per frame in GL, 6.66 ms of it in `glBindVertexArray`.
   * One bind cost more as draws grew, from 1 µs to 4.4 µs.
   * So the VAO usage came before a render thread.
3. rexbox #78 (v0.0.28) removed the VAO per cached vertex range (rexbox #23), so every draw uses one VAO. The decision followed a comparison of both ways over the same drives with an experiment switch.
4. Roles: the Antigravity coder wrote the rexbox code; Claude designed, reviewed, measured and wrote the documents.

## Verification

reCT3 West Coast drives (`scripts/drive_west_coast.ps1`, 21:9, render scale 4, MSAA 16x, RTX 4090), dense stretches:

| rexbox | Draws/frame | fps | cpu ms/frame | GL ms/frame |
|---|---|---|---|---|
| v0.0.27 (VAO per range) | 1315–1586 | 54.2–56.7 | 12.7–14.3 | 4.1–4.6 |
| v0.0.28 (one VAO) | 1229–1297 | 59.2–59.3 | 8.3–8.9 | about 0.9 |

* Each drive passes scenes of different density, so stretches with similar draw counts are compared.
* Win32 x86 Release with 0 warnings, CTest 7/7. The final build's drive had no stream stop, null call, lock timeout, stall or GL error.

## Remaining

* The user reports the frames are still not steady. Items 1 and 2 of the issue (uneven intervals in light scenes, the virtual vertical-blank clock drifting from the monitor) and the rest of the dense scenes' CPU (about 4 ms of title code, the HLE translation) remain. #41 stays open.
