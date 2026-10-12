#ifndef CLOAD_INTERNAL_H
#define CLOAD_INTERNAL_H

#include "overlays/field/field_text.h"
#include "overlays/field/field_ui_text.h"
#include "overlays/cload/cload.h"
#include "common/saved_game.h"
#include "common.h"
#include "common/gpu_packet.h"
#include <libgte.h>
#include <libgpu.h>
#include <kernel.h>
#include <libapi.h>
#include <libmcx.h>
#include <strings.h>
#include "common/encoded_text.h"
#include "common/save_file.h"
#include "common/glyph_cache.h"
#include "common/card_events.h"
#include "common/card_directory.h"
#include "common/card_menu.h"
#include "overlays/field/field_sound.h"
#include "main/audio/akao.h"
#include "overlays/field/field_fade.h"
#include "main/cd_resources.h"
#include "main/cdrom.h"
#include "main/main.h"
/** @brief Bit position and mask of the height inside CardMenuElement.size.word. */
#define CLOAD_ELEMENT_HEIGHT_SHIFT 1
#define CLOAD_ELEMENT_HEIGHT_MASK (0xFF << CLOAD_ELEMENT_HEIGHT_SHIFT)

/** @brief Store the window height of a CardMenuElement. */
#define CLOAD_SET_ELEMENT_HEIGHT(element, height) \
    ((element)->size.word = ((element)->size.word & ~CLOAD_ELEMENT_HEIGHT_MASK) | ((height) << CLOAD_ELEMENT_HEIGHT_SHIFT))

/**
 * @brief Stored height of a CLOAD window that covers @p height pixels.
 * @note cload_emit_window_frame clips its contents to h - 3 rows rather than
 *       h - 4, so CLOAD's windows store one less than the FIELD-launched card
 *       menus' windows of the same size.
 */
#define CLOAD_WINDOW_HEIGHT(height) ((height) - 1)

/** @brief Stored height of CLOAD's status dialog for @p lines lines of text. */
#define CLOAD_DIALOG_HEIGHT(lines) CLOAD_WINDOW_HEIGHT((lines) * FIELD_TEXT_LINE_SPACING)

/** @brief Entry list window (left edge and width as CARD_MENU_EXCHANGE_LIST_*). */
#define CLOAD_LIST_Y 74
#define CLOAD_LIST_HEIGHT 73
#define CLOAD_LIST_LAST_ROW_Y CARD_MENU_LAST_ROW_Y(CLOAD_LIST_HEIGHT)

/** @brief Top of the message window (left edge, width and height as CARD_MENU_MESSAGE_*). */
#define CLOAD_MESSAGE_Y 91

/** @brief Height of the load progress bar drawn in the message window. */
#define CLOAD_PROGRESS_BAR_HEIGHT 53

/**
 * @brief Entries of CLOAD's ordering table, which starts at FIELD's fade entry
 *        (FIELD_FADE_OT_INDEX) of each render half so that the fade covers
 *        everything CLOAD draws.
 */
#define CLOAD_OT_SIZE (FIELD_ORDERING_TABLE_SIZE - FIELD_FADE_OT_INDEX)

extern FieldRenderHalf g_cload_render_buffers[CARD_SLOT_COUNT];
extern u8 *g_cload_icon_resource;
extern u8 g_cload_primitive_buffers[CARD_SLOT_COUNT][0x4000];
extern s32 g_cload_result;
/**
 * @brief Start of the selected entry's save file: only the card header and the
 *        first 0x100 bytes of the saved game are read (CARD_MENU_ENTRY_READ_BYTES).
 */
extern SaveFile g_cload_selected_file;
extern u16 g_cload_text_new_save_prompt;
extern u16 g_cload_text_load;
extern u16 g_cload_text_loading;
/** @brief Save file read by the load sequence. */
extern SaveFile g_cload_save_file;

extern s32 g_playtime_vsync_origin;
extern u16 g_cload_text_version_error;

/* FIELD and main-executable entry points used by CLOAD. */
s32 field_upload_image_resource(RECT *rect, void *resource, s32 mode);

/* Functions shared across the CLOAD translation units. */
void cload_run_menu_loop(void);
void cload_init_display(void);
void cload_build_ui_elements(void);
void card_menu_update_card_sequence(void);
s32 card_menu_handle_input(void);
void cload_close_all_elements(void);
void cload_scroll_to_selection(void);
void* cload_draw_entry_list(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void *cload_draw_header_label(u_long *ot, void *prim, s32 x_offset, s32 y_offset);
void *cload_draw_card_slot0_label(u_long *ot, void *prim, s32 x_offset, s32 y_offset);
void *cload_draw_card_slot1_label(u_long *ot, void *prim, s32 x_offset, s32 y_offset);
void* cload_draw_selected_entry_details(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void cload_clear_elements(void);
CardMenuElement *cload_alloc_element(void);
void *cload_emit_window_frame(void *prim, u_long *ot, s32 x, s32 y, s32 w, s32 h, s32 front_buffer, s32 draw_fill);
void *cload_emit_rect_outline(LINE_F2 *line, u_long *ot, s32 x, s32 y, s32 w, s32 h, s32 color);
void *cload_emit_scroll_arrow(SPRT *sprite, u_long *ot, s32 x, s32 y, s32 upper);
void *cload_draw_load_prompt(u_long *ot, void *prim, s32 x_offset, s32 y_offset);
void *cload_draw_load_progress(u_long *ot, void *prim, s32 x_offset, s32 y_offset);
void *cload_draw_progress_bar(POLY_G4 *quad, u_long *ot);
void cload_open_status_dialog(s32 dialog_state);
void *cload_draw_status_dialog(u_long *ot, void *prim, s32 x_offset, s32 y_offset);
void *cload_draw_icon_highlight(POLY_FT4 *quad, u_long *ot, s32 x, s32 y, s32 width, s32 icon, s32 index, s32 row);
void cload_load_icon_resources(void);
void *cload_draw_backdrop(SPRT *sprite, u_long *ot);
s32 cload_enable_choice_toggle(void);
void* cload_draw_choice_prompt(void* prim, u_long* ot, s32 x, s32 y);
s32 cload_advance_load_sequence(void);
void cload_restart_load_sequence(void);
void cload_init_card_events(void);
s32 cload_begin_entry_scan(s32 slot);
s32 cload_scan_next_entry(s32 slot);
void cload_commit_selected_entry(void);

#endif /* CLOAD_INTERNAL_H */
