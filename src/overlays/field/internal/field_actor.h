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
/** @brief Low seven bits of FieldActor::animation select the animation. */
#define FIELD_ANIMATION_INDEX_MASK 0x7F
/** @brief FieldActor::animation bit that mirrors the animation. */
#define FIELD_ANIMATION_FACING 0x80
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

/**
 * @brief Abilities a player assigns to the action buttons (FieldActionSlot::command).
 * @note An ability's code is also the animation the actor plays while using it
 *       (FieldActor::animation without the facing bit). The names follow the
 *       game's command name table (g_field_command_names), which gives each
 *       unused code below an ability the same name as that ability.
 */
typedef enum FieldAbility
{
    FIELD_ABILITY_JUMP = 0x08,
    /** Guards; held with the button. */
    FIELD_ABILITY_DEFEND = 0x0A,
    FIELD_ABILITY_LUNGE = 0x2F,
    FIELD_ABILITY_RETREAT = 0x30,
    /** Held with the button. Defense counts as 0 while it plays; also the pose after an instrument or a duel knock-down. */
    FIELD_ABILITY_CROUCH = 0x31,
    FIELD_ABILITY_PUSH = 0x32,
    FIELD_ABILITY_CHEER = 0x33,
    FIELD_ABILITY_SPIN = 0x34,
    FIELD_ABILITY_GRAPPLE = 0x35,
    FIELD_ABILITY_WHIRL = 0x36,
    FIELD_ABILITY_BASH = 0x37,
    /** Never touched by effect quads. */
    FIELD_ABILITY_SOMERSAULT = 0x38,
    /** Never touched by effect quads. */
    FIELD_ABILITY_BACK_ROLL = 0x39,
    FIELD_ABILITY_BACK_FLIP = 0x3A,
    FIELD_ABILITY_MOONSAULT = 0x3B,
    /** A player using it is not hit by attack spheres. */
    FIELD_ABILITY_HIGH_JUMP = 0x3C,
    /** A player using it keeps its height while moving. */
    FIELD_ABILITY_DOUBLE_JUMP = 0x3D,
    FIELD_ABILITY_TACKLE = 0x3E,
    /** Answers an effect quad with action 2. */
    FIELD_ABILITY_COUNTERATTACK = 0x3F,
    /** Answers an effect quad with action 3. */
    FIELD_ABILITY_COUNTERSTRIKE = 0x40,
    FIELD_ABILITY_TAUNT = 0x41,
    FIELD_ABILITY_EVADE = 0x42,
    FIELD_ABILITY_TOSS = 0x43,
    /** Ignores hits. */
    FIELD_ABILITY_DEFENSIVE_LUNGE = 0x44,
    FIELD_ABILITY_SLIDE = 0x45,
    /** Follow-up attack with no name of its own; nothing in the field code starts it. */
    FIELD_ABILITY_FOLLOW_UP_4E = 0x4E,
    FIELD_ABILITY_FLIP_KICK = 0x4F,
    /** Follow-up attack Spin chains into while button 2 is held. */
    FIELD_ABILITY_FOLLOW_UP_50 = 0x50,
    /** Follow-up attack Spin chains into while button 3 is held. */
    FIELD_ABILITY_FOLLOW_UP_51 = 0x51
} FieldAbility;

/* Shared actor animations (FieldActor::animation without the facing bit). */
/** @brief Defend pose played when a guarded hit lands. */
#define FIELD_ANIMATION_DEFEND_HIT 11
#define FIELD_ANIMATION_13 0x13
/** @brief First combo attack animation; later combo steps follow it. */
#define FIELD_ANIMATION_COMBO 0x1F
/** @brief First of the two hit animations; one is picked at random. */
#define FIELD_ANIMATION_HIT 20

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
 * @brief Object-state flags bits 0-8: status ailments, in the order of the status text and immunity tables.
 * @note Bit n is status effect n (field_apply_status_effect); g_field_object_flag_handlers starts each one's looping animation.
 */
#define FIELD_OBJECT_FLAG_POISON 0x0001
#define FIELD_OBJECT_FLAG_DARKNESS 0x0002
#define FIELD_OBJECT_FLAG_PARALYSIS 0x0004
#define FIELD_OBJECT_FLAG_CONFUSION 0x0008
#define FIELD_OBJECT_FLAG_FLAME_BURST 0x0010
#define FIELD_OBJECT_FLAG_HYPNOSIS 0x0020
#define FIELD_OBJECT_FLAG_SLEEP 0x0040
#define FIELD_OBJECT_FLAG_PETRIFIED 0x0080
#define FIELD_OBJECT_FLAG_FROZEN 0x0100
/** @brief Object-state flags bit set while the object is knocked out. */
#define FIELD_OBJECT_FLAG_KNOCKED_OUT 0x0200
/** @brief Object-state flags bit set while another object is linked to this one. */
#define FIELD_OBJECT_FLAG_LINK_TARGET 0x2000
/**
 * @brief Object-state flags bit: the Mini status (effect 10) - the object is shrunk to half size and its actions are restricted.
 * @note Pad input may then only start actions 2 and 3; scripted actions and techniques
 *       do not start at all. The bit is cleared when the object is hit.
 */
#define FIELD_OBJECT_FLAG_MINI 0x0400
/** @brief Object-state flags bit set while a dash ability (Lunge, Retreat, Defensive Lunge, Slide) plays. */
#define FIELD_OBJECT_FLAG_DASH 0x4000
/** @brief Object-state flags bit: an action chain is running (cleared with the retry count). */
#define FIELD_OBJECT_CHAINING 0x8000
/** @brief Object-state flags bit set for one frame when the object is hit; tints the actor red. */
#define FIELD_OBJECT_FLAG_HIT_FLASH 0x10000000

/** @brief Object-state contact bits (the contact word of g_field_object_states). */
#define FIELD_CONTACT_ANIMATION_HIDDEN 0x01 /**< An animation actor hides the object; it cannot be targeted. */
#define FIELD_CONTACT_LINKED 0x02           /**< The object is linked to another object. */
#define FIELD_CONTACT_ACTION_BITS 0x1C      /**< Cleared before an action is prepared. */
#define FIELD_CONTACT_NO_HIT_TEST 0x20      /**< The object takes no hits and is never an action target. */
#define FIELD_CONTACT_ACTION_PENDING 0x40   /**< The object charges or waits for an action; hits land even while its own bound animation runs. */
#define FIELD_CONTACT_TARGETED 0x80         /**< The object is being targeted. */
/** @brief Object-state flag bits that make an object an invalid target. */
#define FIELD_OBJECT_UNTARGETABLE_FLAGS (FIELD_OBJECT_FLAG_PETRIFIED | FIELD_OBJECT_FLAG_KNOCKED_OUT | FIELD_OBJECT_FLAG_LINK_TARGET)
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

/** @brief Presence value of an unused actor record. */
#define FIELD_ACTOR_UNUSED 0xFF

/** @brief Presence value of a hidden actor record. */
#define FIELD_ACTOR_HIDDEN 0xFE

/** @brief Actor commands (FieldActor::command). */
enum
{
    FIELD_ACTOR_COMMAND_NONE = 0,
    FIELD_ACTOR_COMMAND_BASE = 0x81,
    FIELD_ACTOR_COMMAND_STEP = 0x81,
    FIELD_ACTOR_COMMAND_HIT = 0x82,
    FIELD_ACTOR_COMMAND_ACTION = 0x85,
    FIELD_ACTOR_COMMAND_RECOVER = 0x86,
    FIELD_ACTOR_COMMAND_INSTRUMENT = 0x87,
    FIELD_ACTOR_COMMAND_WALK = 0x88,
    FIELD_ACTOR_COMMAND_RISE = 0x89,
    FIELD_ACTOR_COMMAND_SINK = 0x8A,
    FIELD_ACTOR_COMMAND_WALK_TO_TARGET = 0x8B,
    FIELD_ACTOR_COMMAND_WALK_FROM_TARGET = 0x8C,
    FIELD_ACTOR_COMMAND_SEQUENCE_STEP = 0x8D,
    FIELD_ACTOR_COMMAND_KNOCKED_DOWN = 0x8E,
    FIELD_ACTOR_COMMAND_JUMP = 0x8F,
    FIELD_ACTOR_COMMAND_DEFEATED = 0x90,
    FIELD_ACTOR_COMMAND_TECHNIQUE = 0x91,
    FIELD_ACTOR_COMMAND_DEFEAT_BOUND = 0x92,
    FIELD_ACTOR_COMMAND_DEFEAT_WAIT = 0x93,
    FIELD_ACTOR_COMMAND_DEFEAT_END = 0x94,
    FIELD_ACTOR_COMMAND_SEQUENCE_WAIT = 0x95,
    /** Follow-up attack started from a jump ability while action button 3 is held; no further chaining. */
    FIELD_ACTOR_COMMAND_AIR_FOLLOW_UP = 0x96,
    /** Runs like FIELD_ACTOR_COMMAND_SECOND_JUMP and counts as an action for hits; nothing in the C code sets it. */
    FIELD_ACTOR_COMMAND_98 = 0x98,
    FIELD_ACTOR_COMMAND_IDLE_AFTER_ANIMATION = 0x99,
    FIELD_ACTOR_COMMAND_IDLE_AFTER_RELOAD = 0x9A,
    /** Second jump of a Double Jump, started while recovering from the first. */
    FIELD_ACTOR_COMMAND_SECOND_JUMP = 0x9B,
    /** Raise the actor by command_param each frame for unk26 frames. */
    FIELD_ACTOR_COMMAND_TIMED_LIFT = 0x9C,
    /** Lower the actor by command_param each frame until it reaches the ground (or unk26 runs out). */
    FIELD_ACTOR_COMMAND_DESCEND_TO_GROUND = 0x9D,
    FIELD_ACTOR_COMMAND_APPROACH_TARGET = 0xA0,
    FIELD_ACTOR_COMMAND_ACTION_END = 0xA4,
    FIELD_ACTOR_COMMAND_TIMED_WALK = 0xA7,
    FIELD_ACTOR_COMMAND_WAIT_ANIMATION = 0xA8,
    FIELD_ACTOR_COMMAND_RUN_TO_TARGET = 0xAC,
    FIELD_ACTOR_COMMAND_RUN = 0xAD,
    FIELD_ACTOR_COMMAND_DEFEAT_DELAY = 0xAE,
    FIELD_ACTOR_COMMAND_FOLLOW_LEADER = 0xAF,
    FIELD_ACTOR_COMMAND_WALK_PATH = 0xB0,
    FIELD_ACTOR_COMMAND_RUN_PATH = 0xB1,
    FIELD_ACTOR_COMMAND_TURN = 0xB2,
    FIELD_ACTOR_COMMAND_STOP = 0xB3,
    FIELD_ACTOR_COMMAND_LEAVE_PATH = 0xB5,
    FIELD_ACTOR_COMMAND_TIMED_SLIDE = 0xB6,
    FIELD_ACTOR_COMMAND_WAIT = 0xB7,
    FIELD_ACTOR_COMMAND_ATTACHED = 0xB8,
    /** Leave the actor and its command untouched (dropped items wait in it until removed). */
    FIELD_ACTOR_COMMAND_HOLD = 0xBA,
    FIELD_ACTOR_COMMAND_RETIRE = 0xBB,
    FIELD_ACTOR_COMMAND_WAIT_BOUND_ACTOR = 0xBC,
    FIELD_ACTOR_COMMAND_INSTRUMENT_END = 0xBD
};

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
