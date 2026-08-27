#include "common.h"

extern u8 g_menuLayoutBuffer[];
extern s16 D_80122C10;

/**
 * @brief Selects the menu state associated with the current history slot.
 *
 * Clamps the signed history-slot byte at offset 0x29D7 to 3, then selects
 * state 2 when the low seven bits of the word at 0xAA8 equal 3. Otherwise it
 * selects state 0 for history slot 3 and state 1 for the remaining slots.
 */
void func_800C5B64(void)
{
    if (g_menuLayoutBuffer[0x29D7] >= 4U)
    {
        g_menuLayoutBuffer[0x29D7] = 3;
    }
    if ((*(u32 *)&g_menuLayoutBuffer[0xAA8] & 0x7F) == 3)
    {
        D_80122C10 = 2;
    }
    else if (*(s8 *)&g_menuLayoutBuffer[0x29D7] == 3)
    {
        D_80122C10 = 0;
    }
    else
    {
        D_80122C10 = 1;
    }
}
