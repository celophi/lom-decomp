#include "common.h"

extern u8 *D_80122B74;

/**
 * @brief Re-arm the pending-input record for a field object, or notify audio.
 *
 * For an in-range @p arg0 (< 5), resolves the object's 0x60-stride record at
 * @c D_80122B74 + 0x2EF4 and hands it to func_800B2844 and field_run_name_entry (using
 * the record's @c 0x2F09 count byte). Out-of-range indices notify the audio
 * driver instead.
 *
 * @param arg0 Field-object index; >= 5 takes the audio-notification path.
 * @see decomp.me (100%) TODO
 */
void func_800C25A0(s32 arg0)
{
    s32 temp_s1;
    s32 off;
    u8 *addr;

    if (arg0 >= 5)
    {
        akao_set_song_params(0x8001, 0x77, arg0, 0);
        return;
    }
    /* Force the D_80122B74 base high-half to materialize before the index. */
    if (D_80122B74)
    {
    }
    temp_s1 = arg0 * 3 << 5;
    off = temp_s1 + 0x2EF4;
    func_800B2844(0, D_80122B74 + off, 0x15);
    addr = D_80122B74 + off;
    field_run_name_entry(addr, addr, 3, *(u8 *)(D_80122B74 + temp_s1 + 0x2F09), 0);
}
