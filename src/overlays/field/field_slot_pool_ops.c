#include "common.h"

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

extern StructFC4 *D_80123FC4;

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
    p = D_80123FC4;
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
        if (mask & D_80123FC4->unk4C)
        {
            e = FC4_BYTES + (i * 2);
            if (e[0xD] != 0)
            {
                D_80123FC4->unk34 |= mask;
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
    D_80123FC4->unk35 = 0;
    do
    {
        if (D_80123FC4->unk4D & mask)
        {
            D_80123FC4->unk35 |= mask;
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
        D_80123FC4->unk8 += entry2->value << entry2->count;
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

    p = D_80123FC4;
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
