/* Partial WMAP decompilation: 93.676765% (gcc280_g0). */
#include "common.h"
#include "saved_game.h"

extern u8 D_800CFDCC[64][12];
extern s32 D_800D8318[64][8];
extern s16 g_music_track_index;
extern void func_8005BD54(s32, s32, s32, s32 (*)[8]);
extern void func_8005C998(void);

/** @brief Place a map record and update its neighboring layout data.
 * @param x Grid column.
 * @param y Grid row.
 * @param index Save record index.
 */
void func_8005BBC8(s32 x, s32 y, s32 index)
{
    s32 record;
    s32 field;
    SavedGame *entry;

    entry = (SavedGame *)(g_saved_game.bytes + index * 12);
    entry->bytes[0x2F1] = (x & 15) | (y * 16);
    entry->bytes[0x2F0] |= 2;
    g_saved_game.bytes[0x2E5]++;
    entry->bytes[0x2F2] = g_saved_game.bytes[0x2E5];
    if (g_saved_game.bytes[0x2E5] == 1)
    {
        g_music_track_index = index;
    }
    for (record = 0; record < 64; record++)
    {
        for (field = 0; field < 8; field++)
        {
            D_800D8318[record][field] = ((SavedGame *)(g_saved_game.bytes + record * 12 + field))->bytes[0x2F4];
        }
    }
    for (field = 0; field < 8; field++)
    {
        D_800D8318[index][field] = D_800CFDCC[index][field];
    }
    func_8005BD54(index, x, y, D_800D8318);
    for (record = 0; record < 64; record++)
    {
        for (field = 0; field < 8; field++)
        {
            ((SavedGame *)(g_saved_game.bytes + record * 12 + field))->bytes[0x2F4] = D_800D8318[record][field];
        }
    }
    func_8005C998();
}
