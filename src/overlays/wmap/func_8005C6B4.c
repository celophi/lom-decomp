/* Partial WMAP decompilation: 94.203540% (gcc280_g0). */
#include "common.h"
#include "cdrom.h"
#include "saved_game.h"

extern void func_8005C998(void);
extern void func_8005CC50(s32);
extern s32 D_800430B8;
extern u8 D_800D8B18[];
extern u8 *D_800D8B3C;
extern u8 D_800D8B40[];
extern u8 *D_800D8FF8;
extern u8 *D_800D8FFC;
extern s32 D_800D9130;

/** @brief Load the map layout and reconcile saved placement data.
 * @return Current layout status.
 */
s32 func_8005C6B4(void)

{
    struct SaveMapHeader
    {
        u8 pad_00[0x28];
        unsigned int unused_flags : 2;
        unsigned int option : 1;
        unsigned int other_flags : 29;
        u8 pad_2C[0xE0 - 0x2C];
        u16 layout_id;
    } *header;
    s32 index;
    u8 *cell;
    u8 saved_second;
    u8 saved_first;

    D_800D9130 = -1;
    cdrom_wait_queue_empty();
    header = (struct SaveMapHeader *)&g_saved_game;
    cdrom_queue_read((header->layout_id + 0x12EA) & 0xFFFF, D_800D8B40);
    cdrom_wait_queue_empty();
    D_800D8B3C = D_800D8B40;
    D_800D8FF8 = D_800D8B40 + 0x1B8;
    D_800D8FFC = D_800D8B40 + 0x3B8;
    if ((header->option != 1) && (g_saved_game.bytes[0x2E5] == 1))
    {
        g_saved_game.words[0x2E8 / 4] = (s32) (g_saved_game.words[0x2E8 / 4] | 2);
    }
    if ((g_saved_game.words[0x410 / 4] & 4) && !(g_saved_game.words[0x47C / 4] & 4))
    {
        g_saved_game.words[0x47C / 4] = (s32) (((((g_saved_game.words[0x47C / 4] | 7) & ~0xF00) | (g_saved_game.words[0x410 / 4] & 0xF00)) & 0xFFFF0FFF) | (g_saved_game.words[0x410 / 4] & 0xF000));
        g_saved_game.bytes[0x480] = 3;
        g_saved_game.bytes[0x481] = 3;
        g_saved_game.bytes[0x482] = 3;
        g_saved_game.bytes[0x483] = 3;
        g_saved_game.bytes[0x484] = 3;
        g_saved_game.bytes[0x485] = 3;
        g_saved_game.bytes[0x486] = 3;
        g_saved_game.bytes[0x487] = 3;
        saved_first = g_saved_game.bytes[0x412];
        saved_second = g_saved_game.bytes[0x413];
        g_saved_game.words[0x410 / 4] &= ~2;
        g_saved_game.words[0x410 / 4] &= ~1;
        g_saved_game.words[0x410 / 4] |= 0xFF00;
        g_saved_game.bytes[0x412] = 0U;
        g_saved_game.bytes[0x413] = 0U;
        g_saved_game.bytes[0x414] = 0;
        g_saved_game.bytes[0x415] = 0;
        g_saved_game.bytes[0x416] = 0;
        g_saved_game.bytes[0x417] = 0;
        g_saved_game.bytes[0x418] = 0;
        g_saved_game.bytes[0x419] = 0;
        g_saved_game.bytes[0x41A] = 0;
        g_saved_game.bytes[0x41B] = 0;
        g_saved_game.bytes[0x47E] = saved_first;
        g_saved_game.bytes[0x47F] = saved_second;
    }
    index = 0x23;
    cell = D_800D8B18 + index;
    do
    {
        *cell = 0xFF;
        index -= 1;
        cell -= 1;
    } while (index >= 0);
    func_8005C998();
    func_8005CC50(0);
    return D_800430B8;
}
