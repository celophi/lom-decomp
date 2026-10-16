#include "../internal/wmap_view_effects.h"
#include "../internal/wmap_sprite_render.h"
#include "../internal/wmap_effect_primitives.h"
#include <inline_c.h>
#include "sdk/gte_dmpsx_compat.h"

/** @brief Halfword index of the animation resource's frame-offset table. */
#define WMAP_ANIMATION_FRAME_TABLE_INDEX 32

/** @brief Radial particle motion and lifetime. */
typedef struct
{
    s16 state, angle;
    s32 velocity, radius;
    s16 lifetime, z;
    s32 angular_velocity;
} WmapMotion;

/** @brief Falling velocity stored as a word and stepped by its low halfword. */
typedef union
{
    s32 value;
    struct
    {
        u16 step;
        u16 unknown_02;
    } parts;
} WmapFallingVelocity;

/**
 * @brief Falling-particle view of a motion slot, with packed coordinate halfwords.
 * @note The coordinates are interpreted as signed values for drawing and fading.
 */
typedef struct
{
    s16 state;
    u8 unknown_02[2];
    WmapFallingVelocity velocity;
    u8 unknown_08[6];
    u16 z;
    u16 x;
    u16 y;
} WmapFallingMotion;

/** @brief Actor slots in one radial particle trail, including its head. */
#define WMAP_PARTICLE_TRAIL_LENGTH 15

/** @brief Inclusive shade range of a live trail slot. */
#define WMAP_PARTICLE_TRAIL_MIN_SHADE 4
#define WMAP_PARTICLE_TRAIL_MAX_SHADE 252

/** @brief Radial trail motion with a cached projected screen position. */
typedef struct
{
    s16 state;
    s16 angle;
    s32 velocity;
    s32 radius;
    s16 lifetime;
    s16 z;
    WmapScreenPosition screen_position;
} WmapTrailMotion;

/** @brief Particle motion slot with packed Y, X, Z coordinate halfwords. */
typedef struct
{
    s16 state;
    u8 unknown_02[12];
    u16 y;
    u16 x;
    u16 z;
} WmapBoxParticleMotion;

/** @brief Position and velocity records, also used for particle bounds. */
typedef struct
{
    SVECTOR position;
    SVECTOR velocity;
} WmapMovingPoint;

/** @brief Map scroll position and projection scale. */
typedef struct
{
    s32 x;
    s32 y;
    s32 projection_scale;
    s32 unknown_0c;
} WmapEffectProjection;

/** @brief Map cell size and projection scales used by effect positions. */
#define WMAP_EFFECT_CELL_SIZE 160
#define WMAP_EFFECT_MAP_SCALE 0x14000
#define WMAP_EFFECT_PROJECTION_SCALE 0x6000

extern s32 g_wmap_frame_count;
extern s32 D_8013B264;
extern s32 D_8013B270;
extern s32 D_8013B278;
extern s32 D_8013B280;
extern s32 D_8013B284;
extern WmapMotion g_wmap_actor_motions[];
extern s32 g_wmap_particle_intensity;
extern SVECTOR g_wmap_camera_rotation;
extern VECTOR g_wmap_camera_translation;
extern MATRIX D_8011D0E8;
extern u16 D_80139980;
extern WmapAnimationSlot g_wmap_actor_animations[];
extern s32 D_800D9150[];
extern s32 D_800DCEA8[];
extern SVECTOR D_800DCEB8;
extern WmapScreenPosition g_wmap_focus_screen_position;
extern s32 g_wmap_focus_cell_x;
extern s32 g_wmap_focus_cell_y;
extern VECTOR D_80139200;
extern SVECTOR D_80139210;
extern WmapEffectProjection g_wmap_view;
extern VECTOR D_80139968;
extern s32 D_8013B29C;

s32 rand(void);

typedef struct
{
    s16 id, state;
    u8 pad04[2];
    s8 scale;
    u8 pad07[7];
    s16 sequence, previous_sequence;
    u8 pad12[2];
    u8* cursor;
    u8* sequence_start;
    u8* frame_data;
    s16 remaining, target_shade, shade, shade_step;
    u8 pad28[4];
} WmapParticleActor;

typedef struct
{
    s32 unknown00;
    s32 velocity_min, velocity_range;
    s32 lifetime_min, lifetime_range;
    s32 spawn_interval, radial_step, first_motion;
    s32 texture_index, sequence, radius;
} WmapParticleConfig;

/**
 * @brief Update, project, and replenish a configured particle effect.
 * @param arg0 Actor records to update.
 * @param animations Animation resource slots, one per actor record.
 * @param arg2 Number of actor records to process.
 * @param arg3 Effect parameter; semantics remain unresolved.
 * @param arg4_value Effect parameter stored in a halfword.
 * @param arg5_value Effect parameter stored in a halfword.
 * @param arg6 Particle behavior selector.
 * @param arg7 Particle effect configuration record.
 */
void func_8006A2FC(void* arg0, WmapAnimationSlot* animations, s32 arg2, s32 arg3, s32 arg4_value, s32 arg5_value, u32 arg6, void* arg7)
{
    SVECTOR position;
    s32 screen_position;
    s32 ot_index;
    s16 sequence;
    s32 falling_velocity;
    s16* offsets;
    u8* data;
    u8* cursor;
    s32 offset;
    s32 lifetime_product;
    s32 packed_radius;
    s32 radius_min;
    s32 radius_range;
    s32 parabola_height;
    s32 phase;
    s32 spawn_interval;
    s32 i;
    u16 new_z;
    s32 frame;
    WmapMotion* motion;
    WmapParticleActor* actor;
    WmapParticleConfig* config = arg7;

    ot_index = 4;
    for (i = 0; i < arg2; i++)
    {
        actor = &((WmapParticleActor*)arg0)[i];
        motion = &g_wmap_actor_motions[i + config->first_motion];
        if (motion->state != 0)
        {
            position.vx = ((motion->radius >> 3) * (ccos(motion->angle) >> 6)) >> 12;
            position.vy = ((motion->radius >> 3) * (csin(motion->angle) >> 6)) >> 12;
            position.vz = (u16)motion->z;
            gte_ldv0(&position);
            gte_rtps();
            switch (arg6)
            {
            case 5:
                if (actor->shade >= arg3)
                {
                    actor->target_shade = 0;
                }
                if ((actor->target_shade == 0) && (actor->shade < 4))
                {
                    motion->state = 0;
                }
                /* fallthrough */
            case 0:
                motion->z = (u16)((u16)motion->z + (u16)motion->velocity);
                motion->radius += config->radial_step;
                if (--motion->lifetime == 0)
                {
                    motion->state = 0;
                }
            default:
                break;
            case 1:
                if (D_8013B264 < motion->lifetime)
                {
                    motion->z = (u16)((u16)motion->z + (D_8013B270 + ((s32)(rand() * D_8013B278) >> 0xF)));
                    motion->radius += config->radial_step;
                }
                else
                {
                    falling_velocity = (rand() * D_8013B284) >> 15;
                    new_z = (u16)motion->z - (D_8013B280 + falling_velocity);
                    motion->z = new_z;
                    if ((s16)new_z < 0)
                    {
                        motion->z = 0U;
                        actor->target_shade = 0;
                        actor->shade_step = 8;
                        if (actor->shade < 4)
                        {
                            motion->state = 0;
                        }
                    }
                }
                ot_index = 0xC;
                if ((u32)((u16)motion->angle - 1) < 0x7FFU)
                {
                    ot_index = 0xA;
                }
                motion->lifetime = (u16)((u16)motion->lifetime - 1);
                break;
            case 2:
                parabola_height = motion->lifetime - D_8013B264;
                parabola_height = (parabola_height * parabola_height) / D_8013B278;
                motion->z = D_8013B280 + (D_8013B270 - parabola_height);
                if (motion->lifetime != 0)
                {
                    motion->lifetime--;
                    motion->radius += config->radial_step;
                }
                break;
            case 3:
                phase = D_8013B264 - motion->lifetime;
                motion->z = (u16)((s32)(phase * phase) / (s32)D_8013B270);
                break;
            case 4:
                motion->z = (u16)((u16)motion->z + (u16)motion->velocity);
                motion->radius += config->radial_step;
                if (--motion->lifetime == 0)
                {
                    motion->state = 0;
                }
                break;
            case 7:
                motion->z = (u16)((u16)motion->z + (u16)motion->velocity);
                motion->angle = (s16)((u16)motion->angle + (u16)config->lifetime_range);
                motion->radius += config->radial_step;
                /* fallthrough */
            case 6:
                if (motion->z < 0)
                {
                    motion->state = 0;
                    break;
                }
                break;
            }
            offsets = (s16*)animations[i].data;
            data = (u8*)offsets;
            if (actor->previous_sequence != actor->sequence)
            {
                u8* sequence_cursor;
                actor->previous_sequence = (u16)actor->sequence;
                offset = offsets[actor->sequence];
                actor->remaining = 1;
                sequence_cursor = (u8*)offsets + offset;
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
                frame = cursor[0];
                actor->remaining = cursor[1];
                if (frame == 255)
                {
                    cursor = actor->sequence_start;
                    actor->cursor = cursor;
                    frame = cursor[0];
                    actor->remaining = cursor[1];
                }
                actor->cursor += 4;
                actor->frame_data = data + offsets[frame + WMAP_ANIMATION_FRAME_TABLE_INDEX];
            }
            gte_stsxy(&screen_position);
            wmap_draw_actor_sprite(actor, screen_position, config->texture_index, ot_index, 0);
        }
    }
    spawn_interval = config->spawn_interval;
    if ((spawn_interval != -1) && (((s32)g_wmap_frame_count % spawn_interval) == 0))
    {
        for (i = 0; i < arg2; i++)
        {
            motion = &g_wmap_actor_motions[i + config->first_motion];
            if (motion->state == 0)
            {
                WmapParticleActor* spawn_actor = &((WmapParticleActor*)arg0)[i];
                spawn_actor->state = 0;
                spawn_actor->scale = 0xF;
                sequence = (u16)config->sequence;
                spawn_actor->previous_sequence = -1;          /* previous sequence */
                spawn_actor->target_shade = (u16)arg3;  /* target shade */
                spawn_actor->shade = arg4_value;
                spawn_actor->sequence = sequence;    /* sequence */
                spawn_actor->shade_step = arg5_value;
                motion->state = 1;
                motion->angle = rand();
                packed_radius = config->radius;
                if (packed_radius < 0)
                {
                    radius_min = (packed_radius & 0x7FFF0000) >> 0xE;
                    radius_range = (packed_radius & 0xFFFF) << 5;
                    motion->radius = ((rand() * radius_range) >> 0xF) + radius_min;
                }
                else
                {
                    motion->radius = packed_radius;
                }
                motion->velocity = (s32)(((s32)(rand() * config->velocity_range) >> 0xF) + config->velocity_min);
                lifetime_product = rand() * config->lifetime_range;
                motion->lifetime = (s16)((u16)config->lifetime_min + (lifetime_product >> 0xF));
                motion->z = 0;
                return;
            }
        }
    }
}

/**
 * @brief Project radial particles and periodically spawn one in a free slot.
 * @param actors Sprite actor records of the particles.
 * @param animations Animation resource slots, one per actor record.
 * @param first First particle index.
 * @param end Exclusive last particle index.
 * @param scale Scale used to set the actor's shade.
 * @param velocity_min Minimum radial velocity.
 * @param velocity_range Range of randomized radial velocity.
 * @param lifetime_min Minimum particle lifetime.
 * @param lifetime_range Range of randomized lifetime.
 * @param initial_z Initial depth divisor.
 * @param frame Sprite texture index passed to the renderer.
 * @param spawn_interval Number of updates between spawn attempts.
 */
void func_8006A9C4(WmapSpriteActor* actors, WmapAnimationSlot* animations, s32 first, s32 end, s32 scale,
                   s32 velocity_min, s32 velocity_range, s32 lifetime_min, s32 lifetime_range, s32 initial_z,
                   s32 frame, s32 spawn_interval)
{
    SVECTOR position;
    s32 screen_position;
    s32 sequence_offset;
    u8* animation_address;
    s32 lifetime_product;
    s32 scaled_size;
    s32 i;
    s32 active;
    s32 sequence_end;
    u16 remaining_lifetime;
    s32 animation_frame;
    u8* cursor;
    u8* sequence_start;
    u8* sequence_cursor;
    WmapSpriteActor* spawn_actor;
    WmapSpriteActor* actor;
    WmapMotion* spawn_motion;
    WmapMotion* motion;
    WmapAnimationSlot* resource;
    WmapMotion* motion_base;

    active = 0;
    i = first;
    if (i < end)
    {
        sequence_end = 255;
        for (; i < end; i++)
        {
            resource = &animations[i];
            actor = &actors[i];
            motion_base = g_wmap_actor_motions;
            motion = &motion_base[i];
            if (motion->state != 0)
            {
                position.vx = ((motion->radius >> 6) * (ccos(motion->angle) >> 6)) >> 12;
                position.vy = ((motion->radius >> 6) * (csin(motion->angle) >> 6)) >> 12;
                position.vz = 200 / motion->z;
                gte_ldv0(&position);
                gte_rtps();
                scaled_size = scale * 0x81;
                motion->z++;
                motion->radius += motion->velocity;
                actor->target_shade = 0;
                if (scaled_size < 0)
                {
                    scaled_size += 0xFF;
                }
                actor->shade = (s16) (scaled_size >> 8);
                gte_stsxy(&screen_position);
                animation_address = resource->data;
                if (actor->previous_sequence != actor->sequence)
                {
                    actor->previous_sequence = actor->sequence;
                    sequence_offset = ((s16*)animation_address)[actor->sequence];
                    actor->remaining = 1;
                    sequence_cursor = animation_address + sequence_offset;
                    actor->sequence_start = sequence_cursor;
                    actor->cursor = sequence_cursor;
                }
                if (actor->remaining != sequence_end)
                {
                    actor->remaining--;
                }
                if (actor->remaining == 0)
                {
                    cursor = actor->cursor;
                    animation_frame = cursor[0];
                    actor->remaining = cursor[1];
                    if (animation_frame == sequence_end)
                    {
                        sequence_start = actor->sequence_start;
                        actor->cursor = sequence_start;
                        animation_frame = sequence_start[0];
                        actor->remaining = sequence_start[1];
                    }
                    actor->cursor += 4;
                    actor->frame_data = animation_address + ((s16*)animation_address)[animation_frame + WMAP_ANIMATION_FRAME_TABLE_INDEX];
                }
                wmap_draw_actor_sprite(actor, screen_position, frame, 0xA, 0);
                remaining_lifetime = (u16)motion->lifetime - 1;
                motion->lifetime = remaining_lifetime;
                if ((remaining_lifetime << 0x10) == 0)
                {
                    motion->state = 0;
                }
                active += 1;
            }
        }
    }
    if (g_wmap_frame_count % spawn_interval == 0)
    {
        for (i = first; i < end; i++)
        {
            spawn_actor = &actors[i];
            spawn_motion = &g_wmap_actor_motions[i];
            if (spawn_motion->state == 0)
            {
                if (g_wmap_particle_intensity >= active)
                {
                    spawn_actor->scale_index = 0xF;
                    spawn_actor->resource_index = 0;
                    spawn_actor->sequence = 1;
                    spawn_actor->previous_sequence = -1;
                    spawn_motion->state = 1;
                    spawn_motion->angle = (u32)rand() >> 3;
                    spawn_motion->radius = 0;
                    spawn_motion->velocity = ((rand() * velocity_range) >> 15) + velocity_min;
                    lifetime_product = rand() * lifetime_range;
                    spawn_motion->z = initial_z;
                    spawn_motion->lifetime = (lifetime_product >> 15) + lifetime_min;
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
void wmap_set_effect_model_transform(VECTOR* translation, SVECTOR* rotation)
{
    MATRIX base;
    MATRIX effect;
    SVECTOR combined_rotation;
    VECTOR combined_translation;

    combined_rotation.vx = g_wmap_camera_rotation.vx + g_wmap_effect_camera_rotation_offset.vx;
    combined_rotation.vy = g_wmap_camera_rotation.vy + g_wmap_effect_camera_rotation_offset.vy;
    combined_rotation.vz = g_wmap_camera_rotation.vz + g_wmap_effect_camera_rotation_offset.vz;
    combined_translation.vx = g_wmap_camera_translation.vx + g_wmap_effect_camera_translation_offset.vx;
    combined_translation.vy = g_wmap_camera_translation.vy + g_wmap_effect_camera_translation_offset.vy;
    combined_translation.vz = g_wmap_camera_translation.vz + g_wmap_effect_camera_translation_offset.vz;
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
void wmap_set_effect_camera_transform(void)
{
    SVECTOR rotation;
    VECTOR translation;

    rotation.vx = g_wmap_camera_rotation.vx + g_wmap_effect_camera_rotation_offset.vx;
    rotation.vy = g_wmap_camera_rotation.vy + g_wmap_effect_camera_rotation_offset.vy;
    rotation.vz = g_wmap_camera_rotation.vz + g_wmap_effect_camera_rotation_offset.vz;
    translation.vx = g_wmap_camera_translation.vx + g_wmap_effect_camera_translation_offset.vx;
    translation.vy = g_wmap_camera_translation.vy + g_wmap_effect_camera_translation_offset.vy;
    translation.vz = g_wmap_camera_translation.vz + g_wmap_effect_camera_translation_offset.vz;
    RotMatrix(&rotation, &D_8011D0E8);
    TransMatrix(&D_8011D0E8, &translation);
    SetRotMatrix(&D_8011D0E8);
    SetTransMatrix(&D_8011D0E8);
}

/**
 * @brief Advance radial particles, then refill inactive slots up to the active limit.
 * @param first First particle index.
 * @param end Exclusive last particle index.
 * @param frame Sprite texture index passed to the renderer.
 * @param depth Ordering-table depth of the particles.
 * @param velocity_base Minimum radial velocity of a new particle.
 * @param velocity_divisor Divisor of the random radial-velocity contribution.
 * @param lifetime_base Minimum lifetime of a new particle.
 * @param sequence Animation sequence of a new particle.
 * @note The spawn limit is tested before counting the next candidate slot.
 */
void func_8006AFAC(s32 first, s32 end, s32 frame, s32 depth,
                   s32 velocity_base, s32 velocity_divisor, s32 lifetime_base, s32 sequence)
{
    SVECTOR position;
    s32 screen_position;
    WmapAnimationSlot* animation_slot;
    s32 offset;
    s32 animation_frame;
    s16* offsets;
    u8* data;
    u8* cursor;
    s32 y_product;
    s32 i;
    s32 active;
    WmapSpriteActor* actor;
    WmapMotion* spawn_motion;
    WmapMotion* motion;
    WmapMotion* motion_base;
    WmapSpriteActor* spawn_actor;

    active = 0;
    for (i = first; i < end; i++)
    {
        motion_base = g_wmap_actor_motions;
        motion = &motion_base[i];
        if (motion->state != 0)
        {
            actor = &g_wmap_sprite_actors[i];
            position.vx = ((motion->radius >> 6) * (ccos(motion->angle) >> 6)) >> 12;
            y_product = (motion->radius >> 6) * (csin(motion->angle) >> 6);
            position.vz = 0;
            position.vy = y_product >> 12;
            gte_ldv0(&position);
            gte_rtps();
            motion->radius += motion->velocity;
            actor->target_shade = D_80139980;
            actor->shade = D_80139980;
            gte_stsxy(&screen_position);
            animation_slot = &g_wmap_actor_animations[i];
            offsets = (s16*)animation_slot->data;
            data = (u8* )offsets;
            if (actor->previous_sequence != actor->sequence)
            {
                u8* sequence_cursor;
                actor->previous_sequence = actor->sequence;
                offset = offsets[actor->sequence];
                actor->remaining = 1;
                sequence_cursor = (u8* )offsets + offset;
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
                actor->frame_data = data + offsets[animation_frame + WMAP_ANIMATION_FRAME_TABLE_INDEX];
            }
            wmap_draw_actor_sprite(actor, screen_position, frame, depth, 0);
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
        spawn_actor = &g_wmap_sprite_actors[i];
        spawn_motion = &g_wmap_actor_motions[i];
        if (spawn_motion->state == 0)
        {
            if (active++ > g_wmap_particle_intensity)
            {
                break;
            }
            spawn_actor->resource_index = 0;
            spawn_actor->scale_index = 0xF;
            spawn_actor->sequence = sequence;
            spawn_actor->previous_sequence = -1;
            spawn_motion->state = 1;
            spawn_motion->angle = (u32)rand() >> 3;
            spawn_motion->radius = 0;
            spawn_motion->velocity = velocity_base + rand() / velocity_divisor;
            spawn_motion->lifetime = lifetime_base + (rand() & 0x3C);
        }
    }
}

/**
 * @brief Update falling particles and refill inactive slots at a timed spawn rate.
 * Incoming values occupy 32-bit argument slots, including values stored in halfwords.
 * @param first First particle slot to update.
 * @param end Exclusive last particle slot.
 * @param spawn_interval Frames between timed spawn attempts.
 * @param scale_override Override for target and current shade, or -1 to preserve them.
 * @param velocity_min Minimum depth step of a new particle.
 * @param velocity_range Range added to the randomized depth step.
 * @param initial_z Initial particle depth.
 * @param frame Sprite texture index passed to the renderer.
 * @param x_min Minimum initial horizontal position.
 * @param x_range Range added to the randomized horizontal position.
 * @param y_min Minimum initial vertical position.
 * @param y_range Range added to the randomized vertical position.
 * @param fade_z Depth below which the target shade becomes zero.
 * @param initial_scale_target Initial target shade of a new particle.
 * @param initial_scale Initial current shade of a new particle.
 * @param scale_step Shade step of a new particle.
 * @param group Index in the spawn timer and remaining-count tables.
 */
void func_8006B328(s32 first, s32 end, s32 spawn_interval, s32 scale_override, s32 velocity_min, s32 velocity_range, 
                   s32 initial_z, s32 frame, s32 x_min, s32 x_range, s32 y_min, s32 y_range, s32 fade_z, 
                   s32 initial_scale_target, s32 initial_scale, s32 scale_step, s32 group)
{
    SVECTOR position;
    s32 screen_position;
    s32 sequence_offset;
    WmapAnimationSlot* animation_slot;
    s32* spawn_count;
    s32* spawn_timer;
    s32* initial_timer;
    s32* final_timer;
    s32 timer_offset;
    u8* timer_base;
    uintptr_t address;
    s32 remaining_spawns;
    s32 i;
    u16 next_z;
    s32 animation_frame;
    s32 sequence_end;
    WmapSpriteActor* actor;
    WmapFallingMotion* motion;
    u8* cursor;
    u8* sequence_start;
    u8* sequence_cursor;

    address = group * 4;
    initial_timer = (s32*)(address + (u8*)D_800D9150);
    i = first;
    *initial_timer -= 1;
    if (i < end)
    {
        sequence_end = 255;
        timer_offset = address;
        for (; i < end; i++)
        {
            motion = (WmapFallingMotion*)&g_wmap_actor_motions[i];
            actor = &g_wmap_sprite_actors[i];
            if (motion->state != 0)
            {
                if (scale_override != -1)
                {
                    actor->shade = scale_override;
                    actor->target_shade = scale_override;
                }
                position.vx = motion->x;
                position.vy = motion->y;
                position.vz = motion->z;
                gte_ldv0(&position);
                gte_rtps();
                animation_slot = &g_wmap_actor_animations[i];
                address = (uintptr_t)animation_slot->data;
                if (actor->previous_sequence != actor->sequence)
                {
                    actor->previous_sequence = actor->sequence;
                    sequence_offset = ((s16*)address)[actor->sequence];
                    actor->remaining = 1;
                    sequence_cursor = (u8*)address + sequence_offset;
                    actor->sequence_start = sequence_cursor;
                    actor->cursor = sequence_cursor;
                }
                if (actor->remaining != sequence_end)
                {
                    actor->remaining--;
                }
                if (actor->remaining == 0)
                {
                    cursor = actor->cursor;
                    animation_frame = cursor[0];
                    actor->remaining = cursor[1];
                    if (animation_frame == sequence_end)
                    {
                        sequence_start = actor->sequence_start;
                        actor->cursor = sequence_start;
                        animation_frame = sequence_start[0];
                        actor->remaining = sequence_start[1];
                    }
                    actor->cursor += 4;
                    actor->frame_data = (u8*)address + ((s16*)address)[animation_frame + WMAP_ANIMATION_FRAME_TABLE_INDEX];
                }
                gte_stsxy(&screen_position);
                wmap_draw_actor_sprite(actor, screen_position, frame, 4, 0);
                next_z = motion->z - motion->velocity.parts.step;
                motion->z = next_z;
                if ((s16) next_z < fade_z)
                {
                    actor->target_shade = 0;
                }
                if ((s16) motion->z < 0)
                {
                    motion->z = 0;
                }
                if ((actor->target_shade == 0) && (actor->shade < 5))
                {
                    motion->state = 0;
                }
            }
            else
            {
                spawn_timer = (s32*)(timer_offset + (u8*)D_800D9150);
                if (*spawn_timer == 0)
                {
                    spawn_count = (s32*)(timer_offset + (u8*)D_800DCEA8);
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
                        motion->state = 1;
                        motion->z = initial_z;
                        motion->x = ((rand() * x_range) >> 15) + x_min;
                        motion->y = ((rand() * y_range) >> 15) + y_min;
                        motion->velocity.value = ((rand() * velocity_range) >> 15) + velocity_min;
                        actor->target_shade = initial_scale_target;
                        actor->shade = initial_scale;
                        actor->shade_step = scale_step;
                    }
                }
            }
        }
    }
    timer_base = (u8*)D_800D9150;
    final_timer = (s32*)((group * 4) + timer_base);
    if (*final_timer == 0)
    {
        *final_timer = spawn_interval;
    }
}

/**
 * @brief Project moving particles, advance their animation, and draw by depth.
 * @param first First actor index.
 * @param end Exclusive last actor index.
 * @param frame Sprite texture index passed to the renderer.
 * @param z_step Per-update change in particle Z.
 * @param depth Base ordering-table depth.
 */
void func_8006B6EC(s32 first, s32 end, s32 frame, s32 z_step, s32 depth)
{
    SVECTOR position;
    s32 screen_position;
    WmapAnimationSlot* animation_slot;
    s32 i;
    WmapMotion* motion;
    WmapSpriteActor* actor, *actor_base;
    u8* data;
    u8* cursor;
    s16* offsets;
    s32 animation_frame;
    s32 offset;

    for (i = first; i < end; i++)
    {
        motion = &g_wmap_actor_motions[i];
        actor_base = g_wmap_sprite_actors;
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
            animation_slot = &g_wmap_actor_animations[i];
            offsets = (s16*)animation_slot->data;
            data = (u8* )offsets;
            if (actor->previous_sequence != actor->sequence)
            {
                u8* sequence_cursor;
                actor->previous_sequence = actor->sequence;
                offset = offsets[actor->sequence];
                actor->remaining = 1;
                sequence_cursor = (u8* )offsets + offset;
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
                actor->frame_data = data + offsets[animation_frame + WMAP_ANIMATION_FRAME_TABLE_INDEX];
            }
            if ((u32)((u16)motion->angle - 1025) < 2047U)
            {
                wmap_draw_actor_sprite(actor, screen_position, frame, depth + 2, 0);
            }
            else
            {
                wmap_draw_actor_sprite(actor, screen_position, frame, depth - 2, 0);
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
 * @param frame Sprite texture index passed to the renderer.
 * @param depth Ordering-table depth.
 */
void func_8006B998(s32 first, s32 end, void *point_data, s32 frame, s32 depth)
{
    WmapMovingPoint* points = point_data;
    s32 screen_position;
    s32 i;
    WmapMovingPoint* point;
    WmapMovingPoint* bounds;
    WmapSpriteActor* actor;
    WmapSpriteActor* actor_base;
    u8* data;
    s16* offsets;
    u8* cursor;
    s32 offset;
    WmapAnimationSlot* animation_slot;
    s32 animation_frame;

    point = &points[first];
    bounds = &points[end];
    for (i = first; i < end; point++, i++)
    {
        actor_base = g_wmap_sprite_actors;
        actor = &actor_base[i];
        gte_ldv0(&point->position);
        gte_rtps();
        animation_slot = &g_wmap_actor_animations[i];
        offsets = (s16*)animation_slot->data;
        data = (u8* )offsets;
        if (actor->previous_sequence != actor->sequence)
        {
            u8* sequence_cursor;
            actor->previous_sequence = actor->sequence;
            offset = offsets[actor->sequence];
            actor->remaining = 1;
            sequence_cursor = (u8* )offsets + offset;
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
            actor->frame_data = data + offsets[animation_frame + WMAP_ANIMATION_FRAME_TABLE_INDEX];
        }
        gte_stsxy(&screen_position);
        wmap_draw_actor_sprite(actor, screen_position, frame, depth, 0);
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
 * @param config Trail timing, motion, shade, and rendering parameters.
 * @param expire_by_shade Clear active slots when their shade leaves [4, 252].
 */
void func_8006BC44(s32 first, s32 count, WmapTrailConfig* config, s32 expire_by_shade)
{
    SVECTOR position;
    WmapScreenPosition screen_position;
    s16 shade;
    s32 next_slot;
    s32 i;
    WmapSpriteActor* copy_actor;
    WmapTrailMotion* motion;
    WmapTrailMotion* copy_motion;
    WmapAnimationSlot* copy_resource;
    WmapSpriteActor* actor;
    WmapSpriteActor* draw_actor;
    WmapAnimationSlot* animation_slot;
    s32 animation_frame;
    s32 offset;
    s16* offsets;
    u8* data;
    u8* cursor;
    WmapTrailMotion* source_motion;
    WmapAnimationSlot* resource;

    config->spawn_timer--;
    for (i = first; i < first + count; i += WMAP_PARTICLE_TRAIL_LENGTH)
    {
        actor = &g_wmap_sprite_actors[i];
        source_motion = (WmapTrailMotion*)&g_wmap_actor_motions[i];
        resource = &g_wmap_actor_animations[i];
        motion = source_motion;
        position.vx = ((motion->radius >> 3) * (ccos(motion->angle) >> 6)) >> 12;
        position.vy = ((motion->radius >> 3) * (csin(motion->angle) >> 6)) >> 12;
        position.vz = motion->z;
        gte_ldv0(&position);
        gte_rtps();
        motion->radius -= config->radial_step;
        motion->angle += config->angle_step;
        gte_stsxy(&screen_position.packed);
        motion->screen_position.point.x = screen_position.point.x;
        motion->screen_position.point.y = screen_position.point.y;
        if (config->spawn_timer == 0)
        {
            if (config->spawn_interval != -1)
            {
                copy_actor = &g_wmap_sprite_actors[i + config->next_slot];
                *copy_actor = *actor;
                copy_motion = (WmapTrailMotion*)&g_wmap_actor_motions[i + config->next_slot];
                *copy_motion = *source_motion;
                copy_resource = &g_wmap_actor_animations[i + config->next_slot];
                *copy_resource = *resource;
                copy_actor->shade_step = config->shade_step;
                copy_actor->target_shade = config->target_shade;
                if (expire_by_shade == 0)
                {
                    copy_actor->sequence = 1;
                }
            }
        }
    }
    if (config->spawn_timer == 0)
    {
        next_slot = config->next_slot + 1;
        config->next_slot = next_slot;
        config->spawn_timer = config->spawn_interval;
        if (next_slot >= WMAP_PARTICLE_TRAIL_LENGTH)
        {
            config->next_slot = 1;
        }
    }
    for (i = first; i < first + count; i++)
    {
        draw_actor = &g_wmap_sprite_actors[i];
        motion = (WmapTrailMotion*)&g_wmap_actor_motions[i];
        if (motion->state != 0)
        {
            animation_slot = &g_wmap_actor_animations[i];
            offsets = (s16*)animation_slot->data;
            data = (u8*)offsets;
            if (draw_actor->previous_sequence != draw_actor->sequence)
            {
                u8* sequence_cursor;
                draw_actor->previous_sequence = draw_actor->sequence;
                offset = offsets[draw_actor->sequence];
                draw_actor->remaining = 1;
                sequence_cursor = (u8*)offsets + offset;
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
                draw_actor->frame_data = data + offsets[animation_frame + WMAP_ANIMATION_FRAME_TABLE_INDEX];
            }
            wmap_draw_actor_sprite(draw_actor, motion->screen_position.packed, config->texture_index, config->depth, 0);
            if (expire_by_shade != 0)
            {
                shade = draw_actor->shade;
                if (shade > WMAP_PARTICLE_TRAIL_MAX_SHADE)
                {
                    motion->state = 0;
                }
                else if (shade < WMAP_PARTICLE_TRAIL_MIN_SHADE)
                {
                    motion->state = 0;
                }
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

    wmap_set_effect_camera_transform();
    position.vz = 0;
    position.vx = ((((g_wmap_focus_cell_x - 1) * WMAP_EFFECT_CELL_SIZE -
                     (g_wmap_view.x * WMAP_EFFECT_MAP_SCALE) / g_wmap_view.projection_scale) *
                    WMAP_EFFECT_PROJECTION_SCALE) / g_wmap_view.projection_scale);
    position.vy = ((((g_wmap_focus_cell_y - 1) * WMAP_EFFECT_CELL_SIZE -
                     (g_wmap_view.y * WMAP_EFFECT_MAP_SCALE) / g_wmap_view.projection_scale) *
                    WMAP_EFFECT_PROJECTION_SCALE) / g_wmap_view.projection_scale);
    gte_ldv0(&position);
    gte_rtps();
    if (D_800DCEB8.vx != g_wmap_effect_camera_rotation_offset.vx)
    {
        if (D_800DCEB8.vx < g_wmap_effect_camera_rotation_offset.vx)
        {
            g_wmap_effect_camera_rotation_offset.vx = (u16)g_wmap_effect_camera_rotation_offset.vx - (u16)D_80139210.vx;
        }
        else
        {
            g_wmap_effect_camera_rotation_offset.vx = (u16)g_wmap_effect_camera_rotation_offset.vx + (u16)D_80139210.vx;
        }
    }
    if (D_800DCEB8.vy != g_wmap_effect_camera_rotation_offset.vy)
    {
        if (D_800DCEB8.vy < g_wmap_effect_camera_rotation_offset.vy)
        {
            g_wmap_effect_camera_rotation_offset.vy = (u16)g_wmap_effect_camera_rotation_offset.vy - (u16)D_80139210.vy;
        }
        else
        {
            g_wmap_effect_camera_rotation_offset.vy = (u16)g_wmap_effect_camera_rotation_offset.vy + (u16)D_80139210.vy;
        }
    }
    if (D_800DCEB8.vz != g_wmap_effect_camera_rotation_offset.vz)
    {
        if (D_800DCEB8.vz < g_wmap_effect_camera_rotation_offset.vz)
        {
            g_wmap_effect_camera_rotation_offset.vz = (u16)g_wmap_effect_camera_rotation_offset.vz - (u16)D_80139210.vz;
        }
        else
        {
            g_wmap_effect_camera_rotation_offset.vz = (u16)g_wmap_effect_camera_rotation_offset.vz + (u16)D_80139210.vz;
        }
    }
    if (D_80139200.vx != g_wmap_effect_camera_translation_offset.vx)
    {
        if (D_80139200.vx < g_wmap_effect_camera_translation_offset.vx)
        {
            g_wmap_effect_camera_translation_offset.vx = g_wmap_effect_camera_translation_offset.vx - D_80139968.vx;
        }
        else
        {
            g_wmap_effect_camera_translation_offset.vx = g_wmap_effect_camera_translation_offset.vx + D_80139968.vx;
        }
    }
    if (D_80139200.vy != g_wmap_effect_camera_translation_offset.vy)
    {
        if (D_80139200.vy < g_wmap_effect_camera_translation_offset.vy)
        {
            g_wmap_effect_camera_translation_offset.vy = g_wmap_effect_camera_translation_offset.vy - D_80139968.vy;
        }
        else
        {
            g_wmap_effect_camera_translation_offset.vy = g_wmap_effect_camera_translation_offset.vy + D_80139968.vy;
        }
    }
    if (D_8013B29C == 0)
    {
        g_wmap_effect_camera_rotation_offset.vx = 0;
        g_wmap_effect_camera_rotation_offset.vy = 0;
        g_wmap_effect_camera_rotation_offset.vz = 0;
        g_wmap_effect_camera_translation_offset.vx = 0;
        g_wmap_effect_camera_translation_offset.vy = 0;
        g_wmap_effect_camera_translation_offset.vz = 0;
        D_80139200.vx = 0;
        D_80139200.vy = 0;
        D_80139200.vz = 0;
        D_800DCEB8.vx = 0;
        D_800DCEB8.vy = 0;
        D_800DCEB8.vz = 0;
        g_wmap_focus_screen_position.point.x = 0xA4;
        g_wmap_focus_screen_position.point.y = 0x69;
        return 0;
    }
    gte_stsxy(&g_wmap_focus_screen_position.packed);
    return D_8013B29C;
}

/**
 * @brief Spawn one free particle, then animate and draw all active particles.
 * @param config Particle range, spawn bounds, animation, and drawing settings.
 */
void func_8006C448(WmapBoxParticleConfig* config)
{
    SVECTOR position;
    s32 screen_position;
    s32 draw_index;
    s32 spawn_index;
    s32 i;
    WmapSpriteActor* actor;
    WmapAnimationSlot* animation_slot;
    s32 animation_frame;
    s32 offset;
    s16* offsets;
    u8* data;
    u8* cursor;
    WmapSpriteActor* spawn_actor;
    WmapBoxParticleMotion* spawn_motion;
    WmapBoxParticleMotion* motion;

    for (i = 0; i < config->count; i++)
    {
        spawn_index = config->first + i;
        spawn_actor = &g_wmap_sprite_actors[spawn_index];
        spawn_motion = (WmapBoxParticleMotion*)&g_wmap_actor_motions[spawn_index];
        if (spawn_motion->state == 0)
        {
            spawn_motion->state = 1;
            spawn_motion->x = config->x_min + ((rand() * config->x_range) >> 15);
            spawn_motion->y = config->y_min + ((rand() * config->y_range) >> 15);
            spawn_motion->z = config->z_min + ((rand() * config->z_range) >> 15);
            spawn_actor->resource_index = 0;
            spawn_actor->scale_index = 0xF;
            spawn_actor->sequence = config->sequence;
            spawn_actor->previous_sequence = -1;
            spawn_actor->target_shade = config->target_shade;
            spawn_actor->shade = config->shade;
            spawn_actor->shade_step = config->shade_step;
            break;
        }
    }
    for (i = 0; i < config->count; i++)
    {
        draw_index = config->first + i;
        actor = &g_wmap_sprite_actors[draw_index];
        motion = (WmapBoxParticleMotion*)&g_wmap_actor_motions[draw_index];
        if (motion->state != 0)
        {
            position.vx = motion->x;
            position.vy = motion->y;
            position.vz = motion->z;
            gte_ldv0(&position);
            gte_rtps();
            animation_slot = &g_wmap_actor_animations[i + config->first];
            offsets = (s16*)animation_slot->data;
            data = (u8*)offsets;
            if (actor->previous_sequence != actor->sequence)
            {
                u8* sequence_cursor;
                actor->previous_sequence = actor->sequence;
                offset = offsets[actor->sequence];
                actor->remaining = 1;
                sequence_cursor = (u8*)offsets + offset;
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
                actor->frame_data = data + offsets[animation_frame + WMAP_ANIMATION_FRAME_TABLE_INDEX];
            }
            gte_stsxy(&screen_position);
            wmap_draw_actor_sprite(actor, screen_position, config->texture_index, config->depth, 0);
        }
    }

}
