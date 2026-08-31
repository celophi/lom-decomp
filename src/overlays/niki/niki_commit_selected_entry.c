#include "common.h"

typedef struct FileHeader100 {
    s32 unk0;
    s16 unk4;
    u8 pad[0xFA];
} FileHeader100;

extern s32 D_80164B78;
extern s32 D_80164B70;
extern s32 D_80164B7C;
extern s32 D_80164B84;
extern s32 D_80164A78;
extern s32 D_80164EB4;
extern u8 D_80165018[];
extern FileHeader100 D_80140090;
extern char D_800ECFC4[];
extern char D_800ECF7C[];
extern u8 D_80164E70[];
extern u8 *D_80164E18;
extern u8 D_801606E0[];

/** @see decomp.me (100.00%) */
void func_80145F68(void)
{
    FileHeader100 local;
    u8 *p;

    if (D_80164B78 == 0)
    {
        D_80164B84 = 3;
        return;
    }
    {
        s32 term1;
        s32 term2;
        term1 = D_80164B70 * 0x320;
        term2 = (D_80164B7C * 0x28) + (s32)D_80165018;
        if (func_8001714C(&D_800ECFC4[0], (void *)(term1 + term2), 8) == 0)
        {
            D_80164B84 = 2;
            return;
        }
    }
    memcpy(&local, &D_80140090, 6);
    p = (u8 *)&local;
    {
        s32 term1;
        s32 term2;
        term1 = D_80164B70 * 0x320;
        term2 = (D_80164B7C * 0x28) + (s32)D_80165018;
        func_80016F9C(p, (void *)(term1 + term2));
    }
    {
        s32 slot;
        s32 value;
        value = *((u8 *)&local + 2);
        slot = (u8)D_80164B70;
        D_80164B84 = 0;
        value += slot;
        *((u8 *)&local + 2) = value;
        func_800170BC(&D_80164E70[0], p, slot);
    }
    D_80164E18 = &D_801606E0[0];
    {
        s32 term1;
        s32 term2;
        term1 = D_80164B70 * 0x320;
        term2 = (D_80164B7C * 0x28) + (s32)D_80165018;
        if (func_8001714C(&D_800ECF7C[0], (void *)(term1 + term2), 0xC) == 0)
            D_80164EB4 = 1;
        else
            D_80164EB4 = 0;
    }
    D_80164A78 = 1;
}
