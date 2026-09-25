#ifndef FIELD_OBJECT_STATE_H
#define FIELD_OBJECT_STATE_H

#include "field_types.h"

#define FIELD_OBJECT_COUNT 13
#define FIELD_OBJECT_HISTORY_COUNT 48
#define FIELD_OBJECT_EFFECT_SCALE_MASK 0x3FF
/** @brief Two-bit movement mode in FieldMovementStatus (0, 0x800 or 0x1000). */
#define FIELD_OBJECT_MOVEMENT_MODE_MASK 0x1800
#define FIELD_OBJECT_FOOTPRINT_STRENGTH_RANGE 256

/** @brief Whole-unit X/Z sample in an object's movement history. */
typedef struct
{
    s16 x;
    s16 z;
} FieldObjectHistoryPoint;

/** @brief Packed collision dimensions with signed and unsigned extent views. */
typedef union
{
    u32 word;
    struct
    {
        s16 center_offset;
        u16 diameter;
    } half;
    struct
    {
        s16 center_offset;
        s16 extent;
    } signed_half;
} FieldCollisionSize;

/** @brief Movement flags/effect scale and resolved collision height. */
typedef union
{
    u32 word;
    struct
    {
        u16 flags;
        s16 height;
    } half;
} FieldMovementStatus;

/** @brief Contact flags, animation/effect actor bindings, and collected target count. */
typedef union
{
    u32 flags;
    struct
    {
        u8 flags_low;
        u8 animation_actor_index;
        u8 controller_index;
        u8 target_count;
    } bytes;
} FieldContactStatus;

/**
 * @brief Runtime HP, movement history, bounds, and effect state for a field object.
 * @note Indices correspond to g_field_actors, not the animation-slot pool.
 * Each record occupies 0x23C bytes. Unresolved regions remain explicit padding.
 */
typedef struct FieldObjectRuntime
{
    s32 maximum_hp;
    s32 current_hp;
    /** @brief Low 24 bits track the animated HP gauge; the high byte carries HUD flags. */
    u32 hp_display_flags;
    u32 object_flags;
    u32 group_flags;
    s32 record_id;
    u16 interaction_flags;
    u16 state_entries[2];
    u8 pad_0x1e[0x3C - 0x1E];
    s32 sequence_command;
    s32 current_sequence_animation;
    s32 sequence_cursor;
    u16 status_intensity;
    u16 effect_intensity;
    u8 pad_0x4c[0x5C - 0x4C];
    s32 effect_angle;
    u8 pad_0x60[8];
    /** @brief Stat-derived footprint strength, saturated to 255 during setup. */
    u16 effect_footprint_strength;
    u8 pad_0x6a[2];
    FieldObjectHistoryPoint position_history[FIELD_OBJECT_HISTORY_COUNT];
    FieldCollisionSize collision;
    Vec2s attachment_points[4];
    /** @brief Projected bounds with a packed-word view for empty-box tests. */
    union
    {
        struct
        {
            s16 left;
            s16 top;
            s16 right;
            s16 bottom;
        } half;
        s32 words[2];
    } bounds;
    /** @brief Projected effect corners, also consumed as packed XY pairs. */
    union
    {
        Vec2s points[8];
        s32 words[8];
    } effect_vertices;
    u8 pad_0x168[5];
    s8 linked_effect_index;
    u8 history_index;
    u8 pad_0x16f[2];
    u8 sequence_delay;
    u8 pad_0x172[2];
    /** @brief The low ten flag bits also supply effect radius or percentage scale. */
    FieldMovementStatus movement;
    FieldContactStatus contact;
    u8 pad_0x17c[4];
    u8 targets[FIELD_OBJECT_COUNT];
    /** @brief Counted while object flag 0x8000 is set; a party member recovers at its type's limit. */
    u8 retry_count;
    u8 interaction_kind;
    u8 pad_0x18f;
    Vec2s ground_attachment_points[3];
    s32 contact_index;
    s32 surface;
    u8 pad_0x1a4[4];
    u8 tint_red;
    u8 tint_green;
    u8 tint_blue;
    u8 tint_flash_timer;
    u8 pad_0x1ac[0x23C - 0x1AC];
} FieldObjectRuntime;

extern FieldObjectRuntime g_field_object_states[FIELD_OBJECT_COUNT];

#endif
