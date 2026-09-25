#ifndef FIELD_ACTOR_TABLES_H
#define FIELD_ACTOR_TABLES_H

#include "common.h"
#include "field_actor.h"

/**
 * @file field_actor_tables.h
 * @brief Field actor record, object state, animation slot and binding tables
 *        shared by the actor command interpreters.
 *
 * The field keeps three parallel per-object tables indexed the same way:
 * g_field_actors (script and motion state), g_field_object_states (runtime
 * state; its key word identifies the object to scripts) and
 * g_field_object_parts (model part data). Animation actors live in the
 * separate g_field_actor_slots pool; g_field_actor_bindings ties up to three
 * of them to the party objects.
 */

/** @brief Resting value of the HUD shake counters (FieldPlayerRecord::hit_state). */
#define FIELD_HUD_SHAKE_IDLE 0xFF

/** @brief Number of general-purpose animation actor slots. */
#define FIELD_ACTOR_SLOT_COUNT 48

/** @brief First animation actor slot owned by an object (one per object, in object order). */
#define FIELD_OBJECT_SLOT_BASE FIELD_ACTOR_SLOT_COUNT
/** @brief Each object's effect animations play in animation actor slot 64 + object index. */
#define FIELD_OBJECT_EFFECT_SLOT_BASE 64

/** @brief Total number of animation actor slots (general, per-object and per-object effect slots). */
#define FIELD_ACTOR_SLOT_TOTAL 80

/** @brief Number of animation tracks (one per target) an animation actor slot can run. */
#define FIELD_ACTOR_TRACK_COUNT 9

/** @brief Number of model parts an animation actor slot can drive. */
#define FIELD_ACTOR_PART_COUNT 16

/** @brief Target entry of an animation track without a target object. */
#define FIELD_TARGET_NONE 0xFF

/** @brief Number of animation actor bindings (party members 0, 1 and everyone else). */
#define FIELD_ACTOR_BINDING_COUNT 3

/** @brief Sentinel returned by the actor lookups when no object matches. */
#define FIELD_ACTOR_NONE ((FieldActor*)-1)

/** @brief Number of action slots in one resource's action row. */
#define FIELD_RESOURCE_ACTION_COUNT 50

/** @brief Script index of an actor without a running script. */
#define FIELD_SCRIPT_NONE 0xFF

/** @brief Script index of an actor running its object state's private script. */
#define FIELD_SCRIPT_OBJECT 0xFE

/** @brief First resource read id of the streamed animation resources. */
#define FIELD_ANIMATION_RESOURCE_BASE 0x2DC

/** @brief FieldActorBinding::state values. */
#define FIELD_BINDING_IDLE 0
#define FIELD_BINDING_LOADING 1
#define FIELD_BINDING_READY 2


/** @brief Map size words of the fixed field map header block at 0x801ED400. */
typedef struct
{
    s16 width;
    u16 depth;
} FieldMapBounds;

/** @brief Fixed address of the field map header block. */
#define FIELD_MAP_BOUNDS ((FieldMapBounds*)0x801ED400)



/** @brief FieldPlayerRecord::flags bit: the party member is present. */
#define FIELD_PLAYER_ACTIVE 0x1

/** @brief FieldPlayerRecord::character_kind values. */
#define FIELD_PLAYER_KIND_HERO 0
#define FIELD_PLAYER_KIND_PARTNER 1
#define FIELD_PLAYER_KIND_COMPANION 2

/** @brief A 24-bit value word whose top byte holds flags. */
typedef union
{
    s32 word;
    struct
    {
        u32 value : 24;
        u32 unk24 : 7;
        u32 flag31 : 1;
    } bits;
    u8 bytes[4];
} FieldValueWord;

/** @brief Packed movement word: low ten bits are the effect scale, bit 15 a flag, the top half a height. */
typedef union
{
    u32 word;
    struct
    {
        u32 scale : 10;
        u32 unk10 : 5;
        u32 flag15 : 1;
        u32 unk16 : 16;
    } bits;
    struct
    {
        u16 lo;
        s16 hi;
    } half;
} FieldMovementWord;

/** @brief Packed contact word: flag bits, the bound animation actor and the collected target count. */
typedef union
{
    u32 word;
    struct
    {
        u32 flag0 : 1;
        /** @brief The object is linked to linked_object_index, which carries a link flag meanwhile. */
        u32 linked : 1;
        /** @brief Damage scale of the object's hits (0 counts as 1); cleared when an action starts. */
        u32 damage_scale : 3;
        u32 flag5 : 1;
        u32 flag6 : 1;
        u32 flag7 : 1;
        u32 unk8 : 24;
    } bits;
    struct
    {
        u8 flags;
        u8 animation_actor_index;
        u8 controller_index;
        u8 target_count;
    } bytes;
} FieldContactWord;

/** @brief Number of recorded positions in an object's route history. */
#define FIELD_ROUTE_HISTORY_LENGTH 48

/** @brief Number of waypoints the path finder can store for an object. */
#define FIELD_PATH_MAX_POINTS 18

/** @brief One recorded X/Z position (whole units) of an object's route history. */
typedef struct FieldRoutePoint
{
    s16 x;
    s16 z;
} FieldRoutePoint;

/** @brief One fixed-point X/Z waypoint written by the path finder. */
typedef struct
{
    s32 x;
    s32 z;
} FieldPathPoint;

/** @brief Collision footprint word with a halfword view of the extent. */
typedef union
{
    u32 word;
    struct
    {
        s16 center_offset;
        u16 extent;
    } half;
} FieldCollisionWord;

/** @brief Runtime state of one field object (0x23C bytes). */
typedef struct
{
    s32 unk0;
    FieldValueWord unk4;
    FieldValueWord unk8;
    u32 flags;
    s32 group_flags;
    s32 key;
    s16 unk18;
    u8 unk1A[0x3C - 0x1A];
    s32 action_parameter;
    s32 sequence_id;
    s32 sequence_position;
    /** @brief Special attack gauge, 0xFF when full. */
    u16 technique_gauge;
    u16 action_charge;
    /** @brief Bit 0 selects the special attack gauge on the HUD; the second byte is the object's own index. */
    union
    {
        s32 word;
        struct
        {
            u8 flags;
            u8 object_index;
            u8 unk4E;
            u8 unk4F;
        } bytes;
    } hud;
    s32 target_x;
    s32 target_y;
    s32 target_z;
    u8 unk5C[0x64 - 0x5C];
    s32 unk64;
    s32 unk68;
    FieldRoutePoint route_history[FIELD_ROUTE_HISTORY_LENGTH];
    FieldCollisionWord collision;
    u8 unk130[0x140 - 0x130];
    s16 unk140;
    s16 unk142;
    s16 unk144;
    s16 unk146;
    u8 unk148[0x168 - 0x148];
    u8* script;
    s8 unk16C;
    /** @brief Effect record the object's HUD panel follows while it is bound to an animation actor. */
    u8 linked_effect_index;
    /** @brief Entry of the leader's route history this follower walks towards. */
    u8 route_index;
    u8 action;
    u8 linked_object_index;
    u8 command_timer;
    u16 sequence;
    FieldMovementWord movement;
    FieldContactWord contact;
    /** @brief Flags as of the last handler run, compared with flags to find changes. */
    s32 previous_flags;
    u8 targets[13];
    u8 retry_count;
    u8 unk18E;
    u8 unk18F[0x19C - 0x18F];
    /** @brief Floor node cached between collision passes (-1 asks for a fresh search). */
    s32 collision_node;
    s32 collision_flags;
    u16 path_length;
    u16 path_index;
    u8 tint_red;
    u8 tint_green;
    u8 tint_blue;
    u8 tint_timer;
    FieldPathPoint path[FIELD_PATH_MAX_POINTS];
} FieldObjectState;

/** @brief Model part data of one field object or animation actor part (0x48 bytes). */
typedef struct
{
    union
    {
        u32 word;
        struct
        {
            u32 unk0 : 24;
            u32 mode : 2;
            u32 unk26 : 6;
        } bits;
    } unk0;
    union
    {
        u32 word;
        struct
        {
            u32 unk0 : 11;
            u32 flag11 : 1;
            u32 unk12 : 10;
            u32 render_mode : 2;
            u32 unk24 : 8;
        } bits;
        u8 bytes[4];
    } unk4;
    u8 unk8;
    u8 unk9;
    u8 unkA;
    u8 unkB;
    u8 unkC;
    u8 unkD;
    u8 tint_red;
    u8 tint_green;
    u8 tint_blue;
    u8 unk11;
    u8 unk12[2];
    union
    {
        u32 word;
        struct
        {
            u32 unk0 : 4;
            u32 mode : 4;
            u32 unk8 : 24;
        } bits;
        struct
        {
            u16 lo;
            s16 hi;
        } half;
    } unk14;
    s16 unk18;
    u8 unk1A[0x23 - 0x1A];
    u8 unk23;
    union
    {
        u32 word;
        u8 bytes[4];
    } unk24;
    union
    {
        u32 word;
        u8 bytes[4];
    } unk28;
    u8 unk2C;
    u8 unk2D;
    /** @brief Z and X model scale, 0x40 for full size. */
    u8 scale_z;
    u8 unk2F[2];
    u8 unk31;
    u8 unk32;
    u8 scale_x;
    u32 flags;
    u8 unk38[0x48 - 0x38];
} FieldObjectPart;

/** @brief FieldObjectPart::flags bit 23: the object ignores map collision. */
#define FIELD_PART_IGNORE_MAP_COLLISION 0x800000

/** @brief Per-party-member record (0x268 bytes). */
typedef struct FieldPlayerRecord
{
    u8 flags;
    /** @brief Weapon type (item type) of the equipped weapon. */
    u8 weapon_type;
    /** @brief Character within character_kind (partner or companion id). */
    u8 character_id;
    /** @brief FIELD_PLAYER_KIND_* value selecting the resource set. */
    u8 character_kind;
    u8 unk4[0x254 - 4];
    /** @brief CD resource id of the loaded sprite package. */
    u16 resource_id;
    /** @brief Portrait currently cached for this member, 0xFF for none. */
    u8 portrait_index;
    /** @brief Frames left to chain a combo action. */
    u8 combo_timer;
    u8 unk258;
    u8 hit_state;
    u8 unk25A;
    u8 unk25B;
    u8 unk25C;
    u8 unk25D;
    /** @brief Frames spent knocked down; at revive_delay the member revives (the HUD shows it as the recovery gauge). */
    s16 revive_time;
    s16 revive_delay;
    /** @brief Animation, effect resource and sound (-1 for none) used when the revive timer runs out. */
    s16 revive_animation;
    s16 revive_effect;
    s16 revive_sound;
} FieldPlayerRecord;

/** @brief Flag halfword of an action slot. */
typedef struct
{
    u16 target_filter : 8; /**< Target predicate index, FIELD_ACTION_TARGET_NONE for none. */
    u16 target_group : 2;  /**< Target group mode (opposite / same group). */
    u16 instrument : 1;    /**< Action plays an instrument (charge) animation. */
    u16 unkB : 5;
} FieldActionFlags;

/** @brief One eight-byte action slot of a resource's action row. */
typedef struct
{
    u16 command;            /**< Action command; bit 15 marks a technique. */
    FieldActionFlags flags;
    u16 animation;          /**< Animation started by the action, 0 for none. */
    u16 parameter;          /**< Sequence id or effect flags, depending on the command. */
} FieldActionSlot;

/** @brief Action row of one resource (g_field_resource_actions). */
typedef struct
{
    FieldActionSlot slots[FIELD_RESOURCE_ACTION_COUNT];
} FieldActionRow;

/** @brief Vibration tracks of an animation: 0 drives the small motor, 1 the large motor. */
#define FIELD_VIBRATION_TRACK_COUNT 2

/** @brief Animation definition referenced by an animation actor slot. */
typedef struct
{
    /** @brief Parameter curves driving the small and large vibration motors, FIELD_CURVE_NONE for none. */
    u8 vibration_curves[FIELD_VIBRATION_TRACK_COUNT];
    /** @brief Two sound events: value compared by non-start events, event type, sound command. */
    u8 sound_subtypes[2];
    u16 sound_events[2];
    u16 sound_commands[2];
    u16 flags;
    /** @brief Bit 15 plays sound bits 8-14 when track 0 reaches the frame in the low byte. */
    union
    {
        u16 word;
        u8 bytes[2];
    } frame_sound;
    u16 unk10;
    /** @brief Length in frames when FIELD_ANIM_OWN_DURATION is set. */
    u16 duration;
    u8 unk14;
    u8 unk15;
    u8 unk16;
    u8 unk17;
    u16 unk18;
    u8 unk1A[2];
} FieldAnimationDef;

/** @brief Parameter curve count of an animation (curve selectors are four bits). */
#define FIELD_CURVE_COUNT 16
/** @brief Curve selector of an unused render-state track. */
#define FIELD_CURVE_NONE 0xFF
/** @brief Neutral global colour scale. */
#define FIELD_COLOR_SCALE_NEUTRAL 0x100

/** @brief FieldAnimationDef::flags bits. */
#define FIELD_ANIM_COLOR_CURVE_MASK 0xFF
#define FIELD_ANIM_COLOR_RGB 0x100
#define FIELD_ANIM_BLEND_SHIFT 9
#define FIELD_ANIM_GLOBAL_COLOR 0x400
#define FIELD_ANIM_KEEP_ALIVE 0x800
#define FIELD_ANIM_CAMERA_OFFSET 0x1000
#define FIELD_ANIM_CAMERA_MODE_SHIFT 13
/** @brief FieldAnimationDef::flags bit 15: the definition carries its own duration. */
#define FIELD_ANIM_OWN_DURATION 0x8000
/** @brief FieldAnimationDef::unkE: bit 15 fires a sound at a frame; bits 8-14 are the sound, the low byte the frame. */
#define FIELD_ANIM_FRAME_SOUND 0x8000
/** @brief FieldAnimationDef::unk10 bits 12-15: curve of the camera offset track. */
#define FIELD_ANIM_CAMERA_CURVE_SHIFT 12
/** @brief FieldAnimationDef::unk14 values and bits. */
#define FIELD_ANIM_ATTACK 2
#define FIELD_ANIM_STARTED 0x80
/** @brief FieldAnimationDef::unk18 bits. */
#define FIELD_ANIM_OWNER_VISIBILITY 0x2
#define FIELD_ANIM_TARGET_VISIBILITY 0x4
#define FIELD_ANIM_OWNER_TRACK_OFF 0x8
#define FIELD_ANIM_TARGET_TRACK_OFF 0x10
#define FIELD_ANIM_RESTART_AT_END 0x20
#define FIELD_ANIM_OWNER_CURVE_SHIFT 8
#define FIELD_ANIM_TARGET_CURVE_SHIFT 12

/** @brief Parameter curve of an animation: segment count, random flag and first segment, then the value range. */
typedef struct
{
    union
    {
        u16 word;
        struct
        {
            u8 segment_count;
            u8 segment_offset;
        } bytes;
    } head;
    s16 end_value;
    s16 start_value;
} FieldParameterCurve;

/** @brief Status word of an animation actor slot. */
typedef union
{
    u32 word;
    u16 half[2];
    u8 bytes[4];
    struct
    {
        /** @brief Bit 0: FIELD_SLOT_OWNER_LINKED; bits 1-4: the action element (FIELD_SLOT_ELEMENT_BITS). */
        u8 flags;
        /** @brief Non-zero while the animation hides its owner or targets. */
        u8 hiding_objects;
        /** @brief Animation resource the slot plays. */
        u16 animation_id;
    } parts;
} FieldActorSlotStatus;


/** @brief FieldActorSlot::status bit 0: the slot plays a streamed animation bound to its owner. */
#define FIELD_SLOT_OWNER_LINKED 0x1
/** @brief FieldActorSlot::status bits holding the action element (stored shifted by one); all set means none. */
#define FIELD_SLOT_ELEMENT_BITS 0x1E

/** @brief Animation actor bound to a party object (0x1C bytes). */
typedef struct
{
    /** @brief FIELD_BINDING_IDLE, FIELD_BINDING_LOADING or FIELD_BINDING_READY. */
    s32 state;
    /** @brief Resource read queued for the animation, 0 once it has been unpacked. */
    s32 load_id;
    /** @brief Animation resource number (load_id - FIELD_ANIMATION_RESOURCE_BASE). */
    s32 resource_id;
    s32 owner;
    s32 unk10;
    s32 unk14;
    s32 slot;
} FieldActorBinding;

/** @brief Animation actor slot (0x244 bytes). */
typedef struct FieldActorSlot
{
    FieldObjectPart* parts;
    FieldParameterCurve* curves;
    /** @brief Curve segments: ten-bit length in frames, six-bit value. */
    u16* curve_segments;
    FieldAnimationDef* animation;
    FieldAnimationDef* default_animation;
    u8 unk14[0x1C - 0x14];
    /** @brief Values played by kind 2 sound commands. */
    s32 sound_params[2];
    u8 active;
    /** @brief Number of entries in parts[]. */
    u8 part_count;
    u8 actor_type;
    /** @brief Per sound event: bit 0 once it has played, bit 7 for a type 5 event. */
    u8 sound_flags[2];
    /** @brief Entry of default_animation[] that is playing. */
    u8 animation_index;
    u8 unk2A;
    u8 unk2B[FIELD_ACTOR_PART_COUNT];
    /** @brief Effects spawned per track and part (the spawner increments it). */
    u8 effect_counts[FIELD_ACTOR_TRACK_COUNT][FIELD_ACTOR_PART_COUNT];
    u8 unkCB;
    u16 effect_totals[FIELD_ACTOR_TRACK_COUNT][FIELD_ACTOR_PART_COUNT];
    /** @brief Frame counter of each track. */
    u16 track_frames[FIELD_ACTOR_TRACK_COUNT];
    u8 unk1FE[0x222 - 0x1FE];
    /** @brief Length of the animation in frames. */
    u16 duration;
    FieldActorSlotStatus status;
    u8 owner_object_index;
    u8 targets[FIELD_ACTOR_TRACK_COUNT];
    u8 target_count;
    u8 slot_index;
    u16 unk234;
    /** @brief Frames since the animation started. */
    u16 frame_counter;
    /** @brief Frames between the starts of consecutive tracks, 0 to start them all at once. */
    u16 track_interval;
    u8 track_mask;
    u8 pending_track_mask;
    u8 unk23C[4];
    /** @brief Per animation, the mask of parts it drives. */
    u16* part_masks;
} FieldActorSlot;

/** @brief Resource table entry selected by an actor's resource index (0x14 bytes). */
typedef struct
{
    u8* start;
    u8* end;
    u8 unk8;
    u8 slot_index;
    u8 unkA[0xE - 0xA];
    u16 unkE;
    u32 flags;
} FieldResourceEntry;

/** @brief FieldResourceEntry::flags bit: the resource has an action table and eight-direction animations. */
#define FIELD_RESOURCE_HAS_ACTIONS 1

extern FieldObjectState g_field_object_states[];
extern FieldActorBinding g_field_actor_bindings[];
extern FieldActorSlot g_field_actor_slots[];
extern FieldResourceEntry g_field_resource_entries[];
extern FieldObjectPart g_field_object_parts[];
extern FieldPlayerRecord g_field_player_records[];

/**
 * @brief Find the field actor whose object state carries @p key.
 * @param key Object key compared against each object state's key word.
 * @return The matching actor record, or FIELD_ACTOR_NONE.
 */
static inline FieldActor* field_find_actor(s32 key)
{
    FieldObjectState* state;
    FieldActor* actor;
    s32 i;

    actor = g_field_actors;
    state = g_field_object_states;
    for (i = 0; i < FIELD_ACTOR_COUNT; i++, state++, actor++)
    {
        if (state->key == key)
        {
            return actor;
        }
    }
    return FIELD_ACTOR_NONE;
}

/**
 * @brief Binding used by @p actor (objects 2 and up share the last binding).
 * @param actor Actor whose object index selects the binding.
 * @return The object's animation actor binding.
 */
static inline FieldActorBinding* field_actor_binding(FieldActor* actor)
{
    FieldActorBinding* bindings;
    s32 offset;

    bindings = g_field_actor_bindings;
    if (actor->object_index < 2)
    {
        offset = actor->object_index * sizeof(FieldActorBinding);
    }
    else
    {
        offset = 2 * sizeof(FieldActorBinding);
    }
    return (FieldActorBinding*)((u8*)bindings + offset);
}

/**
 * @brief Binding used by object @p object_index (objects 2 and up share the last binding).
 * @param object_index Object index; only its low byte selects the binding.
 * @return The object's animation actor binding.
 */
static inline FieldActorBinding* field_object_binding(s32 object_index)
{
    FieldActorBinding* bindings;
    s32 offset;

    bindings = g_field_actor_bindings;
    if ((u8)object_index < 2)
    {
        offset = object_index * sizeof(FieldActorBinding);
    }
    else
    {
        offset = 2 * sizeof(FieldActorBinding);
    }
    return (FieldActorBinding*)((u8*)bindings + offset);
}

#endif
