/** @file field_actor_displacement.c
 * @brief Apply actor displacement, follow leader history, and refresh collision contact.
 */

/* func_80094508 */
#include "common.h"

/** @brief Partial actor state used by the scaled movement update. */
typedef struct
{
    u8 pad0[0x16];
    s16 unk16;
    u8 pad18[0x2A - 0x18];
    s16 unk2A;
    u8 pad2C[0x2E - 0x2C];
    u16 unk2E;
    u8 pad30[0x36 - 0x30];
    u8 unk36;
    u8 pad37[0x3A - 0x37];
    u8 unk3A;
} FieldActorState;

/** @brief Actor-part scaling bytes in the 0x48-byte definition table. */
typedef struct
{
    u8 pad0[0x2E];
    u8 unk2E;
    u8 pad2F[0x33 - 0x2F];
    u8 unk33;
    u8 pad34[0x48 - 0x34];
} FieldActorPartDef;

extern FieldActorPartDef D_800FE3A0[];

void func_8008EBA4();
s32 func_80097FA0();

/**
 * @brief Update actor movement and clear selected states when the transform fails.
 * @param arg0 Actor state to update.
 * @param arg1 Horizontal scale input.
 * @param arg2 Vertical scale input.
 * @param arg3 Depth scale input.
 * @return Unspecified value; callers do not consume the result.
 */
s32 func_80094508(FieldActorState *arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 temp_lo;
    FieldActorPartDef *part;
    s32 *out;
    s16 state;

    out = (s32 *)0x1F800000;
    if (arg0->unk2E == 0)
    {
        arg0->unk2A = 0;
    }
    else
    {
        func_8008EBA4(arg0, arg1, arg3);
        temp_lo = (s8)arg0->unk36 / arg0->unk16;
        arg0->unk36 = (u8)arg0->unk36 - temp_lo;
        part = &D_800FE3A0[arg0->unk3A];
        out[0] = (temp_lo * arg1 * part->unk2E) >> 6;
        out[1] = (arg2 * part->unk33) >> 6;
        out[2] = (temp_lo * arg3 * part->unk2E) >> 6;
        if (func_80097FA0(arg0, out, 0) == 0)
        {
            state = arg0->unk2A;
            if (state == 0x8B || state == 0xAC || state == 0x8C || state == 0xB0 || state == 0xB1)
            {
                arg0->unk2A = 0;
            }
        }
    }
}


/* func_80094690 */
#include "common.h"
#include "vector.h"

/**
 * @brief Field record fields used by the scaled position query.
 */
typedef struct Record94690
{
    u8 pad0[0x20];
    u8 scale;
    u8 pad21[0x2A - 0x21];
    s16 value;
} Record94690;

s32 func_80097FA0();

/**
 * @brief Tests a scaled X/Z displacement for a field record.
 *
 * Builds a scratchpad vector from the supplied X/Z components and the byte
 * scale at record offset 0x20. A failed query clears the record halfword at
 * offset 0x2A.
 *
 * @param record Record supplying the scale and result halfword.
 * @param x X displacement before scaling.
 * @param z Z displacement before scaling.
 * @note 100% match. Reusing one scaled-value local for both products
 *       reproduces the target value web and register reuse.
 */
void func_80094690(Record94690 *record, s32 x, s32 z)
{
    Vec3i *vector;
    s32 scaled;

    scaled = x * record->scale;
    vector = (Vec3i *)0x1F800000;
    vector->y = 0;
    vector->x = scaled;
    scaled = z * record->scale;
    vector->z = scaled;
    if (func_80097FA0(record, vector, 0, scaled) == 0)
    {
        record->value = 0;
    }
}


/* func_800946FC */
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
void func_8008EBA4();
extern void func_80096334(FieldFollowRecord *);
s32 func_80097FA0();

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


/* func_800949CC */
#include "common.h"

typedef struct
{
    u8 pad0[0xC];
    s32 unkC;
    u8 pad10[0x18 - 0x10];
    s32 unk18;
} FieldTrackEntry;

typedef struct
{
    u8 pad0[0x24];
    u8 unk24;
    u8 pad25[0x23A - 0x25];
    u8 unk23A;
    u8 pad23B[0x244 - 0x23B];
} FieldTrackActor;

typedef struct
{
    u8 pad0[0x20];
    u8 scale;
    u8 pad21[0x2A - 0x21];
    s16 state;
    u8 pad2C[0x2E - 0x2C];
    u16 transform_mode;
    u8 pad30[0x3A - 0x30];
    u8 track;
} FieldActorRecord;

extern FieldTrackEntry D_80105880[];
extern FieldTrackActor g_field_actor_slots[];

s32 func_80097FA0();

/**
 * @brief Validate an actor's active track state or submit a scaled scratchpad vector.
 * @param record Actor record to update.
 * @param x X component used by the transform path.
 * @param y Y component used by the transform path.
 * @param z Z component used by the transform path.
 */
void func_800949CC(FieldActorRecord *record, s32 x, s32 y, s32 z)
{
    s32 offset;
    s32 key;
    s32 selector;
    u8 *first_base;
    u8 *second_base;
    u8 *third_base;
    FieldTrackActor *actors;
    FieldTrackActor *actor;
    s32 *scratch;

    scratch = (s32 *)0x1F800000;
    if (record->transform_mode == 0)
    {
        first_base = (u8 *)D_80105880;
        if ((u8)record->track < 2U)
        {
            offset = record->track * 0x1C;
        }
        else
        {
            offset = 0x38;
        }
        selector = record->track;
        key = *(s32 *)(first_base + offset + 0xC);
        if (key == selector)
        {
            actors = g_field_actor_slots;
            second_base = (u8 *)D_80105880;
            if ((u32)(key & 0xFF) < 2U)
            {
                offset = key * 0x1C;
            }
            else
            {
                offset = 0x38;
            }
            actor = actors + *(s32 *)(second_base + offset + 0x18);
            if (actor->unk24 != 0)
            {
                actors = g_field_actor_slots;
                third_base = (u8 *)D_80105880;
                if ((u8)record->track < 2U)
                {
                    offset = record->track * 0x1C;
                }
                else
                {
                    offset = 0x38;
                }
                actor = actors + *(s32 *)(third_base + offset + 0x18);
                if (actor->unk23A == 0)
                {
                    record->state = 0;
                }
            }
            else
            {
                record->state = 0;
            }
        }
        else
        {
            record->state = 0;
        }
    }
    else
    {
        scratch[0] = x * record->scale;
        scratch[1] = y * record->scale;
        scratch[2] = z * record->scale;
        func_80097FA0(record, scratch, 0);
    }
}


/* func_80094B5C */
#include "common.h"

typedef struct
{
    u8 pad0[0x4];
    s32 unk4;  /* 0x04 */
    u8 pad8[0x20 - 0x8];
    u8 unk20;  /* 0x20 */
    u8 pad21[0x26 - 0x21];
    u8 unk26;  /* 0x26 */
    u8 pad27[0x2A - 0x27];
    s16 unk2A; /* 0x2A */
    u8 pad2C[0x30 - 0x2C];
} Struct80094B5C;

/**
 * @brief Advances an actor's countdown and scrolls its 0x04 offset field.
 *
 * Decrements the byte counter at @c unk26 and zeroes @c unk2A when it reaches
 * 0. Then, when @p flag is set, subtracts @c unk20 << 8 from @c unk4; otherwise
 * adds it, and if the sum is non-negative resets @c unk4 and @c unk2A to 0.
 *
 * @param a0 Actor record to update.
 * @param flag Nonzero subtracts the delta from @c unk4; zero adds it (with the
 *             non-negative reset).
 */
void func_80094B5C(Struct80094B5C *a0, s32 flag)
{
    s8 v;

    v = a0->unk26 - 1;
    a0->unk26 = v;
    if (v == 0)
    {
        a0->unk2A = 0;
    }

    if (flag != 0)
    {
        a0->unk4 -= a0->unk20 << 8;
    }
    else
    {
        s32 w = a0->unk4 + (a0->unk20 << 8);
        a0->unk4 = w;
        if (w >= 0)
        {
            a0->unk4 = 0;
            a0->unk2A = 0;
        }
    }
}


/* field40 */
#include "common.h"

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u8 pad0C[0x14];
    u8 unk20;
} UnkStruct21;

void func_80094BC4(UnkStruct21 *arg0, s32 arg1, s32 arg2)
{
    u8 temp_v0;

    temp_v0 = arg0->unk20;
    arg0->unk0 = arg0->unk0 + (arg1 * temp_v0);
    arg0->unk8 = arg0->unk8 + (arg2 * temp_v0);
}


/* func_80094C00 */
#include "common.h"

/** @brief Position, speed, and slot-index prefix of a field actor. */
typedef struct
{
    s32 x, y, z;
    u8 pad_c[0x14];
    u8 speed;
    u8 pad_21[0x19];
    u8 slot;
} FieldMovingActor;
/** @brief Visual kind byte in a 0x48-byte field object record. */
typedef struct
{
    u8 pad[0x2E];
    u8 kind;
    u8 tail[0x19];
} FieldObjectVisualKind;
/** @brief Collision result fields in a 0x23C-byte actor slot. */
typedef struct
{
    u8 pad[0x176];
    s16 height;
    u8 pad178[0x24];
    s32 contact, surface;
    u8 pad1a4[0x98];
} FieldActorCollisionResult;
/** @brief Scratchpad collision request and resolver output. */
typedef struct
{
    s32 x, y, z, dx, dy, dz, height, contact, surface;
    s16 radius, depth;
    union
    {
        s32 flags;
        struct
        {
            unsigned step : 16;
            unsigned bit16 : 1;
            unsigned bit17 : 1;
            unsigned high : 14;
        } bits;
    } mode;
} FieldActorCollisionMover;
/** @brief Map dimensions used to validate fixed-point actor coordinates. */
typedef struct
{
    s16 width;
    u16 height;
} FieldActorCollisionBounds;


extern s32 func_8005B6AC(FieldActorCollisionMover *);

/**
 * @brief Move an actor by its speed and refresh its collision contact and height.
 * @param actor Actor position, speed, and destination slot index.
 * @param dx Horizontal direction or displacement multiplier.
 * @param dz Depth direction or displacement multiplier.
 * @note Uses collision scratchpad memory at 0x1F800000 and map bounds at 0x801ED400.
 * @note 100% match with GCC 2.7.2 CDK: 169 instructions, 676 bytes.
 */
void func_80094C00(FieldMovingActor *actor, s32 dx, s32 dz)
{
    FieldActorCollisionBounds *bounds = (FieldActorCollisionBounds *)0x801ED400;
    FieldActorCollisionMover *mover = (FieldActorCollisionMover *)0x1F800000;
    s32 x, z;
    u8 speed;

    speed = actor->speed;
    actor->x += dx * speed;
    actor->z += dz * speed;
    x = actor->x;
    z = actor->z;
    if (x >= 0 && x < (bounds->width << 8) && z >= 0 &&
        z < ((s32)(bounds->height << 16) >> 7))
    {
        mover->x = x;
        mover->y = actor->y;
        mover->z = actor->z;
        mover->dx = 0;
        mover->dy = 0;
        mover->dz = 0;
        if (((FieldObjectVisualKind *)D_800FE3A0)[actor->slot].kind == 0x40)
        {
            mover->radius = 12;
            mover->mode.bits.step = 8;
        }
        else
        {
            mover->radius = 9;
            mover->mode.bits.step = 6;
        }
        mover->depth = 16;
        /* Separate bitfield clears preserve the two target mask operations. */
        mover->mode.bits.bit17 = 0;
        mover->mode.bits.bit16 = 0;
        mover->contact = ((FieldActorCollisionResult *)D_80105AE0)[actor->slot].contact;
        mover->surface = ((FieldActorCollisionResult *)D_80105AE0)[actor->slot].surface;
        func_8005B6AC(mover);
        ((FieldActorCollisionResult *)D_80105AE0)[actor->slot].contact = mover->contact;
        ((FieldActorCollisionResult *)D_80105AE0)[actor->slot].surface = mover->surface;
        ((FieldActorCollisionResult *)D_80105AE0)[actor->slot].height = mover->height / 256;
    }
    else
    {
        ((FieldActorCollisionResult *)D_80105AE0)[actor->slot].contact = -1;
        ((FieldActorCollisionResult *)D_80105AE0)[actor->slot].surface = 0;
        ((FieldActorCollisionResult *)D_80105AE0)[actor->slot].height = 0;
    }
}
