#ifndef ADDHERO_INTERNAL_H
#define ADDHERO_INTERNAL_H

#include "common.h"
#include "common/saved_game.h"
#include "common/pad.h"
#include "common/vector.h"
#include "main/display.h"
#include "common/gpu_packet.h"
#include <libgte.h>
#include <libgpu.h>
#include <kernel.h>
#include <libapi.h>
#include <libetc.h>
#include <strings.h>
#include <libmcx.h>
#include "main/controller.h"
#include "overlays/field/field_menu_window.h"
#include "overlays/field/field_sound.h"
#include "overlays/field/field_ui_text.h"
#include "common/encoded_text.h"
#include "common/save_file.h"
#include "common/glyph_cache.h"
#include "common/card_events.h"
#include "common/card_directory.h"
#include "common/card_menu.h"
#include "overlays/field/field_fade.h"
#include "overlays/field/field_input.h"

/* Declarations shared by ADDHERO implementation files. */

/**
 * @brief g_card_entry_state values.
 *
 * Below CARD_MENU_ENTRY_COUNT_LIMIT the value is the number of save entries
 * read from the current card (a card holds 15). From 0xF3 up it is a status
 * whose message the list or transfer window shows instead of the entries; the
 * states from 0xF8 up are the shared CARD_MENU_ENTRY_STATE_* values.
 */
#define ADDHERO_ENTRY_STATE_CONFIRM_NO_SAVE 0xF3    /**< Asks whether to leave without saving the 2P data. */
#define ADDHERO_ENTRY_STATE_SAVE_CONFIRM 0xF4       /**< A load file was found; asks whether to overwrite the 2P data. */
#define ADDHERO_ENTRY_STATE_SAVE_PROGRESS 0xF5      /**< Writing the save; progress bar. */
#define ADDHERO_ENTRY_STATE_LOAD_PROGRESS 0xF6      /**< Reading the selected save; progress bar. */
#define ADDHERO_ENTRY_STATE_NO_LOAD_FILE 0xF7       /**< The selected entry is not a load file. */

extern struct DIRENTRY g_card_entries[][CARD_DIRECTORY_ENTRY_COUNT];
extern s32 g_addhero_progress_active;
extern s32 g_addhero_mode;
extern s32 g_addhero_selection_status;
extern s32 g_addhero_io_busy;
extern s32 g_addhero_progress_bar_active;
extern s32 g_addhero_progress_start_tick;
extern s32 g_addhero_entry_scan_active;
extern s32 g_addhero_write_in_progress;
extern s32 g_addhero_selected_entry_extended;
extern s32 g_addhero_has_free_entry_space;
extern u8 g_addhero_loadseq_start;
/** @brief Save file read or written by the load and save sequences. */
extern SaveFile g_addhero_save_file;
/**
 * @brief Start of the selected entry's save file: only the card header and the
 *        first 0x100 bytes of the saved game are read (CARD_MENU_ENTRY_READ_BYTES).
 */
extern SaveFile g_addhero_entry_file;
extern char g_addhero_save_file_path[];

void addhero_open_status_dialog(s32 message_id);
void addhero_open_exit_dialog(s32 message_id);
s32 addhero_begin_entry_scan(s32 page);
s32 addhero_scan_next_entry(s32 page);
void addhero_init_card_events(void);
void addhero_commit_selected_entry(void);
s32 addhero_advance_load_sequence(void);

/** @brief ADDHERO's own g_addhero_dialog_state message, after the shared CARD_MENU_DIALOG_* ones; it shows the load-failed text. */
#define ADDHERO_DIALOG_INVALID_SAVE 4

/** @brief g_addhero_result values, reported to the host when the overlay exits. */
#define ADDHERO_RESULT_LOADED 1
#define ADDHERO_RESULT_SAVED 2
#define ADDHERO_RESULT_CANCELLED 3


extern s32 g_save_compatibility_tag;

/**
 * @brief FIELD data word next to g_field_item_drop_menu_open; ADDHERO stores 3 into it when Circle cancels the browser.
 * @note TODO: purpose unknown; no code in the main executable or any overlay reads it.
 */
extern s32 D_80122718;
extern s32 g_pad_input;
extern u8 g_addhero_loadseq_done[];
extern s32 g_addhero_icon_phase;
/** @brief The saved game's item records (g_saved_game_ctx->items). */
extern FieldItemRecord* g_addhero_items;
extern s32 g_addhero_result;
extern s32 g_addhero_work_ram_base;
extern s32 g_addhero_load_flow_active;
extern s32 g_addhero_dialog_state;
extern u8 g_addhero_loadseq_abort[];
extern u8 g_addhero_loadseq_load_begin[];
extern u8 g_addhero_loadseq_load_progress[];
extern u8 g_addhero_loadseq_save_begin[];
extern u16 g_addhero_text_table;
extern u16 g_addhero_text_not_enough_blocks;
extern u16 g_addhero_text_no_card;
extern u16 g_addhero_text_mana_label;
extern u16 g_addhero_text_other_game_label;
extern u16 g_addhero_text_card_access_failed;
extern u16 g_addhero_text_no_save_data;
extern u16 g_addhero_text_new_save_label;
extern u16 g_addhero_text_saving;
extern u16 g_addhero_text_new_save_title;
extern u16 g_addhero_text_number_label;
extern u16 g_addhero_text_load_prompt;
extern u16 g_addhero_text_loading;
extern u16 g_addhero_text_no_game_save_data;
/** @brief List label of a PocketStation (Ring Ring Land) save; empty in the US release. */
extern u16 g_addhero_text_ring_ring_land_label;
extern u16 g_addhero_text_save_failed;
extern u16 g_addhero_text_load_failed;
extern u16 g_addhero_text_card_not_inserted;
/** @brief "Not a PocketStation" dialog text; empty in the US release. */
extern u16 g_addhero_text_not_pocketstation;
extern u16 g_addhero_text_select_save_data;
extern u16 g_addhero_text_select_item;
extern u16 g_addhero_text_same_hero_data;
extern u16 g_addhero_text_wrong_version;
extern u16 g_addhero_text_2p_data_not_saved;
extern u16 g_addhero_text_no_load_file;
extern u16 g_addhero_text_found_load_file;
extern u16 g_addhero_text_plus_marker;
extern u16 g_addhero_location_text_table[];

/* Overlay function declarations. */
void addhero_init(s32 work_base, s32 mode);
s32 addhero_state_step(FieldRenderHalf* draw_state);
void addhero_build_ui_elements(void);
s32 addhero_update_load_sequence(void);
s32 addhero_handle_input(void);
void addhero_reset_state(void);
void* addhero_draw_entry_list(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* addhero_draw_mode_glyph(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* addhero_draw_selected_entry_details(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void addhero_clear_elements(void);
void* addhero_draw_load_prompt(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* addhero_draw_load_progress(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* addhero_draw_progress_bar(POLY_G4* quad, u_long* ot);
void* addhero_draw_status_dialog(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* addhero_draw_exit_dialog(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* addhero_draw_transfer_status(u_long* ot, void* prim, s32 x_offset, s32 y_offset);

#endif
