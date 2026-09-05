#include "common.h"

typedef struct
{
    u8 _pad00[0x5A];
    u16 unk5A;
    u8 _pad5C[4];
} FieldSmallHistoryRecord;

typedef struct
{
    u8 _pad0000[0x2EF4];
    FieldSmallHistoryRecord small_history_records[5];
} FieldMenuHistoryData;

extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values[];
extern u8 g_menuLayoutBuffer[];
extern u32 D_80122C00;

/**
 * @brief Load a value from the gosub-selected compact history record.
 *
 * If the gosub returned a selection, copies the selected record's unk5A
 * halfword into D_80122C00.
 */
void func_800C7628(void)
{
    FieldMenuHistoryData* history_data;
    s32 history_index;

    if (g_gosub_result_count != 0)
    {
        history_index = g_gosub_result_values[0];
        history_data = (FieldMenuHistoryData*)g_menuLayoutBuffer;
        D_80122C00 = history_data->small_history_records[history_index].unk5A;
    }
}
