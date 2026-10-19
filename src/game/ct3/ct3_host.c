/* The host hooks of include/rect3/ct3_host.h: defined here, outside
 * decomp/, because the decompiled sources are compiled only next to the
 * generated code, while the application sets the hooks in every build. */
#include "rect3/ct3_host.h"

float (*rect3_host_horizontal_view_scale)(void) = 0;
