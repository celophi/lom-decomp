/**
 * @file field_actor_action_defaults.c
 * @brief Restore the default action command maps and player action slots.
 */

#include "common.h"
#include "../internal/field_calls.h"
#include "../internal/field_actor_tables.h"

/** @brief Number of map entries cleared by a map reset; entries 8 to 10 keep their disable flags. */
#define FIELD_ACTION_MAP_RESET_COUNT 8

/**
 * @brief Clear one action command map and restore the default commands and player action slots.
 * @param map_index Action command map to clear before restoring the defaults.
 * @note Slots 0 and 1 keep their player-assigned abilities; only their flags are reset.
 *       The commands 0x25 and 0x27..0x29 and the parameters (1, 16) given to actions 3 and 8..10
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

    for (i = 0; i < FIELD_PLAYER_COUNT; i++)
    {
        g_field_action_command_maps[i].commands[2] = FIELD_ACTION_COMMAND(2);
        g_field_action_command_maps[i].commands[3] = FIELD_ACTION_COMMAND(3);
        g_field_action_command_maps[i].commands[1] = FIELD_ACTION_COMMAND(1);
        g_field_action_command_maps[i].commands[0] = FIELD_ACTION_COMMAND(0);
        g_field_action_command_maps[i].commands[8] = FIELD_ACTION_COMMAND(8);
        g_field_action_command_maps[i].commands[9] = FIELD_ACTION_COMMAND(9);
        g_field_action_command_maps[i].commands[10] = FIELD_ACTION_COMMAND(10);
    }

    for (i = 0; i < FIELD_PARTY_COUNT; i++)
    {
        row = &g_field_resource_actions[i];

        row->slots[FIELD_RESOURCE_ACTION_COMBO].command = FIELD_ANIMATION_COMBO;
        row->slots[FIELD_RESOURCE_ACTION_COMBO].flags.instrument = 0;
        row->slots[FIELD_RESOURCE_ACTION_COMBO].flags.target_filter = FIELD_ACTION_TARGET_NONE;
        row->slots[FIELD_RESOURCE_ACTION_COMBO].animation = 0;
        row->slots[FIELD_RESOURCE_ACTION_COMBO].parameter = 1;
        row->slots[FIELD_RESOURCE_ACTION_COMBO].flags.target_group = 0;

        row->slots[FIELD_RESOURCE_ACTION_POWER_ATTACK].command = FIELD_ANIMATION_POWER_ATTACK;
        row->slots[FIELD_RESOURCE_ACTION_POWER_ATTACK].flags.instrument = 0;
        row->slots[FIELD_RESOURCE_ACTION_POWER_ATTACK].flags.target_filter = FIELD_ACTION_TARGET_NONE;
        row->slots[FIELD_RESOURCE_ACTION_POWER_ATTACK].animation = 0;
        row->slots[FIELD_RESOURCE_ACTION_POWER_ATTACK].parameter = 1;
        row->slots[FIELD_RESOURCE_ACTION_POWER_ATTACK].flags.target_group = 0;

        row->slots[FIELD_RESOURCE_ACTION_ABILITY_1].flags.instrument = 0;
        row->slots[FIELD_RESOURCE_ACTION_ABILITY_1].flags.target_filter = FIELD_ACTION_TARGET_NONE;
        row->slots[FIELD_RESOURCE_ACTION_ABILITY_1].flags.target_group = 0;

        row->slots[FIELD_RESOURCE_ACTION_ABILITY_0].flags.instrument = 0;
        row->slots[FIELD_RESOURCE_ACTION_ABILITY_0].flags.target_filter = FIELD_ACTION_TARGET_NONE;
        row->slots[FIELD_RESOURCE_ACTION_ABILITY_0].flags.target_group = 0;

        row->slots[FIELD_RESOURCE_ACTION_DASH_POWER_ATTACK].command = 0x27;
        row->slots[FIELD_RESOURCE_ACTION_DASH_POWER_ATTACK].flags.instrument = 0;
        row->slots[FIELD_RESOURCE_ACTION_DASH_POWER_ATTACK].flags.target_filter = FIELD_ACTION_TARGET_NONE;
        row->slots[FIELD_RESOURCE_ACTION_DASH_POWER_ATTACK].animation = 0;
        row->slots[FIELD_RESOURCE_ACTION_DASH_POWER_ATTACK].parameter = 16;
        row->slots[FIELD_RESOURCE_ACTION_DASH_POWER_ATTACK].flags.target_group = 0;

        row->slots[FIELD_RESOURCE_ACTION_JUMP_POWER_ATTACK].command = 0x28;
        row->slots[FIELD_RESOURCE_ACTION_JUMP_POWER_ATTACK].flags.instrument = 0;
        row->slots[FIELD_RESOURCE_ACTION_JUMP_POWER_ATTACK].flags.target_filter = FIELD_ACTION_TARGET_NONE;
        row->slots[FIELD_RESOURCE_ACTION_JUMP_POWER_ATTACK].animation = 0;
        row->slots[FIELD_RESOURCE_ACTION_JUMP_POWER_ATTACK].parameter = 16;
        row->slots[FIELD_RESOURCE_ACTION_JUMP_POWER_ATTACK].flags.target_group = 0;

        row->slots[FIELD_RESOURCE_ACTION_TACKLE_POWER_ATTACK].command = 0x29;
        row->slots[FIELD_RESOURCE_ACTION_TACKLE_POWER_ATTACK].flags.instrument = 0;
        row->slots[FIELD_RESOURCE_ACTION_TACKLE_POWER_ATTACK].flags.target_filter = FIELD_ACTION_TARGET_NONE;
        row->slots[FIELD_RESOURCE_ACTION_TACKLE_POWER_ATTACK].animation = 0;
        row->slots[FIELD_RESOURCE_ACTION_TACKLE_POWER_ATTACK].parameter = 16;
        row->slots[FIELD_RESOURCE_ACTION_TACKLE_POWER_ATTACK].flags.target_group = 0;
    }

    for (i = 0; i < FIELD_PLAYER_COUNT; i++)
    {
        g_field_action_command_maps[i].commands[5] = FIELD_ACTION_COMMAND(5);
        g_field_action_command_maps[i].commands[7] = FIELD_ACTION_COMMAND(7);
        g_field_action_command_maps[i].commands[4] = FIELD_ACTION_COMMAND(4);
        g_field_action_command_maps[i].commands[6] = FIELD_ACTION_COMMAND(6);
    }
}
