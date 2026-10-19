# 서드파티 고지 / Third-Party Notices

reCT3는 MIT 라이선스다. 아래 서드파티 코드가 저장소에 포함되거나 옮겨져 있다.

reCT3 is MIT licensed. The third-party code below is included in, or ported into, the repository.

## rexbox

* 출처 / Source: https://github.com/reexec/rexbox
* 라이선스 / License: MIT (Copyright (c) 2026 rexbox contributors (reJSRF contributors)). 전문은 `third_party/rexbox/LICENSE`. / Full text in `third_party/rexbox/LICENSE`.
* 포함 방식 / How included: git submodule `third_party/rexbox`(`07ec6e2`, 브랜치 `i001-project-bootstrap`; 머지되면 태그로 옮긴다). 공유 Xbox 런타임 전체(플랫폼, XBE, 런타임, 재컴파일 ABI 어댑터, HLE, 도구)와 그 안의 submodule(SDL3 zlib, Dear ImGui MIT, SMAA MIT, XbSymbolDatabase MIT, xboxrecomp MIT)을 `add_subdirectory`로 빌드한다. 각 라이브러리의 고지는 `third_party/rexbox/THIRD_PARTY_NOTICES.md`에 있다. / Git submodule (`07ec6e2` on the branch `i001-project-bootstrap`; moved to a tag once merged): the whole shared Xbox runtime and its own submodules (SDL3 zlib, Dear ImGui MIT, SMAA MIT, XbSymbolDatabase MIT, xboxrecomp MIT), built with `add_subdirectory`; each library's notice is in `third_party/rexbox/THIRD_PARTY_NOTICES.md`.
* 옮긴 코드 / Ported code: `src/app/settings.h`, `settings.cpp`, `settings_ui.h`, `settings_ui.cpp`, `main.cpp`, `build_stamp.cmake`와 `src/game/ct3/ct3_recomp.cpp`, `ct3_host.c`, `include/rect3/*.h`는 reJSRF(MIT)의 같은 파일을 이름만 바꿔 가져온 것이다. / Those app and title files are reJSRF's (MIT) under the reCT3 names.

## xboxrecomp

* 출처 / Source: https://github.com/sp00nznet/xboxrecomp
* 라이선스 / License: MIT (Copyright (c) 2026 sp00nz). 전문은 `third_party/xboxrecomp/LICENSE`. / Full text in `third_party/xboxrecomp/LICENSE`.
* 포함 방식 / How included: git submodule `third_party/xboxrecomp`, upstream `v0.15.0` (`6376eb829e035d23c1c65377d8523eceb068ee1d`). `REXBOX_XBOXRECOMP_DIR`로 rexbox에 넘겨 lifter ABI 헤더(`templates/runtime/recomp_types.h`)와 `src/kernel/recomp_icall_feedback.h`를 빌드에, `tools/`(Python 파이프라인)를 `scripts/recomp/regenerate.py`에 쓴다. / Handed to rexbox through `REXBOX_XBOXRECOMP_DIR`: the lifter ABI header and `recomp_icall_feedback.h` enter the build, the Python pipeline under `tools/` is run by `scripts/recomp/regenerate.py`.
* 제외 / Excluded: xemu에서 가져온 LGPL-2.1-or-later 파일(`src/apu/*`, `src/nv2a/nv2a_core.c`, `src/nv2a/nv2a_regs.h`, `src/nv2a/nv2a_state.h`)과 `include/xbox/xboxrecomp.h`는 컴파일·링크하지 않는다. rexbox의 `rexbox_check_license`가 이 체크아웃에 대해서도 강제한다. / The LGPL-2.1-or-later files taken from xemu and `include/xbox/xboxrecomp.h` are never compiled or linked; rexbox's `rexbox_check_license` enforces this for this checkout too.

```text
MIT License

Copyright (c) 2026 sp00nz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
