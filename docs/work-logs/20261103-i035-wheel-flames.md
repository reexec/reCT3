# 작업 로그: 오프닝의 바퀴 불꽃 (issue #35)

작업 지시: [20261103-i035-wheel-flames](../work-orders/20261103-i035-wheel-flames.md) · 설계: [20261103-i035-wheel-flames](../design/20261103-i035-wheel-flames.md) · rexbox: [작업 로그 i042](../../third_party/rexbox/docs/work-logs/20261103-i042-combiner-factors.md)

## 한 일

1. 데모 프레임을 120프레임 간격으로 덤프해 해변 장면(프레임 2140~2185)을 찾았다.
2. 프레임 2155를 캡처했다. 불꽃 사각형(가산 블렌딩, 깊이 쓰기 끔, DXT3 64×64)이 택시 위치에 그려지지만 픽셀을 소유하지 않는다. 텍스처 알파는 0이 아니고 위치는 화면 안이다.
3. 불꽃의 픽셀 셰이더(핸들 14)는 2단계 combiner로 c0을 곱한다. `sub_000A4710`이 그리기마다 `COMBINER_FACTOR0/1`을 쓰는 것을 lift 코드에서 찾았다.
4. rexbox #42(v0.0.19, `7704477`)로 고치고 submodule을 올렸다. 같은 릴리스로 rexbox #43(스트림 정지 보고, issue #29의 진단)도 들어왔다.

## 검증

| 검증 | 결과 |
|---|---|
| Linux x64 Release | 경고 0, CTest 7/7 |
| 데모 프레임 | 해변(2155) 뒷바퀴 불꽃, 잔디(3720) 바퀴 자국 불꽃 꼬리(초록빛), 3960 택시를 둘러싼 불꽃. 다른 장면에 눈에 띄는 변화 없음 |
| 사용자 확인 | 불꽃 확인 |

## 남은 것

* 잔디 위 불꽃 꼬리의 초록빛이 원본과 같은지 확인하지 않았다.

---

# Work log: the opening's wheel flames (issue #35)

Work order: [20261103-i035-wheel-flames](../work-orders/20261103-i035-wheel-flames.md) · Design: [20261103-i035-wheel-flames](../design/20261103-i035-wheel-flames.md) · rexbox: [work log i042](../../third_party/rexbox/docs/work-logs/20261103-i042-combiner-factors.md)

Done: demo frames dumped every 120 frames found the beach scene (frames 2140–2185). Capturing frame 2155 showed the flame quads (additive, no depth write, DXT3 64×64) drawn at the taxi but owning no pixel, with non-zero texture alpha and an on-screen position. Their pixel shader (handle 14) is a two-stage combiner multiplying by c0, and the lifted `sub_000A4710` writes `COMBINER_FACTOR0/1` before every draw. Fixed through rexbox #42 (v0.0.19, `7704477`) and the submodule moved; the same release brings rexbox #43 (the stalled-stream report, issue #29's diagnosis).

Verified: Linux x64 Release with 0 warnings and CTest 7/7; demo frames: rear-wheel flames on the beach (2155), a flame trail along the wheel tracks on the grass (3720, greenish), flames around the taxi at 3960, no visible change elsewhere; the user confirmed the flames.

Left open: whether the grass trail's green tint matches the original was not checked.
