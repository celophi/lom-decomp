#include "common.h"

/*
 * Golem logic-block records kept in the shared menu-layout buffer.
 *
 * Each record is one packed word. Bits 0-1 hold the logic type the block is
 * assigned to, with 3 meaning unassigned; bits 2-7 the block id; bits 8-11
 * a detail value; bits 12-15 the shape index; bits 17-18 the rotation; and
 * bits 19-23 and 24-28 the grid position once placed. The golem overlay reads
 * these fields with the same layout.
 */

#define LOGIC_BLOCK_TYPE_UNASSIGNED 0x3
#define LOGIC_BLOCK_ID_SHIFT 2
#define LOGIC_BLOCK_ID_MASK 0xFC
#define LOGIC_BLOCK_DETAIL_SHIFT 8
#define LOGIC_BLOCK_DETAIL_MASK 0xF00
#define LOGIC_BLOCK_SHAPE_SHIFT 12
#define LOGIC_BLOCK_SHAPE_MASK 0xF000
/* Cleared on append and by func_800CBE64; its meaning is not yet established. */
#define LOGIC_BLOCK_FLAG_UNK16 0x10000

/** @brief Logic-block table inside the menu-layout buffer. */
typedef struct
{
    u8 pad_0000[0x29D6];
    u8 logic_block_count;
    u8 pad_29D7[5];
    u32 logic_blocks[40];
} GolemLogicBlockTable;

/** @brief Validation result for one golem logic-block entry. */
typedef struct
{
    u16 flag;
    u16 value;
} GolemLogicBlockValidationResult;

extern u8 g_menuLayoutBuffer[];

/**
 * @brief Append a new, unassigned logic block to the golem logic-block table.
 * @param block_id Six-bit block id stored in bits 2-7.
 * @param detail Four-bit detail value stored in bits 8-11.
 * @param shape Four-bit shape index stored in bits 12-15.
 */
void golem_logic_block_append(u32 block_id, u32 detail, u32 shape)
{
    ((GolemLogicBlockTable *)g_menuLayoutBuffer)->logic_blocks[((GolemLogicBlockTable *)g_menuLayoutBuffer)->logic_block_count] =
        (((GolemLogicBlockTable *)g_menuLayoutBuffer)->logic_blocks[((GolemLogicBlockTable *)g_menuLayoutBuffer)->logic_block_count] & ~LOGIC_BLOCK_ID_MASK) | ((block_id & 0x3F) << LOGIC_BLOCK_ID_SHIFT);
    ((GolemLogicBlockTable *)g_menuLayoutBuffer)->logic_blocks[((GolemLogicBlockTable *)g_menuLayoutBuffer)->logic_block_count] =
        (((GolemLogicBlockTable *)g_menuLayoutBuffer)->logic_blocks[((GolemLogicBlockTable *)g_menuLayoutBuffer)->logic_block_count] & ~LOGIC_BLOCK_DETAIL_MASK) | ((detail & 0xF) << LOGIC_BLOCK_DETAIL_SHIFT);
    ((GolemLogicBlockTable *)g_menuLayoutBuffer)->logic_blocks[((GolemLogicBlockTable *)g_menuLayoutBuffer)->logic_block_count] =
        (((GolemLogicBlockTable *)g_menuLayoutBuffer)->logic_blocks[((GolemLogicBlockTable *)g_menuLayoutBuffer)->logic_block_count] & ~LOGIC_BLOCK_SHAPE_MASK) | ((shape & 0xF) << LOGIC_BLOCK_SHAPE_SHIFT);
    ((GolemLogicBlockTable *)g_menuLayoutBuffer)->logic_blocks[((GolemLogicBlockTable *)g_menuLayoutBuffer)->logic_block_count] |= LOGIC_BLOCK_TYPE_UNASSIGNED;
    ((GolemLogicBlockTable *)g_menuLayoutBuffer)->logic_blocks[((GolemLogicBlockTable *)g_menuLayoutBuffer)->logic_block_count] &= ~LOGIC_BLOCK_FLAG_UNK16;
    ((GolemLogicBlockTable *)g_menuLayoutBuffer)->logic_block_count++;
}

/** @brief Packed layout entry, including its active bit at bit sixteen. */
typedef union
{
    u32 word;
    struct
    {
        unsigned low : 16;
        unsigned active : 1;
        unsigned high : 15;
    } bits;
} Packed;
extern u8 D_800F1CD0[];
extern s32 D_80122C00;
/**
 * @brief Rebuild the menu grid's entry indices from the active composite layouts.
 * @return Grid bound stored in the selected layout's high nibble.
 * @note Empty cells receive 99; each layout part writes its owning entry index.
 * @note 100% match with GCC 2.7.2 CDK: 112 instructions, 448 bytes.
 */
u32 func_800CB758(void)
{
    s32 empty = 99;
    s32 index, part, count, base, table, x, z, clear_base, return_base;
    u32 packed, value;
    Packed bits;
    u8 *entry, *shape;
    index = 35;
    clear_base = (s32)g_menuLayoutBuffer;
    do
    {
        ((u8 *)(index * 4 + clear_base))[0x2A7F] = empty;
        index--;
    } while (index >= 0);
    index = 0;
    count = g_menuLayoutBuffer[0x29D6];
    if (count != 0)
    {
        table = (s32)D_800F1CD0;
        base = (s32)g_menuLayoutBuffer;
        do
        {
            entry = (u8 *)((s32)g_menuLayoutBuffer + index * 4);
            packed = *(u32 *)(entry + 0x29DC);
            bits.word = packed;
            if (bits.bits.active == 1 && (packed & 3) == ((u8 *)(D_80122C00 + base))[0x29D8])
            {
                part = 0;
                if (*(u8 *)((((packed >> 12) & 15) * 0x58) + table) != 0)
                {
                    do
                    {
                        value = *(u32 *)(entry + 0x29DC);
                        shape = (u8 *)(((((value >> 17) & 3) * 5 + part) * 4) +
                                       (((value >> 12) & 15) * 0x58) + table);
                        x = ((s32)(value << 8) >> 27) + (s8)shape[12];
                        z = ((s32)(value << 3) >> 27) + (s8)shape[13];
                        x += z * 6;
                        ((u8 *)(x * 4 + base))[0x2A7F] = index;
                        part++;
                    } while (part <
                             *(u8 *)((((*(u32 *)(entry + 0x29DC) >> 12) & 15) * 0x58) + table));
                }
            }
            index++;
        } while (index < count);
    }
    return_base = (s32)g_menuLayoutBuffer;
    return ((u8 *)(((u8 *)(D_80122C00 + return_base))[0x29D8] * 0x14C + return_base))[0x2B50] >> 4;
}


/**
 * @brief Update a composite layout record and mark each occupied part in the menu grid.
 * @param arg0 Layout record index to update.
 * @param arg1 Part-group index within the selected layout.
 * @param arg2 Horizontal grid offset applied to each part.
 * @param arg3 Vertical grid offset applied to each part.
 */
void func_800CB918(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 cell_offset;
    s32 count;
    s32 base;
    s32 cursor;
    s32 x;
    s32 y;
    u32 packed;
    u32 mask1;
    u32 mask2;
    u32 mask3;
    s32 layout_index;
    u8 *table;
    u8 *part;

    do
    {
        cell_offset = arg0 * 4;
    } while (0);
    mask1 = 0xFFF9FFFF;
    mask2 = 0xFF07FFFF;
    mask3 = 0xE0FFFFFF;
    base = (s32)g_menuLayoutBuffer;
    layout_index = D_80122C00;
    packed = (((((((*(u32 *)(cell_offset + base + 0x29DC) & ~3)
                        | (((u8 *)base)[layout_index + 0x29D8] & 3) | 0x10000)
                       & mask1)
                      | ((arg1 & 3) << 17))
                     & mask2)
                    | ((arg2 & 0x1F) << 19))
                   & mask3)
                  | ((arg3 & 0x1F) << 24);
    table = D_800F1CD0;
    *(u32 *)(cell_offset + base + 0x29DC) = packed;
    count = 0;
    if (table[((packed >> 12) & 0xF) * 0x58] != 0)
    {
        u8 *loop_table;
        u8 *grid;
        u32 shape;
        u32 limit_shape;

        do
        {
            loop_table = table;
        } while (0);
        if (cell_offset != 0)
        {
            grid = (u8 *)base;
        }
        else
        {
            grid = g_menuLayoutBuffer;
        }
        cursor = arg1 * 0x14;
        do
        {
            shape = *(u32 *)(cell_offset + (s32)grid + 0x29DC);
            shape >>= 12;
            shape &= 0xF;
            part = (u8 *)(cursor + shape * 0x58 + (s32)loop_table);
            x = arg2 + *(s8 *)(part + 0xC);
            y = arg3 + *(s8 *)(part + 0xD);
            x += y * 6;
            grid[x * 4 + 0x2A7F] = arg0;
            limit_shape = *(u32 *)(cell_offset + (s32)grid + 0x29DC);
            do
            {
                count++;
            } while (0);
            limit_shape >>= 12;
            limit_shape &= 0xF;
            cursor += 4;
        } while (count < loop_table[limit_shape * 0x58]);
    }
}

extern u8 D_800F2098[];

/**
 * @brief Check shape occupancy and the selected logic class before placement.
 * @param arg0 Logic-block record index.
 * @param arg1 Shape rotation index.
 * @param arg2 Horizontal grid offset.
 * @param arg3 Vertical grid offset.
 * @return One when the occupied cells are empty and the class is compatible.
 * @note Nonmatching C recovered from the existing m2c-based draft.
 */
s32 func_800CBA9C(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 temp_a1;
    s32 temp_v1;
    s32 cell_offset;
    s32 valid;
    s32 saved_arg0;
    u8 *temp_v0;
    s32 cursor;
    u8 *table;
    u8 *grid;
    s32 idx;
    s32 record_byte;
    s32 lookup;
    s32 limit;

    saved_arg0 = arg0;
    valid = 1;
    cell_offset = saved_arg0 * 4;
    table = D_800F1CD0;
    cursor = (s32)g_menuLayoutBuffer;

    idx = (*(u32 *)(cell_offset + cursor + 0x29DC) >> 12) & 0xF;
    if (table[(((idx << valid) + idx) * 4 - idx) * 8] != 0)
    {
        grid = (u8 *)cursor;
        cursor = arg1 * 0x14;
        arg0 = 0;
        do
        {
            idx = (*(u32 *)(cell_offset + (s32)grid + 0x29DC) >> 12) & 0xF;
            temp_v0 = (u8 *)(cursor + (((idx << 1) + idx) * 4 - idx) * 8 + (s32)table);
            temp_v1 = *(s8 *)(temp_v0 + 0xC) + arg2;
            temp_a1 = *(s8 *)(temp_v0 + 0xD) + arg3;

            if (grid[(temp_v1 + temp_a1 * 6) * 4 + 0x2A7F] != 0x63)
            {
                valid = 0;
            }

            cursor += 4;
        } while (++arg0 < table[(((idx << 1) + idx) * 4 - idx) * 8]);
    }

    cursor = (s32)g_menuLayoutBuffer;
    record_byte = *(u8 *)(cell_offset + cursor + 0x29DC) & 0xFC;
    lookup = *(s32 *)((u8 *)D_800F2098 + record_byte);
    if (lookup != 0)
    {
        idx = *(u8 *)(D_80122C00 + cursor + 0x29D8);
        limit = *(u8 *)(idx * 332 + cursor + 0x2B50) & 0xF;
        if (lookup != limit)
        {
            valid = 0;
        }
    }

    return valid;
}


/**
 * @brief Check whether all parts of a selected composite layout fit within the active menu grid.
 * @param arg0 Packed menu-record index used to select the layout variant.
 * @param arg1 Part-group index within the selected layout.
 * @param arg2 Horizontal offset added to each part.
 * @param arg3 Vertical offset added to each part.
 * @return 1 when every examined part is within the clamped grid bound, otherwise 0.
 */
s32 func_800CBC0C(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 temp_a1;
    s32 temp_v1;
    s32 cell_offset;
    s32 valid;
    u32 limit;
    s32 saved_arg0;
    u8 *temp_v0;
    s32 cursor;
    u8 *table;

    cursor = (s32)g_menuLayoutBuffer;
    limit = ((u8 *)cursor)[((u8 *)cursor)[D_80122C00 + 0x29D8] * 0x14C + 0x2B50] & 0xF0;
    limit >>= 4;
    saved_arg0 = arg0;
    if ((s32)limit >= 6)
    {
        limit = 6;
        valid = 1;
    }
    else
    {
        valid = 1;
    }
    cell_offset = saved_arg0 * 4;
    table = D_800F1CD0;
    arg0 = 0;
    if (table[((((((*(u32 *)(cell_offset + cursor + 0x29DC) >> 12) & 0xF) << valid)
                  + ((*(u32 *)(cell_offset + cursor + 0x29DC) >> 12) & 0xF)) * 4
                 - ((*(u32 *)(cell_offset + cursor + 0x29DC) >> 12) & 0xF)) * 8)] != 0)
    {
        u8 *grid;
        grid = (u8 *)cursor;
        cursor = arg1 * 0x14;
        do
        {
            temp_v0 = (u8 *)(cursor + ((((u32)*(u32 *)(cell_offset + (s32)grid + 0x29DC) >> 12) & 0xF) * 0x58) + (s32)table);
            temp_v1 = *(s8 *)(temp_v0 + 0xC) + arg2;
            temp_a1 = *(s8 *)(temp_v0 + 0xD) + arg3;
            if ((temp_v1 < 0) || (temp_v1 >= (s32)limit))
            {
                valid = 0;
            }
            if ((temp_a1 < 0) || (cell_offset = saved_arg0 * 4, temp_a1 >= (s32)limit))
            {
                valid = 0;
                cell_offset = saved_arg0 * 4;
            }
            cursor += 4;
        } while (++arg0 < (s32)table[(((*(u32 *)(cell_offset + (s32)grid + 0x29DC) >> 12) & 0xF) * 0x58)]);
    }
    return valid;
}

extern u8 D_800459AE;
extern u8 D_800F2180[];

/**
 * @brief Validate menu entries against their reference classes and fill result pairs.
 * @param arg0 Output array of flag/value pairs, one per menu-layout entry.
 * @return The global status byte D_800459AE.
 */
u8 func_800CBD70(void *arg0)
{
    GolemLogicBlockValidationResult *out;
    u8 *buf;
    s32 i;
    s32 word;
    s32 type;
    u8 ref_class;
    s32 table_val;

    out = (GolemLogicBlockValidationResult *)arg0;
    i = 0;
    if (g_menuLayoutBuffer[0x29D6] != 0)
    {
        buf = g_menuLayoutBuffer;
        do
        {
            word = *(s32 *)(buf + 0x29DC + i * 4);
            ref_class = buf[D_80122C00 + 0x29D8];
            ref_class++;
            ref_class--;
            type = word & 3;
            if ((type != ref_class && type != 3) ||
                (((table_val = *(s32 *)(D_800F2098 + (word & 0xFC))) != 0) &&
                 ((buf[ref_class * 332 + 0x2B50] & 0xF) != table_val)))
            {
                *(u16 *)(out + i) = 1;
                *((u16 *)(out + i) + 1) = 0xF;
            }
            else
            {
                *(u16 *)(out + i) = 0;
                *((u16 *)(out + i) + 1) = ((u16 *)D_800F2180)[(*(u8 *)(buf + 0x29DC + i * 4) & 0xFC) >> 1];
            }
            i++;
        } while (i < buf[0x29D6]);
    }
    return D_800459AE;
}

/**
 * @brief Clears a record's logic-block flag and reactivates matching entries.
 *
 * Clears bit 0x10000 of the packed 32-bit logic-block word at
 * @c g_menuLayoutBuffer[arg0*4 + 0x29DC], then scans all 0x24 records (stride
 * 4) and sets the byte flag at +0x2A7F to 0x63 for every record whose flag
 * currently equals @p arg0.
 *
 * @param arg0 Record index whose logic-block bit is cleared and whose value is
 *             matched against each record's +0x2A7F flag.
 */
void func_800CBE64(s32 arg0)
{
    s32 i;
    u8 *p;
    u8 *base = g_menuLayoutBuffer;
    u8 *rec = base + arg0 * 4;
    /* Reserves the target's unused 8-byte stack frame slot (FRAME-03). */
    volatile s32 pad;

    *(u32 *)(rec + 0x29DC) &= ~0x10000;

    for (i = 0; i < 0x24; i++)
    {
        p = base + i * 4;
        if (p[0x2A7F] == arg0)
        {
            p[0x2A7F] = 0x63;
        }
    }
}


typedef struct
{
    u8 pad[0x2A7F];
    u8 unk2A7F;
} FieldCBEC4MenuScan;

/**
 * @brief Clear boundary markers and mark differing occupied vertical neighbors.
 * @param arg0 Output buffer containing 60 word-sized markers.
 */
void func_800CBEC4(void *arg0)
{
    u8 *scan;
    u8 *base;
    s32 *clear;
    s32 count;
    s32 offset;
    s32 sentinel;
    s32 marker;
    u8 first;
    u8 second;
    void *out;

    out = arg0;
    count = 0x3B;
    clear = (s32 *)((u8 *)out + 0xEC);
    do
    {
        *clear = 0;
        count--;
        clear--;
    } while (count >= 0);

    count = 0;
    base = g_menuLayoutBuffer;
    sentinel = 0x63;
    marker = 0x4F;
    offset = 0x18;
    scan = base;
    do
    {
        first = ((FieldCBEC4MenuScan *)scan)->unk2A7F;
        if (first != sentinel)
        {
            second = ((FieldCBEC4MenuScan *)((u32)offset + (u32)base))->unk2A7F;
            if ((second != sentinel) && (first != second))
            {
                *(s32 *)((u8 *)out + 0x78) = marker;
            }
        }
        out = (u8 *)out + 4;
        offset += 4;
        count++;
        scan += 4;
    } while (count < 0x1E);
}
