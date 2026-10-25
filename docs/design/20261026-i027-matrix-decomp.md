# 설계: 운전 화면의 행렬 함수 세 개 디컴파일 (issue #27)

## 1. 문제

issue #17의 측정에서 운전 화면의 lift된 코드(프레임당 약 9 ms) 중 가장 많은 표본이 행렬 함수 세 개에 있었다. 셋 다 `.text`에 정적 링크된 라이브러리 코드이고 XbSymbolDatabase가 식별하지 않는다. lift된 본문은 계산보다 게스트 레지스터(TLS), XMM lane 흉내, x87 스택 흉내에 더 많은 비용을 쓴다.

| 주소 | 크기 | 호출 지점 | 하는 일(**확인됨**, 디스어셈블리) | 원본(**추정**) |
|---|---|---|---|---|
| `000E4B73` | 70 B | 182 | `out = v · M`: 4성분 벡터 × 4×4 행렬. SSE | `D3DXVec4Transform(pOut, pV, pM)` |
| `000E4BB9` | 254 B | 258 | `out = A · B`: 4×4 행렬 곱. SSE, `movaps`(16바이트 정렬 전제) | `D3DXMatrixMultiply(pOut, pA, pB)` |
| `000E4CB7` | 909 B | 11 | 역행렬과 행렬식. x87, 여인수(2×2 소행렬식) 방식 | `D3DXMatrixInverse(pOut, pDeterminant, pM)` |

세 함수 모두 stdcall 3인자(`ret 0xC`)이고 `eax`에 결과 포인터를 돌려준다(역행렬은 실패하면 0). 행렬은 64바이트 행 우선 4×4 float이고, 벡터는 행 벡터다.

## 2. 의미 (분석)

자세한 분석은 [docs/analysis/d3dx-matrix.md](../analysis/d3dx-matrix.md)에 있다. 핵심은 다음과 같다.

* **`000E4B73`**: `v`를 한 번에 읽고 `x·R0`, `y·R1`, `z·R2`, `w·R3`를 lane별로 곱한 뒤 `((x·R0 + y·R1) + z·R2) + w·R3` 순서로 더해 `out`에 쓴다. 남는 것: `xmm1 = y·R1`, `xmm2 = 결과`, `xmm3 = z·R2`, `xmm4 = w·R3`, `eax = pOut`.
* **`000E4BB9`**: 결과 행마다 `((a·B0 + b·B1) + c·B2) + d·B3`. 네 행을 모두 계산한 뒤 쓰므로 `pOut`이 `pA`나 `pB`와 같아도 된다. 남는 것: `xmm2..5 = 결과 행`, `xmm0 = A3.z·B2`, `xmm1 = A3.w·B3`, `eax = ecx = pOut`.
* **`000E4CB7`**: 열 0·1의 2×2 소행렬식 6개와 열 2·3의 소행렬식 6개(그중 2개는 float로 저장, 4개는 x87 스택에만)로 여인수 16개를 만들고, 행렬식 `D`를 계산한다. `pDeterminant`가 NULL이 아니면 `(float)D`를 쓴다. `inv = (float)(1/D)`를 CRT `_finite`(`000E6FA1`)로 검사해, 유한하면 `pOut[k] = (float)(cofactor_k · inv)`를 쓰고 `pOut`을, 아니면 0을 돌려준다. x87의 연산 하나하나가 정밀도 제어(PC)를 따른다. lifter는 x87을 `double`로 흉내 내고 연산마다 `RECOMP_FP_PC`로 반올림한다(PC=24면 float로).
* **부수 효과(역행렬)**: 프레임 슬롯 21개의 최종값(원시 비트 복사 4개 포함), 인자 슬롯 `[esp+12]`(이 함수가 덮어씀), 저장된 `ebp`, `_finite` 인자 qword와 반환 주소 `0x000E4FAF`. `ecx`는 `(double)inv`의 상위 dword, `g_ebp`와 `g_seh_ebp`는 이 함수의 프레임(`esp-4`), x87 top은 들어올 때와 같다.

## 3. 설계

* `src/game/ct3/decomp/d3dx_matrix.c`에 세 함수를 같은 심볼로 정의한다. lift 본문은 `extern void sub_XXXXXXXX_gen(void);` 선언으로 `_gen` 이름에 남는다(lifter의 `--exclude-manual`).
* **같은 결과**: 일반 C(intrinsic 없음), 같은 연산·순서·묶음·피연산자 순서. SSE 쪽은 lane별 float 곱셈·덧셈이고 IEEE 단정밀도라 lift된 lane C와 비트까지 같다. x87 쪽은 `double`로 계산하고 lift 본문이 반올림하는 곳마다 같은 반올림(`recomp_fp_round24`, 정밀도 제어 비트로 선택)과 float 저장 변환을 한다. 같은 값을 다시 쓰는 곳(스택 복사)은 한 번 계산해 재사용한다.
* **같은 부수 효과**: 위의 레지스터, XMM, 메모리(esp 아래의 저장 값과 반환 주소, 인자 슬롯 포함), `g_ebp`, `g_seh_ebp`. `_finite`는 인라인으로 계산하고 그 반환 주소 push와 인자 qword를 메모리에 남긴다.
* **같은 읽기 순서**: 입력을 모두 읽은 뒤 결과를 쓴다(원본과 같음). 원본이 같은 입력을 두 번 읽는 곳은 그 사이에 쓰기가 없으므로 한 번만 읽는다.
* **x87 스택**: 역행렬은 x87 스택 8칸을 모두 쓰므로, 원본도 빈 x87 스택에서만 올바르다. 디컴파일은 top을 그대로 두고, 원본의 push가 8칸에 남기는 마지막 값(pop된 뒤에도 남는 값)을 같게 남긴다. 처음에는 죽은 값이라 두지 않았으나, 운전 측정 중 드문 멈춤 1회(아래)가 나와 비교하지 않는 차이를 없앴다. 호출자가 x87 스택에 값을 남긴 채 부르는 경우(원본에서도 스택 오버플로)는 같게 만들지 않는다.
* **검사 도구 `rect3_decomp_check`**(`tools/decomp_check/`): 디컴파일 함수와 `_gen`을 같은 게스트 상태에서 돌려 GPR, `g_ebp`·`g_seh_ebp`, XMM0~7, x87 top·제어어·슬롯 8칸(비트 단위), 게스트 메모리 전체를 비교한다. x87·SEH 상태는 생성 코드의 헤더가 필요하므로 C 보조 파일로 읽고 쓴다. 경우: 의사 난수 입력 여러 개, 제자리 연산(`pOut = pA`, `pOut = pB`, `pOut = pV`), 정렬되지 않은 벡터, 역행렬의 특이 행렬(행렬식 0 → 실패 경로)·`pDeterminant` NULL·PC=24/53/64·NaN·무한대·아주 큰/작은 값. CTest에 등록한다(생성 코드가 있을 때만 빌드).

### 위험

* 컴파일러가 곱셈·덧셈을 FMA로 합치면 lift 본문과 달라질 수 있다. 지금의 x86/x64 기본 대상에는 FMA가 없다. ARM(Android) 때 다시 본다.
* NaN 페이로드: 곱셈 피연산자 순서를 원본과 같게 둔다.

## 4. 검증

| 검증 | 기대 |
|---|---|
| `rect3_decomp_check` | 모든 경우 차이 0(Win32 Debug·Release, Linux x64) |
| 재생성 | 세 함수가 `--exclude-manual`로 빠지고 `_gen`이 남는다 |
| 회귀 | CTest 전체, 어트랙트 데모·운전 프레임 덤프가 이전과 같아 보인다 |
| 성능 | `drive_west_coast.ps1`로 기준 빌드와 번갈아 운전 구간 측정: 표시 스레드 cpu와 `other`, 세 함수의 표본 비율 |

---

# Design: decompile the three matrix functions of the driving frame (issue #27)

## 1. Problem

In issue #17's measurement the most samples of the driving frame's lifted code (about 9 ms per frame) sat in three matrix functions. All three are statically linked library code in `.text`, unidentified by XbSymbolDatabase, and their lifted bodies spend more on guest registers (TLS), XMM-lane emulation and x87-stack emulation than on arithmetic.

| Address | Size | Call sites | What it does (**confirmed**, disassembly) | Original (**inferred**) |
|---|---|---|---|---|
| `000E4B73` | 70 B | 182 | `out = v · M`, a 4-vector times a 4×4 matrix; SSE | `D3DXVec4Transform(pOut, pV, pM)` |
| `000E4BB9` | 254 B | 258 | `out = A · B`, a 4×4 product; SSE, `movaps` (16-byte alignment assumed) | `D3DXMatrixMultiply(pOut, pA, pB)` |
| `000E4CB7` | 909 B | 11 | inverse and determinant; x87, by cofactors (2×2 minors) | `D3DXMatrixInverse(pOut, pDeterminant, pM)` |

All three are stdcall with three arguments (`ret 0xC`) and return the result pointer in `eax` (the inverse returns 0 on failure). Matrices are 64-byte row-major 4×4 floats; vectors are row vectors.

## 2. Semantics (analysis)

The full analysis is in [docs/analysis/d3dx-matrix.md](../analysis/d3dx-matrix.md). In short: `000E4B73` reads `v` once, multiplies `x·R0`, `y·R1`, `z·R2`, `w·R3` lane by lane and adds `((x·R0 + y·R1) + z·R2) + w·R3` into `out`, leaving `xmm1 = y·R1`, `xmm2 = result`, `xmm3 = z·R2`, `xmm4 = w·R3`, `eax = pOut`. `000E4BB9` computes each row as `((a·B0 + b·B1) + c·B2) + d·B3` and writes all four after computing them, so `pOut` may equal `pA` or `pB`; it leaves `xmm2..5 = the rows`, `xmm0 = A3.z·B2`, `xmm1 = A3.w·B3`, `eax = ecx = pOut`. `000E4CB7` builds six 2×2 minors of columns 0–1 and six of columns 2–3 (two stored as floats, four kept on the x87 stack), sixteen cofactors and the determinant `D`; it writes `(float)D` when `pDeterminant` is not NULL, checks `inv = (float)(1/D)` with the CRT's `_finite` (`000E6FA1`), and when finite writes `pOut[k] = (float)(cofactor_k · inv)` and returns `pOut`, else returns 0. Every x87 operation follows the precision control (PC); the lifter models the x87 in `double` and rounds every operation with `RECOMP_FP_PC` (to float at PC=24). Side effects of the inverse: the final values of 21 frame slots (four of them raw bit copies), the argument slot `[esp+12]` (which it overwrites), the saved `ebp`, the `_finite` argument qword and the return address `0x000E4FAF`; `ecx` is the high dword of `(double)inv`, `g_ebp` and `g_seh_ebp` are its frame (`esp-4`), and the x87 top is as on entry.

## 3. Design

* The three functions are defined under their own symbols in `src/game/ct3/decomp/d3dx_matrix.c`; the lifted bodies stay under `_gen` names through `extern void sub_XXXXXXXX_gen(void);` (the lifter's `--exclude-manual`).
* **Same results**: plain C (no intrinsics) with the same operations, order, grouping and operand order. The SSE side is lane-wise float multiplication and addition in IEEE single precision, bit-identical to the lifted lane C. The x87 side computes in `double` and rounds wherever the lifted body does (`recomp_fp_round24`, chosen by the precision-control bits) and converts on float stores the same way; a value the original copies on the stack is computed once and reused.
* **Same side effects**: the registers, XMM, memory (the stores and return address below esp and the argument slot), `g_ebp` and `g_seh_ebp` above. `_finite` is computed inline, leaving its return-address push and argument qword in memory.
* **Same read order**: every input is read before any result is written (as in the original); where the original reads an input twice with no store in between, it is read once.
* **x87 stack**: the inverse uses all eight x87 slots, so the original is correct only on an empty x87 stack. The decompiled version leaves the top unchanged and the eight slots holding the last values the original's pushes leave (values that stay after a pop); at first it left them alone as dead values, but one rare stall in the driving measurements (below) led to removing every uncompared difference. A caller that leaves values on the x87 stack (a stack overflow in the original too) is not matched.
* **Checker `rect3_decomp_check`** (`tools/decomp_check/`): runs each decompiled function and its `_gen` from the same guest state and compares the GPRs, `g_ebp` and `g_seh_ebp`, XMM0–7, the x87 top, control word and eight slots (bit for bit), and all of guest memory. The x87 and SEH state need the generated code's header, so a C helper reads and writes them. Cases: several pseudo-random inputs, in-place use (`pOut = pA`, `pOut = pB`, `pOut = pV`), an unaligned vector, and for the inverse a singular matrix (determinant 0, the failure path), a NULL `pDeterminant`, PC=24/53/64, NaN, infinity and very large and small values. Registered with CTest (built only with generated code).

Risks: a compiler that fuses multiply and add into FMA would differ from the lifted body; the current x86/x64 baseline targets have no FMA, to be revisited for ARM (Android). NaN payloads: multiplication operands keep the original's order.

## 4. Verification

`rect3_decomp_check` with 0 differences in every case (Win32 Debug and Release, Linux x64); regeneration drops the three through `--exclude-manual` and keeps the `_gen` bodies; regression: all of CTest, and attract-demo and driving frame dumps that look as before; performance: the driving span with `drive_west_coast.ps1`, alternating with the baseline build: the presenting thread's cpu and `other`, and the three functions' share of the samples.
