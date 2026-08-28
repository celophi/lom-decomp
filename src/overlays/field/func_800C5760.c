#include "common.h"

typedef struct
{
    s32 unk0;
    s16 unk4;
} UnkStruct80122C00;

extern UnkStruct80122C00 D_80122C00;
extern u8 g_menuLayoutBuffer[];

/**
 * @brief Selects a menu state from the active record and history index.
 *
 * Stores state 1 or 2 when the active record has index 3. Otherwise, stores
 * state 3 when the record index matches the signed history index, or state 4
 * when it does not.
 */
void func_800C5760(void)
{
    u8 *menu;
    u8 *record;

    menu = g_menuLayoutBuffer;
    record = menu + D_80122C00.unk0;
    if (record[0x29D8] == 3)
    {
        if (menu[0x29D7] >= 3U)
        {
            D_80122C00.unk4 = 1;
            return;
        }
        D_80122C00.unk4 = 2;
        return;
    }
    if (record[0x29D8] == *(s8 *)&menu[0x29D7])
    {
        D_80122C00.unk4 = 3;
        return;
    }
    D_80122C00.unk4 = 4;
}
