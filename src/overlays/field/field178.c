#include "common.h"

extern s16 D_80122C06;
extern s16 g_akao_song_cmd_arg0;
extern u8 D_800459AF;

/**
 * @see decomp.me (100%) N/A -- trivial 7-instruction leaf function, no scratch needed.
 */
void func_800C6834(void)
{
    s32 temp = D_80122C06;
    g_akao_song_cmd_arg0 = temp;
    D_800459AF = temp;
}
