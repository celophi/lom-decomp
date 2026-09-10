#include "common.h"
#include "field_types.h"
#include "sdk/inline_c.h"

/* Apply the matching GTE instruction encodings after the SDK macros. */
#include "sdk/gte_dmpsx_compat.h"

/** @brief Partial field record used by the history-following update. */
typedef struct
{
    s32 x, y, z;
    u8 padc[0x21 - 0xC];
    u8 state;
    u8 pad22[0x2A - 0x22];
    s16 unk2a;
    u8 pad2c[2];
    s16 unk2e;
    u8 pad30[3];
    u8 unk33;
    u8 pad34[6];
    u8 slot;
    u8 resource;
    u8 pad3c[0x54 - 0x3C];
} FieldFollowRecord;

/** @brief Slot history with packed X/Z points and the current follow index. */
typedef struct
{
    u8 pad0[0x6C];
    Vec2s points[48];
    u8 pad12c[0x16E - 0x12C];
    u8 history_index;
    u8 pad16f[0x23C - 0x16F];
} FieldFollowSlot;

/** @brief Partial resource descriptor exposing its behavior flags. */
typedef struct
{
    u8 pad0[0x10];
    u32 flags;
} FieldFollowResource;

extern FieldFollowRecord D_800FDF58[];
extern FieldFollowSlot D_80105AE0[];
extern FieldFollowResource g_field_resource_entries[];
extern void func_8008EBA4(FieldFollowRecord *, s32, s32);
extern void func_80096334(FieldFollowRecord *);
extern s32 func_80097FA0(FieldFollowRecord *, FieldVector *, s32);

/**
 * @brief Follow stored leader positions or restore the record's idle behavior.
 * @param record Follower record whose movement and history index are updated.
 * @note Packed history addressing and scratchpad GTE operations preserve codegen.
 */
void func_800946FC(FieldFollowRecord *record)
{
    FieldVector *delta = (FieldVector *)0x1F800010;
    FieldVector *squares = (FieldVector *)0x1F800000;
    FieldFollowSlot *slots;
    FieldFollowSlot *slot;
    s32 distance;
    u8 index;
    u8 next;
    s32 state;

    delta->vy = 0;
    delta->vx = (D_800FDF58[0].x - record->x) / 256;
    delta->vz = (D_800FDF58[0].z - record->z) / 256;
    gte_ldlvl(delta);
    gte_sqr0();
    gte_stlvnl(squares);
    index = record->slot;
    distance = squares->vx + squares->vz;
    if (((index + 1) * 2000 < distance) &&
        (slots = D_80105AE0, slot = &slots[index], next = slot->history_index, next < 47))
    {
        slot->history_index = next + 1;
        /* Fold the slot and point indices together before the four-byte stride. */
        delta->vx =
            (*(s16 *)((u8 *)slots +
                      (D_800FDF58[0].slot * 0x8F + slots[record->slot].history_index) * 4 + 0x6C)
             << 8) -
            record->x;
        delta->vz =
            (*(s16 *)((u8 *)slots +
                      (D_800FDF58[0].slot * 0x8F + slots[record->slot].history_index) * 4 + 0x6E)
             << 8) -
            record->z;
        gte_ldlvl(delta);
        gte_sqr12();
        gte_stlvnl(squares);
        if (squares->vx + squares->vz > 384 &&
            !(g_field_resource_entries[record->resource].flags & 1))
        {
            record->unk33 = 1;
        }
        else
        {
            record->unk33 = 0;
        }
    }
    else
    {
        if (g_field_resource_entries[record->resource].flags & 1)
        {
            record->state &= 0x80;
        }
        else
        {
            state = record->state & 0x80;
            state += 2;
            record->state = state;
        }
        record->unk2e = 1;
        func_80096334(record);
        record->unk33 = 0;
        goto clear_state;
    }
    func_8008EBA4(record, delta->vx, delta->vz);
    if (func_80097FA0(record, delta, 0) == 0)
    {
        record->unk33 = 0;
    clear_state:
        record->unk2a = 0;
    }
}
