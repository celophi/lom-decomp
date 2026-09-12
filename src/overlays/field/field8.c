/**
 * @file field8.c
 * @brief Actor-owned effect initialization, lifetime handling, placement,
 * movement, target tracking, hit contacts, and reward collection.
 */

#include "common.h"
#include "field_effect_types.h"

typedef struct
{
    u8 pad0[0x10];
    s16 unk10; /* 0x10 */
    s16 unk12; /* 0x12 */
    s16 unk14; /* 0x14 */
} FieldTrackResult;

/** @brief Four-halfword GTE vector; the fourth halfword is padding. */
typedef struct
{
    s16 x;
    s16 y;
    s16 z;
    s16 pad;
} FieldSVector;

/**
 * @brief World-position footprint queried against field collision bounds.
 * @note Matches FieldCollisionQuery in field_collision.c: X/Z extents and Y tolerance.
 */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
    u16 width;
    s16 height_tolerance;
    u16 depth;
} FieldEffectCollisionQuery;

/**
 * @brief Position, step, and contact state resolved by func_8005B6AC.
 * @note The low halfword of mode is footprint depth, not a status code.
 * Bits 16/17 gate special resolution paths; their distinct meanings are unknown.
 */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
    s32 move_x;
    s32 move_y;
    s32 move_z;
    s32 resolved_height;
    void* collision_node;
    s32 contact_flags;
    u16 width;
    s16 height_tolerance;
    union
    {
        u32 word;
        struct
        {
            u32 depth : 16;
            u32 unknown_16 : 1;
            u32 unknown_17 : 1;
            u32 unknown_high : 14;
        } bits;
    } mode;
} FieldEffectCollisionMover;

/** @brief Per-slot 0x268-byte view of byte counters selected by reward kind. */
typedef struct
{
    u8 pad0[0x244];
    u8 counters[0x24];
} FieldCounterView;

/** @brief Selected map dimensions used for effect movement bounds. */
typedef struct
{
    s16 width;
    s16 height;
} FieldEffectMapBounds;

/** @brief Word-wide camera translation at 0x801ED480, also used by FieldCamera. */
typedef struct
{
    u8 pad0[4];
    s32 x;
    s32 y;
    s32 z;
} FieldEffectCamera;

#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

extern s32 g_field_track_index;
extern s32 D_800F22A0;
extern s32 D_800F22A4;
extern s32 D_800F22A8;
extern FieldActorState g_field_actor_slots[80];
extern FieldMotionRecord D_800FDF58[];
extern FieldObjectPlacement D_80105AE0[];
extern FieldVector D_80105778;
extern s32 D_80105760;
/**
 * @brief Packed action context: action in low bits, source at bit 8, recipient at bit 16.
 * @note Also written on pool exhaustion and advanced during collision attempts;
 * the broader protocol of those writes is still unresolved.
 */
extern s32 g_field_action_context;
void field_set_action_context(s32 recipient_id, s32 source_id, s32 action);
extern s32 D_800FE754;
/** @brief Suppress repeated pickup audio until the next frame-command build. */
extern s32 g_field_pickup_sound_played;
extern FieldCounterView D_800FD818[];

/**
 * @brief Roll one particle spawn record's scale and rotation fields from a
 *        part's parameter tracks, falling back to fixed part values or a
 *        random roll where a track is not assigned.
 * @param arg0 Owning actor, indexes g_field_track_index into unkCC/unk3B.
 * @param arg1 Part definition supplying the track selectors and fallback
 *             values (unk1E divisor, unk32 timing-table index, unk9/unkA
 *             track selectors).
 * @param arg2 Output record to fill in (unk10/unk12/unk14).
 * @return Scaled track value added to arg2->unk14.
 */
s32 func_80070CB8(FieldActorState *arg0, FieldActorPartDef *arg1, FieldTrackResult *arg2)
{
    s16 var_v0;
    s32 var_lo;
    s32 track_value;
    s32 temp_s0;

    arg2->unk10 = 0;
    if (arg1->unknown_0x1e != 0)
    {
        var_v0 = ((0x1000 / arg1->unknown_0x1e) * arg0->track_counters[g_field_track_index][arg1->unknown_0x32] + 0x400) & 0xFFF;
        arg2->unk12 = var_v0;
    }
    else
    {
        var_v0 = (u32) rand() >> 3;
        arg2->unk12 = var_v0;
    }
    arg0->track_counters[g_field_track_index][arg1->unknown_0x32]++;

    if ((arg1->track_flags >> 0xB) & 1)
    {
        temp_s0 = field_evaluate_parameter_track(arg0, arg1->unknown_0x9 & 0xF);
        var_lo = temp_s0 * (rand() << 3);
    }
    else
    {
        var_lo = arg1->unknown_0x9 * (rand() << 3);
    }
    arg2->unk14 = var_lo >> 0xF;

    if ((arg1->track_flags >> 0xC) & 1)
    {
        track_value = field_evaluate_parameter_track(arg0, arg1->unknown_0xa & 0xF);
    }
    else
    {
        track_value = arg1->unknown_0xa;
    }
    track_value *= 8;
    arg2->unk14 += track_value;
    return track_value;
}

/**
 * @brief Move an effect to its source and preserve its old position as the new source.
 * @param rec Record whose position and position-source selection are exchanged.
 * @param part Part definition controlling the selected position source.
 * @see decomp.me (100%) TODO
 */
void field_swap_effect_position_source(FieldMotionRecord *rec, FieldActorPartDef *part)
{
    FieldVector new_pos;
    s32 unused[2];

    if (rec->position_source != FIELD_POSITION_NONE)
    {
        field_resolve_effect_position(rec, part, &new_pos);
        rec->position_source = FIELD_POSITION_SAVED;
        rec->work_x = rec->x + D_800F22A0;
        rec->work_y = rec->y + D_800F22A4;
        rec->work_z = rec->z + D_800F22A8;
        rec->x = new_pos.vx;
        rec->y = new_pos.vy;
        rec->z = new_pos.vz;
    }
}

/**
 * @brief Face a record toward the delta between its last stored position and
 *        a freshly rolled position, deriving both a horizontal-plane heading
 *        (unk12) and a vertical pitch (unk14).
 * @param rec Record whose heading/pitch (unk12/unk14) are updated.
 * @param part Passed through unchanged to field_resolve_effect_position.
 * @see decomp.me (100%) TODO
 */
void func_80070EF0(FieldMotionRecord *rec, FieldActorPartDef *part)
{
    FieldVector new_pos;
    FieldVector vec;
    FieldVector sqr;

    field_resolve_effect_position(rec, part, &new_pos);
    vec.vx = (new_pos.vx - rec->x) >> 8;
    vec.vy = (new_pos.vy - rec->y) >> 8;
    vec.vz = (new_pos.vz - rec->z) >> 8;

    gte_ldlvl(&vec);
    gte_sqr0();
    gte_stlvnl(&sqr);

    rec->heading = ratan2(-vec.vz, vec.vx);
    if (rec->heading < 0)
    {
        rec->heading += 0x1000;
    }

    if (vec.vy != 0)
    {
        rec->pitch = ratan2(SquareRoot0(sqr.vx + sqr.vz), -vec.vy);
    }
    else
    {
        rec->pitch = 0x400;
    }
    if (rec->pitch < 0)
    {
        rec->pitch += 0x1000;
    }
    rec->rotation_x = 0;
}

typedef struct
{
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} RECT;

/** @brief Rotation member of successive 0x54-byte effect records, viewed from +0x10. */
typedef struct
{
    FieldSVector angles;
    u8 pad8[0x54 - 8];
} FieldEffectRotationEntry;

extern FieldMotionRecord D_800FF658[];
/** @brief Alias of D_800FF658 + 0x10; preserves the original rotation-base relocation. */
extern FieldEffectRotationEntry D_800FF668[];
extern u8 D_80104B58[];
extern u8 D_80105358[];

void field_update_effect_record(FieldMotionRecord *rec, FieldActorPartDef *part, FieldActorState *actor);

/**
 * @brief Per-frame actor tick: advances every active effect record owned by
 *        this actor (culling off-screen ones, spawning chained effects on
 *        expiry), then refreshes the actor's palette-track texture pages.
 * @param arg0 Actor being ticked.
 * @see decomp.me (80.28%) TODO
 * @note WIP - 80.28% (212/312 rows). Two residues, both investigated at
 *       length without a clean fix:
 *       (1) the D_800FF658[rec->previous_effect_index] "previous record" address is
 *       recomputed from scratch by the target FOUR separate times (once per
 *       field read/write) in straight-line code with no intervening call or
 *       branch; plain C (even with freshly-named index locals each time)
 *       always lets gcc 2.7.2's cse.c merge these into one computation. The
 *       `FieldMotionRecord * volatile pv` reassigned before each use forces
 *       the recompute (pv is `volatile`, but the round-trip through its own
 *       stack home does not exactly match the target's register-only
 *       rederivation) - this closed most but not all of the gap.
 *       (2) the D_800F22A0/A4/A8 sign-fixup chain (building sx/sy) has a
 *       handful of residual sched1/regalloc-order rows that did not respond
 *       to reassociating the sums or reordering the fixups.
 *       A fresh source-model attempt, or sched_oracle on the sign-fixup
 *       block, would be the next lever - not attempted this session.
 */
void func_8007100C(FieldActorState *arg0_param)
{
    FieldActorState *arg0;
    FieldMotionRecord *rec;
    FieldActorPartDef *part;
    void *buf;
    s32 i;
    s32 count;
    s32 newslot;
    FieldMotionRecord *newrec;
    s32 v0, v1, a0, v0_2, a0_2, v1_2;
    s16 sx, sy;
    u8 t;
    s32 dx, dy, dz;
    FieldMotionRecord * volatile pv;
    s32 unused_pad[2];
    s32 sp20, sp24, sp28;
    s32 out_vec[3];

    arg0 = arg0_param;
    for (rec = D_800FF658; rec != &D_800FF658[256]; rec++)
    {
        if (rec->actor_index == arg0->actor_index && rec->state != 0xFF)
        {
            part = &arg0->parts[rec->part_index];
            g_field_track_index = rec->track_index;
            field_update_effect_record(rec, part, arg0);
            rec->age++;
            if (((part->behavior_flags >> 4) & 3) == 1 && (u16) rec->age == rec->lifetime)
            {
                t = rec->state;
                rec->state = 0xFF;
                rec->saved_state = t;
            }
            if (((part->behavior_flags >> 4) & 3) == 3)
            {
                v0 = D_800F22A0;
                if (v0 < 0)
                {
                    v0 += 0xFF;
                }
                v1 = rec->x;
                if (v1 < 0)
                {
                    v1 += 0xFF;
                }
                a0 = D_800F22A4;
                sx = (v0 >> 8) + (v1 >> 8) + 0xA0;
                if (a0 < 0)
                {
                    a0 += 0xFF;
                }
                v0_2 = rec->y;
                if (v0_2 < 0)
                {
                    v0_2 += 0xFF;
                }
                a0_2 = rec->z;
                if (a0_2 < 0)
                {
                    a0_2 += 0x1FF;
                }
                v1_2 = D_800F22A8;
                if (v1_2 < 0)
                {
                    v1_2 += 0x1FF;
                }
                sy = ((a0 >> 8) + (v0_2 >> 8) + 0x70) - (a0_2 >> 9) - (v1_2 >> 9);
                if ((u32) ((sx + 0x140) & 0xFFFF) >= 0x3C1 || sy >= 0x1E1 || sy < -0xF0)
                {
                    t = rec->state;
                    rec->state = 0xFF;
                    rec->saved_state = t;
                }
            }
            if (rec->state == 0xFF)
            {
                func_80071500(rec, part);
            }
            if (part->effect_flags < 0 && rec->state != 0xFF)
            {
                newslot = func_8006D79C(arg0, part->unknown_0x23 & 0xF, 0);
                if (newslot != -1)
                {
                    newrec = &D_800FF658[newslot];
                    newrec->x = rec->x;
                    newrec->y = rec->y;
                    newrec->z = rec->z;
                    pv = &D_800FF658[rec->previous_effect_index];
                    dx = rec->x - pv->x;
                    sp20 = dx;
                    pv = &D_800FF658[rec->previous_effect_index];
                    dy = rec->y - pv->y;
                    sp24 = dy;
                    pv = &D_800FF658[rec->previous_effect_index];
                    dz = rec->z - pv->z;
                    sp28 = dz;
                    pv = &D_800FF658[rec->previous_effect_index];
                    pv->next_effect_index = newslot;
                    sp28 = 0;
                    sx = (s16) (dx >> 8);
                    v0_2 = dz >> 9;
                    sy = (s16) ((dy >> 8) - v0_2);
                    sp20 = sy;
                    sp24 = -(s32) sx;
                    func_8001CDAC(&sp20, out_vec, v0_2);
                    newrec->rotation_x = (s16) (out_vec[0] >> 6);
                    newrec->heading = (s16) (out_vec[1] >> 6);
                    newrec->work_x = 0;
                    newrec->work_y = 0;
                    newrec->pitch = (s16) (out_vec[2] >> 6);
                    newrec->next_effect_index = 0xFF;
                    newrec->previous_effect_index = rec->previous_effect_index;
                    rec->previous_effect_index = newslot;
                }
            }
        }
    }

    if (arg0->owner_object_index < 2)
    {
        buf = &D_80104B58[arg0->owner_object_index << 0xA];
    }
    else
    {
        buf = D_80105358;
    }
    count = 0;
    newslot = 0;
    if (arg0->part_count != 0)
    {
        i = 0;
        do
        {
            part = &arg0->parts[i];
            if ((part->track_flags >> 0x15) & 1)
            {
                newslot++;
                field_interpolate_palette_track(arg0, part->unknown_0x1c, buf, (u8 *) buf + 0x200);
            }
            count++;
            i++;
        } while (count < arg0->part_count);
    }
    if (newslot != 0)
    {
        RECT rect;

        if (arg0->owner_object_index < 2)
        {
            rect.x = 0;
            rect.w = 0x10;
            rect.h = 1;
            rect.y = (arg0->owner_object_index * 2) + 0x1EF;
        }
        else
        {
            rect.y = 0x1F3;
            rect.w = 0x10;
            rect.x = 0;
            rect.h = 1;
        }
        LoadImage(&rect, (u8 *) buf + 0x200);
    }
    func_8007FFC8(arg0);
    func_8008332C(arg0, arg0->parts, arg0->part_count);
}

/**
 * @brief Per-effect-record update: decrements the part's active-count table
 *        entry, dispatches an audio event for the owning actor, rolls a set
 *        of chained spawn effects selected by the part's flag/track fields,
 *        then syncs the owning actor's saved position/flag mirrors
 *        (D_800FDF58/D_80105AE0) when this record's part index matches one
 *        of the actor's per-slot animation-frame triggers.
 * @param rec Effect record being updated (owner/track/position fields).
 * @param part Part definition supplying flags, track selectors, and the
 *             chained-effect table used while rec->state is being armed.
 * @see decomp.me (97.78%) TODO
 * @note WIP - 97.78%. Structure, control flow, and every field/array
 *       mapping are confirmed exact; the entire remaining residue is a
 *       single register-coloring swap: gcc assigns rec to s4 and the
 *       derived `&g_field_actor_slots[rec->actor_index]` pointer to s3, where the
 *       target has them the other way around (s3 = rec, s4 = state). Both
 *       pseudos are close in gcc 2.7.2 global.c allocation priority
 *       (floor_log2(refs)*refs/live_len): rec came out at pri 2303 (41
 *       refs), state at pri 2388 (24 refs) - a ~3.7% gap - so state is
 *       allocated first and takes s3. The standard ALLOC-47 lever (a
 *       dedicated single-use constant local near the top, to shift live
 *       ranges) was tried with several constants/positions and always
 *       measured delta-exact 0 or negative; no C-level reshape found so far
 *       moves rec's priority above state's. A local `FieldObjectPlacement
 *       *slot` was introduced for the owner_object_index-indexed
 *       D_80105AE0 check (function-scope, block-local `slot =
 *       &D_80105AE0[state->owner_object_index];`) to stop the byte-view
 *       cast from folding the 0x17A offset into the array base constant;
 *       the same fix regresses the unk229[]-indexed sibling check three
 *       lines above it, so that one keeps the plain
 *       `((u8*)&D_80105AE0[...].state_flags)[2]` cast form instead - the last
 *       (3-row) structural residue lives there.
 */
void func_80071500(FieldMotionRecord *rec, FieldActorPartDef *part)
{
    FieldActorState *state;
    FieldActorPartDef *track_part;
    FieldMotionRecord *newrec;
    FieldObjectPlacement *slot;
    FieldVector new_pos;
    FieldVector vec;
    FieldVector sqr;
    s32 bit, mask;
    s32 newslot;
    s32 spawn_count;
    s16 anim_flags;
    u8 t;

    g_field_actor_slots[rec->actor_index].active_counts[rec->track_index][part->unknown_0x32]--;

    state = &g_field_actor_slots[rec->actor_index];
    field_dispatch_actor_audio_event(state, 3, rec->part_index);

    if (*(u32 *) &part->unknown_0x2c & 0xF0000000)
    {
        rec->state = (rec->height_or_retired_state == -1) ? 0xFE : (u8) rec->height_or_retired_state;

        for (bit = 0, mask = 1; bit < 4; bit++, mask <<= 1)
        {
            if ((*(u32 *) &part->unknown_0x2c >> 0x1C) & mask)
            {
                D_80105778.vx = rec->x;
                D_80105778.vy = rec->y;
                D_80105760 = 0;
                D_80105778.vz = rec->z;

                track_part = &g_field_actor_slots[rec->actor_index].parts[(part->spawn_flags.halves.part_selectors >> (bit * 4)) & 0xF];
                spawn_count = 1;
                if (((track_part->placement_flags >> 0x12) & 0x3F) == 0x35)
                {
                    if (track_part->unknown_0xc != 0)
                    {
                        spawn_count = track_part->unknown_0xc;
                    }
                }

                if (spawn_count != 0)
                {
                    do
                    {
                        newslot = func_8006D79C(&g_field_actor_slots[rec->actor_index], (part->spawn_flags.halves.part_selectors >> (bit * 4)) & 0xF, 0);
                        if (newslot != -1)
                        {
                            newrec = &D_800FF658[newslot];
                            if (!(((u8 *) &newrec->flags)[3] & 7) && (newrec->position_source != 0))
                            {
                                field_resolve_effect_position(newrec, part, &new_pos);
                                vec.vx = (new_pos.vx - newrec->x) >> 8;
                                vec.vy = (new_pos.vy - newrec->y) >> 8;
                                vec.vz = (new_pos.vz - newrec->z) >> 8;

                                gte_ldlvl(&vec);
                                gte_sqr0();
                                gte_stlvnl(&sqr);

                                newrec->heading = ratan2(-vec.vz, vec.vx);
                                if (newrec->heading < 0)
                                {
                                    newrec->heading += 0x1000;
                                }

                                if (vec.vy != 0)
                                {
                                    newrec->pitch = ratan2(SquareRoot0(sqr.vx + sqr.vz), -vec.vy);
                                }
                                else
                                {
                                    newrec->pitch = 0x400;
                                }
                                if (newrec->pitch < 0)
                                {
                                    newrec->pitch += 0x1000;
                                }
                                newrec->rotation_x = 0;
                            }
                        }
                        spawn_count--;
                    } while (spawn_count != 0);
                }
            }
        }
    }
    rec->state = 0xFF;

    if (((state->animation->sync_flags & 0x14) == 0x14) && ((state->animation->sync_parts >> 0xD) == rec->part_index))
    {
        if (state->track_object_indices[rec->track_index] != 0xFF)
        {
            if (((u8 *) &D_80105AE0[state->track_object_indices[rec->track_index]].state_flags)[2] == state->actor_index)
            {
                anim_flags = D_800FDF58[state->track_object_indices[rec->track_index]].motion_parameter;
                if ((anim_flags != 0x90 && anim_flags != 0x94) || (D_80105AE0[state->track_object_indices[rec->track_index]].object_flags & 0x200))
                {
                    D_800FDF58[state->track_object_indices[rec->track_index]].state = 0;
                }
                else
                {
                    D_800FDF58[state->track_object_indices[rec->track_index]].state = 0xFE;
                }
                D_80105AE0[state->track_object_indices[rec->track_index]].state_flags &= ~1;
            }
        }
    }

    if (((state->animation->sync_flags & 0xA) == 0xA) && (((state->animation->sync_parts >> 0xA) & 7) == rec->part_index))
    {
        slot = &D_80105AE0[state->owner_object_index];
        if (((u8 *) &slot->state_flags)[2] == state->actor_index)
        {
            anim_flags = D_800FDF58[state->owner_object_index].motion_parameter;
            if ((anim_flags != 0x90 && anim_flags != 0x94) || (slot->object_flags & 0x200))
            {
                D_800FDF58[state->owner_object_index].state = 0;
            }
            else
            {
                D_800FDF58[state->owner_object_index].state = 0xFE;
            }
            D_80105AE0[state->owner_object_index].state_flags &= ~1;
        }
    }

    if (rec->part_index == ((state->animation->sync_parts & 0x1F) - 1))
    {
        D_800FDF58[state->owner_object_index].x = rec->x;
        D_800FDF58[state->owner_object_index].y = rec->y;
        D_800FDF58[state->owner_object_index].z = rec->z;
        D_800FDF58[state->owner_object_index].y = 0;
        D_800FDF58[state->owner_object_index].facing_or_reward_kind =
            (D_800FDF58[state->owner_object_index].facing_or_reward_kind & 0x7F) | (rec->facing_or_reward_kind & 0x80);
    }

    if (rec->part_index == (((state->animation->sync_parts >> 5) & 0x1F) - 1))
    {
        if (state->track_object_indices[rec->track_index] != 0xFF)
        {
            D_800FDF58[state->track_object_indices[rec->track_index]].x = rec->x;
            D_800FDF58[state->track_object_indices[rec->track_index]].y = rec->y;
            D_800FDF58[state->track_object_indices[rec->track_index]].z = rec->z;
            D_800FDF58[state->track_object_indices[rec->track_index]].y = 0;
            D_800FDF58[state->track_object_indices[rec->track_index]].facing_or_reward_kind =
                (D_800FDF58[state->track_object_indices[rec->track_index]].facing_or_reward_kind & 0x7F) | (rec->facing_or_reward_kind & 0x80);
        }
    }

    if (rec->height_or_retired_state == 5)
    {
        t = rec->next_effect_index;
        if (t != 0xFF)
        {
            D_800FF658[t].previous_effect_index = 0xFF;
        }
    }
}

/**
 * @brief Placement selector within the part flags word.
 */
typedef struct
{
    unsigned int lower : 18;
    unsigned int opcode : 6;
    unsigned int upper : 8;
} FieldPlacementBits;

/** @brief High-halfword actor action kinds handled as collectible rewards. */
typedef enum
{
    FIELD_PICKUP_EXPERIENCE_OR_CURRENCY = 31,
    FIELD_PICKUP_ITEM = 32,
    FIELD_PICKUP_RESTORE_QUARTER = 33,
    FIELD_PICKUP_RESTORE_HALF = 34
} FieldPickupAction;

/** @brief Bit positions in the part's behavior_flags word (+0x04). */
#define FIELD_PART_PITCH_ACCELERATION_BIT 2
#define FIELD_PART_GROUND_BOUNCE_BIT 6

/** @brief Masks in the part's effect_flags word (+0x24). */
#define FIELD_PART_MAP_COLLISION 0x00100000

/** @brief Bit positions in the part's placement_flags word (+0x28). */
#define FIELD_PART_HEIGHT_TRACK_BIT 3

/** @brief Masks in the part's spawn_flags word (+0x34). */
#define FIELD_PART_HEIGHT_FROM_BASE 0x00080000
#define FIELD_PART_SKIP_MAP_COLLISION 0x00800000
#define FIELD_PART_CAMERA_BOUNDS 0x01000000
#define FIELD_PART_GROUND_STOP 0x02000000
#define FIELD_PART_RETIRE_ON_COLLISION 0x10000000

#define FIELD_PICKUP_DELAY 0x10U
#define FIELD_PICKUP_DISTANCE 5
#define FIELD_PICKUP_SOUND 0x1F
#define FIELD_EFFECT_MOTION_KIND_MASK 0x07000000
#define FIELD_EFFECT_MOTION_PATH 0x05000000
#define FIELD_EFFECT_MOTION_HOMING 0x01000000
#define FIELD_EFFECT_DISTANCE_MASK 0x1FF
#define FIELD_EFFECT_ORIENTATION_LOCK 8

/** @brief Angle units used by the field rotation helpers. */
#define FIELD_ANGLE_TURN 0x1000
#define FIELD_ANGLE_HALF_TURN 0x800
#define FIELD_ANGLE_QUARTER_TURN 0x400
#define FIELD_ANGLE_MASK 0xFFF
#define FIELD_EFFECT_TURN_THRESHOLD 0x200
#define FIELD_EFFECT_TURN_STEP 0x80
#define FIELD_EFFECT_COLLISION_WIDTH 0xC
#define FIELD_EFFECT_COLLISION_DEPTH 8
#define FIELD_EFFECT_COLLISION_HEIGHT 0x10
/** @brief Skip initial surface lookup/inheritance; ordinary movement collisions still run. */
#define FIELD_EFFECT_SKIP_SURFACE_PREPASS ((void *) -2)
#define FIELD_EFFECT_CAMERA_X_MIN 0x500
#define FIELD_EFFECT_CAMERA_X_MAX 0x13B00
#define FIELD_EFFECT_CAMERA_Z_SPAN 0x1E800
#define FIELD_EFFECT_MAP_BOUNDS_ADDRESS 0x801ED400
#define FIELD_EFFECT_CAMERA_ADDRESS 0x801ED480

/** @brief Scratchpad locations shared by the effect update's GTE/collision phases. */
#define FIELD_EFFECT_ORIGIN_ADDRESS 0x1F800000
#define FIELD_EFFECT_VECTOR_ADDRESS 0x1F800010
#define FIELD_EFFECT_TARGET_ADDRESS 0x1F800020
#define FIELD_EFFECT_LOCAL_VECTOR_ADDRESS 0x1F800030
#define FIELD_EFFECT_ROTATED_VECTOR_ADDRESS 0x1F800038
#define FIELD_EFFECT_MATRIX_ADDRESS 0x1F800040
#define FIELD_EFFECT_MOVER_ADDRESS 0x1F800080
#define FIELD_EFFECT_QUERY_ADDRESS 0x1F8000C0

/**
 * @brief Advance an actor-owned effect's placement, motion, pickups, and hit contacts.
 * @param rec Active effect record; positions use eight fractional bits.
 * @param part Packed definition selecting parameter tracks and placement behavior.
 * @param actor Owner supplying track/object bindings and animation action state.
 * @note Called once per active record by func_8007100C before it increments age.
 * Attached effects rebuild their world position; free effects integrate a rotated
 * step, resolve collision, and optionally steer toward a position source.
 * @note Some work scalars serve disjoint phases to preserve original allocation.
 * @see working/func_80071D40/target.s
 * @see docs/decompilation/func_80071D40-semantics.md
 * @note WIP - 99.988310% assembly match; placement registers remain.
 */
void field_update_effect_record(FieldMotionRecord *rec, FieldActorPartDef *part, FieldActorState *actor)
{
    FieldVector *target_position;
    FieldEffectCamera *camera;
    FieldEffectMapBounds *map_bounds;
    FieldEffectCollisionMover *mover;
    FieldEffectCollisionQuery *query;
    FieldVector *work_vector;
    FieldSVector *local_vector;
    FieldSVector *rotated_vector;
    FieldMatrix *rotation;
    FieldVector *placement_origin;
    FieldMotionRecord *reference_record;
    FieldObjectPlacement *reference_object;
    u32 flags;
    u32 record_flags;
    s32 selector;
    s32 x, y, z;
    s32 offset_or_angle;
    s32 current_angle;
    s32 dx, dy;
    void *initial_surface;
    s32 state_or_delta;
    s32 slot;
    s32 recipient_index;
    u8 reference_state;
    u8 retired_state;

    camera = (FieldEffectCamera *) FIELD_EFFECT_CAMERA_ADDRESS;
    map_bounds = (FieldEffectMapBounds *) FIELD_EFFECT_MAP_BOUNDS_ADDRESS;
    mover = (FieldEffectCollisionMover *) FIELD_EFFECT_MOVER_ADDRESS;
    query = (FieldEffectCollisionQuery *) FIELD_EFFECT_QUERY_ADDRESS;
    work_vector = (FieldVector *) FIELD_EFFECT_VECTOR_ADDRESS;
    target_position = (FieldVector *) FIELD_EFFECT_TARGET_ADDRESS;
    local_vector = (FieldSVector *) FIELD_EFFECT_LOCAL_VECTOR_ADDRESS;
    rotated_vector = (FieldSVector *) FIELD_EFFECT_ROTATED_VECTOR_ADDRESS;
    rotation = (FieldMatrix *) FIELD_EFFECT_MATRIX_ADDRESS;
    placement_origin = (FieldVector *) FIELD_EFFECT_ORIGIN_ADDRESS;

    /* Sample transparency and motion tracks; the caller advances age afterward. */
    flags = part->effect_flags;
    if (flags & FIELD_EFFECT_SEMITRANSPARENT)
    {
        s32 semitransparent = field_evaluate_parameter_track_at_time(actor, (flags >> 0x19) & 0xF, (u16) ((u16) rec->age)) != 0;
        rec->flags = (rec->flags & ~FIELD_EFFECT_SEMITRANSPARENT) | (semitransparent << 0x17);
    }
    if ((rec->flags & 0x60000000) == 0x40000000)
    {
        rec->motion_parameter = field_evaluate_parameter_track_at_time(actor, ((s16 *) &part->orientation_flags)[1] & 0xF, ((u16) rec->age));
    }

    /* Linked segment records update their projected work vector and return early. */
    if (rec->state == 5)
    {
        func_8007D078(rec, part, rotation, actor);
        gte_SetRotMatrix(rotation);
        {
            u16 segment_x = rec->heading;
            local_vector->y = 0;
            local_vector->x = segment_x;
        }
        local_vector->z = rec->rotation_x;
        gte_ldv0(local_vector);
        gte_rtv0();
        gte_stlvnl(work_vector);
        rec->work_x = work_vector->vx;
        rec->work_y = work_vector->vy;
        return;
    }

    /* Path mode advances a wrapping byte parameter and interpolates world X/Z. */
    record_flags = rec->flags;
    if ((record_flags & FIELD_EFFECT_MOTION_KIND_MASK) == FIELD_EFFECT_MOTION_PATH)
    {
        rec->position_data.path_time = rec->position_data.path_time + rec->motion_parameter;
        func_800A1D48(&rec->position_data.path_time, rec, rec->path_group);
    }

    /* Attached effects rebuild an origin, then add the rotated local displacement. */
    else if ((u32) ((record_flags >> 0x18) & 7) >= 2U)
    {
        if ((((u32) part->placement_flags >> 0x1A) & 3) == 2)
        {
            rec->rotation_z_16 = field_evaluate_parameter_track_at_time(actor, part->rotation_z_track & 0xF, ((u16) rec->age));
        }
        if ((((u32) part->placement_flags >> 0x1C) & 3) == 2)
        {
            rec->rotation_y_16 = field_evaluate_parameter_track_at_time(actor, part->rotation_y_track & 0xF, ((u16) rec->age));
        }
        if ((rec->flags & 0x600) == 0x400)
        {
            u32 selector_high = *(u32 *) &part->unknown_0x1c >> 29;
            local_vector->y = -field_evaluate_parameter_track_at_time(actor, ((((*(u32 *) &part->unknown_0x20) & 0x3F) * 8) | selector_high) & 0xF, (u16) rec->age);
        }
        else
        {
            local_vector->y = -((u16) rec->flags & FIELD_EFFECT_DISTANCE_MASK);
        }
        if ((*(u32 *) &part->unknown_0x1c) & 0x01000000)
        {
            local_vector->y = (s16) ((D_80105AE0[actor->owner_object_index].scale_percent & 0x3FF) * (s16) (u16) local_vector->y / 100);
        }
        {
            u32 bounds_flags;
            s32 placement_kind;
            bounds_flags = part->placement_flags;
            placement_kind = (bounds_flags >> 0x12) & 0x3F;
            if (placement_kind < 0x14)
            {
                if ((bounds_flags >> 0x10) & 1)
                {
                    s32 previous_distance = local_vector->y;
                    if ((u32) (placement_kind - 0xA) >= 0x1CU)
                    {
                        dy = (D_80105AE0[actor->owner_object_index].bounds_right - D_80105AE0[actor->owner_object_index].bounds_left) >> 1;
                    }
                    else
                    {
                        dy = (D_80105AE0[actor->track_object_indices[g_field_track_index]].bounds_right - D_80105AE0[actor->track_object_indices[g_field_track_index]].bounds_left) >> 1;
                    }
                    dy = abs(dy);
                    local_vector->y = previous_distance - dy;
                    bounds_flags = part->placement_flags;
                }
                if ((bounds_flags >> 0x11) & 1)
                {
                    s32 previous_distance = local_vector->y;
                    if ((u32) (((bounds_flags >> 0x12) & 0x3F) - 0xA) >= 0x1CU)
                    {
                        dy = (D_80105AE0[actor->owner_object_index].bounds_bottom - D_80105AE0[actor->owner_object_index].bounds_top) >> 1;
                    }
                    else
                    {
                        dy = (D_80105AE0[actor->track_object_indices[g_field_track_index]].bounds_bottom - D_80105AE0[actor->track_object_indices[g_field_track_index]].bounds_top) >> 1;
                    }
                    dy = abs(dy);
                    local_vector->y = previous_distance - dy;
                }
            }
        }
        local_vector->x = 0;
        local_vector->z = 0;
        rec->heading = rec->heading + rec->motion_parameter;
        RotMatrix_gte((FieldSVector *) &rec->rotation_x, rotation);
        RotMatrixZ(rec->rotation_z_16 * 0x10, rotation);
        RotMatrixY(rec->rotation_y_16 * 0x10, rotation);
        gte_SetRotMatrix(rotation);
        gte_ldv0(local_vector);
        gte_rtv0();
        gte_stsv(rotated_vector);

        /* 0..9 select owner bounds; 10..19 select the current track object bounds. */
        selector = ((FieldPlacementBits *) &part->placement_flags)->opcode;
        switch (selector)
        {
            case 0x0: case 0x1: case 0x2: case 0x3: case 0x4:
            case 0x5: case 0x6: case 0x7: case 0x8: case 0x9:
            case 0xA: case 0xB: case 0xC: case 0xD: case 0xE:
            case 0xF: case 0x10: case 0x11: case 0x12: case 0x13:
            {
                s32 offset_y;
                FieldObjectPlacement *owner;
                FieldObjectPlacement *owner_base;

                if ((s32) selector >= 0xA)
                {
                    slot = actor->track_object_indices[g_field_track_index];
                    selector -= 0xA;
                    reference_record = &D_800FDF58[slot];
                    reference_object = &D_80105AE0[slot];
                }
                else
                {
                    slot = actor->owner_object_index;
                    reference_record = &D_800FDF58[slot];
                    reference_object = &D_80105AE0[slot];
                }
                owner_base = D_80105AE0;
                owner = &owner_base[actor->owner_object_index];
                offset_y = 0;
                state_or_delta = *(u8 *) &owner->state_flags;
                if ((state_or_delta & 1) && ((u8) actor->actor_index >= 0x40U))
                {
                    offset_or_angle = offset_y;
                    if (!(((u32) owner->state_flags >> 5) & 1))
                    {
                        offset_y = 0x800000;
                        offset_or_angle = offset_y;
                        selector = -1;
                    }
                }
                else
                {
                    offset_or_angle = offset_y;
                }
                switch (selector)
                {
                    case 1:
                        offset_or_angle = (reference_object->bounds_right + reference_object->bounds_left) >> 1;
                        offset_y = (reference_object->bounds_bottom + reference_object->bounds_top) >> 1;
                        break;
                    case 2:
                        offset_y = 0;
                        offset_or_angle = (reference_object->bounds_right + reference_object->bounds_left) >> 1;
                        break;
                    case 3:
                        offset_y = reference_object->bounds_top;
                        offset_or_angle = (reference_object->bounds_right + reference_object->bounds_left) >> 1;
                        break;
                    case 4:
                        offset_or_angle = reference_object->bounds_left;
                        offset_y = (reference_object->bounds_bottom + reference_object->bounds_top) >> 1;
                        break;
                    case 5:
                        offset_or_angle = reference_object->bounds_right;
                        offset_y = (reference_object->bounds_bottom + reference_object->bounds_top) >> 1;
                        break;
                    case 6:
                        offset_or_angle = reference_object->bounds_left;
                        offset_y = reference_object->bounds_top;
                        break;
                    case 7:
                        offset_or_angle = reference_object->bounds_right;
                        offset_y = reference_object->bounds_top;
                        break;
                    case 8:
                        offset_or_angle = reference_object->bounds_left;
                        offset_y = reference_object->bounds_bottom;
                        break;
                    case 9:
                        offset_or_angle = reference_object->bounds_right;
                        offset_y = reference_object->bounds_bottom;
                        break;
                }
                offset_or_angle <<= 8;
                placement_origin->vx = reference_record->x + offset_or_angle;
                offset_y <<= 8;
                placement_origin->vy = reference_record->y + offset_y;
                placement_origin->vz = reference_record->z;
                break;
            }
            case 0x14: case 0x15: case 0x16: case 0x17: case 0x18:
            case 0x19: case 0x1A: case 0x1B: case 0x2A: case 0x2B:
            case 0x2C: case 0x2D: case 0x2E: case 0x2F: case 0x30:
            case 0x31:
            {
                FieldMotionRecord *effect;
                FieldMotionRecord *effect_base;
                effect_base = D_800FF658;
                effect = &effect_base[(u16) rec->reference_index];
                reference_state = effect->state;
                if (reference_state != FIELD_EFFECT_RETIRED)
                {
                    placement_origin->vx = effect->x;
                    placement_origin->vy = D_800FF658[((u16) rec->reference_index)].y;
                    placement_origin->vz = D_800FF658[((u16) rec->reference_index)].z;
                    if (!(part->orientation_flags & FIELD_EFFECT_ORIENTATION_LOCK))
                    {
                        RotMatrix_gte((FieldSVector *) &D_800FF658[((u16) rec->reference_index)].rotation_x, rotation);
                        gte_SetRotMatrix(rotation);
                        gte_ldv0(rotated_vector);
                        gte_rtv0();
                        gte_stsv(local_vector);
                        rotated_vector->x = local_vector->x;
                        rotated_vector->y = local_vector->y;
                        rotated_vector->z = local_vector->z;
                    }
                    break;
                }
                rec->state = reference_state;
                return;
            }
            case 0x25:
                placement_origin->vx = (part->offset_x << 8) - D_800F22A0;
                placement_origin->vy = (part->offset_y << 8) - D_800F22A4;
                placement_origin->vz = (part->offset_z << 8) - D_800F22A8;
                break;
            case 0x1D:
                placement_origin->vx = 0;
                placement_origin->vy = -0x7000;
                placement_origin->vz = 0;
                break;
            case 0x1E:
                placement_origin->vx = 0;
                placement_origin->vy = 0x7000;
                placement_origin->vz = 0;
                break;
            case 0x1F:
                placement_origin->vx = 0xFFFF6000;
                placement_origin->vy = 0;
                placement_origin->vz = 0;
                break;
            case 0x20:
                placement_origin->vx = 0xA000;
                placement_origin->vy = 0;
                placement_origin->vz = 0;
                break;
            case 0x21:
                placement_origin->vx = 0xFFFF6000;
                placement_origin->vy = -0x7000;
                placement_origin->vz = 0;
                break;
            case 0x22:
                placement_origin->vx = 0xA000;
                placement_origin->vy = -0x7000;
                placement_origin->vz = 0;
                break;
            case 0x23:
                placement_origin->vx = 0xFFFF6000;
                placement_origin->vy = 0x7000;
                placement_origin->vz = 0;
                break;
            case 0x24:
                placement_origin->vx = 0xA000;
                placement_origin->vy = 0x7000;
                placement_origin->vz = 0;
                break;
            case 0x27:
                reference_record = &D_800FDF58[actor->owner_object_index];
                if ((((u32) part->placement_flags >> 0xA) & 1) && !(reference_record->facing_or_reward_kind & 0x80))
                {
                    placement_origin->vx = reference_record->x - (part->offset_x << 8);
                }
                else
                {
                    placement_origin->vx = reference_record->x + (part->offset_x << 8);
                }
                placement_origin->vy = reference_record->y + (part->offset_y << 8);
                placement_origin->vz = reference_record->z + (part->offset_z << 8);
                break;
            case 0x28:
                reference_record = &D_800FDF58[actor->track_object_indices[g_field_track_index]];
                if ((part->spawn_flags.word & 0x08000000) && !(D_800FDF58[actor->owner_object_index].facing_or_reward_kind & 0x80))
                {
                    placement_origin->vx = reference_record->x - (part->offset_x << 8);
                }
                else if ((((u32) part->placement_flags >> 0xA) & 1) && !(reference_record->facing_or_reward_kind & 0x80))
                {
                    placement_origin->vx = reference_record->x - (part->offset_x << 8);
                }
                else
                {
                    placement_origin->vx = reference_record->x + (part->offset_x << 8);
                }
                placement_origin->vy = reference_record->y + (part->offset_y << 8);
                placement_origin->vz = reference_record->z + (part->offset_z << 8);
                break;
            case 0x29:
                reference_record = &D_800FDF58[actor->owner_object_index];
                reference_object = &D_80105AE0[actor->owner_object_index];
                placement_origin->vx = reference_record->x;
                placement_origin->vy = reference_record->y + (reference_object->attachment_points[((u32) part->effect_flags >> 0x15) & 3].y << 8);
                placement_origin->vz = reference_record->z;
                placement_origin->vx += reference_object->attachment_points[((u32) part->effect_flags >> 0x15) & 3].x << 8;
                break;
            case 0x32:
                reference_record = &D_800FDF58[actor->owner_object_index];
                reference_object = &D_80105AE0[actor->owner_object_index];
                placement_origin->vx = reference_record->x + (reference_object->attachment_points[((u32) part->effect_flags >> 0x15) & 3].x << 8);
                placement_origin->vy = reference_record->y;
                placement_origin->vz = reference_record->z + (part->offset_z << 8);
                if ((((u32) part->placement_flags >> 0xA) & 1) && !(reference_record->facing_or_reward_kind & 0x80))
                {
                    placement_origin->vx -= part->offset_x << 8;
                }
                else
                {
                    placement_origin->vx += part->offset_x << 8;
                }
                break;
            case 0x33:
                reference_record = &D_800FDF58[actor->owner_object_index];
                reference_object = &D_80105AE0[actor->owner_object_index];
                placement_origin->vx = reference_record->x + (reference_object->ground_attachment_points[((u32) rec->flags >> 0xD) & 3].x << 8);
                placement_origin->vy = reference_record->y;
                placement_origin->vz = reference_record->z + (reference_object->ground_attachment_points[((u32) rec->flags >> 0xD) & 3].y << 8);
                break;
            case 0x34:
                reference_record = &D_800FDF58[actor->track_object_indices[g_field_track_index]];
                if (!(reference_record->facing_or_reward_kind & 0x80))
                {
                    placement_origin->vx = reference_record->x + (actor->track_offsets[g_field_track_index].x << 8);
                }
                else
                {
                    placement_origin->vx = reference_record->x - (actor->track_offsets[g_field_track_index].x << 8);
                }
                placement_origin->vy = reference_record->y + (actor->track_offsets[g_field_track_index].y << 8);
                placement_origin->vz = reference_record->z;
                break;
            case 0x36:
                placement_origin->vx = part->offset_x << 8;
                placement_origin->vy = part->offset_y << 8;
                placement_origin->vz = part->offset_z << 8;
                break;
            case 0x37: case 0x38: case 0x39: case 0x3A: case 0x3B:
            case 0x3C: case 0x3D: case 0x3E:
            {
                FieldMotionRecord *effect;
                FieldMotionRecord *effect_base;
                effect_base = D_800FF658;
                effect = &effect_base[(u16) rec->reference_index];
                reference_state = effect->state;
                if (reference_state == FIELD_EFFECT_RETIRED)
                {
                    rec->state = reference_state;
                    return;
                }
                placement_origin->vx = effect->x;
                placement_origin->vy = D_800FF658[(u16) rec->reference_index].y;
                placement_origin->vz = D_800FF658[(u16) rec->reference_index].z;
                if ((part->spawn_flags.word & 0x08000000) && !(D_800FDF58[actor->owner_object_index].facing_or_reward_kind & 0x80))
                {
                    placement_origin->vx -= part->offset_x << 8;
                }
                else if ((((u32) part->placement_flags >> 0xA) & 1) && !(D_800FF658[(u16) rec->reference_index].facing_or_reward_kind & 0x80))
                {
                    placement_origin->vx -= part->offset_x << 8;
                }
                else
                {
                    placement_origin->vx += part->offset_x << 8;
                }
                placement_origin->vy += part->offset_y << 8;
                placement_origin->vz += part->offset_z << 8;
                if (!(part->orientation_flags & FIELD_EFFECT_ORIENTATION_LOCK))
                {
                    RotMatrix_gte(&D_800FF668[(u16) rec->reference_index].angles, rotation);
                    gte_SetRotMatrix(rotation);
                    gte_ldv0(rotated_vector);
                    gte_rtv0();
                    gte_stsv(local_vector);
                    rotated_vector->x = local_vector->x;
                    rotated_vector->y = local_vector->y;
                    rotated_vector->z = local_vector->z;
                }
                break;
            }
            default:
                placement_origin->pad = 0;
                placement_origin->vz = 0;
                placement_origin->vy = 0;
                placement_origin->vx = 0;
                break;
            case 0x26:
                break;
        }
        rec->x = ((s16) rotated_vector->x << 8) + placement_origin->vx;
        rec->y = (((s32) (rotated_vector->y << 0x10)) >> 8) + placement_origin->vy;
        z = (((s32) (rotated_vector->z << 0x10)) >> 8) + placement_origin->vz;
        rec->z = z;
        if (part->placement_flags & 1)
        {
            rec->z = z + 0x80;
        }
        flags = part->placement_flags;
        if ((flags >> FIELD_PART_HEIGHT_TRACK_BIT) & 1)
        {
            if (part->spawn_flags.word & FIELD_PART_HEIGHT_FROM_BASE)
            {
                rec->y = (rec->height_or_retired_state - field_evaluate_parameter_track_at_time(actor, (flags >> 4) & 0xF, ((u16) rec->age))) << 8;
            }
            else
            {
                rec->y = -field_evaluate_parameter_track_at_time(actor, (flags >> 4) & 0xF, ((u16) rec->age)) << 8;
            }
        }
        state_or_delta = ((u8 *) &rec->flags)[3] & 7;
        switch (state_or_delta)
        {
            case 3:
                rec->y = rec->y + (((u16) rec->age) << 9);
                break;
            case 4:
                rec->y = rec->y - (((u16) rec->age) << 9);
                break;
        }
        if ((((u32) part->behavior_flags >> FIELD_PART_PITCH_ACCELERATION_BIT) & 1) && ((rec->pitch + part->pitch_acceleration) < 0x800))
        {
            rec->pitch = (u16) rec->pitch + part->pitch_acceleration;
        }
    }
    else
    {
        {
            u16 speed = rec->motion_parameter;
            local_vector->z = 0;
            local_vector->x = 0;
            local_vector->y = -speed << 2;
        }
        RotMatrix_gte((FieldSVector *) &rec->rotation_x, rotation);
        if (!(((u32) part->behavior_flags >> FIELD_PART_PITCH_ACCELERATION_BIT) & 1) && (rec->position_source == 0))
        {
            RotMatrixZ(rec->rotation_z_16 * 0x10, rotation);
            RotMatrixY(rec->rotation_y_16 * 0x10, rotation);
        }
        gte_SetRotMatrix(rotation);
        gte_ldv0(local_vector);
        gte_rtv0();
        gte_stsv(rotated_vector);

        /* Free movement uses a rotated step and a fresh collision probe each update. */
        initial_surface = FIELD_EFFECT_SKIP_SURFACE_PREPASS;
        if ((((rec->flags & 0x60000000) != 0x40000000) || ((s16) rec->motion_parameter != 0)) && ((((u32) part->behavior_flags >> FIELD_PART_PITCH_ACCELERATION_BIT) & 1) || ((s32) part->placement_flags < 0)))
        {
            s32 pitch_cosine;
            s16 adjusted_pitch;

            pitch_cosine = rcos(rec->pitch) - (part->pitch_acceleration * 8);
            adjusted_pitch = ratan2(rsin(rec->pitch), pitch_cosine);
            rec->pitch = adjusted_pitch;
            if (adjusted_pitch < 0x400)
            {
                rec->motion_parameter = (((s16) rec->motion_parameter * 0xF) >> 4) - 1;
            }
            else
            {
                rec->motion_parameter = (((s16) rec->motion_parameter << 5) / 30) + 1;
            }
            if (((s16) rec->motion_parameter < 0x14) && (rec->pitch < 0x400))
            {
                rec->pitch = 0x800 - (u16) rec->pitch;
                rec->motion_parameter = 0x14;
            }
        }

        if ((part->effect_flags & FIELD_PART_MAP_COLLISION) && !(part->spawn_flags.word & FIELD_PART_SKIP_MAP_COLLISION))
        {
            z = rec->x;
            if ((z < 0) || (z >= (map_bounds->width << 8)) || ((z = rec->z), (z < 0)) || (z >= ((s32) (map_bounds->height << 0x10) >> 7)))
            {
                if (!(part->spawn_flags.word & FIELD_PART_RETIRE_ON_COLLISION))
                {
                    rotated_vector->x = 0;
                    rotated_vector->z = 0;
                }
                else
                {
                    rec->state = FIELD_EFFECT_RETIRED;
                }
            }
            else
            {
                s32 position_z;
                s32 position_x;
                /* Advance the packed context during a collision attempt; protocol remains unresolved. */
                g_field_action_context += 0x100;
                mover->x = rec->x;
                mover->y = 0;
                mover->z = rec->z;
                mover->move_x = rotated_vector->x;
                mover->move_y = 0;
                mover->move_z = rotated_vector->z;
                mover->width = FIELD_EFFECT_COLLISION_WIDTH;
                mover->mode.bits.depth = FIELD_EFFECT_COLLISION_DEPTH;
                mover->height_tolerance = FIELD_EFFECT_COLLISION_HEIGHT;
                mover->collision_node = initial_surface;
                mover->contact_flags = 0;
                mover->mode.bits.unknown_17 = 0;
                mover->mode.bits.unknown_16 = 0;
                if (func_8005B6AC(mover) & 3)
                {
                    rotated_vector->z = 0;
                    rotated_vector->x = 0;
                    if (part->spawn_flags.word & FIELD_PART_RETIRE_ON_COLLISION)
                    {
                        rec->state = FIELD_EFFECT_RETIRED;
                    }
                }
                else if (!(part->spawn_flags.word & FIELD_PART_RETIRE_ON_COLLISION))
                {
                    position_x = mover->x;
                    query->width = FIELD_EFFECT_COLLISION_WIDTH;
                    query->height_tolerance = FIELD_EFFECT_COLLISION_HEIGHT;
                    /* This scope preserves the depth register's allocation priority. */
                    do
                    {
                        query->depth = FIELD_EFFECT_COLLISION_DEPTH;
                    } while (0);
                    query->x = position_x;
                    position_z = mover->z;
                    {
                        s32 query_y = rec->y;
                        query->z = position_z;
                        query->y = query_y;
                    }
                    if ((D_800FE754 != 0) && (func_8005B368(query) != -1))
                    {
                        rotated_vector->z = 0;
                        rotated_vector->x = 0;
                    }
                    else
                    {
                        rotated_vector->x = (u16) mover->x - (u16) rec->x;
                        rotated_vector->z = (u16) mover->z - (u16) rec->z;
                    }
                }
            }
        }

        if (part->spawn_flags.word & FIELD_PART_CAMERA_BOUNDS)
        {
            s32 camera_x;
            s32 next_z;
            dx = rec->x + (s16) rotated_vector->x;
            camera_x = -camera->x;
            if (((camera_x + FIELD_EFFECT_CAMERA_X_MIN) < dx) && (dx < (camera_x + FIELD_EFFECT_CAMERA_X_MAX)))
            {
                next_z = rec->z + (s16) rotated_vector->z;
                x = -camera->z;
                if ((x < next_z) && (next_z < (x + FIELD_EFFECT_CAMERA_Z_SPAN)))
                {
                    rec->x = dx;
                    rec->z = rec->z + (s16) rotated_vector->z;
                }
            }
        }
        else
        {
            rec->x = rec->x + (s16) rotated_vector->x;
            rec->z = rec->z + (s16) rotated_vector->z;
        }
        {
            u32 ground_flags;
            y = rec->y + (s16) rotated_vector->y;
            rec->y = y;
            if ((part->spawn_flags.word & FIELD_PART_GROUND_STOP) && (y >= 0) && (((ground_flags = rec->flags, state_or_delta = ground_flags & 0x60000000), (state_or_delta == 0)) || (state_or_delta == 0x40000000)))
            {
                rec->flags = ground_flags & 0x9FFFFFFF;
                rec->y = 0;
                rec->motion_parameter = 0;
            }
        }
        flags = part->placement_flags;
        if ((flags >> FIELD_PART_HEIGHT_TRACK_BIT) & 1)
        {
            if (part->spawn_flags.word & FIELD_PART_HEIGHT_FROM_BASE)
            {
                rec->y = (rec->height_or_retired_state - field_evaluate_parameter_track_at_time(actor, (flags >> 4) & 0xF, ((u16) rec->age))) << 8;
            }
            else
            {
                rec->y = -field_evaluate_parameter_track_at_time(actor, (flags >> 4) & 0xF, ((u16) rec->age)) << 8;
            }
        }

        /* Resolve a target and either stop nearby or steer the heading and pitch. */
        if (rec->position_source != 0)
        {
            s32 target_dy, target_dz;

            field_resolve_effect_position(rec, part, target_position);
            work_vector->vx = (rec->x - target_position->vx) >> 8;
            work_vector->vy = (rec->y - target_position->vy) >> 8;
            target_dz = (rec->z - target_position->vz) >> 8;
            work_vector->vz = target_dz;
            if (((u32) (work_vector->vx + 0xF) < 0x1FU) && (target_dz >= -0xF) && (target_dz < 0x10))
            {
                target_dy = work_vector->vy;
                if ((target_dy >= -0xF) && (work_vector->vy < 0x10))
                {
                    if ((((u32) part->behavior_flags >> 4) & 3) == 2)
                    {
                        retired_state = rec->state;
                        rec->state = FIELD_EFFECT_RETIRED;
                        rec->height_or_retired_state = (s8) retired_state;
                        return;
                    }
                    if (!(part->orientation_flags & FIELD_EFFECT_ORIENTATION_LOCK))
                    {
                        rec->motion_parameter = 0;
                        rec->flags = rec->flags & 0x9FFFFFFF;
                    }
                }
            }
            if (!(rec->flags & FIELD_EFFECT_MOTION_KIND_MASK) && !(part->orientation_flags & FIELD_EFFECT_ORIENTATION_LOCK))
            {
                FieldVector new_pos;
                FieldVector delta;
                FieldVector delta_squared;
                s16 target_heading;

                field_resolve_effect_position(rec, part, &new_pos);
                delta.vx = (new_pos.vx - rec->x) >> 8;
                delta.vy = (new_pos.vy - rec->y) >> 8;
                delta.vz = (new_pos.vz - rec->z) >> 8;
                gte_ldlvl(&delta);
                gte_sqr0();
                gte_stlvnl(&delta_squared);
                target_heading = ratan2(-delta.vz, delta.vx);
                rec->heading = target_heading;
                if (target_heading < 0)
                {
                    rec->heading = target_heading + FIELD_ANGLE_TURN;
                }
                if (delta.vy != 0)
                {
                    rec->pitch = ratan2(SquareRoot0(delta_squared.vx + delta_squared.vz), -delta.vy);
                }
                else
                {
                    rec->pitch = FIELD_ANGLE_QUARTER_TURN;
                }
                if (rec->pitch < 0)
                {
                    rec->pitch = (u16) rec->pitch + FIELD_ANGLE_TURN;
                }
                rec->rotation_x = 0;
            }
            if ((rec->flags & FIELD_EFFECT_MOTION_KIND_MASK) == FIELD_EFFECT_MOTION_HOMING)
            {

                s16 next_heading;

                gte_ldlvl(work_vector);
                gte_sqr0();
                gte_stlvnl(target_position);
                offset_or_angle = ratan2(work_vector->vz, -work_vector->vx);
                if (offset_or_angle < 0)
                {
                    offset_or_angle += FIELD_ANGLE_TURN;
                }
                if (((u16) rec->age) == part->turn_end_age)
                {
                    rec->flags = rec->flags & 0xF8FFFFFF;
                    if (!(part->orientation_flags & FIELD_EFFECT_ORIENTATION_LOCK))
                    {
                        rec->heading = (u16) offset_or_angle;
                        if (work_vector->vz != 0)
                        {
                            rec->pitch = ratan2(SquareRoot0(target_position->vx + target_position->vz), work_vector->vy);
                        }
                        else
                        {
                            rec->pitch = FIELD_ANGLE_QUARTER_TURN;
                        }
                        if (rec->pitch < 0)
                        {
                            rec->pitch = (u16) rec->pitch + FIELD_ANGLE_TURN;
                        }
                    }
                }
                else
                {
                    current_angle = (s16) rec->heading;
                    selector = (offset_or_angle - current_angle) & FIELD_ANGLE_MASK;
                    if ((u32) (selector - 8) >= 0xFF1U)
                    {
                        rec->heading = (u16) offset_or_angle;
                    }
                    else
                    {
                        if (selector >= 0x801)
                        {
                            s32 reverse_turn = FIELD_ANGLE_TURN - selector;
                            if (reverse_turn < FIELD_EFFECT_TURN_THRESHOLD)
                            {
                                next_heading = current_angle - (reverse_turn >> 2);
                            }
                            else
                            {
                                next_heading = current_angle - FIELD_EFFECT_TURN_STEP;
                            }
                        }
                        else if (selector < FIELD_EFFECT_TURN_THRESHOLD)
                        {
                            next_heading = current_angle + (selector >> 2);
                        }
                        else
                        {
                            next_heading = current_angle + FIELD_EFFECT_TURN_STEP;
                        }
                        rec->heading = next_heading;
                    }
                    if ((s16) rec->heading < 0)
                    {
                        rec->heading = (u16) rec->heading + FIELD_ANGLE_TURN;
                    }
                    {
                        offset_or_angle = ratan2(SquareRoot0(target_position->vx + target_position->vz), work_vector->vy);
                        current_angle = rec->pitch;
                        if (offset_or_angle < current_angle)
                        {
                            state_or_delta = current_angle - offset_or_angle;
                            if (state_or_delta < FIELD_EFFECT_TURN_THRESHOLD)
                            {
                                rec->pitch = current_angle - (state_or_delta >> 2);
                            }
                            else
                            {
                                rec->pitch = current_angle - FIELD_EFFECT_TURN_STEP;
                            }
                        }
                        else if (current_angle < offset_or_angle)
                        {
                            state_or_delta = offset_or_angle - current_angle;
                            if (state_or_delta < FIELD_EFFECT_TURN_THRESHOLD)
                            {
                                rec->pitch = current_angle + (state_or_delta >> 2);
                            }
                            else
                            {
                                rec->pitch = current_angle + FIELD_EFFECT_TURN_STEP;
                            }
                        }
                        if (rec->pitch < 0)
                        {
                            rec->pitch = (u16) rec->pitch + FIELD_ANGLE_TURN;
                        }
                        rec->rotation_x = 0;
                    }
                }
            }
        }
    }
    /* After the pickup delay, find a nearby recipient and consume the reward effect. */
    if (!(actor->action_flags & 1))
    {
        state_or_delta = ((u16 *) &actor->action_flags)[1];
        switch (state_or_delta)
        {
            case FIELD_PICKUP_EXPERIENCE_OR_CURRENCY:
                if ((u16) actor->track_ages[0] >= FIELD_PICKUP_DELAY)
                {
                    recipient_index = func_8009980C(rec, FIELD_PICKUP_DISTANCE, actor, 0);
                    if (recipient_index != -1)
                    {
                        FieldObjectPlacement *recipient_object;
                        FieldObjectPlacement *object_base;
                        s32 counter_slot;
                        FieldCounterView *counter_base;
                        u32 counter_index;

                        if (g_field_pickup_sound_played == 0)
                        {
                            func_800A3938(FIELD_PICKUP_SOUND, func_8006CE70(rec->source_object_index));
                        }
                        g_field_pickup_sound_played = 1;
                        rec->state = FIELD_EFFECT_RETIRED;
                        object_base = D_80105AE0;
                        recipient_object = &object_base[recipient_index];
                        field_set_action_context(recipient_object->record_id, object_base[actor->owner_object_index].record_id, rec->facing_or_reward_kind - 0x16);
                        counter_base = D_800FD818;
                        counter_index = rec->facing_or_reward_kind;
                        counter_slot = recipient_index < 3 ? recipient_index : 2;
                        counter_base[counter_slot].counters[counter_index] = counter_base[recipient_index < 3 ? recipient_index : 2].counters[counter_index] + 1;
                        func_800C0B40(recipient_object->record_id, object_base[actor->owner_object_index].record_id, rec->facing_or_reward_kind - 0x16);
                        field_release_actor_if_no_effects(rec);
                        return;
                    }
                }
                break;
            case FIELD_PICKUP_ITEM:
                if ((u16) actor->track_ages[0] >= FIELD_PICKUP_DELAY)
                {
                    recipient_index = func_8009980C(rec, FIELD_PICKUP_DISTANCE, actor, 0);
                    if (recipient_index != -1)
                    {
                        FieldObjectPlacement *recipient_object;
                        FieldObjectPlacement *object_base;

                        if (g_field_pickup_sound_played == 0)
                        {
                            func_800A3938(FIELD_PICKUP_SOUND, func_8006CE70(rec->source_object_index));
                        }
                        g_field_pickup_sound_played = 1;
                        rec->state = FIELD_EFFECT_RETIRED;
                        object_base = D_80105AE0;
                        recipient_object = &object_base[recipient_index];
                        field_set_action_context(recipient_object->record_id, object_base[actor->owner_object_index].record_id, 4);
                        func_800C0B40(recipient_object->record_id, object_base[actor->owner_object_index].record_id, 4);
                        field_release_actor_if_no_effects(rec);
                        return;
                    }
                }
                break;
            /* Restore 64/256 of maximum capacity, then play the quarter-restore animation. */
            case FIELD_PICKUP_RESTORE_QUARTER:
                if ((u16) actor->track_ages[0] >= FIELD_PICKUP_DELAY)
                {
                    recipient_index = func_8009980C(rec, FIELD_PICKUP_DISTANCE, actor, 0);
                    if (recipient_index != -1)
                    {
                        FieldObjectPlacement *recipient_object;
                        FieldObjectPlacement *object_base;

                        if (g_field_pickup_sound_played == 0)
                        {
                            func_800A3938(FIELD_PICKUP_SOUND, func_8006CE70(rec->source_object_index));
                        }
                        g_field_pickup_sound_played = 1;
                        rec->state = FIELD_EFFECT_RETIRED;
                        object_base = D_80105AE0;
                        recipient_object = &object_base[recipient_index];
                        field_set_action_context(recipient_object->record_id, object_base[actor->owner_object_index].record_id, 5);
                        func_800C0B40(recipient_object->record_id, object_base[actor->owner_object_index].record_id, 5);
                        func_80092C24(&D_800FDF58[recipient_index], 0x2C);
                        field_release_actor_if_no_effects(rec);
                        return;
                    }
                }
                break;
            /* Restore 128/256 of maximum capacity. */
            case FIELD_PICKUP_RESTORE_HALF:
                if ((u16) actor->track_ages[0] >= FIELD_PICKUP_DELAY)
                {
                    recipient_index = func_8009980C(rec, FIELD_PICKUP_DISTANCE, actor, 0);
                    if (recipient_index != -1)
                    {
                        FieldObjectPlacement *recipient_object;
                        FieldObjectPlacement *object_base;

                        if (g_field_pickup_sound_played == 0)
                        {
                            func_800A3938(FIELD_PICKUP_SOUND, func_8006CE70(rec->source_object_index));
                        }
                        g_field_pickup_sound_played = 1;
                        rec->state = FIELD_EFFECT_RETIRED;
                        object_base = D_80105AE0;
                        recipient_object = &object_base[recipient_index];
                        field_set_action_context(recipient_object->record_id, object_base[actor->owner_object_index].record_id, 6);
                        func_800C0B40(recipient_object->record_id, object_base[actor->owner_object_index].record_id, 6);
                        func_80092C24(&D_800FDF58[recipient_index], 0x2D);
                        field_release_actor_if_no_effects(rec);
                        return;
                    }
                }
                break;
            default:
                break;
        }
    }
    /* Collect effect-centered hits, reflect below-ground motion, then scale speed. */
    {
        FieldActorAnimationDef *anim = actor->animation;
        if ((anim->hit_test_mode == FIELD_HIT_TEST_EFFECT_BOUNDS) && (anim->hit_test_part == rec->part_index))
        {
            field_collect_effect_hits(rec, anim->hit_radius, actor);
        }
    }
    if (((u32) part->behavior_flags >> FIELD_PART_GROUND_BOUNCE_BIT) & 1)
    {
        s32 ground_y = rec->y;
        if (ground_y > 0)
        {
            s32 old_pitch;
            s16 old_motion;

            rec->y = -ground_y;
            old_pitch = (u16) rec->pitch;
            old_motion = rec->motion_parameter;
            rec->pitch = FIELD_ANGLE_HALF_TURN - old_pitch;
            rec->motion_parameter = old_motion / 2;
            field_dispatch_actor_audio_event(actor, 4, rec->part_index, old_pitch);
        }
    }
    rec->motion_parameter = (rec->motion_parameter * rec->motion_scale) >> 8;
}
