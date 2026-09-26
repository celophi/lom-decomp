/** @file field_pair_indicators.c
 * @brief Detect nearby party pairs and draw their animated pair indicators.
 */

#include "common.h"
#include "field_actor.h"
#include "field_calls.h"
#include "field_contact_geometry.h"
#include "field_effect_render_state.h"
#include "field_object_state.h"
#include "field_runtime.h"
#include "gpu_packet.h"
#include "sdk/libgpu.h"

/** @brief Two party members closer than this (whole units) form a pair. */
#define FIELD_PAIR_RANGE 32

/** @brief Object flags that keep a party member out of every pair. */
#define FIELD_PAIR_EXCLUDED_FLAGS 0x23E4

/** @brief Terminator of g_field_pair_indicator_list. */
#define FIELD_PAIR_LIST_END 0xFF

/** @brief Frame counter value of an indicator that is waiting for its pair to come close. */
#define FIELD_INDICATOR_IDLE -2

/** @brief Frame counter value of an indicator that has finished its animation. */
#define FIELD_INDICATOR_DONE -1

/** @brief Length of the indicator animation, in frames. */
#define FIELD_INDICATOR_FRAMES 384

/** @brief Frame after which the indicator fades out. */
#define FIELD_INDICATOR_FADE_START 256

/** @brief Ribbon animation table shared with the effect ribbons. */
#define FIELD_RIBBON_FRAME_COUNT 12
#define FIELD_RIBBON_UV_VARIANT 0x01
#define FIELD_RIBBON_FLIP_V 0x40
#define FIELD_RIBBON_FLIP_U 0x80

/** @brief Ordering-table entry the indicators are drawn into. */
#define FIELD_INDICATOR_OT_INDEX 3

/**
 * @brief Address of element @p index of @p base, summed as integers.
 * @note The original adds the scaled index before the base address; pointer
 *       arithmetic in this unit emits the base first.
 */
#define FIELD_ELEMENT_AT(base, index) ((void*)((index) * sizeof(*(base)) + (u32)(base)))

/** @brief Texture coordinate pair, addressable whole or by coordinate. */
typedef union
{
    u16 word;
    struct
    {
        u8 u;
        u8 v;
    } c;
} FieldIndicatorUv;

/** @brief POLY_FT4 whose corner positions are written as packed x/y words. */
typedef struct
{
    u_long tag;
    u8 r0;
    u8 g0;
    u8 b0;
    u8 code;
    u32 xy0;
    FieldIndicatorUv uv0;
    u16 clut;
    u32 xy1;
    FieldIndicatorUv uv1;
    u16 tpage;
    u32 xy2;
    FieldIndicatorUv uv2;
    u16 pad1;
    u32 xy3;
    FieldIndicatorUv uv3;
    u16 pad2;
} FieldIndicatorQuad;

static void field_draw_pair_indicator(s32 slot, FieldRenderHalf* render_half);

extern s32 g_field_party_hud_order[];
/** @brief Packed x/y screen corners of the four indicators, four words each. */
extern u32 g_field_pair_indicator_corners[];
/** @brief Animation frame of each pair indicator (0 and 1 share the first). */
extern s32 g_field_pair_indicator_counters[];
extern s32 g_field_active_group;
extern s32 g_field_scene_mode_bit;
/** @brief Number of pairs in g_field_pair_indicator_list. */
extern s32 g_field_pair_indicator_count;
/** @brief Nonzero while a script has turned the pair indicators off. */
extern s32 g_field_pair_indicators_disabled;
/** @brief Actor index pairs of the close pairs, ended by FIELD_PAIR_LIST_END. */
extern u8 g_field_pair_indicator_list[];
extern s32 g_field_text_session_active;

/**
 * @brief Find the close party pairs of this frame and draw their indicators.
 *
 * With two party members present they form indicator 0; with three, each
 * member pairs with the next one in g_field_party_hud_order (indicators 1-3).
 *
 * @param render_half Render half receiving the indicator primitives.
 */
void field_update_pair_indicators(FieldRenderHalf* render_half)
{
    s32 active[FIELD_PARTY_COUNT];
    FieldActor* actor;
    /* Fills active[], then points at the second member of a pair. */
    s32* entry;
    s32 first;

    s32 active_count;
    s32 pair;
    s32 index;
    /* The goto loop below is invisible to loop.c, so the tables and the
       FIELD_ACTOR_UNUSED marker are held in locals (one marker per loop). */
    s32 unused;
    s32 unused_marker;
    FieldObjectRuntime* states;
    s32* order;
    FieldActor* actors;
    s32* counters;
    /* Holds the second member's index, then the pair's distance. */
    s32 distance;

    g_field_pair_indicator_count = 0;
    g_field_pair_indicator_list[0] = FIELD_PAIR_LIST_END;
    if ((g_field_pair_indicators_disabled == 0) && (g_field_scene_mode_bit != 0))
    {
        index = 0;
        if (g_field_active_group != 0)
        {
            active_count = index;
            unused_marker = FIELD_ACTOR_UNUSED;
            actor = g_field_actors;
            entry = active;
            do
            {
                if (actor->presence != unused_marker)
                {
                    *entry = index;
                    entry++;
                    active_count++;
                }
                index++;
                actor++;
            } while (index < FIELD_PARTY_COUNT);
            if (active_count >= 2)
            {
                pair = 0;
                if (active_count == 2)
                {
                    distance = FIELD_PAIR_RANGE;
                    if (!(g_field_object_states[active[0]].object_flags & FIELD_PAIR_EXCLUDED_FLAGS))
                    {
                        distance = active[1];
                        if (g_field_object_states[distance].object_flags & FIELD_PAIR_EXCLUDED_FLAGS)
                        {
                            distance = FIELD_PAIR_RANGE;
                        }
                        else
                        {
                            distance = field_get_position_distance((VECTOR*)&g_field_actors[active[0]], (VECTOR*)&g_field_actors[distance]);
                        }
                    }
                    if (distance < FIELD_PAIR_RANGE)
                    {
                        if (g_field_pair_indicator_counters[0] == FIELD_INDICATOR_IDLE)
                        {
                            g_field_pair_indicator_counters[0] = 0;
                        }
                        if (g_field_pair_indicator_counters[0] != FIELD_INDICATOR_DONE)
                        {
                            /* The count is set to 2 here, not 1; readers stop at the terminator. */
                            g_field_pair_indicator_list[2] = FIELD_PAIR_LIST_END;
                            g_field_pair_indicator_count = 2;
                            g_field_pair_indicator_list[0] = active[0];
                            g_field_pair_indicator_list[1] = active[1];
                        }
                    }
                    else
                    {
                        g_field_pair_indicator_counters[0] = FIELD_INDICATOR_IDLE;
                    }
                    field_draw_pair_indicator(0, render_half);
                }
                else
                {
                    states = g_field_object_states;
                    order = g_field_party_hud_order;
                    actors = g_field_actors;
                    unused = FIELD_ACTOR_UNUSED;
                    counters = g_field_pair_indicator_counters;
                next_pair:
                    first = order[pair];
                    distance = FIELD_PAIR_RANGE;
                    if (!(states[first].object_flags & FIELD_PAIR_EXCLUDED_FLAGS))
                    {
                        entry = FIELD_ELEMENT_AT(order, (pair + 1) % FIELD_PARTY_COUNT);
                        distance = *entry;
                        if (states[distance].object_flags & FIELD_PAIR_EXCLUDED_FLAGS)
                        {
                            distance = FIELD_PAIR_RANGE;
                        }
                        else
                        {
                            distance = field_get_position_distance(FIELD_ELEMENT_AT(actors, first), FIELD_ELEMENT_AT(actors, distance));
                        }
                    }
                    if ((distance < FIELD_PAIR_RANGE) && (actors[order[pair]].presence != unused) &&
                        (actors[order[(pair + 1) % FIELD_PARTY_COUNT]].presence != unused))
                    {
                        if (counters[pair] == FIELD_INDICATOR_IDLE)
                        {
                            counters[pair] = 0;
                        }
                        if (counters[pair] != FIELD_INDICATOR_DONE)
                        {
                            g_field_pair_indicator_list[g_field_pair_indicator_count * 2] = order[pair];
                            g_field_pair_indicator_list[g_field_pair_indicator_count * 2 + 1] = order[(pair + 1) % FIELD_PARTY_COUNT];
                            g_field_pair_indicator_count++;
                        }
                    }
                    else
                    {
                        counters[pair] = FIELD_INDICATOR_IDLE;
                    }
                    pair++;
                    field_draw_pair_indicator(pair, render_half);
                    /* A goto loop: a for loop gets strength reduction and loop-depth
                       allocation weights that the original code does not have. */
                    if (pair < FIELD_PARTY_COUNT)
                    {
                        goto next_pair;
                    }
                }
                index = g_field_pair_indicator_count;
                g_field_pair_indicator_list[index * 2] = FIELD_PAIR_LIST_END;
            }
        }
    }
}

/**
 * @brief Frame counter of pair indicator @p slot.
 * @param counters g_field_pair_indicator_counters.
 * @param slot Indicator 0-3; indicators 0 and 1 share the first counter.
 * @return Address of the indicator's frame counter.
 */
static inline s32* field_get_pair_indicator_counter(s32* counters, s32 slot)
{
    s32* counter;

    counter = counters;
    if (slot != 0)
    {
        counter = FIELD_ELEMENT_AT(counters, slot - 1);
    }
    return counter;
}

/**
 * @brief Advance one pair indicator and draw it as a fading animated quad.
 * @param slot Indicator 0-3; indicators 0 and 1 share the first frame counter.
 * @param render_half Render half whose ordering table and packet cursor receive the quad.
 */
static void field_draw_pair_indicator(s32 slot, FieldRenderHalf* render_half)
{
    FieldIndicatorQuad* quad;
    s32 uv_offset;
    s32 frame;
    s32 brightness;
    u8 swap;
    u8 uv_flags;
    u_long* ordering_table;
    s32* counters;

    quad = (FieldIndicatorQuad*)render_half->primitive_cursor;
    ordering_table = render_half->ordering_table;
    counters = g_field_pair_indicator_counters;
    frame = *field_get_pair_indicator_counter(counters, slot);
    if (frame != FIELD_INDICATOR_IDLE && frame != FIELD_INDICATOR_DONE)
    {
        if (frame >= FIELD_INDICATOR_FRAMES)
        {
            *field_get_pair_indicator_counter(counters, slot) = FIELD_INDICATOR_DONE;
            return;
        }
        if (g_field_text_session_active == 0)
        {
            *field_get_pair_indicator_counter(counters, slot) = *field_get_pair_indicator_counter(counters, slot) + 1;
        }
        if (frame > FIELD_INDICATOR_FADE_START)
        {
            brightness = 255 - ((frame - FIELD_INDICATOR_FADE_START) * 2);
        }
        else
        {
            brightness = 255;
        }
        if (brightness < 0)
        {
            brightness = 0;
        }
        if (brightness > 255)
        {
            brightness = 256;
        }
        setlen(quad, 9);
        SET_BGR0(quad, brightness, brightness, brightness);
        setcode(quad, 0x2E);
        quad->xy0 = g_field_pair_indicator_corners[slot * 4];
        quad->xy1 = g_field_pair_indicator_corners[slot * 4 + 1];
        quad->xy2 = g_field_pair_indicator_corners[slot * 4 + 2];
        quad->xy3 = g_field_pair_indicator_corners[slot * 4 + 3];
        uv_flags = g_field_ribbon_frame_flags[frame % FIELD_RIBBON_FRAME_COUNT];
        uv_offset = (uv_flags & FIELD_RIBBON_UV_VARIANT) * 4;
        quad->uv0.word = g_field_ribbon_uv_corners[uv_offset];
        quad->uv1.word = g_field_ribbon_uv_corners[uv_offset + 1];
        quad->uv2.word = g_field_ribbon_uv_corners[uv_offset + 2];
        quad->uv3.word = g_field_ribbon_uv_corners[uv_offset + 3];
        if (uv_flags & FIELD_RIBBON_FLIP_U)
        {
            swap = quad->uv0.c.u;
            quad->uv0.c.u = quad->uv1.c.u;
            quad->uv1.c.u = swap;
            swap = quad->uv2.c.u;
            quad->uv2.c.u = quad->uv3.c.u;
            quad->uv3.c.u = swap;
        }
        if (uv_flags & FIELD_RIBBON_FLIP_V)
        {
            swap = quad->uv0.c.v;
            quad->uv0.c.v = quad->uv2.c.v;
            quad->uv2.c.v = swap;
            swap = quad->uv1.c.v;
            quad->uv1.c.v = quad->uv3.c.v;
            quad->uv3.c.v = swap;
        }
        quad->tpage = getTPage(0, 1, 448, 0);
        quad->clut = getClut(80, 492);
        addPrim(&ordering_table[FIELD_INDICATOR_OT_INDEX], quad);
        quad++;
        render_half->primitive_cursor = (u8*)quad;
    }
}
