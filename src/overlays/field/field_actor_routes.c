#include "common.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

#define FIELD_POSITION_HISTORY_LENGTH 48
#define FIELD_FIXED_POINT_SHIFT 8
#define FIELD_FIXED_POINT_ROUND_BIAS 0xFF

/** @brief One signed X/Z point in a sampled route. */
typedef struct
{
    s16 x, z;
} FieldRoutePoint;

/** @brief Fixed-point position and route-state index in a 0x54-byte actor. */
typedef struct
{
    s32 x, y, z;
    u8 pad_c[0x2E];
    u8 index;
    u8 pad_3b[0x19];
} FieldRouteActor;

/** @brief World-space point pair used for the heading lookup. */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
} PointD104;

/** @brief Actor position and status record sampled by the history update. */
typedef struct
{
    s32 x;
    s32 unk4;
    s32 y;
    u8 padC[0x21 - 0xC];
    u8 status;
    u8 pad22[0x3A - 0x22];
    u8 actor_index;
} FieldPositionRecord;

/** @brief Partial 0x54-byte actor record used for following, collision, and animation. */
typedef struct
{
    s32 unk0, unk4, unk8;
    u8 padc[10];
    s16 unk16;
    u8 pad18[4];
    u32 unk1C;
    u8 pad20;
    u8 unk21;
    u8 pad22[2];
    u8 unk24, unk25, unk26, unk27;
    u8 pad28[2];
    s16 unk2A;
    u8 pad2c[7];
    u8 unk33;
    u8 pad34[2];
    s8 unk36;
    u8 pad37[3];
    u8 unk3A, unk3B;
    u8 pad3c[0x18];
} Actor;

s32 func_8001CDAC(s32 *, s32 *);
void func_8008A0B0(FieldRouteActor *, s32, s32);
extern long ratan2(long y, long x);

/* Forward declarations for member functions defined later in this file. */
void func_8008CAA4(FieldRoutePoint *points, s32 remaining, FieldRouteActor *actor, FieldRouteActor *target,
                   s32 heading_offset, s32 unused_limit);
s32 func_8008D104(PointD104 *a, PointD104 *b);

/**
 * @brief Empty stub function; body is a no-op.
 */
void func_8008C728(void)
{
}

/**
 * @brief Copies the active field position and status into 48 compact entries.
 *
 * The fixed-point coordinates are divided by 256 with truncation toward zero
 * before being stored as signed halfwords.
 */
void func_8008C730(void)
{
    typedef struct
    {
        s32 unk0;
        s32 unk4;
        s32 unk8;
        u8 padC[0x21 - 0xC];
        u8 unk21;
    } FieldPositionRecord;

    typedef struct
    {
        s16 x;
        s16 y;
    } FieldPosition16;

    extern FieldPositionRecord D_800FDF58;
    extern FieldPosition16 D_80105B4C[];
    extern u8 D_8010CFE0[];

    FieldPosition16 *dest;
    s32 index;
    s32 value;

    dest = D_80105B4C;
    index = 0;
    do
    {
        value = D_800FDF58.unk0;
        if (value < 0)
        {
            value += 0xFF;
        }
        dest->x = value >> 8;

        value = D_800FDF58.unk8;
        if (value < 0)
        {
            value += 0xFF;
        }
        dest->y = value >> 8;
        D_8010CFE0[index] = D_800FDF58.unk21;
        index++;
        dest++;
    } while (index < 0x30);
}

/**
 * @brief Refresh actor position histories when their current samples differ.
 * @note Coordinate adjustment before shifting preserves truncation toward zero.
 * @note The primary path requires an active actor and a nonzero low flag field.
 * @note GCC 2.7.2 CDK matches all 191 instructions (764 bytes).
 */
void func_8008C7A8(void)
{
    /** @brief Sparse view of the actor fields used to refresh position history. */
    typedef struct
    {
        u8 pad0[0x54];
        s32 primary_x;
        u8 pad58[4];
        s32 primary_z;
        u8 pad60[0x10];
        s32 primary_flags;
        u8 pad74[5];
        u8 primary_actor;
        u8 pad7a[0xA8 - 0x7A];
        s32 secondary_x;
        u8 pad_ac[4];
        s32 secondary_z;
        u8 pad_b4[0xCD - 0xB4];
        u8 secondary_actor;
    } FieldActors;

    /** @brief Sparse position-history view used at the base and indexed offsets. */
    typedef struct
    {
        u8 pad0[0x6C];
        s16 x;
        s16 z;
        u8 pad70[0x3AA - 0x70];
        u8 primary_index;
        u8 pad3ab[0x5E6 - 0x3AB];
        u8 secondary_index;
    } FieldHistory;

    extern FieldActors D_800FDF58;
    extern u8 D_800FDFAC[];
    extern u8 D_800FE000[];
    extern FieldHistory D_80105AE0;
    extern u8 D_80105B4C[];

    s32 primary_x;
    s32 secondary_x;
    s32 primary_only_x;
    s32 secondary_only_x;
    s32 primary_z;
    s32 secondary_z;
    s32 primary_only_z;
    s32 secondary_only_z;
    FieldHistory *primary_history;
    FieldHistory *secondary_history;
    FieldHistory *primary_only_history;
    FieldHistory *secondary_only_history;
    FieldHistory *history_base;

    if ((D_800FDF58.primary_actor != 0xFF) && (D_800FDF58.primary_flags & 0x1FF))
    {
        if (D_800FDF58.secondary_actor != 0xFF)
        {
            primary_x = D_800FDF58.primary_x;
            if (primary_x < 0)
            {
                primary_x += 0xFF;
            }
            primary_history = (FieldHistory *)((D_80105AE0.primary_index * 4) + (u8 *)&D_80105AE0);
            if ((primary_x >> 8) == primary_history->x)
            {
                primary_z = D_800FDF58.primary_z;
                if (primary_z < 0)
                {
                    primary_z += 0xFF;
                }
                if ((primary_z >> 8) == primary_history->z)
                {
                    secondary_x = D_800FDF58.secondary_x;
                    if (secondary_x < 0)
                    {
                        secondary_x += 0xFF;
                    }
                    secondary_history = (FieldHistory *)((D_80105AE0.secondary_index * 4) + (u8 *)&D_80105AE0);
                    if ((secondary_x >> 8) == secondary_history->x)
                    {
                        secondary_z = D_800FDF58.secondary_z;
                        if (secondary_z < 0)
                        {
                            secondary_z += 0xFF;
                        }
                        if ((secondary_z >> 8) != secondary_history->z)
                        {
                            goto refresh_both;
                        }
                    }
                    else
                    {
                        goto refresh_both;
                    }
                }
                else
                {
                    goto refresh_both;
                }
            }
            else
            {
refresh_both:
                func_8008CAA4(D_80105B4C, 0x18, D_800FDFAC, D_800FDFAC, 0, 0x18);
                func_8008CAA4(D_80105B4C + 0x60, 0x17, D_800FDFAC, D_800FDFAC - 0x54, 0x18, 0x18);
                history_base = (FieldHistory *)(D_80105B4C - 0x6C);
                history_base->primary_index = 0x18;
                func_8008CAA4(D_80105B4C, 0x18, D_800FDFAC + 0x54, D_800FDFAC, 0, 0x18);
                history_base->secondary_index = 0;
            }
        }
        else
        {
            primary_only_x = D_800FDF58.primary_x;
            if (primary_only_x < 0)
            {
                primary_only_x += 0xFF;
            }
            primary_only_history = (FieldHistory *)((D_80105AE0.primary_index * 4) + (u8 *)&D_80105AE0);
            if ((primary_only_x >> 8) == primary_only_history->x)
            {
                primary_only_z = D_800FDF58.primary_z;
                if (primary_only_z < 0)
                {
                    primary_only_z += 0xFF;
                }
                if ((primary_only_z >> 8) != primary_only_history->z)
                {
                    goto refresh_primary;
                }
            }
            else
            {
refresh_primary:
                func_8008CAA4(D_80105B4C, 0x18, D_800FDFAC, D_800FDFAC, 0, 0x18);
                func_8008CAA4(D_80105B4C + 0x60, 0x17, D_800FDFAC, D_800FDFAC - 0x54, 0x18, 0x18);
                D_80105B4C[0x33E] = 0x18;
            }
        }
    }
    else if (D_800FDF58.secondary_actor != 0xFF)
    {
        secondary_only_x = D_800FDF58.secondary_x;
        if (secondary_only_x < 0)
        {
            secondary_only_x += 0xFF;
        }
        secondary_only_history = (FieldHistory *)((D_80105AE0.secondary_index * 4) + (u8 *)&D_80105AE0);
        if ((secondary_only_x >> 8) == secondary_only_history->x)
        {
            secondary_only_z = D_800FDF58.secondary_z;
            if (secondary_only_z < 0)
            {
                secondary_only_z += 0xFF;
            }
            if ((secondary_only_z >> 8) != secondary_only_history->z)
            {
                goto refresh_secondary;
            }
        }
        else
        {
refresh_secondary:
            func_8008CAA4(D_80105B4C, 0x18, D_800FE000, D_800FE000, 0, 0x18);
            func_8008CAA4(D_80105B4C + 0x60, 0x17, D_800FE000, D_800FE000 - 0xA8, 0x18, 0x18);
            D_80105B4C[0x57A] = 0x18;
        }
    }
}

/**
 * @brief Sample an actor route into signed points and parallel heading entries.
 * @param points Output X/Z points.
 * @param remaining Requested point count, including the actor's initial position.
 * @param actor Actor whose generated route is sampled.
 * @param target Destination actor used for routing and heading calculation.
 * @param heading_offset Starting index in the shared heading array.
 * @param unused_limit Unused sixth argument retained to agree with the caller.
 * @note Coordinates are fixed point with eight fractional bits; interpolated
 *       normalized directions use twelve fractional bits.
 */
void func_8008CAA4(FieldRoutePoint *points, s32 remaining, FieldRouteActor *actor, FieldRouteActor *target,
                   s32 heading_offset, s32 unused_limit)
{
    /** @brief Route count and first waypoint in a 0x23C-byte actor state. */
    typedef struct
    {
        u8 pad0[0x1A4];
        u16 count;
        u8 pad_1a6[6];
        s32 x, z;
        u8 pad_1b4[0x88];
    } FieldRouteState;

    extern FieldRouteState D_80105AE0[];
    extern s8 D_8010CFE0[];

    s32 work[12];
    s32 actor_index;
    FieldRouteState *states;
    s32 heading;
    s16 *z_cursor;
    s16 *cursor;
    s16 temp_v0_3;
    s16 temp_v0_4;
    s32 waypoint_offset;
    s32 spacing;
    s32 temp_v0;
    s32 temp_v1;
    s32 temp_v1_2;
    s32 point_index;
    s32 total_distance;
    s32 path_offset;
    s32 path_index;
    s32 sample_path_index;
    s32 var_v0;
    s32 var_v0_10;
    s32 var_v0_11;
    s32 var_v0_2;
    s32 var_v0_3;
    s32 var_v0_4;
    s32 var_v0_5;
    s32 var_v0_6;
    s32 var_v0_7;
    s32 var_v0_8;
    s32 var_v0_9;
    s32 var_v1;
    s32 var_v1_2;
    s32 var_v1_3;
    s32 temp_a1;

    func_8008A0B0(actor, target->index, 0);
    work[0] = actor->x;
    total_distance = 0;
    work[2] = actor->z;
    path_index = 0;
    actor_index = actor->index;
    if (D_80105AE0[actor_index].count != 0)
    {
        do
        {
            waypoint_offset = path_index * 8;
            var_v0 = ((FieldRouteState *)(waypoint_offset + (actor_index * 0x23C) + (u8 *)D_80105AE0))->x - work[0];
            if (var_v0 < 0)
            {
                var_v0 += 0xFF;
            }
            work[4] = var_v0 >> 8;
            var_v0_2 = ((FieldRouteState *)(waypoint_offset + (actor->index * 0x23C) + (u8 *)D_80105AE0))->z - work[2];
            if (var_v0_2 < 0)
            {
                var_v0_2 += 0xFF;
            }
            work[6] = var_v0_2 >> 8;
            work[5] = 0;
            gte_ldlvl(&work[4]);
            gte_sqr0();
            gte_stlvnl(&work[8]);
            temp_v0 = SquareRoot0(work[8] + work[10]);
            work[0] = ((FieldRouteState *)(waypoint_offset + (actor->index * 0x23C) + (u8 *)D_80105AE0))->x;
            work[2] = ((FieldRouteState *)(waypoint_offset + (actor->index * 0x23C) + (u8 *)D_80105AE0))->z;
            path_index += 1;
            total_distance += temp_v0;
            actor_index = actor->index;
        } while (path_index < (s32)D_80105AE0[actor_index].count);
    }
    temp_a1 = func_8008D104(actor, target);
    var_v1 = actor->x;
    if (var_v1 < 0)
    {
        var_v1 += 0xFF;
    }
    points->x = (s16)(var_v1 >> 8);
    var_v0_3 = actor->z;
    if (var_v0_3 < 0)
    {
        var_v0_3 += 0xFF;
    }
    remaining -= 1;
    points->z = (s16)(var_v0_3 >> 8);
    cursor = (s16 *)((u8 *)points + 4);
    *(heading_offset + D_8010CFE0) = temp_a1;
    point_index = 1;
    if ((total_distance / remaining) < 3)
    {
    loop_13:
        *(point_index + heading_offset + D_8010CFE0) = temp_a1;
        var_v0_4 = actor->x;
        if (var_v0_4 < 0)
        {
            var_v0_4 += 0xFF;
        }
        cursor[0] = (s16)(var_v0_4 >> 8);
        var_v0_5 = actor->z;
        if (var_v0_5 < 0)
        {
            var_v0_5 += 0xFF;
        }
        cursor[1] = (s16)(var_v0_5 >> 8);
        cursor += 2;
        remaining -= 1;
        point_index += 1;
        if (remaining != 0)
        {
            if ((total_distance / remaining) >= 3)
            {
                goto block_19;
            }
            goto loop_13;
        }
    }
    else
    {
    block_19:
        work[0] = actor->x;
        work[2] = actor->z;
        spacing = total_distance / remaining;
        sample_path_index = 0;
        states = D_80105AE0;
        if (states[actor->index].count != 0)
        {
            path_offset = 0 * 8;
        loop_21:
            z_cursor = cursor + 1;
        loop_22:
            var_v0_6 = ((FieldRouteState *)(path_offset + (actor->index * 0x23C) + (u8 *)states))->x - work[0];
            if (var_v0_6 < 0)
            {
                var_v0_6 += 0xFF;
            }
            work[4] = var_v0_6 >> 8;
            var_v0_7 = ((FieldRouteState *)(path_offset + (actor->index * 0x23C) + (u8 *)states))->z - work[2];
            if (var_v0_7 < 0)
            {
                var_v0_7 += 0xFF;
            }
            work[6] = var_v0_7 >> 8;
            work[5] = 0;
            if (spacing >= SquareRoot0(func_8001CDAC(&work[4], &work[8])))
            {
                var_v0_8 = ((FieldRouteState *)(path_offset + (actor->index * 0x23C) + (u8 *)states))->x;
                if (var_v0_8 < 0)
                {
                    var_v0_8 += 0xFF;
                }
                *cursor = (s16)(var_v0_8 >> 8);
                var_v0_9 = ((FieldRouteState *)(path_offset + (actor->index * 0x23C) + (u8 *)states))->z;
                if (var_v0_9 < 0)
                {
                    var_v0_9 += 0xFF;
                }
                *z_cursor = (s16)(var_v0_9 >> 8);
                work[0] = *cursor << 8;
                cursor += 2;
                work[2] = *z_cursor << 8;
                heading = func_8008D104(actor, target);
                remaining -= 1;
                temp_v1 = point_index + heading_offset;
                point_index += 1;
                *(temp_v1 + D_8010CFE0) = heading;
            }
            else
            {
                var_v1_2 = work[0];
                if (var_v1_2 < 0)
                {
                    var_v1_2 += 0xFF;
                }
                var_v0_10 = work[8] * spacing;
                if (var_v0_10 < 0)
                {
                    var_v0_10 += 0xFFF;
                }
                temp_v0_3 = (var_v1_2 >> 8) + (var_v0_10 >> 0xC);
                *cursor = temp_v0_3;
                var_v1_3 = work[2];
                work[0] = (s32)(temp_v0_3 << 0x10) >> 8;
                if (var_v1_3 < 0)
                {
                    var_v1_3 += 0xFF;
                }
                var_v0_11 = work[10] * spacing;
                if (var_v0_11 < 0)
                {
                    var_v0_11 += 0xFFF;
                }
                temp_v0_4 = (var_v1_3 >> 8) + (var_v0_11 >> 0xC);
                *z_cursor = temp_v0_4;
                z_cursor += 2;
                cursor += 2;
                work[2] = (s32)(temp_v0_4 << 0x10) >> 8;
                heading = func_8008D104(actor, target);
                remaining -= 1;
                temp_v1_2 = point_index + heading_offset;
                point_index += 1;
                *(temp_v1_2 + D_8010CFE0) = heading;
                if (remaining != 0)
                {
                    goto loop_22;
                }
            }
            sample_path_index += 1;
            if (remaining != 0)
            {
                path_offset = sample_path_index * 8;
                if (sample_path_index < (s32)states[actor->index].count)
                {
                    goto loop_21;
                }
            }
        }
    }
}

/**
 * @brief Maps the heading from @p b to @p a onto a direction-table entry.
 *
 * Computes the ratan2 angle between the two points, quantizes it to a 256-step
 * heading wrapped into [0, 0x100), and returns D_800EB0A4[heading >> 5] plus 10.
 *
 * @param a Destination point whose heading is measured.
 * @param b Source point.
 * @return Direction-table entry for the quantized heading, offset by 10.
 */
s32 func_8008D104(PointD104 *a, PointD104 *b)
{
    extern s32 D_800EB0A4[];

    s32 angle;
    s32 idx;

    angle = ratan2(a->unk8 - b->unk8, b->unk0 - a->unk0) >> 4;
    idx = angle + 0x10;
    if (idx < 0)
    {
        idx = angle + 0x110;
    }
    if (idx >= 0x100)
    {
        idx -= 0x100;
    }
    return D_800EB0A4[idx >> 5] + 0xA;
}

/**
 * @brief Update an actor's position history when its rounded position changes.
 * @param record Actor position and status record to sample.
 */
void func_8008D174(FieldPositionRecord* record)
{
    typedef struct
    {
        s16 x;
        s16 y;
    } FieldPosition16;

    typedef struct
    {
        u8 pad0[0x128];
        FieldPosition16 position;
        u8 pad12C[0x23C - 0x12C];
    } FieldActorState;

    typedef struct
    {
        FieldPosition16 entries[FIELD_POSITION_HISTORY_LENGTH];
        u8 pad[0x23C - FIELD_POSITION_HISTORY_LENGTH * sizeof(FieldPosition16)];
    } FieldPositionHistory;

    extern FieldActorState D_80105AE0[];
    extern FieldPositionHistory D_80105B4C[];
    extern u8 D_8010CFE0[];

    s32 x;
    s32 y;
    s32 index;
    s32 value;
    FieldPosition16* history;
    u8* status_history;

    x = record->x;
    if (x < 0)
    {
        x += FIELD_FIXED_POINT_ROUND_BIAS;
    }
    if ((x >> FIELD_FIXED_POINT_SHIFT) == D_80105AE0[record->actor_index].position.x)
    {
        y = record->y;
        if (y < 0)
        {
            y += FIELD_FIXED_POINT_ROUND_BIAS;
        }
        if ((y >> FIELD_FIXED_POINT_SHIFT) == D_80105AE0[record->actor_index].position.y)
        {
            return;
        }
    }

    index = 0;
    history = D_80105B4C[record->actor_index].entries;
    do
    {
        index++;
        *(s32*)history = *(s32*)(history + 1);
        history++;
    } while (index < FIELD_POSITION_HISTORY_LENGTH - 1);

    value = record->x;
    if (value < 0)
    {
        value += FIELD_FIXED_POINT_ROUND_BIAS;
    }
    history->x = (s16)(value >> FIELD_FIXED_POINT_SHIFT);

    value = record->y;
    if (value < 0)
    {
        value += FIELD_FIXED_POINT_ROUND_BIAS;
    }
    history->y = (s16)(value >> FIELD_FIXED_POINT_SHIFT);

    if (record->actor_index == 0)
    {
        status_history = D_8010CFE0;
        index = 0;
        do
        {
            index++;
            status_history[0] = status_history[1];
            status_history++;
        } while (index < FIELD_POSITION_HISTORY_LENGTH - 1);
        *status_history = record->status;
    }
}

/**
 * @brief Follow the active leader's recorded path and update collision and animation.
 *
 * Selects the first eligible leader, adjusts the follower's history index, and
 * measures movement with the GTE. Collision resolution updates the follower's
 * height and cached surface; horizontal displacement selects its animation.
 *
 * @param actor Follower actor whose position and state are updated.
 * @param follower_index Follower order controlling the permitted path separation.
 * @see decomp.me (100%)
 */
void func_8008D29C(Actor *actor, s32 follower_index)
{
    /** @brief One recorded horizontal position in whole world units. */
    typedef struct
    {
        s16 x, z;
    } Point;

    /** @brief Partial 0x23C-byte slot containing 48 recorded positions and collision state. */
    typedef struct
    {
        u8 pad0[0x6c];
        Point points[48];
        u8 pad12c[0x16e - 0x12c];
        u8 unk16E;
        u8 pad16f[7];
        s16 unk176;
        u8 pad178[0x24];
        s32 unk19C, unk1A0;
        u8 pad1a4[0x98];
    } Slot;

    /** @brief Twenty-byte resource descriptor exposing the behavior flags. */
    typedef struct
    {
        u8 pad0[0x10];
        s32 flags;
    } Resource;

    /** @brief Partial 0x48-byte appearance descriptor with the collision-size selector. */
    typedef struct
    {
        u8 pad0[0x2e];
        u8 unk2E;
        u8 pad2f[0x19];
    } Appearance;

    /** @brief Map dimensions at the fixed field geometry address. */
    typedef struct
    {
        s16 width;
        u16 height;
    } Dimensions;

    /** @brief Scratchpad collision mover with overlapping radius and flag storage. */
    typedef struct
    {
        s32 x, y, z, dx, dy, dz, ground, surface, state;
        s16 radius, height;
        union
        {
            s32 word;
            struct
            {
                s16 radius_z;
                u16 flag0 : 1;
                u16 flag1 : 1;
                u16 rest : 14;
            } parts;
        } flags;
    } Mover;

    extern Actor D_800FDF58[];
    extern Slot D_80105AE0[];
    extern Resource g_field_resource_entries[];
    extern Appearance D_800FE3A0[];
    extern u8 D_800EB20C[], D_8010CFE0[], D_8010AE84;
    void func_8008EF0C(Actor *);
    void field_restart_actor_animation(Actor *);
    s32 func_8005B6AC(Mover *);

    VECTOR square;
    VECTOR delta;
    Dimensions *dimensions = (Dimensions *)0x801ED400;
    Mover *mover = (Mover *)0x1F800000;
    Actor *scan;
    s16 decay_period;
    s32 limit;
    s32 position_z;
    s32 animation_kind;
    s32 actor_x;
    s32 advance_dx;
    s32 retreat_dx;
    s32 position_x;
    s32 leader_index;
    s32 dz;
    s32 dx;
    s32 leader_dx;
    s32 leader_dz;
    s32 decay;
    u8 advance_index;
    s32 sample_index;
    u8 recorded_animation;
    u8 old_animation;
    u8 animation;
    Slot *sample_base;
    Actor *leader;
    Slot *advance_slot;
    Slot *retreat_slot;
    s32 collision_height;
    u8 collision_flag;

    /* The leader search inspects only the low half of the actor flags. */
    for (leader_index = 0; leader_index < 13; leader_index++)
    {
        scan = &D_800FDF58[leader_index];
        if (scan->unk25 != 255 && !(*(u16 *)&scan->unk1C & 0x1FF))
        {
            break;
        }
    }
    if (leader_index == 0xD)
    {
        leader_index = 0;
    }
    delta.vy = 0;
    delta.vz = 0;
    delta.vx = 0;
    decay_period = actor->unk16;
    leader = &D_800FDF58[leader_index];
    if (decay_period != 0)
    {
        decay = (s8)actor->unk36 / decay_period;
    }
    else
    {
        decay = 0;
    }
    if (decay != 0)
    {
        actor->unk36 = (s8)((u8)actor->unk36 - decay);
    }
    if (actor->unk2A != 0)
    {
        func_8008EF0C(actor);
        return;
    }
    actor_x = actor->unk0;
    {
        Slot *base = D_80105AE0;
        Slot *slot = &base[actor->unk3A];
        sample_base = (Slot *)((s32)D_80105AE0 + (leader->unk3A * 0x8F + slot->unk16E) * 4);
    }
    if (sample_base->points[0].x != actor_x / 256 || sample_base->points[0].z != actor->unk8 / 256)
    {
        limit = 0x18;
        if (follower_index != 0)
        {
            limit = 0;
        }
        {
            Slot *base = D_80105AE0;
            retreat_slot = &base[actor->unk3A];
        }
        sample_index = retreat_slot->unk16E;
        dx = 0;
        if (limit < (s32)sample_index)
        {
            dz = dx;
            retreat_slot->unk16E = (u8)(sample_index - 1);
            delta.vx = 0;
            delta.vz = 0;
            goto update_collision;
        }
        else
        {
            retreat_dx = (D_80105AE0[leader->unk3A].points[sample_index].x << 8) - actor->unk0;
            delta.vx = retreat_dx;
            dx = retreat_dx;
            delta.vz = (D_80105AE0[leader->unk3A].points[D_80105AE0[actor->unk3A].unk16E].z << 8) -
                       actor->unk8;
            dz = delta.vz;
            gte_ldlvl(&delta);
            gte_sqr12();
            gte_stlvnl(&square);
            if ((square.vx + square.vz) >= 0x181)
            {
                if (g_field_resource_entries[actor->unk3B].flags & 1)
                {
                    actor->unk33 = 0;
                }
                else
                {
                    goto mark_moving;
                }
            }
            else
            {
                goto mark_walking;
            }
        }
    }
    else
    {
        leader_dx = leader->unk0 - actor_x;
        if (leader_dx < 0)
        {
            leader_dx += 0xFF;
        }
        delta.vx = leader_dx >> 8;
        leader_dz = leader->unk8 - actor->unk8;
        if (leader_dz < 0)
        {
            leader_dz += 0xFF;
        }
        delta.vz = leader_dz >> 8;
        gte_ldlvl(&delta);
        gte_sqr0();
        gte_stlvnl(&square);
        dx = 0;
        if ((square.vx + square.vz) > ((follower_index + 1) * 0xBB8))
        {
            advance_slot = &D_80105AE0[actor->unk3A];
            advance_index = advance_slot->unk16E;
            if (advance_index < 0x2FU)
            {
                advance_slot->unk16E = (u8)(advance_index + 1);
                advance_dx =
                    (D_80105AE0[leader->unk3A].points[D_80105AE0[actor->unk3A].unk16E].x << 8) -
                    actor->unk0;
                delta.vx = advance_dx;
                dx = advance_dx;
                delta.vz =
                    (D_80105AE0[leader->unk3A].points[D_80105AE0[actor->unk3A].unk16E].z << 8) -
                    actor->unk8;
                dz = delta.vz;
                gte_ldlvl(&delta);
                gte_sqr12();
                gte_stlvnl(&square);
                if (((square.vx + square.vz) >= 0x181) &&
                    (!(g_field_resource_entries[actor->unk3B].flags & 1)))
                {
                mark_moving:
                    actor->unk33 = 1;
                }
                else
                {
                mark_walking:
                    actor->unk33 = 0;
                }
                collision_height = actor->unk4;
                collision_flag = D_8010AE84;
                goto apply_collision_state;
            }
        }
        dz = dx;
        goto clear_delta;
    }
    collision_height = actor->unk4;
    collision_flag = D_8010AE84;
    goto apply_collision_state;
clear_delta:
    delta.vx = 0;
    delta.vz = 0;
update_collision:
    collision_height = actor->unk4;
    collision_flag = D_8010AE84;
apply_collision_state:
    mover->y = collision_height;
    if (collision_flag == 0)
    {
        position_x = actor->unk0;
        if ((position_x >= 0) && (position_x < (dimensions->width << 8)))
        {
            position_z = actor->unk8;
            if (position_z >= 0)
            {
                if (position_z < ((s32)(dimensions->height << 0x10) >> 7))
                {
                    mover->x = position_x;
                    mover->y = actor->unk4;
                    mover->z = actor->unk8;
                    mover->dx = dx;
                    mover->dy = 0;
                    mover->dz = dz;
                    if (D_800FE3A0[actor->unk3A].unk2E == 0x40)
                    {
                        mover->radius = 0xC;
                        mover->flags.parts.radius_z = 8;
                    }
                    else
                    {
                        mover->radius = 9;
                        mover->flags.parts.radius_z = 6;
                    }
                    mover->height = 0x10;
                    /* Clear the two collision flags without disturbing the radius. */
                    mover->flags.parts.flag1 = 0;
                    mover->flags.parts.flag0 = 0;
                    mover->surface = D_80105AE0[actor->unk3A].unk19C;
                    mover->state = D_80105AE0[actor->unk3A].unk1A0;
                    func_8005B6AC(mover);
                    D_80105AE0[actor->unk3A].unk19C = (s32)mover->surface;
                    D_80105AE0[actor->unk3A].unk1A0 = (s32)mover->state;
                    actor->unk4 = (s32)mover->y;
                    D_80105AE0[actor->unk3A].unk176 = mover->ground / 256;
                }
                else
                {
                    goto clear_collision;
                }
            }
            else
            {
                goto clear_collision;
            }
        }
        else
        {
        clear_collision:
            D_80105AE0[actor->unk3A].unk19C = -1;
            D_80105AE0[actor->unk3A].unk1A0 = 0;
            D_80105AE0[actor->unk3A].unk176 = 0;
        }
    }
    if ((dx | dz) != 0)
    {
        if (g_field_resource_entries[actor->unk3B].flags & 1)
        {
            recorded_animation = D_8010CFE0[D_80105AE0[actor->unk3A].unk16E];
            animation = D_800EB20C[recorded_animation & 0x7F];
            animation |= recorded_animation & 0x80;
        }
        else
        {
            animation = D_8010CFE0[D_80105AE0[actor->unk3A].unk16E];
        }
        if (actor->unk21 != animation)
        {
            actor->unk21 = animation;
            actor->unk27 = 0;
            actor->unk24 = 1;
            field_restart_actor_animation(actor);
        }
    }
    else
    {
        old_animation = actor->unk21;
        animation_kind = old_animation & 0x7F;
        if ((animation_kind < 5) || (g_field_resource_entries[actor->unk3B].flags & 1))
        {
            if ((animation_kind != 0) && (g_field_resource_entries[actor->unk3B].flags & 1))
            {
                actor->unk21 = (u8)(old_animation & 0x80);
                goto restart_animation;
            }
        }
        else
        {
            actor->unk21 = (u8)((animation_kind % 5) | (old_animation & 0x80));
        restart_animation:
            actor->unk27 = 0;
            actor->unk24 = 1;
            field_restart_actor_animation(actor);
        }
    }
    if ((dx | dz) != 0)
    {
        actor->unk0 = (s32)(actor->unk0 + dx);
        actor->unk8 = (s32)(actor->unk8 + dz);
    }
    if ((delta.vx | delta.vz) != 0)
    {
        actor->unk1C = (actor->unk1C & ~0x600) | 0x200;
    }
    else
    {
        actor->unk1C = actor->unk1C & ~0x600;
    }
}
