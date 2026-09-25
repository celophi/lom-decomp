/**
 * @file field_actor_input_map_init.c
 * @brief Reset the action command maps and apply the weapon-specific action parameters.
 */

#include "common.h"
#include "field_calls.h"
#include "field_actor_tables.h"

/** @brief Number of player actions whose parameter depends on the weapon type. */
#define FIELD_WEAPON_ACTION_COUNT 5

/**
 * @brief Weapon types (FieldPlayerRecord::weapon_type) with their own action parameters.
 * @note Names assume the in-game weapon order (knife, sword, axe, ...).
 */
#define FIELD_WEAPON_HAMMER 5
#define FIELD_WEAPON_STAFF 7
#define FIELD_WEAPON_GLOVE 8
#define FIELD_WEAPON_FLAIL 9
#define FIELD_WEAPON_BOW 10

/** @brief Action slots (2, 3, 8, 9, 10) whose parameter the weapon type selects. */
extern u8 g_field_weapon_action_slots[FIELD_WEAPON_ACTION_COUNT];
/** @brief Action parameters of the weapon types without a table of their own. */
extern u8 g_field_weapon_action_default_params[FIELD_WEAPON_ACTION_COUNT];
/** @brief Action parameters of hammers, staves, gloves and flails. */
extern u8 g_field_weapon_action_blunt_params[FIELD_WEAPON_ACTION_COUNT];
/** @brief Action parameters of bows. */
extern u8 g_field_weapon_action_bow_params[FIELD_WEAPON_ACTION_COUNT];
extern FieldActionRow g_field_resource_actions[];

/**
 * @brief Reset both action command maps and the player action slots to their defaults.
 */
void field_reset_action_command_maps(void)
{
    field_reset_action_command_map(0);
    field_reset_action_command_map(1);
}

/**
 * @brief Set the weapon-dependent action parameters of a player from its weapon type.
 * @param player_index Player record and resource action row to update.
 */
void field_apply_weapon_action_params(s32 player_index)
{
    s32 i;

    for (i = 0; i < FIELD_WEAPON_ACTION_COUNT; i++)
    {
        switch (g_field_player_records[player_index].weapon_type)
        {
        case FIELD_WEAPON_HAMMER:
        case FIELD_WEAPON_STAFF:
        case FIELD_WEAPON_GLOVE:
        case FIELD_WEAPON_FLAIL:
            g_field_resource_actions[player_index].slots[g_field_weapon_action_slots[i]].parameter = g_field_weapon_action_blunt_params[i];
            break;
        case FIELD_WEAPON_BOW:
            g_field_resource_actions[player_index].slots[g_field_weapon_action_slots[i]].parameter = g_field_weapon_action_bow_params[i];
            break;
        default:
            g_field_resource_actions[player_index].slots[g_field_weapon_action_slots[i]].parameter = g_field_weapon_action_default_params[i];
            break;
        }
    }
}
