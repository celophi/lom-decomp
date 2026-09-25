/**
 * @file field_actor_script_ops.c
 * @brief Field actor script interpreter: script entry, the actor command
 *        opcodes, and the key-addressed action and animation commands scripts
 *        call.
 */

#include "common.h"
#include "field_actor_tables.h"
#include "field_actor_behavior.h"
#include "field_actor_runtime.h"
#include "field_calls.h"
#include "field_records.h"
#include "main.h"
#include "scene_state.h"
#include "sdk/libgpu.h"
#include "sdk/libgte.h"

/** @brief Control mode of an actor driven by its script. */
#define FIELD_CONTROL_SCRIPTED 2

/** @brief Binding shared by every object from FIELD_PLAYER_COUNT up. */
#define FIELD_SHARED_BINDING (FIELD_ACTOR_BINDING_COUNT - 1)

/** @brief Lowest actor record a script may spawn a child actor into. */
#define FIELD_FIRST_SPAWN_ACTOR 3

/** @brief Presence value of a visible actor record. */
#define FIELD_ACTOR_VISIBLE 0

/** @brief Directional animations: 5 facings each for standing, walking and running. */
#define FIELD_ANIMATION_WALK 5
#define FIELD_ANIMATION_RUN 10
#define FIELD_DIRECTIONAL_ANIMATION_COUNT 15

/** @brief Shift from a direction byte to its 45 degree sector. */
#define FIELD_DIRECTION_SECTOR_SHIFT 5

/** @brief Entries of the turn animation table per 45 degree facing sector. */
#define FIELD_TURN_SECTOR_ENTRIES 6

/** @brief Half of a 45 degree facing sector in ratan2 units. */
#define FIELD_FACING_HALF_SECTOR (ONE / 16)

/** @brief Full-size object part scale; smaller objects use the small footprint. */
#define FIELD_PART_FULL_SCALE 0x40

/** @brief Collision footprints of full-size and small objects. */
#define FIELD_FOOTPRINT_LARGE_WIDTH 12
#define FIELD_FOOTPRINT_LARGE_DEPTH 8
#define FIELD_FOOTPRINT_SMALL_WIDTH 9
#define FIELD_FOOTPRINT_SMALL_DEPTH 6
#define FIELD_FOOTPRINT_HEIGHT 16

/** @brief Camera-relative bounds a revived party member must be inside to stay where it is. */
#define FIELD_VIEW_MARGIN_LEFT 0xA00
#define FIELD_VIEW_MARGIN_RIGHT 0x13600
#define FIELD_VIEW_MARGIN_BOTTOM 0x1B600

/** @brief Tint timer started by a revive. */
#define FIELD_REVIVE_TINT_FRAMES 60

/** @brief Pan value of a centred sound effect. */
#define FIELD_SOUND_PAN_CENTRE 0x80

/** @brief Wait value that keeps a command running until it finishes by itself. */
#define FIELD_WAIT_UNTIL_DONE 0xFF

/** @brief FieldObjectState::flags bits. */
#define FIELD_OBJECT_FLAG_KNOCKED_OUT 0x0400
#define FIELD_OBJECT_CHAINING 0x8000
#define FIELD_OBJECT_FLAG_HIT 0x10000000

#define FIELD_CONTACT_ACTION_BITS 0x1C
#define FIELD_CONTACT_ACTION_PENDING 0x40
#define FIELD_CONTACT_TARGETED 0x80

/** @brief FieldObjectState::movement bits. */
#define FIELD_MOVEMENT_CHARGED 0x0400
#define FIELD_MOVEMENT_SEQUENCE_0800 0x0800
#define FIELD_MOVEMENT_SEQUENCE_1000 0x1000
#define FIELD_MOVEMENT_TECHNIQUE 0x8000

/** @brief FieldActor::control bits above the control mode. */
#define FIELD_CONTROL_MOVING 0x200
#define FIELD_CONTROL_MOVEMENT_UNK400 0x400

/** @brief FieldObjectPart::flags bit toggled by script opcodes 0xA5 and 0xA6. */
#define FIELD_PART_FLAG_800000 0x800000

/** @brief FieldActionSlot command bit marking a technique; the low bits index the weapon's techniques. */
#define FIELD_ACTION_TECHNIQUE 0x8000
#define FIELD_ACTION_TECHNIQUE_MASK 0x7FFF
/** @brief FieldActionSlot flags bit of an instrument (charge) action. */
#define FIELD_ACTION_INSTRUMENT 0x400
/** @brief FieldObjectState::action_parameter value meaning "none". */
#define FIELD_ACTION_PARAMETER_NONE 0xFFFF
/** @brief FieldActionSlot animation value meaning "none". */
#define FIELD_ACTION_NO_ANIMATION 0xFFFF

/** @brief Request bit of an animation that stays bound to its owner, and its resource mask. */
#define FIELD_REQUEST_BOUND 0x8000
#define FIELD_REQUEST_ANIMATION_MASK 0x3FF
/** @brief Bound action animation request flags used by opcode 0x9F. */
#define FIELD_REQUEST_LAYERED 0x4000
#define FIELD_REQUEST_ALL_LAYERS 0x0400
#define FIELD_REQUEST_LAYER_SHIFT 12

/** @brief FieldAnimationDef::flags bit of an animation played on every track. */
#define FIELD_ANIMATION_DEF_LAYERED 0x800

/** @brief Actions blocked while g_field_actions_limited is set (4 to 7). */
#define FIELD_ACTION_LIMITED_FIRST 4
#define FIELD_ACTION_LIMITED_COUNT 4

/** @brief Combo action slot; its variant animations follow FIELD_ANIMATION_COMBO. */
#define FIELD_RESOURCE_ACTION_COMBO 2
#define FIELD_ANIMATION_COMBO 0x1F
/** @brief Combo steps; a chain at the last step ends. */
#define FIELD_COMBO_VARIANTS 5
/** @brief Frames the sequence wait lasts after a combo chain ends. */
#define FIELD_COMBO_END_WAIT 20

/** @brief Resource entry an actor spawned by script opcode 0xB9 is initialized from. */
#define FIELD_SPAWN_RESOURCE_ENTRY 3

/** @brief Effect resource started on an actor spawned by script opcode 0xB9. */
#define FIELD_SPAWN_EFFECT_RESOURCE 0xB0

/** @brief Highest action forwarded to the battle code; higher ones are clamped. */
#define FIELD_ACTION_BATTLE_LAST 10

/** @brief Free animation actor slots an action needs. */
#define FIELD_ACTION_MIN_FREE_SLOTS 3

/** @brief Technique gauge value of a full gauge. */
#define FIELD_TECHNIQUE_GAUGE_FULL 0xFF

/** @brief FieldObjectState::hud flag showing the technique gauge. */
#define FIELD_HUD_SHOW_TECHNIQUE_GAUGE 0x01

/** @brief Techniques per weapon and the first technique sequence (see field_actor_behavior.c). */
#define FIELD_TECHNIQUES_PER_WEAPON 24
#define FIELD_TECHNIQUE_SEQUENCE_BASE 0x88

/** @brief Tracks of an animation actor slot. */
#define FIELD_ACTOR_TRACK_COUNT 3

/** @brief Frame limits of the approach and timed move commands. */
#define FIELD_APPROACH_TIMEOUT 250
#define FIELD_TIMED_MOVE_TIMEOUT 240

/** @brief Frames the boss HUD panel shakes after a hit. */
#define FIELD_HUD_SHAKE_START 5

/** @brief Saturated value of the saved hit counters. */
#define FIELD_COUNTER_MAX (-1)

/** @brief Capacity of the target list built for a targeted animation actor. */
#define FIELD_ACTOR_TARGET_CAPACITY 16

/** @brief Script opcodes (the first byte of each actor script command). */
enum
{
    FIELD_SCRIPT_OP_NOP = 0x00,
    FIELD_SCRIPT_OP_STEP = FIELD_ACTOR_COMMAND_STEP,
    FIELD_SCRIPT_OP_HIT = 0x82,
    FIELD_SCRIPT_OP_ACTION_0 = 0x83,
    FIELD_SCRIPT_OP_ACTION_1 = 0x84,
    FIELD_SCRIPT_OP_ACTION = FIELD_ACTOR_COMMAND_ACTION,
    FIELD_SCRIPT_OP_WALK = FIELD_ACTOR_COMMAND_WALK,
    FIELD_SCRIPT_OP_RISE = FIELD_ACTOR_COMMAND_RISE,
    FIELD_SCRIPT_OP_SINK = FIELD_ACTOR_COMMAND_SINK,
    FIELD_SCRIPT_OP_WALK_TO_TARGET = FIELD_ACTOR_COMMAND_WALK_TO_TARGET,
    FIELD_SCRIPT_OP_WALK_FROM_TARGET = FIELD_ACTOR_COMMAND_WALK_FROM_TARGET,
    FIELD_SCRIPT_OP_SEQUENCE_STEP = FIELD_ACTOR_COMMAND_SEQUENCE_STEP,
    FIELD_SCRIPT_OP_KNOCK_DOWN = 0x8E,
    FIELD_SCRIPT_OP_JUMP = FIELD_ACTOR_COMMAND_JUMP,
    FIELD_SCRIPT_OP_DEFEAT = 0x90,
    FIELD_SCRIPT_OP_MOVE_TEXTURE = 0x97,
    FIELD_SCRIPT_OP_9C = FIELD_ACTOR_COMMAND_9C,
    FIELD_SCRIPT_OP_9D = FIELD_ACTOR_COMMAND_9D,
    FIELD_SCRIPT_OP_LOAD_TECHNIQUE = 0x9E,
    FIELD_SCRIPT_OP_PLAY_BOUND_ACTION = 0x9F,
    FIELD_SCRIPT_OP_APPROACH_TARGET = FIELD_ACTOR_COMMAND_APPROACH_TARGET,
    FIELD_SCRIPT_OP_LOAD_BOUND_ANIMATION = 0xA1,
    FIELD_SCRIPT_OP_CLEAR_UNK10 = 0xA2,
    FIELD_SCRIPT_OP_SET_UNK10 = 0xA3,
    FIELD_SCRIPT_OP_ACTION_END = FIELD_ACTOR_COMMAND_ACTION_END,
    FIELD_SCRIPT_OP_SET_PART_FLAG = 0xA5,
    FIELD_SCRIPT_OP_CLEAR_PART_FLAG = 0xA6,
    FIELD_SCRIPT_OP_TIMED_WALK = FIELD_ACTOR_COMMAND_TIMED_WALK,
    FIELD_SCRIPT_OP_WAIT_ANIMATION = FIELD_ACTOR_COMMAND_WAIT_ANIMATION,
    FIELD_SCRIPT_OP_LOAD_ACTION = 0xA9,
    FIELD_SCRIPT_OP_PLAY_SOUND = 0xAA,
    FIELD_SCRIPT_OP_PLAY_OBJECT_SOUND = 0xAB,
    FIELD_SCRIPT_OP_RUN_TO_TARGET = FIELD_ACTOR_COMMAND_RUN_TO_TARGET,
    FIELD_SCRIPT_OP_RUN = FIELD_ACTOR_COMMAND_RUN,
    FIELD_SCRIPT_OP_WALK_PATH = FIELD_ACTOR_COMMAND_WALK_PATH,
    FIELD_SCRIPT_OP_RUN_PATH = FIELD_ACTOR_COMMAND_RUN_PATH,
    FIELD_SCRIPT_OP_TURN = FIELD_ACTOR_COMMAND_TURN,
    FIELD_SCRIPT_OP_TOGGLE_HIDDEN = 0xB4,
    FIELD_SCRIPT_OP_TIMED_SLIDE = FIELD_ACTOR_COMMAND_TIMED_SLIDE,
    FIELD_SCRIPT_OP_WAIT = FIELD_ACTOR_COMMAND_WAIT,
    FIELD_SCRIPT_OP_SPAWN_ATTACHED = 0xB9,
    FIELD_SCRIPT_OP_START_BOUND_ANIMATION = 0xBC,
    FIELD_SCRIPT_OP_END = 0xFF
};

/** @brief Evasion request: a battle action block with one more word. */
typedef struct
{
    FieldBattleAction action;
    s32 unk1C;
} FieldEvasionRequest;

/** @brief Saved-game counters kept past the mapped part of PadContext. */
typedef struct
{
    u8 unk0[0x3154];
    /** @brief Hits taken by the first player. */
    s32 player_hits;
    /** @brief Enemies defeated. */
    s32 enemies_defeated;
} FieldSaveCounters;

/**
 * @brief Object state viewed from its target position onwards (0x23C stride).
 * @note Only the target stores in field_route_actor_to_object go through this view: the original
 *       code loads &g_field_object_states[0].target_x as one constant and gcc then
 *       derives the plain table base from it for the later path stores.
 */
typedef struct
{
    s32 target_x;
    s32 target_y;
    s32 target_z;
    u8 unk5C[0x23C - 0x5C];
    u8 unk23C[0x50];
} FieldObjectTarget;

/** @brief g_field_object_states addressed from element 0's target_x. */
#define FIELD_OBJECT_TARGETS ((FieldObjectTarget*)&g_field_object_states[0].target_x)

/**
 * @brief &states[index] with index * 8 passed in precomputed (the 0x23C multiply spelled out).
 * @param states Object state table.
 * @param index8 @p index multiplied by 8.
 * @param index Object index.
 */
#define FIELD_OBJECT_STATE_BY_INDEX8(states, index8, index) \
    ((FieldObjectState*)((((index8) + (index)) * 16 - (index)) * 4 + (u32)(states)))

extern s32 g_field_direction_animation_modes[];
extern s32 g_field_actor_walk_animations[];
extern s32 g_field_binding_restart_pending[FIELD_ACTOR_BINDING_COUNT];
extern u16* g_field_actor_scripts;
extern FieldActionRow g_field_resource_actions[];
extern s32 g_field_actions_limited;
extern s32 D_8010AE58;
extern s32 g_field_boss_hud_shake_frame;

s32 field_collision_find_path(FieldCollisionQuery* start, FieldCollisionQuery* goal, FieldPathPoint* path, s32 mode);
void field_collision_dilate_query(FieldCollisionQuery* query);
/* Declared without parameters: the 0xB9 spawn passes the control word as an extra argument. */
void field_restart_actor_animation();
void field_restart_actor_animation_reverse(FieldActor* actor);
s32 field_get_next_animation_frame_count(FieldActor* actor);
/* Defined (void); the calls pass the object index. */
s32 field_count_free_actor_slots();
void field_start_actor_hit_reaction(FieldActor* actor, s32 guard);
void field_knock_down_actor(FieldActor* actor, s32 clear_recovery);
/* Declared without parameters: the definition takes s8 values and these calls pass them unconverted. */
void field_start_actor_jump();
void field_start_actor_defeat();
void field_count_chain_hit(s32 object_index);
/* Defined (void); the call passes the object key. */
void func_800B48B8();
s32 field_battle_resolve_action(FieldBattleAction* action);
s32 field_battle_roll_evasion(FieldBattleAction* action);

static void field_run_actor_script_command(FieldActor* actor);
void field_route_actor_to_object(FieldActor* actor, s32 target_index, s32 restart);
static void field_move_actor_texture_rect(FieldActor* actor, RECT* rect, s32 x, s32 y);

/**
 * @brief Run the next script command of an actor that has a script and no active command.
 * @param actor Actor to step.
 */
void field_step_actor_script(FieldActor* actor)
{
    if (actor->script_index != FIELD_SCRIPT_NONE)
    {
        if (actor->command == FIELD_ACTOR_COMMAND_NONE)
        {
            field_run_actor_script_command(actor);
        }
    }
}

/**
 * @brief Resolve the current command in a field object's active script.
 * @param actor Actor whose active script and script offset are resolved.
 * @return Pointer to the actor's current script command.
 * @note No FIELD code calls it; field_run_actor_script_command computes the same pointer itself.
 */
u8* field_get_object_script_command(FieldActor* actor)
{
    u8* script_base;

    if (actor->script_index == FIELD_SCRIPT_OBJECT)
    {
        script_base = g_field_object_states[actor->object_index].script;
    }
    else
    {
        script_base = (u8*)g_field_actor_scripts + g_field_actor_scripts[actor->script_index];
    }
    return script_base + actor->script_offset;
}

/**
 * @brief Run the actor's current script command (the FIELD_SCRIPT_OP_* opcodes).
 *
 * Most opcodes start the actor command of the same value with their operands
 * as wait, animation or direction; the others change state at once (sound,
 * visibility, binding loads, spawning an attached actor). Each handler
 * advances the script offset past its operands, except where a binding is
 * still loading and the command runs again next frame. Opcodes without a
 * handler do nothing.
 *
 * @param actor Actor whose current script command runs.
 */
static void field_run_actor_script_command(FieldActor* actor)
{
    RECT rect;
    FieldCollisionQuery query_start;
    FieldCollisionQuery query_goal;
    FieldActorSlot* bound_slot;
    FieldActor* spawn_actor;
    FieldActor* spawn_actors;
    FieldObjectState* action_states;
    FieldObjectState* technique_states;
    u8* technique_bindings;
    FieldObjectState* load_states;
    FieldObjectState* load_state;
    FieldActorSlot* load_slots;
    u8* load_bindings;
    u8* load_check_bindings;
    s32 key_or_index; /* resource id (0x9E/0xA1/0xA9), binding index (0x9F/0xBC), animation slot (0x83) */
    FieldObjectState* spawn_states;
    FieldObjectState* precheck_state;
    FieldObjectState* spawn_state;
    s16 next_offset;
    s32 sequence_offset;
    s32 direct_path_offset;
    s32 fallback_path_offset;
    s32 path_offset;
    s16 timed_offset;
    u16 walk_offset;
    s32* pending_flag;
    s32 bound_state;
    s32 spawn_control;
    s32 actor_x;
    FieldMapBounds* bounds;
    s32 map_depth;
    s32 path_command;
    s32 map_width;
    s32 spawn_slot;
    s32 bound_started;
    s32 goal_x;
    s32 goal_z;
    s32 contact_or_z; /* contact word in 0xB9, actor z in 0xB0/0xB1 */
    s32 index_or_count; /* spawn slot (0xB9), track count (0x9F), path length (0xB0/0xB1) */
    s32 one; /* the constant 1 kept in a register (0xB9, 0xBC) */
    s32 animation_binding_offset;
    s32 technique_binding_offset;
    s32 load_check_offset;
    s32 load_binding_offset;
    u16 request;
    u16 charged_parameter;
    u16 slot_animation;
    u8 chain_object;
    u8 technique_owner;
    s32 opcode;
    u8 direction;
    s32 technique_object;
    u8 slot_owner;
    s32 action_or_index; /* action byte (0xA9), track index (0x9F) */
    u8 old_presence;
    u8 sequence_animation;
    u8 wait_frames;
    s32 timed_command;
    u8 load_object;
    u8 byte_arg; /* an operand byte, or the command byte in the path opcodes */
    FieldActionSlot* action_slot;
    void* bound_binding;
    u8* script;
    FieldActorSlot* animation_slots;
    u8* animation_bindings;
    u8* bound_bindings;
    s32 spawn_animation;
    s32 parent_index;
    FieldObjectState* precheck_states;
    FieldActorSlot* bound_slots;
    s32 end_command;
    s32 old_animation;
    s32 end_animation;
    s32 turn_command;
    s32 track_owner;
    s32 animation_slot_offset;
    s32 load_slot_offset;
    FieldActorBinding* bindings;
    FieldActorBinding* binding;
    s32 binding_state;
    s32 binding_owner;
    FieldActorSlot* slots;
    FieldActorSlot* slot;

    bounds = FIELD_MAP_BOUNDS;

    if (actor->script_index == FIELD_SCRIPT_OBJECT)
    {
        script = g_field_object_states[actor->object_index].script;
    }
    else
    {
        script = (u8*)g_field_actor_scripts + g_field_actor_scripts[actor->script_index];
    }
    script += actor->script_offset;

    opcode = script[0];

    switch (opcode)
    {
    case FIELD_SCRIPT_OP_END:
        actor->script_index = FIELD_SCRIPT_NONE;
        actor->unk10 = 0;
        actor->script_offset += 1;
        return;
    case FIELD_SCRIPT_OP_CLEAR_UNK10:
        actor->unk10 = 0;
        actor->script_offset++;
        return;
    case FIELD_SCRIPT_OP_SET_UNK10:
        actor->unk10 = 1;
        actor->script_offset++;
        return;
    case FIELD_SCRIPT_OP_PLAY_SOUND:
        field_play_sound(script[1], FIELD_SOUND_PAN_CENTRE);
        actor->script_offset += 2;
        return;
    case FIELD_SCRIPT_OP_PLAY_OBJECT_SOUND:
        if (actor->object_index < (u32)FIELD_PARTY_COUNT)
        {
            field_play_weapon_sfx(script[1], FIELD_SOUND_PAN_CENTRE, actor->object_index);
        }
        else if (actor->object_index < (u32)(FIELD_PARTY_COUNT * 2))
        {
            field_play_set_sfx(script[1], FIELD_SOUND_PAN_CENTRE, actor->object_index - FIELD_PARTY_COUNT, actor->object_index);
        }
        actor->script_offset += 2;
        return;
    case FIELD_SCRIPT_OP_SPAWN_ATTACHED:
        /* Spawn into the highest free actor record. A for or while loop here gets its constants hoisted. */
        do
        {
            do
            {
                index_or_count = FIELD_ACTOR_COUNT - 1;
                one = 1;
                spawn_states = g_field_object_states;
                spawn_state = spawn_states + (FIELD_ACTOR_COUNT - 1);
                spawn_actors = g_field_actors;
                spawn_actor = spawn_actors + (FIELD_ACTOR_COUNT - 1);
            find_free_record:
                old_presence = spawn_actor->presence;
                if (old_presence == FIELD_ACTOR_UNUSED)
                {
                    field_initialize_actor_record(index_or_count, FIELD_SPAWN_RESOURCE_ENTRY);
                    spawn_control = spawn_actor->control.word & ~FIELD_CONTROL_MODE_MASK;
                    spawn_actor->x = actor->x;
                    spawn_actor->y = actor->y;
                    spawn_control |= FIELD_CONTROL_SCRIPTED;
                    spawn_actor->z = actor->z;
                    spawn_actor->presence = FIELD_ACTOR_HIDDEN;
                    spawn_animation = script[1];
                    spawn_actor->animation_frame = 0;
                    spawn_actor->animation_active = one;
                    spawn_actor->command = FIELD_ACTOR_COMMAND_ATTACHED;
                    spawn_actor->animation = spawn_animation;
                    parent_index = actor->object_index;
                    spawn_actor->removal_delay = 3;
                    spawn_actor->control.word = spawn_control;
                    spawn_actor->script_index = 0;
                    spawn_actor->unk10 = one;
                    spawn_actor->command_param = parent_index;
                    spawn_state->group_flags = 0;
                    spawn_state->flags = 0;
                    spawn_state->key = index_or_count;
                    spawn_state->unk18 = 0;
                    contact_or_z = spawn_state->contact.word;
                    contact_or_z &= ~FIELD_CONTACT_TARGETED;
                    contact_or_z &= ~FIELD_CONTACT_ANIMATION_HIDDEN;
                    spawn_state->contact.word = contact_or_z;
                    field_restart_actor_animation(spawn_actor, spawn_control);
                    spawn_slot = field_find_free_actor_slot(index_or_count, 0);
                    if (spawn_slot != -1)
                    {
                        if (field_start_builtin_animation(index_or_count, spawn_slot, FIELD_SPAWN_EFFECT_RESOURCE) != 0)
                        {
                            field_start_actor_animation(spawn_slot, 0, 0);
                        }
                    }
                    else
                    {
                        spawn_actor->presence = old_presence;
                    }
                }
                else
                {
                    spawn_state -= 1;
                    index_or_count -= 1;
                    spawn_actor -= 1;
                    if (index_or_count >= FIELD_FIRST_SPAWN_ACTOR)
                    {
                        goto find_free_record;
                    }
                }
            } while (0);
        } while (0);

        actor->script_offset += 2;
        return;
    case FIELD_SCRIPT_OP_STEP:
        actor->command = script[0];
        wait_frames = script[1];
        actor->animation_state = wait_frames;
        if (wait_frames == 0)
        {
            actor->animation_state = 1;
        }
        actor->animation_active = 1;
        actor->script_offset += 2;
        field_restart_actor_animation(actor);
        return;
    case FIELD_SCRIPT_OP_ACTION_0:
    case FIELD_SCRIPT_OP_ACTION_1:
    case FIELD_SCRIPT_OP_ACTION:
        action_states = g_field_object_states;
        action_states[actor->object_index].contact.word &= ~FIELD_CONTACT_ACTION_BITS;
        actor->command = script[0];
        if ((opcode == FIELD_SCRIPT_OP_ACTION_0) || (opcode == FIELD_SCRIPT_OP_ACTION_1))
        {
            actor->command = FIELD_ACTOR_COMMAND_ACTION;
            action_states[actor->object_index].action = opcode - FIELD_SCRIPT_OP_ACTION_0;
            actor->script_offset += 1;
        }
        else
        {
            action_states[actor->object_index].action = script[1];
            actor->script_offset += 2;
        }
        chain_object = actor->object_index;
        if ((g_field_object_states[chain_object].action == FIELD_RESOURCE_ACTION_COMBO) && (actor->variant != 0) && (chain_object < (u32)FIELD_PLAYER_COUNT))
        {
            actor->animation = (actor->animation & FIELD_ANIMATION_FACING) + (u16)((u8)actor->variant + FIELD_ANIMATION_COMBO);
            if ((field_get_next_animation_frame_count(actor) == 0) || (actor->variant >= (u32)FIELD_COMBO_VARIANTS))
            {
                g_field_object_states[actor->object_index].retry_count = 0;
                actor->variant = 0;
                g_field_object_states[actor->object_index].flags &= ~FIELD_OBJECT_CHAINING;
                actor->animation &= FIELD_ANIMATION_FACING;
                field_restart_actor_animation_reverse(actor);
                actor->command = FIELD_ACTOR_COMMAND_SEQUENCE_WAIT;
                actor->command_param = FIELD_COMBO_END_WAIT;
                return;
            }
            actor->animation = (actor->animation & FIELD_ANIMATION_FACING) + FIELD_ANIMATION_COMBO;
            g_field_object_states[actor->object_index].flags |= FIELD_OBJECT_CHAINING;
        }
        else
        {
            g_field_object_states[actor->object_index].retry_count = 0;
            actor->variant = 0;
        }
        precheck_states = g_field_object_states;
        precheck_state = &precheck_states[actor->object_index];
        if (precheck_state->flags & FIELD_OBJECT_FLAG_KNOCKED_OUT)
        {
            actor->command = FIELD_ACTOR_COMMAND_NONE;
            return;
        }
        /* A plain 4 <= action <= 7 test compiles differently on the u8 field. */
        if (g_field_actions_limited != 0 && (u32)(precheck_state->action - FIELD_ACTION_LIMITED_FIRST) < FIELD_ACTION_LIMITED_COUNT)
        {
            actor->command = FIELD_ACTOR_COMMAND_NONE;
            return;
        }
        action_slot = &g_field_resource_actions[actor->resource_index].slots[g_field_object_states[actor->object_index].action];
        if (!(action_slot->flags.instrument) && (action_slot->command == 0) && (action_slot->animation == 0))
        {
            actor->command = FIELD_ACTOR_COMMAND_NONE;
            return;
        }
        if ((action_slot->flags.instrument) &&
            ((field_object_has_active_actor_tracks(actor->object_index) != 0) || (field_count_free_actor_slots(actor->object_index) < FIELD_ACTION_MIN_FREE_SLOTS)))
        {
            actor->command = FIELD_ACTOR_COMMAND_NONE;
            return;
        }
        if ((action_slot->command & FIELD_ACTION_TECHNIQUE) && !(action_slot->flags.instrument))
        {
            if (!((actor->object_index < (u32)FIELD_PARTY_COUNT) && (field_object_has_active_actor_tracks(actor->object_index) == 0) && (D_8010AE58 == 0) &&
                  (field_count_free_actor_slots(actor->object_index) >= FIELD_ACTION_MIN_FREE_SLOTS)))
            {
                actor->command = FIELD_ACTOR_COMMAND_NONE;
                return;
            }
            technique_owner = actor->object_index;
            if (!((g_field_object_states[technique_owner].technique_gauge == FIELD_TECHNIQUE_GAUGE_FULL) &&
                  (field_start_streamed_animation(technique_owner, (action_slot->command & FIELD_ACTION_TECHNIQUE_MASK) +
                                                      (u16)((g_field_player_records[technique_owner].weapon_type * FIELD_TECHNIQUES_PER_WEAPON) + FIELD_TECHNIQUE_SEQUENCE_BASE)) != 0)))
            {
                actor->command = FIELD_ACTOR_COMMAND_NONE;
                return;
            }
            animation_slots = g_field_actor_slots;
            animation_bindings = (u8*)g_field_actor_bindings;
            if (actor->object_index < (u32)FIELD_PLAYER_COUNT)
            {
                animation_binding_offset = actor->object_index * sizeof(FieldActorBinding);
            }
            else
            {
                animation_binding_offset = FIELD_SHARED_BINDING * sizeof(FieldActorBinding);
            }
            animation_slots[((FieldActorBinding*)&animation_bindings[animation_binding_offset])->slot].actor_type = g_field_object_states[actor->object_index].action;
        }
        else
        {
            request = action_slot->parameter;
            if (request & FIELD_REQUEST_BOUND)
            {
                if (field_start_streamed_animation(actor->object_index, request & FIELD_REQUEST_ANIMATION_MASK) == 0)
                {
                    actor->command = FIELD_ACTOR_COMMAND_NONE;
                    return;
                }
                animation_slots = g_field_actor_slots;
                animation_bindings = (u8*)g_field_actor_bindings;
                if (actor->object_index < (u32)FIELD_PLAYER_COUNT)
                {
                    animation_binding_offset = actor->object_index * sizeof(FieldActorBinding);
                }
                else
                {
                    animation_binding_offset = FIELD_SHARED_BINDING * sizeof(FieldActorBinding);
                }
                /* A byte offset: indexing animation_slots[] directly changes the address sum. */
                animation_slot_offset = ((FieldActorBinding*)&animation_bindings[animation_binding_offset])->slot * sizeof(FieldActorSlot);
                ((FieldActorSlot*)((u8*)animation_slots + animation_slot_offset))->actor_type = g_field_object_states[actor->object_index].action;
            }
        }
        if (action_slot->flags.instrument)
        {
            g_field_object_states[actor->object_index].contact.word |= FIELD_CONTACT_ACTION_PENDING;
            g_field_object_states[actor->object_index].movement.word &= ~FIELD_MOVEMENT_CHARGED;
            g_field_object_states[actor->object_index].action_charge = 0;
            g_field_object_states[actor->object_index].hud.word &= ~FIELD_HUD_SHOW_TECHNIQUE_GAUGE;
            charged_parameter = action_slot->animation;
            if ((charged_parameter != FIELD_ACTION_NO_ANIMATION) && (charged_parameter != 0))
            {
                g_field_object_states[actor->object_index].action_parameter = action_slot->animation;
            }
        }
        else if (!(action_slot->command & FIELD_ACTION_TECHNIQUE))
        {
            slot_animation = action_slot->animation;
            if ((slot_animation != FIELD_ACTION_NO_ANIMATION) && (slot_animation != 0))
            {
                key_or_index = field_find_free_actor_slot(actor->object_index, 0);
                if ((key_or_index != -1) && (field_start_builtin_animation(actor->object_index, key_or_index, action_slot->animation) != 0))
                {
                    slot_owner = actor->object_index;
                    g_field_actor_slots[key_or_index].actor_type = g_field_object_states[slot_owner].action;
                    field_start_actor_animation(key_or_index, 0, 0);
                }
            }
        }
        field_start_object_ground_effect((struct FieldMotionRecord*)actor, action_slot->flags.target_filter);
        return;
    case FIELD_SCRIPT_OP_RUN:
        actor->running = 1;
        /* fallthrough */
    case FIELD_SCRIPT_OP_WALK:
        actor->command = script[0];
        direction = script[1];
        actor->direction = direction;
        if (g_field_resource_entries[actor->resource_index].flags & FIELD_RESOURCE_HAS_ACTIONS)
        {
            actor->animation = g_field_actor_walk_animations[direction >> FIELD_DIRECTION_SECTOR_SHIFT];
        }
        else
        {
            actor->animation = g_field_direction_animation_modes[direction >> FIELD_DIRECTION_SECTOR_SHIFT] + ((actor->running & 1) * FIELD_ANIMATION_DIRECTIONS) + FIELD_ANIMATION_WALK;
        }
        byte_arg = script[2];
        actor->animation_active = 1;
        walk_offset = actor->script_offset;
        walk_offset += 3;
        actor->animation_state = byte_arg;
        actor->script_offset = walk_offset;
        field_restart_actor_animation(actor);
        return;
    case FIELD_SCRIPT_OP_SEQUENCE_STEP:
        actor->command = script[0];
        sequence_animation = script[1] | (actor->animation & FIELD_ANIMATION_FACING);
        actor->animation = sequence_animation;
        if (sequence_animation & FIELD_ANIMATION_FACING)
        {
            actor->direction = 0;
        }
        else
        {
            actor->direction = 0x80;
        }
        byte_arg = script[2];
        actor->animation_active = 1;
        sequence_offset = actor->script_offset;
        sequence_offset += 3;
        actor->animation_state = byte_arg;
        actor->script_offset = sequence_offset;
        field_restart_actor_animation(actor);
        return;
    case FIELD_SCRIPT_OP_JUMP:
        field_start_actor_jump(actor, script[1], script[2], script[3]);
        actor->script_offset += 4;
        return;
    case FIELD_SCRIPT_OP_WALK_PATH:
    case FIELD_SCRIPT_OP_RUN_PATH:
        actor_x = actor->x;
        if (actor_x < 0 || actor_x >= (map_width = bounds->width << 8) || (contact_or_z = actor->z) < 0 ||
            contact_or_z >= (map_depth = (s16)bounds->depth << 9) ||
            (goal_x = g_field_object_states[actor->object_index].target_x) < 0 || goal_x >= map_width ||
            (goal_z = g_field_object_states[actor->object_index].target_z) < 0 || goal_z >= map_depth)
        {
            g_field_object_states[actor->object_index].path_index = 0;
            g_field_object_states[actor->object_index].path[0].x = g_field_object_states[actor->object_index].target_x;
            g_field_object_states[actor->object_index].path[0].z = g_field_object_states[actor->object_index].target_z;
            g_field_object_states[actor->object_index].path_length = 1;
            path_command = script[0];
            actor->animation_state = FIELD_WAIT_UNTIL_DONE;
            actor->animation_active = 1;
            direct_path_offset = actor->script_offset + 1;
            actor->command = path_command;
            actor->script_offset = direct_path_offset;
            field_restart_actor_animation(actor);
            return;
        }
        else
        {
            query_start.x = actor_x;
            query_start.y = actor->y;
            query_start.z = actor->z;
            if (g_field_object_parts[actor->object_index].scale_z == FIELD_PART_FULL_SCALE)
            {
                query_start.width = FIELD_FOOTPRINT_LARGE_WIDTH;
                query_start.depth = FIELD_FOOTPRINT_LARGE_DEPTH;
                query_goal.width = FIELD_FOOTPRINT_LARGE_WIDTH;
                query_goal.depth = FIELD_FOOTPRINT_LARGE_DEPTH;
            }
            else
            {
                query_start.width = FIELD_FOOTPRINT_SMALL_WIDTH;
                query_start.depth = FIELD_FOOTPRINT_SMALL_DEPTH;
                query_goal.width = FIELD_FOOTPRINT_SMALL_WIDTH;
                query_goal.depth = FIELD_FOOTPRINT_SMALL_DEPTH;
            }
            query_start.height_tolerance = FIELD_FOOTPRINT_HEIGHT;
            query_goal.height_tolerance = FIELD_FOOTPRINT_HEIGHT;
            field_collision_dilate_query(&query_start);
            query_goal.x = g_field_object_states[actor->object_index].target_x;
            query_goal.y = g_field_object_states[actor->object_index].target_y;
            query_goal.z = g_field_object_states[actor->object_index].target_z;
            index_or_count = field_collision_find_path(&query_start, &query_goal, g_field_object_states[actor->object_index].path, 0);
            if (index_or_count <= 0)
            {
                g_field_object_states[actor->object_index].path_index = 0;
                g_field_object_states[actor->object_index].path[0].x = g_field_object_states[actor->object_index].target_x;
                g_field_object_states[actor->object_index].path[0].z = g_field_object_states[actor->object_index].target_z;
                g_field_object_states[actor->object_index].path_length = 1;
                path_command = script[0];
                actor->animation_state = FIELD_WAIT_UNTIL_DONE;
                actor->animation_active = 1;
                fallback_path_offset = actor->script_offset;
                fallback_path_offset += 1;
                actor->command = path_command;
                actor->script_offset = fallback_path_offset;
                field_restart_actor_animation(actor);
                return;
            }
            else
            {
                g_field_object_states[actor->object_index].path_length = index_or_count;
                g_field_object_states[actor->object_index].path_index = 0;
                byte_arg = script[0];
                actor->animation_state = FIELD_WAIT_UNTIL_DONE;
                actor->animation_active = 1;
                path_offset = actor->script_offset + 1;
                actor->command = byte_arg;
                actor->script_offset = path_offset;
                field_restart_actor_animation(actor);
                return;
            }
        }

    case FIELD_SCRIPT_OP_RISE:
    case FIELD_SCRIPT_OP_SINK:
    case FIELD_SCRIPT_OP_WALK_TO_TARGET:
    case FIELD_SCRIPT_OP_WALK_FROM_TARGET:
    case FIELD_SCRIPT_OP_RUN_TO_TARGET:
        actor->command = script[0];
        actor->animation_state = script[1];
        actor->script_offset += 2;
        actor->animation_active = 1;
        field_restart_actor_animation(actor);
        return;
    case FIELD_SCRIPT_OP_9C:
    case FIELD_SCRIPT_OP_9D:
        actor->command = script[0];
        actor->animation_state = FIELD_WAIT_UNTIL_DONE;
        actor->animation = script[1] + (actor->animation & FIELD_ANIMATION_FACING);
        actor->command_param = script[2];
        actor->unk26 = script[3];
        actor->script_offset += 4;
        actor->animation_active = 1;
        field_restart_actor_animation(actor);
        return;
    case FIELD_SCRIPT_OP_LOAD_TECHNIQUE:
        technique_bindings = (u8*)g_field_actor_bindings;
        if (actor->object_index < (u32)FIELD_PLAYER_COUNT)
        {
            technique_binding_offset = actor->object_index * sizeof(FieldActorBinding);
        }
        else
        {
            technique_binding_offset = FIELD_SHARED_BINDING * sizeof(FieldActorBinding);
        }
        if (((FieldActorBinding*)(technique_bindings + technique_binding_offset))->state == FIELD_BINDING_IDLE)
        {
            key_or_index = script[1] + (script[2] << 8);
            technique_object = actor->object_index;
            actor->script_offset += 3;
            technique_states = g_field_object_states;
            if (technique_states[technique_object].flags & FIELD_OBJECT_FLAG_KNOCKED_OUT)
            {
                actor->script_index = FIELD_SCRIPT_NONE;
                actor->unk10 = 0;
                return;
            }
            if (g_field_actions_limited != 0)
            {
                actor->script_index = FIELD_SCRIPT_NONE;
                actor->unk10 = 0;
                return;
            }
            if (field_start_streamed_animation(technique_object, key_or_index) == 0)
            {
                actor->script_index = FIELD_SCRIPT_NONE;
                actor->unk10 = 0;
                return;
            }
            g_field_object_states[actor->object_index].movement.word |= FIELD_MOVEMENT_TECHNIQUE;
            return;
        }
        break;
    case FIELD_SCRIPT_OP_LOAD_ACTION:
        load_check_bindings = (u8*)g_field_actor_bindings;
        if (actor->object_index < (u32)FIELD_PLAYER_COUNT)
        {
            load_check_offset = actor->object_index * sizeof(FieldActorBinding);
        }
        else
        {
            load_check_offset = FIELD_SHARED_BINDING * sizeof(FieldActorBinding);
        }
        if (((FieldActorBinding*)(load_check_bindings + load_check_offset))->state == FIELD_BINDING_IDLE)
        {
            action_or_index = script[3];
            key_or_index = script[1] + (script[2] << 8);
            actor->script_offset += 4;
            load_states = g_field_object_states;
            load_states[actor->object_index].action_parameter = FIELD_ACTION_PARAMETER_NONE;
            if (g_field_actions_limited != 0)
            {
                actor->script_index = FIELD_SCRIPT_NONE;
                actor->unk10 = 0;
                return;
            }
            if (field_start_streamed_animation(actor->object_index, key_or_index) == 0)
            {
                actor->script_index = FIELD_SCRIPT_NONE;
                actor->unk10 = 0;
                return;
            }
            load_object = actor->object_index;
            load_state = &load_states[load_object];
            load_slots = g_field_actor_slots;
            load_bindings = (u8*)g_field_actor_bindings;
            if (load_object < (u32)FIELD_PLAYER_COUNT)
            {
                load_binding_offset = load_object * sizeof(FieldActorBinding);
            }
            else
            {
                load_binding_offset = FIELD_SHARED_BINDING * sizeof(FieldActorBinding);
            }
            /* A byte offset: indexing load_slots[] directly changes the address sum. */
            load_slot_offset = ((FieldActorBinding*)&load_bindings[load_binding_offset])->slot * sizeof(FieldActorSlot);
            ((FieldActorSlot*)((u8*)load_slots + load_slot_offset))->actor_type = action_or_index;
            load_state->action = action_or_index;
            g_field_object_states[actor->object_index].movement.word |= FIELD_MOVEMENT_TECHNIQUE;
            return;
        }
        break;
    case FIELD_SCRIPT_OP_START_BOUND_ANIMATION:
        key_or_index = FIELD_SHARED_BINDING;
        if (actor->object_index < (u32)FIELD_PLAYER_COUNT)
        {
            key_or_index = actor->object_index;
        }
        bound_bindings = (u8*)g_field_actor_bindings;
        bound_binding = (key_or_index * sizeof(FieldActorBinding)) + bound_bindings;
        bound_state = ((FieldActorBinding*)bound_binding)->state;
        if (((bound_state >= FIELD_BINDING_LOADING) && (bound_state <= FIELD_BINDING_READY)) && (((FieldActorBinding*)bound_binding)->owner == actor->object_index))
        {
            one = 1;
            if (bound_state != one)
            {
                bound_slots = g_field_actor_slots;
                bound_slot = &bound_slots[((FieldActorBinding*)bound_binding)->slot];
                if (bound_slot->track_mask == 0)
                {
                    bound_slot->animation = bound_slot->default_animation;
                    g_field_object_states[actor->object_index].contact.bytes.target_count = 0;
                    field_start_actor_animation(((FieldActorBinding*)bound_binding)->slot, 0, 0);
                    g_field_object_states[key_or_index].contact.bytes.animation_actor_index = ((FieldActorBinding*)bound_binding)->slot;
                    g_field_actor_slots[((FieldActorBinding*)bound_binding)->slot].unk2A = 1;
                    actor->command = FIELD_ACTOR_COMMAND_WAIT_BOUND_ACTOR;
                }
                actor->script_offset++;
                return;
            }
        }
        else
        {
            actor->script_offset++;
            return;
        }
        break;
    case FIELD_SCRIPT_OP_PLAY_BOUND_ACTION:
        key_or_index = FIELD_SHARED_BINDING;
        if (actor->object_index < (u32)FIELD_PLAYER_COUNT)
        {
            key_or_index = actor->object_index;
        }
        bindings = g_field_actor_bindings;
        binding = &bindings[key_or_index];
        binding_state = binding->state;
        if (((binding_state >= FIELD_BINDING_LOADING) && (binding_state <= FIELD_BINDING_READY)) && (track_owner = actor->object_index, binding_owner = binding->owner, (binding_owner == track_owner)))
        {
            if (binding_state == FIELD_BINDING_LOADING)
            {
                return;
            }
            slots = g_field_actor_slots;
            slot = &slots[binding->slot];
            if (slot->track_mask == 0)
            {
                index_or_count = 0;
                if (slot->default_animation->flags & FIELD_ANIMATION_DEF_LAYERED)
                {
                    for (action_or_index = 0; action_or_index < FIELD_ACTOR_TRACK_COUNT; action_or_index++)
                    {
                        if (slot->part_masks[action_or_index] != 0)
                        {
                            index_or_count += 1;
                        }
                    }
                    bound_started = field_start_bound_action_animation(actor->object_index, 0, 0, ((index_or_count - 1) << FIELD_REQUEST_LAYER_SHIFT) | FIELD_REQUEST_LAYERED | FIELD_REQUEST_ALL_LAYERS);
                }
                else
                {
                    bound_started = field_start_bound_action_animation(binding_owner, 0, 0, 0);
                }
                if (bound_started != 0)
                {
                    actor->command = FIELD_ACTOR_COMMAND_WAIT_BOUND_ACTOR;
                    g_field_object_states[actor->object_index].contact.bytes.target_count = 0;
                }
            }
        }
        actor->script_offset += 2;
        return;
    case FIELD_SCRIPT_OP_APPROACH_TARGET:
    {
        u16 off;
        u8 value;
        value = script[0];
        actor->animation_state = FIELD_APPROACH_TIMEOUT;
        off = actor->script_offset;
        actor->command = value;
        value = script[1];
        off += 2;
        actor->script_offset = off;
        actor->animation_active = 1;
        actor->command_param = value;
        field_restart_actor_animation(actor);
        return;
    }
    case FIELD_SCRIPT_OP_TIMED_WALK:
    case FIELD_SCRIPT_OP_TIMED_SLIDE:
    {
        FieldObjectState* states;
        u8 value;
        timed_command = script[0];
        actor->animation_state = FIELD_APPROACH_TIMEOUT;
        actor->command = timed_command;
        actor->direction = script[1];
        states = g_field_object_states;
        actor->command_param = script[2];
        states[actor->object_index].command_timer = script[3];
        byte_arg = script[4];
        timed_offset = actor->script_offset;
        actor->animation_state = FIELD_TIMED_MOVE_TIMEOUT;
        actor->animation_active = 1;
        timed_offset += 5;
        actor->animation = byte_arg;
        actor->script_offset = timed_offset;
        field_restart_actor_animation(actor);
        return;
    }
    case FIELD_SCRIPT_OP_HIT:
        actor->script_offset += 1;
        field_start_actor_hit_reaction(actor, 0);
        return;
    case FIELD_SCRIPT_OP_KNOCK_DOWN:
        actor->script_offset += 1;
        field_knock_down_actor(actor, 1);
        return;
    case FIELD_SCRIPT_OP_DEFEAT:
        actor->script_offset += 1;
        field_start_actor_defeat(actor, -1);
        return;
    case FIELD_SCRIPT_OP_MOVE_TEXTURE:
        script += 1;
        actor->script_offset += 9;
        rect.x = script[0] + (script[1] << 8);
        rect.y = script[2];
        rect.w = script[3];
        rect.h = script[4];
        field_move_actor_texture_rect(actor, &rect, script[5] | (script[6] << 8), script[7]);
        return;
    case FIELD_SCRIPT_OP_LOAD_BOUND_ANIMATION:
        key_or_index = script[1] + (script[2] << 8);
        actor->script_offset += 3;
        if (g_field_actions_limited == 0 && field_start_streamed_animation(actor->object_index, key_or_index) != 0)
        {
            pending_flag = g_field_binding_restart_pending;
            if (actor->object_index < (u32)FIELD_PLAYER_COUNT)
            {
                pending_flag += actor->object_index;
            }
            else
            {
                pending_flag += FIELD_SHARED_BINDING;
            }
            *pending_flag = 1;
            return;
        }
        actor->script_index = FIELD_SCRIPT_NONE;
        actor->unk10 = 0;
        return;
    case FIELD_SCRIPT_OP_ACTION_END:
        if (actor->object_index < (u32)FIELD_PLAYER_COUNT)
        {
            end_command = script[0];
            actor->animation_state = 1;
            actor->command = end_command;
            old_animation = actor->animation;
            end_animation = script[1];
            actor->animation_active = 1;
            actor->animation = end_animation | (old_animation & FIELD_ANIMATION_FACING);
            field_restart_actor_animation(actor);
        }
        actor->script_offset += 2;
        return;
    case FIELD_SCRIPT_OP_SET_PART_FLAG:
        g_field_object_parts[actor->object_index].flags |= FIELD_PART_FLAG_800000;
        actor->script_offset++;
        return;
    case FIELD_SCRIPT_OP_CLEAR_PART_FLAG:
        g_field_object_parts[actor->object_index].flags &= ~FIELD_PART_FLAG_800000;
        actor->script_offset++;
        return;
    case FIELD_SCRIPT_OP_WAIT_ANIMATION:
    {
        u16 off;
        u8 value;
        actor->command = script[0];
        value = script[2];
        actor->animation_state = value;
        off = actor->script_offset;
        value = script[1];
        off += 3;
        actor->script_offset = off;
        actor->animation_frame = 0;
        actor->animation_active = 1;
        actor->animation = value;
        field_restart_actor_animation(actor);
        return;
    }
    case FIELD_SCRIPT_OP_WAIT:
    {
        u16 off;
        u8 value;
        actor->command = script[0];
        value = script[2];
        actor->animation_state = value;
        actor->animation = script[1];
        off = actor->script_offset;
        value = script[3];
        off += 4;
        actor->script_offset = off;
        actor->animation_frame = 0;
        actor->animation_active = 1;
        actor->command_param = value;
        field_restart_actor_animation_reverse(actor);
        return;
    }
    case FIELD_SCRIPT_OP_TURN:
        turn_command = script[0];
        actor->command_param = 0;
        next_offset = actor->script_offset + 1;
        actor->command = turn_command;
        actor->script_offset = next_offset;
        return;
    case FIELD_SCRIPT_OP_TOGGLE_HIDDEN:
        if (actor->presence == FIELD_ACTOR_HIDDEN)
        {
            actor->presence = FIELD_ACTOR_VISIBLE;
        }
        else
        {
            actor->presence = FIELD_ACTOR_HIDDEN;
        }
        actor->script_offset++;
        return;
    case FIELD_SCRIPT_OP_NOP:
        actor->script_offset++;
        return;
    default:
        return;
    }
}

/**
 * @brief Start the turn command on the actor with @p key from its current facing.
 * @param key Object key to look up.
 * @return 0 when the actor was updated, -1 when no actor has @p key.
 */
s32 field_start_actor_turn(s32 key)
{
    FieldActor* actor;
    u8 animation;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    actor->command = FIELD_ACTOR_COMMAND_TURN;
    animation = actor->animation;
    if (animation < (FIELD_ANIMATION_FACING | FIELD_DIRECTIONAL_ANIMATION_COUNT))
    {
        /* The turn table position is the facing sector counted from facing 0. */
        switch (animation)
        {
        case 1:
        case FIELD_ANIMATION_WALK + 1:
        case FIELD_ANIMATION_RUN + 1:
            actor->command_param = FIELD_TURN_SECTOR_ENTRIES;
            break;
        case 2:
        case FIELD_ANIMATION_WALK + 2:
        case FIELD_ANIMATION_RUN + 2:
            actor->command_param = FIELD_TURN_SECTOR_ENTRIES * 2;
            break;
        case 3:
        case FIELD_ANIMATION_WALK + 3:
        case FIELD_ANIMATION_RUN + 3:
            actor->command_param = FIELD_TURN_SECTOR_ENTRIES * 3;
            break;
        case 4:
        case FIELD_ANIMATION_WALK + 4:
        case FIELD_ANIMATION_RUN + 4:
        case FIELD_ANIMATION_FACING | 4:
        case FIELD_ANIMATION_FACING | (FIELD_ANIMATION_WALK + 4):
        case FIELD_ANIMATION_FACING | (FIELD_ANIMATION_RUN + 4):
            actor->command_param = FIELD_TURN_SECTOR_ENTRIES * 4;
            break;
        case FIELD_ANIMATION_FACING | 3:
        case FIELD_ANIMATION_FACING | (FIELD_ANIMATION_WALK + 3):
        case FIELD_ANIMATION_FACING | (FIELD_ANIMATION_RUN + 3):
            actor->command_param = FIELD_TURN_SECTOR_ENTRIES * 5;
            break;
        case FIELD_ANIMATION_FACING | 2:
        case FIELD_ANIMATION_FACING | (FIELD_ANIMATION_WALK + 2):
        case FIELD_ANIMATION_FACING | (FIELD_ANIMATION_RUN + 2):
            actor->command_param = FIELD_TURN_SECTOR_ENTRIES * 6;
            break;
        case FIELD_ANIMATION_FACING | 1:
        case FIELD_ANIMATION_FACING | (FIELD_ANIMATION_WALK + 1):
        case FIELD_ANIMATION_FACING | (FIELD_ANIMATION_RUN + 1):
            actor->command_param = FIELD_TURN_SECTOR_ENTRIES * 7;
            break;
        case 0:
        default:
            actor->command_param = 0;
            break;
        }
    }
    else
    {
        actor->command_param = 0;
    }
    return 0;
}

/**
 * @brief Toggle the hidden state of the actor with @p key.
 * @param key Object key to look up.
 * @return 0 when the actor was updated, -1 when no actor has @p key.
 */
s32 field_toggle_actor_hidden(s32 key)
{
    FieldActor* actor;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    if (actor->presence == FIELD_ACTOR_HIDDEN)
    {
        actor->presence = FIELD_ACTOR_VISIBLE;
    }
    else
    {
        actor->presence = FIELD_ACTOR_HIDDEN;
    }
    return 0;
}

/**
 * @brief Put the actor with @p key under script control and start its retire command.
 * @param key Object key to look up.
 * @param resource_index Resource of an animation actor to start first, or -1 for none.
 * @return 0 when the actor was updated, -1 when no actor has @p key.
 */
s32 field_retire_actor(s32 key, s32 resource_index)
{
    FieldActor* actor;
    s32 slot_index;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    if (resource_index != -1)
    {
        slot_index = field_find_free_actor_slot(actor->object_index, 0);
        if ((slot_index != -1) && (field_start_builtin_animation(actor->object_index, slot_index, resource_index) != 0))
        {
            field_start_actor_animation(slot_index, 0, 0);
        }
    }

    actor->command = FIELD_ACTOR_COMMAND_RETIRE;
    actor->removal_delay = 2;
    actor->unk10 = 1;
    actor->script_index = 0;
    actor->control.word = (actor->control.word & ~FIELD_CONTROL_MODE_MASK) | FIELD_CONTROL_SCRIPTED;
    return 0;
}

/**
 * @brief Arm the revive timer of the party member with @p key.
 * @param key Object key to look up.
 * @param animation Animation the member revives with.
 * @param effect Resource of the animation actor started on revival, or -1 for none.
 * @param sound Sound effect played on revival, or -1 for none.
 * @param delay Frames until the revival; the timer restarts at 0.
 * @return 0 on success, or -1 when the actor is absent or not a party member.
 */
s32 field_schedule_actor_revive(s32 key, s32 animation, s32 effect, s32 sound, s32 delay)
{
    FieldActor* actor;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    if (actor->object_index >= FIELD_PARTY_COUNT)
    {
        return -1;
    }
    g_field_player_records[actor->object_index].revive_delay = delay;
    g_field_player_records[actor->object_index].revive_time = 0;
    g_field_player_records[actor->object_index].revive_animation = animation;
    g_field_player_records[actor->object_index].revive_effect = effect;
    g_field_player_records[actor->object_index].revive_sound = sound;
    return 0;
}

/**
 * @brief Revive the actor with @p key: idle, full HP, and a stray party member routed back into view.
 * @param key Object key to look up.
 * @param animation New animation, or -1 for animation 0.
 * @param effect Resource of an animation actor to start, or -1 for none.
 * @param sound Sound effect to play, or -1 for none.
 * @return 0 on success, or -1 when no actor has @p key.
 */
s32 field_revive_actor(s32 key, s32 animation, s32 effect, s32 sound)
{
    FieldActor* actor;
    s32 companion_index;
    s32 slot_index;
    SceneState* scene;
    FieldObjectState* hp_state;

    scene = SCENE_STATE;
    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    actor->presence = FIELD_ACTOR_VISIBLE;
    actor->command = FIELD_ACTOR_COMMAND_NONE;
    actor->unk10 = 0;
    actor->animation_state = 1;
    if (animation == -1)
    {
        animation = 0;
    }
    actor->animation_frame = 0;
    actor->animation_active = 1;
    actor->animation = (actor->animation & FIELD_ANIMATION_FACING) | animation;
    field_restart_actor_animation(actor);
    if (effect != -1)
    {
        slot_index = field_find_free_actor_slot(actor->object_index, 0);
        if ((slot_index != -1) && (field_start_builtin_animation(actor->object_index, slot_index, effect) != 0))
        {
            field_start_actor_animation(slot_index, 0, 0);
            g_field_object_states[actor->object_index].contact.bytes.animation_actor_index = slot_index;
        }
    }
    if (sound != -1)
    {
        field_play_sound(sound, FIELD_SOUND_PAN_CENTRE);
    }
    func_800B48B8(g_field_object_states[actor->object_index].key);
    g_field_object_states[actor->object_index].flags = 0;
    g_field_object_states[actor->object_index].contact.bits.flag5 = 0;
    /* unk0 holds the maximum HP; unk4 and the low 24 bits of unk8 the current HP. */
    hp_state = &g_field_object_states[actor->object_index];
    hp_state->unk8.word = (hp_state->unk8.word & 0xFF000000) | (hp_state->unk0 & 0xFFFFFF);
    hp_state->unk4.word = hp_state->unk0 & 0xFFFFFF;
    g_field_object_states[actor->object_index].tint_timer = FIELD_REVIVE_TINT_FRAMES;
    g_field_object_states[actor->object_index].movement.bits.flag15 = 1;
    if (actor->object_index < (u32)FIELD_PARTY_COUNT &&
        (actor->x <= -scene->camera_x + FIELD_VIEW_MARGIN_LEFT || actor->x >= -scene->camera_x + FIELD_VIEW_MARGIN_RIGHT ||
         actor->z <= -scene->camera_z + FIELD_VIEW_MARGIN_LEFT || actor->z >= -scene->camera_z + FIELD_VIEW_MARGIN_BOTTOM))
    {
        /* Walk towards the first other party member that is present and alive. */
        for (companion_index = 0; companion_index < FIELD_PARTY_COUNT; companion_index++)
        {
            if (g_field_actors[companion_index].presence != FIELD_ACTOR_UNUSED && g_field_object_states[companion_index].unk4.word != 0 &&
                actor->object_index != companion_index)
            {
                break;
            }
        }
        if (companion_index != FIELD_PARTY_COUNT)
        {
            field_route_actor_to_object(actor, companion_index, 1);
        }
    }
    return 0;
}

/**
 * @brief Route an actor towards another object, falling back to a straight line.
 * @param actor Actor whose path is computed.
 * @param target_index Object whose position is the destination.
 * @param restart Nonzero starts the path command on the actor.
 */
void field_route_actor_to_object(FieldActor* actor, s32 target_index, s32 restart)
{
    FieldCollisionQuery start;
    FieldCollisionQuery goal;
    s32 path_length;
    FieldMapBounds* bounds;
    s32 x;
    s32 map_depth;
    s32 map_width;
    s32 state_x;
    s32 state_z;
    s32 z;
    FieldActor* target;

    bounds = FIELD_MAP_BOUNDS;
    x = actor->x;
    if (x < 0 || x >= (map_width = bounds->width << 8) || (z = actor->z) < 0 ||
        z >= (map_depth = (s16)bounds->depth << 9) ||
        (state_x = g_field_object_states[actor->object_index].target_x) < 0 || state_x >= map_width ||
        (state_z = g_field_object_states[actor->object_index].target_z) < 0 || state_z >= map_depth)
    {
        g_field_object_states[actor->object_index].path_index = 0;
        g_field_object_states[actor->object_index].path[0].x = g_field_actors[target_index].x;
        g_field_object_states[actor->object_index].path[0].z = g_field_actors[target_index].z;
        g_field_object_states[actor->object_index].path_length = 1;
    }
    else
    {
        start.x = x;
        start.y = actor->y;
        start.z = actor->z;
        if (g_field_object_parts[actor->object_index].scale_z == FIELD_PART_FULL_SCALE)
        {
            start.width = FIELD_FOOTPRINT_LARGE_WIDTH;
            start.depth = FIELD_FOOTPRINT_LARGE_DEPTH;
            goal.width = FIELD_FOOTPRINT_LARGE_WIDTH;
            goal.depth = FIELD_FOOTPRINT_LARGE_DEPTH;
        }
        else
        {
            start.width = FIELD_FOOTPRINT_SMALL_WIDTH;
            start.depth = FIELD_FOOTPRINT_SMALL_DEPTH;
            goal.width = FIELD_FOOTPRINT_SMALL_WIDTH;
            goal.depth = FIELD_FOOTPRINT_SMALL_DEPTH;
        }
        start.height_tolerance = FIELD_FOOTPRINT_HEIGHT;
        goal.height_tolerance = FIELD_FOOTPRINT_HEIGHT;
        field_collision_dilate_query(&start);
        FIELD_OBJECT_TARGETS[actor->object_index].target_x = g_field_actors[target_index].x;
        target = &g_field_actors[target_index];
        FIELD_OBJECT_TARGETS[actor->object_index].target_y = target->y;
        FIELD_OBJECT_TARGETS[actor->object_index].target_z = target->z;
        goal.x = target->x;
        goal.y = target->y;
        goal.z = target->z;
        path_length = field_collision_find_path(&start, &goal, g_field_object_states[actor->object_index].path, 0);
        if (path_length <= 0)
        {
            g_field_object_states[actor->object_index].path_index = 0;
            g_field_object_states[actor->object_index].path[0].x = target->x;
            g_field_object_states[actor->object_index].path[0].z = target->z;
            g_field_object_states[actor->object_index].path_length = 1;
        }
        else
        {
            g_field_object_states[actor->object_index].path_length = path_length;
            g_field_object_states[actor->object_index].path_index = 0;
        }
    }
    if (restart != 0)
    {
        actor->command = FIELD_ACTOR_COMMAND_LEAVE_PATH;
        actor->animation_state = FIELD_WAIT_UNTIL_DONE;
        actor->animation_active = 1;
        field_restart_actor_animation(actor);
    }
}

/**
 * @brief Copy a rectangle inside the actor's texture page area of VRAM.
 * @param actor Actor whose unkC (its resource's texture slot) selects the VRAM area.
 * @param rect Source rectangle in 4-bit texels relative to the area; converted in place to VRAM.
 * @param x Destination X in 4-bit texels relative to the area.
 * @param y Destination Y relative to the area.
 */
static void field_move_actor_texture_rect(FieldActor* actor, RECT* rect, s32 x, s32 y)
{
    s32 x_offset;
    s32 y_offset;
    s32 page;

    page = actor->unkC;
    if (page >= 2)
    {
        y_offset = 0;
        if (page >= 9)
        {
            y_offset = 0x100;
            x_offset = 0x3C0 - ((page - 9) << 6);
        }
        else
        {
            x_offset = 0x340 - (page << 6);
        }
    }
    else
    {
        y_offset = 0;
        x_offset = 0x380 - (page << 7);
    }

    /* Four 4-bit texels per VRAM pixel. */
    rect->x = rect->x >> 2;
    rect->x += x_offset;
    rect->w = rect->w >> 2;
    rect->y += y_offset;
    MoveImage2(rect, (x >> 2) + x_offset, y + y_offset);
}

/**
 * @brief Start loading a bound animation actor for the actor with @p key; it starts once loaded.
 * @param key Object key to look up.
 * @param resource_id Resource to load.
 * @return -1 when no actor has @p key, 1 when no slot could be reserved, 0 on success.
 */
s32 field_load_bound_animation(s32 key, s32 resource_id)
{
    FieldActor* actor;
    s32* pending;
    s32* pending_flags;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    if (field_start_streamed_animation(actor->object_index, resource_id) != 0)
    {
        pending_flags = g_field_binding_restart_pending;
        if (actor->object_index < FIELD_PLAYER_COUNT)
        {
            pending = &pending_flags[actor->object_index];
        }
        else
        {
            pending = &pending_flags[FIELD_SHARED_BINDING];
        }
        *pending = 1;
        g_field_object_states[actor->object_index].action_parameter = FIELD_ACTION_PARAMETER_NONE;
    }
    else
    {
        return 1;
    }
    return 0;
}

/**
 * @brief Resolve a hit on each target an object's attack collected, then clear the list.
 * @param object_index Attacking object.
 */
void field_resolve_collected_hits(s32 object_index)
{
    FieldBattleAction request;
    FieldObjectState* states;
    FieldObjectState* loop_states;
    FieldObjectState* source;
    FieldObjectState* saved_source;
    FieldObjectState* current;
    s32 i;
    s32 index8;

    states = g_field_object_states;
    index8 = object_index * 8;
    source = FIELD_OBJECT_STATE_BY_INDEX8(states, index8, object_index);
    i = 0;
    if (source->contact.bytes.target_count != 0)
    {
        /* The copies and the per-pass index8 keep the source address inside the loop; an indexed for loop hoists it. */
        loop_states = states;
        saved_source = source;
        index8 = object_index * 8;
        do
        {
            current = FIELD_OBJECT_STATE_BY_INDEX8(loop_states, index8, object_index);
            loop_states[current->targets[i]].contact.bits.flag7 = 0;
            if (!loop_states[current->targets[i]].contact.bits.flag5 && loop_states[current->targets[i]].unk4.word != 0)
            {
                request.attacker_id = current->key;
                if (current->action <= FIELD_ACTION_BATTLE_LAST)
                {
                    request.action_id = current->action;
                }
                else
                {
                    request.action_id = FIELD_ACTION_BATTLE_LAST;
                }
                request.target_id = loop_states[saved_source->targets[i]].key;
                request.param = 0;
                request.unk10 = 0;
                request.unk14 = 0;
                request.damage_scale = 1;
                field_battle_resolve_action(&request);
            }
            index8 = object_index * 8;
            i++;
        } while (i < FIELD_OBJECT_STATE_BY_INDEX8(loop_states, index8, object_index)->contact.bytes.target_count);
    }
    g_field_object_states[object_index].contact.bytes.target_count = 0;
}

/**
 * @brief Resolve a hit of one object's current action on another object.
 * @param source_index Attacking object.
 * @param target_index Object that is hit.
 * @return Result of field_battle_resolve_action, or 0 when no hit is resolved.
 */
s32 field_resolve_contact_hit(s32 source_index, s32 target_index)
{
    FieldBattleAction request;
    FieldObjectState* source;
    FieldObjectState* target;
    FieldObjectState* states;
    s32 damage_scale;

    if ((g_field_actors[source_index].command == FIELD_ACTOR_COMMAND_TECHNIQUE) || (g_field_actors[source_index].command == FIELD_ACTOR_COMMAND_INSTRUMENT))
    {
        return 0;
    }
    states = g_field_object_states;
    target = &states[target_index];
    target->contact.bits.flag7 = 0;
    if (target->unk4.word == 0)
    {
        return 0;
    }
    source = &states[source_index];
    request.attacker_id = source->key;
    if (source->action <= FIELD_ACTION_BATTLE_LAST)
    {
        request.action_id = source->action;
    }
    else
    {
        request.action_id = 0;
    }
    request.target_id = g_field_object_states[target_index].key;
    if (source_index < FIELD_PLAYER_COUNT)
    {
        request.param = g_field_actors[source_index].variant;
    }
    else
    {
        request.param = 0;
    }
    request.unk10 = 0;
    request.unk14 = 0;
    damage_scale = g_field_object_states[source_index].contact.bits.damage_scale;
    if (damage_scale == 0)
    {
        damage_scale = 1;
    }
    request.damage_scale = damage_scale;
    field_count_chain_hit(source_index);
    return field_battle_resolve_action(&request);
}

/**
 * @brief Resolve a hit with an explicit action of one object on another object.
 * @param source_index Attacking object.
 * @param target_index Object that is hit.
 * @param action Battle action to resolve.
 * @return Result of field_battle_resolve_action, or 0 when no hit is resolved.
 */
s32 field_resolve_object_hit(s32 source_index, s32 target_index, s32 action)
{
    FieldObjectState* states;
    FieldObjectState* target;
    FieldBattleAction request;

    if ((g_field_actors[source_index].command != FIELD_ACTOR_COMMAND_TECHNIQUE) && (g_field_actors[source_index].command != FIELD_ACTOR_COMMAND_INSTRUMENT))
    {
        states = g_field_object_states;
        target = &states[target_index];
        target->contact.bits.flag7 = 0;
        if (target->unk4.word != 0)
        {
            request.attacker_id = states[source_index].key;
            request.action_id = action;
            request.target_id = target->key;
            request.param = 0;
            request.unk10 = 0;
            request.unk14 = 0;
            request.damage_scale = 1;
            field_count_chain_hit(source_index);
            return field_battle_resolve_action(&request);
        }
    }
    return 0;
}

/**
 * @brief Roll whether one object evades another.
 * @param source_index Attacking object.
 * @param target_index Object that may evade.
 * @return Result of field_battle_roll_evasion.
 */
s32 field_roll_object_evasion(s32 source_index, s32 target_index)
{
    FieldEvasionRequest request;

    request.action.attacker_id = g_field_object_states[source_index].key;
    request.action.target_id = g_field_object_states[target_index].key;
    request.action.damage_scale = 1;
    return field_battle_roll_evasion(&request.action);
}

/**
 * @brief Register a hit on the actor with @p key and start its hit reaction unless its command resists it.
 * @param key Object key to look up.
 * @param guard Guard selector forwarded to field_start_actor_hit_reaction.
 * @return 0 when the actor was found, -1 when no actor has @p key.
 */
s32 field_register_actor_hit(s32 key, s32 guard)
{
    FieldActor* actor;
    s16 command;
    s32 hit_count;
    FieldObjectState* states;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    if (actor->object_index == 0)
    {
        hit_count = ((FieldSaveCounters*)g_pad_ctx)->player_hits;
        if (hit_count != FIELD_COUNTER_MAX)
        {
            ((FieldSaveCounters*)g_pad_ctx)->player_hits = hit_count + 1;
        }
    }
    states = g_field_object_states;
    states[actor->object_index].flags |= FIELD_OBJECT_FLAG_HIT;
    if (actor->object_index < FIELD_PARTY_COUNT)
    {
        g_field_player_records[actor->object_index].hit_state = FIELD_HUD_SHAKE_START;
    }
    else if (g_field_object_states[actor->object_index].unk8.word < 0)
    {
        /* Bit 31 of unk8 marks a boss. */
        g_field_boss_hud_shake_frame = FIELD_HUD_SHAKE_START;
    }
    command = actor->command;
    if (command == FIELD_ACTOR_COMMAND_INSTRUMENT)
    {
        return 0;
    }
    if (command >= FIELD_ACTOR_COMMAND_WALK)
    {
        if (command != FIELD_ACTOR_COMMAND_TECHNIQUE)
        {
            field_start_actor_hit_reaction(actor, guard);
        }
        return 0;
    }
    if (command >= FIELD_ACTOR_COMMAND_ACTION)
    {
        /* Actions 0-3 and 8-10 are always interrupted; the others only with contact flag6. */
        switch (g_field_object_states[actor->object_index].action)
        {
        case 0:
        case 1:
        case 2:
        case 3:
        case 8:
        case 9:
        case 10:
            break;
        default:
            if (!g_field_object_states[actor->object_index].contact.bits.flag6)
            {
                return 0;
            }
            break;
        }
    }
    field_start_actor_hit_reaction(actor, guard);
    return 0;
}

/**
 * @brief Knock down the actor with @p key.
 * @param key Object key to look up.
 * @return 0 when the actor was updated, -1 when no actor has @p key.
 */
s32 field_knock_down_actor_by_key(s32 key)
{
    FieldActor* actor;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    field_knock_down_actor(actor, 1);
    return 0;
}

/**
 * @brief Read the animation of the actor with @p key.
 * @param key Object key to look up.
 * @return The animation without its mirror bit, or -1 when no actor has @p key.
 */
s32 field_get_actor_animation(s32 key)
{
    FieldActor* actor;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    return actor->animation & FIELD_ANIMATION_INDEX_MASK;
}

/**
 * @brief Defeat the actor with @p key, counting enemy defeats in the saved game.
 * @param key Object key to look up.
 * @param value Value forwarded to field_start_actor_defeat.
 * @return 0 when the actor was updated, -1 when no actor has @p key.
 */
s32 field_start_actor_defeat_by_key(s32 key, s32 value)
{
    FieldActor* actor;
    s32 defeat_count;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    if (actor->object_index >= FIELD_PARTY_COUNT)
    {
        defeat_count = ((FieldSaveCounters*)g_pad_ctx)->enemies_defeated;
        if (defeat_count != FIELD_COUNTER_MAX)
        {
            ((FieldSaveCounters*)g_pad_ctx)->enemies_defeated = defeat_count + 1;
        }
    }
    field_start_actor_defeat(actor, value);
    return 0;
}

/**
 * @brief Test whether the actor with @p key is idle: no command, and for player control no movement.
 * @param key Object key to look up.
 * @return 1 when idle, 0 when busy, -1 when no actor has @p key.
 */
s32 field_is_actor_motion_idle(s32 key)
{
    s32 result;
    s32 movement;
    FieldActor* actor;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    if ((actor->control.half[0] & FIELD_CONTROL_MODE_MASK) < FIELD_CONTROL_SCRIPTED)
    {
        result = 0;
        if (actor->command == FIELD_ACTOR_COMMAND_NONE)
        {
            movement = actor->control.word & (FIELD_CONTROL_MOVING | FIELD_CONTROL_MOVEMENT_UNK400);
            result = movement == 0;
        }
    }
    else
    {
        result = 0;
        if ((actor->command == FIELD_ACTOR_COMMAND_STEP) || (actor->command == FIELD_ACTOR_COMMAND_NONE))
        {
            result = 1;
        }
    }
    return result;
}

/**
 * @brief Start a jump on the actor with @p key.
 * @param key Object key to look up.
 * @param heading Heading forwarded to field_start_actor_jump.
 * @param animation Animation forwarded to field_start_actor_jump.
 * @param wait Wait count forwarded to field_start_actor_jump.
 * @return 0 when the actor was updated, -1 when no actor has @p key.
 */
s32 field_start_actor_jump_by_key(s32 key, s32 heading, s32 animation, s32 wait)
{
    FieldActor* actor;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    field_start_actor_jump(actor, heading, animation, wait);
    return 0;
}

/**
 * @brief Turn the actor with @p source_key to face the actor with @p target_key.
 * @param source_key Key of the actor that turns.
 * @param target_key Key of the actor to face.
 * @return 0 on success, or -1 when either actor is absent.
 */
s32 field_face_actor(s32 source_key, s32 target_key)
{
    FieldActor* source;
    FieldActor* target;
    s32 angle;

    source = field_find_actor(source_key);
    if (source == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    target = field_find_actor(target_key);
    if (target == FIELD_ACTOR_NONE)
    {
        return -1;
    }

    angle = ratan2(source->z - target->z, target->x - source->x);
    if (!(g_field_resource_entries[source->resource_index].flags & FIELD_RESOURCE_HAS_ACTIONS))
    {
        /* Eight facings; mirrored animations cover the right-hand side. */
        if (angle < -7 * FIELD_FACING_HALF_SECTOR)
        {
            source->animation = 2;
        }
        else if (angle < -5 * FIELD_FACING_HALF_SECTOR)
        {
            source->animation = 3;
        }
        else if (angle < -3 * FIELD_FACING_HALF_SECTOR)
        {
            source->animation = 4;
        }
        else if (angle < -FIELD_FACING_HALF_SECTOR)
        {
            source->animation = FIELD_ANIMATION_FACING | 3;
        }
        else if (angle < FIELD_FACING_HALF_SECTOR)
        {
            source->animation = FIELD_ANIMATION_FACING | 2;
        }
        else if (angle < 3 * FIELD_FACING_HALF_SECTOR)
        {
            source->animation = FIELD_ANIMATION_FACING | 1;
        }
        else if (angle < 5 * FIELD_FACING_HALF_SECTOR)
        {
            source->animation = 0;
        }
        else if (angle < 7 * FIELD_FACING_HALF_SECTOR)
        {
            source->animation = 1;
        }
        else
        {
            source->animation = 2;
        }
    }
    else
    {
        /* Two facings: plain faces left, mirrored faces right. */
        if (angle > ONE / 4 && angle < ONE * 3 / 4)
        {
            source->animation = 0;
        }
        else if (angle < -(ONE / 4))
        {
            if (angle <= -(ONE * 3 / 4))
            {
                source->animation = FIELD_ANIMATION_FACING;
            }
            else
            {
                source->animation = 0;
            }
        }
        else
        {
            source->animation = FIELD_ANIMATION_FACING;
        }
    }

    source->animation_state = 1;
    source->animation_frame = 0;
    source->animation_active = 1;
    g_field_object_states[source->object_index].movement.word &= ~(FIELD_MOVEMENT_SEQUENCE_0800 | FIELD_MOVEMENT_SEQUENCE_1000);
    field_restart_actor_animation(source);
    return 0;
}

/**
 * @brief Set the animation of the actor with @p key and restart it.
 * @param key Object key to look up.
 * @param animation New animation byte.
 * @return 0 on success, or -1 when no actor has @p key.
 */
s32 field_set_actor_animation(s32 key, u8 animation)
{
    FieldActor* actor;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    actor->animation = animation;
    actor->animation_state = 1;
    actor->animation_frame = 0;
    actor->animation_active = 1;
    g_field_object_states[actor->object_index].movement.word &= ~(FIELD_MOVEMENT_SEQUENCE_0800 | FIELD_MOVEMENT_SEQUENCE_1000);
    field_restart_actor_animation(actor);
    return 0;
}

/**
 * @brief Convert the facing of the actor with @p key to a direction byte.
 * @param key Object key to look up.
 * @return The direction (0x00-0xE0 in steps of 0x20), the mirror bit for two-facing actors, or -1 when absent.
 */
s32 field_get_actor_facing(s32 key)
{
    FieldActor* actor;
    s32 result;
    s32 animation;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    if (!(g_field_resource_entries[actor->resource_index].flags & FIELD_RESOURCE_HAS_ACTIONS))
    {
        animation = actor->animation;
        if ((animation & FIELD_ANIMATION_INDEX_MASK) < FIELD_DIRECTIONAL_ANIMATION_COUNT)
        {
            switch (animation)
            {
            case 1:
            case FIELD_ANIMATION_WALK + 1:
            case FIELD_ANIMATION_RUN + 1:
                result = 0x60;
                break;
            case 2:
            case FIELD_ANIMATION_WALK + 2:
            case FIELD_ANIMATION_RUN + 2:
                result = 0x80;
                break;
            case 3:
            case FIELD_ANIMATION_WALK + 3:
            case FIELD_ANIMATION_RUN + 3:
                result = 0xA0;
                break;
            case 4:
            case FIELD_ANIMATION_WALK + 4:
            case FIELD_ANIMATION_RUN + 4:
            case FIELD_ANIMATION_FACING | 4:
            case FIELD_ANIMATION_FACING | (FIELD_ANIMATION_WALK + 4):
            case FIELD_ANIMATION_FACING | (FIELD_ANIMATION_RUN + 4):
                result = 0xC0;
                break;
            case FIELD_ANIMATION_FACING | 1:
            case FIELD_ANIMATION_FACING | (FIELD_ANIMATION_WALK + 1):
            case FIELD_ANIMATION_FACING | (FIELD_ANIMATION_RUN + 1):
                result = 0x20;
                break;
            case FIELD_ANIMATION_FACING | 2:
            case FIELD_ANIMATION_FACING | (FIELD_ANIMATION_WALK + 2):
            case FIELD_ANIMATION_FACING | (FIELD_ANIMATION_RUN + 2):
                result = 0;
                break;
            case FIELD_ANIMATION_FACING | 3:
            case FIELD_ANIMATION_FACING | (FIELD_ANIMATION_WALK + 3):
            case FIELD_ANIMATION_FACING | (FIELD_ANIMATION_RUN + 3):
                result = 0xE0;
                break;
            case 0:
            case FIELD_ANIMATION_WALK:
            case FIELD_ANIMATION_RUN:
            case FIELD_ANIMATION_FACING:
            case FIELD_ANIMATION_FACING | FIELD_ANIMATION_WALK:
            case FIELD_ANIMATION_FACING | FIELD_ANIMATION_RUN:
            default:
                result = 0x40;
                break;
            }
        }
        else
        {
            result = 0x40;
        }
    }
    else
    {
        result = actor->animation & FIELD_ANIMATION_FACING;
    }
    return result;
}

/**
 * @brief Read the binding state of the actor with @p key.
 * @param key Object key to look up.
 * @return The FIELD_BINDING_* state of the actor's animation binding, or -1 when absent.
 */
s32 field_read_actor_binding_state(s32 key)
{
    FieldActor* actor;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    return field_actor_binding(actor)->state;
}

/**
 * @brief Start an animation actor for the actor with @p key in a slot its binding does not hold.
 * @param key Object key to look up.
 * @param resource_index Resource of the animation actor.
 * @return -1 when absent, 0 when the animation started, 1 otherwise.
 */
s32 field_spawn_animation_actor(s32 key, s32 resource_index)
{
    FieldActor* actor;
    s32 slot_index;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    slot_index = field_find_free_actor_slot(actor->object_index, 0);
    if ((slot_index != -1) && (field_start_builtin_animation(actor->object_index, slot_index, resource_index) != 0))
    {
        field_start_actor_animation(slot_index, 0, 0);
        return 0;
    }
    return 1;
}

/**
 * @brief Start an animation actor for the actor with @p key in a slot the first binding does not hold.
 * @param key Object key to look up.
 * @param resource_index Resource of the animation actor.
 * @return -1 when absent, 0 when the animation started, 1 otherwise.
 */
s32 field_spawn_shared_animation_actor(s32 key, s32 resource_index)
{
    FieldActor* actor;
    s32 slot_index;

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    slot_index = field_find_free_actor_slot(0, 0);
    if ((slot_index != -1) && (field_start_builtin_animation(actor->object_index, slot_index, resource_index) != 0))
    {
        field_start_actor_animation(slot_index, 0, 0);
        return 0;
    }
    return 1;
}

/**
 * @brief Start an animation actor for the actor with @p key aimed at a list of targets.
 * @param key Object key to look up.
 * @param resource_index Resource of the animation actor.
 * @param target_keys Number of target lookups; each looks up @p key again, so the actor targets itself.
 * @param unused Target keys the callers pass; never read.
 * @return -1 when absent, 0 when the animation started, 1 otherwise.
 */
s32 field_spawn_targeted_animation_actor(s32 key, s32 resource_index, s32 target_keys, s32* unused)
{
    FieldActor* actor;
    s32 i;
    s32 target_count;
    s32 slot_index;
    s32 targets[FIELD_ACTOR_TARGET_CAPACITY];

    target_count = 0;
    for (i = 0; i < target_keys; i++)
    {
        actor = field_find_actor(key);
        if (actor != FIELD_ACTOR_NONE)
        {
            targets[target_count] = actor->object_index;
            target_count++;
        }
    }

    actor = field_find_actor(key);
    if (actor == FIELD_ACTOR_NONE)
    {
        return -1;
    }
    slot_index = field_find_free_actor_slot(actor->object_index, 0);
    if ((slot_index != -1) && (field_start_builtin_animation(actor->object_index, slot_index, resource_index) != 0))
    {
        field_start_actor_animation(slot_index, target_count, (u8*)targets);
        return 0;
    }
    return 1;
}

/**
 * @brief Clear the pending animation restart flags of the three bindings.
 */
void field_clear_pending_binding_restarts(void)
{
    g_field_binding_restart_pending[2] = 0;
    g_field_binding_restart_pending[1] = 0;
    g_field_binding_restart_pending[0] = 0;
}

/**
 * @brief Restart the loaded animation actors whose restart is pending.
 */
void field_restart_pending_bindings(void)
{
    s32 i;
    s32 owner;

    for (i = 0; i < FIELD_ACTOR_BINDING_COUNT; i++)
    {
        if (g_field_binding_restart_pending[i] != 0 && g_field_actor_bindings[i].state == FIELD_BINDING_READY)
        {
            g_field_actor_slots[g_field_actor_bindings[i].slot].animation = g_field_actor_slots[g_field_actor_bindings[i].slot].default_animation;
            field_start_actor_animation(g_field_actor_bindings[i].slot, 0, 0);
            owner = g_field_actor_bindings[i].owner;
            if (owner >= FIELD_PARTY_COUNT)
            {
                owner = FIELD_PARTY_COUNT - 1;
            }
            g_field_object_states[owner].contact.bytes.animation_actor_index = g_field_actor_bindings[i].slot;
            g_field_binding_restart_pending[i] = 0;
            g_field_actor_slots[g_field_actor_bindings[i].slot].unk2A = 1;
        }
    }
}
