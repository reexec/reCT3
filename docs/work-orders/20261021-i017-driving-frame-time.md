# 작업 지시: 운전 화면의 프레임 시간 (issue #17)

배경: v0.0.8(rexbox v0.0.8)에서 Start로 운전까지 가지만 운전 화면이 23~27 fps다. 게임 시간은 present 하나에 1/60초이므로 게임이 실시간의 약 0.43배로 간다(분석 문서 §7). 메뉴는 60 fps다.

1. 작업 브랜치 `i017-driving-frame-time`. 커밋 제목 끝에 `(#17)`.
2. 입력: `REXBOX_INPUT=keyboard`, 게임 창에 `WM_KEYDOWN`/`WM_KEYUP`을 보낸다(`keybd_event`는 포커스 앱으로 새므로 쓰지 않는다). 실행 후 40 s Enter, 43 s Enter, 46 s Z, 49 s Enter, 52 s Z, 57 s Z로 West Coast 일반 규칙 운전에 들어간다.
3. 측정: `REXBOX_D3D8_PROFILE`(120 present 구간), `REXBOX_FRAME_LOG`, 운전 구간의 `REXBOX_SAMPLE`과 `resolve_samples.py`로 프레임을 CPU(타이틀 코드, D3D8 setup, 업로드)와 GPU/present 대기로 나눈다. 창 크기(`REXBOX_RENDER_SCALE`, 숨김 창), vsync, 프레임 덤프의 영향을 비교한다.
4. 가장 큰 비용의 설계를 쓴다. 런타임 비용이면 rexbox issue로 설계·구현하고, 작으면 이번 작업에서 고친다.
5. 분석 문서 §7, 작업 로그 `docs/work-logs/20261021-i017-driving-frame-time.md`.

검증: 같은 입력 스크립트로 운전 구간의 fps, 단계별 시간, CPU, game time 진행 속도를 전후 비교한다. CTest.

---

# Work order: frame time while driving (issue #17)

Background: with v0.0.8 (rexbox v0.0.8) Start reaches driving, but driving runs at 23–27 fps. Game time advances 1/60 s per present, so the game runs at about 0.43× real time (analysis section 7); the menus run at 60 fps.

1. Task branch `i017-driving-frame-time`; commit titles end with `(#17)`.
2. Input: `REXBOX_INPUT=keyboard`, with `WM_KEYDOWN`/`WM_KEYUP` posted to the game window (`keybd_event` leaks to the focused application and is not used). From launch, 40 s Enter, 43 s Enter, 46 s Z, 49 s Enter, 52 s Z and 57 s Z enter West Coast driving under the normal rules.
3. Measure: split the frame into CPU (title code, D3D8 setup, uploads) and the GPU/present wait with `REXBOX_D3D8_PROFILE` (120-present windows), `REXBOX_FRAME_LOG`, and `REXBOX_SAMPLE` over the driving span resolved by `resolve_samples.py`. Compare window size (`REXBOX_RENDER_SCALE`, a hidden window), vsync and frame dumps.
4. Write the design for the biggest cost. A runtime cost is designed and implemented under a rexbox issue; a small one is fixed in this task.
5. Analysis section 7 and the work log `docs/work-logs/20261021-i017-driving-frame-time.md`.

Verification: fps, phase times, CPU and the game-time rate over the driving span before and after, with the same input script; CTest.
