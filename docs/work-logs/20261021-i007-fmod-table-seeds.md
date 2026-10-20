# 작업 로그: fmod 분류 테이블 항목 lift — 완료 (issue #7)

작업 지시: [20261021-i007-fmod-table-seeds](../work-orders/20261021-i007-fmod-table-seeds.md)

## 한 일

1. 사용자 실행(Release, 호스트 오디오, 창 표시)의 멈춤을 확인했다. 먼저 받은 실행은 로컬 `third_party/rexbox`가 고정 커밋 `0fce514`가 아닌 `93d3b71`로 체크아웃되어 있었고, `gen/ct3_jp`가 `lifted` 목록 변경 전에 생성되어 `XAudioDownloadEffectsImage`에서 멈췄다. `git submodule update`와 재생성으로 해소한 뒤 아래 멈춤이 나왔다.
2. `000E9C7C`를 담은 dword는 `.data 002148B0` 하나다. 그 앞은 이름 `"\x04fmod"`(`00214890`)이고, `002148A0`~`002148DC`의 16칸이 `fmod`의 피연산자 분류 테이블이다. `.data 00214600`~`00214C00` 전체에서 `000E9000`~`000EC000`을 가리키는 미lift dword는 `000E9C7C`(1칸)와 `000E9D1E`(`002148A4`, `AC`, `B4`, `BC`, `D0`, `D4`, `DC`)뿐이다.
3. `000E9C7C`는 `fstp st(0)` 뒤 `sub_000E9C7E`(`fstp st(0)`, `fldz`, `ret`)로 이어지고, `000E9D1E`는 `fstp st(0)` 두 번 뒤 `fld xword [0x214F54]`를 한다. 둘 다 명령어 경계다.
4. 두 주소를 `seed_pointer_targets.json`에 넣고 재생성했다. `recomp_dispatch.c`에 `sub_000E9C7C`, `sub_000E9D1E`가 생겼다.

## 검증

| 검증 | 결과 |
|---|---|
| Win32 x86 Release, CTest | 오류 0, 5/5 |
| `rect3.exe roms`, 호스트 오디오, hidden, 90초 | `no target`·`[stall]` 없음. `_bank10.mdl`, `_taxi2hot.bin` 캐시 조회(없음, `C0000034`)까지 진행 |

멈춤은 실행 타이밍에 따라 나타난다(오디오 없는 40초 실행에서는 나오지 않았다). 창을 띄운 실행은 사용자 환경에서 확인한다.

## 남은 것

* rexbox의 `find_pointer_targets.py`가 이 테이블을 후보로 내지 않는 이유(정렬 또는 시작 바이트 조건으로 **추정**). CRT의 다른 분류 테이블도 같은 형태다.
* 새로 보인 보고: `NtSetInformationFile class 10 unsupported`, 사전 변환 정점에 대한 텍스처 좌표 생성·변환 미적용(stage 0).

---

# Work log: lift the fmod class-table entries — done (issue #7)

Work order: [20261021-i007-fmod-table-seeds](../work-orders/20261021-i007-fmod-table-seeds.md)

Done: the user's run (Release, host audio, window shown) was checked. The first run received had the local `third_party/rexbox` checked out at `93d3b71` instead of the pinned `0fce514`, and `gen/ct3_jp` generated before the `lifted` list changed, so it stopped at `XAudioDownloadEffectsImage`; `git submodule update` and regenerating cleared that, after which the stall below appeared. The only dword holding `000E9C7C` is `.data 002148B0`; it follows the name `"\x04fmod"` (`00214890`), and the 16 slots `002148A0`–`002148DC` are `fmod`'s operand-class table. Across `.data 00214600`–`00214C00`, the only unlifted dwords pointing into `000E9000`–`000EC000` are `000E9C7C` (one slot) and `000E9D1E` (`002148A4`, `AC`, `B4`, `BC`, `D0`, `D4`, `DC`). `000E9C7C` is `fstp st(0)` running into `sub_000E9C7E` (`fstp st(0)`, `fldz`, `ret`); `000E9D1E` is two `fstp st(0)` and `fld xword [0x214F54]`; both are instruction boundaries. Both addresses went into `seed_pointer_targets.json`; after regenerating, `recomp_dispatch.c` has `sub_000E9C7C` and `sub_000E9D1E`.

Verification: Win32 x86 Release with no errors and CTest 5/5; `rect3.exe roms` on host audio, hidden, 90 s: no `no target` and no `[stall]`, reaching the cache lookups `_bank10.mdl` and `_taxi2hot.bin` (absent, `C0000034`). The stall depends on run timing (a 40 s run without audio did not show it); a run with the window shown is checked on the user's machine.

Left open: why rexbox's `find_pointer_targets.py` does not list this table (**inferred**: its alignment or start-byte conditions); the CRT's other class tables have the same shape. Newly seen reports: `NtSetInformationFile class 10 unsupported`, and texture-coordinate generation or transforms not applied to pretransformed vertices (stage 0).
