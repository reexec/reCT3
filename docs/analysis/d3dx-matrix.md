# D3DX 행렬 함수 세 개 (issue #27)

근거: 디스어셈블리(`gen/ct3_jp/disasm/asm/text.asm`)와 lift된 본문(`sub_X_gen`), 실행 검사(`rect3_decomp_check`). 설계: [20261026-i027-matrix-decomp](../design/20261026-i027-matrix-decomp.md).

## 1. 정체

세 함수 모두 `.text`의 `000E4B73`~`000E5044`에 붙어 있다. XbSymbolDatabase가 식별하지 않는다(D3DX는 XDK 라이브러리 목록에 없다). 시그니처와 동작이 Direct3D 8 D3DX의 `D3DXVec4Transform`, `D3DXMatrixMultiply`, `D3DXMatrixInverse`와 같으므로 그 정적 링크본으로 **추정**한다. 셋 다 stdcall 3인자(`ret 0xC`)이고 `eax`에 결과 포인터를 돌려준다. 행렬은 64바이트 행 우선 4×4 float이고, 벡터는 행 벡터다(`v' = x·R0 + y·R1 + z·R2 + w·R3`).

| 주소 | 크기 | 호출 지점 | 명령 |
|---|---|---|---|
| `000E4B73` | 70 B | 182 | SSE(`movups`, `shufps`, `mulps`, `addps`) |
| `000E4BB9` | 254 B | 258 | SSE(`movaps`: 16바이트 정렬 전제) |
| `000E4CB7` | 909 B | 11 | x87, ebp 프레임 0x54바이트, `_finite`(`000E6FA1`) 호출 |
| `000E6FA1` | 21 B | 1 | CRT `_finite(double)`(cdecl): 지수 비트가 모두 1이 아니면 1 |

## 2. `000E4B73`: `D3DXVec4Transform(pOut, pV, pM)`

* `pV`의 네 성분을 한 번에 읽고, `x·R0`, `y·R1`, `z·R2`, `w·R3`를 lane별로 곱한다(브로드캐스트한 성분이 첫 피연산자).
* 합의 순서는 `((x·R0 + y·R1) + z·R2) + w·R3`.
* 입력을 모두 읽은 뒤 `pOut`에 16바이트를 쓰므로 `pOut = pV`여도 된다. `movups`라 정렬은 필요 없다.
* 남는 레지스터: `eax = pOut`, `xmm1 = y·R1`, `xmm2 = 결과`, `xmm3 = z·R2`, `xmm4 = w·R3`. 다른 GPR과 XMM은 그대로.

## 3. `000E4BB9`: `D3DXMatrixMultiply(pOut, pA, pB)`

* 결과 행 r마다 `((a·B0 + b·B1) + c·B2) + d·B3`(a~d는 A의 r행 성분). 원본은 곱을 `a·B0`, `b·B1`, `c·B2` 순으로 만들고 더한 뒤 `d·B3`을 만든다. 결과는 같은 순서의 합이다.
* A의 각 행을 네 번(성분마다) 다시 읽지만 그 사이에 쓰기가 없다. 네 행을 모두 계산한 뒤 쓰므로 `pOut = pA` 또는 `pOut = pB`여도 된다.
* 남는 레지스터: `eax = ecx = pOut`, `xmm2..5 = 결과 행`, `xmm0 = A3.z·B2`, `xmm1 = A3.w·B3`.

## 4. `000E4CB7`: `D3DXMatrixInverse(pOut, pDeterminant, pM)`

계산(m0~m15는 `pM`의 float, 행 우선):

1. 열 0·1의 2×2 소행렬식 6개, float로 저장: `lo01 = m5·m0 − m4·m1`, `lo02 = m9·m0 − m8·m1`, `lo03 = m13·m0 − m12·m1`, `lo12 = m9·m4 − m8·m5`, `lo13 = m13·m4 − m12·m5`, `lo23 = m13·m8 − m12·m9`.
2. 그것으로 역행렬의 행 2·3 여인수 8개(`c8`~`c15`), float로 저장.
3. 열 2·3의 소행렬식: `hi01 = m2·m7 − m6·m3`, `hi02 = m2·m11 − m10·m3`는 float로 저장한다. `hi12 = m6·m11 − m10·m7`, `hi13 = m6·m15 − m14·m7`, `hi03 = m2·m15 − m14·m3`, `hi23 = m10·m15 − m14·m11`은 x87 스택에만 둔다(저장 반올림 없음).
4. 그것으로 행 0·1 여인수 8개(`c0`~`c7`), float로 저장.
5. `D = ((c3·m12 + c2·m8) + c1·m4) + c0·m0`. `pDeterminant != NULL`이면 `(float)D`를 쓴다.
6. `inv = (float)(1 / D)`. `_finite((double)inv)`가 0이면 `pOut`을 건드리지 않고 0을 돌려준다(특이 행렬: 1/0 = 무한대).
7. 아니면 `pOut[k] = (float)(c_k · inv)`, k = 0..15 순서, `pOut`을 돌려준다.

곱셈의 피연산자 순서는 원본 그대로다(예: `c5`의 첫 항만 `m0·hi23`이고 나머지는 `hi·m` 순서). 모든 x87 연산은 정밀도 제어(PC)를 따른다. lifter는 x87 스택을 `double`로 흉내 내고 연산마다 `RECOMP_FP_PC`로 반올림한다. PC=24(Xbox가 쓰는 값)면 float로 반올림하되 지수 범위는 x87대로 유지하고(`recomp_fp_round24`), PC=53/64면 그대로 둔다. `rexbox_recomp_reset_thread`의 기본값은 `0x037F`(PC=64)다.

부수 효과(호출 시 `esp`를 E라 하면, 프레임 `ebp = E−4`):

* 프레임 슬롯 21개의 최종값: `ebp−0x54..−0x3C`는 `c15..c9`, `−0x38`은 m0의 원시 비트, `−0x34` `c0`, `−0x30` `c4`, `−0x2C` `c8`, `−0x28` `c1`, `−0x24` `c2`, `−0x20` `c5`, `−0x1C/−0x18/−0x14`는 m4/m8/m12의 원시 비트, `−0x10` `c3`, `−0xC` `hi01`, `−0x8` `inv`, `−0x4` `c6`, `+0x10`(인자 `pM`의 슬롯) `c7`. 그리고 `[E−4]`에 저장된 `ebp`.
* `_finite` 호출: `[E−0x60]`에 `(double)inv` qword, `[E−0x64]`에 반환 주소 `0x000E4FAF`.
* 레지스터: `eax = pOut` 또는 0, `ecx = (double)inv`의 상위 dword(호출 뒤 `pop ecx` 두 번), `edx`는 그대로. `g_ebp = g_seh_ebp = E−4`, `esp = E+16`.
* x87: 스택 8칸을 모두 쓰고 top은 들어올 때와 같다. 따라서 호출 시점에 x87 스택이 비어 있어야 한다(실기에서도 그렇지 않으면 스택 오버플로).

## 5. 확인

`rect3_decomp_check`는 디컴파일 버전과 lift 본문을 같은 상태에서 돌려 GPR, `g_seh_ebp`, XMM0~7, x87 제어어·top, 게스트 메모리 전체를 비교한다. 경우는 50개다. 무작위 입력은 유효 비트 24개를 모두 쓴다. 그 밖에 제자리 연산, 정렬되지 않은 벡터, NaN·무한대·신호 NaN, 역행렬의 PC=24/53/64별 무작위·강체 변환·특이 행렬·큰 값(곱이 float 범위를 넘음)·작은 값, `pDeterminant` NULL, x87 top이 0이 아닌 빈 스택을 넣었다. 덧셈 묶음 변경, 프레임 슬롯 누락, x87 중간 반올림 생략을 일부러 넣으면 각각 차이로 잡힌다.

---

# Three D3DX matrix functions (issue #27)

Basis: the disassembly (`gen/ct3_jp/disasm/asm/text.asm`), the lifted bodies (`sub_X_gen`) and run-time checks (`rect3_decomp_check`). Design: [20261026-i027-matrix-decomp](../design/20261026-i027-matrix-decomp.md).

## 1. Identity

All three sit together in `.text` at `000E4B73`–`000E5044`, unidentified by XbSymbolDatabase (D3DX is not among the XDK libraries). Their signatures and behaviour match Direct3D 8's D3DX `D3DXVec4Transform`, `D3DXMatrixMultiply` and `D3DXMatrixInverse`, so they are **inferred** to be its statically linked copies. All are stdcall with three arguments (`ret 0xC`) and return the result pointer in `eax`. Matrices are 64-byte row-major 4×4 floats; vectors are row vectors (`v' = x·R0 + y·R1 + z·R2 + w·R3`). The table above gives sizes, call sites and instructions; `000E6FA1` is the CRT's `_finite(double)` (cdecl): 1 unless every exponent bit is set.

## 2. `000E4B73`: `D3DXVec4Transform(pOut, pV, pM)`

Reads the four components of `pV` at once and multiplies `x·R0`, `y·R1`, `z·R2`, `w·R3` lane by lane (the broadcast component first), summing `((x·R0 + y·R1) + z·R2) + w·R3`. It writes 16 bytes to `pOut` after reading every input, so `pOut = pV` is fine; `movups` needs no alignment. It leaves `eax = pOut`, `xmm1 = y·R1`, `xmm2 = the result`, `xmm3 = z·R2`, `xmm4 = w·R3`, and the other GPRs and XMM registers unchanged.

## 3. `000E4BB9`: `D3DXMatrixMultiply(pOut, pA, pB)`

Each result row r is `((a·B0 + b·B1) + c·B2) + d·B3` for row r's components a–d of A; the original forms `a·B0`, `b·B1`, `c·B2`, adds, and then forms `d·B3`, giving the sum in that order. It reads each row of A four times (once per component) with no store in between, and writes the four rows after computing them all, so `pOut = pA` or `pOut = pB` is fine. It leaves `eax = ecx = pOut`, `xmm2..5 = the result rows`, `xmm0 = A3.z·B2`, `xmm1 = A3.w·B3`.

## 4. `000E4CB7`: `D3DXMatrixInverse(pOut, pDeterminant, pM)`

Computation (m0–m15 are `pM`'s floats, row-major): (1) six 2×2 minors of columns 0–1, stored as floats (`lo01 = m5·m0 − m4·m1` through `lo23 = m13·m8 − m12·m9`); (2) from them the eight cofactors of inverse rows 2–3 (`c8`–`c15`), stored; (3) the minors of columns 2–3: `hi01 = m2·m7 − m6·m3` and `hi02 = m2·m11 − m10·m3` stored as floats, `hi12`, `hi13`, `hi03` and `hi23` kept on the x87 stack only (no store rounding); (4) from them the eight cofactors of rows 0–1 (`c0`–`c7`), stored; (5) `D = ((c3·m12 + c2·m8) + c1·m4) + c0·m0`, with `(float)D` written when `pDeterminant != NULL`; (6) `inv = (float)(1 / D)`, and when `_finite((double)inv)` is 0 it returns 0 leaving `pOut` alone (a singular matrix: 1/0 is infinite); (7) otherwise `pOut[k] = (float)(c_k · inv)` for k = 0..15 in order, returning `pOut`. Multiplication operands keep the original's order (for example only `c5`'s first term is `m0·hi23`; the others are `hi·m`). Every x87 operation follows the precision control (PC): the lifter models the x87 stack in `double` and rounds each operation with `RECOMP_FP_PC`, to float at PC=24 (the Xbox's setting) while keeping the x87's exponent range (`recomp_fp_round24`), unchanged at PC=53/64; `rexbox_recomp_reset_thread` defaults to `0x037F` (PC=64).

Side effects, with `esp` on entry E and the frame `ebp = E−4`: the final values of 21 frame slots as listed above (`c15..c9` at `ebp−0x54..−0x3C`, raw bits of m0 at `−0x38` and of m4/m8/m12 at `−0x1C/−0x18/−0x14`, `inv` at `−0x8`, `c7` in `pM`'s argument slot at `+0x10`, and so on) plus the saved `ebp` at `[E−4]`; for the `_finite` call the qword `(double)inv` at `[E−0x60]` and the return address `0x000E4FAF` at `[E−0x64]`; `eax = pOut` or 0, `ecx` = the high dword of `(double)inv` (the caller's two `pop ecx` after the call), `edx` unchanged, `g_ebp = g_seh_ebp = E−4`, `esp = E+16`. It uses all eight x87 registers and leaves the top as it found it, so the x87 stack must be empty on entry (otherwise the hardware overflows too).

## 5. Checks

`rect3_decomp_check` runs the decompiled version and the lifted body from the same state and compares the GPRs, `g_seh_ebp`, XMM0–7, the x87 control word and top, and all of guest memory, over 50 cases: random inputs with full 24-bit significands, in-place use, an unaligned vector, NaN, infinity and a signalling NaN, and for the inverse random, rigid-transform, singular, large (products beyond float's range) and small matrices at PC=24/53/64, a NULL `pDeterminant` and an empty x87 stack whose top is not 0. A regrouped sum, a missing frame slot and a skipped intermediate x87 rounding, injected on purpose, each show as differences.
