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
extern u8 *D_80122B74;
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
                level = ((Context *)D_80122B74)->level;
                saved_flag = ((Context *)D_80122B74)->flags.byte[0] >> 7;
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
                ((Context *)D_80122B74)->flags.word =
                    (s32)(((s32)((Context *)D_80122B74)->flags.word & ~0x80) | (saved_flag << 7));
                if ((u8)((Context *)D_80122B74)->level < 0x20U)
                {
                    level_index = ((Context *)D_80122B74)->level - 1;
                }
                else
                {
                    level_index = 0x1F;
                }
                bonus_offset = (record_id + 0x68) * 4;
                bonus_record = (u8 *)D_80122B74 + bonus_offset;
                packed_value = ((Context *)D_80122B74)->packed.byte[0] |
                               (((*(s32 *)(bonus_record + 0xE4)) + bonus_table[level_index]) << 8);
                ((Context *)D_80122B74)->packed.word = packed_value;
                if ((s32)(packed_value >> 8) > 0x98967F)
                {
                    ((Context *)D_80122B74)->packed.word = (s32)((packed_value & 0xFF) | 0x98967F00);
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

#define ACTIVE_U8(p, o) (*(u8 *)((u8 *)(p) + (o)))
#define ACTIVE_U16(p, o) (*(u16 *)((u8 *)(p) + (o)))
#define ACTIVE_U32(p, o) (*(u32 *)((u8 *)(p) + (o)))
void func_800C3B50(void);
void func_800C3A00(s32);
void func_800C32C8(void);
void func_8006AB38(s32);


extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values[];
extern u8 *D_80122B74;
extern s32 D_801227F0;
extern void akao_set_song_params(s32 command, s32 arg1, s32 arg2, s32 arg3);
extern void func_800BD520(s32 arg0, s32 arg1, s32 arg2);
extern s32 func_800BD414(s32 arg0, s32 arg1);
extern void func_800C2E30(s32 arg0);

/**
 * @brief Dispatch a queued gosub result, or fall back to a default song cue.
 *
 * @note 100% match with the FIELD GCC 2.8.0 G0 toolchain.
 *
 * @return D_80122B74[0xAA9] on a successful dispatch, else 0xFF.
 */
s32 func_800C2D08(void)
{
    s32 index;

    D_801227F0 = 0;
    if (g_gosub_result_count != 0)
    {
        index = g_gosub_result_values[0];
        if (index < 5)
        {
            u8 *entry = D_80122B74 + index * 0x60;

            if (entry[0x2EF4] != 0)
            {
                *(s32 *)&D_80122B74[0x2EF0] = index;
                func_800BD520(0, 0x1F10, g_gosub_result_values[0]);
                func_800C2E30(g_gosub_result_values[0]);
                return D_80122B74[0xAA9];
            }
        }
        akao_set_song_params(0x8001, 0x6E, index, 0);
    }
    return 0xFF;
}

/**
 * @brief Selects a scene entry or restarts music based on a rolled index.
 *
 * Rolls func_800BD414; an index below 5 is stored to the buffer's 0x2EF0 slot,
 * handed to func_800C2E30, and the buffer's 0xAA9 byte is returned. Otherwise
 * akao_set_song_params is re-armed and 0xFF is returned.
 *
 * 100% match with the FIELD GCC 2.8.0 G0 toolchain.
 */
s32 func_800C2DC0(void)
{
    s32 v = func_800BD414(0, 0x1F10);
    s32 ret;

    if ((u32)v < 5)
    {
        *(s32 *)(D_80122B74 + 0x2EF0) = v;
        func_800C2E30(v);
        ret = *(u8 *)(D_80122B74 + 0xAA9);
    }
    else
    {
        akao_set_song_params(0x8001, 0x6E, v, 1);
        ret = 0xFF;
    }
    return ret;
}

/** @brief Restores a saved secondary record and its equipment tables.
 * @note Initial nonmatching C recovered from assembly.
 */
void func_800C2E30(s32 arg0)
{
    s32 temp_s1;
    u8 *temp_v0_4;
    u8 *temp_v0_5;
    s32 temp_v0_7;
    s32 temp_v1;
    s32 temp_v1_2;
    s32 var_a0;
    s32 var_a0_2;
    s32 var_a2;
    s32 var_s0;
    s32 var_s0_2;
    s32 var_s0_3;
    s32 var_s0_5;
    s32 var_s0_6;
    s32 var_s1;
    s8 var_s0_4;
    u16 temp_v0_3;
    u8 temp_v0;
    u8 *temp_a0;
    u8 *temp_a0_2;
    u8 *temp_a1;
    u8 *temp_a2;
    u8 *temp_v0_2;
    u8 *temp_v0_6;
    u8 *var_a1;
    u8 *var_v1;
    u8 *var_v1_2;

    var_s0 = 0;
    do
    {
        temp_a0 = D_80122B74 + var_s0;
        temp_v0 = ACTIVE_U8(D_80122B74, var_s0 + arg0 * 0x60 + 0x2EF4);
        var_s0 += 1;
        ACTIVE_U8(temp_a0, 0xA90) = temp_v0;
    } while (var_s0 < 0x15);
    var_a0 = arg0 * 0x60;
    ACTIVE_U32(D_80122B74, 0xAA8) = (s32) (((ACTIVE_U32(D_80122B74, 0xAA8) & ~0x7F) | 3) & ~0x80);
    ACTIVE_U8(D_80122B74, 0xAA9) = (u8) ACTIVE_U8(D_80122B74 + var_a0, 0x2F09);
    ACTIVE_U8(D_80122B74, 0xAB0) = ACTIVE_U8(D_80122B74 + var_a0, 0x2F0C);
    temp_a2 = D_80122B74 + var_a0;
    ACTIVE_U32(D_80122B74, 0xAB0) = (s32) (ACTIVE_U8(D_80122B74, 0xAB0) | (((u32) ACTIVE_U32(temp_a2, 0x2F0C) >> 8) << 8));
    var_s0_2 = 0;
    ACTIVE_U16(D_80122B74, 0xAB4) = (u16) ACTIVE_U16(temp_a2, 0x2F10);
    var_v1 = D_80122B74;
    ACTIVE_U16(D_80122B74, 0xAB6) = (u16) ACTIVE_U16(temp_a2, 0x2F12);
    do
    {
        temp_v0_2 = D_80122B74 + var_a0;
        var_a0 += 2;
        var_s0_2 += 1;
        ACTIVE_U16(var_v1, 0xAB8) = (u16) ACTIVE_U16(temp_v0_2, 0x2F14);
        var_v1 += 2;
    } while (var_s0_2 < 4);
    var_s0_3 = 0;
    var_a2 = arg0 * 0x60;
    var_a1 = D_80122B74;
    do
    {
        temp_a0_2 = D_80122B74 + var_a2;
        var_a2 += 2;
        var_s0_3 += 1;
        temp_v0_3 = (ACTIVE_U16(var_a1, 0xAC0) & 0xFE00) | (ACTIVE_U16(temp_a0_2, 0x2F1C) & 0x1FF);
        ACTIVE_U16(var_a1, 0xAC0) = temp_v0_3;
        ACTIVE_U16(var_a1, 0xAC0) = (u16) ((temp_v0_3 & 0x1FF) | (ACTIVE_U16(temp_a0_2, 0x2F1C) & 0xFE00));
        var_a1 += 2;
    } while (var_s0_3 < 8);
    temp_v1 = arg0 * 0x60;
    ACTIVE_U8(D_80122B74, 0xAD0) = (u8) ACTIVE_U8(D_80122B74 + temp_v1, 0x2F2C);
    ACTIVE_U8(D_80122B74, 0xAD1) = (u8) ACTIVE_U8(D_80122B74 + temp_v1, 0x2F2D);
    ACTIVE_U8(D_80122B74, 0xAD2) = (u8) ACTIVE_U8(D_80122B74 + temp_v1, 0x2F2E);
    var_s0_4 = 0;
    ACTIVE_U8(D_80122B74, 0xAD3) = (u8) ACTIVE_U8(D_80122B74 + temp_v1, 0x2F2F);
    do
    {
        ACTIVE_U8(D_80122B74 + var_s0_4, 0xAD8) = var_s0_4;
        var_s0_4 += 1;
    } while (var_s0_4 < 8);
    temp_v0_4 = func_800C1E40(0xD);
    if (temp_v0_4 != 0)
    {
        func_800C1EC8(temp_v0_4 + ((ACTIVE_U8(D_80122B74, arg0 * 0x60 + 0x2F30) << 6) + 4), D_80122B74 + 0xAE0, 0x40);
    }
    temp_s1 = arg0 * 0x60;
    ACTIVE_U16(D_80122B74, 0xB04) = (u16) ACTIVE_U16(D_80122B74 + temp_s1, 0x2F12);
    temp_v0_5 = func_800C1E40(0xE);
    var_s0_5 = 0;
    if (temp_v0_5 != 0)
    {
        var_s1 = 0xB20;
        do
        {
            temp_v1_2 = var_s0_5 + temp_s1;
            var_s0_5 += 1;
            temp_a1 = D_80122B74 + var_s1;
            var_s1 += 0x40;
            func_800C1EC8(temp_v0_5 + ((ACTIVE_U8(D_80122B74 + temp_v1_2, 0x2F31) << 6) + 4), temp_a1, 0x40);
        } while (var_s0_5 < 3);
    }
    var_s0_6 = 0;
    var_a0_2 = arg0 * 0x60;
    var_v1_2 = D_80122B74 + 0xB20;
    do
    {
        temp_v0_6 = D_80122B74 + var_a0_2;
        var_a0_2 += 2;
        var_s0_6 += 1;
        ACTIVE_U16(var_v1_2, 0x24) = (u16) ACTIVE_U16(temp_v0_6, 0x2F14);
        var_v1_2 += 2;
    } while (var_s0_6 < 4);
    temp_v0_7 = arg0 * 0x60;
    ACTIVE_U8(D_80122B74, 0xB4C) = (u8) ACTIVE_U8(D_80122B74 + temp_v0_7, 0x2F2C);
    ACTIVE_U8(D_80122B74, 0xB4D) = (u8) ACTIVE_U8(D_80122B74 + temp_v0_7, 0x2F2E);
    func_800B7C58(2);
}

/**
 * @brief Refresh the block and return its byte at 0xAA9 offset by 0x41.
 * @return The adjusted byte value.
 */
s32 func_800C318C(void)
{
    func_800C3B50();
    return D_80122B74[0xAA9] + 0x41;
}

/**
 * @brief Close either the primary (arg0 == 0) or secondary record and notify func_8006AB38.
 * @param arg0 Zero selects the record at 0x840, nonzero the record at 0xA90.
 */
void func_800C31BC(s32 arg0)
{
    if (arg0 == 0)
    {
        if ((*(s32 *)(D_80122B74 + 0x858) & 0x7F) == 2)
        {
            func_800BD520(0, (D_80122B74[0x859] << 3) + 0xF87, 0);
        }
        func_800BD520(0, 0x2F08, 0xFF);
        D_80122B74[0x840] = 0;
        *(s32 *)(D_80122B74 + 0x858) |= 0x7F;
    }
    else
    {
        if ((*(s32 *)(D_80122B74 + 0xAA8) & 0x7F) == 3)
        {
            func_800C32C8();
            *(s32 *)(D_80122B74 + 0x2EF0) = 5;
        }
        else
        {
            func_800C3A00(0);
        }
        D_80122B74[0xA90] = 0;
        *(s32 *)(D_80122B74 + 0xAA8) |= 0x7F;
        func_800BD520(0, 0x2F00, 0xFF);
    }
    func_8006AB38(arg0);
}

/**
 * @brief Copy the pending record at 0xA90 into the menu slot selected by the word at 0x2EF0.
 */
void func_800C32C8(void)
{
    s32 i;
    s32 dst_off;
    u8 *src;
    u8 *p;

    if ((u32)*(s32 *)(D_80122B74 + 0x2EF0) < 5)
    {
        i = 0;
        do
        {
            dst_off = i + *(s32 *)(D_80122B74 + 0x2EF0) * 0x60;
            src = D_80122B74 + i;
            i += 1;
            *(u8 *)(D_80122B74 + dst_off + 0x2EF4) = *(u8 *)(src + 0xA90);
        } while (i < 0x15);

        {
            u8 *b; s32 idx; s32 off; u32 val;
            b = D_80122B74;
            idx = *(s32 *)(b + 0x2EF0);
            off = idx * 0x60;
            val = *(u8 *)(b + 0xAB0);
            b += off;
            *(u8 *)(b + 0x2F0C) = val;
        }
        p = D_80122B74 + *(s32 *)(D_80122B74 + 0x2EF0) * 0x60;
        {
            u32 word = *(u32 *)(D_80122B74 + 0xAB0);
            u32 low = *(u8 *)(p + 0x2F0C);
            low |= (word >> 8) << 8;
            *(u32 *)(p + 0x2F0C) = low;
        }
        {
            u8 *b = D_80122B74;
            s32 off = *(s32 *)(b + 0x2EF0) * 0x60;
            u16 val = *(u16 *)(b + 0xAB4);
            b += off;
            *(u16 *)(b + 0x2F10) = val;
        }
        {
            u8 *b = D_80122B74;
            s32 off = *(s32 *)(b + 0x2EF0) * 0x60;
            off += (s32)b;
            func_800C1EC8(b + 0xAC0, (void *)(off + 0x2F1C), 0x10);
        }
    }
}

