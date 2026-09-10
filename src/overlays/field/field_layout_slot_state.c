#include "common.h"

extern u8 g_menuLayoutBuffer[];

/**
 * @brief Activate a layout slot and assign its insertion sequence number.
 * @param arg0 Index of the 12-byte layout-slot record.
 */
void func_800CA1A0(s32 arg0)
{
    u8 *rec;

    rec = &g_menuLayoutBuffer[arg0 * 0xC];
    g_menuLayoutBuffer[0x2E4]++;
    rec[0x2F0] |= 1;
    rec[0x2F3] = g_menuLayoutBuffer[0x2E4];
}




/**
 * @brief Reset the 0x40 per-slot layout records in g_menuLayoutBuffer.
 *
 * Clears the two header bytes at 0x2E4/0x2E5, then walks 0x40 records of 0xC
 * bytes each (base offset 0x2F0): sets the first field to 0xFF, zeroes the
 * rest, and clears the low three bits of the flag byte. A second pass sets
 * bit 2 of every record's flag byte, and finally clears bit 2 of the word at
 * 0x410.
 *
 * @see decomp.me (100%) TODO
 */
void func_800CA1E0(void)
{
    s32 i;
    u8 *p;
    u8 *q;
    u8 *r;

    i = 0;
    g_menuLayoutBuffer[0x2E4] = 0;
    g_menuLayoutBuffer[0x2E5] = 0;
    p = g_menuLayoutBuffer;
reset_slots:
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
    if (i < 0x40)
    {
        goto reset_slots;
    }
    i = 0;
    q = g_menuLayoutBuffer;
enable_slots:
    i += 1;
    q[0x2F0] |= 4;
    q += 0xC;
    if (i < 0x40)
    {
        goto enable_slots;
    }
    r = g_menuLayoutBuffer;
    *((s32 *) (r + 0x410)) &= ~4;
}
