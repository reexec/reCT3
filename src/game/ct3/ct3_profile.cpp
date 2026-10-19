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
// The Japan build is the primary target and is not registered yet: the
// values below are filled in by scripts/recomp/register_xbe.py from the
// user's default.xbe (it prints this initializer), together with the MD5 in
// src/game/ct3/recomp/variants.json. Until then IdentifyXbe matches nothing
// and the app reports an unsupported build.
constexpr std::array<XbeVariant, 1> kVariants = {{
    // Primary target: the Japan retail build's default.xbe.
    {"Japan", 0x00000000, 0x00000000, 0x00000000, 0x00000000},
}};

struct XdkInternal
{
    const char* name;
    uint32_t address;
};

// XDK internals XbSymbolDatabase does not name, found by reading the lifted
// library. The D3D8 HLE device runs "D3D_InitializeDeviceState" (the device's
// default-state routine) when the profile binds it; its address in this
// title is unknown until the XBE is analyzed (docs/analysis/ct3-xbe-overview.md).
constexpr std::array<XdkInternal, 0> kXdkInternals = {{}};

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
