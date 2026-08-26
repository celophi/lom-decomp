#include "common.h"

typedef struct
{
    s32 unk0;
    u8 pad4[2];
    s16 unk6;
    u8 pad8[0x16];
    s16 unk1E;
} UnkStruct80122C00;

typedef struct
{
    u8 pad[0x29D8];
    u8 unk29D8;
} Rec29D8;

typedef struct
{
    u8 pad[0x2B50];
    u8 unk2B50;
} Rec2B50;

extern UnkStruct80122C00 D_80122C00;
extern u8 g_menuLayoutBuffer[];

void func_800C5B10(void)
{
    s32 temp_v1;
    Rec2B50 *p;
    u8 *menu;

    menu = g_menuLayoutBuffer;
    temp_v1 = ((Rec29D8 *)(D_80122C00.unk0 + menu))->unk29D8;
    p = (Rec2B50 *)((temp_v1 * 0x14C) + menu);
    D_80122C00.unk6 = temp_v1;
    D_80122C00.unk1E = p->unk2B50 & 0xF;
}
