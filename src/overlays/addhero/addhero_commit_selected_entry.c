#include "common.h"

typedef struct FileHeader100 {
    s32 unk0;
    s16 unk4;
    u8 pad[0xFA];
} FileHeader100;

extern s32 D_801609A4;
extern s32 D_801609A8;
extern s32 D_801609AC;
extern s32 D_801609B8;
extern s32 D_80164A40;
extern s32 D_80164A50;
extern u8 D_80164B60[];
extern FileHeader100 D_80140090;
extern char D_800ECFC4[];
extern char D_800ECF7C[];
extern u8 D_801654E0[];
extern u8 *D_80165488;
extern u8 D_8016058C[];

/** @see decomp.me (100.00%) */
void func_80145E14(void)
{
    FileHeader100 local;
    u8 *p;

    if (D_801609A4 == 0)
    {
        D_801609B8 = 3;
        return;
    }
    {
        s32 term1;
        s32 term2;
        term1 = D_801609A8 * 0x320;
        term2 = (D_801609AC * 0x28) + (s32)D_80164B60;
        if (func_8001714C(&D_800ECFC4[0], (void *)(term1 + term2), 8) == 0)
        {
            D_801609B8 = 2;
            return;
        }
    }
    memcpy(&local, &D_80140090, 6);
    p = (u8 *)&local;
    {
        s32 term1;
        s32 term2;
        term1 = D_801609A8 * 0x320;
        term2 = (D_801609AC * 0x28) + (s32)D_80164B60;
        func_80016F9C(p, (void *)(term1 + term2));
    }
    {
        s32 slot;
        s32 value;
        value = *((u8 *)&local + 2);
        slot = (u8)D_801609A8;
        D_801609B8 = 0;
        value += slot;
        *((u8 *)&local + 2) = value;
        func_800170BC(&D_801654E0[0], p, slot);
    }
    D_80165488 = &D_8016058C[0];
    {
        s32 term1;
        s32 term2;
        term1 = D_801609A8 * 0x320;
        term2 = (D_801609AC * 0x28) + (s32)D_80164B60;
        if (func_8001714C(&D_800ECF7C[0], (void *)(term1 + term2), 0xC) == 0)
            D_80164A50 = 1;
        else
            D_80164A50 = 0;
    }
    D_80164A40 = 1;
}
