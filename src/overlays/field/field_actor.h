#ifndef FIELD_ACTOR_H
#define FIELD_ACTOR_H

#include "common.h"

/**
 * @file field_actor.h
 * @brief The field actor record (g_field_actors), usable without the rest of
 *        field_actor_tables.h.
 */

/** @brief Number of objects in the parallel field object tables. */
#define FIELD_ACTOR_COUNT 13
/** @brief Objects 0 and 1 are the two player-controlled party members; each has its own binding. */
#define FIELD_PLAYER_COUNT 2
/** @brief Objects 0 to 2 are the party: the two players and the companion. */
#define FIELD_PARTY_COUNT 3
/** @brief Animation binding of an object: party members own bindings 0-2, all others share the last. */
#define FIELD_BINDING_INDEX(object_index) ((object_index) < FIELD_PARTY_COUNT ? (object_index) : FIELD_PARTY_COUNT - 1)

/** @brief Low nine bits of FieldActor::control holding the control mode. */
#define FIELD_CONTROL_MODE_MASK 0x1FF
/** @brief FieldActor::control bit: the animation stops on its last frame instead of looping. */
#define FIELD_CONTROL_PLAY_ONCE 0x800
/** @brief Low seven bits of FieldActor::animation select the animation. */
#define FIELD_ANIMATION_INDEX_MASK 0x7F
/** @brief FieldActor::animation bit that mirrors the animation. */
#define FIELD_ANIMATION_FACING 0x80
/** @brief Directional animations come in groups of five (stand, then walk, then run). */
#define FIELD_ANIMATION_DIRECTIONS 5

/** @brief Shared actor animations (FieldActor::animation without the facing bit). */
#define FIELD_ANIMATION_GUARD 10
#define FIELD_ANIMATION_GUARD_ALT 11
#define FIELD_ANIMATION_13 0x13
/** @brief First of the two hit animations; one is picked at random. */
#define FIELD_ANIMATION_HIT 20
/** @brief Exposed pose after an instrument or a knock-down, also held with the action button; the actor takes more damage. */
#define FIELD_ANIMATION_DEFENSELESS 0x31

/** @brief Object-state movement bits holding the running action sequence. */
#define FIELD_MOVEMENT_SEQUENCE_MASK 0x1800
/** @brief Object-state group_flags bits holding the object's group. */
#define FIELD_OBJECT_GROUP_MASK 0x0F
/** @brief Object-state flags bit set while the object is knocked out. */
#define FIELD_OBJECT_FLAG_KNOCKED_OUT 0x0200

/** @brief Object-state contact bits (the contact word of g_field_object_states). */
#define FIELD_CONTACT_ANIMATION_HIDDEN 0x01 /**< An animation actor hides the object; it cannot be targeted. */
#define FIELD_CONTACT_LINKED 0x02           /**< The object is linked to another object. */
#define FIELD_CONTACT_NO_HIT_TEST 0x20      /**< The object takes no hits. */
#define FIELD_CONTACT_IGNORE_BINDING 0x40   /**< Hits land even while the object's own bound animation runs. */
#define FIELD_CONTACT_TARGETED 0x80         /**< The object is being targeted. */
/** @brief Object-state movement bit: the object's tint is flashing. */
#define FIELD_MOVEMENT_TINT_FLASH 0x8000
/** @brief Object-state flag bits that make an object an invalid target. */
#define FIELD_OBJECT_UNTARGETABLE_FLAGS 0x2280

/** @brief Word, halfword and bit views of an actor's control-mode word. */
typedef union
{
    u32 word;
    u16 half[2];
} FieldActorControl;

/** @brief Script, motion and animation state of one field object (0x54 bytes). */
typedef struct FieldActor
{
    s32 x;
    s32 y;
    s32 z;
    u32 unkC;
    s16 unk10;
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
    /** @brief Decoded data of the displayed frame; negative values use the alternate renderer. */
    s32 frame_data;
    u32 unk44;
    u32 unk48;
    u32 unk4C;
    u8 unk50[4];
} FieldActor;

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
    FIELD_ACTOR_COMMAND_96 = 0x96,
    FIELD_ACTOR_COMMAND_98 = 0x98,
    FIELD_ACTOR_COMMAND_IDLE_AFTER_ANIMATION = 0x99,
    FIELD_ACTOR_COMMAND_IDLE_AFTER_RELOAD = 0x9A,
    FIELD_ACTOR_COMMAND_9B = 0x9B,
    FIELD_ACTOR_COMMAND_9C = 0x9C,
    FIELD_ACTOR_COMMAND_9D = 0x9D,
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
    FIELD_ACTOR_COMMAND_RETIRE = 0xBB,
    FIELD_ACTOR_COMMAND_WAIT_BOUND_ACTOR = 0xBC,
    FIELD_ACTOR_COMMAND_INSTRUMENT_END = 0xBD
};

/** @brief Actor command word that performs action slot @p action. */
#define FIELD_ACTION_COMMAND(action) (((action) << 8) | FIELD_ACTOR_COMMAND_ACTION)

extern FieldActor g_field_actors[];

#endif
