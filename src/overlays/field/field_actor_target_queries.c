#include "common.h"
#include "field_types.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

#define ACCESS(type, base, offset) (*(type *)((u8 *)(base) + (offset)))

typedef struct
{
    s32 x;
    u8 pad4[4];
    s32 z;
    u8 padC[0x21 - 0xC];
    u8 flags;
} FieldAngleRecord;

typedef struct
{
    s32 unk0;  /* 0x00 */
    u8 pad4[0x8 - 0x4];
    s32 unk8;  /* 0x08 */
    u8 padC[0x25 - 0xC];
    u8 unk25;  /* 0x25 */
    u8 pad26[0x30 - 0x26];
} Struct8009CF1C;

typedef struct
{
    s32 vx;
    s32 vy;
    s32 vz;
    u8 unk0C[0x2E];
    u8 unk3A;
} RefEntity;

/** @brief Selection descriptor containing the callback table index. */
typedef struct
{
    u8 pad[2];
    u8 filter;
} FilterSpec;
/** @brief Predicate for a source actor, candidate actor and caller argument. */
typedef s32 (*ActorFilter)(u8 *, u8 *, s32);

/** @brief Fields consulted in a 0x23C-byte FIELD actor state. */
typedef struct
{
    u8 pad0[4];
    s32 active;
    u8 pad8[4];
    s32 flags_c;
    s32 category;
    u8 pad14[0x12C - 0x14];
    s32 valid;
    u8 pad130[0x174 - 0x130];
    s32 flags174;
    s32 flags178;
    u8 pad17c[0x23C - 0x17C];
} FieldState;
/** @brief Binding state and its owning actor in a 0x1C-byte record. */
typedef struct
{
    s32 state;
    u8 pad4[8];
    s32 owner;
    u8 pad10[12];
} Binding;

extern u8 D_80105C70[];
extern ActorFilter D_800EC2D8[];
extern u8 D_800FDF58[];
extern s32 D_800FE754;
extern Binding D_80105880[];
extern FieldState D_80105AE0[];
extern s32 D_8010D020;

/**
 * @brief Test whether entity @p b is within @p max_dist of position @p a.
 * @param a Reference position (vx/vy/vz).
 * @param b Target entity; skipped when its unk25 flag is 0xFF.
 * @param max_dist Maximum distance for a positive result.
 * @return 1 if the GTE-computed distance is below @p max_dist, else 0.
 */
s32 func_8009CC60(FieldVector *a, FieldEntity *b, s32 max_dist)
{
    FieldVector *delta = (FieldVector *)0x1F800000;
    FieldVector *sqr = (FieldVector *)0x1F800010;
    s32 dist;

    if (b->unk25 != 0xFF)
    {
        delta->vx = (b->vx - a->vx) >> 8;
        delta->vy = (b->vy - a->vy) >> 8;
        delta->vz = (b->vz - a->vz) >> 8;
        gte_ldlvl(delta);
        gte_sqr0();
        gte_stlvnl(sqr);
        dist = SquareRoot0(sqr->vx + sqr->vy + sqr->vz);
        if (dist < max_dist)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Test whether entity @p b sits in the annulus around position @p a.
 * @param a Reference position (vx/vy/vz).
 * @param b Target entity; skipped when its unk25 flag is 0xFF.
 * @param max_dist Outer band is @p max_dist + 0x40; inner band is 0x40.
 * @return 1 if 0x40 < distance < @p max_dist + 0x40, else 0.
 */
s32 func_8009CD30(FieldVector *a, FieldEntity *b, s32 max_dist)
{
    FieldVector *delta = (FieldVector *)0x1F800000;
    FieldVector *sqr = (FieldVector *)0x1F800010;
    s32 dist;

    if (b->unk25 != 0xFF)
    {
        delta->vx = (b->vx - a->vx) >> 8;
        delta->vy = (b->vy - a->vy) >> 8;
        delta->vz = (b->vz - a->vz) >> 8;
        gte_ldlvl(delta);
        gte_sqr0();
        gte_stlvnl(sqr);
        dist = SquareRoot0(sqr->vx + sqr->vy + sqr->vz);
        if (dist < max_dist + 0x40)
        {
            if (dist > 0x40)
            {
                return 1;
            }
        }
    }
    return 0;
}

/**
 * @brief Test whether one field record faces another within an angular limit.
 * @param a Record whose facing direction is tested.
 * @param b Record used as the facing target.
 * @param distance Distance value used to derive the angular limit.
 * @return 1 when the target is within the angular limit, otherwise 0.
 */
s32 func_8009CE10(FieldAngleRecord *a, FieldAngleRecord *b, s32 distance)
{
    s32 limit;
    s32 angle;

    limit = ratan2(distance, 100);
    if (!(a->flags & 0x80))
    {
        angle = ratan2((a->z - b->z) >> 8, (a->x - b->x) >> 8);
        if (angle >= 0x801)
        {
            angle = 0x1000 - angle;
        }
        if (angle < limit && -limit < angle)
        {
            return 1;
        }
    }
    else
    {
        angle = ratan2((b->z - a->z) >> 8, (b->x - a->x) >> 8);
        if (angle >= 0x801)
        {
            angle = 0x1000 - angle;
        }
        if (angle < limit && -limit < angle)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Tests whether entity @p a1 lies within a bounding box around @p a0.
 *
 * Returns false when @p a1 is inactive (@c unk25 == 0xFF), when the y delta
 * (@c a1->unk8 - @c a0->unk8) is outside [-0x2000, 0x2000], or when the x delta
 * (@c a1->unk0 - @c a0->unk0) is outside [-(t << 8), t << 8]; otherwise true.
 *
 * @param a0 Reference entity (box center).
 * @param a1 Candidate entity to test.
 * @param t Half-width of the x range; scaled by 256.
 * @return 1 when @p a1 is inside the box, 0 otherwise.
 */
s32 func_8009CF1C(Struct8009CF1C *a0, Struct8009CF1C *a1, s32 t)
{
    s32 dy;
    s32 dx;

    if (a1->unk25 == 0xFF)
    {
        return 0;
    }
    dy = a1->unk8 - a0->unk8;
    if (dy >= 0x2001)
    {
        return 0;
    }
    if (dy < -0x2000)
    {
        return 0;
    }
    dx = a1->unk0;
    dx = dx - a0->unk0;
    t <<= 8;
    dy = dx;
    if (t < dy)
    {
        return 0;
    }
    return dy >= -t;
}

/**
 * @brief Distance test against three consecutive correction anchors.
 *
 * Loops the corrected GTE distance (D_80105C70[a->unk3A * 0x23C], stepping the
 * s16 correction pair by one each pass) up to three times, returning 1 on the
 * first anchor within @p max_dist.
 *
 * @return 1 if any anchor is within @p max_dist, else 0 (also 0 when unk25 == 0xFF).
 */
s32 func_8009CF84(RefEntity *a, FieldEntity *b, s32 max_dist)
{
    FieldVector *delta = (FieldVector *)0x1F800000;
    FieldVector *sqr = (FieldVector *)0x1F800010;
    s16 *corr;
    s32 i;
    s32 dist;
    static void *const keep[] __attribute__((section(".discard"))) = {
        &&success,
    };

    if (b->unk25 == 0xFF)
    {
        return 0;
    }
    corr = (s16 *)&D_80105C70[a->unk3A * 0x23C];
    i = 0;
    do
    {
        delta->vx = ((b->vx - a->vx) - (corr[0] << 8)) >> 8;
        delta->vy = (b->vy - a->vy) >> 8;
        delta->vz = ((b->vz - a->vz) - (corr[1] << 8)) >> 8;
        gte_ldlvl(delta);
        gte_sqr0();
        gte_stlvnl(sqr);
        dist = SquareRoot0(sqr->vx + sqr->vy + sqr->vz);
        if (dist < max_dist)
        {
success:
            return 1;
        }
        i++;
        corr += 2;
    } while (i < 3);
    return 0;
}

/**
 * @brief Distance test with a per-index x/z correction from D_80105C70.
 * @param a Reference entity; unk3A selects a 0x23C-stride correction record.
 * @param b Target entity; skipped when its unk25 flag is 0xFF.
 * @param max_dist Maximum corrected distance for a positive result.
 * @return 1 if the corrected GTE distance is below @p max_dist, else 0.
 */
s32 func_8009D0D8(RefEntity *a, FieldEntity *b, s32 max_dist)
{
    FieldVector *delta = (FieldVector *)0x1F800000;
    FieldVector *sqr = (FieldVector *)0x1F800010;
    s16 *corr;
    s32 dist;

    corr = (s16 *)&D_80105C70[a->unk3A * 0x23C];
    if (b->unk25 != 0xFF)
    {
        delta->vx = ((b->vx - a->vx) - (corr[0] << 8)) >> 8;
        delta->vy = (b->vy - a->vy) >> 8;
        delta->vz = ((b->vz - a->vz) - (corr[1] << 8)) >> 8;
        gte_ldlvl(delta);
        gte_sqr0();
        gte_stlvnl(sqr);
        dist = SquareRoot0(sqr->vx + sqr->vy + sqr->vz);
        if (dist < max_dist)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Collect eligible FIELD actor indices using a selected predicate.
 * @param source_index Actor excluded from the candidate list.
 * @param spec Descriptor selecting a predicate from D_800EC2D8.
 * @param group_mode Zero selects the opposite group; one selects the same group.
 * @param filter_arg Additional predicate argument.
 * @param output Destination for the accepted indices.
 * @return Number of indices written to output.
 */
s32 func_8009D1E4(s32 source_index, FilterSpec *spec, s32 group_mode, s32 filter_arg, s32 *output)
{
    enum
    {
        STATE_ACTIVE_WORD = -93,
        STATE_FLAGS_C_WORD = -91,
        STATE_CATEGORY_WORD = -90,
        STATE_VALID_WORD = -19,
        STATE_FLAGS174_WORD = -1
    };
    s16 actor_state;
    s32 *output_cursor;
    s32 flags_or_offset;
    s32 index;
    s32 count;
    s32 end;
    s32 start;
    s32 owner_slot;
    s32 state_slot;
    s32 prior;
    u8 *actor_start;
    volatile s32 *state_fields;
    FieldState *state_start;
    u8 *actor_state_ptr;
    u8 *actor;
    Binding *bindings;

    if (D_8010D020 != 0)
    {
        start = 0;
        end = 13;
    }
    else
    {
        switch (group_mode)
        {
        case 0:
            if (source_index < 3)
            {
                start = 3;
                end = 13;
            }
            else
            {
                start = 0;
                end = 3;
            }
            break;
        case 1:
            if (source_index < 3)
            {
                start = 0;
                end = 3;
            }
            else
            {
                start = 3;
                end = 13;
            }
            break;
        }
    }
    count = 0;
    index = start;
    actor_start = (index * 0x54) + D_800FDF58;
    state_start = &D_80105AE0[index];
    if (index < end)
    {
        bindings = D_80105880;
        state_fields = &state_start->flags178;
        actor_state_ptr = actor_start + 0x2A;
        actor = actor_start;
        output_cursor = output;
        do
        {
            if ((index != source_index) && (ACCESS(u8, actor_state_ptr, -5) != 0xFF) &&
                (state_fields[STATE_ACTIVE_WORD] != 0))
            {
                flags_or_offset = *((s32 *)state_fields);
                if (!(flags_or_offset & 1) &&
                    ((D_800FE754 == (state_fields[STATE_CATEGORY_WORD] & 0xF)) || (index < 3)) &&
                    !(flags_or_offset & 0x20))
                {
                    actor_state = ACCESS(s16, actor_state_ptr, 0);
                    if ((actor_state != 0x91) && (actor_state != 0xAE) && (actor_state != 0x87))
                    {
                        if (!(flags_or_offset & 0x40))
                        {
                            flags_or_offset = index < 3;
                            owner_slot = index;
                            if (flags_or_offset == 0)
                            {
                                owner_slot = 2;
                            }
                            if (bindings[owner_slot].owner == index)
                            {
                                state_slot = index;
                                if (flags_or_offset == 0)
                                {
                                    state_slot = 2;
                                }
                                if (bindings[state_slot].state != 0)
                                {
                                    goto next_actor;
                                }
                            }
                        }
                        if (!(state_fields[STATE_FLAGS_C_WORD] & 0x2280) && (state_fields[STATE_VALID_WORD] != 0) &&
                            !(state_fields[STATE_FLAGS174_WORD] & 0x8000) && !(*((s32 *)state_fields) & 0x80))
                        {
                            u8 *actor_base;

                            actor_start = actor;
                            flags_or_offset = source_index;
                            flags_or_offset <<= 2;
                            flags_or_offset += source_index;
                            flags_or_offset <<= 2;
                            flags_or_offset += source_index;
                            flags_or_offset <<= 2;
                            actor_base = D_800FDF58;
                            if (D_800EC2D8[spec->filter](actor_base + flags_or_offset, actor_start,
                                                         filter_arg) != 0)
                            {
                                for (prior = 0; prior < count; prior++)
                                {
                                }
                                *output_cursor = index;
                                output_cursor++;
                                count += 1;
                            }
                        }
                    }
                }
            }
    next_actor:
            actor += 0x54;
            index += 1;
            actor_state_ptr += 0x54;
            state_fields += 0x8F;
        } while (index < end);
    }
    return count;
}
