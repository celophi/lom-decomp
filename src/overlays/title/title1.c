#include "title.h"


/**
 * decomp.me (100%) https://decomp.me/scratch/mEAXF
 */
s32 title_func_8004FC74(s32 arg0)
{
    s32 pad;
    S_801ED480* ptr = (S_801ED480*)0x801ED480;
    s32* base;
    u8* d8_base;
    u32 const_ff;
    s32 temp1, temp2;
    u8 d92;
    pad = arg0;

    func_80050244();
    func_80050300(0);
    func_80050394();
    base = (s32*)0x80100000; /* base address for D_80102640 */
    const_ff = 0xFF;
    d8_base = D_80042FD8;
    while (1)
    {
        func_800500CC(pad);
        ptr->unk0 = 0;
        ptr->unk2 = 0;
        ptr->unk4 = 0;
        ptr->unk8 = 0;
        ptr->unkC = 0;
        do
        {
            func_8004FDBC(pad);
        } while (base[0x990] == 0); /* 0x80102640 offset = 0x2640, 0x2640/4 = 0x990 */

        D_80042FB4 = VSync(-1);
        d92 = D_80102692;

        if (d92 == 0)
        {
            func_80052220(0);
            base[0x990] = 0; /* D_80102640 = 0 */
            if (func_8004FF48(pad) == 2)
            {
                GFX_Transition(0);
                continue;
            }
            return 3;
        }
        else if (d92 == 1)
        {
            return 7;
        }
        else if (d92 == const_ff)
        {
            func_80050374();
            return 8;
        }
        else
        {
            func_800227D0(0, 0x3C, 0);
            func_80052220(-1);
            D_8003EC9C = const_ff;
            temp1 = rand();
            temp2 = rand();
            *(s16*)(d8_base + 0xD4) = (s16)(temp1 | (temp2 << 0xF));
            return 0;
        }
    }
}