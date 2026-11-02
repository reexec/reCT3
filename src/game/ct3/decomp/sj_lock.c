/* Host-concurrency adapter for CRI's stream-joint critical section (issue #29,
 * docs/design/20261103-i029-sj-lock.md, docs/analysis/cri-middleware.md).
 *
 * Not a decompilation: the lifted bodies run unchanged, with a host lock
 * around them. Every SJ ring-buffer operation (the SJRBF GetChunk, PutChunk,
 * UngetChunk and friends, 0x000B7E80..0x000B82C0 and 0x000B9A20..0x000B9FC0)
 * enters 0x000BC950 first and leaves through 0x000BC970. In this build the two
 * only touch a global ([0x2E05F0]): on the Xbox's single core the CRI servers
 * never ran together, so the section needed no lock. On the host the file
 * server (thread 0x000B3520, filling the input ring after each read) and the
 * ADX server (the vsync thread, taking from it to decode) run in parallel and
 * interleave their updates of one ring's counts and positions. PutChunk then
 * finds a chunk that no longer meets the lane's position, reports error -3 and
 * drops it, and the ring loses those bytes for good; once too few are left
 * the stream stops and the DirectSound buffer replays its last stretch. */

#include "recomp_types.h"
#include "rexbox/runtime/guest_lock.h"

extern void sub_000BC950_gen(void);
extern void sub_000BC970_gen(void);

static RexboxGuestLock g_sj_lock = {"CRI SJ lock", 0};

/* 0x000BC950, cdecl: void (void)
 *
 * SJ critical section: enter. Recursive per thread through the guest lock. */
void sub_000BC950(void)
{
    rexbox_guest_lock_enter(&g_sj_lock);
    sub_000BC950_gen();
}

/* 0x000BC970, cdecl: void (void)
 *
 * SJ critical section: leave. */
void sub_000BC970(void)
{
    sub_000BC970_gen();
    rexbox_guest_lock_leave(&g_sj_lock);
}
