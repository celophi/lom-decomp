#include "common.h"

typedef struct FileHeader100 {
    s32 unk0;
    s16 unk4;
    u8 pad[0xFA];
} FileHeader100;

extern s32 D_80165FEC;
extern s32 D_801660A0;
extern s32 D_80165FF4;
extern s32 D_801660FC;
extern s32 D_80166000;
extern s32 D_80166AD0;
extern u8 D_80166440[];
extern FileHeader100 D_801401C0;
extern char D_800ECFC4[];
extern char D_800ECFD0[];
extern char D_800ECF7C[];
extern u8 D_801663F8[];
extern u8 *D_801663A0;
extern u8 D_80165BA0[];

void func_80149DF4(void)
{
    FileHeader100 local;
    u8 *p;

    if (D_80165FEC == 0)
    {
        D_801660FC = 3;
        return;
    }
    {
        s32 term1;
        s32 term2;
        term1 = D_801660A0 * 0x320;
        term2 = (D_80165FF4 * 0x28) + (s32)D_80166440;
        if (func_8001714C(&D_800ECFC4[0], (void *)(term1 + term2), 8) == 0)
        {
            D_801660FC = 2;
            return;
        }
    }
    {
        s32 term1;
        s32 term2;
        term1 = D_801660A0 * 0x320;
        term2 = (D_80165FF4 * 0x28) + (s32)D_80166440;
        if (func_8001714C(&D_800ECFD0[0], (void *)(term1 + term2), 9) == 0)
        {
            D_801660FC = 4;
            return;
        }
    }
    memcpy(&local, &D_801401C0, 6);
    p = (u8 *)&local;
    {
        s32 term1;
        s32 term2;
        term1 = D_801660A0 * 0x320;
        term2 = (D_80165FF4 * 0x28) + (s32)D_80166440;
        func_80016F9C(p, (void *)(term1 + term2));
    }
    {
        s32 slot;
        s32 value;
        value = *((u8 *)&local + 2);
        slot = (u8)D_801660A0;
        D_801660FC = 0;
        value += slot;
        *((u8 *)&local + 2) = value;
        func_800170BC(&D_801663F8[0], p, slot);
    }
    D_801663A0 = &D_80165BA0[0];
    D_80166000 = 1;
    {
        s32 term1;
        s32 term2;
        term1 = D_801660A0 * 0x320;
        term2 = (D_80165FF4 * 0x28) + (s32)D_80166440;
        if (func_8001714C(&D_800ECF7C[0], (void *)(term1 + term2), 0xC) == 0)
            D_80166AD0 = 1;
        else
            D_80166AD0 = 0;
    }
}
