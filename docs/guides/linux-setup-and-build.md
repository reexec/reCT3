# Linux 개발 환경 준비, 빌드, 실행

근거: [설계 i025](../design/20261025-i025-linux-build.md), [작업 로그 i025](../work-logs/20261025-i025-linux-build.md), rexbox의 [Linux 가이드](../../third_party/rexbox/docs/guides/linux-setup-and-build.md)

Linux x64와 x86(-m32)에서 게임을 빌드하고 실행한다. WSL2(Ubuntu 24.04, WSLg)에서 확인했다. 재컴파일 코드 생성(`regenerate.py`)은 아직 Windows에서만 확인했으므로, WSL2에서는 Windows 작업 트리를 `/mnt/<드라이브>`로 그대로 쓴다.

## 1. 필요한 것

```bash
sudo apt-get install -y build-essential cmake ninja-build git \
  libasound2-dev libpulse-dev libx11-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libxss-dev \
  libxfixes-dev libxkbcommon-dev libwayland-dev libdecor-0-dev libdrm-dev libgbm-dev libgl-dev libegl-dev
# x86 (-m32) 빌드라면 추가로:
sudo dpkg --add-architecture i386 && sudo apt-get update
sudo apt-get install -y gcc-multilib g++-multilib libasound2-dev:i386 libx11-dev:i386 libxext-dev:i386 libgl-dev:i386
```

GCC 13, CMake 3.28, Ninja 1.11로 확인했다. 창 표시에는 OpenGL 3.3 이상이 필요하다(WSLg는 Mesa llvmpipe, 즉 소프트웨어 렌더링).

## 2. 소스와 게임 파일

* Linux 실기: [Win32 가이드](win32-setup-and-build.md) §2~§5대로 저장소를 받고 `roms/`를 두고 `gen/ct3_jp/`를 만든다(생성은 Windows에서). 생성 코드는 플랫폼 중립 C이므로 `gen/`을 그대로 복사하면 된다.
* WSL2: Windows 작업 트리를 `/mnt/e/...`처럼 그대로 쓴다. 빌드 트리는 `/mnt` 아래에 두지 않는다(9p 파일 시스템이라 느리다). `RECT3_BUILD_DIR`로 Linux 파일 시스템에 둔다.

## 3. 빌드와 테스트

```bash
cd /mnt/e/MYWORK/Projects/reCT3                       # WSL2: Windows 작업 트리
export RECT3_BUILD_DIR=$HOME/build/reCT3              # 빌드 트리를 Linux 쪽에
scripts/build_linux.sh x64 release                    # configure + build + ctest
scripts/build_linux.sh x86 release                    # -m32
# 또는 직접(저장소 안의 build/ 에):
cmake --preset linux-x64-release && cmake --build --preset linux-x64-release && ctest --preset linux-x64-release
```

실행 파일은 `<빌드 트리>/src/app/rect3`다. 생성 코드가 있으면 CTest가 `rect3_decomp_check`(`<빌드 트리>/tools/`)도 돌린다. SDL3의 Wayland 프로토콜 스캐너가 configure 때 내는 `validity error`(XML DTD)는 무시한다.

## 4. 실행

```bash
mkdir -p ~/ct3run && cd ~/ct3run                      # hdd/ (캐시·세이브)와 rect3.ini 가 여기에 생긴다
$HOME/build/reCT3/linux-x64-release/src/app/rect3 /mnt/e/MYWORK/Projects/reCT3/roms
# 창 없이 진단만:
REXBOX_DISPLAY=hidden REXBOX_AUDIO=none REXBOX_WATCHDOG=100 REXBOX_D3D8_PROFILE=600 \
  $HOME/build/reCT3/linux-x64-release/src/app/rect3 /mnt/e/MYWORK/Projects/reCT3/roms
```

환경 변수(`REXBOX_*`), 창 조작, 설정 메뉴는 Win32와 같다([rexbox ARCHITECTURE §5](../../third_party/rexbox/ARCHITECTURE.md)). 처음 실행에서는 타이틀이 캐시 파티션(`hdd/Partition5`)을 채우느라 데모 시작이 늦다.

## 5. WSL2의 한계

* OpenGL이 llvmpipe라 GPU가 없다. 타이틀 화면은 60 fps지만 3D 장면은 x64에서 46 fps, x86에서 그 이하다. 실기 수치가 아니다.
* soft-dirty 페이지 추적이 없어 D3D8이 정점·인덱스 버퍼를 매 프레임 모두 비교한다(정확성은 같다).
* `REXBOX_SAMPLE`(샘플링 프로파일러)은 Linux에서 거부된다.
* `/mnt` 아래의 디스크 자산 읽기가 느리다.

---

# Linux development setup, build and run

Basis: [design i025](../design/20261025-i025-linux-build.md), [work log i025](../work-logs/20261025-i025-linux-build.md), rexbox's [Linux guide](../../third_party/rexbox/docs/guides/linux-setup-and-build.md)

Builds and runs the game on Linux x64 and x86 (-m32), checked on WSL2 (Ubuntu 24.04, WSLg). Generating the recompiled code (`regenerate.py`) is verified on Windows only so far, so on WSL2 the Windows working tree is used as it is through `/mnt/<drive>`.

## 1. Requirements

The apt packages above (the x86 build adds the i386 architecture, `gcc-multilib`/`g++-multilib` and the `:i386` development libraries). Checked with GCC 13, CMake 3.28 and Ninja 1.11. A window needs OpenGL 3.3 or newer (WSLg gives Mesa llvmpipe, software rendering).

## 2. Sources and game files

On a Linux machine, follow the [Win32 guide](win32-setup-and-build.md) sections 2–5 for the repository, `roms/` and `gen/ct3_jp/` (generated on Windows; the generated code is platform-neutral C, so copying `gen/` is enough). On WSL2, use the Windows working tree through `/mnt/e/...`, and keep the build tree off `/mnt` (9p is slow) with `RECT3_BUILD_DIR`.

## 3. Build and test

`scripts/build_linux.sh x64 release` (or `x86`) configures, builds and runs `ctest`, in `$RECT3_BUILD_DIR/<preset>` or `build/<preset>`; the presets also work directly (`cmake --preset linux-x64-release` and so on). The executable is `<tree>/src/app/rect3`; with generated code CTest also runs `rect3_decomp_check` (`<tree>/tools/`). The `validity error` lines from SDL3's Wayland protocol scanner at configure time (XML DTD) are harmless.

## 4. Run

Run from a directory of your own (`hdd/` and `rect3.ini` appear there) with the `roms` path as the argument, as in section 4 above; `REXBOX_DISPLAY=hidden REXBOX_AUDIO=none REXBOX_WATCHDOG=100` gives a diagnostic run without a window. The `REXBOX_*` variables, the window and the settings menu are the same as on Win32 ([rexbox ARCHITECTURE section 5](../../third_party/rexbox/ARCHITECTURE.md)). On the first run the title fills the cache partition (`hdd/Partition5`), which delays the demo.

## 5. WSL2's limits

OpenGL is llvmpipe, so there is no GPU: the title screen runs at 60 fps, 3D scenes at 46 fps on x64 and less on x86, none of it a real machine's numbers. Without soft-dirty page tracking D3D8 compares every vertex and index buffer each frame (same correctness). `REXBOX_SAMPLE` is refused on Linux. Disc asset reads under `/mnt` are slow.
