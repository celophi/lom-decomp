/* Partial WMAP decompilation: 95.224000% (gcc280_g0). */
#include "common.h"
#include "saved_game.h"

/** @brief Saved map entry status at its original byte offsets. */
typedef struct
{
    u8 pad_000[0x2F0];
    union
    {
        struct
        {
            unsigned int flag0 : 1;
            unsigned int hidden : 1;
            unsigned int other_flags : 6;
        } flags;
        struct
        {
            u8 flags;
            u8 pad[2];
            u8 priority;
        } data;
    } entry;
} WmapSavedEntry;
extern s32 D_800D82F8;
extern s32 D_800D82FC;
extern s32 D_800D8300;
extern s32 D_800D8304;
extern s32 D_800D8308;
extern s32 D_800D9000[];
extern s32 func_8005D948(s32, s32, s32);

/** @brief Sort visible map entries by priority and initialize the selection range.
 * @param selection Requested selection position.
 * @return Number of visible map entries.
 */
s32 func_8005CC50(s32 selection)
{
    s32 i;
    s32 j;
    s32 current;
    s32 previous;
    s32 previous_index;
    s32 *current_slot;
    s32 *previous_slot;
    u32 end;
    WmapSavedEntry *entry;

    D_800D82F8 = 0;
    for (i = 63; i >= 0; i--)
    {
        D_800D9000[i] = -1;
    }
    for (i = 0; i < 64; i++)
    {
        entry = (WmapSavedEntry *)((u8 *)&g_saved_game + i * 12);
        if (entry->entry.data.priority != 0 && entry->entry.flags.hidden != 1)
        {
            j = D_800D82F8;
            D_800D82F8 = j + 1;
            D_800D9000[j] = i;
            while (j > 0)
            {
                current_slot = &D_800D9000[j];
                previous_index = j - 1;
                previous_slot = &D_800D9000[previous_index];
                current = *current_slot;
                previous = *previous_slot;
                if (((WmapSavedEntry *)((u8 *)&g_saved_game + previous * 12))->entry.data.priority <
                    ((WmapSavedEntry *)((u8 *)&g_saved_game + current * 12))->entry.data.priority)
                {
                    *current_slot = previous;
                    *previous_slot = current;
                }
                j = previous_index;
            }
        }
    }
    i = D_800D82F8;
    end = i + 12;
    for (; (u32)i < end; i++)
    {
        D_800D9000[i] = -1;
    }
    D_800D82F8 += 11;
    D_800D8300 = func_8005D948(selection + 3, 0, 11);
    D_800D82FC = func_8005D948(D_800D82F8 - 11, 0, D_800D82F8);
    D_800D8308 = func_8005D948(D_800D8300 + 11, 0, 11);
    D_800D8304 = func_8005D948(D_800D82FC + 11, 0, D_800D82F8);
    return D_800D82F8 - 11;
}
