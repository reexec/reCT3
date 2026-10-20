# 작업 로그: 캐시 자산의 이름 바꾸기 — 완료 (issue #9)

작업 지시: [20261021-i009-cache-rename](../work-orders/20261021-i009-cache-rename.md) · rexbox: issue #14, [작업 로그](../../third_party/rexbox/docs/work-logs/20261021-i014-file-rename.md)

## 한 일

1. 사용자 보고(도로와 건물이 없음)를 재현했다. 데모 frame 1500, 2500에 차, 그림자, 하늘, 바다만 그려진다.
2. `REXBOX_TRACE_FILES`: 타이틀은 `D:\Media\bank10.mdl`을 읽어 `Z:\tmp`에 쓰고, XAPI `MoveFileEx`(`sub_000DFEB1`)로 `Z:\_bank10.mdl`로 바꾼 뒤 연다. rexbox가 class 10을 지원하지 않아 열기가 `C0000034`로 실패했다.
3. 원본을 손으로 `_bank10.mdl`에 복사해 본 실행은 지형 일부만 그렸다. 캐시 파일은 원본의 복사가 아니기 때문이다(아래).
4. rexbox #14에 이름 바꾸기를 넣고 submodule을 작업 브랜치 커밋 `c0bb5a2`로 올렸고, 머지 뒤 rexbox v0.0.5(`b22db2c`)에 고정했다.

## 검증

| 검증 | 결과 |
|---|---|
| Win32 x86 Release, CTest | 오류 0, 5/5(rexbox kernel probe 94개 검사) |
| `rect3.exe roms`(hidden, 100초, `hdd/Partition5`의 `_bank10.mdl`·`tmp` 삭제 후) | `rename ...\Partition5\tmp -> \??\Z:\_bank10.mdl -> 00000000`, 이어지는 열기 성공. frame 1500에 실내 주차장(바닥, 천장 조명, 벽, 기둥)이 그려진다 |
| 캐시 파일 | 13,115,392바이트. 디스크의 `bank10.mdl`(15,849,024바이트)과 첫 바이트부터 다르다. 타이틀이 가공해 쓴다 |

## 남은 것

* 프레임 시간: 지형이 그려지는 구간에서 평균 22 fps, 600프레임 중 406프레임이 33.4 ms를 넘는다. CPU 20.6 ms 가운데 D3D8 setup이 11.1 ms(프레임당 정점·인덱스 비교 약 1.4 MB, 753 구간)다. 다음 작업에서 본다.
* 100초 실행에서 이름 바꾸기는 `_bank10.mdl` 하나였다. 게임 플레이에서 다른 자산도 같은 경로를 타는지는 사용자 환경에서 확인한다.

---

# Work log: renaming the cached asset — done (issue #9)

Work order: [20261021-i009-cache-rename](../work-orders/20261021-i009-cache-rename.md) · rexbox: issue #14, [work log](../../third_party/rexbox/docs/work-logs/20261021-i014-file-rename.md)

Done: the user's report (no roads or buildings) was reproduced; demo frames 1500 and 2500 show only cars, shadows, sky and sea. `REXBOX_TRACE_FILES` shows the title reading `D:\Media\bank10.mdl`, writing `Z:\tmp`, renaming it to `Z:\_bank10.mdl` with XAPI `MoveFileEx` (`sub_000DFEB1`) and opening it; rexbox did not support class 10, so the open failed with `C0000034`. A run with the disc file copied by hand to `_bank10.mdl` drew only part of the geometry, because the cache file is not a copy of it (below). Rename went into rexbox #14, and the submodule moved to that task-branch commit `c0bb5a2`, then was pinned to the merged rexbox v0.0.5 (`b22db2c`).

Verification: Win32 x86 Release with no errors, CTest 5/5 (rexbox kernel probe, 94 checks). `rect3.exe roms` (hidden, 100 s, with `_bank10.mdl` and `tmp` removed from `hdd/Partition5`) logs `rename ...\Partition5\tmp -> \??\Z:\_bank10.mdl -> 00000000`, the open after it succeeds, and frame 1500 draws an indoor car park (floor, ceiling lights, walls, pillars). The cache file is 13,115,392 bytes and differs from the first byte from the disc's `bank10.mdl` (15,849,024 bytes): the title processes it on the way.

Left open: frame time. Where the geometry draws, the average is 22 fps and 406 of 600 frames exceed 33.4 ms; of 20.6 ms of CPU time, D3D8 setup takes 11.1 ms (about 1.4 MB of vertex and index comparison per frame over 753 ranges). That is the next task. In the 100 s run `_bank10.mdl` was the only rename; whether other assets take the same path in play is checked on the user's machine.
