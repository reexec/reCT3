#include "rect3/ct3_profile.h"

#include "rexbox/recomp/recomp_hle_entry.h"
#include "rexbox/runtime/hle_registry.h"
#include "rexbox/xbe/xbe_image.h"

#include <array>
#include <cstdint>

namespace rect3
{

namespace
{

// Certificate identity of each known build (docs/analysis/ct3-xbe-overview.md).
//
// The Japan build is the primary target (MD5 18a8873932612bb8ad86910ffb5d8cfe,
// XDK 4531), registered with scripts/recomp/register_xbe.py, which also
// keeps the MD5 in src/game/ct3/recomp/variants.json. An entry with title id
// 0 is a placeholder IdentifyXbe skips.
constexpr std::array<XbeVariant, 1> kVariants = {{
    // Primary target: the Japan retail build's default.xbe (region 2).
    {"Japan", 0x53450004, 0x3D1FC5C5, 0x00000002, 0x3D088B87},
}};

struct XdkInternal
{
    const char* name;
    uint32_t address;
};

// XDK internals XbSymbolDatabase does not name, found by reading the lifted
// library. The D3D8 HLE device runs "D3D_InitializeDeviceState" (the device's
// default-state routine): the routine Direct3D_CreateDevice reaches through
// 0x000FB140 that sets the identity transforms and the render and
// texture-stage defaults (docs/analysis/ct3-xbe-overview.md section 4).
constexpr std::array<XdkInternal, 1> kXdkInternals = {{
    {"D3D_InitializeDeviceState", 0x000FAA20},
}};

bool Registered(const XbeVariant& variant)
{
    return variant.title_id != 0;
}

}  // namespace

const XbeVariant* IdentifyXbe(const rexbox::xbe::XbeImage& image)
{
    const auto& cert = image.certificate();
    for (const XbeVariant& variant : kVariants)
    {
        if (Registered(variant) && cert.title_id == variant.title_id && cert.time_date == variant.certificate_time &&
            cert.version == variant.certificate_version && image.time_date() == variant.image_time)
        {
            return &variant;
        }
    }
    return nullptr;
}

const XbeVariant* PrimaryVariant()
{
    return &kVariants[0];
}

size_t BindTitleHle(const rexbox::xbe::XbeImage& image, rexbox::runtime::HleRegistry& registry)
{
    // The variable addresses were identified in the primary variant's XBE.
    const RexboxXdkVariable* variables = RecompiledXdkVariables();
    if (variables == nullptr || IdentifyXbe(image) != PrimaryVariant())
    {
        return 0;
    }
    size_t count = 0;
    for (; variables[count].name != nullptr; ++count)
    {
        registry.BindVariable(variables[count].name, variables[count].address);
    }
    for (const auto& internal : kXdkInternals)
    {
        registry.BindVariable(internal.name, internal.address);
        ++count;
    }
    return count;
}

}  // namespace rect3
