#include "common.h"

/** @brief Context level, preserved flag byte, and packed value updated by the record load. */
typedef struct
{
    u8 pad0[0x2E5];
    u8 level;
    u8 pad2E6[0x858 - 0x2E6];
    union
    {
        u32 word;
        u8 byte[4];
    } flags;
    u8 pad85C[4];
    union
    {
        u32 word;
        u8 byte[4];
    } packed;
} Context;
extern Context *D_80122B74;
extern s32 D_800F190C[];
extern u8 *func_800C1E40(s32);
extern void func_800C1EC8(void *, void *, s32);
extern void func_800C11F0(s32, s32);
extern void func_800B7C58(s32);
extern void func_800BD520(s32, s32, s32);
extern void akao_set_song_params(s32, s32, s32, s32);
/**
 * @brief Load a matching level-dependent record or issue the fallback audio command.
 * @param record_id Record identifier to find.
 * @return -1 after applying a matching record, or zero after the fallback.
 */
s32 func_800C2B14(s32 record_id)
{
    s32 var_a3;
    s32 bonus_offset;
    u8 *bonus_record;
    s32 *bonus_table;
    u8 **context_ref;
    s32 record_offset;
    s32 level_index;
    s32 record_index;
    u32 packed_value;
    u32 saved_flag;
    u8 level;
    u8 *table;
    u8 *temp_v0_2;
    u8 *cursor;
    u8 *bank;

    if (record_id < 0xC)
    {
        table = func_800C1E40(3);
        record_index = 0;
        if ((*(u16 *)(table + 2)) != 0)
        {
            bonus_table = D_800F190C;
            record_offset = 4;
            cursor = table;
        loop_3:
            record_index += 1;
            if ((*(s32 *)(cursor + 4)) == record_id)
            {
                level = D_80122B74->level;
                saved_flag = D_80122B74->flags.byte[0] >> 7;
                if (level < 6U)
                {
                    bank = table + record_offset + 4;
                }
                else if (level < 0xCU)
                {
                    bank = table + record_offset + 0x254;
                }
                else
                {
                    if (level < 0x12U)
                    {
                        bank = table + record_offset + 0x4A4;
                    }
                    else
                    {
                        bank = table + record_offset + 0x6F4;
                    }
                }
                func_800C1EC8(bank, (u8 *)D_80122B74 + 0x840, 0x250);
                D_80122B74->flags.word =
                    (s32)(((s32)D_80122B74->flags.word & ~0x80) | (saved_flag << 7));
                if ((u8)D_80122B74->level < 0x20U)
                {
                    level_index = D_80122B74->level - 1;
                }
                else
                {
                    level_index = 0x1F;
                }
                bonus_offset = (record_id + 0x68) * 4;
                bonus_record = (u8 *)D_80122B74 + bonus_offset;
                packed_value = D_80122B74->packed.byte[0] |
                               (((*(s32 *)(bonus_record + 0xE4)) + bonus_table[level_index]) << 8);
                D_80122B74->packed.word = packed_value;
                if ((s32)(packed_value >> 8) > 0x98967F)
                {
                    D_80122B74->packed.word = (s32)((packed_value & 0xFF) | 0x98967F00);
                }
                func_800C11F0(1, 0);
                func_800B7C58(1);
                func_800BD520(0, (record_id * 8) + 0xF87, 1);
                return -1;
            }
            record_offset += 0x944;
            cursor += 0x944;
            if (record_index >= (s32)(*(u16 *)(table + 2)))
            {
                goto block_18;
            }
            goto loop_3;
        }
    block_18:
        akao_set_song_params(0x8001, 0x6D, record_id, 0);
    }
    else
    {
        akao_set_song_params(0x8001, 0x6D, record_id, 1);
    }
    return 0;
}
