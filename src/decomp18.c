#include "decomp4.h"

extern s32 D_8004F76C[];

void func_8002DDDC(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_8002DFA4(s32 arg0, s32 arg1);

/**
 * @see decomp.me (100%)
 */
void func_8002E204(s32* arg0)
{
    func_8002DDDC(arg0[0], arg0[1], arg0[2], arg0[3]);
    g_akao_sfx_control.unk0 &= ~D_8004F76C[0];
}

/**
 * @see decomp.me (100%)
 */
void func_8002E250(s32* arg0)
{
    func_8002DFA4(arg0[0], arg0[1]);
    g_akao_sfx_control.unk0 &= ~D_8004F76C[0];
}

/**
 * @see decomp.me (100%)
 */
s32 func_8002E294(s32 *p)
{
    g_akao_xa_tracker.unk28++;
    (*p)++;
    if ((u32)(g_akao_xa_tracker.unk3C - 1) < (u32)*p)
    {
        *p = 0;
    }
    return *p;
}
