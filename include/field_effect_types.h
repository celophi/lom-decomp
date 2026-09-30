/**
 * @file field_effect_types.h
 * @brief Shared actor definitions and motion records used by effect update/lifetime code.
 */
#ifndef FIELD_EFFECT_TYPES_H
#define FIELD_EFFECT_TYPES_H

#include "field_types.h"
#include "field_object_state.h"

#define FIELD_EFFECT_POOL_COUNT 0x103
#define FIELD_EFFECT_ACTIVE_RECORD_COUNT 256
#define FIELD_EFFECT_DISABLED 0xFE
#define FIELD_EFFECT_RETIRED 0xFF
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

/** @brief FieldAnimationDef::flags: the definition carries its extension fields (duration onward). */
#define FIELD_ANIMATION_HAS_EXTENSION 0x8000
/** @brief FieldAnimationDef::flags: two alternate definitions and a three-halfword tail follow. */
#define FIELD_ANIMATION_HAS_ALTERNATES 0x800

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
    u32 unknown_0xc;
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
    u8 unknown_0x34;
    u8 unknown_0x35;
    u8 motion_remainder;
    u8 vertical_offset;
    u8 unknown_0x38;
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
