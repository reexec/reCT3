// rect3: what the title's hand-written adapters ask of the host. Plain C so
// the decompiled sources (C) can include it; the application fills the hooks
// in, and rect3_game links without the HLE (the decomp check tool), where
// each hook's null means its 4:3, no-host answer.
#ifndef RECT3_CT3_HOST_H
#define RECT3_CT3_HOST_H

#ifdef __cplusplus
extern "C" {
#endif

// How much wider the picture's horizontal view is than the title's 4:3 one:
// the widened back buffer's width over 640, or 1
// (reJSRF design i097). Null: 1.
extern float (*rect3_host_horizontal_view_scale)(void);

#ifdef __cplusplus
}
#endif

#endif  // RECT3_CT3_HOST_H
