# 운전 화면 성능 측정

근거: 작업 로그 [20261021-i017-driving-frame-time](../work-logs/20261021-i017-driving-frame-time.md), rexbox 설계 [i023 그리기 CPU 비용](../../third_party/rexbox/docs/design/20261021-i023-draw-cpu-cost.md).

## 준비

* Release 빌드: `.\scripts\build_win32_x86.ps1 -Configuration Release`.
* 게임 패드나 키보드를 만지지 않는다. 스크립트가 게임 창에 키 메시지를 보낸다(포커스와 무관).

## 실행

PowerShell에서(환경 변수는 게임 프로세스에만 적용되고 끝나면 원래대로 돌아간다):

```powershell
.\scripts\drive_west_coast.ps1 -Log drive.log -Seconds 130 -Env @{REXBOX_D3D8_PROFILE='120'; REXBOX_FRAME_LOG='600'}
```

* 실행 후 57초쯤 West Coast 일반 규칙 운전이 시작되고 present 약 2400부터 운전 구간이다.
* 표본: `-Env`에 `REXBOX_SAMPLE='2500-3200:samples.txt'`를 더하고(실행 140초), `python third_party\rexbox\scripts\perf\resolve_samples.py samples.txt build\win32_x86\src\app\Release\rect3.map`으로 함수 이름을 본다. ucrtbase처럼 export가 듬성한 DLL은 이름이 가까운 export일 뿐이니 비율이 크면 `dumpbin /disasm /range:`로 주소를 직접 본다. 바깥 DLL 표본의 "caller"는 스택에서 찾은 첫 실행 파일 주소라 오래된 값일 수 있다.

## 읽는 법

`[d3d8 profile]` 줄(120 present마다): fps, ms/frame, cpu(표시 스레드의 CPU 시간), draws, 그리고 단계 `indices … draw`(D3D8 그리기), `present`(SwapBuffers 포함), `wait`(vblank 대기), `other`(타이틀 코드와 그리지 않는 HLE 호출). 운전 구간(draws 약 800)의 줄만 평균한다.

## 주의

* 이 기계(i5-7200U 노트북)는 같은 바이너리로도 실행마다 fps가 크게 흔들린다(23.5 ~ 16.9 fps). 두 빌드를 비교할 때는 **번갈아 바로 이어서** 실행하고, fps보다 단계 시간, cpu, 표본 비율을 비교한다.
* 측정마다 새 PowerShell 프로세스나 이 스크립트를 쓴다. 한 세션에서 `$env:`를 바꾸면 다음 실행에 남는다.
* 드물게 어트랙트 데모 중(present 약 600) `[recomp] indirect call to 00000000 ... (returns to 000C11B4)` 뒤 멈춘다(작업 로그 참조). 그 실행은 버린다.

---

# Measuring performance while driving

Rests on: work log [20261021-i017-driving-frame-time](../work-logs/20261021-i017-driving-frame-time.md), rexbox design [i023 draw CPU cost](../../third_party/rexbox/docs/design/20261021-i023-draw-cpu-cost.md).

Preparation: a Release build (`.\scripts\build_win32_x86.ps1 -Configuration Release`); leave the gamepad and keyboard alone, since the script posts key messages to the game window (independent of the focus).

Run, from PowerShell (the variables apply to the game's process only and are restored afterwards):

```powershell
.\scripts\drive_west_coast.ps1 -Log drive.log -Seconds 130 -Env @{REXBOX_D3D8_PROFILE='120'; REXBOX_FRAME_LOG='600'}
```

* West Coast driving under the normal rules starts about 57 s after launch; the driving span begins around present 2400.
* Samples: add `REXBOX_SAMPLE='2500-3200:samples.txt'` to `-Env` (run 140 s) and resolve names with `python third_party\rexbox\scripts\perf\resolve_samples.py samples.txt build\win32_x86\src\app\Release\rect3.map`. In a DLL with sparse exports such as ucrtbase the name is only the nearest export, so when its share is large look at the addresses with `dumpbin /disasm /range:`. The "caller" of a sample in an outside DLL is the first executable address found on the stack and may be a stale value.

Reading: each `[d3d8 profile]` line (every 120 presents) gives fps, ms/frame, cpu (the presenting thread's CPU time), draws, and the phases `indices … draw` (D3D8 drawing), `present` (including SwapBuffers), `wait` (the vblank wait) and `other` (title code and HLE calls that do not draw). Average only the driving lines (about 800 draws).

Cautions:

* This machine (an i5-7200U laptop) swings widely from run to run with the same binary (23.5 to 16.9 fps). To compare two builds, run them **alternately, back to back**, and compare phase times, cpu and sample shares rather than fps.
* Use a fresh PowerShell process or this script for every measurement; `$env:` changes in one session carry into the next run.
* Rarely, during the attract demo (around present 600), the run stops after `[recomp] indirect call to 00000000 ... (returns to 000C11B4)` (see the work log); discard that run.
