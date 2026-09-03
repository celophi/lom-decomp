#include "common.h"

/** @brief Per-actor animation/geometry slot; array element stride 0x23C. */
typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x23C - 0x18];
} FieldActorSlot;

/** @brief Parallel per-actor record; array element stride 0x54. */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u8 pad0C[0x28 - 0x0C];
    u8 unk28;
    u8 pad29;
    s16 unk2A;
    s16 unk2C;
    u8 pad2E[0x54 - 0x2E];
} FieldActorRecord;

extern FieldActorSlot D_80105AE0[];
extern FieldActorRecord D_800FDF58[];

s32 func_80087CE0(s32 key, u8 value)
{
    FieldActorRecord *scan;
    FieldActorRecord *found;
    FieldActorSlot *e;
    s32 i;
    s32 result;
    s16 state;

    scan = D_800FDF58;
    e = D_80105AE0;
    i = 0;
loop:
    i++;
    if (e->unk14 == key)
        goto found_label;
    e++;
    scan++;
    if (i < 13)
        goto loop;
    found = (FieldActorRecord *)-1;
check:
    if (found != (FieldActorRecord *)-1)
        goto body;
    result = -1;
    goto done;
found_label:
    found = scan;
    goto check;
body:
    state = found->unk2A;
    if ((u16)(state - 0x93) < 2)
    {
        result = -1;
        goto done;
    }
    if (state == 0x90 || state == 0xAE || state == 0x8E)
    {
        result = -1;
        goto done;
    }
    found->unk28 = value;
    found->unk2C = 0;
    found->unk2A = 0;
    result = 0;
done:
    return result;
}

/**
 * @brief Stores a scaled position into the actor record matching @p key.
 *
 * Scans the first 13 D_80105AE0 slots for one whose 0x14 field equals @p key.
 * On a hit, writes @p x, @p y and @p z (each shifted left 8) into the parallel
 * D_800FDF58 record's first three words and returns 0; otherwise returns -1.
 */
s32 func_80087D8C(s32 key, s32 x, s32 y, s32 z)
{
    FieldActorRecord *p = D_800FDF58;
    FieldActorSlot *e = D_80105AE0;
    FieldActorRecord *result;
    s32 i;

    i = 0;
    while (i < 13)
    {
        if (e->unk14 == key)
        {
            result = p;
            goto found;
        }
        i++;
        e++;
        p++;
    }
    result = (FieldActorRecord *)-1;
found:
    if (result == (FieldActorRecord *)-1)
    {
        return -1;
    }
    result->unk0 = x << 8;
    result->unk4 = y << 8;
    result->unk8 = z << 8;
    return 0;
}
