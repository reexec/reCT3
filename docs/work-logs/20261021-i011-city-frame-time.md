# 작업 로그: 도시 장면의 프레임 시간 — 완료 (issue #11)

작업 지시: [20261021-i011-city-frame-time](../work-orders/20261021-i011-city-frame-time.md) · rexbox: issue #16, [작업 로그](../../third_party/rexbox/docs/work-logs/20261021-i016-p8-palette-variants.md)

## 한 일

1. 측정: 도시 장면에서 22~26 fps, D3D8 `setup`이 프레임당 최대 12.4 ms. `REXBOX_SAMPLE`에서 vblank 대기 다음이 텍스처 디코딩이었다.
2. 원인: 타이틀이 같은 P8 텍스처 헤더(차량 등)를 한 프레임에 최대 7개 팔레트로 그리고, rexbox가 헤더당 GL 텍스처 하나만 두어 팔레트가 바뀔 때마다 다시 디코딩했다(프레임당 최대 42회).
3. rexbox #16에 팔레트별 변형 캐시를 넣고 submodule을 작업 브랜치 커밋 `61ffe64`로 올렸고, 머지 뒤 rexbox v0.0.6(`bc46161`)에 고정했다.
4. 게임 속도 확인: `REXBOX_WATCH32=2dfda8,2dfdac`와 `REXBOX_FRAME_LOG`로 10초 동안 vblank 스레드(`sub_000B3110`, `0x2DFDA8`)와 메인 루프(`sub_000B3180`, `0x2DFDAC`)가 각각 600회, present가 300회였다. 로직은 60 Hz로 돌고 화면은 두 틱에 한 번 그린다(타이틀의 설계). 진행 속도는 정상이다.

## 검증

| 검증 | 결과 |
|---|---|
| Win32 x86 Release, CTest | 오류 0, 5/5 |
| 도시 장면(300프레임 구간) | 22.0~26.1 fps, `setup` 5.9~12.4 ms → 27.9~30.0 fps, `setup` 1.2~2.3 ms. CPU 19.3 → 9.5~12.8 ms |
| 화면 | frame 1500(상점가), 2100(언덕길): 지형·간판 정상, 같은 차 텍스처가 색마다 따로 그려진다 |

## 남은 것

* 300프레임마다 "33.4 ms 초과"가 140~170개다. 30 fps에서 한 프레임이 vblank 두 번이라 경계 근처의 흔들림으로 **추정**한다. 가장 나쁜 프레임은 43~99 ms.
* 장면 전환(로딩) 구간의 긴 프레임(100~130 ms).

---

# Work log: frame time in the city scenes — done (issue #11)

Work order: [20261021-i011-city-frame-time](../work-orders/20261021-i011-city-frame-time.md) · rexbox: issue #16, [work log](../../third_party/rexbox/docs/work-logs/20261021-i016-p8-palette-variants.md)

Done: measured 22–26 fps in the city scenes with D3D8 `setup` at up to 12.4 ms per frame, and `REXBOX_SAMPLE` put texture decoding first after the vblank wait. The cause: the title draws the same P8 texture header (vehicles and the like) with up to 7 palettes in a frame, and rexbox kept one GL texture per header, decoding again at every palette switch (up to 42 per frame). rexbox #16 caches a variant per palette; the submodule moved to that task-branch commit `61ffe64`, then was pinned to the merged rexbox v0.0.6 (`bc46161`). Game speed: with `REXBOX_WATCH32=2dfda8,2dfdac` and `REXBOX_FRAME_LOG`, in 10 s the vblank thread (`sub_000B3110`, `0x2DFDA8`) and the main loop (`sub_000B3180`, `0x2DFDAC`) ran 600 times each against 300 presents: the logic runs at 60 Hz and the picture is drawn every other tick (the title's design), so the game runs at the right speed.

Verification: Win32 x86 Release with no errors and CTest 5/5. Over 300-frame spans in the city, 22.0–26.1 fps with `setup` at 5.9–12.4 ms became 27.9–30.0 fps with `setup` at 1.2–2.3 ms, and CPU time went from 19.3 ms to 9.5–12.8 ms. Frames 1500 (the shopping street) and 2100 (the hill road) draw the geometry and signs correctly, with the same car texture drawn separately in each colour.

Left open: 140–170 of every 300 frames count as "over 33.4 ms", **inferred** to be jitter at the boundary since a frame is two vblanks at 30 fps; the worst frames are 43–99 ms. Long frames (100–130 ms) at scene changes (loading).
