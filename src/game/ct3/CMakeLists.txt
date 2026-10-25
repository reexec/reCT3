# rect3_game: everything specific to Crazy Taxi 3: High Roller.
#
#   ct3_profile.cpp    identity of supported XBE builds, HLE address bindings
#   ct3_recomp.cpp     the recompiled entry point, when generated code exists
#   ct3_host.c         the host hooks the decompiled sources call (include/rect3/ct3_host.h)
#   decomp/            hand-decompiled functions that replace lifted ones
#   recomp/            the recompilation configuration (variants, seeds, HLE libraries)
#   <gen dir>          lifted C from scripts/recomp/regenerate.py (gitignored)
file(GLOB RECT3_DECOMP_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/decomp/*.c")

add_library(rect3_game STATIC
    ct3_profile.cpp
    ct3_recomp.cpp
    ct3_host.c
    ${RECT3_DECOMP_SOURCES}
)
target_include_directories(rect3_game PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(rect3_game PUBLIC rexbox_recomp)

# The generated code, when scripts/recomp/regenerate.py has run (rexbox's
# cmake/rexbox_recomp.cmake: ABI header check, warnings off, the cycle with
# rect3_game declared both ways).
rexbox_add_recompiled_library(rect3_recomp GEN_DIR "${RECT3_RECOMP_GEN_DIR}" TITLE rect3_game)
if(TARGET rect3_recomp)
    # Decompiled sources include the generated declarations.
    target_include_directories(rect3_game PRIVATE "${RECT3_RECOMP_GEN_DIR}")
    target_compile_definitions(rect3_game PRIVATE RECT3_HAS_RECOMP=1)
    if(MSVC AND RECT3_DECOMP_SOURCES)
        # The lifter's header as its authors wrote it: its constant casts
        # (C4310, upstream xboxrecomp v0.15.0's x87 helpers) are not ours to
        # change; rexbox's ABI object turns the same warning off.
        set_source_files_properties(${RECT3_DECOMP_SOURCES} PROPERTIES COMPILE_OPTIONS /wd4310)
    endif()
elseif(RECT3_DECOMP_SOURCES)
    # Decompiled functions only make sense next to the code they replace.
    set_source_files_properties(${RECT3_DECOMP_SOURCES} PROPERTIES HEADER_FILE_ONLY ON)
endif()
