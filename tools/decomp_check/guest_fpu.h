/* rect3_decomp_check: guest x87 and SEH state (guest_fpu.c). */

#ifndef RECT3_TOOLS_DECOMP_CHECK_GUEST_FPU_H
#define RECT3_TOOLS_DECOMP_CHECK_GUEST_FPU_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void rect3_check_set_fpu(uint16_t control_word, uint32_t top);
void rect3_check_get_fpu(uint16_t* control_word, uint32_t* top);
void rect3_check_set_seh_ebp(uint32_t value);
uint32_t rect3_check_get_seh_ebp(void);

#ifdef __cplusplus
}
#endif

#endif  // RECT3_TOOLS_DECOMP_CHECK_GUEST_FPU_H
