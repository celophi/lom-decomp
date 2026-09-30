#ifndef FIELD_ACTOR_TABLES_H
#define FIELD_ACTOR_TABLES_H

#include "common.h"
#include "field_actor.h"
#include "field_actor_records.h"

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

/** @brief Target entry of an animation track without a target object. */
#define FIELD_TARGET_NONE 0xFF

/** @brief Number of animation actor bindings (party members 0, 1 and everyone else). */
#define FIELD_ACTOR_BINDING_COUNT 3

/** @brief FieldObjectState::interaction_kind of a dropped item the player can pick up. */
#define FIELD_INTERACTION_ITEM 1

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
    /** Bit 1 of the selected object's background flags (field_select_object). */
    u8 unk4;
} FieldMapBounds;

/** @brief Fixed address of the field map header block. */
#define FIELD_MAP_BOUNDS ((FieldMapBounds*)MAP_BOUNDS_ADDRESS)



/** @brief FieldPlayerRecord::flags bit: the party member is present. */
#define FIELD_PLAYER_ACTIVE 0x1

/** @brief FieldPlayerRecord::character_kind values. */
#define FIELD_PLAYER_KIND_HERO 0
#define FIELD_PLAYER_KIND_PARTNER 1
#define FIELD_PLAYER_KIND_COMPANION 2

/** @brief FieldObjectState::hud flag (FieldStatusState::level byte) showing the technique gauge. */
#define FIELD_HUD_SHOW_TECHNIQUE_GAUGE 0x01

/** @brief Sentinel returned by field_find_object_state when no object matches. */
#define FIELD_OBJECT_STATE_NONE ((FieldObjectState*)-1)

/** @brief FieldObjectPart::spawn_flags bit 23: the object ignores map collision. */
#define FIELD_PART_IGNORE_MAP_COLLISION 0x800000

/** @brief Flag byte and weapon type of a party member, also updated as one halfword. */
typedef union FieldPlayerHead
{
    u16 word;
    struct
    {
        /** @brief FIELD_PLAYER_ACTIVE and the other bits of @c bits. */
        u8 flags;
        /** @brief Weapon type (item type) of the equipped weapon, 0xFF before the first party update. */
        u8 weapon_type;
    } bytes;
    struct
    {
        /** @brief Same bit as FIELD_PLAYER_ACTIVE. */
        u16 active : 1;
        /** @brief Selects the hero's alternate sprite and portrait set. */
        u16 alt_appearance : 1;
        u16 unk2 : 14;
    } bits;
} FieldPlayerHead;

/** @brief Per-party-member record (0x268 bytes). */
typedef struct FieldPlayerRecord
{
    FieldPlayerHead head;
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
/** @brief FieldAnimationDef::frame_sound: bit 15 fires a sound at a frame; bits 8-14 are the sound, the low byte the frame. */
#define FIELD_ANIM_FRAME_SOUND 0x8000
/** @brief FieldAnimationDef::palette_animation bits 12-15: curve of the camera offset track. */
#define FIELD_ANIM_CAMERA_CURVE_SHIFT 12
/** @brief FieldAnimationDef::hit_test_mode values and bits. */
#define FIELD_ANIM_ATTACK 2
#define FIELD_ANIM_STARTED 0x80
/** @brief FieldAnimationDef::sync_flags bits. */
#define FIELD_ANIM_OWNER_VISIBILITY 0x2
#define FIELD_ANIM_TARGET_VISIBILITY 0x4
#define FIELD_ANIM_OWNER_TRACK_OFF 0x8
#define FIELD_ANIM_TARGET_TRACK_OFF 0x10
#define FIELD_ANIM_RESTART_AT_END 0x20
#define FIELD_ANIM_OWNER_CURVE_SHIFT 8
#define FIELD_ANIM_TARGET_CURVE_SHIFT 12


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

/** @brief FieldResourceEntry::flags bit: the resource has an action table and eight-direction animations. */
#define FIELD_RESOURCE_HAS_ACTIONS 1
/** @brief FieldResourceEntry::flags bit: the entry holds loaded data. */
#define FIELD_RESOURCE_LOADED 2

extern FieldObjectState g_field_scene_object_states[];
extern FieldActorBinding g_field_actor_bindings[];
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
