#include "common.h"
typedef struct { u8 unk0; u8 pad[0x11]; u8 unk12; } StructC0D;
extern StructC0D D_80122C0D;
extern u8 g_menuLayoutBuffer[];
void func_800C7F44(void)
{
    s32 va;
    s32 vb;
    s32 value;
    s32 out;
    u8 idx;

    va = D_80122C0D.unk0;
    va -= 4;
    vb = D_80122C0D.unk12;
    {
        u8 *buf = g_menuLayoutBuffer;
        u8 *base = buf + (va * 0x10 + vb * 0x8C);
        idx = base[0x26F4];
    }
    value = g_menuLayoutBuffer[idx + 0x25E0];
    value += 1;
    if (value >= 0) {
        out = 0x63;
        if (value < 0x64)
            out = value;
    } else {
        out = 0;
    }
    {
        u8 *buf = g_menuLayoutBuffer;
        u8 *base;
        buf[idx + 0x25E0] = out;
        base = buf + (va * 0x10 + vb * 0x8C);
        *(s32 *)(base + 0x26F0) = 0;
        base[0x26F4] = 0xFF;
        base[0x26F8] = 0;
        base[0x26F9] = 0;
        base[0x26FA] = 0;
        base[0x26FB] = 0;
        base[0x26FC] = 0;
        base[0x26FD] = 0;
        base[0x26FE] = 0;
        base[0x26FF] = 0;
    }
}
