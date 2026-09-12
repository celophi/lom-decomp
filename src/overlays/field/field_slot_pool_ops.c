#include "common.h"

extern u8 *D_80123FC4;
extern u8 *D_80123FC0;
s32 func_800BF514(s32);
void func_800BF8E0(void);
void func_800BF944(void);
void func_800BF730(void);


/** @brief Active-sequence record; base holds unk0/unk4, records stride 0xC. */
typedef struct
{
    u8 unk0;    /* 0x00 sequence id */
    u8 pad1[3];
    s32 unk4;   /* 0x04 current record index */
    s32 unk8;   /* 0x08 record payload */
    u8 padC[4];
    s32 unk10;  /* 0x10 per-record flags */
} SeqRec;

extern u8 *g_field_script;


/**
 * @brief Advance the sequence cursor and stage a new timed record.
 *
 * If the current record already has a payload, advances the cursor @c unk4.
 * Writes @c D_80123FC0 + (arg0 low 16 bits) into the (possibly advanced)
 * record's @c unk8, clears then re-masks its @c unk10 low bit, and finally
 * dispatches func_800BD434 with the base @c unk0 id and notifies field_script_run.
 *
 * @param arg0 Duration/parameter; the low 16 bits are added to @c D_80123FC0.
 * @see decomp.me (100%) TODO
 */
void func_800BF2F0(s32 arg0)
{
    s32 temp_v1;
    u8 *p;

    temp_v1 = ((SeqRec *)g_field_script)->unk4;
    if (((SeqRec *)(g_field_script + (temp_v1 * 3 << 2)))->unk8 != 0)
    {
        ((SeqRec *)g_field_script)->unk4 = temp_v1 + 1;
    }
    p = g_field_script;
    ((SeqRec *)(p + (((SeqRec *)p)->unk4 * 3 << 2)))->unk8 = (s32)D_80123FC0 + (arg0 & 0xFFFF);
    ((SeqRec *)(p + (((SeqRec *)p)->unk4 * 3 << 2)))->unk10 &= ~1;
    ((SeqRec *)(p + (((SeqRec *)p)->unk4 * 3 << 2)))->unk10 &= 1;
    func_800BD434(((SeqRec *)p)->unk0, 0xD0000000, 0);
    field_script_run(g_field_script);
}


typedef struct
{
    u8 pad0[0x984];
    u16 unk984;
    u16 unk986;
    u16 unk988;
    u16 unk98A;
} RecFC0;

s32 func_800BF514(s32 arg0);
void func_800BF2F0(s32 arg0);

extern u8 *D_80123FC0;
extern u8 *D_80123FC4;

/**
 * @brief Process the active field effect indices stored in the shared state block.
 */
void func_800BF3D8(void)
{
    s32 i;

    func_800BF514(0);
    if (D_80123FC4[0x2D] < 0xA0)
    {
        func_800BF2F0(((RecFC0 *)(D_80123FC0 + (D_80123FC4[0x2D] << 3)))->unk98A);
    }
    i = 4;
    do
    {
        if (*(D_80123FC4 + i + 0x28) < 0xA0)
        {
            func_800BF2F0(((RecFC0 *)(D_80123FC0 + (*(D_80123FC4 + i + 0x28) << 3)))->unk988);
        }
        i -= 1;
    } while (i >= 3);
    if (D_80123FC4[0x2A] < 0xA0)
    {
        func_800BF2F0(((RecFC0 *)(D_80123FC0 + (D_80123FC4[0x2A] << 3)))->unk986);
    }
    if (D_80123FC4[0x29] < 0xA0)
    {
        func_800BF2F0(((RecFC0 *)(D_80123FC0 + (D_80123FC4[0x29] << 3)))->unk984);
    }
}


extern u8 *D_80123FC4;

/**
 * @brief Recursively make room in the active field slot chain.
 * @param arg0 Slot index to inspect.
 * @return 0 when the current occupant must be retained, or -1 when the slot is free or shifted.
 */
s32 func_800BF514(s32 arg0)
{
    s32 next;

    if ((D_80123FC4 + arg0)[0x28] == 0xFF)
    {
        return -1;
    }

    if (arg0 == 4)
    {
        if ((u32)(D_80123FC4[0x2C] - 0x51) < 7 && *(s32 *)(D_80123FC4 + 0x58) >= 0)
        {
            return 0;
        }

        if ((u32)((D_80123FC4 + arg0)[0x28] - 0x3E) < 0xE)
        {
            if (((*(u32 *)(D_80123FC4 + 0x58) >> 30) & 1) != 0)
            {
                return 0;
            }
            next = arg0 + 1;
        }
        else
        {
            next = arg0 + 1;
        }
    }
    else
    {
        next = arg0 + 1;
    }

    if (func_800BF514(next) != 0)
    {
        (D_80123FC4 + next)[0x28] = (D_80123FC4 + arg0)[0x28];
        (D_80123FC4 + arg0)[0x28] = 0xFF;
        return -1;
    }

    if ((u32)((D_80123FC4 + arg0)[0x28] - 0x51) < 7 && *(s32 *)(D_80123FC4 + 0x58) >= 0)
    {
        return 0;
    }
    if ((u32)((D_80123FC4 + arg0)[0x28] - 0x3E) < 0xE && ((*(u32 *)(D_80123FC4 + 0x58) >> 30) & 1) != 0)
    {
        return 0;
    }

    return -1;
}


extern u8 *D_80123FC4;

s32 func_800BF9F0(s32 arg0);

/**
 * @brief Replaces a matching byte in one of the active field slots.
 *
 * @param arg0 Minimum active-slot state passed to func_800BF9F0.
 * @param arg1 Byte value to find in slots 4 through 2.
 * @param arg2 Replacement byte.
 * @return The replaced slot index, or 0xFF when no slot was replaced.
 */
s32 func_800BF68C(s32 arg0, s32 arg1, u8 arg2)
{
    s32 index;
    s32 match;
    u8 *entry;
    u8 value;

    match = arg1;
    value = arg2;

    if (func_800BF9F0(arg0) != 0)
    {
        index = 4;
        do
        {
            entry = D_80123FC4 + index;
            if (entry[0x28] == match)
            {
                entry[0x28] = value;
                return index;
            }
            index--;
        } while (index >= 2);
    }

    return 0xFF;
}


void func_800BF700(void)
{
    func_800BF8E0();
    func_800BF944();
    func_800BF730();
}


extern u8 *D_80123FC4;
extern s8 D_800F0C38[];
extern u8 D_800F0E88[];

/**
 * @brief Clamp eight slot low nibbles against the selected table bounds.
 */
void func_800BF730(void)
{
    s32 i;
    u8 *entry;
    u8 packed_value, result;
    s32 alternate_value;
    s32 first_score, second_score, pair_index, selected_value, upper_bound, low_nibble;
    s8 *score_table, *score_entry, *bounds_entry;

    for (i = 0; i < 8; i++)
    {
        score_table = D_800F0C38;
        entry = D_80123FC4 + i;
        do
        {
            packed_value = entry[0x20];
            alternate_value = entry[0x50];
        } while (0);

        low_nibble = packed_value & 0xF;
        score_entry = (s8 *)((s32)low_nibble + (s32)score_table);
        selected_value = low_nibble;
        do
        {
            score_entry = (s8 *)((s32)selected_value + (s32)score_table);
        } while (0);
        bounds_entry = (s8 *)((s32)alternate_value + (s32)score_table);

        first_score = *score_entry;
        second_score = *bounds_entry;
        if (first_score < 0)
        {
            first_score = -first_score;
        }
        if (second_score < 0)
        {
            second_score = -second_score;
        }

        first_score = first_score < second_score;
        if (first_score)
        {
            selected_value = alternate_value;
        }

        pair_index = (packed_value >> 4) * 2;
        bounds_entry = (s8 *)&D_800F0E88[pair_index];
        result = ((u8 *)bounds_entry)[0];
        if (!(selected_value < result))
        {
            upper_bound = ((u8 *)bounds_entry)[1];
            result = upper_bound;
            if (!(upper_bound < selected_value))
            {
                result = selected_value;
            }
        }

        entry[0x20] = (entry[0x20] & 0xF0) | (result & 0xF);
    }
}


/**
 * @brief Block at *D_80123FC4: a shared pool word at 0x8, eight two-byte
 *        entries starting at 0xC (value byte, then level or count byte),
 *        bitmask bytes at 0x34/0x35/0x4C/0x4D, a per-slot counter byte at
 *        0x44, and a flags word at 0x58.
 */
typedef struct
{
    u8 pad0[8];
    s32 unk8;    /* 0x08 shared pool */
    u8 padC[0x34 - 0xC];
    u8 unk34;    /* 0x34 */
    u8 unk35;    /* 0x35 */
    u8 pad36[0x44 - 0x36];
    u8 unk44;    /* 0x44 */
    u8 pad45[0x4C - 0x45];
    u8 unk4C;    /* 0x4C */
    u8 unk4D;    /* 0x4D */
    u8 pad4E[0x58 - 0x4E];
    s32 unk58;   /* 0x58 */
} StructFC4;

/** @brief Two-byte entry at offset 0xC + 2 * index, viewed from the block base. */
typedef struct FieldEntry
{
    u8 pad0[0xC];
    u8 value;
    u8 count;
} FieldEntry;

#define FC4_BYTES ((u8 *)D_80123FC4)

void func_800BF880(s32 arg0);



/**
 * @brief Drain a per-slot counter to zero across eight records.
 *
 * For each of the eight byte-strided records at @c D_80123FC4, repeatedly
 * dispatches the slot index to func_800BF880 and decrements the record's
 * @c unk44 counter until it reaches zero.
 *
 * @see decomp.me (100%) TODO
 */
void func_800BF800(void)
{
    s32 i;

    for (i = 0; i < 8; i++)
    {
        while (((StructFC4 *)(FC4_BYTES + i))->unk44 != 0)
        {
            func_800BF880(i);
            ((StructFC4 *)(FC4_BYTES + i))->unk44--;
        }
    }
}

/**
 * @brief Deducts a scaled resource cost from the shared pool for one record.
 *
 * The per-record entry lives at byte offset @p arg0 * 2 within the global
 * @c *D_80123FC4 block: a "cost" byte at 0xC (defaulting to 1 when zero) and a
 * "level" byte at 0xD. The cost is shifted left by the level; if the pool total
 * at @c unk8 can cover it and the level is still below 0xF, the shifted cost is
 * subtracted from the pool and the level is incremented.
 *
 * @param arg0 Record index; the entry byte offset is @p arg0 * 2.
 */
void func_800BF880(s32 arg0)
{
    StructFC4 *p;
    u8 *e;
    s32 amount;
    s32 count;

    amount = arg0 * 2;
    p = (StructFC4 *)D_80123FC4;
    e = (u8 *)p + amount;

    amount = 1;
    if (e[0xC] != 0)
    {
        amount = e[0xC];
    }

    count = p->unk8;
    amount = amount << e[0xD];

    if ((count >= amount) && (e[0xD] < 0xF))
    {
        p->unk8 = count - amount;
        e[0xD] = e[0xD] + 1;
    }
}

/**
 * @brief Set each bit of unk34 whose bit is set in unk4C and whose entry level byte is nonzero.
 */
void func_800BF8E0(void)
{
    s32 mask;
    s32 i;
    u8 *e;

    i = 0;
    mask = 1;
    do
    {
        if (mask & ((StructFC4 *)D_80123FC4)->unk4C)
        {
            e = FC4_BYTES + (i * 2);
            if (e[0xD] != 0)
            {
                ((StructFC4 *)D_80123FC4)->unk34 |= mask;
            }
        }
        i++;
        mask *= 2;
    } while (i < 8);
}

/**
 * @brief Rebuild unk35 from the bits set in unk4D.
 */
void func_800BF944(void)
{
    s32 mask;
    s32 i;

    i = 0;
    mask = 1;
    ((StructFC4 *)D_80123FC4)->unk35 = 0;
    do
    {
        if (((StructFC4 *)D_80123FC4)->unk4D & mask)
        {
            ((StructFC4 *)D_80123FC4)->unk35 |= mask;
        }
        i++;
        mask *= 2;
    } while (i < 8);
}

/**
 * @brief Decrements an entry counter and adds its weighted value to the
 *        shared pool.
 *
 * @param index Index of the two-byte field entry.
 */
void func_800BF9A0(s32 index)
{
    s32 offset;
    u8 count;
    FieldEntry *entry;
    FieldEntry *entry2;

    offset = index * 2;
    entry = (FieldEntry *)(FC4_BYTES + offset);
    count = entry->count;
    if (count != 0)
    {
        entry->count = count - 1;
        entry2 = (FieldEntry *)(FC4_BYTES + offset);
        ((StructFC4 *)D_80123FC4)->unk8 += entry2->value << entry2->count;
    }
}

/**
 * @brief Test whether the shared pool can cover arg0, treating it as 0 when the low nibble of unk58 is clear.
 * @param arg0 Amount to test against the pool.
 * @return 0 when the pool is smaller than the amount, else -1.
 */
s32 func_800BF9F0(s32 arg0)
{
    StructFC4 *p;

    p = (StructFC4 *)D_80123FC4;
    if ((p->unk58 & 0xF) == 0)
    {
        arg0 = 0;
    }
    if (p->unk8 < arg0)
    {
        return 0;
    }
    return -1;
}
