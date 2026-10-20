# 작업 지시: fmod 분류 테이블 항목 lift (issue #7)

## 설계

* **증상**: 어트랙트 데모 중 `[recomp] indirect call to 000E9C7C has no target; skipped (returns to 000EA2F0)` 뒤 present가 멈춘다(frame 967 이후 10.8초). 스레드 2는 `running`으로 남는다.
* **원인**: `sub_000E9BDF`는 MSVC CRT의 두 인자 x87 수학 함수 디스패처다. 두 피연산자를 `fxam`으로 분류해 4×4 = 16칸 테이블에서 처리 코드를 골라 `jmp [ebx]`로 간다. `fmod` 테이블(`.data 002148A0`)의 항목 `000E9C7C`(`002148B0`)와 `000E9D1E`(7칸)는 이 테이블로만 도달하므로 lifter가 함수로 찾지 못했다. `find_pointer_targets.py`도 후보로 내지 않는다. 런타임이 호출을 건너뛰면 x87 스택과 결과가 틀어진다.
* **해결**: 두 주소를 `src/game/ct3/recomp/seed_pointer_targets.json`에 함수 시작점으로 둔다. rexbox는 바꾸지 않는다(타이틀 고유의 seed). 탐지 스크립트가 이런 테이블을 놓치는 이유는 따로 본다.

## 단계

1. 작업 브랜치 `i007-fmod-table-seeds`. 커밋 제목 끝에 `(#7)`.
2. XBE에서 `000E9C7C`를 담은 dword를 찾아 테이블 경계와 미lift 항목을 확인한다(`.data 00214600`~`00214C00`에서 `000E9000`~`000EC000`을 가리키는 dword 전체).
3. seed 두 개를 넣고 `regenerate.py`, Win32 x86 Release 빌드, CTest.
4. 검증: 호스트 오디오로 90초 실행(`REXBOX_DISPLAY=hidden REXBOX_WATCHDOG=90`). `no target`과 `[stall]`이 없어야 한다.
5. 분석 문서 §7과 작업 로그 `docs/work-logs/20261021-i007-fmod-table-seeds.md`.

---

# Work order: lift the fmod class-table entries (issue #7)

## Design

* **Symptom**: during the attract demo, `[recomp] indirect call to 000E9C7C has no target; skipped (returns to 000EA2F0)` is followed by no present for 10.8 s after frame 967, with thread 2 left `running`.
* **Cause**: `sub_000E9BDF` is the MSVC CRT dispatcher for two-argument x87 math functions: it classifies both operands with `fxam` and jumps through `jmp [ebx]` into a 4×4 = 16-entry table. In the `fmod` table (`.data 002148A0`) the entries `000E9C7C` (`002148B0`) and `000E9D1E` (seven slots) are reached only through the table, so the lifter never found them as functions, and `find_pointer_targets.py` does not list them. When the runtime skips the call, the x87 stack and the result go wrong.
* **Fix**: seed both addresses as function starts in `src/game/ct3/recomp/seed_pointer_targets.json`. rexbox is not changed (the seeds are title specific); why the detection script misses such tables is looked at separately.

## Steps

(1) Task branch `i007-fmod-table-seeds`; commit titles end with `(#7)`. (2) Find the dwords in the XBE that hold `000E9C7C` and confirm the table's bounds and its unlifted entries (every dword in `.data 00214600`–`00214C00` pointing into `000E9000`–`000EC000`). (3) Add the two seeds, run `regenerate.py`, build Win32 x86 Release, run CTest. (4) Verify with a 90 s run on host audio (`REXBOX_DISPLAY=hidden REXBOX_WATCHDOG=90`): no `no target` and no `[stall]`. (5) Analysis section 7 and the work log `docs/work-logs/20261021-i007-fmod-table-seeds.md`.
