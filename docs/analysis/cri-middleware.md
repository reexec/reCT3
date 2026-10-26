# CRI 미들웨어: 스레드와 잠금

CT3는 CRI의 사운드·스트리밍 미들웨어를 정적 링크한다. 이 문서는 우리 런타임과 부딪히는 부분(스레드, 우선순위, 잠금)을 모은다. 같은 미들웨어의 이전 판을 reJSRF가 분석했다([reJSRF cri-middleware](https://github.com/nworkers/reJSRF/blob/main/docs/analysis/cri-middleware.md)).

## 1. 구성 (**확인됨**, XBE 문자열)

| 모듈 | 버전 문자열 |
|---|---|
| ADXXTL | `ADXXTL Ver.4531 Build:May  9 2002` |
| ADXXB | `ADXXB Ver.1.38 Build:May  9 2002` |
| ADXT/XB | `ADXT/XB Ver.7.95` |
| WXCI/XB | `WXCI/XB Ver.1.40` |

음악 데이터는 `D:\Media\ADX\ADX0.AFS`(AFS 묶음, 약 145 MB)다.

## 2. ADXM 스레드 (**확인됨**, lift 코드; 함수 이름은 reJSRF와 같은 구조로 보아 **추정**)

`ADXM_SetupThrd`(`000B3230`)가 SVM 잠금 콜백을 걸고(`SVM_SetCbLock` `000B94A0` ← `000B2FF0`, `SVM_SetCbUnlock` `000B94C0` ← `000B3030`) 스레드 셋을 만든다. 우선순위는 `[0x2DFD8C]`, `[0x2DFD90]`, `[0x2DFD98]`의 기본값이다.

| 핸들 변수 | 시작 함수 | 우선순위 | 하는 일 |
|---|---|---|---|
| `[0x34EA40]` | `000B30D0` safe | 2 | HLE 호출 없이 `[0x2DFDA4]`를 올리며 돈다. 잠금 동안만 재개된다 |
| `[0x34EA64]` | `000B3110` vsync | 1 | 서버(`000B9540`)를 돌리고 mwidle 스레드를 재개한다 |
| `[0x34EA48]` | `000B3180` mwidle | −2 | 서버(`000B9550`)를 돌린다 |

* 잠금의 TIME_CRITICAL 값은 `[0x2DFD88]`(15)다.
* 다른 스레드에 대한 `NtSuspendThread`는 rexbox에서 횟수만 센다(`[kernel] NtSuspendThread other running threads are not stopped`). 그래서 safe 스레드는 한 번 재개된 뒤로 호스트 코어 하나를 계속 쓴다(Linux 실기에서 CPU 100%로 돌고 자발적 문맥 전환이 2회뿐인 게스트 스레드가 이것으로 **추정**).

## 3. SVM 잠금 (**확인됨**, lift 코드)

* `SVM_Lock`(`000B9170`)과 `SVM_Unlock`(`000B9190`)은 `[0x2E0430]`, `[0x2E0438]`의 콜백을 인자 `[0x2E0434]`, `[0x2E043C]`로 부른다.
* lock `000B2FF0`: 전역 횟수 `[0x2DFD9C]`가 0이면 자기 우선순위(`GetThreadPriority` `000DDD95`)를 `[0x34EA60]`에 저장하고, 자기를 TIME_CRITICAL로 올리고(`SetThreadPriority` `000DDD43`), safe 스레드를 재개한다(`ResumeThread` `000DDE55`). 그다음 횟수를 1 올린다.
* unlock `000B3030`: 횟수를 1 내리고, 0이면 safe 스레드를 정지하고(`SuspendThread` `000DDE2F`) 저장한 우선순위로 되돌린다.
* `000BB9C0`은 "TIME_CRITICAL인 동안 `SVM_Unlock`"을 반복해 잠금을 모두 풀고 그 횟수를 돌려준다. `000BB9E0`이 같은 횟수만큼 `SVM_Lock`으로 다시 잡는다.
* 횟수와 저장 우선순위는 전역 하나다. 상호 배제는 단일 코어 + 엄격한 우선순위 스케줄링에만 기댄다. 호스트에서 게스트 스레드가 동시에 돌면 잠금이 아무것도 막지 못한다.
* 데모 150초 숨김 실행(Linux x64, 어댑터 적용)에서 다른 스레드가 잠금을 가진 동안 들어오려는 경우와, 잠금을 놓은 스레드가 우선순위 15로 남은 경우가 모두 나왔다(**확인됨**). 어댑터가 없으면 이때 두 스레드가 잠금 안에서 함께 돈다.
* 대응: [설계 i029](../design/20261027-i029-adxm-lock.md). `src/game/ct3/decomp/adxm_lock.c`가 두 콜백을 런타임 게스트 잠금으로 감싸고, 우선순위를 스레드마다 되돌리고, `000BB9C0`이 이 스레드의 깊이만큼만 푼다.

## 4. 미확정

* Linux 플레이 중 음악이 한 구간을 반복하는 증상(issue #29)이 이 경합 때문인지. 어댑터를 넣은 빌드로 사용자 플레이에서 확인한다.
* Windows에서 같은 증상이 드문 이유(타이밍 차이로 **추정**).

---

# CRI middleware: threads and locks

CT3 statically links CRI's sound and streaming middleware. This document gathers the parts that meet our runtime (threads, priorities, locks). reJSRF analyzed an earlier version of the same middleware ([reJSRF cri-middleware](https://github.com/nworkers/reJSRF/blob/main/docs/analysis/cri-middleware.md)).

## 1. Components (**confirmed**, XBE strings)

ADXXTL `Ver.4531`, ADXXB `Ver.1.38`, ADXT/XB `Ver.7.95` and WXCI/XB `Ver.1.40`, all built on May 9 2002. The music is `D:\Media\ADX\ADX0.AFS` (an AFS archive, about 145 MB).

## 2. ADXM threads (**confirmed**, lifted code; the function names **inferred** from reJSRF's identical structure)

`ADXM_SetupThrd` (`000B3230`) installs the SVM lock callbacks (`SVM_SetCbLock` `000B94A0` ← `000B2FF0`, `SVM_SetCbUnlock` `000B94C0` ← `000B3030`) and creates three threads with the default priorities in `[0x2DFD8C]`, `[0x2DFD90]` and `[0x2DFD98]`: the safe thread `[0x34EA40]` (start `000B30D0`, priority 2), which loops raising `[0x2DFDA4]` with no HLE call and is resumed only during the lock; the vsync thread `[0x34EA64]` (start `000B3110`, priority 1), which runs the server (`000B9540`) and resumes the mwidle thread; and the mwidle thread `[0x34EA48]` (start `000B3180`, priority −2), which runs the server (`000B9550`). The lock's TIME_CRITICAL value is `[0x2DFD88]` (15). rexbox's `NtSuspendThread` on another thread only counts (`[kernel] NtSuspendThread other running threads are not stopped`), so the safe thread, once resumed, keeps a host core busy (the guest thread seen on the Linux machine at 100% CPU with only two voluntary context switches is **inferred** to be this one).

## 3. SVM lock (**confirmed**, lifted code)

`SVM_Lock` (`000B9170`) and `SVM_Unlock` (`000B9190`) call the callbacks in `[0x2E0430]` and `[0x2E0438]` with the arguments `[0x2E0434]` and `[0x2E043C]`. The lock `000B2FF0`, at global count `[0x2DFD9C]` 0, saves its priority (`GetThreadPriority` `000DDD95`) in `[0x34EA60]`, raises itself to TIME_CRITICAL (`SetThreadPriority` `000DDD43`) and resumes the safe thread (`ResumeThread` `000DDE55`); then count + 1. The unlock `000B3030` does count − 1 and, at 0, suspends the safe thread (`SuspendThread` `000DDE2F`) and restores the saved priority. `000BB9C0` repeats `SVM_Unlock` while the thread is TIME_CRITICAL and returns the count, and `000BB9E0` takes the lock back as many times with `SVM_Lock`. The count and the saved priority are single globals, and the exclusion comes only from a single core with strict priority scheduling; with guest threads running in parallel on the host the lock excludes nothing. A hidden 150 s demo run (Linux x64, with the adapter) showed both a thread asking for the lock while another held it and a thread left at priority 15 after leaving the lock (**confirmed**); without the adapter, two threads would have run inside the lock together. Remedy: [design i029](../design/20261027-i029-adxm-lock.md); `src/game/ct3/decomp/adxm_lock.c` wraps the two callbacks in a runtime guest lock, restores each thread's own priority, and has `000BB9C0` release only this thread's depth.

## 4. Unresolved

Whether the music repeating one section during play on Linux (issue #29) comes from this contention, to be checked in user play with the adapter build; why Windows rarely shows it (timing, **inferred**).
