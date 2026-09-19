/**
 * @file field_effect_dispatch.c
 * @brief Dispatch and render active field effect records.
 */

#include "common.h"
#include "field_types.h"
#include "field_effect_types.h"
#include "field_mesh_render.h"
#include "field_effect_dispatch.h"
#include "sdk/libgpu.h"
#include "sdk/inline_c.h"

#define FIELD_EFFECT_SCRATCH_MATRIX ((void *) 0x1F800000)
#define FIELD_EFFECT_SCRATCH_SCREEN_ORIGIN ((Vec2s *) 0x1F800040)
#define FIELD_EFFECT_SCRATCH_FOOTPRINT ((s16 *) 0x1F800064)
#define FIELD_EFFECT_SCREEN_CENTER_X 160
#define FIELD_EFFECT_SCREEN_CENTER_Y 112
#define FIELD_EFFECT_OT_MAX_DEPTH 0xFFF

#define FIELD_SPRITE_FRAME_FOOTPRINT 0x20
#define FIELD_SPRITE_FRAME_FLIP_X 0x40
#define FIELD_SPRITE_FRAME_FLIP_Y 0x80
#define FIELD_SPRITE_TEXTURE_MODE_MASK 3
#define FIELD_PART_EFFECT_FOOTPRINT 0x00100000

typedef enum
{
    FIELD_EFFECT_RENDER_SPRITE = 0,
    FIELD_EFFECT_RENDER_ACTOR_SPRITE = 1,
    FIELD_EFFECT_RENDER_OWNER_SPRITE = 2,
    FIELD_EFFECT_RENDER_TRACK_SPRITE = 3,
    FIELD_EFFECT_RENDER_RADIAL_FAN = 4,
    FIELD_EFFECT_RENDER_TRAIL = 5,
    FIELD_EFFECT_RENDER_RIBBON = 6,
    FIELD_EFFECT_RENDER_RADIAL_LINES = 0xF6,
    FIELD_EFFECT_RENDER_MESH_2 = 0xF7,
    FIELD_EFFECT_RENDER_MESH_1 = 0xF8,
    FIELD_EFFECT_RENDER_MESH_0 = 0xF9,
    FIELD_EFFECT_RENDER_RING = 0xFA,
    FIELD_EFFECT_RENDER_FAN = 0xFB,
    FIELD_EFFECT_RENDER_MARKER = 0xFC,
    FIELD_EFFECT_RENDER_LINKED_SPRITE = 0xFD
} FieldEffectRenderState;

/** @brief Loaded field resource slot. */
typedef struct
{
    u8 *start;
    u8 *end;
    u8 unknown_0x08;
    u8 slot_index;
    u8 padA[4];
    s16 unknown_0x0e;
    u32 flags;
} FieldResourceEntry;

extern FieldActorState g_field_actor_slots[80];
extern FieldMotionRecord D_800FDF58[];
extern FieldMotionRecord g_field_effect_records[256];
extern FieldObjectPlacement D_80105AE0[];
extern FieldResourceEntry g_field_resource_entries[];
extern FieldMotionRecord g_field_effect_records_end;
extern s32 g_field_track_index;
extern u8 *D_801058D4;
extern s32 D_800F22A0;
extern s32 D_800F22A4;
extern s32 D_800F22A8;
extern u16 g_field_texture_slot_flags[];

static s32 *field_render_effect_sprite_frames(FieldMotionRecord *effect, s32 *packet_cursor, s32 *ordering_table, u8 *frame_data);

/**
 * @brief Render each active field effect according to its render state.
 * @param render_context Ordering table and primitive packet cursor.
 * @see decomp.me (100%)
 */
void field_render_effects(FieldRenderContext *render_context)
{
    FieldMotionRecord *effect;
    FieldObjectPlacement *object_state;
    FieldActorPartDef *part;
    u32 *ordering_table;
    FieldResourceEntry *resources;
    s32 *packet_cursor;
    s32 frame_result;
    s32 object_index;
    s32 actor_or_object_index;
    s32 actor_address;
    FieldActorState *actor;
    s32 part_offset;
    s32 value;

    effect = g_field_effect_records;
    resources = g_field_resource_entries;
    ordering_table = &render_context->ordering_table;
    packet_cursor = render_context->packet_cursor;

    if (effect != &g_field_effect_records[FIELD_EFFECT_ACTIVE_RECORD_COUNT])
    {
        do
        {
            g_field_track_index = effect->track_index;
            if (effect->state != FIELD_EFFECT_DISABLED && effect->state != FIELD_EFFECT_RETIRED)
            {
                switch (effect->state)
                {
                case FIELD_EFFECT_RENDER_RADIAL_LINES:
                    packet_cursor = field_render_effect_radial_lines(effect, packet_cursor, ordering_table);
                    break;

                case FIELD_EFFECT_RENDER_MESH_2:
                case FIELD_EFFECT_RENDER_MESH_1:
                case FIELD_EFFECT_RENDER_MESH_0:
                    if (g_field_actor_slots[effect->actor_index].parts[effect->part_index].unknown_0x23 < 8)
                    {
                        packet_cursor = field_render_lit_effect_mesh(effect, FIELD_EFFECT_RENDER_MESH_0 - effect->state, packet_cursor, ordering_table);
                    }
                    else
                    {
                        packet_cursor = field_render_effect_mesh(effect, FIELD_EFFECT_RENDER_MESH_0 - effect->state, packet_cursor, ordering_table);
                    }
                    break;

                case FIELD_EFFECT_RENDER_RING:
                    packet_cursor = field_render_effect_ring(effect, packet_cursor, ordering_table);
                    break;

                case FIELD_EFFECT_RENDER_FAN:
                    packet_cursor = field_render_effect_fan(effect, packet_cursor, ordering_table);
                    break;

                case FIELD_EFFECT_RENDER_MARKER:
                    packet_cursor = field_render_effect_marker(effect, packet_cursor, ordering_table);
                    break;

                case FIELD_EFFECT_RENDER_LINKED_SPRITE:
                    actor_or_object_index = (s32) &g_field_actor_slots[effect->actor_index];
                    part = &((FieldActorState *) actor_or_object_index)->parts[effect->part_index];
                    value = ((u32) part->placement_flags >> 0x12) & 0x3F;
                    if (((u32) (value - 0xA) < 0xA) || value == 0x28)
                    {
                        actor_or_object_index = ((FieldActorState *) actor_or_object_index)->track_object_indices[effect->track_index];
                        value = actor_or_object_index << 2;
                        value += actor_or_object_index;
                        value <<= 2;
                        value += actor_or_object_index;
                        value <<= 2;
                        value += (s32) D_800FDF58;
                        object_state = &D_80105AE0[actor_or_object_index];
                        value = *(u8 *) (value + 0x3B);
                        frame_result = (s32) resources[value].start;
                    }
                    else
                    {
                        actor_or_object_index = ((FieldActorState *) actor_or_object_index)->owner_object_index;
                        value = actor_or_object_index << 2;
                        value += actor_or_object_index;
                        value <<= 2;
                        value += actor_or_object_index;
                        value <<= 2;
                        value += (s32) D_800FDF58;
                        object_state = &D_80105AE0[actor_or_object_index];
                        value = *(u8 *) (value + 0x3B);
                        frame_result = (s32) resources[value].start;
                    }
                    object_state->linked_effect_index = (s8) (effect - g_field_effect_records);
                    if (frame_result != 0)
                    {
                        frame_result = field_advance_actor_part_animation_frame(effect, (u8 *) frame_result);
                        if (frame_result != 0)
                        {
                            if (frame_result >= 0)
                            {
                                packet_cursor = func_80077FB4(effect, packet_cursor, ordering_table, frame_result, (*(u8 *) &object_state->state_flags & 1) ^ 1, part);
                            }
                            else
                            {
                                packet_cursor = func_80075C88(effect, packet_cursor, ordering_table, frame_result, (*(u8 *) &object_state->state_flags & 1) ^ 1, part);
                            }
                        }
                    }
                    break;

                case FIELD_EFFECT_RENDER_SPRITE:
                    frame_result = field_advance_actor_part_animation_frame(effect, D_801058D4);
                    if (frame_result != 0)
                    {
                        packet_cursor = field_render_effect_sprite_frames(effect, packet_cursor, ordering_table, (u8 *) frame_result);
                    }
                    break;

                case FIELD_EFFECT_RENDER_ACTOR_SPRITE:
                    frame_result = (s32) g_field_actor_slots[effect->actor_index].track_data;
                    if (frame_result != 0)
                    {
                        frame_result = field_advance_actor_part_animation_frame(effect, (u8 *) frame_result);
                        if (frame_result != 0)
                        {
                            packet_cursor = field_render_effect_sprite_frames(effect, packet_cursor, ordering_table, (u8 *) frame_result);
                        }
                    }
                    break;

                case FIELD_EFFECT_RENDER_OWNER_SPRITE:
                    actor = &g_field_actor_slots[effect->actor_index];
                    actor_or_object_index = effect->part_index;
                    part_offset = actor_or_object_index * sizeof(FieldActorPartDef);
                    actor_or_object_index = (s32) actor->parts;
                    object_index = actor->owner_object_index;
                    part = (FieldActorPartDef *) (actor_or_object_index + part_offset);
                    value = object_index << 2;
                    value += object_index;
                    value <<= 2;
                    value += object_index;
                    value <<= 2;
                    value += (s32) D_800FDF58;
                    object_state = &D_80105AE0[object_index];
                    value = *(u8 *) (value + 0x3B);
                    frame_result = (s32) resources[value].start;
                    object_state->linked_effect_index = (s8) (effect - g_field_effect_records);
                    if (frame_result != 0)
                    {
                        frame_result = field_advance_actor_part_animation_frame(effect, (u8 *) frame_result);
                        if (frame_result != 0)
                        {
                            if (frame_result >= 0)
                            {
                                packet_cursor = func_80077FB4(effect, packet_cursor, ordering_table, frame_result, (*(u8 *) &object_state->state_flags & 1) ^ 1, part);
                            }
                            else
                            {
                                packet_cursor = func_80075C88(effect, packet_cursor, ordering_table, frame_result, (*(u8 *) &object_state->state_flags & 1) ^ 1, part);
                            }
                        }
                    }
                    break;

                case FIELD_EFFECT_RENDER_TRACK_SPRITE:
                    value = effect->actor_index;
                    actor_address = (s32) &g_field_actor_slots[value];
                    value = effect->part_index;
                    part = &((FieldActorState *) actor_address)->parts[value];
                    value = effect->track_index;
                    object_index = ((FieldActorState *) actor_address)->track_object_indices[value];
                    value = object_index << 2;
                    value += object_index;
                    value <<= 2;
                    value += object_index;
                    value <<= 2;
                    value += (s32) D_800FDF58;
                    object_state = &D_80105AE0[object_index];
                    value = *(u8 *) (value + 0x3B);
                    frame_result = (s32) resources[value].start;
                    object_state->linked_effect_index = (s8) (effect - g_field_effect_records);
                    if (frame_result != 0)
                    {
                        frame_result = field_advance_actor_part_animation_frame(effect, (u8 *) frame_result);
                        if (frame_result != 0)
                        {
                            if (frame_result >= 0)
                            {
                                packet_cursor = func_80077FB4(effect, packet_cursor, ordering_table, frame_result, (*(u8 *) &object_state->state_flags & 1) ^ 1, part);
                            }
                            else
                            {
                                packet_cursor = func_80075C88(effect, packet_cursor, ordering_table, frame_result, (*(u8 *) &object_state->state_flags & 1) ^ 1, part);
                            }
                        }
                    }
                    break;

                case FIELD_EFFECT_RENDER_RADIAL_FAN:
                    packet_cursor = field_render_effect_radial_fan(effect, packet_cursor, ordering_table);
                    break;

                case FIELD_EFFECT_RENDER_TRAIL:
                    packet_cursor = field_render_effect_trail(effect, packet_cursor, ordering_table);
                    break;

                case FIELD_EFFECT_RENDER_RIBBON:
                    packet_cursor = field_render_effect_ribbon(effect, packet_cursor, ordering_table);
                    break;
                }
            }
            effect++;
        } while (effect != &g_field_effect_records_end);
    }

    render_context->packet_cursor = packet_cursor;
}

/**
 * @brief Render the sprite frames for one field effect.
 * @param effect Effect state and world position.
 * @param packet_cursor Current primitive packet cursor.
 * @param ordering_table Depth ordering table.
 * @param frame_data Encoded sprite-frame data.
 * @return Updated primitive packet cursor.
 * @see decomp.me (100%)
 */
static s32 *field_render_effect_sprite_frames(FieldMotionRecord *effect, s32 *packet_cursor, s32 *ordering_table, u8 *frame_data)
{
    Vec2s *screen_origin = FIELD_EFFECT_SCRATCH_SCREEN_ORIGIN;
    s16 *shadow_footprint = FIELD_EFFECT_SCRATCH_FOOTPRINT;
    s32 matrix_address = (s32) FIELD_EFFECT_SCRATCH_MATRIX;
    s32 frame_count;
    s32 shadow_count;
    s32 texture_slot;
    s32 packed_color;
    FieldActorState *actor;
    FieldActorPartDef *part;
    FieldActorState *actors;
    s32 height_minus_one;
    s32 width_or_mode;
    s32 frame_x;
    s32 frame_y_or_clut;
    s32 value0;
    s32 value1;
    s32 clut_x;
    s32 clut;
    u16 *texture_slot_flags;

    shadow_count = 0;
    texture_slot = 2;
    actors = g_field_actor_slots;
    part = &actors[effect->actor_index].parts[effect->part_index];
    actor = &actors[effect->actor_index];
    if (actor->owner_object_index < 2)
    {
        texture_slot = actor->owner_object_index;
    }
    field_build_effect_part_matrix(effect, part, FIELD_EFFECT_SCRATCH_MATRIX, actor);
    gte_SetRotMatrix(FIELD_EFFECT_SCRATCH_MATRIX);

    screen_origin->x = FIELD_EFFECT_SCREEN_CENTER_X + D_800F22A0 / 256 + effect->x / 256;
    screen_origin->y = FIELD_EFFECT_SCREEN_CENTER_Y + D_800F22A4 / 256 + effect->y / 256 - effect->z / 512 - D_800F22A8 / 512;

    frame_count = *frame_data++;
    field_resolve_effect_part_color(actor, effect, part, &packed_color);

    if (frame_count != 0)
    {
        texture_slot_flags = g_field_texture_slot_flags;
        do
        {
            u8 flags = frame_data[7];
            if (!(flags & FIELD_SPRITE_FRAME_FOOTPRINT))
            {
                value1 = packed_color;
                setlen(packet_cursor, 9);
                *(s32 *) &((POLY_FT4 *) packet_cursor)->r0 = value1;
                setcode(packet_cursor, 0x2C);
                setSemiTrans((POLY_FT4 *) packet_cursor, effect->flags & FIELD_EFFECT_SEMITRANSPARENT);
                width_or_mode = frame_data[4];
                height_minus_one = frame_data[5] - 1;
                effect->sprite_height_minus_one = height_minus_one;
                {
                    s8 frame_y = frame_data[1];
                    frame_y_or_clut = frame_y;
                }
                if (!(effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED))
                {
                    frame_x = *(s8 *) frame_data;
                }
                else
                {
                    frame_x = -*(s8 *) frame_data - width_or_mode;
                }
                width_or_mode -= 1;
                if (((Vec2s *) part)->y & 1)
                {
                    frame_y_or_clut -= FIELD_EFFECT_SCREEN_CENTER_Y;
                }
                field_project_effect_sprite_quad(effect, screen_origin, packet_cursor, width_or_mode, height_minus_one, frame_x, frame_y_or_clut, frame_data, matrix_address);

                if ((frame_data[7] ^ (effect->facing_or_reward_kind >> 1)) & FIELD_SPRITE_FRAME_FLIP_X)
                {
                    value0 = frame_data[2];
                    ((POLY_FT4 *) packet_cursor)->u3 = value0;
                    ((POLY_FT4 *) packet_cursor)->u1 = value0;
                    do { value0 += width_or_mode; } while (0);
                    ((POLY_FT4 *) packet_cursor)->u2 = value0;
                    ((POLY_FT4 *) packet_cursor)->u0 = value0;
                }
                else
                {
                    value0 = frame_data[2];
                    ((POLY_FT4 *) packet_cursor)->u2 = value0;
                    ((POLY_FT4 *) packet_cursor)->u0 = value0;
                    do { value0 += width_or_mode; } while (0);
                    ((POLY_FT4 *) packet_cursor)->u3 = value0;
                    ((POLY_FT4 *) packet_cursor)->u1 = value0;
                }
                if (frame_data[7] & FIELD_SPRITE_FRAME_FLIP_Y)
                {
                    value0 = frame_data[3];
                    ((POLY_FT4 *) packet_cursor)->v3 = value0;
                    ((POLY_FT4 *) packet_cursor)->v2 = value0;
                    do { value0 += height_minus_one; } while (0);
                    ((POLY_FT4 *) packet_cursor)->v1 = value0;
                    ((POLY_FT4 *) packet_cursor)->v0 = value0;
                }
                else
                {
                    value0 = frame_data[3];
                    ((POLY_FT4 *) packet_cursor)->v1 = value0;
                    ((POLY_FT4 *) packet_cursor)->v0 = value0;
                    value0 += height_minus_one;
                    ((POLY_FT4 *) packet_cursor)->v3 = value0;
                    ((POLY_FT4 *) packet_cursor)->v2 = value0;
                }
                width_or_mode = frame_data[7] & FIELD_SPRITE_TEXTURE_MODE_MASK;
                if (width_or_mode == 2)
                {
                    frame_y_or_clut = 0x1F2;
                    if (actor->owner_object_index < 2)
                    {
                        *(s16 *) &((POLY_FT4 *) packet_cursor)->tpage = ((texture_slot_flags[texture_slot] & 3) << 7) | (((u32) part->behavior_flags >> 0x11) & 0x60) | 0x10 | ((((actor->owner_object_index << 6) + 0x340) & 0x3FF) >> 6);
                        frame_y_or_clut = (actor->owner_object_index * 2) + 0x1EE;
                        goto mode_done;
                    }
                    value0 = texture_slot;
                    value0 <<= 1;
                    value0 += (s32) texture_slot_flags;
                    value1 = ((*(u16 *) value0 & 3) << 7) | (((u32) part->behavior_flags >> 0x11) & 0x60);
                    value1 |= 5;
                }
                else
                {
                    frame_y_or_clut = (width_or_mode * 2) + 0x1EA;
                    value0 = (((width_or_mode << 6) + 0x180) & 0x3FF) >> 6;
                    value1 = ((u32) part->behavior_flags >> 0x11) & 0x60;
                    value1 |= value0;
                }
                *(s16 *) &((POLY_FT4 *) packet_cursor)->tpage = value1;
mode_done:

                if (((u32) part->track_flags >> 0x15) & 1)
                {
                    *(s16 *) &((POLY_FT4 *) packet_cursor)->clut = (frame_y_or_clut + 1) << 6;
                }
                else
                {
                    value0 = part->placement_flags >> 0xC;
                    switch (value0 & 3)
                    {
                    case 1:
                    {
                        u8 uv = part->palette_selector;
                        clut_x = uv & 0xF;
                        if (uv >= 0x10)
                        {
                            value0 = (frame_y_or_clut + 1) << 6;
                        }
                        else
                        {
                            value0 = frame_y_or_clut << 6;
                        }
                        value0 |= clut_x;
                        *(s16 *) &((POLY_FT4 *) packet_cursor)->clut = value0;
                        break;
                    }
                    case 2:
                        clut = frame_y_or_clut << 6;
                        if (actor->owner_object_index >= 3)
                        {
                            value0 = part->palette_selector & 0x3F;
                        }
                        else
                        {
                    case 0:
                            clut = frame_y_or_clut << 6;
                            value0 = frame_data[6] & 0x3F;
                        }
                        clut |= value0;
                        *(s16 *) &((POLY_FT4 *) packet_cursor)->clut = clut;
                        break;
                    }
                }

                {
                    if ((effect->flags & FIELD_EFFECT_SCREEN_SPACE) || ((value1 = effect->z >> 7), value1 < 0))
                    {
                        addPrim(&ordering_table[0], packet_cursor);
                        packet_cursor = (s32 *) ((u8 *) packet_cursor + sizeof(POLY_FT4));
                    }
                    else if (value1 > FIELD_EFFECT_OT_MAX_DEPTH)
                    {
                        addPrim(&ordering_table[FIELD_EFFECT_OT_MAX_DEPTH], packet_cursor);
                        packet_cursor = (s32 *) ((u8 *) packet_cursor + sizeof(POLY_FT4));
                    }
                    else
                    {
                        setaddr(packet_cursor, getaddr(&ordering_table[value1]));
                        setaddr(&ordering_table[effect->z >> 7], packet_cursor);
                        packet_cursor = (s32 *) ((u8 *) packet_cursor + sizeof(POLY_FT4));
                    }
                }
            }
            else if (((flags & 0xF) == 2) && (part->effect_flags & FIELD_PART_EFFECT_FOOTPRINT))
            {
                shadow_footprint[0] = (*(s8 *) frame_data * part->footprint_scale_x) >> 6;
                shadow_footprint[1] = ((s8) frame_data[1] * part->footprint_scale_y) >> 6;
                shadow_footprint[2] = ((s8) frame_data[2] * part->footprint_scale_x) >> 6;
                shadow_footprint[3] = ((s8) frame_data[3] * part->footprint_scale_y) >> 6;
                shadow_footprint[4] = ((s8) frame_data[4] * part->footprint_scale_x) >> 6;
                shadow_footprint[5] = ((s8) frame_data[5] * part->footprint_scale_y) >> 6;
                shadow_footprint[6] = ((s8) frame_data[6] * part->footprint_scale_x) >> 6;
                shadow_count += 1;
                shadow_footprint[7] = ((s8) frame_data[8] * part->footprint_scale_y) >> 6;
            }
            frame_data += 9;
            frame_count -= 1;
            value0 = *(s32 *) &((POLY_FT4 *) packet_cursor)[-1].r0;
            *(s32 *) &((POLY_FT4 *) packet_cursor)->r0 = value0;
        } while (frame_count != 0);
    }

    if (shadow_count != 0)
    {
        do { do { do { do { do { packet_cursor = field_render_actor_ground_shadow(effect, packet_cursor, ordering_table, shadow_footprint); } while (0); } while (0); } while (0); } while (0); } while (0);
    }
    return packet_cursor;
}
