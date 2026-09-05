#include "common.h"

/*
 * Actor slot lookups keyed by the word at 0x14 of D_80105AE0. The first 13
 * slots are scanned in parallel with the D_800FDF58 records, so a hit in one
 * array selects the same index in the other.
 */

/** @brief Per-actor slot in D_80105AE0; stride 0x23C. */
typedef struct
{
    u8 pad0[0x14];
    s32 unk14;   /* 0x14 lookup key */
    u8 pad18[0x224];
} SlotA;

/** @brief Per-actor record in D_800FDF58; stride 0x54. */
typedef struct
{
    u8 pad0[0x3A];
    u8 unk3A;    /* 0x3A object index / track selector */
    u8 pad3B[0x19];
} EntryB;

s32 func_800839F8(s32 arg0, s32 arg1, EntryB *arg2);
s32 func_80083EEC(u8 arg0, s32 arg1, s32 arg2);
void field_start_actor_animation(s32 arg0, s32 arg1, s32 arg2);

extern SlotA D_80105AE0[];
extern EntryB D_800FDF58[];
extern s32 D_80105880[];

/**
 * @brief Finds the track value associated with the actor slot matching @p key.
 *
 * Scans the first 13 actor slots in parallel with D_800FDF58. On a hit, the
 * record's track selector at 0x3A chooses one of the three 0x1C-byte entries
 * in D_80105880; selectors >= 2 clamp to the third entry.
 *
 * @param key Value compared against each slot's unk14.
 * @return The selected D_80105880 word, or -1 when no slot matches.
 */
s32 func_8008B398(s32 key)
{
    EntryB *scan;
    EntryB *found;
    SlotA *e;
    s32 i;
    s32 offset;
    s32 result;
    u8 *base;

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
    found = (EntryB *)-1;
check:
    if (found != (EntryB *)-1)
    {
        goto lookup;
    }
    result = -1;
    goto done;
found_label:
    found = scan;
    goto check;
lookup:
    base = (u8 *)D_80105880;
    if (found->unk3A < 2)
    {
        offset = found->unk3A * 0x1C;
    }
    else
    {
        offset = 0x38;
    }
    result = *(s32 *)(base + offset);
done:
    return result;
}

/**
 * @brief Start an animation on the actor whose slot key matches, resolving it through the record's own index.
 * @param arg0 Slot key compared against unk14.
 * @param arg1 Forwarded to func_80083EEC as its third argument.
 * @return -1 when no slot matches, 0 when the animation started, 1 otherwise.
 */
s32 func_8008B42C(s32 arg0, s32 arg1)
{
    SlotA *ra;
    EntryB *rb;
    EntryB *found;
    s32 i;
    s32 anim;

    rb = D_800FDF58;
    ra = D_80105AE0;
    for (i = 0; i < 0xD; i++, ra++, rb++)
    {
        if (ra->unk14 == arg0)
        {
            goto found_it;
        }
    }
    found = (EntryB *) -1;
check:
    if (found != (EntryB *) -1)
    {
        goto body;
    }
    return -1;
found_it:
    found = rb;
    goto check;
body:
    anim = func_800839F8(found->unk3A, 0, rb);
    if ((anim != -1) && (func_80083EEC(found->unk3A, anim, arg1) != 0))
    {
        field_start_actor_animation(anim, 0, 0);
        return 0;
    }
    return 1;
}

/**
 * @brief Start an animation on the actor whose slot key matches, resolving it with index 0.
 * @param arg0 Slot key compared against unk14.
 * @param arg1 Forwarded to func_80083EEC as its third argument.
 * @return -1 when no slot matches, 0 when the animation started, 1 otherwise.
 */
s32 func_8008B500(s32 arg0, s32 arg1)
{
    SlotA *ra;
    EntryB *rb;
    EntryB *found;
    s32 i;
    s32 anim;

    rb = D_800FDF58;
    ra = D_80105AE0;
    for (i = 0; i < 0xD; i++, ra++, rb++)
    {
        if (ra->unk14 == arg0)
        {
            goto found_it;
        }
    }
    found = (EntryB *) -1;
check:
    if (found != (EntryB *) -1)
    {
        goto body;
    }
    return -1;
found_it:
    found = rb;
    goto check;
body:
    anim = func_800839F8(0, 0, rb);
    if ((anim != -1) && (func_80083EEC(found->unk3A, anim, arg1) != 0))
    {
        field_start_actor_animation(anim, 0, 0);
        return 0;
    }
    return 1;
}
