#include "common.h"

extern u8 *D_80122B74;
extern s32 func_800BD414(s32 arg0, s32 arg1);
void func_800C2E30(s32 arg0);
void akao_set_song_params(s32 flags, s32 duration, s32 field_id, s32 sub_id);

/**
 * @brief Selects a scene entry or restarts music based on a rolled index.
 *
 * Rolls func_800BD414; an index below 5 is stored to the buffer's 0x2EF0 slot,
 * handed to func_800C2E30, and the buffer's 0xAA9 byte is returned. Otherwise
 * akao_set_song_params is re-armed and 0xFF is returned.
 *
 * WIP: 94.29%. Body matches (27/28 rows); the sole residual is the epilogue
 * delay-slot fill (a maspsx/dbr tooling artifact that the permuter confirms is
 * not source-reachable).
 */
s32 func_800C2DC0(void)
{
    s32 v = func_800BD414(0, 0x1F10);
    s32 ret;

    if ((u32)v < 5)
    {
        *(s32 *)(D_80122B74 + 0x2EF0) = v;
        func_800C2E30(v);
        ret = *(u8 *)(D_80122B74 + 0xAA9);
    }
    else
    {
        akao_set_song_params(0x8001, 0x6E, v, 1);
        ret = 0xFF;
    }
    return ret;
}
