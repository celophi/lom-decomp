#include "wmap_sprite_render.h"
#include "wmap_effect_primitives.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8*)(expr) + (offset)))
#define M2C_UNALIGNED32(expr) (expr)
#define M2C_BITWISE(type, expr) ((type)(expr))

typedef s32 M2C_UNK;
typedef s8 M2C_UNK8;
typedef s16 M2C_UNK16;
typedef s32 M2C_UNK32;

/** @brief Actor animation fields in a 44-byte effect slot. */
typedef struct
{
    u8 pad_00[14];
    s16 sequence;
    s16 previous_sequence;
    u8 pad_12[2];
    u8* cursor;
    u8* sequence_start;
    u8* frame_data;
    s16 remaining;
    u8 pad_22[10];
} WmapActor;

/** @brief Radial particle motion and lifetime. */
typedef struct
{
    s16 state, angle;
    s32 velocity, radius;
    s16 lifetime, z;
    s32 angular_velocity;
} WmapMotion;

/** @brief Resource slot containing an animation-data pointer. */
typedef struct
{
    s32 field_00;
    u8* data;
} WmapResource;

/** @brief Position and velocity records, also used for particle bounds. */
typedef struct
{
    SVECTOR position;
    SVECTOR velocity;
} WmapMovingPoint;

extern s32 D_8011CF74;
extern s32 D_8013B278;
extern u16 D_8013B280;
extern s32 D_8013B284;
extern WmapMotion D_801AFBD0[];
extern s32 D_801B0FD0;
extern SVECTOR D_80139278;
extern SVECTOR D_801398C8;
extern VECTOR D_80182DC0;
extern VECTOR D_80182D48;
extern MATRIX D_8011D0E8;
extern WmapActor D_800D9268[];
extern u16 D_80139980;
extern WmapResource D_80139988[];
extern u8 D_800D9150;
extern u8 D_800DCEA8;
extern u8 D_800DCEB8;
extern s16 D_800DCEBA;
extern s16 D_800DCEBC;
extern u8 D_8011CF4C;
extern s32 D_8011D510;
extern s32 D_8011D530;
extern u8 D_80139200;
extern s32 D_80139204;
extern u16 D_80139210;
extern u16 D_80139212;
extern u16 D_80139214;
extern u8 D_80139950;
extern s32 D_80139968;
extern s32 D_8013996C;
extern s32 D_8013B29C;

s32 rand(void);

/**
 * @brief Update, project, and replenish a configured particle effect.
 * @param arg0 Actor records to update.
 * @param arg1 Address of the resource-slot bytes associated with the actors.
 * @param arg2 Number of actor records to process.
 * @param arg3 Effect parameter; semantics remain unresolved.
 * @param arg4_value Effect parameter stored in a halfword.
 * @param arg5_value Effect parameter stored in a halfword.
 * @param arg6 Particle behavior selector.
 * @param arg7 Particle effect configuration record.
 */
void func_8006A2FC(void* arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4_value, s32 arg5_value, u32 arg6, void* arg7)
{
    u16 arg4 = arg4_value;
    u16 arg5 = arg5_value;
    SVECTOR position;
    s32 sp20;
    M2C_UNK var_s4;
    s16* temp_s2_2;
    s16 temp_v0_6;
    s32 temp_a1;
    s32 temp_lo;
    s32 temp_s0;
    s32 temp_v0_3;
    s32 temp_v0_4;
    s32 temp_v1_2;
    s32 var_s1;
    s32 var_s1_2;
    s32 var_v0;
    u16 temp_v0;
    u16 temp_v0_2;
    u16 temp_v0_5;
    u8 var_v1;
    void* temp_s2;
    void* temp_v0_7;
    void* temp_v0_8;
    void* temp_v1;
    void* var_a0;
    void* var_s0;

    var_s4 = 4;
    var_s1 = 0;
    if (arg2 > 0)
    {
        var_s0 = arg0;
        do
        {
            temp_s2 = ((var_s1 + M2C_FIELD(arg7, s32*, 0x1C)) * 0x14) + (u8*)&D_801AFBD0;
            if (M2C_FIELD(temp_s2, s16*, 0) != 0)
            {
                position.vx = (s16)((s32)(((s32)M2C_FIELD(temp_s2, s32*, 8) >> 3) * (ccos(M2C_FIELD(temp_s2, s16*, 2)) >> 6)) >> 0xC);
                position.vy = (s16)((s32)(((s32)M2C_FIELD(temp_s2, s32*, 8) >> 3) * (csin(M2C_FIELD(temp_s2, s16*, 2)) >> 6)) >> 0xC);
                position.vz = M2C_FIELD(temp_s2, u16*, 0xE);
                gte_ldv0(&position);
                gte_rtps();
                switch (arg6)
                {
                case 5:
                    if (M2C_FIELD(var_s0, s16*, 0x24) >= arg3)
                    {
                        M2C_FIELD(var_s0, s16*, 0x22) = 0;
                    }
                    if ((M2C_FIELD(var_s0, s16*, 0x22) == 0) && (M2C_FIELD(var_s0, s16*, 0x24) < 4))
                    {
                        M2C_FIELD(temp_s2, s16*, 0) = 0;
                    }
                    /* fallthrough */
                case 0:
                    M2C_FIELD(temp_s2, u16*, 0xE) = (u16)(M2C_FIELD(temp_s2, u16*, 0xE) + M2C_FIELD(temp_s2, u16*, 4));
                    temp_v0 = M2C_FIELD(temp_s2, u16*, 0xC) - 1;
                    M2C_FIELD(temp_s2, u16*, 0xC) = temp_v0;
                    M2C_FIELD(temp_s2, s32*, 8) = (s32)(M2C_FIELD(temp_s2, s32*, 8) + M2C_FIELD(arg7, s32*, 0x18));
                    if ((temp_v0 << 0x10) == 0)
                    {
                        M2C_FIELD(temp_s2, s16*, 0) = 0;
                    }
                default:
                block_28:
                    var_v0 = var_s1 * 8;
                    break;
                case 1:
                    if (*(s32*)0x8013B264 < (s16)M2C_FIELD(temp_s2, u16*, 0xC))
                    {
                        M2C_FIELD(temp_s2, u16*, 0xE) = (u16)(M2C_FIELD(temp_s2, u16*, 0xE) + (*(u16*)0x8013B270 + ((s32)(rand() * D_8013B278) >> 0xF)));
                        M2C_FIELD(temp_s2, s32*, 8) = (s32)(M2C_FIELD(temp_s2, s32*, 8) + M2C_FIELD(arg7, s32*, 0x18));
                    }
                    else
                    {
                        temp_v0_2 = M2C_FIELD(temp_s2, u16*, 0xE) - (D_8013B280 + ((s32)(rand() * D_8013B284) >> 0xF));
                        M2C_FIELD(temp_s2, u16*, 0xE) = temp_v0_2;
                        if (temp_v0_2 & 0x8000)
                        {
                            M2C_FIELD(temp_s2, u16*, 0xE) = 0U;
                            M2C_FIELD(var_s0, s16*, 0x22) = 0;
                            M2C_FIELD(var_s0, s16*, 0x26) = 8;
                            if (M2C_FIELD(var_s0, s16*, 0x24) < 4)
                            {
                                M2C_FIELD(temp_s2, s16*, 0) = 0;
                            }
                        }
                    }
                    var_s4 = 0xC;
                    if ((u32)((u16)M2C_FIELD(temp_s2, s16*, 2) - 1) < 0x7FFU)
                    {
                        var_s4 = 0xA;
                    }
                    M2C_FIELD(temp_s2, u16*, 0xC) = (u16)(M2C_FIELD(temp_s2, u16*, 0xC) - 1);
                    goto block_28;
                case 2:
                    temp_v0_3 = (s16)M2C_FIELD(temp_s2, u16*, 0xC) - *(s32*)0x8013B264;
                    M2C_FIELD(temp_s2, u16*, 0xE) = (u16)(D_8013B280 + (*(u16*)0x8013B270 - ((s32)(temp_v0_3 * temp_v0_3) / (s32)D_8013B278)));
                    if ((s16)M2C_FIELD(temp_s2, u16*, 0xC) != 0)
                    {
                        M2C_FIELD(temp_s2, u16*, 0xC) = (u16)(M2C_FIELD(temp_s2, u16*, 0xC) - 1);
                        M2C_FIELD(temp_s2, s32*, 8) = (s32)(M2C_FIELD(temp_s2, s32*, 8) + M2C_FIELD(arg7, s32*, 0x18));
                    }
                    goto block_28;
                case 3:
                    temp_v0_4 = *(s32*)0x8013B264 - (s16)M2C_FIELD(temp_s2, u16*, 0xC);
                    M2C_FIELD(temp_s2, u16*, 0xE) = (u16)((s32)(temp_v0_4 * temp_v0_4) / (s32) * (u16*)0x8013B270);
                    goto block_28;
                case 4:
                    M2C_FIELD(temp_s2, u16*, 0xE) = (u16)(M2C_FIELD(temp_s2, u16*, 0xE) + M2C_FIELD(temp_s2, u16*, 4));
                    temp_v0_5 = M2C_FIELD(temp_s2, u16*, 0xC) - 1;
                    M2C_FIELD(temp_s2, u16*, 0xC) = temp_v0_5;
                    M2C_FIELD(temp_s2, s32*, 8) = (s32)(M2C_FIELD(temp_s2, s32*, 8) + M2C_FIELD(arg7, s32*, 0x18));
                    if ((temp_v0_5 << 0x10) == 0)
                    {
                        M2C_FIELD(temp_s2, s16*, 0) = 0;
                    }
                    goto block_28;
                case 7:
                    M2C_FIELD(temp_s2, u16*, 0xE) = (u16)(M2C_FIELD(temp_s2, u16*, 0xE) + M2C_FIELD(temp_s2, u16*, 4));
                    M2C_FIELD(temp_s2, s16*, 2) = (s16)((u16)M2C_FIELD(temp_s2, s16*, 2) + M2C_FIELD(arg7, u16*, 0x10));
                    M2C_FIELD(temp_s2, s32*, 8) = (s32)(M2C_FIELD(temp_s2, s32*, 8) + M2C_FIELD(arg7, s32*, 0x18));
                    /* fallthrough */
                case 6:
                    var_v0 = var_s1 * 8;
                    if ((s16)M2C_FIELD(temp_s2, u16*, 0xE) < 0)
                    {
                        M2C_FIELD(temp_s2, s16*, 0) = 0;
                        goto block_28;
                    }
                    break;
                }
                temp_a1 = M2C_FIELD((var_v0 + arg1), s32*, 4);
                temp_v0_6 = (s16)M2C_FIELD(var_s0, u16*, 0xE);
                if (M2C_FIELD(var_s0, s16*, 0x10) != temp_v0_6)
                {
                    M2C_FIELD(var_s0, s16*, 0x10) = (s16)M2C_FIELD(var_s0, u16*, 0xE);
                    M2C_FIELD(var_s0, s16*, 0x20) = 1;
                    temp_v1 = temp_a1 + *(s16*)((temp_v0_6 * 2) + temp_a1);
                    M2C_FIELD(var_s0, void**, 0x18) = temp_v1;
                    M2C_FIELD(var_s0, void**, 0x14) = temp_v1;
                }
                if (M2C_FIELD(var_s0, s16*, 0x20) != 0xFF)
                {
                    M2C_FIELD(var_s0, s16*, 0x20) = (s16)((u16)M2C_FIELD(var_s0, s16*, 0x20) - 1);
                }
                if (M2C_FIELD(var_s0, s16*, 0x20) == 0)
                {
                    temp_v0_7 = M2C_FIELD(var_s0, void**, 0x14);
                    var_v1 = M2C_FIELD(temp_v0_7, u8*, 0);
                    M2C_FIELD(var_s0, s16*, 0x20) = (s16)M2C_FIELD(temp_v0_7, u8*, 1);
                    if (var_v1 == 0xFF)
                    {
                        temp_v0_8 = M2C_FIELD(var_s0, void**, 0x18);
                        M2C_FIELD(var_s0, void**, 0x14) = temp_v0_8;
                        var_v1 = M2C_FIELD(temp_v0_8, u8*, 0);
                        M2C_FIELD(var_s0, s16*, 0x20) = (s16)M2C_FIELD(temp_v0_8, u8*, 1);
                    }
                    M2C_FIELD(var_s0, void**, 0x14) = (void*)(M2C_FIELD(var_s0, void**, 0x14) + 4);
                    M2C_FIELD(var_s0, s32*, 0x1C) = (s32)(temp_a1 + M2C_FIELD(((var_v1 * 2) + temp_a1), s16*, 0x40));
                }
                gte_stsxy(&sp20);
                func_80066F9C(var_s0, sp20, M2C_FIELD(arg7, s32*, 0x20), var_s4, 0);
            }
            var_s1 += 1;
            var_s0 += 0x2C;
        } while (var_s1 < arg2);
    }
    temp_v1_2 = M2C_FIELD(arg7, s32*, 0x14);
    if ((temp_v1_2 != -1) && (((s32)D_8011CF74 % temp_v1_2) == 0) && (var_s1_2 = 0, (arg2 > 0)))
    {
        var_a0 = arg0;
    loop_43:
        temp_s2_2 = ((var_s1_2 + M2C_FIELD(arg7, s32*, 0x1C)) * 0x14) + (u8*)&D_801AFBD0;
        var_s1_2 += 1;
        if (M2C_FIELD(temp_s2_2, s16*, 0) == 0)
        {
            M2C_FIELD(var_a0, s16*, 2) = 0;
            M2C_FIELD(var_a0, s8*, 6) = 0xF;
            M2C_FIELD(var_a0, s16*, 0x10) = -1;
            M2C_FIELD(var_a0, u16*, 0x22) = (u16)arg3;
            M2C_FIELD(var_a0, u16*, 0x24) = arg4;
            M2C_FIELD(var_a0, u16*, 0xE) = (u16)M2C_FIELD(arg7, u16*, 0x24);
            M2C_FIELD(var_a0, u16*, 0x26) = arg5;
            M2C_FIELD(temp_s2_2, s16*, 0) = 1;
            M2C_FIELD(temp_s2_2, s16*, 2) = rand();
            temp_s0 = M2C_FIELD(arg7, s32*, 0x28);
            if (temp_s0 < 0)
            {
                M2C_FIELD(temp_s2_2, s32*, 8) = (s32)(((s32)(rand() * ((temp_s0 & 0xFFFF) << 5)) >> 0xF) + ((s32)(temp_s0 & 0x7FFF0000) >> 0xE));
            }
            else
            {
                M2C_FIELD(temp_s2_2, s32*, 8) = temp_s0;
            }
            M2C_FIELD(temp_s2_2, s32*, 4) = (s32)(((s32)(rand() * M2C_FIELD(arg7, s32*, 8)) >> 0xF) + M2C_FIELD(arg7, s32*, 4));
            temp_lo = rand() * (s32)M2C_FIELD(arg7, u16*, 0x10);
            M2C_FIELD(temp_s2_2, s16*, 0xE) = 0;
            M2C_FIELD(temp_s2_2, s16*, 0xC) = (s16)(M2C_FIELD(arg7, u16*, 0xC) + (temp_lo >> 0xF));
            return;
        }
        var_a0 += 0x2C;
        if (var_s1_2 >= arg2)
        {
        }
        else
        {
            goto loop_43;
        }
    }
}

/**
 * @brief Project radial particles and periodically spawn one in a free slot.
 * @param actor_address Address of the 44-byte actor records.
 * @param resource_address Address of their animation resource records.
 * @param first First particle index.
 * @param end Exclusive last particle index.
 * @param scale Size used for the projected actor.
 * @param velocity_min Minimum radial velocity.
 * @param velocity_range Range of randomized radial velocity.
 * @param lifetime_min Minimum particle lifetime.
 * @param lifetime_range Range of randomized lifetime.
 * @param initial_z Initial depth divisor.
 * @param frame Sprite frame passed to the renderer.
 * @param spawn_interval Number of updates between spawn attempts.
 */
void func_8006A9C4(s32 actor_address, s32 resource_address, s32 first, s32 end, s32 scale,
                   s32 velocity_min, s32 velocity_range, s32 lifetime_min, s32 lifetime_range, s32 initial_z,
                   s32 frame, s32 spawn_interval)
{
    SVECTOR position;
    s32 screen_position;
    s32 sequence_offset;
    s32 animation_address;
    s32 lifetime_product;
    s32 scaled_size;
    s32 i;
    s32 active;
    s32 sequence_end;
    u16 remaining_lifetime;
    s32 animation_frame;
    void *cursor;
    void *sequence_start;
    void *sequence_cursor;
    void *spawn_actor;
    void *actor;
    void *spawn_motion;
    void *motion;
    void *resource;
    WmapMotion *motion_base;

    active = 0;
    i = first;
    if (i < end)
    {
        sequence_end = 255;
        for (; i < end; i++)
        {
            resource = (i * 8) + resource_address;
            actor = (i * 0x2C) + actor_address;
            motion_base = D_801AFBD0;
            motion = &motion_base[i];
            if (M2C_FIELD(motion, s16 *, 0) != 0)
            {
                position.vx = (s16) ((s32) (((s32) M2C_FIELD(motion, s32 *, 8) >> 6) * (ccos(M2C_FIELD(motion, s16 *, 2)) >> 6)) >> 0xC);
                position.vy = (s16) ((s32) (((s32) M2C_FIELD(motion, s32 *, 8) >> 6) * (csin(M2C_FIELD(motion, s16 *, 2)) >> 6)) >> 0xC);
                position.vz = (s16) (0xC8 / (s16) M2C_FIELD(motion, s16 *, 0xE));
                gte_ldv0(&position);
                gte_rtps();
                scaled_size = scale * 0x81;
                M2C_FIELD(motion, s16 *, 0xE) = (s16) ((u16) M2C_FIELD(motion, s16 *, 0xE) + 1);
                M2C_FIELD(motion, s32 *, 8) = (s32) (M2C_FIELD(motion, s32 *, 8) + M2C_FIELD(motion, s32 *, 4));
                M2C_FIELD(actor, s16 *, 0x22) = 0;
                if (scaled_size < 0)
                {
                    scaled_size += 0xFF;
                }
                M2C_FIELD(actor, s16 *, 0x24) = (s16) (scaled_size >> 8);
                gte_stsxy(&screen_position);
                animation_address = M2C_FIELD(resource, s32 *, 4);
                if (M2C_FIELD(actor, s16 *, 0x10) != M2C_FIELD(actor, s16 *, 0xE))
                {
                    M2C_FIELD(actor, s16 *, 0x10) = (s16) (u16) M2C_FIELD(actor, s16 *, 0xE);
                    sequence_offset = *(s16 *)((M2C_FIELD(actor, s16 *, 0xE) * 2) + animation_address);
                    M2C_FIELD(actor, s16 *, 0x20) = 1;
                    sequence_cursor = animation_address + sequence_offset;
                    M2C_FIELD(actor, void **, 0x18) = sequence_cursor;
                    M2C_FIELD(actor, void **, 0x14) = sequence_cursor;
                }
                if (M2C_FIELD(actor, s16 *, 0x20) != sequence_end)
                {
                    M2C_FIELD(actor, s16 *, 0x20) = (s16) ((u16) M2C_FIELD(actor, s16 *, 0x20) - 1);
                }
                if (M2C_FIELD(actor, s16 *, 0x20) == 0)
                {
                    cursor = M2C_FIELD(actor, void **, 0x14);
                    animation_frame = M2C_FIELD(cursor, u8 *, 0);
                    M2C_FIELD(actor, s16 *, 0x20) = (s16) M2C_FIELD(cursor, u8 *, 1);
                    if (animation_frame == sequence_end)
                    {
                        sequence_start = M2C_FIELD(actor, void **, 0x18);
                        M2C_FIELD(actor, void **, 0x14) = sequence_start;
                        animation_frame = M2C_FIELD(sequence_start, u8 *, 0);
                        M2C_FIELD(actor, s16 *, 0x20) = (s16) M2C_FIELD(sequence_start, u8 *, 1);
                    }
                    M2C_FIELD(actor, void **, 0x14) = (void *) (M2C_FIELD(actor, void **, 0x14) + 4);
                    M2C_FIELD(actor, s32 *, 0x1C) = (s32) (animation_address + M2C_FIELD(((animation_frame * 2) + animation_address), s16 *, 0x40));
                }
                func_80066F9C(actor, screen_position, frame, 0xA, 0);
                remaining_lifetime = M2C_FIELD(motion, u16 *, 0xC) - 1;
                M2C_FIELD(motion, u16 *, 0xC) = remaining_lifetime;
                if ((remaining_lifetime << 0x10) == 0)
                {
                    M2C_FIELD(motion, s16 *, 0) = 0;
                }
                active += 1;
            }
        }
    }
    if (D_8011CF74 % spawn_interval == 0)
    {
        for (i = first; i < end; i++)
        {
            spawn_actor = (i * 0x2C) + actor_address;
            spawn_motion = (i * 0x14) + (u8 *)&D_801AFBD0;
            if (M2C_FIELD(spawn_motion, s16 *, 0) == 0)
            {
                if (D_801B0FD0 >= active)
                {
                    M2C_FIELD(spawn_actor, s8 *, 6) = 0xF;
                    M2C_FIELD(spawn_actor, s16 *, 2) = 0;
                    M2C_FIELD(spawn_actor, s16 *, 0xE) = 1;
                    M2C_FIELD(spawn_actor, s16 *, 0x10) = -1;
                    M2C_FIELD(spawn_motion, s16 *, 0) = 1;
                    M2C_FIELD(spawn_motion, s16 *, 2) = (s16) ((u32) rand() >> 3);
                    M2C_FIELD(spawn_motion, s32 *, 8) = 0;
                    M2C_FIELD(spawn_motion, s32 *, 4) = (s32) (((s32) (rand() * velocity_range) >> 0xF) + velocity_min);
                    lifetime_product = rand() * lifetime_range;
                    M2C_FIELD(spawn_motion, u16 *, 0xE) = initial_z;
                    M2C_FIELD(spawn_motion, s16 *, 0xC) = (s16) ((lifetime_product >> 0xF) + lifetime_min);
                }
                break;
            }
        }
    }
}

/**
 * @brief Combine global and local transforms and install the resulting matrix.
 * @param translation Local translation applied to the base matrix.
 * @param rotation Local rotation composed with the base matrix.
 */
void func_8006ADD0(VECTOR* translation, SVECTOR* rotation)
{
    MATRIX base;
    MATRIX effect;
    SVECTOR combined_rotation;
    VECTOR combined_translation;

    combined_rotation.vx = D_80139278.vx + D_801398C8.vx;
    combined_rotation.vy = D_80139278.vy + D_801398C8.vy;
    combined_rotation.vz = D_80139278.vz + D_801398C8.vz;
    combined_translation.vx = D_80182DC0.vx + D_80182D48.vx;
    combined_translation.vy = D_80182DC0.vy + D_80182D48.vy;
    combined_translation.vz = D_80182DC0.vz + D_80182D48.vz;
    RotMatrix(&combined_rotation, &base);
    TransMatrix(&base, translation);
    SetRotMatrix(&base);
    SetTransMatrix(&base);
    RotMatrix(rotation, &effect);
    CompMatrix(&base, &effect, &effect);
    TransMatrix(&effect, &combined_translation);
    SetRotMatrix(&effect);
    SetTransMatrix(&effect);
}

/** @brief Combine the base and effect transforms and install the resulting matrix. */
void func_8006AEE0(void)
{
    SVECTOR rotation;
    VECTOR translation;

    rotation.vx = D_80139278.vx + D_801398C8.vx;
    rotation.vy = D_80139278.vy + D_801398C8.vy;
    rotation.vz = D_80139278.vz + D_801398C8.vz;
    translation.vx = D_80182DC0.vx + D_80182D48.vx;
    translation.vy = D_80182DC0.vy + D_80182D48.vy;
    translation.vz = D_80182DC0.vz + D_80182D48.vz;
    RotMatrix(&rotation, &D_8011D0E8);
    TransMatrix(&D_8011D0E8, &translation);
    SetRotMatrix(&D_8011D0E8);
    SetTransMatrix(&D_8011D0E8);
}

/**
 * @brief Update radial particles and replenish inactive entries.
 * @param arg0 Effect parameter; semantics remain unresolved.
 * @param arg1 Effect parameter; semantics remain unresolved.
 * @param arg2 Effect parameter; semantics remain unresolved.
 * @param arg3 Effect parameter; semantics remain unresolved.
 * @param arg4 Effect parameter; semantics remain unresolved.
 * @param arg5 Effect parameter; semantics remain unresolved.
 * @param arg6 Effect parameter; semantics remain unresolved.
 * @param arg7 Effect parameter; semantics remain unresolved.
 */
/**
 * @brief Advance radial particles, then refill inactive slots up to the active limit.
 * The spawn limit is tested before counting the next candidate slot.
 */
void func_8006AFAC(s32 first, s32 end, s32 frame, s32 depth,
                   s32 velocity_base, s32 velocity_divisor, s32 lifetime_base, s32 sequence)
{
    SVECTOR position;
    s32 screen_position;
    s32 resource_offset;
    s32 offset;
    s32 animation_frame;
    s16 *offsets;
    u8 *data;
    u8 *cursor;
    s32 y_product;
    s32 i;
    s32 active;
    s32 actor_offset;
    WmapActor *actor;
    WmapMotion *spawn_motion;
    WmapMotion *motion;
    WmapMotion *motion_base;
    WmapActor *spawn_actor;

    active = 0;
    for (i = first; i < end; i++)
    {
        actor_offset = i * 44;
        motion_base = D_801AFBD0;
        motion = &motion_base[i];
        if (motion->state != 0)
        {
            actor = (WmapActor *)((u8 *)D_800D9268 + actor_offset);
            position.vx = (s16) ((s32) (((s32) motion->radius >> 6) * (ccos(motion->angle) >> 6)) >> 0xC);
            y_product = ((s32) motion->radius >> 6) * (csin(motion->angle) >> 6);
            position.vz = 0;
            position.vy = (s16) (y_product >> 0xC);
            gte_ldv0(&position);
            gte_rtps();
            motion->radius = (s32) (motion->radius + motion->velocity);
            M2C_FIELD(actor, u16 *, 0x22) = (u16) D_80139980;
            M2C_FIELD(actor, u16 *, 0x24) = (u16) D_80139980;
            gte_stsxy(&screen_position);
            resource_offset = i * 8;
            offsets = (s16 *)((WmapResource *)((u8 *)D_80139988 + resource_offset))->data;
            data = (u8 *)offsets;
            if (actor->previous_sequence != actor->sequence)
            {
                u8 *sequence_cursor;
                actor->previous_sequence = (u16)actor->sequence;
                offset = offsets[actor->sequence];
                actor->remaining = 1;
                sequence_cursor = (u8 *)offsets + offset;
                actor->sequence_start = sequence_cursor;
                actor->cursor = sequence_cursor;
            }
            if (actor->remaining != 255)
            {
                actor->remaining--;
            }
            if (actor->remaining == 0)
            {
                cursor = actor->cursor;
                animation_frame = cursor[0];
                actor->remaining = cursor[1];
                if (animation_frame == 255)
                {
                    cursor = actor->sequence_start;
                    actor->cursor = cursor;
                    animation_frame = cursor[0];
                    actor->remaining = cursor[1];
                }
                actor->cursor += 4;
                actor->frame_data = data + ((s16 *)(animation_frame * 2 + data))[32];
            }
            func_80066F9C(actor, screen_position, frame, depth, 0);
            motion->lifetime--;
            if (motion->lifetime == 0)
            {
                motion->state = 0;
            }
            active += 1;
        }
    }
    for (i = first; i < end; i++)
    {
        spawn_actor = &D_800D9268[i];
        spawn_motion = &D_801AFBD0[i];
        if (spawn_motion->state == 0)
        {
            if (active++ > D_801B0FD0)
            {
                break;
            }
            M2C_FIELD(spawn_actor, s16 *, 2) = 0;
            M2C_FIELD(spawn_actor, s8 *, 6) = 0xF;
            spawn_actor->sequence = sequence;
            spawn_actor->previous_sequence = -1;
            spawn_motion->state = 1;
            spawn_motion->angle = (s16) ((u32)rand() >> 3);
            spawn_motion->radius = 0;
            spawn_motion->velocity = (s32) (velocity_base + (rand() / velocity_divisor));
            spawn_motion->lifetime = (s16) (lifetime_base + (rand() & 0x3C));
        }
    }
}

/**
 * @brief Update falling particles and refill inactive slots at a timed spawn rate.
 * Incoming values occupy 32-bit argument slots, including values stored in halfwords.
 */
void func_8006B328(s32 first, s32 end, s32 spawn_interval, s32 scale_override, s32 velocity_min, s32 velocity_range, 
                   s32 initial_z, s32 frame, s32 x_min, s32 x_range, s32 y_min, s32 y_range, s32 fade_z, 
                   s32 initial_scale_target, s32 initial_scale, s32 scale_step, s32 group)
{
    SVECTOR position;
    s32 screen_position;
    s32 sequence_offset;
    s32 resource_offset;
    s32 *spawn_count;
    s32 *spawn_timer;
    s32 *initial_timer;
    s32 *final_timer;
    u8 *timer_base;
    s32 timer_offset;
    s32 address;
    s32 remaining_spawns;
    s32 i;
    s32 motion_offset;
    s32 actor_offset;
    u16 next_z;
    s32 animation_frame;
    s32 sequence_end;
    void *actor;
    void *motion;
    void *cursor;
    void *sequence_start;
    void *sequence_cursor;

    address = group * 4;
    initial_timer = address + (u8 *)&D_800D9150;
    i = first;
    *initial_timer -= 1;
    if (i < end)
    {
        sequence_end = 255;
        timer_offset = address;
        actor_offset = i * 0x2C;
        motion_offset = i * 0x14;
        do
        {
            motion = motion_offset + (u8 *)&D_801AFBD0;
            actor = actor_offset + (u8 *)&D_800D9268;
            if (M2C_FIELD(motion, s16 *, 0) != 0)
            {
                if (scale_override != -1)
                {
                    M2C_FIELD(actor, s16 *, 0x24) = scale_override;
                    M2C_FIELD(actor, s16 *, 0x22) = scale_override;
                }
                position.vx = M2C_FIELD(motion, u16 *, 0x10);
                position.vy = M2C_FIELD(motion, u16 *, 0x12);
                position.vz = M2C_FIELD(motion, u16 *, 0xE);
                gte_ldv0(&position);
                gte_rtps();
                resource_offset = i * sizeof(WmapResource);
                address = (s32)((WmapResource *)((u8 *)D_80139988 + resource_offset))->data;
                if (M2C_FIELD(actor, s16 *, 0x10) != M2C_FIELD(actor, s16 *, 0xE))
                {
                    M2C_FIELD(actor, s16 *, 0x10) = (s16) (u16) M2C_FIELD(actor, s16 *, 0xE);
                    sequence_offset = *(s16 *)((M2C_FIELD(actor, s16 *, 0xE) * 2) + address);
                    M2C_FIELD(actor, s16 *, 0x20) = 1;
                    sequence_cursor = address + sequence_offset;
                    M2C_FIELD(actor, void **, 0x18) = sequence_cursor;
                    M2C_FIELD(actor, void **, 0x14) = sequence_cursor;
                }
                if (M2C_FIELD(actor, s16 *, 0x20) != sequence_end)
                {
                    M2C_FIELD(actor, s16 *, 0x20) = (s16) ((u16) M2C_FIELD(actor, s16 *, 0x20) - 1);
                }
                if (M2C_FIELD(actor, s16 *, 0x20) == 0)
                {
                    cursor = M2C_FIELD(actor, void **, 0x14);
                    animation_frame = M2C_FIELD(cursor, u8 *, 0);
                    M2C_FIELD(actor, s16 *, 0x20) = (s16) M2C_FIELD(cursor, u8 *, 1);
                    if (animation_frame == sequence_end)
                    {
                        sequence_start = M2C_FIELD(actor, void **, 0x18);
                        M2C_FIELD(actor, void **, 0x14) = sequence_start;
                        animation_frame = M2C_FIELD(sequence_start, u8 *, 0);
                        M2C_FIELD(actor, s16 *, 0x20) = (s16) M2C_FIELD(sequence_start, u8 *, 1);
                    }
                    M2C_FIELD(actor, void **, 0x14) = (void *) (M2C_FIELD(actor, void **, 0x14) + 4);
                    M2C_FIELD(actor, s32 *, 0x1C) = (s32) (address + M2C_FIELD(((animation_frame * 2) + address), s16 *, 0x40));
                }
                gte_stsxy(&screen_position);
                func_80066F9C(actor, screen_position, frame, 4, 0);
                next_z = M2C_FIELD(motion, u16 *, 0xE) - M2C_FIELD(motion, u16 *, 4);
                M2C_FIELD(motion, u16 *, 0xE) = next_z;
                if ((s16) next_z < fade_z)
                {
                    M2C_FIELD(actor, s16 *, 0x22) = 0;
                }
                if ((s16) M2C_FIELD(motion, u16 *, 0xE) < 0)
                {
                    M2C_FIELD(motion, u16 *, 0xE) = 0U;
                }
                if ((M2C_FIELD(actor, s16 *, 0x22) == 0) && (M2C_FIELD(actor, s16 *, 0x24) < 5))
                {
                    M2C_FIELD(motion, s16 *, 0) = 0;
                }
            }
            else
            {
                spawn_timer = timer_offset + (u8 *)&D_800D9150;
                if (*spawn_timer == 0)
                {
                    spawn_count = timer_offset + (u8 *)&D_800DCEA8;
                    remaining_spawns = *spawn_count;
                    if (remaining_spawns != 0)
                    {
                        if (remaining_spawns == 1)
                        {
                            *spawn_timer = spawn_interval;
                        }
                        else
                        {
                            *spawn_count = remaining_spawns - 1;
                        }
                        M2C_FIELD(motion, s16 *, 0) = 1;
                        M2C_FIELD(motion, u16 *, 0xE) = initial_z;
                        M2C_FIELD(motion, u16 *, 0x10) = (u16) (((s32) (rand() * x_range) >> 0xF) + x_min);
                        M2C_FIELD(motion, u16 *, 0x12) = (u16) (((s32) (rand() * y_range) >> 0xF) + y_min);
                        M2C_FIELD(motion, s32 *, 4) = (s32) (((s32) (rand() * velocity_range) >> 0xF) + velocity_min);
                        M2C_FIELD(actor, s16 *, 0x22) = (s16) initial_scale_target;
                        M2C_FIELD(actor, s16 *, 0x24) = (s16) initial_scale;
                        M2C_FIELD(actor, u16 *, 0x26) = scale_step;
                    }
                }
            }
            actor_offset += 0x2C;
            i += 1;
            motion_offset += 0x14;
        } while (i < end);
    }
    timer_base = &D_800D9150;
    final_timer = (group * 4) + timer_base;
    if (*final_timer == 0)
    {
        *final_timer = spawn_interval;
    }
}

/**
 * @brief Project moving particles, advance their animation, and draw by depth.
 * @param first First actor index.
 * @param end Exclusive last actor index.
 * @param frame Sprite frame passed to the renderer.
 * @param z_step Per-update change in particle Z.
 * @param depth Base ordering-table depth.
 */
void func_8006B6EC(s32 first, s32 end, s32 frame, s32 z_step, s32 depth)
{
    SVECTOR position;
    s32 screen_position;
    s32 i;
    WmapMotion *motion;
    WmapActor *actor, *actor_base;
    u8 *data;
    u8 *cursor;
    s16 *offsets;
    s32 animation_frame;
    s32 offset, resource_offset;

    for (i = first; i < end; i++)
    {
        motion = &D_801AFBD0[i];
        actor_base = D_800D9268;
        actor = &actor_base[i];
        if (motion->state != 0)
        {
            position.vx = ((motion->radius >> 6) * (ccos(motion->angle) >> 6)) >> 12;
            position.vy = ((motion->radius >> 6) * (csin(motion->angle) >> 6)) >> 12;
            position.vz = motion->z;
            gte_ldv0(&position);
            gte_rtps();
            motion->z += z_step;
            motion->radius += motion->velocity;
            motion->angle = (motion->angle + motion->angular_velocity) & 4095;
            gte_stsxy(&screen_position);
            resource_offset = i * 8;
            offsets = (s16 *)((WmapResource *)((u8 *)D_80139988 + resource_offset))->data;
            data = (u8 *)offsets;
            if (actor->previous_sequence != actor->sequence)
            {
                u8 *sequence_cursor;
                actor->previous_sequence = (u16)actor->sequence;
                offset = offsets[actor->sequence];
                actor->remaining = 1;
                sequence_cursor = (u8 *)offsets + offset;
                actor->sequence_start = sequence_cursor;
                actor->cursor = sequence_cursor;
            }
            if (actor->remaining != 255)
            {
                actor->remaining--;
            }
            if (actor->remaining == 0)
            {
                cursor = actor->cursor;
                animation_frame = cursor[0];
                actor->remaining = cursor[1];
                if (animation_frame == 255)
                {
                    cursor = actor->sequence_start;
                    actor->cursor = cursor;
                    animation_frame = cursor[0];
                    actor->remaining = cursor[1];
                }
                actor->cursor += 4;
                actor->frame_data = data + ((s16 *)(animation_frame * 2 + data))[32];
            }
            if ((u32)((u16)motion->angle - 1025) < 2047U)
            {
                func_80066F9C(actor, screen_position, frame, depth + 2, 0);
            }
            else
            {
                func_80066F9C(actor, screen_position, frame, depth - 2, 0);
            }
            motion->lifetime--;
            if (motion->lifetime == 0)
            {
                motion->state = 0;
            }
        }
    }
}

/**
 * @brief Advance and project animated particles, bouncing at the supplied bounds.
 * @param first First actor index.
 * @param end Exclusive last actor index; this point contains the bounds.
 * @param point_data Packed position/velocity records.
 * @param frame Sprite frame passed to the renderer.
 * @param depth Ordering-table depth.
 */
void func_8006B998(s32 first, s32 end, void *point_data, s32 frame, s32 depth)
{
    WmapMovingPoint *points = point_data;
    s32 screen_position;
    s32 i;
    WmapMovingPoint *point;
    WmapMovingPoint *bounds;
    WmapActor *actor;
    WmapActor *actor_base;
    u8 *data;
    s16 *offsets;
    u8 *cursor;
    s32 offset;
    s32 resource_offset;
    s32 animation_frame;

    point = &points[first];
    bounds = &points[end];
    for (i = first; i < end; point++, i++)
    {
        actor_base = D_800D9268;
        actor = &actor_base[i];
        gte_ldv0(&point->position);
        gte_rtps();
        resource_offset = i * sizeof(WmapResource);
        offsets = (s16 *)((WmapResource *)((u8 *)D_80139988 + resource_offset))->data;
        data = (u8 *)offsets;
        if (actor->previous_sequence != actor->sequence)
        {
            u8 *sequence_cursor;
            actor->previous_sequence = (u16)actor->sequence;
            offset = offsets[actor->sequence];
            actor->remaining = 1;
            sequence_cursor = (u8 *)offsets + offset;
            actor->sequence_start = sequence_cursor;
            actor->cursor = sequence_cursor;
        }
        if (actor->remaining != 255)
        {
            actor->remaining--;
        }
        if (actor->remaining == 0)
        {
            cursor = actor->cursor;
            animation_frame = cursor[0];
            actor->remaining = cursor[1];
            if (animation_frame == 255)
            {
                cursor = actor->sequence_start;
                actor->cursor = cursor;
                animation_frame = cursor[0];
                actor->remaining = cursor[1];
            }
            actor->cursor += 4;
            actor->frame_data = data + ((s16 *)(animation_frame * 2 + data))[32];
        }
        gte_stsxy(&screen_position);
        func_80066F9C(actor, screen_position, frame, depth, 0);
        point->position.vx += point->velocity.vx;
        point->position.vy += point->velocity.vy;
        point->position.vz += point->velocity.vz;
        if (point->position.vx < bounds->position.vx || bounds->velocity.vx < point->position.vx)
        {
            point->velocity.vx = -point->velocity.vx;
        }
        if (point->position.vy < bounds->position.vy || bounds->velocity.vy < point->position.vy)
        {
            point->velocity.vy = -point->velocity.vy;
        }
        if (point->position.vz < bounds->position.vz || bounds->velocity.vz < point->position.vz)
        {
            point->velocity.vz = -point->velocity.vz;
        }
    }
}

/**
 * @brief Advance radial trail heads, copy trail records, and draw active slots.
 * @param first First actor index.
 * @param count Number of actor slots, grouped into trails of 15.
 * @param config Trail timing, motion, size, and rendering parameters.
 * @param expire_by_size Clear active slots when their size leaves [4, 252].
 */
void func_8006BC44(s32 first, s32 count, void *config, s32 expire_by_size)
{
    SVECTOR position;
    u32 screen_position;
    s16 size;
    s32 next_slot;
    s32 i;
    void *copy_actor;
    void *motion;
    void *copy_motion;
    void *copy_resource;
    void *actor;
    WmapActor *draw_actor;
    s32 resource_offset;
    s32 animation_frame;
    s32 offset;
    s16 *offsets;
    u8 *data;
    u8 *cursor;
    void *source_motion;
    void *resource;

    M2C_FIELD(config, s32 *, 8) = (s32) (M2C_FIELD(config, s32 *, 8) - 1);
    for (i = first; i < first + count; i += 15)
    {
        actor = &D_800D9268[i];
        source_motion = &D_801AFBD0[i];
        resource = &D_80139988[i];
        motion = source_motion;
        position.vx = (s16) ((s32) (((s32) M2C_FIELD(motion, s32 *, 8) >> 3) * (ccos(M2C_FIELD(motion, s16 *, 2)) >> 6)) >> 0xC);
        position.vy = (s16) ((s32) (((s32) M2C_FIELD(motion, s32 *, 8) >> 3) * (csin(M2C_FIELD(motion, s16 *, 2)) >> 6)) >> 0xC);
        position.vz = M2C_FIELD(motion, u16 *, 0xE);
        gte_ldv0(&position);
        gte_rtps();
        M2C_FIELD(motion, s32 *, 8) = (s32) (M2C_FIELD(motion, s32 *, 8) - M2C_FIELD(config, s32 *, 0x10));
        M2C_FIELD(motion, s16 *, 2) = (s16) ((u16) M2C_FIELD(motion, s16 *, 2) + M2C_FIELD(config, u16 *, 0x14));
        gte_stsxy(&screen_position);
        M2C_FIELD(motion, u16 *, 0x10) = M2C_FIELD(&screen_position, u16 *, 0);
        M2C_FIELD(motion, u16 *, 0x12) = (u16) M2C_FIELD(&screen_position, u16 *, 2);
        if (M2C_FIELD(config, s32 *, 8) == 0)
        {
            if (M2C_FIELD(config, s32 *, 0) != -1)
            {
                copy_actor = ((i + M2C_FIELD(config, s32 *, 4)) * 0x2C) + (u8 *)&D_800D9268;
                *(WmapActor *)copy_actor = *(WmapActor *)actor;
                copy_motion = ((i + M2C_FIELD(config, s32 *, 4)) * 0x14) + (u8 *)&D_801AFBD0;
                *(WmapMotion *)copy_motion = *(WmapMotion *)source_motion;
                copy_resource = ((i + M2C_FIELD(config, s32 *, 4)) * 8) + (u8 *)&D_80139988;
                *(WmapResource *)copy_resource = *(WmapResource *)resource;
                M2C_FIELD(copy_actor, u16 *, 0x26) = (u16) M2C_FIELD(config, u16 *, 0x18);
                M2C_FIELD(copy_actor, u16 *, 0x22) = (u16) M2C_FIELD(config, u16 *, 0x1C);
                if (expire_by_size == 0)
                {
                    M2C_FIELD(copy_actor, s16 *, 0xE) = 1;
                }
            }
        }
    }
    if (M2C_FIELD(config, s32 *, 8) == 0)
    {
        next_slot = M2C_FIELD(config, s32 *, 4) + 1;
        M2C_FIELD(config, s32 *, 4) = next_slot;
        M2C_FIELD(config, s32 *, 8) = (s32) M2C_FIELD(config, s32 *, 0);
        if (next_slot >= 0xF)
        {
            M2C_FIELD(config, s32 *, 4) = 1;
        }
    }
    for (i = first; i < first + count; i++)
    {
        draw_actor = &D_800D9268[i];
        motion = (i * 0x14) + (u8 *)&D_801AFBD0;
        if (M2C_FIELD(motion, s16 *, 0) != 0)
        {
            resource_offset = i * 8;
            offsets = (s16 *)((WmapResource *)((u8 *)D_80139988 + resource_offset))->data;
            data = (u8 *)offsets;
            if (draw_actor->previous_sequence != draw_actor->sequence)
            {
                u8 *sequence_cursor;
                draw_actor->previous_sequence = (u16)draw_actor->sequence;
                offset = offsets[draw_actor->sequence];
                draw_actor->remaining = 1;
                sequence_cursor = (u8 *)offsets + offset;
                draw_actor->sequence_start = sequence_cursor;
                draw_actor->cursor = sequence_cursor;
            }
            if (draw_actor->remaining != 255)
            {
                draw_actor->remaining--;
            }
            if (draw_actor->remaining == 0)
            {
                cursor = draw_actor->cursor;
                animation_frame = cursor[0];
                draw_actor->remaining = cursor[1];
                if (animation_frame == 255)
                {
                    cursor = draw_actor->sequence_start;
                    draw_actor->cursor = cursor;
                    animation_frame = cursor[0];
                    draw_actor->remaining = cursor[1];
                }
                draw_actor->cursor += 4;
                draw_actor->frame_data = data + ((s16 *)(animation_frame * 2 + data))[32];
            }
            func_80066F9C(draw_actor, M2C_FIELD(motion, s32 *, 0x10), M2C_FIELD(config, s32 *, 0x24), M2C_FIELD(config, s32 *, 0x20), 0);
            if ((expire_by_size != 0) && ((size = M2C_FIELD(draw_actor, s16 *, 0x24), ((size < 0xFD) == 0)) || (size < 4)))
            {
                M2C_FIELD(motion, s16 *, 0) = 0;
            }
        }
    }
}

/**
 * @brief Advance the map transform effect toward its configured position.
 * @return Sequence continuation flag.
 */
s32 func_8006C0EC(void)
{
    SVECTOR position;

    func_8006AEE0();
    position.vz = 0;
    position.vx =
        (s16)((s32)((((D_8011D510 - 1) * 0xA0) - ((s32)(M2C_FIELD(&D_80139950, s32*, 0) * 0x14000) / (s32)M2C_FIELD(&D_80139950, s32*, 8))) * 0x6000) /
              (s32)M2C_FIELD(&D_80139950, s32*, 8));
    position.vy =
        (s16)((s32)((((D_8011D530 - 1) * 0xA0) - ((s32)(M2C_FIELD(&D_80139950, s32*, 4) * 0x14000) / (s32)M2C_FIELD(&D_80139950, s32*, 8))) * 0x6000) /
              (s32)M2C_FIELD(&D_80139950, s32*, 8));
    gte_ldv0(&position);
    gte_rtps();
    if (M2C_FIELD(&D_800DCEB8, s16*, 0) != D_801398C8.vx)
    {
        if (M2C_FIELD(&D_800DCEB8, s16*, 0) < D_801398C8.vx)
        {
            D_801398C8.vx = (u16)D_801398C8.vx - D_80139210;
        }
        else
        {
            D_801398C8.vx = (u16)D_801398C8.vx + D_80139210;
        }
    }
    if (D_800DCEBA != D_801398C8.vy)
    {
        if (D_800DCEBA < D_801398C8.vy)
        {
            D_801398C8.vy = (u16)D_801398C8.vy - D_80139212;
        }
        else
        {
            D_801398C8.vy = (u16)D_801398C8.vy + D_80139212;
        }
    }
    if (D_800DCEBC != D_801398C8.vz)
    {
        if (D_800DCEBC < D_801398C8.vz)
        {
            D_801398C8.vz = (u16)D_801398C8.vz - D_80139214;
        }
        else
        {
            D_801398C8.vz = (u16)D_801398C8.vz + D_80139214;
        }
    }
    if (M2C_FIELD(&D_80139200, s32*, 0) != D_80182D48.vx)
    {
        if (M2C_FIELD(&D_80139200, s32*, 0) < D_80182D48.vx)
        {
            D_80182D48.vx = D_80182D48.vx - D_80139968;
        }
        else
        {
            D_80182D48.vx = D_80182D48.vx + D_80139968;
        }
    }
    if (D_80139204 != D_80182D48.vy)
    {
        if (D_80139204 < D_80182D48.vy)
        {
            D_80182D48.vy = D_80182D48.vy - D_8013996C;
        }
        else
        {
            D_80182D48.vy = D_80182D48.vy + D_8013996C;
        }
    }
    if (D_8013B29C == 0)
    {
        D_801398C8.vx = 0;
        D_801398C8.vy = 0;
        D_801398C8.vz = 0;
        D_80182D48.vx = 0;
        D_80182D48.vy = 0;
        D_80182D48.vz = 0;
        M2C_FIELD(&D_80139200, s32*, 0) = 0;
        M2C_FIELD(&D_80139200, s32*, 4) = 0;
        M2C_FIELD(&D_80139200, s32*, 8) = 0;
        M2C_FIELD(&D_800DCEB8, s16*, 0) = 0;
        M2C_FIELD(&D_800DCEB8, s16*, 2) = 0;
        M2C_FIELD(&D_800DCEB8, s16*, 4) = 0;
        M2C_FIELD(&D_8011CF4C, s16*, 0) = 0xA4;
        M2C_FIELD(&D_8011CF4C, s16*, 2) = 0x69;
        return 0;
    }
    gte_stsxy(&D_8011CF4C);
    return D_8013B29C;
}

/**
 * @brief Spawn one free particle, then animate and draw all active particles.
 * @param config Particle range, spawn bounds, animation, and drawing settings.
 */
void func_8006C448(void *config)
{
    SVECTOR position;
    s32 screen_position;
    s32 draw_index;
    s32 spawn_index;
    s32 i;
    WmapActor *actor;
    s32 resource_offset;
    s32 animation_frame;
    s32 offset;
    s16 *offsets;
    u8 *data;
    u8 *cursor;
    void *spawn_actor;
    void *spawn_motion;
    void *motion;

    for (i = 0; i < M2C_FIELD(config, s32 *, 0); i++)
    {
        spawn_index = M2C_FIELD(config, s32 *, 4) + i;
        spawn_actor = &D_800D9268[spawn_index];
        spawn_motion = (spawn_index * 0x14) + (u8 *)&D_801AFBD0;
        if (M2C_FIELD(spawn_motion, s16 *, 0) == 0)
        {
            M2C_FIELD(spawn_motion, s16 *, 0) = 1;
            M2C_FIELD(spawn_motion, s16 *, 0x10) = (s16) (M2C_FIELD(config, u16 *, 8) + ((s32) (rand() * M2C_FIELD(config, s32 *, 0xC)) >> 0xF));
            M2C_FIELD(spawn_motion, s16 *, 0xE) = (s16) (M2C_FIELD(config, u16 *, 0x10) + ((s32) (rand() * M2C_FIELD(config, s32 *, 0x14)) >> 0xF));
            M2C_FIELD(spawn_motion, s16 *, 0x12) = (s16) (M2C_FIELD(config, u16 *, 0x18) + ((s32) (rand() * M2C_FIELD(config, s32 *, 0x1C)) >> 0xF));
            M2C_FIELD(spawn_actor, s16 *, 2) = 0;
            M2C_FIELD(spawn_actor, s8 *, 6) = 0xF;
            M2C_FIELD(spawn_actor, u16 *, 0xE) = (u16) M2C_FIELD(config, u16 *, 0x34);
            M2C_FIELD(spawn_actor, s16 *, 0x10) = -1;
            M2C_FIELD(spawn_actor, u16 *, 0x22) = (u16) M2C_FIELD(config, u16 *, 0x20);
            M2C_FIELD(spawn_actor, u16 *, 0x24) = (u16) M2C_FIELD(config, u16 *, 0x24);
            M2C_FIELD(spawn_actor, u16 *, 0x26) = (u16) M2C_FIELD(config, u16 *, 0x28);
            break;
        }
    }
    for (i = 0; i < M2C_FIELD(config, s32 *, 0); i++)
    {
        draw_index = M2C_FIELD(config, s32 *, 4) + i;
        actor = &D_800D9268[draw_index];
        motion = (draw_index * 0x14) + (u8 *)&D_801AFBD0;
        if (M2C_FIELD(motion, s16 *, 0) != 0)
        {
            position.vx = M2C_FIELD(motion, u16 *, 0x10);
            position.vy = M2C_FIELD(motion, u16 *, 0xE);
            position.vz = M2C_FIELD(motion, u16 *, 0x12);
            gte_ldv0(&position);
            gte_rtps();
            resource_offset = (i + M2C_FIELD(config, s32 *, 4)) * 8;
            offsets = (s16 *)((WmapResource *)((u8 *)D_80139988 + resource_offset))->data;
            data = (u8 *)offsets;
            if (actor->previous_sequence != actor->sequence)
            {
                u8 *sequence_cursor;
                actor->previous_sequence = (u16)actor->sequence;
                offset = offsets[actor->sequence];
                actor->remaining = 1;
                sequence_cursor = (u8 *)offsets + offset;
                actor->sequence_start = sequence_cursor;
                actor->cursor = sequence_cursor;
            }
            if (actor->remaining != 255)
            {
                actor->remaining--;
            }
            if (actor->remaining == 0)
            {
                cursor = actor->cursor;
                animation_frame = cursor[0];
                actor->remaining = cursor[1];
                if (animation_frame == 255)
                {
                    cursor = actor->sequence_start;
                    actor->cursor = cursor;
                    animation_frame = cursor[0];
                    actor->remaining = cursor[1];
                }
                actor->cursor += 4;
                actor->frame_data = data + ((s16 *)(animation_frame * 2 + data))[32];
            }
            gte_stsxy(&screen_position);
            func_80066F9C(actor, screen_position, M2C_FIELD(config, s32 *, 0x30), M2C_FIELD(config, s32 *, 0x2C), 0);
        }
    }

}
