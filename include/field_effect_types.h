/**
 * @file field_effect_types.h
 * @brief Shared actor definitions and motion records used by effect update/lifetime code.
 * @note Other field translation units still use partial views of these records.
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

/** @brief Packed part flags accessed as words, halfwords, or individual bytes. */
typedef union
{
    u32 word;
    struct
    {
        u16 low;
        u16 high;
    } halves;
    struct
    {
        u8 low;
        u8 middle_low;
        u8 middle_high;
        u8 high;
    } bytes;
} FieldPartFlags;

/**
 * @brief Packed part definition controlling tracks, placement, and effect motion.
 * @note Packed selector words have byte/halfword views for resource fields.
 */
typedef struct FieldActorPartDef
{
    FieldPartFlags track_flags;
    FieldPartFlags behavior_flags;
    u8 unknown_0x8;
    u8 unknown_0x9;
    u8 unknown_0xa;
    u8 effect_kind;
    u8 unknown_0xc;
    u8 unknown_0xd;
    u8 red_or_track;
    u8 green_or_track;
    u8 blue_or_track;
    u8 turn_end_age;
    u8 unknown_0x12;
    u8 rotation_y_16;
    FieldPartFlags orientation_flags; /* Also read through its upper halfword. */
    s16 unknown_0x18;
    u8 unknown_0x1a;
    u8 pad1B;
    /** @brief Palette track, angular divisions, footprint flags, and extent low bits. */
    union
    {
        u32 word;
        struct
        {
            u16 palette_track;
            u8 angular_divisions;
            u8 scale_extent_flags;
        } fields;
    } palette_extent;
    /** @brief Extent high bits/mode followed by rotation and effect selectors. */
    union
    {
        u32 word;
        struct
        {
            u8 extent_value_mode;
            u8 rotation_z_track;
            u8 rotation_y_track;
            u8 unknown_0x23;
        } fields;
    } rotation_extent;
    s32 effect_flags;
    FieldPartFlags placement_flags;
    /** @brief Color-track flags, palette selector, footprint width, and spawn controls. */
    union
    {
        u32 word;
        struct
        {
            u8 color_track_flags;
            u8 palette_selector;
            u8 footprint_scale_x;
            u8 spawn_flags;
        } fields;
    } appearance;
    u8 pitch_acceleration;
    u8 unknown_0x31;
    u8 unknown_0x32;
    u8 footprint_scale_y;
    union
    {
        u32 word;
        struct
        {
            u16 part_selectors;
            u16 motion_flags;
        } halves;
    } spawn_flags; /* halfword selectors and packed word flags */
    s16 offset_x;
    s16 offset_y;
    s16 offset_z;
    s16 pad3E;
    s16 unknown_0x40;
    s16 unknown_0x42;
    s16 unknown_0x44;
    s16 unknown_0x46;
} FieldActorPartDef;

/** @brief Hit-test selection and packed owner/track synchronization selectors. */
typedef struct FieldActorAnimationDef
{
    u8 pad0[0x10];
    u16 palette_animation;
    u16 unknown_0x12;
    u8 hit_test_mode;
    u8 hit_test_part;
    u8 pad16;
    u8 hit_radius;
    u16 sync_flags;
    u16 sync_parts;
} FieldActorAnimationDef;

/**
 * @brief Actor-local part definitions, per-track counters, and object bindings.
 * @note actor_index identifies this actor in g_field_actor_slots; object indices
 * refer to the separate g_field_actors / g_field_object_states arrays.
 */
typedef struct FieldActorState
{
    FieldActorPartDef* parts;
    u8 pad4[0xC - 4];
    FieldActorAnimationDef* animation;
    u8 pad10[0x14 - 0x10];
    u8* track_data;
    u8* mesh_data;
    u8 pad1C[0x24 - 0x1C];
    u8 is_active;
    u8 part_count;
    u8 hit_reaction; /* reaction selector applied to collected targets */
    u8 unknown_0x27;
    u8 unknown_0x28;
    u8 unknown_0x29;
    u8 unknown_0x2a;
    u8 unknown_0x2b[16];
    u8 active_counts[9][16];
    u8 padCB;
    u16 track_counters[9][16];
    u16 track_ages[9];
    Vec2s track_offsets[9];
    u16 unknown_0x222;
    u32 action_flags;
    u8 owner_object_index;
    u8 track_object_indices[9]; /* Also receives dynamically collected hit targets. */
    u8 track_count;             /* Includes tracks activated by new hit contacts. */
    u8 actor_index;
    u16 unknown_0x234;
    u16 unknown_0x236;
    u8 pad238[2];
    u8 active_track_mask;
    u8 unknown_0x23b;
    u8 pad23C[0x240 - 0x23C];
    u16* unknown_0x240;
} FieldActorState;

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
    s16 unknown_0x16;
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
    u8 unknown_0x36;
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

void field_collect_effect_hits(FieldMotionRecord* effect, s32 radius, FieldActorState* actor);
void field_release_actor_if_no_effects(FieldMotionRecord* effect);
s32 field_actor_has_live_effects(s32 actor_index);
void field_swap_effect_position_source(FieldMotionRecord* rec, FieldActorPartDef* part);
void field_resolve_effect_position(FieldMotionRecord* rec, FieldActorPartDef* part, VECTOR* out);

#endif
