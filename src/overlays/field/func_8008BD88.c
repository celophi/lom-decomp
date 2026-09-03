#include "common.h"

typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x23C - 0x18];
} Struct_D80105AE0;

typedef struct
{
    u8 pad0[0x10];
    s16 unk10;
    u8 pad12[0x28 - 0x12];
    u8 unk28;
    u8 pad29[0x2C - 0x29];
    u16 unk2C;
    u8 pad2E[0x3A - 0x2E];
    u8 unk3A;
    u8 pad3B[0x54 - 0x3B];
} Struct_D800FDF58;

extern Struct_D80105AE0 D_80105AE0[];
extern Struct_D800FDF58 D_800FDF58[];

void func_800952DC(Struct_D800FDF58 *record, s32 value);
void func_80083B38(Struct_D800FDF58 *record, s32 value);
void func_80084424(s32 index);

s32 func_8008BD88(s32 key)
{
    Struct_D800FDF58 *scan;
    Struct_D800FDF58 *found;
    Struct_D80105AE0 *e;
    s32 i;

    scan = D_800FDF58;
    e = D_80105AE0;
    i = 0;
loop:
    i++;
    if (e->unk14 == key)
        goto found_label;
    e++;
    scan++;
    if (i < 13)
        goto loop;
    found = (Struct_D800FDF58 *)-1;
check:
    if (found == (Struct_D800FDF58 *)-1)
        return -1;

    found->unk28 = 0xFF;
    found->unk10 = 0;
    found->unk2C++;
    func_800952DC(found, 0);
    func_80083B38(found, 1);
    func_80084424(found->unk3A);
    return 0;

found_label:
    found = scan;
    goto check;
}
