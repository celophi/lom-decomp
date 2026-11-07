#ifndef FIELD_OVERLAY_HANDOFF_H
#define FIELD_OVERLAY_HANDOFF_H

#include "common.h"

#include "common/saved_game.h"
#include "main/overlay_memory.h"

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

/**
 * @brief Results CARDA leaves in g_field_card_overlay_mode for FIELD.
 * @note FIELD sets it to the mode plus one before CARDA starts, so a finished
 *       save leaves CARDA_RESULT_SAVED and a finished load CARDA_RESULT_LOADED.
 */
#define CARDA_RESULT_SAVED 1
#define CARDA_RESULT_LOADED 2
#define CARDA_RESULT_CANCELLED 3
#define CARDA_RESULT_PET_SENT 4
#define CARDA_RESULT_PET_RETURNED 5
#define CARDA_RESULT_SEND_PET_CANCELLED 6
#define CARDA_RESULT_RETURN_PET_CANCELLED 7 /**< Also set when the ranch has no free pet slot. */

/** @brief Ranch pet slot CARDA transfers to or from a PocketStation. */
extern s32 g_field_card_pet_slot;

/**
 * @brief NIKI/ADDHERO run flag: FIELD sets 1 when it opens either and 0 when it closes; NIKI stores 3 on cancel.
 * @note Every reader only tests it for non-zero.
 */
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

/** @brief GNAME history-list entry: g_script_repeat_last for an item, the character's info.actions.variant (+0x80 for a golem) for a character. */
extern s32 g_field_rename_source;

/** @brief Extra text GNAME offers for an item rename (the item's type name). */
extern u8 g_field_rename_custom_name[];

/** @brief After an item or character rename FIELD runs script g_field_rename_script_index + 1: the item category, or 3 + the character's party position. */
extern s32 g_field_rename_script_index;

/**
 * @brief Outcome of the last NIKI run: 0 none or written back, 1 save loaded, 2 failed or declined.
 * @note Field scripts wait on it through opcode 0x14 selectors 3 and 4.
 */
extern s32 g_field_niki_state;

/** @brief Item records exchanged with the trade partner's SavedGameLayout::pending_items. */
extern FieldItemRecord g_field_shared_items[SAVED_GAME_PENDING_ITEM_COUNT];

/** @brief SaveIdentity of the trade partner's save; NIKI writes back only to a save with the same ids. */
extern s32 g_field_niki_partner_game_id;
extern s32 g_field_niki_partner_save_id;

/** @brief Card slot holding the trade partner's save. */
extern s32 g_field_niki_card_slot;

/** @brief Card path of the trade partner's save. */
extern u8 g_field_niki_save_path[];

struct FieldRenderHalf;

/** @brief Render buffers FIELD hands to golem_run. */
#define FIELD_GOLEM_RENDER_BUFFERS ((void*)SECONDARY_OVERLAY_AT(0x10000))

/**
 * @brief Entry point of the loaded secondary overlay: gname_run, shop_init or gover_show_screen.
 * @note Declared without parameters: the three overlays take different parameter lists.
 */
void secondary_overlay_entry();

/**
 * @brief niki_init or addhero_init, whichever overlay is loaded (both start at the same address).
 * @param work Work buffer.
 * @param mode Mode FIELD opens the overlay in.
 */
void niki_addhero_init(void* work, s32 mode);

/**
 * @brief NIKI's card_menu_update_frame at its load address.
 * @param render FIELD's render half for the frame.
 * @return Nonzero once NIKI has finished.
 */
s32 niki_update_frame(struct FieldRenderHalf* render);

/**
 * @brief CARDA's card_menu_update_frame at its load address.
 * @param render FIELD's render half for the frame.
 * @return Nonzero once CARDA has finished.
 */
s32 carda_update_frame(struct FieldRenderHalf* render);

#endif
