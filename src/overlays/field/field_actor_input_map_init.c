/**
 * @file field_actor_input_map_init.c
 * @brief Initialize weapon-specific input maps and request the shared action defaults.
 */

#include "common.h"

/** @brief Number of action slots rewritten from a weapon-specific table. */
#define FIELD_INPUT_MAP_ACTION_COUNT 5

/** @brief Player record fields read here (stride 0x268). */
typedef struct
{
    u8 unk0;
    u8 weapon_type;
    u8 pad2[0x268 - 0x2];
} FieldInputMapPlayer;

/** @brief One eight-byte action slot; only @c unk6 is written here. */
typedef struct
{
    u8 pad0[0x6];
    u16 unk6;
} FieldInputMapSlot;

void func_80091518(s32 map_index);

extern u8 D_800EB21C[];
extern u8 D_800EB224[];
extern u8 D_800EB22C[];
extern u8 D_800EB234[];
extern FieldInputMapPlayer g_field_player_records[];
extern FieldInputMapSlot g_field_resource_actions[][50];

/**
 * @brief Reset both action-animation maps to their defaults.
 */
void func_80091410(void)
{
    func_80091518(0);
    func_80091518(1);
}

/**
 * @brief Set five action slot values from the table for the player's weapon type.
 * @param player_index Player record and action table index.
 */
void func_80091438(s32 player_index)
{
    s32 i;

    for (i = 0; i < FIELD_INPUT_MAP_ACTION_COUNT; i++)
    {
        switch (g_field_player_records[player_index].weapon_type)
        {
        case 5:
        case 7:
        case 8:
        case 9:
            g_field_resource_actions[player_index][D_800EB21C[i]].unk6 = D_800EB22C[i];
            break;
        case 10:
            g_field_resource_actions[player_index][D_800EB21C[i]].unk6 = D_800EB234[i];
            break;
        default:
            g_field_resource_actions[player_index][D_800EB21C[i]].unk6 = D_800EB224[i];
            break;
        }
    }
}
