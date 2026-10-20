# 작업 로그: 프로젝트 부트스트랩 (issue #1)

설계: [20261020-i001](../design/20261020-i001-project-bootstrap.md) · 작업 지시: [20261020-i001](../work-orders/20261020-i001-project-bootstrap.md)

## 한 일

1. 저장소 초기화: `main`에 `LICENSE`, `.gitignore`, `.gitattributes`를 커밋하고 push. 작업 브랜치 `i001-project-bootstrap`.
2. 공유 런타임 분리(rexbox issue [#1](https://github.com/reexec/rexbox/issues/1)): reJSRF v0.0.42에서 rexbox 경로의 이력(41개 커밋)을 `git filter-repo`로 추출해 `reexec/rexbox`를 만들고, 독립 CMake 프로젝트와 소비자 계약, upstream xboxrecomp v0.15.0 ABI 지원, 일반화한 스크립트, 규칙 문서를 더했다(rexbox 브랜치 커밋 `807e07a`). 상세는 rexbox의 작업 로그.
3. submodule: `third_party/rexbox`(`807e07a`), `third_party/xboxrecomp`(upstream `v0.15.0`, `6376eb8`).
4. 코드: 루트 `CMakeLists.txt`, `src/game/ct3/`(프로파일의 일본판 자리, lookup, 호스트 훅, `decomp/README.md`, `recomp/` 설정 4개), `src/app/`(reJSRF의 앱을 `rect3` 이름으로; `SetApplicationIdentity` 호출 추가), `include/rect3/`.
5. 스크립트: 빌드 스크립트, rexbox 파이프라인 wrapper 3개, `register_xbe.py`.
6. 문서: `AGENTS.md`, `CLAUDE.md`, `README.md`, `ARCHITECTURE.md`, 헌장, 코딩 스타일, `XBE_DESIGN.*`, `analysis/`, `kb/`, `guides/`, 고지, 크레딧, CI.

reJSRF 저장소는 읽기만 했고 바꾸지 않았다(사용자 지시; reJSRF의 전환은 별도 작업). reJSRF에 만들었던 issue #102는 같은 이유로 닫았다.

## 검증

| 검증 | 결과 |
|---|---|
| Win32 x86 Debug / Release 빌드 | 우리 타깃 경고 0, 오류 0 (rexbox가 upstream v0.15.0 헤더로 컴파일; upstream 헤더의 C4310은 rexbox에서 `rexbox_recomp`에만 억제) |
| `rexbox_kernel_probe` / `rexbox_dsound_probe` / `rexbox_d3d8_probe` | 56 / 48 / 163 check, 0 failure (Debug·Release) |
| `rect3.exe LICENSE`, `rect3.exe docs` | 종료 코드 1, 파일·XBE 없는 디렉터리 거부 메시지 |
| 등록되지 않은 XBE(다른 타이틀의 XBE로 시험) | `not a supported Crazy Taxi 3: High Roller build (title id ..., certificate version ...)`, 종료 코드 1 |
| `register_xbe.py --dry-run` | 신원과 `kVariants` 초기화 식, 분석 표 행 출력 |
| `regenerate.py` | `variant ct3_jp (Japan) is not registered yet ...`로 멈춤, 종료 코드 1 |
| 스크립트 | `py_compile` 통과 |
| 일본판 XBE 등록·재생성·실행 | 대기(XBE 준비 뒤) |

## 남은 것

* 일본판 XBE가 준비되면: `register_xbe.py` → 프로파일에 값 붙여 넣기 → `docs/analysis/ct3-xbe-overview.md` 채우기 → `rexbox_xdk_symbols` 빌드 → `regenerate.py` → 빌드·부팅. XDK 버전 차이와 lifter 수정 필요 여부는 그때 확인한다(설계 §5).
* rexbox 브랜치가 머지되면 submodule을 태그로 옮긴다.

---

# Work log: project bootstrap (issue #1)

Design: [20261020-i001](../design/20261020-i001-project-bootstrap.md) · Work order: [20261020-i001](../work-orders/20261020-i001-project-bootstrap.md)

## Done

1. Repository initialized: `LICENSE`, `.gitignore` and `.gitattributes` committed to `main` and pushed; task branch `i001-project-bootstrap`.
2. The shared runtime split (rexbox issue [#1](https://github.com/reexec/rexbox/issues/1)): the history of reJSRF v0.0.42's rexbox paths (41 commits) extracted with `git filter-repo` into `reexec/rexbox`, plus a standalone CMake project with its consumer contract, upstream xboxrecomp v0.15.0 ABI support, generalized scripts and the rule documents (rexbox branch commit `807e07a`); details in rexbox's work log.
3. Submodules: `third_party/rexbox` (`807e07a`) and `third_party/xboxrecomp` (upstream `v0.15.0`, `6376eb8`).
4. Code: the root `CMakeLists.txt`, `src/game/ct3/` (the profile with the Japan placeholder, the lookup, the host hooks, `decomp/README.md`, four `recomp/` configuration files), `src/app/` (reJSRF's app under the `rect3` names, with the `SetApplicationIdentity` call added), `include/rect3/`.
5. Scripts: the build scripts, three wrappers over rexbox's pipeline, `register_xbe.py`.
6. Documents: `AGENTS.md`, `CLAUDE.md`, `README.md`, `ARCHITECTURE.md`, the charter, the coding style, `XBE_DESIGN.*`, `analysis/`, `kb/`, `guides/`, the notices, the credits, CI.

The reJSRF repository was read only, never changed (user instruction; its migration is a separate task); reJSRF issue #102, opened for that, was closed for the same reason.

## Verification

| check | result |
|---|---|
| Win32 x86 Debug and Release builds | 0 warnings on our targets, 0 errors (rexbox compiled against the upstream v0.15.0 header; the header's C4310 is silenced in rexbox for `rexbox_recomp` only) |
| `rexbox_kernel_probe` / `rexbox_dsound_probe` / `rexbox_d3d8_probe` | 56 / 48 / 163 checks, 0 failures (Debug and Release) |
| `rect3.exe LICENSE`, `rect3.exe docs` | exit code 1 with the file and no-XBE messages |
| An unregistered XBE (another title's XBE used for the test) | `not a supported Crazy Taxi 3: High Roller build (title id ..., certificate version ...)`, exit code 1 |
| `register_xbe.py --dry-run` | prints the identity, the `kVariants` initializer and the analysis row |
| `regenerate.py` | stops with `variant ct3_jp (Japan) is not registered yet ...`, exit code 1 |
| Scripts | `py_compile` passes |
| Registering, regenerating and running the Japan XBE | pending (once the XBE is in hand) |

## Left open

* Once the Japan XBE is in hand: `register_xbe.py`, paste the values into the profile, fill `docs/analysis/ct3-xbe-overview.md`, build `rexbox_xdk_symbols`, `regenerate.py`, build and boot; the XDK version difference and any lifter fixes are checked then (design §5).
* Move the submodule to a tag once the rexbox branch is merged.
