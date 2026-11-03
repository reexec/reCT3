# 작업 로그: 로딩 화면 멈춤 — CRI 레벨 2 서버 실행과 SJ 링 리셋 (issue #42)

작업 지시: [20261103-i042](../work-orders/20261103-i042-cri-server-lock.md) · 설계: [20261103-i042](../design/20261103-i042-cri-server-lock.md) · 분석: [cri-middleware](../analysis/cri-middleware.md) §4·§5

## 한 일

1. 재현: `#41` 측정 중 운전 진입 약 27번 중 5번이 로딩 화면에서 멈췄다(v0.0.18~v0.0.20). 숨긴 창 어트랙트 데모 6개를 동시에 돌리는 묶음(실행마다 HDD 복사본)으로 약 10%의 실행에서 재현했다.
2. 임시 계측(커밋하지 않음)으로 원인을 좁혔다.
   * SJ 구간 진입 이력(스레드, 반환 주소, 레지스터, 스택 인자).
   * `sub_000B7C20`·`sub_000B7E20`(오류 콜백) 감싸기.
   * 음악 링 세 개의 불변식 검사.
   * `sub_000B7EE0`(리셋) 기록.
3. 원인(**확인됨**, 두 실행에서 같은 순서): ADX 디코더가 입력 링 `0x003448A0`에서 조각을 쥔 사이에, 게임 메인 스레드의 ADXT 정지(`sub_000B1700`)가 SVM 잠금을 놓은 뒤 그 링을 리셋했다. 디코더의 PutChunk가 빈 공간을 크기보다 크게 만들고, UngetChunk가 −3으로 오류 트랩 `sub_000B7E20`(`jmp $`)에 들어갔다. 트랩이 SJ 잠금을 쥔 채라 게임 메인 스레드도 멈췄다.
4. 기각한 방안:
   * 서버 실행기 전체(`sub_000B94E0`)를 ADXM 잠금으로: 리셋이 잠금 밖이라 트랩이 남았고, 다른 스트림 멈춤이 늘었다.
   * 그것에 파일 서버(`sub_000B35F0`)까지 더함: 역시 트랩이 남고 멈춤이 늘었다.
   * 정밀한 집계는 설계 §3.
5. 채택: `src/game/ct3/decomp/adxm_lock.c`에 `sub_000B9540`(레벨 2 서버 실행)과 `sub_000B7EE0`(SJRBF 리셋)을 ADXM 호스트 잠금 아래에 두었다. 분석 문서에서 레인과 수의 짝(이전 판은 반대)과 −3을 내는 함수(UngetChunk `sub_000B8120`)를 바로잡고, 서버·정지 경로 절을 더했다. `decomp/README.md`와 ARCHITECTURE를 갱신했다.

## 검증

| 빌드 (숨긴 데모 6개 동시, 130~140초) | 실행 | 트랩/장부 깨짐 | 음악 멈춤 | 다른 스트림 멈춤 | 잠금 시한 |
|---|---|---|---|---|---|
| 수정 전(계측) | 42 | 4 | 4 | 1 | 트랩 뒤에만 |
| 서버 실행기 전체 잠금(기각) | 54 | 4 | 3 | 16 | 트랩 뒤에만 |
| 레벨 2 + 리셋 잠금(계측) | 24 | 0 | 0 | 0 | 0 |
| 레벨 2 + 리셋 잠금(최종) | 18 | – | 0 | 1 | 0 |

* 재생성, Win32 x86 Release 빌드 경고 0(생성 코드의 기존 D9025 제외), CTest 7/7.
* 최종 빌드 묶음에서 멈춤 보고(present 없음)와 null 호출은 0이었다. 다른 스트림 멈춤 1번은 22050 Hz 버퍼로, 수정 전 기준선(42번 중 1번)과 같은 빈도다. 두 실행은 6개 동시 실행의 부하로 16~28 fps라 보고가 적었을 뿐, 멈추지 않았다.
* 최종 빌드 운전 진입(사용자 설정: 21:9, 렌더 배율 4, MSAA 16x): 2번 모두 운전 구간에 들어갔고, 음악 멈춤·null 호출·잠금 시한이 없었다.
* Windows Defender가 임시 폴더에 복사한 실행 파일과 빌드 폴더의 `rect3.exe`를 `Trojan:Win32/Bearfoos.B!ml`(머신러닝 판정)로 격리·차단했다. 사용자가 빌드 폴더를 제외에 추가한 뒤 다시 빌드해 검증했다. 그 전에 만든 계측 빌드 몇 개도 같은 이유로 사라졌다.

## 남은 것

* 영상 구간의 null 간접 호출(`sub_000C9630`, 반환 `000C967E`)은 별개의 CRI 경합으로 보인다. 원인은 모른다. 데모 묶음 실행 중에는 나오지 않았고, 운전 진입 10번 중 1번 나왔다.

---

# Work log: the loading-screen hang — CRI's level-2 server pass and the SJ ring reset (issue #42)

Work order: [20261103-i042](../work-orders/20261103-i042-cri-server-lock.md) · Design: [20261103-i042](../design/20261103-i042-cri-server-lock.md) · Analysis: [cri-middleware](../analysis/cri-middleware.md) sections 4 and 5

Done: reproduced the stop (5 of about 27 drives into gameplay during #41 on v0.0.18–v0.0.20; about 10% of runs in batches of six hidden attract demos in parallel, each with its own HDD copy); narrowed it with temporary instrumentation, not committed (an SJ section history with threads, return addresses, registers and stack arguments; wraps of `sub_000B7C20` and the error callback `sub_000B7E20`; an invariant check of the three music rings; a log of `sub_000B7EE0` resets). Cause (**confirmed**, the same order in two runs): while the ADX decoder held a chunk of the input ring `0x003448A0`, the game's main thread's ADXT stop (`sub_000B1700`) reset that ring after leaving its SVM lock; the decoder's PutChunk then made the free lane larger than the ring and its UngetChunk went with −3 into the error trap `sub_000B7E20` (`jmp $`), holding the SJ lock, which stopped the game's main thread. Rejected: the whole server executor (`sub_000B94E0`) under the ADXM lock (the trap stayed, the reset being outside the lock, and other streams stopped more often), and the same plus the file server (`sub_000B35F0`); figures in design section 3. Chosen: `src/game/ct3/decomp/adxm_lock.c` puts `sub_000B9540` (the level-2 server pass) and `sub_000B7EE0` (the SJRBF reset) under the ADXM host lock. The analysis corrects the lane-count pairing (the earlier revision had it the other way round) and the function that reports −3 (UngetChunk `sub_000B8120`) and adds a section on the server and stop paths; `decomp/README.md` and ARCHITECTURE are updated.

Verification: the table above (batches of six hidden demos, 130–140 s each); regeneration, a Win32 x86 Release build with 0 warnings (apart from the generated code's existing D9025), CTest 7/7; the final build's batches (18 runs) had no music stop, stall, null call or lock timeout and one stop of the 22050 Hz buffer, the same rate as before the fix (1 in 42); two of them ran at 16–28 fps under the load of six instances and only reported less; two drives into gameplay with the user's settings (21:9, render scale 4, MSAA 16x) reached driving with no music stop, null call or lock timeout. Windows Defender quarantined or blocked the copied executable in the temp folder and the build folder's `rect3.exe` as `Trojan:Win32/Bearfoos.B!ml` (a machine-learning verdict); the user added the build folder to the exclusions and the build was redone before verifying; some earlier instrumented builds were removed for the same reason.

Left: the null indirect call in the movie (`sub_000C9630`, returning to `000C967E`) looks like a separate CRI race, cause unknown; it did not appear in the demo batches and appeared once in ten drives into gameplay.
