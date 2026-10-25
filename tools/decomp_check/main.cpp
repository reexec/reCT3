// rect3_decomp_check: runs each decompiled function and the lifted body it
// replaced (sub_X_gen) from identical guest states and compares what they
// leave behind: the general registers, g_seh_ebp, XMM0..7, the x87 control
// word and stack top, and all of guest memory.
//
// Usage: rect3_decomp_check
// Exit code 0 when every case matches.

#include "guest_fpu.h"

#include "rexbox/recomp/recomp_abi.h"
#include "rexbox/runtime/guest_memory.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <functional>
#include <limits>
#include <string>
#include <vector>

extern "C"
{
// D3DX matrix routines (src/game/ct3/decomp/d3dx_matrix.c).
void sub_000E4B73(void);
void sub_000E4B73_gen(void);
void sub_000E4BB9(void);
void sub_000E4BB9_gen(void);
void sub_000E4CB7(void);
void sub_000E4CB7_gen(void);
}

namespace
{

using rexbox::runtime::GuestAddress;
using rexbox::runtime::GuestMemory;
using GuestFunction = void (*)();

constexpr GuestAddress kStackTop = 0x00F00000;
constexpr GuestAddress kEsp = kStackTop - 0x100;  // the return address; room below for frames
constexpr GuestAddress kMatrixA = 0x00400000;
constexpr GuestAddress kMatrixB = 0x00400040;
constexpr GuestAddress kOut = 0x00400080;
constexpr GuestAddress kVector = 0x004000C0;
constexpr GuestAddress kUnaligned = 0x00400104;
constexpr GuestAddress kDeterminant = 0x00400200;

// x87 control words: all exceptions masked, rounding to nearest, PC=24/53/64.
constexpr uint16_t kPc24 = 0x007F;
constexpr uint16_t kPc53 = 0x027F;
constexpr uint16_t kPc64 = 0x037F;

void WriteU32(GuestMemory& memory, GuestAddress address, uint32_t value)
{
    std::memcpy(memory.Translate(address), &value, 4);
}

void WriteF32(GuestMemory& memory, GuestAddress address, float value)
{
    std::memcpy(memory.Translate(address), &value, 4);
}

struct GuestState
{
    RexboxRecompGprs gprs = {};
    uint32_t xmm[8][4] = {};
    uint16_t control_word = kPc24;
    uint32_t fpu_top = 0;
    uint32_t seh_ebp = 0;
};

struct Case
{
    std::string name;
    std::function<void(GuestMemory&, GuestState&)> setup;
};

struct Outcome
{
    GuestState state;
    std::vector<uint8_t> memory;
};

Outcome RunFrom(GuestMemory& memory, const std::vector<uint8_t>& initial_memory, const GuestState& initial,
                GuestFunction function)
{
    std::memcpy(memory.base(), initial_memory.data(), initial_memory.size());
    rexbox_recomp_reset_thread(initial.gprs.esp, 0);
    rexbox_recomp_set_gprs(&initial.gprs);
    for (int i = 0; i < 8; ++i)
    {
        rexbox_recomp_set_xmm(i, initial.xmm[i]);
    }
    rect3_check_set_fpu(initial.control_word, initial.fpu_top);
    rect3_check_set_seh_ebp(initial.seh_ebp);
    function();
    Outcome outcome;
    rexbox_recomp_get_gprs(&outcome.state.gprs);
    for (int i = 0; i < 8; ++i)
    {
        rexbox_recomp_get_xmm(i, outcome.state.xmm[i]);
    }
    rect3_check_get_fpu(&outcome.state.control_word, &outcome.state.fpu_top);
    outcome.state.seh_ebp = rect3_check_get_seh_ebp();
    outcome.memory.assign(memory.base(), memory.base() + memory.size());
    return outcome;
}

bool Compare(const char* function, const Case& test, const Outcome& decompiled, const Outcome& lifted)
{
    bool same = true;
    const uint32_t* a = &decompiled.state.gprs.eax;
    const uint32_t* b = &lifted.state.gprs.eax;
    static const char* kNames[] = {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"};
    for (int i = 0; i < 8; ++i)
    {
        if (a[i] != b[i])
        {
            std::printf("  %s %s: %s decompiled %08X lifted %08X\n", function, test.name.c_str(), kNames[i], a[i], b[i]);
            same = false;
        }
    }
    for (int i = 0; i < 8; ++i)
    {
        if (std::memcmp(decompiled.state.xmm[i], lifted.state.xmm[i], 16) != 0)
        {
            const uint32_t* x = decompiled.state.xmm[i];
            const uint32_t* y = lifted.state.xmm[i];
            std::printf("  %s %s: xmm%d decompiled %08X %08X %08X %08X lifted %08X %08X %08X %08X\n", function,
                        test.name.c_str(), i, x[0], x[1], x[2], x[3], y[0], y[1], y[2], y[3]);
            same = false;
        }
    }
    if (decompiled.state.control_word != lifted.state.control_word ||
        decompiled.state.fpu_top != lifted.state.fpu_top)
    {
        std::printf("  %s %s: x87 control/top decompiled %04X/%u lifted %04X/%u\n", function, test.name.c_str(),
                    decompiled.state.control_word, decompiled.state.fpu_top, lifted.state.control_word,
                    lifted.state.fpu_top);
        same = false;
    }
    if (decompiled.state.seh_ebp != lifted.state.seh_ebp)
    {
        std::printf("  %s %s: g_seh_ebp decompiled %08X lifted %08X\n", function, test.name.c_str(),
                    decompiled.state.seh_ebp, lifted.state.seh_ebp);
        same = false;
    }
    if (decompiled.memory != lifted.memory)
    {
        int shown = 0;
        for (size_t i = 0; i < decompiled.memory.size() && shown < 8; i += 4)
        {
            uint32_t x = 0;
            uint32_t y = 0;
            std::memcpy(&x, &decompiled.memory[i], 4);
            std::memcpy(&y, &lifted.memory[i], 4);
            if (x != y)
            {
                std::printf("  %s %s: memory %08zX decompiled %08X lifted %08X\n", function, test.name.c_str(), i, x,
                            y);
                ++shown;
            }
        }
        same = false;
    }
    return same;
}

struct Suite
{
    const char* name;
    GuestFunction decompiled;
    GuestFunction lifted;
    std::vector<Case> cases;
};

// A reproducible float source: an LCG mapped to [-4, 4) with full 24-bit
// significands, so products and sums round in nearly every lane and a change
// of order or grouping shows.
class Random
{
public:
    explicit Random(uint32_t seed) : state_(seed * 2654435761u + 1u) {}

    float Next()
    {
        state_ = state_ * 1664525u + 1013904223u;
        return static_cast<float>(static_cast<int32_t>(state_) / 256) / 2097152.0f;
    }

private:
    uint32_t state_;
};

// Registers and XMM lanes with recognizable values, and stdcall arguments.
void Frame(GuestMemory& m, GuestState& s, uint32_t a1, uint32_t a2, uint32_t a3)
{
    RexboxRecompGprs& r = s.gprs;
    r.eax = 0xDEADBEEF;
    r.ecx = 0x0C0C0C0C;
    r.edx = 0x11111111;
    r.ebx = 0x22222222;
    r.esi = 0x33333333;
    r.edi = 0x44444444;
    r.ebp = 0x55555555;
    r.esp = kEsp;
    s.seh_ebp = 0x66666666;
    for (uint32_t i = 0; i < 8; ++i)
    {
        for (uint32_t j = 0; j < 4; ++j)
        {
            s.xmm[i][j] = 0x7F000000u | (i << 8) | j;
        }
    }
    // Stack contents the callee may overwrite, so a store it misses shows.
    for (GuestAddress a = kEsp - 0x80; a < kEsp + 0x20; a += 4)
    {
        WriteU32(m, a, 0xA5A50000u | (a & 0xFFFF));
    }
    WriteU32(m, kEsp, 0x00012345);  // return address
    WriteU32(m, kEsp + 4, a1);
    WriteU32(m, kEsp + 8, a2);
    WriteU32(m, kEsp + 12, a3);
}

void FillMatrix(GuestMemory& m, GuestAddress address, Random& random)
{
    for (uint32_t k = 0; k < 16; ++k)
    {
        WriteF32(m, address + 4 * k, random.Next());
    }
}

void WriteMatrix(GuestMemory& m, GuestAddress address, const float (&values)[16])
{
    for (uint32_t k = 0; k < 16; ++k)
    {
        WriteF32(m, address + 4 * k, values[k]);
    }
}

constexpr float kNaN = std::numeric_limits<float>::quiet_NaN();
constexpr float kInf = std::numeric_limits<float>::infinity();

// ---- sub_000E4B73: D3DXVec4Transform(pOut, pV, pM).

Suite TransformSuite()
{
    Suite suite{"sub_000E4B73", &sub_000E4B73, &sub_000E4B73_gen, {}};
    // Eight seeds: only four lanes per case, and a regrouped sum differs in
    // some lanes only.
    for (uint32_t seed : {1u, 2u, 3u, 16u, 17u, 18u, 19u, 20u})
    {
        suite.cases.push_back({"random " + std::to_string(seed), [seed](GuestMemory& m, GuestState& s)
                               {
                                   Random random(seed);
                                   FillMatrix(m, kMatrixA, random);
                                   for (uint32_t k = 0; k < 4; ++k)
                                   {
                                       WriteF32(m, kVector + 4 * k, random.Next());
                                   }
                                   Frame(m, s, kOut, kVector, kMatrixA);
                               }});
    }
    suite.cases.push_back({"in place", [](GuestMemory& m, GuestState& s)
                           {
                               Random random(4);
                               FillMatrix(m, kMatrixA, random);
                               for (uint32_t k = 0; k < 4; ++k)
                               {
                                   WriteF32(m, kVector + 4 * k, random.Next());
                               }
                               Frame(m, s, kVector, kVector, kMatrixA);
                           }});
    suite.cases.push_back({"unaligned", [](GuestMemory& m, GuestState& s)
                           {
                               Random random(5);
                               FillMatrix(m, kMatrixA, random);
                               for (uint32_t k = 0; k < 4; ++k)
                               {
                                   WriteF32(m, kUnaligned + 4 * k, random.Next());
                               }
                               Frame(m, s, kUnaligned + 0x20, kUnaligned, kMatrixA);
                           }});
    suite.cases.push_back({"nan and inf", [](GuestMemory& m, GuestState& s)
                           {
                               Random random(6);
                               FillMatrix(m, kMatrixA, random);
                               WriteF32(m, kMatrixA + 0x14, kNaN);
                               WriteF32(m, kMatrixA + 0x28, kInf);
                               WriteU32(m, kMatrixA + 0x3C, 0x7F800001u);  // a signalling NaN
                               const float v[4] = {1.5f, -2.0f, 0.0f, 3.25f};
                               for (uint32_t k = 0; k < 4; ++k)
                               {
                                   WriteF32(m, kVector + 4 * k, v[k]);
                               }
                               Frame(m, s, kOut, kVector, kMatrixA);
                           }});
    return suite;
}

// ---- sub_000E4BB9: D3DXMatrixMultiply(pOut, pA, pB).

Suite MultiplySuite()
{
    Suite suite{"sub_000E4BB9", &sub_000E4BB9, &sub_000E4BB9_gen, {}};
    struct Shape
    {
        const char* name;
        uint32_t out, a, b;
    };
    const Shape shapes[] = {{"product", kOut, kMatrixA, kMatrixB},
                            {"out = A", kMatrixA, kMatrixA, kMatrixB},
                            {"out = B", kMatrixB, kMatrixA, kMatrixB},
                            {"A = B = out", kMatrixA, kMatrixA, kMatrixA}};
    for (uint32_t seed : {1u, 2u, 3u})
    {
        for (const Shape& shape : shapes)
        {
            suite.cases.push_back({std::string(shape.name) + " random " + std::to_string(seed),
                                   [seed, shape](GuestMemory& m, GuestState& s)
                                   {
                                       Random random(seed);
                                       FillMatrix(m, kMatrixA, random);
                                       FillMatrix(m, kMatrixB, random);
                                       Frame(m, s, shape.out, shape.a, shape.b);
                                   }});
        }
    }
    suite.cases.push_back({"nan and inf", [](GuestMemory& m, GuestState& s)
                           {
                               Random random(7);
                               FillMatrix(m, kMatrixA, random);
                               FillMatrix(m, kMatrixB, random);
                               WriteF32(m, kMatrixA + 0x08, kNaN);
                               WriteF32(m, kMatrixB + 0x30, kInf);
                               WriteF32(m, kMatrixB + 0x34, -kInf);
                               Frame(m, s, kOut, kMatrixA, kMatrixB);
                           }});
    return suite;
}

// ---- sub_000E4CB7: D3DXMatrixInverse(pOut, pDeterminant, pM).

Suite InverseSuite()
{
    Suite suite{"sub_000E4CB7", &sub_000E4CB7, &sub_000E4CB7_gen, {}};
    struct Precision
    {
        const char* name;
        uint16_t control_word;
    };
    const Precision precisions[] = {{"pc24", kPc24}, {"pc53", kPc53}, {"pc64", kPc64}};
    for (const Precision& pc : precisions)
    {
        for (uint32_t seed : {1u, 2u, 3u})
        {
            suite.cases.push_back({std::string(pc.name) + " random " + std::to_string(seed),
                                   [seed, pc](GuestMemory& m, GuestState& s)
                                   {
                                       Random random(seed);
                                       FillMatrix(m, kMatrixA, random);
                                       Frame(m, s, kOut, kDeterminant, kMatrixA);
                                       s.control_word = pc.control_word;
                                   }});
        }
        suite.cases.push_back({std::string(pc.name) + " rigid", [pc](GuestMemory& m, GuestState& s)
                               {
                                   // A rotation about z with a translation: the game's usual input.
                                   const float c = 0.8f;
                                   const float n = 0.6f;
                                   const float rigid[16] = {c, n, 0, 0, -n, c, 0, 0, 0, 0, 1, 0, 12.5f, -3.0f, 7.25f, 1};
                                   WriteMatrix(m, kMatrixA, rigid);
                                   Frame(m, s, kOut, kDeterminant, kMatrixA);
                                   s.control_word = pc.control_word;
                               }});
        suite.cases.push_back({std::string(pc.name) + " singular", [pc](GuestMemory& m, GuestState& s)
                               {
                                   // Two equal rows: determinant 0, 1/0 is infinite, the failure path.
                                   Random random(8);
                                   FillMatrix(m, kMatrixA, random);
                                   for (uint32_t k = 0; k < 4; ++k)
                                   {
                                       std::memcpy(m.Translate(kMatrixA + 0x20 + 4 * k),
                                                   m.Translate(kMatrixA + 0x10 + 4 * k), 4);
                                   }
                                   Frame(m, s, kOut, kDeterminant, kMatrixA);
                                   s.control_word = pc.control_word;
                               }});
        suite.cases.push_back({std::string(pc.name) + " large", [pc](GuestMemory& m, GuestState& s)
                               {
                                   // Products beyond float's range: the x87 keeps its exponent.
                                   Random random(9);
                                   for (uint32_t k = 0; k < 16; ++k)
                                   {
                                       WriteF32(m, kMatrixA + 4 * k, random.Next() * 1.0e30f);
                                   }
                                   Frame(m, s, kOut, kDeterminant, kMatrixA);
                                   s.control_word = pc.control_word;
                               }});
        suite.cases.push_back({std::string(pc.name) + " small", [pc](GuestMemory& m, GuestState& s)
                               {
                                   // Products below float's normal range.
                                   Random random(10);
                                   for (uint32_t k = 0; k < 16; ++k)
                                   {
                                       WriteF32(m, kMatrixA + 4 * k, random.Next() * 1.0e-12f);
                                   }
                                   Frame(m, s, kOut, kDeterminant, kMatrixA);
                                   s.control_word = pc.control_word;
                               }});
    }
    suite.cases.push_back({"no determinant", [](GuestMemory& m, GuestState& s)
                           {
                               Random random(11);
                               FillMatrix(m, kMatrixA, random);
                               Frame(m, s, kOut, 0, kMatrixA);
                           }});
    suite.cases.push_back({"in place", [](GuestMemory& m, GuestState& s)
                           {
                               Random random(12);
                               FillMatrix(m, kMatrixA, random);
                               Frame(m, s, kMatrixA, kDeterminant, kMatrixA);
                           }});
    suite.cases.push_back({"nan", [](GuestMemory& m, GuestState& s)
                           {
                               Random random(13);
                               FillMatrix(m, kMatrixA, random);
                               WriteF32(m, kMatrixA + 0x18, kNaN);
                               Frame(m, s, kOut, kDeterminant, kMatrixA);
                           }});
    suite.cases.push_back({"signalling nan in a copied cell", [](GuestMemory& m, GuestState& s)
                           {
                               // m0, m4, m8 and m12 are copied as raw dwords into the frame.
                               Random random(14);
                               FillMatrix(m, kMatrixA, random);
                               WriteU32(m, kMatrixA + 0x10, 0x7F800001u);
                               Frame(m, s, kOut, kDeterminant, kMatrixA);
                           }});
    suite.cases.push_back({"x87 top 5", [](GuestMemory& m, GuestState& s)
                           {
                               // An empty stack whose top is not slot 0.
                               Random random(15);
                               FillMatrix(m, kMatrixA, random);
                               Frame(m, s, kOut, kDeterminant, kMatrixA);
                               s.fpu_top = 5;
                           }});
    return suite;
}

}  // namespace

int main()
{
    // 16 MiB is enough for these cases and keeps the per-case copies cheap.
    GuestMemory memory;
    if (!memory.Initialize(16u * 1024u * 1024u))
    {
        std::fprintf(stderr, "cannot reserve guest memory\n");
        return 2;
    }
    rexbox_recomp_set_memory(reinterpret_cast<ptrdiff_t>(memory.base()), 0, 0);

    const std::vector<Suite> suites = {TransformSuite(), MultiplySuite(), InverseSuite()};

    size_t total = 0;
    int failures = 0;
    for (const Suite& suite : suites)
    {
        for (const Case& test : suite.cases)
        {
            std::memset(memory.base(), 0, memory.size());
            GuestState initial;
            test.setup(memory, initial);
            const std::vector<uint8_t> initial_memory(memory.base(), memory.base() + memory.size());
            const Outcome decompiled = RunFrom(memory, initial_memory, initial, suite.decompiled);
            const Outcome lifted = RunFrom(memory, initial_memory, initial, suite.lifted);
            const bool same = Compare(suite.name, test, decompiled, lifted);
            std::printf("%s %s %s\n", same ? "match" : "DIFF ", suite.name, test.name.c_str());
            failures += same ? 0 : 1;
            ++total;
        }
    }
    std::printf("%zu case(s), %d difference(s)\n", total, failures);
    return failures == 0 ? 0 : 1;
}
