#ifndef CLOAD_INTERNAL_H
#define CLOAD_INTERNAL_H

#include "field_text.h"
#include "cload.h"
#include "saved_game.h"
#include "common.h"
#include "gpu_packet.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"
#include "sdk/libmcx.h"
#include "encoded_text.h"
#include "save_file.h"
#include "glyph_cache.h"
#include "card_events.h"
#include "card_directory.h"

/**
 * @brief Draw callback of a CLOAD UI element: emits the element's content at
 *        the given transition offsets and returns the advanced primitive cursor.
 */
typedef void *(*CloadElementDrawFunc)(u_long *ot, void *prim, s32 x_offset, s32 y_offset);

/**
 * @brief One animated CLOAD UI element (a framed window plus its content).
 *
 * The nine-bit window width straddles the two state words: its low eight bits
 * are the top byte of attr and its high bit is size.f.width_high.  No bitfield
 * can span that boundary, so the low byte is always read and written through
 * attr.word (see CLOAD_ELEMENT_WIDTH and CLOAD_SET_ELEMENT_WIDTH_LOW).  The
 * height is likewise written through size.word (CLOAD_SET_ELEMENT_HEIGHT).
 */
typedef struct CloadElement CloadElement;
struct CloadElement
{
    union
    {
        u32 word;
        struct
        {
            u32 state : 3;
            u32 phase : 4;
            u32 x : 9;
            u32 y : 8;
            u32 width_low : 8;
        } f;
    } attr;
    union
    {
        u32 word;
        struct
        {
            u32 width_high : 1;
            u32 height : 8;
            u32 framed : 1;
        } f;
    } size;
    CloadElementDrawFunc draw;
};

/** @brief CloadElement.attr.f.state values. */
#define CLOAD_ELEMENT_FREE 0
#define CLOAD_ELEMENT_OPENING 1
#define CLOAD_ELEMENT_OPEN 2
#define CLOAD_ELEMENT_CLOSING 3
#define CLOAD_ELEMENT_CLOSED 4

/** @brief Number of phase steps an opening window takes to reach full size. */
#define CLOAD_ELEMENT_PHASE_STEPS 8

/** @brief Bit position of the width's low byte inside CloadElement.attr.word. */
#define CLOAD_ELEMENT_WIDTH_SHIFT 24

/** @brief Low eight bits of a CloadElement's window width. */
#define CLOAD_ELEMENT_WIDTH_LOW(element) ((element)->attr.word >> CLOAD_ELEMENT_WIDTH_SHIFT)

/**
 * @brief Full nine-bit window width of a CloadElement.
 * @param element Element whose width is read.
 * @param width_low The width's low byte, as read by CLOAD_ELEMENT_WIDTH_LOW.
 */
#define CLOAD_ELEMENT_WIDTH(element, width_low) ((s32)(((element)->size.f.width_high << 8) | (width_low)))

/** @brief Store the low eight bits of a CloadElement's window width. */
#define CLOAD_SET_ELEMENT_WIDTH_LOW(element, width)                                                                        \
    ((element)->attr.word = ((element)->attr.word & ((1 << CLOAD_ELEMENT_WIDTH_SHIFT) - 1)) | ((width) << CLOAD_ELEMENT_WIDTH_SHIFT))

/** @brief Bit position and mask of the height inside CloadElement.size.word. */
#define CLOAD_ELEMENT_HEIGHT_SHIFT 1
#define CLOAD_ELEMENT_HEIGHT_MASK (0xFF << CLOAD_ELEMENT_HEIGHT_SHIFT)

/** @brief Store the window height of a CloadElement. */
#define CLOAD_SET_ELEMENT_HEIGHT(element, height) \
    ((element)->size.word = ((element)->size.word & ~CLOAD_ELEMENT_HEIGHT_MASK) | ((height) << CLOAD_ELEMENT_HEIGHT_SHIFT))

/**
 * @brief Element-pool head viewed as the CD-load prompt element: the 0x0 state
 *        word split into its state/phase/x/code bitfields, plus the 0x4
 *        active/y sub-fields and the 0x8 draw callback.
 */
typedef struct
{
    union
    {
        u32 word;
        struct
        {
            u32 state : 3;
            u32 phase : 4;
            u32 x : 9;
            u32 code : 8;
        } f;
    } attr;
    u32 active : 1;
    u32 y : 8;
    u32 rest : 23;
    void *draw;
    s32 unused;
} CloadPromptElement;

/**
 * @brief 0x20-byte, word-aligned memory-card path scratch buffer.
 * The first six bytes are initialized from the "bu00:" device prefix before a
 * filename suffix is appended.
 */
typedef union
{
    u8 bytes[0x20];
    u32 align;
} CloadCardPathScratch;

/** @brief 0x68-byte, word-aligned scratch buffer used by cload_advance_load_sequence. */
typedef union
{
    u8 bytes[0x68];
    u32 align;
} CloadLoadScratch;

/** @brief 0x100-byte, word-aligned memory-card path buffer. */
typedef union
{
    u8 bytes[0x100];
    u32 align;
} CloadCardPathBuffer;

/** @brief 0x10-byte, word-aligned buffer initialized from the "bu00:*" search path. */
typedef union
{
    u8 bytes[0x10];
    u32 align;
} CloadCardSearchPathBuffer;

/** @brief Eight-byte, word-aligned memory-card path template. */
typedef union
{
    char text[8];
    u32 align[2];
} CloadCardPathTemplate;

/* CLOAD layout/state constants. */
#define CLOAD_ELEMENT_COUNT 8
#define CLOAD_CARD_COUNT 2
#define CLOAD_ELEMENT_STATE_MASK 7
#define CLOAD_ENTRY_GROUP_COUNT 8
#define CLOAD_MEMORY_CARD_BLOCK_BYTES 8192
/** @brief Bytes read to show an entry: the card header and the first 0x100 bytes of the saved game. */
#define CLOAD_ENTRY_READ_BYTES 0x280
/** @brief Bytes read to show an entry that is not a Legend of Mana save: its card header title and CLUT. */
#define CLOAD_ENTRY_TITLE_READ_BYTES 0x80
#define CLOAD_ENTRY_ROW_HEIGHT 14
#define CLOAD_GLYPH_CACHE_SLOTS 0x100
#define CLOAD_GLYPH_CACHE_COLUMNS 16
#define CLOAD_GLYPH_CACHE_ROW_MASK 0xF0
#define CLOAD_GLYPH_RASTER_BYTES 0x80
#define CLOAD_GLYPH_CACHE_USED 0x10000
#define CLOAD_GLYPH_RASTER_BUFFER_BYTES 0x8000
#define CLOAD_COLOR_WHITE 0xFFFFFF

/**
 * @brief Address of CLOAD text @p index, reached through its own u16 offset-table entry @p entry.
 * @note The table start is derived back from the entry symbol, like FIELD_UI_TEXT_AT.
 */
#define CLOAD_TEXT_AT(entry, index) ((u8 *)&(entry) - (index) * 2 + (entry))

/**
 * @brief Address of FIELD UI string @p index, given its two-byte offset entry @p entry.
 * @note The table start is derived back from the entry symbol, as in FIELD's own lookups.
 */
#define FIELD_UI_TEXT_AT(entry, index) ((entry) - (index) * 2 + (entry)[0] + ((entry)[1] << 8))

/**
 * @brief Address of FIELD UI string @p index, given the start of the offset table in @p table.
 * @note Summed as integers, offset bytes first, like FIELD's own string lookups.
 */
#define FIELD_UI_TEXT(table, index) ((u8 *)((table)[(index) * 2] + (((table)[(index) * 2 + 1] << 8) + (s32)(table))))

/** @brief Start of the CLOAD text offset table, derived from entry @p entry at @p index. */
#define CLOAD_TEXT_TABLE(entry, index) (&(entry) - (index))

/** @brief Address of CLOAD text @p index in the u16 offset table starting at @p table. */
#define CLOAD_TEXT(table, index) ((u8 *)(table) + (table)[index])

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

/**
 * @brief One half of CLOAD's double-buffered GPU render state.
 *
 * The 0x7CC4-byte stride and the SDK object boundaries are recovered from the
 * fixed offsets used by cload_run_menu_loop/cload_init_display.  The trailing
 * region is still unknown and is intentionally left opaque.
 */
typedef struct
{
    /* 0x0000 */ u8 header[0x40];
    /* 0x0040 */ u_long ordering_table[0x1000];
    /* 0x4040 */ DISPENV disp_env;
    /* 0x4054 */ DRAWENV draw_env;
    /* 0x40B0 */ RECT clear_rect;
    /* 0x40B8 */ CloadGpuPacket *prim_cursor;
    /* 0x40BC */ u8 trailing[0x3C08];
} CloadRenderBuffer;

extern s32 g_pad_input;
extern s32 g_cload_exit_requested;
extern CloadElement g_cload_element_pool[CLOAD_ELEMENT_COUNT];
extern CloadRenderBuffer g_cload_render_buffers[CLOAD_CARD_COUNT];
extern s32 g_cload_io_busy;
extern u8 *g_cload_icon_resource;
extern s32 g_cload_scroll_y;
extern s32 g_cload_icon_palette;
extern s32 g_cload_progress_active;
extern s32 g_cload_scroll_target_y;
extern s32 g_cload_icon_phase;
extern u_long g_cload_icon_context[8];
extern u8 g_cload_primitive_buffers[CLOAD_CARD_COUNT][0x4000];
extern s32 g_cload_selected_row;
extern s32 g_cload_result;
extern s32 g_cload_scroll_frames;
extern s32 g_cload_selection_status;
extern s32 g_cload_frame_parity;
extern s32 D_80162370;
/**
 * @brief Start of the selected entry's save file: only the card header and the
 *        first 0x100 bytes of the saved game are read (CLOAD_ENTRY_READ_BYTES).
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
extern char g_lom_pocketstation_filename_prefix[];
extern char g_new_save_entry_prefix[];
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
extern u8 g_text_time_separator_offset_bytes[2];
extern u16 g_cload_location_names[];

/* Globals used by the memory-card I/O, load-state, and glyph-cache block. */
extern u8 g_text_choice_glyph_offsets[];
extern char g_lom_save_dummy_filename[];
extern char g_lom_pocketstation_dummy_filename[];
extern u8 g_cload_steps_idle[];
extern u8 g_cload_steps_read_selected_header[];
extern char g_cload_selected_card_path[0x40];
extern const CloadCardPathTemplate g_cload_card_path_prefix;
extern const CloadCardPathTemplate g_cload_card_search_path;
extern s32 g_cload_retry_count;
extern s32 g_cload_primary_poll_countdown;
extern s32 g_cload_entry_value_limit;
extern s32 g_cload_selected_entry_extended;
extern s32 g_cload_secondary_poll_countdown;
extern s32 g_cload_file_handle;

extern int strncmp(char *, char *, int);
void *func_800A88A0(void *prim, u_long *ot, u8 *text, s32 color, s32 x, s32 y, s32 mode);
void* func_800A8A78(u_long* ot, void* prim, s32 value, s32 color, DVECTOR* pos, s32 mode);

/* External callees used by the memory-card I/O/load-state block. */
/* strncmp is declared above with the original visible signature. */
s32 open(void *, s32);
s32 read(s32, void *, s32);
s32 close(s32);
void erase(void *);
s32 _card_info(s32);
s32 _card_load(s32);
s32 _card_wait(s32);
s32 _card_clear(s32);
void EnterCriticalSection(void);
void func_800A55E4(void *clut, s32 palette);
u8 *Krom2RawAdd(u16 sjis_code);
s32 field_upload_image_resource(RECT *rect, void *resource, s32 mode);
s32 cdrom_queue_read(s32 resource_index, void *dst_buffer);
void cdrom_wait_queue_empty(void);
void func_800A5638(void *clut, s32 icon);
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
s32 cload_update_frame(CloadRenderBuffer *frame);
void cload_build_ui_elements(void);
void cload_update_menu(CloadRenderBuffer *frame);
void cload_update_load_sequence(void);
s32 cload_handle_input(void);
void cload_close_all_elements(void);
void cload_scroll_to_selection(void);
void cload_update_elements(CloadRenderBuffer *frame);
void* cload_draw_entry_list(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void *cload_draw_header_label(u_long *ot, void *prim, s32 x_offset, s32 y_offset);
void *cload_draw_card_slot0_label(u_long *ot, void *prim, s32 x_offset, s32 y_offset);
void *cload_draw_card_slot1_label(u_long *ot, void *prim, s32 x_offset, s32 y_offset);
void* cload_draw_selected_entry_details(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void cload_clear_elements(void);
CloadElement *cload_alloc_element(void);
void cload_update_and_draw_elements(CloadRenderBuffer* frame);
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
