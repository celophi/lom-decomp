/**
 * @file field_effect_types.h
 * @brief Shared actor definitions and motion records used by effect update/lifetime code.
 * @note Other field translation units still use partial views of these records.
 */
#ifndef FIELD_EFFECT_TYPES_H
#define FIELD_EFFECT_TYPES_H

#include "field_types.h"

#define FIELD_EFFECT_POOL_COUNT 0x103
#define FIELD_EFFECT_RETIRED 0xFF
#define FIELD_EFFECT_SEMITRANSPARENT 0x00800000U
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

/**
 * @brief Packed part definition controlling tracks, placement, and effect motion.
 * @note Some selectors span adjacent members. Keep explicit word reads where used.
 */
typedef struct
{
    u32 track_flags;  /* 0x00 */
    u32 behavior_flags;  /* 0x04 */
    u8 unknown_0x8;   /* 0x08 */
    u8 unknown_0x9;   /* 0x09 */
    u8 unknown_0xa;   /* 0x0A */
    u8 unknown_0xb;   /* 0x0B */
    u8 unknown_0xc;   /* 0x0C */
    u8 unknown_0xd;   /* 0x0D */
    u8 unknown_0xe;   /* 0x0E */
    u8 unknown_0xf;   /* 0x0F */
    u8 unknown_0x10;  /* 0x10 */
    u8 turn_end_age;  /* 0x11 */
    u8 pad12[0x14 - 0x12];
    u32 orientation_flags; /* 0x14 (halfword view at 0x16) */
    s16 unknown_0x18; /* 0x18 */
    u8 unknown_0x1a;  /* 0x1A */
    u8 pad1B;
    u16 unknown_0x1c; /* 0x1C */
    u8 unknown_0x1e; /* 0x1E */
    u8 pad1F;
    u8 unknown_0x20;
    u8 rotation_z_track;
    u8 rotation_y_track;
    u8 unknown_0x23;
    s32 effect_flags;
    u32 placement_flags;
    u8 unknown_0x2c;
    u8 pad2D;
    u8 unknown_0x2e;
    u8 unknown_0x2f;
    u8 pitch_acceleration;
    u8 unknown_0x31;
    u8 unknown_0x32;
    u8 unknown_0x33;
    union
    {
        u32 word;
        struct
        {
            u16 part_selectors;
            u16 motion_flags;
        } halves;
    } spawn_flags; /* 0x34: halfword selectors and packed word flags */
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
typedef struct
{
    u8 pad0[0x14];
    u8 hit_test_mode; /* 0x14 */
    u8 hit_test_part; /* 0x15 */
    u8 pad16;
    u8 hit_radius; /* 0x17 */
    u16 sync_flags; /* 0x18 */
    u16 sync_parts; /* 0x1A */
} FieldActorAnimationDef;

/**
 * @brief Actor-local part definitions, per-track counters, and object bindings.
 * @note actor_index identifies this actor in g_field_actor_slots; object indices
 * refer to the separate D_800FDF58 / D_80105AE0 arrays.
 */
typedef struct
{
    FieldActorPartDef* parts;
    u8 pad4[0xC - 4];
    FieldActorAnimationDef* animation;
    u8 pad10[0x14 - 0x10];
    u8* track_data;
    u8 pad18[0x24 - 0x18];
    u8 is_active;
    u8 part_count;
    u8 hit_reaction; /* 0x26: reaction selector applied to collected targets */
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
    u8 track_count; /* Includes tracks activated by new hit contacts. */
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
typedef struct
{
    s32 x;  /* 0x00 */
    s32 y;  /* 0x04 */
    s32 z;  /* 0x08 */
    u32 unknown_0xc;  /* 0x0C */
    s16 rotation_x; /* 0x10 */
    s16 heading; /* 0x12 */
    s16 pitch; /* 0x14 */
    s16 unknown_0x16; /* 0x16 */
    u8 unknown_0x18;  /* 0x18 */
    u8 unknown_0x19;  /* 0x19 */
    u8 unknown_0x1a;  /* 0x1A */
    u8 position_source;  /* 0x1B */
    s32 flags; /* 0x1C */
    union
    {
        u8 path_time; /* Interpolated path progress. */
        u8 linked_effect_index; /* Position sources 8 and 15. */
    } position_data; /* 0x20 */
    u8 facing_or_reward_kind;  /* 0x21 */
    u8 actor_index;  /* 0x22 */
    u8 part_index;  /* 0x23 */
    u8 unknown_0x24;  /* 0x24 */
    u8 state;  /* 0x25 */
    s8 height_or_retired_state;  /* 0x26 */
    u8 saved_state;  /* 0x27 */
    u8 lifetime;  /* 0x28 */
    u8 track_index;  /* 0x29 */
    s16 motion_parameter; /* 0x2A */
    s16 age; /* 0x2C */
    u16 motion_scale; /* 0x2E */
    s16 reference_index; /* 0x30 */
    u8 rotation_z_16;  /* 0x32 */
    u8 rotation_y_16;  /* 0x33 */
    u8 unknown_0x34;  /* 0x34 */
    u8 unknown_0x35;  /* 0x35 */
    u8 unknown_0x36;  /* 0x36 */
    u8 unknown_0x37;  /* 0x37 */
    u8 unknown_0x38;  /* 0x38 */
    u8 path_group;  /* 0x39 */
    u8 source_object_index;  /* 0x3A */
    u8 unknown_0x3b;  /* 0x3B */
    u8 unknown_0x3c;  /* 0x3C */
    u8 previous_effect_index;  /* 0x3D */
    u8 next_effect_index;  /* 0x3E */
    u8 pad3F;
    s32 unknown_0x40; /* 0x40 */
    u32 work_x; /* 0x44 */
    u32 work_y; /* 0x48 */
    u32 work_z; /* 0x4C */
    u8 pad50[0x54 - 0x50];
} FieldMotionRecord;

/**
 * @brief 0x23C-byte object view supplying bounds, anchors, scale, and action identity.
 * @note state_flags has both byte and word consumers. record_id is the word-wide
 * identifier consumed by the action/reward record lookup functions.
 */
typedef struct
{
    u8 pad0[0xC];
    u32 object_flags;    /* 0x0C */
    u8 pad10[0x14 - 0x10];
    s32 record_id; /* 0x14: identifier passed to action/reward helpers */
    u8 pad18[0x6C - 0x18];
    s16 unknown_0x6c; /* Whole-unit X used by position source 7. */
    s16 unknown_0x6e; /* Whole-unit Z used by position source 7. */
    u8 pad70[0x130 - 0x70];
    Vec2s attachment_points[4]; /* 0x130 */
    s16 bounds_left;  /* 0x140 */
    s16 bounds_top;  /* 0x142 */
    s16 bounds_right;  /* 0x144 */
    s16 bounds_bottom;  /* 0x146 */
    u8 pad148[0x174 - 0x148];
    u16 scale_percent;  /* 0x174 */
    u8 pad176[0x178 - 0x176];
    u32 state_flags;  /* 0x178 (byte view at 0x17A) */
    u8 pad17C[0x18E - 0x17C];
    u8 unknown_0x18e;   /* 0x18E */
    u8 pad18F[0x190 - 0x18F];
    Vec2s ground_attachment_points[3]; /* 0x190 */
    s32 unknown_0x19c;  /* 0x19C */
    s32 unknown_0x1a0;  /* 0x1A0 */
    u8 pad1A4[0x1A8 - 0x1A4];
    u8 unknown_0x1a8;   /* 0x1A8 */
    u8 unknown_0x1a9;   /* 0x1A9 */
    u8 unknown_0x1aa;   /* 0x1AA */
    u8 pad1AB[0x23C - 0x1AB];
} FieldObjectPlacement;

void field_collect_effect_hits(FieldMotionRecord *effect, s32 radius, FieldActorState *actor);
void field_release_actor_if_no_effects(FieldMotionRecord *effect);
s32 field_actor_has_live_effects(s32 actor_index);
void field_swap_effect_position_source(FieldMotionRecord *rec, FieldActorPartDef *part);
void field_resolve_effect_position(FieldMotionRecord *rec, FieldActorPartDef *part, FieldVector *out);

#endif
