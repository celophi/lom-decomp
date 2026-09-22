/* Partial WMAP decompilation: 88.707310% (gcc280_g0). */
#include "common.h"
#include "saved_game.h"

extern s8 D_800D8B18[];

/** @brief Rebuild the world-map lookup from placed save records. */
void func_8005C998(void)
{
    s32 index;
    SavedGame *record;
    s32 row;
    u32 column;
    s32 doubled_column;
    u8 position;

    index = 0;
    record = &g_saved_game;
    do
    {
        if ((((u32)record->bytes[0x2F0] >> 1) & 1) == 1)
        {
            if (index == 0x18)
            {
                doubled_column = column * 2;
                if (!(g_saved_game.words[0x47C / 4] & 4))
                {
                    position = ((SavedGame *)(g_saved_game.bytes + 0x120))->bytes[0x2F1];
                    row = position & 0xF;
                    goto decode_column;
                }
            }
            else
            {
                position = record->bytes[0x2F1];
                row = position & 0xF;
decode_column:
                column = position >> 4;
                doubled_column = column * 2;
            }
            D_800D8B18[row + ((doubled_column + column) * 2)] = index;
        }
        index++;
        record = (SavedGame *)((u8 *)record + 12);
    } while (index < 0x40);
}
