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
* **확인됨**: 4531의 렌더 상태 번호는 4134와 다르다. 식별된 변수로 `D3D_g_DeferredRenderState`는 slot 83, `D3D_g_ComplexRenderState`는 118(4134: 82, 117)이며 `D3DRS_FillMode` 121, `CullMode` 129, `ZBias` 131, `DoNotCullUncompressed` 146 등 deferred 블록부터 모두 +1이다. 단순 블록(57..81)에 하나가 더 들어간 것으로 **추정**. rexbox의 D3D8 HLE는 4134 표를 쓰므로 맞춰야 한다([rexbox #7](https://github.com/reexec/rexbox/issues/7)).
* **확인됨**: 4134의 `D3D_InitializeDeviceState`에 해당하는 내부 루틴은 식별되지 않아 디바이스 생성 때 렌더 상태가 비어 있다(`no D3D_InitializeDeviceState binding`).

* **Confirmed**: XbSymbolDatabase identifies 43 XAPILIB, 118 D3D8, 2 XGRAPHC and 124 DSOUND functions; there is no D3D8LTCG.
* **Confirmed**: 4531 numbers the render states differently: the identified variables put `D3D_g_DeferredRenderState` at slot 83 and `D3D_g_ComplexRenderState` at 118 (4134: 82, 117), with `D3DRS_FillMode` 121, `CullMode` 129, `ZBias` 131, `DoNotCullUncompressed` 146: everything from the deferred block on is one slot higher, so one state was added to the simple block (**inferred**). rexbox's D3D8 HLE uses the 4134 table and must follow ([rexbox #7](https://github.com/reexec/rexbox/issues/7)).
* **Confirmed**: no equivalent of 4134's `D3D_InitializeDeviceState` is identified, so the render state starts empty at device creation.

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
    T->>D: GetProjectionViewportMatrix (rexbox #7), then SetVertexShaderInput (unimplemented)
```

* **확인됨**: `0x4D014`는 `IOCTL_SCSI_PASS_THROUGH`이며 CDB `5A 00 3E 00 00 00 00 00 1C 00`(MODE SENSE(10) 페이지 0x3E). 답의 바이트 11이 1이고 10 또는 12가 0이 아니어야 계속한다(`sub_000E412D`). → [rexbox kb: xbox-dvd-drive](../../third_party/rexbox/docs/kb/xbox-dvd-drive.md)
* **확인됨**: 타이틀 코드는 D3D 리소스의 `Data`에 `0x80000000`을 OR해 CPU 포인터로 쓴다(`sub_000ABAA0`: `ecx = MEM32(eax + 4) | 0x80000000`, 30곳; `0xF0000000` 11곳). → [rexbox kb: xbox-memory-map](../../third_party/rexbox/docs/kb/xbox-memory-map.md)
* **확인됨**: D3D8 내부 `Get2DSurfaceDesc`는 식별되지 않은 공개 함수가 lift된 채 부른다. 순수 함수이므로 `hle_libraries.json`의 `lifted`로 둔다.
* **확인됨**: lifter가 4886개 함수를 옮겼고, 미해결 스텁 4개, 미구현 명령 22개(`insb`/`outsb`/`hlt`/`cli`/`int`/`wbinvd`/`arpl`/`daa`: 커널·보안 라이브러리 코드로 **추정**).
* **확인됨**: DSOUND `DirectSoundEnterCriticalSection`(`001057C2`)은 `FS:[0x24]`(KPCR IRQL)가 0일 때만 `RtlEnterCriticalSection(0x122908)`(thunk `0x12A264`, 커널 277)을 부르고 1을, 아니면 0을 돌려준다. 게스트 데이터만 쓰므로 `lifted`로 둔다.
* **확인됨**: `000B3110`, `000B3180`은 즉시값(`000B3309`, `000B3341`)으로만 전달되어 lifter가 놓친 함수이며, 스레드 루틴 `000DDF9D`가 간접 호출한다. `find_pointer_targets.py --write`로 이런 후보 29개를 `seed_pointer_targets.json`에 두자 4,929개 함수가 lift되었다.
* **확인됨**: 그 다음 frontier는 `D3DDevice_GetProjectionViewportMatrix`(`000F4F80`) → `D3DDevice_SetVertexShaderInput`(`000F7170`, stdcall 3인자)이다.
* **미확정**: `SetVertexShaderInput` 다음의 frontier와, 타이틀이 투영-뷰포트 행렬로 무엇을 계산하는지(화면 좌표의 z 배율).

* **Confirmed**: `0x4D014` is `IOCTL_SCSI_PASS_THROUGH` with CDB `5A 00 3E 00 00 00 00 00 1C 00` (MODE SENSE(10) page 0x3E); byte 11 of the answer must be 1 and byte 10 or 12 non-zero (`sub_000E412D`); the title turns a resource's `Data` into a pointer with `| 0x80000000` (`sub_000ABAA0`; 30 places, 11 with `0xF0000000`); D3D8's internal `Get2DSurfaceDesc` is reached from an unidentified public function running lifted and stays lifted (pure); the lifter moved 4,886 functions with 4 unresolved stubs and 22 unimplemented instructions (`insb`/`outsb`/`hlt`/`cli`/`int`/`wbinvd`/`arpl`/`daa`, **inferred** to be kernel-mode or security library code).
* **Confirmed**: DSOUND's `DirectSoundEnterCriticalSection` (`001057C2`) calls `RtlEnterCriticalSection(0x122908)` (thunk `0x12A264`, kernel 277) and returns 1 only when `FS:[0x24]` (the KPCR IRQL) is 0, else returns 0; it touches only guest data and stays lifted. `000B3110` and `000B3180` are passed only as immediates (`000B3309`, `000B3341`), so the lifter missed them; the thread routine `000DDF9D` calls them indirectly. With the 29 such candidates written to `seed_pointer_targets.json` by `find_pointer_targets.py --write`, 4,929 functions are lifted. The next frontiers are `D3DDevice_GetProjectionViewportMatrix` (`000F4F80`), then `D3DDevice_SetVertexShaderInput` (`000F7170`, stdcall, 3 arguments).
* **Unresolved**: the frontier after `SetVertexShaderInput`, and what the title computes with the projection-viewport matrix (the z scale of its screen coordinates).
