#include "main.h"

typedef struct
{
    s16 unk0;
    s16 unk2;
} FieldC7090State;

extern u16 g_gosub_result_count;
extern s32 g_gosub_result_values[];
extern u8 g_menuLayoutBuffer[];
extern s16 D_80122C10;
extern u16 D_80122C16;
extern s32 D_80045EC8;
extern void akao_set_song_params(s32 flags, s32 duration, s32 field_id, s32 sub_id);

/**
 * @brief Decode the gosub-selected record flags into its display id and mode.
 * @see decomp.me (100%)
 */
void func_800C7090(void)
{
    s32 idx;
    u8 *base;
    u8 *rec;
    s32 flags;

    if (*(s32 *)&g_gosub_result_count != 0)
    {
        idx = g_gosub_result_values[0];
        if (idx < 5)
        {
            base = g_menuLayoutBuffer;
            rec = base + idx * 0x60;
            flags = *(s32 *)(rec + 0x2F38);
            if (flags < 0)
            {
                ((FieldC7090State *)&D_80122C10)->unk0 = rec[0x2F0A] + 0x53;
                ((FieldC7090State *)&D_80122C10)->unk2 = 0;
            }
            else
            {
                s16 masked = (s16)(((u32) flags >> 30) & 1);
                ((FieldC7090State *)&D_80122C10)->unk0 = rec[0x2F09] + 0x12;
                ((FieldC7090State *)&D_80122C10)->unk2 = masked;
            }
            if (idx == D_80045EC8)
            {
                ((FieldC7090State *)&D_80122C10)->unk0 = 0xFE;
            }
        }
        else
        {
            akao_set_song_params(0x8002, 0x27, idx, 0);
        }
    }
    D_80122C16 = (u16) *(s32 *)&g_gosub_result_count;
}

/**
 * @brief Three-word parameter block forwarded to field_open_gosub_screen_sequence.
 */
typedef struct
{
    s32 unk0; /* 0x00 */
    s32 unk4; /* 0x04 */
    s32 unk8; /* 0x08 */
} UnkStruct80051EC0;

typedef struct
{
    u8 unk0;
    u8 pad1[0xD];
    s16 unkE;
    u8 pad10[4];
    u16 unk14;
} UnkStruct80122C02;

typedef struct
{
    s16 unk0;
    s16 unk2;
    u16 unk4;
} UnkStruct80122C12;

void field_open_gosub_screen_sequence(UnkStruct80051EC0 *arg0);
void func_800B2844(s32 arg0, u8 *arg1, u8 arg2);

extern UnkStruct80051EC0 D_80051EC0;
extern UnkStruct80051EC0 D_80051ECC;
extern u8 D_80045ECC[];
extern s32 D_801227F0;
extern UnkStruct80122C02 D_80122C02;
extern UnkStruct80122C12 D_80122C12;
extern u16 D_80122C16;
extern u16 g_gosub_result_count;
extern s32 g_gosub_result_values[];
extern u8 g_menuLayoutBuffer[];

/**
 * @brief Counts active gosub-result entries whose bit 30 flag is set.
 *
 * When there are gosub results, walks the five 0x60-byte entries starting at
 * g_menuLayoutBuffer[0x2EF4]; for each entry whose leading byte is nonzero and
 * whose word at +0x44 has bit 30 set, increments the tally stored to
 * D_80122C16.
 *
 * @note Reads g_gosub_result_count as a full word here, while the other
 *       functions in this file read it as a halfword.
 */
void func_800C7168(void)
{
    s32 count;
    s32 i;
    s32 one = 1;
    u8 *p;

    if (*(s32 *)&g_gosub_result_count != 0)
    {
        count = 0;
        for (i = 0; i < 5; i++)
        {
            p = &g_menuLayoutBuffer[i * 0x60];
            if (p[0x2EF4] != 0 &&
                (((*(u32 *)(p + 0x2F38) >> 30) & 1) == one))
            {
                count++;
            }
        }
    }
    D_80122C16 = count;
}

/**
 * @brief Flag the current gosub result's menu layout entry, or trigger a song-select cue.
 * @note Calls akao_set_song_params with no prototype in scope, matching the field116.c
 *       convention; this is required to match.
 */
void func_800C71D4(void)
{
    s32 idx;
    u8 *base;
    u8 *rec;

    idx = g_gosub_result_values[0];
    if (idx < 5)
    {
        base = g_menuLayoutBuffer;
        rec = &base[idx * 0x60];
        *(u32 *)(rec + 0x2F38) |= 0x40000000;
    }
    else
    {
        akao_set_song_params(0x8002, 0x29, idx, 0);
    }
}

/**
 * @brief Copy the D_80051EC0 constant onto the stack and forward it to field_open_gosub_screen_sequence.
 */
void func_800C7238(void)
{
    UnkStruct80051EC0 local;

    local = D_80051EC0;
    field_open_gosub_screen_sequence(&local);
}

/**
 * @brief Reset D_801227F0 and latch the current gosub result index and count.
 */
void func_800C7278(void)
{
    s32 temp;

    D_801227F0 = 0;
    temp = g_gosub_result_values[0];
    D_80122C12.unk0 = temp;
    D_80122C12.unk4 = g_gosub_result_count;
}

/**
 * @brief Copy the D_80051ECC constant onto the stack and forward it to field_open_gosub_screen_sequence.
 */
void func_800C72A4(void)
{
    UnkStruct80051EC0 sp10;

    sp10 = D_80051ECC;
    field_open_gosub_screen_sequence(&sp10);
}

/**
 * @brief Record the current gosub result index and count, then emit its portrait icon.
 */
void func_800C72E4(void)
{
    s32 temp;

    D_80122C02.unkE = temp = g_gosub_result_values[0];
    D_80122C02.unk14 = g_gosub_result_count;
    func_800B2844(D_80122C02.unk0, (temp * 0x60) + D_80045ECC, 0xFF);
}

void func_800B2844();
void akao_set_song_params(s32 flags, s32 duration, s32 field_id, s32 sub_id);

extern u8 D_800F0E98[];
extern u8 D_80045ECC[];
extern s16 D_80122C10;
extern u16 D_80122C16;
extern u8 g_menuLayoutBuffer[];

/**
 * @brief Dispatch the selected menu record's extra slots and fixed trailing slot.
 * @note Skip 0xFE/0xFF entries and record the count of dispatched extra slots.
 * @note Selections of five or greater issue an AKAO command instead.
 * @note WIP: instruction ordering and temporary-register differences remain.
 */
void func_800C7340(void)
{
    s32 selection;
    s32 slot_index;
    s32 dispatch_count;
    s32 record_offset;
    u8 *menu_base;
    u8 slot_flag;
    s32 table_offset;
    u8 *table_base;

    selection = D_80122C10;
    if (selection < 5)
    {
        dispatch_count = 0;
        slot_index = 0;
        menu_base = g_menuLayoutBuffer;
        record_offset = selection * 0x60;
        do
        {
            table_base = D_800F0E98;
            slot_flag = menu_base[slot_index + record_offset + 0x2F38];
            if (slot_flag != 0xFF)
            {
                if (slot_flag != 0xFE)
                {
                    table_offset = slot_flag * 2;
                    func_800B2844(dispatch_count, D_800F0E98[table_offset] + (D_800F0E98[table_offset + 1] << 8) + table_base, 0xFF);
                    dispatch_count++;
                }
            }
            slot_index++;
        } while (slot_index < 3);
        func_800B2844(3, (selection * 0x60) + D_80045ECC, 0xFF);
        D_80122C16 = (u16)dispatch_count;
        return;
    }
    akao_set_song_params(0x8002, 0x2E, selection, 0);
}

/** @brief Clear the selected large-history record status. */
void func_800C745C(void)
{
    PadContext* ctx = (PadContext*)g_menuLayoutBuffer;
    s32 idx = ctx->large_history_index;
    ((u8*)ctx)[0xC06] = 0;
    ctx->large_history_records[idx].unknown_0x46 = 0;
}

typedef struct
{
    s16 unk0;
    s16 unk2;
} FieldC7494State;

extern s16 D_80122C10;
extern u8 g_menuLayoutBuffer[];
extern void akao_set_song_params(s32 flags, s32 duration, s32 field_id, s32 sub_id);

/** @brief Write an available extra history slot or issue audio command 0x30. */
void func_800C7494(void)
{
    s32 idx;
    s32 val;
    s32 count;
    s32 row;
    u8 *base;
    u8 *slot;

    idx = ((FieldC7494State *)&D_80122C10)->unk0;
    val = ((FieldC7494State *)&D_80122C10)->unk2;
    if (idx < 5)
    {
        count = 0;
        base = g_menuLayoutBuffer;
        row = idx * 0x60;
        do
        {
            slot = (u8 *)((count + row) + (s32)base);
            count++;
            if ((u32)((slot[0x2F38] + 2) & 0xFF) < 2)
            {
                slot[0x2F38] = val;
                return;
            }
        } while (count < 3);
        return;
    }
    akao_set_song_params(0x8002, 0x30, idx, 0);
}

extern s16 D_80122C10;

/** @brief Replace the selected history index with its entry id. */
void func_800C752C(void)
{
    D_80122C10 = g_menuLayoutBuffer[(D_80122C10 * 0x60) + 0x2F09];
}

extern s32 g_gosub_result_values[];
extern u8 g_menuLayoutBuffer[];

/**
 * @brief Clears the active flag for the selected small history slot.
 *
 * Uses the small-history index at offset 0x2EF0 to select a 0x60-byte record
 * and clears bit 30 of its word at offset 0x2F38. Out-of-range indices trigger
 * the corresponding song-parameter command instead.
 *
 * @note The implicit akao_set_song_params declaration is required for the
 *       matching call convention used by this field code.
 */
void func_800C7558(void)
{
    s32 index;
    u8 *base;
    u8 *record;

    base = g_menuLayoutBuffer;
    index = *(s32 *)(base + 0x2EF0);
    if (index < 5)
    {
        record = base + index * 0x60;
        *(u32 *)(record + 0x2F38) &= 0xBFFFFFFF;
    }
    else
    {
        akao_set_song_params(0x8002, 0x32, index, 0);
    }
}

/**
 * @brief Clears the active flag for the gosub-selected small history slot.
 *
 * Uses the first gosub result to select a 0x60-byte record and clears bit 30
 * of its word at offset 0x2F38. Out-of-range indices trigger the corresponding
 * song-parameter command instead.
 *
 * @note The implicit akao_set_song_params declaration is required for the
 *       matching call convention used by this field code.
 */
void func_800C75C0(void)
{
    s32 index;
    u8 *base;
    u8 *record;

    index = g_gosub_result_values[0];
    if (index < 5)
    {
        base = g_menuLayoutBuffer;
        record = base + index * 0x60;
        *(u32 *)(record + 0x2F38) &= 0xBFFFFFFF;
    }
    else
    {
        akao_set_song_params(0x8002, 0x32, index, 0);
    }
}

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

extern u16 g_gosub_result_count;
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

    if (*(s32 *)&g_gosub_result_count != 0)
    {
        history_index = g_gosub_result_values[0];
        history_data = (FieldMenuHistoryData*)g_menuLayoutBuffer;
        D_80122C00 = history_data->small_history_records[history_index].unk5A;
    }
}
