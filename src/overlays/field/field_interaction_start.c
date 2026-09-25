#include "field_scene_transition.h"
#include "field_text.h"
#include "game_audio.h"
#include "saved_game.h"
#include "scene_state.h"
#include "common.h"
#include "field_calls.h"
#include "field_interaction_start.h"
#include "field_records.h"
#include "field_scene_internal.h"

#define FIELD_ACTION_ENTRY_FLAG_MASK 0xF
#define FIELD_ACTION_ACTIVE 0x80000000
#define FIELD_ACTION_INACTIVE_MASK 0x7FFFFFFF
#define FIELD_ACTION_SCRIPT_ONLY 0x40000000
#define FIELD_ACTION_SOURCE_MASK 0x3FF0
#define FIELD_ACTION_GROUP_CLEAR_MASK 0xCFFFFFFF
#define FIELD_ACTION_EVENT_MODE 0x60000
#define FIELD_SCRIPT_LOCAL_BASE_CLEAR_MASK 0xFFFF01FF
#define FIELD_ACTION_NO_EVENT 0xFF
#define FIELD_ACTION_NO_SCRIPT 0xFFFFU

#define FIELD_ACTION_KIND_SHIFT 4
#define FIELD_ACTION_GROUP_SHIFT 28
#define FIELD_ACTION_GROUP_MASK 3
#define FIELD_ACTION_SOURCE_ACTOR_MASK 7
#define FIELD_ACTION_DYNAMIC_ID_BASE 3
#define FIELD_ACTION_EVENT_OWNER 0x80
#define FIELD_ACTION_START_EVENT 15
#define FIELD_SCRIPT_LOCAL_BASE_MASK 0x7F
#define FIELD_SCRIPT_LOCAL_BASE_SHIFT 9

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
    FIELD_ACTION_ACTOR_0 = 2,
    FIELD_ACTION_ACTOR_1 = 3,
    FIELD_ACTION_ACTOR_2 = 4,
    FIELD_ACTION_MENU = 5,
    FIELD_ACTION_SCRIPT = 6,
    FIELD_ACTION_GROUP_ACTOR = 7
} FieldActionKind;

/** @brief Runtime actor or event entry and its script dispatch table. */
typedef struct
{
    s8 id;
    u8 selector;
    u8 padding_02[2];
    u8 event;
    u8 event_argument;
    u16 enabled_events;
    u16 scripts[FIELD_ACTION_SCRIPT_COUNT];
    s32 script_state;
    u8 padding_2c[0x90 - 0x2C];
    union
    {
        s32 word;
        struct
        {
            unsigned options : 4;
            unsigned source : 10;
            unsigned reserved : 16;
            unsigned script_only : 1;
            unsigned active : 1;
        } bits;
    } flags;
} FieldActionEntry;

/** @brief Field action allocation state and actor/event records. */
typedef struct
{
    s32 local_variable_base;
    u8 padding_04[0x400 - 4];
    union
    {
        u16 count;
        s32 flags;
    } actors;
    u8 padding_404[0x430 - 0x404];
    FieldActionEntry entries[16];
    FieldActionEntry event_entries[2];
} FieldActionTable;

/** @brief Layout setting supplying the default actor group. */
typedef struct
{
    u8 padding_00[0x29D4];
    u8 default_group;
} FieldActionLayout;

/** @brief Region trigger: map bounds and the command started on entry (12 bytes). */
typedef struct
{
    u16 min_x;
    u16 min_z;
    u16 max_x;
    u16 max_z;
    /** @brief Bit 15 set: script id for func_800B22F0; otherwise the monster group for field_battle_start. */
    u16 command;
    u16 unk0A;
} FieldTriggerRegion;

/** @brief Scene trigger table loaded into the runtime context. */
typedef struct
{
    u16 unk0;
    u16 count;
    FieldTriggerRegion regions[1];
} FieldTriggerTable;

/** @brief The current scene's trigger table, read through the context pointer on every use. */
#define FIELD_TRIGGERS ((FieldTriggerTable*)g_field_runtime->trigger_table)

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
extern s16 D_800EF600[];
extern u8 D_800F0B48[];
extern u16 g_music_track_index;

void func_800B0BDC(void);
s32* func_800C1EC8(s32* src, s32* dest, s32 n);
void func_800B0C10(void);
void func_800B0C54(void);
void func_800B0D3C(void);
void func_800B0E80(void);
void func_800B0EFC(void);
s32 func_800BD414(s32 owner_id, s32 variable_id);
void func_800BD520(s32 owner_id, u32 variable_id, s32 value);
s32 rand(void);

/**
 * @brief Initialize the field runtime context for a new scene.
 * @note Reports a diagnostic when a present partner's script variable is still unset.
 */
void func_800B0AF8(void)
{
    func_800B0BDC();
    func_800C1EC8(NULL, (s32*)&g_field_runtime->state, 0xB04);
    func_800B0C10();
    func_800B0C54();
    func_800B0D3C();
    func_800B0E80();
    func_800B0EFC();

    if (g_field_game_state->characters[1].name[0] != 0)
    {
        if (func_800BD414(0, 0x2F08) == 0xFF)
        {
            record_game_diagnostic(0x8001, 0x320, 1, 0);
        }
    }

    if (g_field_game_state->characters[2].name[0] != 0)
    {
        if (func_800BD414(0, 0x2F00) == 0xFF)
        {
            record_game_diagnostic(0x8001, 0x320, 2, 0);
        }
    }
}

/** @brief Bind the game-state, runtime-context and camera pointers. */
void func_800B0BDC(void)
{
    g_field_game_state = (FieldGameState*)&g_saved_game;
    g_field_runtime = &D_80122C00;
    g_field_scene_state = SCENE_STATE;
}

/** @brief Reset the pending scene entry and the fade parameters. */
void func_800B0C10(void)
{
    FieldRuntimeContext* context;

    context = g_field_runtime;
    context->fade_timer = 0x10;
    context->scene_entry = -1;
    context->fade_color.bits.red = 0;
    context->fade_color.bits.green = 0;
    context->fade_color.bits.blue = 0;
}

/**
 * @brief Initialize the reserved field text macros and their default selection state.
 */
void func_800B0C54(void)
{
    s32 slot;

    for (slot = 0; slot < FIELD_PARTY_SIZE; slot++)
    {
        g_field_text_macros[15 - slot].character_limit = 0x15;
        g_field_text_macros[15 - slot].text = g_field_game_state->characters[slot].name;
    }

    g_field_text_macros[12].character_limit = 0xFF;
    g_field_text_macros[12].text = FIELD_OFFSET_TABLE_TEXT(D_800EF600, g_field_game_state->control.fields.unk2E6 & 0x7F);

    if ((func_800BD414(0, 0xA02) != 0) || ((g_field_game_state->characters[1].info.word & 0x80) != 0))
    {
        func_800BD520(0, 0xA03, 1);
    }
    else
    {
        func_800BD520(0, 0xA03, 0);
    }
}

/**
 * @brief Initialize the party actor records and the script local-variable bases.
 * @note Party actors get local bases 0x30, 0x31 and 0x38; the other actors count down from 0x7F.
 */
void func_800B0D3C(void)
{
    s32 i;
    s32 j;

    for (i = 0; i < FIELD_PARTY_SIZE; i++)
    {
        g_field_runtime->actors[i].flags.bits.active = 1;
        g_field_runtime->actors[i].flags.bits.script_only = 0;
        g_field_runtime->actors[i].flags.bits.spawned = 0;
        g_field_runtime->actors[i].id = i;
        g_field_runtime->actors[i].selector = i - 0x80;
        g_field_runtime->actors[i].event = FIELD_NO_EVENT;
        for (j = 0; j < FIELD_ACTOR_SCRIPT_COUNT; j++)
        {
            g_field_runtime->actors[i].scripts[j] = FIELD_NO_SCRIPT;
        }
        g_field_runtime->state.actor_count++;
    }

    g_field_runtime->actors[0].script.status.bits.local_base = 0x30;
    g_field_runtime->actors[1].script.status.bits.local_base = 0x31;
    g_field_runtime->actors[2].script.status.bits.local_base = 0x38;

    /* j is reused as the descending local base (the 7-bit field keeps 0x7F, 0x7E, ...). */
    for (i = FIELD_PARTY_SIZE, j = 0xFF; i < FIELD_ACTOR_RECORD_COUNT; i++, j--)
    {
        g_field_runtime->actors[i].script.status.bits.local_base = j;
    }
    g_field_runtime->local_variable_base = 0x40;
}

/**
 * @brief Initialize the two event records.
 */
void func_800B0E80(void)
{
    s32 i;
    s32 j;

    for (i = 0; i < FIELD_EVENT_RECORD_COUNT; i++)
    {
        g_field_runtime->events[i].id = i - 0x80;
        g_field_runtime->events[i].selector = 0xFF;
        g_field_runtime->actors[i].event = FIELD_NO_EVENT;
        for (j = 0; j < FIELD_ACTOR_SCRIPT_COUNT; j++)
        {
            g_field_runtime->events[i].scripts[j] = FIELD_NO_SCRIPT;
        }
    }
}

/**
 * @brief Initialize the field script variables derived from the game state and music track.
 */
void func_800B0EFC(void)
{
    s32 flags;

    flags = g_field_game_state->control.word;
    if (flags & 0x800000)
    {
        g_field_game_state->control.word = flags & 0xFF7FFFFF;
        func_800C1EC8(NULL, g_field_game_state->words, 0x20);
        func_800BD520(0, 0xFA, rand() & 0xFF);
    }

    if (g_field_game_state->control.fields.hero_level >= 0x12)
    {
        func_800BD520(0, 0xA00, 1);
    }

    func_800BD520(0, 0x429C, g_field_game_state->control.fields.unk2E6 & 0x7F);
    func_800BD520(0, 0x4300, D_800F0B48[g_field_game_state->lands[g_music_track_index].levels[0]]);
    func_800BD520(0, 0x4304, D_800F0B48[g_field_game_state->lands[g_music_track_index].levels[1]]);
    func_800BD520(0, 0x4308, D_800F0B48[g_field_game_state->lands[g_music_track_index].levels[2]]);
    func_800BD520(0, 0x430C, D_800F0B48[g_field_game_state->lands[g_music_track_index].levels[3]]);
    func_800BD520(0, 0x4310, D_800F0B48[g_field_game_state->lands[g_music_track_index].levels[4]]);
    func_800BD520(0, 0x4314, D_800F0B48[g_field_game_state->lands[g_music_track_index].levels[5]]);
    func_800BD520(0, 0x4318, D_800F0B48[g_field_game_state->lands[g_music_track_index].levels[6]]);
    func_800BD520(0, 0x431C, D_800F0B48[g_field_game_state->lands[g_music_track_index].levels[7]]);
    func_800BD520(0, 0x5320, func_800C3688(g_music_track_index));
    func_800BD520(0, 0x5328, g_music_track_index);
}

extern u8 D_800EF84C[];
extern s32 g_field_interaction_active;
extern s32 g_pending_game_state;
extern s32 g_layout_sub_mode;
extern s32 g_layout_option;
extern FieldMapPoint D_80042FC8;

void field_start_actor_script(s32 actor_id, s32 mode);
u8* field_get_event_script(s32 script_id);
s32 field_get_actor_position(s32 actor_id, s32* position);
void field_set_actor_control_mode(s32 party_index, s32 mode);
s32 field_get_actor_facing(s32 actor_id);
void func_800B168C(s32 mode);
void func_800B177C(void);
s32 func_800B1894(FieldActionRequest* request, FieldActionEntry** entry_out, s32 request_index, s32* action_index);
void func_800B1AA8(void);
void func_800B1BBC(void);
void func_800B1D10(void);
void func_800B1F10(void);
void func_800B20B4(void);
s32 func_800B22F0(s32 actor_id, s32 script);
void func_800B2654(s32* actor_id, s32* plane, s32* effect, s32* selector);
/* Int parameters on purpose: with the (s32, u8, s8) definition the calls would narrow their arguments. */
s32 field_queue_actor_event(s32 owner_id, s32 event_id, s32 argument);
s32 func_800BD3B0(s32 owner_id, s32 variable);
/* Declared without a prototype: func_800B2198 forwards its own a0. */
FieldActorRecord* func_800C1B98();
FieldActorRecord* func_800C1B60(u32 actor_id, FieldRuntimeContext* context);
void func_800C1D14(s32 actor_id, s32 flags);
void func_800C299C(s32 source);
void field_script_run(FieldScriptState* state);
s32 akao_cmd_c1(s32 arg0, s32 arg1, s32 arg2);

/**
 * @brief Install a conditional actor action and initialize its event scripts.
 * @param request Packed action definition, updated with activation and source state.
 * @param request_index Index in the load list; zero resets the action subsystem.
 * @note Action kind 3 uses the previous, uninitialized entry pointer before selecting
 * actor 1. This unresolved behavior is present in the original code.
 * @note Actor cases 2-4 are written as independent bodies; jump2 cross-jumps their
 * identical activation tails into the shared tail seen in the binary.
 * @note The script case keeps its first flag store in a do/while(0) block: GCC 2.8
 * sched2 would otherwise sink the store below the following flags copy.
 */
void field_install_actor_action(FieldActionRequest* request, s32 request_index)
{
    s32 action_index;
    FieldActionEntry* entry;
    s32 value;
    s32 source_actor;
    s32 interaction_id;
    s32 entry_flags;
    s32 script_index;
    s32 flags;
    s32 flags_to_set;
    u16 actor_count;
    u16 group_actor_count;
    u16 script_actor_count;
    u32 action_kind;
    FieldActionEntry* actor_entry;
    FieldActionEntry* group_entry;
    FieldActionEntry* script_entry;
    FieldActionEntry* event_entry;
    FieldActionEntry* flag_entry;

    if (request_index == 0)
    {
        func_800B0AF8();
    }
    value = func_800BD3B0(0, request->condition.variable << 0x10);
    if ((value >= (s32)request->condition.minimum) && ((s32)request->condition.maximum >= value))
    {
        action_kind = request->control.bytes.kind_flags >> FIELD_ACTION_KIND_SHIFT;
        /* Access packed entry flags as words so stores may alias the request data. */
        switch (action_kind)
        {
        case FIELD_ACTION_ACTOR:
            request->control.flags = request->control.flags | FIELD_ACTION_ACTIVE;
            actor_count = ((FieldActionTable*)g_field_runtime)->actors.count;
            ((FieldActionTable*)g_field_runtime)->actors.count = (u16)(actor_count + 1);
            actor_entry = &((FieldActionTable*)g_field_runtime)->entries[actor_count & 0xFFFF];
            entry = actor_entry;
            actor_entry->id = (s8)(request_index + FIELD_ACTION_DYNAMIC_ID_BASE);
            *(s32*)&actor_entry->flags = *(s32*)&actor_entry->flags | FIELD_ACTION_ACTIVE;
            *(s32*)&entry->flags = (*(s32*)&entry->flags & ~FIELD_ACTION_ENTRY_FLAG_MASK) | (request->control.flags & FIELD_ACTION_ENTRY_FLAG_MASK);
            request->control.flags = request->control.flags & ~FIELD_ACTION_ENTRY_FLAG_MASK;
            *(s32*)&entry->flags = (*(s32*)&entry->flags & ~FIELD_ACTION_SOURCE_MASK) | ((request->source.actor * 2) & FIELD_ACTION_SOURCE_MASK);
            source_actor = *(u16*)&request->source;
            action_index = 0;
            request->source.actor = source_actor & FIELD_ACTION_SOURCE_ACTOR_MASK;
            break;
        case FIELD_ACTION_GROUP_ACTOR:
            request->control.flags = request->control.flags | FIELD_ACTION_ACTIVE;
            group_actor_count = ((FieldActionTable*)g_field_runtime)->actors.count;
            ((FieldActionTable*)g_field_runtime)->actors.count = (u16)(group_actor_count + 1);
            group_entry = &((FieldActionTable*)g_field_runtime)->entries[group_actor_count & 0xFFFF];
            entry = group_entry;
            group_entry->id = (s8)(request_index + FIELD_ACTION_DYNAMIC_ID_BASE);
            *(s32*)&group_entry->flags = *(s32*)&group_entry->flags | FIELD_ACTION_ACTIVE;
            entry_flags = (*(s32*)&entry->flags & ~FIELD_ACTION_ENTRY_FLAG_MASK) | (request->control.flags & FIELD_ACTION_ENTRY_FLAG_MASK);
            *(s32*)&entry->flags = entry_flags;
            *(s32*)&entry->flags = (entry_flags & ~FIELD_ACTION_SOURCE_MASK) | ((request->source.actor * 2) & FIELD_ACTION_SOURCE_MASK);
            request->control.flags &= ~FIELD_ACTION_ENTRY_FLAG_MASK;
            action_index = 0;
            value = request->control.flags;
            if (!(((u32)value >> FIELD_ACTION_GROUP_SHIFT) & FIELD_ACTION_GROUP_MASK))
            {
                request->control.flags = (value & FIELD_ACTION_GROUP_CLEAR_MASK) |
                                         ((((((FieldActionLayout*)g_field_game_state)->default_group >> 4) + 1) & FIELD_ACTION_GROUP_MASK) << FIELD_ACTION_GROUP_SHIFT);
            }
            source_actor = request->source.actor;
            request->source.actor = source_actor & FIELD_ACTION_SOURCE_ACTOR_MASK;
            break;
        case FIELD_ACTION_SCRIPT:
            request->control.flags = request->control.flags & FIELD_ACTION_INACTIVE_MASK;
            script_actor_count = ((FieldActionTable*)g_field_runtime)->actors.count;
            ((FieldActionTable*)g_field_runtime)->actors.count = (u16)(script_actor_count + 1);
            script_entry = &((FieldActionTable*)g_field_runtime)->entries[script_actor_count & 0xFFFF];
            entry = script_entry;
            script_entry->id = (s8)(request_index + FIELD_ACTION_DYNAMIC_ID_BASE);
            action_index = 0;
            *(s32*)&script_entry->flags = *(s32*)&script_entry->flags | FIELD_ACTION_ACTIVE;
            flags_to_set = (*(s32*)&entry->flags & ~FIELD_ACTION_ENTRY_FLAG_MASK) | (request->control.flags & FIELD_ACTION_ENTRY_FLAG_MASK);
            /* Loop notes keep this store ahead of the flags copy below (sched2 tie). */
            do
            {
                *(s32*)&entry->flags = flags_to_set;
            } while (0);
            flag_entry = entry;
            flags = *(s32*)&flag_entry->flags;
            flags_to_set = FIELD_ACTION_SCRIPT_ONLY;
            *(s32*)&flag_entry->flags = flags | flags_to_set;
            break;
        case FIELD_ACTION_ACTOR_0:
            action_index = 0;
            request->control.flags = request->control.flags & FIELD_ACTION_INACTIVE_MASK;
            entry = &((FieldActionTable*)g_field_runtime)->entries[0];
            *(s32*)&entry->flags = *(s32*)&entry->flags | FIELD_ACTION_ACTIVE;
            break;
        case FIELD_ACTION_ACTOR_1:
            /* The original path activates the previous entry before selecting actor 1. */
            action_index = 0;
            request->control.flags = request->control.flags & FIELD_ACTION_INACTIVE_MASK;
            flag_entry = entry;
            entry = &((FieldActionTable*)g_field_runtime)->entries[1];
            *(s32*)&flag_entry->flags = *(s32*)&flag_entry->flags | FIELD_ACTION_ACTIVE;
            break;
        case FIELD_ACTION_ACTOR_2:
            action_index = 0;
            request->control.flags = request->control.flags & FIELD_ACTION_INACTIVE_MASK;
            entry = &((FieldActionTable*)g_field_runtime)->entries[2];
            *(s32*)&entry->flags = *(s32*)&entry->flags | FIELD_ACTION_ACTIVE;
            break;
        case FIELD_ACTION_EVENT:
            request->control.flags = request->control.flags & FIELD_ACTION_INACTIVE_MASK;
            if (request->control.bytes.selector < 2U)
            {
                func_800C0490(request->control.bytes.selector);
            }
            event_entry = &((FieldActionTable*)g_field_runtime)->event_entries[0];
            *(s32*)&event_entry->flags = *(s32*)&event_entry->flags | FIELD_ACTION_ACTIVE;
            ((FieldActionTable*)g_field_runtime)->actors.flags = ((FieldActionTable*)g_field_runtime)->actors.flags | FIELD_ACTION_EVENT_MODE;
            interaction_id = request->scripts[FIELD_ACTION_START_EVENT];
            action_index = 0;
            entry = event_entry;
            func_800B22F0(FIELD_ACTION_EVENT_OWNER, interaction_id);
            request->scripts[FIELD_ACTION_START_EVENT] = FIELD_ACTION_NO_SCRIPT;
            func_800B168C(3);
            break;
        case FIELD_ACTION_MENU:
            request->control.flags =
                (request->control.flags & FIELD_ACTION_INACTIVE_MASK) | (func_800B1894(request, &entry, request_index, &action_index) << 0x1F);
            break;
        }
        /* Install the event table and reserve this owner's local variables. */
        if (entry != NULL)
        {
            entry->selector = request->control.bytes.selector;
            entry->event = FIELD_ACTION_NO_EVENT;
            entry->enabled_events = request->enabled_events;
            for (script_index = 0; script_index < FIELD_ACTION_SCRIPT_COUNT; script_index++)
            {
                entry->scripts[script_index] = request->scripts[script_index];
            }
            entry->script_state = (entry->script_state & FIELD_SCRIPT_LOCAL_BASE_CLEAR_MASK) |
                                  ((((FieldActionTable*)g_field_runtime)->local_variable_base & FIELD_SCRIPT_LOCAL_BASE_MASK) << FIELD_SCRIPT_LOCAL_BASE_SHIFT);
            ((FieldActionTable*)g_field_runtime)->local_variable_base += request->control.bytes.local_variable_count;
            field_queue_actor_event((u8)entry->id, FIELD_ACTION_START_EVENT, (u8)action_index);
        }
    }
    else
    {
        request->control.flags = request->control.flags & FIELD_ACTION_INACTIVE_MASK;
    }
}

/**
 * @brief Apply a party control mode to the present party members.
 * @param mode 1 or 3 hands every member to AI control, 2 only the AI-controlled ones.
 */
void func_800B168C(s32 mode)
{
    s32 i;

    for (i = 0; i < FIELD_PARTY_SIZE; i++)
    {
        if (g_field_game_state->characters[i].name[0] != 0)
        {
            switch (mode)
            {
            case 1:
            case 3:
                field_set_actor_control_mode(i, 2);
                break;
            case 2:
                if ((g_field_game_state->characters[i].info.bytes[0] >> 7) != 0)
                {
                    field_set_actor_control_mode(i, 2);
                }
                break;
            }
        }
    }

    g_field_interaction_active = 1;
    g_field_runtime->state.bits.party_mode = mode;
}

/**
 * @brief Return party members from the pending party control mode and clear it.
 */
void func_800B177C(void)
{
    s32 i;
    s32 mode;

    for (i = 0; i < FIELD_PARTY_SIZE; i++)
    {
        mode = (g_field_runtime->state.flags >> 17) & 3;
        switch (mode)
        {
        case 1:
        case 3:
            if ((g_field_game_state->characters[i].info.bytes[0] >> 7) != 0)
            {
                field_set_actor_control_mode(i, 0);
                func_800C1D14(i, 0);
            }
            else
            {
                field_set_actor_control_mode(i, 1);
                func_800C1D14(i, 0);
            }
            break;
        case 2:
            if ((g_field_game_state->characters[i].info.bytes[0] >> 7) != 0)
            {
                field_set_actor_control_mode(i, 0);
                func_800C1D14(i, 0);
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
 * @param request_index Request index; the record id is request_index + 3.
 * @param action_index Receives the selected action index when the slot kind needs one.
 * @return -1 when a record is appended, or 0 when the slot is unused or empty.
 */
s32 func_800B1894(FieldActionRequest* request, FieldActionEntry** entry_out, s32 request_index, s32* action_index)
{
    FieldActionEntry* entry;
    u32 group;
    u32 handle;
    s32 count;
    s16 result_type;
    u8 slot;

    slot = request->control.bytes.selector;
    group = slot >> 7;
    slot &= 7;

    if (g_field_game_state->menu_slots[group].slots[slot].entry.index < 0xFF)
    {
        handle = g_field_game_state->menu_slots[group].slots[slot].handle;
        switch (handle)
        {
        case 0:
            *entry_out = NULL;
            return 0;

        case 1:
            *action_index = 0;
            request->source.result_type = 2;
            break;

        case 2:
            *action_index = g_field_game_state->menu_slots[group].slots[slot].entry.index - 0x30;
            result_type = (g_field_game_state->menu_slots[group].slots[slot].entry.word >> 8) & 3;
            request->source.result_type = result_type;
            break;

        case 3:
            *action_index = g_field_game_state->menu_slots[group].slots[slot].entry.index;
            result_type = (g_field_game_state->menu_slots[group].slots[slot].entry.word >> 8) & 3;
            request->source.result_type = result_type;
            break;

        default:
            break;
        }

        count = g_field_runtime->state.actor_count;
        g_field_runtime->state.actor_count = count + 1;
        entry = (FieldActionEntry*)&g_field_runtime->actors[count & 0xFFFF];
        *entry_out = entry;
        entry->flags.word |= 0x80000000;
        (*entry_out)->id = request_index + 3;
        (*entry_out)->flags.word = ((*entry_out)->flags.word & ~0xF) | (request->control.flags & 0xF);
        request->control.flags &= ~0xF;
        return -1;
    }

    *entry_out = NULL;
    return 0;
}

/**
 * @brief Run one frame of the field runtime: transitions, triggers, scripts and events.
 */
void func_800B19FC(void)
{
    s32 transition;

    transition = g_field_runtime->transition.flags;
    if (transition < 0)
    {
        func_800B1AA8();
        return;
    }
    if (((u32)transition >> 30) & 1)
    {
        func_800B1BBC();
    }
    func_800B1D10();
    func_800B1F10();
    func_800B20B4();
    if (g_field_runtime->state.flags & 0x10000)
    {
        func_800B49C0();
    }
    g_field_runtime->frame_count++;
}

/**
 * @brief Leave the field: request the pending game state or start the next scene.
 */
void func_800B1AA8(void)
{
    u16 scene_id;

    func_800BD520(0, 0xFE2, 0);
    scene_id = g_field_runtime->transition.fields.scene_id;
    switch (scene_id)
    {
    case 0xFFFE:
        g_pending_game_state = 4;
        g_field_runtime->scene_entry = 0xFFFF;
        g_layout_sub_mode = -1;
        g_layout_option = -1;
        return;
    case 0xFFFF:
        g_field_runtime->scene_entry = scene_id;
        g_pending_game_state = 1;
        g_layout_sub_mode = -1;
        g_layout_option = -1;
        if (func_800BD414(0, 0xFFF) != 0)
        {
            g_pending_game_state = 0;
            g_field_runtime->transition.fields.scene_id = 1;
            g_field_runtime->scene_entry = 0;
        }
        return;
    default:
        field_set_scene_parameters(g_field_runtime->transition.fields.scene_id, g_field_runtime->transition.fields.unk41A, g_field_runtime->transition.fields.unk41B & 0x1F,
                                   g_field_runtime->scene_entry, g_field_runtime->scene_argument1, g_field_runtime->scene_argument2);
        break;
    }
}

/**
 * @brief Advance the scene transition: start the fade on the first frame, then count it down.
 */
void func_800B1BBC(void)
{
    FieldRuntimeContext* context;
    u32 transition;
    s32 i;
    u32 flags;
    u16 scene_id;
    s32 timer;

    context = g_field_runtime;
    transition = context->transition.flags;
    if (!((transition >> 29) & 1))
    {
        for (i = 0; i < FIELD_PARTY_SIZE; i++)
        {
            field_set_actor_control_mode(i, 2);
        }

        flags = g_field_runtime->transition.flags | 0x20000000;
        g_field_runtime->transition.flags = flags;
        if (g_field_runtime->fade_timer != 0xFF)
        {
            scene_id = g_field_runtime->transition.fields.scene_id;
            if ((scene_id != 0xFFFE) && (scene_id != 0xFFFF))
            {
                field_seek_scene_resource(scene_id & 0x7FFF);
            }

            /* Int arguments on purpose: the original loads the whole fade_timer word. */
            ((void (*)(s32, s32, s32, s32))field_set_fade_target)(g_field_runtime->fade_color.bits.red, g_field_runtime->fade_color.bits.green, g_field_runtime->fade_color.bits.blue, g_field_runtime->fade_timer);

            if (g_field_runtime->transition.fields.scene_id == 0xFFFF)
            {
                timer = g_field_runtime->fade_timer << 2;
                g_layout_option = -1;
                akao_cmd_c1(0, timer, 0);
            }

            g_field_runtime->fade_timer++;
            return;
        }
        g_field_runtime->transition.flags = flags | 0x80000000;
        return;
    }

    if (context->fade_timer <= 0)
    {
        context->transition.flags = transition | 0x80000000;
    }
    g_field_runtime->fade_timer--;
}

/**
 * @brief Refresh the party map positions and start the first newly entered trigger region.
 */
void func_800B1D10(void)
{
    VECTOR positions[FIELD_PARTY_SIZE];
    s32* packed_position;
    VECTOR* position;
    s32 triggered;
    FieldMapPoint* point;
    FieldRuntimeContext* context;
    s32 region_bit;
    s32 index;
    s32 packed;
    s32 sentinel;
    FieldTriggerTable* table;

    index = 0;
    sentinel = -1;
    position = positions;
    g_field_runtime->view_x = -g_field_scene_state->camera_x;
    packed_position = g_field_runtime->actor_positions;
    g_field_runtime->view_z = -(g_field_scene_state->camera_y + g_field_scene_state->camera_z);
next_actor:
    packed = field_get_actor_position(index, (s32*)position);
    if (packed != sentinel)
    {
        packed = ((position->vx << 8) & 0xFFFF0000) | ((position->vz >> 8) & 0xFFFF);
    }
    /* The goto loop and this block reproduce the original register allocation. */
    do
    {
        do
        {
            do
            {
                *packed_position = packed;
            } while (0);
        } while (0);
    } while (0);
    position++;
    index += 1;
    packed_position++;
    if (index < FIELD_PARTY_SIZE)
    {
        goto next_actor;
    }
    context = g_field_runtime;
    point = &D_80042FC8;
    point->x = positions[0].vx >> 8;
    point->z = positions[0].vz >> 8;
    table = (FieldTriggerTable*)context->trigger_table;
    if (table != NULL)
    {
        region_bit = 1;
        triggered = context->triggered_regions;
        for (index = 0; index < FIELD_TRIGGERS->count; index++)
        {
            if (!(triggered & region_bit))
            {
                if ((D_80042FC8.x >= FIELD_TRIGGERS->regions[index].min_x) && (FIELD_TRIGGERS->regions[index].max_x >= D_80042FC8.x) &&
                    (D_80042FC8.z >= FIELD_TRIGGERS->regions[index].min_z) && (FIELD_TRIGGERS->regions[index].max_z >= D_80042FC8.z))
                {
                    g_field_runtime->triggered_regions |= region_bit;
                    if (FIELD_TRIGGERS->regions[index].command & 0x8000)
                    {
                        func_800B22F0(0, FIELD_TRIGGERS->regions[index].command);
                        return;
                    }
                    field_battle_start(FIELD_TRIGGERS->regions[index].command);
                    return;
                }
            }
            region_bit *= 2;
        }
    }
}

/**
 * @brief Run the field script, or deliver the pending talk start and end events to the actors.
 */
void func_800B1F10(void)
{
    s32 i;

    if (g_field_runtime->script.frames[g_field_runtime->script.depth].pc != NULL)
    {
        field_script_run(&g_field_runtime->script);
        return;
    }

    if (g_field_interaction_active != 0)
    {
        for (i = 0; i < (s32)g_field_runtime->state.actor_count; i++)
        {
            field_queue_actor_event(g_field_runtime->actors[i].id, 0xD, 0x82);
        }
        if (((((u32)g_field_runtime->transition.flags >> 30) & 1) == 0) && (func_800BD414(0, 0xFE2) == 0))
        {
            func_800B177C();
        }
    }
    else if ((g_field_runtime->state.flags & 0x80000) && (field_text_get_status(0) == -1))
    {
        for (i = 0; i < (s32)g_field_runtime->state.actor_count; i++)
        {
            field_queue_actor_event(g_field_runtime->actors[i].id, 0xD, 0x85);
        }
        g_field_runtime->state.flags &= 0xFFF7FFFF;
    }
}

/**
 * @brief Run the two event records' scripts and deliver their pending events.
 */
void func_800B20B4(void)
{
    s32 i;

    for (i = 0; i < FIELD_EVENT_RECORD_COUNT; i++)
    {
        if (g_field_runtime->events[i].script.frames[g_field_runtime->events[i].script.depth].pc != NULL)
        {
            field_script_run(&g_field_runtime->events[i].script);
        }
        field_run_actor_event(i + 0x80, g_field_runtime->events[i].event, g_field_runtime->events[i].event_argument);
        g_field_runtime->events[i].event = FIELD_NO_EVENT;
        field_run_actor_event(0x80, 0xE, 0);
        g_field_runtime->events[i].event = FIELD_NO_EVENT;
    }
}

/**
 * @brief Update a spawned actor: deliver its pending event, its on-screen event and its script.
 * @param actor_id Actor id; also passed through to func_800C1B98 in a0.
 * @param unused Unused.
 */
void func_800B2198(s32 actor_id, void* unused)
{
    FieldActorRecord* actor;
    VECTOR position;

    actor = func_800C1B98();
    if ((actor != NULL) && (actor->flags.word < 0))
    {
        if (actor->event != FIELD_NO_EVENT)
        {
            field_run_actor_event(actor->id, actor->event, actor->event_argument);
            actor->event = FIELD_NO_EVENT;
        }
        if (!(((u32)actor->flags.word >> 30) & 1))
        {
            field_get_actor_position(actor_id, (s32*)&position);
            if (((u32)position.vx > (u32)g_field_runtime->view_x) && ((u32)position.vz > (u32)g_field_runtime->view_z) &&
                ((u32)position.vx < (u32)g_field_runtime->view_x + 0x140) && ((u32)position.vz < (u32)g_field_runtime->view_z + 0x1C0))
            {
                field_run_actor_event(actor->id, 2, 0);
            }
            else
            {
                field_run_actor_event(actor->id, 3, 0);
            }
            field_run_actor_event(actor->id, 0xE, 0);
            if (actor->script.frames[actor->script.depth].pc == NULL)
            {
                field_run_actor_event(actor->id, 8, 0);
                return;
            }
            field_script_run(&actor->script);
        }
    }
}

/**
 * @brief Start an actor interaction through its script or the talk presentation.
 * @param actor_id Actor that is talked to.
 * @param script Script id; bit 15 runs it as the field script, otherwise it is a talk message.
 * @return -1 when the interaction starts, or 0 when it cannot start.
 */
s32 func_800B22F0(s32 actor_id, s32 script)
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
    if ((script_id & 0xFFFF) == 0xFFFF)
    {
        return 0;
    }

    if (((g_field_runtime->transition.flags >> 30) & 1) != 0)
    {
        return 0;
    }
    if (func_800BD414(0, 0xFE1) != 0)
    {
        return 0;
    }

    actor = func_800C1B98(actor_id);
    if (actor == NULL)
    {
        return 0;
    }

    flags = actor->flags.word;
    if ((flags >> 30) & 1)
    {
        return 0;
    }
    if ((flags >> 29) & 1)
    {
        return 0;
    }

    source = (flags >> 4) & 0x3FF;
    if (source != 0)
    {
        func_800C299C(source);
    }

    if ((script & 0x8000) != 0)
    {
        if (g_field_runtime->script.frames[g_field_runtime->script.depth].pc != NULL)
        {
            return 0;
        }

        for (i = 0; i < (s32)g_field_runtime->state.actor_count; i++)
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
                    field_queue_actor_event(actor_id, 0xD, 0x80);
                }
                else
                {
                    field_queue_actor_event(other_id, 0xD, 0x81);
                }
            }
        }

        g_field_interaction_active = 1;
        g_field_runtime->script.status.owner_id = actor_id;
        g_field_runtime->script.status.word = (g_field_runtime->script.status.word & 0xFFFF01FF) | (actor->script.status.word & 0xFE00);
        g_field_runtime->script.frames[g_field_runtime->script.depth].pc = field_get_event_script(script_id & 0x7FFF);
        g_field_runtime->script.frames[g_field_runtime->script.depth].wait.bits.resume = 0;
        g_field_runtime->script.frames[g_field_runtime->script.depth].wait.bits.frames = 0;
        return -1;
    }

    for (i = 0; i < (s32)g_field_runtime->state.actor_count; i++)
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
                field_queue_actor_event(actor_id, 0xD, 0x83);
            }
            else
            {
                field_queue_actor_event(other_id, 0xD, 0x84);
            }
        }
    }

    speaker = actor_id;
    plane = 0xFF;
    effect = 0xFE;
    selector = 0xFF;
    g_field_runtime->state.flags |= 0x80000;
    func_800B2654(&speaker, &plane, &effect, &selector);
    func_8009C620(plane, selector, speaker, effect);
    func_8009C77C(plane, script_id & 0xFFFF, 1);
    return -1;
}

/**
 * @brief Return an actor's selector, or -1 for the 0xFF sentinel.
 * @param actor Actor record.
 * @return Selector value, or -1.
 */
static inline s32 actor_selector(FieldActorRecord* actor)
{
    s32 value = -1;

    if (actor->selector != 0xFF)
    {
        value = actor->selector;
    }
    return value;
}

/**
 * @brief Resolve the talk presentation operands in place.
 * @param actor_id Speaking actor; invalid identifiers are replaced with zero.
 * @param plane Window plane, or 0xFF to choose it from the actor's screen position.
 * @param effect Window effect, 0xFE for the actor's selector or 0xFF for none; receives facing flags.
 * @param selector Window selector, or 0xFF for the current one.
 */
void func_800B2654(s32* actor_id, s32* plane, s32* effect, s32* selector)
{
    s32 position[3];
    s32 plane_value;
    s32 effect_value;
    s32 facing_flag;
    s32 selector_value;
    s32 facing_angle;
    u32 original_actor_id;

    original_actor_id = *actor_id;
    if (original_actor_id < 0x80U)
    {
        field_get_actor_position(original_actor_id, position);
    }
    else
    {
        *actor_id = 0;
    }
    plane_value = *plane;
    if (plane_value != 0xFF)
    {
        if (plane_value & 0x80)
        {
            facing_flag = 0;
        }
        else if (plane_value & 0x40)
        {
            facing_flag = 0x40;
        }
        else
        {
            facing_angle = field_get_actor_facing(*actor_id);
            facing_flag = ((facing_angle >= 0x41) && (facing_angle < 0xC1)) << 6;
        }
        *plane &= 3;
    }
    else
    {
        if ((position[2] - g_field_runtime->view_z) <= 0xBFFF)
        {
            *plane = 0;
        }
        else
        {
            *plane = 1;
        }
        facing_angle = field_get_actor_facing(*actor_id);
        facing_flag = ((facing_angle >= 0x41) && (facing_angle < 0xC1)) << 6;
    }
    g_field_runtime->unk41C = (g_field_runtime->unk41C & ~0x300) | ((*plane & 3) << 8);
    effect_value = *effect;
    switch (effect_value)
    {
    case 0xFE:
        *effect = actor_selector(func_800C1B60(original_actor_id, g_field_runtime));
        break;
    case 0xFF:
        *effect = -1;
        break;
    }
    *effect |= facing_flag;
    selector_value = *selector;
    if (selector_value == 0xFF)
    {
        selector_value = (u8)g_field_runtime->unk41C;
    }
    *selector = selector_value;
    if (!(((s32)D_800EF84C[selector_value] >> *plane) & 1))
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
void func_800B2844(s32 slot, u8* text, u8 character_limit)
{
    if (slot < 0x10)
    {
        g_field_text_macros[slot].character_limit = character_limit;
        g_field_text_macros[slot].text = text;
    }
}
