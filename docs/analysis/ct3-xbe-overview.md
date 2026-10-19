# CT3 XBE 개요

일본판 `default.xbe`가 아직 준비되지 않았다(issue #1 시점). 이 문서는 XBE를 읽으면 채울 표와, 그 전에 알고 있는 것을 적는다. 표기: **확인됨**은 우리 도구로 직접 읽은 값, **추정**은 정황이나 외부 출처, **미확정**은 다음에 검증할 질문이다.

The Japan `default.xbe` is not in hand yet (as of issue #1). This document holds the tables to fill once the XBE is read, and what is known before that. **Confirmed** values were read by our tool directly; **inferred** ones rest on circumstance or an outside source; **unresolved** ones are questions to check next.

## 1. 빌드 변형 / Build variants

`scripts/recomp/register_xbe.py`가 아래 행을 출력한다. / `scripts/recomp/register_xbe.py` prints the row below.

| 레이블 / Label | 지역 / Region | MD5 | 크기 / Size | title id | 인증서 시각 / Certificate time | 이미지 시각 / Image time |
|---|---|---|---|---|---|---|
| Japan (1차 타깃 / primary) | — | — | — | — | — | — |

* **미확정**: 일본판의 title id, 인증서 시각, 이미지 시각, MD5. `register_xbe.py`로 채우고 `src/game/ct3/ct3_profile.cpp`의 `kVariants`에 같은 값을 넣는다.
* **미확정**: 디스크의 `default.xbe`가 유일한 XBE인지, 런처가 다른 XBE를 고르는지.
* **추정**: CT3는 2002년 7월에 발매되어 JSRF(2002년 2월, XDK 4134)보다 늦은 XDK로 빌드되었을 수 있다. XDK 버전은 XBE의 라이브러리 버전 표(`rexbox_xbe_info`)에서 **확인**한다. XDK 버전이 다르면 D3D8 렌더 상태 번호, `D3DCAPS8`, 디바이스 기본 상태 루틴이 달라 rexbox의 D3D8 HLE(XDK 4134 기준 측정값)를 손봐야 할 수 있다.

* **Unresolved**: the Japan build's title id, certificate time, image time and MD5; fill them with `register_xbe.py` and put the same values into `kVariants` in `src/game/ct3/ct3_profile.cpp`.
* **Unresolved**: whether the disc's `default.xbe` is its only XBE, and whether the dashboard launches another.
* **Inferred**: CT3 shipped in July 2002, later than JSRF (February 2002, XDK 4134), so it may be built with a later XDK. The XDK version is **confirmed** from the XBE's library version table (`rexbox_xbe_info`). A different XDK may change D3D8 render-state numbers, `D3DCAPS8` and the device's default-state routine, which rexbox's D3D8 HLE measured against XDK 4134, so the HLE may need adjusting.

## 2. 이미지 헤더 / Image header

`rexbox_xbe_info`의 출력으로 채운다. / To be filled from `rexbox_xbe_info`.

| 항목 / Field | 값 / Value |
|---|---|
| 키 / Keys | — |
| base | — |
| image size | — |
| entry point | — |
| kernel thunk table | — |
| TLS | — |

## 3. 섹션 / Sections

| 이름 / Name | 플래그 / Flags | vaddr | vsize | raw | raw size |
|---|---|---|---|---|---|
| — | | | | | |

## 4. XDK 라이브러리 / XDK libraries

| 라이브러리 / Library | 버전 / Version | 비고 / Note |
|---|---|---|
| — | | |

## 5. 커널 import / Kernel imports

* **미확정**: import 수와 rexbox 커널 HLE(104개 export)가 모두 덮는지. 빠진 것은 부팅에서 `stopped at unimplemented kernel import`로 드러난다.
* **Unresolved**: the import count and whether rexbox's kernel HLE (104 exports) covers them all; a missing one shows as `stopped at unimplemented kernel import` at boot.

## 6. 디스크 자산 / Disc assets

* **미확정**: 자산 형식과 미들웨어(CRI ADX/Sofdec 등). 디스크 폴더를 읽으면 채운다.
* **Unresolved**: asset formats and middleware (CRI ADX/Sofdec and the like); to be filled from the disc folder.
