#include "menu.h"

/**
 * decomp.me (100%) https://decomp.me/scratch/tG03R
 */
void menu_func_801410FC(ArgStruct* arg0)
{
    u8* base = D_80160260;
    u8* s0 = base + 0xC;
    s32 s3 = *(s32*)(s0 + 8);
    ArgStruct sp10;
    u16* p;
    int i;

    D_8016910C = *(s32*)(s0 + 0x14);

    /* First loop */
    sp10.unk0 = arg0->unk4;
    sp10.unk2 = arg0->unk6;
    sp10.unk4 = 0x100;
    sp10.unk6 = 1;

    p = (u16*)(base + 0x20);
    for (i = 0; i < 0x100; i++)
    {
        if (*p != 0)
            *p |= 0x8000;
        p++;
    }
    func_80019A34(&sp10, s0 + 0x14);

    /* Second call */
    sp10.unk0 = arg0->unk0;
    sp10.unk2 = arg0->unk2;
    {
        // Enforce specific instruction ordering: addiu a1, s3, 8 then addu a1, s0, a1
        u8* temp = s0 + (s3 + 8);
        sp10.unk4 = *(u16*)(temp + 8);
        sp10.unk6 = *(u16*)(temp + 10);
        func_80019A34(&sp10, temp + 0xC);
    }

    /* Third loop */
    sp10.unk0 = arg0->unk4;
    sp10.unk2 = arg0->unk6 + 1;
    sp10.unk4 = 0x100;
    sp10.unk6 = 1;

    p = (u16*)(base + 0x822C);
    for (i = 0; i < 0x100; i++)
    {
        if (*p != 0)
            *p |= 0x8000;
        p++;
    }
    func_80019A34(&sp10, base + 0x822C);
}