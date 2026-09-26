#include "game_audio.h"
#include "common.h"
#include "display.h"
#include "field_calls.h"
#include "field_actor_runtime.h"
#include "field_actor_tables.h"
#include "field_records.h"
#include "field_script.h"
#include "scene_state.h"
#include "vector.h"

/*
 * Script commands.
 *
 * Script opcode 0x03 runs command n of g_field_script_commands through
 * field_script_command. A command takes its arguments from, and writes its
 * results to, a parameter block in the field runtime context that the script
 * fills through its variables: two halfwords followed by eight words. Each
 * command reads the block through its own view type. An actor id argument of
 * FIELD_SCRIPT_OWNER stands for the script's owner.
 */

/** @brief Number of entries in g_field_script_commands. */
#define FIELD_SCRIPT_COMMAND_COUNT 17

/** @brief record_game_diagnostic code for a command number past the table. */
#define DIAG_BAD_SCRIPT_COMMAND 3

/** @brief field_command_actor_status operations. */
enum
{
    FIELD_ACTOR_STATUS_SET_POSITION = 0,
    FIELD_ACTOR_STATUS_SET_HP = 1,
    FIELD_ACTOR_STATUS_SET_FLAGS = 2,
    FIELD_ACTOR_STATUS_MASK_FLAGS = 3,
    FIELD_ACTOR_STATUS_GET_GAUGES = 4,
    FIELD_ACTOR_STATUS_GET_HP = 5,
    FIELD_ACTOR_STATUS_SET_DROPS = 6,
    FIELD_ACTOR_STATUS_SET_GAUGE = 7
};

/** @brief HP fraction steps returned by FIELD_ACTOR_STATUS_GET_HP. */
#define FIELD_HP_EIGHTHS 8

/** @brief field_command_test_area modes. */
enum
{
    FIELD_AREA_RECT = 0,
    FIELD_AREA_CENTERED = 1,
    FIELD_AREA_ACTOR = 2
};

/** @brief Resource holding the nibble table read by field_command_read_nibble_table. */
#define FIELD_RESOURCE_NIBBLE_TABLE 0x13

/** @brief Column id of the first column of the nibble table. */
#define FIELD_NIBBLE_TABLE_FIRST_COLUMN 0x60

/** @brief Columns per row of the nibble table. */
#define FIELD_NIBBLE_TABLE_COLUMNS 48

/** @brief Part argument of field_command_get_part_position that selects the object itself. */
#define FIELD_NO_PART 0xFF

/** @brief field_command_items operations. */
enum
{
    FIELD_ITEM_COMMAND_RESET = 0,
    FIELD_ITEM_COMMAND_CREATE = 1,
    FIELD_ITEM_COMMAND_TEMPER = 2,
    FIELD_ITEM_COMMAND_LOWER_LEVEL = 3,
    FIELD_ITEM_COMMAND_RAISE_LEVEL = 4,
    FIELD_ITEM_COMMAND_TEST_COST = 5,
    FIELD_ITEM_COMMAND_REPLACE_SLOT = 6,
    /** @brief Has a case of its own in the dispatch but reports a bad operation. */
    FIELD_ITEM_COMMAND_RESERVED = 7
};

/** @brief field_command_items result codes. */
#define FIELD_ITEM_COMMAND_NO_FREE_RECORD 0xFA
#define FIELD_ITEM_COMMAND_NO_ITEM 0xFB
#define FIELD_ITEM_COMMAND_NO_STOCK 0xFD
#define FIELD_ITEM_COMMAND_BAD_SUBTYPE 0xFE
#define FIELD_ITEM_COMMAND_BAD_OPERATION 0xFF

/** @brief Item subtypes with a generation table entry. */
#define FIELD_ITEM_SUBTYPE_COUNT 64

/** @brief field_command_query_records query bits: handler index, and "take the last entry". */
#define FIELD_RECORD_QUERY_INDEX_MASK 0x7FFF
#define FIELD_RECORD_QUERY_LAST 0x8000

/** @brief field_command_query_records result when the query matched nothing. */
#define FIELD_RECORD_QUERY_NONE 0xFFFF

/** @brief Entries a record query list has room for. */
#define FIELD_RECORD_QUERY_CAPACITY 16

/** @brief field_command_scroll_camera modes. */
enum
{
    FIELD_SCROLL_SAVE = 0,
    FIELD_SCROLL_TO = 1,
    FIELD_SCROLL_TO_ACTOR = 2,
    FIELD_SCROLL_RESTORE = 3
};

/** @brief Size of the scrolled view; map z is drawn at half depth, so the view covers twice the screen height. */
#define FIELD_SCROLL_VIEW_WIDTH SCREEN_WIDTH
#define FIELD_SCROLL_VIEW_DEPTH (2 * VRAM_DRAW_HEIGHT)

/** @brief field_command_set_render_state mode bit selecting the tinted render with the low seven bits as mode. */
#define FIELD_RENDER_STATE_TINT 0x80
#define FIELD_RENDER_STATE_MODE_MASK 0x7F

/** @brief Argument of field_command_spawn_monster that keeps the default revive animation, effect or sound. */
#define FIELD_SPAWN_DEFAULT 0xFF

/** @brief Generic view of the command parameter block. */
typedef struct FieldCommandParams
{
    u16 arg0;
    u16 arg1;
    s32 words[8];
} FieldCommandParams;

/** @brief Parameters of field_command_actor_status. */
typedef struct FieldActorStatusParams
{
    u16 actor;
    u16 operation;
    u32 values[3];
    u32 drop_counts[4];
} FieldActorStatusParams;

/** @brief Parameters of field_command_set_fade. */
typedef struct FieldFadeParams
{
    u16 duration;
    u16 pad2;
    s32 red;
    s32 green;
    s32 blue;
} FieldFadeParams;

/**
 * @brief Parameters of field_command_test_area.
 * @note Positions and areas are in whole units; the explicit position is in
 *       fixed point with eight fractional bits.
 */
typedef struct FieldAreaTestParams
{
    u16 result;
    u16 mode;
    /** @brief Position x, or the actor id in FIELD_AREA_ACTOR mode. */
    u32 x;
    u32 y;
    u32 z;
    union
    {
        struct
        {
            u32 left;
            u32 top;
            u32 width;
            u32 height;
        } rect;
        struct
        {
            u32 center_x;
            u32 center_z;
            u32 half_width;
            u32 half_height;
        } centered;
        struct
        {
            u32 left;
            u32 top;
            u32 right;
            u32 bottom;
        } bounds;
    } area;
} FieldAreaTestParams;

/** @brief Parameters of field_command_read_nibble_table. */
typedef struct FieldNibbleTableParams
{
    u16 column;
    u16 row;
    s32 low;
    s32 high;
} FieldNibbleTableParams;

/** @brief Nibble table resource: a header, then rows of FIELD_NIBBLE_TABLE_COLUMNS bytes. */
typedef struct FieldNibbleTable
{
    u16 resource_id;
    u16 unk2;
    u8 cells[1][FIELD_NIBBLE_TABLE_COLUMNS];
} FieldNibbleTable;

/** @brief Parameters of field_command_get_part_position. */
typedef struct FieldPartPositionParams
{
    u16 unk0;
    u16 unk2;
    s32 x;
    s32 y;
    s32 z;
    s32 object;
    /** @brief Part index, or FIELD_NO_PART for the object itself. */
    s32 part;
} FieldPartPositionParams;

/** @brief Parameters of field_command_items. */
typedef struct FieldItemCommandParams
{
    u16 result;
    u16 operation;
    u32 args[3];
} FieldItemCommandParams;

/** @brief Parameters of field_command_query_records and its query handlers. */
typedef struct FieldRecordQueryParams
{
    /** @brief Actor id on input, the selected record's actor id on output. */
    u16 actor;
    u16 query;
    /** @brief FieldStatusRecord::unk0 side bits of the records to collect. */
    s32 side_mask;
} FieldRecordQueryParams;

/** @brief One collected record: its actor id and its distance from the querying actor. */
typedef struct FieldRecordDistance
{
    s32 actor;
    s32 distance;
} FieldRecordDistance;

/** @brief Records collected by a query; field_sort_keyed_list moves the nearest one to the front. */
typedef struct FieldRecordDistanceList
{
    s32 count;
    FieldRecordDistance entries[FIELD_RECORD_QUERY_CAPACITY];
} FieldRecordDistanceList;

/** @brief Parameters of field_command_scroll_camera. */
typedef struct FieldScrollParams
{
    u16 actor;
    u16 mode;
    s32 x;
    s32 z;
    s32 frames;
} FieldScrollParams;

/** @brief Parameters of field_command_place_lands. */
typedef struct FieldPlaceLandsParams
{
    u16 placed_count;
    u16 pad2;
    s32 first_land;
    s32 second_land;
    s32 unkC;
    /** @brief Placed lands, as written by field_place_lands. */
    s32 placed[4];
} FieldPlaceLandsParams;

/** @brief Parameters of the actor position commands. */
typedef struct FieldPositionParams
{
    u16 actor;
    u16 pad2;
    s32 x;
    s32 y;
    s32 z;
} FieldPositionParams;

/** @brief Parameters of field_command_set_render_state. */
typedef struct FieldRenderStateParams
{
    u16 actor;
    u16 mode;
    s32 red;
    s32 green;
    s32 blue;
} FieldRenderStateParams;

/** @brief Parameters of field_command_spawn_monster. */
typedef struct FieldSpawnParams
{
    u16 actor;
    u16 pad2;
    s32 x;
    s32 y;
    s32 z;
    s32 animation;
    s32 effect;
    s32 sound;
} FieldSpawnParams;

typedef void (*FieldCommandHandler)(s32 command, void* params);
typedef void (*FieldRecordQuery)(FieldRecordDistanceList* list, FieldStatusRecord* source, FieldRecordQueryParams* params);

extern FieldCommandHandler g_field_script_commands[FIELD_SCRIPT_COMMAND_COUNT];
extern FieldRecordQuery g_field_record_queries[];

extern FieldGameState* g_field_game_state;
extern FieldRuntimeContext* g_field_runtime;
extern FieldBattleContext* g_field_battle;
extern FieldItemStaging* D_80123FC4;
extern SceneState* g_field_scene_state;
extern s32 g_field_interaction_active;
extern s32 g_field_scripted_scroll_frames;
extern s32 g_field_scripted_scroll_target_x;
extern s32 g_field_scripted_scroll_target_z;

s32* func_800C1EC8(s32* src, s32* dest, s32 size);
u16* func_800C1E40(s32 resource_id);
FieldStatusState* field_find_object_state(s32 key);
FieldActorRecord* field_find_actor_record_or_default(s32 id);
s32 field_get_actor_position(s32 key, Vec3i* position);
s32 field_set_actor_position(s32 key, s32 x, s32 y, s32 z);
/* Int parameters on purpose: with the (s32, u8, s8) definition the calls would narrow their arguments. */
s32 field_queue_actor_event(s32 owner_id, s32 event_id, s32 argument);
s32 field_revive_actor(s32 key, s32 animation, s32 effect, s32 sound);
FieldItemRecord* field_find_free_inventory_record(void);
void field_sort_keyed_list(FieldRecordDistanceList* list);
s32 field_distance_xz(Vec3i* first, Vec3i* second);
s32 field_place_lands(s32 first_land, s32 second_land, s32* placed);
void field_begin_party_script_control(s32 mode);

/**
 * @brief Run a script command.
 * @param command Command number, an index into g_field_script_commands.
 * @param params Command parameter block.
 */
void field_script_command(s32 command, void* params)
{
    if (command < FIELD_SCRIPT_COMMAND_COUNT)
    {
        g_field_script_commands[command](command, params);
        return;
    }
    record_game_diagnostic(DIAG_ERROR, DIAG_BAD_SCRIPT_COMMAND, command, 0);
}

/**
 * @brief Command 0: clear the parameter words.
 * @param command Command number (unused).
 * @param params Parameter block.
 */
void field_command_clear_params(s32 command, FieldCommandParams* params)
{
    func_800C1EC8(NULL, params->words, sizeof(params->words));
}

/**
 * @brief Command 1: read or change an actor's status state.
 * @param command Command number (unused).
 * @param params Actor, FIELD_ACTOR_STATUS_* operation and values.
 */
void field_command_actor_status(s32 command, FieldActorStatusParams* params)
{
    s32 actor;
    FieldStatusState* state;

    if (params->actor == FIELD_SCRIPT_OWNER)
    {
        actor = g_field_script->status.owner_id;
    }
    else
    {
        actor = params->actor;
    }
    state = field_find_object_state(actor);
    if (state != (FieldStatusState*)FIELD_OBJECT_STATE_NONE)
    {
        switch (params->operation)
        {
        case FIELD_ACTOR_STATUS_SET_POSITION:
            state->position_x = params->values[0];
            state->position_y = params->values[1];
            state->position_z = params->values[2];
            break;
        case FIELD_ACTOR_STATUS_SET_HP:
            state->current = params->values[0];
            state->maximum = params->values[1];
            break;
        case FIELD_ACTOR_STATUS_SET_FLAGS:
            state->effect_flags |= params->values[0];
            state->unk18 |= params->values[1];
            break;
        case FIELD_ACTOR_STATUS_MASK_FLAGS:
            state->effect_flags &= params->values[0];
            state->unk18 &= params->values[1];
            break;
        case FIELD_ACTOR_STATUS_GET_GAUGES:
            params->values[0] = state->status_intensity;
            params->values[1] = state->action_charge;
            params->values[2] = state->level.byte & FIELD_HUD_SHOW_TECHNIQUE_GAUGE;
            break;
        case FIELD_ACTOR_STATUS_GET_HP:
            params->values[0] = state->current;
            params->values[1] = state->maximum;
            if (state->maximum != 0)
            {
                params->values[2] = (state->current * FIELD_HP_EIGHTHS) / state->maximum;
            }
            else
            {
                params->values[2] = 0;
            }
            break;
        case FIELD_ACTOR_STATUS_SET_DROPS:
            state->signal.drop_counts[0] = params->drop_counts[0];
            state->signal.drop_counts[1] = params->drop_counts[1];
            state->signal.drop_counts[2] = params->drop_counts[2];
            state->signal.drop_counts[3] = params->drop_counts[3];
            break;
        case FIELD_ACTOR_STATUS_SET_GAUGE:
            state->status_intensity = params->values[0];
            break;
        }
    }
}

/**
 * @brief Command 2: put every party member under script control.
 */
void field_command_script_party(void)
{
    field_begin_party_script_control(FIELD_PARTY_MODE_ALL);
}

/**
 * @brief Command 14: put the pad-controlled party members under script control.
 */
void field_command_script_pad_party(void)
{
    field_begin_party_script_control(FIELD_PARTY_MODE_PAD_CONTROLLED);
}

/**
 * @brief Command 3: end the interaction, hand the party back and tell every actor.
 */
void field_command_end_interaction(void)
{
    s32 i;

    field_end_party_script_control();
    for (i = 0; i < g_field_runtime->state.actor_count; i++)
    {
        field_queue_actor_event(g_field_runtime->actors[i].id, FIELD_EVENT_INTERACTION, FIELD_INTERACTION_END);
    }
    g_field_runtime->state.flags &= ~FIELD_STATE_PARTY_MODE_MASK;
    g_field_interaction_active = 0;
}

/**
 * @brief Command 4: start a fade to a colour.
 * @param command Command number (unused).
 * @param params Target colour and fade length.
 */
void field_command_set_fade(s32 command, FieldFadeParams* params)
{
    /* Int arguments on purpose: the original passes the words without narrowing them to s16. */
    ((void (*)(s32, s32, s32, s32))field_set_fade_target)(params->red, params->green, params->blue, params->duration);
}

/**
 * @brief Command 5: test whether a position lies inside an area.
 * @param command Command number (unused).
 * @param params FIELD_AREA_* mode, position and area; receives 1 when inside, else 0.
 */
void field_command_test_area(s32 command, FieldAreaTestParams* params)
{
    Vec3i position;
    u32 x;
    u32 z;
    u32 left;
    u32 top;
    u32 right;
    u32 bottom;

    switch (params->mode)
    {
    case FIELD_AREA_RECT:
        left = params->area.rect.left;
        top = params->area.rect.top;
        x = params->x >> 8;
        z = params->z >> 8;
        right = left + params->area.rect.width;
        bottom = top + params->area.rect.height;
        break;
    case FIELD_AREA_CENTERED:
        x = params->x >> 8;
        z = params->z >> 8;
        left = params->area.centered.center_x - params->area.centered.half_width;
        top = params->area.centered.center_z - params->area.centered.half_height;
        right = params->area.centered.center_x + params->area.centered.half_width;
        bottom = params->area.centered.center_z + params->area.centered.half_height;
        break;
    case FIELD_AREA_ACTOR:
        /* x holds the actor id until the position is known; a separate local changes the register allocation. */
        if (params->x != FIELD_SCRIPT_OWNER)
        {
            x = params->x;
        }
        else
        {
            x = g_field_script->status.owner_id;
        }
        field_get_actor_position(x, &position);
        left = params->area.bounds.left;
        top = params->area.bounds.top;
        right = params->area.bounds.right;
        bottom = params->area.bounds.bottom;
        x = position.x >> 8;
        z = position.z >> 8;
        break;
    }
    params->result = x >= left && x <= right && z >= top && z <= bottom;
}

/**
 * @brief Command 6: read both nibbles of one cell of the nibble table resource.
 * @param command Command number (unused).
 * @param params Cell column (from FIELD_NIBBLE_TABLE_FIRST_COLUMN) and row; receives the nibbles.
 */
void field_command_read_nibble_table(s32 command, FieldNibbleTableParams* params)
{
    FieldNibbleTable* table;

    table = (FieldNibbleTable*)func_800C1E40(FIELD_RESOURCE_NIBBLE_TABLE);
    if (table != NULL)
    {
        params->low = table->cells[params->row][params->column - FIELD_NIBBLE_TABLE_FIRST_COLUMN] & 0xF;
        params->high = table->cells[params->row][params->column - FIELD_NIBBLE_TABLE_FIRST_COLUMN] >> 4;
    }
}

/**
 * @brief Command 7: get the position of an object or one of its parts.
 * @param command Command number (unused).
 * @param params Object and part; receives the position in whole units.
 */
void field_command_get_part_position(s32 command, FieldPartPositionParams* params)
{
    FieldPos position;
    s32 part;

    part = -1;
    if (params->part != FIELD_NO_PART)
    {
        part = params->part;
    }
    field_get_object_position(params->object, part, &position);
    params->x = position.x;
    params->y = position.y;
    params->z = position.z;
}

/**
 * @brief Command 8: item creation and staging operations.
 * @param command Command number (unused).
 * @param params FIELD_ITEM_COMMAND_* operation and arguments; most operations write a result.
 */
void field_command_items(s32 command, FieldItemCommandParams* params)
{
    FieldItemRecord* record;
    s32 stock;

    switch (params->operation)
    {
    case FIELD_ITEM_COMMAND_RESET:
        D_80123FC4 = &g_field_runtime->item_staging;
        func_800C1EC8(NULL, (s32*)&g_field_runtime->item_staging, sizeof(FieldItemStaging));
        return;
    case FIELD_ITEM_COMMAND_CREATE:
        record = field_find_free_inventory_record();
        if (record != NULL)
        {
            if (params->args[2] < FIELD_ITEM_SUBTYPE_COUNT)
            {
                stock = g_field_game_state->item_counts[params->args[2]];
                if (stock != 0)
                {
                    g_field_game_state->item_counts[params->args[2]] = stock - 1;
                    field_create_equipment_item(record, params->args[0], params->args[1], params->args[2]);
                    /* Divides the byte offset by 4096, not by the size of an item record. */
                    params->result = (u32)((u8*)record - (u8*)g_field_game_state->items) >> 12;
                    return;
                }
                params->result = FIELD_ITEM_COMMAND_NO_STOCK;
                return;
            }
            params->result = FIELD_ITEM_COMMAND_BAD_SUBTYPE;
            return;
        }
        params->result = FIELD_ITEM_COMMAND_NO_FREE_RECORD;
        return;
    case FIELD_ITEM_COMMAND_TEMPER:
        record = &g_field_game_state->items[params->args[0]];
        if (record->kind != 0)
        {
            stock = g_field_game_state->item_counts[params->args[1]];
            if (stock != 0)
            {
                g_field_game_state->item_counts[params->args[1]] = stock - 1;
                field_temper_item(record, params->args[1]);
                params->result = params->args[0];
                return;
            }
            params->result = FIELD_ITEM_COMMAND_NO_STOCK;
            return;
        }
        params->result = FIELD_ITEM_COMMAND_NO_ITEM;
        return;
    case FIELD_ITEM_COMMAND_LOWER_LEVEL:
        while (params->args[1] != 0)
        {
            func_800BF9A0(params->args[0]);
            params->args[1]--;
        }
        return;
    case FIELD_ITEM_COMMAND_RAISE_LEVEL:
        while (params->args[1] != 0)
        {
            func_800BF880(params->args[0]);
            params->args[1]--;
        }
        return;
    case FIELD_ITEM_COMMAND_TEST_COST:
        FIELD_SCRIPT_ACTIVE_RECORD_STATE()->flags = (FIELD_SCRIPT_ACTIVE_RECORD_STATE()->flags & ~FIELD_SCRIPT_COND) | (func_800BF9F0(params->args[0]) & FIELD_SCRIPT_COND);
        return;
    case FIELD_ITEM_COMMAND_REPLACE_SLOT:
        field_set_script_var(0, FIELD_VAR_RESULT, func_800BF68C(params->args[0], params->args[1], params->args[2]));
        return;
    case FIELD_ITEM_COMMAND_RESERVED:
    default:
        params->result = FIELD_ITEM_COMMAND_BAD_OPERATION;
        return;
    }
}

/**
 * @brief Command 9: collect records with a query handler and select the nearest or the farthest.
 * @param command Command number (unused).
 * @param params Querying actor, query index and flags, and query arguments; receives the selected actor id.
 */
void field_command_query_records(s32 command, FieldRecordQueryParams* params)
{
    FieldRecordDistanceList list;
    FieldStatusRecord* source;

    if (params->actor != FIELD_SCRIPT_OWNER)
    {
        source = field_find_status_record(params->actor);
    }
    else
    {
        source = field_find_status_record(g_field_script->status.owner_id);
    }
    g_field_record_queries[params->query & FIELD_RECORD_QUERY_INDEX_MASK](&list, source, params);

    if (list.count != 0)
    {
        field_sort_keyed_list(&list);
        if (params->query & FIELD_RECORD_QUERY_LAST)
        {
            params->actor = list.entries[list.count - 1].actor;
        }
        else
        {
            params->actor = list.entries[0].actor;
        }
    }
    else
    {
        params->actor = FIELD_RECORD_QUERY_NONE;
    }
}

/**
 * @brief Record query 0: the standing battle records on the given sides, with their distances.
 * @param list Receives the records.
 * @param source Status record of the querying actor.
 * @param params Query parameters; side_mask selects the sides.
 */
void field_query_records_by_side(FieldRecordDistanceList* list, FieldStatusRecord* source, FieldRecordQueryParams* params)
{
    Vec3i source_position;
    Vec3i record_position;
    s32 i;

    field_get_actor_position(source->meta.bytes.id, &source_position);
    list->count = 0;
    for (i = 0; i < FIELD_BATTLE_RECORD_COUNT; i++)
    {
        if ((g_field_battle->records[i].unk0 & params->side_mask) && g_field_battle->records[i].state->current != 0)
        {
            field_get_actor_position(g_field_battle->records[i].meta.bytes.id, &record_position);
            list->entries[list->count].actor = g_field_battle->records[i].meta.bytes.id;
            list->entries[list->count].distance = field_distance_xz(&record_position, &source_position);
            list->count += 1;
        }
    }
}

/**
 * @brief Command 10: save, restore or set the scripted scroll target.
 * @param command Command number (unused).
 * @param params FIELD_SCROLL_* mode, target and frame count.
 * @note A (0, 0) target turns the scripted scroll off, so a computed (0, 0) becomes (0, 1).
 */
void field_command_scroll_camera(s32 command, FieldScrollParams* params)
{
    Vec3i position;
    FieldMapBounds* bounds;
    s32* target;
    FieldRuntimeContext* runtime;
    s32 offset_x;
    s32 offset_z;
    s32 scroll_z;
    s32 target_z;
    s32 scroll_x;
    s32 limit_x;
    s32 limit_z;
    s32 actor;

    switch (params->mode)
    {
    case FIELD_SCROLL_SAVE:
        scroll_x = -g_field_scene_state->camera_x >> 8;
        g_field_runtime->saved_scroll_x = scroll_x;
        g_field_scripted_scroll_target_x = scroll_x;
        scroll_z = -g_field_scene_state->camera_z >> 9;
        g_field_runtime->saved_scroll_z = scroll_z;
        g_field_scripted_scroll_target_z = scroll_z;
        if ((scroll_x | scroll_z) == 0)
        {
            g_field_scripted_scroll_target_z = 1;
        }
        g_field_scripted_scroll_frames = 0;
        return;
    case FIELD_SCROLL_TO:
        g_field_scripted_scroll_target_x = params->x;
        g_field_scripted_scroll_target_z = params->z;
        g_field_scripted_scroll_frames = params->frames;
        return;
    case FIELD_SCROLL_TO_ACTOR:
        bounds = FIELD_MAP_BOUNDS;
        if (params->actor == FIELD_SCRIPT_OWNER)
        {
            actor = g_field_script->status.owner_id;
        }
        else
        {
            actor = params->actor;
        }
        field_get_actor_position(actor, &position);
        offset_x = (position.x >> 8) - FIELD_SCROLL_VIEW_WIDTH / 2;
        target = &g_field_scripted_scroll_target_x;
        if (offset_x > 0)
        {
            limit_x = bounds->width - FIELD_SCROLL_VIEW_WIDTH;
            if (offset_x < limit_x)
            {
                limit_x = offset_x;
            }
            *target = limit_x;
        }
        else
        {
            *target = 0;
        }
        offset_z = ((position.z - position.y) >> 8) - FIELD_SCROLL_VIEW_DEPTH / 2;
        target = &g_field_scripted_scroll_target_z;
        if (offset_z > 0)
        {
            /* Loaded unsigned and sign-extended; an s16 field would load it with lh. */
            limit_z = (s16)bounds->depth - FIELD_SCROLL_VIEW_DEPTH;
            if (offset_z < limit_z)
            {
                limit_z = offset_z;
            }
            *target = limit_z;
        }
        else
        {
            *target = 0;
        }
        target_z = g_field_scripted_scroll_target_z / 2;
        g_field_scripted_scroll_target_z = target_z;
        if ((g_field_scripted_scroll_target_x | target_z) == 0)
        {
            g_field_scripted_scroll_target_z = 1;
        }
        g_field_scripted_scroll_frames = params->frames;
        return;
    case FIELD_SCROLL_RESTORE:
        runtime = g_field_runtime;
        g_field_scripted_scroll_frames = params->frames;
        g_field_scripted_scroll_target_x = runtime->saved_scroll_x;
        g_field_scripted_scroll_target_z = runtime->saved_scroll_z;
        return;
    }
}

/**
 * @brief Command 11: place up to two lands on the map.
 * @param command Command number (unused).
 * @param params Lands to place; receives the placed count and the placed lands.
 */
void field_command_place_lands(s32 command, FieldPlaceLandsParams* params)
{
    params->placed_count = field_place_lands(params->first_land, params->second_land, params->placed);
}

/**
 * @brief Command 12: move an actor.
 * @param command Command number (unused).
 * @param params Actor and position; y is given upwards and negated for the actor.
 */
void field_command_set_actor_position(s32 command, FieldPositionParams* params)
{
    s32 actor;

    if (params->actor == FIELD_SCRIPT_OWNER)
    {
        actor = g_field_script->status.owner_id;
    }
    else
    {
        actor = params->actor;
    }
    field_set_actor_position(actor, params->x, -params->y, params->z);
}

/**
 * @brief Command 13: set an actor's render colour and mode.
 * @param command Command number (unused).
 * @param params Actor, packed mode (FIELD_RENDER_STATE_TINT plus mode, or a plain mode) and colour.
 */
void field_command_set_render_state(s32 command, FieldRenderStateParams* params)
{
    s32 actor;
    s32 render_mode;
    s32 tint;

    if (params->actor == FIELD_SCRIPT_OWNER)
    {
        actor = g_field_script->status.owner_id;
    }
    else
    {
        actor = params->actor;
    }
    if (params->mode & FIELD_RENDER_STATE_TINT)
    {
        tint = 1;
        render_mode = params->mode & FIELD_RENDER_STATE_MODE_MASK;
    }
    else
    {
        tint = 0;
        render_mode = params->mode;
    }
    field_set_actor_render_state(params->red, params->green, params->blue, tint, render_mode, actor);
}

/**
 * @brief Command 15: bring a script-only actor into the battle as a monster.
 * @param command Command number (unused).
 * @param params Actor, position, and the revive animation, effect and sound (FIELD_SPAWN_DEFAULT for the default).
 */
void field_command_spawn_monster(s32 command, FieldSpawnParams* params)
{
    s32 actor;
    FieldStatusRecord* record;
    s32 animation;
    s32 effect;
    s32 sound;

    if (params->actor == FIELD_SCRIPT_OWNER)
    {
        actor = g_field_script->status.owner_id;
    }
    else
    {
        actor = params->actor;
    }
    if (actor != 0 && field_find_actor_record_or_default(actor)->flags.bits.script_only)
    {
        record = field_find_free_status_record();
        if (record != NULL)
        {
            field_set_actor_position(actor, params->x, params->y, params->z);
            field_set_actor_group(actor, g_field_runtime->state.bytes.trigger_group);
            animation = -1;
            if (params->animation != FIELD_SPAWN_DEFAULT)
            {
                animation = params->animation;
            }
            effect = -1;
            if (params->effect != FIELD_SPAWN_DEFAULT)
            {
                effect = params->effect;
            }
            sound = -1;
            if (params->sound != FIELD_SPAWN_DEFAULT)
            {
                sound = params->sound;
            }
            func_800C1EC8(NULL, (s32*)record, sizeof(FieldStatusRecord));
            field_init_monster_record(actor, record, field_find_object_state(actor));
            field_revive_actor(actor, animation, effect, sound);
        }
    }
}

/**
 * @brief Command 16: get an actor's position.
 * @param command Command number (unused).
 * @param params Actor; receives the position with y given upwards.
 */
void field_command_get_actor_position(s32 command, FieldPositionParams* params)
{
    s32 actor;
    Vec3i position;

    if (params->actor == FIELD_SCRIPT_OWNER)
    {
        actor = g_field_script->status.owner_id;
    }
    else
    {
        actor = params->actor;
    }
    field_get_actor_position(actor, &position);
    params->x = position.x;
    params->y = -position.y;
    params->z = position.z;
}
