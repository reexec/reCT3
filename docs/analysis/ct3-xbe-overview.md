# CT3 XBE 개요

`rexbox_xbe_info`와 `scripts/recomp/register_xbe.py`로 사용자 덤프의 일본판 `default.xbe`를 읽은 결과와, 첫 실행(issue #3)에서 확인한 부팅 순서다. 표기: **확인됨**은 우리 도구로 직접 읽거나 실행으로 본 값, **추정**은 정황이나 외부 출처, **미확정**은 다음에 검증할 질문이다.

What `rexbox_xbe_info` and `scripts/recomp/register_xbe.py` read from the user's Japan `default.xbe`, and the boot sequence seen in the first runs (issue #3). **Confirmed** values were read by our tools or seen running; **inferred** ones rest on circumstance or an outside source; **unresolved** ones are questions to check next.

## 1. 빌드 변형 / Build variants

| 레이블 / Label | 지역 / Region | MD5 | 크기 / Size | title id | 인증서 시각 / Certificate time | 이미지 시각 / Image time |
|---|---|---|---|---|---|---|
| Japan (1차 타깃 / primary) | 2 | `18a8873932612bb8ad86910ffb5d8cfe` | 2,179,072 | `53450004` | `3D1FC5C5` | `3D088B87` |

* **확인됨**: 인증서 제목 `CRAZY TAXI 3 High Roller`, 인증서 버전 2, 미디어 2(DVD), 키 retail. 프로파일(`ct3_profile.cpp`)은 title id, 인증서 시각·버전, 이미지 시각으로 식별한다.
* **미확정**: 다른 지역판의 존재와 신원.

* **Confirmed**: certificate title `CRAZY TAXI 3 High Roller`, certificate version 2, media 2 (DVD), retail keys; the profile identifies the build by title id, certificate time and version, and image time.
* **Unresolved**: other regional builds.

## 2. 이미지 헤더 / Image header

| 항목 / Field | 값 / Value |
|---|---|
| base | `00010000` |
| image size | `0034B7A0` (3.3 MiB; JSRF는 `00278620`) |
| headers size | `00001000` |
| init flags | `00000005` |
| entry point | `000E0934` |
| kernel thunk table | `0012A260` (`.rdata`의 시작 / start of `.rdata`) |
| stack commit | `00008000` |
| heap reserve / commit | `00100000` / `00001000` |
| TLS | raw data 없음, zero-fill `0x0C`, index `00336290` / no raw data, zero-fill `0x0C`, index `00336290` |

## 3. 섹션 / Sections

플래그: W 쓰기, X 실행, P preload. / Flags: W writable, X executable, P preload.

| 이름 / Name | 플래그 / Flags | vaddr | vsize | raw | raw size |
|---|---|---|---|---|---|
| `.text` | -XP | `00011000` | `000DE480` | `00001000` | `000DE480` |
| `PSGSFD00` | -XP | `000EF480` | `00003310` | `000E0000` | `00003310` |
| `PSGSFD_I` | -XP | `000F27A0` | `00000220` | `000E4000` | `00000220` |
| `PSGSFD_B` | -XP | `000F29C0` | `000006E0` | `000E5000` | `000006E0` |
| `PSGSFD_P` | -XP | `000F30A0` | `00000460` | `000E6000` | `00000460` |
| `D3D` | WXP | `000F3500` | `00011F00` | `000E7000` | `0000EA40` |
| `XGRPH` | WXP | `00105400` | `000002DC` | `000F6000` | `000002DC` |
| `DSOUND` | WXP | `001056E0` | `0001DA5C` | `000F7000` | `0001D814` |
| `XPP` | WXP | `00123140` | `00007120` | `00115000` | `00007120` |
| `.rdata` | -XP | `0012A260` | `00091400` | `0011D000` | `000913F0` |
| `.data` | WXP | `001BB660` | `001967BC` | `001AF000` | `00059F08` |
| `DOLBY` | -XP | `00351E20` | `00007180` | `00209000` | `0000716C` |
| `$$XTIMAGE` | --- | `00358FA0` | `00002800` | `00211000` | `00002800` |

* **추정**: `PSGSFD*` 네 섹션은 XDK의 보안·서명 라이브러리 코드(이름 패턴). 타이틀 코드는 `.text` 하나다.
* **Inferred**: the four `PSGSFD*` sections are XDK security/signing library code (by name); title code is `.text` alone.

## 4. XDK 라이브러리 / XDK libraries

| 라이브러리 / Library | 버전 / Version | 비고 / Note |
|---|---|---|
| XAPILIB, D3D8, XGRAPHC, DSOUND, XBOXKRNL, LIBCMT | 1.0.**4531** | 모두 같은 XDK. JSRF는 4134 / all one XDK; JSRF's is 4134 |

* **확인됨**: XbSymbolDatabase가 XAPILIB 43, D3D8 118, XGRAPHC 2, DSOUND 124개 함수를 식별했다(`gen/ct3_jp/xdk_symbols.json`). D3D8LTCG는 없다(4134 JSRF와 다름).
* **확인됨**: 4531의 렌더 상태 번호는 4134와 다르다. 식별된 변수로 `D3D_g_DeferredRenderState`는 slot 83, `D3D_g_ComplexRenderState`는 118(4134: 82, 117)이며 `D3DRS_FillMode` 121, `CullMode` 129, `ZBias` 131, `DoNotCullUncompressed` 146 등 deferred 블록부터 모두 +1이다. 단순 블록에 더해진 하나는 **슬롯 82**다(**확인됨**: 기본 상태 루틴의 기본값 표에서 57..81이 4134와 같은 순서·값이고 82가 새 상태(기본값 1, `D3DRS_DEPTHCLIPCONTROL`로 **추정**), 83부터 4134의 82 이후가 이어진다). 텍스처 스테이지 번호는 4134와 같다(**확인됨**). rexbox 설계: [i007-render-state-4531-palette](../../third_party/rexbox/docs/design/20261020-i007-render-state-4531-palette.md).
* **확인됨**(포그): 상태 내보내기 `sub_000FD590`이 FogEnable(83), FogTableMode(84), FogStart(85), FogEnd(86), FogDensity(87), RangeFogEnable(88)을 읽어 NV2A 포그 모드(NONE → 정점 값, LINEAR `0x2601`, EXP `0x800`, EXP2 `0x801`)와 생성 모드(RangeFog 0 → PLANAR, 1 → RADIAL)를 정한다. 데모의 값은 FogEnable 1, 모드 1(EXP), start −10, end −500, density −0.0, FogColor `0x0F000000`이다. EXP에 density 0이라 **보이는 포그가 없다**.
* **확인됨**(정점 블렌딩): `SetRenderState_VertexBlend`(`000F3DA0`)는 값을 NV2A `0x328`(SET_SKIN_MODE)로 그대로 보낸다. 데모 동안 슬롯 119가 0, 1, 3, 5(행렬 2/3/4개, 마지막 가중치 암묵)를 오간다. 모델뷰는 `SetVertexBlendModelView`로 준다. rexbox 설계: [i007-fog-vertex-blend](../../third_party/rexbox/docs/design/20261020-i007-fog-vertex-blend.md).
* **확인됨**: 4134의 `D3D_InitializeDeviceState`에 해당하는 루틴은 `sub_000FAA20`이다(`Direct3D_CreateDevice` → `000FB140`이 부른다). 변환 10개를 단위 행렬로, 상태 57..146을 `0x00101358`의 표로(117, 136 제외), 125(ZEnable)를 0으로, 스테이지 상태를 `0x00101338`의 표로 설정한다. 지금은 프로파일이 바인딩하지 않아 렌더 상태가 0으로 시작한다(`no D3D_InitializeDeviceState binding`).
* **확인됨**: `D3DDevice_CreatePalette`(`000F93A0`)는 12바이트 헤더(`Common = (Size << 30) | 0x01030001`)와 `MmAllocateContiguousMemoryEx({256, 128, 64, 32}[Size])` 데이터를 만든다. `D3DDevice_SetPalette`(`000F6050`)는 디바이스 `+0xA88 + Stage×4`에 두고 NV2A `0x1B20 + Stage×64`를 푸시한다.

* **Confirmed**: XbSymbolDatabase identifies 43 XAPILIB, 118 D3D8, 2 XGRAPHC and 124 DSOUND functions; there is no D3D8LTCG.
* **Confirmed**: 4531 numbers the render states differently: the identified variables put `D3D_g_DeferredRenderState` at slot 83 and `D3D_g_ComplexRenderState` at 118 (4134: 82, 117), with `D3DRS_FillMode` 121, `CullMode` 129, `ZBias` 131, `DoNotCullUncompressed` 146: everything from the deferred block on is one slot higher, so one state was added to the simple block (**inferred**). rexbox's D3D8 HLE uses the 4134 table and must follow ([rexbox #7](https://github.com/reexec/rexbox/issues/7)).
* **Confirmed**: the one state added to the simple block is **slot 82**: the default-state routine's table has 57..81 in 4134's order and values, 82 new (default 1, **inferred** to be `D3DRS_DEPTHCLIPCONTROL`), and 4134's 82 on from 83; texture-stage numbering equals 4134's. rexbox design: [i007-render-state-4531-palette](../../third_party/rexbox/docs/design/20261020-i007-render-state-4531-palette.md).
* **Confirmed** (fog): the state flush `sub_000FD590` reads FogEnable (83), FogTableMode (84), FogStart (85), FogEnd (86), FogDensity (87) and RangeFogEnable (88) to choose the NV2A fog mode (NONE → the vertex's value, LINEAR `0x2601`, EXP `0x800`, EXP2 `0x801`) and generation mode (RangeFog 0 → PLANAR, 1 → RADIAL); the demo sets FogEnable 1, mode 1 (EXP), start −10, end −500, density −0.0 and FogColor `0x0F000000`, i.e. EXP with density 0 and **no visible fog**. **Confirmed** (vertex blending): `SetRenderState_VertexBlend` (`000F3DA0`) sends its value unchanged to NV2A `0x328` (SET_SKIN_MODE); during the demo slot 119 moves between 0, 1, 3 and 5 (two, three and four matrices, the last weight implicit), the model views coming from `SetVertexBlendModelView`. rexbox design: [i007-fog-vertex-blend](../../third_party/rexbox/docs/design/20261020-i007-fog-vertex-blend.md).
* **Confirmed**: 4134's `D3D_InitializeDeviceState` is `sub_000FAA20` here (called from `Direct3D_CreateDevice` → `000FB140`): ten identity transforms, states 57..146 from the table at `0x00101358` (skipping 117 and 136), 125 (ZEnable) = 0 and stage states from `0x00101338`; the profile does not bind it yet, so render state starts at 0 (`no D3D_InitializeDeviceState binding`). `D3DDevice_CreatePalette` (`000F93A0`) makes a 12-byte header (`Common = (Size << 30) | 0x01030001`) and `MmAllocateContiguousMemoryEx({256, 128, 64, 32}[Size])` data; `D3DDevice_SetPalette` (`000F6050`) keeps it at device `+0xA88 + Stage×4` and pushes NV2A `0x1B20 + Stage×64`.

## 5. 커널 import / Kernel imports

* **확인됨**: 121개 중 rexbox가 104개를 묶는다. 없는 17개: `RtlRaiseException`, `NtYieldExecution`, `NtQueryFullAttributesFile`, `HalReturnToFirmware`, `IoCreateDevice`, `IoInvalidDeviceRequest`, `IofCompleteRequest`, `KeSetTimerEx`, `KeWaitForMultipleObjects`, `IoStartPacket`, `IoStartNextPacket`, `IoMarkIrpMustComplete`, `HalInitiateShutdown`, `KeSaveFloatingPointState`, `KeRestoreFloatingPointState`, `RtlUnwind`, `KeBugCheck`. JSRF에 없던 import: `IdexChannelObject`, `IoFileObjectType`, `KeSetTimerEx`, `KeWaitForMultipleObjects`, `NtPulseEvent`, `NtQueryFullAttributesFile`, `NtYieldExecution`.
* **Confirmed**: rexbox binds 104 of 121; the 17 without an export are listed above (`HalReturnToFirmware` is the title's way back to the dashboard). Imports JSRF lacked: `IdexChannelObject`, `IoFileObjectType`, `KeSetTimerEx`, `KeWaitForMultipleObjects`, `NtPulseEvent`, `NtQueryFullAttributesFile`, `NtYieldExecution`.

## 6. 디스크 자산 / Disc assets

* **확인됨**: `Media/` 아래 400개 파일: `.bin` 230, `.art` 92, `.mdl` 61, `.xgt` 6, `.D00` 6, `.xbx` 2, `.mov` 2, `.afs` 1. 폴더 `ADX`(CRI ADX 오디오, **추정**), `Collision`, `FcvEdit`, `Movies`, `PasDat`, `People`, `Ref`, `Replay`, `SE`, `SetObj`, `Spline`, `Sprites`, `Sprites_eu`, `Sprites_us`, 그리고 `bank00..15.art/.mdl`.
* **확인됨**: 부팅 때 캐시 파티션 Z:에서 `_bank06.mdl`, `_bank06.art`, `_fcvdata*.bin`, `_landmark_*.bin`, 차량·드라이버 이름의 `_*.bin`을 찾는다(없으면 `C0000034`로 넘어감: 첫 실행에서 만드는 캐시).
* **Confirmed**: 400 files under `Media/` (counts above; `ADX` is CRI ADX audio, **inferred**); at boot the title looks for cached `_*.bin/.mdl/.art` files on the Z: cache partition and goes on without them (`C0000034`).

## 7. 부팅 순서와 frontier / Boot sequence and frontier

```mermaid
sequenceDiagram
    participant T as title (thread 2)
    participant K as kernel HLE
    participant D as D3D8 / DSOUND HLE
    T->>K: ExQueryNonVolatileSetting 17, KeSetTimer
    T->>K: NtDeviceIoControlFile(CdRom0, IOCTL_SCSI_PASS_THROUGH: MODE SENSE page 0x3E)
    Note over T,K: answer must say "authenticated Xbox partition", else HalReturnToFirmware (rexbox #6)
    T->>K: symbolic links D:, T:, U:; Partition0 raw read; Partition5 formatted and linked as Z:
    T->>K: cache files on Z: (absent on a first run)
    T->>D: Direct3D_CreateDevice 640x480, format 06, depth 2A, multisample 0011
    T->>D: XInitDevices, CreateVertexShader x N, CreateTexture 512x256
    T->>T: Data | 0x80000000 -> NtReadFile into the texture (rexbox #8: the physical window)
    T->>D: DirectSoundCreate, XAudioDownloadEffectsImage, SetMixBinHeadroom (rexbox #9)
    T->>K: PsCreateSystemThreadEx x 5 (routine 000DDF9D: workers 000B3110, 000B3180)
    T->>D: GetProjectionViewportMatrix, SetVertexShaderInput, first Swap, SetModelView (rexbox #7)
    T->>D: BeginPush_4 / EndPush: SET_BEGIN_END + INLINE_ARRAY strips (rexbox #7)
    T->>D: DirectSoundCreateBuffer x 135 (rexbox #9)
    T->>K: thread 9, KeWaitForMultipleObjects, KeSetTimerEx (rexbox #12: timers fire)
    T->>D: IDirectSoundBuffer_SetFormat, SetPitch ... (rexbox #9); hundreds of black frames
    T->>D: D3D_InitializeDeviceState (000FAA20), title screen drawn; CreatePalette (rexbox #7)
    T->>D: Suspend (state flush), SetVertexBlendModelView (000F61E0, named here)
    T->>D: PRESS START screen, then the attract demo in 3D (rexbox #7)
```

* **확인됨**: `0x4D014`는 `IOCTL_SCSI_PASS_THROUGH`이며 CDB `5A 00 3E 00 00 00 00 00 1C 00`(MODE SENSE(10) 페이지 0x3E). 답의 바이트 11이 1이고 10 또는 12가 0이 아니어야 계속한다(`sub_000E412D`). → [rexbox kb: xbox-dvd-drive](../../third_party/rexbox/docs/kb/xbox-dvd-drive.md)
* **확인됨**: 타이틀 코드는 D3D 리소스의 `Data`에 `0x80000000`을 OR해 CPU 포인터로 쓴다(`sub_000ABAA0`: `ecx = MEM32(eax + 4) | 0x80000000`, 30곳; `0xF0000000` 11곳). → [rexbox kb: xbox-memory-map](../../third_party/rexbox/docs/kb/xbox-memory-map.md)
* **확인됨**: D3D8 내부 `Get2DSurfaceDesc`는 식별되지 않은 공개 함수가 lift된 채 부른다. 순수 함수이므로 `hle_libraries.json`의 `lifted`로 둔다.
* **확인됨**: lifter가 4886개 함수를 옮겼고, 미해결 스텁 4개, 미구현 명령 22개(`insb`/`outsb`/`hlt`/`cli`/`int`/`wbinvd`/`arpl`/`daa`: 커널·보안 라이브러리 코드로 **추정**).
* **확인됨**: DSOUND `DirectSoundEnterCriticalSection`(`001057C2`)은 `FS:[0x24]`(KPCR IRQL)가 0일 때만 `RtlEnterCriticalSection(0x122908)`(thunk `0x12A264`, 커널 277)을 부르고 1을, 아니면 0을 돌려준다. 게스트 데이터만 쓰므로 `lifted`로 둔다.
* **확인됨**: `000B3110`, `000B3180`은 즉시값(`000B3309`, `000B3341`)으로만 전달되어 lifter가 놓친 함수이며, 스레드 루틴 `000DDF9D`가 간접 호출한다. `find_pointer_targets.py --write`로 이런 후보 29개를 `seed_pointer_targets.json`에 두자 4,929개 함수가 lift되었다.
* **확인됨**: 그 다음 frontier는 `D3DDevice_GetProjectionViewportMatrix`(`000F4F80`) → `D3DDevice_SetVertexShaderInput`(`000F7170`, stdcall 3인자)이다.
* **확인됨**: 타이틀은 `SetModelView`의 composite를 `GetProjectionViewportMatrix`의 결과로 만든다(rexbox 실행에서 모델뷰 × 투영 × 뷰포트와 차이 0, composite 행 3 = (320, 240, 0.1, 1)).
* **확인됨**: `BeginPush_4`(`000F81F0`) 호출 지점 5곳(`000A44D2`, `000A8799`, `000A88F9`, `000A8A63`, `000A8BEE`). 첫 지점은 NV2A 헤더 `0x000417FC`(메서드 `0x17FC` `SET_BEGIN_END`, 1개)에 primitive를 쓰고 정점 DWORD를 `rep movsd`/MMX로 복사한다.
* **확인됨**: 실행에서 푸시 버퍼는 `SET_BEGIN_END`와 `INLINE_ARRAY`만 썼다(모르는 메서드 보고 없음). 그린 것은 `FVF 000001C4`(XYZRHW·diffuse·specular·tex1, stride 32) strip이다.
* **추정**: 스레드 9는 타이머를 포함한 여러 객체를 `KeWaitForMultipleObjects`로 기다린다. 타이머가 발화하지 않으면 멈출 것이다.
* **확인됨**: 타이머가 발화하자(rexbox #12) 대기가 풀려 프레임 루프가 30 fps 간격으로 돈다. 그 뒤 스레드 4가 `IDirectSoundBuffer_SetFormat`(`0010712B`)에서 멈췄고, 그것을 더하자(rexbox #9) 프레임 루프가 60초 동안 정체 없이 돌다가 스레드 10이 `D3DDevice_CreatePalette`(`000F93A0`)에서 멈춘다. 그 사이 프레임은 모두 검다.
* **확인됨**: XDK 4531 라이브러리에는 `D3D_g_Stream`이 식별되지 않는다(`SetStreamSource`도 식별되지 않음). rexbox가 이 변수까지 요구하느라 전역 주소를 확정하지 못해 모든 상태가 0이었고, 이것이 검은 화면의 주된 원인이었다. 그것을 고치고 기본 상태 루틴을 바인딩하자 타이틀 화면이 그려진다. 그 뒤 `CreatePalette`를 지나 스레드 2가 `D3DDevice_Suspend`(`000F9330`)에서 멈춘다.
* **확인됨**: 4531의 `D3DDevice_Suspend`(`000F9330`)는 `CDevice_SetStateVB(0)`만 부른다. 유일한 호출자는 푸시 버퍼 그리기 함수 `000A44C0`이다. 미식별 `000F61E0`은 `D3DDevice_SetVertexBlendModelView(Count, pModelViews, pInverseModelViews, pProjectionViewport)`다: Count 0이면 dirty 비트 `0x200`만 세우고, 아니면 NV2A `0x0480 + 0x40×i`, `0x0580`, `0x0680`을 푸시하고 사본을 디바이스 `+0x4B0`, `+0x5B0`에 둔다. 타이틀 함수 `000A4380`, `000A5D70`, `000A7EB0`이 부른다. 이것들을 처리하자 "PRESS START" 화면까지 간다.
* **확인됨**: 4531의 `D3DDevice_SetRenderState_Simple`(`000F3590`)은 `(method, value)`를 푸시 포인터 `0x001023D0`에 쓰기만 하고 렌더 상태 슬롯에 쓰지 않는다. 타이틀도 슬롯을 따로 쓰지 않는다. 상태 → NV2A 메서드 표는 `0x0012A590`이다(상태 82 = `0x1D78`). `D3D_g_Stream`은 `0x00104ED8`(`CDevice_SetStateVB`가 `+stream×12`로 읽음)이지만, 타이틀은 스트림을 `SetVertexShaderInput`으로 준다. 3D 메시는 declaration만 있는 셰이더와 `D3DVSDT_NORMPACKED3` 법선을 쓴다. 디스크에서 읽은 인덱스로 푸시 버퍼 조각을 만든다(`000AAD20`: `ARRAY_ELEMENT16` 묶음, 홀수면 `ARRAY_ELEMENT32`). 이것들을 처리하자 어트랙트 데모의 3D 장면이 그려진다(frame 1900, 2100).
* **확인됨**: `XAudioCalculatePitch`(`0010587A`)는 `round(4096 × log2(f / 48000))`(0 Hz는 0)이다. 도우미 `sub_00109E9D`가 `fyl2x`와 `fistp`를 쓰고, 상수는 `0x1BA5F0` = 1/48000, `0x17DDC0` = 2^32(음수 DWORD 보정)이다.
* **미확정**: DPC 루틴 `000E0A69`가 언제 큐에 들어가는지(지금까지의 실행에서는 그 전에 멈춘다)와 실기 화면 좌표의 z 배율.

* **Confirmed**: `0x4D014` is `IOCTL_SCSI_PASS_THROUGH` with CDB `5A 00 3E 00 00 00 00 00 1C 00` (MODE SENSE(10) page 0x3E); byte 11 of the answer must be 1 and byte 10 or 12 non-zero (`sub_000E412D`); the title turns a resource's `Data` into a pointer with `| 0x80000000` (`sub_000ABAA0`; 30 places, 11 with `0xF0000000`); D3D8's internal `Get2DSurfaceDesc` is reached from an unidentified public function running lifted and stays lifted (pure); the lifter moved 4,886 functions with 4 unresolved stubs and 22 unimplemented instructions (`insb`/`outsb`/`hlt`/`cli`/`int`/`wbinvd`/`arpl`/`daa`, **inferred** to be kernel-mode or security library code).
* **Confirmed**: DSOUND's `DirectSoundEnterCriticalSection` (`001057C2`) calls `RtlEnterCriticalSection(0x122908)` (thunk `0x12A264`, kernel 277) and returns 1 only when `FS:[0x24]` (the KPCR IRQL) is 0, else returns 0; it touches only guest data and stays lifted. `000B3110` and `000B3180` are passed only as immediates (`000B3309`, `000B3341`), so the lifter missed them; the thread routine `000DDF9D` calls them indirectly. With the 29 such candidates written to `seed_pointer_targets.json` by `find_pointer_targets.py --write`, 4,929 functions are lifted. The next frontiers are `D3DDevice_GetProjectionViewportMatrix` (`000F4F80`), then `D3DDevice_SetVertexShaderInput` (`000F7170`, stdcall, 3 arguments).
* **Confirmed**: the title builds `SetModelView`'s composite from `GetProjectionViewportMatrix` (on rexbox it equals model view x projection x viewport with difference 0; composite row 3 = (320, 240, 0.1, 1)). `BeginPush_4` (`000F81F0`) has five call sites (`000A44D2`, `000A8799`, `000A88F9`, `000A8A63`, `000A8BEE`); the first writes NV2A header `0x000417FC` (method `0x17FC`, `SET_BEGIN_END`, one value) with the primitive, then copies vertex DWORDs with `rep movsd`/MMX.
* **Confirmed**: at run time the push buffers used only `SET_BEGIN_END` and `INLINE_ARRAY` (no unknown-method reports), drawing `FVF 000001C4` (XYZRHW, diffuse, specular, tex1; stride 32) strips. **Inferred**: thread 9 waits with `KeWaitForMultipleObjects` on objects including a timer, and would hang while timers do not fire.
* **Confirmed**: with timers firing (rexbox #12) the wait ends and the frame loop runs at 30 fps intervals; thread 4 then stopped at `IDirectSoundBuffer_SetFormat` (`0010712B`); with it added (rexbox #9) the frame loop runs 60 s without a stall until thread 10 stops at `D3DDevice_CreatePalette` (`000F93A0`), every frame meanwhile black. **Confirmed**: the 4531 library has no identified `D3D_g_Stream` (nor `SetStreamSource`); rexbox required that variable before settling its global addresses, so every state was 0, the main cause of the black frames; with that fixed and the default-state routine bound, the title screen draws, `CreatePalette` is passed and thread 2 stops at `D3DDevice_Suspend` (`000F9330`). **Confirmed**: 4531's `D3DDevice_Suspend` (`000F9330`) only calls `CDevice_SetStateVB(0)`, for its one caller, the push-buffer draw `000A44C0`; the unidentified `000F61E0` is `D3DDevice_SetVertexBlendModelView(Count, pModelViews, pInverseModelViews, pProjectionViewport)`, which for Count 0 only sets dirty bit `0x200` and otherwise pushes NV2A `0x0480 + 0x40×i`, `0x0580` and `0x0680` and keeps copies at device `+0x4B0` and `+0x5B0`, called from the title functions `000A4380`, `000A5D70` and `000A7EB0`; with both handled the run reaches the PRESS START screen. **Confirmed**: 4531's `D3DDevice_SetRenderState_Simple` (`000F3590`) only writes `(method, value)` at the push pointer `0x001023D0` and no render-state slot, and the title stores no slot either; the state-to-NV2A-method table is at `0x0012A590` (state 82 = `0x1D78`); `D3D_g_Stream` is at `0x00104ED8` (read by `CDevice_SetStateVB` at `+stream×12`), but the title gives its streams through `SetVertexShaderInput`; the 3D meshes use declaration-only shaders with `D3DVSDT_NORMPACKED3` normals and push-buffer fragments built from indices read from disk (`000AAD20`: runs of `ARRAY_ELEMENT16`, an `ARRAY_ELEMENT32` for an odd count); with these handled the attract demo's 3D scene draws (frames 1900, 2100). **Confirmed**: `XAudioCalculatePitch` (`0010587A`) is `round(4096 × log2(f / 48000))` (0 for 0 Hz); its helper `sub_00109E9D` uses `fyl2x` and `fistp`, and the constants are `0x1BA5F0` = 1/48000 and `0x17DDC0` = 2^32 (the negative-DWORD correction).
* **Unresolved**: when the DPC routine `000E0A69` is queued (the runs so far stop before it), and the hardware's z scale of screen coordinates.
