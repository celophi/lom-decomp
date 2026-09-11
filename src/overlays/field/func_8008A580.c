#include "common.h"

/** @brief Per-actor animation/geometry slot; array element stride 0x23C. */
typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x3C - 0x18];
    u32 unk3C;
    u8 pad40[0x23C - 0x40];
} FieldActorSlot;

/** @brief Parallel per-actor record; array element stride 0x54. */
typedef struct
{
    u8 pad0[0x3A];
    u8 unk3A;
    u8 pad3B[0x54 - 0x3B];
} FieldActorRecord;

extern FieldActorSlot D_80105AE0[];
extern FieldActorRecord D_800FDF58[];
extern s32 D_8010A020[];

s32 func_8008404C();

/**
 * @brief Find an actor by key and request its slot update.
 * @param key Actor-slot lookup key.
 * @param arg1 Value forwarded to the actor update helper; meaning unknown.
 * @return -1 when absent, 1 when the helper returns zero, or 0 after marking the slot.
 * @see decomp.me (100%) TODO
 */
s32 func_8008A580(s32 key, s32 arg1)
{
    FieldActorRecord *scan;
    FieldActorRecord *found;
    FieldActorSlot *e;
    s32 i;
    s32 *slot;
    s32 *table;

    scan = D_800FDF58;
    e = D_80105AE0;
    i = 0;
loop:
    i++;
    if (e->unk14 == key)
    {
        goto found_label;
    }
    e++;
    scan++;
    if (i < 13)
    {
        goto loop;
    }
    found = (FieldActorRecord *)-1;
check:
    if (found == (FieldActorRecord *)-1)
    {
        return (s32)found;
    }
    goto body;
found_label:
    found = scan;
    goto check;
body:
    if (func_8008404C(found->unk3A, arg1) != 0)
    {
        table = D_8010A020;
        if (found->unk3A < 2)
        {
            slot = &table[found->unk3A];
        }
        else
        {
            slot = table + 2;
        }
        *slot = 1;
        D_80105AE0[found->unk3A].unk3C = 0xFFFF;
    }
    else
    {
        return 1;
    }
    return 0;
}
