#include "common.h"

extern u8 g_menuLayoutBuffer[];

/**
 * @brief Reset the 0x40 per-slot layout records in g_menuLayoutBuffer.
 *
 * Clears the two header bytes at 0x2E4/0x2E5, then walks 0x40 records of 0xC
 * bytes each (base offset 0x2F0): sets the first field to 0xFF, zeroes the
 * rest, and clears the low three bits of the flag byte. A second pass sets
 * bit 2 of every record's flag byte, and finally clears bit 2 of the word at
 * 0x410.
 *
 * @note 85.17% match (gcc272_cdk). The target is compiled with loop strength
 *       reduction disabled: it keeps the base pointer and uses fixed 0x2F0+
 *       offsets, whereas the default -O2 build derives an induction variable
 *       at base+0x2F0. Reaches 99.76% under -fno-strength-reduce (not enabled
 *       here); the last 0.24% is a v0/v1 coalescing tie on the base-copy insn.
 */
void func_800CA1E0(void)
{
    s32 i;
    u8 *p;
    u8 *q;

    i = 0;
    p = g_menuLayoutBuffer;
    p[0x2E4] = 0;
    p[0x2E5] = 0;
    do
    {
        i += 1;
        p[0x2F1] = 0xFF;
        p[0x2F2] = 0;
        p[0x2F3] = 0;
        p[0x2F4] = 0;
        p[0x2F5] = 0;
        p[0x2F6] = 0;
        p[0x2F7] = 0;
        p[0x2F8] = 0;
        p[0x2F9] = 0;
        p[0x2FA] = 0;
        p[0x2FB] = 0;
        p[0x2F0] &= 0xF8;
        p += 0xC;
    } while (i < 0x40);
    i = 0;
    q = g_menuLayoutBuffer;
    do
    {
        i += 1;
        q[0x2F0] |= 4;
        q += 0xC;
    } while (i < 0x40);
    *(s32 *)(g_menuLayoutBuffer + 0x410) &= ~4;
}
