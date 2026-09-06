#include "common.h"

/** @brief Per-actor slot in D_80105AE0; stride 0x23C. */
typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x224];
} SlotA;

/** @brief Per-actor record in D_800FDF58; stride 0x54. */
typedef struct
{
    u8 pad0[0x3A];
    u8 unk3A;
    u8 pad3B[0x19];
} EntryB;

/** @brief Record in the D_800FD818 table indexed by EntryB::unk3A; stride 0x268. */
typedef struct
{
    u8 pad0[0x25E];
    s16 unk25E;
    s16 unk260;
    s16 unk262;
    s16 unk264;
    s16 unk266;
} RecFD818;

extern SlotA D_80105AE0[];
extern EntryB D_800FDF58[];
extern RecFD818 D_800FD818[];

/**
 * @brief Update the indexed field record associated with an actor lookup key.
 * @param arg0 Actor lookup key.
 * @param arg1 Value written to the record's first configurable field.
 * @param arg2 Value written to the record's second configurable field.
 * @param arg3 Value written to the record's third configurable field.
 * @param arg4 Value written to the record's fourth configurable field.
 * @return Zero on success, or -1 when the actor or target record is unavailable.
 */
s32 func_80089BE8(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4)
{
    EntryB *scan;
    EntryB *found;
    SlotA *e;
    s32 i;

    scan = D_800FDF58;
    e = D_80105AE0;
    i = 0;
loop:
    i++;
    if (e->unk14 == arg0)
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
    if (found == (EntryB *)-1)
    {
        return -1;
    }
    goto success;

found_label:
    found = scan;
    goto check;

success:
    if (found->unk3A >= 3)
    {
        return -1;
    }
    D_800FD818[found->unk3A].unk260 = arg4;
    D_800FD818[found->unk3A].unk25E = 0;
    D_800FD818[found->unk3A].unk262 = arg1;
    D_800FD818[found->unk3A].unk264 = arg2;
    D_800FD818[found->unk3A].unk266 = arg3;
    return 0;
}
