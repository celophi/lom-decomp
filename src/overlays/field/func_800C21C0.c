#include "common.h"

extern u8 *D_80122B74;

void func_800C2228(s32 arg0);
void akao_set_song_params(s32 command, s32 arg1, s32 arg2, s32 arg3);

/**
 * @brief Ticks down a party member's counter, or triggers a fallback song.
 *
 * For a valid member index (@p arg0 < 0xFF), decrements the counter byte at
 * @c D_80122B74[arg0 + 0x25E0] when nonzero and runs func_800C2228; for an
 * out-of-range index, issues akao_set_song_params(0x8001, 0x72, arg0, 0).
 *
 * @param arg0 Party member index, or >= 0xFF to take the song fallback.
 */
void func_800C21C0(s32 arg0)
{
    u8 *rec;
    u8 v;

    if (arg0 < 0xFF)
    {
        rec = D_80122B74 + arg0;
        v = rec[0x25E0];
        if (v != 0)
        {
            rec[0x25E0] = v - 1;
        }
        func_800C2228(arg0);
    }
    else
    {
        akao_set_song_params(0x8001, 0x72, arg0, 0);
    }
}
