# Title-specific tools. Each needs the generated code, so this directory is
# added only when rect3_recomp exists (scripts/recomp/regenerate.py has run).
#
#   decomp_check/   rect3_decomp_check: every decompiled function against the
#                   lifted body it replaced (src/game/ct3/decomp/)

add_executable(rect3_decomp_check decomp_check/main.cpp decomp_check/guest_fpu.c)
target_link_libraries(rect3_decomp_check PRIVATE rect3_game)
# guest_fpu.c reads the x87 and SEH state through the generated code's header.
target_include_directories(rect3_decomp_check PRIVATE "${RECT3_RECOMP_GEN_DIR}")
rexbox_strict_warnings(rect3_decomp_check)
if(MSVC)
    # The lifter's header's constant casts (C4310), as for the decompiled sources.
    set_source_files_properties(decomp_check/guest_fpu.c PROPERTIES COMPILE_OPTIONS /wd4310)
endif()
add_test(NAME rect3_decomp_check COMMAND rect3_decomp_check)
