# 작업 로그: 운전 화면의 행렬 함수 세 개 디컴파일 — 완료 (issue #27)

작업 지시: [20261026-i027-matrix-decomp](../work-orders/20261026-i027-matrix-decomp.md) · 설계: [20261026-i027-matrix-decomp](../design/20261026-i027-matrix-decomp.md) · 분석: [d3dx-matrix](../analysis/d3dx-matrix.md)

## 한 일

1. 분석: 세 함수는 정적 링크된 D3DX의 `D3DXVec4Transform`(`000E4B73`), `D3DXMatrixMultiply`(`000E4BB9`), `D3DXMatrixInverse`(`000E4CB7`)로 **추정**된다. 역행렬의 lift 본문(x87 연산 약 300개)을 기호 실행해 연산 순서, 반올림 지점, 프레임 슬롯 21개와 인자 슬롯, x87 슬롯 8칸의 최종값을 정리했다(분석 문서).
2. `src/game/ct3/decomp/d3dx_matrix.c`: 세 함수를 일반 C로 다시 썼다. SSE 쪽은 lane별 단정밀도, 역행렬은 `double`에 lifter와 같은 연산별 정밀도 제어 반올림과 float 저장 변환을 한다. 레지스터, XMM, x87 스택, 메모리 부수 효과를 원본과 같게 남긴다.
3. `tools/decomp_check/`(`rect3_decomp_check`): 디컴파일 함수와 lift 원본(`sub_X_gen`)을 같은 상태에서 돌려 GPR, `g_seh_ebp`, XMM0~7, x87 제어어·top·슬롯 8칸, 게스트 메모리 전체를 비교한다. x87·SEH 상태는 생성 코드의 헤더로 읽는 C 보조 파일(`guest_fpu.c`)이 다룬다. CTest에 등록했다. 최상위 CMake는 생성 코드가 있을 때만 `tools/`를 더한다.
4. 생성 코드 헤더의 C4310 경고는 rexbox의 ABI 객체처럼 디컴파일 소스와 `guest_fpu.c`에서만 끈다.
5. 측정 중 드문 멈춤 1회(아래)가 나와, 처음에는 죽은 값이라 두었던 x87 슬롯의 마지막 값까지 원본과 같게 남기도록 고치고 검사에 넣었다.
6. ARCHITECTURE(검사 도구, #25 때 빠진 rexbox v0.0.14 표기), `decomp/README.md`, Win32·Linux 가이드.

## 검증

| 검증 | 결과 |
|---|---|
| `rect3_decomp_check` | 50개 경우 모두 차이 0: Win32 Debug·Release, Linux x64·x86 Release(WSL2) |
| 검사의 검출력 | 일부러 넣은 덧셈 묶음 변경, 프레임 슬롯 누락, x87 중간 반올림 생략이 각각 차이로 잡힌다. 무작위 입력은 유효 비트 24개를 모두 쓴다 |
| 빌드 | Win32 Debug·Release와 Linux x64·x86 경고 0(생성 코드 대상의 기존 D9025 제외), CTest 7/7 |
| 재생성 | `--exclude-manual`로 세 함수가 빠지고 `_gen` 본문이 남는다(HLE 246개 + 3 = 249개) |
| 운전 구간 측정 1(첫 버전, 기준과 번갈아, 120 present 간격) | 표시 스레드 CPU 6.07 → 5.18 ms, `other` 3.30 → 2.83 ms, fps 58.2 → 58.9. 세 함수의 표본 165 → 105개(역행렬 78 → 65, 그중 반올림 보조 함수 `Pc` 30; 곱 62 → 21; 변환 25 → 19) |
| 운전 구간 측정 2(최종 버전, 600 present 간격) | `other` 2.76 → 2.47~2.50 ms, CPU 5.31 → 5.06~5.36 ms(흔들림 범위) |
| 운전 진입 반복 | 첫 버전 9회 중 1회 멈춤(present 2381, 스레드 2가 `sub_000BB9C0`의 로딩 대기 루프, `[recomp]` 경고 없음). 최종 버전 6회와 기준 12회는 멈춤 없음 |

## 남은 것

* 첫 버전의 멈춤 1회는 원인을 확인하지 못했다. 그 뒤 비교하지 않던 유일한 차이(x87 슬롯)를 없앴고, 최종 버전은 검사하는 모든 상태에서 원본과 같다. 다시 나오면 issue #18과 같은 드문 기존 문제로 보고 따로 다룬다.
* 역행렬의 `Pc`(정밀도 반올림)가 인라인되지 않아 표본 30개를 차지한다. 더 줄이려면 PC=24일 때의 빠른 경로를 따로 본다.
* ARM(Android)처럼 곱셈·덧셈을 FMA로 합치는 대상에서는 결과가 달라질 수 있다(설계 §3 위험).

---

# Work log: decompile the three matrix functions of the driving frame — done (issue #27)

Work order: [20261026-i027-matrix-decomp](../work-orders/20261026-i027-matrix-decomp.md) · Design: [20261026-i027-matrix-decomp](../design/20261026-i027-matrix-decomp.md) · Analysis: [d3dx-matrix](../analysis/d3dx-matrix.md)

Done: the three functions are **inferred** to be the statically linked D3DX `D3DXVec4Transform` (`000E4B73`), `D3DXMatrixMultiply` (`000E4BB9`) and `D3DXMatrixInverse` (`000E4CB7`); a symbolic run of the inverse's lifted body (about 300 x87 operations) gave its operation order, rounding points and the final values of 21 frame slots, the argument slot and the eight x87 slots (analysis document). `src/game/ct3/decomp/d3dx_matrix.c` rewrites the three in plain C: lane-wise single precision on the SSE side, and for the inverse `double` with the lifter's per-operation precision-control rounding and float store conversions, leaving the registers, XMM, x87 stack and memory side effects as the originals do. `tools/decomp_check/` (`rect3_decomp_check`) runs each against its lifted original (`sub_X_gen`) from the same state and compares the GPRs, `g_seh_ebp`, XMM0–7, the x87 control word, top and eight slots, and all of guest memory; a C helper (`guest_fpu.c`) handles the x87 and SEH state through the generated header; it is registered with CTest, and the top-level CMake adds `tools/` only with generated code. The generated header's C4310 warning is turned off for the decompiled sources and `guest_fpu.c` only, as rexbox does for its ABI object. After one rare stall in the measurements (below) the x87 slots' last values, first left alone as dead values, are left as the original leaves them and compared. ARCHITECTURE (the checker, and the rexbox v0.0.14 pin missed in #25), `decomp/README.md`, the Win32 and Linux guides.

Verification: `rect3_decomp_check` has 0 differences in all 50 cases on Win32 Debug and Release and Linux x64 and x86 Release (WSL2); a regrouped sum, a missing frame slot and a skipped intermediate x87 rounding injected on purpose each show as differences, with random inputs using full 24-bit significands. Win32 Debug and Release and Linux x64 and x86 build with 0 warnings (apart from the existing D9025 of the generated-code target) and CTest 7/7. Regeneration drops the three through `--exclude-manual` and keeps their `_gen` bodies (246 HLE + 3 = 249). Driving measurement 1 (first version, alternating with the baseline, every 120 presents): presenting-thread CPU 6.07 → 5.18 ms, `other` 3.30 → 2.83 ms, fps 58.2 → 58.9, and the three functions' samples 165 → 105 (inverse 78 → 65, of which 30 in the rounding helper `Pc`; product 62 → 21; transform 25 → 19). Driving measurement 2 (final version, every 600 presents): `other` 2.76 → 2.47–2.50 ms, CPU 5.31 → 5.06–5.36 ms (within the noise). Repeated entries into driving: the first version stalled once in 9 runs (present 2381, thread 2 in `sub_000BB9C0`'s loading wait loop, no `[recomp]` warning); the final version in 6 runs and the baseline in 12 did not.

Left open: the first version's single stall is not explained; the only uncompared difference (the x87 slots) has since been removed and the final version matches the original in every state the check compares, so a recurrence is treated as a rare existing problem like issue #18. The inverse's `Pc` (precision rounding) is not inlined and takes 30 samples; a fast path for PC=24 would cut more. Targets that fuse multiply and add into FMA, such as ARM (Android), may differ (design section 3, risks).
