# 작업 로그: GitHub Actions (issue #2)

설계: [20261020-i002](../design/20261020-i002-github-actions.md) · 작업 지시: [20261020-i002](../work-orders/20261020-i002-github-actions.md)

## 한 일

1. 루트 `CMakeLists.txt`에 `enable_testing()`, `src/app/CMakeLists.txt`에 앱 smoke 테스트 두 개(`rect3_rejects_file`, `rect3_rejects_directory_without_xbe`).
2. `ci.yml`: PowerShell smoke 단계를 `ctest --test-dir build\win32_x86 -C <config> --output-on-failure`로 바꿨다(rexbox probe 3 + smoke 2).
3. `release.yml`: rexbox와 같은 태그 검증(`VERSION`, `main`)과 릴리스 생성(`docs/release-notes/<tag>.md`, 제목 `reCT3 <tag>`, 소스만).
4. `AGENTS.md` Git 규칙, 가이드(`ctest`), `ARCHITECTURE.md` §4. rexbox submodule을 Actions가 들어간 커밋으로 올렸다(`THIRD_PARTY_NOTICES.md`, `ARCHITECTURE.md`의 pin 갱신).

## 검증

| 검증 | 결과 |
|---|---|
| Win32 x86 Debug / Release 빌드 | 우리 타깃 경고 0, 오류 0 |
| `ctest -C Debug` / `-C Release` | 5/5 통과(probe 56/48/163 check 0 failure, smoke 2) |
| 워크플로 YAML 파싱 | 통과(네 파일) |
| Actions 실행 | push 뒤 PR과 첫 태그에서 돈다. 대기 |

---

# Work log: GitHub Actions (issue #2)

Design: [20261020-i002](../design/20261020-i002-github-actions.md) · Work order: [20261020-i002](../work-orders/20261020-i002-github-actions.md)

## Done

`enable_testing()` at the root and two app smoke tests in `src/app/CMakeLists.txt`; `ci.yml` runs `ctest` (the three rexbox probes and the two smoke tests) instead of the PowerShell smoke step; `release.yml` verifies a `v*` tag against `VERSION` and `main` and creates a source-only release from `docs/release-notes/<tag>.md` titled `reCT3 <tag>`, as rexbox does; the Git rules in `AGENTS.md`, the guide (`ctest`) and `ARCHITECTURE.md` §4 updated; the rexbox submodule moved to the commit that carries its Actions.

## Verification

Win32 x86 Debug and Release builds with 0 warnings on our targets; `ctest -C Debug` and `-C Release`: 5/5 passed; the four workflow files parse as YAML. The Actions runs themselves come after the push, on the pull request and the first tag; pending.
