// Title profile: Crazy Taxi 3: High Roller.
//
// The only place where the title's identity and title-specific bindings are
// declared. The reusable runtime (rexbox) asks nothing about the title; the
// app hands it this profile.

#ifndef RECT3_CT3_PROFILE_H
#define RECT3_CT3_PROFILE_H

#include <cstdint>
#include <string>

namespace rexbox::runtime
{
class HleRegistry;
}

namespace rexbox::xbe
{
class XbeImage;
}

struct RexboxXdkVariable;

namespace rect3
{

// One XBE build the project supports, identified by its certificate.
struct XbeVariant
{
    const char* label;
    uint32_t title_id;
    uint32_t certificate_time;
    uint32_t certificate_version;
    uint32_t image_time;
};

// The variant `image` matches, or nullptr for an unsupported (or not yet
// registered) build.
const XbeVariant* IdentifyXbe(const rexbox::xbe::XbeImage& image);

// The variant the recompiled code and title bindings are made for: the
// Japan build (docs/PROJECT_CHARTER.md).
const XbeVariant* PrimaryVariant();

// Binds what the HLE needs to know about the title's statically linked XDK
// code: the identified XDK variables (device types and the like), for the
// primary variant only. XDK functions reach the HLE through generated
// overrides instead. Returns the number of bindings made.
size_t BindTitleHle(const rexbox::xbe::XbeImage& image, rexbox::runtime::HleRegistry& registry);

using RecompiledFunction = void (*)();

using RecompiledLookup = RecompiledFunction (*)(uint32_t guest_va);

// The generated guest-VA-to-function lookup, or nullptr when the build has no
// recompiled code (gen/ was not generated). Lifted for the primary variant only.
RecompiledLookup RecompiledLookupFunction();

// The XDK variables identified in the primary variant (generated with the
// recompiled code; the table ends with a null name), or nullptr when the build
// has no recompiled code.
const RexboxXdkVariable* RecompiledXdkVariables();

// Builds the generated code's flat dispatch table. Returns false when there is
// no recompiled code or the table could not be built (lookups then fall back
// to a binary search).
bool InitializeRecompiledDispatch();

}  // namespace rect3

#endif  // RECT3_CT3_PROFILE_H
