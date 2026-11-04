#ifndef FIELD_OVERLAY_HANDOFF_H
#define FIELD_OVERLAY_HANDOFF_H

#include "common.h"

/**
 * @file field_overlay_handoff.h
 * @brief FIELD-resident state passed between FIELD and the menu overlays it
 *        loads into the secondary overlay area (GOSUB, CARDA, NIKI and ADDHERO).
 */

/** @brief Progress of the running menu overlay as FIELD scripts see it: 1 while it runs, 2 once it has finished. */
extern s32 g_field_gosub_state;

/** @brief Number of values the menu overlay left in g_gosub_result_values. */
extern s32 g_gosub_result_count;

/** @brief Values a menu overlay hands back to FIELD: selected indexes and materials, or the transferred pet slot. */
extern s32 g_gosub_result_values[];

/** @brief CARDA overlay mode plus one while CARDA is open, 0 otherwise; CARDA replaces it with its CARDA_RESULT_* code. */
extern s32 g_field_card_overlay_mode;

/** @brief Ranch pet slot CARDA transfers to or from a PocketStation. */
extern s32 g_field_card_pet_slot;

/** @brief NIKI and ADDHERO progress: FIELD sets 1 when it opens them. TODO: later values unknown. */
extern s32 g_field_niki_addhero_state;

/**
 * @brief menu_run results: the screen FIELD opens once MENU closes. The rename
 *        results are also the GNAME source mode (the Equipment and Ability page
 *        renames use mode FIELD_MENU_NAME_ITEM).
 */
#define FIELD_MENU_CLOSED 0
#define FIELD_MENU_NAME_ITEM 1
#define FIELD_MENU_NAME_CHARACTER 3
#define FIELD_MENU_GOLEM 10
#define FIELD_MENU_NAME_EQUIPMENT_ITEM 11
#define FIELD_MENU_NAME_ABILITY_ITEM 12

/** @brief Name GNAME starts from when MENU requests a rename. */
extern u8 g_field_rename_initial_name[];

/** @brief Record whose name GNAME edits: an item record or a FieldCharacterRecord. */
extern u8* g_field_rename_target;

/** @brief GNAME history-list entry: g_script_repeat_last for an item, the character's info.actions.unk19 (+0x80 for a golem) for a character. */
extern s32 g_field_rename_source;

/** @brief Extra text GNAME offers for an item rename (the item's type name). */
extern u8 g_field_rename_custom_name[];

/** @brief After an item or character rename FIELD runs script g_field_rename_script_index + 1: the item category, or 3 + the character's party position. */
extern s32 g_field_rename_script_index;

#endif
