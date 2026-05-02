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

/**
 * decomp.me (100%) https://decomp.me/scratch/rgoAG
 */
void func_8004FDBC(void* arg0)
{
    RECT rect;
    u_char* base = (u_char*)arg0;
    u_char* s0;
    u_char* s1;
    volatile int* flag_base;

    DrawSync(0);
    VSync(0);

    rect.x = 0;
    rect.y = 0;
    rect.w = 0x140;
    rect.h = 0x1D8;
    ClearImage(&rect, 0, 0, 0);

    s0 = base;
    ClearOTagR((u_long*)(s0 + 0x40), 0x1000);
    ClearOTagR((u_long*)(s0 + 0xBD0C), 0x1000);
    PutDispEnv((DISPENV*)(s0 + 0x4040));
    func_800157DC();

    flag_base = (volatile int*)0x80100000;

    SetDispMask(1);

    for (;;)
    {
        s1 = s0 + 0x40;
        ClearOTagR((u_long*)s1, 0x1000);
        *(u_long**)(s0 + 0x80B8) = (u_long*)(s0 + 0x40B8);
        rand();
        VSync(1);
        func_8005041C(s0);
        func_80050734(s0);
        func_80050A50(s0);
        func_80050864();

        if (flag_base[0x990] == 0)
        {
            DrawSync(0);
            func_800157B0(2);
            VSync(2);

            {
                void* tmp = base;
                if (s0 == base)
                {
                    tmp = s0 + 0xBCCC;
                }
                s0 = tmp;
            }

            PutDispEnv((DISPENV*)(s0 + 0x4040));
            PutDrawEnv((DRAWENV*)(s0 + 0x4054));
            DrawOTag((u_long*)(s1 + 0x3FFC));
            func_800157DC();
            cdrom_process_state();

            if (flag_base[0x990] == 0)
                continue;
        }
        break;
    }

    func_800158E0();
    VSync(0);
    DrawSync(0);
}