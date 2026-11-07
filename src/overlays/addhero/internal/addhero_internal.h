#ifndef ADDHERO_INTERNAL_H
#define ADDHERO_INTERNAL_H

#include "common.h"
#include <kernel.h>
#include <libapi.h>
#include <libetc.h>
#include <libgpu.h>
#include <libgte.h>
#include <libmcx.h>
#include <strings.h>

#include "common/card_directory.h"
#include "common/card_events.h"
#include "common/card_menu.h"
#include "common/encoded_text.h"
#include "common/glyph_cache.h"
#include "common/gpu_packet.h"
#include "common/pad.h"
#include "common/save_file.h"
#include "common/saved_game.h"
#include "common/vector.h"
#include "main/controller.h"
#include "main/display.h"
#include "overlays/field/field_fade.h"
#include "overlays/field/field_input.h"
#include "overlays/field/field_menu_window.h"
#include "overlays/field/field_sound.h"
#include "overlays/field/field_ui_text.h"
#include "overlays/addhero/addhero.h"

/* Declarations shared by ADDHERO implementation files. */

/** @brief Save file read or written by the load and save sequences. */
extern SaveFile g_addhero_save_file;
/**
 * @brief Start of the selected entry's save file: only the card header and the
 *        first 0x100 bytes of the saved game are read (CARD_MENU_ENTRY_READ_BYTES).
 */
extern SaveFile g_addhero_entry_file;

void addhero_open_exit_dialog(s32 message_id);
s32 addhero_scan_next_entry(s32 slot);
s32 addhero_advance_load_sequence(void);

/** @brief g_addhero_result values, reported to the host when the overlay exits. */
#define ADDHERO_RESULT_LOADED 1
#define ADDHERO_RESULT_SAVED 2
#define ADDHERO_RESULT_CANCELLED 3

extern s32 g_save_compatibility_tag;

/**
 * @brief FIELD data word next to g_field_item_drop_menu_open; ADDHERO stores ADDHERO_RESULT_CANCELLED into it when Circle cancels the browser.
 * @note No code in the main executable or any overlay of either version reads it.
 */
extern s32 g_field_addhero_cancel_result;
extern s32 g_pad_input;
/** @brief The saved game's item records (g_saved_game_ctx->items). */
extern FieldItemRecord* g_addhero_items;
extern s32 g_addhero_result;
/** @brief Work buffer FIELD passed to addhero_init; stored but never read. */
extern void* g_addhero_work_buffer;
extern s32 g_addhero_load_flow_active;
extern u16 g_addhero_text_saving;
extern u16 g_addhero_text_new_save_title;
extern u16 g_addhero_text_loading;
extern u16 g_addhero_text_same_hero_data;
extern u16 g_addhero_text_wrong_version;
extern u16 g_addhero_text_2p_data_not_saved;
extern u16 g_addhero_text_no_load_file;
extern u16 g_addhero_text_found_load_file;

/* Overlay function declarations. */
void addhero_build_ui_elements(void);
s32 card_menu_update_card_sequence(void);
s32 card_menu_handle_input(void);
void addhero_reset_state(void);
void* addhero_draw_selected_entry_details(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void addhero_clear_elements(void);
void* card_menu_draw_transfer_window(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* addhero_draw_exit_dialog(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* addhero_draw_transfer_status(u_long* ot, void* prim, s32 x_offset, s32 y_offset);

#endif
