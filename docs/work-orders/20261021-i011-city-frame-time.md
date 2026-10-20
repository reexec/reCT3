# 작업 지시: 도시 장면의 프레임 시간 (issue #11)

설계: rexbox [20261021-i016-p8-palette-variants](../../third_party/rexbox/docs/design/20261021-i016-p8-palette-variants.md)(런타임 작업은 rexbox issue #16, 브랜치 `i016-p8-palette-variants`)

1. 작업 브랜치 `i011-city-frame-time`. 커밋 제목 끝에 `(#11)`.
2. 측정: `REXBOX_D3D8_PROFILE`, `REXBOX_SAMPLE`(present 1700~2300), 임시 계측으로 원인을 좁힌다. 게임 진행 속도가 맞는지(vblank 대비 메인 루프의 횟수)도 확인한다.
3. rexbox 구현이 들어가면 `third_party/rexbox`를 그 커밋으로 올리고 재생성 없이 빌드한다.
4. 검증: 같은 구간의 fps·`setup`·CPU 비교, frame 1500·2100 덤프, CTest.
5. 분석 문서 §7, 작업 로그 `docs/work-logs/20261021-i011-city-frame-time.md`.

---

# Work order: frame time in the city scenes (issue #11)

Design: rexbox [20261021-i016-p8-palette-variants](../../third_party/rexbox/docs/design/20261021-i016-p8-palette-variants.md) (the runtime work is rexbox issue #16 on branch `i016-p8-palette-variants`).

(1) Task branch `i011-city-frame-time`; commit titles end with `(#11)`. (2) Measure: narrow the cause with `REXBOX_D3D8_PROFILE`, `REXBOX_SAMPLE` (presents 1700–2300) and temporary counters; also check the game runs at the right speed (main-loop iterations against vblanks). (3) Once the rexbox implementation is in, move `third_party/rexbox` to that commit and build without regenerating. (4) Verify: fps, `setup` and CPU over the same spans, dumps of frames 1500 and 2100, CTest. (5) Analysis section 7 and the work log `docs/work-logs/20261021-i011-city-frame-time.md`.
