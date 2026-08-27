#include "common.h"

extern u8 g_menuLayoutBuffer[];

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
