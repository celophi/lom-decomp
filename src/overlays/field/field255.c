#include "common.h"

typedef struct
{
    u8 pad0[0x24];
    u8 unk24;              // 0x24
    u8 pad25[0x27 - 0x25];
    u8 unk27;               // 0x27
    u8 pad28[0x2A - 0x28];
    s16 unk2A;              // 0x2A
    u8 pad2C[0x2E - 0x2C];
    s16 unk2E;               // 0x2E
    u8 pad30[0x3A - 0x30];
    u8 unk3A;                // 0x3A
    u8 pad3B[0x54 - 0x3B];
} Struct_D800FDF58;

typedef struct
{
    u8 pad0[0x174];
    s32 unk174;              // 0x174
    u8 pad178[0x23C - 0x178];
} Struct_D80105AE0;

extern Struct_D80105AE0 D_80105AE0[];

void func_8006C3FC(Struct_D800FDF58 *rec);

void func_80096334(Struct_D800FDF58 *a0)
{
    a0->unk2E = 1;
    a0->unk27 = 0;
    a0->unk24 = 1;

    D_80105AE0[a0->unk3A].unk174 &= ~0x1800;

    func_8006C3FC(a0);
}
