# decomp/

손으로 디컴파일한 함수가 들어가는 자리다. 각 파일은 대체하는 lift 함수와 같은 심볼 `void sub_XXXXXXXX(void)`을 정의하고, `scripts/recomp/regenerate.py`(파이프라인은 `scripts/recomp/lifter.py`)가 그 정의를 lifter의 `--exclude-manual`로 넘긴다. 규칙은 [docs/CODING_STYLE.md](../../../../docs/CODING_STYLE.md)의 "재컴파일 코드와 디컴파일 코드" 항목이다. 아직 함수가 없다(게임 XBE가 준비되면 시작).

Hand-decompiled functions go here. Each file defines the same symbol as the lifted function it replaces, `void sub_XXXXXXXX(void)`, and `scripts/recomp/regenerate.py` (pipeline in `scripts/recomp/lifter.py`) hands those definitions to the lifter as `--exclude-manual`. The rules are the "Recompiled and Decompiled Code" section of [docs/CODING_STYLE.md](../../../../docs/CODING_STYLE.md). No function yet (work starts once the game XBE is in place).
