# 설계: 오프닝의 바퀴 불꽃 — rexbox #42를 따라감 (issue #35)

상태: 구현됨([작업 지시](../work-orders/20261103-i035-wheel-flames.md), [작업 로그](../work-logs/20261103-i035-wheel-flames.md)). 구현 설계: rexbox [설계 i042](../../third_party/rexbox/docs/design/20261103-i042-combiner-factors.md).

## 1. 요구사항

오프닝(어트랙트 데모)에서 택시 바퀴를 따라다니는 불꽃이 원본에는 있는데 reCT3에서는 보이지 않는다(사용자 보고, 참고 이미지).

## 2. 원인과 설계

* 불꽃 사각형은 그려지지만 픽셀을 바꾸지 않는다. 불꽃의 픽셀 셰이더는 combiner 상수 c0을 곱하는데, 게임(`sub_000A4710`)은 그리기마다 NV2A `COMBINER_FACTOR0/1`(0xA60, 0xA80)을 `SetRenderState_Simple`로 쓴다. rexbox는 그 값을 쓰지 않고 정의의 상수(알파 0)를 썼다(**확인됨**).
* 구현은 공유 런타임에서 한다(rexbox #42, v0.0.19). 이 저장소는 submodule을 올린다. 재생성은 필요 없다.

## 3. 검증

빌드·CTest, 데모 프레임(해변 2155, 잔디 3720, 3960)과 전체 비교, 사용자 확인.

---

# Design: the opening's wheel flames, following rexbox #42 (issue #35)

Status: implemented ([work order](../work-orders/20261103-i035-wheel-flames.md), [work log](../work-logs/20261103-i035-wheel-flames.md)). Implementation design: rexbox [design i042](../../third_party/rexbox/docs/design/20261103-i042-combiner-factors.md).

Requirement: the opening (attract demo) shows flames trailing the taxi's wheels in the original but not in reCT3 (user report, reference image). Cause and design: the flame quads are drawn but change no pixel; their pixel shader multiplies by combiner constant c0, which the game (`sub_000A4710`) sets every draw by writing NV2A `COMBINER_FACTOR0/1` (0xA60, 0xA80) through `SetRenderState_Simple`, and rexbox used the definition's constant (alpha 0) instead (**confirmed**). The fix is in the shared runtime (rexbox #42, v0.0.19); this repository moves the submodule, no regeneration needed. Verification: build and CTest, demo frames (beach 2155, grass 3720, 3960) and a whole-demo comparison, user check.
