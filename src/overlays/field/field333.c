#include "common.h"

/**
 * @brief Finds an available SFX slot in the selected three-bit group.
 *
 * @note 100% match. The function intentionally has no explicit return: the
 *       original codegen preserves the last status/call result in v0.
 */
s32 func_800A3E10(s32 arg0, s32 arg1, s32 arg2)
{
    s32 base;
    s32 i;
    s32 mask;

    if (arg2 * 3 < 0x18)
    {
        i = 0;
        base = arg2 * 3;
        for (; i < 3; i++)
        {
            mask = 1 << (base + i);
            if (!akao_is_sfx_playing(mask))
            {
                akao_play_sfx_from_buffer(arg0, mask, arg1, 0x7F);
                break;
            }
        }
    }
}
