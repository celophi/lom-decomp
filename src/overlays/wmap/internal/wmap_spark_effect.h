#ifndef WMAP_SPARK_EFFECT_H
#define WMAP_SPARK_EFFECT_H

#include "wmap_sequence_runtime.h"
#include "wmap_sprite_render.h"
#include <inline_c.h>
#include "sdk/gte_dmpsx_compat.h"
#include <rand.h>

/** @brief Radial spark state in one 0x14-byte motion slot. */
typedef struct
{
    s16 active;
    s16 angle;
    s32 radius_step;
    s32 radius;
    s16 timer;
    s16 unknown_0e;
    s16 unknown_10;
    s16 unknown_12;
} WmapSpark;

extern WmapSpark g_wmap_actor_motions[];
extern WmapAnimationSlot g_wmap_actor_animations[];

#define WMAP_SPARK_OPERAND_SHIFT 6
#define WMAP_SPARK_PRODUCT_SHIFT 12
#define WMAP_SPARK_SCALE_INDEX 15
#define WMAP_SPARK_RANDOM_ANGLE_SHIFT 3
#define WMAP_SPARK_MIN_RADIUS_STEP 0x1000
#define WMAP_SPARK_LIFETIME_MASK 0x3C

/**
 * @brief Define an update that draws radial sparks and fills empty slots.
 * @param name Update function name.
 * @param slot_count Number of motion slots, starting at slot zero.
 * @param first_actor First sprite actor and animation slot for these sparks.
 * @param texture_index Texture used to draw each spark.
 * @param ot_index Ordering-table index for the sprites.
 * @param lifetime_min Frames added to the masked random lifetime.
 * @param shade_value Shared shade global; only its low 16 bits are read.
 * @note Projects sparks with the current GTE transform.
 */
#define WMAP_DEFINE_RADIAL_SPARK_UPDATE(name, slot_count, first_actor, texture_index,                \
                                       ot_index, lifetime_min, shade_value)                          \
    void name(void)                                                                                  \
    {                                                                                                \
        SVECTOR position;                                                                            \
        s32 screen;                                                                                  \
        WmapSpark* spark;                                                                            \
        WmapSpriteActor* actor;                                                                      \
        s32 count;                                                                                   \
        s32 i;                                                                                       \
                                                                                                     \
        count = 0;                                                                                   \
        for (i = 0; i < (slot_count); i++)                                                           \
        {                                                                                            \
            spark = &g_wmap_actor_motions[i];                                                        \
            if (spark->active != 0)                                                                  \
            {                                                                                        \
                actor = &g_wmap_sprite_actors[(first_actor) + i];                                    \
                position.vx = ((spark->radius >> WMAP_SPARK_OPERAND_SHIFT) *                         \
                    (ccos(spark->angle) >> WMAP_SPARK_OPERAND_SHIFT)) >> WMAP_SPARK_PRODUCT_SHIFT;   \
                position.vy = ((spark->radius >> WMAP_SPARK_OPERAND_SHIFT) *                         \
                    (csin(spark->angle) >> WMAP_SPARK_OPERAND_SHIFT)) >> WMAP_SPARK_PRODUCT_SHIFT;   \
                position.vz = 0;                                                                     \
                gte_ldv0(&position);                                                                 \
                gte_rtps();                                                                          \
                spark->radius += spark->radius_step;                                                 \
                actor->target_shade = *(u16*)&(shade_value);                                         \
                actor->shade = *(u16*)&(shade_value);                                                \
                gte_stsxy(&screen);                                                                  \
                wmap_step_actor_animation(actor, &g_wmap_actor_animations[(first_actor) + i]);       \
                wmap_draw_actor_sprite(actor, screen, (texture_index), (ot_index), 0);               \
                if (--spark->timer == 0)                                                             \
                {                                                                                    \
                    spark->active = 0;                                                               \
                }                                                                                    \
                count++;                                                                             \
            }                                                                                        \
        }                                                                                            \
                                                                                                     \
        /* Sparks that expired above still count for this frame. */                                  \
        for (i = 0; i < (slot_count); i++)                                                           \
        {                                                                                            \
            spark = &g_wmap_actor_motions[i];                                                        \
            if (spark->active == 0)                                                                  \
            {                                                                                        \
                /* This allows one more spark than the intensity value. */                           \
                if (g_wmap_particle_intensity < count++)                                             \
                {                                                                                    \
                    break;                                                                           \
                }                                                                                    \
                actor = &g_wmap_sprite_actors[(first_actor) + i];                                    \
                actor->scale_index = WMAP_SPARK_SCALE_INDEX;                                         \
                actor->previous_sequence = -1;                                                       \
                actor->resource_index = 0;                                                           \
                actor->sequence = 0;                                                                 \
                spark->active = 1;                                                                   \
                spark->angle = (u32)rand() >> WMAP_SPARK_RANDOM_ANGLE_SHIFT;                         \
                spark->radius = 0;                                                                   \
                spark->radius_step = rand() / 2 + WMAP_SPARK_MIN_RADIUS_STEP;                        \
                spark->timer = (rand() & WMAP_SPARK_LIFETIME_MASK) + (lifetime_min);                 \
            }                                                                                        \
        }                                                                                            \
    }

#endif
