# 작업 로그: ADXF Close와 파일 서버 사이의 객체 경쟁 (issue #44)

작업 지시: [20261106-i044](../work-orders/20261106-i044-adxt-stop-lock.md) · 설계: [20261106-i044](../design/20261106-i044-adxt-stop-lock.md) · 분석: [cri-middleware](../analysis/cri-middleware.md) §5

## 한 일

1. 발견: #41에서 rexbox를 v0.0.23(`25587f7`)으로 올린 뒤, 숨긴 데모 6개 동시 실행 24번 가운데 2번에서 부팅 초기에 `000B27B0`으로 돌아가는 null 간접 호출이 나왔다. v0.0.21 빌드로 같은 24번을 돌렸을 때는 0번이었다. 이 브랜치는 rexbox 갱신 커밋 `7211b8e` 위에 만들었다.
2. 첫 추정은 ADXT 정지 `sub_000B1700`이었다. 이 함수는 SVM 잠금을 잡기 전에 같은 필드를 지운다. 임시 계측(커밋하지 않음)으로 확인해 보니, null 호출 때 `sub_000B1700`은 한 번도 불리지 않았다. 그 대신 서버 실행 동안 객체 `0x00343520`이 통째로 0이 됐다(상태 2→0, `+8` `0x003448A0`→0).
3. 객체를 통째로 지우는 코드를 찾았다. ADXF Close `sub_000B2A90`이 잠금 없이 `rep stosd`로 0x138바이트를 지운다. 상태가 2일 때만 잠금을 잡는 정지 `sub_000B2640`을 거친다.
4. 원인(**확인됨**): 계측을 `sub_000B2A90`에도 넣고 12번 돌렸다. null 호출이 2번 잡혔는데, 두 번 모두 다음과 같았다.
   * 게임 스레드의 Close(호출 `0x000B2D51`, `sub_000B2DF0`: 부팅 때 AFS 파티션 표 읽기)가 thread 8의 서버 실행 구간 안에서 시작하고 끝났다.
   * 대상은 같은 객체 `0x00343520`이었다.
   * Close가 본 상태는 3이었다. 서버가 막 바꾼 값이라 잠금 경로를 건너뛰었다.
5. 고침: `src/game/ct3/decomp/adxm_lock.c`에 `sub_000B2A90`을 두어 ADXM 호스트 잠금 아래에서 돌게 했다. 코더 커밋 `be3bc75`이고, 계측은 모두 걷어냈다. 설계와 작업 지시를 확인된 원인으로 고쳤다(`2b9eb80`). 분석 §5, `decomp/README.md`, ARCHITECTURE도 갱신했다(`4195b8e`).
6. 역할: 계측과 코드 변경은 코더 세션이 했다. 계측 설계, 검토, 측정, 문서는 팀장 세션이 했다.

## 검증

| 빌드 (숨긴 데모 6개 동시, 130초) | 실행 | null 호출(`000B27B0`) | 음악 멈춤 | 다른 스트림 멈춤 | stall | 잠금 시한 |
|---|---|---|---|---|---|---|
| rexbox v0.0.21(기준) | 24 | 0 | 0 | 1 | 0 | 0 |
| rexbox v0.0.23, 수정 전 | 24 | 2 | 0 | 0 | 0 | 0 |
| v0.0.23, 수정 전(계측) | 18 | 4 | 0 | 0 | 0 | 0 |
| v0.0.23, 수정 후(최종) | 48 | 0 | 0 | 0 | 0 | 0 |

* 재생성, Win32 x86 Release 빌드 경고 0, CTest 7/7.
* 수정 전 v0.0.23의 null 호출은 42번 중 6번(약 14%)이었다. 그 빈도라면 48번이 모두 0일 확률은 0.1% 아래다.
* 최종 빌드 운전 진입: West Coast 운전 구간(draw 약 800)에서 59.5~59.8 fps였고, 스트림 멈춤·null 호출·잠금 시한·stall이 없었다.

## 남은 것

* 영상 구간의 null 간접 호출(`sub_000C9630`, 반환 `000C967E`)은 별개의 CRI 경합으로 보이며, 원인은 모른다.
* v0.0.21 기준선에서 25071 Hz 효과음 스트림(`03F691C0`) 멈춤이 24번 중 1번 나왔다. #42 때 기준선(42번 중 1번)과 같은 빈도다.

---

# Work log: the object race between ADXF Close and the file server (issue #44)

Work order: [20261106-i044](../work-orders/20261106-i044-adxt-stop-lock.md) · Design: [20261106-i044](../design/20261106-i044-adxt-stop-lock.md) · Analysis: [cri-middleware](../analysis/cri-middleware.md) section 5

## Done

1. Found: after #41 moved rexbox to v0.0.23 (`25587f7`), 2 of 24 runs of six hidden demos in parallel logged, early in boot, a null indirect call returning to `000B27B0`. The same 24 runs on a v0.0.21 build logged none. This branch sits on the rexbox update commit `7211b8e`.
2. The first suspect was ADXT's stop `sub_000B1700`, which clears the same field before it takes the SVM lock. Temporary instrumentation (not committed) showed `sub_000B1700` was never called at the null calls. Instead, object `0x00343520` was zeroed as a whole during the server pass (state 2→0, `+8` `0x003448A0`→0).
3. Looked for the code that zeroes the whole object: ADXF Close `sub_000B2A90` clears its 0x138 bytes with `rep stosd` and no lock. It goes through the stop `sub_000B2640`, which takes the lock, only while the state is 2.
4. Cause (**confirmed**): with instrumentation in `sub_000B2A90` too, 12 runs caught 2 null calls, and both showed:
   * the game thread's Close (call `0x000B2D51`, in `sub_000B2DF0`: reading the AFS partition table at boot) started and ended inside thread 8's server pass;
   * the target was the same object `0x00343520`;
   * the Close saw state 3, the value the server had just written, so it skipped the locked path.
5. Fix: `sub_000B2A90` in `src/game/ct3/decomp/adxm_lock.c` runs under the ADXM host lock (coder commit `be3bc75`, instrumentation removed). The design and work order were corrected to the confirmed cause (`2b9eb80`). Analysis section 5, `decomp/README.md` and ARCHITECTURE were updated (`4195b8e`).
6. Roles: the coder session wrote the instrumentation and the code change. The lead session designed the instrumentation, reviewed, measured and wrote the documents.

## Verification

| Build (six hidden demos in parallel, 130 s) | Runs | Null calls (`000B27B0`) | Music stops | Other stream stops | Stalls | Lock timeouts |
|---|---|---|---|---|---|---|
| rexbox v0.0.21 (baseline) | 24 | 0 | 0 | 1 | 0 | 0 |
| rexbox v0.0.23, before the fix | 24 | 2 | 0 | 0 | 0 | 0 |
| v0.0.23, before the fix (instrumented) | 18 | 4 | 0 | 0 | 0 | 0 |
| v0.0.23, after the fix (final) | 48 | 0 | 0 | 0 | 0 | 0 |

* Regeneration, Win32 x86 Release build with 0 warnings, CTest 7/7.
* Before the fix v0.0.23 had 6 null calls in 42 runs (about 14%); at that rate 48 clean runs have a chance below 0.1%.
* A drive into gameplay with the final build: 59.5–59.8 fps in the West Coast driving section (about 800 draws), with no stream stop, null call, lock timeout or stall.

## Remaining

* The null indirect call in the movie (`sub_000C9630`, returning to `000C967E`) looks like a separate CRI race; its cause is unknown.
* The v0.0.21 baseline stopped a 25071 Hz effect stream (`03F691C0`) in 1 of 24 runs, the same rate as #42's baseline (1 of 42).
