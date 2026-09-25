/**
 * @file field_actor_action_defaults.c
 * @brief Restore the default action command maps and player action slots.
 */

#include "common.h"
#include "field_calls.h"
#include "field_actor_tables.h"

/** @brief Number of player resources whose action row gets default slots. */
#define FIELD_PLAYER_RESOURCE_COUNT 3

/** @brief Number of action command maps (one per controller). */
#define FIELD_ACTION_MAP_COUNT 2

/** @brief Number of actions with a command map entry. */
#define FIELD_ACTION_MAP_ENTRY_COUNT 11

/** @brief Number of map entries cleared by a map reset. */
#define FIELD_ACTION_MAP_RESET_COUNT 8

/** @brief Actor command that performs the action given in the command's high byte. */
#define FIELD_COMMAND_PERFORM_ACTION 0x85

/** @brief Actor command word that performs action @p action. */
#define FIELD_ACTION_COMMAND(action) (((action) << 8) | FIELD_COMMAND_PERFORM_ACTION)

/** @brief Target filter value of an action that does not look for targets. */
#define FIELD_ACTION_TARGET_NONE 0xFF

/** @brief Action command map of one controller: command words and disable flags. */
typedef struct
{
    u16 commands[FIELD_ACTION_MAP_ENTRY_COUNT];
    u8 disabled[FIELD_ACTION_MAP_ENTRY_COUNT];
    u8 pad;
} FieldActionCommandMap;

extern FieldActionRow g_field_resource_actions[];
extern FieldActionCommandMap g_field_action_command_maps[];

/**
 * @brief Clear one action command map and restore the default commands and player action slots.
 * @param map_index Action command map to clear before restoring the defaults.
 * @note The commands (0x1F..0x29) and parameters (1, 16) given to actions 2, 3 and 8..10
 *       have no known names yet; the weapon input map later overrides the parameters.
 */
void field_reset_action_command_map(s32 map_index)
{
    FieldActionRow* row;
    s32 i;

    for (i = 0; i < FIELD_ACTION_MAP_RESET_COUNT; i++)
    {
        g_field_action_command_maps[map_index].commands[i] = 0;
        g_field_action_command_maps[map_index].disabled[i] = 0;
    }

    for (i = 0; i < FIELD_ACTION_MAP_COUNT; i++)
    {
        g_field_action_command_maps[i].commands[2] = FIELD_ACTION_COMMAND(2);
        g_field_action_command_maps[i].commands[3] = FIELD_ACTION_COMMAND(3);
        g_field_action_command_maps[i].commands[1] = FIELD_ACTION_COMMAND(1);
        g_field_action_command_maps[i].commands[0] = FIELD_ACTION_COMMAND(0);
        g_field_action_command_maps[i].commands[8] = FIELD_ACTION_COMMAND(8);
        g_field_action_command_maps[i].commands[9] = FIELD_ACTION_COMMAND(9);
        g_field_action_command_maps[i].commands[10] = FIELD_ACTION_COMMAND(10);
    }

    for (i = 0; i < FIELD_PLAYER_RESOURCE_COUNT; i++)
    {
        row = &g_field_resource_actions[i];
        row->slots[2].command = 0x1F;
        row->slots[3].command = 0x25;
        row->slots[8].command = 0x27;
        row->slots[2].flags.instrument = 0;
        row->slots[3].flags.instrument = 0;
        row->slots[2].flags.target_filter = FIELD_ACTION_TARGET_NONE;
        row->slots[1].flags.instrument = 0;
        row->slots[0].flags.instrument = 0;
        row->slots[3].flags.target_filter = FIELD_ACTION_TARGET_NONE;
        row->slots[1].flags.target_filter = FIELD_ACTION_TARGET_NONE;
        row->slots[8].flags.instrument = 0;
        row->slots[2].flags.target_group = 0;
        row->slots[2].animation = 0;
        row->slots[2].parameter = 1;
        row->slots[3].animation = 0;
        row->slots[3].parameter = 1;
        row->slots[8].animation = 0;
        row->slots[8].parameter = 16;
        row->slots[0].flags.target_filter = FIELD_ACTION_TARGET_NONE;
        row->slots[8].flags.target_filter = FIELD_ACTION_TARGET_NONE;
        row->slots[3].flags.target_group = 0;
        row->slots[1].flags.target_group = 0;
        row->slots[0].flags.target_group = 0;
        row->slots[8].flags.target_group = 0;
        row->slots[9].command = 0x28;
        row->slots[10].command = 0x29;
        row->slots[9].animation = 0;
        row->slots[9].parameter = 16;
        row->slots[10].animation = 0;
        row->slots[10].parameter = 16;
        row->slots[9].flags.instrument = 0;
        row->slots[9].flags.target_filter = FIELD_ACTION_TARGET_NONE;
        row->slots[10].flags.instrument = 0;
        row->slots[10].flags.target_filter = FIELD_ACTION_TARGET_NONE;
        row->slots[9].flags.target_group = 0;
        row->slots[10].flags.target_group = 0;
    }

    for (i = 0; i < FIELD_ACTION_MAP_COUNT; i++)
    {
        g_field_action_command_maps[i].commands[5] = FIELD_ACTION_COMMAND(5);
        g_field_action_command_maps[i].commands[7] = FIELD_ACTION_COMMAND(7);
        g_field_action_command_maps[i].commands[4] = FIELD_ACTION_COMMAND(4);
        g_field_action_command_maps[i].commands[6] = FIELD_ACTION_COMMAND(6);
    }
}
