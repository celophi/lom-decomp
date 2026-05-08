#include "menu.h"

void func_80140908(void)
{
    volatile u8 padding;
    func_801410B0();
    func_801410E8();
    func_80141324();
    D_801690F4 = -1;
    func_800AA02C();
    D_801690E8 = 0;
    func_80140968();
    D_801690AC = 0;
    D_80169120 = 0;
    func_801423D8();
}

/**
 * decomp.me (99.64%) https://decomp.me/scratch/AGd9K
 */
void func_80140968(void)
{
    s32 s0 = 0;

    /* Force base address into s3 early */
    u8* base = D_800FE778;

    s32 s2 = 0x20;
    s32 s1 = 0;
    s16 params[4];

    do
    {
        /* First call */
        params[0] = 0x110;
        params[1] = s0 + 0x1D8;
        params[2] = 0x10;
        params[3] = 1;
        func_80019A34(params, base + ((s1 >> 2) * 4));

        /* Second call */
        params[0] = (s0 == 2) ? 0x3E8 : 0x3F4;
        params[1] = (s0 == 0) ? 0x120 : 0x150;
        params[2] = 0xC;
        params[3] = 0x30;
        func_80019A34(params, base + ((s2 >> 2) * 4));

        s2 += 0x4A0;
        s0++;
        s1 += 0x4A0;
    } while (s0 < 3);
}