# 작업 지시: 프로젝트 부트스트랩 (issue #1)

설계: [20261020-i001](../design/20261020-i001-project-bootstrap.md)

1. 저장소 초기화: `main`에 MIT `LICENSE`, `.gitignore`(게임 데이터·생성 코드·빌드 제외), `.gitattributes`. 작업 브랜치 `i001-project-bootstrap`.
2. 공유 런타임: rexbox를 먼저 분리한다(rexbox issue #1). `third_party/rexbox`(rexbox 브랜치 커밋)와 `third_party/xboxrecomp`(upstream v0.15.0) submodule.
3. 규칙 문서: `AGENTS.md`(reJSRF와 동일), `CLAUDE.md`, `docs/CODING_STYLE.md`, `docs/PROJECT_CHARTER.md`.
4. 코드: 루트 `CMakeLists.txt`(rexbox 소비자 계약), `src/game/ct3/`(프로파일, lookup, 호스트 훅, `decomp/`, `recomp/` 설정), `src/app/`(reJSRF의 앱을 `rect3` 이름으로), `include/rect3/`.
5. 스크립트: `build_win32_x86.*`, `scripts/recomp/{regenerate,find_pointer_targets,check_hle_pops}.py` wrapper, `register_xbe.py`.
6. 문서: `README.md`, `ARCHITECTURE.md`, `docs/XBE_DESIGN.*`(자리), `docs/analysis/`(자리), `docs/kb/`(색인), `docs/guides/`, `THIRD_PARTY_NOTICES.md`, `CREDITS.md`.
7. CI: Win32 x86 Debug/Release 빌드, probe, 앱 smoke.
8. 검증: 설계 §7.

완료 기준: Win32에서 경고 없이 빌드되고, 앱이 XBE 없는 입력을 거부하며, 일본판 XBE가 준비되면 등록 → 재생성 → 빌드로 이어갈 수 있는 절차가 문서로 남아 있다.

---

# Work order: project bootstrap (issue #1)

Design: [20261020-i001](../design/20261020-i001-project-bootstrap.md)

(1) Initialize the repository: MIT `LICENSE`, `.gitignore` (excluding game data, generated code and builds) and `.gitattributes` on `main`; task branch `i001-project-bootstrap`. (2) Shared runtime: split rexbox first (rexbox issue #1); the `third_party/rexbox` (rexbox branch commit) and `third_party/xboxrecomp` (upstream v0.15.0) submodules. (3) Rule documents: `AGENTS.md` (as reJSRF's), `CLAUDE.md`, `docs/CODING_STYLE.md`, `docs/PROJECT_CHARTER.md`. (4) Code: the root `CMakeLists.txt` (rexbox consumer contract), `src/game/ct3/` (profile, lookup, host hooks, `decomp/`, `recomp/` configuration), `src/app/` (reJSRF's app under the `rect3` names), `include/rect3/`. (5) Scripts: `build_win32_x86.*`, the `scripts/recomp/` wrappers, `register_xbe.py`. (6) Documents: `README.md`, `ARCHITECTURE.md`, `docs/XBE_DESIGN.*` and `docs/analysis/` placeholders, the `docs/kb/` index, `docs/guides/`, `THIRD_PARTY_NOTICES.md`, `CREDITS.md`. (7) CI: Win32 x86 Debug/Release builds, probes, app smoke. (8) Verify: design §7.

Done when it builds on Win32 without warnings, the app rejects input without an XBE, and the procedure to register the Japan XBE, regenerate and build is documented for when the XBE arrives.
