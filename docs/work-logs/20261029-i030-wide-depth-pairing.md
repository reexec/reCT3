# 작업 로그: 와이드스크린에서 글로우 패스가 길에 구멍을 내는 문제 (issue #30)

작업 지시: [20261029-i030](../work-orders/20261029-i030-wide-depth-pairing.md). rexbox: reexec/rexbox#38, [작업 로그](../../third_party/rexbox/docs/work-logs/20261029-i038-wide-depth-pairing.md).

## 원인

사용자 캡처(F9, 프레임 7289, 21:9, 렌더 배율 4)에서 확인했다. 검은 점은 백버퍼를 지운 뒤 아무것도 그리지 않은 구멍이다. 길 그리기가 깊이 검사에서 떨어졌다. 게임은 헤드라이트 같은 빛나는 물체를 640x480 텍스처 타깃에 깊이 쓰기를 켠 채 그린다(장면 사이에 31건). 이 텍스처 타깃은 640 그대로인데 함께 묶인 깊이 스텐실은 1120으로 넓혀져 있어서, 깊이가 4:3 x 자리에 들어갔다. 원래 들어가야 할 자리는 x + 240이다. 그래서 점이 헤드라이트에서 왼쪽으로 240 게스트 픽셀 떨어져 따라다녔다.

## 한 일

* `third_party/rexbox`를 v0.0.16 위의 i038 커밋 `8e6aa3d`로 올렸다. 넓힌 깊이와 같은 크기로 묶인 색 타깃도 넓히고, 그 샘플링 복사는 가운데 4:3만 가져간다. 캡처 줄에는 `zs`(깊이 표면)가 적힌다.

## 검증

| 검증 | 결과 |
|---|---|
| Win32 x86 Release 빌드 | 경고·오류 0 |
| `ctest -C Release` | 7/7 통과 |
| 21:9 밤 운전(사용자 플레이) | 검은 점이 사라짐(**확인됨**) |
| 21:9 어트랙트 데모 150 s | 끝까지 진행(첫 실행의 present 1794 정지는 다시 실행하면 재현되지 않음) |

---

# Work log: the glow pass leaves holes in the road at widescreen (issue #30)

Work order: [20261029-i030](../work-orders/20261029-i030-wide-depth-pairing.md). rexbox: reexec/rexbox#38, [work log](../../third_party/rexbox/docs/work-logs/20261029-i038-wide-depth-pairing.md).

Cause, from the user's capture (F9, frame 7289, 21:9, render scale 4): the black dots are holes that nothing drew after the back buffer's Clear, because the road draw failed the depth test there. The title draws its glow sources (the headlights among them) into a 640x480 texture target with depth writes on (31 draws between scene draws). That texture target stays 640 wide while the depth stencil bound with it is widened to 1120, so its depth landed at the 4:3 x instead of x + 240. The dots therefore followed the headlights 240 guest pixels to their left.

Done: `third_party/rexbox` moved to the i038 commit `8e6aa3d` on top of v0.0.16. A color target bound at the widened depth's size is widened too, its sampling copy takes the 4:3 middle, and capture lines record `zs` (the depth surface).

Verification: Win32 x86 Release build with no warnings or errors; `ctest -C Release` 7/7; no black dots while driving at night at 21:9 (**confirmed** by the user); the 21:9 attract demo ran 150 s (a stop at present 1794 in the first run did not reproduce on a rerun).
