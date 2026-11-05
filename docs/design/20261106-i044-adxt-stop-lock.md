# 설계: ADXT 정지와 파일 서버 사이의 스트림 필드 경쟁 (issue #44)

상태: 설계. 이전: [i042 CRI 서버 잠금](20261103-i042-cri-server-lock.md), [i029 ADXM 잠금](20261027-i029-adxm-lock.md). 분석: [cri-middleware](../analysis/cri-middleware.md).

## 1. 요구사항

rexbox v0.0.23으로 올린 뒤(#41), 숨긴 어트랙트 데모 6개 동시 실행 24번 가운데 2번에서 부팅 초기에 null 간접 호출이 기록됐다.

```
[recomp] indirect call to 00000000 is outside code; skipped (returns to 000B27B0)
```

실행은 계속됐다. v0.0.21 빌드로 같은 24번을 돌렸을 때는 0번이었다. 이 표본만으로 v0.0.23을 원인으로 볼 수는 없다. v0.0.23의 `RtlLeaveCriticalSection`이 첫 대기자에게 구간을 넘기면서 스레드 순서가 바뀌었고, 그 결과 원래 있던 경쟁이 더 자주 드러났을 수 있다. 호출이 null 대신 살아 있는 객체에 닿으면 같은 스트림을 두 번 닫게 된다.

## 2. 코드에서 읽은 것 (**추정**, 1단계 계측으로 확인한다)

1. 파일 서버 `sub_000B9860`이 `sub_000B2730`을 부른다. 이 함수는 SVM 잠금(`sub_000B5870`/`sub_000B5890`) 안에서 `0x343520`의 ADXT 객체 표(간격 `0x138`)를 돈다. 디코더가 끝난 객체(상태 3 또는 4)를 만나면 다음 순서로 처리한다.
   * `0x000B277B`에서 스트림 필드 `+8`(`[esi+7]`, `esi` = 객체 + 1)이 0이 아닌지 본다.
   * `[0x3430C4] == 1`이면 `sub_000C7A00`을 부른다.
   * `0x000B27A7`에서 `+8`을 **다시 읽고**, `0x000B27AD`에서 그 vtable의 `+0x0C`를 부른 뒤 `+8`을 0으로 지운다.
   * 두 번째 읽기에서 null이 나왔다.
2. ADXT 정지 `sub_000B1700`은 SVM 잠금을 잡기 **전에** `+8`의 스트림을 멈추고 없앤 뒤(`sub_000B52E0`, `sub_000B5810`) `+8`을 0으로 지운다. 잠금은 끝의 `sub_000B16A0`에서만 잡는다. #42에서 본 입력 링 리셋도 이 잠금 밖에 있다.
3. 게임 코드가 `sub_000B1700`을 직접 부른다(`0x00083108`, `0x00083113`, `0x000837CA`, `0x000837EC`, `0x00083970`, `0x00083E96`, `0x00083EA2`). `sub_000B1E80`, `sub_000B1F80`, `sub_000B3860`, `sub_000B62B0`, `sub_000C1550`, `sub_000C1870`도 부른다.
4. Xbox의 단일 코어에서는 TIME_CRITICAL로 올라간 서버 실행이 게임 스레드에 선점되지 않았다. 그래서 두 읽기 사이에 정지가 끼어들 수 없었다. 호스트에서는 두 스레드가 동시에 돈다.
5. 정지 경로의 `sub_000B52E0`, `sub_000B5810`, `sub_000B8A30`, `sub_000B8880`에는 회전 대기가 없다. 장치의 정지·닫기 함수 포인터를 부를 뿐이다. 따라서 이 경로를 호스트 잠금 안에 넣어도 서버를 기다리며 돌지는 않는다. 잠금은 커널 대기 중에는 내려놓는다.

## 3. 방안

| 방안 | 내용 | 판단 |
|---|---|---|
| **A. `sub_000B1700` 전체를 ADXM 호스트 잠금으로** | 정지는 진행 중인 파일 서버·레벨 2 서버 실행이 끝날 때까지 기다린다. 끝의 `sub_000B16A0`이 SVM 잠금을 다시 잡아도, 같은 스레드이므로 잠금이 재진입된다 | **채택** (1단계 확인 뒤) |
| B. `sub_000B2730`에서 `+8`을 한 번만 읽게 | lift 결과를 바꾸는 디컴파일 | 기각. null만 피할 뿐, 정지와 서버가 같은 스트림을 함께 닫는 경쟁은 남는다 |
| C. 게임 쪽 호출 지점마다 잠금 | 13곳 | 기각. A와 효과가 같고 호출 지점이 많다 |

## 4. 구조

1단계 (임시 계측, 커밋하지 않음):
* `sub_000B1700`이 `+8`을 지울 때와 `sub_000B2730`이 `+8`을 두 번 읽을 때, 스레드 번호·객체 주소·값을 기록한다.
* 두 번째 읽기가 null일 때, 직전에 다른 스레드의 정지가 같은 객체를 지웠는지 확인한다.

2단계 (`src/game/ct3/decomp/adxm_lock.c`):
* `sub_000B1700`을 정의한다. `g_adxm_lock`을 잡고 lift 원본(`sub_000B1700_gen`)을 부른 뒤 놓는다. #42의 `sub_000B7EE0`과 같은 형태다. 디컴파일이 아니라 호스트 동시성 어댑터다.
* 잠금 순서는 ADXM → SJ로, #42와 같다. 정지가 부르는 입력 링 리셋 `sub_000B7EE0`도 같은 잠금을 재진입으로 잡는다.
* 1단계에서 다른 스레드가 지운 것이 아니면(`sub_000C7A00` 등), 이 설계로 돌아와 고친다.

## 5. 검증

* 재생성, Win32 x86 Release 빌드, CTest.
* 1단계: 계측 빌드로 숨긴 어트랙트 데모 6개 동시 실행 묶음을, null 호출이 잡힐 때까지(최대 8묶음) 돌린다.
* 2단계: 계측 없는 최종 빌드로 4묶음(24번) 이상 돌려, null 호출·음악 멈춤·다른 스트림 멈춤·stall·잠금 시한을 v0.0.23 수정 전 48번(null 2번)과 비교한다. 운전 진입도 확인한다.

---

# Design: the stream-field race between ADXT's stop and the file server (issue #44)

Status: design. Earlier: [i042 CRI server lock](20261103-i042-cri-server-lock.md), [i029 ADXM lock](20261027-i029-adxm-lock.md). Analysis: [cri-middleware](../analysis/cri-middleware.md).

## 1. Requirement

After the update to rexbox v0.0.23 (#41), 2 of 24 runs of six hidden attract demos in parallel logged a null indirect call early in boot:

```
[recomp] indirect call to 00000000 is outside code; skipped (returns to 000B27B0)
```

The runs went on. The same 24 runs on a v0.0.21 build logged none, but that sample is too small to blame v0.0.23. Its `RtlLeaveCriticalSection` now hands the section to the first waiter, which changes the thread order and may only make an old race show more often. When the call lands on a live object instead of null, the same stream is closed twice.

## 2. Read from the code (**suspected**; step 1's instrumentation confirms it)

1. The file server `sub_000B9860` calls `sub_000B2730`, which loops over the ADXT objects at `0x343520` (stride `0x138`) inside the SVM lock (`sub_000B5870`/`sub_000B5890`). For an object whose decoder has ended (state 3 or 4), it works in this order:
   * at `0x000B277B` it tests that the stream field `+8` (`[esi+7]`, `esi` = object + 1) is nonzero;
   * if `[0x3430C4] == 1` it calls `sub_000C7A00`;
   * at `0x000B27A7` it **reads `+8` again**, calls its vtable `+0x0C` at `0x000B27AD`, then clears `+8`.
   * The second read found null.
2. ADXT's stop `sub_000B1700` stops and destroys the stream in `+8` (`sub_000B52E0`, `sub_000B5810`) and clears `+8` **before** it takes the SVM lock; it takes the lock only in `sub_000B16A0` at its end. #42's input-ring reset is outside that lock too.
3. Game code calls `sub_000B1700` directly (`0x00083108`, `0x00083113`, `0x000837CA`, `0x000837EC`, `0x00083970`, `0x00083E96`, `0x00083EA2`), as do `sub_000B1E80`, `sub_000B1F80`, `sub_000B3860`, `sub_000B62B0`, `sub_000C1550` and `sub_000C1870`.
4. On the Xbox's single core the server pass, raised to TIME_CRITICAL, was not preempted by the game thread, so a stop could not land between the two reads. On the host both threads run at once.
5. The stop path's `sub_000B52E0`, `sub_000B5810`, `sub_000B8A30` and `sub_000B8880` do not spin. They only call the device's stop and close function pointers. So putting this path under the host lock does not make it spin waiting for the server, and the lock is put down during kernel waits.

## 3. Options

| Option | What | Decision |
|---|---|---|
| **A. All of `sub_000B1700` under the ADXM host lock** | a stop waits for a running file-server or level-2 pass to end; the SVM lock that `sub_000B16A0` takes at its end is re-entered on the same thread | **Chosen** (after step 1 confirms) |
| B. Read `+8` once in `sub_000B2730` | a decompilation that changes the lifted logic | Rejected: it only avoids the null; the stop and the server still race to close the same stream |
| C. A lock at each game call site | 13 call sites | Rejected: the same effect as A, with more call sites |

## 4. Structure

Step 1 (temporary instrumentation, not committed):
* Log the thread, object address and value when `sub_000B1700` clears `+8` and when `sub_000B2730` reads `+8` both times.
* When the second read is null, check that another thread's stop cleared the same object just before.

Step 2 (`src/game/ct3/decomp/adxm_lock.c`):
* Define `sub_000B1700`: take `g_adxm_lock`, call the lifted original (`sub_000B1700_gen`), release it. The same shape as #42's `sub_000B7EE0`; a host-concurrency adapter, not a decompilation.
* The lock order stays ADXM → SJ, as in #42. The input-ring reset `sub_000B7EE0` that the stop calls re-enters the same lock.
* If step 1 shows that no other thread cleared it (`sub_000C7A00` or another path did), come back to this design.

## 5. Verification

* Regeneration, Win32 x86 Release build, CTest.
* Step 1: batches of six hidden attract demos in parallel with the instrumented build, until the null call is caught (up to 8 batches).
* Step 2: at least 4 batches (24 runs) with the final build without instrumentation. Compare null calls, music stops, other stream stops, stalls and lock timeouts with the 48 runs on v0.0.23 before the fix (2 null calls), and check a drive into gameplay.
