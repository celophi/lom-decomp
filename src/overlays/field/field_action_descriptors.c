/**
 * @file field_action_descriptors.c
 * @brief Selection of the action descriptor for the attacker's current battle action.
 */

#include "game_audio.h"
#include "common.h"
#include "field_records.h"

/** @brief Error status passed to record_game_diagnostic. */
#define DIAG_ERROR 0x8001

/** @brief Diagnostic code: unknown command in a character's command slot. */
#define DIAG_BAD_COMMAND 0x66

/** @brief Diagnostic code: monster action id past the end of its template's list. */
#define DIAG_BAD_MONSTER_ACTION 0x69

/** @brief Record kind bits (FieldStatusRecordMeta bits.kind) within the packed word. */
#define STATUS_KIND_MASK 0xFC00

/** @brief A record kind as it appears in the packed word. */
#define STATUS_KIND_BITS(kind) ((kind) << 10)

/** @brief Record kind of the stored companion (character type 3). */
#define STATUS_KIND_COMPANION 3

/** @brief Record kind of the second companion type (character type 4). */
#define STATUS_KIND_COMPANION_B 4

/** @brief Resource page holding the companion action descriptors. */
#define COMPANION_ACTION_PAGE 2

/** @brief Monster action ids without a descriptor that are not an error. */
#define MONSTER_ACTION_NONE_A 23
#define MONSTER_ACTION_NONE_B 24

/** @brief Tables of the action descriptor bank. */
enum
{
    ACTION_TABLE_WEAPON,      /**< Indexed by the weapon's action bytes. */
    ACTION_TABLE_WEAPON_TYPE, /**< WEAPON_TYPE_ACTION_COUNT skills per weapon type. */
    ACTION_TABLE_ITEM,        /**< One ItemActionRow per item row. */
    ACTION_TABLE_COMMAND      /**< Command actions, and action ids from FIRST_COMMAND_ACTION up. */
};

/** @brief Skills per weapon type in ACTION_TABLE_WEAPON_TYPE. */
#define WEAPON_TYPE_ACTION_COUNT 24

/** @brief Descriptors per row of ACTION_TABLE_ITEM. */
#define ITEM_ROW_ACTION_COUNT 14

/** @brief Action id of the first entry of ACTION_TABLE_COMMAND. */
#define FIRST_COMMAND_ACTION 7

/** @brief Character action ids. */
enum
{
    ACTION_COMMAND_0 = 0, /**< Command in info.actions.commands[0]. */
    ACTION_COMMAND_1 = 1, /**< Command in info.actions.commands[1]. */
    ACTION_WEAPON_2 = 2,  /**< Weapon action byte 0, offset by the action parameter. */
    ACTION_WEAPON_3 = 3,  /**< Weapon action byte 1. */
    ACTION_SKILL_0 = 4,   /**< Actions 4 to 7: info.actions.skills[0] to [3]. */
    ACTION_SKILL_1 = 5,
    ACTION_SKILL_2 = 6,
    ACTION_SKILL_3 = 7,
    ACTION_WEAPON_8 = 8,  /**< Actions 8 to 10: weapon action bytes 2 to 4. */
    ACTION_WEAPON_9 = 9,
    ACTION_WEAPON_10 = 10
};

/** @brief Skill slot bit: the low bits index the item records after the equipment slots. */
#define SKILL_SLOT_ITEM 0x80

/** @brief One row of ACTION_TABLE_ITEM. */
typedef struct ItemActionRow
{
    FieldActionDescriptor actions[ITEM_ROW_ACTION_COUNT];
} ItemActionRow;

extern FieldBattleContext *g_field_battle;
extern FieldActionBank *g_field_action_bank;
extern FieldGameState *g_field_game_state;

void *func_800C2958(s32 page, u16 index);

static FieldActionDescriptor *field_command_action_descriptor(s32 command);

/**
 * @brief Return one table of the action descriptor bank.
 * @param table ACTION_TABLE_* index.
 * @return First descriptor of the table.
 */
static inline FieldActionDescriptor *action_table(s32 table)
{
    return (FieldActionDescriptor *)((u8 *)g_field_action_bank + g_field_action_bank->table_offsets[table]);
}

/**
 * @brief Select the descriptor of the attacker's current action and reset the action power.
 * @return The descriptor, or NULL when the action has none.
 * @note Weapon action bytes are derived.weapon.stats[0] to [4] of the attacker's weapon.
 */
FieldActionDescriptor *field_select_action_descriptor(void)
{
    s32 weapon_action;
    s32 action_id;
    s32 kind;
    u32 selector;
    u32 slot;
    FieldItemRecord *weapon;
    FieldItemRecord *item;
    FieldActionDescriptor *descriptor;
    FieldStatusRecord *attacker;
    FieldBattleAction *action;
    FieldActorTemplate *template;
    FieldBattleContext *ctx;

    g_field_battle->power = g_field_battle->attacker->unk18;
    g_field_battle->power_flags = 0;
    attacker = g_field_battle->attacker;
    kind = attacker->meta.packed & STATUS_KIND_MASK;
    if (kind == STATUS_KIND_BITS(FIELD_STATUS_KIND_MONSTER))
    {
        action = g_field_battle->action;
        template = attacker->template;
        action_id = action->action_id;
        if (action_id < template->action_count)
        {
            descriptor = (FieldActionDescriptor *)template->actions[action_id];
        }
        else if ((action_id != MONSTER_ACTION_NONE_A) && (action_id != MONSTER_ACTION_NONE_B))
        {
            record_game_diagnostic(DIAG_ERROR, DIAG_BAD_MONSTER_ACTION, action->attacker_id, action_id);
            descriptor = NULL;
        }
        else
        {
            descriptor = NULL;
        }
    }
    else if ((kind == STATUS_KIND_BITS(STATUS_KIND_COMPANION)) || (kind == STATUS_KIND_BITS(STATUS_KIND_COMPANION_B)))
    {
        descriptor = func_800C2958(COMPANION_ACTION_PAGE, g_field_battle->action->action_id);
    }
    else
    {
        weapon = &g_field_game_state->characters[attacker->meta.bytes.id].equipment[0];
        g_field_battle->power_flags = weapon->flags2C;
        selector = g_field_battle->action->action_id;
        switch (selector)
        {
        case ACTION_COMMAND_0:
        case ACTION_COMMAND_1:
            descriptor = field_command_action_descriptor(
                g_field_game_state->characters[g_field_battle->attacker->meta.bytes.id].info.actions.commands[g_field_battle->action->action_id]);
            break;
        case ACTION_WEAPON_2:
            descriptor = &action_table(ACTION_TABLE_WEAPON)[weapon->derived.weapon.stats[0]];
            descriptor += g_field_battle->action->param;
            break;
        case ACTION_WEAPON_3:
            descriptor = &action_table(ACTION_TABLE_WEAPON)[weapon->derived.weapon.stats[g_field_battle->action->action_id - 2]];
            break;
        case ACTION_SKILL_0:
        case ACTION_SKILL_1:
        case ACTION_SKILL_2:
        case ACTION_SKILL_3:
            ctx = g_field_battle;
            slot = g_field_game_state->characters[ctx->attacker->meta.bytes.id].info.actions.skills[ctx->action->action_id - ACTION_SKILL_0];
            if (slot < SKILL_SLOT_ITEM)
            {
                s32 index;

                index = weapon->info.bits.item_type * WEAPON_TYPE_ACTION_COUNT + slot;
                descriptor = &action_table(ACTION_TABLE_WEAPON_TYPE)[index];
            }
            else
            {
                item = &g_field_game_state->characters[ctx->attacker->meta.bytes.id].equipment[(slot + FIELD_EQUIPMENT_SLOT_COUNT) & (SKILL_SLOT_ITEM - 1)];
                slot = item->derived.bytes[2];
                ctx->power_flags = 0;
                ctx->power = slot;
                descriptor = &((ItemActionRow *)action_table(ACTION_TABLE_ITEM))[item->derived.bytes[0]].actions[item->derived.bytes[1]];
            }
            break;
        case ACTION_WEAPON_8:
        case ACTION_WEAPON_9:
        case ACTION_WEAPON_10:
            weapon_action = g_field_battle->action->action_id;
            descriptor = &action_table(ACTION_TABLE_WEAPON)[weapon->derived.weapon.stats[weapon_action - 6]];
            /* Never true for actions 8 to 10. */
            if (weapon_action == 18 || weapon_action == 19)
            {
                g_field_battle->attacker->counter--;
            }
            break;
        default:
            descriptor = &action_table(ACTION_TABLE_COMMAND)[selector] - FIRST_COMMAND_ACTION;
            break;
        }
    }
    return descriptor;
}

/**
 * @brief Return the ACTION_TABLE_COMMAND descriptor of a character command.
 * @param command Command code from info.actions.commands.
 * @return The descriptor, or NULL for command 0x37 and for unknown commands.
 */
static FieldActionDescriptor *field_command_action_descriptor(s32 command)
{
    FieldActionDescriptor *descriptor;

    descriptor = action_table(ACTION_TABLE_COMMAND);
    switch (command)
    {
    case 0x33:
        /* field_battle_run_party_event takes no arguments; the original call still passes the attacker id. */
        field_battle_run_party_event(g_field_battle->attacker->meta.bytes.id);
        descriptor += 16;
        break;
    case 0x34:
        descriptor += 15;
        g_field_battle->attacker->counter--;
        break;
    case 0x32:
        break;
    case 0x36:
        descriptor += 1;
        break;
    case 0x37:
        descriptor = NULL;
        break;
    case 0x3E:
        descriptor += 2;
        break;
    case 0x43:
        descriptor += 3;
        break;
    case 0x45:
        descriptor += 4;
        break;
    case 0x4F:
        descriptor += 14;
        break;
    default:
        record_game_diagnostic(DIAG_ERROR, DIAG_BAD_COMMAND, command, -1);
        return NULL;
    }
    return descriptor;
}
