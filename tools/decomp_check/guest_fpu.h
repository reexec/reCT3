/* rect3_decomp_check: guest x87 and SEH state (guest_fpu.c). */

#ifndef RECT3_TOOLS_DECOMP_CHECK_GUEST_FPU_H
#define RECT3_TOOLS_DECOMP_CHECK_GUEST_FPU_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void rect3_check_set_fpu(uint16_t control_word, uint32_t top);
void rect3_check_get_fpu(uint16_t* control_word, uint32_t* top);
/* The eight physical x87 slots (double-backed), by physical index. */
void rect3_check_set_fpu_stack(const double slots[8]);
void rect3_check_get_fpu_stack(double slots[8]);
void rect3_check_set_seh_ebp(uint32_t value);
uint32_t rect3_check_get_seh_ebp(void);

#ifdef __cplusplus
}
#endif

#endif  // RECT3_TOOLS_DECOMP_CHECK_GUEST_FPU_H
