# 작업 지시: 캐시 자산의 이름 바꾸기 (issue #9)

설계: rexbox [20261021-i014-file-rename](../../third_party/rexbox/docs/design/20261021-i014-file-rename.md)(런타임 작업은 rexbox issue #14, 브랜치 `i014-file-rename`)

1. 작업 브랜치 `i009-cache-rename`. 커밋 제목 끝에 `(#9)`.
2. 원인 확인: `REXBOX_TRACE_FILES`로 `Z:\tmp` 쓰기 → class 10 실패 → `Z:\_bank10.mdl` 열기 실패를 본다. 호출하는 XAPI `MoveFileEx`(`sub_000DFEB1`)의 정보 블록을 분석 문서에 적는다.
3. rexbox 구현이 들어가면 `third_party/rexbox`를 그 커밋으로 올리고 재생성 없이 빌드한다(rexbox만 바뀜).
4. 검증: `hdd/Partition5`의 `_bank10.mdl`과 `tmp`를 지운 뒤 실행해 이름 바꾸기와 열기가 성공하는지, frame 1500의 장면에 지형이 그려지는지 본다. CTest.
5. 작업 로그 `docs/work-logs/20261021-i009-cache-rename.md`.

---

# Work order: renaming the cached asset (issue #9)

Design: rexbox [20261021-i014-file-rename](../../third_party/rexbox/docs/design/20261021-i014-file-rename.md) (the runtime work is rexbox issue #14 on branch `i014-file-rename`).

(1) Task branch `i009-cache-rename`; commit titles end with `(#9)`. (2) Confirm the cause with `REXBOX_TRACE_FILES`: the write of `Z:\tmp`, the failing class 10, the failing open of `Z:\_bank10.mdl`; record the information block of the calling XAPI `MoveFileEx` (`sub_000DFEB1`) in the analysis. (3) Once the rexbox implementation is in, move `third_party/rexbox` to that commit and build without regenerating (only rexbox changes). (4) Verify: with `_bank10.mdl` and `tmp` removed from `hdd/Partition5`, the rename and the open succeed and frame 1500's scene draws its geometry; CTest. (5) Work log `docs/work-logs/20261021-i009-cache-rename.md`.
