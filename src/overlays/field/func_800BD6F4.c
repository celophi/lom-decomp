#include "common.h"

typedef void (*FieldCommandHandler)(s32 value, u8 *params);

extern FieldCommandHandler D_800F0E10[];
extern void akao_set_song_params(s32 command, s32 arg1, s32 arg2, s32 arg3);

/**
 * @brief Dispatches a small field command or forwards it to the audio system.
 *
 * @param value Command value or function-table index.
 * @param params Parameter block forwarded to table-dispatched commands.
 * @note 100% match. The second argument is part of the original handler ABI;
 *       preserving it also reproduces the target index register lifetime.
 */
void func_800BD6F4(s32 value, u8 *params)
{
    if (value < 0x11)
    {
        D_800F0E10[value](value, params);
        return;
    }
    akao_set_song_params(0x8001, 3, value, 0);
}
