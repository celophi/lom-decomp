#include "common.h"

typedef struct {
    u8 unk0;
    u8 pad1[0xE];
    u8 unkF;
    u8 unk10;
    u8 pad11;
    u8 unk12;
} StructC0D;
extern StructC0D D_80122C0D;
extern u8 g_menuLayoutBuffer[];
extern u8 D_800F0E98[];

void func_800C8014(void)
{
    u8 *base;
    u8 *menu;
    u8 *table;
    u8 *lo;
    u8 *hi;
    s32 va;
    s32 offset;
    s32 offset2;
    s32 value;
    u8 idx;

    va = D_80122C0D.unk0 - 4;
    menu = g_menuLayoutBuffer;
    table = D_800F0E98;
    base = menu + (va * 0x10 + D_80122C0D.unk12 * 0x8C);
    value = *(volatile s32 *)(base + 0x26F0);
    D_80122C0D.unk10 = value;
    idx = base[0x26F4];
    offset = idx * 2;
    lo = table + offset;
    offset2 = offset + 1;
    hi = table + offset2;
    func_800B2844(0, *lo + (*hi << 8) + table, 0xFF);
    D_80122C0D.unkF = idx;
}
