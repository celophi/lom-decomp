/**
 * @file field_effect_types.h
 * @brief Shared actor definitions and motion records used by effect update/lifetime code.
 */
#ifndef FIELD_EFFECT_TYPES_H
#define FIELD_EFFECT_TYPES_H

#include "overlays/field/field_object_state.h"
#include "overlays/field/field_types.h"

#define FIELD_EFFECT_POOL_COUNT 0x103
#define FIELD_EFFECT_ACTIVE_RECORD_COUNT 256
/** @brief FieldMotionRecord::state of a hidden effect that is skipped when rendering. */
#define FIELD_EFFECT_DISABLED 0xFE
/** @brief FieldMotionRecord::state of a free (retired) effect record. */
#define FIELD_EFFECT_RETIRED 0xFF

/**
 * @brief Placement kinds of a part's effects (FieldObjectPart::placement_flags bits 18-23).
 * @note The bounds and sibling kinds are ranges: the kind's offset from the first value selects a bounds point or a part.
 */
typedef enum
{
    /** @brief On one of FIELD_PLACEMENT_BOUNDS_POINT_COUNT points of the owner's bounds. */
    FIELD_PLACEMENT_OWNER_BOUNDS = 0x00,
    /** @brief On the same points of the tracked target's bounds. */
    FIELD_PLACEMENT_TRACK_BOUNDS = 0x0A,
    /** @brief On the live effect of part 0 to 7 of the same actor. */
    FIELD_PLACEMENT_SIBLING = 0x14,
    /** @brief At the centre of the screen and the screen edges and corners (screen space). */
    FIELD_PLACEMENT_SCREEN_CENTER = 0x1C,
    FIELD_PLACEMENT_SCREEN_TOP = 0x1D,
    FIELD_PLACEMENT_SCREEN_BOTTOM = 0x1E,
    FIELD_PLACEMENT_SCREEN_LEFT = 0x1F,
    FIELD_PLACEMENT_SCREEN_RIGHT = 0x20,
    FIELD_PLACEMENT_SCREEN_TOP_LEFT = 0x21,
    FIELD_PLACEMENT_SCREEN_TOP_RIGHT = 0x22,
    FIELD_PLACEMENT_SCREEN_BOTTOM_LEFT = 0x23,
    FIELD_PLACEMENT_SCREEN_BOTTOM_RIGHT = 0x24,
    /** @brief At the part's offset in field coordinates. */
    FIELD_PLACEMENT_FIELD_POSITION = 0x25,
    /** @brief Not placed. */
    FIELD_PLACEMENT_NONE = 0x26,
    /** @brief At the owner or the tracked target, moved by the part's offset. */
    FIELD_PLACEMENT_OWNER_OFFSET = 0x27,
    FIELD_PLACEMENT_TRACK_OFFSET = 0x28,
    /** @brief At one of the owner's attachment points; the second kind takes only its x and adds the part's x/z offset. */
    FIELD_PLACEMENT_OWNER_ATTACHMENT = 0x29,
    /** @brief On the live effect of part 8 to 15 of the same actor. */
    FIELD_PLACEMENT_SIBLING_HIGH = 0x2A,
    FIELD_PLACEMENT_OWNER_ATTACHMENT_X = 0x32,
    /** @brief On the owner's current ground-effect point (g_field_spawn_ground_point). */
    FIELD_PLACEMENT_GROUND_POINT = 0x33,
    /** @brief At the tracked target, moved by the actor's per-track offset. */
    FIELD_PLACEMENT_TRACK_ACTION_OFFSET = 0x34,
    /** @brief At the shared point D_80105778. */
    FIELD_PLACEMENT_GLOBAL_POINT = 0x35,
    /** @brief At the part's offset, without the view offset. */
    FIELD_PLACEMENT_PART_OFFSET = 0x36,
    /** @brief On the live effect of part 0 to 7, moved by the part's offset. */
    FIELD_PLACEMENT_SIBLING_OFFSET = 0x37
} FieldEffectPlacement;

/** @brief Point on an object's bounds selected by a bounds placement (the offset from FIELD_PLACEMENT_OWNER_BOUNDS or FIELD_PLACEMENT_TRACK_BOUNDS). */
typedef enum
{
    FIELD_BOUNDS_POINT_ORIGIN = 0,
    FIELD_BOUNDS_POINT_CENTER = 1,
    /** @brief Horizontal centre at the object's origin height. */
    FIELD_BOUNDS_POINT_CENTER_X = 2,
    FIELD_BOUNDS_POINT_TOP_CENTER = 3,
    FIELD_BOUNDS_POINT_LEFT_CENTER = 4,
    FIELD_BOUNDS_POINT_RIGHT_CENTER = 5,
    FIELD_BOUNDS_POINT_TOP_LEFT = 6,
    FIELD_BOUNDS_POINT_TOP_RIGHT = 7,
    FIELD_BOUNDS_POINT_BOTTOM_LEFT = 8,
    FIELD_BOUNDS_POINT_BOTTOM_RIGHT = 9
} FieldBoundsPoint;

/** @brief Bounds points per bounds placement range, and parts per sibling placement range. */
#define FIELD_PLACEMENT_BOUNDS_POINT_COUNT 10
#define FIELD_PLACEMENT_SIBLING_COUNT 8

/** @brief FieldMotionRecord::state: how an active effect is rendered. */
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

/**
 * @brief FieldActor::control colour bits of an effect record (field_resolve_effect_part_color).
 * @note FIELD_CONTROL_EFFECT_COLOR (bit 15): the effect supplies its own colour instead of the part tint;
 *       with FIELD_CONTROL_LITERAL_COLOR (bit 28) that colour is the stored one, otherwise colour tracks.
 *       An effect that takes its owner's part colour sets both (FIELD_CONTROL_OWNER_TINT).
 */
#define FIELD_CONTROL_EFFECT_COLOR 0x8000
#define FIELD_CONTROL_LITERAL_COLOR 0x10000000
#define FIELD_CONTROL_OWNER_TINT (FIELD_CONTROL_LITERAL_COLOR | FIELD_CONTROL_EFFECT_COLOR)

/** @brief FieldObjectPart word at effect_count with neutral red and green tints (bytes 0xE and 0xF), as one masked compare. */
#define FIELD_PART_NEUTRAL_RED_GREEN 0x80800000
/** @brief FieldObjectPart::behavior_flags bit 11: the part keeps its own colour. */
#define FIELD_PART_OWN_COLOR_SHIFT 11
/** @brief FieldObjectPart::behavior_flags bits 4-5: when a spawned effect retires (FIELD_PART_END_*). */
#define FIELD_PART_END_MODE(part) (((part)->behavior_flags.word >> 4) & 3)
/** @brief Effect end modes: when its animation ends (otherwise the animation loops), at its lifetime, on reaching its target, or off screen. */
#define FIELD_PART_END_ANIMATION 0
#define FIELD_PART_END_LIFETIME 1
#define FIELD_PART_END_AT_TARGET 2
#define FIELD_PART_END_OFFSCREEN 3
/** @brief FieldObjectPart::appearance color_track_flags bits 5-7: must be clear for a part to pass on its owner's tint. */
#define FIELD_PART_BURST_SHIFT 5
/**
 * @brief Non-zero when @p part shows the plain neutral colour, so the effects it spawns take their owner's tint.
 * @note Tests the own-colour, track-colour and burst bits, then the tint (red and green as one masked word).
 */
#define FIELD_PART_PASSES_OWNER_TINT(part)                                                                                                                     \
    (!(((part)->behavior_flags.word >> FIELD_PART_OWN_COLOR_SHIFT) & 1) && !(((part)->placement_flags.word >> FIELD_PART_TRACK_COLOR_BIT) & 1) &&              \
     ((part)->appearance.fields.color_track_flags >> FIELD_PART_BURST_SHIFT) == 0 &&                                                                           \
     (*(u32*)&(part)->effect_count & 0xFFFF0000) == FIELD_PART_NEUTRAL_RED_GREEN && (part)->tint_blue == FIELD_TINT_NEUTRAL)
#define FIELD_EFFECT_SCREEN_SPACE 0x00001000U
#define FIELD_EFFECT_SEMITRANSPARENT 0x00800000U
#define FIELD_EFFECT_FACING_FLIPPED 0x80
#define FIELD_EFFECT_RETIRE_ON_HIT 0x08000000

/** @brief High-halfword actor action kinds handled as collectible rewards; also the drop a defeated monster leaves (field_roll_defeat_drop). */
typedef enum
{
    FIELD_PICKUP_EXPERIENCE_OR_CURRENCY = 31,
    FIELD_PICKUP_ITEM = 32,
    FIELD_PICKUP_RESTORE_QUARTER = 33,
    FIELD_PICKUP_RESTORE_HALF = 34
} FieldPickupAction;

/** @brief Hit-test paths observed in actor animation resources. */
typedef enum
{
    FIELD_HIT_TEST_EFFECT_BOUNDS = 1,
    FIELD_HIT_TEST_PART_SPHERES = 2
} FieldHitTestMode;

/** @brief Sources for an effect's target or attachment position. */
typedef enum
{
    FIELD_POSITION_NONE = 0,
    FIELD_POSITION_TRACK_OBJECT = 1,
    FIELD_POSITION_OWNER_OBJECT = 2,
    FIELD_POSITION_SAVED = 3,
    FIELD_POSITION_REFERENCE_EFFECT = 4,
    FIELD_POSITION_OWNER_ATTACHMENT = 5,
    FIELD_POSITION_FACING_OFFSET = 6,
    FIELD_POSITION_TRACK_STORED_XZ = 7,
    FIELD_POSITION_LINKED_EFFECT = 8,
    FIELD_POSITION_RELATIVE_SIDE_OFFSET = 9,
    FIELD_POSITION_OWNER_BOUNDS_CENTER = 10,
    FIELD_POSITION_TRACK_BOUNDS_CENTER = 11,
    FIELD_POSITION_REFLECT_TRACK_X = 12,
    FIELD_POSITION_EXTEND_TRACK_X = 13,
    FIELD_POSITION_EXTEND_TRACK_XZ = 14,
    FIELD_POSITION_EXTEND_LINK_XZ = 15
} FieldPositionSource;

/**
 * @brief Shared 0x54-byte position/state record used by world objects and effects.
 * @note Positions have eight fractional bits; angles use 0x1000 units per turn.
 * In the effect pool, motion_parameter is speed or an angular increment,
 * age counts updates, and state 0xFF marks retirement. World-object records
 * reuse some fields for animation state, so this is not a universal effect schema.
 * work_x/work_y/work_z also hold different data for linked segment effects.
 */
typedef struct FieldMotionRecord
{
    s32 x;
    s32 y;
    s32 z;
    /** @brief FieldActor::texture_slot. */
    u32 texture_slot;
    s16 rotation_x;
    s16 heading;
    s16 pitch;
    s16 motion_divisor;
    /** @brief Literal effect color; the fourth byte also selects the position source. */
    union
    {
        u32 word;
        struct
        {
            u8 red;
            u8 green;
            u8 blue;
            u8 position_source;
        } fields;
    } color_position;
    s32 flags;
    union
    {
        u8 path_time;           /* Interpolated path progress. */
        u8 linked_effect_index; /* Position sources 8 and 15. */
    } position_data;
    u8 facing_or_reward_kind;
    u8 actor_index;
    u8 part_index;
    u8 animation_active;
    u8 state;
    s8 height_or_retired_state;
    u8 saved_state;
    u8 lifetime;
    u8 track_index;
    s16 motion_parameter;
    s16 age;
    u16 motion_scale;
    s16 reference_index;
    u8 rotation_z_16;
    u8 rotation_y_16;
    /** @brief Ticks spent on the current animation frame, and the frame's length in ticks (FieldActor::frame_ticks/frame_length). */
    u8 frame_ticks;
    u8 frame_length;
    u8 motion_remainder;
    u8 vertical_offset;
    /** @brief Height of the next animation frame; the sprite lift interpolates vertical_offset towards it over the frame. */
    u8 next_height;
    u8 path_group;
    u8 source_object_index;
    u8 resource_index;
    u8 sprite_height_minus_one;
    u8 previous_effect_index;
    u8 next_effect_index;
    u8 pad3F;
    s32 unknown_0x40;
    u32 work_x;
    u32 work_y;
    u32 work_z;
    u8 pad50[0x54 - 0x50];
} FieldMotionRecord;

void field_collect_effect_hits(FieldMotionRecord* effect, s32 radius, FieldActorSlot* actor);
void field_release_actor_if_no_effects(FieldMotionRecord* effect);
s32 field_actor_has_live_effects(s32 actor_index);
void field_swap_effect_position_source(FieldMotionRecord* rec, FieldObjectPart* part);
void field_resolve_effect_position(FieldMotionRecord* rec, FieldObjectPart* part, VECTOR* out);

#endif
