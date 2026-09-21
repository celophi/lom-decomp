#include "common.h"

extern u8 D_80139988[];
extern void *D_80121538;
extern s16 D_801AFBD0;
extern u8 *D_80139280;
extern s32 D_801B3040;
extern s32 D_801B3044;
extern void func_800B5BBC(s32 arg);

/**
 * @brief World-map step handler: seed a 100-entry table, init an actor, advance.
 * @note Best match ~81.31% (gcc280_g0); residual is the base+offset pointer
 *       fold and the tail counter CSE (permuter territory).
 */
void func_800B44EC(void)
{
    s16 *p;
    u8 *q;
    u8 *s;
    void *base;
    s32 i;

    i = 0x14;
    base = &D_80121538;
    q = (u8 *)&D_80139988 + 0xA0;
    p = (s16 *)((u8 *)&D_801AFBD0 + 0x190);
    do
    {
        *p = 0;
        *(void **)(q + 4) = base;
        q += 8;
        i += 1;
        p += 0xA;
    } while (i < 0x78);
    D_801B3044 = 0xB4;
    s = D_80139280;
    *(s32 *)(s + 0x7C) = 5;
    *(s32 *)(s + 0x8C) = 5;
    *(s32 *)(s + 0x78) = 1;
    *(s32 *)(s + 0x80) = -0x3E8;
    *(s32 *)(s + 0x84) = 0x1770;
    *(s32 *)(s + 0x88) = 0x1388;
    *(s32 *)(s + 0x90) = -0x64;
    *(s32 *)(s + 0x98) = -0x12C;
    *(s32 *)(s + 0x94) = 0;
    *(s32 *)(s + 0xA0) = -1;
    *(s32 *)(s + 0xA4) = 0;
    D_801B3040 += 1;
    func_800B5BBC(D_801B3040);
}
