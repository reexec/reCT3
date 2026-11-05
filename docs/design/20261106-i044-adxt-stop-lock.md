# 설계: ADXF Close와 파일 서버 사이의 객체 경쟁 (issue #44)

상태: 설계(1단계 계측으로 원인 확인). 이전: [i042 CRI 서버 잠금](20261103-i042-cri-server-lock.md), [i029 ADXM 잠금](20261027-i029-adxm-lock.md). 분석: [cri-middleware](../analysis/cri-middleware.md).

## 1. 요구사항

rexbox v0.0.23으로 올린 뒤(#41), 숨긴 어트랙트 데모 6개 동시 실행 24번 가운데 2번에서 부팅 초기에 null 간접 호출이 기록됐다.

```
[recomp] indirect call to 00000000 is outside code; skipped (returns to 000B27B0)
```

실행은 계속됐다. v0.0.21 빌드로 같은 24번을 돌렸을 때는 0번이었다. 이 표본만으로 v0.0.23을 원인으로 볼 수는 없다. v0.0.23의 `RtlLeaveCriticalSection`이 첫 대기자에게 구간을 넘기면서 스레드 순서가 바뀌었고, 그 결과 원래 있던 경쟁이 더 자주 드러났을 수 있다. 서버의 호출이 null 대신 지워지기 전의 값에 닿으면, Close가 없애는 중인 객체를 서버가 한 번 더 닫게 된다.

## 2. 확인한 것 (**확인됨**, 임시 계측: `sub_000B1700`·`sub_000B2730`·`sub_000B2A90`의 진입·종료 기록, null 호출 때 덤프)

1. 파일 서버 `sub_000B9860`(thread 8)이 `sub_000B2730`을 부른다. 이 함수는 SVM 잠금(`sub_000B5870`/`sub_000B5890`) 안에서 `0x343520`의 ADXF 객체 표(10개, 간격 `0x138`)를 돈다. 읽는 중(상태 `+1` = 2)인 객체는 다음 순서로 처리한다.
   * `0x000B275D`에서 상태를 갱신한다(읽기 끝이면 3, 오류면 4).
   * 3이나 4이면 `0x000B277B`에서 `+8`(`[esi+7]`, `esi` = 객체 + 1)이 0이 아닌지 본다.
   * `[0x3430C4] == 1`이면 `sub_000C7A00`을 부른다.
   * `0x000B27A7`에서 `+8`을 다시 읽고, `0x000B27AD`에서 그 vtable의 `+0x0C`를 부른 뒤 `+8`을 0으로 지운다.
2. ADXF Close `sub_000B2A90`은 상태가 2일 때만 정지 `sub_000B2640`(안에서 SVM 잠금을 잡는다)을 부른다. 그 뒤 `+4`를 없애고(`sub_000B5810`), 객체 `0x138`바이트 전체를 `rep stosd`로 0으로 지운다. 이 부분에는 **잠금이 없다**.
3. 두 번 잡힌 null 호출 모두 같은 순서였다.
   * thread 8의 `sub_000B2730` 실행 도중, 게임 스레드가 객체 `0x00343520`에 `sub_000B2A90`을 불렀다.
   * 호출 지점은 `0x000B2D51`(반환 `0x000B2D56`, `sub_000B2DF0`)이다.
   * Close는 서버 실행 구간 안에서 시작하고 끝났다.
   * Close가 본 상태는 3이었다. 서버가 `0x000B275D`에서 막 바꾼 값이라 Close는 잠금 경로를 건너뛰었고, 객체를 지웠다. 서버의 두 번째 읽기는 0을 읽었다.
4. `sub_000B2DF0`은 부팅 때 AFS 파티션 표를 읽는다(`sub_000B2ED0` ← `sub_0003B5F0`). 임시 ADXF(`[0x343100]`)로 표를 읽고, 읽기가 끝나면(상태 3) 곧바로 파싱하고 닫는다.
5. ADXT 정지 `sub_000B1700`은 그동안 한 번도 불리지 않았다. 처음의 추정은 틀렸다.
6. Xbox의 단일 코어에서는 TIME_CRITICAL로 올라간 서버 실행이 게임 스레드에 선점되지 않았다. 그래서 Close가 서버 실행 도중에 끼어들 수 없었다.
7. Close 경로의 `sub_000B2640`, `sub_000B5810`, `sub_000B8A30`, `sub_000B8880`, `sub_000B5D00`(CRI 오류 보고)에는 회전 대기가 없다. 그래서 이 경로를 호스트 잠금 안에 넣어도 서버를 기다리며 돌지 않는다.

## 3. 방안

| 방안 | 내용 | 판단 |
|---|---|---|
| **A. `sub_000B2A90` 전체를 ADXM 호스트 잠금으로** | Close는 진행 중인 파일 서버·레벨 2 서버 실행이 끝날 때까지 기다린다. 안에서 `sub_000B2640`이 SVM 잠금을 다시 잡아도, 같은 스레드이므로 잠금이 재진입된다 | **채택** |
| B. `sub_000B2730`에서 `+8`을 한 번만 읽게 | lift 결과를 바꾸는 디컴파일 | 기각. null만 피할 뿐, 서버가 지워지는 중인 객체를 다루는 경쟁은 남는다 |
| C. `sub_000B2DF0`의 호출만 잠금 | 잡힌 경로 하나 | 기각. Close의 다른 호출자 5곳(`sub_000AE5A0`, `sub_000B2B60`, `sub_000B2B90`, `sub_000B2BC0`, `sub_000B2ED0`)에도 같은 틈이 있다 |
| D. `sub_000B1700`을 잠금으로(처음 설계) | | 기각. 계측에서 불리지 않았다 |

## 4. 구조 (`src/game/ct3/decomp/adxm_lock.c`)

* `sub_000B2A90`을 정의한다. `g_adxm_lock`을 잡고 lift 원본(`sub_000B2A90_gen`)을 부른 뒤 놓는다. #42의 `sub_000B7EE0`과 같은 형태다. 디컴파일이 아니라 호스트 동시성 어댑터다.
* 잠금 순서는 ADXM → SJ로, #42와 같다. 정지 `sub_000B2640`이 잡는 SVM 잠금 콜백(`sub_000B2FF0`)은 같은 호스트 잠금을 재진입으로 잡는다.
* `sub_000B8420`(ADXF 종료)의 표 전체 지우기는 그대로 둔다. 그 전에 `sub_000B2B60`이 모든 객체를 Close로 닫는다.

## 5. 검증

* 재생성, Win32 x86 Release 빌드, CTest.
* 계측 없는 최종 빌드로 숨긴 어트랙트 데모 6개 동시 실행 4묶음(24번) 이상을 돌린다.
* null 호출·음악 멈춤·다른 스트림 멈춤·stall·잠금 시한을 수정 전과 비교한다. 수정 전 v0.0.23에서는 계측 포함 84번 가운데 null 호출이 6번이었다.
* 운전 진입도 확인한다.

---

# Design: the object race between ADXF Close and the file server (issue #44)

Status: design (cause confirmed with step 1's instrumentation). Earlier: [i042 CRI server lock](20261103-i042-cri-server-lock.md), [i029 ADXM lock](20261027-i029-adxm-lock.md). Analysis: [cri-middleware](../analysis/cri-middleware.md).

## 1. Requirement

After the update to rexbox v0.0.23 (#41), 2 of 24 runs of six hidden attract demos in parallel logged a null indirect call early in boot:

```
[recomp] indirect call to 00000000 is outside code; skipped (returns to 000B27B0)
```

The runs went on. The same 24 runs on a v0.0.21 build logged none, but that sample is too small to blame v0.0.23. Its `RtlLeaveCriticalSection` now hands the section to the first waiter, which changes the thread order and may only make an old race show more often. When the server's call lands on the value from before the clear instead of null, the server closes again an object that Close is tearing down.

## 2. Findings (**confirmed** with temporary instrumentation: entry and exit records of `sub_000B1700`, `sub_000B2730` and `sub_000B2A90`, dumped at the null call)

1. The file server `sub_000B9860` (thread 8) calls `sub_000B2730`, which loops over the ADXF objects at `0x343520` (10 of them, stride `0x138`) inside the SVM lock (`sub_000B5870`/`sub_000B5890`). For an object that is reading (state `+1` = 2) it works in this order:
   * at `0x000B275D` it updates the state (3 when the read is done, 4 on error);
   * on 3 or 4, at `0x000B277B` it tests that `+8` (`[esi+7]`, `esi` = object + 1) is nonzero;
   * if `[0x3430C4] == 1` it calls `sub_000C7A00`;
   * at `0x000B27A7` it reads `+8` again, calls its vtable `+0x0C` at `0x000B27AD`, then clears `+8`.
2. ADXF Close `sub_000B2A90` calls the stop `sub_000B2640` (which takes the SVM lock inside) only when the state is 2. It then destroys `+4` (`sub_000B5810`) and zeroes the whole `0x138`-byte object with `rep stosd`. That part takes **no lock**.
3. Both null calls caught showed the same order:
   * during thread 8's `sub_000B2730` pass, the game thread called `sub_000B2A90` on object `0x00343520`;
   * the call site is `0x000B2D51` (return `0x000B2D56`, in `sub_000B2DF0`);
   * the Close started and ended inside the server pass;
   * the Close saw state 3, the value the server had just written at `0x000B275D`, so it skipped the locked path and zeroed the object, and the server's second read found 0.
4. `sub_000B2DF0` reads the AFS partition table at boot (`sub_000B2ED0` ← `sub_0003B5F0`). It reads the table through a temporary ADXF (`[0x343100]`), and as soon as the read is done (state 3) it parses the table and closes the handle.
5. ADXT's stop `sub_000B1700` was never called meanwhile; the first suspicion was wrong.
6. On the Xbox's single core the server pass, raised to TIME_CRITICAL, was not preempted by the game thread, so a Close could not land inside it.
7. The Close path's `sub_000B2640`, `sub_000B5810`, `sub_000B8A30`, `sub_000B8880` and `sub_000B5D00` (CRI's error report) do not spin. So putting this path under the host lock does not make it spin waiting for the server.

## 3. Options

| Option | What | Decision |
|---|---|---|
| **A. All of `sub_000B2A90` under the ADXM host lock** | a Close waits for a running file-server or level-2 pass to end; the SVM lock that `sub_000B2640` takes inside is re-entered on the same thread | **Chosen** |
| B. Read `+8` once in `sub_000B2730` | a decompilation that changes the lifted logic | Rejected: it only avoids the null; the server still works on an object being torn down |
| C. Lock only `sub_000B2DF0`'s call | the one path caught | Rejected: Close's other five callers (`sub_000AE5A0`, `sub_000B2B60`, `sub_000B2B90`, `sub_000B2BC0`, `sub_000B2ED0`) have the same gap |
| D. `sub_000B1700` under the lock (the first design) | | Rejected: the instrumentation never saw it called |

## 4. Structure (`src/game/ct3/decomp/adxm_lock.c`)

* Define `sub_000B2A90`: take `g_adxm_lock`, call the lifted original (`sub_000B2A90_gen`), release it. The same shape as #42's `sub_000B7EE0`; a host-concurrency adapter, not a decompilation.
* The lock order stays ADXM → SJ, as in #42. The SVM lock callback (`sub_000B2FF0`) that the stop `sub_000B2640` takes re-enters the same host lock.
* The whole-table clear in `sub_000B8420` (ADXF finish) stays as it is; `sub_000B2B60` closes every object through Close before it.

## 5. Verification

* Regeneration, Win32 x86 Release build, CTest.
* At least 4 batches (24 runs) of six hidden attract demos in parallel with the final build without instrumentation.
* Compare null calls, music stops, other stream stops, stalls and lock timeouts with the runs before the fix. Before the fix, v0.0.23 showed 6 null calls in 84 runs, instrumented ones included.
* Check a drive into gameplay.
