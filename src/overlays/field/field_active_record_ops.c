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
    s32 level_index;
    s32 record_index;
    s32 record_offset;
    u32 packed_value;
    u32 saved_flag;
    u8 level;
    u8 *table;
    u8 *bank;

    if (record_id < 0xC)
    {
        table = func_800C1E40(3);
        do
        {
            do
            {
                record_index = 0;
            } while (0);
        } while (0);
        if (*(u16 *)(table + 2) != 0)
        {
            do
            {
                if ((*(s32 *)(table + record_index * 0x944 + 4)) == record_id)
                {
                    record_offset = record_index * 0x944 + 4;
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
                    else if (level < 0x12U)
                    {
                        bank = table + record_offset + 0x4A4;
                    }
                    else
                    {
                        bank = table + record_offset + 0x6F4;
                    }
                    func_800C1EC8(bank, D_80122B74 + 0x840, 0x250);
                    ((Context *)D_80122B74)->flags.word = (((Context *)D_80122B74)->flags.word & ~0x80) | (saved_flag << 7);
                    if (((Context *)D_80122B74)->level < 0x20U)
                    {
                        level_index = ((Context *)D_80122B74)->level - 1;
                    }
                    else
                    {
                        level_index = 0x1F;
                    }
                    packed_value = ((Context *)D_80122B74)->packed.byte[0];
                    packed_value = packed_value | ((*(s32 *)((u8 *)((s32)D_80122B74 - -((record_id + 0x68 + level_index - level_index) * 4)) + 0xE4) +
                                                    D_800F190C[level_index])
                                                   << 8);
                    ((Context *)D_80122B74)->packed.word = packed_value;
                    if ((s32)(packed_value >> 8) > 0x98967F)
                    {
                        u32 clamped_value;

                        clamped_value = packed_value & 0xFF;
                        clamped_value |= 0x98967F00;
                        ((Context *)D_80122B74)->packed.word = clamped_value;
                    }
                    func_800C11F0(1, 0);
                    func_800B7C58(1);
                    func_800BD520(0, record_id * 8 + 0xF87, 1);
                    return -1;
                }
                record_index++;
            } while (record_index < (s32)*(u16 *)(table + 2));
        }
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

typedef struct
{
    unsigned short low : 9;
    unsigned short high : 7;
} ActivePacked16;

typedef struct
{
    unsigned int low : 8;
    unsigned int high : 24;
} ActivePacked32;

typedef struct
{
    u8 pad0[0x24];
    u16 values24[4];
    u8 bytes2C[0x14];
} ActiveSlot;

typedef struct
{
    u8 head[0x15];
    u8 pad15[3];
    u32 flags18;
    u8 pad1C[4];
    ActivePacked32 packed20;
    u16 value24;
    u16 value26;
    u16 values28[4];
    ActivePacked16 packed30[8];
    u8 bytes40[4];
    u8 pad44[4];
    u8 init48[8];
    u8 data50[0x24];
    u16 value74;
    u8 pad76[0x1A];
    ActiveSlot slots90[3];
} ActiveRecordFull;

typedef struct
{
    u8 head[0x15];
    u8 byte15;
    u8 pad16[2];
    ActivePacked32 packed18;
    u16 value1C;
    u16 value1E;
    u16 values20[4];
    ActivePacked16 packed28[8];
    u8 bytes38[4];
    u8 resource3C;
    u8 resources3D[3];
    u8 pad40[0x20];
} SavedRecordFull;

typedef struct
{
    u8 pad0[0xA90];
    ActiveRecordFull active;
    u8 padBE0[0x2EF4 - 0xBE0];
    SavedRecordFull saved[5];
} FieldRecordBufferFull;

typedef struct
{
    u8 bytes[0x60];
} SavedRecordRaw;

typedef struct
{
    u8 pad0[0xA90];
    u8 active_head[0x15];
    u8 padAA5[0x244F];
    SavedRecordRaw saved[5];
} FieldRecordBuffer;

void func_800C3B50(void);
void func_800C3A00(s32);
void func_800C32C8(void);
void field_release_actor_resource_slot(s32);


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

/**
 * @brief Restore a saved secondary record and its equipment tables.
 * @param arg0 Saved record index to restore.
 */
void func_800C2E30(s32 arg0)
{
    s32 temp_s1;
    u8 *temp_v0_4;
    s32 temp_v1_2;
    s32 var_a0_2;
    s32 var_a2;
    s32 var_s0;
    s32 var_s1;
    u8 *temp_a0;
    u8 *temp_a0_2;
    u8 *temp_a1;
    u8 *temp_a2;
    u8 *temp_v0_2;
    u8 *temp_v0_6;
    u8 *var_v1_2;
    u8 **record_global;

    var_s0 = 0;
    do
    {
        ((FieldRecordBuffer *)D_80122B74)->active_head[var_s0] = ((FieldRecordBuffer *)D_80122B74)->saved[arg0].bytes[var_s0];
        var_s0 += 1;
    } while (var_s0 < 0x15);
    ACTIVE_U32(D_80122B74, 0xAA8) = (s32) (((ACTIVE_U32(D_80122B74, 0xAA8) & ~0x7F) | 3) & ~0x80);
    ACTIVE_U8(D_80122B74, 0xAA9) = ((FieldRecordBufferFull *)D_80122B74)->saved[arg0].byte15;
    ((FieldRecordBufferFull *)D_80122B74)->active.packed20.low = ((FieldRecordBufferFull *)D_80122B74)->saved[arg0].packed18.low;
    ((FieldRecordBufferFull *)D_80122B74)->active.packed20.high = ((FieldRecordBufferFull *)D_80122B74)->saved[arg0].packed18.high;
    var_s0 = 0;
    ((FieldRecordBufferFull *)D_80122B74)->active.value24 = ((FieldRecordBufferFull *)D_80122B74)->saved[arg0].value1C;
    ((FieldRecordBufferFull *)D_80122B74)->active.value26 = ((FieldRecordBufferFull *)D_80122B74)->saved[arg0].value1E;
    do
    {
        ((FieldRecordBufferFull *)D_80122B74)->active.values28[var_s0] = ((FieldRecordBufferFull *)D_80122B74)->saved[arg0].values20[var_s0];
        var_s0 += 1;
    } while (var_s0 < 4);
    var_s0 = 0;
    temp_a2 = D_80122B74;
    var_a2 = arg0 * 0x60;
    do
    {
        temp_a0_2 = temp_a2 + var_a2;
        var_a2 += 2;
        ((ActivePacked16 *)(temp_a2 + var_s0 * 2 + 0xAC0))->low = ((ActivePacked16 *)(temp_a0_2 + 0x2F1C))->low;
        ((ActivePacked16 *)(temp_a2 + var_s0 * 2 + 0xAC0))->high = ((ActivePacked16 *)(temp_a0_2 + 0x2F1C))->high;
        var_s0 += 1;
    } while (var_s0 < 8);
    ((FieldRecordBufferFull *)D_80122B74)->active.bytes40[0] = ((FieldRecordBufferFull *)D_80122B74)->saved[arg0].bytes38[0];
    ((FieldRecordBufferFull *)D_80122B74)->active.bytes40[1] = ((FieldRecordBufferFull *)D_80122B74)->saved[arg0].bytes38[1];
    ((FieldRecordBufferFull *)D_80122B74)->active.bytes40[2] = ((FieldRecordBufferFull *)D_80122B74)->saved[arg0].bytes38[2];
    var_s0 = 0;
    ((FieldRecordBufferFull *)D_80122B74)->active.bytes40[3] = ((FieldRecordBufferFull *)D_80122B74)->saved[arg0].bytes38[3];
    do
    {
        ((FieldRecordBufferFull *)D_80122B74)->active.init48[var_s0] = var_s0;
        var_s0 += 1;
    } while (var_s0 < 8);
    temp_v0_4 = func_800C1E40(0xD);
    if (temp_v0_4 != 0)
    {
        temp_a0 = D_80122B74 + arg0 * 0x60;
        func_800C1EC8(temp_v0_4 + ((ACTIVE_U8(temp_a0, 0x2F30) << 6) + 4), D_80122B74 + 0xAE0, 0x40);
    }
    record_global = &D_80122B74;
    var_s1 = arg0 * 0x60;
    ACTIVE_U16(*record_global, 0xB04) = (u16) ACTIVE_U16(*record_global + var_s1, 0x2F12);
    temp_v0_4 = func_800C1E40(0xE);
    if (temp_v0_4 != 0)
    {
        for (var_s0 = 0; var_s0 < 3; var_s0 += 1)
        {
            temp_v0_2 = D_80122B74;
            temp_s1 = arg0 * 0x60;
            var_s1 = 0xB20 + var_s0 * 0x40;
            temp_v1_2 = var_s0 + temp_s1;
            func_800C1EC8(temp_v0_4 + ((ACTIVE_U8(temp_v0_2 + temp_v1_2, 0x2F31) << 6) + 4), temp_v0_2 + var_s1, 0x40);
        }
    }
    var_s0 = 0;
    temp_a1 = D_80122B74;
    var_v1_2 = temp_a1 + 0xB20;
    var_a0_2 = arg0 * 0x60;
    do
    {
        temp_v0_6 = temp_a1 + var_a0_2;
        var_a0_2 += 2;
        var_s0 += 1;
        ACTIVE_U16(var_v1_2, 0x24) = (u16) ACTIVE_U16(temp_v0_6, 0x2F14);
        var_v1_2 += 2;
    } while (var_s0 < 4);
    ((FieldRecordBufferFull *)D_80122B74)->active.slots90[0].bytes2C[0] = ((FieldRecordBufferFull *)D_80122B74)->saved[arg0].bytes38[0];
    ((FieldRecordBufferFull *)D_80122B74)->active.slots90[0].bytes2C[1] = ((FieldRecordBufferFull *)D_80122B74)->saved[arg0].bytes38[2];
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
 * @brief Close either the primary (arg0 == 0) or secondary record and notify field_release_actor_resource_slot.
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
    field_release_actor_resource_slot(arg0);
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
