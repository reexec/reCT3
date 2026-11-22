# 작업 로그: CRI safe thread 핸드셰이크와 ADXM 호스트 잠금의 교착 (issue #52)

작업 지시: [20261123-i052](../work-orders/20261123-i052-safe-thread-handshake.md) · 설계: [20261123-i052](../design/20261123-i052-safe-thread-handshake.md) · rexbox: reexec/rexbox#87

## 한 일

1. #51 검토 중 숨긴 데모 48번 가운데 1번에서 약 11초씩 되풀이되는 멈춤을 보았다. stall 보고와 lift 코드로 원인을 찾았다(설계 §2).
   * `sub_000B3060`(SVM 콜백 5)이 ADXM 호스트 잠금을 쥔 채, safe thread가 플래그를 지울 때까지 `SetThreadPriority`/`ResumeThread`를 되풀이한다.
   * safe thread(`sub_000B3180`)는 같은 잠금을 잡는 레벨 5 서버 실행을 마친 뒤에야 플래그를 지운다. 그래서 교착이다.
2. rexbox#87(v0.0.31)로, 커널 대기가 아닌 대기를 알리는 C API `rexbox_guest_wait_begin`/`end`를 열었다.
3. `src/game/ct3/decomp/adxm_lock.c`에 `sub_000B3060` 어댑터를 두었다(`7635e72`). lift 원본을 그 대기로 감싸, 회전하는 동안 게스트 잠금을 내려놓는다. `decomp/README.md`, ARCHITECTURE, 분석 `cri-middleware.md` §5도 갱신했다.
4. 역할: 코드와 문서 갱신은 Antigravity, 원인 분석·설계·검토·측정은 Claude가 했다.

## 검증

* 재생성(대체 함수 256 → 257), Win32 x86 Release 빌드 경고 0(D9025 제외), CTest 7/7. rexbox kernel probe 106 checks.
* 숨긴 어트랙트 데모 6개 동시 실행, 130초:

| 빌드 | 실행 | 교착(잠금 시한) | stall | 음악 멈춤 | 다른 스트림 멈춤 |
|---|---|---|---|---|---|
| 수정 전(#51의 v0.17.0·v0.18.0) | 48 | 1 | 2(같은 실행) | 3(같은 실행) | 1(같은 실행) |
| 수정 후 | 48 | 0 | 0 | 0 | 2(22050 Hz, 14193 Hz 효과음 버퍼) |

* 다른 스트림 멈춤 2번은 효과음 버퍼다. #42·#44의 기준선에서도 비슷한 빈도(24~42번에 1번)로 나왔다.
* 교착이 드물어(48번 중 1번) 재현 표본이 작다. 그래서 판단은 정적 분석으로 확인한 원인, rexbox probe의 동작 확인, 새 문제가 없음을 함께 근거로 한다.
* 주행: 59.5~59.8 fps(draw 약 800), 멈춤·null 호출·stall·잠금 시한 없음.

## 남은 것

* 효과음 스트림의 드문 멈춤(기존, 원인 미확인).
* 영상 구간의 null 간접 호출 `000C967E`(기존).

---

# Work log: the deadlock between CRI's safe-thread handshake and the ADXM host lock (issue #52)

Work order: [20261123-i052](../work-orders/20261123-i052-safe-thread-handshake.md) · Design: [20261123-i052](../design/20261123-i052-safe-thread-handshake.md) · rexbox: reexec/rexbox#87

## Done

1. During the #51 review, 1 of 48 hidden demo runs stopped repeatedly for about 11 s at a time. The stall report and the lifted code gave the cause (design section 2):
   * `sub_000B3060` (SVM callback 5) repeats `SetThreadPriority`/`ResumeThread` while holding the ADXM host lock, until the safe thread clears a flag;
   * the safe thread (`sub_000B3180`) clears the flag only after its level-5 server pass, which takes the same lock, so they deadlock.
2. rexbox#87 (v0.0.31) opened a C API, `rexbox_guest_wait_begin`/`end`, to announce a wait that is not a kernel wait.
3. A `sub_000B3060` adapter in `src/game/ct3/decomp/adxm_lock.c` (`7635e72`) wraps the lifted original in that wait, so the guest locks are down while it spins. `decomp/README.md`, ARCHITECTURE and the analysis `cri-middleware.md` section 5 were updated.
4. Roles: Antigravity wrote the code and the document updates; Claude did the cause analysis, the design, the review and the measurement.

## Verification

* Regeneration (replacements 256 → 257), Win32 x86 Release with no warnings (apart from D9025), CTest 7/7; rexbox kernel probe 106 checks.
* Six hidden attract demos in parallel, 130 s:

| Build | Runs | Deadlocks (lock timeouts) | Stalls | Music stops | Other stream stops |
|---|---|---|---|---|---|
| Before the fix (#51's v0.17.0 and v0.18.0) | 48 | 1 | 2 (same run) | 3 (same run) | 1 (same run) |
| After the fix | 48 | 0 | 0 | 0 | 2 (22050 Hz and 14193 Hz effect buffers) |

* The two other stream stops are effect buffers, at a rate similar to #42's and #44's baselines (one in 24–42).
* The deadlock is rare (one in 48), so the reproduction sample is small; the verdict rests on the cause confirmed by static analysis, the behavior checked by the rexbox probe, and the absence of new problems.
* Drive: 59.5–59.8 fps (about 800 draws), no stops, null calls, stalls or lock timeouts.

## Left open

* The rare effect-stream stops (existing, cause unknown).
* The null indirect call `000C967E` in the movie (existing).
