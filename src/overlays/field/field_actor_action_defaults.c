/**
 * @file field_actor_action_defaults.c
 * @brief Reset action-animation maps and restore animation command defaults.
 */

#include "common.h"
#include "field_calls.h"

/** @brief Number of player records that own an action table. */
#define FIELD_ACTION_RECORD_COUNT 3

/** @brief Number of action-animation maps. */
#define FIELD_ACTION_MAP_COUNT 2

/** @brief Number of map entries cleared by a map reset. */
#define FIELD_ACTION_MAP_RESET_COUNT 8

/** @brief Control halfword of one action slot; the reset sets @c value to 0xFF and clears both flag fields. */
typedef struct
{
    u16 value : 8;
    u16 unk8 : 2;
    u16 unkA : 1;
    u16 unkB : 5;
} FieldActionControl;

/** @brief One eight-byte action slot of a player's action table. */
typedef struct
{
    u16 animation;
    FieldActionControl control;
    u16 unk4;
    u16 unk6;
} FieldActionSlot;

/** @brief Per-player table of fifty action slots. */
typedef struct
{
    FieldActionSlot slots[50];
} FieldActionRecord;

/** @brief One action-animation map: eleven animation ids and their disable flags. */
typedef struct
{
    u16 animation_ids[11];
    u8 disabled[11];
    u8 pad;
} FieldActionMap;

extern FieldActionRecord g_field_resource_actions[];
extern FieldActionMap g_field_action_animation_maps[];

/**
 * @brief Reset the selected action map and restore shared animation command defaults.
 * @param map_index Action map index to clear before restoring defaults.
 */
void func_80091518(s32 map_index)
{
    FieldActionRecord* record;
    s32 i;

    for (i = 0; i < FIELD_ACTION_MAP_RESET_COUNT; i++)
    {
        g_field_action_animation_maps[map_index].animation_ids[i] = 0;
        g_field_action_animation_maps[map_index].disabled[i] = 0;
    }

    for (i = 0; i < FIELD_ACTION_MAP_COUNT; i++)
    {
        g_field_action_animation_maps[i].animation_ids[2] = 0x285;
        g_field_action_animation_maps[i].animation_ids[3] = 0x385;
        g_field_action_animation_maps[i].animation_ids[1] = 0x185;
        g_field_action_animation_maps[i].animation_ids[0] = 0x85;
        g_field_action_animation_maps[i].animation_ids[8] = 0x885;
        g_field_action_animation_maps[i].animation_ids[9] = 0x985;
        g_field_action_animation_maps[i].animation_ids[10] = 0xA85;
    }

    for (i = 0; i < FIELD_ACTION_RECORD_COUNT; i++)
    {
        record = &g_field_resource_actions[i];
        record->slots[2].animation = 0x1F;
        record->slots[3].animation = 0x25;
        record->slots[8].animation = 0x27;
        record->slots[2].control.unkA = 0;
        record->slots[3].control.unkA = 0;
        record->slots[2].control.value = 0xFF;
        record->slots[1].control.unkA = 0;
        record->slots[0].control.unkA = 0;
        record->slots[3].control.value = 0xFF;
        record->slots[1].control.value = 0xFF;
        record->slots[8].control.unkA = 0;
        record->slots[2].control.unk8 = 0;
        record->slots[2].unk4 = 0;
        record->slots[2].unk6 = 1;
        record->slots[3].unk4 = 0;
        record->slots[3].unk6 = 1;
        record->slots[8].unk4 = 0;
        record->slots[8].unk6 = 16;
        record->slots[0].control.value = 0xFF;
        record->slots[8].control.value = 0xFF;
        record->slots[3].control.unk8 = 0;
        record->slots[1].control.unk8 = 0;
        record->slots[0].control.unk8 = 0;
        record->slots[8].control.unk8 = 0;
        record->slots[9].animation = 0x28;
        record->slots[10].animation = 0x29;
        record->slots[9].unk4 = 0;
        record->slots[9].unk6 = 16;
        record->slots[10].unk4 = 0;
        record->slots[10].unk6 = 16;
        record->slots[9].control.unkA = 0;
        record->slots[9].control.value = 0xFF;
        record->slots[10].control.unkA = 0;
        record->slots[10].control.value = 0xFF;
        record->slots[9].control.unk8 = 0;
        record->slots[10].control.unk8 = 0;
    }

    for (i = 0; i < FIELD_ACTION_MAP_COUNT; i++)
    {
        g_field_action_animation_maps[i].animation_ids[5] = 0x585;
        g_field_action_animation_maps[i].animation_ids[7] = 0x785;
        g_field_action_animation_maps[i].animation_ids[4] = 0x485;
        g_field_action_animation_maps[i].animation_ids[6] = 0x685;
    }
}
