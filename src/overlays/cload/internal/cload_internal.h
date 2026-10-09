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
#include <libmcx.h>
#include "common/encoded_text.h"
#include "common/save_file.h"
#include "common/glyph_cache.h"
#include "common/card_events.h"
#include "common/card_directory.h"
#include "common/card_menu.h"
#include "overlays/field/field_sound.h"
#include "main/audio/akao.h"
#include "overlays/field/field_fade.h"
/** @brief Bit position and mask of the height inside CardMenuElement.size.word. */
#define CLOAD_ELEMENT_HEIGHT_SHIFT 1
#define CLOAD_ELEMENT_HEIGHT_MASK (0xFF << CLOAD_ELEMENT_HEIGHT_SHIFT)

/** @brief Store the window height of a CardMenuElement. */
#define CLOAD_SET_ELEMENT_HEIGHT(element, height) \
    ((element)->size.word = ((element)->size.word & ~CLOAD_ELEMENT_HEIGHT_MASK) | ((height) << CLOAD_ELEMENT_HEIGHT_SHIFT))

/* CLOAD layout/state constants. */
#define CLOAD_GLYPH_CACHE_SLOTS 0x100
#define CLOAD_GLYPH_CACHE_COLUMNS 16
#define CLOAD_GLYPH_CACHE_ROW_MASK 0xF0
#define CLOAD_GLYPH_RASTER_BYTES 0x80
#define CLOAD_GLYPH_CACHE_USED 0x10000
#define CLOAD_GLYPH_RASTER_BUFFER_BYTES 0x8000
#define CLOAD_COLOR_WHITE 0xFFFFFF

/** @brief Generic GPU packet prefix used while advancing the primitive buffer. */
typedef struct
{
    /* 0x0 */ s32 tag;
    /* 0x4 */ s32 word4;
    /* 0x8 */ s16 x0;
    /* 0xA */ s16 y0;
    /* 0xC */ s16 unkC;
    /* 0xE */ u16 unkE;
} CloadGpuPacket;

extern s32 g_pad_input;
extern s32 g_cload_exit_requested;
extern CardMenuElement g_cload_element_pool[CARD_MENU_ELEMENT_COUNT];
/**
 * @brief Entries of CLOAD's ordering table, which starts at FIELD's fade entry
 *        (FIELD_FADE_OT_INDEX) of each render half so that the fade covers
 *        everything CLOAD draws.
 */
#define CLOAD_OT_SIZE (FIELD_ORDERING_TABLE_SIZE - FIELD_FADE_OT_INDEX)

extern FieldRenderHalf g_cload_render_buffers[CARD_SLOT_COUNT];
extern s32 g_cload_io_busy;
extern u8 *g_cload_icon_resource;
extern s32 g_cload_scroll_y;
extern s32 g_cload_icon_palette;
extern s32 g_cload_progress_active;
extern s32 g_cload_scroll_target_y;
extern s32 g_cload_icon_phase;
extern u8 g_cload_icon_context[GPU_CLUT_4BIT_COLORS * 2];
extern u8 g_cload_primitive_buffers[CARD_SLOT_COUNT][0x4000];
extern s32 g_cload_selected_row;
extern s32 g_cload_result;
extern s32 g_cload_scroll_frames;
extern s32 g_cload_selection_status;
extern s32 g_cload_frame_parity;
extern s32 D_80162370;
/**
 * @brief Start of the selected entry's save file: only the card header and the
 *        first 0x100 bytes of the saved game are read (CARD_MENU_ENTRY_READ_BYTES).
 */
extern SaveFile g_cload_selected_file;
extern s32 g_cload_element1_state;
extern u8 g_cload_steps_initial_scan[];
extern u8 g_cload_steps_refresh_entries[];
extern u8 g_cload_steps_card_reset[];
extern u8 g_cload_steps_load_selected_save[];
extern s32 g_cload_choice_toggle;
extern u8 *g_cload_load_step;
extern s32 g_save_compatibility_tag;
extern s32 g_cload_entry_scan_active;
extern u16 g_cload_text_check_memory_card;
extern u16 g_cload_text_not_enough_blocks;
extern u16 g_cload_text_no_memory_card;
extern u16 g_cload_text_mana;
extern u16 g_cload_text_other_game;
extern u16 g_cload_text_card_slot_1;
extern u16 g_cload_text_card_slot_2;
extern u16 g_cload_text_card_access_failed;
extern u16 g_cload_text_no_save_data;
extern u16 g_cload_text_new_save;
extern u16 g_cload_text_new_save_prompt;
extern u16 g_cload_text_load;
extern u16 g_cload_text_number_prefix;
extern u16 g_cload_text_load_prompt;
extern u16 g_cload_text_loading;
/** @brief Save file read by the load sequence. */
extern SaveFile g_cload_save_file;

extern s32 g_playtime_vsync_origin;
extern s32 g_cload_progress_bar_active;
extern s32 g_cload_progress_start_tick;
extern s32 g_cload_dialog_state;
extern u16 g_cload_text_no_lom_save_data;
extern u16 g_cload_text_alt_save;
extern u16 g_cload_text_save_failed;
extern u16 g_cload_text_load_failed;
extern u16 g_cload_text_card_insert_error;
extern u16 D_80145EDE;
extern u16 g_cload_text_version_error;
extern u16 g_cload_text_plus_marker;
extern s32 g_cload_rank_count;
extern s32 g_cload_entry_ranks[];
extern u16 g_cload_location_names[];

/* Globals used by the memory-card I/O, load-state, and glyph-cache block. */
extern u8 g_cload_steps_idle[];
extern u8 g_cload_steps_read_selected_header[];
extern char g_cload_selected_card_path[0x40];
extern s32 g_cload_retry_count;
extern s32 g_cload_primary_poll_countdown;
extern s32 g_cload_entry_value_limit;
extern s32 g_cload_selected_entry_extended;
extern s32 g_cload_secondary_poll_countdown;
extern s32 g_cload_file_handle;

extern int strncmp(char *, char *, int);

/* External callees used by the memory-card I/O/load-state block. */
s32 open(void *, s32);
s32 read(s32, void *, s32);
s32 close(s32);
void erase(void *);
s32 _card_info(s32);
s32 _card_load(s32);
s32 _card_wait(s32);
s32 _card_clear(s32);
void EnterCriticalSection(void);
u8 *Krom2RawAdd(u16 sjis_code);
s32 field_upload_image_resource(RECT *rect, void *resource, s32 mode);
s32 cdrom_queue_read(s32 resource_index, void *dst_buffer);
void cdrom_wait_queue_empty(void);
void CloseEvent(s32);
void ExitCriticalSection(void);
s32 OpenEvent(u32, s32, s32, s32);
void EnableEvent(s32);
s32 TestEvent(s32);
s32 firstfile(void *, void *);
s32 nextfile(void *);
char *strcpy(char *, const char *);

/* Functions shared across the CLOAD translation units. */
s32 cload_main(void);
void cload_run_menu_loop(void);
void cload_init_display(void);
s32 cload_update_frame(FieldRenderHalf *frame);
void cload_build_ui_elements(void);
void cload_update_menu(FieldRenderHalf *frame);
void cload_update_load_sequence(void);
s32 cload_handle_input(void);
void cload_close_all_elements(void);
void cload_scroll_to_selection(void);
void cload_update_elements(FieldRenderHalf *frame);
void* cload_draw_entry_list(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void *cload_draw_header_label(u_long *ot, void *prim, s32 x_offset, s32 y_offset);
void *cload_draw_card_slot0_label(u_long *ot, void *prim, s32 x_offset, s32 y_offset);
void *cload_draw_card_slot1_label(u_long *ot, void *prim, s32 x_offset, s32 y_offset);
void* cload_draw_selected_entry_details(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void cload_clear_elements(void);
CardMenuElement *cload_alloc_element(void);
void cload_update_and_draw_elements(FieldRenderHalf* frame);
CloadGpuPacket *cload_emit_window_frame(CloadGpuPacket *prim, u_long *ot, s32 x, s32 y, s32 w, s32 h, s32 flag, s32 draw_fill);
CloadGpuPacket *cload_emit_rect_outline(LINE_F2 *line, u_long *ot, s32 x, s32 y, s32 w, s32 h, s32 color);
CloadGpuPacket *cload_emit_scroll_arrow(SPRT *sprite, u_long *ot, s32 x, s32 y, s32 flag);
void *cload_draw_load_prompt(u_long *ot, void *prim, s32 x_offset, s32 y_offset);
void *cload_draw_load_progress(u_long *ot, void *prim, s32 x_offset, s32 y_offset);
CloadGpuPacket *cload_draw_progress_bar(POLY_G4 *quad, u_long *ot);
void cload_open_status_dialog(s32 dialog_state);
void *cload_draw_status_dialog(u_long *ot, void *prim, s32 x_offset, s32 y_offset);
void *cload_draw_icon_highlight(POLY_FT4 *quad, u_long *ot, s32 x, s32 y, s32 width, s32 icon, s32 index, s32 row);
void cload_deactivate_primary_element(void);
void cload_load_icon_resources(void);
CloadGpuPacket *cload_emit_icon_highlight_strip(SPRT *sprite, u_long *ot);
s32 cload_enable_choice_toggle(void);
void* cload_draw_choice_prompt(void* prim, u_long* ot, s32 x, s32 y);
s32 cload_rank_entries(void);
void cload_reset_entry_ranks();
s32 cload_has_known_entry_type(void);
s32 cload_entry_blocks_reach_limit(void);
void cload_erase_fixed_card_files(void);
s32 cload_advance_load_sequence(void);
void cload_restart_load_sequence();
s32 cload_begin_entry_scan();
s32 cload_scan_next_entry();
void cload_commit_selected_entry();
void cload_sort_entries_by_type();

#endif /* CLOAD_INTERNAL_H */
