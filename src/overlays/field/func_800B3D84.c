#include "common.h"

extern u8 *D_80122B74;
/* No prototype is in scope at the original call site, so the arguments pass as
 * plain int (the target does not truncate the field to s16). */
void akao_set_song_params(s32 flags, s32 duration, s32 field_id, s32 sub_id);
void func_800BD520(s32 arg0, u32 arg1, s32 arg2);

/**
 * @brief Restarts field music when the active scene index is out of range.
 *
 * Reads the active scene index at offset 0x2EF0 of the D_80122B74 buffer; if it
 * is 5 or greater it re-arms akao_set_song_params, then forwards the scene
 * entry's 0x2F3C word (stride 0x60) to func_800BD520.
 *
 * Matches under GCC 2.8.0. The pre-diagnostic scene index and the index
 * reloaded afterward are distinct value webs; materializing the second
 * index's 0x60-byte offset reproduces the target allocation exactly.
 */
void func_800B3D84(void)
{
    s32 idx1;
    s32 idx2;
    s32 off;

    idx1 = *(s32 *)(D_80122B74 + 0x2EF0);
    if ((u32)idx1 >= 5)
    {
        akao_set_song_params(0x8001, 0x75, idx1, 0);
    }

    idx2 = *(s32 *)(D_80122B74 + 0x2EF0);
    off = idx2 * 0x60;
    func_800BD520(2, 0xF020, *(s32 *)(D_80122B74 + off + 0x2F3C));
}
