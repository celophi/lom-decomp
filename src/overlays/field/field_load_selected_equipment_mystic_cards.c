#include "common.h"

typedef struct
{
    s16 mystic_card_ids[3];
} FieldMysticCardSlots;

extern FieldMysticCardSlots D_80122C00;
extern s32 g_gosub_result_values[];
extern u8 g_menuLayoutBuffer[];

/**
 * @brief Load the gosub-selected equipment's visible Mystic Card IDs.
 */
void func_800C6F60(void)
{
    s32 *selection_results;
    u8 *layout_buffer;
    u8 *equipment_record;

    selection_results = g_gosub_result_values;
    layout_buffer = g_menuLayoutBuffer;
    equipment_record = (u8 *)((selection_results[0] * 64) + (s32)layout_buffer);
    D_80122C00.mystic_card_ids[0] = equipment_record[0xD00];
    D_80122C00.mystic_card_ids[1] = equipment_record[0xD01];
    D_80122C00.mystic_card_ids[2] = equipment_record[0xD02];
}
