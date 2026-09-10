#include "common.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
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
void func_8006C3FC(Actor *);
s32 func_8005B6AC(Mover *);
/**
 * @brief Follow the active leader's recorded path and update collision and animation.
 *
 * Selects the first eligible leader, adjusts the follower's history index, and
 * measures movement with the GTE. Collision resolution updates the follower's
 * height and cached surface; horizontal displacement selects its animation.
 *
 * @param actor Follower actor whose position and state are updated.
 * @param follower_index Follower order controlling the permitted path separation.
 */
void func_8008D29C(Actor *actor, s32 follower_index)
{
    VECTOR square;
    VECTOR delta;
    Dimensions *dimensions = (Dimensions *)0x801ED400;
    Mover *mover = (Mover *)0x1F800000;
    Actor *scan;
    s16 decay_period;
    s16 radius_z;
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

    /* The leader search inspects only the low half of the actor flags. */
    for (leader_index = 0, scan = D_800FDF58; leader_index < 13; leader_index++, scan++)
    {
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
    sample_base =
        (Slot *)((s32)D_80105AE0 + (D_80105AE0[actor->unk3A].unk16E + leader->unk3A * 0x8F) * 4);
    if (sample_base->points[0].x != actor_x / 256 || sample_base->points[0].z != actor->unk8 / 256)
    {
        limit = 0x18;
        if (follower_index != 0)
        {
            limit = 0;
        }
        retreat_slot = &D_80105AE0[actor->unk3A];
        sample_index = retreat_slot->unk16E;
        dx = 0;
        if (limit < (s32)sample_index)
        {
            dz = dx;
            retreat_slot->unk16E = (u8)(sample_index - 1);
            goto clear_delta;
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
                /* A signed low-bit test preserves the original separate distance checks. */
                if (((square.vx + square.vz) >= 0x181) &&
                    ((g_field_resource_entries[actor->unk3B].flags << 31) >= 0))
                {
                mark_moving:
                    actor->unk33 = 1;
                }
                else
                {
                mark_walking:
                    actor->unk33 = 0;
                }
                goto update_collision;
            }
        }
        dz = dx;
        goto clear_delta;
    }
    goto update_collision;
clear_delta:
    delta.vx = 0;
    delta.vz = 0;
update_collision:
    mover->y = actor->unk4;
    if (D_8010AE84 == 0)
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
                        radius_z = 8;
                    }
                    else
                    {
                        mover->radius = 9;
                        radius_z = 6;
                    }
                    mover->flags.parts.radius_z = radius_z;
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
            func_8006C3FC(actor);
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
        restart_animation_shared:
            actor->unk27 = 0;
            actor->unk24 = 1;
            func_8006C3FC(actor);
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
