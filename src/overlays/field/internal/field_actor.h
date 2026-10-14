#ifndef FIELD_ACTOR_H
#define FIELD_ACTOR_H

#include "common.h"
#include "overlays/field/field_actor_records.h"
#include "field_actor_binding.h"

/**
 * @file field_actor.h
 * @brief The field actor record (g_field_actors), usable without the rest of
 *        field_actor_tables.h.
 */

/** @brief Number of objects in the parallel field object tables. */
#define FIELD_ACTOR_COUNT 13
/** @brief Animation binding of an object: party members own bindings 0-2, all others share the last. */
#define FIELD_BINDING_INDEX(object_index) ((object_index) < FIELD_PARTY_COUNT ? (object_index) : FIELD_PARTY_COUNT - 1)

/** @brief Low nine bits of FieldActor::control holding the control mode. */
#define FIELD_CONTROL_MODE_MASK 0x1FF
/** @brief Control modes (FieldActor::control & FIELD_CONTROL_MODE_MASK): pad input, following the leader, or the actor's script. */
#define FIELD_CONTROL_PLAYER 0
#define FIELD_CONTROL_FOLLOWER 1
#define FIELD_CONTROL_SCRIPTED 2
/** @brief FieldActor::control bit set while the actor moves. */
#define FIELD_CONTROL_MOVING 0x200
/** @brief FieldActor::control second movement bit (placeholder name: FIELD code only clears and tests it; nothing sets it). */
#define FIELD_CONTROL_MOVEMENT_UNK400 0x400
/** @brief FieldActor::control movement bits. */
#define FIELD_CONTROL_MOVEMENT_MASK (FIELD_CONTROL_MOVING | FIELD_CONTROL_MOVEMENT_UNK400)
/** @brief Shift of the two FieldActor::control movement bits. */
#define FIELD_CONTROL_MOVEMENT_SHIFT 9
/** @brief FieldActor::control bit: the animation stops on its last frame instead of looping. */
#define FIELD_CONTROL_PLAY_ONCE 0x800
/** @brief FieldActor::control bits 15 and 28, set together on an effect that takes its owner's part colour. */
#define FIELD_CONTROL_UNK8000 0x8000
#define FIELD_CONTROL_UNK10000000 0x10000000
#define FIELD_CONTROL_OWNER_TINT (FIELD_CONTROL_UNK10000000 | FIELD_CONTROL_UNK8000)
/** @brief FieldActor::control bit 18, cleared when an actor is initialized. */
#define FIELD_CONTROL_UNK40000 0x40000
/** @brief FieldActor::control bit 23: the actor uses its own tint. */
#define FIELD_CONTROL_TINTED_SHIFT 23
#define FIELD_CONTROL_TINTED 0x800000
/* FieldActor::direction headings. */
/** @brief FieldActor::direction headings are 256 steps per turn (ratan2/rcos use 4096). */
#define FIELD_HEADING_STEPS 256
/** @brief Shift from a 4096-step angle to a heading. */
#define FIELD_HEADING_ANGLE_SHIFT 4
/** @brief Half of one eighth of a turn, added before a heading is split into eight sectors. */
#define FIELD_HEADING_HALF_SECTOR 16
/** @brief Shift from a heading to one of eight sectors. */
#define FIELD_HEADING_SECTOR_SHIFT 5
/** @brief Heading bits of a sector (the heading snapped to an eighth of a turn). */
#define FIELD_HEADING_SECTOR_MASK 0xE0
/** @brief Directional animations come in groups of five (stand, then walk, then run). */
#define FIELD_ANIMATION_DIRECTIONS 5
/** @brief First walking animation. */
#define FIELD_ANIMATION_WALK 5
/** @brief First running animation. */
#define FIELD_ANIMATION_RUN 10
/** @brief Distance a raised actor (negative FieldActor::y) drops each frame on its way back to the ground. */
#define FIELD_ACTOR_FALL_STEP 0x800
/** @brief World coordinates are 24.8 fixed point. */
#define FIELD_POSITION_SHIFT 8
/** @brief Scratchpad vector that receives an actor displacement. */
#define FIELD_SCRATCH_DISPLACEMENT ((Vec3i*)SCRATCHPAD_ADDRESS)

/* Shared actor animations (FieldActor::animation without the facing bit). */
/** @brief Defend pose played when a guarded hit lands. */
#define FIELD_ANIMATION_DEFEND_HIT 11
/** @brief Pose held while an action chain winds down, and by the party after a battle reset (placeholder name). */
#define FIELD_ANIMATION_13 0x13
/** @brief First combo attack animation; later combo steps follow it. */
#define FIELD_ANIMATION_COMBO 0x1F
/** @brief First of the two hit animations; one is picked at random. */
#define FIELD_ANIMATION_HIT 20
/** @brief Knocked-down pose (also the leader's pose when a duel is lost). */
#define FIELD_ANIMATION_KNOCKED_DOWN 29

/** @brief Object-state movement bit: the charge animation was started. */
#define FIELD_MOVEMENT_CHARGED 0x400
/** @brief Object-state movement bit: the object overlaps another one. */
#define FIELD_MOVEMENT_OVERLAPPING_BIT 14
#define FIELD_MOVEMENT_OVERLAPPING (1 << FIELD_MOVEMENT_OVERLAPPING_BIT)
/**
 * @brief Object-state movement bit: the object can be neither hit nor targeted.
 * @note Set when a technique sequence starts and during the battle-entry tint flash;
 *       the tint update clears it when the flash ends or the object's bound animation stops.
 */
#define FIELD_MOVEMENT_INVULNERABLE 0x8000
/** @brief Object-state technique_gauge value of a full gauge. */
#define FIELD_TECHNIQUE_GAUGE_FULL 0xFF
/** @brief Companion character ids from this value on are drawn in a selectable colour and get the companion HUD panel. */
#define FIELD_COMPANION_COLOR_ID_MIN 0x41
/** @brief FieldActor::control bits 19-22 (FieldActorControl::bits.palette) read as a word, and the CLUT column the partner gets when the partner is the hero. */
#define FIELD_CONTROL_PALETTE_SHIFT 19
#define FIELD_CONTROL_PALETTE_PARTNER 10
#define FIELD_CONTROL_PALETTE(word) (((word) >> FIELD_CONTROL_PALETTE_SHIFT) & 0xF)
/** @brief FieldActor::control bits 16-17: colour variant of the companion (the same bits hold a layout actor's group). */
#define FIELD_CONTROL_VARIANT_SHIFT 16
#define FIELD_CONTROL_VARIANT_MASK 0x30000
/** @brief Object-state group_flags bits holding the object's group. */
#define FIELD_OBJECT_GROUP_MASK 0x0F
/**
 * @brief Object-state flag bits that hold an object in place.
 * @note Such an object takes no knockback, is skipped by overlap tests, stops its looping
 *       effects, and is left out of the camera's party average.
 */
#define FIELD_OBJECT_IMMOBILE_FLAGS                                                                                                   \
    (FIELD_OBJECT_FLAG_PARALYSIS | FIELD_OBJECT_FLAG_HYPNOSIS | FIELD_OBJECT_FLAG_SLEEP | FIELD_OBJECT_FLAG_FROZEN | FIELD_OBJECT_UNTARGETABLE_FLAGS)

/** @brief Word, halfword and bit views of an actor's control-mode word. */
typedef union
{
    u32 word;
    u16 half[2];
    struct
    {
        u32 unk0 : 16;
        /** @brief Actor group of a layout actor (FieldLayoutRecord group bits); on the companion the colour variant (FIELD_CONTROL_VARIANT_MASK). */
        u32 group : 2;
        u32 unk18 : 1;
        /** @brief CLUT column of the actor's sprite palette. */
        u32 palette : 4;
        u32 unk23 : 9;
    } bits;
} FieldActorControl;

/** @brief Script, motion and animation state of one field object (0x54 bytes). */
typedef struct FieldActor
{
    s32 x;
    s32 y;
    s32 z;
    u32 unkC;
    /** @brief Non-zero while an actor script blocks scene scripts that wait for this actor (scene opcode 0x16). */
    s16 script_busy;
    s16 unk12;
    s16 unk14;
    /** @brief Ticks left on the current animation frame; speed_accumulator is spread over them. */
    s16 frame_timer;
    /** @brief Actor tint, refreshed from the object part every frame. */
    u8 tint_red;
    u8 tint_green;
    u8 tint_blue;
    /** @brief Facing angle in 256 steps. */
    u8 direction;
    /** @brief Low nine bits hold the control mode; higher bits are flags. */
    FieldActorControl control;
    /** @brief Argument of the running command (a timer, step or actor index). */
    u8 command_param;
    /** @brief Low seven bits select the animation; bit 7 mirrors it. */
    u8 animation;
    /** @brief Animation actor slot and part that own the record (effect records point at their emitter). */
    u8 owner_slot;
    u8 owner_part;
    u8 animation_active;
    u8 presence;
    u8 unk26;
    u8 animation_frame;
    u8 script_index;
    u8 unk29;
    s16 command;
    u16 script_offset;
    /** @brief Non-zero while the current animation is still playing. */
    u16 animation_state;
    u16 variant;
    u8 unk32;
    /** @brief Bit 0 selects the running animations. */
    u8 running;
    /** @brief Ticks spent on the current animation frame and the frame's length in ticks. */
    u8 frame_ticks;
    u8 frame_length;
    s8 speed_accumulator;
    s8 height;
    /** @brief Height of the next animation frame. */
    u8 next_height;
    /** @brief Frames left before a stopped actor settles into its idle animation. */
    u8 stop_delay;
    u8 object_index;
    u8 resource_index;
    u8 unk3C;
    /** @brief Frames left before a retiring actor is removed. */
    u8 removal_delay;
    u8 unk3E;
    u8 unk3F;
    /** @brief Address of the displayed frame's data; bit 31 set selects the alternate (8-bit) renderer. */
    u32 frame_data;
    u32 unk44;
    u32 unk48;
    u32 unk4C;
    u8 unk50[4];
} FieldActor;

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
    if (actor->object_index < FIELD_SHARED_BINDING)
    {
        offset = actor->object_index * sizeof(FieldActorBinding);
    }
    else
    {
        offset = FIELD_SHARED_BINDING * sizeof(FieldActorBinding);
    }
    return (FieldActorBinding*)((u8*)bindings + offset);
}

/** @brief Actor command word that performs action slot @p action. */
#define FIELD_ACTION_COMMAND(action) (((action) << 8) | FIELD_ACTOR_COMMAND_ACTION)

/** @brief Number of actions with an entry in an action command map. */
#define FIELD_ACTION_MAP_ENTRY_COUNT 11

/** @brief Action command map of one player: command words and disable flags. */
typedef struct
{
    u16 commands[FIELD_ACTION_MAP_ENTRY_COUNT];
    u8 disabled[FIELD_ACTION_MAP_ENTRY_COUNT];
    u8 pad;
} FieldActionCommandMap;

/** @brief Action command maps, one per player. */
extern FieldActionCommandMap g_field_action_command_maps[FIELD_PLAYER_COUNT];

extern FieldActor g_field_actors[];
extern FieldActor g_field_scene_actors[];

/** @brief Group (1-based) of the area the party is in, 0 for none; non-party objects of other groups are not hit or targeted. */
extern s32 g_field_active_group;
/** @brief Non-zero while actions from 4 up and techniques are blocked. */
extern s32 g_field_actions_limited;
/** @brief Battle entry sequence state (FIELD_BATTLE_ENTRY_*); nonzero locks player input and enemy HUD while a battle starts. */
extern s32 g_field_battle_entry_state;
/** @brief Frames left while the camera scroll limits ease towards their targets. */
extern s32 g_field_scroll_ease_timer;

#endif
