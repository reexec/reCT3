# decomp/

손으로 디컴파일한 함수가 들어가는 자리다. 각 파일은 대체하는 lift 함수와 같은 심볼 `void sub_XXXXXXXX(void)`을 정의하고, `scripts/recomp/regenerate.py`(파이프라인은 `scripts/recomp/lifter.py`)가 그 정의를 lifter의 `--exclude-manual`로 넘긴다. 파일이 `extern void sub_XXXXXXXX_gen(void);`을 선언하면 lift 본문이 그 이름으로 남고, `rect3_decomp_check`(`tools/decomp_check/`)가 둘을 같은 게스트 상태에서 돌려 비교한다. 규칙은 [docs/CODING_STYLE.md](../../../../docs/CODING_STYLE.md)의 "재컴파일 코드와 디컴파일 코드" 항목이다.

| 파일 | 함수 | 근거 |
|---|---|---|
| `d3dx_matrix.c` | `000E4B73` D3DXVec4Transform, `000E4BB9` D3DXMatrixMultiply, `000E4CB7` D3DXMatrixInverse(정체는 **추정**) | issue #27, [분석](../../../../docs/analysis/d3dx-matrix.md) |

이 디렉터리의 파일을 더하거나 바꾸면 `regenerate.py`를 다시 돌리고, `rect3_decomp_check`에 경우를 더한다.

Hand-decompiled functions go here. Each file defines the same symbol as the lifted function it replaces, `void sub_XXXXXXXX(void)`, and `scripts/recomp/regenerate.py` (pipeline in `scripts/recomp/lifter.py`) hands those definitions to the lifter as `--exclude-manual`. A file that declares `extern void sub_XXXXXXXX_gen(void);` keeps the lifted body under that name, and `rect3_decomp_check` (`tools/decomp_check/`) runs the two from the same guest state and compares them. The rules are the "Recompiled and Decompiled Code" section of [docs/CODING_STYLE.md](../../../../docs/CODING_STYLE.md). Files: `d3dx_matrix.c`, the three D3DX matrix routines above (issue #27, [analysis](../../../../docs/analysis/d3dx-matrix.md)). After adding or changing a file here, run `regenerate.py` again and add cases to `rect3_decomp_check`.
