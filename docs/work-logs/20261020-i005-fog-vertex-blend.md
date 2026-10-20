# 작업 로그: 포그와 정점 블렌딩 — 완료 (issue #5)

작업 지시: [20261020-i005-fog-vertex-blend](../work-orders/20261020-i005-fog-vertex-blend.md) · 설계와 rexbox 작업 로그: [rexbox i007-fog-vertex-blend](../../third_party/rexbox/docs/work-logs/20261020-i007-fog-vertex-blend.md)

## 한 일

1. rexbox v0.0.3을 `main`에 머지하고 submodule을 머지 커밋 `b2e8db7`로 옮겼다(v0.0.2의 작업 브랜치 고정을 해소).
2. 측정: CT3 데모의 포그 상태(EXP, density −0, start −10, end −500, FogColor `0x0F000000`)와 스킨 모드(0, 1, 3, 5)를 분석 문서 §4에 적었다. 상태 내보내기 `sub_000FD590`의 포그 규칙과 `SetRenderState_VertexBlend`의 SET_SKIN_MODE도 함께 적었다.
3. rexbox(#7)에 포그와 정점 블렌딩을 넣었다. CT3의 블렌드 가중치가 `PBYTE4`(`0x44`)임을 실행으로 확인했다. submodule을 그 커밋으로 올렸고, 머지 뒤 rexbox v0.0.4(`0fce514`)에 고정했다.

## 검증

| 검증 | 결과 |
|---|---|
| Win32 x86 Debug, CTest | 오류 0, probe 83 / 55 / 220 |
| `rect3.exe roms` | 셰이더 오류·"not drawn" 없음. 블렌딩 프로그램 `;b21`, `;b32`, `;b43`과 포그 `;f30`이 만들어진다. 데모 장면이 정상으로 그려진다(실행마다 시점이 달라 같은 장면 비교는 못 함) |

포그는 데모에서 계수 1이라 화면 변화가 없다. 게임 플레이에서의 확인은 사용자 환경에서 한다.

---

# Work log: fog and vertex blending — done (issue #5)

Work order: [20261020-i005-fog-vertex-blend](../work-orders/20261020-i005-fog-vertex-blend.md) · Design and rexbox work log: [rexbox i007-fog-vertex-blend](../../third_party/rexbox/docs/work-logs/20261020-i007-fog-vertex-blend.md)

Done: rexbox v0.0.3 merged to its `main` and the submodule moved to the merge commit `b2e8db7` (ending v0.0.2's task-branch pin); the measurements of CT3's demo fog (EXP, density −0, start −10, end −500, FogColor `0x0F000000`) and skin modes (0, 1, 3, 5) recorded in analysis section 4, with the fog rules of the state flush `sub_000FD590` and `SetRenderState_VertexBlend`'s SET_SKIN_MODE; fog and vertex blending added in rexbox (#7), CT3's blend weights found at run time to be `PBYTE4` (`0x44`), and the submodule moved to that commit, then pinned to the merged rexbox v0.0.4 (`0fce514`).

Verification: Win32 x86 Debug and CTest with no errors, probes 83 / 55 / 220; `rect3.exe roms` reports no shader error or "not drawn", builds the blending programs `;b21`, `;b32`, `;b43` and the fog program `;f30`, and draws the demo scenes correctly (their timing differs between runs, so the same scene could not be compared). Fog has factor 1 in the demo and changes nothing on screen; gameplay is checked on the user's machine.
