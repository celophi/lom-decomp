#include "common.h"
/** @brief Field header and actor event fields used by the reset routine. */
typedef struct
{
    u8 pad[0x400];
    union
    {
        u32 word;
        u16 count;
        struct
        {
            u8 low[3];
            u8 high;
        } bytes;
    } header;
    u8 pad404[0x2C];
    u8 id;
    u8 pad431[5];
    u16 enabled;
    u16 event;
} State;
extern State *D_80122B78;
extern u8 *D_80122B74;
extern s32 D_80123FB0, D_8010D020, g_layout_flag;
extern void func_80087FC0(s32, s32);
extern void func_800C1D14(s32, s32);
extern s32 func_800B28E0(s32, s32, s32);
extern void akao_cmd_c1(s32, s32, s32);
/**
 * @brief Reset the first three actor event lists and dispatch initialization events.
 * @note Sends audio command C1 for layouts 3, 34, 35, 37, 43, 45, 46, and 47.
 * @note Best current match: 93.089290% with GCC 2.8.
 */
void func_800B4684(void)
{
    s32 i, offset, source_offset, inner;
    u32 j;
    State *state;
    D_80123FB0 = 0;
    D_80122B78->header.word &= 0xFFFEFFFF;
    D_80122B78->header.bytes.high = 0;
    i = 0;
    offset = 0;
    source_offset = 0;
    do
    {
        if (*((u8 *)((s32)D_80122B74 + source_offset) + 0x608) >> 7)
        {
            func_80087FC0(i, 0);
        }
        else
        {
            j = 0;
            state = D_80122B78;
            inner = offset;
            ((State *)((u8 *)state + offset))->enabled = 0;
            do
            {
                ((State *)((u8 *)state + inner))->event = 0xFFFF;
                j++;
                inner += 2;
            } while (j < 16);
            func_80087FC0(i, 1);
            func_800C1D14(i, 0);
        }
        offset += 0x94;
        i++;
        source_offset += 0x250;
    } while (i < 3);
    i = 0;
    if (D_80122B78->header.count != 0)
    {
        offset = 0;
        do
        {
            func_800B28E0(((State *)((s32)D_80122B78 + offset))->id, 13, 1);
            i++;
            offset += 0x94;
        } while (i < D_80122B78->header.count);
    }
    if (D_8010D020 != 0)
    {
        D_8010D020 = 0;
    }
    else
    {
        switch (g_layout_flag)
        {
        case 3:
        case 34:
        case 35:
        case 37:
        case 43:
        case 45:
        case 46:
        case 47:
            akao_cmd_c1(0, 0x40, 0);
            break;
        }
    }
}
