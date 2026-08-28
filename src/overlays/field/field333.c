#include "common.h"

/**
 * @note NOT YET MATCHED (98.02%). residue is a 2-insn scheduling difference: the preheader init order (i=0 vs base copy) and one loop-back branch-delay slot the target leaves as nop but gcc fills. Not reachable by source shape (permuter found no real gain).
 * @see decomp.me (98.02%) TODO
 */
void func_800A3E10(s32 arg0, s32 arg1, s32 arg2)
{
    s32 base;
    s32 i;
    s32 mask;

    if (arg2 * 3 < 0x18)
    {
        base = arg2 * 3;
        for (i = 0; i < 3; i++)
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
