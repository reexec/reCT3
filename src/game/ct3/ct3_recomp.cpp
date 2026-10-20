#include "rect3/ct3_profile.h"

#include "rexbox/recomp/recomp_hle_entry.h"

#if defined(RECT3_HAS_RECOMP)
extern "C" rect3::RecompiledFunction recomp_lookup(uint32_t xbox_va);
extern "C" int recomp_dispatch_init(void);
extern "C" const RexboxXdkVariable rexbox_xdk_variables[];
#endif

namespace rect3
{

RecompiledLookup RecompiledLookupFunction()
{
#if defined(RECT3_HAS_RECOMP)
    return &recomp_lookup;
#else
    return nullptr;
#endif
}

const RexboxXdkVariable* RecompiledXdkVariables()
{
#if defined(RECT3_HAS_RECOMP)
    return rexbox_xdk_variables;
#else
    return nullptr;
#endif
}

bool InitializeRecompiledDispatch()
{
#if defined(RECT3_HAS_RECOMP)
    return recomp_dispatch_init() != 0;
#else
    return false;
#endif
}

}  // namespace rect3
