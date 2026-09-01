#include "common.h"
extern u8 g_menuLayoutBuffer[];
void func_800C3CB4(void)
{
    s32 i;
    s32 offset;
    s8 value;
    s32 sentinel;
    u8 *base;
    u8 *p;
    u8 a;
    u8 b;

    value = 0x63;
    i = 0x23;
    base = g_menuLayoutBuffer;
    p = base;
    p += 0x8C;
    do {
        p[0x2A7E] = value;
        i--;
        p -= 4;
    } while (i >= 0);

    i = 0;
    sentinel = 0x63;
    offset = 0x18;
    base = g_menuLayoutBuffer;
    do {
        p = base + i * 4;
        a = p[0x2A7F];
        if (a != sentinel) {
            b = ((u8 *)((u32)offset + (u32)base))[0x2A7F];
            value = i + 6;
            if (b != sentinel) {
                if (a != b) {
                    p[0x2A7E] = value;
                }
            }
        }
        offset += 4;
        i++;
    } while (i < 0x1E);
}
