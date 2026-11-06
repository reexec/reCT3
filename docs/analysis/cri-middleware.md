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

## 4. SJ 임계 구역 (**확인됨**)

* SJ 링 함수(SJRBF GetChunk·PutChunk 등, `000B7E80`~`000B82C0`, `000B9A20`~`000B9FC0`)는 모두 `000BC950`에 들어가 `000BC970`(꼬리 점프)으로 나온다. 이 빌드에서 둘은 전역 `[0x2E05F0]`만 읽고 쓰는 빈 임계 구역이다.
* SJ 링 객체: `+0x00` vtable(`0x210478`), `+0x0C` 레인 1(데이터) 바이트 수, `+0x10` 레인 0(빈 공간) 바이트 수, `+0x14`·`+0x18` 레인 0·1의 위치, `+0x1C` 버퍼, `+0x20` 크기, `+0x24` 끝을 넘는 여분, `+0x28` 오류 콜백(기본은 `000B7E20`), `+0x2C` 그 인자. 레인과 수의 짝은 리셋 `000B7EE0`(데이터 0, 빈 공간 = 크기, 위치 0)과 GetChunk에서 읽었다(이전 판은 `+0x0C`·`+0x10`을 반대로 적었다).
* 연산(lift 코드에서 읽음): GetChunk `000B7F40`(레인에서 조각을 떼고 그 레인 수를 줄임), PutChunk `000B8030`(조각을 다른 레인 수에 더함), UngetChunk `000B8120`(조각을 같은 레인에 되돌림; 조각 끝이 레인 위치와 맞지 않으면 오류 콜백에 −3), 리셋 `000B7EE0`(vtable로만 불림, **SJ 구간에 들어가지 않음**), 파괴 `000B7E80`, 생성 `000B82C0`(풀 `0x3448A0`~`0x3478D4`에서 빈 칸을 찾음).
* 기본 오류 콜백 `000B7E20`은 `jmp $`(CRI의 오류 트랩)다. 오류가 나면 그 스레드는 영원히 돈다.
* 호스트에서 파일 서버(thread 8)와 ADX 서버(vsync 스레드)가 음악의 입력 링을 동시에 고치면 바이트가 새고 스트림이 멈춘다(issue #29의 원인). 대응: [설계 i029 SJ 잠금](../design/20261103-i029-sj-lock.md), `src/game/ct3/decomp/sj_lock.c`.

## 5. 서버 실행과 정지 경로 (**확인됨**, issue #42의 임시 계측)

* vsync 스레드의 레벨 2 서버 실행(`000B9540` → SVM 서버 실행기 `000B94E0(2)`)은 `000B35E0`(ADX 서버 `000B1AE0` → `000B5060` → 디코더 `000B4A70`, 출력 단계 `000B5950` → `000B7420` → `000B72A0` → `000B6AA0`)와 `000B35C0`(`000B5C30`, AIX 서버 `000B9830` → `000BD050` → AIX 분리 `000BCDB0`)을 돈다. 이 경로들은 SVM 잠금을 잡지 않는다. Xbox에서는 우선순위로 게임 스레드보다 위에서 돌아, 게임 호출이 서버 실행 도중에 끼어들지 못했다.
* ADXT 정지 `000B1700`은 `000B16A0` 안에서 SVM 잠금을 잡았다 놓은 **뒤에** 입력 링을 리셋한다(`0x000B1728`, vtable `+0x14`).
* 호스트에서는 디코더가 입력 링 `0x003448A0`에서 조각을 쥔 사이에 게임 메인 스레드의 정지가 그 링을 리셋했다. 디코더의 PutChunk가 빈 공간을 크기보다 크게 만들고, UngetChunk가 −3으로 `000B7E20` 트랩에 들어간다. 트랩은 SJ 구간 안이라 SJ 잠금을 쥔 채 멈추고, 그 잠금을 기다리는 게임 메인 스레드도 멈춘다(로딩 화면 멈춤, issue #42). AIX 분리 `000BCDB0`에도 같은 꼴의 트랩(`000BCFA0`, `000BCFA2`)이 있다. 대응: [설계 i042](../design/20261103-i042-cri-server-lock.md).
* 파일 서버(thread 8, `000B9860` → `000B2730`)는 SVM 잠금 안에서 `0x343520`의 ADXF 객체 표(10개, 간격 `0x138`)를 돈다. 읽는 중(`+1` = 2)인 객체의 상태를 3(끝)이나 4(오류)로 바꾼 뒤, `+8`을 확인하고 다시 읽어 그 vtable `+0x0C`를 부르고 지운다. ADXF Close `000B2A90`은 상태가 2일 때만 정지 `000B2640`(SVM 잠금)을 거치고, 객체 전체를 잠금 없이 0으로 지운다. 호스트에서는 부팅 때 AFS 파티션 표를 읽는 `000B2DF0`(반환 `0x000B2D56`)이 상태 3을 보자마자 Close를 불러, 서버의 두 읽기 사이에 객체를 지웠다(null 간접 호출, 반환 `000B27B0`, issue #44). 대응: [설계 i044](../design/20261106-i044-adxt-stop-lock.md).
* 레벨 5 서버 실행(`000B9550`, mwidle 스레드)을 ADXM 잠금으로 감싸면 다른 스트림이 멈췄다(54번 중 16번). mwidle은 ADXM 잠금 콜백이 깨우고 재우는 스레드다.
* 영상 구간의 null 간접 호출(`000C9630`, 반환 `000C967E`): SVM 잠금(`000C5CA0`/`000C5CB0`) 안에서 사용 중인 슬롯(`[slot+0x14]`)의 객체 vtable이 0이다. CRI의 다른 경합으로 **추정**, 원인 미확인.

## 6. 미확정

* SJ 잠금이 플레이에서도 음악 반복을 없애는지(데모 대조 실험에서는 없어짐, [작업 로그 i029](../work-logs/20261027-i029-adxm-lock.md)).
* `000C967E`의 null 간접 호출의 원인.
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

## 4. SJ critical section (**confirmed**)

Every SJ ring function (the SJRBF GetChunk, PutChunk and friends, `000B7E80`–`000B82C0`, `000B9A20`–`000B9FC0`) enters `000BC950` and leaves through `000BC970` (a tail jump), an empty critical section in this build that only reads and writes the global `[0x2E05F0]`. An SJ ring object: `+0x00` vtable (`0x210478`), `+0x0C` the byte count of lane 1 (data), `+0x10` that of lane 0 (free), `+0x14`/`+0x18` the positions of lanes 0 and 1, `+0x1C` the buffer, `+0x20` its size, `+0x24` the overrun margin, `+0x28` the error callback (by default `000B7E20`), `+0x2C` its argument; the pairing of lanes and counts is read from the reset `000B7EE0` (no data, all free, positions 0) and GetChunk (the earlier revision had `+0x0C`/`+0x10` the other way round). Operations (read from the lifted code): GetChunk `000B7F40` (takes a chunk from a lane and lowers its count), PutChunk `000B8030` (adds a chunk to the other lane's count), UngetChunk `000B8120` (returns a chunk to its lane; −3 to the error callback when the chunk's end does not meet the lane's position), reset `000B7EE0` (called only through the vtable, **outside any SJ section**), destroy `000B7E80`, create `000B82C0` (finds a free slot in the pool `0x3448A0`–`0x3478D4`). The default error callback `000B7E20` is `jmp $`, CRI's error trap: the thread that hits an error spins for ever. On the host the file server (thread 8) and the ADX server (the vsync thread) updating the music's input ring together leak its bytes until the stream stops (the cause of issue #29). Remedy: [design i029 SJ lock](../design/20261103-i029-sj-lock.md), `src/game/ct3/decomp/sj_lock.c`.

## 5. Server passes and the stop path (**confirmed**, issue #42's temporary instrumentation)

The vsync thread's level-2 server pass (`000B9540` → the SVM server executor `000B94E0(2)`) runs `000B35E0` (the ADX server `000B1AE0` → `000B5060` → the decoder `000B4A70`, and the output stage `000B5950` → `000B7420` → `000B72A0` → `000B6AA0`) and `000B35C0` (`000B5C30`, and the AIX server `000B9830` → `000BD050` → the AIX demultiplexer `000BCDB0`), none of which takes the SVM lock; on the Xbox it ran above the game thread, so no game call landed inside a pass. ADXT's stop `000B1700` resets the input ring (`0x000B1728`, vtable `+0x14`) **after** leaving the SVM lock it took in `000B16A0`. On the host the game's stop reset the input ring `0x003448A0` while the decoder held a chunk of it: the decoder's PutChunk made the free lane larger than the ring and its UngetChunk went with −3 into the `000B7E20` trap, inside the SJ section, so it held the SJ lock for ever and the game's main thread waiting for it stopped (the loading-screen hang, issue #42); the AIX demultiplexer `000BCDB0` has traps of the same kind (`000BCFA0`, `000BCFA2`). Remedy: [design i042](../design/20261103-i042-cri-server-lock.md). The file server (thread 8, `000B9860` → `000B2730`) loops over the ADXF object table at `0x343520` (10 objects, stride `0x138`) inside the SVM lock; for a reading object (`+1` = 2) it sets the state to 3 (done) or 4 (error), then tests `+8`, reads it again, calls its vtable `+0x0C` and clears it. ADXF Close `000B2A90` goes through the stop `000B2640` (SVM lock) only while the state is 2, and zeroes the whole object with no lock. On the host `000B2DF0`, which reads the AFS partition table at boot (return `0x000B2D56`), called Close as soon as it saw state 3 and zeroed the object between the server's two reads (a null indirect call returning to `000B27B0`, issue #44). Remedy: [design i044](../design/20261106-i044-adxt-stop-lock.md). Putting the level-5 pass (`000B9550`, the mwidle thread, which the ADXM lock callbacks resume and suspend) under the ADXM lock stopped other streams (16 of 54 runs). The null indirect call in the movie (`000C9630`, returning to `000C967E`): inside the SVM lock (`000C5CA0`/`000C5CB0`) an active slot's object (`[slot+0x14]`) has a null vtable; another CRI race, **inferred**, cause unknown.

## 6. Unresolved

Whether the SJ lock removes the music repeat in play too (it did in the demo comparison, [work log i029](../work-logs/20261027-i029-adxm-lock.md)); the cause of the null indirect call at `000C967E`; why Windows rarely shows it (timing, **inferred**).
