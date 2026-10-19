# 작업 지시: GitHub Actions (issue #2)

설계: [20261020-i002](../design/20261020-i002-github-actions.md)

1. 루트 `enable_testing()`, `src/app/CMakeLists.txt`에 smoke 테스트 두 개 `add_test`.
2. `.github/workflows/ci.yml`: 빌드 뒤 `ctest`.
3. `.github/workflows/release.yml`: rexbox와 같은 태그 검증과 릴리스(제목 `reCT3 <tag>`).
4. `AGENTS.md` Git 규칙, 가이드에 `ctest` 추가. rexbox submodule을 Actions가 들어간 커밋으로 올린다.
5. 검증: 설계 §3. 작업 브랜치 `i001-project-bootstrap`에서 이어서 작업한다.

---

# Work order: GitHub Actions (issue #2)

Design: [20261020-i002](../design/20261020-i002-github-actions.md)

(1) `enable_testing()` at the root and two smoke `add_test`s in `src/app/CMakeLists.txt`. (2) `ci.yml`: `ctest` after the build. (3) `release.yml`: the same tag verification and release as rexbox (title `reCT3 <tag>`). (4) The Git rules in `AGENTS.md` and `ctest` in the guide; move the rexbox submodule to the commit that carries its Actions. (5) Verify as in design §3, continuing on the task branch `i001-project-bootstrap`.
