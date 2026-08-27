#include "common.h"

typedef struct
{
    u8 pad0[0xC];
    u32 unkC;    /* 0x0C */
    u8 pad10[0x170 - 0x10];
    u8 unk170;   /* 0x170 */
    u8 pad171[0x178 - 0x171];
    u32 unk178;  /* 0x178 */
    u8 pad17C[0x23C - 0x17C];
} Slot;

typedef struct
{
    u8 pad0[0x3A];
    u8 unk3A;    /* 0x3A */
    u8 pad3B[0x54 - 0x3B];
} Rec;

extern Slot D_80105AE0[];

void func_8008BCF8(Rec *rec)
{
    s32 i;

    for (i = 0; i < 13; i++)
    {
        if ((D_80105AE0[i].unk178 >> 1) & 1)
        {
            if (D_80105AE0[i].unk170 == rec->unk3A)
            {
                D_80105AE0[i].unk178 = D_80105AE0[i].unk178 & ~2;
                D_80105AE0[rec->unk3A].unkC &= ~0x2000;
            }
        }
    }
}
