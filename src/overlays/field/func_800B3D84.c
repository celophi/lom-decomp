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
 * WIP: 92.50%. Body matches; residuals are the epilogue delay-slot fill (the
 * target parks `addiu sp` in the `jr` delay slot) and a register-naming cascade
 * (v0/a1 vs v1/a2) - the same allocation/dbr artifacts that resist source
 * control elsewhere in this overlay.
 */
void func_800B3D84(void)
{
    s32 idx;

    idx = *(s32 *)(D_80122B74 + 0x2EF0);
    if ((u32)idx >= 5)
    {
        akao_set_song_params(0x8001, 0x75, idx, 0);
    }
    idx = *(s32 *)(D_80122B74 + 0x2EF0);
    func_800BD520(2, 0xF020, *(s32 *)(D_80122B74 + idx * 0x60 + 0x2F3C));
}
