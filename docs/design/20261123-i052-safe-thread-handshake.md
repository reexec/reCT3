# 설계: CRI safe thread 핸드셰이크와 ADXM 호스트 잠금의 교착 (issue #52)

상태: 설계. 작업 지시: [20261123-i052](../work-orders/20261123-i052-safe-thread-handshake.md) · 이전: [i029 ADXM 잠금](20261027-i029-adxm-lock.md), [i042](20261103-i042-cri-server-lock.md), [i044](20261106-i044-adxt-stop-lock.md) · 분석: [cri-middleware](../analysis/cri-middleware.md) · rexbox: reexec/rexbox#87

## 1. 요구사항

#51 검토 중 숨긴 어트랙트 데모 48번 가운데 1번에서, 게임이 약 11초씩 present하지 않는 일이 되풀이됐다. 음악 스트림도 멈췄다.
* thread 2(게임)는 `running`이고, 스택은 `0x000B3082 0x000C5877 0x000C5BB2 0x000C009E 0x0003B894 …`다. CRI ADXM 호스트 잠금을 쥐고 있다.
* thread 6(vsync), 7(mwidle, 기본 우선순위 16), 8(파일 서버)은 모두 "CRI ADXM lock"을 기다린다.
* 2000 ms 뒤 thread 7이 배타 없이 들어가면, 약 12초 뒤 present가 다시 시작됐다가 되풀이된다.

## 2. 확인한 것 (lift 코드, 정적 분석)

1. `sub_000B3060`은 SVM 콜백 5다(`sub_000B33xx`의 `sub_000B9430(5, 0x000B3060, …)`로 등록). 영상 경로 `sub_000C5880`(반환 `0x000C5877`)에서 불린다.
2. `sub_000B3060`이 하는 일:
   * `[0x2DFDA0] = 1`로 플래그를 세운다.
   * 최대 200,000,000번 되풀이한다. 매번 safe thread(`[0x34EA48]`, mwidle)의 우선순위를 `[0x2DFD88]`로 올리고(`SetThreadPriority`, `sub_000DDD43`) `ResumeThread`(`sub_000DDE55`)를 부른다.
   * 플래그가 0이 되면 멈춘다.
3. safe thread 본문 `sub_000B3180`은 레벨 5 서버 실행 `sub_000B9550`을 돈 **다음에야** 플래그를 0으로 지우고(`0x000B31C2`) 우선순위를 되돌린다. `sub_000B9550`은 SVM 잠금, 곧 ADXM 호스트 잠금을 잡는다.
4. 그래서 호출자가 ADXM 호스트 잠금을 쥔 채 `sub_000B3060`에서 돌면, safe thread는 `sub_000B9550`에서 그 잠금을 기다리고 플래그를 지우지 못한다. 교착이다.
5. Xbox의 단일 코어에서는 우선순위가 올라간 safe thread가 `ResumeThread` 순간 호출자를 선점했다. SVM 잠금 카운트는 전역이라 safe thread도 그대로 안에서 돌았다. 호출자가 쥔 상태로 다른 스레드가 안에 들어가는 것이 원래 동작이다.
6. rexbox의 게스트 잠금은 기록되는 대기(커널 대기, 지연, 수직 귀선) 동안 쥔 잠금을 내려놓는다. 그런데 `SetThreadPriority`/`ResumeThread` 회전은 대기로 기록되지 않는다.

## 3. 방안

| 방안 | 내용 | 판단 |
|---|---|---|
| **A. `sub_000B3060`을 대기로 알림** | 어댑터가 lift 원본을 `rexbox_guest_wait_begin("CRI safe-thread handshake")`/`rexbox_guest_wait_end()`로 감싼다(rexbox#87). 회전 동안 이 스레드의 게스트 잠금이 내려가 safe thread가 서버를 돌고 플래그를 지운다. 끝나면 잠금을 다시 잡는다 | **채택**. Xbox의 선점과 같은 뜻이다. stall 보고에도 대기로 보인다 |
| B. 어댑터에서 ADXM 잠금 깊이를 직접 다 풀었다 다시 잡기 | `adxm_lock.c`의 `t_depth`만으로는 다른 어댑터(`000B9540`, `000B7EE0`, `000B2A90`)가 잡은 호스트 깊이를 알 수 없다 | 기각 |
| C. safe thread 쪽(`sub_000B9550`)을 잠금 없이 | 서버 실행을 배타 없이 돌리게 된다(#42에서 기각한 방향) | 기각 |

## 4. 구조 (`src/game/ct3/decomp/adxm_lock.c`)

* `sub_000B3060`을 정의한다. `rexbox_guest_wait_begin("CRI safe-thread handshake (000B3060)")`, lift 원본(`sub_000B3060_gen`), `rexbox_guest_wait_end()` 순서다. 디컴파일이 아니라 호스트 동시성 어댑터다.
* 회전 중 이 스레드의 ADXM 깊이는 rexbox가 기억했다가 그대로 되돌린다. 잠금을 다시 잡을 때 safe thread가 안에 있으면 그것이 나갈 때까지 기다린다.
* 게스트 쪽의 SVM 잠금 카운트(`[0x2DFD9C]`)와 저장된 우선순위는 건드리지 않는다. Xbox에서도 safe thread는 같은 전역 카운트로 안에서 돌았다.
* 앞서 rexbox#87이 머지돼야 한다. reCT3는 rexbox 고정을 그 릴리스로 올린다.

## 5. 검증

* 재생성, Win32 x86 Release 빌드, CTest(rexbox probe 포함).
* 숨긴 어트랙트 데모 6개 동시 실행을 8묶음(48번) 이상 돌려, 수정 전(48번 중 1번 교착)과 비교한다. 잠금 시한·stall·음악 멈춤을 본다. 교착이 드물어 재현 표본이 작으므로, 1차 판단은 "새 문제 없음"과 정적 분석으로 한다.
* 가능하면 재현을 높인다. 영상(어트랙트 데모의 동영상) 구간을 길게 도는 실행을 더한다.
* 주행 진입 확인.

---

# Design: the deadlock between CRI's safe-thread handshake and the ADXM host lock (issue #52)

Status: design. Work order: [20261123-i052](../work-orders/20261123-i052-safe-thread-handshake.md) · Earlier: [i029 ADXM lock](20261027-i029-adxm-lock.md), [i042](20261103-i042-cri-server-lock.md), [i044](20261106-i044-adxt-stop-lock.md) · Analysis: [cri-middleware](../analysis/cri-middleware.md) · rexbox: reexec/rexbox#87

## 1. Requirement

During the #51 review, 1 of 48 hidden attract-demo runs repeatedly stopped presenting for about 11 s at a time, and the music stream stopped:
* thread 2 (the game) is `running`, with stack `0x000B3082 0x000C5877 0x000C5BB2 0x000C009E 0x0003B894 …`, holding the CRI ADXM host lock;
* threads 6 (vsync), 7 (mwidle, base priority 16) and 8 (file server) all wait for "CRI ADXM lock";
* after 2000 ms thread 7 enters without exclusion, presents resume about 12 s later, and it repeats.

## 2. Findings (lifted code, static analysis)

1. `sub_000B3060` is SVM callback 5 (registered with `sub_000B9430(5, 0x000B3060, …)` in `sub_000B33xx`), called from the movie path `sub_000C5880` (return `0x000C5877`).
2. What `sub_000B3060` does:
   * sets a flag, `[0x2DFDA0] = 1`;
   * repeats up to 200,000,000 times: raise the safe thread's (`[0x34EA48]`, mwidle) priority to `[0x2DFD88]` (`SetThreadPriority`, `sub_000DDD43`) and call `ResumeThread` (`sub_000DDE55`);
   * stops when the flag is 0.
3. The safe thread's body `sub_000B3180` clears the flag (`0x000B31C2`) and restores its priority only **after** running the level-5 server pass `sub_000B9550`, which takes the SVM lock, that is the ADXM host lock.
4. So when the caller spins in `sub_000B3060` holding the ADXM host lock, the safe thread waits for that lock in `sub_000B9550` and never clears the flag: a deadlock.
5. On the Xbox's single core the raised safe thread preempted the caller at `ResumeThread`, and since the SVM lock count is global it ran inside as well. Another thread entering while the caller holds the lock is the original behavior.
6. rexbox's guest locks put a holder's locks down for noted waits (kernel waits, delays, vertical blanks), but a `SetThreadPriority`/`ResumeThread` spin is not noted as a wait.

## 3. Options

| Option | What | Decision |
|---|---|---|
| **A. Announce `sub_000B3060` as a wait** | the adapter wraps the lifted original in `rexbox_guest_wait_begin("CRI safe-thread handshake")`/`rexbox_guest_wait_end()` (rexbox#87); during the spin this thread's guest locks are down, so the safe thread runs its server and clears the flag; afterwards the lock is taken again | **Chosen**: the same meaning as the Xbox's preemption, and the stall report shows it as a wait |
| B. Release and retake all of the ADXM depth in the adapter | `adxm_lock.c`'s `t_depth` cannot see the host depth taken by the other adapters (`000B9540`, `000B7EE0`, `000B2A90`) | Rejected |
| C. Run the safe thread's side (`sub_000B9550`) without the lock | the server pass would run without exclusion (the direction #42 rejected) | Rejected |

## 4. Structure (`src/game/ct3/decomp/adxm_lock.c`)

* Define `sub_000B3060`: `rexbox_guest_wait_begin("CRI safe-thread handshake (000B3060)")`, the lifted original (`sub_000B3060_gen`), `rexbox_guest_wait_end()`. A host-concurrency adapter, not a decompilation.
* rexbox remembers this thread's ADXM depth during the spin and restores it as it was; if the safe thread is inside when the lock is taken again, the caller waits until it leaves.
* The guest side's SVM lock count (`[0x2DFD9C]`) and saved priority are left alone; on the Xbox, too, the safe thread ran inside with the same global count.
* rexbox#87 must be merged first; reCT3 moves its rexbox pin to that release.

## 5. Verification

* Regeneration, Win32 x86 Release build, CTest (rexbox probes included).
* At least 8 batches (48 runs) of six hidden attract demos in parallel, compared with before the fix (one deadlock in 48), watching lock timeouts, stalls and music stops. The deadlock is rare and the reproduction sample small, so the first verdict rests on "no new problems" and the static analysis.
* If possible, raise the reproduction rate with runs that spend longer in the movie stretch of the attract demo.
* A drive into gameplay.
