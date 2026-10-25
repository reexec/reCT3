/* Decompiled CT3 functions: three matrix routines of the statically linked
 * D3DX library (issue #27, docs/design/20261026-i027-matrix-decomp.md,
 * docs/analysis/d3dx-matrix.md).
 *
 * Matrices are row-major 4x4 floats (64 bytes); vectors are row vectors, so
 * v' = x*R0 + y*R1 + z*R2 + w*R3. All three are stdcall with three arguments
 * and return the result pointer in eax.
 *
 * The originals lift to bodies that pay more for the guest-register, XMM-lane
 * and x87-stack emulation than for their arithmetic. These versions compute
 * the same operations in the same order, grouping and operand order, in plain
 * C (no intrinsics: this code builds for every target):
 *   - the SSE routines are lane-wise IEEE single precision, bit-identical to
 *     the lifted lane-wise C;
 *   - the x87 routine works in double and rounds wherever the lifted body
 *     does, by the guest's precision control (recomp_fp_round24 at PC=24),
 *     and converts to float wherever the original stores a float.
 * They leave the general registers, XMM0..7, the x87 stack and the memory
 * (including the stores below esp and the argument slot the inverse
 * overwrites) as the originals do; rect3_decomp_check compares all of it. */

#include "recomp_types.h"

#include <string.h>

extern void sub_000E4B73_gen(void);
extern void sub_000E4BB9_gen(void);
extern void sub_000E4CB7_gen(void);

/* ---- XMM lanes ---------------------------------------------------------- */

typedef struct Lanes
{
    float f[4];
} Lanes;

static __inline float LoadFloat(uint32_t addr)
{
    const uint32_t bits = MEM32(addr);
    float value;
    memcpy(&value, &bits, 4);
    return value;
}

static __inline void StoreFloat(uint32_t addr, float value)
{
    uint32_t bits;
    memcpy(&bits, &value, 4);
    MEM32(addr) = bits;
}

/* movups / movaps: 16 bytes, bit patterns unchanged. */
static __inline Lanes Load4(uint32_t addr)
{
    Lanes r;
    for (int i = 0; i < 4; ++i)
    {
        r.f[i] = LoadFloat(addr + 4u * (uint32_t)i);
    }
    return r;
}

static __inline void Store4(uint32_t addr, Lanes v)
{
    for (int i = 0; i < 4; ++i)
    {
        StoreFloat(addr + 4u * (uint32_t)i, v.f[i]);
    }
}

/* shufps x, x, lane * 0x55: every lane from one lane. */
static __inline Lanes Broadcast(Lanes v, int lane)
{
    const float x = v.f[lane];
    Lanes r = {{x, x, x, x}};
    return r;
}

/* mulps, addps: lane by lane, first operand first. */
static __inline Lanes Mul(Lanes a, Lanes b)
{
    Lanes r = {{a.f[0] * b.f[0], a.f[1] * b.f[1], a.f[2] * b.f[2], a.f[3] * b.f[3]}};
    return r;
}

static __inline Lanes Add(Lanes a, Lanes b)
{
    Lanes r = {{a.f[0] + b.f[0], a.f[1] + b.f[1], a.f[2] + b.f[2], a.f[3] + b.f[3]}};
    return r;
}

#define SET_XMM(reg, v) memcpy(&(reg), (v).f, 16)

/* 0x000E4B73, stdcall (D3DXVECTOR4* pOut, const D3DXVECTOR4* pV,
 * const D3DXMATRIX* pM): pOut = pV * pM. Inferred: D3DXVec4Transform.
 * eax = pOut; xmm1 = y*R1, xmm2 = result, xmm3 = z*R2, xmm4 = w*R3. */
void sub_000E4B73(void)
{
    const uint32_t esp = g_esp;
    const uint32_t out = MEM32(esp + 4);
    const uint32_t m = MEM32(esp + 0xC);
    const Lanes v = Load4(MEM32(esp + 8));
    const Lanes x = Mul(Broadcast(v, 0), Load4(m));
    const Lanes y = Mul(Broadcast(v, 1), Load4(m + 0x10));
    const Lanes z = Mul(Broadcast(v, 2), Load4(m + 0x20));
    const Lanes w = Mul(Broadcast(v, 3), Load4(m + 0x30));
    const Lanes result = Add(Add(Add(x, y), z), w);
    Store4(out, result);
    SET_XMM(g_xmm1, y);
    SET_XMM(g_xmm2, result);
    SET_XMM(g_xmm3, z);
    SET_XMM(g_xmm4, w);
    g_eax = out;
    g_esp = esp + 16; /* ret 12 */
}

/* 0x000E4BB9, stdcall (D3DXMATRIX* pOut, const D3DXMATRIX* pA,
 * const D3DXMATRIX* pB): pOut = pA * pB. Inferred: D3DXMatrixMultiply.
 * Every row is computed before any is stored, so pOut may be pA or pB.
 * eax = ecx = pOut; xmm2..5 = the rows, xmm0 = A3.z*B2, xmm1 = A3.w*B3. */
void sub_000E4BB9(void)
{
    const uint32_t esp = g_esp;
    const uint32_t a = MEM32(esp + 8);
    const uint32_t b = MEM32(esp + 0xC);
    const Lanes b0 = Load4(b);
    const Lanes b1 = Load4(b + 0x10);
    const Lanes b2 = Load4(b + 0x20);
    const Lanes b3 = Load4(b + 0x30);
    Lanes rows[4];
    Lanes cz = {{0.0f, 0.0f, 0.0f, 0.0f}};
    Lanes cw = cz;
    for (uint32_t r = 0; r < 4; ++r)
    {
        const Lanes ar = Load4(a + r * 0x10);
        const Lanes cx = Mul(Broadcast(ar, 0), b0);
        const Lanes cy = Mul(Broadcast(ar, 1), b1);
        cz = Mul(Broadcast(ar, 2), b2);
        cw = Mul(Broadcast(ar, 3), b3);
        rows[r] = Add(Add(Add(cx, cy), cz), cw);
    }
    const uint32_t out = MEM32(esp + 4);
    for (uint32_t r = 0; r < 4; ++r)
    {
        Store4(out + r * 0x10, rows[r]);
    }
    SET_XMM(g_xmm0, cz);
    SET_XMM(g_xmm1, cw);
    SET_XMM(g_xmm2, rows[0]);
    SET_XMM(g_xmm3, rows[1]);
    SET_XMM(g_xmm4, rows[2]);
    SET_XMM(g_xmm5, rows[3]);
    g_ecx = out;
    g_eax = out;
    g_esp = esp + 16; /* ret 12 */
}

/* ---- x87 ---------------------------------------------------------------- */

/* One x87 arithmetic result under the guest's precision control, as
 * RECOMP_FP_PC rounds it: to float at PC=24, unchanged otherwise. */
static __inline double Pc(int pc24, double x)
{
    return pc24 ? recomp_fp_round24(x) : x;
}

/* fstp dword: the value as the float the original stores and reloads. */
static __inline double AsFloat(double x)
{
    return (double)(float)x;
}

/* 0x000E4CB7, stdcall (D3DXMATRIX* pOut, float* pDeterminant,
 * const D3DXMATRIX* pM): pOut = inverse of pM by cofactors, and
 * *pDeterminant = det(pM) when pDeterminant is not NULL. Returns pOut, or 0
 * (leaving pOut untouched) when 1/det is not finite. Inferred:
 * D3DXMatrixInverse.
 *
 * x87 code with an ebp frame of 0x54 bytes. The 2x2 minors of columns 0-1
 * (lo_ij for rows i and j) and the two of columns 2-3 for rows 0-1 and 0-2
 * are stored as floats; the other four minors of columns 2-3 stay on the
 * x87 stack, at its precision. The original uses all eight x87 registers, so
 * it is only correct on an empty x87 stack; this version leaves the top
 * unchanged and the eight slots holding what the original's pushes left. */
void sub_000E4CB7(void)
{
    const uint32_t esp = g_esp;       /* -> return address */
    const uint32_t frame = esp - 4u;  /* ebp after push ebp; mov ebp, esp */
    const uint32_t mp = MEM32(esp + 0xC);
    const int pc24 = (g_fp_control_word & 0x300u) == 0;
#define R(x) Pc(pc24, (x))

    /* The input, read once (the original reads m0, m4, m5, m8, m9, m12 and
     * m13 twice, with no store in between). raw* are the bit patterns the
     * original copies through ecx into its frame. */
    double m[16];
    for (int k = 0; k < 16; ++k)
    {
        m[k] = (double)LoadFloat(mp + 4u * (uint32_t)k);
    }
    const uint32_t raw0 = MEM32(mp + 0x00);
    const uint32_t raw4 = MEM32(mp + 0x10);
    const uint32_t raw8 = MEM32(mp + 0x20);
    const uint32_t raw12 = MEM32(mp + 0x30);

    /* Minors of columns 0-1, stored as floats. */
    const double lo01 = AsFloat(R(R(m[5] * m[0]) - R(m[4] * m[1])));
    const double lo02 = AsFloat(R(R(m[9] * m[0]) - R(m[8] * m[1])));
    const double lo03 = AsFloat(R(R(m[13] * m[0]) - R(m[12] * m[1])));
    const double lo12 = AsFloat(R(R(m[9] * m[4]) - R(m[8] * m[5])));
    const double lo13 = AsFloat(R(R(m[13] * m[4]) - R(m[12] * m[5])));
    const double lo23 = AsFloat(R(R(m[13] * m[8]) - R(m[12] * m[9])));

    /* Cofactors of columns 2-3 (inverse rows 2 and 3, transposed). */
    const double c15 = AsFloat(R(R(R(m[2] * lo12) - R(m[6] * lo02)) + R(m[10] * lo01)));
    const double c14 = AsFloat(R(R(R(m[6] * lo03) - R(m[14] * lo01)) - R(m[2] * lo13)));
    const double c13 = AsFloat(R(R(R(m[2] * lo23) - R(m[10] * lo03)) + R(m[14] * lo02)));
    const double c12 = AsFloat(R(R(R(m[10] * lo13) - R(m[14] * lo12)) - R(m[6] * lo23)));
    const double c11 = AsFloat(R(R(R(lo02 * m[7]) - R(lo01 * m[11])) - R(lo12 * m[3])));
    const double c10 = AsFloat(R(R(R(lo13 * m[3]) - R(lo03 * m[7])) + R(lo01 * m[15])));
    const double c9 = AsFloat(R(R(R(lo03 * m[11]) - R(lo02 * m[15])) - R(lo23 * m[3])));
    const double c8 = AsFloat(R(R(R(lo23 * m[7]) - R(lo13 * m[11])) + R(lo12 * m[15])));

    /* Minors of columns 2-3: hi01 and hi02 stored, the rest on the stack. */
    const double hi01 = AsFloat(R(R(m[2] * m[7]) - R(m[6] * m[3])));
    const double hi02 = AsFloat(R(R(m[2] * m[11]) - R(m[10] * m[3])));
    const double hi12 = R(R(m[6] * m[11]) - R(m[10] * m[7]));
    const double hi13 = R(R(m[6] * m[15]) - R(m[14] * m[7]));
    const double hi03 = R(R(m[2] * m[15]) - R(m[14] * m[3]));
    const double hi23 = R(R(m[10] * m[15]) - R(m[14] * m[11]));

    /* Cofactors of columns 0-1 (inverse rows 0 and 1, transposed). */
    const double c3 = AsFloat(R(R(R(hi02 * m[5]) - R(hi01 * m[9])) - R(hi12 * m[1])));
    const double c2 = AsFloat(R(R(R(hi13 * m[1]) - R(hi03 * m[5])) + R(hi01 * m[13])));
    const double c1 = AsFloat(R(R(R(hi03 * m[9]) - R(hi02 * m[13])) - R(hi23 * m[1])));
    const double c0_last = R(hi12 * m[13]);  /* left on the x87 stack */
    const double c0_x87 = R(R(R(hi23 * m[5]) - R(hi13 * m[9])) + c0_last);
    const double c0 = AsFloat(c0_x87);
    const double c7 = AsFloat(R(R(R(hi12 * m[0]) - R(hi02 * m[4])) + R(hi01 * m[8])));
    const double c6 = AsFloat(R(R(R(hi03 * m[4]) - R(hi01 * m[12])) - R(hi13 * m[0])));
    const double c5 = AsFloat(R(R(R(m[0] * hi23) - R(hi03 * m[8])) + R(hi02 * m[12])));
    const double c4_last = R(hi23 * m[4]);  /* left on the x87 stack */
    const double c4_x87 = R(R(R(hi13 * m[8]) - R(hi12 * m[12])) - c4_last);
    const double c4 = AsFloat(c4_x87);

    const double det = R(R(R(R(c3 * m[12]) + R(c2 * m[8])) + R(c1 * m[4])) + R(c0 * m[0]));
    const uint32_t det_out = MEM32(esp + 8);
    if (det_out != 0)
    {
        StoreFloat(det_out, (float)det);
    }
    const double inv_x87 = R(1.0 / det);
    const double inv = AsFloat(inv_x87);
#undef R

    /* The frame as the original leaves it: the saved ebp and the final value
     * of every slot it wrote (ebp-relative offsets in the comments). */
    MEM32(frame) = g_ebp;
    StoreFloat(frame + 0x10, (float)c7);  /* +0x10: the pM argument slot */
    StoreFloat(frame - 0x04, (float)c6);
    StoreFloat(frame - 0x08, (float)inv);
    StoreFloat(frame - 0x0C, (float)hi01);
    StoreFloat(frame - 0x10, (float)c3);
    MEM32(frame - 0x14) = raw12;
    MEM32(frame - 0x18) = raw8;
    MEM32(frame - 0x1C) = raw4;
    StoreFloat(frame - 0x20, (float)c5);
    StoreFloat(frame - 0x24, (float)c2);
    StoreFloat(frame - 0x28, (float)c1);
    StoreFloat(frame - 0x2C, (float)c8);
    StoreFloat(frame - 0x30, (float)c4);
    StoreFloat(frame - 0x34, (float)c0);
    MEM32(frame - 0x38) = raw0;
    StoreFloat(frame - 0x3C, (float)c9);
    StoreFloat(frame - 0x40, (float)c10);
    StoreFloat(frame - 0x44, (float)c11);
    StoreFloat(frame - 0x48, (float)c12);
    StoreFloat(frame - 0x4C, (float)c13);
    StoreFloat(frame - 0x50, (float)c14);
    StoreFloat(frame - 0x54, (float)c15);

    /* _finite((double)inv) (0x000E6FA1, cdecl): its argument qword and the
     * return address of the call stay below the frame. eax = ecx = finite;
     * then ecx takes the two argument dwords as the caller pops them. */
    const uint32_t arg = frame - 0x54u - 8u;
    uint64_t inv_bits;
    memcpy(&inv_bits, &inv, 8);
    MEM32(arg) = (uint32_t)inv_bits;
    MEM32(arg + 4) = (uint32_t)(inv_bits >> 32);
    MEM32(arg - 4) = 0x000E4FAFu;
    const int finite = ((inv_bits >> 52) & 0x7FFu) != 0x7FFu;
    g_ecx = (uint32_t)(inv_bits >> 32);

    /* The x87 registers the original pushed keep their last values after
     * being popped; they are left the same, so even a caller that read an
     * empty register would see what it saw before. Slot -k is the k-th push
     * below the entry top. */
    double second_slot = inv_x87;  /* the last value in slot -2 */
    if (finite)
    {
        const double cofactor[16] = {c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15};
        const uint32_t out = MEM32(esp + 4);
        for (int k = 0; k < 16; ++k)
        {
            second_slot = Pc(pc24, cofactor[k] * inv);
            StoreFloat(out + 4u * (uint32_t)k, (float)second_slot);
        }
        g_eax = out;
    }
    else
    {
        g_eax = 0;
    }
    const uint32_t top = (uint32_t)g_fp_top;
    g_fp_stack[(top + 7u) & 7u] = inv;
    g_fp_stack[(top + 6u) & 7u] = second_slot;
    g_fp_stack[(top + 5u) & 7u] = hi13;
    g_fp_stack[(top + 4u) & 7u] = hi23;
    g_fp_stack[(top + 3u) & 7u] = c4_x87;
    g_fp_stack[(top + 2u) & 7u] = c4_last;
    g_fp_stack[(top + 1u) & 7u] = c0_x87;
    g_fp_stack[top & 7u] = c0_last;
    g_ebp = frame;
    g_seh_ebp = frame;
    g_esp = esp + 16; /* leave; ret 12 */
}
