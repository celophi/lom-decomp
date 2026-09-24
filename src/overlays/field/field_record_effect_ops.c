#include "game_audio.h"
#include "common.h"
#include "field_calls.h"
#include "field_records.h"

extern void *func_800C1E40(s32);
extern s32 rand(void);
extern FieldGameState *D_80122B74;

/** @brief Resource 0xB: random effect picks per selector row, then the effect codes. */
typedef struct
{
    u8 header[4];
    /** @brief Sixteen candidate effects for each selector 0x58-0x5F. */
    u8 picks[8][16];
    /** @brief Low six bits: slot entry index minus 0x60; high two bits: result type. */
    u8 codes[1];
} FieldSlotEffectTable;

/** @brief Resource 0x12 row applied to a region record (0x14 bytes). */
typedef struct
{
    /** @brief High nibble subtracted, low nibble added to each stat growth accumulator. */
    u8 stat_deltas[FIELD_CHARACTER_STAT_COUNT];
    /** @brief Same for the four total growth accumulators. */
    u8 total_deltas[4];
    /** @brief Pairs of (kind, value): kinds 0-7 set, 8-15 clear a flag bit; 0xF0/0xF1 set equipment ids. */
    u8 effects[4][2];
} FieldRegionEffectRow;

/** @brief Resource 0x12: a header, then one row per effect id 0x60-0x87. */
typedef struct
{
    u8 header[4];
    FieldRegionEffectRow rows[0x28];
} FieldRegionEffectTable;

/** @brief Low/high threshold pair of resource 0x10. */
typedef struct
{
    u8 low;
    u8 high;
} EffectThreshold;

/** @brief Resource 0x10: threshold pairs; the bytes from 0x14 on also index them per effect row. */
typedef struct
{
    u8 unk00[4];
    EffectThreshold thresholds[256];
} EffectThresholdTable;

/** @brief Byte @p i of the eight threshold indexes for effect row @p row (the rows overlap thresholds[]). */
#define EFFECT_THRESHOLD_INDEX(table, row, i) (*((u8 *)(table) + ((i) + (row)) + 0x14))

/** @brief Offset of menu_slots[0].slots[0].entry in FieldGameState. */
#define MENU_SLOT_ENTRY_OFFSET 0x26F4
/** @brief Offset of menu_slots[0].slots[0].pad8 in FieldGameState. */
#define MENU_SLOT_PAYLOAD_OFFSET 0x26F8

void func_800C0814(FieldRegionRecord *record, s32 effect_id, FieldRegionEffectTable *table);
s32 func_800C0560(s32 group_index, s32 slot_index, EffectThresholdTable *table);

/**
 * @brief Combine random picks for a slot group's selectors into one slot effect and clear its payload.
 * @param group_index Menu slot group index.
 * @param slot_index Slot within the group that receives the effect.
 */
void func_800C0260(s32 group_index, s32 slot_index)
{
    FieldSlotEffectTable *table;
    s32 random_value;
    s32 index;
    s32 effect_index;
    s32 selection;

    table = func_800C1E40(0xB);
    if (((D_80122B74->menu_slots[group_index].flags >> 0xC) & 0xF) == 1)
    {
        index = 0;
        random_value = rand();
        effect_index = table->picks[D_80122B74->menu_slots[group_index].selectors[0] - 0x58][random_value & 0xF];
        random_value = rand();
        selection = table->picks[D_80122B74->menu_slots[group_index].selectors[0] - 0x58][random_value & 0xF];
        effect_index |= selection;
    }
    else
    {
        index = 1;
        random_value = rand();
        effect_index = table->picks[D_80122B74->menu_slots[group_index].selectors[0] - 0x58][random_value & 0xF];
        for (; index < (s32)((D_80122B74->menu_slots[group_index].flags >> 0xC) & 0xF); index++)
        {
            random_value = rand();
            selection = table->picks[D_80122B74->menu_slots[group_index].selectors[index] - 0x58][random_value & 0xF];
            effect_index |= selection;
        }
        index = 0;
    }
    D_80122B74->menu_slots[group_index].slots[slot_index].entry.index = (table->codes[effect_index] & 0x3F) + 0x60;
    D_80122B74->menu_slots[group_index].slots[slot_index].entry.word =
        (D_80122B74->menu_slots[group_index].slots[slot_index].entry.word & ~0x300) | ((table->codes[effect_index] >> 6) << 8);
    do
    {
        D_80122B74->menu_slots[group_index].slots[slot_index].pad8[index] = 0;
        index += 1;
    } while (index < 8);
}

/**
 * @brief Classify every set slot of a menu slot group against the effect thresholds.
 * @param group_index Menu slot group index.
 */
void func_800C0490(s32 group_index)
{
    EffectThresholdTable *table;
    s32 slot_index;

    table = func_800C1E40(0x10);
    if (table == NULL)
    {
        record_game_diagnostic(0x8001, 0x3E7, 0, 0);
        return;
    }
    slot_index = 0;
    do
    {
        if (D_80122B74->menu_slots[group_index].slots[slot_index].entry.index != 0xFF)
        {
            D_80122B74->menu_slots[group_index].slots[slot_index].handle = func_800C0560(group_index, slot_index, table);
        }
        slot_index += 1;
    } while (slot_index < 8);
}

/**
 * @brief Classify a slot's eight payload bytes against its effect thresholds.
 * @param group_index Menu slot group index.
 * @param slot_index Slot within the group.
 * @param table Threshold table (resource 0x10).
 * @return 3 when every byte reaches its high threshold, 2 when every byte reaches its low one,
 *         1 when any byte is nonzero, 0 otherwise.
 */
s32 func_800C0560(s32 group_index, s32 slot_index, EffectThresholdTable *table)
{
    s32 flag;
    s32 i;

    /* Byte view of D_80122B74: the loops index it with integer offset sums (typed menu_slots access: 86.52%). */
    {
        s32 group_offset;
        s32 slot_offset;
        s32 row;
        s32 scan_offset;
        u8 *base;

        i = 0;
        base = (u8 *)D_80122B74;
        slot_offset = slot_index * sizeof(FieldMenuSlot);
        group_offset = group_index * sizeof(FieldMenuSlotGroup);
        row = (*(base + (slot_offset + group_offset) + MENU_SLOT_ENTRY_OFFSET) - 0x60) * 8;
        flag = -1;
        for (i = 0; i < 8; i++)
        {
            scan_offset = i + slot_offset;
            if (*(base + (scan_offset + group_offset) + MENU_SLOT_PAYLOAD_OFFSET) <
                table->thresholds[EFFECT_THRESHOLD_INDEX(table, row, i)].high)
            {
                flag = 0;
                break;
            }
        }
    }
    if (flag != 0)
    {
        return 3;
    }

    {
        s32 group_offset;
        s32 slot_offset;
        s32 row;
        s32 scan_offset;
        u8 *base;

        i = 0;
        base = (u8 *)D_80122B74;
        slot_offset = slot_index * sizeof(FieldMenuSlot);
        group_offset = group_index * sizeof(FieldMenuSlotGroup);
        row = (*(base + (slot_offset + group_offset) + MENU_SLOT_ENTRY_OFFSET) - 0x60) * 8;
        flag = -1;
        for (i = 0; i < 8; i++)
        {
            scan_offset = i + slot_offset;
            if (*(base + (scan_offset + group_offset) + MENU_SLOT_PAYLOAD_OFFSET) <
                table->thresholds[EFFECT_THRESHOLD_INDEX(table, row, i)].low)
            {
                flag = 0;
                break;
            }
        }
    }
    if (flag != 0)
    {
        return 2;
    }

    {
        s32 group_offset;
        s32 slot_offset;
        u8 *base;

        i = 0;
        base = (u8 *)D_80122B74;
        slot_offset = slot_index * sizeof(FieldMenuSlot);
        group_offset = group_index * sizeof(FieldMenuSlotGroup);
        for (i = 0; i < 8; i++)
        {
            if (*(base + (i + slot_offset + group_offset) + MENU_SLOT_PAYLOAD_OFFSET) != 0)
            {
                flag = -1;
            }
        }
    }
    return -flag;
}

/**
 * @brief Apply each pending, not yet applied effect of the stored region records.
 */
void func_800C06E8(void)
{
    FieldRegionEffectTable *table;
    s32 record_index;
    s32 effect_index;
    u32 status;
    u32 value;
    s32 applied;
    FieldRegionRecord *record;

    table = func_800C1E40(0x12);
    record_index = 0;
    do
    {
        if (D_80122B74->regions[record_index].name[0] != 0)
        {
            effect_index = 0;
            do
            {
                value = D_80122B74->regions[record_index].status.effects[effect_index];
                if (value != 0xFF)
                {
                    status = D_80122B74->regions[record_index].status.word;
                    applied = (status >> 24) & 7;
                    if (!((applied >> effect_index) & 1))
                    {
                        record = &D_80122B74->regions[record_index];
                        value = (status & 0xF8FFFFFF) | (((applied | (1 << effect_index)) & 7) << 24);
                        D_80122B74->regions[record_index].status.word = value;
                        func_800C0814(record, D_80122B74->regions[record_index].status.effects[effect_index], table);
                    }
                }
                effect_index += 1;
            } while (effect_index < 3);
        }
        record_index += 1;
    } while (record_index < FIELD_REGION_COUNT);
}

/**
 * @brief Apply one effect row to a stored region record: growth deltas, then flag and equipment changes.
 * @param record Region record to update.
 * @param effect_id Effect id; values outside 0x60 to 0x87 are ignored.
 * @param table Effect table (resource 0x12).
 */
void func_800C0814(FieldRegionRecord *record, s32 effect_id, FieldRegionEffectTable *table)
{
    u32 offset;
    s32 row;
    s32 i;
    s32 value;
    s32 clamped;
    u8 stat_delta;
    u8 total_delta;
    s32 kind;
    s32 amount;
    u8 *bytes;

    offset = effect_id - 0x60;
    if (offset < 0x28U)
    {
        i = 0;
        /* Allocation lever: without the wrapper row and the effect pointer swap s3/s4 (99.42%). */
        do
        {
            row = offset;
        } while (0);
        do
        {
            /* A byte view keeps record + i recomputed per pass, as in the original. */
            bytes = (u8 *)record + i;
            value = bytes[0x4C];
            stat_delta = table->rows[row].stat_deltas[i];
            value = ((u32)value >> 4) + (stat_delta & 0xF) - (stat_delta >> 4);
            if (value >= 0)
            {
                clamped = 0xF;
                if (value < 0x10)
                {
                    clamped = value;
                }
            }
            else
            {
                clamped = 0;
            }
            i += 1;
            bytes[0x4C] = (bytes[0x4C] & 0xF) | (clamped * 0x10);
        } while (i < FIELD_CHARACTER_STAT_COUNT);
        i = 0;
        do
        {
            bytes = (u8 *)record + i;
            value = bytes[0x54];
            total_delta = table->rows[row].total_deltas[i];
            value = ((u32)value >> 4) + (total_delta & 0xF) - (total_delta >> 4);
            if (value >= 0)
            {
                clamped = 0xF;
                if (value < 0x10)
                {
                    clamped = value;
                }
            }
            else
            {
                clamped = 0;
            }
            i += 1;
            bytes[0x54] = (bytes[0x54] & 0xF) | (clamped * 0x10);
        } while (i < 4);
        i = 0;
        do
        {
            kind = table->rows[row].effects[i][0];
            amount = table->rows[row].effects[i][1];
            if (row < 8)
            {
                value = rand() & 0xFF;
                if (value < amount)
                {
                    record->unk48.flags |= 1 << kind;
                }
            }
            else if (row < 0x10)
            {
                value = rand() & 0xFF;
                if (value < amount)
                {
                    record->unk48.flags &= ~(1 << (kind - 8));
                }
            }
            else
            {
                switch (kind)
                {
                case 0xF0:
                    record->weapon_id = amount;
                    break;
                case 0xF1:
                    record->armor_ids[0] = amount;
                    break;
                }
            }
            i += 1;
        } while (i < 4);
    }
}
