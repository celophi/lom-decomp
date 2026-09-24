#include "field_scene_transition.h"
#include "field_text.h"
#include "game_audio.h"
#include "saved_game.h"
#include "common.h"
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

/** @brief Region trigger record: bounds and the command started on entry (12 bytes). */
typedef struct
{
    u16 unk0;
    u16 unk2;
    u16 min_x;
    u16 min_z;
    u16 max_x;
    u16 max_z;
    u16 command;
    u16 unkE;
} FieldTriggerRegion;

/** @brief Player map position in whole units. */
typedef struct
{
    u16 x;
    u16 z;
} FieldMapPoint;

extern FieldGameState* D_80122B74;
extern FieldRuntimeContext* D_80122B78;
extern FieldCamera* D_80122B70;
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
s32 func_800C3688(s32 track);
s32 rand(void);

/**
 * @brief Initialize the field runtime context for a new scene.
 * @note Reports a diagnostic when a present partner's script variable is still unset.
 */
void func_800B0AF8(void)
{
    func_800B0BDC();
    func_800C1EC8(NULL, (s32*)&D_80122B78->state, 0xB04);
    func_800B0C10();
    func_800B0C54();
    func_800B0D3C();
    func_800B0E80();
    func_800B0EFC();

    if (D_80122B74->characters[1].name[0] != 0)
    {
        if (func_800BD414(0, 0x2F08) == 0xFF)
        {
            record_game_diagnostic(0x8001, 0x320, 1, 0);
        }
    }

    if (D_80122B74->characters[2].name[0] != 0)
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
    D_80122B74 = (FieldGameState*)&g_saved_game;
    D_80122B78 = &D_80122C00;
    D_80122B70 = (FieldCamera*)0x801ED480;
}

/** @brief Reset the pending scene entry and the fade parameters. */
void func_800B0C10(void)
{
    FieldRuntimeContext* context;

    context = D_80122B78;
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

    slot = 0;
    do
    {
        g_field_text_macros[15 - slot].character_limit = 0x15;
        g_field_text_macros[15 - slot].text = D_80122B74->characters[slot].name;
        slot++;
    } while (slot < FIELD_PARTY_SIZE);

    g_field_text_macros[12].character_limit = 0xFF;
    g_field_text_macros[12].text = (u8*)D_800EF600 + *(s16*)((u8*)D_800EF600 + ((D_80122B74->control.fields.unk2E6 & 0x7F) * 2));

    if ((func_800BD414(0, 0xA02) != 0) || ((D_80122B74->characters[1].info.word & 0x80) != 0))
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
 * @note Still the raw decompiler form: the natural typed loops reach 95.4% (register
 *       coloring and the second loop's increment order); see the lane report.
 */
void func_800B0D3C(void)
{
    s32 temp_v1;
    s32 current;
    u16 invalid;
    s32 fixed0;
    s32 fixed1;
    s32 fixed2;
    s32 masked1;
    s32 masked2;
    u32 fixed_mask;
    u32 loop_mask;
    u32 mask_d;
    s32 var_a0;
    s32 var_a2;
    s32 var_a3;
    s32 var_t1_2;
    s32 var_t1;
    u8* temp_a0;
    u8* var_t0;
    u8* base;

    var_t1 = 0;
    do
    {
        var_a2 = var_t1 * 0x94;
        temp_a0 = (u8*)D_80122B78;
        temp_a0 += 1;
        temp_a0 -= 1;
        temp_a0 += var_a2;
        var_a3 = 0;
        var_a3 += 1;
        var_a3 += 1;
        var_a3 -= 2;
        (*(u8*)((u8*)temp_a0 + 0x430)) = var_t1;
        (*(s32*)((u8*)temp_a0 + 0x4C0)) |= 0x80000000;
        invalid = 0xFFFF;
        (*(s32*)((u8*)temp_a0 + 0x4C0)) &= 0xBFFFFFFF;
        mask_d = 0xDFFFFFFF;
        mask_d += (u32)temp_a0;
        mask_d -= (u32)temp_a0;
        (*(s32*)((u8*)temp_a0 + 0x4C0)) &= mask_d;
        var_a0 = var_a2;
        (*(u8*)((u8*)(u8*)D_80122B78 + var_a2 + 0x431)) = (s8)(var_t1 - 0x80);
        (*(u8*)((u8*)(u8*)D_80122B78 + var_a2 + 0x434)) = 0xFF;
        do
        {
            do
            {
                do
                {
                    do
                    {
                        do
                        {
                            do
                            {
                                base = (u8*)D_80122B78;
                            } while (0);
                        } while (0);
                    } while (0);
                } while (0);
            } while (0);
        } while (0);
    loop_2:
        (*(u16*)(base + var_a0 + 0x438)) = invalid;
        var_a3 += 1;
        var_a0 += 2;
        if (var_a3 < 0x10)
        {
            goto loop_2;
        }
        var_t1 += 1;
        (*(u16*)((u8*)(u8*)D_80122B78 + 0x400)) = (u16)((*(u16*)((u8*)(u8*)D_80122B78 + 0x400)) + 1);
    } while (var_t1 < 3);
    fixed_mask = 0xFFFF01FF;
    var_t1_2 = 3;
    var_a3 = 0xFF;
    var_t0 = (u8*)D_80122B78 + 0x1BC;
    fixed0 = *(s32*)((u8*)D_80122B78 + 0x458);
    fixed1 = *(s32*)((u8*)D_80122B78 + 0x4EC);
    fixed2 = *(s32*)((u8*)D_80122B78 + 0x580);
    fixed0 &= fixed_mask;
    fixed0 |= 0x6000;
    masked1 = fixed1 & fixed_mask;
    masked1 |= 0x6200;
    masked2 = fixed2 & fixed_mask;
    masked2 |= 0x7000;
    *(s32*)((u8*)D_80122B78 + 0x458) = fixed0;
    *(s32*)((u8*)D_80122B78 + 0x4EC) = masked1;
    *(s32*)((u8*)D_80122B78 + 0x580) = masked2;
    loop_mask = 0xFFFF01FF;
loop_3:
    var_t1_2 += 1;
    temp_v1 = var_a3 & 0x7F;
    var_a3 -= 1;
    current = *(s32*)((u8*)var_t0 + 0x458);
    current &= loop_mask;
    current |= temp_v1 << 9;
    (*(s32*)((u8*)var_t0 + 0x458)) = current;
    var_t0 += 0x94;
    if (var_t1_2 < 0x10)
    {
        goto loop_3;
    }
    (*(s32*)((u8*)(u8*)D_80122B78 + 0x0)) = 0x40;
}

/**
 * @brief Initialize the two event records.
 * @see decomp.me (100%) TODO
 */
void func_800B0E80(void)
{
    s32 i;
    s32 j;

    i = 0;
    do
    {
        D_80122B78->events[i].id = i - 0x80;
        D_80122B78->events[i].selector = 0xFF;
        D_80122B78->actors[i].event = FIELD_NO_EVENT;
        for (j = 0; j < FIELD_ACTOR_SCRIPT_COUNT; j++)
        {
            D_80122B78->events[i].scripts[j] = FIELD_NO_SCRIPT;
        }
        i++;
    } while (i < FIELD_EVENT_RECORD_COUNT);
}

/**
 * @brief Initialize the field script variables derived from the game state and music track.
 */
void func_800B0EFC(void)
{
    s32 flags;

    flags = D_80122B74->control.word;
    if (flags & 0x800000)
    {
        D_80122B74->control.word = flags & 0xFF7FFFFF;
        func_800C1EC8(NULL, D_80122B74->words, 0x20);
        func_800BD520(0, 0xFA, rand() & 0xFF);
    }

    if (D_80122B74->control.fields.hero_level >= 0x12)
    {
        func_800BD520(0, 0xA00, 1);
    }

    func_800BD520(0, 0x429C, D_80122B74->control.fields.unk2E6 & 0x7F);
    func_800BD520(0, 0x4300, D_800F0B48[D_80122B74->lands[g_music_track_index].levels[0]]);
    func_800BD520(0, 0x4304, D_800F0B48[D_80122B74->lands[g_music_track_index].levels[1]]);
    func_800BD520(0, 0x4308, D_800F0B48[D_80122B74->lands[g_music_track_index].levels[2]]);
    func_800BD520(0, 0x430C, D_800F0B48[D_80122B74->lands[g_music_track_index].levels[3]]);
    func_800BD520(0, 0x4310, D_800F0B48[D_80122B74->lands[g_music_track_index].levels[4]]);
    func_800BD520(0, 0x4314, D_800F0B48[D_80122B74->lands[g_music_track_index].levels[5]]);
    func_800BD520(0, 0x4318, D_800F0B48[D_80122B74->lands[g_music_track_index].levels[6]]);
    func_800BD520(0, 0x431C, D_800F0B48[D_80122B74->lands[g_music_track_index].levels[7]]);
    func_800BD520(0, 0x5320, func_800C3688(g_music_track_index));
    func_800BD520(0, 0x5328, g_music_track_index);
}

extern u8 D_800EF84C[];
extern s32 D_8010AE78;
extern s32 g_pending_game_state;
extern s32 g_layout_sub_mode;
extern s32 g_layout_option;
extern FieldMapPoint D_80042FC8;

void func_80087CE0(s32 actor_id, s32 mode);
u8* func_80087EF0(s32 script_id);
s32 func_80087F44(s32 actor_id, s32* position);
void func_80087FC0(s32 party_index, s32 mode);
s32 func_8008B288(s32 actor_id);
void func_8009C620(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_8009C77C(s32 arg0, s32 arg1, s32 arg2);
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
s32 func_800B286C(s32 owner_id, s32 event_id, s32 argument);
void func_800B28E0(s32 owner_id, s32 event_id, s32 mode);
void func_800B4410(u16 group);
void func_800B49C0(void);
s32 func_800BD3B0(s32 owner_id, s32 variable);
void func_800C0490(u8 selector);
/* Declared without a prototype: func_800B2198 forwards its own a0. */
FieldActorRecord* func_800C1B98();
FieldActorRecord* func_800C1B60(u32 actor_id, FieldRuntimeContext* context);
void func_800C1D14(s32 actor_id, s32 flags);
void func_800C299C(s32 source);
void field_script_run(FieldScriptState* state);
s32 akao_cmd_c1(s32 arg0, s32 arg1, s32 arg2);
void field_set_fade_target(s32 red, s32 green, s32 blue, s32 frames);

/**
 * @brief Install a conditional actor action and initialize its event scripts.
 * @param request Packed action definition, updated with activation and source state.
 * @param request_index Index in the load list; zero resets the action subsystem.
 * @note Action kind 3 uses the previous, uninitialized entry pointer before selecting
 * actor 1. This unresolved behavior is present in the original code.
 * @note Packed-word accesses and shared switch tails preserve the current reconstruction.
 * @see decomp.me (99.765625%) TODO: no scratch link yet
 */
void field_install_actor_action(FieldActionRequest* request, s32 request_index)
{
    s32 masked_request_flags;
    s32 flags_mask;
    s32 action_index;
    FieldActionEntry* entry;
    s32 value;
    s32 source_actor;
    s32 request_flags;
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
            actor_count = ((FieldActionTable*)D_80122B78)->actors.count;
            ((FieldActionTable*)D_80122B78)->actors.count = (u16)(actor_count + 1);
            actor_entry = &((FieldActionTable*)D_80122B78)->entries[actor_count & 0xFFFF];
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
            group_actor_count = ((FieldActionTable*)D_80122B78)->actors.count;
            ((FieldActionTable*)D_80122B78)->actors.count = (u16)(group_actor_count + 1);
            group_entry = &((FieldActionTable*)D_80122B78)->entries[group_actor_count & 0xFFFF];
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
                                         ((((((FieldActionLayout*)D_80122B74)->default_group >> 4) + 1) & FIELD_ACTION_GROUP_MASK) << FIELD_ACTION_GROUP_SHIFT);
            }
            source_actor = request->source.actor;
            request->source.actor = source_actor & FIELD_ACTION_SOURCE_ACTOR_MASK;
            break;
        case FIELD_ACTION_SCRIPT:
            request->control.flags = request->control.flags & FIELD_ACTION_INACTIVE_MASK;
            script_actor_count = ((FieldActionTable*)D_80122B78)->actors.count;
            ((FieldActionTable*)D_80122B78)->actors.count = (u16)(script_actor_count + 1);
            script_entry = &((FieldActionTable*)D_80122B78)->entries[script_actor_count & 0xFFFF];
            entry = script_entry;
            script_entry->id = (s8)(request_index + FIELD_ACTION_DYNAMIC_ID_BASE);
            action_index = 0;
            *(s32*)&script_entry->flags = *(s32*)&script_entry->flags | FIELD_ACTION_ACTIVE;
            flags_mask = ~FIELD_ACTION_ENTRY_FLAG_MASK;
            flag_entry = entry;
            flags_to_set = (*(s32*)&flag_entry->flags & flags_mask) | (request->control.flags & FIELD_ACTION_ENTRY_FLAG_MASK);
            do
            {
                *(s32*)&flag_entry->flags = flags_to_set;
            } while (0);
            flags = flags_to_set;
            flags_to_set = FIELD_ACTION_SCRIPT_ONLY;
            goto set_entry_flags;
        case FIELD_ACTION_ACTOR_0:
            action_index = 0;
            flags_mask = FIELD_ACTION_INACTIVE_MASK;
            masked_request_flags = request->control.flags & flags_mask;
            flag_entry = &((FieldActionTable*)D_80122B78)->entries[0];
            request->control.flags = masked_request_flags;
            entry = flag_entry;
            flags = *(s32*)&flag_entry->flags;
            flags_to_set = FIELD_ACTION_ACTIVE;
            *(s32*)&flag_entry->flags = flags | flags_to_set;
            break;
        case FIELD_ACTION_ACTOR_1:
            /* The original path reads entry before assigning actor 1. */
            request_flags = *(s32*)&request->control;
            action_index = 0;
            *(s32*)&request->control = request_flags & FIELD_ACTION_INACTIVE_MASK;
            flag_entry = entry;
            entry = &((FieldActionTable*)D_80122B78)->entries[1];
            goto activate_entry;
        case FIELD_ACTION_ACTOR_2:
            action_index = 0;
            flags_mask = FIELD_ACTION_INACTIVE_MASK;
            masked_request_flags = request->control.flags & flags_mask;
            flag_entry = &((FieldActionTable*)D_80122B78)->entries[2];
            request->control.flags = masked_request_flags;
            entry = flag_entry;
        activate_entry:
            flags = *(s32*)&flag_entry->flags;
            flags_to_set = FIELD_ACTION_ACTIVE;
        set_entry_flags:
            *(s32*)&flag_entry->flags = flags | flags_to_set;
            break;
        case FIELD_ACTION_EVENT:
            request->control.flags = request->control.flags & FIELD_ACTION_INACTIVE_MASK;
            if (request->control.bytes.selector < 2U)
            {
                func_800C0490(request->control.bytes.selector);
            }
            event_entry = &((FieldActionTable*)D_80122B78)->event_entries[0];
            *(s32*)&event_entry->flags = *(s32*)&event_entry->flags | FIELD_ACTION_ACTIVE;
            ((FieldActionTable*)D_80122B78)->actors.flags = ((FieldActionTable*)D_80122B78)->actors.flags | FIELD_ACTION_EVENT_MODE;
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
                                  ((((FieldActionTable*)D_80122B78)->local_variable_base & FIELD_SCRIPT_LOCAL_BASE_MASK) << FIELD_SCRIPT_LOCAL_BASE_SHIFT);
            ((FieldActionTable*)D_80122B78)->local_variable_base += request->control.bytes.local_variable_count;
            func_800B286C((u8)entry->id, FIELD_ACTION_START_EVENT, (u8)action_index);
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

    i = 0;
    do
    {
        if (D_80122B74->characters[i].name[0] != 0)
        {
            switch (mode)
            {
            case 1:
            case 3:
                func_80087FC0(i, 2);
                break;
            case 2:
                if ((D_80122B74->characters[i].info.bytes[0] >> 7) != 0)
                {
                    func_80087FC0(i, 2);
                }
                break;
            }
        }
        i++;
    } while (i < FIELD_PARTY_SIZE);

    D_8010AE78 = 1;
    D_80122B78->state.bits.party_mode = mode;
}

/**
 * @brief Return party members from the pending party control mode and clear it.
 */
void func_800B177C(void)
{
    s32 i;
    s32 mode;

    i = 0;
    do
    {
        mode = (D_80122B78->state.flags >> 17) & 3;
        switch (mode)
        {
        case 1:
        case 3:
            if ((D_80122B74->characters[i].info.bytes[0] >> 7) != 0)
            {
                func_80087FC0(i, 0);
                func_800C1D14(i, 0);
            }
            else
            {
                func_80087FC0(i, 1);
                func_800C1D14(i, 0);
            }
            break;
        case 2:
            if ((D_80122B74->characters[i].info.bytes[0] >> 7) != 0)
            {
                func_80087FC0(i, 0);
                func_800C1D14(i, 0);
            }
            break;
        }
        i++;
    } while (i < FIELD_PARTY_SIZE);

    D_80122B78->state.bits.party_mode = 0;
    D_8010AE78 = 0;
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

    if (D_80122B74->menu_slots[group].slots[slot].entry.index < 0xFF)
    {
        handle = D_80122B74->menu_slots[group].slots[slot].handle;
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
            *action_index = D_80122B74->menu_slots[group].slots[slot].entry.index - 0x30;
            result_type = (D_80122B74->menu_slots[group].slots[slot].entry.word >> 8) & 3;
            request->source.result_type = result_type;
            break;

        case 3:
            *action_index = D_80122B74->menu_slots[group].slots[slot].entry.index;
            result_type = (D_80122B74->menu_slots[group].slots[slot].entry.word >> 8) & 3;
            request->source.result_type = result_type;
            break;

        default:
            break;
        }

        count = D_80122B78->state.actor_count;
        D_80122B78->state.actor_count = count + 1;
        entry = (FieldActionEntry*)&D_80122B78->actors[count & 0xFFFF];
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

    transition = D_80122B78->transition.flags;
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
    if (D_80122B78->state.flags & 0x10000)
    {
        func_800B49C0();
    }
    D_80122B78->frame_count++;
}

/**
 * @brief Leave the field: request the pending game state or start the next scene.
 */
void func_800B1AA8(void)
{
    u16 scene_id;

    func_800BD520(0, 0xFE2, 0);
    scene_id = D_80122B78->transition.fields.scene_id;
    switch (scene_id)
    {
    case 0xFFFE:
        g_pending_game_state = 4;
        D_80122B78->scene_entry = 0xFFFF;
        g_layout_sub_mode = -1;
        g_layout_option = -1;
        return;
    case 0xFFFF:
        D_80122B78->scene_entry = scene_id;
        g_pending_game_state = 1;
        g_layout_sub_mode = -1;
        g_layout_option = -1;
        if (func_800BD414(0, 0xFFF) != 0)
        {
            g_pending_game_state = 0;
            D_80122B78->transition.fields.scene_id = 1;
            D_80122B78->scene_entry = 0;
        }
        return;
    default:
        field_set_scene_parameters(D_80122B78->transition.fields.scene_id, D_80122B78->transition.fields.unk41A, D_80122B78->transition.fields.unk41B & 0x1F,
                                   D_80122B78->scene_entry, D_80122B78->scene_argument1, D_80122B78->scene_argument2);
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

    context = D_80122B78;
    transition = context->transition.flags;
    if (!((transition >> 29) & 1))
    {
        for (i = 0; i < FIELD_PARTY_SIZE; i++)
        {
            func_80087FC0(i, 2);
        }

        flags = D_80122B78->transition.flags | 0x20000000;
        D_80122B78->transition.flags = flags;
        if (D_80122B78->fade_timer != 0xFF)
        {
            scene_id = D_80122B78->transition.fields.scene_id;
            if ((u16)(scene_id + 2) >= 2)
            {
                field_seek_scene_resource(scene_id & 0x7FFF);
            }

            field_set_fade_target(D_80122B78->fade_color.bits.red, D_80122B78->fade_color.bits.green, D_80122B78->fade_color.bits.blue, D_80122B78->fade_timer);

            if (D_80122B78->transition.fields.scene_id == 0xFFFF)
            {
                timer = D_80122B78->fade_timer << 2;
                g_layout_option = -1;
                akao_cmd_c1(0, timer, 0);
            }

            D_80122B78->fade_timer++;
            return;
        }
        D_80122B78->transition.flags = flags | 0x80000000;
        return;
    }

    if (context->fade_timer <= 0)
    {
        context->transition.flags = transition | 0x80000000;
    }
    D_80122B78->fade_timer--;
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
    u8* table;
    FieldTriggerRegion* region;
    FieldTriggerRegion* trigger;

    index = 0;
    sentinel = -1;
    position = positions;
    D_80122B78->view_x = -D_80122B70->x;
    packed_position = D_80122B78->actor_positions;
    D_80122B78->view_z = -(D_80122B70->y + D_80122B70->z);
next_actor:
    packed = func_80087F44(index, (s32*)position);
    if (packed != sentinel)
    {
        packed = ((position->vx << 8) & 0xFFFF0000) | ((position->vz >> 8) & 0xFFFF);
    }
    /* Kept: this block and the goto loop above are required by the original allocation. */
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
    context = D_80122B78;
    point = &D_80042FC8;
    point->x = positions[0].vx >> 8;
    point->z = positions[0].vz >> 8;
    table = context->trigger_table;
    if (table != NULL)
    {
        region_bit = 1;
        triggered = context->triggered_regions;
        index = 0;
        while (index < (s32)((FieldTriggerRegion*)D_80122B78->trigger_table)->unk2)
        {
            if (!(triggered & region_bit))
            {
                region = (FieldTriggerRegion*)(*(u8* volatile*)&D_80122B78->trigger_table + (index * 12));
                if ((D_80042FC8.x >= region->min_x) && (region->max_x >= D_80042FC8.x) && (D_80042FC8.z >= region->min_z) && (region->max_z >= D_80042FC8.z))
                {
                    trigger = (FieldTriggerRegion*)(*(u8* volatile*)&D_80122B78->trigger_table + (index * 12));
                    D_80122B78->triggered_regions |= region_bit;
                    if (trigger->command & 0x8000)
                    {
                        func_800B22F0(0, trigger->command);
                        return;
                    }
                    func_800B4410(trigger->command);
                    return;
                }
            }
            region_bit *= 2;
            index += 1;
        }
    }
}

/**
 * @brief Run the field script, or deliver the pending talk start and end events to the actors.
 */
void func_800B1F10(void)
{
    s32 i;

    if (D_80122B78->script.frames[D_80122B78->script.depth].pc != NULL)
    {
        field_script_run(&D_80122B78->script);
        return;
    }

    if (D_8010AE78 != 0)
    {
        i = 0;
        if (D_80122B78->state.actor_count != 0)
        {
            do
            {
                func_800B286C(D_80122B78->actors[i].id, 0xD, 0x82);
                i++;
            } while (i < (s32)D_80122B78->state.actor_count);
        }
        if (((((u32)D_80122B78->transition.flags >> 30) & 1) == 0) && (func_800BD414(0, 0xFE2) == 0))
        {
            func_800B177C();
        }
    }
    else if ((D_80122B78->state.flags & 0x80000) && (field_text_get_status(0) == -1))
    {
        i = 0;
        if (D_80122B78->state.actor_count != 0)
        {
            do
            {
                func_800B286C(D_80122B78->actors[i].id, 0xD, 0x85);
                i++;
            } while (i < (s32)D_80122B78->state.actor_count);
        }
        D_80122B78->state.flags &= 0xFFF7FFFF;
    }
}

/**
 * @brief Run the two event records' scripts and deliver their pending events.
 */
void func_800B20B4(void)
{
    s32 i;

    i = 0;
    do
    {
        if (D_80122B78->events[i].script.frames[D_80122B78->events[i].script.depth].pc != NULL)
        {
            field_script_run(&D_80122B78->events[i].script);
        }
        func_800B28E0(i + 0x80, D_80122B78->events[i].event, D_80122B78->events[i].event_argument);
        D_80122B78->events[i].event = FIELD_NO_EVENT;
        func_800B28E0(0x80, 0xE, 0);
        D_80122B78->events[i].event = FIELD_NO_EVENT;
        i++;
    } while (i < FIELD_EVENT_RECORD_COUNT);
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
            func_800B28E0(actor->id, actor->event, actor->event_argument);
            actor->event = FIELD_NO_EVENT;
        }
        if (!(((u32)actor->flags.word >> 30) & 1))
        {
            func_80087F44(actor_id, (s32*)&position);
            if (((u32)position.vx > (u32)D_80122B78->view_x) && ((u32)position.vz > (u32)D_80122B78->view_z) &&
                ((u32)position.vx < (u32)D_80122B78->view_x + 0x140) && ((u32)position.vz < (u32)D_80122B78->view_z + 0x1C0))
            {
                func_800B28E0(actor->id, 2, 0);
            }
            else
            {
                func_800B28E0(actor->id, 3, 0);
            }
            func_800B28E0(actor->id, 0xE, 0);
            if (actor->script.frames[actor->script.depth].pc == NULL)
            {
                func_800B28E0(actor->id, 8, 0);
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

    if (((D_80122B78->transition.flags >> 30) & 1) != 0)
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
        if (D_80122B78->script.frames[D_80122B78->script.depth].pc != NULL)
        {
            return 0;
        }

        i = 0;
        if (D_80122B78->state.actor_count != 0)
        {
            do
            {
                if (i < FIELD_PARTY_SIZE)
                {
                    func_80087CE0(i, 0);
                }
                else
                {
                    other_id = D_80122B78->actors[i].id;
                    if (other_id == actor_id)
                    {
                        func_800B286C(actor_id, 0xD, 0x80);
                    }
                    else
                    {
                        func_800B286C(other_id, 0xD, 0x81);
                    }
                }
                i++;
            } while (i < (s32)D_80122B78->state.actor_count);
        }

        D_8010AE78 = 1;
        D_80122B78->script.status.owner_id = actor_id;
        D_80122B78->script.status.word = (D_80122B78->script.status.word & 0xFFFF01FF) | (actor->script.status.word & 0xFE00);
        D_80122B78->script.frames[D_80122B78->script.depth].pc = func_80087EF0(script_id & 0x7FFF);
        D_80122B78->script.frames[D_80122B78->script.depth].wait.bits.resume = 0;
        D_80122B78->script.frames[D_80122B78->script.depth].wait.bits.frames = 0;
        return -1;
    }

    i = 0;
    if (D_80122B78->state.actor_count != 0)
    {
        do
        {
            if (i < FIELD_PARTY_SIZE)
            {
                func_80087CE0(i, 0);
            }
            else
            {
                other_id = D_80122B78->actors[i].id;
                if (other_id == actor_id)
                {
                    func_800B286C(actor_id, 0xD, 0x83);
                }
                else
                {
                    func_800B286C(other_id, 0xD, 0x84);
                }
            }
            i++;
        } while (i < (s32)D_80122B78->state.actor_count);
    }

    speaker = actor_id;
    plane = 0xFF;
    effect = 0xFE;
    selector = 0xFF;
    D_80122B78->state.flags |= 0x80000;
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
    u32 original_actor_id;

    original_actor_id = *actor_id;
    if (original_actor_id < 0x80U)
    {
        func_80087F44(original_actor_id, position);
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
            facing_flag = ((u32)(func_8008B288(*actor_id) - 0x41) < 0x80U) << 6;
        }
        *plane &= 3;
    }
    else
    {
        if ((position[2] - D_80122B78->view_z) <= 0xBFFF)
        {
            *plane = 0;
        }
        else
        {
            *plane = 1;
        }
        facing_flag = ((u32)(func_8008B288(*actor_id) - 0x41) < 0x80U) << 6;
    }
    D_80122B78->unk41C = (D_80122B78->unk41C & ~0x300) | ((*plane & 3) << 8);
    effect_value = *effect;
    switch (effect_value)
    {
    case 0xFE:
        *effect = actor_selector(func_800C1B60(original_actor_id, D_80122B78));
        break;
    case 0xFF:
        *effect = -1;
        break;
    }
    *effect |= facing_flag;
    selector_value = *selector;
    if (selector_value == 0xFF)
    {
        selector_value = (u8)D_80122B78->unk41C;
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
