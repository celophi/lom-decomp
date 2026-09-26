#include "field_scene_transition.h"
#include "field_text.h"
#include "game_audio.h"
#include "game_state.h"
#include "main.h"
#include "saved_game.h"
#include "scene_state.h"
#include "sdk/rand.h"
#include "common.h"
#include "field_calls.h"
#include "field_script.h"
#include "field_interaction_start.h"
#include "field_records.h"
#include "field_scene_internal.h"

/** @brief Number of field text macro slots. */
#define FIELD_TEXT_MACRO_COUNT 16

/** @brief Text macro holding the name of the current day of the week. */
#define FIELD_TEXT_MACRO_WEEKDAY 12

/** @brief Text macro holding the name of party member 0; member n uses this slot minus n. */
#define FIELD_TEXT_MACRO_PARTY_NAME 15

/** @brief Character budget of a party member name macro. */
#define FIELD_NAME_CHARACTER_LIMIT 21

/** @brief Character budget of a macro with no practical limit. */
#define FIELD_TEXT_NO_LIMIT 0xFF

/** @brief Fade length set up for a new scene, in frames. */
#define FIELD_SCENE_FADE_FRAMES 16

/** @brief fade_timer value that skips the transition fade. */
#define FIELD_FADE_SKIP 0xFF

/** @brief Script local-variable bases of the three party actors and the first scene actor. */
#define FIELD_LOCAL_BASE_PARTY_0 0x30
#define FIELD_LOCAL_BASE_PARTY_1 0x31
#define FIELD_LOCAL_BASE_PARTY_2 0x38
#define FIELD_LOCAL_BASE_FIRST_FREE 0x40

/** @brief Selector of a record without one. */
#define FIELD_NO_SELECTOR 0xFF

/** @brief Owner id of event record 0; event record n is this plus n. */
#define FIELD_EVENT_OWNER 0x80

/** @brief Selector of a party actor: its party slot plus this value. */
#define FIELD_PARTY_SELECTOR_BASE 0x80

/** @brief Record id of an installed action: its load-list index plus this value. */
#define FIELD_ACTION_ID_BASE 3

/** @brief Script variables written or read by the field runtime. */
#define FIELD_VAR_RANDOM_SEED 0xFA
#define FIELD_VAR_UNKA00 0xA00
#define FIELD_VAR_UNKA02 0xA02
#define FIELD_VAR_UNKA03 0xA03
#define FIELD_VAR_WEEKDAY 0x429C
/** @brief First of eight words (4 bytes apart) with the element levels of the current land. */
#define FIELD_VAR_ELEMENT_LEVELS 0x4300
#define FIELD_VAR_UNK5320 0x5320
#define FIELD_VAR_MUSIC_TRACK 0x5328
/** @brief Non-zero while talk and trigger interactions are blocked. */
#define FIELD_VAR_INTERACTIONS_BLOCKED 0xFE1
/** @brief Non-zero while the party stays under script control after an interaction. */
#define FIELD_VAR_KEEP_PARTY_SCRIPTED 0xFE2
/** @brief Non-zero when leaving for the world map returns to scene 1 instead. */
#define FIELD_VAR_WORLD_MAP_REDIRECT 0xFFF

/** @brief Game control word bit: the game state words must be cleared and a new seed drawn. */
#define FIELD_CONTROL_RESET_WORDS 0x800000

/** @brief Day-of-week bits of the game control word's weekday half. */
#define FIELD_WEEKDAY_MASK 0x7F

/** @brief Diagnostic: a guest or companion is present but its variant variable is unset. */
#define DIAG_MISSING_PARTY_VARIANT 0x320

/** @brief Scene ids that leave the field instead of loading a scene. */
#define FIELD_SCENE_TITLE 0xFFFE
#define FIELD_SCENE_WORLD_MAP 0xFFFF

/** @brief Scene id bits of a transition's scene_id. */
#define FIELD_SCENE_ID_MASK 0x7FFF

/** @brief Spawn id bits of a transition's unk41B byte. */
#define FIELD_SPAWN_ID_MASK 0x1F

/** @brief Runtime state flag: a talk window is open. */
#define FIELD_STATE_TALKING 0x80000

/** @brief Actor control modes of field_set_actor_control_mode. */
#define FIELD_CONTROL_PLAYER 0
#define FIELD_CONTROL_FOLLOWER 1
#define FIELD_CONTROL_SCRIPTED 2

/** @brief Script id bit that runs the script as the field script instead of a talk message. */
#define FIELD_INTERACTION_SCRIPT 0x8000

/** @brief Script id bits of an interaction script. */
#define FIELD_INTERACTION_SCRIPT_ID_MASK 0x7FFF

/** @brief Actor view extent used for the on-screen event. */
#define FIELD_VIEW_WIDTH 320
#define FIELD_VIEW_DEPTH 448

/** @brief field_resolve_talk_window operand values. */
#define FIELD_TALK_AUTO 0xFF
#define FIELD_TALK_EFFECT_SELECTOR 0xFE
#define FIELD_TALK_PLANE_MASK 3
#define FIELD_TALK_PLANE_NO_FACING 0x80
#define FIELD_TALK_PLANE_FACING 0x40
/** @brief Effect flag set when the speaker's facing angle is in the flipped range. */
#define FIELD_TALK_FACING_FLAG 0x40
/** @brief Depth below the view top from which the talk window uses the lower plane. */
#define FIELD_TALK_LOWER_PLANE_DEPTH 0xC000
/** @brief Facing angles (0x100 per turn) that set FIELD_TALK_FACING_FLAG. */
#define FIELD_FACING_FLIP_MIN 0x41
#define FIELD_FACING_FLIP_END 0xC1

/** @brief Actor ids below this are real actors; larger ids are talk-only speakers. */
#define FIELD_ACTOR_ID_LIMIT 0x80

/** @brief Trigger group bits of FieldActionRequest control flags. */
#define FIELD_ACTION_TRIGGER_GROUP_MASK 0xF

/** @brief FieldActorRecord flags bit set for records that only run scripts. */
#define FIELD_ACTOR_SCRIPT_ONLY 0x40000000

/** @brief Source field of FieldActorRecord flags, read from the packed word. */
#define FIELD_ACTOR_SOURCE_SHIFT 4
#define FIELD_ACTOR_SOURCE_MASK 0x3FF

/** @brief Party slot bits of FieldActionRequest.source.actor; the bits above hold the record's source. */
#define FIELD_ACTION_SOURCE_ACTOR_MASK 7
#define FIELD_ACTION_SOURCE_SHIFT 3

/** @brief Number of menu slot groups (FieldGameState.menu_slots). */
#define FIELD_MENU_GROUP_COUNT 2

/** @brief First item entry index of a FIELD_MENU_SLOT_ITEM slot. */
#define FIELD_MENU_ITEM_BASE 0x30

/**
 * @brief Text in a table that starts with s16 offsets relative to the table base.
 * @param table Table base; entry @p index holds the text offset.
 * @param index Text index.
 * @note The integer sum emits the index before the table base, as in the original.
 */
#define FIELD_OFFSET_TABLE_TEXT(table, index) ((u8*)(table) + *(s16*)((index) * 2 + (s32)(table)))

/** @brief Destination and activation policy encoded in an action request. */
typedef enum
{
    FIELD_ACTION_ACTOR = 0,
    FIELD_ACTION_EVENT = 1,
    FIELD_ACTION_PARTY_0 = 2,
    FIELD_ACTION_PARTY_1 = 3,
    FIELD_ACTION_PARTY_2 = 4,
    FIELD_ACTION_MENU = 5,
    FIELD_ACTION_SCRIPT = 6,
    FIELD_ACTION_GROUP_ACTOR = 7
} FieldActionKind;

/** @brief Player map position in whole units. */
typedef struct
{
    u16 x;
    u16 z;
} FieldMapPoint;

extern FieldGameState* g_field_game_state;
extern FieldRuntimeContext* g_field_runtime;
extern SceneState* g_field_scene_state;
extern FieldRuntimeContext D_80122C00;
extern u8 g_field_weekday_names[];
extern u8 g_field_element_level_by_land_level[];
extern u8 g_field_talk_plane_masks[];
extern s32 g_field_interaction_active;
extern s32 g_pending_game_state;
extern FieldMapPoint g_field_player_map_position;

/* Functions of other FIELD files without a shared prototype. */
s32* func_800C1EC8(s32* src, s32* dest, s32 n);
s32 field_read_script_var(s32 owner_id, s32 variable);
void field_start_actor_script(s32 actor_id, s32 mode);
u8* field_get_event_script(s32 script_id);
s32 field_get_actor_position(s32 key, Vec3i* position);
void field_set_actor_control_mode(s32 party_index, s32 mode);
s32 field_get_actor_facing(s32 actor_id);
/* Int parameters on purpose: with the (s32, u8, s8) definition the calls would narrow their arguments. */
s32 field_queue_actor_event(s32 owner_id, s32 event_id, s32 argument);
/* Declared without a prototype: field_update_actor_record forwards its own a0. */
FieldActorRecord* field_find_actor_record();
FieldActorRecord* field_find_actor_record_or_default(u32 actor_id, FieldRuntimeContext* context);
void field_stop_actor_script(s32 actor_id, s32 flags);
void field_script_run(FieldScriptState* state);
s32 akao_cmd_c1(s32 arg0, s32 arg1, s32 arg2);

/* Public functions of this file that other files call without a prototype. */
void field_begin_party_script_control(s32 mode);
s32 field_start_interaction(s32 actor_id, s32 script);

static void field_runtime_init(void);
static void field_bind_runtime_pointers(void);
static void field_reset_scene_fade(void);
static void field_init_text_macros(void);
static void field_init_party_actors(void);
static void field_init_event_records(void);
static void field_init_script_variables(void);
static s32 field_install_menu_action(FieldActionRequest* request, FieldActorRecord** entry_out, s32 request_index, s32* action_index);
static void field_leave_scene(void);
static void field_update_scene_transition(void);
static void field_update_triggers(void);
static void field_update_main_script(void);
static void field_update_event_records(void);

/**
 * @brief Initialize the field runtime context for a new scene.
 * @note Reports a diagnostic when a present guest or companion has no variant set.
 */
static void field_runtime_init(void)
{
    field_bind_runtime_pointers();
    func_800C1EC8(NULL, (s32*)&g_field_runtime->state, 0xB04);
    field_reset_scene_fade();
    field_init_text_macros();
    field_init_party_actors();
    field_init_event_records();
    field_init_script_variables();

    if (g_field_game_state->characters[1].name[0] != 0)
    {
        if (field_get_script_var(0, FIELD_VARIABLE_GUEST_VARIANT) == FIELD_NO_VARIANT)
        {
            record_game_diagnostic(DIAG_ERROR, DIAG_MISSING_PARTY_VARIANT, 1, 0);
        }
    }

    if (g_field_game_state->characters[2].name[0] != 0)
    {
        if (field_get_script_var(0, FIELD_VARIABLE_COMPANION_VARIANT) == FIELD_NO_VARIANT)
        {
            record_game_diagnostic(DIAG_ERROR, DIAG_MISSING_PARTY_VARIANT, 2, 0);
        }
    }
}

/** @brief Bind the game-state, runtime-context and camera pointers. */
static void field_bind_runtime_pointers(void)
{
    g_field_game_state = (FieldGameState*)&g_saved_game;
    g_field_runtime = &D_80122C00;
    g_field_scene_state = SCENE_STATE;
}

/** @brief Reset the pending scene entry and the fade parameters. */
static void field_reset_scene_fade(void)
{
    FieldRuntimeContext* context;

    context = g_field_runtime;
    context->fade_timer = FIELD_SCENE_FADE_FRAMES;
    context->scene_entry = -1;
    context->fade_color.bits.red = 0;
    context->fade_color.bits.green = 0;
    context->fade_color.bits.blue = 0;
}

/** @brief Set up the party name and weekday text macros and script variable 0xA03. */
static void field_init_text_macros(void)
{
    s32 slot;

    for (slot = 0; slot < FIELD_PARTY_SIZE; slot++)
    {
        g_field_text_macros[FIELD_TEXT_MACRO_PARTY_NAME - slot].character_limit = FIELD_NAME_CHARACTER_LIMIT;
        g_field_text_macros[FIELD_TEXT_MACRO_PARTY_NAME - slot].text = g_field_game_state->characters[slot].name;
    }

    g_field_text_macros[FIELD_TEXT_MACRO_WEEKDAY].character_limit = FIELD_TEXT_NO_LIMIT;
    g_field_text_macros[FIELD_TEXT_MACRO_WEEKDAY].text = FIELD_OFFSET_TABLE_TEXT(g_field_weekday_names, g_field_game_state->control.fields.weekday & FIELD_WEEKDAY_MASK);

    if ((field_get_script_var(0, FIELD_VAR_UNKA02) != 0) || ((g_field_game_state->characters[1].info.word & FIELD_CHARACTER_AI) != 0))
    {
        field_set_script_var(0, FIELD_VAR_UNKA03, 1);
    }
    else
    {
        field_set_script_var(0, FIELD_VAR_UNKA03, 0);
    }
}

/**
 * @brief Initialize the party actor records and the script local-variable bases.
 * @note The scene actors count their local bases down from 0x7F.
 */
static void field_init_party_actors(void)
{
    s32 i;
    s32 j;

    for (i = 0; i < FIELD_PARTY_SIZE; i++)
    {
        g_field_runtime->actors[i].flags.bits.active = 1;
        g_field_runtime->actors[i].flags.bits.script_only = 0;
        g_field_runtime->actors[i].flags.bits.spawned = 0;
        g_field_runtime->actors[i].id = i;
        g_field_runtime->actors[i].selector = i - FIELD_PARTY_SELECTOR_BASE;
        g_field_runtime->actors[i].event = FIELD_NO_EVENT;
        for (j = 0; j < FIELD_ACTOR_SCRIPT_COUNT; j++)
        {
            g_field_runtime->actors[i].scripts[j] = FIELD_NO_SCRIPT;
        }
        g_field_runtime->state.actor_count++;
    }

    g_field_runtime->actors[0].script.status.bits.local_base = FIELD_LOCAL_BASE_PARTY_0;
    g_field_runtime->actors[1].script.status.bits.local_base = FIELD_LOCAL_BASE_PARTY_1;
    g_field_runtime->actors[2].script.status.bits.local_base = FIELD_LOCAL_BASE_PARTY_2;

    /* j is reused as the descending local base (the 7-bit field keeps 0x7F, 0x7E, ...). */
    for (i = FIELD_PARTY_SIZE, j = 0xFF; i < FIELD_ACTOR_RECORD_COUNT; i++, j--)
    {
        g_field_runtime->actors[i].script.status.bits.local_base = j;
    }
    g_field_runtime->local_variable_base = FIELD_LOCAL_BASE_FIRST_FREE;
}

/** @brief Initialize the two event records. */
static void field_init_event_records(void)
{
    s32 i;
    s32 j;

    for (i = 0; i < FIELD_EVENT_RECORD_COUNT; i++)
    {
        g_field_runtime->events[i].id = i - FIELD_EVENT_OWNER;
        g_field_runtime->events[i].selector = FIELD_NO_SELECTOR;
        /* The original clears the pending event of actor record i, not of the event record. */
        g_field_runtime->actors[i].event = FIELD_NO_EVENT;
        for (j = 0; j < FIELD_ACTOR_SCRIPT_COUNT; j++)
        {
            g_field_runtime->events[i].scripts[j] = FIELD_NO_SCRIPT;
        }
    }
}

/** @brief Initialize the script variables derived from the game state and the current land. */
static void field_init_script_variables(void)
{
    s32 flags;

    flags = g_field_game_state->control.word;
    if (flags & FIELD_CONTROL_RESET_WORDS)
    {
        g_field_game_state->control.word = flags & ~FIELD_CONTROL_RESET_WORDS;
        func_800C1EC8(NULL, g_field_game_state->words, 0x20);
        field_set_script_var(0, FIELD_VAR_RANDOM_SEED, rand() & 0xFF);
    }

    if (g_field_game_state->control.fields.hero_level >= 18)
    {
        field_set_script_var(0, FIELD_VAR_UNKA00, 1);
    }

    field_set_script_var(0, FIELD_VAR_WEEKDAY, g_field_game_state->control.fields.weekday & FIELD_WEEKDAY_MASK);
    field_set_script_var(0, FIELD_VAR_ELEMENT_LEVELS + 0x00, g_field_element_level_by_land_level[g_field_game_state->lands[g_music_track_index].levels[0]]);
    field_set_script_var(0, FIELD_VAR_ELEMENT_LEVELS + 0x04, g_field_element_level_by_land_level[g_field_game_state->lands[g_music_track_index].levels[1]]);
    field_set_script_var(0, FIELD_VAR_ELEMENT_LEVELS + 0x08, g_field_element_level_by_land_level[g_field_game_state->lands[g_music_track_index].levels[2]]);
    field_set_script_var(0, FIELD_VAR_ELEMENT_LEVELS + 0x0C, g_field_element_level_by_land_level[g_field_game_state->lands[g_music_track_index].levels[3]]);
    field_set_script_var(0, FIELD_VAR_ELEMENT_LEVELS + 0x10, g_field_element_level_by_land_level[g_field_game_state->lands[g_music_track_index].levels[4]]);
    field_set_script_var(0, FIELD_VAR_ELEMENT_LEVELS + 0x14, g_field_element_level_by_land_level[g_field_game_state->lands[g_music_track_index].levels[5]]);
    field_set_script_var(0, FIELD_VAR_ELEMENT_LEVELS + 0x18, g_field_element_level_by_land_level[g_field_game_state->lands[g_music_track_index].levels[6]]);
    field_set_script_var(0, FIELD_VAR_ELEMENT_LEVELS + 0x1C, g_field_element_level_by_land_level[g_field_game_state->lands[g_music_track_index].levels[7]]);
    field_set_script_var(0, FIELD_VAR_UNK5320, field_get_land_distance(g_music_track_index));
    field_set_script_var(0, FIELD_VAR_MUSIC_TRACK, g_music_track_index);
}

/**
 * @brief Install a conditional actor action and initialize its event scripts.
 * @param request Packed action definition, updated with activation and source state.
 * @param request_index Index in the load list; zero resets the action subsystem.
 * @note FIELD_ACTION_PARTY_1 activates the previous (possibly uninitialized) entry
 *       instead of party actor 1, as the original code does.
 */
void field_install_actor_action(FieldActionRequest* request, s32 request_index)
{
    s32 action_index;
    FieldActorRecord* entry;
    s32 value;
    s32 script_index;
    FieldActorRecord* flag_entry;

    if (request_index == 0)
    {
        field_runtime_init();
    }
    value = field_read_script_var(0, request->condition.variable << 16);
    if ((value >= request->condition.minimum) && (value <= request->condition.maximum))
    {
        switch (request->control.bits.kind)
        {
        case FIELD_ACTION_ACTOR:
        {
            u16 count;
            FieldActorRecord* record;

            request->control.bits.active = 1;
            count = g_field_runtime->state.actor_count++;
            record = &g_field_runtime->actors[count];
            entry = record;
            record->id = request_index + FIELD_ACTION_ID_BASE;
            record->flags.bits.active = 1;
            entry->flags.bits.trigger_group = request->control.flags;
            request->control.flags &= ~FIELD_ACTION_TRIGGER_GROUP_MASK;
            entry->flags.bits.source = request->source.actor >> FIELD_ACTION_SOURCE_SHIFT;
            action_index = 0;
            request->source.actor &= FIELD_ACTION_SOURCE_ACTOR_MASK;
            break;
        }
        case FIELD_ACTION_GROUP_ACTOR:
        {
            u16 count;
            FieldActorRecord* record;

            request->control.bits.active = 1;
            count = g_field_runtime->state.actor_count++;
            record = &g_field_runtime->actors[count];
            entry = record;
            record->id = request_index + FIELD_ACTION_ID_BASE;
            record->flags.bits.active = 1;
            entry->flags.bits.trigger_group = request->control.flags;
            entry->flags.bits.source = request->source.actor >> FIELD_ACTION_SOURCE_SHIFT;
            request->control.flags &= ~FIELD_ACTION_TRIGGER_GROUP_MASK;
            action_index = 0;
            if (request->control.bits.group == 0)
            {
                request->control.bits.group = (g_field_game_state->default_group >> 4) + 1;
            }
            request->source.actor &= FIELD_ACTION_SOURCE_ACTOR_MASK;
            break;
        }
        case FIELD_ACTION_SCRIPT:
        {
            u16 count;
            FieldActorRecord* record;
            s32 flags;
            s32 flags_to_set;

            request->control.bits.active = 0;
            count = g_field_runtime->state.actor_count++;
            record = &g_field_runtime->actors[count];
            entry = record;
            record->id = request_index + FIELD_ACTION_ID_BASE;
            action_index = 0;
            record->flags.bits.active = 1;
            flags_to_set = (entry->flags.word & ~FIELD_ACTION_TRIGGER_GROUP_MASK) | (request->control.flags & FIELD_ACTION_TRIGGER_GROUP_MASK);
            /* The loop notes keep this store ahead of the flags copy below; without them sched2 sinks it. */
            do
            {
                entry->flags.word = flags_to_set;
            } while (0);
            /* flag_entry keeps flow from deleting the store above; reusing flags_to_set keeps the copy of its value. */
            flag_entry = entry;
            flags = flag_entry->flags.word;
            flags_to_set = FIELD_ACTOR_SCRIPT_ONLY;
            flag_entry->flags.word = flags | flags_to_set;
            break;
        }
        case FIELD_ACTION_PARTY_0:
            action_index = 0;
            request->control.bits.active = 0;
            entry = &g_field_runtime->actors[0];
            entry->flags.bits.active = 1;
            break;
        case FIELD_ACTION_PARTY_1:
            action_index = 0;
            request->control.bits.active = 0;
            /* The original activates the previous entry, not party actor 1 (flag_entry is shared with the script case). */
            flag_entry = entry;
            entry = &g_field_runtime->actors[1];
            flag_entry->flags.bits.active = 1;
            break;
        case FIELD_ACTION_PARTY_2:
            action_index = 0;
            request->control.bits.active = 0;
            entry = &g_field_runtime->actors[2];
            entry->flags.bits.active = 1;
            break;
        case FIELD_ACTION_EVENT:
        {
            FieldActorRecord* event;

            request->control.bits.active = 0;
            if (request->control.bits.selector < FIELD_MENU_GROUP_COUNT)
            {
                field_classify_menu_slots(request->control.bits.selector);
            }
            event = &g_field_runtime->events[0];
            event->flags.bits.active = 1;
            g_field_runtime->state.flags |= FIELD_STATE_PARTY_MODE_MASK;
            action_index = 0;
            entry = event;
            field_start_interaction(FIELD_EVENT_OWNER, request->scripts[FIELD_EVENT_START]);
            request->scripts[FIELD_EVENT_START] = FIELD_NO_SCRIPT;
            field_begin_party_script_control(FIELD_PARTY_MODE_EVENT);
            break;
        }
        case FIELD_ACTION_MENU:
            request->control.bits.active = field_install_menu_action(request, &entry, request_index, &action_index);
            break;
        }
        if (entry != NULL)
        {
            entry->selector = request->control.bits.selector;
            entry->event = FIELD_NO_EVENT;
            entry->enabled_events = request->enabled_events;
            for (script_index = 0; script_index < FIELD_ACTION_SCRIPT_COUNT; script_index++)
            {
                entry->scripts[script_index] = request->scripts[script_index];
            }
            entry->script.status.bits.local_base = g_field_runtime->local_variable_base;
            g_field_runtime->local_variable_base += request->control.bits.local_variable_count;
            field_queue_actor_event(entry->id, FIELD_EVENT_START, (u8)action_index);
        }
    }
    else
    {
        request->control.bits.active = 0;
    }
}

/**
 * @brief Put the present party members under script control.
 * @param mode FIELD_PARTY_MODE_ALL or FIELD_PARTY_MODE_EVENT takes every member,
 *        FIELD_PARTY_MODE_PAD_CONTROLLED only the pad-controlled ones.
 */
void field_begin_party_script_control(s32 mode)
{
    s32 i;

    for (i = 0; i < FIELD_PARTY_SIZE; i++)
    {
        if (g_field_game_state->characters[i].name[0] != 0)
        {
            switch (mode)
            {
            case FIELD_PARTY_MODE_ALL:
            case FIELD_PARTY_MODE_EVENT:
                field_set_actor_control_mode(i, FIELD_CONTROL_SCRIPTED);
                break;
            case FIELD_PARTY_MODE_PAD_CONTROLLED:
                if (g_field_game_state->characters[i].info.bits.pad_controlled)
                {
                    field_set_actor_control_mode(i, FIELD_CONTROL_SCRIPTED);
                }
                break;
            }
        }
    }

    g_field_interaction_active = 1;
    g_field_runtime->state.bits.party_mode = mode;
}

/** @brief Give the party members back their normal control and clear the party mode. */
void field_end_party_script_control(void)
{
    s32 i;

    for (i = 0; i < FIELD_PARTY_SIZE; i++)
    {
        switch (g_field_runtime->state.bits.party_mode)
        {
        case FIELD_PARTY_MODE_ALL:
        case FIELD_PARTY_MODE_EVENT:
            if (g_field_game_state->characters[i].info.bits.pad_controlled)
            {
                field_set_actor_control_mode(i, FIELD_CONTROL_PLAYER);
                field_stop_actor_script(i, 0);
            }
            else
            {
                field_set_actor_control_mode(i, FIELD_CONTROL_FOLLOWER);
                field_stop_actor_script(i, 0);
            }
            break;
        case FIELD_PARTY_MODE_PAD_CONTROLLED:
            if (g_field_game_state->characters[i].info.bits.pad_controlled)
            {
                field_set_actor_control_mode(i, FIELD_CONTROL_PLAYER);
                field_stop_actor_script(i, 0);
            }
            break;
        }
    }

    g_field_runtime->state.bits.party_mode = 0;
    g_field_interaction_active = 0;
}

/**
 * @brief Resolve a menu action slot and append an actor record when it is in use.
 * @param request Action request containing the packed slot selection; receives the result type.
 * @param entry_out Receives the appended actor record, or NULL when no record is appended.
 * @param request_index Request index; the record id is request_index + FIELD_ACTION_ID_BASE.
 * @param action_index Receives the selected action index when the slot kind needs one.
 * @return -1 when a record is appended, or 0 when the slot is unused or empty.
 */
static s32 field_install_menu_action(FieldActionRequest* request, FieldActorRecord** entry_out, s32 request_index, s32* action_index)
{
    FieldActorRecord* entry;
    u32 group;
    u32 handle;
    u16 count;
    u8 slot;

    slot = request->control.bits.selector;
    group = slot >> 7;
    slot &= 7;

    if (g_field_game_state->menu_slots[group].slots[slot].entry.index < FIELD_MENU_ENTRY_EMPTY)
    {
        handle = g_field_game_state->menu_slots[group].slots[slot].handle;
        switch (handle)
        {
        case FIELD_MENU_SLOT_UNUSED:
            *entry_out = NULL;
            return 0;

        case FIELD_MENU_SLOT_PLAIN:
            *action_index = 0;
            request->source.result_type = 2;
            break;

        case FIELD_MENU_SLOT_ITEM:
            *action_index = g_field_game_state->menu_slots[group].slots[slot].entry.index - FIELD_MENU_ITEM_BASE;
            request->source.result_type = g_field_game_state->menu_slots[group].slots[slot].entry.bits.result_type;
            break;

        case FIELD_MENU_SLOT_INDEXED:
            *action_index = g_field_game_state->menu_slots[group].slots[slot].entry.index;
            request->source.result_type = g_field_game_state->menu_slots[group].slots[slot].entry.bits.result_type;
            break;

        default:
            break;
        }

        count = g_field_runtime->state.actor_count++;
        entry = &g_field_runtime->actors[count];
        *entry_out = entry;
        entry->flags.bits.active = 1;
        (*entry_out)->id = request_index + FIELD_ACTION_ID_BASE;
        (*entry_out)->flags.bits.trigger_group = request->control.flags;
        request->control.flags &= ~FIELD_ACTION_TRIGGER_GROUP_MASK;
        return -1;
    }

    *entry_out = NULL;
    return 0;
}

/** @brief Run one frame of the field runtime: transitions, triggers, scripts and events. */
void field_runtime_update(void)
{
    if (g_field_runtime->transition.bits.finished)
    {
        field_leave_scene();
        return;
    }
    if (g_field_runtime->transition.bits.requested)
    {
        field_update_scene_transition();
    }
    field_update_triggers();
    field_update_main_script();
    field_update_event_records();
    if (g_field_runtime->state.flags & FIELD_STATE_PARTY_PAGE_SCRIPTS)
    {
        func_800B49C0();
    }
    g_field_runtime->frame_count++;
}

/** @brief Leave the field: request the pending game state or start the next scene. */
static void field_leave_scene(void)
{
    u16 scene_id;

    field_set_script_var(0, FIELD_VAR_KEEP_PARTY_SCRIPTED, 0);
    scene_id = g_field_runtime->transition.fields.scene_id;
    switch (scene_id)
    {
    case FIELD_SCENE_TITLE:
        g_pending_game_state = GAME_STATE_RETURN_TO_TITLE;
        g_field_runtime->scene_entry = 0xFFFF;
        g_layout_sub_mode = -1;
        g_layout_option = -1;
        return;
    case FIELD_SCENE_WORLD_MAP:
        g_field_runtime->scene_entry = scene_id;
        g_pending_game_state = GAME_STATE_WORLD_MAP;
        g_layout_sub_mode = -1;
        g_layout_option = -1;
        if (field_get_script_var(0, FIELD_VAR_WORLD_MAP_REDIRECT) != 0)
        {
            g_pending_game_state = GAME_STATE_FIELD;
            g_field_runtime->transition.fields.scene_id = 1;
            g_field_runtime->scene_entry = 0;
        }
        return;
    default:
        field_set_scene_parameters(g_field_runtime->transition.fields.scene_id, g_field_runtime->transition.fields.unk41A, g_field_runtime->transition.fields.unk41B & FIELD_SPAWN_ID_MASK,
                                   g_field_runtime->scene_entry, g_field_runtime->scene_argument1, g_field_runtime->scene_argument2);
        break;
    }
}

/** @brief Advance the scene transition: start the fade on the first frame, then count it down. */
static void field_update_scene_transition(void)
{
    FieldRuntimeContext* context;
    s32 i;
    u16 scene_id;

    context = g_field_runtime;
    if (!context->transition.bits.fade_started)
    {
        for (i = 0; i < FIELD_PARTY_SIZE; i++)
        {
            field_set_actor_control_mode(i, FIELD_CONTROL_SCRIPTED);
        }

        g_field_runtime->transition.bits.fade_started = 1;
        if (g_field_runtime->fade_timer != FIELD_FADE_SKIP)
        {
            scene_id = g_field_runtime->transition.fields.scene_id;
            if ((scene_id != FIELD_SCENE_TITLE) && (scene_id != FIELD_SCENE_WORLD_MAP))
            {
                field_seek_scene_resource(scene_id & FIELD_SCENE_ID_MASK);
            }

            /* Int arguments on purpose: the original loads the whole fade_timer word. */
            ((void (*)(s32, s32, s32, s32))field_set_fade_target)(g_field_runtime->fade_color.bits.red, g_field_runtime->fade_color.bits.green, g_field_runtime->fade_color.bits.blue, g_field_runtime->fade_timer);

            if (g_field_runtime->transition.fields.scene_id == FIELD_SCENE_WORLD_MAP)
            {
                g_layout_option = -1;
                akao_cmd_c1(0, g_field_runtime->fade_timer * 4, 0);
            }

            g_field_runtime->fade_timer++;
            return;
        }
        g_field_runtime->transition.bits.finished = 1;
        return;
    }

    if (context->fade_timer <= 0)
    {
        context->transition.bits.finished = 1;
    }
    g_field_runtime->fade_timer--;
}

/**
 * @brief Pack a position's whole-unit x and z into one word (x high, z low).
 * @param position Position in 24.8 fixed point.
 * @return Packed map position.
 */
static inline s32 field_pack_map_position(VECTOR* position)
{
    return ((position->vx << 8) & 0xFFFF0000) | ((position->vz >> 8) & 0xFFFF);
}

/** @brief Refresh the party map positions and start the first newly entered trigger region. */
static void field_update_triggers(void)
{
    VECTOR positions[FIELD_PARTY_SIZE];
    s32* packed_position;
    s32 triggered;
    FieldRuntimeContext* context;
    s32 region_bit;
    s32 index;
    s32 result;

    g_field_runtime->view_x = -g_field_scene_state->camera_x;
    packed_position = g_field_runtime->actor_positions;
    g_field_runtime->view_z = -(g_field_scene_state->camera_y + g_field_scene_state->camera_z);
    for (index = 0; index < FIELD_PARTY_SIZE; index++, packed_position++)
    {
        result = field_get_actor_position(index, (Vec3i*)&positions[index]);
        if (result != -1)
        {
            *packed_position = field_pack_map_position(&positions[index]);
        }
        else
        {
            *packed_position = result;
        }
    }
    context = g_field_runtime;
    g_field_player_map_position.x = positions[0].vx >> 8;
    g_field_player_map_position.z = positions[0].vz >> 8;
    if (context->trigger_table != NULL)
    {
        region_bit = 1;
        triggered = context->triggered_regions;
        for (index = 0; index < g_field_runtime->trigger_table->count; index++)
        {
            if (!(triggered & region_bit))
            {
                if ((g_field_player_map_position.x >= g_field_runtime->trigger_table->regions[index].min_x) &&
                    (g_field_runtime->trigger_table->regions[index].max_x >= g_field_player_map_position.x) &&
                    (g_field_player_map_position.z >= g_field_runtime->trigger_table->regions[index].min_z) &&
                    (g_field_runtime->trigger_table->regions[index].max_z >= g_field_player_map_position.z))
                {
                    g_field_runtime->triggered_regions |= region_bit;
                    if (g_field_runtime->trigger_table->regions[index].command & FIELD_TRIGGER_SCRIPT)
                    {
                        field_start_interaction(0, g_field_runtime->trigger_table->regions[index].command);
                        return;
                    }
                    field_battle_start(g_field_runtime->trigger_table->regions[index].command);
                    return;
                }
            }
            region_bit *= 2;
        }
    }
}

/** @brief Run the field script, or deliver the pending interaction end events to the actors. */
static void field_update_main_script(void)
{
    s32 i;

    if (g_field_runtime->script.frames[g_field_runtime->script.depth].pc != NULL)
    {
        field_script_run(&g_field_runtime->script);
        return;
    }

    if (g_field_interaction_active != 0)
    {
        for (i = 0; i < g_field_runtime->state.actor_count; i++)
        {
            field_queue_actor_event(g_field_runtime->actors[i].id, FIELD_EVENT_INTERACTION, FIELD_INTERACTION_END);
        }
        if (!g_field_runtime->transition.bits.requested && (field_get_script_var(0, FIELD_VAR_KEEP_PARTY_SCRIPTED) == 0))
        {
            field_end_party_script_control();
        }
    }
    else if ((g_field_runtime->state.flags & FIELD_STATE_TALKING) && (field_text_get_status(0) == -1))
    {
        for (i = 0; i < g_field_runtime->state.actor_count; i++)
        {
            field_queue_actor_event(g_field_runtime->actors[i].id, FIELD_EVENT_INTERACTION, FIELD_INTERACTION_TALK_END);
        }
        g_field_runtime->state.flags &= ~FIELD_STATE_TALKING;
    }
}

/** @brief Run the two event records' scripts and deliver their pending events. */
static void field_update_event_records(void)
{
    s32 i;

    for (i = 0; i < FIELD_EVENT_RECORD_COUNT; i++)
    {
        if (g_field_runtime->events[i].script.frames[g_field_runtime->events[i].script.depth].pc != NULL)
        {
            field_script_run(&g_field_runtime->events[i].script);
        }
        field_run_actor_event(i + FIELD_EVENT_OWNER, g_field_runtime->events[i].event, g_field_runtime->events[i].event_argument);
        g_field_runtime->events[i].event = FIELD_NO_EVENT;
        field_run_actor_event(FIELD_EVENT_OWNER, FIELD_EVENT_FRAME, 0);
        g_field_runtime->events[i].event = FIELD_NO_EVENT;
    }
}

/**
 * @brief Update a spawned actor: deliver its pending event, its on-screen event and its script.
 * @param actor_id Actor id; also passed through to field_find_actor_record in a0.
 * @param unused Unused.
 */
void field_update_actor_record(s32 actor_id, void* unused)
{
    FieldActorRecord* actor;
    Vec3i position;

    actor = field_find_actor_record();
    if ((actor != NULL) && actor->flags.bits.active)
    {
        if (actor->event != FIELD_NO_EVENT)
        {
            field_run_actor_event(actor->id, actor->event, actor->event_argument);
            actor->event = FIELD_NO_EVENT;
        }
        if (!actor->flags.bits.script_only)
        {
            field_get_actor_position(actor_id, &position);
            if (((u32)position.x > (u32)g_field_runtime->view_x) && ((u32)position.z > (u32)g_field_runtime->view_z) &&
                ((u32)position.x < (u32)g_field_runtime->view_x + FIELD_VIEW_WIDTH) && ((u32)position.z < (u32)g_field_runtime->view_z + FIELD_VIEW_DEPTH))
            {
                field_run_actor_event(actor->id, FIELD_EVENT_ON_SCREEN, 0);
            }
            else
            {
                field_run_actor_event(actor->id, FIELD_EVENT_OFF_SCREEN, 0);
            }
            field_run_actor_event(actor->id, FIELD_EVENT_FRAME, 0);
            if (actor->script.frames[actor->script.depth].pc == NULL)
            {
                field_run_actor_event(actor->id, FIELD_EVENT_IDLE, 0);
                return;
            }
            field_script_run(&actor->script);
        }
    }
}

/**
 * @brief Start an actor interaction through its script or the talk presentation.
 * @param actor_id Actor that is talked to.
 * @param script Script id; FIELD_INTERACTION_SCRIPT runs it as the field script, otherwise it is a talk message.
 * @return -1 when the interaction starts, or 0 when it cannot start.
 */
s32 field_start_interaction(s32 actor_id, s32 script)
{
    u16 script_id;
    s32 selector;
    s32 effect;
    s32 plane;
    s32 speaker;
    s32 source;
    s32 i;
    u32 flags;
    s32 other_id;
    FieldActorRecord* actor;

    script_id = script;
    if (script_id == FIELD_NO_SCRIPT)
    {
        return 0;
    }

    if (g_field_runtime->transition.bits.requested)
    {
        return 0;
    }
    if (field_get_script_var(0, FIELD_VAR_INTERACTIONS_BLOCKED) != 0)
    {
        return 0;
    }

    actor = field_find_actor_record(actor_id);
    if (actor == NULL)
    {
        return 0;
    }

    flags = actor->flags.word;
    if (actor->flags.bits.script_only)
    {
        return 0;
    }
    if (actor->flags.bits.spawned)
    {
        return 0;
    }

    source = (flags >> FIELD_ACTOR_SOURCE_SHIFT) & FIELD_ACTOR_SOURCE_MASK;
    if (source != 0)
    {
        field_unlock_encyclopedia_entry(source);
    }

    if ((script & FIELD_INTERACTION_SCRIPT) != 0)
    {
        if (g_field_runtime->script.frames[g_field_runtime->script.depth].pc != NULL)
        {
            return 0;
        }

        for (i = 0; i < g_field_runtime->state.actor_count; i++)
        {
            if (i < FIELD_PARTY_SIZE)
            {
                field_start_actor_script(i, 0);
            }
            else
            {
                other_id = g_field_runtime->actors[i].id;
                if (other_id == actor_id)
                {
                    field_queue_actor_event(actor_id, FIELD_EVENT_INTERACTION, FIELD_INTERACTION_SCRIPT_TARGET);
                }
                else
                {
                    field_queue_actor_event(other_id, FIELD_EVENT_INTERACTION, FIELD_INTERACTION_SCRIPT_OTHER);
                }
            }
        }

        g_field_interaction_active = 1;
        g_field_runtime->script.status.owner_id = actor_id;
        g_field_runtime->script.status.bits.local_base = actor->script.status.bits.local_base;
        g_field_runtime->script.frames[g_field_runtime->script.depth].pc = field_get_event_script(script_id & FIELD_INTERACTION_SCRIPT_ID_MASK);
        g_field_runtime->script.frames[g_field_runtime->script.depth].wait.bits.resume = 0;
        g_field_runtime->script.frames[g_field_runtime->script.depth].wait.bits.frames = 0;
        return -1;
    }

    for (i = 0; i < g_field_runtime->state.actor_count; i++)
    {
        if (i < FIELD_PARTY_SIZE)
        {
            field_start_actor_script(i, 0);
        }
        else
        {
            other_id = g_field_runtime->actors[i].id;
            if (other_id == actor_id)
            {
                field_queue_actor_event(actor_id, FIELD_EVENT_INTERACTION, FIELD_INTERACTION_TALK_TARGET);
            }
            else
            {
                field_queue_actor_event(other_id, FIELD_EVENT_INTERACTION, FIELD_INTERACTION_TALK_OTHER);
            }
        }
    }

    speaker = actor_id;
    plane = FIELD_TALK_AUTO;
    effect = FIELD_TALK_EFFECT_SELECTOR;
    selector = FIELD_TALK_AUTO;
    g_field_runtime->state.flags |= FIELD_STATE_TALKING;
    field_resolve_talk_window(&speaker, &plane, &effect, &selector);
    func_8009C620(plane, selector, speaker, effect);
    func_8009C77C(plane, script_id, 1);
    return -1;
}

/**
 * @brief Return an actor's selector, or -1 when it has none.
 * @param actor Actor record.
 * @return Selector value, or -1.
 */
static inline s32 field_get_actor_selector(FieldActorRecord* actor)
{
    s32 value = -1;

    if (actor->selector != FIELD_NO_SELECTOR)
    {
        value = actor->selector;
    }
    return value;
}

/**
 * @brief Resolve the talk presentation operands in place.
 * @param actor_id Speaking actor; invalid identifiers are replaced with zero.
 * @param plane Window plane, or FIELD_TALK_AUTO to choose it from the actor's screen position.
 * @param effect Window effect, FIELD_TALK_EFFECT_SELECTOR for the actor's selector or FIELD_TALK_AUTO
 *        for none; receives the facing flag.
 * @param selector Window selector, or FIELD_TALK_AUTO for the current one.
 */
void field_resolve_talk_window(s32* actor_id, s32* plane, s32* effect, s32* selector)
{
    Vec3i position;
    s32 plane_value;
    s32 effect_value;
    s32 facing_flag;
    s32 selector_value;
    s32 facing_angle;
    u32 original_actor_id;

    original_actor_id = *actor_id;
    if (original_actor_id < FIELD_ACTOR_ID_LIMIT)
    {
        field_get_actor_position(original_actor_id, &position);
    }
    else
    {
        *actor_id = 0;
    }
    plane_value = *plane;
    if (plane_value != FIELD_TALK_AUTO)
    {
        if (plane_value & FIELD_TALK_PLANE_NO_FACING)
        {
            facing_flag = 0;
        }
        else if (plane_value & FIELD_TALK_PLANE_FACING)
        {
            facing_flag = FIELD_TALK_FACING_FLAG;
        }
        else
        {
            facing_angle = field_get_actor_facing(*actor_id);
            facing_flag = ((facing_angle >= FIELD_FACING_FLIP_MIN) && (facing_angle < FIELD_FACING_FLIP_END)) ? FIELD_TALK_FACING_FLAG : 0;
        }
        *plane &= FIELD_TALK_PLANE_MASK;
    }
    else
    {
        if ((position.z - g_field_runtime->view_z) < FIELD_TALK_LOWER_PLANE_DEPTH)
        {
            *plane = 0;
        }
        else
        {
            *plane = 1;
        }
        facing_angle = field_get_actor_facing(*actor_id);
        facing_flag = ((facing_angle >= FIELD_FACING_FLIP_MIN) && (facing_angle < FIELD_FACING_FLIP_END)) ? FIELD_TALK_FACING_FLAG : 0;
    }
    g_field_runtime->talk_window.bits.plane = *plane;
    effect_value = *effect;
    switch (effect_value)
    {
    case FIELD_TALK_EFFECT_SELECTOR:
        *effect = field_get_actor_selector(field_find_actor_record_or_default(original_actor_id, g_field_runtime));
        break;
    case FIELD_TALK_AUTO:
        *effect = -1;
        break;
    }
    *effect |= facing_flag;
    selector_value = *selector;
    if (selector_value == FIELD_TALK_AUTO)
    {
        selector_value = g_field_runtime->talk_window.bits.selector;
    }
    *selector = selector_value;
    if (!((g_field_talk_plane_masks[selector_value] >> *plane) & 1))
    {
        *effect = -1;
    }
}

/**
 * @brief Set a field text macro's replacement string and character budget.
 * @param slot Macro slot index, checked against the upper bound only.
 * @param text Replacement text.
 * @param character_limit Unsigned character budget.
 */
void field_set_text_macro(s32 slot, u8* text, u8 character_limit)
{
    if (slot < FIELD_TEXT_MACRO_COUNT)
    {
        g_field_text_macros[slot].character_limit = character_limit;
        g_field_text_macros[slot].text = text;
    }
}
