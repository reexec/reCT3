# 설계: CRI 레벨 2 서버 실행과 SJ 링 리셋의 호스트 배타 (issue #42)

상태: 설계. 이전: [i029 ADXM 잠금](20261027-i029-adxm-lock.md), [i029 SJ 잠금](20261103-i029-sj-lock.md). 분석: [cri-middleware](../analysis/cri-middleware.md).

## 1. 요구사항

v0.0.18(#29의 SJ 잠금) 이후, 로딩 화면에서 게임이 가끔 멈춘다. 어트랙트 데모를 숨긴 창으로 여러 개 동시에 돌리면 약 10%의 실행에서 재현된다. 멈추면 음악이 끊기고, CRI vsync 스레드(thread 6)는 HLE 호출 없이 계속 돌며, 게임 메인 스레드는 SJ 잠금을 기다린다.

## 2. 확인한 것 (**확인됨**, 임시 계측: SJ 구간 이력, 링 불변식 검사, 리셋 기록, 오류 콜백 진입 기록)

1. thread 6은 SJ 링의 기본 오류 콜백 `sub_000B7E20`(`jmp $`, CRI의 오류 트랩)에 들어가 있다. ADX 디코더(`sub_000B4A70`)가 음악 입력 링 `0x003448A0`에 UngetChunk(`sub_000B8120`)를 했다가 −3(위치 불일치)을 받은 것이다. 트랩은 SJ 구간 안이라 SJ 잠금을 쥔 채 멈추고, 그 잠금을 기다리는 게임 메인 스레드도 멈춘다.
2. 그 직전 링 장부가 깨졌다. 레인 수의 합이 링 크기를 넘었다(`빈 공간 0x28480`, 크기 `0x28000`).
3. 장부를 깨뜨린 것은 리셋이다. 디코더가 입력 링에서 GetChunk한 뒤 PutChunk/UngetChunk하기 전에, 게임 메인 스레드가 같은 링을 `sub_000B7EE0`(SJRBF Reset)으로 비웠다. 호출 경로는 `0x000B172B`(`sub_000B1700`, ADXT 정지) ← `0x000837CF` ← 게임 코드다. 두 실행에서 같은 순서가 잡혔다.
4. `sub_000B7EE0`은 SJ 구간에 들어가지 않는다. `sub_000B1700`은 SVM 잠금(`sub_000B16A0` 안)을 놓은 **뒤에** 입력 링을 리셋한다. ADX 서버(`sub_000B35E0` → `sub_000B1AE0` → `sub_000B5060`), 출력 단계(`sub_000B72A0` → `sub_000B6AA0`), AIX 분리(`sub_000B9830`)는 SVM 잠금을 잡지 않는다.
5. Xbox에서는 vsync 스레드의 서버 실행이 게임 스레드보다 높은 우선순위로 단일 코어에서 돌았다. 그래서 게임 스레드의 호출이 서버 실행 도중에 끼어들 수 없었고, 리셋은 늘 서버 실행 사이에 일어났다.

## 3. 방안

| 방안 | 내용 | 판단 |
|---|---|---|
| A. 서버 실행기 전체(`sub_000B94E0`, 레벨 2와 5)를 ADXM 잠금으로 | 레벨 5까지 | 기각. 트랩은 남았고(리셋이 잠금 밖), 다른 스트림 멈춤이 54번 중 16번으로 늘었다(없을 때 42번 중 1번). 레벨 5는 ADXM 잠금 콜백이 깨우고 재우는 safe thread에서 돈다 |
| B. A에 파일 서버(`sub_000B35F0`)까지 | | 기각. 트랩이 남고 스트림 멈춤이 늘었다 |
| **C. 레벨 2 서버 실행(`sub_000B9540`)과 SJ 리셋(`sub_000B7EE0`)을 ADXM 잠금으로** | 리셋은 진행 중인 레벨 2 실행이 끝날 때까지 기다린다 | **채택** |

## 4. 구조 (`src/game/ct3/decomp/adxm_lock.c`)

* `sub_000B9540`(레벨 2 서버 실행: `sub_000B94E0(2)`)과 `sub_000B7EE0`(SJRBF Reset)을 정의한다. 각각 `g_adxm_lock`을 잡고 lift 원본(`_gen`)을 부른 뒤 놓는다. 디컴파일이 아니라 호스트 동시성 어댑터다.
* 잠금 순서는 ADXM → SJ다. 서버 실행, SVM 잠금 구간, 리셋이 모두 이 순서를 따른다. 리셋은 SJ 구간 밖에서 불린다.
* 레벨 5(`sub_000B9550`)와 파일 서버는 바꾸지 않는다.

## 5. 검증

* 재생성, Win32 x86 Release 빌드, CTest.
* 숨긴 어트랙트 데모 6개 동시 실행을 여러 번 해서, 계측 빌드로 트랩·장부 깨짐·음악 멈춤·다른 스트림 멈춤·잠금 시한을 수정 전(42번)과 비교한다.
* 계측 없는 최종 빌드로 같은 묶음 실행, 운전 진입.

---

# Design: host exclusion of CRI's level-2 server pass and the SJ ring reset (issue #42)

Status: design. Earlier: [i029 ADXM lock](20261027-i029-adxm-lock.md), [i029 SJ lock](20261103-i029-sj-lock.md). Analysis: [cri-middleware](../analysis/cri-middleware.md).

## 1. Requirement

Since v0.0.18 (#29's SJ lock) the game sometimes stops at a loading screen; hidden attract demos run in parallel reproduce it in about 10% of runs. The music stops, CRI's vsync thread (thread 6) keeps running with no HLE calls, and the game's main thread waits for the SJ lock.

## 2. Findings (**confirmed** with temporary instrumentation: an SJ section history, a ring invariant check, a reset log, an error-callback entry log)

1. Thread 6 is inside the SJ ring's default error callback `sub_000B7E20` (`jmp $`, CRI's error trap): the ADX decoder (`sub_000B4A70`) got −3 (position mismatch) from UngetChunk (`sub_000B8120`) on the music's input ring `0x003448A0`. The trap is inside the SJ section, so it holds the SJ lock for ever, and the game's main thread waiting for that lock stops too.
2. Just before, the ring's books broke: the lane counts summed to more than its size (free `0x28480`, size `0x28000`).
3. A reset broke them: between the decoder's GetChunk on the input ring and its PutChunk/UngetChunk, the game's main thread emptied the same ring with `sub_000B7EE0` (SJRBF reset), through `0x000B172B` (`sub_000B1700`, ADXT's stop) ← `0x000837CF` ← game code; two runs caught the same order.
4. `sub_000B7EE0` takes no SJ section, and `sub_000B1700` resets the input ring **after** leaving its SVM lock (inside `sub_000B16A0`). The ADX server (`sub_000B35E0` → `sub_000B1AE0` → `sub_000B5060`), its output stage (`sub_000B72A0` → `sub_000B6AA0`) and the AIX demultiplexer (`sub_000B9830`) take no SVM lock.
5. On the Xbox the vsync thread's server pass ran above the game thread's priority on one core, so a game call never landed inside it and resets always fell between passes.

## 3. Options

| Option | What | Decision |
|---|---|---|
| A. The whole server executor (`sub_000B94E0`, levels 2 and 5) under the ADXM lock | | Rejected: the trap stayed (the reset is outside the lock) and other streams stopped in 16 of 54 runs (1 of 42 without); level 5 runs on the ADXM safe thread that the lock callbacks resume and suspend |
| B. A plus the file server (`sub_000B35F0`) | | Rejected: the trap stayed and stream stops grew |
| **C. The level-2 server pass (`sub_000B9540`) and the SJ reset (`sub_000B7EE0`) under the ADXM lock** | a reset waits for a running level-2 pass to end | **Chosen** |

## 4. Structure (`src/game/ct3/decomp/adxm_lock.c`)

* Defines `sub_000B9540` (the level-2 server pass, `sub_000B94E0(2)`) and `sub_000B7EE0` (SJRBF reset); each takes `g_adxm_lock`, calls the lifted original (`_gen`) and releases it. A host-concurrency adapter, not a decompilation.
* The lock order is ADXM → SJ for the server pass, SVM-locked sections and the reset alike; the reset is called outside SJ sections.
* Level 5 (`sub_000B9550`) and the file server are unchanged.

## 5. Verification

* Regeneration, Win32 x86 Release build, CTest.
* Batches of six hidden attract demos in parallel with the instrumented build, comparing traps, broken books, music stops, other stream stops and lock timeouts with the 42 runs before the fix.
* The same batches with the final build without instrumentation, and a drive into gameplay.
