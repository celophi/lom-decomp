/**
 * @file field_effect_update.c
 * @brief Spawn, update, position, retire, and dispatch rendering of field actor effects.
 */

#include "common.h"
#include <abs.h>
#include <inline_c.h>
#include <libgpu.h>
#include <libgte.h>
#include <rand.h>

#include "../internal/field_actor.h"
#include "../internal/field_actor_behavior.h"
#include "../internal/field_actor_palette.h"
#include "../internal/field_actor_runtime.h"
#include "../internal/field_actor_sequence_runtime.h"
#include "../internal/field_calls.h"
#include "../internal/field_contact_geometry.h"
#include "../internal/field_effect_dispatch.h"
#include "../internal/field_effect_geometry.h"
#include "../internal/field_effect_primitives.h"
#include "../internal/field_effect_render_state.h"
#include "../internal/field_effect_transform.h"
#include "../internal/field_effect_types.h"
#include "../internal/field_mesh_render.h"
#include "../internal/field_mesh_transform.h"
#include "../internal/field_path_interpolation.h"
#include "../internal/field_state_ops.h"
#include "common/gpu_packet.h"
#include "overlays/field/field_actor_records.h"
#include "overlays/field/field_player_records.h"
#include "overlays/field/field_sound.h"
#include "overlays/field/field_types.h"
#include "sdk/gte_dmpsx_compat.h"

#define FIELD_EFFECT_SCRATCH_ORIGIN ((VECTOR*)SCRATCHPAD_ADDRESS)
#define FIELD_EFFECT_SCRATCH_VECTOR ((VECTOR*)SCRATCHPAD_AT(0x10))
#define FIELD_EFFECT_SCRATCH_TARGET ((VECTOR*)SCRATCHPAD_AT(0x20))
#define FIELD_EFFECT_SCRATCH_LOCAL_VECTOR ((SVECTOR*)SCRATCHPAD_AT(0x30))
#define FIELD_EFFECT_SCRATCH_ROTATED_VECTOR ((SVECTOR*)SCRATCHPAD_AT(0x38))
#define FIELD_EFFECT_SCRATCH_ROTATION ((MATRIX*)SCRATCHPAD_AT(0x40))
#define FIELD_EFFECT_SCRATCH_MOVER ((FieldCollisionMover*)SCRATCHPAD_AT(0x80))
#define FIELD_EFFECT_SCRATCH_QUERY ((FieldCollisionQuery*)SCRATCHPAD_AT(0xC0))

/**
 * @brief Address of element @p index in an array of @p type starting at @p base.
 */
#define FIELD_ELEMENT_AT(type, base, index) ((type*)((index) * sizeof(type) + (uintptr_t)(base)))

#define FIELD_PART_PLACEMENT_KIND_SHIFT 18
#define FIELD_PART_PLACEMENT_KIND_MASK 0x3F
#define FIELD_PART_PLACEMENT_KIND(part) (((part)->placement_flags.word >> FIELD_PART_PLACEMENT_KIND_SHIFT) & FIELD_PART_PLACEMENT_KIND_MASK)
#define FIELD_PART_SCALE_X_FROM_BOUNDS_SHIFT 9
#define FIELD_PART_SCALE_Y_FROM_BOUNDS_SHIFT 1
#define FIELD_PART_MIRROR_X_WITH_FACING_SHIFT 10
#define FIELD_PART_ATTACHMENT_INDEX_SHIFT 21
#define FIELD_PART_ATTACHMENT_INDEX_MASK 3
#define FIELD_PART_MIRROR_X_WITH_OWNER 0x08000000
/** @brief Placement flags bit 0: the new effect sorts one ordering-table step further (FIELD_WORLD_OT_DEPTH_STEP). */
#define FIELD_PART_DEPTH_STEP 0x00000001
/** @brief Effect flags bits 16-19: the position source used instead of behavior bits 8-10 when orientation_flags has a mode. */
#define FIELD_PART_ORIENTED_POSITION_SOURCE_SHIFT 16
#define FIELD_PART_ORIENTED_POSITION_SOURCE_MASK 0xF
#define FIELD_PART_DISTANCE_MODE_SHIFT 6
#define FIELD_PART_KIND_RANGE_BIT 8
#define FIELD_PART_FACE_TRAVEL_BIT 14
#define FIELD_PART_ROTATION_Z_SOURCE_SHIFT 26
#define FIELD_PART_ROTATION_Y_SOURCE_SHIFT 28
#define FIELD_PART_ROTATION_SOURCE_MASK 3
#define FIELD_PART_KIND_FROM_REWARD_COUNTERS 0x00010000
#define FIELD_PART_FLIP_FACING 0x00200000
#define FIELD_PART_RETIRE_ON_HIT_BIT 18
#define FIELD_PART_KIND_RANGE_COUNT_SHIFT 24
#define FIELD_PART_KIND_RANGE_COUNT_MASK 0xF
#define FIELD_PART_ORIENTATION_MODE_MASK 0xF0
#define FIELD_PART_COPY_SIBLING_ROTATION 0x08000000
#define FIELD_PART_SWAP_POSITION_SOURCE 0x10000000
#define FIELD_PART_MOTION_SCALE_TRACK_BIT 13

#define FIELD_EFFECT_FLAGS(record) (((FieldEffectFlagsView*)(record))->flags)

/** @brief Bit positions in the part's behavior flags. */
#define FIELD_PART_SEMITRANSPARENT_BIT 1
#define FIELD_PART_PITCH_ACCELERATION_BIT 2
#define FIELD_PART_POSITION_SOURCE_SHIFT 8
#define FIELD_PART_POSITION_SOURCE_MASK 7
#define FIELD_PART_MOTION_KIND_SHIFT 13
#define FIELD_PART_GROUND_BOUNCE_BIT 6

/** @brief Bit positions in the part's placement flags. */
#define FIELD_PART_HEIGHT_TRACK_BIT 3
#define FIELD_PART_HEIGHT_TRACK_INDEX_SHIFT 4

/** @brief Masks in the part's spawn flags. */
#define FIELD_PART_HEIGHT_FROM_BASE 0x00080000
#define FIELD_PART_CAMERA_BOUNDS 0x01000000
#define FIELD_PART_GROUND_STOP 0x02000000
#define FIELD_PART_RETIRE_ON_COLLISION 0x10000000

#define FIELD_PICKUP_DELAY 0x10U
#define FIELD_PICKUP_DISTANCE 5
#define FIELD_PICKUP_SOUND 0x1F
#define FIELD_PART_BLEND_TPAGE_SHIFT (FIELD_PART_BLEND_MODE_SHIFT - GPU_TPAGE_BLEND_SHIFT)

/** @brief FieldObjectPart::effect_flags bits 25-28: track that switches semitransparency when FIELD_EFFECT_SEMITRANSPARENT is set. */
#define FIELD_PART_SEMITRANSPARENT_TRACK_SHIFT 25
#define FIELD_EFFECT_SEMITRANSPARENT_SHIFT 23
/** @brief facing_or_reward_kind value of a new effect that takes the animation of the record it is attached to. */
#define FIELD_EFFECT_ANIMATION_FROM_SOURCE 0xFF

/** @brief FieldObjectPart::effect_flags bits 29-30: the effect starts at the owner or at the current target. */
#define FIELD_PART_ORIGIN_SHIFT 29
#define FIELD_PART_ORIGIN_MASK 0x60000000
#define FIELD_PART_ORIGIN_OWNER 1
#define FIELD_PART_ORIGIN_TARGET 2

#define FIELD_EFFECT_PARAMETER_MODE_SHIFT 29
#define FIELD_EFFECT_PARAMETER_MODE_MASK 0x60000000
#define FIELD_EFFECT_PARAMETER_MODE_TIMED_TRACK (FIELD_PART_VALUE_TIMED_TRACK << FIELD_EFFECT_PARAMETER_MODE_SHIFT)
#define FIELD_EFFECT_MOTION_KIND_MASK 0x07000000
#define FIELD_EFFECT_MOTION_PATH 0x05000000
#define FIELD_EFFECT_MOTION_HOMING 0x01000000
#define FIELD_EFFECT_DISTANCE_MASK 0x1FF
#define FIELD_EFFECT_ORIENTATION_LOCK 8

#define FIELD_EFFECT_TURN_THRESHOLD 0x200
#define FIELD_EFFECT_TURN_STEP 0x80

#define FIELD_EFFECT_CAMERA_X_MIN 0x500
#define FIELD_EFFECT_CAMERA_X_MAX 0x13B00
#define FIELD_EFFECT_CAMERA_Z_SPAN 0x1E800

/* Effect render dispatch and sprite-frame rendering. */
#define FIELD_EFFECT_SCRATCH_MATRIX ((void*)SCRATCHPAD_ADDRESS)
#define FIELD_EFFECT_SCRATCH_SCREEN_ORIGIN ((Vec2s*)SCRATCHPAD_AT(0x40))
#define FIELD_EFFECT_SCRATCH_FOOTPRINT ((Vec2s*)SCRATCHPAD_AT(0x64))

#define FIELD_SPRITE_FRAME_FOOTPRINT 0x20
#define FIELD_SPRITE_FRAME_FLIP_X 0x40
#define FIELD_SPRITE_FRAME_FLIP_Y 0x80
#define FIELD_SPRITE_TEXTURE_MODE_MASK 3

/* Untextured effect renderers: ring, fan, marker, trail, radial fan, and radial lines. */
#define FIELD_RING_SEGMENTS 32
#define FIELD_RING_ANGLE_STEP (ONE / FIELD_RING_SEGMENTS)
#define FIELD_RADIAL_MAX_SEGMENTS 20
#define FIELD_GPU_ADDRESS_MASK 0x00FFFFFF
#define FIELD_GPU_LENGTH_MASK 0xFF000000
#define FIELD_RADIAL_SCRATCH ((FieldRadialScratch*)SCRATCHPAD_ADDRESS)

#define FIELD_LINK_PACKET(entry, packet, address_mask, length_mask)                                                                                            \
    ((packet)->tag = ((packet)->tag & (length_mask)) | (*(entry) & (address_mask)),                                                                            \
     *(entry) = (*(entry) & (length_mask)) | ((uintptr_t)(packet) & (address_mask)))

/* Ribbon renderer and the shared part transform, color, texture-page, sprite, and extent helpers. */
#define FIELD_RIBBON_MAX_SEGMENTS 20
#define FIELD_PART_REVERSE_ROTATION_WITH_FACING 0x00020000
#define FIELD_PART_SCALE_DISTANCE_BY_TARGET 0x01000000
#define FIELD_PART_SCALE_DISTANCE_BY_OWNER 0x01000000
#define FIELD_RIBBON_SCRATCH ((FieldRibbonScratch*)SCRATCHPAD_ADDRESS)
#define FIELD_MATRIX_SCRATCH ((FieldMatrixScratch*)SCRATCHPAD_AT(0xC0))
#define FIELD_SPRITE_SCRATCH ((FieldSpriteScratch*)SCRATCHPAD_AT(0x100))

#define FIELD_TRACK_COUNT 16
#define FIELD_EFFECT_TARGET_PITCH (ONE / 12)
#define FIELD_EFFECT_DEFAULT_PITCH (ONE / 16)
#define FIELD_PART_SCALE_TRACK_SHIFT 7
#define FIELD_PART_ROTATION_MODE_SHIFT 6
#define FIELD_PART_ROTATION_TRACK_SHIFT 26

/** @brief Rotation of a part's effect (track_flags bits 6-7). */
typedef enum
{
    FIELD_PART_ROTATION_NONE = 0,
    /** @brief Angle from a parameter track; the axis comes from track_flags. */
    FIELD_PART_ROTATION_TRACK = 1,
    /** @brief The effect's heading, about the Y axis. */
    FIELD_PART_ROTATION_HEADING = 2,
    /** @brief Constant spin: a per-frame step times the effect's age. */
    FIELD_PART_ROTATION_SPIN = 3
} FieldPartRotationMode;

/** @brief Axis a part's rotation turns about. */
typedef enum
{
    FIELD_ROTATION_AXIS_X = 0,
    FIELD_ROTATION_AXIS_Y = 1,
    FIELD_ROTATION_AXIS_Z = 2,
    /** @brief The same angle about Z, then Y, then X. */
    FIELD_ROTATION_AXIS_ALL = 3
} FieldRotationAxis;

/** @brief Which axes a part's scale track drives (behavior_flags bits 20-21). */
typedef enum
{
    FIELD_PART_SCALE_Z = 0,
    FIELD_PART_SCALE_X = 1,
    FIELD_PART_SCALE_UNIFORM = 2,
    /** @brief Z from the track, X from the next track. */
    FIELD_PART_SCALE_Z_THEN_X = 3
} FieldPartScaleMode;

/** @brief Source of a part's extent (rotation_extent bits 6-7). */
typedef enum
{
    FIELD_PART_EXTENT_CONSTANT = 0,
    FIELD_PART_EXTENT_TRACK = 1,
    /** @brief The track's value at time 0. */
    FIELD_PART_EXTENT_TRACK_START = 2
} FieldPartExtentMode;
#define FIELD_PART_ARC_MODE_SHIFT 6
#define FIELD_PART_ARC_HEIGHT_SHIFT 26
#define FIELD_PART_ROTATE_QUARTER_X_SHIFT 17
#define FIELD_PART_PROJECT_SCALE_SHIFT 23
#define FIELD_PART_RGB_TRACKS_SHIFT 12
#define FIELD_PART_COLOR_TRACK_INDEX_SHIFT 16
#define FIELD_PART_RANDOM_EXTENT_SHIFT 15
#define FIELD_PART_ADD_HALF_WIDTH_SHIFT 16
#define FIELD_PART_ADD_HALF_HEIGHT_SHIFT 17
#define FIELD_PART_COLOR_TRACK_SHIFT 5
#define FIELD_PART_RED_TRACK 0x4
#define FIELD_PART_GREEN_TRACK 0x2
#define FIELD_PART_BLUE_TRACK 0x1
/** @brief Range width from FIELD_PLACEMENT_TRACK_BOUNDS of the kinds whose extent uses the tracked target's bounds; below FIELD_PLACEMENT_SIBLING only 10-19
 * reach it. */
#define FIELD_ATTACHMENT_LINKED_BOUNDS_COUNT 28
#define FIELD_PART_OWNER_BOUNDS 2

/**
 * @brief Bit layout of FieldMotionRecord.flags, with a halfword view.
 * @note FieldMotionRecord declares flags as a plain s32; this is read through FIELD_EFFECT_FLAGS.
 */
typedef union
{
    s32 word;
    u16 half[2];
    struct
    {
        unsigned distance : 9;      /* spawn distance along the rotated direction */
        unsigned distance_mode : 2; /* 2: distance comes from a parameter track */
        unsigned play_once : 1;     /* FIELD_CONTROL_PLAY_ONCE of the actor role; effects only clear it */
        unsigned screen_space : 1;
        unsigned ground_point : 2; /* ground attachment point for placement 0x33 */
        unsigned effect_color : 1;
        unsigned group : 2;
        unsigned petrified : 1; /* FIELD_CONTROL_PETRIFIED: drawn with the petrified CLUT column; cleared at spawn */
        unsigned kind : 4;      /* copied from the source record */
        unsigned semitransparent : 1;
        unsigned motion_kind : 3;
        unsigned retire_on_hit : 1;
        unsigned literal_color : 1;
        unsigned parameter_mode : 2;
        unsigned unused_31 : 1; /* no code reads or writes it */
    } bits;
} FieldEffectFlags;

/** @brief FieldMotionRecord prefix up to its flags word, typed as FieldEffectFlags. */
typedef struct
{
    u8 pad0[0x1C];
    FieldEffectFlags flags;
} FieldEffectFlagsView;

/** @brief Where a part setting comes from: a fixed value, a parameter track, a parameter track at a given time, or (motion parameter only) the distance to the
 * target. */
enum
{
    FIELD_PART_VALUE_FIXED = 0,
    FIELD_PART_VALUE_TRACK = 1,
    FIELD_PART_VALUE_TIMED_TRACK = 2,
    FIELD_PART_VALUE_DISTANCE = 3
};

/** @brief Selected map dimensions used for effect movement bounds. */
typedef struct
{
    s16 width;
    s16 height;
} FieldEffectMapBounds;

/** @brief Camera translation used while updating effect movement. */
typedef struct
{
    u8 pad0[4];
    s32 x;
    s32 y;
    s32 z;
} FieldEffectCamera;

/**
 * @brief Placement selector within the part flags word.
 */
typedef struct
{
    unsigned int lower : 18;
    unsigned int opcode : 6;
    unsigned int upper : 8;
} FieldPlacementBits;

/** @brief GPU coordinates accessed individually or as a packed XY word. */
typedef union
{
    s32 word;
    struct
    {
        s16 x;
        s16 y;
    } signed_pair;
    struct
    {
        u16 x;
        u16 y;
    } unsigned_pair;
} FieldScreenPoint;

/** @brief Unsigned projected origin used by ring geometry. */
typedef struct
{
    u16 x;
    u16 y;
} FieldScreenPair;

/** @brief Gouraud triangle packet with packed color and coordinate access. */
typedef struct
{
    s32 tag;
    FieldPrimitiveColor color0;
    FieldScreenPoint xy0;
    FieldPrimitiveColor color1;
    FieldScreenPoint xy1;
    FieldPrimitiveColor color2;
    FieldScreenPoint xy2;
} FieldGouraudTriangle;

/** @brief Gouraud line packet with packed color and coordinate access. */
typedef struct
{
    s32 tag;
    FieldPrimitiveColor color0;
    FieldScreenPoint xy0;
    FieldPrimitiveColor color1;
    FieldScreenPoint xy1;
} FieldGouraudLine;

/** @brief Flat quad packet with paired signed and unsigned coordinate views. */
typedef struct
{
    s32 tag;
    FieldPrimitiveColor color0;
    FieldScreenPoint xy0;
    FieldScreenPoint xy1;
    FieldScreenPoint xy2;
    FieldScreenPoint xy3;
} FieldFlatQuad;

/** @brief Flat line packet with packed color and coordinate access. */
typedef struct
{
    s32 tag;
    FieldPrimitiveColor color0;
    FieldScreenPoint xy0;
    FieldScreenPoint xy1;
} FieldFlatLine;

/** @brief Shared scratchpad workspace for radial projection and random rotations. */
typedef struct
{
    VECTOR origin;
    VECTOR transformed;
    union
    {
        VECTOR vector;
        Vec2s screen;
    } center;
    VECTOR delta;
    VECTOR jitter;
    SVECTOR direction;
    MATRIX matrices[FIELD_RADIAL_MAX_SEGMENTS];
} FieldRadialScratch;

/** @brief SDK angle vector with word access to paired halfwords. */
typedef union
{
    SVECTOR vector;
    struct
    {
        s32 xy;
        s32 z_pad;
    } packed;
} FieldRotationAngles;

/** @brief A packed XY pair, UV pair, and texture-page or palette halfword. */
typedef struct
{
    s32 xy;
    u16 uv;
    u16 page;
} FieldPackedQuadVertex;

/** @brief SDK quad with packed views for copying ribbon edges and color. */
typedef union
{
    POLY_FT4 gpu;
    struct
    {
        s32 tag;
        FieldPrimitiveColor color;
        FieldPackedQuadVertex vertices[4];
    } packed;
} FieldRibbonQuad;

/** @brief Ribbon path vectors followed by one rotation matrix per segment. */
typedef struct
{
    VECTOR endpoint;
    VECTOR position;
    VECTOR midpoint;
    VECTOR half_delta;
    VECTOR delta;
    FieldRotationAngles direction;
    MATRIX rotations[FIELD_RIBBON_MAX_SEGMENTS];
} FieldRibbonScratch;

/** @brief Temporary rotation and distance calculations for a part matrix. */
typedef struct
{
    SVECTOR direction;
    VECTOR scale;
    VECTOR delta;
    VECTOR squared_delta;
} FieldMatrixScratch;

/** @brief GTE output with unsigned low halfwords for GPU vertex additions. */
typedef union
{
    VECTOR vector;
    struct
    {
        u16 x;
        u16 x_high;
        u16 y;
        u16 y_high;
        u16 z;
        u16 z_high;
        u16 pad[2];
    } low;
} FieldSpriteProjection;

/** @brief Scratchpad vectors and local tilt matrix for sprite projection. */
typedef struct
{
    SVECTOR rotated_vertex;
    SVECTOR vertex;
    FieldSpriteProjection projected;
    FieldMatrixStorage rotation;
} FieldSpriteScratch;

/** @brief Position of the effect spawning child parts; FIELD_PLACEMENT_GLOBAL_POINT parts start there. */
extern VECTOR g_field_effect_spawn_point;
extern s32 g_field_active_group;

static s32 field_roll_effect_rotation(FieldActorSlot* actor, FieldObjectPart* part, FieldMotionRecord* effect);
static void field_face_effect_target(FieldMotionRecord* effect, FieldObjectPart* part);
static void field_update_effect_record(FieldMotionRecord* record, FieldObjectPart* part, FieldActorSlot* actor);
static void field_set_action_context(s32 recipient_id, s32 source_id, s32 action);
static s32* field_render_effect_sprite_frames(FieldMotionRecord* effect, s32* packet_cursor, s32* ordering_table, u8* frame_data);

/**
 * @brief Free a just-claimed effect record and undo the spawn counts of its part.
 * @param actor Actor whose part was spawning.
 * @param effect Claimed effect record.
 * @param part_index Part whose active and track counters are rolled back.
 * @return -1, the spawn failure result.
 */
static inline s32 field_cancel_effect_spawn(FieldActorSlot* actor, FieldMotionRecord* effect, s32 part_index)
{
    effect->state = FIELD_EFFECT_RETIRED;
    actor->active_counts[g_field_track_index][part_index]--;
    actor->track_counters[g_field_track_index][part_index]--;
    return -1;
}

/**
 * @brief Give a linked sprite effect the sprite and animation of the object record it is placed on.
 * @param effect New linked sprite effect.
 * @param source_record Object record the effect is placed on.
 * @param actor Actor spawning the effect; the effect is disabled while another slot controls the object.
 */
static inline void field_adopt_source_sprite(FieldMotionRecord* effect, FieldMotionRecord* source_record, FieldActorSlot* actor)
{
    FieldObjectState* source_object;
    u8* resource;

    effect->source_object_index = source_record->source_object_index;
    source_object = field_object_state(source_record->source_object_index);

    if (source_object->contact.bytes.flags & 1)
    {
        if (source_object->contact.bytes.controller_index != actor->slot_index)
        {
            effect->state = FIELD_EFFECT_DISABLED;
            return;
        }
    }

    effect->resource_index = source_record->resource_index;
    effect->texture_slot = source_record->texture_slot;
    effect->facing_or_reward_kind = source_record->facing_or_reward_kind;
    effect->saved_state = source_record->saved_state;
    FIELD_EFFECT_FLAGS(effect).bits.kind = FIELD_EFFECT_FLAGS(source_record).bits.kind;
    FIELD_EFFECT_FLAGS(effect).bits.group = FIELD_EFFECT_FLAGS(source_record).half[1];
    resource = g_field_resource_entries[source_record->resource_index].start;

    if (resource != 0)
    {
        field_restart_actor_animation(effect, resource);
    }
}

/**
 * @brief Give a linked sprite effect the sprite, state and animation of the sibling effect it is placed on.
 * @param effect New linked sprite effect.
 * @param sibling Index of the sibling effect record.
 */
static inline void field_adopt_sibling_sprite(FieldMotionRecord* effect, s32 sibling)
{
    u8* resource;

    effect->resource_index = g_field_effect_records[sibling].resource_index;
    effect->texture_slot = g_field_effect_records[sibling].texture_slot;
    effect->state = g_field_effect_records[sibling].state;
    effect->source_object_index = g_field_effect_records[sibling].source_object_index;
    FIELD_EFFECT_FLAGS(effect).bits.kind = FIELD_EFFECT_FLAGS(&g_field_effect_records[sibling]).bits.kind;
    FIELD_EFFECT_FLAGS(effect).bits.group = FIELD_EFFECT_FLAGS(&g_field_effect_records[sibling]).bits.group;

    if (effect->facing_or_reward_kind == FIELD_EFFECT_ANIMATION_FROM_SOURCE)
    {
        effect->facing_or_reward_kind = g_field_effect_records[sibling].facing_or_reward_kind;
        effect->saved_state = g_field_effect_records[sibling].saved_state;
        effect->frame_ticks = g_field_effect_records[sibling].frame_ticks;
        effect->frame_length = g_field_effect_records[sibling].frame_length;
        effect->track_index = g_field_effect_records[sibling].track_index;
        effect->motion_remainder = g_field_effect_records[sibling].motion_remainder;
        effect->vertical_offset = g_field_effect_records[sibling].vertical_offset;
        effect->next_height = g_field_effect_records[sibling].next_height;
        return;
    }

    effect->source_object_index = g_field_effect_records[sibling].source_object_index;
    effect->state = g_field_effect_records[sibling].state;
    FIELD_EFFECT_FLAGS(effect).bits.kind = FIELD_EFFECT_FLAGS(&g_field_effect_records[sibling]).bits.kind;
    FIELD_EFFECT_FLAGS(effect).bits.group = FIELD_EFFECT_FLAGS(&g_field_effect_records[sibling]).bits.group;
    effect->saved_state = g_field_effect_records[sibling].saved_state;
    effect->facing_or_reward_kind = g_field_effect_records[sibling].facing_or_reward_kind;
    resource = g_field_resource_entries[g_field_actors[g_field_actor_slots[effect->owner_slot].owner_object_index].resource_index].start;

    if (resource != 0)
    {
        field_begin_actor_animation_forward((FieldActor*)effect, resource);
    }
}

/**
 * @brief Make a linked sprite effect placed on an owner attachment point draw its owner's sprite.
 * @param effect New linked sprite effect.
 * @param source_record Owner record the effect is attached to.
 * @param actor Actor spawning the effect; the effect is disabled while another slot controls the owner.
 */
static inline void field_attach_owner_sprite(FieldMotionRecord* effect, FieldMotionRecord* source_record, FieldActorSlot* actor)
{
    FieldObjectState* source_object;
    u8* resource;

    source_object = field_object_state(source_record->source_object_index);

    if (!(source_object->contact.bytes.flags & 1) || source_object->contact.bytes.controller_index == actor->slot_index)
    {
        effect->state = FIELD_EFFECT_RENDER_OWNER_SPRITE;

        if (effect->facing_or_reward_kind == FIELD_EFFECT_ANIMATION_FROM_SOURCE)
        {
            effect->resource_index = source_record->resource_index;
            effect->texture_slot = source_record->texture_slot;
            effect->facing_or_reward_kind = source_record->facing_or_reward_kind;
            effect->saved_state = source_record->saved_state;
        }

        resource = g_field_resource_entries[source_record->resource_index].start;

        if (resource != 0)
        {
            field_restart_actor_animation(effect, resource);
        }

        return;
    }

    effect->state = FIELD_EFFECT_DISABLED;
}

/**
 * @brief Spawn an effect record for one actor part and resolve its initial placement.
 * @param actor Actor spawning the effect.
 * @param part_index Part definition to instantiate.
 * @param start First effect slot to inspect when searching for a related effect.
 * @return Spawned effect index, or -1 when the effect cannot be created.
 * @note Calls itself to place one more effect on the next matching sibling (searching from the slot after the one
 *       found) and on the next ground point of a scattered-domes action.
 */
s32 field_spawn_actor_effect(FieldActorSlot* actor, s32 part_index, s32 start)
{
    VECTOR* direction_vector = FIELD_EFFECT_SCRATCH_ORIGIN;
    VECTOR* squared_vector = FIELD_EFFECT_SCRATCH_VECTOR;
    SVECTOR* local_direction = FIELD_EFFECT_SCRATCH_LOCAL_VECTOR;
    MATRIX* rotation_matrix = FIELD_EFFECT_SCRATCH_ROTATION;
    FieldMotionRecord* effect;
    FieldMotionRecord* free_effect;
    FieldObjectState* placement_object;
    FieldObjectState* owner_object;
    FieldObjectState* owner_placement_state;
    FieldObjectState* owner_placement_guard;
    FieldObjectState* attached_source_object;
    FieldMotionRecord* source_record;
    FieldMotionRecord* record_base;
    FieldObjectState* owner_object_state;
    FieldObjectPart* part;
    s32 rotation_z_source;
    s32 rotation_y_source;
    s32 rotated_x;
    s32 work_value;
    s32 extent;
    s32 effect_index;
    s32 work_index;
    s32 object_index;
    s32 direction_z;
    s32 placement_kind;
    s32 work_a;
    s32 sibling_index;
    s32 sibling_part_index;
    s32 binding_index;
    u8 track_object_index;
    u8* resource;

    effect_index = 0;
    work_value = FIELD_EFFECT_RETIRED;
    record_base = g_field_effect_records;
    free_effect = record_base;

    for (; effect_index < FIELD_EFFECT_ACTIVE_RECORD_COUNT; effect_index++, free_effect++)
    {
        if (free_effect->state == work_value)
        {
            break;
        }
    }

    if (effect_index == FIELD_EFFECT_ACTIVE_RECORD_COUNT)
    {
        g_field_action_context = 0x10101010;
        return -1;
    }

    effect = FIELD_ELEMENT_AT(FieldMotionRecord, record_base, effect_index);
    part = &actor->parts[part_index];

    if (part->effect_flags.word & FIELD_PART_SPAWN_LINKED)
    {
        effect->previous_effect_index = work_value;
    }

    if (FIELD_PART_PLACEMENT_KIND(part) >= FIELD_PLACEMENT_SIBLING_HIGH &&
        FIELD_PART_PLACEMENT_KIND(part) < FIELD_PLACEMENT_SIBLING_HIGH + FIELD_PLACEMENT_SIBLING_COUNT &&
        FIELD_PART_PLACEMENT_KIND(part) - (FIELD_PLACEMENT_SIBLING_HIGH - FIELD_PLACEMENT_SIBLING_COUNT) == part_index)
    {
        return -1;
    }

    if (FIELD_PART_PLACEMENT_KIND(part) >= FIELD_PLACEMENT_SIBLING &&
        FIELD_PART_PLACEMENT_KIND(part) < FIELD_PLACEMENT_SIBLING + FIELD_PLACEMENT_SIBLING_COUNT &&
        FIELD_PART_PLACEMENT_KIND(part) - FIELD_PLACEMENT_SIBLING == part_index)
    {
        return -1;
    }

    if (FIELD_PART_PLACEMENT_KIND(part) >= FIELD_PLACEMENT_SIBLING_OFFSET &&
        FIELD_PART_PLACEMENT_KIND(part) < FIELD_PLACEMENT_SIBLING_OFFSET + FIELD_PLACEMENT_SIBLING_COUNT &&
        FIELD_PART_PLACEMENT_KIND(part) - FIELD_PLACEMENT_SIBLING_OFFSET == part_index)
    {
        return -1;
    }

    if (part->orientation_flags.word & FIELD_PART_ORIENTATION_MODE_MASK)
    {
        effect->color_position.fields.position_source =
            (part->effect_flags.word >> FIELD_PART_ORIENTED_POSITION_SOURCE_SHIFT) & FIELD_PART_ORIENTED_POSITION_SOURCE_MASK;
    }
    else
    {
        effect->color_position.fields.position_source = (part->behavior_flags.word >> FIELD_PART_POSITION_SOURCE_SHIFT) & FIELD_PART_POSITION_SOURCE_MASK;
    }

    FIELD_EFFECT_FLAGS(effect).bits.motion_kind = part->behavior_flags.word >> FIELD_PART_MOTION_KIND_SHIFT;
    FIELD_EFFECT_FLAGS(effect).bits.distance_mode = part->rotation_extent.fields.extent_value_mode >> FIELD_PART_DISTANCE_MODE_SHIFT;
    effect->lifetime = part->active_frames;
    effect->motion_parameter = part->orientation_flags.halves.high;
    effect->motion_scale = part->motion_scale;
    effect->track_index = g_field_track_index;
    FIELD_EFFECT_FLAGS(effect).bits.parameter_mode = part->orientation_flags.bytes.low;
    FIELD_EFFECT_FLAGS(effect).bits.screen_space = 0;

    FIELD_EFFECT_FLAGS(effect).bits.retire_on_hit = part->spawn_flags.word >> FIELD_PART_RETIRE_ON_HIT_BIT;
    FIELD_EFFECT_FLAGS(effect).bits.ground_point = 0;
    FIELD_EFFECT_FLAGS(effect).bits.petrified = 0;

    FIELD_EFFECT_FLAGS(effect).bits.kind = 0;

    if (part->effect_flags.word & FIELD_EFFECT_SEMITRANSPARENT)
    {
        s32 eval =
            field_evaluate_parameter_track_at_time(actor, (part->effect_flags.word >> FIELD_PART_SEMITRANSPARENT_TRACK_SHIFT) & FIELD_TRACK_INDEX_MASK, 0) != 0;
        FIELD_EFFECT_FLAGS(effect).bits.semitransparent = eval;
    }
    else
    {
        FIELD_EFFECT_FLAGS(effect).bits.semitransparent = part->behavior_flags.word >> FIELD_PART_SEMITRANSPARENT_BIT;
    }

    FIELD_EFFECT_FLAGS(effect).bits.group = 0;

    if (effect->color_position.fields.position_source == FIELD_POSITION_LINKED_EFFECT)
    {
        s32 found;

        work_index = 0;
        found = 0;
        work_value = actor->active_counts[g_field_track_index][part_index];

        for (; work_index < FIELD_EFFECT_ACTIVE_RECORD_COUNT; work_index++)
        {
            if (g_field_effect_records[work_index].state != FIELD_EFFECT_RETIRED && g_field_effect_records[work_index].owner_part == part->linked_effect_part &&
                g_field_effect_records[work_index].owner_slot == actor->slot_index)
            {
                found = 1;

                if (work_value == 0)
                {
                    effect->position_data.linked_effect_index = work_index;
                    break;
                }

                effect->position_data.linked_effect_index = work_index;
                work_value--;
            }
        }

        if (found == 0)
        {
            effect->state = FIELD_EFFECT_RETIRED;
            return -1;
        }
    }

    effect->state = part->effect_kind;
    effect->saved_state = 0;
    effect->frame_ticks = 0;

    effect->work_x = part->target_offset_x << 8;
    effect->work_y = part->target_offset_y << 8;
    effect->work_z = part->target_offset_z << 8;

    if (((part->placement_flags.word >> FIELD_PART_KIND_RANGE_BIT) & 1) &&
        (part->appearance.word & (FIELD_PART_KIND_RANGE_COUNT_MASK << FIELD_PART_KIND_RANGE_COUNT_SHIFT)))
    {
        if (part->spawn_flags.word & FIELD_PART_KIND_FROM_REWARD_COUNTERS)
        {
            for (work_index = 0; work_index < (s32)((part->appearance.word >> FIELD_PART_KIND_RANGE_COUNT_SHIFT) & FIELD_PART_KIND_RANGE_COUNT_MASK);
                 work_index++)
            {
                FieldObjectState* states = g_field_object_states;

                owner_object = &states[actor->owner_object_index];

                if (owner_object->signal.reward_counters[work_index] != 0)
                {
                    owner_object->signal.reward_counters[work_index]--;
                    effect->facing_or_reward_kind = part->facing_or_reward_kind + work_index;
                    break;
                }
            }

            if (work_index == (part->appearance.fields.spawn_flags & FIELD_PART_KIND_RANGE_COUNT_MASK))
            {
                effect->state = FIELD_EFFECT_RETIRED;
                return -1;
            }
        }
        else
        {
            effect->facing_or_reward_kind =
                part->facing_or_reward_kind + (((part->appearance.fields.spawn_flags & FIELD_PART_KIND_RANGE_COUNT_MASK) * rand()) >> 15);
        }
    }
    else
    {
        effect->facing_or_reward_kind = part->facing_or_reward_kind;
    }

    rotation_z_source = (part->placement_flags.word >> FIELD_PART_ROTATION_Z_SOURCE_SHIFT) & FIELD_PART_ROTATION_SOURCE_MASK;

    switch (rotation_z_source)
    {
        case FIELD_PART_VALUE_FIXED:
            effect->rotation_z_16 = part->rotation_extent.fields.rotation_z_track;
            break;
        case FIELD_PART_VALUE_TRACK:
            effect->rotation_z_16 = field_evaluate_parameter_track(actor, part->rotation_extent.fields.rotation_z_track & FIELD_TRACK_INDEX_MASK);
            break;
        case FIELD_PART_VALUE_TIMED_TRACK:
            effect->rotation_z_16 = field_evaluate_parameter_track_at_time(actor, part->rotation_extent.fields.rotation_z_track & FIELD_TRACK_INDEX_MASK, 0);
            break;
    }

    rotation_y_source = (part->placement_flags.word >> FIELD_PART_ROTATION_Y_SOURCE_SHIFT) & FIELD_PART_ROTATION_SOURCE_MASK;

    switch (rotation_y_source)
    {
        case FIELD_PART_VALUE_FIXED:
            effect->rotation_y_16 = part->rotation_extent.fields.rotation_y_track;
            break;
        case FIELD_PART_VALUE_TRACK:
            effect->rotation_y_16 = field_evaluate_parameter_track(actor, part->rotation_extent.fields.rotation_y_track & FIELD_TRACK_INDEX_MASK);
            break;
        case FIELD_PART_VALUE_TIMED_TRACK:
            effect->rotation_y_16 = field_evaluate_parameter_track_at_time(actor, part->rotation_extent.fields.rotation_y_track & FIELD_TRACK_INDEX_MASK, 0);
            break;
    }

    if ((((part->placement_flags.word >> FIELD_PART_MIRROR_X_WITH_FACING_SHIFT) & 1) || (part->spawn_flags.word & FIELD_PART_MIRROR_X_WITH_OWNER)) &&
        effect->color_position.fields.position_source == FIELD_POSITION_NONE &&
        !(g_field_actors[actor->owner_object_index].animation & FIELD_EFFECT_FACING_FLIPPED))
    {
        u8 angle;
        u8 original_angle;

        original_angle = effect->rotation_y_16;
        angle = original_angle;

        if (angle >= FIELD_HEADING_QUARTER_TURN && original_angle < FIELD_HEADING_HALF_TURN)
        {
            effect->rotation_y_16 = FIELD_HEADING_HALF_TURN - original_angle;
        }
        else
        {
            effect->rotation_y_16 = -(original_angle - FIELD_HEADING_HALF_TURN);
        }
    }

    effect->age = 0;
    actor->active_counts[g_field_track_index][part_index]++;
    effect->saved_state = 0;
    effect->owner_part = part_index;
    effect->owner_slot = actor->slot_index;

    if ((part->placement_flags.word >> FIELD_PART_TRACK_COLOR_BIT) & 1)
    {
        if ((part->behavior_flags.word >> FIELD_PART_RGB_TRACKS_SHIFT) & 1)
        {
            effect->color_position.fields.red =
                field_evaluate_parameter_track(actor, (part->behavior_flags.word >> FIELD_PART_COLOR_TRACK_INDEX_SHIFT) & FIELD_TRACK_INDEX_MASK);
            effect->color_position.fields.green =
                field_evaluate_parameter_track(actor, (((part->behavior_flags.halves.high & FIELD_TRACK_INDEX_MASK) + 1) & FIELD_TRACK_INDEX_MASK));
            effect->color_position.fields.blue =
                field_evaluate_parameter_track(actor, (((part->behavior_flags.halves.high & FIELD_TRACK_INDEX_MASK) + 2) & FIELD_TRACK_INDEX_MASK));
        }
        else
        {
            effect->color_position.fields.red = effect->color_position.fields.green = effect->color_position.fields.blue =
                field_evaluate_parameter_track(actor, (part->behavior_flags.word >> FIELD_PART_COLOR_TRACK_INDEX_SHIFT) & FIELD_TRACK_INDEX_MASK);
        }
    }

    FIELD_EFFECT_FLAGS(effect).bits.effect_color = part->behavior_flags.word >> FIELD_PART_OWN_COLOR_SHIFT;

    FIELD_EFFECT_FLAGS(effect).bits.literal_color = part->placement_flags.word >> FIELD_PART_TRACK_COLOR_BIT;
    field_roll_effect_rotation(actor, part, effect);
    RotMatrix_gte((SVECTOR*)&effect->rotation_x, rotation_matrix);
    RotMatrixZ(effect->rotation_z_16 * 16, rotation_matrix);
    RotMatrixY(effect->rotation_y_16 * 16, rotation_matrix);
    local_direction->vx = 0;
    local_direction->vy = -ONE;
    local_direction->vz = 0;
    gte_SetRotMatrix(rotation_matrix);
    gte_ldv0(local_direction);
    gte_rtv0();
    gte_stlvnl(direction_vector);

    if (((part->behavior_flags.word >> FIELD_PART_PITCH_ACCELERATION_BIT) & 1) || effect->color_position.fields.position_source != FIELD_POSITION_NONE)
    {
        effect->heading = ratan2(-direction_vector->vz, direction_vector->vx);
        gte_ldlvl(direction_vector);
        gte_sqr0();
        gte_stlvnl(squared_vector);
        effect->pitch = ratan2(SquareRoot0(squared_vector->vx + squared_vector->vz), -direction_vector->vy);
        effect->rotation_x = 0;

        if (effect->pitch < 0)
        {
            effect->pitch = -effect->pitch;
        }
    }

    rotated_x = direction_vector->vx;
    work_index = field_resolve_effect_extent(actor, part);
    FIELD_EFFECT_FLAGS(effect).bits.distance = work_index;
    effect->x = (work_index * direction_vector->vx) >> FIELD_GTE_TO_POSITION_SHIFT;
    effect->y = (work_index * direction_vector->vy) >> FIELD_GTE_TO_POSITION_SHIFT;
    effect->z = (work_index * direction_vector->vz) >> FIELD_GTE_TO_POSITION_SHIFT;
    effect->animation_active = part->effect_animation_active;
    effect->source_object_index = actor->owner_object_index;

    if (effect->state == FIELD_EFFECT_RENDER_SPRITE)
    {
        field_begin_actor_animation_forward((FieldActor*)effect, g_field_builtin_track_data);
    }
    else if (effect->state == FIELD_EFFECT_RENDER_ACTOR_SPRITE && (resource = g_field_actor_slots[effect->owner_slot].track_data) != 0)
    {
        field_begin_actor_animation_forward((FieldActor*)effect, resource);
    }
    else if (effect->state == FIELD_EFFECT_RENDER_OWNER_SPRITE)
    {
        owner_object_state = field_object_state(actor->owner_object_index);

        if ((owner_object_state->contact.bytes.flags & 1) && owner_object_state->contact.bytes.controller_index != actor->slot_index)
        {
            effect->state = FIELD_EFFECT_RETIRED;
        }
        else
        {
            record_base = &FIELD_ACTOR_MOTION_RECORDS[actor->owner_object_index];

            if (FIELD_PART_PASSES_OWNER_TINT(part))
            {
                FIELD_EFFECT_FLAGS(effect).bits.effect_color = 1;
                FIELD_EFFECT_FLAGS(effect).bits.literal_color = 1;

                effect->color_position.fields.red = g_field_object_parts[record_base->source_object_index].tint_red;
                effect->color_position.fields.green = g_field_object_parts[record_base->source_object_index].tint_green;
                effect->color_position.fields.blue = g_field_object_parts[record_base->source_object_index].tint_blue;
            }

            effect->source_object_index = g_field_actors[actor->owner_object_index].object_index;
            effect->resource_index = g_field_actors[actor->owner_object_index].resource_index;
            effect->texture_slot = g_field_actors[actor->owner_object_index].texture_slot;
            effect->facing_or_reward_kind |= g_field_actors[actor->owner_object_index].animation & FIELD_EFFECT_FACING_FLIPPED;
            FIELD_EFFECT_FLAGS(effect).bits.group = FIELD_EFFECT_FLAGS(&g_field_actors[actor->owner_object_index]).half[1];
            FIELD_EFFECT_FLAGS(effect).bits.kind = FIELD_EFFECT_FLAGS(&g_field_actors[actor->owner_object_index]).bits.kind;
            resource = g_field_resource_entries[g_field_actors[actor->owner_object_index].resource_index].start;

            if (resource != 0)
            {
                field_restart_actor_animation(effect, resource);
            }
        }
    }
    else if (effect->state == FIELD_EFFECT_RENDER_TRACK_SPRITE)
    {
        track_object_index = actor->targets[g_field_track_index];

        if (track_object_index == FIELD_TARGET_NONE)
        {
            return field_cancel_effect_spawn(actor, effect, part_index);
        }

        if ((!((g_field_object_states[actor->targets[g_field_track_index]].contact.word >> FIELD_CONTACT_ACTION_PENDING_BIT) & 1) &&
             (binding_index = field_object_binding(actor->targets[g_field_track_index])->owner) == actor->targets[g_field_track_index] &&
             field_object_binding(binding_index)->state != 0) ||
            ((g_field_object_states[actor->targets[g_field_track_index]].contact.bytes.flags & 1) &&
             g_field_object_states[actor->targets[g_field_track_index]].contact.bytes.controller_index != actor->slot_index))
        {
            effect->state = FIELD_EFFECT_RETIRED;
        }
        else
        {
            record_base = &FIELD_ACTOR_MOTION_RECORDS[actor->targets[g_field_track_index]];

            if (FIELD_PART_PASSES_OWNER_TINT(part))
            {
                FIELD_EFFECT_FLAGS(effect).bits.effect_color = 1;
                FIELD_EFFECT_FLAGS(effect).bits.literal_color = 1;

                effect->color_position.fields.red = g_field_object_parts[record_base->source_object_index].tint_red;
                effect->color_position.fields.green = g_field_object_parts[record_base->source_object_index].tint_green;
                effect->color_position.fields.blue = g_field_object_parts[record_base->source_object_index].tint_blue;
            }

            effect->source_object_index = g_field_actors[actor->targets[g_field_track_index]].object_index;
            effect->resource_index = g_field_actors[actor->targets[g_field_track_index]].resource_index;
            effect->texture_slot = g_field_actors[actor->targets[g_field_track_index]].texture_slot;
            effect->facing_or_reward_kind |= g_field_actors[actor->targets[g_field_track_index]].animation & FIELD_EFFECT_FACING_FLIPPED;
            FIELD_EFFECT_FLAGS(effect).bits.group = FIELD_EFFECT_FLAGS(&g_field_actors[actor->targets[g_field_track_index]]).half[1];
            FIELD_EFFECT_FLAGS(effect).bits.kind = FIELD_EFFECT_FLAGS(&g_field_actors[actor->targets[g_field_track_index]]).bits.kind;
            resource = g_field_resource_entries[g_field_actors[actor->targets[g_field_track_index]].resource_index].start;

            if (resource != 0)
            {
                field_restart_actor_animation(effect, resource);
            }
        }
    }

    if ((part->track_flags.word >> FIELD_PART_MOTION_SCALE_TRACK_BIT) & 1)
    {
        effect->motion_scale = field_evaluate_parameter_track(actor, part->motion_scale & FIELD_TRACK_INDEX_MASK);
    }

    if ((((part->placement_flags.word >> FIELD_PART_MIRROR_X_WITH_FACING_SHIFT) & 1) || (part->spawn_flags.word & FIELD_PART_MIRROR_X_WITH_OWNER)) &&
        effect->color_position.fields.position_source != FIELD_POSITION_NONE &&
        !(g_field_actors[actor->owner_object_index].animation & FIELD_EFFECT_FACING_FLIPPED))
    {
        rotated_x = -rotated_x;
        effect->x = -effect->x;
    }

    if ((effect->flags & FIELD_EFFECT_MOTION_KIND_MASK) == FIELD_EFFECT_MOTION_PATH)
    {
        effect->x = 0;
        effect->y = 0;
        effect->z = 0;
    }

    if (part->effect_flags.word & FIELD_PART_ORIGIN_MASK)
    {
        effect->x = 0;
        effect->y = 0;
        effect->z = 0;
    }

    placement_kind = part->placement_flags.word >> FIELD_PART_PLACEMENT_KIND_SHIFT;
    placement_kind &= FIELD_PART_PLACEMENT_KIND_MASK;

    switch (placement_kind)
    {
        case FIELD_PLACEMENT_OWNER_BOUNDS:
        case FIELD_PLACEMENT_OWNER_BOUNDS + 1:
        case FIELD_PLACEMENT_OWNER_BOUNDS + 2:
        case FIELD_PLACEMENT_OWNER_BOUNDS + 3:
        case FIELD_PLACEMENT_OWNER_BOUNDS + 4:
        case FIELD_PLACEMENT_OWNER_BOUNDS + 5:
        case FIELD_PLACEMENT_OWNER_BOUNDS + 6:
        case FIELD_PLACEMENT_OWNER_BOUNDS + 7:
        case FIELD_PLACEMENT_OWNER_BOUNDS + 8:
        case FIELD_PLACEMENT_OWNER_BOUNDS + 9:
        case FIELD_PLACEMENT_TRACK_BOUNDS:
        case FIELD_PLACEMENT_TRACK_BOUNDS + 1:
        case FIELD_PLACEMENT_TRACK_BOUNDS + 2:
        case FIELD_PLACEMENT_TRACK_BOUNDS + 3:
        case FIELD_PLACEMENT_TRACK_BOUNDS + 4:
        case FIELD_PLACEMENT_TRACK_BOUNDS + 5:
        case FIELD_PLACEMENT_TRACK_BOUNDS + 6:
        case FIELD_PLACEMENT_TRACK_BOUNDS + 7:
        case FIELD_PLACEMENT_TRACK_BOUNDS + 8:
        case FIELD_PLACEMENT_TRACK_BOUNDS + 9:
            if (placement_kind >= FIELD_PLACEMENT_TRACK_BOUNDS)
            {
                track_object_index = actor->targets[g_field_track_index];

                if (track_object_index == FIELD_TARGET_NONE)
                {
                    return field_cancel_effect_spawn(actor, effect, part_index);
                }

                object_index = actor->targets[g_field_track_index];
                placement_kind -= FIELD_PLACEMENT_TRACK_BOUNDS;
                source_record = &FIELD_ACTOR_MOTION_RECORDS[object_index];
                placement_object = &g_field_object_states[object_index];
            }
            else
            {
                owner_placement_guard = field_object_state(actor->owner_object_index);

                if ((owner_placement_guard->contact.bytes.flags & 1) && actor->slot_index >= 0x40U &&
                    !((owner_placement_guard->contact.word >> FIELD_CONTACT_NO_HIT_TEST_BIT) & 1) &&
                    owner_placement_guard->contact.bytes.controller_index != actor->slot_index)
                {
                    return field_cancel_effect_spawn(actor, effect, part_index);
                }

                object_index = actor->owner_object_index;
                source_record = &FIELD_ACTOR_MOTION_RECORDS[object_index];
                placement_object = &g_field_object_states[object_index];
            }

            if ((part->placement_flags.word >> FIELD_PART_SCALE_X_FROM_BOUNDS_SHIFT) & 1)
            {
                extent = abs(placement_object->bounds.half.right - placement_object->bounds.half.left);
                part->appearance.fields.scale_xz = extent * 2;
            }

            if ((part->placement_flags.word >> FIELD_PART_SCALE_Y_FROM_BOUNDS_SHIFT) & 1)
            {
                extent = abs(placement_object->bounds.half.bottom - placement_object->bounds.half.top);
                part->scale_y = extent * 2;
            }

            work_a = 0;
            work_value = 0;

            switch (placement_kind)
            {
                case FIELD_BOUNDS_POINT_CENTER:
                    work_value = (placement_object->bounds.half.right + placement_object->bounds.half.left) >> 1;
                    work_a = (placement_object->bounds.half.bottom + placement_object->bounds.half.top) >> 1;
                    break;
                case FIELD_BOUNDS_POINT_CENTER_X:
                    work_value = (placement_object->bounds.half.right + placement_object->bounds.half.left) >> 1;
                    work_a = 0;
                    break;
                case FIELD_BOUNDS_POINT_TOP_CENTER:
                    work_value = (placement_object->bounds.half.right + placement_object->bounds.half.left) >> 1;
                    work_a = placement_object->bounds.half.top;
                    break;
                case FIELD_BOUNDS_POINT_LEFT_CENTER:
                    work_value = placement_object->bounds.half.left;
                    work_a = (placement_object->bounds.half.bottom + placement_object->bounds.half.top) >> 1;
                    break;
                case FIELD_BOUNDS_POINT_RIGHT_CENTER:
                    work_value = placement_object->bounds.half.right;
                    work_a = (placement_object->bounds.half.bottom + placement_object->bounds.half.top) >> 1;
                    break;
                case FIELD_BOUNDS_POINT_TOP_LEFT:
                    work_value = placement_object->bounds.half.left;
                    work_a = placement_object->bounds.half.top;
                    break;
                case FIELD_BOUNDS_POINT_TOP_RIGHT:
                    work_value = placement_object->bounds.half.right;
                    work_a = placement_object->bounds.half.top;
                    break;
                case FIELD_BOUNDS_POINT_BOTTOM_LEFT:
                    work_value = placement_object->bounds.half.left;
                    work_a = placement_object->bounds.half.bottom;
                    break;
                case FIELD_BOUNDS_POINT_BOTTOM_RIGHT:
                    work_value = placement_object->bounds.half.right;
                    work_a = placement_object->bounds.half.bottom;
                    break;
                default:
                    break;
            }

            work_value <<= 8;
            work_a <<= 8;
            effect->x += source_record->x + work_value;
            effect->y += source_record->y + work_a;
            effect->z += source_record->z;

            if (effect->state == FIELD_EFFECT_RENDER_LINKED_SPRITE)
            {
                effect->source_object_index = source_record->source_object_index;
                attached_source_object = field_object_state(source_record->source_object_index);

                if (!(attached_source_object->contact.bytes.flags & 1) || attached_source_object->contact.bytes.controller_index == actor->slot_index)
                {
                    effect->texture_slot = source_record->texture_slot;
                    effect->resource_index = source_record->resource_index;
                    FIELD_EFFECT_FLAGS(effect).bits.kind = FIELD_EFFECT_FLAGS(source_record).bits.kind;
                    FIELD_EFFECT_FLAGS(effect).bits.group = FIELD_EFFECT_FLAGS(source_record).half[1];

                    if (FIELD_PART_PASSES_OWNER_TINT(part))
                    {
                        FIELD_EFFECT_FLAGS(effect).bits.effect_color = 1;
                        FIELD_EFFECT_FLAGS(effect).bits.literal_color = 1;

                        effect->color_position.fields.red = g_field_object_parts[source_record->source_object_index].tint_red;
                        effect->color_position.fields.green = g_field_object_parts[source_record->source_object_index].tint_green;
                        effect->color_position.fields.blue = g_field_object_parts[source_record->source_object_index].tint_blue;
                    }

                    if (effect->facing_or_reward_kind == FIELD_EFFECT_ANIMATION_FROM_SOURCE)
                    {
                        effect->facing_or_reward_kind = source_record->facing_or_reward_kind;
                        effect->saved_state = source_record->saved_state;
                        part->appearance.fields.scale_xz = g_field_object_parts[source_record->source_object_index].appearance.fields.scale_xz;
                        part->scale_y = g_field_object_parts[source_record->source_object_index].scale_y;
                        effect->frame_ticks = source_record->frame_ticks;
                        effect->frame_length = source_record->frame_length;
                        effect->track_index = source_record->track_index;
                        effect->motion_remainder = source_record->motion_remainder;
                        effect->vertical_offset = source_record->vertical_offset;
                        effect->next_height = source_record->next_height;
                        effect->motion_divisor = source_record->motion_divisor;
                    }
                    else
                    {
                        effect->facing_or_reward_kind |= source_record->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED;
                        resource = g_field_resource_entries[source_record->resource_index].start;

                        if (resource != 0)
                        {
                            field_restart_actor_animation(effect, resource);
                        }
                    }
                }
                else
                {
                    effect->state = FIELD_EFFECT_DISABLED;
                    break;
                }
            }

            break;
        case FIELD_PLACEMENT_SIBLING:
        case FIELD_PLACEMENT_SIBLING + 1:
        case FIELD_PLACEMENT_SIBLING + 2:
        case FIELD_PLACEMENT_SIBLING + 3:
        case FIELD_PLACEMENT_SIBLING + 4:
        case FIELD_PLACEMENT_SIBLING + 5:
        case FIELD_PLACEMENT_SIBLING + 6:
        case FIELD_PLACEMENT_SIBLING + 7:
        case FIELD_PLACEMENT_SIBLING_HIGH:
        case FIELD_PLACEMENT_SIBLING_HIGH + 1:
        case FIELD_PLACEMENT_SIBLING_HIGH + 2:
        case FIELD_PLACEMENT_SIBLING_HIGH + 3:
        case FIELD_PLACEMENT_SIBLING_HIGH + 4:
        case FIELD_PLACEMENT_SIBLING_HIGH + 5:
        case FIELD_PLACEMENT_SIBLING_HIGH + 6:
        case FIELD_PLACEMENT_SIBLING_HIGH + 7:
            if (placement_kind < FIELD_PLACEMENT_SIBLING_HIGH)
            {
                sibling_part_index = placement_kind - FIELD_PLACEMENT_SIBLING;
            }
            else
            {
                sibling_part_index = placement_kind - (FIELD_PLACEMENT_SIBLING_HIGH - FIELD_PLACEMENT_SIBLING_COUNT);
            }

            for (work_a = start; work_a < FIELD_EFFECT_ACTIVE_RECORD_COUNT; work_a++)
            {
                if (g_field_effect_records[work_a].state != FIELD_EFFECT_RETIRED && g_field_effect_records[work_a].owner_part == sibling_part_index &&
                    g_field_effect_records[work_a].owner_slot == actor->slot_index &&
                    ((actor->parts[sibling_part_index].orientation_flags.word & FIELD_PART_FIRST_TRACK_ONLY) ||
                     g_field_effect_records[work_a].track_index == g_field_track_index))
                {
                    effect->x += g_field_effect_records[work_a].x;
                    effect->y += g_field_effect_records[work_a].y;
                    effect->z += g_field_effect_records[work_a].z;
                    FIELD_EFFECT_FLAGS(effect).bits.screen_space = FIELD_EFFECT_FLAGS(&g_field_effect_records[work_a]).bits.screen_space;

                    if (part->palette_extent.word & FIELD_PART_COPY_SIBLING_ROTATION)
                    {
                        s16* rotation = &g_field_effect_records[work_a].rotation_x;

                        effect->rotation_x = rotation[0];
                        effect->heading = rotation[1];
                        effect->pitch = rotation[2];
                    }

                    effect->reference_index = work_a;

                    if (effect->state == FIELD_EFFECT_RENDER_LINKED_SPRITE)
                    {
                        field_adopt_sibling_sprite(effect, work_a);
                    }

                    break;
                }
            }

            if (work_a == FIELD_EFFECT_ACTIVE_RECORD_COUNT)
            {
                return field_cancel_effect_spawn(actor, effect, part_index);
            }

            field_spawn_actor_effect(actor, part_index, work_a + 1);
            break;

        case FIELD_PLACEMENT_FIELD_POSITION:
            effect->x += (part->offset_x << 8) - g_field_view_offset_x;
            effect->y += (part->offset_y << 8) - g_field_view_offset_y;
            effect->z += (part->offset_z << 8) - g_field_view_offset_z;

            if (effect->state == FIELD_EFFECT_RENDER_LINKED_SPRITE)
            {
                effect->state = FIELD_EFFECT_DISABLED;
            }

            break;

        case FIELD_PLACEMENT_SCREEN_CENTER:
            effect->x -= g_field_view_offset_x;
            effect->y -= g_field_view_offset_y;
            effect->z -= g_field_view_offset_z;
            FIELD_EFFECT_FLAGS(effect).bits.screen_space = 1;
            break;

        case FIELD_PLACEMENT_SCREEN_TOP:
            effect->y -= FIELD_SCREEN_HALF_HEIGHT_FIXED;
            effect->x -= g_field_view_offset_x;
            effect->z -= g_field_view_offset_z;
            FIELD_EFFECT_FLAGS(effect).bits.screen_space = 1;
            effect->y -= g_field_view_offset_y;
            break;

        case FIELD_PLACEMENT_SCREEN_BOTTOM:
            effect->y += FIELD_SCREEN_HALF_HEIGHT_FIXED;
            effect->x -= g_field_view_offset_x;
            effect->z -= g_field_view_offset_z;
            FIELD_EFFECT_FLAGS(effect).bits.screen_space = 1;
            effect->y -= g_field_view_offset_y;
            break;

        case FIELD_PLACEMENT_SCREEN_LEFT:
            effect->x -= FIELD_SCREEN_HALF_WIDTH_FIXED;
            effect->x -= g_field_view_offset_x;
            effect->y -= g_field_view_offset_y;
            effect->z -= g_field_view_offset_z;
            FIELD_EFFECT_FLAGS(effect).bits.screen_space = 1;
            break;

        case FIELD_PLACEMENT_SCREEN_RIGHT:
            effect->x += FIELD_SCREEN_HALF_WIDTH_FIXED;
            effect->x -= g_field_view_offset_x;
            effect->y -= g_field_view_offset_y;
            effect->z -= g_field_view_offset_z;
            FIELD_EFFECT_FLAGS(effect).bits.screen_space = 1;
            break;

        case FIELD_PLACEMENT_SCREEN_TOP_LEFT:
            effect->x -= FIELD_SCREEN_HALF_WIDTH_FIXED;
            effect->y -= FIELD_SCREEN_HALF_HEIGHT_FIXED;
            effect->x -= g_field_view_offset_x;
            effect->z -= g_field_view_offset_z;
            FIELD_EFFECT_FLAGS(effect).bits.screen_space = 1;
            effect->y -= g_field_view_offset_y;
            break;

        case FIELD_PLACEMENT_SCREEN_TOP_RIGHT:
            effect->x += FIELD_SCREEN_HALF_WIDTH_FIXED;
            effect->y -= FIELD_SCREEN_HALF_HEIGHT_FIXED;
            effect->x -= g_field_view_offset_x;
            effect->z -= g_field_view_offset_z;
            FIELD_EFFECT_FLAGS(effect).bits.screen_space = 1;
            effect->y -= g_field_view_offset_y;
            break;

        case FIELD_PLACEMENT_SCREEN_BOTTOM_LEFT:
            effect->x -= FIELD_SCREEN_HALF_WIDTH_FIXED;
            effect->y += FIELD_SCREEN_HALF_HEIGHT_FIXED;
            effect->x -= g_field_view_offset_x;
            effect->z -= g_field_view_offset_z;
            FIELD_EFFECT_FLAGS(effect).bits.screen_space = 1;
            effect->y -= g_field_view_offset_y;
            break;

        case FIELD_PLACEMENT_SCREEN_BOTTOM_RIGHT:
            effect->x += FIELD_SCREEN_HALF_WIDTH_FIXED;
            effect->y += FIELD_SCREEN_HALF_HEIGHT_FIXED;
            effect->x -= g_field_view_offset_x;
            effect->z -= g_field_view_offset_z;
            FIELD_EFFECT_FLAGS(effect).bits.screen_space = 1;
            effect->y -= g_field_view_offset_y;
            break;

        case FIELD_PLACEMENT_OWNER_OFFSET:
            owner_placement_state = field_object_state(actor->owner_object_index);

            if ((owner_placement_state->contact.bytes.flags & 1) && owner_placement_state->contact.bytes.controller_index != actor->slot_index &&
                actor->slot_index >= 0x40U && !((owner_placement_state->contact.word >> FIELD_CONTACT_NO_HIT_TEST_BIT) & 1))
            {
                return field_cancel_effect_spawn(actor, effect, part_index);
            }

            source_record = &FIELD_ACTOR_MOTION_RECORDS[actor->owner_object_index];

            if (((part->placement_flags.word >> FIELD_PART_MIRROR_X_WITH_FACING_SHIFT) & 1) &&
                !(source_record->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED))
            {
                effect->x += source_record->x - (part->offset_x << 8);
            }
            else
            {
                effect->x += source_record->x + (part->offset_x << 8);
            }

            effect->y += source_record->y + (part->offset_y << 8);
            effect->z += source_record->z + (part->offset_z << 8);

            if (effect->state == FIELD_EFFECT_RENDER_LINKED_SPRITE && effect->facing_or_reward_kind == FIELD_EFFECT_ANIMATION_FROM_SOURCE)
            {
                field_adopt_source_sprite(effect, source_record, actor);
            }

            break;

        case FIELD_PLACEMENT_TRACK_OFFSET:
            if (actor->targets[g_field_track_index] == FIELD_TARGET_NONE)
            {
                return field_cancel_effect_spawn(actor, effect, part_index);
            }

            source_record = &FIELD_ACTOR_MOTION_RECORDS[actor->targets[g_field_track_index]];

            if ((part->spawn_flags.word & FIELD_PART_MIRROR_X_WITH_OWNER) &&
                !(g_field_actors[actor->owner_object_index].animation & FIELD_EFFECT_FACING_FLIPPED))
            {
                effect->x += source_record->x - (part->offset_x << 8);
            }
            else if (((part->placement_flags.word >> FIELD_PART_MIRROR_X_WITH_FACING_SHIFT) & 1) &&
                     !(source_record->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED))
            {
                effect->x += source_record->x - (part->offset_x << 8);
            }
            else
            {
                effect->x += source_record->x + (part->offset_x << 8);
            }

            effect->y += source_record->y + (part->offset_y << 8);
            effect->z += source_record->z + (part->offset_z << 8);

            if (effect->state == FIELD_EFFECT_RENDER_LINKED_SPRITE && effect->facing_or_reward_kind == FIELD_EFFECT_ANIMATION_FROM_SOURCE)
            {
                field_adopt_source_sprite(effect, source_record, actor);
            }

            break;

        case FIELD_PLACEMENT_OWNER_ATTACHMENT:
            placement_object = &g_field_object_states[actor->owner_object_index];
            source_record = &FIELD_ACTOR_MOTION_RECORDS[actor->owner_object_index];
            effect->x +=
                source_record->x +
                (placement_object->attachment_points[(part->effect_flags.word >> FIELD_PART_ATTACHMENT_INDEX_SHIFT) & FIELD_PART_ATTACHMENT_INDEX_MASK].x << 8);
            effect->y +=
                source_record->y +
                (placement_object->attachment_points[(part->effect_flags.word >> FIELD_PART_ATTACHMENT_INDEX_SHIFT) & FIELD_PART_ATTACHMENT_INDEX_MASK].y << 8);
            effect->z += source_record->z;

            if (effect->state == FIELD_EFFECT_RENDER_LINKED_SPRITE)
            {
                effect->source_object_index = source_record->source_object_index;
                FIELD_EFFECT_FLAGS(effect).bits.kind = FIELD_EFFECT_FLAGS(source_record).bits.kind;
                FIELD_EFFECT_FLAGS(effect).bits.group = FIELD_EFFECT_FLAGS(source_record).half[1];
                field_attach_owner_sprite(effect, source_record, actor);
                break;
            }

            break;

        case FIELD_PLACEMENT_OWNER_ATTACHMENT_X:
            placement_object = &g_field_object_states[actor->owner_object_index];
            source_record = &FIELD_ACTOR_MOTION_RECORDS[actor->owner_object_index];
            effect->x +=
                source_record->x +
                (placement_object->attachment_points[(part->effect_flags.word >> FIELD_PART_ATTACHMENT_INDEX_SHIFT) & FIELD_PART_ATTACHMENT_INDEX_MASK].x << 8);
            effect->y += source_record->y;
            effect->z += source_record->z + (part->offset_z << 8);

            if (((part->placement_flags.word >> FIELD_PART_MIRROR_X_WITH_FACING_SHIFT) & 1) &&
                !(source_record->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED))
            {
                effect->x = effect->x - (part->offset_x << 8);
            }
            else
            {
                effect->x = effect->x + (part->offset_x << 8);
            }

            if (effect->state == FIELD_EFFECT_RENDER_LINKED_SPRITE)
            {
                FIELD_EFFECT_FLAGS(effect).bits.kind = FIELD_EFFECT_FLAGS(source_record).bits.kind;
                FIELD_EFFECT_FLAGS(effect).bits.group = FIELD_EFFECT_FLAGS(source_record).half[1];
                effect->source_object_index = source_record->source_object_index;
                field_attach_owner_sprite(effect, source_record, actor);
                break;
            }

            break;

        case FIELD_PLACEMENT_GROUND_POINT:
            placement_object = &g_field_object_states[actor->owner_object_index];
            source_record = &FIELD_ACTOR_MOTION_RECORDS[actor->owner_object_index];
            effect->x += source_record->x + (placement_object->ground_effect_points[g_field_spawn_ground_point].x << 8);
            effect->y += source_record->y;
            effect->z += source_record->z + (placement_object->ground_effect_points[g_field_spawn_ground_point].y << 8);
            FIELD_EFFECT_FLAGS(effect).bits.ground_point = (u16)g_field_spawn_ground_point;
            break;

        case FIELD_PLACEMENT_TRACK_ACTION_OFFSET:
            if (actor->targets[g_field_track_index] == FIELD_TARGET_NONE)
            {
                return field_cancel_effect_spawn(actor, effect, part_index);
            }

            source_record = &FIELD_ACTOR_MOTION_RECORDS[actor->targets[g_field_track_index]];

            if (!(source_record->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED))
            {
                effect->x += source_record->x + (actor->track_offsets[g_field_track_index].x << 8);
            }
            else
            {
                effect->x += source_record->x - (actor->track_offsets[g_field_track_index].x << 8);
            }

            effect->y += source_record->y + (actor->track_offsets[g_field_track_index].y << 8);
            effect->z += source_record->z;
            break;

        case FIELD_PLACEMENT_GLOBAL_POINT:
            effect->x += g_field_effect_spawn_point.vx;
            effect->y += g_field_effect_spawn_point.vy;
            effect->z += g_field_effect_spawn_point.vz;
            break;

        case FIELD_PLACEMENT_PART_OFFSET:
            effect->x += part->offset_x << 8;
            effect->y += part->offset_y << 8;
            effect->z += part->offset_z << 8;

            if (effect->state == FIELD_EFFECT_RENDER_LINKED_SPRITE)
            {
                effect->state = FIELD_EFFECT_DISABLED;
            }

            break;

        case FIELD_PLACEMENT_SIBLING_OFFSET:
        case FIELD_PLACEMENT_SIBLING_OFFSET + 1:
        case FIELD_PLACEMENT_SIBLING_OFFSET + 2:
        case FIELD_PLACEMENT_SIBLING_OFFSET + 3:
        case FIELD_PLACEMENT_SIBLING_OFFSET + 4:
        case FIELD_PLACEMENT_SIBLING_OFFSET + 5:
        case FIELD_PLACEMENT_SIBLING_OFFSET + 6:
        case FIELD_PLACEMENT_SIBLING_OFFSET + 7:
            work_value = placement_kind - FIELD_PLACEMENT_SIBLING_OFFSET;

            for (sibling_index = start; sibling_index < FIELD_EFFECT_ACTIVE_RECORD_COUNT; sibling_index++)
            {
                if (g_field_effect_records[sibling_index].state != FIELD_EFFECT_RETIRED && g_field_effect_records[sibling_index].owner_part == work_value &&
                    g_field_effect_records[sibling_index].owner_slot == actor->slot_index &&
                    ((actor->parts[work_value].orientation_flags.word & FIELD_PART_FIRST_TRACK_ONLY) ||
                     g_field_effect_records[sibling_index].track_index == g_field_track_index))
                {
                    effect->x += g_field_effect_records[sibling_index].x;
                    effect->y += g_field_effect_records[sibling_index].y;
                    effect->z += g_field_effect_records[sibling_index].z;

                    if ((part->spawn_flags.word & FIELD_PART_MIRROR_X_WITH_OWNER) &&
                        !(g_field_actors[actor->owner_object_index].animation & FIELD_EFFECT_FACING_FLIPPED))
                    {
                        effect->x -= part->offset_x << 8;
                    }
                    else if (((part->placement_flags.word >> FIELD_PART_MIRROR_X_WITH_FACING_SHIFT) & 1) &&
                             !(g_field_effect_records[sibling_index].facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED))
                    {
                        effect->x -= part->offset_x << 8;
                    }
                    else
                    {
                        effect->x += part->offset_x << 8;
                    }

                    effect->y += part->offset_y << 8;
                    effect->z += part->offset_z << 8;
                    FIELD_EFFECT_FLAGS(effect).bits.screen_space = FIELD_EFFECT_FLAGS(&g_field_effect_records[sibling_index]).bits.screen_space;

                    if (part->palette_extent.word & FIELD_PART_COPY_SIBLING_ROTATION)
                    {
                        s16* rotation = &g_field_effect_records[sibling_index].rotation_x;

                        effect->rotation_x = rotation[0];
                        effect->heading = rotation[1];
                        effect->pitch = rotation[2];
                    }

                    effect->reference_index = sibling_index;

                    if (effect->state == FIELD_EFFECT_RENDER_LINKED_SPRITE)
                    {
                        field_adopt_sibling_sprite(effect, sibling_index);
                    }

                    break;
                }
            }

            if (sibling_index == FIELD_EFFECT_ACTIVE_RECORD_COUNT)
            {
                return field_cancel_effect_spawn(actor, effect, part_index);
            }

            field_spawn_actor_effect(actor, part_index, sibling_index + 1);
            break;
    }

    switch ((part->effect_flags.word >> FIELD_PART_ORIGIN_SHIFT) & 3)
    {
        case FIELD_PART_ORIGIN_OWNER:
            direction_vector->vx = (g_field_actors[actor->owner_object_index].x - effect->x) >> 8;
            direction_vector->vy = (g_field_actors[actor->owner_object_index].y - effect->y) >> 8;
            direction_z = (direction_vector->vz = (g_field_actors[actor->owner_object_index].z - effect->z) >> 8);
            effect->heading = ratan2(-direction_z, direction_vector->vx);
            gte_ldlvl(direction_vector);
            gte_sqr0();
            gte_stlvnl(squared_vector);
            effect->pitch = ratan2(SquareRoot0(squared_vector->vx + squared_vector->vz), -direction_vector->vy);
            effect->rotation_x = 0;
            effect->x = g_field_actors[actor->owner_object_index].x;
            effect->y = g_field_actors[actor->owner_object_index].y;
            effect->z = g_field_actors[actor->owner_object_index].z;
            break;
        case FIELD_PART_ORIGIN_TARGET:
            direction_vector->vx = (g_field_actors[actor->targets[g_field_track_index]].x - effect->x) >> 8;
            direction_vector->vy = (g_field_actors[actor->targets[g_field_track_index]].y - effect->y) >> 8;
            direction_z = (g_field_actors[actor->targets[g_field_track_index]].z - effect->z) >> 8;
            direction_vector->vz = direction_z;
            effect->heading = ratan2(-direction_z, direction_vector->vx);
            gte_ldlvl(direction_vector);
            gte_sqr0();
            gte_stlvnl(squared_vector);
            effect->pitch = ratan2(SquareRoot0(squared_vector->vx + squared_vector->vz), -direction_vector->vy);
            effect->rotation_x = 0;
            effect->x = g_field_actors[actor->targets[g_field_track_index]].x;
            effect->y = g_field_actors[actor->targets[g_field_track_index]].y;
            effect->z = g_field_actors[actor->targets[g_field_track_index]].z;
            break;
    }

    if ((effect->flags & FIELD_EFFECT_MOTION_KIND_MASK) == FIELD_EFFECT_MOTION_PATH)
    {
        field_init_path(effect, field_resolve_effect_extent(actor, part), (part->placement_flags.word >> FIELD_PART_RANDOM_EXTENT_SHIFT) & 1,
                        g_field_next_path_group);
        effect->position_data.path_time = 0;
        effect->path_group = g_field_next_path_group;
        field_update_path_position(&effect->position_data.path_time, effect, g_field_next_path_group);
        g_field_next_path_group = g_field_next_path_group + 1;

        if (g_field_next_path_group == FIELD_PATH_GROUP_COUNT)
        {
            g_field_next_path_group = 0;
        }
    }

    if (part->palette_extent.word & FIELD_PART_SWAP_POSITION_SOURCE)
    {
        field_swap_effect_position_source(effect, part);
    }

    if (!(effect->flags & FIELD_EFFECT_MOTION_KIND_MASK) && effect->color_position.fields.position_source != FIELD_POSITION_NONE)
    {
        field_face_effect_target(effect, part);
    }

    if ((part->placement_flags.word >> FIELD_PART_HEIGHT_TRACK_BIT) & 1)
    {
        if (part->spawn_flags.word & FIELD_PART_HEIGHT_FROM_BASE)
        {
            effect->height_or_retired_state = effect->y >> 8;
            effect->y -= field_evaluate_parameter_track_at_time(actor, part->placement_flags.bytes.low >> FIELD_PART_HEIGHT_TRACK_INDEX_SHIFT, 0) << 8;
        }
        else
        {
            effect->y = (-field_evaluate_parameter_track_at_time(
                            actor, (part->placement_flags.word >> FIELD_PART_HEIGHT_TRACK_INDEX_SHIFT) & FIELD_TRACK_INDEX_MASK, 0))
                        << 8;
        }
    }

    if (part->placement_flags.word & FIELD_PART_DEPTH_STEP)
    {
        effect->z += FIELD_WORLD_OT_DEPTH_STEP;
    }

    switch (FIELD_EFFECT_FLAGS(effect).bits.parameter_mode)
    {
        case FIELD_PART_VALUE_TRACK:
            effect->motion_parameter = field_evaluate_parameter_track(actor, part->orientation_flags.halves.high & FIELD_TRACK_INDEX_MASK);
            break;
        case FIELD_PART_VALUE_TIMED_TRACK:
            effect->motion_parameter = field_evaluate_parameter_track_at_time(actor, part->orientation_flags.halves.high & FIELD_TRACK_INDEX_MASK, 0);
            break;
        case FIELD_PART_VALUE_DISTANCE:
            field_resolve_effect_position(effect, part, direction_vector);
            direction_vector->vx -= effect->x;
            direction_vector->vy -= effect->y;
            direction_vector->vz -= effect->z;
            direction_vector->vx >>= 8;
            direction_vector->vy >>= 8;
            direction_vector->vz >>= 8;
            gte_ldlvl(direction_vector);
            gte_sqr0();
            gte_stlvnl(squared_vector);
            {
                s32 distance = SquareRoot0(squared_vector->vx + squared_vector->vy + squared_vector->vz) << 8;

                if (part->orientation_flags.halves.high != 0)
                {
                    effect->motion_parameter = (u32)(distance / part->orientation_flags.halves.high) >> 2;
                }
                else
                {
                    effect->motion_parameter = (u32)distance >> 2;
                }
            }

            break;
    }

    if (part->spawn_flags.word & FIELD_PART_MIRROR_X_WITH_OWNER)
    {
        effect->facing_or_reward_kind &= ~FIELD_EFFECT_FACING_FLIPPED;
    }

    if ((((part->placement_flags.word >> FIELD_PART_MIRROR_X_WITH_FACING_SHIFT) & 1) || (part->spawn_flags.word & FIELD_PART_MIRROR_X_WITH_OWNER)) &&
        !(g_field_actors[actor->owner_object_index].animation & FIELD_EFFECT_FACING_FLIPPED))
    {
        effect->facing_or_reward_kind ^= FIELD_EFFECT_FACING_FLIPPED;
    }

    if ((part->placement_flags.word >> FIELD_PART_FACE_TRAVEL_BIT) & 1)
    {
        if (rotated_x < 0)
        {
            effect->facing_or_reward_kind ^= FIELD_EFFECT_FACING_FLIPPED;
        }

        if (part->pitch_spread == 0)
        {
            effect->facing_or_reward_kind = (effect->facing_or_reward_kind & ~FIELD_EFFECT_FACING_FLIPPED) |
                                            (g_field_actors[actor->owner_object_index].animation & FIELD_EFFECT_FACING_FLIPPED);
        }
    }

    if (part->spawn_flags.word & FIELD_PART_FLIP_FACING)
    {
        effect->facing_or_reward_kind ^= FIELD_EFFECT_FACING_FLIPPED;
    }

    if ((actor->status.word & FIELD_SLOT_GROUND_EFFECT_BITS) == (GROUND_EFFECT_SCATTERED_DOMES << FIELD_SLOT_GROUND_EFFECT_SHIFT) &&
        FIELD_PART_PLACEMENT_KIND(part) == FIELD_PLACEMENT_GROUND_POINT)
    {
        g_field_spawn_ground_point = g_field_spawn_ground_point + 1;

        if (g_field_spawn_ground_point < FIELD_GROUND_EFFECT_POINT_COUNT)
        {
            actor->active_counts[g_field_track_index][part_index]--;
            actor->track_counters[g_field_track_index][part_index]--;

            if (field_spawn_actor_effect(actor, part_index, 0) == -1)
            {
                actor->active_counts[g_field_track_index][part_index]++;
                actor->track_counters[g_field_track_index][part_index]++;
            }
        }
    }

    if (effect->state != FIELD_EFFECT_DISABLED && effect->state != FIELD_EFFECT_RETIRED)
    {
        if (actor->targets[g_field_track_index] != FIELD_TARGET_NONE)
        {
            if ((actor->animation->sync_flags & FIELD_ANIM_TARGET_PART_HIDES) == FIELD_ANIM_TARGET_PART_HIDES &&
                (actor->animation->sync_flags >> FIELD_ANIM_TARGET_CURVE_SHIFT) == part_index)
            {
                g_field_actors[actor->targets[g_field_track_index]].presence = FIELD_ACTOR_HIDDEN;
                g_field_object_states[actor->targets[g_field_track_index]].contact.word |= FIELD_CONTACT_ANIMATION_HIDDEN;
                g_field_object_states[actor->targets[g_field_track_index]].contact.bytes.controller_index = actor->slot_index;
                actor->status.parts.hiding_objects = 1;
            }
        }

        if ((actor->animation->sync_flags & FIELD_ANIM_OWNER_PART_HIDES) == FIELD_ANIM_OWNER_PART_HIDES)
        {
            if (((actor->animation->sync_flags >> FIELD_ANIM_OWNER_CURVE_SHIFT) & FIELD_ANIM_CURVE_MASK) == part_index)
            {
                g_field_actors[actor->owner_object_index].presence = FIELD_ACTOR_HIDDEN;
                g_field_object_states[actor->owner_object_index].contact.word |= FIELD_CONTACT_ANIMATION_HIDDEN;
                g_field_object_states[actor->owner_object_index].contact.bytes.controller_index = actor->slot_index;
            }
        }
    }

    field_dispatch_actor_audio_event(actor, FIELD_SOUND_EVENT_PART_SPAWN, part_index);
    field_dispatch_actor_audio_event(actor, FIELD_SOUND_EVENT_PART_SPAWN_ONCE, part_index);
    return effect_index;
}

/**
 * @brief Roll one particle spawn record's scale and rotation fields from a
 *        part's parameter tracks, falling back to fixed part values or a
 *        random roll where a track is not assigned.
 * @param actor Owning actor supplying per-track counters.
 * @param part Part definition supplying rotation tracks and fallback values.
 * @param effect Spawned effect whose rotation fields are written.
 * @return Scaled pitch contribution added to @p effect.
 */
static s32 field_roll_effect_rotation(FieldActorSlot* actor, FieldObjectPart* part, FieldMotionRecord* effect)
{
    s16 angle;
    s32 random_product;
    s32 track_value;
    s32 track_scale;

    effect->rotation_x = 0;

    if (part->palette_extent.fields.angular_divisions != 0)
    {
        angle = ((FIELD_ANGLE_TURN / part->palette_extent.fields.angular_divisions) * actor->track_counters[g_field_track_index][part->part_index] +
                 FIELD_ANGLE_QUARTER_TURN) &
                FIELD_ANGLE_MASK;
        effect->heading = angle;
    }
    else
    {
        angle = (u32)rand() >> 3;
        effect->heading = angle;
    }

    actor->track_counters[g_field_track_index][part->part_index]++;

    if ((part->track_flags.word >> 0xB) & 1)
    {
        track_scale = field_evaluate_parameter_track(actor, part->pitch_spread & FIELD_TRACK_INDEX_MASK);
        random_product = track_scale * (rand() << 3);
    }
    else
    {
        random_product = part->pitch_spread * (rand() << 3);
    }

    effect->pitch = random_product >> 0xF;

    if ((part->track_flags.word >> 0xC) & 1)
    {
        track_value = field_evaluate_parameter_track(actor, part->pitch_offset & FIELD_TRACK_INDEX_MASK);
    }
    else
    {
        track_value = part->pitch_offset;
    }

    track_value *= 8;
    effect->pitch += track_value;
    return track_value;
}

/**
 * @brief Move an effect to its source and preserve its old position as the new source.
 * @param effect Record whose position and position-source selection are exchanged.
 * @param part Part definition controlling the selected position source.
 */
void field_swap_effect_position_source(FieldMotionRecord* effect, FieldObjectPart* part)
{
    VECTOR source_position;
    s32 unused[2];

    if (effect->color_position.fields.position_source != FIELD_POSITION_NONE)
    {
        field_resolve_effect_position(effect, part, &source_position);
        effect->color_position.fields.position_source = FIELD_POSITION_SAVED;
        effect->work_x = effect->x + g_field_view_offset_x;
        effect->work_y = effect->y + g_field_view_offset_y;
        effect->work_z = effect->z + g_field_view_offset_z;
        effect->x = source_position.vx;
        effect->y = source_position.vy;
        effect->z = source_position.vz;
    }
}

/**
 * @brief Face a record toward the delta between its last stored position and
 *        a freshly rolled position, deriving both a horizontal-plane heading
 *        and vertical pitch.
 * @param effect Record whose heading and pitch are updated.
 * @param part Part definition used to resolve the position source.
 */
static void field_face_effect_target(FieldMotionRecord* effect, FieldObjectPart* part)
{
    VECTOR target_position;
    VECTOR direction;
    VECTOR squared_direction;

    field_resolve_effect_position(effect, part, &target_position);
    direction.vx = (target_position.vx - effect->x) >> 8;
    direction.vy = (target_position.vy - effect->y) >> 8;
    direction.vz = (target_position.vz - effect->z) >> 8;

    gte_ldlvl(&direction);
    gte_sqr0();
    gte_stlvnl(&squared_direction);

    effect->heading = ratan2(-direction.vz, direction.vx);

    if (effect->heading < 0)
    {
        effect->heading += FIELD_ANGLE_TURN;
    }

    if (direction.vy != 0)
    {
        effect->pitch = ratan2(SquareRoot0(squared_direction.vx + squared_direction.vz), -direction.vy);
    }
    else
    {
        effect->pitch = FIELD_ANGLE_QUARTER_TURN;
    }

    if (effect->pitch < 0)
    {
        effect->pitch += FIELD_ANGLE_TURN;
    }

    effect->rotation_x = 0;
}

/**
 * @brief Per-frame actor tick: advances every active effect record owned by
 *        this actor (retiring expired and off-screen ones, spawning linked
 *        effects), then fills the animated CLUT row from the parts' palette
 *        tracks and advances palette and mesh texture animation.
 * @param actor Actor being ticked.
 */
void field_advance_actor_effects(FieldActorSlot* actor)
{
    FieldMotionRecord* effect;
    FieldObjectPart* part;
    FieldObjectPart* palette_part;
    u8* palette_buffer;
    s32 part_index;
    s32 new_effect_index;
    s32 animated_part_count;
    FieldMotionRecord* new_effect;
    RECT palette_rect;
    SVECTOR screen;

    for (effect = g_field_effect_records; effect != &g_field_effect_records[FIELD_EFFECT_ACTIVE_RECORD_COUNT]; effect++)
    {
        if (effect->owner_slot == actor->slot_index && effect->state != FIELD_EFFECT_RETIRED)
        {
            part = &actor->parts[effect->owner_part];
            g_field_track_index = effect->track_index;
            field_update_effect_record(effect, part, actor);
            effect->age++;

            if (FIELD_PART_END_MODE(part) == FIELD_PART_END_LIFETIME && (u16)effect->age == effect->lifetime)
            {
                FIELD_EFFECT_RETIRE(effect);
            }

            if (FIELD_PART_END_MODE(part) == FIELD_PART_END_OFFSCREEN)
            {
                screen.vx = FIELD_SCREEN_X(effect->x);
                screen.vy = FIELD_SCREEN_Y(effect->y, effect->z);

                /* Cull once the effect is more than a screen beyond any edge. */
                if (screen.vx < -SCREEN_WIDTH || screen.vx > SCREEN_WIDTH * 2 || screen.vy > SCREEN_HEIGHT * 2 || screen.vy < -SCREEN_HEIGHT)
                {
                    FIELD_EFFECT_RETIRE(effect);
                }
            }

            if (effect->state == FIELD_EFFECT_RETIRED)
            {
                field_retire_effect(effect, part);
            }

            if ((part->effect_flags.word & FIELD_PART_SPAWN_LINKED) && effect->state != FIELD_EFFECT_RETIRED)
            {
                new_effect_index = field_spawn_actor_effect(actor, part->rotation_extent.fields.linked_part & 0xF, 0);

                if (new_effect_index != -1)
                {
                    VECTOR segment;
                    VECTOR segment_normal;

                    new_effect = &g_field_effect_records[new_effect_index];
                    new_effect->x = effect->x;
                    new_effect->y = effect->y;
                    new_effect->z = effect->z;
                    segment.vx = effect->x - g_field_effect_records[effect->previous_effect_index].x;
                    segment.vy = effect->y - g_field_effect_records[effect->previous_effect_index].y;
                    segment.vz = effect->z - g_field_effect_records[effect->previous_effect_index].z;
                    g_field_effect_records[effect->previous_effect_index].next_effect_index = new_effect_index;

                    /* Orient the new link along the screen-space normal of the segment from the previous link. */
                    screen.vx = segment.vx >> 8;
                    screen.vy = (segment.vy >> 8) - (segment.vz >> 9);
                    segment.vz = 0;
                    segment.vx = screen.vy;
                    segment.vy = -screen.vx;
                    VectorNormal(&segment, &segment_normal);
                    new_effect->rotation_x = segment_normal.vx >> 6;
                    new_effect->heading = segment_normal.vy >> 6;
                    new_effect->pitch = segment_normal.vz >> 6;
                    new_effect->work_x = 0;
                    new_effect->work_y = 0;
                    new_effect->previous_effect_index = effect->previous_effect_index;
                    new_effect->next_effect_index = FIELD_EFFECT_NO_LINK;
                    effect->previous_effect_index = new_effect_index;
                }
            }
        }
    }

    if (actor->owner_object_index < FIELD_ACTOR_PALETTE_OWNERS)
    {
        palette_buffer = FIELD_ACTOR_CLUT_BUFFER(actor->owner_object_index);
    }
    else
    {
        palette_buffer = g_field_shared_clut_buffer;
    }

    animated_part_count = 0;

    for (part_index = 0; part_index < actor->part_count; part_index++)
    {
        palette_part = FIELD_ELEMENT_AT(FieldObjectPart, actor->parts, part_index);

        if (FIELD_PART_HAS_ANIMATED_PALETTE(palette_part))
        {
            field_interpolate_palette_track(actor, palette_part->palette_extent.fields.palette_track, palette_buffer,
                                            (s16*)(palette_buffer + FIELD_CLUT_BYTES));
            animated_part_count++;
        }
    }

    if (animated_part_count != 0)
    {
        if (actor->owner_object_index < FIELD_ACTOR_PALETTE_OWNERS)
        {
            palette_rect.x = 0;
            palette_rect.y = (actor->owner_object_index * 2) + FIELD_ACTOR_CLUT_VRAM_Y + FIELD_ANIMATED_CLUT_ROW;
            palette_rect.w = GPU_CLUT_4BIT_COLORS;
            palette_rect.h = 1;
        }
        else
        {
            palette_rect.x = 0;
            palette_rect.y = FIELD_SHARED_CLUT_VRAM_Y + FIELD_ANIMATED_CLUT_ROW;
            palette_rect.w = GPU_CLUT_4BIT_COLORS;
            palette_rect.h = 1;
        }

        LoadImage(&palette_rect, (u_long*)(palette_buffer + FIELD_CLUT_BYTES));
    }

    field_update_actor_palette_animation(actor);
    field_animate_mesh_textures(actor, actor->parts, actor->part_count);
}

/**
 * @brief Retire an effect, spawn configured follow-up effects, and synchronize actor state.
 * @param effect Effect record being retired.
 * @param part Part definition controlling follow-up effects and synchronization.
 */
void field_retire_effect(FieldMotionRecord* effect, FieldObjectPart* part)
{
    FieldActorSlot* actor;
    FieldObjectPart* spawn_part;
    FieldMotionRecord* new_effect;
    FieldMotionRecord* spawned_effect;
    FieldMotionRecord* effect_pool;
    FieldObjectState* object;
    FieldObjectState* object_table;
    VECTOR target_position;
    VECTOR direction;
    VECTOR squared_direction;
    s32 spawn_slot, spawn_mask;
    s32 new_effect_index;
    s32 spawn_count;
    s32 placement_kind;
    s16 animation_state;
    u8 next_effect_index;

    g_field_actor_slots[effect->owner_slot].active_counts[effect->track_index][part->part_index]--;

    actor = &g_field_actor_slots[effect->owner_slot];
    field_dispatch_actor_audio_event(actor, FIELD_SOUND_EVENT_EFFECT_RETIRED, effect->owner_part);

    if (part->appearance.word & 0xF0000000)
    {
        if (effect->height_or_retired_state == -1)
        {
            effect->state = FIELD_EFFECT_DISABLED;
        }
        else
        {
            effect->state = effect->height_or_retired_state;
        }

        effect_pool = g_field_effect_records;

        for (spawn_slot = 0, spawn_mask = 1; spawn_slot < 4; spawn_slot++, spawn_mask <<= 1)
        {
            if ((part->appearance.word >> 0x1C) & spawn_mask)
            {
                g_field_effect_spawn_point.vx = effect->x;
                g_field_effect_spawn_point.vy = effect->y;
                g_field_spawn_ground_point = 0;
                g_field_effect_spawn_point.vz = effect->z;

                spawn_part = &g_field_actor_slots[effect->owner_slot].parts[(part->spawn_flags.halves.part_selectors >> (spawn_slot * 4)) & 0xF];
                spawn_count = 1;
                placement_kind = 0x35;

                if (FIELD_PART_PLACEMENT_KIND(spawn_part) == placement_kind)
                {
                    if (spawn_part->effect_count != 0)
                    {
                        spawn_count = spawn_part->effect_count;
                    }
                }

                for (; spawn_count != 0; spawn_count--)
                {
                    new_effect_index = field_spawn_actor_effect(&g_field_actor_slots[effect->owner_slot],
                                                                (part->spawn_flags.halves.part_selectors >> (spawn_slot * 4)) & 0xF, 0);
                    if (new_effect_index != -1)
                    {
                        spawned_effect = FIELD_ELEMENT_AT(FieldMotionRecord, effect_pool, new_effect_index);

                        if (!(((u8*)&spawned_effect->flags)[3] & 7) && (spawned_effect->color_position.fields.position_source != FIELD_POSITION_NONE))
                        {
                            new_effect = spawned_effect;
                            field_resolve_effect_position(new_effect, part, &target_position);
                            direction.vx = (target_position.vx - new_effect->x) >> 8;
                            direction.vy = (target_position.vy - new_effect->y) >> 8;
                            direction.vz = (target_position.vz - new_effect->z) >> 8;

                            gte_ldlvl(&direction);
                            gte_sqr0();
                            gte_stlvnl(&squared_direction);

                            new_effect->heading = ratan2(-direction.vz, direction.vx);

                            if (new_effect->heading < 0)
                            {
                                new_effect->heading += FIELD_ANGLE_TURN;
                            }

                            if (direction.vy != 0)
                            {
                                new_effect->pitch = ratan2(SquareRoot0(squared_direction.vx + squared_direction.vz), -direction.vy);
                            }
                            else
                            {
                                new_effect->pitch = FIELD_ANGLE_QUARTER_TURN;
                            }

                            if (new_effect->pitch < 0)
                            {
                                new_effect->pitch += FIELD_ANGLE_TURN;
                            }

                            new_effect->rotation_x = 0;
                        }
                    }
                }
            }
        }
    }

    effect->state = FIELD_EFFECT_RETIRED;

    if (((actor->animation->sync_flags & FIELD_ANIM_TARGET_PART_HIDES) == FIELD_ANIM_TARGET_PART_HIDES) &&
        ((actor->animation->sync_parts >> 0xD) == effect->owner_part))
    {
        if (actor->targets[effect->track_index] != FIELD_TARGET_NONE)
        {
            object_table = g_field_object_states;
            object = &object_table[actor->targets[effect->track_index]];

            if (object->contact.bytes.controller_index == actor->slot_index)
            {
                animation_state = g_field_actors[actor->targets[effect->track_index]].command;

                if ((animation_state != FIELD_ACTOR_COMMAND_DEFEATED && animation_state != FIELD_ACTOR_COMMAND_DEFEAT_END) ||
                    (object->flags & FIELD_OBJECT_FLAG_KNOCKED_OUT))
                {
                    g_field_actors[actor->targets[effect->track_index]].presence = FIELD_ACTOR_VISIBLE;
                }
                else
                {
                    g_field_actors[actor->targets[effect->track_index]].presence = FIELD_ACTOR_HIDDEN;
                }

                g_field_object_states[actor->targets[effect->track_index]].contact.word &= ~FIELD_CONTACT_ANIMATION_HIDDEN;
            }
        }
    }

    if (((actor->animation->sync_flags & FIELD_ANIM_OWNER_PART_HIDES) == FIELD_ANIM_OWNER_PART_HIDES) &&
        (((actor->animation->sync_parts >> 0xA) & 7) == effect->owner_part))
    {
        object = &g_field_object_states[actor->owner_object_index];

        if (object->contact.bytes.controller_index == actor->slot_index)
        {
            animation_state = g_field_actors[actor->owner_object_index].command;

            if ((animation_state != FIELD_ACTOR_COMMAND_DEFEATED && animation_state != FIELD_ACTOR_COMMAND_DEFEAT_END) ||
                (object->flags & FIELD_OBJECT_FLAG_KNOCKED_OUT))
            {
                g_field_actors[actor->owner_object_index].presence = FIELD_ACTOR_VISIBLE;
            }
            else
            {
                g_field_actors[actor->owner_object_index].presence = FIELD_ACTOR_HIDDEN;
            }

            g_field_object_states[actor->owner_object_index].contact.word &= ~FIELD_CONTACT_ANIMATION_HIDDEN;
        }
    }

    if (effect->owner_part == ((actor->animation->sync_parts & 0x1F) - 1))
    {
        g_field_actors[actor->owner_object_index].x = effect->x;
        g_field_actors[actor->owner_object_index].y = effect->y;
        g_field_actors[actor->owner_object_index].z = effect->z;
        g_field_actors[actor->owner_object_index].y = 0;
        g_field_actors[actor->owner_object_index].animation =
            (g_field_actors[actor->owner_object_index].animation & FIELD_ANIMATION_INDEX_MASK) | (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED);
    }

    if (effect->owner_part == (((actor->animation->sync_parts >> 5) & 0x1F) - 1))
    {
        if (actor->targets[effect->track_index] != FIELD_TARGET_NONE)
        {
            g_field_actors[actor->targets[effect->track_index]].x = effect->x;
            g_field_actors[actor->targets[effect->track_index]].y = effect->y;
            g_field_actors[actor->targets[effect->track_index]].z = effect->z;
            g_field_actors[actor->targets[effect->track_index]].y = 0;
            g_field_actors[actor->targets[effect->track_index]].animation =
                (g_field_actors[actor->targets[effect->track_index]].animation & FIELD_ANIMATION_INDEX_MASK) |
                (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED);
        }
    }

    if (effect->height_or_retired_state == 5)
    {
        next_effect_index = effect->next_effect_index;

        if (next_effect_index != FIELD_EFFECT_NO_LINK)
        {
            g_field_effect_records[effect->next_effect_index].previous_effect_index = FIELD_EFFECT_NO_LINK;
        }
    }
}

/**
 * @brief Advance an actor-owned effect's placement, motion, pickups, and hit contacts.
 * @param record Active effect record.
 * @param part Part definition selecting parameter tracks and placement behavior.
 * @param actor Actor that owns the effect.
 * @see decomp.me (100%) https://decomp.me/scratch/i1ZHZ
 */
static void field_update_effect_record(FieldMotionRecord* record, FieldObjectPart* part, FieldActorSlot* actor)
{
    VECTOR* target_position;
    FieldEffectCamera* camera;
    FieldEffectMapBounds* map_bounds;
    FieldCollisionMover* mover;
    FieldCollisionQuery* query;
    VECTOR* work_vector;
    SVECTOR* local_vector;
    SVECTOR* rotated_vector;
    MATRIX* rotation;
    VECTOR* placement_origin;
    FieldMotionRecord* reference_record;
    FieldObjectState* reference_object;
    u32 flags;
    u32 record_flags;
    s32 selector;
    s32 x, y, z;
    s32 offset_or_angle;
    s32 current_angle;
    s32 dx, dy;
    struct FieldCollisionNode* initial_surface;
    s32 state_or_delta;
    s32 flags_byte;
    s32 slot;
    s32 recipient_index;
    u8 reference_state;

    camera = (FieldEffectCamera*)SCENE_STATE_ADDRESS;
    map_bounds = (FieldEffectMapBounds*)MAP_BOUNDS_ADDRESS;
    mover = FIELD_EFFECT_SCRATCH_MOVER;
    query = FIELD_EFFECT_SCRATCH_QUERY;
    work_vector = FIELD_EFFECT_SCRATCH_VECTOR;
    target_position = FIELD_EFFECT_SCRATCH_TARGET;
    local_vector = FIELD_EFFECT_SCRATCH_LOCAL_VECTOR;
    rotated_vector = FIELD_EFFECT_SCRATCH_ROTATED_VECTOR;
    rotation = FIELD_EFFECT_SCRATCH_ROTATION;
    placement_origin = FIELD_EFFECT_SCRATCH_ORIGIN;

    /* Sample transparency and motion tracks; the caller advances age afterward. */
    flags = part->effect_flags.word;

    if (flags & FIELD_EFFECT_SEMITRANSPARENT)
    {
        s32 semitransparent =
            field_evaluate_parameter_track_at_time(actor, (flags >> FIELD_PART_SEMITRANSPARENT_TRACK_SHIFT) & FIELD_TRACK_INDEX_MASK, (u16)record->age) != 0;
        record->flags = (record->flags & ~FIELD_EFFECT_SEMITRANSPARENT) | (semitransparent << FIELD_EFFECT_SEMITRANSPARENT_SHIFT);
    }

    if ((record->flags & FIELD_EFFECT_PARAMETER_MODE_MASK) == FIELD_EFFECT_PARAMETER_MODE_TIMED_TRACK)
    {
        record->motion_parameter =
            field_evaluate_parameter_track_at_time(actor, part->orientation_flags.halves.high & FIELD_TRACK_INDEX_MASK, ((u16)record->age));
    }

    /* Linked segment records update their projected work vector and return early. */
    if (record->state == 5)
    {
        field_build_effect_part_matrix(record, part, rotation, actor);
        gte_SetRotMatrix(rotation);
        {
            u16 segment_x = record->heading;
            local_vector->vy = 0;
            local_vector->vx = segment_x;
        }

        local_vector->vz = record->rotation_x;
        gte_ldv0(local_vector);
        gte_rtv0();
        gte_stlvnl(work_vector);
        record->work_x = work_vector->vx;
        record->work_y = work_vector->vy;
        return;
    }

    /* Path mode advances a wrapping byte parameter and interpolates world X/Z. */
    record_flags = record->flags;

    if ((record_flags & FIELD_EFFECT_MOTION_KIND_MASK) == FIELD_EFFECT_MOTION_PATH)
    {
        record->position_data.path_time = record->position_data.path_time + record->motion_parameter;
        field_update_path_position(&record->position_data.path_time, record, record->path_group);
    }

    /* Attached effects rebuild an origin, then add the rotated local displacement. */
    else if (((record_flags >> 0x18) & 7) >= 2)
    {
        if (((part->placement_flags.word >> 0x1A) & 3) == 2)
        {
            record->rotation_z_16 =
                field_evaluate_parameter_track_at_time(actor, part->rotation_extent.fields.rotation_z_track & FIELD_TRACK_INDEX_MASK, ((u16)record->age));
        }

        if (((part->placement_flags.word >> 0x1C) & 3) == 2)
        {
            record->rotation_y_16 =
                field_evaluate_parameter_track_at_time(actor, part->rotation_extent.fields.rotation_y_track & FIELD_TRACK_INDEX_MASK, ((u16)record->age));
        }

        if ((record->flags & 0x600) == 0x400)
        {
            u32 selector_high = part->palette_extent.word >> 29;
            local_vector->vy = -field_evaluate_parameter_track_at_time(
                actor, ((((part->rotation_extent.word) & 0x3F) * 8) | selector_high) & FIELD_TRACK_INDEX_MASK, (u16)record->age);
        }
        else
        {
            local_vector->vy = -((u16)record->flags & FIELD_EFFECT_DISTANCE_MASK);
        }

        if ((part->palette_extent.word) & 0x01000000)
        {
            local_vector->vy =
                (s16)((g_field_object_states[actor->owner_object_index].movement.half.flags & FIELD_MOVEMENT_SCALE_MASK) * (s16)(u16)local_vector->vy / 100);
        }

        {
            u32 bounds_flags;
            s32 placement_kind;
            bounds_flags = part->placement_flags.word;
            placement_kind = (bounds_flags >> FIELD_PART_PLACEMENT_KIND_SHIFT) & FIELD_PART_PLACEMENT_KIND_MASK;

            if (placement_kind < 0x14)
            {
                if ((bounds_flags >> 0x10) & 1)
                {
                    s32 previous_distance = local_vector->vy;

                    if (placement_kind < FIELD_PLACEMENT_TRACK_BOUNDS || placement_kind >= FIELD_PLACEMENT_NONE)
                    {
                        dy = (g_field_object_states[actor->owner_object_index].bounds.half.right -
                              g_field_object_states[actor->owner_object_index].bounds.half.left) >>
                             1;
                    }
                    else
                    {
                        dy = (g_field_object_states[actor->targets[g_field_track_index]].bounds.half.right -
                              g_field_object_states[actor->targets[g_field_track_index]].bounds.half.left) >>
                             1;
                    }

                    dy = abs(dy);
                    local_vector->vy = previous_distance - dy;
                    bounds_flags = part->placement_flags.word;
                }

                if ((bounds_flags >> 0x11) & 1)
                {
                    s32 previous_distance = local_vector->vy;

                    if (((bounds_flags >> FIELD_PART_PLACEMENT_KIND_SHIFT) & FIELD_PART_PLACEMENT_KIND_MASK) < FIELD_PLACEMENT_TRACK_BOUNDS ||
                        ((bounds_flags >> FIELD_PART_PLACEMENT_KIND_SHIFT) & FIELD_PART_PLACEMENT_KIND_MASK) >= FIELD_PLACEMENT_NONE)
                    {
                        dy = (g_field_object_states[actor->owner_object_index].bounds.half.bottom -
                              g_field_object_states[actor->owner_object_index].bounds.half.top) >>
                             1;
                    }
                    else
                    {
                        dy = (g_field_object_states[actor->targets[g_field_track_index]].bounds.half.bottom -
                              g_field_object_states[actor->targets[g_field_track_index]].bounds.half.top) >>
                             1;
                    }

                    dy = abs(dy);
                    local_vector->vy = previous_distance - dy;
                }
            }
        }

        local_vector->vx = 0;
        local_vector->vz = 0;
        record->heading = record->heading + record->motion_parameter;
        RotMatrix_gte((SVECTOR*)&record->rotation_x, rotation);
        RotMatrixZ(record->rotation_z_16 * 0x10, rotation);
        RotMatrixY(record->rotation_y_16 * 0x10, rotation);
        gte_SetRotMatrix(rotation);
        gte_ldv0(local_vector);
        gte_rtv0();
        gte_stsv(rotated_vector);

        /* 0..9 select owner bounds; 10..19 select the current track object bounds. */
        selector = ((FieldPlacementBits*)&part->placement_flags.word)->opcode;

        switch (selector)
        {
            case FIELD_PLACEMENT_OWNER_BOUNDS:
            case FIELD_PLACEMENT_OWNER_BOUNDS + 1:
            case FIELD_PLACEMENT_OWNER_BOUNDS + 2:
            case FIELD_PLACEMENT_OWNER_BOUNDS + 3:
            case FIELD_PLACEMENT_OWNER_BOUNDS + 4:
            case FIELD_PLACEMENT_OWNER_BOUNDS + 5:
            case FIELD_PLACEMENT_OWNER_BOUNDS + 6:
            case FIELD_PLACEMENT_OWNER_BOUNDS + 7:
            case FIELD_PLACEMENT_OWNER_BOUNDS + 8:
            case FIELD_PLACEMENT_OWNER_BOUNDS + 9:
            case FIELD_PLACEMENT_TRACK_BOUNDS:
            case FIELD_PLACEMENT_TRACK_BOUNDS + 1:
            case FIELD_PLACEMENT_TRACK_BOUNDS + 2:
            case FIELD_PLACEMENT_TRACK_BOUNDS + 3:
            case FIELD_PLACEMENT_TRACK_BOUNDS + 4:
            case FIELD_PLACEMENT_TRACK_BOUNDS + 5:
            case FIELD_PLACEMENT_TRACK_BOUNDS + 6:
            case FIELD_PLACEMENT_TRACK_BOUNDS + 7:
            case FIELD_PLACEMENT_TRACK_BOUNDS + 8:
            case FIELD_PLACEMENT_TRACK_BOUNDS + 9:
            {
                s32 offset_y;
                FieldObjectState* owner;
                FieldObjectState* owner_base;

                if ((s32)selector >= FIELD_PLACEMENT_TRACK_BOUNDS)
                {
                    slot = actor->targets[g_field_track_index];
                    selector -= FIELD_PLACEMENT_TRACK_BOUNDS;
                    reference_record = &FIELD_ACTOR_MOTION_RECORDS[slot];
                    reference_object = &g_field_object_states[slot];
                }
                else
                {
                    slot = actor->owner_object_index;
                    reference_record = &FIELD_ACTOR_MOTION_RECORDS[slot];
                    reference_object = &g_field_object_states[slot];
                }

                owner_base = g_field_object_states;
                owner = &owner_base[actor->owner_object_index];
                flags_byte = owner->contact.bytes.flags;

                if ((flags_byte & 1) && ((u8)actor->slot_index >= 0x40U))
                {
                    if (!(((u32)owner->contact.word >> FIELD_CONTACT_NO_HIT_TEST_BIT) & 1))
                    {
                        offset_y = 0x800000;
                        offset_or_angle = offset_y;
                        selector = -1;
                    }
                    else
                    {
                        offset_y = 0;
                        offset_or_angle = offset_y;
                    }
                }
                else
                {
                    offset_y = 0;
                    offset_or_angle = offset_y;
                }

                switch (selector)
                {
                    case FIELD_BOUNDS_POINT_CENTER:
                        offset_or_angle = (reference_object->bounds.half.right + reference_object->bounds.half.left) >> 1;
                        offset_y = (reference_object->bounds.half.bottom + reference_object->bounds.half.top) >> 1;
                        break;
                    case FIELD_BOUNDS_POINT_CENTER_X:
                        offset_y = 0;
                        offset_or_angle = (reference_object->bounds.half.right + reference_object->bounds.half.left) >> 1;
                        break;
                    case FIELD_BOUNDS_POINT_TOP_CENTER:
                        offset_y = reference_object->bounds.half.top;
                        offset_or_angle = (reference_object->bounds.half.right + reference_object->bounds.half.left) >> 1;
                        break;
                    case FIELD_BOUNDS_POINT_LEFT_CENTER:
                        offset_or_angle = reference_object->bounds.half.left;
                        offset_y = (reference_object->bounds.half.bottom + reference_object->bounds.half.top) >> 1;
                        break;
                    case FIELD_BOUNDS_POINT_RIGHT_CENTER:
                        offset_or_angle = reference_object->bounds.half.right;
                        offset_y = (reference_object->bounds.half.bottom + reference_object->bounds.half.top) >> 1;
                        break;
                    case FIELD_BOUNDS_POINT_TOP_LEFT:
                        offset_or_angle = reference_object->bounds.half.left;
                        offset_y = reference_object->bounds.half.top;
                        break;
                    case FIELD_BOUNDS_POINT_TOP_RIGHT:
                        offset_or_angle = reference_object->bounds.half.right;
                        offset_y = reference_object->bounds.half.top;
                        break;
                    case FIELD_BOUNDS_POINT_BOTTOM_LEFT:
                        offset_or_angle = reference_object->bounds.half.left;
                        offset_y = reference_object->bounds.half.bottom;
                        break;
                    case FIELD_BOUNDS_POINT_BOTTOM_RIGHT:
                        offset_or_angle = reference_object->bounds.half.right;
                        offset_y = reference_object->bounds.half.bottom;
                        break;
                }

                offset_or_angle <<= 8;
                placement_origin->vx = reference_record->x + offset_or_angle;
                offset_y <<= 8;
                placement_origin->vy = reference_record->y + offset_y;
                placement_origin->vz = reference_record->z;
                break;
            }
            case FIELD_PLACEMENT_SIBLING:
            case FIELD_PLACEMENT_SIBLING + 1:
            case FIELD_PLACEMENT_SIBLING + 2:
            case FIELD_PLACEMENT_SIBLING + 3:
            case FIELD_PLACEMENT_SIBLING + 4:
            case FIELD_PLACEMENT_SIBLING + 5:
            case FIELD_PLACEMENT_SIBLING + 6:
            case FIELD_PLACEMENT_SIBLING + 7:
            case FIELD_PLACEMENT_SIBLING_HIGH:
            case FIELD_PLACEMENT_SIBLING_HIGH + 1:
            case FIELD_PLACEMENT_SIBLING_HIGH + 2:
            case FIELD_PLACEMENT_SIBLING_HIGH + 3:
            case FIELD_PLACEMENT_SIBLING_HIGH + 4:
            case FIELD_PLACEMENT_SIBLING_HIGH + 5:
            case FIELD_PLACEMENT_SIBLING_HIGH + 6:
            case FIELD_PLACEMENT_SIBLING_HIGH + 7:
            {
                FieldMotionRecord* effect;
                FieldMotionRecord* effect_base;
                effect_base = g_field_effect_records;
                effect = &effect_base[(u16)record->reference_index];
                reference_state = effect->state;

                if (reference_state != FIELD_EFFECT_RETIRED)
                {
                    placement_origin->vx = effect->x;
                    placement_origin->vy = g_field_effect_records[((u16)record->reference_index)].y;
                    placement_origin->vz = g_field_effect_records[((u16)record->reference_index)].z;

                    if (!(part->orientation_flags.word & FIELD_EFFECT_ORIENTATION_LOCK))
                    {
                        RotMatrix_gte((SVECTOR*)&g_field_effect_records[((u16)record->reference_index)].rotation_x, rotation);
                        gte_SetRotMatrix(rotation);
                        gte_ldv0(rotated_vector);
                        gte_rtv0();
                        gte_stsv(local_vector);
                        rotated_vector->vx = local_vector->vx;
                        rotated_vector->vy = local_vector->vy;
                        rotated_vector->vz = local_vector->vz;
                    }

                    break;
                }

                record->state = reference_state;
                return;
            }
            case FIELD_PLACEMENT_FIELD_POSITION:
                placement_origin->vx = (part->offset_x << 8) - g_field_view_offset_x;
                placement_origin->vy = (part->offset_y << 8) - g_field_view_offset_y;
                placement_origin->vz = (part->offset_z << 8) - g_field_view_offset_z;
                break;
            case FIELD_PLACEMENT_SCREEN_TOP:
                placement_origin->vx = 0;
                placement_origin->vy = -FIELD_SCREEN_HALF_HEIGHT_FIXED;
                placement_origin->vz = 0;
                break;
            case FIELD_PLACEMENT_SCREEN_BOTTOM:
                placement_origin->vx = 0;
                placement_origin->vy = FIELD_SCREEN_HALF_HEIGHT_FIXED;
                placement_origin->vz = 0;
                break;
            case FIELD_PLACEMENT_SCREEN_LEFT:
                placement_origin->vx = -FIELD_SCREEN_HALF_WIDTH_FIXED;
                placement_origin->vy = 0;
                placement_origin->vz = 0;
                break;
            case FIELD_PLACEMENT_SCREEN_RIGHT:
                placement_origin->vx = FIELD_SCREEN_HALF_WIDTH_FIXED;
                placement_origin->vy = 0;
                placement_origin->vz = 0;
                break;
            case FIELD_PLACEMENT_SCREEN_TOP_LEFT:
                placement_origin->vx = -FIELD_SCREEN_HALF_WIDTH_FIXED;
                placement_origin->vy = -FIELD_SCREEN_HALF_HEIGHT_FIXED;
                placement_origin->vz = 0;
                break;
            case FIELD_PLACEMENT_SCREEN_TOP_RIGHT:
                placement_origin->vx = FIELD_SCREEN_HALF_WIDTH_FIXED;
                placement_origin->vy = -FIELD_SCREEN_HALF_HEIGHT_FIXED;
                placement_origin->vz = 0;
                break;
            case FIELD_PLACEMENT_SCREEN_BOTTOM_LEFT:
                placement_origin->vx = -FIELD_SCREEN_HALF_WIDTH_FIXED;
                placement_origin->vy = FIELD_SCREEN_HALF_HEIGHT_FIXED;
                placement_origin->vz = 0;
                break;
            case FIELD_PLACEMENT_SCREEN_BOTTOM_RIGHT:
                placement_origin->vx = FIELD_SCREEN_HALF_WIDTH_FIXED;
                placement_origin->vy = FIELD_SCREEN_HALF_HEIGHT_FIXED;
                placement_origin->vz = 0;
                break;
            case FIELD_PLACEMENT_OWNER_OFFSET:
                reference_record = &FIELD_ACTOR_MOTION_RECORDS[actor->owner_object_index];

                if (((part->placement_flags.word >> FIELD_PART_MIRROR_X_WITH_FACING_SHIFT) & 1) &&
                    !(reference_record->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED))
                {
                    placement_origin->vx = reference_record->x - (part->offset_x << 8);
                }
                else
                {
                    placement_origin->vx = reference_record->x + (part->offset_x << 8);
                }

                placement_origin->vy = reference_record->y + (part->offset_y << 8);
                placement_origin->vz = reference_record->z + (part->offset_z << 8);
                break;
            case FIELD_PLACEMENT_TRACK_OFFSET:
                reference_record = &FIELD_ACTOR_MOTION_RECORDS[actor->targets[g_field_track_index]];

                if ((part->spawn_flags.word & FIELD_PART_MIRROR_X_WITH_OWNER) &&
                    !(g_field_actors[actor->owner_object_index].animation & FIELD_EFFECT_FACING_FLIPPED))
                {
                    placement_origin->vx = reference_record->x - (part->offset_x << 8);
                }
                else if (((part->placement_flags.word >> FIELD_PART_MIRROR_X_WITH_FACING_SHIFT) & 1) &&
                         !(reference_record->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED))
                {
                    placement_origin->vx = reference_record->x - (part->offset_x << 8);
                }
                else
                {
                    placement_origin->vx = reference_record->x + (part->offset_x << 8);
                }

                placement_origin->vy = reference_record->y + (part->offset_y << 8);
                placement_origin->vz = reference_record->z + (part->offset_z << 8);
                break;
            case FIELD_PLACEMENT_OWNER_ATTACHMENT:
                reference_record = &FIELD_ACTOR_MOTION_RECORDS[actor->owner_object_index];
                reference_object = &g_field_object_states[actor->owner_object_index];
                placement_origin->vx = reference_record->x;
                placement_origin->vy =
                    reference_record->y +
                    (reference_object->attachment_points[(part->effect_flags.word >> FIELD_PART_ATTACHMENT_INDEX_SHIFT) & FIELD_PART_ATTACHMENT_INDEX_MASK].y
                     << 8);
                placement_origin->vz = reference_record->z;
                placement_origin->vx +=
                    reference_object->attachment_points[(part->effect_flags.word >> FIELD_PART_ATTACHMENT_INDEX_SHIFT) & FIELD_PART_ATTACHMENT_INDEX_MASK].x
                    << 8;
                break;
            case FIELD_PLACEMENT_OWNER_ATTACHMENT_X:
                reference_record = &FIELD_ACTOR_MOTION_RECORDS[actor->owner_object_index];
                reference_object = &g_field_object_states[actor->owner_object_index];
                placement_origin->vx =
                    reference_record->x +
                    (reference_object->attachment_points[(part->effect_flags.word >> FIELD_PART_ATTACHMENT_INDEX_SHIFT) & FIELD_PART_ATTACHMENT_INDEX_MASK].x
                     << 8);
                placement_origin->vy = reference_record->y;
                placement_origin->vz = reference_record->z + (part->offset_z << 8);

                if (((part->placement_flags.word >> FIELD_PART_MIRROR_X_WITH_FACING_SHIFT) & 1) &&
                    !(reference_record->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED))
                {
                    placement_origin->vx -= part->offset_x << 8;
                }
                else
                {
                    placement_origin->vx += part->offset_x << 8;
                }

                break;
            case FIELD_PLACEMENT_GROUND_POINT:
                reference_record = &FIELD_ACTOR_MOTION_RECORDS[actor->owner_object_index];
                reference_object = &g_field_object_states[actor->owner_object_index];
                placement_origin->vx = reference_record->x + (reference_object->ground_effect_points[((u32)record->flags >> 0xD) & 3].x << 8);
                placement_origin->vy = reference_record->y;
                placement_origin->vz = reference_record->z + (reference_object->ground_effect_points[((u32)record->flags >> 0xD) & 3].y << 8);
                break;
            case FIELD_PLACEMENT_TRACK_ACTION_OFFSET:
                reference_record = &FIELD_ACTOR_MOTION_RECORDS[actor->targets[g_field_track_index]];

                if (!(reference_record->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED))
                {
                    placement_origin->vx = reference_record->x + (actor->track_offsets[g_field_track_index].x << 8);
                }
                else
                {
                    placement_origin->vx = reference_record->x - (actor->track_offsets[g_field_track_index].x << 8);
                }

                placement_origin->vy = reference_record->y + (actor->track_offsets[g_field_track_index].y << 8);
                placement_origin->vz = reference_record->z;
                break;
            case FIELD_PLACEMENT_PART_OFFSET:
                placement_origin->vx = part->offset_x << 8;
                placement_origin->vy = part->offset_y << 8;
                placement_origin->vz = part->offset_z << 8;
                break;
            case FIELD_PLACEMENT_SIBLING_OFFSET:
            case FIELD_PLACEMENT_SIBLING_OFFSET + 1:
            case FIELD_PLACEMENT_SIBLING_OFFSET + 2:
            case FIELD_PLACEMENT_SIBLING_OFFSET + 3:
            case FIELD_PLACEMENT_SIBLING_OFFSET + 4:
            case FIELD_PLACEMENT_SIBLING_OFFSET + 5:
            case FIELD_PLACEMENT_SIBLING_OFFSET + 6:
            case FIELD_PLACEMENT_SIBLING_OFFSET + 7:
            {
                FieldMotionRecord* effect;
                FieldMotionRecord* effect_base;
                effect_base = g_field_effect_records;
                effect = &effect_base[(u16)record->reference_index];
                reference_state = effect->state;

                if (reference_state == FIELD_EFFECT_RETIRED)
                {
                    record->state = reference_state;
                    return;
                }

                placement_origin->vx = effect->x;
                placement_origin->vy = g_field_effect_records[(u16)record->reference_index].y;
                placement_origin->vz = g_field_effect_records[(u16)record->reference_index].z;

                if ((part->spawn_flags.word & FIELD_PART_MIRROR_X_WITH_OWNER) &&
                    !(g_field_actors[actor->owner_object_index].animation & FIELD_EFFECT_FACING_FLIPPED))
                {
                    placement_origin->vx -= part->offset_x << 8;
                }
                else if (((part->placement_flags.word >> FIELD_PART_MIRROR_X_WITH_FACING_SHIFT) & 1) &&
                         !(g_field_effect_records[(u16)record->reference_index].facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED))
                {
                    placement_origin->vx -= part->offset_x << 8;
                }
                else
                {
                    placement_origin->vx += part->offset_x << 8;
                }

                placement_origin->vy += part->offset_y << 8;
                placement_origin->vz += part->offset_z << 8;

                if (!(part->orientation_flags.word & FIELD_EFFECT_ORIENTATION_LOCK))
                {
                    RotMatrix_gte((SVECTOR*)&g_field_effect_records[(u16)record->reference_index].rotation_x, rotation);
                    gte_SetRotMatrix(rotation);
                    gte_ldv0(rotated_vector);
                    gte_rtv0();
                    gte_stsv(local_vector);
                    rotated_vector->vx = local_vector->vx;
                    rotated_vector->vy = local_vector->vy;
                    rotated_vector->vz = local_vector->vz;
                }

                break;
            }
            default:
                placement_origin->pad = 0;
                placement_origin->vz = 0;
                placement_origin->vy = 0;
                placement_origin->vx = 0;
                break;
            case FIELD_PLACEMENT_NONE:
                break;
        }

        record->x = ((s16)rotated_vector->vx << 8) + placement_origin->vx;
        record->y = (((s32)(rotated_vector->vy << 0x10)) >> 8) + placement_origin->vy;
        z = (((s32)(rotated_vector->vz << 0x10)) >> 8) + placement_origin->vz;
        record->z = z;

        if (part->placement_flags.word & FIELD_PART_DEPTH_STEP)
        {
            record->z = z + FIELD_WORLD_OT_DEPTH_STEP;
        }

        flags = part->placement_flags.word;

        if ((flags >> FIELD_PART_HEIGHT_TRACK_BIT) & 1)
        {
            if (part->spawn_flags.word & FIELD_PART_HEIGHT_FROM_BASE)
            {
                record->y =
                    (record->height_or_retired_state - field_evaluate_parameter_track_at_time(actor, (flags >> 4) & FIELD_TRACK_INDEX_MASK, ((u16)record->age)))
                    << 8;
            }
            else
            {
                record->y = -field_evaluate_parameter_track_at_time(actor, (flags >> 4) & FIELD_TRACK_INDEX_MASK, ((u16)record->age)) << 8;
            }
        }

        state_or_delta = ((u8*)&record->flags)[3] & 7;

        switch (state_or_delta)
        {
            case 3:
                record->y = record->y + (((u16)record->age) << 9);
                break;
            case 4:
                record->y = record->y - (((u16)record->age) << 9);
                break;
        }

        if (((part->behavior_flags.word >> FIELD_PART_PITCH_ACCELERATION_BIT) & 1) && ((record->pitch + part->pitch_acceleration) < FIELD_ANGLE_HALF_TURN))
        {
            record->pitch = (u16)record->pitch + part->pitch_acceleration;
        }
    }
    else
    {
        {
            u16 speed = record->motion_parameter;
            local_vector->vz = 0;
            local_vector->vx = 0;
            local_vector->vy = -speed << 2;
        }

        RotMatrix_gte((SVECTOR*)&record->rotation_x, rotation);

        if (!((part->behavior_flags.word >> FIELD_PART_PITCH_ACCELERATION_BIT) & 1) && (record->color_position.fields.position_source == FIELD_POSITION_NONE))
        {
            RotMatrixZ(record->rotation_z_16 * 0x10, rotation);
            RotMatrixY(record->rotation_y_16 * 0x10, rotation);
        }

        gte_SetRotMatrix(rotation);
        gte_ldv0(local_vector);
        gte_rtv0();
        gte_stsv(rotated_vector);

        /* Free movement uses a rotated step and a fresh collision probe each update; a detached
           mover skips the initial floor lookup, but ordinary movement collisions still run. */
        initial_surface = FIELD_COLLISION_NODE_DETACHED;

        if ((((record->flags & FIELD_EFFECT_PARAMETER_MODE_MASK) != FIELD_EFFECT_PARAMETER_MODE_TIMED_TRACK) || ((s16)record->motion_parameter != 0)) &&
            (((part->behavior_flags.word >> FIELD_PART_PITCH_ACCELERATION_BIT) & 1) || ((s32)part->placement_flags.word < 0)))
        {
            s32 pitch_cosine;
            s16 adjusted_pitch;

            pitch_cosine = rcos(record->pitch) - (part->pitch_acceleration * 8);
            adjusted_pitch = ratan2(rsin(record->pitch), pitch_cosine);
            record->pitch = adjusted_pitch;

            if (adjusted_pitch < FIELD_ANGLE_QUARTER_TURN)
            {
                record->motion_parameter = (((s16)record->motion_parameter * 0xF) >> 4) - 1;
            }
            else
            {
                record->motion_parameter = (((s16)record->motion_parameter << 5) / 30) + 1;
            }

            if (((s16)record->motion_parameter < 0x14) && (record->pitch < FIELD_ANGLE_QUARTER_TURN))
            {
                record->pitch = FIELD_ANGLE_HALF_TURN - (u16)record->pitch;
                record->motion_parameter = 0x14;
            }
        }

        if ((part->effect_flags.word & FIELD_PART_SOLID) && !(part->spawn_flags.word & FIELD_PART_IGNORE_MAP_COLLISION))
        {
            if (record->x < 0 || record->x >= (map_bounds->width << 8) || record->z < 0 || record->z >= (map_bounds->height << 9))
            {
                if (!(part->spawn_flags.word & FIELD_PART_RETIRE_ON_COLLISION))
                {
                    rotated_vector->vx = 0;
                    rotated_vector->vz = 0;
                }
                else
                {
                    record->state = FIELD_EFFECT_RETIRED;
                }
            }
            else
            {
                /* Advance the packed context during a collision attempt; protocol remains unresolved. */
                g_field_action_context += 0x100;
                mover->x = record->x;
                mover->height = 0;
                mover->z = record->z;
                mover->move_x = rotated_vector->vx;
                mover->move_height = 0;
                mover->move_z = rotated_vector->vz;
                mover->footprint_width = FIELD_FOOTPRINT_LARGE_WIDTH;
                mover->mode.bits.footprint_depth = FIELD_FOOTPRINT_LARGE_DEPTH;
                mover->height_bias = FIELD_FOOTPRINT_STEP;
                mover->collision_node = initial_surface;
                mover->flags = 0;
                mover->mode.bits.footprint_depth = FIELD_FOOTPRINT_LARGE_DEPTH;
                mover->mode.bits.airborne_high = 0;
                mover->mode.bits.airborne_low = 0;

                if (field_collision_resolve_move(mover) & (FIELD_COLLISION_RESULT_BLOCKED | FIELD_COLLISION_RESULT_SLID))
                {
                    rotated_vector->vz = 0;
                    rotated_vector->vx = 0;

                    if (part->spawn_flags.word & FIELD_PART_RETIRE_ON_COLLISION)
                    {
                        record->state = FIELD_EFFECT_RETIRED;
                    }
                }
                else if (!(part->spawn_flags.word & FIELD_PART_RETIRE_ON_COLLISION))
                {
                    query->width = FIELD_FOOTPRINT_LARGE_WIDTH;
                    query->height_tolerance = FIELD_FOOTPRINT_STEP;
                    query->depth = FIELD_FOOTPRINT_LARGE_DEPTH;
                    query->x = mover->x;
                    query->y = record->y;
                    query->z = mover->z;

                    if ((g_field_active_group != 0) && (field_collision_hit_markers(query) != FIELD_COLLISION_MARKER_NONE))
                    {
                        rotated_vector->vz = 0;
                        rotated_vector->vx = 0;
                    }
                    else
                    {
                        rotated_vector->vx = (u16)mover->x - (u16)record->x;
                        rotated_vector->vz = (u16)mover->z - (u16)record->z;
                    }
                }
            }
        }

        if (part->spawn_flags.word & FIELD_PART_CAMERA_BOUNDS)
        {
            s32 camera_x;
            s32 next_z;
            dx = record->x + (s16)rotated_vector->vx;
            camera_x = -camera->x;

            if (((camera_x + FIELD_EFFECT_CAMERA_X_MIN) < dx) && (dx < (camera_x + FIELD_EFFECT_CAMERA_X_MAX)))
            {
                next_z = record->z + (s16)rotated_vector->vz;
                x = -camera->z;

                if ((x < next_z) && (next_z < (x + FIELD_EFFECT_CAMERA_Z_SPAN)))
                {
                    record->x = dx;
                    record->z = record->z + (s16)rotated_vector->vz;
                }
            }
        }
        else
        {
            record->x = record->x + (s16)rotated_vector->vx;
            record->z = record->z + (s16)rotated_vector->vz;
        }

        y = record->y + (s16)rotated_vector->vy;
        record->y = y;

        if ((part->spawn_flags.word & FIELD_PART_GROUND_STOP) && y >= 0 &&
            ((record->flags & FIELD_EFFECT_PARAMETER_MODE_MASK) == 0 ||
             (record->flags & FIELD_EFFECT_PARAMETER_MODE_MASK) == FIELD_EFFECT_PARAMETER_MODE_TIMED_TRACK))
        {
            record->flags &= ~FIELD_EFFECT_PARAMETER_MODE_MASK;
            record->y = 0;
            record->motion_parameter = 0;
        }

        flags = part->placement_flags.word;

        if ((flags >> FIELD_PART_HEIGHT_TRACK_BIT) & 1)
        {
            if (part->spawn_flags.word & FIELD_PART_HEIGHT_FROM_BASE)
            {
                record->y =
                    (record->height_or_retired_state - field_evaluate_parameter_track_at_time(actor, (flags >> 4) & FIELD_TRACK_INDEX_MASK, ((u16)record->age)))
                    << 8;
            }
            else
            {
                record->y = -field_evaluate_parameter_track_at_time(actor, (flags >> 4) & FIELD_TRACK_INDEX_MASK, ((u16)record->age)) << 8;
            }
        }

        /* Resolve a target and either stop nearby or steer the heading and pitch. */
        if (record->color_position.fields.position_source != FIELD_POSITION_NONE)
        {
            s32 target_dy, target_dz;

            field_resolve_effect_position(record, part, target_position);
            work_vector->vx = (record->x - target_position->vx) >> 8;
            work_vector->vy = (record->y - target_position->vy) >> 8;
            target_dz = (record->z - target_position->vz) >> 8;
            work_vector->vz = target_dz;

            if ((work_vector->vx >= -0xF) && (work_vector->vx < 0x10) && (target_dz >= -0xF) && (target_dz < 0x10))
            {
                target_dy = work_vector->vy;

                if ((target_dy >= -0xF) && (work_vector->vy < 0x10))
                {
                    if (FIELD_PART_END_MODE(part) == FIELD_PART_END_AT_TARGET)
                    {
                        FIELD_EFFECT_RETIRE(record);
                        return;
                    }

                    if (!(part->orientation_flags.word & FIELD_EFFECT_ORIENTATION_LOCK))
                    {
                        record->motion_parameter = 0;
                        record->flags = record->flags & ~FIELD_EFFECT_PARAMETER_MODE_MASK;
                    }
                }
            }

            if (!(record->flags & FIELD_EFFECT_MOTION_KIND_MASK) && !(part->orientation_flags.word & FIELD_EFFECT_ORIENTATION_LOCK))
            {
                VECTOR new_pos;
                VECTOR delta;
                VECTOR delta_squared;
                s16 target_heading;

                field_resolve_effect_position(record, part, &new_pos);
                delta.vx = (new_pos.vx - record->x) >> 8;
                delta.vy = (new_pos.vy - record->y) >> 8;
                delta.vz = (new_pos.vz - record->z) >> 8;
                gte_ldlvl(&delta);
                gte_sqr0();
                gte_stlvnl(&delta_squared);
                target_heading = ratan2(-delta.vz, delta.vx);
                record->heading = target_heading;

                if (target_heading < 0)
                {
                    record->heading = target_heading + FIELD_ANGLE_TURN;
                }

                if (delta.vy != 0)
                {
                    record->pitch = ratan2(SquareRoot0(delta_squared.vx + delta_squared.vz), -delta.vy);
                }
                else
                {
                    record->pitch = FIELD_ANGLE_QUARTER_TURN;
                }

                if (record->pitch < 0)
                {
                    record->pitch = (u16)record->pitch + FIELD_ANGLE_TURN;
                }

                record->rotation_x = 0;
            }

            if ((record->flags & FIELD_EFFECT_MOTION_KIND_MASK) == FIELD_EFFECT_MOTION_HOMING)
            {

                s16 next_heading;

                gte_ldlvl(work_vector);
                gte_sqr0();
                gte_stlvnl(target_position);
                offset_or_angle = ratan2(work_vector->vz, -work_vector->vx);

                if (offset_or_angle < 0)
                {
                    offset_or_angle += FIELD_ANGLE_TURN;
                }

                if (((u16)record->age) == part->turn_end_age)
                {
                    record->flags = record->flags & ~FIELD_EFFECT_MOTION_KIND_MASK;

                    if (!(part->orientation_flags.word & FIELD_EFFECT_ORIENTATION_LOCK))
                    {
                        record->heading = (u16)offset_or_angle;

                        if (work_vector->vz != 0)
                        {
                            record->pitch = ratan2(SquareRoot0(target_position->vx + target_position->vz), work_vector->vy);
                        }
                        else
                        {
                            record->pitch = FIELD_ANGLE_QUARTER_TURN;
                        }

                        if (record->pitch < 0)
                        {
                            record->pitch = (u16)record->pitch + FIELD_ANGLE_TURN;
                        }
                    }
                }
                else
                {
                    current_angle = (s16)record->heading;
                    selector = (offset_or_angle - current_angle) & FIELD_ANGLE_MASK;

                    if (selector < 8 || selector > 0xFF8)
                    {
                        record->heading = (u16)offset_or_angle;
                    }
                    else
                    {
                        if (selector >= 0x801)
                        {
                            s32 reverse_turn = FIELD_ANGLE_TURN - selector;

                            if (reverse_turn < FIELD_EFFECT_TURN_THRESHOLD)
                            {
                                next_heading = current_angle - (reverse_turn >> 2);
                            }
                            else
                            {
                                next_heading = current_angle - FIELD_EFFECT_TURN_STEP;
                            }
                        }
                        else if (selector < FIELD_EFFECT_TURN_THRESHOLD)
                        {
                            next_heading = current_angle + (selector >> 2);
                        }
                        else
                        {
                            next_heading = current_angle + FIELD_EFFECT_TURN_STEP;
                        }

                        record->heading = next_heading;
                    }

                    if ((s16)record->heading < 0)
                    {
                        record->heading = (u16)record->heading + FIELD_ANGLE_TURN;
                    }

                    {
                        offset_or_angle = ratan2(SquareRoot0(target_position->vx + target_position->vz), work_vector->vy);
                        current_angle = record->pitch;

                        if (offset_or_angle < current_angle)
                        {
                            state_or_delta = current_angle - offset_or_angle;

                            if (state_or_delta < FIELD_EFFECT_TURN_THRESHOLD)
                            {
                                record->pitch = current_angle - (state_or_delta >> 2);
                            }
                            else
                            {
                                record->pitch = current_angle - FIELD_EFFECT_TURN_STEP;
                            }
                        }
                        else if (current_angle < offset_or_angle)
                        {
                            state_or_delta = offset_or_angle - current_angle;

                            if (state_or_delta < FIELD_EFFECT_TURN_THRESHOLD)
                            {
                                record->pitch = current_angle + (state_or_delta >> 2);
                            }
                            else
                            {
                                record->pitch = current_angle + FIELD_EFFECT_TURN_STEP;
                            }
                        }

                        if (record->pitch < 0)
                        {
                            record->pitch = (u16)record->pitch + FIELD_ANGLE_TURN;
                        }

                        record->rotation_x = 0;
                    }
                }
            }
        }
    }

    /* After the pickup delay, find a nearby recipient and consume the reward effect. */
    if (!(actor->status.word & 1))
    {
        state_or_delta = actor->status.parts.animation_id;

        switch (state_or_delta)
        {
            case FIELD_PICKUP_EXPERIENCE_OR_CURRENCY:
                if ((u16)actor->track_frames[0] >= FIELD_PICKUP_DELAY)
                {
                    recipient_index = field_find_actor_in_range(&record->x, FIELD_PICKUP_DISTANCE, actor, 0);

                    if (recipient_index != -1)
                    {
                        FieldObjectState* recipient_object;
                        FieldObjectState* object_base;
                        s32 counter_slot;
                        FieldPlayerRecord* counter_base;
                        u32 counter_index;

                        if (g_field_pickup_sound_played == 0)
                        {
                            field_play_sound(FIELD_PICKUP_SOUND, field_get_actor_sound_pan(record->source_object_index));
                        }

                        g_field_pickup_sound_played = 1;
                        record->state = FIELD_EFFECT_RETIRED;
                        object_base = g_field_object_states;
                        recipient_object = &object_base[recipient_index];
                        field_set_action_context(recipient_object->key, object_base[actor->owner_object_index].key,
                                                 record->facing_or_reward_kind - FIELD_REWARD_EFFECT_KIND_FIRST);
                        counter_base = g_field_player_records;
                        counter_index = record->facing_or_reward_kind;
                        counter_slot = recipient_index < 3 ? recipient_index : 2;
                        counter_base[counter_slot].pickup_counts[counter_index - FIELD_REWARD_EFFECT_KIND_FIRST] =
                            counter_base[recipient_index < 3 ? recipient_index : 2].pickup_counts[counter_index - FIELD_REWARD_EFFECT_KIND_FIRST] + 1;
                        field_grant_reward(recipient_object->key, object_base[actor->owner_object_index].key,
                                           record->facing_or_reward_kind - FIELD_REWARD_EFFECT_KIND_FIRST);
                        field_release_actor_if_no_effects(record);
                        return;
                    }
                }

                break;
            case FIELD_PICKUP_ITEM:
                if ((u16)actor->track_frames[0] >= FIELD_PICKUP_DELAY)
                {
                    recipient_index = field_find_actor_in_range(&record->x, FIELD_PICKUP_DISTANCE, actor, 0);

                    if (recipient_index != -1)
                    {
                        FieldObjectState* recipient_object;
                        FieldObjectState* object_base;

                        if (g_field_pickup_sound_played == 0)
                        {
                            field_play_sound(FIELD_PICKUP_SOUND, field_get_actor_sound_pan(record->source_object_index));
                        }

                        g_field_pickup_sound_played = 1;
                        record->state = FIELD_EFFECT_RETIRED;
                        object_base = g_field_object_states;
                        recipient_object = &object_base[recipient_index];
                        field_set_action_context(recipient_object->key, object_base[actor->owner_object_index].key, 4);
                        field_grant_reward(recipient_object->key, object_base[actor->owner_object_index].key, 4);
                        field_release_actor_if_no_effects(record);
                        return;
                    }
                }

                break;
            /* Restore 64/256 of maximum capacity, then play the quarter-restore animation. */
            case FIELD_PICKUP_RESTORE_QUARTER:
                if ((u16)actor->track_frames[0] >= FIELD_PICKUP_DELAY)
                {
                    recipient_index = field_find_actor_in_range(&record->x, FIELD_PICKUP_DISTANCE, actor, 0);

                    if (recipient_index != -1)
                    {
                        FieldObjectState* recipient_object;
                        FieldObjectState* object_base;

                        if (g_field_pickup_sound_played == 0)
                        {
                            field_play_sound(FIELD_PICKUP_SOUND, field_get_actor_sound_pan(record->source_object_index));
                        }

                        g_field_pickup_sound_played = 1;
                        record->state = FIELD_EFFECT_RETIRED;
                        object_base = g_field_object_states;
                        recipient_object = &object_base[recipient_index];
                        field_set_action_context(recipient_object->key, object_base[actor->owner_object_index].key, 5);
                        field_grant_reward(recipient_object->key, object_base[actor->owner_object_index].key, 5);
                        field_play_object_animation((struct FieldActor*)&g_field_actors[recipient_index], 0x2C);
                        field_release_actor_if_no_effects(record);
                        return;
                    }
                }

                break;
            /* Restore 128/256 of maximum capacity. */
            case FIELD_PICKUP_RESTORE_HALF:
                if ((u16)actor->track_frames[0] >= FIELD_PICKUP_DELAY)
                {
                    recipient_index = field_find_actor_in_range(&record->x, FIELD_PICKUP_DISTANCE, actor, 0);

                    if (recipient_index != -1)
                    {
                        FieldObjectState* recipient_object;
                        FieldObjectState* object_base;

                        if (g_field_pickup_sound_played == 0)
                        {
                            field_play_sound(FIELD_PICKUP_SOUND, field_get_actor_sound_pan(record->source_object_index));
                        }

                        g_field_pickup_sound_played = 1;
                        record->state = FIELD_EFFECT_RETIRED;
                        object_base = g_field_object_states;
                        recipient_object = &object_base[recipient_index];
                        field_set_action_context(recipient_object->key, object_base[actor->owner_object_index].key, 6);
                        field_grant_reward(recipient_object->key, object_base[actor->owner_object_index].key, 6);
                        field_play_object_animation((struct FieldActor*)&g_field_actors[recipient_index], 0x2D);
                        field_release_actor_if_no_effects(record);
                        return;
                    }
                }

                break;
            default:
                break;
        }
    }

    /* Collect effect-centered hits, reflect below-ground motion, then scale speed. */
    {
        FieldAnimationDef* anim = actor->animation;

        if ((anim->hit_test_mode == FIELD_HIT_TEST_EFFECT_BOUNDS) && (anim->hit_test_part == record->owner_part))
        {
            field_collect_effect_hits(record, anim->hit_radius, actor);
        }
    }

    if ((part->behavior_flags.word >> FIELD_PART_GROUND_BOUNCE_BIT) & 1)
    {
        s32 ground_y = record->y;

        if (ground_y > 0)
        {
            s32 old_pitch;
            s16 old_motion;

            record->y = -ground_y;
            old_pitch = (u16)record->pitch;
            old_motion = record->motion_parameter;
            record->pitch = FIELD_ANGLE_HALF_TURN - old_pitch;
            record->motion_parameter = old_motion / 2;
            field_dispatch_actor_audio_event(actor, FIELD_SOUND_EVENT_EFFECT_BOUNCE, record->owner_part, old_pitch);
        }
    }

    record->motion_parameter = (record->motion_parameter * record->motion_scale) >> 8;
}

/**
 * @brief Release an effect's owning actor after its last live effect retires.
 * @param effect Motion record whose actor_index identifies the owning actor slot.
 * @note Callers retire the effect before this check. Other live effects keep the
 * actor active; this function does not retire records or free their storage.
 */
void field_release_actor_if_no_effects(FieldMotionRecord* effect)
{
    FieldActorSlot* actor_slots;
    FieldActorSlot* actor_slot;

    if (field_actor_has_live_effects(effect->owner_slot) == 0)
    {
        actor_slots = g_field_actor_slots;
        actor_slot = &actor_slots[effect->owner_slot];
        actor_slot->active = 0;
        actor_slot->pending_track_mask = 0;
        actor_slot->track_mask = 0;
    }
}

/**
 * @brief Check the effect pool for an unretired record owned by an actor.
 * @param actor_index Actor slot index to compare with each record's owner.
 * @return 1 if a matching live effect exists, otherwise 0.
 */
s32 field_actor_has_live_effects(s32 actor_index)
{
    s32 effect_index;
    s32 retired_state;
    FieldMotionRecord* effect;

    effect_index = 0;
    retired_state = FIELD_EFFECT_RETIRED;
    effect = g_field_effect_records;

    for (; effect_index < FIELD_EFFECT_POOL_COUNT; effect_index++)
    {
        if (effect->state != retired_state && effect->owner_slot == actor_index)
        {
            return 1;
        }

        effect++;
    }

    return 0;
}

/**
 * @brief Pack recipient, source, and action identifiers into shared action context.
 * @param recipient_id Identifier placed starting at bit 16.
 * @param source_id Identifier placed starting at bit 8.
 * @param action Action/reward selector placed in the low bits.
 * @note Inputs are not masked here; preserve their original word-wide behavior.
 */
static void field_set_action_context(s32 recipient_id, s32 source_id, s32 action)
{
    g_field_action_context = (recipient_id << 0x10) | (source_id << 8) | action;
}

/**
 * @brief Motion record of the object the effect's actor is tracking in the current track.
 * @param effect Effect whose actor slot selects the tracked object.
 * @return Motion record of the tracked object.
 */
static inline FieldMotionRecord* field_effect_track_object(FieldMotionRecord* effect)
{
    return &FIELD_ACTOR_MOTION_RECORDS[g_field_actor_slots[effect->owner_slot].targets[g_field_track_index]];
}

/**
 * @brief Motion record of the object that owns the effect's actor.
 * @param effect Effect whose actor slot selects the owner object.
 * @return Motion record of the owner object.
 */
static inline FieldMotionRecord* field_effect_owner_object(FieldMotionRecord* effect)
{
    return &FIELD_ACTOR_MOTION_RECORDS[g_field_actor_slots[effect->owner_slot].owner_object_index];
}

/**
 * @brief Resolve an effect's selected target/attachment position in world coordinates.
 * @param effect Record selecting a source and supplying saved positions or effect links.
 * @param part Part definition controlling anchor choice, placement, and facing updates.
 * @param position Destination X/Y/Z; its pad word is untouched.
 * @note Sources 0 and values above 15 leave @p position unchanged. Source 9 can update
 * the record's facing bit. References at +0x30 use an unsigned halfword read.
 */
void field_resolve_effect_position(FieldMotionRecord* effect, FieldObjectPart* part, VECTOR* position)
{
    FieldMotionRecord* source_record;
    FieldMotionRecord* opposite_record;
    FieldMotionRecord* linked_record;
    FieldObjectState* source_object;
    s32 object_index;
    s32 source_index;
    s32 placement;
    s32 delta_x;
    s32 position_x;
    s32 source_x;
    s32 horizontal_offset;

    switch (effect->color_position.fields.position_source)
    {
        case FIELD_POSITION_TRACK_OBJECT:
            position->vx = field_effect_track_object(effect)->x;
            position->vy = field_effect_track_object(effect)->y;
            position->vz = field_effect_track_object(effect)->z;
            return;
        case FIELD_POSITION_OWNER_OBJECT:
            position->vx = field_effect_owner_object(effect)->x;
            position->vy = field_effect_owner_object(effect)->y;
            position->vz = field_effect_owner_object(effect)->z;
            return;
        case FIELD_POSITION_SAVED:
            position->vx = effect->work_x - g_field_view_offset_x;
            position->vy = effect->work_y - g_field_view_offset_y;
            position->vz = effect->work_z - g_field_view_offset_z;
            return;
        case FIELD_POSITION_REFERENCE_EFFECT:
            if (g_field_effect_records[(u16)effect->reference_index].state != FIELD_EFFECT_RETIRED)
            {
                position->vx = g_field_effect_records[(u16)effect->reference_index].x;
                position->vy = g_field_effect_records[(u16)effect->reference_index].y;
                position->vz = g_field_effect_records[(u16)effect->reference_index].z;
                return;
            }

            position->vx = effect->x;
            position->vy = effect->y;
            position->vz = effect->z;
            return;
        case FIELD_POSITION_OWNER_ATTACHMENT:
            source_record = &FIELD_ACTOR_MOTION_RECORDS[g_field_actor_slots[effect->owner_slot].owner_object_index];
            source_object = &g_field_object_states[g_field_actor_slots[effect->owner_slot].owner_object_index];
            position->vx =
                source_record->x +
                (source_object->attachment_points[(part->effect_flags.word >> FIELD_PART_ATTACHMENT_INDEX_SHIFT) & FIELD_PART_ATTACHMENT_INDEX_MASK].x << 8);
            position->vy =
                source_record->y +
                (source_object->attachment_points[(part->effect_flags.word >> FIELD_PART_ATTACHMENT_INDEX_SHIFT) & FIELD_PART_ATTACHMENT_INDEX_MASK].y << 8);
            position->vz = source_record->z;
            return;
        case FIELD_POSITION_FACING_OFFSET:
            placement = FIELD_PART_PLACEMENT_KIND(part);

            if (placement >= FIELD_PLACEMENT_TRACK_BOUNDS)
            {
                if (placement < FIELD_PLACEMENT_NONE)
                {
                    source_index = g_field_actor_slots[effect->owner_slot].targets[g_field_track_index];
                }
                else
                {
                    source_index = g_field_actor_slots[effect->owner_slot].owner_object_index;
                }
            }
            else
            {
                source_index = g_field_actor_slots[effect->owner_slot].owner_object_index;
            }

            source_record = &FIELD_ACTOR_MOTION_RECORDS[source_index];

            if (source_record->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
            {
                position_x = effect->work_x;
                source_x = source_record->x;
                position_x = position_x + source_x;
            }
            else
            {
                source_x = effect->work_x;
                position_x = -source_x;
                position_x += source_record->x;
            }

            position->vx = position_x;
            position->vy = effect->work_y + source_record->y;
            position->vz = effect->work_z + source_record->z;
            return;
        case FIELD_POSITION_TRACK_STORED_XZ:
            position->vx = g_field_object_states[g_field_actor_slots[effect->owner_slot].targets[g_field_track_index]].route_history[0].x << 8;
            position->vz = g_field_object_states[g_field_actor_slots[effect->owner_slot].targets[g_field_track_index]].route_history[0].z << 8;
            position->vy = 0;
            return;
        case FIELD_POSITION_LINKED_EFFECT:
            position->vx = g_field_effect_records[effect->position_data.linked_effect_index].x;
            position->vy = g_field_effect_records[effect->position_data.linked_effect_index].y;
            linked_record = &g_field_effect_records[effect->position_data.linked_effect_index];
            position->vz = linked_record->z;
            return;
        case FIELD_POSITION_RELATIVE_SIDE_OFFSET:
            placement = FIELD_PART_PLACEMENT_KIND(part);

            if (placement < FIELD_PLACEMENT_TRACK_BOUNDS)
            {
                source_record = &FIELD_ACTOR_MOTION_RECORDS[g_field_actor_slots[effect->owner_slot].owner_object_index];
                opposite_record = &FIELD_ACTOR_MOTION_RECORDS[g_field_actor_slots[effect->owner_slot].targets[g_field_track_index]];
            }
            else
            {
                if (placement < FIELD_PLACEMENT_NONE)
                {
                    object_index = g_field_actor_slots[effect->owner_slot].targets[g_field_track_index];
                    source_record = &FIELD_ACTOR_MOTION_RECORDS[object_index];
                }
                else
                {
                    object_index = g_field_actor_slots[effect->owner_slot].owner_object_index;
                    source_record = &FIELD_ACTOR_MOTION_RECORDS[object_index];
                }

                opposite_record = &FIELD_ACTOR_MOTION_RECORDS[g_field_actor_slots[effect->owner_slot].owner_object_index];
            }

            if (source_record->x > opposite_record->x)
            {
                if (((part->placement_flags.word >> FIELD_PART_MIRROR_X_WITH_FACING_SHIFT) & 1) || (part->spawn_flags.word & FIELD_PART_MIRROR_X_WITH_OWNER))
                {
                    effect->facing_or_reward_kind = effect->facing_or_reward_kind & FIELD_ANIMATION_INDEX_MASK;
                }

                horizontal_offset = effect->work_x;
                position_x = source_record->x - horizontal_offset;
            }
            else
            {
                if (((part->placement_flags.word >> FIELD_PART_MIRROR_X_WITH_FACING_SHIFT) & 1) || (part->spawn_flags.word & FIELD_PART_MIRROR_X_WITH_OWNER))
                {
                    effect->facing_or_reward_kind = effect->facing_or_reward_kind | FIELD_EFFECT_FACING_FLIPPED;
                }

                position_x = effect->work_x;
                source_x = source_record->x;
                position_x = position_x + source_x;
            }

            position->vx = position_x;
            position->vy = effect->work_y + source_record->y;
            position->vz = effect->work_z + source_record->z;
            return;
        case FIELD_POSITION_OWNER_BOUNDS_CENTER:
            source_object = &g_field_object_states[g_field_actor_slots[effect->owner_slot].owner_object_index];
            position->vx = field_effect_owner_object(effect)->x + ((source_object->bounds.half.right + source_object->bounds.half.left) << 7);
            position->vy = field_effect_owner_object(effect)->y + ((source_object->bounds.half.bottom + source_object->bounds.half.top) << 7);
            position->vz = field_effect_owner_object(effect)->z;
            return;
        case FIELD_POSITION_TRACK_BOUNDS_CENTER:
            source_object = &g_field_object_states[g_field_actor_slots[effect->owner_slot].targets[g_field_track_index]];
            position->vx = field_effect_track_object(effect)->x + ((source_object->bounds.half.right + source_object->bounds.half.left) << 7);
            position->vy = field_effect_track_object(effect)->y + ((source_object->bounds.half.bottom + source_object->bounds.half.top) << 7);
            position->vz = field_effect_track_object(effect)->z;
            return;
        case FIELD_POSITION_REFLECT_TRACK_X:
        case FIELD_POSITION_EXTEND_TRACK_X:
            delta_x = g_field_actors[g_field_actor_slots[effect->owner_slot].targets[g_field_track_index]].x -
                      g_field_actors[g_field_actor_slots[effect->owner_slot].owner_object_index].x;

            if (effect->color_position.fields.position_source == FIELD_POSITION_REFLECT_TRACK_X)
            {
                position->vx = g_field_actors[g_field_actor_slots[effect->owner_slot].owner_object_index].x - delta_x;
            }
            else
            {
                position->vx = g_field_actors[g_field_actor_slots[effect->owner_slot].targets[g_field_track_index]].x + delta_x;
            }

            position->vy = g_field_actors[g_field_actor_slots[effect->owner_slot].owner_object_index].y;
            position->vz = g_field_actors[g_field_actor_slots[effect->owner_slot].owner_object_index].z;
            return;
        case FIELD_POSITION_EXTEND_TRACK_XZ:
            position->vx = (g_field_actors[g_field_actor_slots[effect->owner_slot].targets[g_field_track_index]].x * 2) -
                           g_field_actors[g_field_actor_slots[effect->owner_slot].owner_object_index].x;
            position->vy = g_field_actors[g_field_actor_slots[effect->owner_slot].targets[g_field_track_index]].y;
            position->vz = (g_field_actors[g_field_actor_slots[effect->owner_slot].targets[g_field_track_index]].z * 2) -
                           g_field_actors[g_field_actor_slots[effect->owner_slot].owner_object_index].z;
            return;
        case FIELD_POSITION_EXTEND_LINK_XZ:
            position->vx = (g_field_actors[g_field_actor_slots[effect->owner_slot].targets[g_field_track_index]].x * 2) -
                           g_field_effect_records[effect->position_data.linked_effect_index].x;
            position->vy = g_field_actors[g_field_actor_slots[effect->owner_slot].targets[g_field_track_index]].y;
            position->vz = (g_field_actors[g_field_actor_slots[effect->owner_slot].targets[g_field_track_index]].z * 2) -
                           g_field_effect_records[effect->position_data.linked_effect_index].z;
            return;
    }
}

/**
 * @brief Render each active field effect according to its render state.
 * @param render_context Ordering table and primitive packet cursor.
 */
void field_render_effects(FieldRenderContext* render_context)
{
    FieldMotionRecord* effect;
    FieldObjectState* object_state;
    FieldObjectPart* part;
    u32* ordering_table;
    FieldResourceEntry* resources;
    s32* packet_cursor;
    u8* frame_data;
    s32 object_index;
    uintptr_t actor_or_object_index;
    u8* actor_address;
    FieldActorSlot* actor;
    s32 part_offset;
    s32 value;

    effect = g_field_effect_records;
    resources = g_field_resource_entries;
    ordering_table = &render_context->ordering_table;
    packet_cursor = render_context->packet_cursor;

    for (; effect != &g_field_effect_records[FIELD_EFFECT_ACTIVE_RECORD_COUNT]; effect++)
    {
        g_field_track_index = effect->track_index;

        if (effect->state != FIELD_EFFECT_DISABLED && effect->state != FIELD_EFFECT_RETIRED)
        {
            switch (effect->state)
            {
                case FIELD_EFFECT_RENDER_RADIAL_LINES:
                    packet_cursor = (s32*)field_render_effect_radial_lines(effect, (u8*)packet_cursor, (s32*)ordering_table);
                    break;

                case FIELD_EFFECT_RENDER_MESH_2:
                case FIELD_EFFECT_RENDER_MESH_1:
                case FIELD_EFFECT_RENDER_MESH_0:
                    if (g_field_actor_slots[effect->owner_slot].parts[effect->owner_part].rotation_extent.fields.linked_part < 8)
                    {
                        packet_cursor = field_render_lit_effect_mesh(effect, FIELD_EFFECT_RENDER_MESH_0 - effect->state, packet_cursor, ordering_table);
                    }
                    else
                    {
                        packet_cursor = field_render_effect_mesh(effect, FIELD_EFFECT_RENDER_MESH_0 - effect->state, packet_cursor, ordering_table);
                    }

                    break;

                case FIELD_EFFECT_RENDER_RING:
                    packet_cursor = (s32*)field_render_effect_ring(effect, (u8*)packet_cursor, (s32*)ordering_table);
                    break;

                case FIELD_EFFECT_RENDER_FAN:
                    packet_cursor = (s32*)field_render_effect_fan(effect, (u8*)packet_cursor, (s32*)ordering_table);
                    break;

                case FIELD_EFFECT_RENDER_MARKER:
                    packet_cursor = (s32*)field_render_effect_marker(effect, (u8*)packet_cursor, (s32*)ordering_table);
                    break;

                case FIELD_EFFECT_RENDER_LINKED_SPRITE:
                    part = &g_field_actor_slots[effect->owner_slot].parts[effect->owner_part];
                    value = FIELD_PART_PLACEMENT_KIND(part);

                    if ((value >= 0xA && value < 0x14) || value == 0x28)
                    {
                        actor_or_object_index = g_field_actor_slots[effect->owner_slot].targets[effect->track_index];
                        frame_data = resources[g_field_actors[actor_or_object_index].resource_index].start;
                        object_state = &g_field_object_states[actor_or_object_index];
                    }
                    else
                    {
                        actor_or_object_index = g_field_actor_slots[effect->owner_slot].owner_object_index;
                        frame_data = resources[g_field_actors[actor_or_object_index].resource_index].start;
                        object_state = &g_field_object_states[actor_or_object_index];
                    }

                    object_state->linked_effect_index = (effect - g_field_effect_records);

                    if (frame_data != NULL)
                    {
                        frame_data = field_advance_actor_part_animation_frame((FieldActor*)effect, frame_data);

                        if (frame_data != NULL)
                        {
                            if (!((uintptr_t)frame_data & FIELD_FRAME_DATA_8BIT))
                            {
                                packet_cursor = field_render_effect_frame16(effect, packet_cursor, ordering_table, frame_data,
                                                                            (object_state->contact.bytes.flags & 1) ^ 1, part);
                            }
                            else
                            {
                                packet_cursor = field_render_effect_frame8(effect, packet_cursor, ordering_table, frame_data,
                                                                           (object_state->contact.bytes.flags & 1) ^ 1, part);
                            }
                        }
                    }

                    break;

                case FIELD_EFFECT_RENDER_SPRITE:
                    frame_data = field_advance_actor_part_animation_frame((FieldActor*)effect, g_field_builtin_track_data);

                    if (frame_data != NULL)
                    {
                        packet_cursor = field_render_effect_sprite_frames(effect, packet_cursor, ordering_table, frame_data);
                    }

                    break;

                case FIELD_EFFECT_RENDER_ACTOR_SPRITE:
                    frame_data = g_field_actor_slots[effect->owner_slot].track_data;

                    if (frame_data != NULL)
                    {
                        frame_data = field_advance_actor_part_animation_frame((FieldActor*)effect, frame_data);

                        if (frame_data != NULL)
                        {
                            packet_cursor = field_render_effect_sprite_frames(effect, packet_cursor, ordering_table, frame_data);
                        }
                    }

                    break;

                case FIELD_EFFECT_RENDER_OWNER_SPRITE:
                    actor = &g_field_actor_slots[effect->owner_slot];
                    actor_or_object_index = effect->owner_part;
                    part_offset = actor_or_object_index * sizeof(FieldObjectPart);
                    actor_or_object_index = (uintptr_t)actor->parts;
                    object_index = actor->owner_object_index;
                    part = (FieldObjectPart*)(actor_or_object_index + part_offset);
                    frame_data = resources[g_field_actors[object_index].resource_index].start;
                    object_state = &g_field_object_states[object_index];
                    object_state->linked_effect_index = (effect - g_field_effect_records);

                    if (frame_data != NULL)
                    {
                        frame_data = field_advance_actor_part_animation_frame((FieldActor*)effect, frame_data);

                        if (frame_data != NULL)
                        {
                            if (!((uintptr_t)frame_data & FIELD_FRAME_DATA_8BIT))
                            {
                                packet_cursor = field_render_effect_frame16(effect, packet_cursor, ordering_table, frame_data,
                                                                            (object_state->contact.bytes.flags & 1) ^ 1, part);
                            }
                            else
                            {
                                packet_cursor = field_render_effect_frame8(effect, packet_cursor, ordering_table, frame_data,
                                                                           (object_state->contact.bytes.flags & 1) ^ 1, part);
                            }
                        }
                    }

                    break;

                case FIELD_EFFECT_RENDER_TRACK_SPRITE:
                    value = effect->owner_slot;
                    actor_address = (u8*)&g_field_actor_slots[value];
                    value = effect->owner_part;
                    part = &((FieldActorSlot*)actor_address)->parts[value];
                    value = effect->track_index;
                    object_index = ((FieldActorSlot*)actor_address)->targets[value];
                    frame_data = resources[g_field_actors[object_index].resource_index].start;
                    object_state = &g_field_object_states[object_index];
                    object_state->linked_effect_index = (effect - g_field_effect_records);

                    if (frame_data != NULL)
                    {
                        frame_data = field_advance_actor_part_animation_frame((FieldActor*)effect, frame_data);

                        if (frame_data != NULL)
                        {
                            if (!((uintptr_t)frame_data & FIELD_FRAME_DATA_8BIT))
                            {
                                packet_cursor = field_render_effect_frame16(effect, packet_cursor, ordering_table, frame_data,
                                                                            (object_state->contact.bytes.flags & 1) ^ 1, part);
                            }
                            else
                            {
                                packet_cursor = field_render_effect_frame8(effect, packet_cursor, ordering_table, frame_data,
                                                                           (object_state->contact.bytes.flags & 1) ^ 1, part);
                            }
                        }
                    }

                    break;

                case FIELD_EFFECT_RENDER_RADIAL_FAN:
                    packet_cursor = (s32*)field_render_effect_radial_fan(effect, (u8*)packet_cursor, (s32*)ordering_table);
                    break;

                case FIELD_EFFECT_RENDER_TRAIL:
                    packet_cursor = (s32*)field_render_effect_trail(effect, (u8*)packet_cursor, (s32*)ordering_table);
                    break;

                case FIELD_EFFECT_RENDER_RIBBON:
                    packet_cursor = (s32*)field_render_effect_ribbon(effect, (u8*)packet_cursor, ordering_table);
                    break;
            }
        }
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
 */
static s32* field_render_effect_sprite_frames(FieldMotionRecord* effect, s32* packet_cursor, s32* ordering_table, u8* frame_data)
{
    Vec2s* screen_origin = FIELD_EFFECT_SCRATCH_SCREEN_ORIGIN;
    Vec2s* shadow_footprint = FIELD_EFFECT_SCRATCH_FOOTPRINT;
    MATRIX* matrix = FIELD_EFFECT_SCRATCH_MATRIX;
    s32 frame_count;
    s32 shadow_count;
    s32 texture_slot;
    FieldPrimitiveColor packed_color;
    FieldActorSlot* actor;
    FieldObjectPart* part;
    FieldActorSlot* actors;
    s32 height_minus_one;
    s32 width_or_mode;
    s32 frame_x;
    s32 frame_y_or_clut;
    intptr_t value0;
    s32 value1;
    s32 clut_x;
    s32 clut;
    u16* texture_slot_flags;

    shadow_count = 0;
    texture_slot = 2;
    actors = g_field_actor_slots;
    part = &actors[effect->owner_slot].parts[effect->owner_part];
    actor = &actors[effect->owner_slot];

    if (actor->owner_object_index < 2)
    {
        texture_slot = actor->owner_object_index;
    }

    field_build_effect_part_matrix(effect, part, FIELD_EFFECT_SCRATCH_MATRIX, actor);
    gte_SetRotMatrix(FIELD_EFFECT_SCRATCH_MATRIX);

    screen_origin->x = FIELD_SCREEN_X(effect->x);
    screen_origin->y = FIELD_SCREEN_Y(effect->y, effect->z);

    frame_count = *frame_data++;
    field_resolve_effect_part_color(actor, effect, part, &packed_color);

    if (frame_count != 0)
    {
        texture_slot_flags = g_field_texture_slot_flags;

        while (frame_count != 0)
        {
            u8 flags = frame_data[7];

            if (!(flags & FIELD_SPRITE_FRAME_FOOTPRINT))
            {
                value1 = packed_color.signed_word;
                setlen(packet_cursor, 9);
                SET_BGR0_PACKED(((POLY_FT4*)packet_cursor), value1);
                setcode(packet_cursor, 0x2C);
                setSemiTrans((POLY_FT4*)packet_cursor, effect->flags & FIELD_EFFECT_SEMITRANSPARENT);
                width_or_mode = frame_data[4];
                height_minus_one = frame_data[5] - 1;
                effect->sprite_height_minus_one = height_minus_one;
                frame_y_or_clut = (s8)frame_data[1];

                if (!(effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED))
                {
                    frame_x = (s8)frame_data[0];
                }
                else
                {
                    frame_x = -(s8)frame_data[0] - width_or_mode;
                }

                width_or_mode--;

                if (part->track_flags.halves.high & 1)
                {
                    frame_y_or_clut -= FIELD_SCREEN_CENTER_Y;
                }

                field_project_effect_sprite_quad(effect, screen_origin, (POLY_FT4*)packet_cursor, width_or_mode, height_minus_one, frame_x, frame_y_or_clut,
                                                 (FieldSpriteFrame*)frame_data, matrix);

                if ((frame_data[7] ^ (effect->facing_or_reward_kind >> 1)) & FIELD_SPRITE_FRAME_FLIP_X)
                {
                    value0 = frame_data[2];
                    ((POLY_FT4*)packet_cursor)->u3 = value0;
                    ((POLY_FT4*)packet_cursor)->u1 = value0;
                    value0 += width_or_mode;
                    ((POLY_FT4*)packet_cursor)->u2 = value0;
                    ((POLY_FT4*)packet_cursor)->u0 = value0;
                }
                else
                {
                    value0 = frame_data[2];
                    ((POLY_FT4*)packet_cursor)->u2 = value0;
                    ((POLY_FT4*)packet_cursor)->u0 = value0;
                    value0 += width_or_mode;
                    ((POLY_FT4*)packet_cursor)->u3 = value0;
                    ((POLY_FT4*)packet_cursor)->u1 = value0;
                }

                if (frame_data[7] & FIELD_SPRITE_FRAME_FLIP_Y)
                {
                    value0 = frame_data[3];
                    ((POLY_FT4*)packet_cursor)->v3 = value0;
                    ((POLY_FT4*)packet_cursor)->v2 = value0;
                    value0 += height_minus_one;
                    ((POLY_FT4*)packet_cursor)->v1 = value0;
                    ((POLY_FT4*)packet_cursor)->v0 = value0;
                }
                else
                {
                    value0 = frame_data[3];
                    ((POLY_FT4*)packet_cursor)->v1 = value0;
                    ((POLY_FT4*)packet_cursor)->v0 = value0;
                    value0 += height_minus_one;
                    ((POLY_FT4*)packet_cursor)->v3 = value0;
                    ((POLY_FT4*)packet_cursor)->v2 = value0;
                }

                width_or_mode = frame_data[7] & FIELD_SPRITE_TEXTURE_MODE_MASK;

                if (width_or_mode == 2)
                {
                    frame_y_or_clut = FIELD_SHARED_CLUT_VRAM_Y;

                    if (actor->owner_object_index < 2)
                    {
                        *(s16*)&((POLY_FT4*)packet_cursor)->tpage = ((texture_slot_flags[texture_slot] & 3) << 7) |
                                                                    ((part->behavior_flags.word >> FIELD_PART_BLEND_TPAGE_SHIFT) & GPU_TPAGE_BLEND_MASK) |
                                                                    0x10 |
                                                                    ((((actor->owner_object_index << 6) + FIELD_ACTOR_TEXTURE_VRAM_X) & (VRAM_WIDTH - 1)) >> 6);
                        frame_y_or_clut = (actor->owner_object_index * 2) + FIELD_ACTOR_CLUT_VRAM_Y;
                    }
                    else
                    {
                        value0 = texture_slot;
                        value0 <<= 1;
                        value0 += (uintptr_t)texture_slot_flags;
                        value1 = ((*(u16*)value0 & 3) << 7) | ((part->behavior_flags.word >> FIELD_PART_BLEND_TPAGE_SHIFT) & GPU_TPAGE_BLEND_MASK);
                        value1 |= 5;
                        *(s16*)&((POLY_FT4*)packet_cursor)->tpage = value1;
                    }
                }
                else
                {
                    frame_y_or_clut = (width_or_mode * 2) + FIELD_EFFECT_CLUT_VRAM_Y;
                    value0 = (((width_or_mode << 6) + FIELD_EFFECT_TEXTURE_VRAM_X) & (VRAM_WIDTH - 1)) >> 6;
                    value1 = (part->behavior_flags.word >> FIELD_PART_BLEND_TPAGE_SHIFT) & GPU_TPAGE_BLEND_MASK;
                    value1 |= value0;
                    ((POLY_FT4*)packet_cursor)->tpage = value1;
                }

                if (FIELD_PART_HAS_ANIMATED_PALETTE(part))
                {
                    ((POLY_FT4*)packet_cursor)->clut = (frame_y_or_clut + FIELD_ANIMATED_CLUT_ROW) << 6;
                }
                else
                {
                    value0 = part->placement_flags.word >> FIELD_PART_PALETTE_SOURCE_SHIFT;

                    switch (value0 & FIELD_PART_PALETTE_SOURCE_MASK)
                    {
                        case FIELD_PART_PALETTE_SELECTOR:
                        {
                            u8 uv = part->appearance.fields.palette_selector;
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
                            ((POLY_FT4*)packet_cursor)->clut = value0;
                            break;
                        }
                        case FIELD_PART_PALETTE_FRAME:
                            if (actor->owner_object_index >= 3)
                            {
                                clut = frame_y_or_clut << 6;
                                value0 = part->appearance.fields.palette_selector & 0x3F;
                                clut |= value0;
                                ((POLY_FT4*)packet_cursor)->clut = clut;
                                break;
                            }

                            /* Player slots use the frame's own palette, like mode 0. */
                        case FIELD_PART_PALETTE_RESOURCE:
                            clut = frame_y_or_clut << 6;
                            value0 = frame_data[6] & 0x3F;
                            clut |= value0;
                            ((POLY_FT4*)packet_cursor)->clut = clut;
                            break;
                    }
                }

                /* Screen-space and negative-depth sprites use the front entry. */
                if (effect->flags & FIELD_EFFECT_SCREEN_SPACE)
                {
                    addPrim(&ordering_table[0], packet_cursor);
                    packet_cursor = (s32*)((u8*)packet_cursor + sizeof(POLY_FT4));
                }
                else
                {
                    value1 = effect->z >> FIELD_WORLD_OT_DEPTH_SHIFT;

                    if (value1 < 0)
                    {
                        addPrim(&ordering_table[0], packet_cursor);
                        packet_cursor = (s32*)((u8*)packet_cursor + sizeof(POLY_FT4));
                    }
                    else if (value1 > FIELD_WORLD_OT_LAST)
                    {
                        addPrim(&ordering_table[FIELD_WORLD_OT_LAST], packet_cursor);
                        packet_cursor = (s32*)((u8*)packet_cursor + sizeof(POLY_FT4));
                    }
                    else
                    {
                        addPrim(&ordering_table[effect->z >> 7], packet_cursor);
                        packet_cursor = (s32*)((u8*)packet_cursor + sizeof(POLY_FT4));
                    }
                }
            }
            else if (((flags & 0xF) == 2) && (part->effect_flags.word & FIELD_PART_SOLID))
            {
                shadow_footprint[0].x = ((s8)frame_data[0] * part->appearance.fields.scale_xz) >> FIELD_PART_SCALE_SHIFT;
                shadow_footprint[0].y = ((s8)frame_data[1] * part->scale_y) >> FIELD_PART_SCALE_SHIFT;
                shadow_footprint[1].x = ((s8)frame_data[2] * part->appearance.fields.scale_xz) >> FIELD_PART_SCALE_SHIFT;
                shadow_footprint[1].y = ((s8)frame_data[3] * part->scale_y) >> FIELD_PART_SCALE_SHIFT;
                shadow_footprint[2].x = ((s8)frame_data[4] * part->appearance.fields.scale_xz) >> FIELD_PART_SCALE_SHIFT;
                shadow_footprint[2].y = ((s8)frame_data[5] * part->scale_y) >> FIELD_PART_SCALE_SHIFT;
                shadow_footprint[3].x = ((s8)frame_data[6] * part->appearance.fields.scale_xz) >> FIELD_PART_SCALE_SHIFT;
                shadow_count++;
                shadow_footprint[3].y = ((s8)frame_data[8] * part->scale_y) >> FIELD_PART_SCALE_SHIFT;
            }

            frame_data += 9;
            frame_count--;
            value0 = *(s32*)&((POLY_FT4*)packet_cursor)[-1].r0;
            SET_BGR0_PACKED(((POLY_FT4*)packet_cursor), value0);
        }
    }

    if (shadow_count != 0)
    {
        packet_cursor = (s32*)field_render_actor_ground_shadow((FieldActor*)effect, (POLY_FT4*)packet_cursor, ordering_table, shadow_footprint);
    }

    return packet_cursor;
}

/* Frame-record processors: field_render_effect_frame8 (8-bit frame coordinates) and field_render_effect_frame16 (16-bit). */

/**
 * @brief Look up the actor binding that serves an object slot.
 * @param object_index Object slot; slots 0 and 1 own their bindings, later slots share entry 2.
 * @return Binding record for the slot.
 */
static inline FieldActorBinding* field_get_object_binding(s32 object_index)
{
    FieldActorBinding* bindings = g_field_actor_bindings;

    return &bindings[object_index < 3 ? object_index : 2];
}

/**
 * @brief Draw one frame of an effect's 8-bit frame records and run its placement opcodes.
 * @param rec Effect record being drawn; supplies position, facing, palette resource and depth.
 * @param cursor Primitive buffer cursor; POLY_FT4 packets are written here.
 * @param base Ordering table the packets are linked into (depth = z >> 7, clamped to 0..0xFFF).
 * @param item Frame data: a count byte followed by that many 9-byte records.
 * @param flag Zero runs every placement record; non-zero runs only opcode 2, and opcode 0
 *        while the object's contact bit 0 is set (the caller passes the inverse of that bit).
 * @param part Part definition supplying tpage/clut selectors, placement and footprint flags.
 * @return The advanced primitive cursor.
 * @note Clears the object's collision word (and, when @p flag is 0, its bounds and effect
 *       vertices), then stores the frame's quad corners as its bounds.
 * @note Records with item[7] bit 0x20 clear are sprites: signed 8-bit x/y offsets, u, v,
 *       width, height, clut column, flags (0x40 mirror U, 0x80 mirror V, low 2 bits tpage
 *       column; value 2 selects the alternate palette block). A sprite whose low bits and
 *       clut column are both 2 is skipped for player objects 0-2 with a non-zero hit state.
 * @note Records with bit 0x20 set are placement records selected by the low nibble:
 *       0/3 transform the effect quad vertices, 1 hit-tests the projected quad and collects
 *       or reacts to contacts, 2 queues ground-shadow corners, 4 and 6 set the attachment
 *       points and start the bound animation, 5 plays the resource's sound cue.
 * @note Sibling of field_render_effect_frame16, which handles 11/17-byte records with 16-bit deltas.
 */
s32* field_render_effect_frame8(FieldMotionRecord* rec, s32* cursor, s32* base, u8* item, s32 flag, FieldObjectPart* part)
{
    MATRIX* mtx = (MATRIX*)SCRATCHPAD_ADDRESS;
    Vec2s* sxy = (Vec2s*)SCRATCHPAD_AT(0x40);
    s32 item_count;
    VECTOR* gte_out = (VECTOR*)SCRATCHPAD_AT(0x44);
    SVECTOR* dir = (SVECTOR*)SCRATCHPAD_AT(0x54);
    Vec2s* quad_bounds = FIELD_EFFECT_SCRATCH_FOOTPRINT;
    FieldContactPoint* contact_quad;
    Vec2s* target_screen;
    FieldActorSlot* actor;
    s32 vertical_lift;
    s32 shadow_count;
    FieldContactResult contact;
    struct
    {
        s32 target_index;
        u8 pad34[0x24];
    } scratch;

    POLY_FT4* poly;
    s32 tpage_x;
    s32 placement_flags;
    s32 layer;
    s32 v_extent;
    s32 contact_result;
    s32 bound_index;
    s32 depth;
    s32 frame_kind;
    s32 y_offset; /* Sprite y offset, later the palette row and reaction result; one local for all three. */
    s32 u_extent;
    s32 x_offset; /* Sprite x offset, later the bound animation actor; one local for both. */
    u16 screen_y;
    u16 attach_x;
    u16 attach_y;
    u32 effect_flags;
    s32 sound_kind;
    u32 spawn_flags;
    u8 lift_end;
    u8 u_left;
    u8 u_left_flat;
    u8 v_bottom;
    u8 v_top;
    u8 lift_start;
    u8 target_count;
    u8 frame_flags;
    s32 u_right_sum;
    s32 u_right_sum_flat;
    s32 v_top_sum;
    s32 v_bottom_sum;
    u8 palette_selector;
    u8 u_right;
    u8 u_right_flat;
    u8 v_top_end;
    u8 v_bottom_end;
    FieldObjectState* slot;
    FieldObjectState* target_state;

    shadow_count = 0;
    contact_quad = (FieldContactPoint*)SCRATCHPAD_AT(0x80);
    target_screen = (Vec2s*)SCRATCHPAD_AT(0x94);
    slot = &g_field_object_states[rec->source_object_index];
    slot->collision.word = 0;

    if (flag == 0)
    {
        s32 vertex_index;

        for (vertex_index = 7; vertex_index >= 0; vertex_index--)
        {
            slot->effect_vertices.words[vertex_index] = 0;
        }

        slot->bounds.words[1] = 0;
        slot->bounds.words[0] = 0;
    }

    actor = &g_field_actor_slots[rec->owner_slot];
    field_build_effect_part_matrix(rec, part, mtx, actor);
    gte_SetRotMatrix(mtx);

    sxy->x = FIELD_SCREEN_X(rec->x);
    screen_y = FIELD_SCREEN_Y(rec->y, rec->z);
    sxy->y = screen_y;
    lift_start = rec->vertical_offset;
    lift_end = rec->next_height;

    if ((lift_start | lift_end) != 0)
    {
        vertical_lift = (s8)lift_start + (((s8)lift_end - (s8)lift_start) * rec->frame_ticks) / rec->frame_length;
        sxy->y = screen_y - vertical_lift;
    }
    else
    {
        vertical_lift = 0;
    }

    field_extract_effect_quad_corners8(rec, item, quad_bounds);
    spawn_flags = part->spawn_flags.word;

    if (spawn_flags & 0x100000)
    {
        field_apply_effect_quad_center_offset(rec, sxy, quad_bounds, (uintptr_t)mtx, (spawn_flags >> 0x14) & 1);
    }

    slot->bounds.half.left = quad_bounds[0].x;
    slot->bounds.half.top = quad_bounds[0].y;
    slot->bounds.half.right = quad_bounds[2].x;
    slot->bounds.half.bottom = quad_bounds[2].y;
    item_count = *item++;

    poly = (POLY_FT4*)cursor;

    for (; item_count != 0; item_count--)
    {
        frame_flags = item[7];

        if (!(frame_flags & FIELD_SPRITE_FRAME_FOOTPRINT))
        {
            if (rec->source_object_index >= 3 || (frame_flags & FIELD_SPRITE_TEXTURE_MODE_MASK) != 2 || item[6] != 2 ||
                g_field_player_records[rec->source_object_index].hide_shield_sprite == 0)
            {
                field_resolve_effect_part_color(actor, rec, part, (FieldPrimitiveColor*)&((P_TAG*)cursor)->r0);
                setPolyFT4(poly);
                setSemiTrans(poly, rec->flags & FIELD_EFFECT_SEMITRANSPARENT);
                u_extent = item[4];
                y_offset = (s8)item[1];
                v_extent = item[5] - 1;

                /* Decrement in each arm: one shared mirrored negate (as in frame16) loses 7 instructions. */
                if (!(rec->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED))
                {
                    x_offset = (s8)*item;
                    u_extent--;
                }
                else
                {
                    x_offset = -(s8)*item - u_extent;
                    u_extent--;
                }

                field_project_effect_sprite_quad(rec, sxy, (POLY_FT4*)cursor, u_extent, v_extent, x_offset, y_offset, (FieldSpriteFrame*)item, mtx);

                if ((item[7] ^ (rec->facing_or_reward_kind >> 1)) & FIELD_SPRITE_FRAME_FLIP_X)
                {
                    u_left = item[2];
                    poly->u1 = u_left;
                    poly->u3 = u_left;
                    u_right_sum = poly->u1 + u_extent;
                    u_right = 0xFF;

                    if (u_right_sum != 0x100)
                    {
                        u_right = u_right_sum;
                    }

                    poly->u2 = u_right;
                    poly->u0 = u_right;
                }
                else
                {
                    u_left_flat = item[2];
                    poly->u0 = u_left_flat;
                    poly->u2 = u_left_flat;
                    u_right_sum_flat = poly->u0 + u_extent;
                    u_right_flat = 0xFF;

                    if (u_right_sum_flat != 0x100)
                    {
                        u_right_flat = u_right_sum_flat;
                    }

                    poly->u3 = u_right_flat;
                    poly->u1 = u_right_flat;
                }

                if (item[7] & FIELD_SPRITE_FRAME_FLIP_Y)
                {
                    v_bottom = item[3];
                    poly->v2 = v_bottom;
                    poly->v3 = v_bottom;
                    v_top_sum = poly->v2 + v_extent;
                    v_top_end = 0xFF;

                    if (v_top_sum != 0x100)
                    {
                        v_top_end = v_top_sum;
                    }

                    poly->v1 = v_top_end;
                    poly->v0 = v_top_end;
                }
                else
                {
                    v_top = item[3];
                    poly->v0 = v_top;
                    poly->v1 = v_top;
                    v_bottom_sum = poly->v0 + v_extent;
                    v_bottom_end = 0xFF;

                    if (v_bottom_sum != 0x100)
                    {
                        v_bottom_end = v_bottom_sum;
                    }

                    poly->v3 = v_bottom_end;
                    poly->v2 = v_bottom_end;
                }

                if ((item[7] & 3) == 2)
                {
                    poly->v0 |= 0x80;
                    poly->v1 |= 0x80;
                    poly->v2 |= 0x80;
                    poly->v3 |= 0x80;
                }

                layer = rec->texture_slot;
                tpage_x = layer << 7;

                if (layer >= 2)
                {
                    tpage_x = layer << 6;

                    if (layer >= FIELD_LOWER_TEXTURE_FIRST_SLOT)
                    {
                        s32 page_offset;
                        s32 page_x;
                        s32 page_bits;

                        page_offset = layer - FIELD_LOWER_TEXTURE_FIRST_SLOT;
                        page_offset <<= 6;
                        page_x = item[7];
                        page_x &= 3;
                        page_x <<= 6;
                        page_x += FIELD_LOWER_TEXTURE_VRAM_X;
                        page_x -= page_offset;
                        page_x &= 0x3FF;
                        page_bits = part->behavior_flags.word;
                        page_x >>= 6;
                        page_bits = (u32)page_bits >> FIELD_PART_BLEND_TPAGE_SHIFT;
                        page_bits &= GPU_TPAGE_BLEND_MASK;
                        page_bits |= 0x10;
                        page_bits |= page_x;
                        poly->tpage = page_bits;
                    }
                    else
                    {
                        s32 page_x;
                        s32 page_abr;

                        page_x = item[7];
                        page_x &= 3;
                        page_x <<= 6;
                        page_x += FIELD_COMPANION_TEXTURE_VRAM_X;
                        page_x -= tpage_x;
                        page_x = (page_x & (VRAM_WIDTH - 1)) >> 6;
                        page_abr = (part->behavior_flags.word >> FIELD_PART_BLEND_TPAGE_SHIFT) & GPU_TPAGE_BLEND_MASK;
                        poly->tpage = page_x | page_abr;
                    }
                }
                else
                {
                    s32 page_x;
                    s32 page_abr;

                    tpage_x = FIELD_PARTY_TEXTURE_VRAM_X - tpage_x;
                    page_abr = (part->behavior_flags.word >> FIELD_PART_BLEND_TPAGE_SHIFT) & GPU_TPAGE_BLEND_MASK;

                    if (item[7] & 3)
                    {
                        page_x = (tpage_x + FIELD_TEXTURE_COLUMN_WIDTH) & (VRAM_WIDTH - 1);
                    }
                    else
                    {
                        page_x = tpage_x & (VRAM_WIDTH - 1);
                    }

                    poly->tpage = page_abr | (page_x >> 6);
                }

                effect_flags = rec->flags;

                if ((effect_flags & 0x7F0000) && (item[6] == 0) && ((item[7] & FIELD_SPRITE_TEXTURE_MODE_MASK) != 2))
                {
                    if (effect_flags & 0x40000)
                    {
                        if (FIELD_PART_PALETTE_SOURCE(part) == FIELD_PART_PALETTE_RESOURCE)
                        {
                            poly->clut = getClut(0x90, rec->resource_index + FIELD_RESOURCE_CLUT_VRAM_Y);
                        }
                        else
                        {
                            palette_selector = part->appearance.fields.palette_selector;

                            if (palette_selector >= FIELD_PALETTE_SELECTOR_SHARED)
                            {
                                y_offset = FIELD_SHARED_CLUT_VRAM_Y;
                            }
                            else
                            {
                                y_offset = (palette_selector >> FIELD_PALETTE_SELECTOR_ROW_SHIFT) + FIELD_EFFECT_CLUT_VRAM_Y;
                            }

                            switch (FIELD_PART_PALETTE_SOURCE(part))
                            {
                                case FIELD_PART_PALETTE_SELECTOR:
                                    poly->clut = getClut((part->appearance.fields.palette_selector & FIELD_PALETTE_SELECTOR_COLUMN_MASK) << 4, y_offset);
                                    break;
                                case FIELD_PART_PALETTE_FRAME:
                                    poly->clut = getClut(0x90, rec->resource_index + FIELD_RESOURCE_CLUT_VRAM_Y);
                                    break;
                            }
                        }
                    }
                    else if (effect_flags & 0x780000)
                    {
                        if (FIELD_PART_PALETTE_SOURCE(part) == FIELD_PART_PALETTE_RESOURCE)
                        {
                            poly->clut = getClut(((effect_flags >> 0x13) & 0xF) << 4, rec->resource_index + FIELD_RESOURCE_CLUT_VRAM_Y);
                        }
                        else
                        {
                            palette_selector = part->appearance.fields.palette_selector;

                            if (palette_selector >= FIELD_PALETTE_SELECTOR_SHARED)
                            {
                                y_offset = FIELD_SHARED_CLUT_VRAM_Y;
                            }
                            else
                            {
                                y_offset = (palette_selector >> FIELD_PALETTE_SELECTOR_ROW_SHIFT) + FIELD_EFFECT_CLUT_VRAM_Y;
                            }

                            switch (FIELD_PART_PALETTE_SOURCE(part))
                            {
                                case FIELD_PART_PALETTE_SELECTOR:
                                    poly->clut = getClut((part->appearance.fields.palette_selector & FIELD_PALETTE_SELECTOR_COLUMN_MASK) << 4, y_offset);
                                    break;
                                case FIELD_PART_PALETTE_FRAME:
                                    poly->clut = getClut((((u32)rec->flags >> 0x13) & 0xF) << 4, rec->resource_index + FIELD_RESOURCE_CLUT_VRAM_Y);
                                    break;
                            }
                        }
                    }
                    else
                    {
                        if (FIELD_PART_PALETTE_SOURCE(part) == FIELD_PART_PALETTE_RESOURCE)
                        {
                            poly->clut = getClut(0x100, ((effect_flags >> 16) & 3) + 0x1EF);
                        }
                        else
                        {
                            palette_selector = part->appearance.fields.palette_selector;

                            if (palette_selector >= FIELD_PALETTE_SELECTOR_SHARED)
                            {
                                y_offset = FIELD_SHARED_CLUT_VRAM_Y;
                            }
                            else
                            {
                                y_offset = (palette_selector >> FIELD_PALETTE_SELECTOR_ROW_SHIFT) + FIELD_EFFECT_CLUT_VRAM_Y;
                            }

                            switch (FIELD_PART_PALETTE_SOURCE(part))
                            {
                                case FIELD_PART_PALETTE_SELECTOR:
                                    poly->clut = getClut((part->appearance.fields.palette_selector & FIELD_PALETTE_SELECTOR_COLUMN_MASK) << 4, y_offset);
                                    break;
                                case FIELD_PART_PALETTE_FRAME:
                                    poly->clut = getClut(0x100, (((u32)rec->flags >> 16) & 3) + 0x1EF);
                                    break;
                            }
                        }
                    }
                }
                else if (FIELD_PART_PALETTE_SOURCE(part) == FIELD_PART_PALETTE_RESOURCE)
                {
                    if ((item[7] & FIELD_SPRITE_TEXTURE_MODE_MASK) != 2)
                    {
                        if (rec->resource_index == FIELD_RESOURCE_FIXED)
                        {
                            poly->clut = getClut(item[6] << 4, FIELD_EFFECT_CLUT_VRAM_Y);
                        }
                        else
                        {
                            poly->clut = getClut(item[6] << 4, rec->resource_index + FIELD_RESOURCE_CLUT_VRAM_Y);
                        }

                        if (item[6] == 0xB)
                        {
                            setSemiTrans(poly, 1);
                        }
                    }
                    else
                    {
                        if (item[6] == 1)
                        {
                            setSemiTrans(poly, 1);
                        }

                        poly->clut = getClut((item[6] * 0x10) + 0xC0, rec->resource_index + FIELD_RESOURCE_CLUT_VRAM_Y);
                    }
                }
                else
                {
                    palette_selector = part->appearance.fields.palette_selector;

                    if (palette_selector >= FIELD_PALETTE_SELECTOR_SHARED)
                    {
                        y_offset = FIELD_SHARED_CLUT_VRAM_Y;

                        if (actor->owner_object_index < 2)
                        {
                            y_offset = (actor->owner_object_index * 2) + FIELD_ACTOR_CLUT_VRAM_Y;
                        }
                    }
                    else
                    {
                        y_offset = (palette_selector >> FIELD_PALETTE_SELECTOR_ROW_SHIFT) + FIELD_EFFECT_CLUT_VRAM_Y;
                    }

                    switch (FIELD_PART_PALETTE_SOURCE(part))
                    {
                        case FIELD_PART_PALETTE_SELECTOR:
                            poly->clut = getClut((part->appearance.fields.palette_selector & FIELD_PALETTE_SELECTOR_COLUMN_MASK) << 4, y_offset);
                            break;
                        case FIELD_PART_PALETTE_FRAME:
                            if (actor->owner_object_index >= 3)
                            {
                                poly->clut = getClut((part->appearance.fields.palette_selector & FIELD_PALETTE_SELECTOR_COLUMN_MASK) << 4, y_offset);
                                break;
                            }

                            if ((item[7] & FIELD_SPRITE_TEXTURE_MODE_MASK) != 2)
                            {
                                poly->clut = getClut(item[6] << 4, rec->resource_index + FIELD_RESOURCE_CLUT_VRAM_Y);
                            }
                            else
                            {
                                if (item[6] == 1)
                                {
                                    setSemiTrans(poly, 1);
                                }

                                poly->clut = getClut((item[6] * 0x10) + 0xC0, rec->resource_index + FIELD_RESOURCE_CLUT_VRAM_Y);
                                break;
                            }

                            break;
                    }
                }

                depth = rec->z >> 7;

                if (depth < 0)
                {
                    addPrim(base, cursor);
                    poly++;
                    cursor += sizeof(POLY_FT4) / sizeof(s32);
                }
                else if (depth >= FIELD_WORLD_OT_SIZE)
                {
                    addPrim(&base[FIELD_WORLD_OT_LAST], cursor);
                    poly++;
                    cursor += sizeof(POLY_FT4) / sizeof(s32);
                }
                else
                {
                    poly++;
                    addPrim(&base[rec->z >> 7], cursor);
                    cursor += sizeof(POLY_FT4) / sizeof(s32);
                }
            }
        }
        else
        {
            frame_kind = frame_flags & 0xF;

            if ((flag == 0) || (frame_kind == 2) || ((slot->contact.word & 1) && (frame_kind == 0)))
            {
                switch (item[7] & 0xF)
                {
                    case 0:
                        field_transform_effect_quad_vertices8(rec, slot, item, 0, sxy, dir, gte_out);
                        break;
                    case 3:
                        field_transform_effect_quad_vertices8(rec, slot, item, 4, sxy, dir, gte_out);
                        break;
                    case 6:
                        if (!(slot->action_parameter & FIELD_ACTION_PARAMETER_TRANSIENT_ACTOR) && !(slot->movement.word & FIELD_MOVEMENT_SEQUENCE_MASK) &&
                            (slot->action_parameter != FIELD_ACTION_PARAMETER_NONE) &&
                            (((rec->motion_parameter == FIELD_ACTOR_COMMAND_TECHNIQUE) &&
                              ((rec->facing_or_reward_kind & FIELD_ANIMATION_INDEX_MASK) == 0x2E)) ||
                             (rec->motion_parameter == FIELD_ACTOR_COMMAND_ACTION) || (rec->motion_parameter == FIELD_ACTOR_COMMAND_98)) &&
                            !(FIELD_FRAME_WORD(rec) & FIELD_FRAME_UNCHANGED))
                        {
                            x_offset = field_find_free_actor_slot(rec->source_object_index, 0);

                            if (x_offset != FIELD_ACTOR_SLOT_NONE)
                            {
                                if (field_start_builtin_animation(rec->source_object_index, x_offset, slot->action_parameter) != 0)
                                {
                                    slot->contact.bytes.animation_actor_index = x_offset;
                                    field_start_actor_animation(x_offset, 0, NULL);

                                    if (rec->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
                                    {
                                        dir->vx = (s8)item[1];
                                        dir->vy = 0;
                                        dir->vz = -(s8)*item;
                                    }
                                    else
                                    {
                                        dir->vx = (s8)item[1];
                                        dir->vy = 0;
                                        dir->vz = (s8)*item;
                                    }

                                    gte_ldv0(dir);
                                    gte_rtv0();
                                    gte_stlvnl(gte_out);
                                    slot->attachment_points[0].x = gte_out->vx;
                                    slot->attachment_points[0].y = gte_out->vy;

                                    if (rec->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
                                    {
                                        dir->vx = (s8)item[3];
                                        dir->vy = 0;
                                        dir->vz = -(s8)item[2];
                                    }
                                    else
                                    {
                                        dir->vx = (s8)item[3];
                                        dir->vy = 0;
                                        dir->vz = (s8)item[2];
                                    }

                                    gte_ldv0(dir);
                                    gte_rtv0();
                                    gte_stlvnl(gte_out);
                                    slot->attachment_points[1].x = gte_out->vx;
                                    slot->attachment_points[1].y = gte_out->vy;

                                    if (rec->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
                                    {
                                        dir->vx = (s8)item[5];
                                        dir->vy = 0;
                                        dir->vz = -(s8)item[4];
                                    }
                                    else
                                    {
                                        dir->vx = (s8)item[5];
                                        dir->vy = 0;
                                        dir->vz = (s8)item[4];
                                    }

                                    gte_ldv0(dir);
                                    gte_rtv0();
                                    gte_stlvnl(gte_out);
                                    slot->attachment_points[2].x = gte_out->vx;
                                    slot->attachment_points[2].y = gte_out->vy;

                                    if (rec->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
                                    {
                                        dir->vx = (s8)item[8];
                                        dir->vy = 0;
                                        dir->vz = -(s8)item[6];
                                    }
                                    else
                                    {
                                        dir->vx = (s8)item[8];
                                        dir->vy = 0;
                                        dir->vz = (s8)item[6];
                                    }

                                    gte_ldv0(dir);
                                    gte_rtv0();
                                    gte_stlvnl(gte_out);
                                    slot->attachment_points[3].x = gte_out->vx;
                                    slot->attachment_points[3].y = gte_out->vy;
                                }
                            }

                            slot->action_parameter = FIELD_ACTION_PARAMETER_NONE;
                            slot->movement.word = (slot->movement.word & ~FIELD_MOVEMENT_SEQUENCE_MASK) | FIELD_MOVEMENT_SEQUENCE_CONTACT;
                        }

                        break;
                    case 1:
                        if ((part->effect_flags.word & FIELD_PART_SOLID) && !(slot->movement.word & FIELD_MOVEMENT_SEQUENCE_MASK) &&
                            ((slot->contact.bytes.target_count == 0) || (rec->motion_parameter == FIELD_ACTOR_COMMAND_TECHNIQUE)) &&
                            (((slot->action_parameter != FIELD_ACTION_PARAMETER_NONE) && (slot->action_parameter != 0)) ||
                             (actor->animation->hit_test_mode == 3)))
                        {
                            if (rec->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
                            {
                                dir->vx = (s8)item[1];
                                dir->vy = 0;
                                dir->vz = -(s8)*item;
                            }
                            else
                            {
                                dir->vx = (s8)item[1];
                                dir->vy = 0;
                                dir->vz = (s8)*item;
                            }

                            gte_ldv0(dir);
                            gte_rtv0();
                            gte_stlvnl(gte_out);
                            contact_quad[0].coord.x = sxy->x + gte_out->vx;
                            contact_quad[0].coord.y = sxy->y + gte_out->vy;

                            if (rec->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
                            {
                                dir->vx = (s8)item[3];
                                dir->vy = 0;
                                dir->vz = -(s8)item[2];
                            }
                            else
                            {
                                dir->vx = (s8)item[3];
                                dir->vy = 0;
                                dir->vz = (s8)item[2];
                            }

                            gte_ldv0(dir);
                            gte_rtv0();
                            gte_stlvnl(gte_out);
                            contact_quad[1].coord.x = sxy->x + gte_out->vx;
                            contact_quad[1].coord.y = sxy->y + gte_out->vy;

                            if (rec->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
                            {
                                dir->vx = (s8)item[5];
                                dir->vy = 0;
                                dir->vz = -(s8)item[4];
                            }
                            else
                            {
                                dir->vx = (s8)item[5];
                                dir->vy = 0;
                                dir->vz = (s8)item[4];
                            }

                            gte_ldv0(dir);
                            gte_rtv0();
                            gte_stlvnl(gte_out);
                            contact_quad[2].coord.x = sxy->x + gte_out->vx;
                            contact_quad[2].coord.y = sxy->y + gte_out->vy;

                            if (rec->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
                            {
                                dir->vx = (s8)item[8];
                                dir->vy = 0;
                                dir->vz = -(s8)item[6];
                            }
                            else
                            {
                                dir->vx = (s8)item[8];
                                dir->vy = 0;
                                dir->vz = (s8)item[6];
                            }

                            gte_ldv0(dir);
                            gte_rtv0();
                            gte_stlvnl(gte_out);
                            contact_quad[3].coord.x = sxy->x + gte_out->vx;
                            contact_quad[3].coord.y = sxy->y + gte_out->vy;
                            contact_result = field_test_quad_actor_contacts(contact_quad, rec, &contact);

                            if (contact_result == 1)
                            {
                                g_field_object_states[contact.actor].flags &= ~0x400;
                                slot->targets[slot->contact.bytes.target_count] = contact.actor;
                                target_count = slot->contact.bytes.target_count;

                                if (target_count < 9U)
                                {
                                    slot->contact.bytes.target_count = target_count + 1;
                                }

                                if (actor->animation->hit_test_mode == 3)
                                {
                                    actor->track_mask |= 1 << actor->target_count;
                                    actor->targets[actor->target_count] = contact.actor;
                                    target_screen->x = FIELD_SCREEN_X(g_field_actors[contact.actor].x);
                                    target_screen->y = FIELD_SCREEN_Y(g_field_actors[contact.actor].y, g_field_actors[contact.actor].z);

                                    if (g_field_actors[contact.actor].animation & FIELD_EFFECT_FACING_FLIPPED)
                                    {
                                        actor->track_offsets[actor->target_count].x = target_screen->x - contact.point.coord.x;
                                    }
                                    else
                                    {
                                        actor->track_offsets[actor->target_count].x = contact.point.coord.x - target_screen->x;
                                    }

                                    actor->track_offsets[actor->target_count].y = contact.point.coord.y - target_screen->y;
                                    actor->target_count++;
                                    g_field_object_states[contact.actor].contact.word |= FIELD_CONTACT_TARGETED;

                                    field_resolve_contact_hit(actor->owner_object_index, contact.actor);
                                    attach_x = contact.point.coord.x - sxy->x;
                                    layer = ~FIELD_MOVEMENT_SEQUENCE_MASK;
                                    placement_flags = slot->movement.word & layer;
                                    slot->attachment_points[3].x = attach_x;
                                    slot->attachment_points[2].x = attach_x;
                                    slot->attachment_points[1].x = attach_x;
                                    slot->attachment_points[0].x = attach_x;
                                    attach_y = contact.point.coord.y - sxy->y;
                                    placement_flags |= FIELD_MOVEMENT_SEQUENCE_CONTACT;
                                    slot->movement.word = placement_flags;
                                    slot->attachment_points[3].y = attach_y;
                                    slot->attachment_points[2].y = attach_y;
                                    slot->attachment_points[1].y = attach_y;
                                    slot->attachment_points[0].y = attach_y;
                                }
                                else
                                {
                                    if (slot->action_parameter == 0x24)
                                    {
                                        if (g_field_actors[contact.actor].presence != 0)
                                        {
                                            break;
                                        }

                                        slot->contact.word |= 2;
                                        slot->linked_object_index = contact.actor;
                                        g_field_object_states[contact.actor].flags |= 0x2000;
                                    }
                                    else
                                    {
                                        if ((slot->action_parameter == 0x59) || (slot->action_parameter == 0x66) || (slot->action_parameter == 0x2B))
                                        {
                                            if ((!((g_field_object_states[contact.actor].contact.word >> FIELD_CONTACT_ACTION_PENDING_BIT) & 1) &&
                                                 (bound_index = field_get_object_binding(contact.actor)->owner) == contact.actor &&
                                                 field_get_object_binding(bound_index)->state != 0) ||
                                                (g_field_object_states[contact.actor].contact.bytes.flags & 1))
                                            {
                                                slot->action_parameter = 0;
                                            }
                                            else
                                            {
                                                g_field_actors[contact.actor].animation &= 0x7F;

                                                if (contact.actor < 2)
                                                {
                                                    field_command_history_clear(contact.actor);
                                                    g_field_object_states[contact.actor].retry_count = 0;
                                                    g_field_actors[contact.actor].variant = 0;
                                                }
                                            }
                                        }
                                        else
                                        {
                                            target_state = &g_field_object_states[contact.actor];
                                            y_offset = 1;

                                            if (!(target_state->contact.bytes.flags & 1) ||
                                                ((g_field_actor_slots[target_state->contact.bytes.controller_index].active != 0) &&
                                                 (g_field_actor_slots[target_state->contact.bytes.controller_index].owner_object_index ==
                                                  rec->source_object_index)))
                                            {
                                                switch (rec->facing_or_reward_kind & FIELD_ANIMATION_INDEX_MASK)
                                                {
                                                    case FIELD_ANIMATION_AIR_ATTACK:
                                                        y_offset = field_resolve_object_hit(rec->source_object_index, contact.actor, 0x10);
                                                        break;
                                                    case FIELD_ANIMATION_AIR_POWER_ATTACK:
                                                        y_offset = field_resolve_object_hit(rec->source_object_index, contact.actor, 0x11);
                                                        break;
                                                    case FIELD_ABILITY_TACKLE:
                                                        y_offset = field_resolve_object_hit(rec->source_object_index, contact.actor, 0x19);
                                                        break;
                                                    case FIELD_ABILITY_SLIDE:
                                                        y_offset = field_resolve_object_hit(rec->source_object_index, contact.actor, 0x1A);
                                                        break;
                                                    default:
                                                        y_offset = field_resolve_contact_hit(rec->source_object_index, contact.actor);
                                                        break;
                                                }
                                            }

                                            if (contact.actor < 2)
                                            {
                                                if (!((u16)g_field_actors[contact.actor].control.word & 0x1FF))
                                                {
                                                    field_command_history_clear(contact.actor);
                                                }
                                            }

                                            if (y_offset == 1)
                                            {
                                                if ((slot->action_parameter == 1) || (slot->action_parameter == 3) || (slot->action_parameter == 0x10))
                                                {
                                                    slot->action_parameter = 0x1E;
                                                }
                                            }
                                        }
                                    }

                                    if (slot->action_parameter & FIELD_ACTION_PARAMETER_TRANSIENT_ACTOR)
                                    {
                                        scratch.target_index = contact.actor;

                                        if (rec->motion_parameter == FIELD_ACTOR_COMMAND_TECHNIQUE)
                                        {
                                            if (slot->contact.bytes.animation_actor_index != FIELD_ANIMATION_ACTOR_NONE)
                                            {
                                                g_field_object_states[contact.actor].contact.word |= FIELD_CONTACT_TARGETED;
                                                field_start_actor_animation(slot->contact.bytes.animation_actor_index, 1, (u8*)&scratch.target_index);
                                            }
                                        }
                                        else if (field_start_bound_action_animation(rec->source_object_index, 1, (u8*)&scratch.target_index,
                                                                                    slot->action_parameter) != 0)
                                        {
                                            slot->movement.word = (slot->movement.word & ~FIELD_MOVEMENT_SEQUENCE_MASK) | FIELD_MOVEMENT_SEQUENCE_CONTACT;
                                        }

                                        attach_x = contact.point.coord.x - sxy->x;
                                        slot->attachment_points[3].x = attach_x;
                                        slot->attachment_points[2].x = attach_x;
                                        slot->attachment_points[1].x = attach_x;
                                        slot->attachment_points[0].x = attach_x;
                                        attach_y = contact.point.coord.y - sxy->y;
                                        slot->attachment_points[3].y = attach_y;
                                        slot->attachment_points[2].y = attach_y;
                                        slot->attachment_points[1].y = attach_y;
                                        slot->attachment_points[0].y = attach_y;
                                    }
                                    else if (slot->action_parameter != 0)
                                    {
                                        x_offset = field_find_free_actor_slot(rec->source_object_index, 0);

                                        if (x_offset != FIELD_ACTOR_SLOT_NONE)
                                        {
                                            switch ((slot->contact.word >> 2) & 7)
                                            {
                                                case 2:
                                                    slot->action_parameter = 0x74;
                                                    break;
                                                case 4:
                                                    slot->action_parameter = 0x75;
                                                    break;
                                                default:
                                                    slot->action_parameter = slot->action_parameter;
                                                    break;
                                            }

                                            if (field_start_builtin_animation(rec->source_object_index, x_offset, slot->action_parameter) != 0)
                                            {
                                                slot->contact.bytes.animation_actor_index = x_offset;
                                                scratch.target_index = contact.actor;
                                                field_start_actor_animation(x_offset, 1, (u8*)&scratch.target_index);
                                                attach_x = contact.point.coord.x - sxy->x;
                                                slot->attachment_points[3].x = attach_x;
                                                slot->attachment_points[2].x = attach_x;
                                                slot->attachment_points[1].x = attach_x;
                                                slot->attachment_points[0].x = attach_x;
                                                attach_y = contact.point.coord.y - sxy->y;
                                                slot->attachment_points[3].y = attach_y;
                                                slot->attachment_points[2].y = attach_y;
                                                slot->attachment_points[1].y = attach_y;
                                                slot->attachment_points[0].y = attach_y;
                                            }
                                        }
                                    }

                                    slot->action_parameter = FIELD_ACTION_PARAMETER_NONE;
                                    slot->movement.word = (slot->movement.word & ~FIELD_MOVEMENT_SEQUENCE_MASK) | FIELD_MOVEMENT_SEQUENCE_CONTACT;
                                    break;
                                }
                            }
                            else if (contact_result == 2)
                            {
                                slot->action_parameter = 0x1E;
                                x_offset = field_find_free_actor_slot(rec->source_object_index, 0);

                                if (x_offset != FIELD_ACTOR_SLOT_NONE)
                                {
                                    if (field_start_builtin_animation(rec->source_object_index, x_offset, slot->action_parameter) != 0)
                                    {
                                        slot->contact.bytes.animation_actor_index = x_offset;
                                        scratch.target_index = contact.actor;
                                        field_start_actor_animation(x_offset, 1, (u8*)&scratch.target_index);
                                        attach_x = contact.point.coord.x - sxy->x;
                                        slot->attachment_points[3].x = attach_x;
                                        slot->attachment_points[2].x = attach_x;
                                        slot->attachment_points[1].x = attach_x;
                                        slot->attachment_points[0].x = attach_x;
                                        attach_y = contact.point.coord.y - sxy->y;
                                        slot->attachment_points[3].y = attach_y;
                                        slot->attachment_points[2].y = attach_y;
                                        slot->attachment_points[1].y = attach_y;
                                        slot->attachment_points[0].y = attach_y;
                                    }
                                }

                                slot->action_parameter = FIELD_ACTION_PARAMETER_NONE;
                                slot->movement.word = (slot->movement.word & ~FIELD_MOVEMENT_SEQUENCE_MASK) | FIELD_MOVEMENT_SEQUENCE_CONTACT;
                            }
                            else if (contact_result == 3)
                            {
                                slot->action_parameter = FIELD_ACTION_PARAMETER_NONE;
                                slot->movement.word = (slot->movement.word & ~FIELD_MOVEMENT_SEQUENCE_MASK) | FIELD_MOVEMENT_SEQUENCE_CONTACT;
                            }
                        }

                        break;
                    case 5:
                        if ((rec->frame_ticks == 0) && !(FIELD_FRAME_WORD(rec) & FIELD_FRAME_UNCHANGED))
                        {
                            sound_kind = g_field_resource_entries[rec->resource_index].attribute.sound_cue >> FIELD_SOUND_CUE_KIND_SHIFT;

                            if (sound_kind != FIELD_SOUND_CUE_SET_SFX)
                            {
                                if (sound_kind < 2)
                                {
                                    if (sound_kind == FIELD_SOUND_CUE_PLAY)
                                    {

                                        field_play_sound(g_field_resource_entries[rec->resource_index].attribute.sound_cue & FIELD_SOUND_CUE_ID_MASK,
                                                         field_get_actor_sound_pan(rec->source_object_index));
                                    }
                                }
                            }
                            else
                            {

                                field_play_set_sfx(g_field_resource_entries[rec->resource_index].attribute.sound_cue & FIELD_SOUND_CUE_ID_MASK,
                                                   field_get_actor_sound_pan(rec->source_object_index), rec->resource_index - FIELD_RESOURCE_VOICE_FIRST,
                                                   rec->source_object_index);
                            }
                        }

                        break;
                    case 2:
                        if (part->effect_flags.word & FIELD_PART_SOLID)
                        {
                            field_unpack_effect_quad_corners8(quad_bounds, rec->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED, (s8*)item);
                            shadow_count++;
                        }

                        break;
                    case 4:
                        if ((slot->movement.word & FIELD_MOVEMENT_SEQUENCE_MASK) != FIELD_MOVEMENT_SEQUENCE_LAUNCHED)
                        {
                            if ((slot->action_parameter != FIELD_ACTION_PARAMETER_NONE) && (slot->action_parameter != 0))
                            {
                                if (rec->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
                                {
                                    dir->vx = (s8)item[1];
                                    dir->vy = 0;
                                    dir->vz = -(s8)*item;
                                }
                                else
                                {
                                    dir->vx = (s8)item[1];
                                    dir->vy = 0;
                                    dir->vz = (s8)*item;
                                }

                                gte_ldv0(dir);
                                gte_rtv0();
                                gte_stlvnl(gte_out);
                                slot->attachment_points[0].x = gte_out->vx;
                                slot->attachment_points[0].y = gte_out->vy - vertical_lift;

                                if (rec->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
                                {
                                    dir->vx = (s8)item[3];
                                    dir->vy = 0;
                                    dir->vz = -(s8)item[2];
                                }
                                else
                                {
                                    dir->vx = (s8)item[3];
                                    dir->vy = 0;
                                    dir->vz = (s8)item[2];
                                }

                                gte_ldv0(dir);
                                gte_rtv0();
                                gte_stlvnl(gte_out);
                                slot->attachment_points[1].x = gte_out->vx;
                                slot->attachment_points[1].y = gte_out->vy - vertical_lift;

                                if (rec->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
                                {
                                    dir->vx = (s8)item[5];
                                    dir->vy = 0;
                                    dir->vz = -(s8)item[4];
                                }
                                else
                                {
                                    dir->vx = (s8)item[5];
                                    dir->vy = 0;
                                    dir->vz = (s8)item[4];
                                }

                                gte_ldv0(dir);
                                gte_rtv0();
                                gte_stlvnl(gte_out);
                                slot->attachment_points[2].x = gte_out->vx;
                                slot->attachment_points[2].y = gte_out->vy - vertical_lift;

                                if (rec->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
                                {
                                    dir->vx = (s8)item[8];
                                    dir->vy = 0;
                                    dir->vz = -(s8)item[6];
                                }
                                else
                                {
                                    dir->vx = (s8)item[8];
                                    dir->vy = 0;
                                    dir->vz = (s8)item[6];
                                }

                                gte_ldv0(dir);
                                gte_rtv0();
                                gte_stlvnl(gte_out);
                                slot->attachment_points[3].x = gte_out->vx;
                                slot->attachment_points[3].y = gte_out->vy - vertical_lift;

                                if (slot->action_parameter & FIELD_ACTION_PARAMETER_TRANSIENT_ACTOR)
                                {
                                    if (field_start_bound_action_animation(rec->source_object_index, 0, NULL, slot->action_parameter) != 0)
                                    {
                                        slot->movement.word = (slot->movement.word & ~FIELD_MOVEMENT_SEQUENCE_MASK) | FIELD_MOVEMENT_SEQUENCE_CONTACT;
                                    }
                                }
                                else
                                {
                                    x_offset = field_find_free_actor_slot(rec->source_object_index, 0);

                                    if (x_offset != FIELD_ACTOR_SLOT_NONE)
                                    {
                                        if (field_start_builtin_animation(rec->source_object_index, x_offset, slot->action_parameter) != 0)
                                        {
                                            slot->contact.bytes.animation_actor_index = x_offset;
                                            field_start_actor_animation(x_offset, 0, NULL);
                                        }
                                    }
                                }

                                slot->action_parameter = FIELD_ACTION_PARAMETER_NONE;
                                slot->movement.word = (slot->movement.word & ~FIELD_MOVEMENT_SEQUENCE_MASK) | FIELD_MOVEMENT_SEQUENCE_LAUNCHED;
                            }
                        }

                        break;
                    default:
                        break;
                }
            }
        }

        item += 9;
    }

    if (shadow_count != 0)
    {
        Vec2s* corner;

        for (corner = quad_bounds; corner != quad_bounds + 4; corner++)
        {
            corner->x = ((corner->x * part->appearance.fields.scale_xz >> FIELD_PART_SCALE_SHIFT) * g_field_effect_track_scale.x) >> 12;
            corner->y = ((corner->y * part->scale_y >> FIELD_PART_SCALE_SHIFT) * g_field_effect_track_scale.z) >> 12;
        }

        slot->collision.signed_half.center_offset = (quad_bounds[1].x + quad_bounds[0].x) >> 1;
        slot->collision.signed_half.extent = abs(quad_bounds[1].x - quad_bounds[0].x);

        if (slot->collision.signed_half.extent == 0)
        {
            slot->collision.signed_half.extent = abs(quad_bounds[2].x - quad_bounds[0].x);
            slot->collision.signed_half.center_offset = (quad_bounds[2].x + quad_bounds[0].x) >> 1;
        }

        cursor = (s32*)field_render_actor_ground_shadow((FieldActor*)rec, (POLY_FT4*)cursor, base, quad_bounds);
    }

    return cursor;
}

/**
 * @brief Draw one frame of an effect's 16-bit frame records and run its placement records.
 * @param effect Effect record being drawn.
 * @param packet_cursor Primitive buffer cursor; each sprite record writes one POLY_FT4.
 * @param ordering_table Ordering table the sprites are linked into by depth.
 * @param frame_data Frame data: a count byte, then 11-byte sprite or 17-byte placement records.
 * @param limit_placements Non-zero runs only ground-shadow placements, plus quad-vertex
 *        placements while an animation actor hides the source object.
 * @param part Part definition of the effect.
 * @return The advanced primitive cursor.
 * @note Placement records have FIELD_SPRITE_FRAME_FOOTPRINT set; the low nibble of the flag byte
 *       selects the kind: 0 and 3 effect quad vertices, 1 contact test, 2 ground shadow,
 *       4 and 6 attachment points and bound animation, 5 sound cue.
 * @see field_render_effect_frame8
 */
s32* field_render_effect_frame16(FieldMotionRecord* effect, s32* packet_cursor, s32* ordering_table, u8* frame_data, s32 limit_placements,
                                 FieldObjectPart* part)
{
    MATRIX* matrix = FIELD_EFFECT_SCRATCH_MATRIX;
    Vec2s* screen_origin = FIELD_EFFECT_SCRATCH_SCREEN_ORIGIN;
    s32 record_count;
    VECTOR* transformed = (VECTOR*)SCRATCHPAD_AT(0x44);
    SVECTOR* direction = (SVECTOR*)SCRATCHPAD_AT(0x54);
    Vec2s* corners = FIELD_EFFECT_SCRATCH_FOOTPRINT;
    FieldContactPoint* contact_quad;
    Vec2s* target_screen;
    FieldActorSlot* actor;
    s32 vertical_lift;
    s32 shadow_count;
    FieldContactResult contact;
    s32 targets[10];
    POLY_FT4* quad;
    s32 offset_x; /* Sprite x offset, later the bound animation actor; one local for both. */
    s32 contact_result;
    s32 tpage_x;
    s32 height;
    s32 offset_y; /* Sprite y offset, later the palette row; one local for both. */
    s32 depth;
    s32 corner_span;
    s32 layer;
    u16 screen_y;
    u32 sound_kind;
    u32 spawn_flags;
    u8 lift_end;
    s32 width;
    u8 lift_start;
    u8 frame_flags;
    s32 u_sum;
    s32 v_sum;
    u8 palette_selector;
    u8 u_end;
    u8 v_end;
    FieldObjectState* object_state;

    shadow_count = 0;
    contact_quad = (FieldContactPoint*)SCRATCHPAD_AT(0x80);
    target_screen = (Vec2s*)SCRATCHPAD_AT(0x94);
    object_state = &g_field_object_states[effect->source_object_index];
    actor = &g_field_actor_slots[effect->owner_slot];
    object_state->collision.word = 0;

    for (layer = 7; layer >= 0; layer--)
    {
        object_state->effect_vertices.words[layer] = 0;
    }

    object_state->bounds.words[1] = 0;
    object_state->bounds.words[0] = 0;
    field_build_effect_part_matrix(effect, part, matrix, actor);
    gte_SetRotMatrix(matrix);

    screen_origin->x = FIELD_SCREEN_X(effect->x);
    screen_y = FIELD_SCREEN_Y(effect->y, effect->z);
    screen_origin->y = screen_y;
    lift_start = effect->vertical_offset;
    lift_end = effect->next_height;

    if ((lift_start | lift_end) != 0)
    {
        vertical_lift = (s8)lift_start + (((s8)lift_end - (s8)lift_start) * effect->frame_ticks) / effect->frame_length;
        screen_origin->y = screen_y - vertical_lift;
    }
    else
    {
        vertical_lift = 0;
    }

    field_extract_effect_quad_corners16(effect, frame_data, corners);
    spawn_flags = part->spawn_flags.word;

    if (spawn_flags & 0x100000)
    {
        field_apply_effect_quad_center_offset(effect, screen_origin, corners, (uintptr_t)matrix, (spawn_flags >> 0x14) & 1);
    }

    object_state->bounds.half.left = corners[0].x;
    object_state->bounds.half.top = corners[0].y;
    object_state->bounds.half.right = corners[2].x;
    object_state->bounds.half.bottom = corners[2].y;

    quad = (POLY_FT4*)packet_cursor;

    for (record_count = *frame_data++; record_count != 0; record_count--)
    {
        frame_flags = frame_data[7];

        if (!(frame_flags & FIELD_SPRITE_FRAME_FOOTPRINT))
        {
            field_resolve_effect_part_color(actor, effect, part, (FieldPrimitiveColor*)&((P_TAG*)packet_cursor)->r0);
            setPolyFT4(quad);
            setSemiTrans(quad, effect->flags & FIELD_EFFECT_SEMITRANSPARENT);
            width = frame_data[4];
            height = frame_data[5] - 1;
            offset_y = (s16)(frame_data[1] + (frame_data[10] << 8));
            offset_x = (s16)(*frame_data + (frame_data[9] << 8));

            if (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
            {
                offset_x = -offset_x - width;
            }

            width--;
            field_project_effect_sprite_quad(effect, screen_origin, (POLY_FT4*)packet_cursor, width, height, offset_x, offset_y, (FieldSpriteFrame*)frame_data,
                                             matrix);

            if ((frame_data[7] ^ (effect->facing_or_reward_kind >> 1)) & FIELD_SPRITE_FRAME_FLIP_X)
            {
                quad->u3 = quad->u1 = frame_data[2];
                u_sum = quad->u1 + width;
                u_end = 0xFF;

                if (u_sum != 0x100)
                {
                    u_end = u_sum;
                }

                quad->u0 = quad->u2 = u_end;
            }
            else
            {
                quad->u2 = quad->u0 = frame_data[2];
                u_sum = quad->u0 + width;
                u_end = 0xFF;

                if (u_sum != 0x100)
                {
                    u_end = u_sum;
                }

                quad->u1 = quad->u3 = u_end;
            }

            if (frame_data[7] & FIELD_SPRITE_FRAME_FLIP_Y)
            {
                quad->v3 = quad->v2 = frame_data[3];
                v_sum = quad->v2 + height;
                v_end = 0xFF;

                if (v_sum != 0x100)
                {
                    v_end = v_sum;
                }

                quad->v0 = quad->v1 = v_end;
            }
            else
            {
                quad->v1 = quad->v0 = frame_data[3];
                v_sum = quad->v0 + height;
                v_end = 0xFF;

                if (v_sum != 0x100)
                {
                    v_end = v_sum;
                }

                quad->v2 = quad->v3 = v_end;
            }

            layer = effect->texture_slot;
            tpage_x = layer << 7;

            if (layer >= 2)
            {
                tpage_x = layer << 6;

                if (layer >= FIELD_LOWER_TEXTURE_FIRST_SLOT)
                {
                    s32 page_offset;
                    s32 page_x;
                    s32 page_bits;

                    page_offset = layer - FIELD_LOWER_TEXTURE_FIRST_SLOT;
                    page_offset <<= 6;
                    page_x = frame_data[7];
                    page_x &= 3;
                    page_x <<= 6;
                    page_x += FIELD_LOWER_TEXTURE_VRAM_X;
                    page_x -= page_offset;
                    page_x &= 0x3FF;
                    page_bits = part->behavior_flags.word;
                    page_x = (s32)page_x >> 6;
                    page_bits = (s32)((u32)page_bits >> FIELD_PART_BLEND_TPAGE_SHIFT);
                    page_bits &= GPU_TPAGE_BLEND_MASK;
                    page_bits |= 0x10;
                    page_bits |= page_x;
                    quad->tpage = page_bits;
                }
                else
                {
                    s32 page_x;
                    s32 page_abr;

                    page_x = frame_data[7];
                    page_x &= 3;
                    page_x <<= 6;
                    page_x += FIELD_COMPANION_TEXTURE_VRAM_X;
                    page_x -= tpage_x;
                    page_x = (page_x & (VRAM_WIDTH - 1)) >> 6;
                    page_abr = ((u32)part->behavior_flags.word >> FIELD_PART_BLEND_TPAGE_SHIFT) & GPU_TPAGE_BLEND_MASK;
                    quad->tpage = page_x | page_abr;
                }
            }
            else
            {
                s32 page_x;
                s32 page_abr;

                tpage_x = FIELD_PARTY_TEXTURE_VRAM_X - tpage_x;
                page_abr = getTPage(0, FIELD_PART_BLEND_MODE(part), 0, 0);

                if (frame_data[7] & 3)
                {
                    page_x = (tpage_x + FIELD_TEXTURE_COLUMN_WIDTH) & (VRAM_WIDTH - 1);
                }
                else
                {
                    page_x = tpage_x & (VRAM_WIDTH - 1);
                }

                quad->tpage = page_abr | (page_x >> 6);
            }

            if (FIELD_PART_PALETTE_SOURCE(part) == FIELD_PART_PALETTE_RESOURCE)
            {
                if (frame_data[6] == 0xB)
                {
                    setSemiTrans(quad, 1);
                }

                quad->clut = getClut(frame_data[6] << 4, effect->resource_index + FIELD_RESOURCE_CLUT_VRAM_Y);
            }
            else
            {
                palette_selector = part->appearance.fields.palette_selector;

                if (palette_selector >= FIELD_PALETTE_SELECTOR_SHARED)
                {
                    offset_y = FIELD_SHARED_CLUT_VRAM_Y;

                    if (actor->owner_object_index < 2)
                    {
                        offset_y = (actor->owner_object_index * 2) + FIELD_ACTOR_CLUT_VRAM_Y;
                    }
                }
                else
                {
                    offset_y = (palette_selector >> FIELD_PALETTE_SELECTOR_ROW_SHIFT) + FIELD_EFFECT_CLUT_VRAM_Y;
                }

                switch (FIELD_PART_PALETTE_SOURCE(part))
                {
                    case FIELD_PART_PALETTE_SELECTOR:
                        quad->clut = getClut((part->appearance.fields.palette_selector & FIELD_PALETTE_SELECTOR_COLUMN_MASK) << 4, offset_y);
                        break;
                    case FIELD_PART_PALETTE_FRAME:
                        if (actor->owner_object_index >= 3)
                        {
                            quad->clut = getClut((part->appearance.fields.palette_selector & FIELD_PALETTE_SELECTOR_COLUMN_MASK) << 4, offset_y);
                        }
                        else
                        {
                            if (frame_data[6] == 0xB)
                            {
                                setSemiTrans(quad, 1);
                            }

                            quad->clut = getClut(frame_data[6] << 4, effect->resource_index + FIELD_RESOURCE_CLUT_VRAM_Y);
                        }

                        break;
                }
            }

            depth = effect->z >> 7;

            if (depth < 0)
            {
                addPrim(ordering_table, packet_cursor);
                quad++;
                packet_cursor += sizeof(POLY_FT4) / sizeof(s32);
            }
            else if (depth >= FIELD_WORLD_OT_SIZE)
            {
                addPrim(&ordering_table[FIELD_WORLD_OT_LAST], packet_cursor);
                quad++;
                packet_cursor += sizeof(POLY_FT4) / sizeof(s32);
            }
            else
            {
                addPrim(&ordering_table[effect->z >> 7], packet_cursor);
                quad++;
                packet_cursor += sizeof(POLY_FT4) / sizeof(s32);
            }

            frame_data += 0xB;
        }
        else
        {

            if ((limit_placements == 0) || ((frame_flags & 0xF) == 2) ||
                ((object_state->contact.word & FIELD_CONTACT_ANIMATION_HIDDEN) && ((frame_flags & 0xF) == 0)))
            {
                switch (frame_data[7] & 0xF)
                {
                    case 0:
                        field_transform_effect_quad_vertices16(effect, object_state, frame_data, 0, screen_origin, direction, transformed);
                        break;
                    case 3:
                        field_transform_effect_quad_vertices16(effect, object_state, frame_data, 4, screen_origin, direction, transformed);
                        break;
                    case 1:
                        if (part->effect_flags.word & FIELD_PART_SOLID)
                        {
                            if (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
                            {
                                direction->vx = frame_data[2] + (frame_data[3] << 8);
                                direction->vy = 0;
                                direction->vz = -(*frame_data + (frame_data[1] << 8));
                            }
                            else
                            {
                                direction->vx = frame_data[2] + (frame_data[3] << 8);
                                direction->vy = 0;
                                direction->vz = *frame_data + (frame_data[1] << 8);
                            }

                            gte_ldv0(direction);
                            gte_rtv0();
                            gte_stlvnl(transformed);
                            contact_quad[0].coord.x = screen_origin->x + transformed->vx;
                            contact_quad[0].coord.y = screen_origin->y + transformed->vy;

                            if (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
                            {
                                direction->vx = frame_data[6] + (frame_data[8] << 8);
                                direction->vy = 0;
                                direction->vz = -(frame_data[4] + (frame_data[5] << 8));
                            }
                            else
                            {
                                direction->vx = frame_data[6] + (frame_data[8] << 8);
                                direction->vy = 0;
                                direction->vz = frame_data[4] + (frame_data[5] << 8);
                            }

                            gte_ldv0(direction);
                            gte_rtv0();
                            gte_stlvnl(transformed);
                            contact_quad[1].coord.x = screen_origin->x + transformed->vx;
                            contact_quad[1].coord.y = screen_origin->y + transformed->vy;

                            if (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
                            {
                                direction->vx = frame_data[11] + (frame_data[12] << 8);
                                direction->vy = 0;
                                direction->vz = -(frame_data[9] + (frame_data[10] << 8));
                            }
                            else
                            {
                                direction->vx = frame_data[11] + (frame_data[12] << 8);
                                direction->vy = 0;
                                direction->vz = frame_data[9] + (frame_data[10] << 8);
                            }

                            gte_ldv0(direction);
                            gte_rtv0();
                            gte_stlvnl(transformed);
                            contact_quad[2].coord.x = screen_origin->x + transformed->vx;
                            contact_quad[2].coord.y = screen_origin->y + transformed->vy;

                            if (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
                            {
                                direction->vx = frame_data[15] + (frame_data[16] << 8);
                                direction->vy = 0;
                                direction->vz = -(frame_data[13] + (frame_data[14] << 8));
                            }
                            else
                            {
                                direction->vx = frame_data[15] + (frame_data[16] << 8);
                                direction->vy = 0;
                                direction->vz = frame_data[13] + (frame_data[14] << 8);
                            }

                            gte_ldv0(direction);
                            gte_rtv0();
                            gte_stlvnl(transformed);
                            contact_quad[3].coord.x = screen_origin->x + transformed->vx;
                            contact_quad[3].coord.y = screen_origin->y + transformed->vy;

                            if (!(object_state->movement.word & FIELD_MOVEMENT_SEQUENCE_MASK) &&
                                (((object_state->action_parameter != FIELD_ACTION_PARAMETER_NONE) && (object_state->action_parameter != 0)) ||
                                 (actor->animation->hit_test_mode == 3)))
                            {
                                contact_result = field_test_quad_actor_contacts(contact_quad, effect, &contact);

                                if (contact_result == 1)
                                {
                                    g_field_object_states[contact.actor].flags &= ~0x400;
                                    object_state->targets[object_state->contact.bytes.target_count] = contact.actor;

                                    if (object_state->contact.bytes.target_count < 9)
                                    {
                                        object_state->contact.bytes.target_count++;
                                    }

                                    if (actor->animation->hit_test_mode == 3)
                                    {
                                        actor->track_mask |= 1 << actor->target_count;
                                        actor->targets[actor->target_count] = contact.actor;
                                        target_screen->x = FIELD_SCREEN_X(g_field_actors[contact.actor].x);
                                        target_screen->y = FIELD_SCREEN_Y(g_field_actors[contact.actor].y, g_field_actors[contact.actor].z);

                                        if (g_field_actors[contact.actor].animation & FIELD_EFFECT_FACING_FLIPPED)
                                        {
                                            actor->track_offsets[actor->target_count].x = target_screen->x - contact.point.coord.x;
                                        }
                                        else
                                        {
                                            actor->track_offsets[actor->target_count].x = contact.point.coord.x - target_screen->x;
                                        }

                                        actor->track_offsets[actor->target_count].y = contact.point.coord.y - target_screen->y;
                                        actor->target_count++;
                                        field_resolve_contact_hit(actor->owner_object_index, contact.actor);
                                        object_state->attachment_points[0].x = object_state->attachment_points[1].x = object_state->attachment_points[2].x =
                                            object_state->attachment_points[3].x = contact.point.coord.x - screen_origin->x;
                                        object_state->attachment_points[0].y = object_state->attachment_points[1].y = object_state->attachment_points[2].y =
                                            object_state->attachment_points[3].y = contact.point.coord.y - screen_origin->y;
                                        object_state->movement.bits.sequence = FIELD_MOVEMENT_SEQUENCE_CONTACT >> FIELD_MOVEMENT_SEQUENCE_SHIFT;
                                    }
                                    else
                                    {
                                        if ((field_resolve_contact_hit(effect->source_object_index, contact.actor) == 1) &&
                                            ((object_state->action_parameter == 1) || (object_state->action_parameter == 3) ||
                                             (object_state->action_parameter == 0x10)))
                                        {
                                            object_state->action_parameter = 0x1E;
                                        }

                                        if (object_state->action_parameter & FIELD_ACTION_PARAMETER_TRANSIENT_ACTOR)
                                        {
                                            targets[0] = contact.actor;

                                            if (field_start_bound_action_animation(effect->source_object_index, 1, (u8*)targets,
                                                                                   object_state->action_parameter) != 0)
                                            {
                                                object_state->movement.bits.sequence = FIELD_MOVEMENT_SEQUENCE_CONTACT >> FIELD_MOVEMENT_SEQUENCE_SHIFT;
                                            }
                                        }
                                        else
                                        {
                                            offset_x = field_find_free_actor_slot(effect->source_object_index, 0);

                                            if (offset_x != FIELD_ACTOR_SLOT_NONE)
                                            {
                                                if (field_start_builtin_animation(effect->source_object_index, offset_x, object_state->action_parameter) != 0)
                                                {
                                                    object_state->contact.bytes.animation_actor_index = offset_x;
                                                    targets[0] = contact.actor;
                                                    field_start_actor_animation(offset_x, 1, (u8*)targets);
                                                    object_state->attachment_points[0].x = object_state->attachment_points[1].x =
                                                        object_state->attachment_points[2].x = object_state->attachment_points[3].x =
                                                            contact.point.coord.x - screen_origin->x;
                                                    object_state->attachment_points[0].y = object_state->attachment_points[1].y =
                                                        object_state->attachment_points[2].y = object_state->attachment_points[3].y =
                                                            contact.point.coord.y - screen_origin->y;
                                                }
                                            }
                                        }

                                        object_state->action_parameter = FIELD_ACTION_PARAMETER_NONE;
                                        object_state->movement.bits.sequence = FIELD_MOVEMENT_SEQUENCE_CONTACT >> FIELD_MOVEMENT_SEQUENCE_SHIFT;
                                    }
                                }
                                else if (contact_result == 2)
                                {
                                    object_state->action_parameter = 0x1E;
                                    offset_x = field_find_free_actor_slot(effect->source_object_index, 0);

                                    if (offset_x != FIELD_ACTOR_SLOT_NONE)
                                    {
                                        if (field_start_builtin_animation(effect->source_object_index, offset_x, object_state->action_parameter) != 0)
                                        {
                                            object_state->contact.bytes.animation_actor_index = offset_x;
                                            targets[0] = contact.actor;
                                            field_start_actor_animation(offset_x, 1, (u8*)targets);
                                            object_state->attachment_points[0].x = object_state->attachment_points[1].x = object_state->attachment_points[2].x =
                                                object_state->attachment_points[3].x = contact.point.coord.x - screen_origin->x;
                                            object_state->attachment_points[0].y = object_state->attachment_points[1].y = object_state->attachment_points[2].y =
                                                object_state->attachment_points[3].y = contact.point.coord.y - screen_origin->y;
                                        }
                                    }

                                    object_state->action_parameter = FIELD_ACTION_PARAMETER_NONE;
                                    object_state->movement.bits.sequence = FIELD_MOVEMENT_SEQUENCE_CONTACT >> FIELD_MOVEMENT_SEQUENCE_SHIFT;
                                }
                                else if (contact_result == 3)
                                {
                                    object_state->action_parameter = FIELD_ACTION_PARAMETER_NONE;
                                    object_state->movement.bits.sequence = FIELD_MOVEMENT_SEQUENCE_CONTACT >> FIELD_MOVEMENT_SEQUENCE_SHIFT;
                                }
                            }
                        }

                        break;
                    case 6:
                        if (!(object_state->action_parameter & FIELD_ACTION_PARAMETER_TRANSIENT_ACTOR) &&
                            (object_state->action_parameter != FIELD_ACTION_PARAMETER_NONE) &&
                            ((effect->motion_parameter == FIELD_ACTOR_COMMAND_TECHNIQUE) || (effect->motion_parameter == FIELD_ACTOR_COMMAND_ACTION) ||
                             (effect->motion_parameter == FIELD_ACTOR_COMMAND_RECOVER) || (effect->motion_parameter == FIELD_ACTOR_COMMAND_98)) &&
                            !(FIELD_FRAME_WORD(effect) & FIELD_FRAME_UNCHANGED))
                        {
                            offset_x = field_find_free_actor_slot(effect->source_object_index, 0);

                            if ((offset_x != FIELD_ACTOR_SLOT_NONE) &&
                                (field_start_builtin_animation(effect->source_object_index, offset_x, object_state->action_parameter) != 0))
                            {
                                object_state->contact.bytes.animation_actor_index = offset_x;
                                field_start_actor_animation(offset_x, 0, NULL);

                                if (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
                                {
                                    direction->vx = frame_data[2] + (frame_data[3] << 8);
                                    direction->vy = 0;
                                    direction->vz = -(*frame_data + (frame_data[1] << 8));
                                }
                                else
                                {
                                    direction->vx = frame_data[2] + (frame_data[3] << 8);
                                    direction->vy = 0;
                                    direction->vz = *frame_data + (frame_data[1] << 8);
                                }

                                gte_ldv0(direction);
                                gte_rtv0();
                                gte_stlvnl(transformed);
                                object_state->attachment_points[0].x = transformed->vx;
                                object_state->attachment_points[0].y = transformed->vy;

                                if (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
                                {
                                    direction->vx = frame_data[6] + (frame_data[8] << 8);
                                    direction->vy = 0;
                                    direction->vz = -(frame_data[4] + (frame_data[5] << 8));
                                }
                                else
                                {
                                    direction->vx = frame_data[6] + (frame_data[8] << 8);
                                    direction->vy = 0;
                                    direction->vz = frame_data[4] + (frame_data[5] << 8);
                                }

                                gte_ldv0(direction);
                                gte_rtv0();
                                gte_stlvnl(transformed);
                                object_state->attachment_points[1].x = transformed->vx;
                                object_state->attachment_points[1].y = transformed->vy;

                                if (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
                                {
                                    direction->vx = frame_data[11] + (frame_data[12] << 8);
                                    direction->vy = 0;
                                    direction->vz = -(frame_data[9] + (frame_data[10] << 8));
                                }
                                else
                                {
                                    direction->vx = frame_data[11] + (frame_data[12] << 8);
                                    direction->vy = 0;
                                    direction->vz = frame_data[9] + (frame_data[10] << 8);
                                }

                                gte_ldv0(direction);
                                gte_rtv0();
                                gte_stlvnl(transformed);
                                object_state->attachment_points[2].x = transformed->vx;
                                object_state->attachment_points[2].y = transformed->vy;

                                if (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
                                {
                                    direction->vx = frame_data[15] + (frame_data[16] << 8);
                                    direction->vy = 0;
                                    direction->vz = -(frame_data[13] + (frame_data[14] << 8));
                                }
                                else
                                {
                                    direction->vx = frame_data[15] + (frame_data[16] << 8);
                                    direction->vy = 0;
                                    direction->vz = frame_data[13] + (frame_data[14] << 8);
                                }

                                gte_ldv0(direction);
                                gte_rtv0();
                                gte_stlvnl(transformed);
                                object_state->attachment_points[3].x = transformed->vx;
                                object_state->attachment_points[3].y = transformed->vy;
                            }

                            object_state->movement.bits.sequence = FIELD_MOVEMENT_SEQUENCE_CONTACT >> FIELD_MOVEMENT_SEQUENCE_SHIFT;
                        }

                        break;
                    case 5:
                        if ((effect->frame_ticks == 0) && !(FIELD_FRAME_WORD(effect) & FIELD_FRAME_UNCHANGED))
                        {
                            FieldResourceEntry* resources = g_field_resource_entries;
                            u8 idx = effect->resource_index;
                            sound_kind = (u16)resources[idx].attribute.sound_cue >> FIELD_SOUND_CUE_KIND_SHIFT;

                            if (sound_kind != FIELD_SOUND_CUE_SET_SFX)
                            {
                                if ((s32)sound_kind < 2)
                                {
                                    if (sound_kind == FIELD_SOUND_CUE_PLAY)
                                    {
                                        field_play_sound(resources[effect->resource_index].attribute.sound_cue & FIELD_SOUND_CUE_ID_MASK,
                                                         field_get_actor_sound_pan(effect->source_object_index));
                                    }
                                }
                            }
                            else
                            {
                                field_play_set_sfx(resources[effect->resource_index].attribute.sound_cue & FIELD_SOUND_CUE_ID_MASK,
                                                   field_get_actor_sound_pan(effect->source_object_index), effect->resource_index - FIELD_RESOURCE_VOICE_FIRST,
                                                   effect->source_object_index);
                            }
                        }

                        break;
                    case 2:
                        if (part->effect_flags.word & FIELD_PART_SOLID)
                        {
                            field_unpack_effect_quad_corners16(corners, effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED, frame_data);
                            shadow_count++;
                        }

                        break;
                    case 4:
                        if ((object_state->action_parameter != FIELD_ACTION_PARAMETER_NONE) && !(object_state->movement.word & FIELD_MOVEMENT_SEQUENCE_MASK))
                        {
                            if (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
                            {
                                direction->vx = frame_data[2] + (frame_data[3] << 8);
                                direction->vy = 0;
                                direction->vz = -(*frame_data + (frame_data[1] << 8));
                            }
                            else
                            {
                                direction->vx = frame_data[2] + (frame_data[3] << 8);
                                direction->vy = 0;
                                direction->vz = *frame_data + (frame_data[1] << 8);
                            }

                            gte_ldv0(direction);
                            gte_rtv0();
                            gte_stlvnl(transformed);
                            object_state->attachment_points[0].x = transformed->vx;
                            object_state->attachment_points[0].y = transformed->vy - vertical_lift;

                            if (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
                            {
                                direction->vx = frame_data[6] + (frame_data[8] << 8);
                                direction->vy = 0;
                                direction->vz = -(frame_data[4] + (frame_data[5] << 8));
                            }
                            else
                            {
                                direction->vx = frame_data[6] + (frame_data[8] << 8);
                                direction->vy = 0;
                                direction->vz = frame_data[4] + (frame_data[5] << 8);
                            }

                            gte_ldv0(direction);
                            gte_rtv0();
                            gte_stlvnl(transformed);
                            object_state->attachment_points[1].x = transformed->vx;
                            object_state->attachment_points[1].y = transformed->vy - vertical_lift;

                            if (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
                            {
                                direction->vx = frame_data[11] + (frame_data[12] << 8);
                                direction->vy = 0;
                                direction->vz = -(frame_data[9] + (frame_data[10] << 8));
                            }
                            else
                            {
                                direction->vx = frame_data[11] + (frame_data[12] << 8);
                                direction->vy = 0;
                                direction->vz = frame_data[9] + (frame_data[10] << 8);
                            }

                            gte_ldv0(direction);
                            gte_rtv0();
                            gte_stlvnl(transformed);
                            object_state->attachment_points[2].x = transformed->vx;
                            object_state->attachment_points[2].y = transformed->vy - vertical_lift;

                            if (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
                            {
                                direction->vx = frame_data[15] + (frame_data[16] << 8);
                                direction->vy = 0;
                                direction->vz = -(frame_data[13] + (frame_data[14] << 8));
                            }
                            else
                            {
                                direction->vx = frame_data[15] + (frame_data[16] << 8);
                                direction->vy = 0;
                                direction->vz = frame_data[13] + (frame_data[14] << 8);
                            }

                            gte_ldv0(direction);
                            gte_rtv0();
                            gte_stlvnl(transformed);
                            object_state->attachment_points[3].x = transformed->vx;
                            object_state->attachment_points[3].y = transformed->vy - vertical_lift;

                            if (object_state->action_parameter & FIELD_ACTION_PARAMETER_TRANSIENT_ACTOR)
                            {
                                if (field_start_bound_action_animation(effect->source_object_index, 0, NULL, object_state->action_parameter) != 0)
                                {
                                    object_state->movement.bits.sequence = FIELD_MOVEMENT_SEQUENCE_CONTACT >> FIELD_MOVEMENT_SEQUENCE_SHIFT;
                                }
                            }
                            else
                            {
                                offset_x = field_find_free_actor_slot(effect->source_object_index, 0);

                                if (offset_x != FIELD_ACTOR_SLOT_NONE)
                                {
                                    if (field_start_builtin_animation(effect->source_object_index, offset_x, object_state->action_parameter) != 0)
                                    {
                                        object_state->contact.bytes.animation_actor_index = offset_x;
                                        field_start_actor_animation(offset_x, 0, NULL);
                                    }
                                }
                            }

                            object_state->action_parameter = FIELD_ACTION_PARAMETER_NONE;
                            object_state->movement.bits.sequence = FIELD_MOVEMENT_SEQUENCE_CONTACT >> FIELD_MOVEMENT_SEQUENCE_SHIFT;
                        }

                        break;
                }
            }

            frame_data += 0x11;
        }
    }

    if (shadow_count != 0)
    {
        Vec2s* p = corners;

        while (p != corners + 4)
        {
            p->x = ((p->x * part->appearance.fields.scale_xz >> FIELD_PART_SCALE_SHIFT) * g_field_effect_track_scale.x) >> 12;
            p->y = ((p->y * part->scale_y >> FIELD_PART_SCALE_SHIFT) * g_field_effect_track_scale.z) >> 12;
            p++;
        }

        corner_span = abs(corners[1].x - corners[0].x);
        object_state->collision.signed_half.extent = (corner_span * 7) / 10;
        object_state->collision.signed_half.center_offset = (corners[1].x + corners[0].x) >> 1;
        packet_cursor = (s32*)field_render_actor_ground_shadow((FieldActor*)effect, (POLY_FT4*)packet_cursor, ordering_table, corners);
    }

    return packet_cursor;
}

/**
 * @brief Emit a 32-segment shaded ring, using three Gouraud triangles per segment.
 * @param effect Effect position, rendering flags, and part selection.
 * @param packet_cursor Destination GPU packet buffer.
 * @param ordering_table Depth-indexed ordering table.
 * @return Packet cursor after the geometry and texture-page command.
 */
u8* field_render_effect_ring(FieldMotionRecord* effect, u8* packet_cursor, s32* ordering_table)
{
    FieldScreenPair screen_origin;
    SVECTOR direction;
    VECTOR transformed;
    MATRIX matrix;
    FieldObjectPart* part;
    SVECTOR* direction_ptr;
    VECTOR* transformed_ptr;
    s32 next_angle;
    s16 rim_y;
    s16 inner_start_x;
    s16 inner_start_y;
    s16 inner_end_x;
    s16 outer_start_x;
    s16 outer_y;
    FieldGouraudTriangle* next_packet;
    FieldActorSlot* actor;
    s32 scaled_cosine;
    s32 address_mask;
    s32 length_mask;
    s32 outer_next_angle;
    s32 packed_color;
    s32 second_depth;
    s32 third_depth;
    s32 packet_tag;
    s32 first_depth;
    s32 camera_x;
    s32 inner_angle;
    s32 outer_angle;
    s32 segment;
    s32 next_scaled_cosine;
    s32 angle;
    s32 outer_cosine;
    s32 radius;
    FieldGouraudTriangle* second_triangle;
    FieldGouraudTriangle* packets;

    packets = (FieldGouraudTriangle*)packet_cursor;
    part = &g_field_actor_slots[effect->owner_slot].parts[effect->owner_part];
    actor = &g_field_actor_slots[effect->owner_slot];
    field_build_effect_part_matrix(effect, part, &matrix, actor);
    gte_SetRotMatrix(&matrix);

    camera_x = g_field_view_offset_x / 256;
    screen_origin.x = (camera_x + (effect->x / 256 + FIELD_SCREEN_CENTER_X));
    screen_origin.y = FIELD_SCREEN_Y(effect->y, effect->z);
    field_resolve_effect_part_color(actor, effect, part, &packets->color0);
    setPolyG3(packets);
    setSemiTrans(packets, effect->flags & FIELD_EFFECT_SEMITRANSPARENT);
    segment = 0;
    /* This render mode reuses animation_active as the inner-radius scale. */
    radius = effect->animation_active;
    direction_ptr = &direction;
    transformed_ptr = &transformed;
    address_mask = FIELD_GPU_ADDRESS_MASK;
    length_mask = FIELD_GPU_LENGTH_MASK;
    angle = segment;

    for (; 1; segment++)
    {
        next_angle = (segment + 1) << 7;
        packets[0].xy1.unsigned_pair.x = screen_origin.x;
        packets[0].xy1.unsigned_pair.y = screen_origin.y;
        scaled_cosine = (rcos(angle) >> 6) * radius;
        direction.vy = 0;
        direction.vx = (scaled_cosine >> 8);
        direction.vz = (((rsin(angle) >> 6) * radius) >> 8);
        gte_ldv0(direction_ptr);
        gte_rtv0();
        gte_stlvnl(transformed_ptr);
        inner_start_x = screen_origin.x + transformed.vx;
        packets[0].xy0.signed_pair.x = inner_start_x;
        packets[1].xy0.signed_pair.x = inner_start_x;
        inner_start_y = screen_origin.y + transformed.vy;
        packets[0].xy0.signed_pair.y = inner_start_y;
        packets[1].xy0.signed_pair.y = inner_start_y;

        if (segment == (FIELD_RING_SEGMENTS - 1))
        {
            next_scaled_cosine = (rcos(0) >> 6) * radius;
            inner_angle = 0;
            direction.vy = 0;
            direction.vx = (next_scaled_cosine >> 8);
            direction.vz = (((rsin(inner_angle) >> 6) * radius) >> 8);
        }
        else
        {
            next_scaled_cosine = (rcos(next_angle) >> 6) * radius;
            inner_angle = next_angle;
            direction.vy = 0;
            direction.vx = (next_scaled_cosine >> 8);
            direction.vz = (((rsin(inner_angle) >> 6) * radius) >> 8);
        }

        gte_ldv0(direction_ptr);
        gte_rtv0();
        gte_stlvnl(transformed_ptr);
        second_triangle = &packets[1];
        inner_end_x = screen_origin.x + transformed.vx;
        packets[0].xy2.signed_pair.x = inner_end_x;
        second_triangle[0].xy2.signed_pair.x = inner_end_x;
        packets[2].xy0.signed_pair.x = inner_end_x;
        rim_y = screen_origin.y + transformed.vy;
        packets[0].xy2.signed_pair.y = rim_y;
        second_triangle[0].xy2.signed_pair.y = rim_y;
        packets[2].xy0.signed_pair.y = rim_y;
        direction.vx = (rcos(angle) >> 6);
        direction.vy = 0;
        direction.vz = (rsin(angle) >> 6);
        gte_ldv0(direction_ptr);
        gte_rtv0();
        gte_stlvnl(transformed_ptr);
        outer_start_x = screen_origin.x + transformed.vx;
        second_triangle[0].xy1.signed_pair.x = outer_start_x;
        packets[2].xy1.signed_pair.x = outer_start_x;
        outer_y = screen_origin.y + transformed.vy;
        second_triangle[0].xy1.signed_pair.y = outer_y;
        packets[2].xy1.signed_pair.y = outer_y;

        if (segment == (FIELD_RING_SEGMENTS - 1))
        {
            outer_cosine = rcos(0);
            outer_angle = 0;
            direction.vx = (outer_cosine >> 6);
            direction.vy = 0;
            direction.vz = (rsin(outer_angle) >> 6);
        }
        else
        {
            outer_next_angle = angle + FIELD_RING_ANGLE_STEP;
            outer_cosine = rcos(outer_next_angle);
            outer_angle = outer_next_angle;
            direction.vx = (outer_cosine >> 6);
            direction.vy = 0;
            direction.vz = (rsin(outer_angle) >> 6);
        }

        gte_ldv0(direction_ptr);
        gte_rtv0();
        gte_stlvnl(transformed_ptr);
        outer_y = (screen_origin.x + transformed.vx);
        packets[2].xy2.signed_pair.x = outer_y;
        packed_color = packets[0].color0.signed_word;
        rim_y = (screen_origin.y + transformed.vy);
        packet_tag = packets[0].tag;
        packets[0].color1.signed_word = 0;
        packets[2].color2.signed_word = 0;
        packets[2].color1.signed_word = 0;
        packets[1].color1.signed_word = 0;
        packets[2].color0.signed_word = packed_color;
        packets[1].color2.signed_word = packed_color;
        packets[1].color0.signed_word = packed_color;
        packets[0].color2.signed_word = packed_color;
        packets[1].tag = packet_tag;
        packets[2].tag = packet_tag;
        packets[3].tag = packet_tag;
        packets[3].color0.signed_word = packets[0].color0.signed_word;
        packets[2].xy2.signed_pair.y = rim_y;
        first_depth = effect->z >> FIELD_WORLD_OT_DEPTH_SHIFT;
        next_packet = &packets[1];

        if (first_depth < 0)
        {
            FIELD_LINK_PACKET(&ordering_table[0], packets, address_mask, length_mask);
            packets = next_packet;
        }
        else if (first_depth >= FIELD_WORLD_OT_SIZE)
        {
            FIELD_LINK_PACKET(&ordering_table[FIELD_WORLD_OT_LAST], packets, address_mask, length_mask);
            packets = next_packet;
        }
        else
        {
            FIELD_LINK_PACKET(&ordering_table[effect->z >> FIELD_WORLD_OT_DEPTH_SHIFT], packets, address_mask, length_mask);
            packets = next_packet;
        }

        second_depth = effect->z >> FIELD_WORLD_OT_DEPTH_SHIFT;

        if (second_depth < 0)
        {
            FIELD_LINK_PACKET(&ordering_table[0], packets, address_mask, length_mask);
            packets++;
        }
        else if (second_depth >= FIELD_WORLD_OT_SIZE)
        {
            FIELD_LINK_PACKET(&ordering_table[FIELD_WORLD_OT_LAST], packets, address_mask, length_mask);
            packets++;
        }
        else
        {
            FIELD_LINK_PACKET(&ordering_table[effect->z >> FIELD_WORLD_OT_DEPTH_SHIFT], packets, address_mask, length_mask);
            packets++;
        }

        third_depth = effect->z >> FIELD_WORLD_OT_DEPTH_SHIFT;

        if (third_depth < 0)
        {
            FIELD_LINK_PACKET(&ordering_table[0], packets, address_mask, length_mask);
            packets++;
        }
        else if (third_depth >= FIELD_WORLD_OT_SIZE)
        {
            FIELD_LINK_PACKET(&ordering_table[FIELD_WORLD_OT_LAST], packets, address_mask, length_mask);
            packets++;
        }
        else
        {
            FIELD_LINK_PACKET(&ordering_table[effect->z >> FIELD_WORLD_OT_DEPTH_SHIFT], packets, address_mask, length_mask);
            packets++;
        }

        angle += FIELD_RING_ANGLE_STEP;

        if (segment == (FIELD_RING_SEGMENTS - 1))
        {
            break;
        }
    }

    return field_emit_effect_texture_page(effect, part, (u8*)packets, ordering_table);
}

/**
 * @brief Set a horizontal direction vector from an angle.
 * @param destination Receives (cos, 0, sin) of @p phase.
 * @param phase Angle in 4096ths of a turn.
 * @param shift Right shift applied to the cosine and sine.
 */
static inline void field_set_fan_direction(SVECTOR* destination, s32 phase, s32 shift)
{
    destination->vx = (s16)(rcos(phase) >> shift);
    destination->vy = 0;
    destination->vz = (s16)(rsin(phase) >> shift);
}

/**
 * @brief Emit a shaded circular fan with two to 32 segments and four triangles per segment.
 * @param effect Effect position, rendering flags, and part selection.
 * @param packet_cursor Destination GPU packet buffer.
 * @param ordering_table Depth-indexed ordering table.
 * @return Packet cursor after the geometry and texture-page command.
 */
u8* field_render_effect_fan(FieldMotionRecord* effect, u8* packet_cursor, s32* ordering_table)
{
    SVECTOR screen_origin;
    SVECTOR direction;
    VECTOR transformed;
    MATRIX matrix;
    FieldObjectPart* part;
    FieldActorSlot* actor;
    FieldGouraudTriangle* packets;
    FieldGouraudTriangle* second_triangle;
    FieldGouraudTriangle* third_triangle;
    FieldGouraudTriangle* fourth_triangle;
    FieldGouraudTriangle* next_packet;
    s32 segment_count;
    s32 segment;
    s32 quotient;
    s32 angle_step;
    s32 half_step;
    s32 next_half_angle;
    s32 color;
    s32 tag;
    s32 depth;
    s16 x;
    s16 y;

    packets = (FieldGouraudTriangle*)packet_cursor;
    part = &g_field_actor_slots[effect->owner_slot].parts[effect->owner_part];
    actor = &g_field_actor_slots[effect->owner_slot];
    field_build_effect_part_matrix(effect, part, &matrix, actor);
    gte_SetRotMatrix(&matrix);

    screen_origin.vx = FIELD_SCREEN_X(effect->x);
    screen_origin.vy = FIELD_SCREEN_Y(effect->y, effect->z);
    field_resolve_effect_part_color(actor, effect, part, &packets->color0);
    setPolyG3(packets);
    setSemiTrans(packets, effect->flags & FIELD_EFFECT_SEMITRANSPARENT);
    segment_count = 2;

    /* This part byte selects the fan segment count; other uses are unresolved. */
    if ((part->effect_animation_active >= 2) && (part->effect_animation_active <= FIELD_RING_SEGMENTS))
    {
        segment_count = part->effect_animation_active;
    }

    quotient = ONE / segment_count;
    angle_step = quotient;

    if ((ONE % segment_count) != 0)
    {
        angle_step++;
    }

    half_step = quotient >> 1;

    for (segment = 0;; segment++)
    {
        /* Copy the two screen coordinates as one GPU word. */
        packets[0].xy1.word = *(s32*)&screen_origin;
        field_set_fan_direction(&direction, segment * angle_step, 8);
        gte_ldv0(&direction);
        gte_rtv0();
        gte_stlvnl(&transformed);
        x = screen_origin.vx + transformed.vx;
        packets[0].xy0.signed_pair.x = x;
        packets[1].xy0.signed_pair.x = x;
        y = screen_origin.vy + transformed.vy;
        packets[0].xy0.signed_pair.y = y;
        packets[1].xy0.signed_pair.y = y;

        if (segment == segment_count - 1)
        {
            field_set_fan_direction(&direction, 0, 8);
        }
        else
        {
            field_set_fan_direction(&direction, (segment + 1) * angle_step, 8);
        }

        gte_ldv0(&direction);
        gte_rtv0();
        gte_stlvnl(&transformed);
        second_triangle = &packets[1];
        fourth_triangle = &packets[3];
        x = screen_origin.vx + transformed.vx;
        packets[0].xy2.signed_pair.x = x;
        second_triangle[0].xy2.signed_pair.x = x;
        fourth_triangle[0].xy0.signed_pair.x = x;
        packets[2].xy0.signed_pair.x = x;
        y = screen_origin.vy + transformed.vy;
        packets[0].xy2.signed_pair.y = y;
        second_triangle[0].xy2.signed_pair.y = y;
        fourth_triangle[0].xy0.signed_pair.y = y;
        packets[2].xy0.signed_pair.y = y;
        field_set_fan_direction(&direction, half_step + segment * angle_step, 6);
        gte_ldv0(&direction);
        gte_rtv0();
        gte_stlvnl(&transformed);
        x = screen_origin.vx + transformed.vx;
        second_triangle[0].xy1.signed_pair.x = x;
        packets[2].xy1.signed_pair.x = x;
        y = screen_origin.vy + transformed.vy;
        second_triangle[0].xy1.signed_pair.y = y;
        packets[2].xy1.signed_pair.y = y;

        if (segment == segment_count - 1)
        {
            field_set_fan_direction(&direction, 0, 7);
        }
        else
        {
            field_set_fan_direction(&direction, segment * angle_step + angle_step, 7);
        }

        gte_ldv0(&direction);
        gte_rtv0();
        gte_stlvnl(&transformed);
        third_triangle = &packets[2];
        x = screen_origin.vx + transformed.vx;
        third_triangle[0].xy2.signed_pair.x = x;
        packets[3].xy1.signed_pair.x = x;
        y = screen_origin.vy + transformed.vy;
        third_triangle[0].xy2.signed_pair.y = y;
        packets[3].xy1.signed_pair.y = y;

        if (segment == segment_count - 1)
        {
            field_set_fan_direction(&direction, half_step, 6);
        }
        else
        {
            next_half_angle = half_step + segment * angle_step + angle_step;
            field_set_fan_direction(&direction, next_half_angle, 6);
        }

        gte_ldv0(&direction);
        gte_rtv0();
        gte_stlvnl(&transformed);
        x = screen_origin.vx + transformed.vx;
        packets[3].xy2.signed_pair.x = x;
        color = packets[0].color0.signed_word;
        y = screen_origin.vy + transformed.vy;
        tag = packets[0].tag;
        packets[3].color2.signed_word = 0;
        packets[3].color1.signed_word = 0;
        packets[3].color0.signed_word = color;
        packets[2].color2.signed_word = 0;
        packets[2].color1.signed_word = 0;
        packets[2].color0.signed_word = color;
        packets[0].color1.signed_word = color;
        packets[1].color2.signed_word = color;
        packets[1].color1.signed_word = 0;
        packets[1].color0.signed_word = color;
        packets[0].color2.signed_word = color;
        packets[1].tag = tag;
        packets[2].tag = tag;
        packets[3].tag = tag;
        packets[4].tag = tag;
        packets[4].color0.signed_word = packets[0].color0.signed_word;
        packets[3].xy2.signed_pair.y = y;
        depth = effect->z >> FIELD_WORLD_OT_DEPTH_SHIFT;
        next_packet = &packets[1];

        if (depth < 0)
        {
            addPrim(&ordering_table[0], packets);
            packets = next_packet;
        }
        else if (depth >= FIELD_WORLD_OT_SIZE)
        {
            addPrim(&ordering_table[FIELD_WORLD_OT_LAST], packets);
            packets = next_packet;
        }
        else
        {
            addPrim(&ordering_table[effect->z >> FIELD_WORLD_OT_DEPTH_SHIFT], packets);
            packets = next_packet;
        }

        depth = effect->z >> FIELD_WORLD_OT_DEPTH_SHIFT;

        if (depth < 0)
        {
            addPrim(&ordering_table[0], packets);
            packets++;
        }
        else if (depth >= FIELD_WORLD_OT_SIZE)
        {
            addPrim(&ordering_table[FIELD_WORLD_OT_LAST], packets);
            packets++;
        }
        else
        {
            addPrim(&ordering_table[effect->z >> FIELD_WORLD_OT_DEPTH_SHIFT], packets);
            packets++;
        }

        depth = effect->z >> FIELD_WORLD_OT_DEPTH_SHIFT;

        if (depth < 0)
        {
            addPrim(&ordering_table[0], packets);
            packets++;
        }
        else if (depth >= FIELD_WORLD_OT_SIZE)
        {
            addPrim(&ordering_table[FIELD_WORLD_OT_LAST], packets);
            packets++;
        }
        else
        {
            addPrim(&ordering_table[effect->z >> FIELD_WORLD_OT_DEPTH_SHIFT], packets);
            packets++;
        }

        depth = effect->z >> FIELD_WORLD_OT_DEPTH_SHIFT;

        if (depth < 0)
        {
            addPrim(&ordering_table[0], packets);
            packets++;
        }
        else if (depth >= FIELD_WORLD_OT_SIZE)
        {
            addPrim(&ordering_table[FIELD_WORLD_OT_LAST], packets);
            packets++;
        }
        else
        {
            addPrim(&ordering_table[effect->z >> FIELD_WORLD_OT_DEPTH_SHIFT], packets);
            packets++;
        }

        if (segment == segment_count - 1)
        {
            break;
        }
    }

    return field_emit_effect_texture_page(effect, part, (u8*)packets, ordering_table);
}

/**
 * @brief Emit a fading line from an effect to its target or a rotated local offset.
 * @param effect Effect position, rendering flags, and part selection.
 * @param packet_cursor Destination GPU packet buffer.
 * @param ordering_table Depth-indexed ordering table.
 * @return Packet cursor after the geometry and texture-page command.
 */
u8* field_render_effect_marker(FieldMotionRecord* effect, u8* packet_cursor, s32* ordering_table)
{
    SVECTOR screen_origin;
    SVECTOR direction;
    VECTOR transformed;
    VECTOR target_position;
    MATRIX matrix;
    FieldActorSlot* actor;
    FieldObjectPart* part;
    s32 depth;

    part = &g_field_actor_slots[effect->owner_slot].parts[effect->owner_part];
    actor = &g_field_actor_slots[effect->owner_slot];
    field_build_effect_part_matrix(effect, part, &matrix, actor);
    gte_SetRotMatrix(&matrix);

    screen_origin.vx = FIELD_SCREEN_X(effect->x);
    screen_origin.vy = FIELD_SCREEN_Y(effect->y, effect->z);

    field_resolve_effect_part_color(actor, effect, part, (FieldPrimitiveColor*)&((P_TAG*)packet_cursor)->r0);
    setLineG2((LINE_G2*)packet_cursor);
    setSemiTrans((LINE_G2*)packet_cursor, effect->flags & FIELD_EFFECT_SEMITRANSPARENT);
    ((FieldGouraudLine*)packet_cursor)[0].color1.signed_word = 0;

    if (effect->color_position.fields.position_source != FIELD_POSITION_NONE)
    {
        ((FieldGouraudLine*)packet_cursor)[0].xy0.word = *(s32*)&screen_origin;
        field_resolve_effect_position(effect, part, &target_position);
        screen_origin.vx = FIELD_SCREEN_X(target_position.vx);
        screen_origin.vy = FIELD_SCREEN_Y(target_position.vy, target_position.vz);
        ((FieldGouraudLine*)packet_cursor)[0].xy1.word = *(s32*)&screen_origin;
    }
    else if ((part->behavior_flags.word >> FIELD_PART_ORIENTED_SHIFT) & 1)
    {
        direction.vx = -30;
        direction.vy = 0;
        direction.vz = 0;
        gte_ldv0(&direction);
        gte_rtv0();
        gte_stlvnl(&transformed);
        ((FieldGouraudLine*)packet_cursor)[0].xy0.signed_pair.x = screen_origin.vx + transformed.vx;
        {
            s16 y = screen_origin.vy + transformed.vy;
            ((FieldGouraudLine*)packet_cursor)[0].xy1.word = *(s32*)&screen_origin;
            ((FieldGouraudLine*)packet_cursor)[0].xy0.signed_pair.y = y;
        }
    }
    else
    {
        ((FieldGouraudLine*)packet_cursor)[0].xy1.word = *(s32*)&screen_origin;
        ((FieldGouraudLine*)packet_cursor)[0].xy0.word = *(s32*)&screen_origin;
    }

    depth = effect->z >> FIELD_WORLD_OT_DEPTH_SHIFT;

    if (depth < 0)
    {
        addPrim(&ordering_table[0], packet_cursor);
        packet_cursor += sizeof(LINE_G2);
    }
    else if (depth >= FIELD_WORLD_OT_SIZE)
    {
        addPrim(&ordering_table[(FIELD_WORLD_OT_SIZE - 1)], packet_cursor);
        packet_cursor += sizeof(LINE_G2);
    }
    else
    {
        setaddr(packet_cursor, getaddr(&ordering_table[depth]));
        setaddr(&ordering_table[effect->z >> FIELD_WORLD_OT_DEPTH_SHIFT], packet_cursor);
        packet_cursor += sizeof(LINE_G2);
    }

    return field_emit_effect_texture_page(effect, part, packet_cursor, ordering_table);
}

/**
 * @brief Join an effect to its live predecessor with a flat-shaded quad.
 * @param effect Effect position, rendering flags, and part selection.
 * @param packet_cursor Destination GPU packet buffer.
 * @param ordering_table Depth-indexed ordering table.
 * @return Advanced packet cursor; unchanged for a trail without a live predecessor.
 */
u8* field_render_effect_trail(FieldMotionRecord* effect, u8* packet_cursor, s32* ordering_table)
{
    FieldActorSlot* actor;
    FieldObjectPart* part;
    s32 depth;
    s32 camera_x;
    s32 effect_x;
    s32 camera_y;

    part = &g_field_actor_slots[effect->owner_slot].parts[effect->owner_part];
    actor = &g_field_actor_slots[effect->owner_slot];

    if (effect->previous_effect_index != FIELD_EFFECT_RETIRED && g_field_effect_records[effect->previous_effect_index].state != FIELD_EFFECT_RETIRED)
    {
        camera_x = g_field_view_offset_x / 256;
        effect_x = effect->x / 256 + FIELD_SCREEN_CENTER_X;
        camera_y = g_field_view_offset_y;
        ((FieldFlatQuad*)packet_cursor)[0].xy0.unsigned_pair.x = (camera_x + effect_x);

        ((FieldFlatQuad*)packet_cursor)[0].xy0.unsigned_pair.y =
            (FIELD_SCREEN_CENTER_Y + camera_y / 256 + effect->y / 256 - effect->z / 512 - g_field_view_offset_z / 512);

        ((FieldFlatQuad*)packet_cursor)[0].xy2.word = ((FieldFlatQuad*)packet_cursor)[0].xy0.word;
        ((FieldFlatQuad*)packet_cursor)[0].xy0.unsigned_pair.x = (((FieldFlatQuad*)packet_cursor)[0].xy0.unsigned_pair.x - effect->work_x);
        ((FieldFlatQuad*)packet_cursor)[0].xy0.unsigned_pair.y = (((FieldFlatQuad*)packet_cursor)[0].xy0.unsigned_pair.y - effect->work_y);
        ((FieldFlatQuad*)packet_cursor)[0].xy2.unsigned_pair.x = (((FieldFlatQuad*)packet_cursor)[0].xy2.unsigned_pair.x + effect->work_x);
        ((FieldFlatQuad*)packet_cursor)[0].xy2.unsigned_pair.y = (((FieldFlatQuad*)packet_cursor)[0].xy2.unsigned_pair.y + effect->work_y);

        ((FieldFlatQuad*)packet_cursor)[0].xy1.unsigned_pair.x =
            (g_field_view_offset_x / 256 + (g_field_effect_records[effect->previous_effect_index].x / 256 + FIELD_SCREEN_CENTER_X));

        ((FieldFlatQuad*)packet_cursor)[0].xy1.unsigned_pair.y =
            (FIELD_SCREEN_CENTER_Y + camera_y / 256 + g_field_effect_records[effect->previous_effect_index].y / 256 -
             g_field_effect_records[effect->previous_effect_index].z / 512 - g_field_view_offset_z / 512);

        ((FieldFlatQuad*)packet_cursor)[0].xy3.word = ((FieldFlatQuad*)packet_cursor)[0].xy1.word;
        ((FieldFlatQuad*)packet_cursor)[0].xy1.unsigned_pair.x =
            (((FieldFlatQuad*)packet_cursor)[0].xy1.unsigned_pair.x - g_field_effect_records[effect->previous_effect_index].work_x);
        ((FieldFlatQuad*)packet_cursor)[0].xy1.unsigned_pair.y =
            (((FieldFlatQuad*)packet_cursor)[0].xy1.unsigned_pair.y - g_field_effect_records[effect->previous_effect_index].work_y);
        ((FieldFlatQuad*)packet_cursor)[0].xy3.unsigned_pair.x =
            (((FieldFlatQuad*)packet_cursor)[0].xy3.unsigned_pair.x + g_field_effect_records[effect->previous_effect_index].work_x);
        ((FieldFlatQuad*)packet_cursor)[0].xy3.unsigned_pair.y =
            (((FieldFlatQuad*)packet_cursor)[0].xy3.unsigned_pair.y + g_field_effect_records[effect->previous_effect_index].work_y);

        field_resolve_effect_part_color(actor, effect, part, (FieldPrimitiveColor*)&((P_TAG*)packet_cursor)->r0);

        setPolyF4((POLY_F4*)packet_cursor);
        setSemiTrans((POLY_F4*)packet_cursor, effect->flags & FIELD_EFFECT_SEMITRANSPARENT);

        depth = effect->z >> FIELD_WORLD_OT_DEPTH_SHIFT;

        if (depth < 0)
        {
            addPrim(&ordering_table[0], (POLY_F4*)packet_cursor);
            packet_cursor += sizeof(POLY_F4);
        }
        else if (depth >= FIELD_WORLD_OT_SIZE)
        {
            addPrim(&ordering_table[(FIELD_WORLD_OT_SIZE - 1)], (POLY_F4*)packet_cursor);
            packet_cursor += sizeof(POLY_F4);
        }
        else
        {
            addPrim(&ordering_table[effect->z >> FIELD_WORLD_OT_DEPTH_SHIFT], (POLY_F4*)packet_cursor);
            packet_cursor += sizeof(POLY_F4);
        }

        packet_cursor = field_emit_effect_texture_page(effect, part, packet_cursor, ordering_table);
    }

    return packet_cursor;
}

/**
 * @brief Emit a closed line loop with alternating inner and outer radii.
 * @param effect Effect position, rendering flags, and part selection.
 * @param packet_cursor Destination GPU packet buffer.
 * @param ordering_table Depth-indexed ordering table.
 * @return Packet cursor after the geometry and texture-page command.
 */
u8* field_render_effect_radial_fan(FieldMotionRecord* effect, u8* packet_cursor, s32* ordering_table)
{
    MATRIX* matrix;
    SVECTOR* direction;
    FieldObjectPart* part;
    VECTOR* transformed;
    s32 radius;
    Vec2s* screen_origin;
    FieldActorSlot* actor;
    FieldFlatLine* next_line;
    s32 angle;
    s32 segment_count;
    s32 first_endpoint;
    s32 segment;
    s32 screen_x;
    s32 camera_y;
    s32 depth;

    transformed = &FIELD_RADIAL_SCRATCH->transformed;
    screen_origin = &FIELD_RADIAL_SCRATCH->center.screen;
    direction = &FIELD_RADIAL_SCRATCH->direction;
    matrix = FIELD_RADIAL_SCRATCH->matrices;

    part = &g_field_actor_slots[effect->owner_slot].parts[effect->owner_part];
    actor = &g_field_actor_slots[effect->owner_slot];

    screen_origin->x = (s16)FIELD_SCREEN_X(effect->x);
    screen_origin->y = (s16)FIELD_SCREEN_Y(effect->y, effect->z);

    field_build_effect_part_matrix(effect, part, matrix, actor);
    gte_SetRotMatrix(matrix);

    screen_x = FIELD_SCREEN_X(effect->x);
    camera_y = g_field_view_offset_y;
    ((FieldFlatLine*)packet_cursor)[0].xy0.signed_pair.x = (s16)screen_x;

    if (camera_y < 0)
    {
        camera_y += 255;
    }

    ((FieldFlatLine*)packet_cursor)[0].xy0.signed_pair.y =
        (s16)(FIELD_SCREEN_CENTER_Y + (camera_y >> 8) + effect->y / 256 - effect->z / 512 - g_field_view_offset_z / 512);

    field_resolve_effect_part_color(actor, effect, part, (FieldPrimitiveColor*)&((P_TAG*)packet_cursor)->r0);

    /* Line effects reuse animation_active as their segment count. */
    segment_count = 1;

    if (effect->animation_active != 0)
    {
        segment_count = effect->animation_active;
    }

    /* This mode interprets the shared selector byte as an outer-radius scale. */
    radius = (u32)((part->rotation_extent.fields.linked_part + 1) * 5) >> 4;

    direction->vx = (s16)((u32)(rsin(0) * 5) >> 8);
    direction->vy = 0;
    direction->vz = (s16)((s32)(rcos(0) * 80) >> 12);

    gte_ldv0(direction);
    gte_rtv0();
    gte_stlvnl(transformed);

    ((FieldFlatLine*)packet_cursor)[0].xy0.signed_pair.x = (s16)(screen_origin->x + *(s16*)&transformed->vx);
    ((FieldFlatLine*)packet_cursor)[0].xy0.signed_pair.y = (s16)(screen_origin->y + *(s16*)&transformed->vy);
    first_endpoint = ((FieldFlatLine*)packet_cursor)[0].xy0.word;

    for (segment = 1; segment < segment_count; segment++)
    {
        next_line = &((FieldFlatLine*)packet_cursor)[1];

        /* Each arm computes its own angle; one shared shift before the if reorders the arms. */
        if (segment & 1)
        {
            angle = segment << 12;
            angle /= segment_count;
            direction->vx = (s16)((rsin(angle) * radius) >> 12);
            direction->vy = 0;
            direction->vz = (s16)((rcos(angle) * radius) >> 12);
        }
        else
        {
            angle = segment << 12;
            angle /= segment_count;
            direction->vx = (s16)((u32)(rsin(angle) * 5) >> 8);
            direction->vy = 0;
            direction->vz = (s16)((s32)(rcos(angle) * 80) >> 12);
        }

        setLineF2(next_line - 1);
        setSemiTrans(next_line - 1, effect->flags & FIELD_EFFECT_SEMITRANSPARENT);

        gte_ldv0(direction);
        gte_rtv0();
        gte_stlvnl(transformed);

        next_line[-1].xy1.signed_pair.x = (s16)(screen_origin->x + *(s16*)&transformed->vx);
        next_line[-1].xy1.signed_pair.y = (s16)(screen_origin->y + *(s16*)&transformed->vy);
        ((FieldFlatLine*)packet_cursor)[1].xy0.word = ((FieldFlatLine*)packet_cursor)[0].xy1.word;
        ((FieldFlatLine*)packet_cursor)[1].color0.signed_word = ((FieldFlatLine*)packet_cursor)[0].color0.signed_word;

        depth = (s32)effect->z >> FIELD_WORLD_OT_DEPTH_SHIFT;

        if (depth < 0)
        {
            addPrim(&ordering_table[0], packet_cursor);
            packet_cursor += sizeof(LINE_F2);
        }
        else if (depth >= FIELD_WORLD_OT_SIZE)
        {
            addPrim(&ordering_table[FIELD_WORLD_OT_LAST], packet_cursor);
            packet_cursor += sizeof(LINE_F2);
        }
        else
        {
            setaddr(packet_cursor, getaddr(&ordering_table[depth]));
            setaddr(FIELD_ELEMENT_AT(s32, ordering_table, effect->z >> FIELD_WORLD_OT_DEPTH_SHIFT), packet_cursor);
            packet_cursor += sizeof(LINE_F2);
        }
    }

    setLineF2(packet_cursor);
    setSemiTrans(packet_cursor, effect->flags & FIELD_EFFECT_SEMITRANSPARENT);
    ((FieldFlatLine*)packet_cursor)[0].xy1.word = first_endpoint;

    depth = (s32)effect->z >> FIELD_WORLD_OT_DEPTH_SHIFT;

    if (depth < 0)
    {
        addPrim(&ordering_table[0], packet_cursor);
        packet_cursor += sizeof(LINE_F2);
    }
    else if (depth >= FIELD_WORLD_OT_SIZE)
    {
        addPrim(&ordering_table[FIELD_WORLD_OT_LAST], packet_cursor);
        packet_cursor += sizeof(LINE_F2);
    }
    else
    {
        setaddr(packet_cursor, getaddr(&ordering_table[depth]));
        setaddr(FIELD_ELEMENT_AT(s32, ordering_table, effect->z >> FIELD_WORLD_OT_DEPTH_SHIFT), packet_cursor);
        packet_cursor += sizeof(LINE_F2);
    }

    packet_cursor = field_emit_effect_texture_page(effect, part, packet_cursor, ordering_table);

    return packet_cursor;
}

/**
 * @brief Emit a jittered line chain between an effect and its resolved target.
 * @param effect Effect position, rendering flags, and part selection.
 * @param packet_cursor Destination GPU packet buffer.
 * @param ordering_table Depth-indexed ordering table.
 * @return Packet cursor after the geometry and texture-page command.
 */
u8* field_render_effect_radial_lines(FieldMotionRecord* effect, u8* packet_cursor, s32* ordering_table)
{
    FieldObjectPart* part;
    FieldActorSlot* actor;
    VECTOR* position;
    VECTOR* center;
    VECTOR* delta;
    VECTOR* jitter;
    SVECTOR* direction;
    MATRIX* matrix;
    FieldFlatLine* next_line;
    s32 segment_count;
    s32 segment;
    s32 arc_height;
    s32 angle_step;
    s32 depth;
    VECTOR* origin;

    origin = &FIELD_RADIAL_SCRATCH->origin;
    position = &FIELD_RADIAL_SCRATCH->transformed;
    center = &FIELD_RADIAL_SCRATCH->center.vector;
    delta = &FIELD_RADIAL_SCRATCH->delta;
    jitter = &FIELD_RADIAL_SCRATCH->jitter;
    direction = &FIELD_RADIAL_SCRATCH->direction;

    part = &g_field_actor_slots[effect->owner_slot].parts[effect->owner_part];
    actor = &g_field_actor_slots[effect->owner_slot];

    matrix = FIELD_RADIAL_SCRATCH->matrices;
    field_build_effect_part_matrix(effect, part, matrix, actor);
    gte_SetRotMatrix(matrix);

    {
        s32 camera_x;
        s32 position_x;
        s32 camera_y;
        camera_x = g_field_view_offset_x / 256;
        position_x = effect->x / 256;
        camera_y = g_field_view_offset_y;
        ((FieldFlatLine*)packet_cursor)[0].xy0.signed_pair.x = camera_x + (s16)(position_x + FIELD_SCREEN_CENTER_X);

        if (camera_y < 0)
        {
            camera_y += 255;
        }

        ((FieldFlatLine*)packet_cursor)[0].xy0.signed_pair.y =
            FIELD_SCREEN_CENTER_Y + (camera_y >> 8) + effect->y / 256 - effect->z / 512 - g_field_view_offset_z / 512;
    }

    field_resolve_effect_part_color(actor, effect, part, (FieldPrimitiveColor*)&((P_TAG*)packet_cursor)->r0);

    setLineF2(packet_cursor);
    setSemiTrans(packet_cursor, effect->flags & FIELD_EFFECT_SEMITRANSPARENT);

    matrix = FIELD_RADIAL_SCRATCH->matrices;

    /* Line effects reuse animation_active as their segment count. */
    segment_count = FIELD_RADIAL_MAX_SEGMENTS;

    if (effect->animation_active < FIELD_RADIAL_MAX_SEGMENTS)
    {
        segment_count = effect->animation_active;
    }

    if (segment_count <= 0)
    {
        segment_count = 1;
    }

    angle_step = (ONE / 2) / segment_count;

    field_resolve_effect_position(effect, part, origin);

    segment = segment_count - 1;

    center->vx = (origin->vx + effect->x) >> 1;
    center->vy = origin->vy;
    center->vz = (origin->vz + effect->z) >> 1;
    delta->vx = (origin->vx - effect->x) >> 1;
    delta->vy = effect->y - origin->vy;
    delta->vz = (origin->vz - effect->z) >> 1;

    for (; segment > 0; segment--)
    {
        direction->vx = 0;
        direction->vy = (s16)((rand() << 12) >> 15);
        direction->vz = (s16)((rand() << 12) >> 16);
        RotMatrix_gte(direction, matrix);
        matrix++;
    }

    matrix = FIELD_RADIAL_SCRATCH->matrices;

    if (part->behavior_flags.bytes.low >> 7)
    {
        direction->vx = 0;
        direction->vy = (s16)((part->behavior_flags.word >> 28) << 8);
        direction->vz = 0;
    }
    else
    {
        /* Clear both vector words, including the SDK padding halfword. */
        *(s32*)&direction->vz = 0;
        *(s32*)&direction->vx = 0;
    }

    if (((part->track_flags.word >> FIELD_PART_ARC_MODE_SHIFT) & 3) != 0)
    {
        arc_height = (part->track_flags.word >> FIELD_PART_ARC_HEIGHT_SHIFT) << 9;
    }
    else
    {
        arc_height = 0;
    }

    segment = segment_count - 1;

    if (segment > 0)
    {
        next_line = &((FieldFlatLine*)packet_cursor)[1];

        for (; segment > 0; segment--, matrix++)
        {
            next_line[0].color0.signed_word = next_line[-1].color0.signed_word;
            setLineF2(next_line - 1);
            setSemiTrans(next_line - 1, effect->flags & FIELD_EFFECT_SEMITRANSPARENT);

            gte_SetRotMatrix(matrix);
            gte_ldv0(direction);
            gte_rtv0();
            gte_stlvnl(jitter);

            if (arc_height != 0)
            {
                position->vy = center->vy + (delta->vy * segment) / segment_count -
                               ((s32)(((part->track_flags.word >> FIELD_PART_ARC_HEIGHT_SHIFT) << 9) * rsin(segment * angle_step)) >> 12) + jitter->vy;
            }
            else
            {
                position->vy = center->vy + (delta->vy * segment) / segment_count + jitter->vy;
            }

            position->vx = ((delta->vx * rcos(segment * angle_step)) >> 12) + center->vx + jitter->vx;
            position->vz = ((delta->vz * rcos(segment * angle_step)) >> 12) + center->vz + jitter->vz;

            {
                s32 camera_x;
                s32 position_x;
                s32 camera_y;
                camera_x = g_field_view_offset_x / 256;
                position_x = position->vx / 256;
                camera_y = g_field_view_offset_y;
                next_line[-1].xy1.signed_pair.x = camera_x + (s16)(position_x + FIELD_SCREEN_CENTER_X);

                if (camera_y < 0)
                {
                    camera_y += 255;
                }

                next_line[-1].xy1.signed_pair.y =
                    FIELD_SCREEN_CENTER_Y + (camera_y >> 8) + position->vy / 256 - position->vz / 512 - g_field_view_offset_z / 512;
            }

            next_line[0].xy0.word = next_line[-1].xy1.word;

            depth = (s32)effect->z >> FIELD_WORLD_OT_DEPTH_SHIFT;

            if (depth < 0)
            {
                next_line++;
                addPrim(&ordering_table[0], packet_cursor);
                packet_cursor += sizeof(LINE_F2);
            }
            else if (depth >= FIELD_WORLD_OT_SIZE)
            {
                next_line++;
                addPrim(&ordering_table[FIELD_WORLD_OT_LAST], packet_cursor);
                packet_cursor += sizeof(LINE_F2);
            }
            else
            {
                next_line++;
                setaddr(packet_cursor, getaddr(&ordering_table[depth]));
                setaddr(FIELD_ELEMENT_AT(s32, ordering_table, effect->z >> FIELD_WORLD_OT_DEPTH_SHIFT), packet_cursor);
                packet_cursor += sizeof(LINE_F2);
            }
        }
    }

    setLineF2(packet_cursor);
    setSemiTrans(packet_cursor, effect->flags & FIELD_EFFECT_SEMITRANSPARENT);

    {
        s32 camera_x;
        s32 position_x;
        s32 camera_y;
        camera_x = g_field_view_offset_x / 256;
        position_x = origin->vx / 256;
        camera_y = g_field_view_offset_y;
        ((FieldFlatLine*)packet_cursor)[0].xy1.signed_pair.x = camera_x + (s16)(position_x + FIELD_SCREEN_CENTER_X);

        if (camera_y < 0)
        {
            camera_y += 255;
        }

        camera_y = FIELD_SCREEN_CENTER_Y + (camera_y >> 8) + origin->vy / 256;
        camera_y -= origin->vz / 512;
        ((FieldFlatLine*)packet_cursor)[0].xy1.signed_pair.y = camera_y - g_field_view_offset_z / 512;
    }

    depth = (s32)effect->z >> FIELD_WORLD_OT_DEPTH_SHIFT;

    if (depth < 0)
    {
        addPrim(&ordering_table[0], packet_cursor);
        packet_cursor += sizeof(LINE_F2);
    }
    else if (depth >= FIELD_WORLD_OT_SIZE)
    {
        addPrim(&ordering_table[FIELD_WORLD_OT_LAST], packet_cursor);
        packet_cursor += sizeof(LINE_F2);
    }
    else
    {
        setaddr(packet_cursor, getaddr(&ordering_table[depth]));
        setaddr(FIELD_ELEMENT_AT(s32, ordering_table, effect->z >> FIELD_WORLD_OT_DEPTH_SHIFT), packet_cursor);
        packet_cursor += sizeof(LINE_F2);
    }

    packet_cursor = field_emit_effect_texture_page(effect, part, packet_cursor, ordering_table);

    return packet_cursor;
}

/**
 * @brief Render a textured ribbon from an effect to its resolved target.
 * @param effect Position, facing, color, age, and segment count source.
 * @param packet_cursor Receives consecutive POLY_FT4 packets.
 * @param ordering_table Depth-indexed GPU ordering table.
 * @return Cursor after the last emitted quad.
 * @note animation_active supplies the segment count for this effect kind.
 */
u8* field_render_effect_ribbon(FieldMotionRecord* effect, u8* packet_cursor, s32* ordering_table)
{
    FieldRibbonQuad* quad = (FieldRibbonQuad*)packet_cursor;
    FieldObjectPart* part;
    FieldActorSlot* state;
    VECTOR* position;
    VECTOR* midpoint;
    VECTOR* half_delta;
    VECTOR* delta;
    MATRIX* rotation;
    VECTOR* endpoint;
    s32 segment_count;
    s32 i;
    s32 arc_height;
    s32 step;
    s32 angle;
    s32 edge_x;
    FieldRotationAngles* direction;
    s32 depth;
    u8 uv_flags;

    position = &FIELD_RIBBON_SCRATCH->position;
    midpoint = &FIELD_RIBBON_SCRATCH->midpoint;
    half_delta = &FIELD_RIBBON_SCRATCH->half_delta;
    delta = &FIELD_RIBBON_SCRATCH->delta;
    direction = &FIELD_RIBBON_SCRATCH->direction;
    endpoint = &FIELD_RIBBON_SCRATCH->endpoint;

    part = &g_field_actor_slots[effect->owner_slot].parts[effect->owner_part];
    state = &g_field_actor_slots[effect->owner_slot];
    field_build_effect_part_matrix(effect, part, FIELD_RIBBON_SCRATCH->rotations, state);

    rotation = FIELD_RIBBON_SCRATCH->rotations;
    uv_flags = g_field_ribbon_frame_flags[((u16)effect->age) % FIELD_RIBBON_FRAME_COUNT];
    quad->packed.vertices[0].uv = g_field_ribbon_uv_corners[(uv_flags & FIELD_RIBBON_UV_VARIANT) * 4 + 0];
    quad->packed.vertices[1].uv = g_field_ribbon_uv_corners[(uv_flags & FIELD_RIBBON_UV_VARIANT) * 4 + 1];
    quad->packed.vertices[2].uv = g_field_ribbon_uv_corners[(uv_flags & FIELD_RIBBON_UV_VARIANT) * 4 + 2];
    quad->packed.vertices[3].uv = g_field_ribbon_uv_corners[(uv_flags & FIELD_RIBBON_UV_VARIANT) * 4 + 3];

    if (uv_flags & FIELD_RIBBON_FLIP_U)
    {
        i = quad->gpu.u0;
        quad->gpu.u0 = quad->gpu.u1;
        quad->gpu.u1 = i;
        i = quad->gpu.u2;
        quad->gpu.u2 = quad->gpu.u3;
        quad->gpu.u3 = i;
    }

    if (uv_flags & FIELD_RIBBON_FLIP_V)
    {
        i = quad->gpu.v0;
        quad->gpu.v0 = quad->gpu.v2;
        quad->gpu.v2 = i;
        i = quad->gpu.v1;
        quad->gpu.v1 = quad->gpu.v3;
        quad->gpu.v3 = i;
    }

    {
        u32 behavior_flags = part->behavior_flags.word;
        quad->gpu.clut = getClut(80, 492);
        quad->gpu.tpage = (u16)(((behavior_flags >> FIELD_PART_BLEND_TPAGE_SHIFT) & GPU_TPAGE_BLEND_MASK) | getTPage(GPU_TEXTURE_4BIT, GPU_BLEND_HALF, 448, 0));
    }

    gte_SetRotMatrix(rotation);

    {
        s32 view_x;
        s32 screen_x;
        s32 view_y;
        view_x = g_field_view_offset_x / 256;
        screen_x = effect->x / 256 + FIELD_SCREEN_CENTER_X;
        view_y = g_field_view_offset_y;
        quad->gpu.x0 = (s16)(view_x + screen_x);

        if (view_y < 0)
        {
            view_y += 255;
        }

        quad->gpu.y0 = (s16)(FIELD_SCREEN_CENTER_Y + (view_y >> 8) + effect->y / 256 - effect->z / 512 - g_field_view_offset_z / 512);
    }

    field_resolve_effect_part_color(state, effect, part, &quad->packed.color);
    setPolyFT4(&quad->gpu);
    setSemiTrans(&quad->gpu, effect->flags & FIELD_EFFECT_SEMITRANSPARENT);

    rotation = FIELD_RIBBON_SCRATCH->rotations;
    segment_count = FIELD_RIBBON_MAX_SEGMENTS;

    if (effect->animation_active < FIELD_RIBBON_MAX_SEGMENTS)
    {
        segment_count = effect->animation_active;
    }

    if (segment_count <= 0)
    {
        segment_count = 1;
    }

    step = FIELD_ANGLE_HALF_TURN / segment_count;

    field_resolve_effect_position(effect, part, endpoint);
    midpoint->vx = (endpoint->vx + effect->x) >> 1;
    midpoint->vy = endpoint->vy;
    midpoint->vz = (endpoint->vz + effect->z) >> 1;
    delta->vx = endpoint->vx - effect->x;
    half_delta->vx = delta->vx >> 1;
    delta->vy = effect->y - endpoint->vy;
    half_delta->vy = delta->vy;
    delta->vz = endpoint->vz - effect->z;
    half_delta->vz = delta->vz >> 1;

    for (i = segment_count - 1; i > 0; i--, rotation++)
    {
        direction->vector.vx = 0;
        direction->vector.vy = (s16)((rand() << 12) >> 15);
        direction->vector.vz = (s16)((rand() << 12) >> 16);
        RotMatrix_gte(&direction->vector, rotation);
    }

    rotation = FIELD_RIBBON_SCRATCH->rotations;

    if ((part->behavior_flags.bytes.low) >> FIELD_PART_SCALE_TRACK_SHIFT)
    {
        direction->vector.vx = 0;
        direction->vector.vy = (s16)((part->behavior_flags.word >> 28) << 8);
        direction->vector.vz = 0;
    }
    else
    {
        direction->packed.z_pad = 0;
        direction->packed.xy = 0;
    }

    if (((part->track_flags.word >> FIELD_PART_ARC_MODE_SHIFT) & 3) != 0)
    {
        arc_height = (part->track_flags.word >> FIELD_PART_ARC_HEIGHT_SHIFT) << 10;
    }
    else
    {
        arc_height = 0;
    }

    arc_height = (arc_height * rand()) >> 15;

    if (rand() & 1)
    {
        arc_height = -arc_height;
    }

    angle = ratan2(delta->vx, delta->vy + (delta->vz >> 1));
    edge_x = (rcos(angle) * part->scale_y) >> 12;
    angle = (rsin(angle) * part->scale_y) >> 12;

    if (uv_flags & FIELD_RIBBON_UV_VARIANT)
    {
        angle *= 2;
        edge_x *= 2;
    }

    i = segment_count - 1;
    quad->gpu.x2 = quad->gpu.x0 + edge_x;
    quad->gpu.y2 = quad->gpu.y0 + angle;
    quad->gpu.y0 -= angle;
    quad->gpu.x0 -= edge_x;

    if (i > 0)
    {
        s32 angle_step;
        s32 addr_mask = FIELD_GPU_ADDRESS_MASK;
        s32 tag_mask = FIELD_GPU_LENGTH_MASK;

        for (angle_step = i * step; i > 0; angle_step -= step, i--, rotation++)
        {
            depth = quad->packed.color.signed_word;
            quad[1].packed.color.signed_word = depth;
            setPolyFT4(&quad->gpu);
            setSemiTrans(&quad->gpu, effect->flags & FIELD_EFFECT_SEMITRANSPARENT);

            gte_SetRotMatrix(rotation);
            gte_ldv0(direction);
            gte_rtv0();
            gte_stlvnl(delta);

            if (arc_height != 0)
            {
                position->vy = midpoint->vy + (half_delta->vy * i) / segment_count - ((arc_height * rsin(angle_step)) >> 12) + delta->vy;
            }
            else
            {
                position->vy = midpoint->vy + (half_delta->vy * i) / segment_count + delta->vy;
            }

            position->vx = ((half_delta->vx * rcos(angle_step)) >> 12) + midpoint->vx + delta->vx;
            position->vz = ((half_delta->vz * rcos(angle_step)) >> 12) + midpoint->vz + delta->vz;

            {
                s32 view_x;
                s32 screen_x;
                s32 view_y;
                view_x = g_field_view_offset_x / 256;
                screen_x = position->vx / 256 + FIELD_SCREEN_CENTER_X;
                view_y = g_field_view_offset_y;
                quad->gpu.x1 = (s16)(view_x + screen_x);

                if (view_y < 0)
                {
                    view_y += 255;
                }

                quad->gpu.y1 = (s16)(FIELD_SCREEN_CENTER_Y + (view_y >> 8) + position->vy / 256 - position->vz / 512 - g_field_view_offset_z / 512);
            }

            quad->gpu.x3 = quad->gpu.x1 + edge_x;
            quad->gpu.y3 = quad->gpu.y1 + angle;
            quad->gpu.y1 -= angle;
            quad->gpu.x1 -= edge_x;

            quad[1].packed.vertices[0].uv = quad->packed.vertices[0].uv;
            quad[1].packed.vertices[2].xy = quad->packed.vertices[3].xy;
            quad[1].packed.vertices[0].xy = quad->packed.vertices[1].xy;
            quad[1].packed.vertices[1].uv = quad->packed.vertices[1].uv;
            quad[1].packed.vertices[2].uv = quad->packed.vertices[2].uv;
            quad[1].packed.vertices[3].uv = quad->packed.vertices[3].uv;
            quad[1].gpu.tpage = quad->gpu.tpage;
            quad[1].gpu.clut = quad->gpu.clut;

            depth = (s32)effect->z >> FIELD_WORLD_OT_DEPTH_SHIFT;

            if (depth < 0)
            {
                s32 addr = (uintptr_t)quad & addr_mask;
                quad->packed.tag = (quad->packed.tag & tag_mask) | (ordering_table[0] & addr_mask);
                quad++;
                ordering_table[0] = (ordering_table[0] & tag_mask) | addr;
            }
            else if (depth >= FIELD_WORLD_OT_SIZE)
            {
                s32 addr = (uintptr_t)quad & addr_mask;
                quad->packed.tag = (quad->packed.tag & tag_mask) | (ordering_table[FIELD_WORLD_OT_LAST] & addr_mask);
                quad++;
                ordering_table[FIELD_WORLD_OT_LAST] = (ordering_table[FIELD_WORLD_OT_LAST] & tag_mask) | addr;
            }
            else
            {
                s32 addr;
                s32* entry;
                addr = (uintptr_t)quad & addr_mask;
                quad->packed.tag = (quad->packed.tag & tag_mask) | (ordering_table[depth] & addr_mask);
                entry = (s32*)(((s32)effect->z >> FIELD_WORLD_OT_DEPTH_SHIFT) * sizeof(*ordering_table) + (uintptr_t)ordering_table);
                quad++;
                *entry = (*entry & tag_mask) | addr;
            }
        }
    }

    setPolyFT4(&quad->gpu);
    setSemiTrans(&quad->gpu, effect->flags & FIELD_EFFECT_SEMITRANSPARENT);

    {
        s32 view_x;
        s32 screen_x;
        s32 view_y;
        view_x = g_field_view_offset_x / 256;
        screen_x = endpoint->vx / 256 + FIELD_SCREEN_CENTER_X;
        view_y = g_field_view_offset_y;
        quad->gpu.x1 = (s16)(view_x + screen_x);

        if (view_y < 0)
        {
            view_y += 255;
        }

        quad->gpu.y1 = (s16)(FIELD_SCREEN_CENTER_Y + (view_y >> 8) + endpoint->vy / 256 - endpoint->vz / 512 - g_field_view_offset_z / 512);
    }

    quad->gpu.x3 = quad->gpu.x1 + edge_x;
    quad->gpu.y3 = quad->gpu.y1 + angle;
    quad->gpu.y1 -= angle;
    quad->gpu.x1 -= edge_x;

    depth = (s32)effect->z >> FIELD_WORLD_OT_DEPTH_SHIFT;

    if (depth < 0)
    {
        s32 addr = (uintptr_t)quad & FIELD_GPU_ADDRESS_MASK;
        setaddr(quad, getaddr(&ordering_table[0]));
        quad++;
        ordering_table[0] = (ordering_table[0] & FIELD_GPU_LENGTH_MASK) | addr;
    }
    else if (depth >= FIELD_WORLD_OT_SIZE)
    {
        s32 addr = (uintptr_t)quad & FIELD_GPU_ADDRESS_MASK;
        setaddr(quad, getaddr(&ordering_table[FIELD_WORLD_OT_LAST]));
        quad++;
        ordering_table[FIELD_WORLD_OT_LAST] = (ordering_table[FIELD_WORLD_OT_LAST] & FIELD_GPU_LENGTH_MASK) | addr;
    }
    else
    {
        s32 addr;
        s32* entry;
        s32 srcval;
        addr = (uintptr_t)quad & FIELD_GPU_ADDRESS_MASK;
        srcval = ordering_table[depth];
        setaddr(quad, srcval);
        entry = (s32*)(((s32)effect->z >> FIELD_WORLD_OT_DEPTH_SHIFT) * sizeof(*ordering_table) + (uintptr_t)ordering_table);
        quad++;
        *entry = (*entry & FIELD_GPU_LENGTH_MASK) | addr;
    }

    return (u8*)quad;
}

/**
 * @brief Build an effect part's rotation, target alignment, and animated scale.
 * @param effect Position, orientation, and track sampling age.
 * @param part Rotation, alignment, and scale selectors.
 * @param matrix Receives the resulting transform.
 * @param actor Owner supplying parameter tracks and object bindings.
 * @return Unspecified; callers use only the matrix output.
 */
s32 field_build_effect_part_matrix(FieldMotionRecord* effect, FieldObjectPart* part, MATRIX* matrix, FieldActorSlot* actor)
{
    FieldMatrixStorage* storage = (FieldMatrixStorage*)matrix;
    SVECTOR* direction = &FIELD_MATRIX_SCRATCH->direction;
    VECTOR* scale = &FIELD_MATRIX_SCRATCH->scale;
    VECTOR* delta = &FIELD_MATRIX_SCRATCH->delta;
    VECTOR* squared_delta = &FIELD_MATRIX_SCRATCH->squared_delta;
    s32 angle;
    s32 final_angle;
    s32 axis;
    s32 scale_mode;
    u32 distance;
    s32 rotation_mode;
    s32 target_component;
    s32 next_scale_track;
    u32 flags;

    storage->packed.rotation[4] = ONE;
    storage->packed.rotation[2] = ONE;
    storage->packed.rotation[0] = ONE;
    matrix->t[2] = 0;
    matrix->t[1] = 0;
    matrix->t[0] = 0;
    storage->packed.rotation[3] = 0;
    storage->packed.rotation[1] = 0;

    flags = part->track_flags.word;
    rotation_mode = (flags >> FIELD_PART_ROTATION_MODE_SHIFT) & 3;

    if (rotation_mode != FIELD_PART_ROTATION_NONE)
    {
        switch (rotation_mode)
        {
            case FIELD_PART_ROTATION_TRACK:
                angle = field_evaluate_parameter_track_at_time(actor, flags >> 26, (u16)effect->age) << 4;
                axis = part->track_flags.bytes.high;
                axis &= 3;
                break;
            case FIELD_PART_ROTATION_HEADING:
                angle = effect->heading;
                axis = FIELD_ROTATION_AXIS_Y;
                break;
            case FIELD_PART_ROTATION_SPIN:
                angle = (((flags >> FIELD_PART_ROTATION_TRACK_SHIFT) * (u16)effect->age) << 4) & FIELD_ANGLE_MASK;
                axis = flags >> 24;
                axis &= 3;
                break;
            default:
                break;
        }

        if (part->spawn_flags.word & FIELD_PART_REVERSE_ROTATION_WITH_FACING)
        {
            if (!(g_field_actors[actor->owner_object_index].animation & FIELD_EFFECT_FACING_FLIPPED))
            {
                angle = -angle;
            }
        }

        switch (axis)
        {
            case FIELD_ROTATION_AXIS_X:
                RotMatrixX(angle, matrix);
                break;
            case FIELD_ROTATION_AXIS_Y:
                RotMatrixY(angle, matrix);
                break;
            case FIELD_ROTATION_AXIS_Z:
                RotMatrixZ(angle, matrix);
                break;
            case FIELD_ROTATION_AXIS_ALL:
                RotMatrixZ(angle, matrix);
                RotMatrixY(angle, matrix);
                RotMatrixX(angle, matrix);
                break;
        }
    }

    if ((part->track_flags.word >> FIELD_PART_ROTATE_QUARTER_X_SHIFT) & 1)
    {
        RotMatrixX(FIELD_ANGLE_QUARTER_TURN, matrix);
    }

    if (part->rotation_y_16 != 0)
    {
        RotMatrixY(part->rotation_y_16 << 4, matrix);
    }

    if ((part->behavior_flags.word >> FIELD_PART_ORIENTED_SHIFT) & 1)
    {
        RotMatrixZ(FIELD_ANGLE_QUARTER_TURN, matrix);
        RotMatrixY(FIELD_ANGLE_QUARTER_TURN, matrix);

        if ((part->placement_flags.word >> FIELD_PART_AIM_AT_TARGET_SHIFT) & 1)
        {
            field_resolve_effect_position(effect, part, scale);
            target_component = scale->vz;
            angle = ratan2(effect->z - target_component, scale->vx - effect->x);
            delta->vx = (scale->vx - effect->x) >> 8;
            delta->vy = (scale->vy - effect->y) >> 8;
            delta->vz = (scale->vz - effect->z) >> 8;
            gte_ldlvl(delta);
            gte_sqr0();
            gte_stlvnl(squared_delta);
            distance = SquareRoot0(squared_delta->vx + squared_delta->vz + squared_delta->vy);
            target_component = scale->vy;
            RotMatrixZ(ratan2(distance << 8, effect->y - target_component), matrix);
            RotMatrixY(angle, matrix);
            final_angle = FIELD_EFFECT_TARGET_PITCH;
        }
        else
        {
            RotMatrixZ(effect->pitch, matrix);
            RotMatrixY(effect->heading, matrix);
            RotMatrixZ(effect->rotation_z_16 << 4, matrix);
            RotMatrixY(effect->rotation_y_16 << 4, matrix);
            final_angle = FIELD_EFFECT_DEFAULT_PITCH;
        }
    }
    else
    {
        RotMatrixY(FIELD_ANGLE_QUARTER_TURN, matrix);
        final_angle = FIELD_ANGLE_QUARTER_TURN;
    }

    RotMatrixX(final_angle, matrix);

    if ((part->placement_flags.word >> FIELD_PART_STRETCH_TO_TARGET_SHIFT) & 1)
    {
        field_resolve_effect_position(effect, part, scale);
        delta->vx = (scale->vx - effect->x) >> 8;
        delta->vy = (scale->vy - effect->y) >> 8;
        delta->vz = (scale->vz - effect->z) >> 8;
        gte_ldlvl(delta);
        gte_sqr0();
        gte_stlvnl(squared_delta);
        distance = SquareRoot0(squared_delta->vx + squared_delta->vy + squared_delta->vz);

        if (distance == 0)
        {
            distance = 1;
        }

        scale->vy = ONE;
        scale->vz = ONE;

        if (effect->sprite_height_minus_one != 0)
        {
            scale->vx = (distance << 12) / effect->sprite_height_minus_one;
        }
        else
        {
            scale->vx = (distance << 12) >> 6;
        }

        ScaleMatrix(matrix, scale);
    }

    if ((part->track_flags.word >> FIELD_PART_PROJECT_SCALE_SHIFT) & 1)
    {
        if ((part->behavior_flags.word >> FIELD_PART_ORIENTED_SHIFT) & 1)
        {
            direction->vx = -ONE;
            direction->vy = 0;
            direction->vz = 0;
            gte_SetRotMatrix(matrix);
            gte_ldv0(direction);
            gte_rtv0();
            gte_stlvnl(scale);
            direction->vx = 0;
            direction->vy = FIELD_ANGLE_QUARTER_TURN;
            direction->vz = FIELD_ANGLE_QUARTER_TURN;
            RotMatrix_gte(direction, matrix);
            RotMatrixZ(ratan2(scale->vy, scale->vx) + FIELD_ANGLE_QUARTER_TURN, matrix);
            gte_ldlvl(scale);
            gte_sqr0();
            gte_stlvnl(squared_delta);
            scale->vx = SquareRoot0(squared_delta->vx + squared_delta->vy);
            scale->vz = ONE;
            scale->vy = ONE;
            ScaleMatrix(matrix, scale);
        }
    }

    scale->vy = ONE;

    /* Stat-derived strength reduces the footprint in ten-percent steps. */
    if (part->palette_extent.word & FIELD_PART_FOOTPRINT_SCALES_XZ)
    {
        scale->vz = FIELD_FOOTPRINT_SCALED_SIZE(part->appearance.fields.scale_xz, actor->owner_object_index) << 6;
    }
    else
    {
        scale->vz = part->appearance.fields.scale_xz << 6;
    }

    if (part->palette_extent.word & FIELD_PART_FOOTPRINT_SCALES_Y)
    {
        scale->vx = FIELD_FOOTPRINT_SCALED_SIZE(part->scale_y, actor->owner_object_index) << 6;
    }
    else
    {
        scale->vx = part->scale_y << 6;
    }

    ScaleMatrix(matrix, scale);

    g_field_effect_track_scale.x = ONE;
    g_field_effect_track_scale.y = ONE;
    g_field_effect_track_scale.z = ONE;

    if ((part->behavior_flags.word >> FIELD_PART_SCALE_TRACK_SHIFT) & 1)
    {
        scale->vz = ONE;
        scale->vy = ONE;
        scale->vx = ONE;
        flags = part->behavior_flags.word;
        scale_mode = (flags >> 20) & 3;

        switch (scale_mode)
        {
            case FIELD_PART_SCALE_Z:
                scale->vz = field_evaluate_parameter_track_at_time(actor, flags >> 28, (u16)effect->age) << 4;
                break;
            case FIELD_PART_SCALE_X:
                scale->vx = field_evaluate_parameter_track_at_time(actor, flags >> 28, (u16)effect->age) << 4;
                break;
            case FIELD_PART_SCALE_UNIFORM:
            {
                u32 value = field_evaluate_parameter_track_at_time(actor, flags >> 28, (u16)effect->age) << 4;
                scale->vz = value;
                scale->vy = value;
                scale->vx = value;
                break;
            }
            case FIELD_PART_SCALE_Z_THEN_X:
                scale->vz = field_evaluate_parameter_track_at_time(actor, flags >> 28, (u16)effect->age) << 4;
                next_scale_track = (part->behavior_flags.word >> 28) + 1;
                scale->vx = field_evaluate_parameter_track_at_time(actor, next_scale_track % FIELD_TRACK_COUNT, (u16)effect->age) << 4;
                break;
        }

        g_field_effect_track_scale.x = (u16)scale->vx;
        g_field_effect_track_scale.y = (u16)scale->vy;
        g_field_effect_track_scale.z = (u16)scale->vz;
        ScaleMatrix(matrix, scale);
    }
}

/**
 * @brief Resolve a part's literal or animated primitive color.
 * @param actor Owner supplying parameter tracks.
 * @param effect Color flags, packed literal color, and track sampling age.
 * @param part Literal channel values and track selectors.
 * @param out Four-byte color storage; the literal path also copies its fourth byte.
 */
void field_resolve_effect_part_color(FieldActorSlot* actor, FieldMotionRecord* effect, FieldObjectPart* part, FieldPrimitiveColor* out)
{
    s32 flags;
    u8 value;

    flags = effect->flags;

    if (!(flags & FIELD_CONTROL_EFFECT_COLOR))
    {
        out->channels.r = ((part->appearance.word >> FIELD_PART_COLOR_TRACK_SHIFT) & FIELD_PART_RED_TRACK)
                              ? field_evaluate_parameter_track_at_time(actor, part->tint_red & FIELD_TRACK_INDEX_MASK, (u16)effect->age)
                              : part->tint_red;
        out->channels.g = ((part->appearance.word >> FIELD_PART_COLOR_TRACK_SHIFT) & FIELD_PART_GREEN_TRACK)
                              ? field_evaluate_parameter_track_at_time(actor, part->tint_green & FIELD_TRACK_INDEX_MASK, (u16)effect->age)
                              : part->tint_green;
        out->channels.b = ((part->appearance.word >> FIELD_PART_COLOR_TRACK_SHIFT) & FIELD_PART_BLUE_TRACK)
                              ? field_evaluate_parameter_track_at_time(actor, part->tint_blue & FIELD_TRACK_INDEX_MASK, (u16)effect->age)
                              : part->tint_blue;
        return;
    }

    if (flags & FIELD_CONTROL_LITERAL_COLOR)
    {
        /* The fourth byte travels with RGB and is overwritten by the packet builder. */
        out->word = effect->color_position.word;
        return;
    }

    {
        u32 part_flags = part->behavior_flags.word;

        if ((part_flags >> FIELD_PART_RGB_TRACKS_SHIFT) & 1)
        {
            out->channels.r =
                field_evaluate_parameter_track_at_time(actor, (part_flags >> FIELD_PART_COLOR_TRACK_INDEX_SHIFT) & FIELD_TRACK_INDEX_MASK, (u16)effect->age);
            out->channels.g = field_evaluate_parameter_track_at_time(actor, (part->behavior_flags.halves.high & FIELD_TRACK_INDEX_MASK) + 1, (u16)effect->age);
            out->channels.b = field_evaluate_parameter_track_at_time(actor, (part->behavior_flags.halves.high & FIELD_TRACK_INDEX_MASK) + 2, (u16)effect->age);
            return;
        }

        value = field_evaluate_parameter_track_at_time(actor, (part_flags >> FIELD_PART_COLOR_TRACK_INDEX_SHIFT) & FIELD_TRACK_INDEX_MASK, (u16)effect->age);
        out->channels.r = out->channels.g = out->channels.b = value;
    }
}

/**
 * @brief Append the effect's texture-page command at its clamped depth.
 * @param effect Supplies the ordering-table depth.
 * @param part Supplies the semi-transparency blend mode.
 * @param packet_cursor Receives one DR_TPAGE packet.
 * @param ordering_table Depth-indexed GPU ordering table.
 * @return Cursor immediately after the texture-page packet.
 */
u8* field_emit_effect_texture_page(FieldMotionRecord* effect, FieldObjectPart* part, u8* packet_cursor, s32* ordering_table)
{
    s32 index;

    setDrawTPage(packet_cursor, 0, 0, getTPage(GPU_TEXTURE_4BIT, (part->behavior_flags.word >> 22), 320, 0));

    index = effect->z >> FIELD_WORLD_OT_DEPTH_SHIFT;

    if (index < 0)
    {
        addPrim(&ordering_table[0], packet_cursor);
        packet_cursor += sizeof(DR_TPAGE);
    }
    else if (index >= FIELD_WORLD_OT_SIZE)
    {
        addPrim(&ordering_table[FIELD_WORLD_OT_LAST], packet_cursor);
        packet_cursor += sizeof(DR_TPAGE);
    }
    else
    {
        setaddr(packet_cursor, getaddr(&ordering_table[index]));
        setaddr(FIELD_ELEMENT_AT(s32, ordering_table, effect->z >> FIELD_WORLD_OT_DEPTH_SHIFT), packet_cursor);
        packet_cursor += sizeof(DR_TPAGE);
    }

    return packet_cursor;
}

/**
 * @brief Project sprite corners, applying optional tilt and facing reversal.
 * @param effect Supplies the horizontal facing flag.
 * @param origin Screen-space origin, read as unsigned low halfwords.
 * @param quad POLY_FT4 packet receiving projected coordinates.
 * @param width Sprite width.
 * @param height Sprite height.
 * @param x Horizontal offset from the anchor.
 * @param y Vertical offset from the anchor.
 * @param frame Sprite record with tilt in 16 GTE angle units per step.
 * @param matrix Caller rotation matrix, already loaded into the GTE on entry.
 */
void field_project_effect_sprite_quad(FieldMotionRecord* effect, Vec2s* origin, POLY_FT4* quad, s32 width, s32 height, s32 x, s32 y, FieldSpriteFrame* frame,
                                      MATRIX* matrix)
{
    SVECTOR* rotated_vertex = &FIELD_SPRITE_SCRATCH->rotated_vertex;
    SVECTOR* vertex = &FIELD_SPRITE_SCRATCH->vertex;
    FieldSpriteProjection* out = &FIELD_SPRITE_SCRATCH->projected;
    FieldMatrixStorage* tilt_matrix = &FIELD_SPRITE_SCRATCH->rotation;

    /* The output uses unsigned low halves, including wrapped negative coordinates. */
    if (frame->tilt_16 == 0)
    {
        rotated_vertex->vx = y;
        rotated_vertex->vy = 0;
        rotated_vertex->vz = x;
        gte_ldv0(rotated_vertex);
        gte_rtv0();
        gte_stlvnl(out);
        quad->x0 = origin->x + out->low.x;
        quad->y0 = origin->y + out->low.y;

        rotated_vertex->vx = y;
        rotated_vertex->vy = 0;
        rotated_vertex->vz = x + width;
        gte_ldv0(rotated_vertex);
        gte_rtv0();
        gte_stlvnl(out);
        quad->x1 = origin->x + out->low.x;
        quad->y1 = origin->y + out->low.y;

        rotated_vertex->vx = y + height;
        rotated_vertex->vy = 0;
        rotated_vertex->vz = x;
        gte_ldv0(rotated_vertex);
        gte_rtv0();
        gte_stlvnl(out);
        quad->x2 = origin->x + out->low.x;
        quad->y2 = origin->y + out->low.y;

        rotated_vertex->vx = y + height;
        rotated_vertex->vy = 0;
        rotated_vertex->vz = x + width;
        gte_ldv0(rotated_vertex);
        gte_rtv0();
        gte_stlvnl(out);
        quad->x3 = origin->x + out->low.x;
        quad->y3 = origin->y + out->low.y;
        return;
    }

    if (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
    {
        tilt_matrix->packed.rotation[4] = ONE;
        tilt_matrix->packed.rotation[2] = ONE;
        tilt_matrix->packed.rotation[0] = ONE;
        tilt_matrix->matrix.t[2] = 0;
        tilt_matrix->matrix.t[1] = 0;
        tilt_matrix->matrix.t[0] = 0;
        tilt_matrix->packed.rotation[3] = 0;
        tilt_matrix->packed.rotation[1] = 0;

        vertex->vx = y;
        vertex->vy = 0;
        vertex->vz = x + width;
        gte_SetRotMatrix(&tilt_matrix->matrix);
        gte_ldv0(vertex);
        gte_rtv0();
        gte_stsv(rotated_vertex);
        gte_SetRotMatrix(matrix);
        gte_ldv0(rotated_vertex);
        gte_rtv0();
        gte_stlvnl(out);
        quad->x1 = origin->x + out->low.x;
        quad->y1 = origin->y + out->low.y;

        vertex->vx = 0;
        vertex->vy = -(frame->tilt_16 << 4);
        vertex->vz = 0;
        RotMatrix_gte(vertex, &tilt_matrix->matrix);

        vertex->vx = 0;
        vertex->vy = 0;
        vertex->vz = -width;
        gte_SetRotMatrix(&tilt_matrix->matrix);
        gte_ldv0(vertex);
        gte_rtv0();
        gte_stsv(rotated_vertex);
        gte_SetRotMatrix(matrix);
        gte_ldv0(rotated_vertex);
        gte_rtv0();
        gte_stlvnl(out);
        quad->x0 = quad->x1 + out->low.x;
        quad->y0 = quad->y1 + out->low.y;

        vertex->vx = height;
        vertex->vy = 0;
        vertex->vz = 0;
        gte_SetRotMatrix(&tilt_matrix->matrix);
        gte_ldv0(vertex);
        gte_rtv0();
        gte_stsv(rotated_vertex);
        gte_SetRotMatrix(matrix);
        gte_ldv0(rotated_vertex);
        gte_rtv0();
        gte_stlvnl(out);
        quad->x3 = quad->x1 + out->low.x;
        quad->y3 = quad->y1 + out->low.y;

        vertex->vx = height;
        vertex->vy = 0;
        vertex->vz = -width;
        gte_SetRotMatrix(&tilt_matrix->matrix);
        gte_ldv0(vertex);
        gte_rtv0();
        gte_stsv(rotated_vertex);
        gte_SetRotMatrix(matrix);
        gte_ldv0(rotated_vertex);
        gte_rtv0();
        gte_stlvnl(out);
        quad->x2 = quad->x1 + out->low.x;
        quad->y2 = quad->y1 + out->low.y;
    }
    else
    {
        tilt_matrix->packed.rotation[4] = ONE;
        tilt_matrix->packed.rotation[2] = ONE;
        tilt_matrix->packed.rotation[0] = ONE;
        tilt_matrix->matrix.t[2] = 0;
        tilt_matrix->matrix.t[1] = 0;
        tilt_matrix->matrix.t[0] = 0;
        tilt_matrix->packed.rotation[3] = 0;
        tilt_matrix->packed.rotation[1] = 0;

        vertex->vx = y;
        vertex->vy = 0;
        vertex->vz = x;
        gte_SetRotMatrix(&tilt_matrix->matrix);
        gte_ldv0(vertex);
        gte_rtv0();
        gte_stsv(rotated_vertex);
        gte_SetRotMatrix(matrix);
        gte_ldv0(rotated_vertex);
        gte_rtv0();
        gte_stlvnl(out);
        quad->x0 = origin->x + out->low.x;
        quad->y0 = origin->y + out->low.y;

        vertex->vx = 0;
        vertex->vy = frame->tilt_16 << 4;
        vertex->vz = 0;
        RotMatrix_gte(vertex, &tilt_matrix->matrix);

        vertex->vx = 0;
        vertex->vy = 0;
        vertex->vz = width;
        gte_SetRotMatrix(&tilt_matrix->matrix);
        gte_ldv0(vertex);
        gte_rtv0();
        gte_stsv(rotated_vertex);
        gte_SetRotMatrix(matrix);
        gte_ldv0(rotated_vertex);
        gte_rtv0();
        gte_stlvnl(out);
        quad->x1 = quad->x0 + out->low.x;
        quad->y1 = quad->y0 + out->low.y;

        vertex->vx = height;
        vertex->vy = 0;
        vertex->vz = 0;
        gte_SetRotMatrix(&tilt_matrix->matrix);
        gte_ldv0(vertex);
        gte_rtv0();
        gte_stsv(rotated_vertex);
        gte_SetRotMatrix(matrix);
        gte_ldv0(rotated_vertex);
        gte_rtv0();
        gte_stlvnl(out);
        quad->x2 = quad->x0 + out->low.x;
        quad->y2 = quad->y0 + out->low.y;

        vertex->vx = height;
        vertex->vy = 0;
        vertex->vz = width;
        gte_SetRotMatrix(&tilt_matrix->matrix);
        gte_ldv0(vertex);
        gte_rtv0();
        gte_stsv(rotated_vertex);
        gte_SetRotMatrix(matrix);
        gte_ldv0(rotated_vertex);
        gte_rtv0();
        gte_stlvnl(out);
        quad->x3 = quad->x0 + out->low.x;
        quad->y3 = quad->y0 + out->low.y;
    }
}

/**
 * @brief Decode signed byte geometry corners and optionally mirror them.
 * @param out Receives four interleaved x/y pairs.
 * @param flip Nonzero to reverse corner order and negate x coordinates.
 * @param item Packed corner record; byte 7 is not a coordinate.
 */
void field_unpack_effect_quad_corners8(Vec2s* out, s32 flip, s8* item)
{
    if (flip)
    {
        out[0].x = -item[2];
        out[0].y = item[3];
        out[1].x = -item[0];
        out[1].y = item[1];
        out[2].x = -item[6];
        out[2].y = item[8];
        out[3].x = -item[4];
        out[3].y = item[5];
    }
    else
    {
        out[0].x = item[0];
        out[0].y = item[1];
        out[1].x = item[2];
        out[1].y = item[3];
        out[2].x = item[4];
        out[2].y = item[5];
        out[3].x = item[6];
        out[3].y = item[8];
    }
}

/**
 * @brief Decode little-endian geometry corners and optionally mirror them.
 * @param out Receives four interleaved x/y pairs.
 * @param mirror Nonzero to reverse corner order and negate x coordinates.
 * @param item Packed corner record; byte 7 interrupts the second y component.
 */
void field_unpack_effect_quad_corners16(Vec2s* out, s32 mirror, u8* item)
{
    if (mirror != 0)
    {
        out[0].x = -(item[4] + (item[5] << 8));
        out[0].y = item[6] + (item[8] << 8);
        out[1].x = -(item[0] + (item[1] << 8));
        out[1].y = item[2] + (item[3] << 8);
        out[2].x = -(item[13] + (item[14] << 8));
        out[2].y = item[15] + (item[16] << 8);
        out[3].x = -(item[9] + (item[10] << 8));
        out[3].y = item[11] + (item[12] << 8);
    }
    else
    {
        out[0].x = item[0] + (item[1] << 8);
        out[0].y = item[2] + (item[3] << 8);
        out[1].x = item[4] + (item[5] << 8);
        out[1].y = item[6] + (item[8] << 8);
        out[2].x = item[9] + (item[10] << 8);
        out[2].y = item[11] + (item[12] << 8);
        out[3].x = item[13] + (item[14] << 8);
        out[3].y = item[15] + (item[16] << 8);
    }
}

/**
 * @brief Resolve a part's extent from tracks, distance, randomness, and bounds.
 * @param actor Owner supplying tracks and linked object indices.
 * @param part Packed extent value, modifiers, and attachment category.
 * @return Extent after scaling and optional bounding-box half-extents.
 * @note Sibling-offset placements (FIELD_PLACEMENT_SIBLING_OFFSET to +7) never set part_index, so the
 *       half-extent test reads parts[] at an uninitialized index (a bug in the original code).
 */
s32 field_resolve_effect_extent(FieldActorSlot* actor, FieldObjectPart* part)
{
    VECTOR delta;
    VECTOR squared_delta;
    s32 extent;
    s32 part_index;
    s32 half_extent;
    u32 flags;
    FieldActorSlot* owner;

    {
        u32 value_flags = part->rotation_extent.word;
        s32 value_mode = (value_flags >> FIELD_PART_ROTATION_MODE_SHIFT) & 3;
        owner = actor;

        switch (value_mode)
        {
            case FIELD_PART_EXTENT_CONSTANT:
            {
                s32 value_low_bits = part->palette_extent.word >> 29;
                extent = ((value_flags & 0x3F) << 3) | value_low_bits;
                break;
            }
            case FIELD_PART_EXTENT_TRACK:
            {
                u32 value_low_bits = part->palette_extent.word >> 29;
                extent = field_evaluate_parameter_track(owner, (((value_flags & 0x3F) << 3) | value_low_bits) & FIELD_TRACK_INDEX_MASK);
                break;
            }
            case FIELD_PART_EXTENT_TRACK_START:
            {
                u32 value_low_bits = part->palette_extent.word;
                extent = field_evaluate_parameter_track_at_time(owner, (((value_flags & 0x3F) << 3) | (value_low_bits >> 29)) & FIELD_TRACK_INDEX_MASK, 0);
                break;
            }
        }
    }

    if (part->effect_flags.word & FIELD_PART_SCALE_DISTANCE_BY_TARGET)
    {
        delta.vx = (g_field_actors[owner->targets[g_field_track_index]].x - g_field_actors[owner->owner_object_index].x) >> 8;
        delta.vy = (g_field_actors[owner->targets[g_field_track_index]].y - g_field_actors[owner->owner_object_index].y) >> 8;
        delta.vz = (g_field_actors[owner->targets[g_field_track_index]].z - g_field_actors[owner->owner_object_index].z) >> 8;
        gte_ldlvl(&delta);
        gte_sqr0();
        gte_stlvnl(&squared_delta);
        extent = (SquareRoot0(squared_delta.vx + squared_delta.vy + squared_delta.vz) * extent) / 100;
    }

    if ((part->placement_flags.word >> FIELD_PART_RANDOM_EXTENT_SHIFT) & 1)
    {
        extent = (extent * rand()) >> 15;
    }

    if (part->palette_extent.word & FIELD_PART_SCALE_DISTANCE_BY_OWNER)
    {
        extent = ((g_field_object_states[owner->owner_object_index].movement.half.flags & FIELD_MOVEMENT_SCALE_MASK) * extent) / 100;
    }

    flags = part->placement_flags.word;
    {
        s32 category = (flags >> FIELD_PART_PLACEMENT_KIND_SHIFT) & FIELD_PART_PLACEMENT_KIND_MASK;

        if (category < FIELD_PLACEMENT_SIBLING)
        {
            if ((flags >> FIELD_PART_ADD_HALF_WIDTH_SHIFT) & 1)
            {
                if ((u32)(category - FIELD_PLACEMENT_TRACK_BOUNDS) >= FIELD_ATTACHMENT_LINKED_BOUNDS_COUNT)
                {
                    half_extent = (g_field_object_states[owner->owner_object_index].bounds.half.right -
                                   g_field_object_states[owner->owner_object_index].bounds.half.left) >>
                                  1;
                }
                else
                {
                    half_extent = (g_field_object_states[owner->targets[g_field_track_index]].bounds.half.right -
                                   g_field_object_states[owner->targets[g_field_track_index]].bounds.half.left) >>
                                  1;
                }

                if (half_extent < 0)
                {
                    half_extent = -half_extent;
                }

                extent += half_extent;
            }

            if ((part->placement_flags.word >> FIELD_PART_ADD_HALF_HEIGHT_SHIFT) & 1)
            {
                if ((u32)(FIELD_PART_PLACEMENT_KIND(part) - FIELD_PLACEMENT_TRACK_BOUNDS) >= FIELD_ATTACHMENT_LINKED_BOUNDS_COUNT)
                {
                    half_extent = (g_field_object_states[owner->owner_object_index].bounds.half.bottom -
                                   g_field_object_states[owner->owner_object_index].bounds.half.top) >>
                                  1;
                }
                else
                {
                    half_extent = (g_field_object_states[owner->targets[g_field_track_index]].bounds.half.bottom -
                                   g_field_object_states[owner->targets[g_field_track_index]].bounds.half.top) >>
                                  1;
                }

                if (half_extent < 0)
                {
                    half_extent = -half_extent;
                }

                extent += half_extent;
            }
        }
    }

    {
        s32 category = FIELD_PART_PLACEMENT_KIND(part);

        if ((u32)(category - FIELD_PLACEMENT_SIBLING_HIGH) < FIELD_PLACEMENT_SIBLING_COUNT ||
            (u32)(category - FIELD_PLACEMENT_SIBLING) < FIELD_PLACEMENT_SIBLING_COUNT ||
            (u32)(category - FIELD_PLACEMENT_SIBLING_OFFSET) < FIELD_PLACEMENT_SIBLING_COUNT)
        {
            s32 attachment_category = FIELD_PART_PLACEMENT_KIND(part);

            if (attachment_category < FIELD_PLACEMENT_SIBLING_OFFSET)
            {
                part_index = attachment_category - FIELD_PLACEMENT_SIBLING;

                if (attachment_category >= FIELD_PLACEMENT_SIBLING_HIGH)
                {
                    part_index = attachment_category - (FIELD_PLACEMENT_SIBLING_HIGH - FIELD_PLACEMENT_SIBLING_COUNT);
                }
            }

            if ((part->placement_flags.halves.high) & 1)
            {
                if (owner->parts[part_index].effect_kind == FIELD_PART_OWNER_BOUNDS)
                {
                    half_extent = (g_field_object_states[owner->owner_object_index].bounds.half.right -
                                   g_field_object_states[owner->owner_object_index].bounds.half.left) >>
                                  1;
                }
                else
                {
                    half_extent = (g_field_object_states[owner->targets[g_field_track_index]].bounds.half.right -
                                   g_field_object_states[owner->targets[g_field_track_index]].bounds.half.left) >>
                                  1;
                }

                if (half_extent < 0)
                {
                    half_extent = -half_extent;
                }

                extent += half_extent;
            }

            if ((part->placement_flags.word >> FIELD_PART_ADD_HALF_HEIGHT_SHIFT) & 1)
            {
                if (owner->parts[part_index].effect_kind == FIELD_PART_OWNER_BOUNDS)
                {
                    half_extent = (g_field_object_states[owner->owner_object_index].bounds.half.bottom -
                                   g_field_object_states[owner->owner_object_index].bounds.half.top) >>
                                  1;
                }
                else
                {
                    half_extent = (g_field_object_states[owner->targets[g_field_track_index]].bounds.half.bottom -
                                   g_field_object_states[owner->targets[g_field_track_index]].bounds.half.top) >>
                                  1;
                }

                if (half_extent < 0)
                {
                    half_extent = -half_extent;
                }

                extent += half_extent;
            }
        }
    }

    return extent;
}

/* Actor-attached effect geometry: part anchors, quad corners, and screen-space vertex caches. */

/**
 * @brief Resolve the world-space anchor point for an actor part.
 * @param actor Owning actor state.
 * @param part Actor part definition selecting the anchor mode.
 * @param out Receives the resolved x/y/z anchor.
 * @param attachment_index Attachment point index used by ground-relative anchor modes.
 */
void field_resolve_actor_part_anchor(FieldActorSlot* actor, FieldObjectPart* part, Vec3i* out, s32 attachment_index)
{
    s32 anchor_mode;
    s32 object_index;
    FieldMotionRecord* object_record;
    FieldObjectState* object;
    s32 value;
    s32 index;

    anchor_mode = part->placement_flags.word >> FIELD_PART_PLACEMENT_KIND_SHIFT;
    anchor_mode &= FIELD_PART_PLACEMENT_KIND_MASK;

    switch (anchor_mode)
    {
        case FIELD_PLACEMENT_OWNER_BOUNDS:
        case FIELD_PLACEMENT_OWNER_BOUNDS + 1:
        case FIELD_PLACEMENT_OWNER_BOUNDS + 2:
        case FIELD_PLACEMENT_OWNER_BOUNDS + 3:
        case FIELD_PLACEMENT_OWNER_BOUNDS + 4:
        case FIELD_PLACEMENT_OWNER_BOUNDS + 5:
        case FIELD_PLACEMENT_OWNER_BOUNDS + 6:
        case FIELD_PLACEMENT_OWNER_BOUNDS + 7:
        case FIELD_PLACEMENT_OWNER_BOUNDS + 8:
        case FIELD_PLACEMENT_OWNER_BOUNDS + 9:
        case FIELD_PLACEMENT_TRACK_BOUNDS:
        case FIELD_PLACEMENT_TRACK_BOUNDS + 1:
        case FIELD_PLACEMENT_TRACK_BOUNDS + 2:
        case FIELD_PLACEMENT_TRACK_BOUNDS + 3:
        case FIELD_PLACEMENT_TRACK_BOUNDS + 4:
        case FIELD_PLACEMENT_TRACK_BOUNDS + 5:
        case FIELD_PLACEMENT_TRACK_BOUNDS + 6:
        case FIELD_PLACEMENT_TRACK_BOUNDS + 7:
        case FIELD_PLACEMENT_TRACK_BOUNDS + 8:
        case FIELD_PLACEMENT_TRACK_BOUNDS + 9:
        {
            if (anchor_mode >= FIELD_PLACEMENT_TRACK_BOUNDS)
            {
                object_index = actor->targets[g_field_track_index];
                anchor_mode -= FIELD_PLACEMENT_TRACK_BOUNDS;
                object_record = &FIELD_ACTOR_MOTION_RECORDS[object_index];
                object = &g_field_object_states[object_index];
            }
            else
            {
                object_index = actor->owner_object_index;
                object_record = &FIELD_ACTOR_MOTION_RECORDS[object_index];
                object = &g_field_object_states[object_index];
            }

            if ((part->placement_flags.word >> FIELD_PART_SCALE_X_FROM_BOUNDS_SHIFT) & 1)
            {
                part->appearance.fields.scale_xz = (*(u8*)&object->bounds.half.right - *(u8*)&object->bounds.half.left) * 2;
            }

            if ((part->placement_flags.word >> FIELD_PART_SCALE_Y_FROM_BOUNDS_SHIFT) & 1)
            {
                index = 0;
                part->scale_y = (*(u8*)&object->bounds.half.bottom - *(u8*)&object->bounds.half.top) * 2;
            }
            else
            {
                index = 0;
            }

            value = index;

            switch (anchor_mode)
            {
                case FIELD_BOUNDS_POINT_CENTER:
                    value = (object->bounds.half.right + object->bounds.half.left) >> 1;
                    index = (object->bounds.half.bottom + object->bounds.half.top) >> 1;
                    break;
                case FIELD_BOUNDS_POINT_CENTER_X:
                    value = (object->bounds.half.right + object->bounds.half.left) >> 1;
                    index = 0;
                    break;
                case FIELD_BOUNDS_POINT_TOP_CENTER:
                    value = (object->bounds.half.right + object->bounds.half.left) >> 1;
                    index = object->bounds.half.top;
                    break;
                case FIELD_BOUNDS_POINT_LEFT_CENTER:
                    value = object->bounds.half.left;
                    index = (object->bounds.half.bottom + object->bounds.half.top) >> 1;
                    break;
                case FIELD_BOUNDS_POINT_RIGHT_CENTER:
                    value = object->bounds.half.right;
                    index = (object->bounds.half.bottom + object->bounds.half.top) >> 1;
                    break;
                case FIELD_BOUNDS_POINT_TOP_LEFT:
                    value = object->bounds.half.left;
                    index = object->bounds.half.top;
                    break;
                case FIELD_BOUNDS_POINT_TOP_RIGHT:
                    value = object->bounds.half.right;
                    index = object->bounds.half.top;
                    break;
                case FIELD_BOUNDS_POINT_BOTTOM_LEFT:
                    value = object->bounds.half.left;
                    index = object->bounds.half.bottom;
                    break;
                case FIELD_BOUNDS_POINT_BOTTOM_RIGHT:
                    value = object->bounds.half.right;
                    index = object->bounds.half.bottom;
                    break;
            }

            value <<= 8;
            out->x = object_record->x + value;
            index <<= 8;
            out->y = object_record->y + index;
            out->z = object_record->z;
            return;
        }

        case FIELD_PLACEMENT_SIBLING:
        case FIELD_PLACEMENT_SIBLING + 1:
        case FIELD_PLACEMENT_SIBLING + 2:
        case FIELD_PLACEMENT_SIBLING + 3:
        case FIELD_PLACEMENT_SIBLING + 4:
        case FIELD_PLACEMENT_SIBLING + 5:
        case FIELD_PLACEMENT_SIBLING + 6:
        case FIELD_PLACEMENT_SIBLING + 7:
        case FIELD_PLACEMENT_SIBLING_HIGH:
        case FIELD_PLACEMENT_SIBLING_HIGH + 1:
        case FIELD_PLACEMENT_SIBLING_HIGH + 2:
        case FIELD_PLACEMENT_SIBLING_HIGH + 3:
        case FIELD_PLACEMENT_SIBLING_HIGH + 4:
        case FIELD_PLACEMENT_SIBLING_HIGH + 5:
        case FIELD_PLACEMENT_SIBLING_HIGH + 6:
        case FIELD_PLACEMENT_SIBLING_HIGH + 7:
            value = anchor_mode - 0x14;

            if (anchor_mode >= 0x2A)
            {
                value = anchor_mode - 0x22;
            }

            for (index = 0; index < FIELD_EFFECT_ACTIVE_RECORD_COUNT; index++)
            {
                if (g_field_effect_records[index].state != FIELD_EFFECT_RETIRED && g_field_effect_records[index].owner_part == value &&
                    g_field_effect_records[index].owner_slot == actor->slot_index && g_field_effect_records[index].track_index == 0)
                {
                    out->x = g_field_effect_records[index].x;
                    out->y = g_field_effect_records[index].y;
                    out->z = g_field_effect_records[index].z;
                    return;
                }
            }

            return;

        case FIELD_PLACEMENT_FIELD_POSITION:
            out->x = (part->offset_x << 8) - g_field_view_offset_x;
            out->y = (part->offset_y << 8) - g_field_view_offset_y;
            out->z = (part->offset_z << 8) - g_field_view_offset_z;
            return;
        case FIELD_PLACEMENT_SCREEN_CENTER:
            out->x = -g_field_view_offset_x;
            out->y = -g_field_view_offset_y;
            out->z = -g_field_view_offset_z;
            return;
        case FIELD_PLACEMENT_SCREEN_TOP:
            out->y = -FIELD_SCREEN_HALF_HEIGHT_FIXED;
            out->x = -g_field_view_offset_x;
            out->y -= g_field_view_offset_y;
            out->z = -g_field_view_offset_z;
            return;
        case FIELD_PLACEMENT_SCREEN_BOTTOM:
            out->y = FIELD_SCREEN_HALF_HEIGHT_FIXED;
            out->x = -g_field_view_offset_x;
            out->y -= g_field_view_offset_y;
            out->z = -g_field_view_offset_z;
            return;
        case FIELD_PLACEMENT_SCREEN_LEFT:
            out->x = -FIELD_SCREEN_HALF_WIDTH_FIXED;
            out->x -= g_field_view_offset_x;
            out->y = -g_field_view_offset_y;
            out->z = -g_field_view_offset_z;
            return;
        case FIELD_PLACEMENT_SCREEN_RIGHT:
            out->x = FIELD_SCREEN_HALF_WIDTH_FIXED;
            out->x -= g_field_view_offset_x;
            out->y = -g_field_view_offset_y;
            out->z = -g_field_view_offset_z;
            return;
        case FIELD_PLACEMENT_SCREEN_TOP_LEFT:
            out->x = -FIELD_SCREEN_HALF_WIDTH_FIXED;
            out->y = -FIELD_SCREEN_HALF_HEIGHT_FIXED;
            out->x -= g_field_view_offset_x;
            out->y -= g_field_view_offset_y;
            out->z = -g_field_view_offset_z;
            return;
        case FIELD_PLACEMENT_SCREEN_TOP_RIGHT:
            out->x = FIELD_SCREEN_HALF_WIDTH_FIXED;
            out->y = -FIELD_SCREEN_HALF_HEIGHT_FIXED;
            out->x -= g_field_view_offset_x;
            out->y -= g_field_view_offset_y;
            out->z = -g_field_view_offset_z;
            return;
        case FIELD_PLACEMENT_SCREEN_BOTTOM_LEFT:
            out->x = -FIELD_SCREEN_HALF_WIDTH_FIXED;
            out->y = FIELD_SCREEN_HALF_HEIGHT_FIXED;
            out->x -= g_field_view_offset_x;
            out->y -= g_field_view_offset_y;
            out->z = -g_field_view_offset_z;
            return;
        case FIELD_PLACEMENT_SCREEN_BOTTOM_RIGHT:
            out->x = FIELD_SCREEN_HALF_WIDTH_FIXED;
            out->y = FIELD_SCREEN_HALF_HEIGHT_FIXED;
            out->x -= g_field_view_offset_x;
            out->y -= g_field_view_offset_y;
            out->z = -g_field_view_offset_z;
            return;
        case FIELD_PLACEMENT_NONE:
            return;

        case FIELD_PLACEMENT_OWNER_OFFSET:
            object_record = &FIELD_ACTOR_MOTION_RECORDS[actor->owner_object_index];

            if ((((part->placement_flags.word >> FIELD_PART_MIRROR_X_WITH_FACING_SHIFT) & 1) || (part->spawn_flags.word & FIELD_PART_MIRROR_X_WITH_OWNER)) &&
                !(object_record->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED))
            {
                out->x = object_record->x - (part->offset_x << 8);
            }
            else
            {
                out->x = object_record->x + (part->offset_x << 8);
            }

            out->y = object_record->y + (part->offset_y << 8);
            out->z = object_record->z + (part->offset_z << 8);
            return;

        case FIELD_PLACEMENT_TRACK_OFFSET:
            object_record = &FIELD_ACTOR_MOTION_RECORDS[actor->targets[g_field_track_index]];

            if ((part->spawn_flags.word & FIELD_PART_MIRROR_X_WITH_OWNER) &&
                !(g_field_actors[actor->owner_object_index].animation & FIELD_EFFECT_FACING_FLIPPED))
            {
                out->x = object_record->x - (part->offset_x << 8);
            }
            else if (((part->placement_flags.word >> FIELD_PART_MIRROR_X_WITH_FACING_SHIFT) & 1) &&
                     !(object_record->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED))
            {
                out->x = object_record->x - (part->offset_x << 8);
            }
            else
            {
                out->x = object_record->x + (part->offset_x << 8);
            }

            out->y = object_record->y + (part->offset_y << 8);
            out->z = object_record->z + (part->offset_z << 8);
            return;

        case FIELD_PLACEMENT_OWNER_ATTACHMENT:
            object_index = actor->owner_object_index;
            object_record = &FIELD_ACTOR_MOTION_RECORDS[object_index];
            object = &g_field_object_states[object_index];
            out->x = object_record->x +
                     (object->attachment_points[(part->effect_flags.word >> FIELD_PART_ATTACHMENT_INDEX_SHIFT) & FIELD_PART_ATTACHMENT_INDEX_MASK].x << 8);
            out->y = object_record->y +
                     (object->attachment_points[(part->effect_flags.word >> FIELD_PART_ATTACHMENT_INDEX_SHIFT) & FIELD_PART_ATTACHMENT_INDEX_MASK].y << 8);
            out->z = object_record->z;
            return;

        case FIELD_PLACEMENT_OWNER_ATTACHMENT_X:
            object_index = actor->owner_object_index;
            object_record = &FIELD_ACTOR_MOTION_RECORDS[object_index];
            object = &g_field_object_states[object_index];
            out->x = object_record->x +
                     (object->attachment_points[(part->effect_flags.word >> FIELD_PART_ATTACHMENT_INDEX_SHIFT) & FIELD_PART_ATTACHMENT_INDEX_MASK].x << 8);
            out->y = object_record->y;
            out->z = object_record->z + (part->offset_z << 8);

            if (((part->placement_flags.word >> FIELD_PART_MIRROR_X_WITH_FACING_SHIFT) & 1) &&
                !(object_record->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED))
            {
                out->x -= part->offset_x << 8;
            }
            else
            {
                out->x += part->offset_x << 8;
            }

            return;

        case FIELD_PLACEMENT_GROUND_POINT:
            object_index = actor->owner_object_index;
            object_record = &FIELD_ACTOR_MOTION_RECORDS[object_index];
            object = &g_field_object_states[object_index];
            out->x = object_record->x + (object->ground_effect_points[attachment_index].x << 8);
            out->y = object_record->y;
            out->z = object_record->z + (object->ground_effect_points[attachment_index].y << 8);
            return;

        case FIELD_PLACEMENT_PART_OFFSET:
            out->x = part->offset_x << 8;
            out->y = part->offset_y << 8;
            out->z = part->offset_z << 8;
            return;

        case FIELD_PLACEMENT_SIBLING_OFFSET:
        case FIELD_PLACEMENT_SIBLING_OFFSET + 1:
        case FIELD_PLACEMENT_SIBLING_OFFSET + 2:
        case FIELD_PLACEMENT_SIBLING_OFFSET + 3:
        case FIELD_PLACEMENT_SIBLING_OFFSET + 4:
        case FIELD_PLACEMENT_SIBLING_OFFSET + 5:
        case FIELD_PLACEMENT_SIBLING_OFFSET + 6:
        case FIELD_PLACEMENT_SIBLING_OFFSET + 7:
            value = anchor_mode - 0x37;

            for (index = 0; index < FIELD_EFFECT_ACTIVE_RECORD_COUNT; index++)
            {
                if (g_field_effect_records[index].state != FIELD_EFFECT_RETIRED && g_field_effect_records[index].owner_part == value &&
                    g_field_effect_records[index].owner_slot == actor->slot_index && g_field_effect_records[index].track_index == 0)
                {
                    out->x = g_field_effect_records[index].x;
                    out->y = g_field_effect_records[index].y;
                    out->z = g_field_effect_records[index].z;
                }
            }

            return;

        default:
            out->x = 0;
            out->y = 0;
            out->z = 0;
            return;
    }
}

/**
 * @brief Extract four signed-byte quad corners from an effect frame.
 * @param effect Effect record whose facing flag selects the mirrored layout.
 * @param frame_data Encoded frame entry list; the first byte is the entry count.
 * @param corners Receives four x/y corner pairs.
 */
void field_extract_effect_quad_corners8(FieldMotionRecord* effect, u8* frame_data, Vec2s* corners)
{
    s32 count;

    corners[0].x = corners[0].y = 0;
    corners[1].x = corners[1].y = 0;
    corners[2].x = corners[2].y = 0;
    corners[3].x = corners[3].y = 0;

    for (count = *frame_data++; count != 0; frame_data += 9, count--)
    {
        if (frame_data[7] & FIELD_SPRITE_FRAME_FOOTPRINT)
        {
            if ((frame_data[7] & 0xF) == 2)
            {
                if (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
                {
                    corners[0].x = -(s8)frame_data[2];
                    corners[0].y = (s8)frame_data[3];
                    corners[1].x = -(s8)frame_data[0];
                    corners[1].y = (s8)frame_data[1];
                    corners[2].x = -(s8)frame_data[6];
                    corners[2].y = (s8)frame_data[8];
                    corners[3].x = -(s8)frame_data[4];
                    corners[3].y = (s8)frame_data[5];
                }
                else
                {
                    corners[0].x = (s8)frame_data[0];
                    corners[0].y = (s8)frame_data[1];
                    corners[1].x = (s8)frame_data[2];
                    corners[1].y = (s8)frame_data[3];
                    corners[2].x = (s8)frame_data[4];
                    corners[2].y = (s8)frame_data[5];
                    corners[3].x = (s8)frame_data[6];
                    corners[3].y = (s8)frame_data[8];
                }
            }
        }
    }
}

/**
 * @brief Extract four 16-bit quad corners from a variable-stride effect frame.
 * @param effect Effect record whose facing flag selects the mirrored layout.
 * @param frame_data Encoded frame entry list; the first byte is the entry count.
 * @param corners Receives four x/y corner pairs.
 */
void field_extract_effect_quad_corners16(FieldMotionRecord* effect, u8* frame_data, Vec2s* corners)
{
    s32 count;

    corners[0].x = corners[0].y = 0;
    corners[1].x = corners[1].y = 0;
    corners[2].x = corners[2].y = 0;
    corners[3].x = corners[3].y = 0;

    for (count = *frame_data++; count != 0; count--)
    {
        if (frame_data[7] & FIELD_SPRITE_FRAME_FOOTPRINT)
        {
            if ((frame_data[7] & 0xF) == 2)
            {
                field_unpack_effect_quad_corners16(corners, effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED, frame_data);
            }

            frame_data += 0x11;
        }
        else
        {
            frame_data += 0xB;
        }
    }
}

/**
 * @brief Apply the actor rotation to a bounding-box centre and accumulate it
 *        into the screen-space position.
 * @param effect Effect record whose facing flag mirrors the horizontal center offset.
 * @param screen_origin Screen-space position accumulator.
 * @param quad_bounds Bounding-box extents (indices 0/1/2/5 used in mode 1).
 * @param fallback_depth Horizontal offset used when mode does not derive it from the quad.
 * @param mode Zero skips adjustment; one derives the center from the quad; other values use fallback_depth.
 */
void field_apply_effect_quad_center_offset(FieldMotionRecord* effect, Vec2s* screen_origin, Vec2s* quad_bounds, s32 fallback_depth, s32 mode)
{
    SVECTOR offset;
    VECTOR rotated;
    s32 center_y;
    s32 center_x = fallback_depth;

    if (mode != 0)
    {
        if (mode == 1)
        {
            center_x = (quad_bounds[1].x + quad_bounds[0].x) / 2;
            center_y = (quad_bounds[2].y + quad_bounds[0].y) / 2;
        }

        offset.vx = -center_y;
        offset.vy = 0;
        offset.vz = -center_x;
        gte_ldv0(&offset);
        gte_rtv0();
        gte_stlvnl(&rotated);

        if (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
        {
            center_x = -center_x;
        }

        screen_origin->x += rotated.vx + center_x;
        screen_origin->y += rotated.vy + center_y;
    }
}

/**
 * @brief Transform four signed-byte quad vertices into screen space.
 * @param effect Effect record whose facing flag mirrors the horizontal component.
 * @param object Object placement receiving the transformed vertices.
 * @param quad_data Encoded signed-byte corner pairs.
 * @param vertex_index Base vertex index into object->effect_vertices.
 * @param screen_origin Screen-space origin added to every transformed vertex.
 * @param direction Scratch direction vector fed to the GTE.
 * @param gte_out Scratch GTE output vector.
 */
void field_transform_effect_quad_vertices8(FieldMotionRecord* effect, FieldObjectState* object, u8* quad_data, s32 vertex_index, Vec2s* screen_origin,
                                           SVECTOR* direction, VECTOR* gte_out)
{
    s16 depth_component;

    if (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
    {
        direction->vx = (s8)quad_data[1];
        direction->vy = 0;
        depth_component = -(s8)quad_data[0];
    }
    else
    {
        direction->vx = (s8)quad_data[1];
        direction->vy = 0;
        depth_component = (s8)quad_data[0];
    }

    direction->vz = depth_component;
    gte_ldv0(direction);
    gte_rtv0();
    gte_stlvnl(gte_out);

    if (vertex_index == 0)
    {
        object->bounds.half.left = (u16)gte_out->vx;
        object->bounds.half.top = (u16)gte_out->vy - (s8)effect->vertical_offset;
    }

    object->effect_vertices.points[vertex_index].x = (u16)gte_out->vx + (u16)screen_origin->x;
    object->effect_vertices.points[vertex_index].y = (u16)gte_out->vy + (u16)screen_origin->y;

    if (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
    {
        direction->vx = (s8)quad_data[3];
        direction->vy = 0;
        depth_component = -(s8)quad_data[2];
    }
    else
    {
        direction->vx = (s8)quad_data[3];
        direction->vy = 0;
        depth_component = (s8)quad_data[2];
    }

    direction->vz = depth_component;
    gte_ldv0(direction);
    gte_rtv0();
    gte_stlvnl(gte_out);
    object->effect_vertices.points[vertex_index + 1].x = (u16)gte_out->vx + (u16)screen_origin->x;
    object->effect_vertices.points[vertex_index + 1].y = (u16)gte_out->vy + (u16)screen_origin->y;

    if (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
    {
        direction->vx = (s8)quad_data[5];
        direction->vy = 0;
        depth_component = -(s8)quad_data[4];
    }
    else
    {
        direction->vx = (s8)quad_data[5];
        direction->vy = 0;
        depth_component = (s8)quad_data[4];
    }

    direction->vz = depth_component;
    gte_ldv0(direction);
    gte_rtv0();
    gte_stlvnl(gte_out);

    if (vertex_index == 0)
    {
        object->bounds.half.right = (u16)gte_out->vx;
        object->bounds.half.bottom = (u16)gte_out->vy - (s8)effect->vertical_offset;
    }

    object->effect_vertices.points[vertex_index + 2].x = (u16)gte_out->vx + (u16)screen_origin->x;
    object->effect_vertices.points[vertex_index + 2].y = (u16)gte_out->vy + (u16)screen_origin->y;

    if (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
    {
        direction->vx = (s8)quad_data[8];
        direction->vy = 0;
        depth_component = -(s8)quad_data[6];
    }
    else
    {
        direction->vx = (s8)quad_data[8];
        direction->vy = 0;
        depth_component = (s8)quad_data[6];
    }

    direction->vz = depth_component;
    gte_ldv0(direction);
    gte_rtv0();
    gte_stlvnl(gte_out);
    object->effect_vertices.points[vertex_index + 3].x = (u16)gte_out->vx + (u16)screen_origin->x;
    object->effect_vertices.points[vertex_index + 3].y = (u16)gte_out->vy + (u16)screen_origin->y;
}

/**
 * @brief Transform four little-endian 16-bit quad vertices into screen space.
 * @param effect Effect record whose facing flag mirrors the horizontal component.
 * @param object Object placement receiving the transformed vertices.
 * @param quad_data Encoded little-endian 16-bit corner pairs.
 * @param vertex_index Base vertex index into object->effect_vertices.
 * @param screen_origin Screen-space origin added to every transformed vertex.
 * @param direction Scratch direction vector fed to the GTE.
 * @param gte_out Scratch GTE output vector.
 */
void field_transform_effect_quad_vertices16(FieldMotionRecord* effect, FieldObjectState* object, u8* quad_data, s32 vertex_index, Vec2s* screen_origin,
                                            SVECTOR* direction, VECTOR* gte_out)
{
    if (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
    {
        direction->vx = quad_data[2] + (quad_data[3] << 8);
        direction->vy = 0;
        direction->vz = -(quad_data[0] + (quad_data[1] << 8));
    }
    else
    {
        direction->vx = quad_data[2] + (quad_data[3] << 8);
        direction->vy = 0;
        direction->vz = quad_data[0] + (quad_data[1] << 8);
    }

    gte_ldv0(direction);
    gte_rtv0();
    gte_stlvnl(gte_out);

    if (vertex_index == 0)
    {
        object->bounds.half.left = gte_out->vx;
        object->bounds.half.top = gte_out->vy;
    }

    object->effect_vertices.points[vertex_index].x = gte_out->vx + screen_origin->x;
    object->effect_vertices.points[vertex_index].y = gte_out->vy + screen_origin->y;

    if (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
    {
        direction->vx = quad_data[6] + (quad_data[8] << 8);
        direction->vy = 0;
        direction->vz = -(quad_data[4] + (quad_data[5] << 8));
    }
    else
    {
        direction->vx = quad_data[6] + (quad_data[8] << 8);
        direction->vy = 0;
        direction->vz = quad_data[4] + (quad_data[5] << 8);
    }

    gte_ldv0(direction);
    gte_rtv0();
    gte_stlvnl(gte_out);
    object->effect_vertices.points[vertex_index + 1].x = gte_out->vx + screen_origin->x;
    object->effect_vertices.points[vertex_index + 1].y = gte_out->vy + screen_origin->y;

    if (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
    {
        direction->vx = quad_data[0xB] + (quad_data[0xC] << 8);
        direction->vy = 0;
        direction->vz = -(quad_data[9] + (quad_data[0xA] << 8));
    }
    else
    {
        direction->vx = quad_data[0xB] + (quad_data[0xC] << 8);
        direction->vy = 0;
        direction->vz = quad_data[9] + (quad_data[0xA] << 8);
    }

    gte_ldv0(direction);
    gte_rtv0();
    gte_stlvnl(gte_out);

    if (vertex_index == 0)
    {
        object->bounds.half.right = gte_out->vx;
        object->bounds.half.bottom = gte_out->vy;
    }

    object->effect_vertices.points[vertex_index + 2].x = gte_out->vx + screen_origin->x;
    object->effect_vertices.points[vertex_index + 2].y = gte_out->vy + screen_origin->y;

    if (effect->facing_or_reward_kind & FIELD_EFFECT_FACING_FLIPPED)
    {
        direction->vx = quad_data[0xF] + (quad_data[0x10] << 8);
        direction->vy = 0;
        direction->vz = -(quad_data[0xD] + (quad_data[0xE] << 8));
    }
    else
    {
        direction->vx = quad_data[0xF] + (quad_data[0x10] << 8);
        direction->vy = 0;
        direction->vz = quad_data[0xD] + (quad_data[0xE] << 8);
    }

    gte_ldv0(direction);
    gte_rtv0();
    gte_stlvnl(gte_out);
    object->effect_vertices.points[vertex_index + 3].x = gte_out->vx + screen_origin->x;
    object->effect_vertices.points[vertex_index + 3].y = gte_out->vy + screen_origin->y;
}
