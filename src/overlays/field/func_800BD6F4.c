#include "common.h"

extern void (*D_800F0E10[])(s32 value);
extern void akao_set_song_params(s32 command, s32 arg1, s32 arg2, s32 arg3);

/**
 * @brief Dispatches a small field command or forwards it to the audio system.
 *
 * @param value Command value or function-table index.
 * @note 99.783% match. The sole residue is whether the function-table index
 *       uses `a0` or the identical copied value in `a2`.
 */
void func_800BD6F4(s32 value)
{
    if (value < 0x11)
    {
        D_800F0E10[value](value);
        return;
    }
    akao_set_song_params(0x8001, 3, value, 0);
}
