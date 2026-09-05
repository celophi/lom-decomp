#include "common.h"

void func_800B2844(s32 arg0, void *arg1, s32 arg2);

extern u8 D_800F0E98[];
extern s16 D_80122C14;
extern u8 D_80122C1F;
extern u8 g_menuLayoutBuffer[];

/**
 * @brief Clears the active menu entry's high flag nibble and status bytes.
 *
 * For the entry selected by D_80122C1F (stride 0x8C in g_menuLayoutBuffer),
 * masks off bits 12-15 of its 0x26E4 word and writes 0xFF to the four status
 * bytes at 0x26EC.
 */
void func_800C7C88(void)
{
    u8 *base = g_menuLayoutBuffer;
    u8 *base2;
    s32 offset = D_80122C1F * 0x8C;
    s32 offset2;
    s32 i;

    *(u32 *)(base + offset + 0x26E4) &= 0xFFFF0FFF;
    i = 0;
    base2 = base;
    offset2 = offset;
    for (; i < 4; i++)
    {
        base2[i + offset2 + 0x26EC] = 0xFF;
    }
}

/**
 * @brief Resets the eight menu action slots and clears a nibble of the flags.
 *
 * Reads the flags word at @c g_menuLayoutBuffer + 0x26E4, zeroes the eight
 * slots at stride 0x10 from +0x26F0 (each slot's word set to 0 and its status
 * byte at +4 set to 0xFF), then clears bits 12-15 of the flags word.
 */
void func_800C7CF8(void)
{
    u8 *base = g_menuLayoutBuffer;
    s32 flags = *(s32 *)(base + 0x26E4);

    *(s32 *)(base + 0x26F0) = 0;
    *(base + 0x26F4) = 0xFF;
    *(s32 *)(base + 0x2700) = 0;
    *(base + 0x2704) = 0xFF;
    *(s32 *)(base + 0x2710) = 0;
    *(base + 0x2714) = 0xFF;
    *(s32 *)(base + 0x2720) = 0;
    *(base + 0x2724) = 0xFF;
    *(s32 *)(base + 0x2730) = 0;
    *(base + 0x2734) = 0xFF;
    *(s32 *)(base + 0x2740) = 0;
    *(base + 0x2744) = 0xFF;
    *(s32 *)(base + 0x2750) = 0;
    *(base + 0x2754) = 0xFF;
    *(s32 *)(base + 0x2760) = 0;
    *(base + 0x2764) = 0xFF;

    *(s32 *)(base + 0x26E4) = flags & 0xFFFF0FFF;
}

/**
 * @brief Decodes a little-endian offset from the field table and dispatches
 *        the referenced entry.
 */
void func_800C7D5C(void)
{
    s32 offset = (D_80122C14 + 0x58) * 2;

    func_800B2844(4, D_800F0E98[offset] +
                         (D_800F0E98[offset + 1] << 8) + D_800F0E98, 0xFF);
}
