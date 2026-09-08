#include "common.h"
#include "pad.h"
#include "vector.h"
#include "display.h"
#include "gpu_packet.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"
#include "sdk/kernel.h"

/** @brief Animated panel or list element used by the ADDHERO interface. */
typedef struct AddheroElement
{
    union
    {
        u32 word;
        struct
        {
            u32 state : 3;
            u32 transition_step : 4;
            u32 x : 9;
            u32 y : 8;
            u32 width_low : 8;
        } bits;
        struct
        {
            u16 control_x;
            u8 y;
            u8 width_low;
        } bytes;
    } attr;
    union
    {
        u32 word;
        struct
        {
            u32 width_high : 1;
            u32 height : 8;
            u32 scrollable : 1;
            u32 reserved : 22;
        } bits;
    } size;
    void *draw_handler;
} AddheroElement;

typedef struct AddheroRecord
{
    u8 pad0[0x17];
    u8 marker_17;
    u8 pad18[0xCF - 0x18];
    u8 owner_id;
    u8 padD0[4];
    u16 hero_id;
    u16 reserved_d6;
    s32 identity;
} AddheroRecord;

/* ADDHERO layout/state constants recovered from the element and card-directory loops. */
#define ADDHERO_ELEMENT_COUNT 8
#define ADDHERO_ELEMENT_WORD_STRIDE 3
#define ADDHERO_ELEMENT_STATE_MASK 7
#define ADDHERO_ELEMENT_PHASE_MASK 0x78
#define ADDHERO_ELEMENT_STATE_INACTIVE 0
#define ADDHERO_ELEMENT_STATE_OPENING 1
#define ADDHERO_ELEMENT_STATE_ACTIVE 2
#define ADDHERO_ELEMENT_STATE_CLOSING 3
#define ADDHERO_ELEMENT_STATE_FINISHING 4
#define ADDHERO_DIRECTORY_ENTRY_COUNT 20
#define ADDHERO_DIRECTORY_ENTRY_BYTES sizeof(struct DIRENTRY)
#define ADDHERO_CARD_DIRECTORY_BYTES (ADDHERO_DIRECTORY_ENTRY_COUNT * ADDHERO_DIRECTORY_ENTRY_BYTES)
#define ADDHERO_ENTRY_ROW_HEIGHT 14
#define ADDHERO_ENTRY_COUNT_LIMIT 0x10
#define ADDHERO_NO_ICON 0x7F
#define ADDHERO_SAVE_FILENAME_PREFIX_LENGTH 12
#define ADDHERO_NEW_SAVE_FILENAME_PREFIX_LENGTH 8

/* Return codes produced by addhero_advance_load_sequence. */
#define ADDHERO_LOAD_RESULT_NONE 0
#define ADDHERO_LOAD_RESULT_PENDING 1
#define ADDHERO_LOAD_RESULT_ABORT 2
#define ADDHERO_LOAD_RESULT_CONTINUE 3
#define ADDHERO_LOAD_RESULT_COMPLETE 4
#define ADDHERO_LOAD_RESULT_CARD_ERROR 5

#define ADDHERO_LOAD_STEP_COUNT 31

/*
 * g_addhero_entry_state is dual-purpose:
 *
 *   0x00-0x0F  Number of save entries loaded from the current card. A PSX
 *              memory card holds 15 directory blocks, so the count never
 *              reaches 0x10; the value is used directly as a count/index
 *              while scanning and browsing the entry list.
 *   0xF3-0xFF  Modal state-machine sentinel. The `state >= 0x10` test tells a
 *              sentinel apart from a live entry count.
 *
 * Only the sentinels whose role is unambiguous from the control flow are named
 * below. The remaining sentinels are message/status screens whose exact meaning
 * depends on the (binary-resident) message resource each one draws, so they are
 * left as raw values until identified:
 *   0xF3  companion prompt page to SAVE_CONFIRM (navigates to it or cancels)
 *   0xF7  drawn when the selected entry is not found while advancing the list
 *   0xF8/0xF9  mode-dependent card read/scan failure notice
 *   0xFA/0xFB/0xFC/0xFD  status/result notices
 *   0xFE  no-op (draws nothing)
 */
#define ADDHERO_ENTRY_STATE_IDLE          0xFF  /* browsing / reset; no operation active */
#define ADDHERO_ENTRY_STATE_LOAD_PROGRESS 0xF6  /* importing a matched entry; progress bar */
#define ADDHERO_ENTRY_STATE_SAVE_PROGRESS 0xF5  /* write in progress; progress bar */
#define ADDHERO_ENTRY_STATE_SAVE_CONFIRM  0xF4  /* confirm dialog; accepting writes the save */
#define ADDHERO_ENTRY_STATE_CONFIRM_PROMPT 0xF3 /* companion choice prompt for save/exit flow */

#define ADDHERO_CONFIRM_BUTTON_MASK (PAD_BTN_CROSS | PAD_BTN_L3)
#define ADDHERO_CARD_SWITCH_BUTTON_MASK (PAD_BTN_SELECT | PAD_BTN_RIGHT | PAD_BTN_LEFT)
#define ADDHERO_INPUT_INJECTION_ENABLED 0x80
#define ADDHERO_SAVE_MAGIC 0x414E41
#define ADDHERO_SAVE_CHECKSUM_BYTES 0x33E0
#define ADDHERO_SAVE_CHECKSUM_BIAS 0x0414E410

typedef struct AddheroFallbackText
{
    u8 pad[0x24];
    u8 text[0x20];
} AddheroFallbackText;

/** @brief Saved context block copied between the live game state and the save image. */
typedef struct
{
    u8 inject_enable;
    u8 _pad01[0x17];
    u32 inject_flags;
    u8 _pad1C[0x234];
} AddheroSaveContextBlock;

/** @brief Portion of the ADDHERO save image whose layout is used by this overlay. */
typedef struct
{
    u8 _pad0000[0x770];
    AddheroSaveContextBlock context;
    u8 _pad09C0[0x2A20];
    s32 checksum;
    s32 magic;
} AddheroSaveBlob;

typedef struct AddheroEntryHeader
{
    s32 unk0;
    s16 unk4;
    s8 unk6;
    u8 pad[9];
} AddheroEntryHeader;

/** @brief Save-file header block at g_addhero_file_template (only the first 6 bytes used). */
typedef struct
{
    s32 unk0;
    s16 unk4;
    u8 pad[0xFA];
} AddheroFileHeader;

typedef struct
{
    s32 unk0;
    s16 unk4;
    s16 unk6;
    u8 unk8[0x18];
} AddheroFileHeaderScratch;

typedef struct
{
    s32 unk0;
    s16 unk4;
    u8 pad[0x62];
} AddheroLoadScratch;

/* --- text renderer (addhero_draw_cached_text family) --- */

typedef union
{
    u32 raw;
    struct
    {
        u16 code;
        u16 flags;
    } data;
} AddheroGlyphCacheEntry;

typedef struct
{
    SPRT_16 packet;
    u32 padding;
} AddheroGlyphSprite;

#define GLYPH_CACHE_SLOTS 0x100
#define GLYPH_CACHE_COLUMNS 16
#define GLYPH_CACHE_ROW_MASK 0xF0
#define GLYPH_RASTER_BYTES 0x80
#define GPU_ADDR_MASK 0xFFFFFF
#define GPU_TAG_HIGH_MASK 0xFF000000

/* addhero_update_and_draw_elements element-draw pipeline types */
typedef struct
{
    s32 tag;
    s32 word4;
    s16 x0;
    s16 y0;
    s16 unkC;
    u16 unkE;
} AddheroGpuPacket;

typedef struct
{
    s32 tag;
    u8 pad4[0x40AE];
    s16 frame_flag;
    u8 pad40B4[4];
    AddheroGpuPacket *prim_cursor;
} AddheroDrawState;

typedef AddheroGpuPacket *(*AddheroElementDrawFunc)();

/** @brief Pool of animated UI elements used by the ADDHERO screen. */
extern AddheroElement g_addhero_element_pool[ADDHERO_ELEMENT_COUNT];
extern AddheroElement g_addhero_element1;
extern struct DIRENTRY g_addhero_entries[][ADDHERO_DIRECTORY_ENTRY_COUNT];
extern AddheroRecord g_addhero_entry_metadata;
extern AddheroFileHeader g_addhero_file_template;
extern AddheroEntryHeader g_addhero_entry_header_template;
extern AddheroGlyphCacheEntry g_addhero_glyph_cache[];
/* Shared controller/game context (main.h PadContext); addressed here as a byte
   buffer for the save-blob copies and metadata reads. */
extern u8 *g_pad_ctx;
extern u8 *g_addhero_load_step;
extern u8 *g_addhero_glyph_raster_cursor;
extern void *jtbl_80140098[];

extern s32 g_save_slot_index;
extern s32 D_80122718;
extern s32 g_pad_input;
extern s32 g_menu_element_counter;
extern s32 g_addhero_loadseq_done;
extern s32 g_addhero_icon_phase;
extern s32 g_addhero_pad_work_ptr;
extern s32 g_addhero_scroll_y;
extern s32 g_addhero_result;
extern s32 g_addhero_work_ram_base;
extern s32 g_addhero_progress_active;
extern s32 g_addhero_scroll_target_y;
extern s32 g_addhero_mode;
extern s32 g_addhero_exit_requested;
extern s32 g_addhero_entry_state;
extern s32 g_addhero_card_slot;
extern s32 g_addhero_selected_row;
extern s32 g_addhero_choice_toggle;
extern s32 g_addhero_load_flow_active;
extern s32 g_addhero_selection_status;
extern s32 g_addhero_scroll_frames;
extern s32 g_addhero_icon_palette;
extern s32 g_addhero_frame_parity;
extern s32 g_addhero_dialog_state;
extern s32 g_addhero_io_busy;
extern s32 g_addhero_progress_bar_active;
extern s32 g_addhero_progress_start_tick;
extern s32 g_addhero_entry_scan_active;
extern s32 g_addhero_entry_identity;
extern s32 g_addhero_write_in_progress;
extern s32 g_addhero_rank_count;
extern s32 g_addhero_icon_image_table[];
extern s32 g_addhero_entry_suffix_values[];
extern s32 g_addhero_entry_ranks[];
extern s32 g_addhero_retry_count;
extern s32 g_addhero_selected_entry_extended;
extern s32 g_addhero_primary_poll_countdown;
extern s32 g_addhero_entry_value_limit;
extern s32 g_addhero_entry_fields[];
extern s32 g_addhero_secondary_poll_countdown;
extern s32 g_addhero_primary_handle0;
extern s32 g_addhero_primary_handle1;
extern s32 g_addhero_primary_handle2;
extern s32 g_addhero_primary_handle3;
extern s32 g_addhero_has_free_entry_space;
extern s32 g_addhero_file_handle;
extern s32 g_addhero_secondary_handle0;
extern s32 g_addhero_secondary_handle1;
extern s32 g_addhero_secondary_handle2;
extern s32 g_addhero_secondary_handle3;
extern s32 g_addhero_glyph_cursor_x;
extern s32 g_addhero_glyph_cursor_y;
extern s32 g_addhero_text_line_start_x;
extern s32 g_addhero_glyph_upload_x;
extern s32 g_addhero_glyph_upload_y;

extern u8 g_addhero_loadseq_start;
extern u8 g_addhero_loadseq_abort[];
extern u8 g_addhero_loadseq_load_begin[];
extern u8 g_addhero_loadseq_load_progress;
extern u8 g_addhero_loadseq_save_begin;
extern u8 g_addhero_loadseq_card[];
extern u8 g_addhero_loadseq_file_ready[];
extern u8 g_addhero_single_byte_char_table[];
extern u8 g_addhero_double_byte_char_table[];
extern u8 g_addhero_icon_context[];
extern u8 g_addhero_save_blob[];
extern u8 g_addhero_entry_read_buffer;
extern u8 g_addhero_entry_record;
extern u8 g_addhero_entry_owner_id;
extern u8 g_addhero_target_file_path[];
extern u8 g_addhero_glyph_raster_buffer[];
extern u8 g_addhero_save_file_path[];
extern u8 g_text_time_separator_offset_bytes[2];
extern u8 g_text_choice_glyph_offsets[];

extern char g_lom_save_filename_prefix[];
extern char g_lom_alt_save_filename_prefix[];
extern char g_new_save_entry_prefix[];
extern char g_lom_save_dummy_filename[];
extern char g_lom_alt_save_dummy_filename[];

extern u16 g_addhero_glyph_table;
extern u16 g_addhero_glyph_status_fa;
extern u16 g_addhero_glyph_status_fd;
extern u16 g_addhero_glyph_save_entry_label;
extern u16 g_addhero_glyph_default_entry_label;
extern u16 g_addhero_glyph_card_slot0_label;
extern u16 g_addhero_glyph_card_slot1_label;
extern u16 g_addhero_glyph_status_fb;
extern u16 g_addhero_glyph_status_fc;
extern u16 g_addhero_glyph_new_entry_label;
extern u16 g_addhero_glyph_save_progress;
extern u16 g_addhero_glyph_details_status2_msg;
extern u16 g_addhero_glyph_entry_value_label;
extern u16 g_addhero_glyph_load_prompt;
extern u16 g_addhero_glyph_load_progress;
extern u16 g_addhero_glyph_status_f8;
extern u16 g_addhero_glyph_alt_save_entry_label;
extern u16 g_addhero_glyph_dialog_msg0;
extern u16 g_addhero_glyph_dialog_msg1;
extern u16 g_addhero_glyph_dialog_msg2;
extern u16 g_addhero_glyph_dialog_msg3;
extern u16 g_addhero_glyph_mode0;
extern u16 g_addhero_glyph_mode1;
extern u16 g_addhero_glyph_current_hero_marker;
extern u16 g_addhero_glyph_owner_mismatch_msg;
extern u16 g_addhero_glyph_status_f3;
extern u16 g_addhero_glyph_status_f7;
extern u16 g_addhero_glyph_save_confirm_msg;
extern u16 g_addhero_glyph_plus_marker;
extern u16 g_addhero_entry_glyph_table[];
extern u16 g_addhero_decimal_glyphs[];
extern u16 g_addhero_hex_glyphs[];

/* In-file functions */
void addhero_init(s32 work_base, s32 mode);
s32 addhero_state_step(s32 render_half);
void addhero_build_ui_elements(void);
void addhero_update_state();
s32 addhero_update_load_sequence(void);
s32 addhero_handle_input(void);
void addhero_reset_state(void);
void addhero_close_all_elements(void);
void addhero_scroll_to_selection(void);
void addhero_update_elements(void);
s32 addhero_draw_entry_list(s32 *ot, s32 prim, s32 x_offset, s32 y_offset);
s32 addhero_draw_mode_glyph(s32 *ot, s32 prim, s32 x_offset, s32 y_offset);
s32 addhero_draw_card_slot0_label(s32 *ot, s32 prim, s32 x_offset, s32 y_offset);
s32 addhero_draw_card_slot1_label(s32 *ot, s32 prim, s32 x_offset, s32 y_offset);
s32 addhero_draw_selected_entry_details(s32 *ot, s32 prim, s32 x_offset, s32 y_offset);
u8 *addhero_skip_hex_digits(void *text);
void addhero_terminate_multibyte_text(void *buffer);
void addhero_clear_elements();
AddheroElement *addhero_alloc_element(void);
void addhero_update_and_draw_elements();
void addhero_deactivate_primary_element(void);
void addhero_text_append(u8 *dst, u8 *src);
s32 addhero_text_byte_length(u8 *str);
void addhero_text_copy(u8 *dst, u8 *src);
s32 addhero_draw_load_prompt(s32 *ot, s32 prim, s32 x_offset, s32 y_offset);
s32 addhero_draw_load_progress(s32 *ot, s32 prim, s32 x_offset, s32 y_offset);
s32 addhero_draw_progress_bar(s32 prim, s32 *ot);
void addhero_open_status_dialog(s32 message_id);
void addhero_open_exit_dialog(s32 message_id);
s32 addhero_draw_status_dialog(s32 *ot, s32 prim, s32 x_offset, s32 y_offset);
s32 addhero_draw_exit_dialog(s32 *ot, s32 prim, s32 x_offset, s32 y_offset);
s32 addhero_draw_transfer_status(s32 *ot, s32 prim, s32 x_offset, s32 y_offset);
s32 addhero_draw_icon_highlight(s32 result, s32 *ot, s32 x, s32 y, s32 adjust, s32 slot, s32 i, s32 j);
void addhero_enable_choice_toggle(void);
s32 addhero_draw_choice_prompt(s32 prim, s32 *ot, s32 x, s32 y);
s32 addhero_validate_save_blob(u8 *base);
s32 addhero_compute_save_checksum(u8 *data);
s8 *addhero_format_decimal(s8 *out, s32 value);
void addhero_format_hex(s8 *out, s32 value, s32 max_chars);
void addhero_hex_nibble_to_ascii(s8 *out, s32 value);
u32 addhero_parse_hex(u8 *s, s32 len);
s32 addhero_parse_hex_suffix_byte(u8 *text);
s32 addhero_rank_entries(s32 unused0, s32 unused1, s32 unused2);
s32 addhero_has_known_entry_type(void);
s32 addhero_entry_blocks_reach_limit(void);
void addhero_render_fixed_prompts(void);
s32 addhero_begin_entry_scan(s32 page);
s32 addhero_scan_next_entry(s32 page);
void addhero_release_primary_handles(void);
void addhero_release_secondary_handles(void);
s32 addhero_poll_primary_handle_group(void);
s32 addhero_poll_secondary_handle_group(void);
void addhero_sort_entries_by_type(void);
s32 addhero_draw_signed_decimal(s32 prim, s32 *ot, s32 value, s32 x, s32 y, s32 palette, s32 alignment);
void addhero_draw_hex_byte(s32 prim, s32 ot, s32 byte_value, s32 x, s32 y, s32 alignment);
s32 addhero_render_cached_glyph(s32 prim, s32 *ot, s32 character_code, s32 palette);
s32 addhero_emit_glyph_sprite(AddheroGlyphSprite *sprite, s32 *ot, s32 cache_slot, s32 palette);
void addhero_expand_text_glyph_codes(u8 *out, u8 *in);

/* External functions */
s32 func_800A88A0(s32 prim, s32 *ot, void *glyph, s32 a3, s32 x, s32 y, s32 mode);
s32 func_800A8A78(s32 *ot, s32 prim, s32 ch, s32 a3, Vec2s *pos, s32 mode);
s32 strncmp(void *a, void *b, s32 n);
void play_menu_sfx();
void func_800AA02C(void);
void field_restore_fade_target(void);
void field_set_default_fade_target(void);
void field_restore_fade_target_with_duration(s32 arg0);
void func_80063194(void);
void func_80019788(s32 arg0);
void func_8001990C(RECT *rect, s32 a1, s32 a2, s32 a3);
void func_80019A34(RECT *rect, void *str);
void func_800A55E4(void *buf, s32 arg1);
void func_800A5638(void *buf, s32 arg1);
void func_8001A5D4(s32 arg0, s32 *arg1);
void func_8001C56C(s32 *arg0, s32 a1, s32 a2, s32 a3, s32 a4);
s32 func_800AD850();
s32 func_800AE76C();
s32 VSync(s32 arg0);
void bcopy(void *dst, void *src, s32 len);
void field_text_reset_scratch(void);
void field_text_reset_windows(void);
void addhero_shutdown_stream_handles(void);
void addhero_begin_glyph_cache_frame(void);
void addhero_evict_unused_glyphs(void);
void addhero_reset_glyph_cache(void);
void addhero_reset_entry_ranks(void);
void addhero_init_stream_handles(void);
void addhero_enable_choice_toggle(void);
void addhero_restart_load_sequence(void);
s32 addhero_poll_and_rewind_primary_handles(void);
void addhero_commit_selected_entry(void);
s32 addhero_draw_cached_text(s32 result, s32 *ot, u8 *name, s32 x, s32 y, s32 a5, s32 a6);
s32 addhero_advance_load_sequence();
s32 strcat(void *a, void *b);
s32 open(void *a, s32 b);
s32 read(s32 a, void *b, s32 c);
s32 write(s32 a, void *b, s32 c);
s32 close(s32 a);
s32 rename(void *a, void *b);
s32 erase(void *a);
s32 strcpy(void *a, void *b, ...);
s32 _card_info(s32 a);
s32 _card_load(s32 a);
s32 _card_wait(s32 a);
s32 _card_clear(s32 a);
s32 func_80032174(s32 a, void *b, s32 *c);
s32 McxCardType(s32 a);
s32 firstfile(void *a, void *b);
void func_800B0170(void *a);
s32 nextfile(void *a);
s32 Krom2RawAdd(s32 a);
void reset_controller_vsync_state(void);
s32 OpenEvent(s32 a, s32 b, s32 c, s32 d);
void CloseEvent(s32 a);
s32 TestEvent(s32 a);
void EnableEvent(s32 a);
void EnterCriticalSection(void);
void ExitCriticalSection(void);

#define SET_ELEM_WIDTH_LOW(element, width) ((element)->attr.word = ((element)->attr.word & 0x00FFFFFF) | ((u32)(width) << 24))
#define GLYPH_SYM(sym, off) ((void *)(((u8 *)&(sym) - (off)) + (sym)))
#define GLYPH_OFF(base, off) ((void *)((base) + *(u16 *)((base) + (off))))
/** Resolve a glyph string from a preloaded table @p base plus the u16 offset
 *  held in @p sym (@p sym is a table entry naming its own offset value). */
#define GLYPH_ENTRY(base, sym) ((void *)((s32)(sym) + (s32)(base)))

/**
 * @brief Reset overlay state and build the initial UI elements.
 * @param work_base Work-RAM base (always 0x80170000); stored in g_addhero_work_ram_base, unused so far.
 * @param mode Mode selector, stored in g_addhero_mode.
 * @see decomp.me (100%)
 */
void addhero_init(s32 work_base, s32 mode)
{
    RECT rect;

    g_addhero_mode = mode;
    g_addhero_entry_state = ADDHERO_ENTRY_STATE_IDLE;
    g_addhero_card_slot = 0;
    
    addhero_reset_entry_ranks();
    g_addhero_result = 3;
    addhero_init_stream_handles();
    g_addhero_icon_phase = 0;
    field_set_default_fade_target();

    rect.x = OVERLAY_INIT_CLEAR_VRAM_X;
    rect.y = OVERLAY_INIT_CLEAR_VRAM_Y;
    rect.w = OVERLAY_INIT_CLEAR_VRAM_W;
    rect.h = OVERLAY_INIT_CLEAR_VRAM_H;

    func_8001990C(&rect, 0, 0, 0);
    addhero_reset_glyph_cache();

    g_addhero_write_in_progress = 0;
    g_addhero_progress_active = 0;
    g_addhero_selection_status = 0;
    g_addhero_io_busy = 0;
    g_addhero_frame_parity = 0;
    g_addhero_exit_requested = 0;

    func_800AA02C();
    addhero_build_ui_elements();

    g_addhero_work_ram_base = work_base;
}

/**
 * @brief Run one frame: tear down and exit if requested, else update and render.
 * @param render_half Render buffer half being drawn; forwarded to addhero_update_state, which ignores it.
 * @return Non-zero exit code when exiting, 0 while running.
 * @see decomp.me (100%)
 */
s32 addhero_state_step(s32 render_half)
{
    if (g_addhero_exit_requested != 0)
    {
        addhero_shutdown_stream_handles();
        field_text_reset_windows();
        func_80019788(0);
        return g_addhero_exit_requested;
    }
    
    field_text_reset_scratch();
    addhero_begin_glyph_cache_frame();
    addhero_update_state(render_half);
    addhero_evict_unused_glyphs();
    func_80063194();
    g_addhero_frame_parity ^= 1;
    return 0;
}

/**
 * @brief Reset scroll/selection state and populate the UI element pool for the current mode.
 * @note mode != 0: transfer layout (status + two card-slot labels). mode == 0: full browser
 *       (entry list, mode glyph, two slot labels, entry details).
 * @see decomp.me (100%)
 */
void addhero_build_ui_elements(void)
{
    AddheroElement *p;
    g_addhero_scroll_frames = 0;
    g_addhero_scroll_target_y = 0;
    g_addhero_scroll_y = 0;
    g_addhero_selected_row = 0;
    g_addhero_selection_status = 0;
    g_addhero_pad_work_ptr = (s32)g_pad_ctx + 0xCE0;
    if (0) addhero_clear_elements(0,0,0,0,0);
    addhero_clear_elements();
    g_addhero_load_flow_active = 0;
    if (g_addhero_mode != 0)
    {
        g_addhero_element_pool[0].attr.bits.state = ADDHERO_ELEMENT_STATE_OPENING;
        p = addhero_alloc_element();
        p->draw_handler = (void *)addhero_draw_transfer_status;
        p->attr.bits.transition_step = 1;
        p->attr.bits.x = 0x10;
        p->attr.bits.y = 0x61;
        p->size.bits.width_high = 1;
        p->size.bits.height = 0x2C;
        SET_ELEM_WIDTH_LOW(p, 0x20);

        p = addhero_alloc_element();
        p->draw_handler = (void *)addhero_draw_card_slot0_label;
        p->attr.bits.transition_step = 1;
        p->attr.bits.x = 0x18;
        p->attr.bits.y = 0x4D;
        p->size.bits.width_high = 0;
        p->size.bits.height = 0x10;
        SET_ELEM_WIDTH_LOW(p, 0x80);

        p = addhero_alloc_element();
        p->draw_handler = (void *)addhero_draw_card_slot1_label;
        p->attr.bits.transition_step = 1;
        p->attr.bits.x = 0xA0;
        p->attr.bits.y = 0x4D;
        p->size.bits.width_high = 0;
        p->size.bits.height = 0x10;
        SET_ELEM_WIDTH_LOW(p, 0x80);
        g_addhero_element_pool[0].attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
        return;
    }

    g_addhero_element_pool[0].attr.bits.state = ADDHERO_ELEMENT_STATE_OPENING;
    p = addhero_alloc_element();
    p->draw_handler = (void *)addhero_draw_entry_list;
    p->attr.bits.transition_step = 1;
    p->attr.bits.x = 0x1C;
    p->attr.bits.y = 0x32;
    p->size.bits.width_high = 1;
    p->size.bits.height = 0x58;
    SET_ELEM_WIDTH_LOW(p, 8);
    p->size.bits.scrollable = 1;

    p = addhero_alloc_element();
    p->draw_handler = (void *)addhero_draw_mode_glyph;
    p->attr.bits.transition_step = 1;
    p->attr.bits.x = 0x24;
    p->attr.bits.y = 0x0A;
    p->size.bits.width_high = 0;
    p->size.bits.height = 0x10;
    SET_ELEM_WIDTH_LOW(p, 0xF0);

    p = addhero_alloc_element();
    p->draw_handler = (void *)addhero_draw_card_slot0_label;
    p->attr.bits.transition_step = 1;
    p->attr.bits.x = 0x18;
    p->attr.bits.y = 0x1E;
    p->size.bits.width_high = 0;
    p->size.bits.height = 0x10;
    SET_ELEM_WIDTH_LOW(p, 0x80);

    p = addhero_alloc_element();
    p->draw_handler = (void *)addhero_draw_card_slot1_label;
    p->attr.bits.transition_step = 1;
    p->attr.bits.x = 0xA0;
    p->attr.bits.y = 0x1E;
    p->size.bits.width_high = 0;
    p->size.bits.height = 0x10;
    SET_ELEM_WIDTH_LOW(p, 0x80);

    p = addhero_alloc_element();
    p->draw_handler = (void *)addhero_draw_selected_entry_details;
    p->attr.bits.transition_step = 1;
    p->attr.bits.x = 0x1E;
    p->attr.bits.y = 0x8E;
    p->size.bits.width_high = 1;
    p->size.bits.height = 0x34;
    SET_ELEM_WIDTH_LOW(p, 4);
    g_addhero_element_pool[0].attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
}

/**
 * @brief Run one frame of overlay logic: update elements, advance the load
 *        sequence when armed, sample pad input, and step the scroll animation.
 * @see decomp.me (100%)
 */
void addhero_update_state(void)
{
    s32 delta;

    addhero_update_elements();
    g_addhero_icon_phase += 2;
    if ((g_addhero_element1.attr.word & 0x7F) == 2)
    {
        addhero_update_load_sequence();
    }
    if ((u16)g_pad_input == 0xFFFF)
    {
        g_pad_input = 0;
    }
    addhero_handle_input();
    if (g_addhero_scroll_frames != 0)
    {
        s32 base = g_addhero_scroll_y;
        delta = (g_addhero_scroll_target_y - g_addhero_scroll_y) / g_addhero_scroll_frames;
        g_addhero_scroll_frames -= 1;
        g_addhero_scroll_y += delta;
    }
    else
    {
        g_addhero_scroll_y = g_addhero_scroll_target_y;
    }
}

/**
 * @brief Drive the card load/scan state machine one frame, mapping its result
 *        code onto the next load step and any error entry-state sentinel.
 * @return Unused; declared s32 for the original signature but falls through
 *         without an explicit return value.
 * @see decomp.me (100%)
 */
s32 addhero_update_load_sequence(void)
{
    s32 result;

    if (g_addhero_entry_state >= 0x10)
    {
        if (g_addhero_load_step == 0)
        {
            g_addhero_load_step = (u8 *)&g_addhero_loadseq_start;
        }
    }

    do
    {
        result = addhero_advance_load_sequence();
    } while (result == ADDHERO_LOAD_RESULT_CONTINUE);

    if ((g_addhero_load_flow_active != 0) && (g_pad_input & 0x220))
    {
        if (g_addhero_mode == 0)
        {
            g_addhero_entry_state = 0xF9;
        }
        else
        {
            g_addhero_entry_state = 0xF8;
        }
        g_addhero_load_step = (u8 *)&g_addhero_loadseq_abort;
    }
    else
    {
        switch (result)
        {
        case ADDHERO_LOAD_RESULT_NONE:
            break;
        case ADDHERO_LOAD_RESULT_COMPLETE:
            g_addhero_load_step = (u8 *)&g_addhero_loadseq_done;
            g_addhero_load_flow_active = 0;
            break;
        case ADDHERO_LOAD_RESULT_CARD_ERROR:
            if (g_addhero_mode == 0)
            {
                g_addhero_entry_state = 0xF9;
            }
            else
            {
                g_addhero_entry_state = 0xF8;
            }
            /* fallthrough */
        case ADDHERO_LOAD_RESULT_ABORT:
            g_addhero_load_step = (u8 *)&g_addhero_loadseq_abort;
            break;
        }
    }
}

/**
 * @brief Handle browser input, entry navigation, and load confirmation.
 * @return Unused.
 */
s32 addhero_handle_input(void)
{
    s32 entry_count;
    s32 input;
    s32 move_count;
    AddheroElement *prompt;
    struct DIRENTRY *selected_entry;

    if (g_addhero_element_pool[1].attr.bits.state == ADDHERO_ELEMENT_STATE_INACTIVE)
    {
        g_addhero_exit_requested = g_addhero_result;
        return;
    }
    if (g_addhero_exit_requested != 0)
    {
        return;
    }
    if (g_addhero_element_pool[1].attr.bits.state >= ADDHERO_ELEMENT_STATE_CLOSING)
    {
        return;
    }
    if (g_addhero_element_pool[0].attr.bits.state != ADDHERO_ELEMENT_STATE_INACTIVE)
    {
        return;
    }

    entry_count = g_addhero_entry_state;
    if (entry_count == ADDHERO_ENTRY_STATE_IDLE)
    {
        return;
    }
    if (g_addhero_entry_scan_active != 0)
    {
        return;
    }
    if (g_addhero_io_busy != 0)
    {
        return;
    }
    if ((u32)(*g_addhero_load_step - 6) < 2U)
    {
        return;
    }
    if (g_addhero_mode != 0)
    {
        return;
    }

    input = g_pad_input;
    if (input & PAD_BTN_CIRCLE)
    {
        D_80122718 = 3;
        play_menu_sfx(0x78, 0x80);
        addhero_close_all_elements();
        return;
    }
    if (input & ADDHERO_CARD_SWITCH_BUTTON_MASK)
    {
        play_menu_sfx(0x7D, 0x80);
        addhero_reset_state();
        return;
    }
    if (entry_count >= ADDHERO_ENTRY_COUNT_LIMIT)
    {
        return;
    }

    move_count = 1;
    if (input & PAD_BTN_R1)
    {
        g_pad_input = PAD_BTN_DOWN;
        move_count = 1;
    }
    if (g_pad_input & PAD_BTN_L1)
    {
        g_pad_input = PAD_BTN_UP;
        move_count = 1;
    }

    while (move_count != 0)
    {
        if (g_pad_input & PAD_BTN_UP)
        {
            g_addhero_selected_row--;
            if (g_addhero_selected_row < 0)
            {
                g_addhero_selected_row = g_addhero_entry_state - 1;
            }
        }
        if (g_pad_input & PAD_BTN_DOWN)
        {
            g_addhero_selected_row++;
            if (g_addhero_selected_row >= g_addhero_entry_state)
            {
                g_addhero_selected_row = 0;
            }
        }
        move_count--;
    }

    if (g_pad_input & (PAD_BTN_UP | PAD_BTN_DOWN))
    {
        addhero_commit_selected_entry();
        play_menu_sfx(0x7D, 0x80);
        addhero_scroll_to_selection();
        return;
    }

    if (g_pad_input & ADDHERO_CONFIRM_BUTTON_MASK)
    {
        selected_entry = &g_addhero_entries[g_addhero_card_slot][g_addhero_selected_row];
        if (strncmp(g_lom_save_filename_prefix, selected_entry->name, 0xC) == 0)
        {
            if ((g_addhero_entry_metadata.hero_id != ((AddheroRecord *)g_pad_ctx)->hero_id) &&
                ((g_save_slot_index == 0xFF) || (g_addhero_entry_metadata.owner_id == g_save_slot_index)))
            {
                prompt = addhero_alloc_element();
                prompt->attr.bits.transition_step = 1;
                prompt->attr.bits.x = 0x10;
                prompt->attr.bits.y = 0x61;
                prompt->size.bits.width_high = 1;
                prompt->size.bits.height = 0x1E;
                SET_ELEM_WIDTH_LOW(prompt, 0x20);
                addhero_enable_choice_toggle();
                prompt->draw_handler = addhero_draw_load_prompt;
                addhero_restart_load_sequence();
                play_menu_sfx(0x7E, 0x80);
                return;
            }
        }
        play_menu_sfx(0x78, 0x80);
    }
}

/**
 * @brief Reset scroll/selection state and flip to the other card slot, then
 *        clear ranks and pad input to restart browsing.
 * @see decomp.me (100%)
 */
void addhero_reset_state(void)
{
    g_addhero_load_flow_active = 0;
    g_addhero_load_step = 0;
    g_addhero_entry_state = ADDHERO_ENTRY_STATE_IDLE;
    g_addhero_scroll_frames = 0;
    g_addhero_scroll_target_y = 0;
    g_addhero_scroll_y = 0;
    g_addhero_selected_row = 0;
    g_addhero_selection_status = 0;
    g_addhero_card_slot ^= 1;
    addhero_reset_entry_ranks();
    func_800AA02C();
    g_pad_input = 0;
}

/**
 * @brief Put every active pool element into the closing transition (state 3,
 *        phase 0x40) so they animate out.
 * @see decomp.me (100%)
 */
void addhero_close_all_elements(void)
{
    AddheroElement *element;
    s32 slot;

    field_restore_fade_target();
    element = g_addhero_element_pool;
    for (slot = 0; slot < ADDHERO_ELEMENT_COUNT; slot++, element++)
    {
        if (element->attr.bits.state != ADDHERO_ELEMENT_STATE_INACTIVE)
        {
            element->attr.bits.state = ADDHERO_ELEMENT_STATE_CLOSING;
            element->attr.bits.transition_step = 8;
        }
    }
}

/**
 * @brief Retarget the list scroll so the selected row stays on screen,
 *        animating over four frames when it falls above or below the window.
 * @see decomp.me (100%)
 */
void addhero_scroll_to_selection(void)
{
    s32 index;
    s32 temp;
    s32 base;
    s32 pos;
    s32 diff;

    index = g_addhero_selected_row;
    temp = (index << 3) - index;
    base = g_addhero_scroll_y;
    pos = temp << 1;
    diff = pos - base;

    if (diff >= 0x4B)
    {
        g_addhero_scroll_target_y = pos - 0x46;
        g_addhero_scroll_frames = 4;
    }
    if (diff < 0)
    {
        g_addhero_scroll_target_y = pos;
        g_addhero_scroll_frames = 4;
    }
}

/**
 * @brief Thin wrapper that runs the element update/draw pass on the active
 *        draw state.
 * @see decomp.me (100%)
 */
void addhero_update_elements(void)
{
    addhero_update_and_draw_elements();
}

/**
 * @brief Draw the save-entry browser: status/error screens by entry-state
 *        sentinel, the per-row entry list with rank glyphs, and the selection
 *        highlight tile.
 * @param ot   Ordering table the primitives are linked into.
 * @param prim Current primitive pointer/index within the ordering table.
 * @param x_offset Horizontal offset; screen X is derived from it.
 * @param y_offset Vertical offset applied to each row.
 * @return The updated primitive pointer after linking this frame's glyphs.
 */
s32 addhero_draw_entry_list(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    s32 entry_state = g_addhero_entry_state;

    switch (entry_state)
    {
    case 0xF8:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_status_f8, 0x34), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xF9:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_status_f8, 0x34), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFA:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_status_fa, 2), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFD:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_status_fd, 4), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFB:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_status_fb, 0x10), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFC:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_status_fc, 0x12), 4, -x_offset + 0x84, -y_offset, 2);
        break;
    case 0xFE:
        break;
    case ADDHERO_ENTRY_STATE_IDLE:
        {
            s32 message_x;
            u8 *glyph_base;

            message_x = -x_offset + 0x84;
            glyph_base = (u8 *)&g_addhero_glyph_table;
            prim = func_800A88A0(prim, ot, glyph_base + g_addhero_glyph_table, 4, message_x, -y_offset, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(glyph_base, 0x1E), 4, message_x, ADDHERO_ENTRY_ROW_HEIGHT - y_offset, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(glyph_base, 0xB2), 4, message_x, (ADDHERO_ENTRY_ROW_HEIGHT * 2) - y_offset, 2);
        }
        break;
    default:
        {
            s32 row_y;
            s32 entry_index;

            if (g_addhero_entry_scan_active != 0)
            {
                s32 message_x;
                u8 *glyph_base;

                message_x = -x_offset + 0x84;
                glyph_base = (u8 *)&g_addhero_glyph_table;
                prim = func_800A88A0(prim, ot, glyph_base + g_addhero_glyph_table, 4, message_x, -y_offset, 2);
                prim = func_800A88A0(prim, ot, GLYPH_OFF(glyph_base, 0x1E), 4, message_x, ADDHERO_ENTRY_ROW_HEIGHT - y_offset, 2);
                prim = func_800A88A0(prim, ot, GLYPH_OFF(glyph_base, 0xB2), 4, message_x, (ADDHERO_ENTRY_ROW_HEIGHT * 2) - y_offset, 2);
                break;
            }
            entry_index = 0;
            if (entry_state > 0)
            {
                s32 list_x;
                u16 rank_glyph;
                Vec2s value_pos;
                u8 *glyph_base;

                glyph_base = (u8 *)&g_addhero_glyph_table;
                list_x = -x_offset;
                do
                {
                    row_y = ((entry_index * ADDHERO_ENTRY_ROW_HEIGHT) - y_offset) - g_addhero_scroll_y + 1;
                    if ((u32)(row_y + ADDHERO_ENTRY_ROW_HEIGHT - 1) < 0x65U)
                    {
                        if (g_addhero_entry_ranks[entry_index] >= 0)
                        {
                            value_pos.x = list_x + 0x86;
                            value_pos.y = row_y;
                            prim = func_800A88A0(
                                func_800A8A78(ot, prim, g_addhero_entry_suffix_values[entry_index], 4, &value_pos, 0), ot,
                                GLYPH_ENTRY(glyph_base, g_addhero_glyph_entry_value_label), 4, list_x + 0x70, row_y, 0);
                            if ((g_addhero_rank_count - 1) == g_addhero_entry_ranks[entry_index])
                            {
                                rank_glyph = *(u16 *)(glyph_base + 0x36);
                                prim = func_800A88A0(prim, ot, GLYPH_ENTRY(glyph_base, rank_glyph), 4, list_x + 0xC0, row_y, 0);
                            }
                            else if (g_addhero_entry_ranks[entry_index] < 2)
                            {
                                rank_glyph = *(u16 *)(glyph_base + 0x38);
                                prim = func_800A88A0(prim, ot, GLYPH_ENTRY(glyph_base, rank_glyph), 4, list_x + 0xC0, row_y, 0);
                            }
                            if (*addhero_skip_hex_digits(
                                    &g_addhero_entries[g_addhero_card_slot][entry_index].name[ADDHERO_SAVE_FILENAME_PREFIX_LENGTH]) == '+')
                            {
                                prim = func_800A88A0(prim, ot, GLYPH_ENTRY(glyph_base, g_addhero_glyph_plus_marker), 4, 0xF2 - x_offset, row_y, 1);
                            }
                        }
                        if (strncmp(g_lom_save_filename_prefix, g_addhero_entries[g_addhero_card_slot][entry_index].name,
                                    ADDHERO_SAVE_FILENAME_PREFIX_LENGTH) == 0)
                        {
                            prim = func_800A88A0(prim, ot, GLYPH_ENTRY(glyph_base, g_addhero_glyph_save_entry_label), 4, 1 - x_offset, row_y, 0);
                        }
                        else if (strncmp(g_lom_alt_save_filename_prefix, g_addhero_entries[g_addhero_card_slot][entry_index].name,
                                         ADDHERO_SAVE_FILENAME_PREFIX_LENGTH) == 0)
                        {
                            prim = func_800A88A0(prim, ot, GLYPH_ENTRY(glyph_base, g_addhero_glyph_alt_save_entry_label), 4, 1 - x_offset, row_y, 0);
                        }
                        else if (strncmp(g_new_save_entry_prefix, g_addhero_entries[g_addhero_card_slot][entry_index].name,
                                         ADDHERO_NEW_SAVE_FILENAME_PREFIX_LENGTH) == 0)
                        {
                            prim = func_800A88A0(prim, ot, GLYPH_ENTRY(glyph_base, g_addhero_glyph_new_entry_label), 4, 1 - x_offset, row_y, 0);
                        }
                        else
                        {
                            prim = func_800A88A0(prim, ot, GLYPH_ENTRY(glyph_base, g_addhero_glyph_default_entry_label), 4, 1 - x_offset, row_y, 0);
                        }
                    }
                    entry_index++;
                } while (entry_index < g_addhero_entry_state);
            }
            row_y = ((g_addhero_selected_row * ADDHERO_ENTRY_ROW_HEIGHT) - y_offset) - g_addhero_scroll_y;

            if (g_addhero_entry_scan_active == 0)
            {
                TILE *tile = (TILE *)prim;

                *(u32 *)&tile->r0 = 0xF080F0;
                setlen(tile, 3);
                setcode(tile, 0x62);
                tile->w = 0x108;
                tile->x0 = 0;
                tile->y0 = row_y;
                tile->h = ADDHERO_ENTRY_ROW_HEIGHT;
                addPrim(ot, tile);
                prim += sizeof(TILE);
            }
        }
        break;
    }
    return prim;
}

/**
 * @brief Draw the header glyph that reflects the current mode (load vs save).
 * @param ot   Ordering table the glyph is linked into.
 * @param prim Current primitive pointer/index.
 * @param x_offset Horizontal offset; screen X is derived from it.
 * @param y_offset Vertical offset.
 * @return The updated primitive pointer.
 * @see decomp.me (100%)
 */
s32 addhero_draw_mode_glyph(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;

    if (g_addhero_mode == 1)
    {
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_mode1, 0x46), 4, -x_offset + 0x78, -y_offset, 2);
    }
    else
    {
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_mode0, 0x44), 4, -x_offset + 0x78, -y_offset, 2);
    }
    return prim;
}

/**
 * @brief Draw the slot-0 card label, dimming it with a backing tile when that
 *        slot is not the active one.
 * @param ot   Ordering table the primitives are linked into.
 * @param prim Current primitive pointer/index.
 * @param x_offset Horizontal offset; screen X is derived from it.
 * @param y_offset Vertical offset.
 * @return The updated primitive pointer.
 * @see decomp.me (100%)
 */
s32 addhero_draw_card_slot0_label(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    TILE *tile;

    if (g_addhero_card_slot != 0)
    {
        tile = (TILE *)prim;
        *(u32 *)&tile->r0 = 0x101010;
        *((u8 *)tile + 3) = 3;
        tile->code = 0x62;
        tile->x0 = 0;
        tile->y0 = 0;
        tile->w = 0x80;
        tile->h = 0x10;
        addPrim(ot, tile);
        prim += 0x10;
    }
    return func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_card_slot0_label, 0xC), 4, -x_offset + 0x40, -y_offset, 2);
}

/**
 * @brief Draw the slot-1 card label, dimming it with a backing tile when that
 *        slot is not the active one.
 * @param ot   Ordering table the primitives are linked into.
 * @param prim Current primitive pointer/index.
 * @param x_offset Horizontal offset; screen X is derived from it.
 * @param y_offset Vertical offset.
 * @return The updated primitive pointer.
 * @see decomp.me (100%)
 */
s32 addhero_draw_card_slot1_label(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    TILE *tile;

    if (g_addhero_card_slot == 0)
    {
        tile = (TILE *)prim;
        *(u32 *)&tile->r0 = 0x101010;
        *((u8 *)tile + 3) = 3;
        tile->code = 0x62;
        tile->x0 = 0;
        tile->y0 = 0;
        tile->w = 0x80;
        tile->h = 0x10;
        addPrim(ot, tile);
        prim += 0x10;
    }
    return func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_card_slot1_label, 0xE), 4, -x_offset + 0x40, -y_offset, 2);
}

/**
 * @brief Draw the detail panel for the selected entry: animated character
 *        icons, play-time, hero name, and either the cached name text or a
 *        fallback message depending on entry type.
 * @param ot   Ordering table the primitives are linked into.
 * @param prim Current primitive pointer/index.
 * @param x_offset Horizontal offset; screen X is derived from it.
 * @param y_offset Vertical offset.
 * @return The updated primitive pointer.
 * @see decomp.me (100%)
 */
s32 addhero_draw_selected_entry_details(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    s32 result;
    Vec2s pos;
    u8 name[0x21];
    char unused_pad[212];
    s32 slot[3];

    result = prim;
    if (g_addhero_selection_status == 0)
    {
        return result;
    }
    if (g_addhero_entry_scan_active != 0)
    {
        return result;
    }
    if (g_addhero_selection_status != 3 && g_addhero_entry_state < 0x10)
    {
        if (g_addhero_selection_status == 2)
        {
            s32 x = -x_offset;
            u8 *base;

            result = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_details_status2_msg, 0x28), 4, x, -y_offset, 0);
            base = (u8 *)&g_addhero_glyph_details_status2_msg - 0x28;
            return func_800A88A0(result, ot, GLYPH_OFF(base, 0x2A), 4, x, 0x10 - y_offset, 0);
        }
        else
        {
            s32 term1 = g_addhero_card_slot * ADDHERO_CARD_DIRECTORY_BYTES;
            s32 term2 = (g_addhero_selected_row * ADDHERO_DIRECTORY_ENTRY_BYTES) + (s32)g_addhero_entries;

            if (strncmp(g_lom_save_filename_prefix, (char *)(term1 + term2), 0xC) == 0)
            {
                if (g_save_slot_index == 0xFF || g_addhero_entry_owner_id == g_save_slot_index)
                {
                    s32 present_count;
                    s32 i;
                    s32 j;
                    s32 step;
                    s32 half_step;
                    s32 base_x;
                    s32 base_y;
                    s32 total;
                    s32 hours;
                    s32 time_val;

                    {
                        u8 *record = (u8 *)&g_addhero_entry_metadata;
                        slot[0] = (u32)(*(s32 *)(record + 0x18)) >> 0x19;
                        slot[1] = ((u32)(*(s32 *)(record + 0x20)) >> 0x12) & 0x7F;
                        slot[2] = (u32)(*(s32 *)(record + 0x20)) >> 0x19;
                        g_addhero_icon_palette = (s32)record[0x1F];
                    }

                    total = 0;
                    present_count = 0;
                    for (i = 0; i < 3; i++)
                    {
                        if (slot[i] != 0x7F)
                        {
                            present_count += 1;
                        }
                    }

                    switch (present_count)
                    {
                    case 2:
                        step = 0x20;
                        half_step = 0x10;
                        time_val = g_addhero_icon_phase;
                        if (g_addhero_icon_phase < 0)
                        {
                            time_val = g_addhero_icon_phase + 0x1F;
                        }
                        g_addhero_icon_phase -= (time_val >> 5) << 5;
                        break;
                    case 3:
                        step = 0x10;
                        half_step = 0x20;
                        g_addhero_icon_phase %= 0x60;
                        break;
                    default:
                        step = 0x10;
                        half_step = 0x20;
                        g_addhero_icon_phase = 0x1F;
                        break;
                    }

                    i = 0;
                    j = i;
                    for (; j < 3; j++)
                    {
                        base_y = i * half_step;
                        base_x = base_y + half_step;
                        if (slot[j] != 0x7F)
                        {
                            s32 adjust = step;
                            s32 rem;
                            s32 hi;
                            s32 delta;

                            if ((g_addhero_icon_phase >= base_y && g_addhero_icon_phase < base_x && (delta = g_addhero_icon_phase - base_y, 1))
                                || (rem = base_x % (half_step * present_count), g_addhero_icon_phase >= rem && g_addhero_icon_phase < (hi = rem + half_step) && (delta = hi - g_addhero_icon_phase, 1)))
                            {
                                adjust += delta;
                            }
                            result = addhero_draw_icon_highlight(result, ot, total - x_offset, -y_offset, adjust, slot[j], i, j);
                            i += 1;
                            total += adjust;
                        }
                    }

                    {
                        u8 *base90 = (u8 *)&g_addhero_entry_metadata;
                        s32 x = -x_offset;
                        s32 y = -y_offset;

                        base_y = *(s32 *)(base90 + 0x30);
                        pos.x = (s16)(x + 0x70);
                        pos.y = (s16)y;
                        hours = base_y / 216000;
                        result = func_800A8A78(ot, result, hours, 4, &pos, 1);
                        result = func_800A88A0(result, ot,
                            g_text_time_separator_offset_bytes[0] + ((s32)&g_text_time_separator_offset_bytes - 0x32) + (g_text_time_separator_offset_bytes[1] << 8), 4, x + 0x6F, y, 0);
                        base_y = (base_y / 3600) - (hours * 0x3C);
                        if (base_y < 0xA)
                        {
                            pos.x = (s16)(x + 0x7D);
                            pos.y = (s16)y;
                            result = func_800A8A78(ot, result, 0, 4, &pos, 1);
                        }
                        pos.x = (s16)(x + 0x85);
                        pos.y = (s16)y;
                        result = func_800A8A78(ot, result, base_y, 4, &pos, 1);
                        result = func_800A88A0(result, ot, base90, 4, x + 0x54, y + 0x10, 0);

                        if (*(u16 *)(base90 + 0xD4) == ((AddheroRecord *)g_pad_ctx)->hero_id)
                        {
                            do { result = func_800A88A0(result, ot, GLYPH_SYM(g_addhero_glyph_current_hero_marker, 0x50), 4, x + 0x54, y + 0x20, 0); } while (0);
                        }
                        else
                        {
                            result = func_800A88A0(result, ot, GLYPH_OFF((u8 *)g_addhero_entry_glyph_table, (*(s32 *)(base90 + 0x20) & 0x3FFFF) * 2), 4,
                                x + 0x54, y + 0x20, 0);
                        }
                    }
                }
                else
                {
                    result = func_800A88A0(result, ot, GLYPH_SYM(g_addhero_glyph_owner_mismatch_msg, 0x54), 4, -x_offset, -y_offset, 0);
                }
            }
            else
            {
                s32 j;
                u8 *record;

                addhero_terminate_multibyte_text(&g_addhero_entry_record);
                record = &g_addhero_entry_record;
                record -= 4;
                if ((u32)(record[0x24] - 1) >= 0x7FU)
                {
                    for (j = 0; j < 0x20; j++)
                    {
                        name[j] = record[4 + j];
                    }
                    name[j] = 0;
                    result = addhero_draw_cached_text(result, ot, name, -x_offset, -y_offset, 4, 0);

                    for (j = 0; j < 0x20; j++)
                    {
                        name[j] = ((AddheroFallbackText *)&g_addhero_entry_read_buffer)->text[j];
                    }
                    name[j] = 0;
                    result = addhero_draw_cached_text(result, ot, name, -x_offset, -y_offset + 0x10, 4, 0);
                }
            }
        }
    }
    return result;
}

/**
 * @brief Advance past a run of hex digit characters (0-9, a-f, A-F) and return
 *        the pointer to the first non-hex byte.
 * @param text Start of the text to scan.
 * @return Pointer to the first byte that is not a hex digit.
 * @see decomp.me (100%)
 */
u8 *addhero_skip_hex_digits(void *text)
{
    u8 *p;
    u32 c;

    p = text;
    while (1)
    {
        c = *p;
        p++;
        if ((u32)(c - '0') < 10)
        {
            continue;
        }
        p--;
        if (p)
        {
            p++;
            p--;
        }

        p++;
        if ((u32)(c - 'a') < 6)
        {
            continue;
        }
        p--;
        if (p)
        {
            p++;
            p--;
        }

        p++;
        if ((u32)(c - 'A') < 6)
        {
            continue;
        }
        p--;
        if (p)
        {
            p++;
            p--;
        }
        break;
    }
    return p;
}

/**
 * @brief Zero-fill a 0x40-byte text field from the first null byte onward,
 *        walking multibyte (>= 0x80 lead) characters two bytes at a time.
 * @param buffer Start of the 0x40-byte text buffer to terminate/clear.
 * @see decomp.me (100%)
 */
void addhero_terminate_multibyte_text(void *buffer)
{
    u8 *p;
    s32 i;

    p = (u8 *)buffer;
    i = 0;
    for (;;)
    {
        if (i >= 0x40)
        {
            return;
        }
        if (*p == 0)
        {
            while (i < 0x40)
            {
                *p = 0;
                i++;
                p++;
            }
            return;
        }
        if (*p >= 0x80)
        {
            p += 2;
            i += 2;
        }
        else
        {
            p += 1;
            i += 1;
        }
    }
}

/**
 * @brief Clear the eight-element pool: drop the ADDHERO flag and free (state 0)
 *        every element, and reset the shared draw scale to 0x20.
 * @see decomp.me (100%)
 */
void addhero_clear_elements(void)
{
    AddheroElement *p;
    s32 i;

    g_menu_element_counter = 0x20;
    p = g_addhero_element_pool;
    for (i = 0; i < ADDHERO_ELEMENT_COUNT; i++)
    {
        p->size.bits.scrollable = 0;
        p->attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
        p++;
    }
}

/**
 * @brief Claim the first free pool element, marking it state 1 (opening).
 * @return The claimed element, or the pool base element when none are free.
 * @see decomp.me (100%)
 */
AddheroElement *addhero_alloc_element(void)
{
    AddheroElement *p;
    s32 i;

    p = g_addhero_element_pool;
    for (i = 0; i < ADDHERO_ELEMENT_COUNT; i++, p++)
    {
        if (p->attr.bits.state == ADDHERO_ELEMENT_STATE_INACTIVE)
        {
            p->attr.bits.state = ADDHERO_ELEMENT_STATE_OPENING;
            return p;
        }
    }
    return g_addhero_element_pool;
}

/**
 * @brief Update and render the active ADDHERO UI elements.
 * @param draw_state Draw state holding the primitive cursor and frame flag.
 * @see decomp.me (100%)
 */
void addhero_update_and_draw_elements(AddheroDrawState *draw_state)
{
    AddheroGpuPacket *packet_cursor;
    AddheroDrawState *ordering_table;
    volatile u32 *element_words;
    s32 animated_width;
    s32 animated_height;
    s32 element_index;
    s32 draw_env[24];
    u32 state_word;
    s32 state;
    u32 size_word;
    u32 width;
    s32 transition_step;
    s32 scaled_width;
    s32 height;
    s32 scaled_height;
    s32 remaining_height;
    u32 opening_word;
    u32 width_low;
    s32 closing_word;
    s32 closing_scaled_width;
    s32 closing_height;
    s32 closing_scaled_height;
    s32 closing_remaining_height;
    u32 finishing_word;
    u32 updated_word;
    s32 entry_count;

    packet_cursor = draw_state->prim_cursor;
    ordering_table = draw_state;

    entry_count = g_addhero_entry_state;
    if ((entry_count < 0x10) &&
        (g_addhero_element_pool[1].attr.bits.state == ADDHERO_ELEMENT_STATE_ACTIVE) &&
        (g_addhero_element_pool[1].size.bits.scrollable != 0))
    {
        entry_count *= ADDHERO_ENTRY_ROW_HEIGHT;
        if ((g_addhero_scroll_y + 0x58) < entry_count)
        {
            packet_cursor = (AddheroGpuPacket *)func_800AE76C(packet_cursor, ordering_table, 0x114, 0x82, 0);
        }
        if (g_addhero_scroll_y != 0)
        {
            packet_cursor = (AddheroGpuPacket *)func_800AE76C(packet_cursor, ordering_table, 0x114, 0x3A, 1);
        }
    }

    if (draw_state->frame_flag != 0)
    {
        func_8001C56C(draw_env, 0, 0xF0, 0x140, 0xE0);
    }
    else
    {
        func_8001C56C(draw_env, 0, 8, 0x140, 0xE0);
    }

    element_words = (volatile u32 *)&g_addhero_element_pool[0];
    element_index = 0;

    for (; element_index < ADDHERO_ELEMENT_COUNT; element_index++, element_words += ADDHERO_ELEMENT_WORD_STRIDE)
    {
        if (*element_words & ADDHERO_ELEMENT_STATE_MASK)
        {
            func_8001A5D4((s32)packet_cursor, draw_env);

            addPrim(ordering_table, packet_cursor);

            state_word = *element_words;
            state = state_word & ADDHERO_ELEMENT_STATE_MASK;

            packet_cursor = (AddheroGpuPacket *)((u8 *)packet_cursor + 0x40);

            switch (state)
            {
            case ADDHERO_ELEMENT_STATE_OPENING:
                opening_word = *element_words;
                size_word = ((AddheroElement *)element_words)->size.word;
                width_low = opening_word >> 24;
                width = ((size_word & 1) << 8) | width_low;
                transition_step = (opening_word >> 3) & 0xF;
                scaled_width = width * transition_step;
                g_pad_input = 0;
                if (scaled_width < 0)
                {
                    scaled_width += 7;
                }
                height = (size_word >> 1) & 0xFF;
                scaled_height = height * transition_step;
                animated_width = scaled_width >> 3;
                if (scaled_height < 0)
                {
                    scaled_height += 7;
                }
                animated_height = scaled_height >> 3;
                remaining_height = (s32)(height - animated_height);

                packet_cursor = ((AddheroElementDrawFunc)((AddheroElement *)element_words)->draw_handler)(ordering_table, packet_cursor, (s32)(width - animated_width) / 2, remaining_height / 2);
                {
                    u32 post_word;
                    u32 field;
                    u32 high;
                    post_word = *element_words;
                    field = (post_word >> 7) & 0x1FF;
                    high = post_word >> 24;
                    packet_cursor = (AddheroGpuPacket *)func_800AD850(packet_cursor, ordering_table,
                                           field + (s32)((((((AddheroElement *)element_words)->size.word & 1) << 8) | high) - animated_width) / 2,
                                           (((AddheroElement *)element_words)->attr.bytes.y) + ((s32)((((AddheroElement *)element_words)->size.word >> 1) & 0xFF) - animated_height) / 2,
                                           animated_width, animated_height, draw_state->frame_flag, element_index == 0);
                }
                {
                    u32 old_word;
                    u32 new_word;
                    old_word = *element_words;
                    new_word = (old_word & ~ADDHERO_ELEMENT_PHASE_MASK) | (((((old_word >> 3) & 0xF) + 1) & 0xF) * 8);
                    *(u32 *)element_words = new_word;
                    if (((new_word >> 3) & 0xF) == 8)
                    {
                        func_800AA02C();
                        *(u32 *)element_words = (*element_words & ~ADDHERO_ELEMENT_STATE_MASK) | ADDHERO_ELEMENT_STATE_ACTIVE;
                    }
                }
                break;

            case ADDHERO_ELEMENT_STATE_ACTIVE:
                packet_cursor = ((AddheroElementDrawFunc)((AddheroElement *)element_words)->draw_handler)(ordering_table, packet_cursor, 0, 0);
                {
                    u32 case_word;
                    u32 high;
                    case_word = *element_words;
                    high = case_word >> 24;
                    packet_cursor = (AddheroGpuPacket *)func_800AD850(packet_cursor, ordering_table,
                                           (case_word >> 7) & 0x1FF, ((AddheroElement *)element_words)->attr.bytes.y,
                                           ((((AddheroElement *)element_words)->size.word & 1) << 8) | high,
                                           (((AddheroElement *)element_words)->size.word >> 1) & 0xFF, draw_state->frame_flag, element_index == 0);
                }
                updated_word = *element_words;
                if (((updated_word >> 3) & 0xF) != 0)
                {
                    *(u32 *)element_words = (updated_word & ~ADDHERO_ELEMENT_PHASE_MASK) | (((((updated_word >> 3) & 0xF) - 1) & 0xF) * 8);
                }
                break;

            case ADDHERO_ELEMENT_STATE_CLOSING:
                closing_word = *element_words;
                size_word = ((AddheroElement *)element_words)->size.word;
                closing_scaled_width = (u32)closing_word >> 24;
                width = ((size_word & 1) << 8) | closing_scaled_width;
                closing_word = (u32)closing_word >> 3;
                closing_word &= 0xF;
                closing_scaled_width = width * closing_word;
                g_pad_input = 0;
                if (closing_scaled_width < 0)
                {
                    closing_scaled_width += 7;
                }
                closing_height = (size_word >> 1) & 0xFF;
                closing_scaled_height = closing_height * closing_word;
                animated_width = closing_scaled_width >> 3;
                if (closing_scaled_height < 0)
                {
                    closing_scaled_height += 7;
                }
                animated_height = closing_scaled_height >> 3;
                closing_remaining_height = (s32)(closing_height - animated_height);

                packet_cursor = ((AddheroElementDrawFunc)((AddheroElement *)element_words)->draw_handler)(ordering_table, packet_cursor, (s32)(width - animated_width) / 2, closing_remaining_height / 2);
                {
                    u32 post_word;
                    u32 field;
                    u32 high;
                    post_word = *element_words;
                    field = (post_word >> 7) & 0x1FF;
                    high = post_word >> 24;
                    packet_cursor = (AddheroGpuPacket *)func_800AD850(packet_cursor, ordering_table,
                                           field + (s32)((((((AddheroElement *)element_words)->size.word & 1) << 8) | high) - animated_width) / 2,
                                           (((AddheroElement *)element_words)->attr.bytes.y) + ((s32)((((AddheroElement *)element_words)->size.word >> 1) & 0xFF) - animated_height) / 2,
                                           animated_width, animated_height, draw_state->frame_flag, element_index == 0);
                }
                {
                    u32 old_word;
                    old_word = *element_words;
                    closing_scaled_width = old_word & ~ADDHERO_ELEMENT_PHASE_MASK;
                    old_word >>= 3;
                    old_word &= 0xF;
                    old_word--;
                    old_word &= 0xF;
                    old_word <<= 3;
                    closing_scaled_width |= old_word;
                    *(u32 *)element_words = closing_scaled_width;
                    if (!(((u32)closing_scaled_width >> 3) & 0xF))
                    {
                        *(u32 *)element_words = ((((u32)closing_scaled_width & ~ADDHERO_ELEMENT_PHASE_MASK) | 0x18) & ~ADDHERO_ELEMENT_STATE_MASK) | ADDHERO_ELEMENT_STATE_FINISHING;
                    }
                }
                break;

            case ADDHERO_ELEMENT_STATE_FINISHING:
                finishing_word = *(u32 *)element_words;
                g_pad_input = 0;
                updated_word = (finishing_word & ~ADDHERO_ELEMENT_PHASE_MASK) | (((((finishing_word >> 3) & 0xF) - 1) & 0xF) * 8);
                *(u32 *)element_words = updated_word;
                if (!((updated_word >> 3) & 0xF))
                {
                    *(u32 *)element_words = updated_word & ~ADDHERO_ELEMENT_STATE_MASK;
                }
                break;
            }
        }
    }

    draw_state->prim_cursor = packet_cursor;
}

/**
 * @brief Free the primary pool element by clearing its state bits.
 * @see decomp.me (100%)
 */
void addhero_deactivate_primary_element(void)
{
    g_addhero_element_pool[0].attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
}

/**
 * @brief Append the multibyte string @p src onto the end of @p dst and
 *        null-terminate the result.
 * @param dst Destination string; appended to in place.
 * @param src Source string copied onto the end of @p dst.
 * @see decomp.me (100%)
 */
void addhero_text_append(u8 *dst, u8 *src)
{
    s32 dst_len;
    s32 src_len;
    s32 i;

    dst_len = addhero_text_byte_length(dst);
    src_len = addhero_text_byte_length(src);
    for (i = 0; i < src_len; i++)
    {
        dst[dst_len + i] = src[i];
    }
    dst[dst_len + i] = 0;
}

/**
 * @brief Measure the byte length of a string, counting characters in the
 *        0x19-0x1F lead range as two bytes.
 * @param str Null-terminated string to measure.
 * @return Length in bytes, excluding the terminator.
 * @see decomp.me (100%)
 */
s32 addhero_text_byte_length(u8 *str)
{
    u8 *p;
    u8 c;
    s32 len;

    p = str;
    c = *p;
    len = 0;
    while (c != 0)
    {
        if ((u32)(c - 0x19) < 7)
        {
            p += 2;
            len += 2;
        }
        else
        {
            p += 1;
            len += 1;
        }
        c = *p;
    }
    return len;
}

/**
 * @brief Copy a multibyte string, counting 0x19-0x1F lead bytes as two-byte
 *        characters when computing its length, and null-terminate the result.
 * @param dst Destination buffer.
 * @param src Source string to copy.
 * @see decomp.me (100%)
 */
void addhero_text_copy(u8 *dst, u8 *src)
{
    u8 *p;
    u8 c;
    s32 len;
    s32 i;

    p = src;
    len = 0;
    while (*p != 0)
    {
        c = *(volatile u8 *)p;
        if ((u32)(c - 0x19) < 7)
        {
            p += 2;
            len += 2;
        }
        else
        {
            p += 1;
            len += 1;
        }
    }
    for (i = 0; i < len; i++)
    {
        dst[i] = src[i];
    }
    dst[i] = 0;
}

/**
 * @brief Draw the load confirmation prompt and its yes/no choice, then act on
 *        input: cancel/back to the browser, or accept and swap this element to
 *        the load-progress bar.
 * @param ot   Ordering table the primitives are linked into.
 * @param prim Current primitive pointer/index.
 * @param x_offset Horizontal offset; screen X = 0x90 - x_offset.
 * @param y_offset Vertical offset applied to the prompt rows.
 * @return The updated primitive pointer after linking the prompt.
 * @see decomp.me (100%)
 */
s32 addhero_draw_load_prompt(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    s32 result;
    s32 x;
    s32 status;
    AddheroElement *p;

    x = -x_offset + 0x90;
    result = addhero_draw_choice_prompt(
        func_800A88A0(prim, ot,
                      (u8 *)&g_addhero_glyph_load_prompt + g_addhero_glyph_load_prompt - 0x30,
                      4, x, -y_offset, 2),
        ot, x, 0xE - y_offset);

    if ((u32)(addhero_poll_and_rewind_primary_handles() - 1) < 2U)
    {
        g_addhero_element_pool[0].attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
        func_800AA02C();
        play_menu_sfx(0x78, 0x80);
        g_addhero_entry_state = ADDHERO_ENTRY_STATE_IDLE;
        addhero_reset_entry_ranks();
        g_addhero_load_step = 0;
    }
    else
    {
        status = g_pad_input;
        if (status & 0x40)
        {
            g_addhero_element_pool[0].attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
            func_800AA02C();
            play_menu_sfx(0x78, 0x80);
            g_addhero_load_step = g_addhero_loadseq_abort;
        }
        else if (status & 0x220)
        {
            if (g_addhero_choice_toggle != 0)
            {
                g_addhero_element_pool[0].attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
                func_800AA02C();
                play_menu_sfx(0x78, 0x80);
                g_addhero_load_step = g_addhero_loadseq_abort;
            }
            else
            {
                play_menu_sfx(0x7E, 0x80);
                g_addhero_progress_active = 1;
                g_addhero_load_step = g_addhero_loadseq_load_begin;
                p = &g_addhero_element_pool[0];
                p->draw_handler = addhero_draw_load_progress;
                p->attr.bits.transition_step = 1;
                p->attr.bits.state = ADDHERO_ELEMENT_STATE_OPENING;
                p->attr.bits.x = 0x10;
                p->attr.bits.y = 0x61;
                p->size.bits.width_high = 1;
                p->size.bits.height = 0x2C;
                SET_ELEM_WIDTH_LOW(p, 0x20);
            }
        }
    }
    return result;
}

/**
 * @brief Draw the "loading new hero" prompt and, once the load has finished,
 *        validate and commit the freshly loaded save data.
 * @param ot   Ordering table the prompt primitives are linked into.
 * @param prim Current primitive pointer / index within the ordering table.
 * @param x_offset Horizontal offset used to place the prompt (screen X = 0x90 - x_offset).
 * @param y_offset Vertical offset used to place the prompt rows.
 * @return The updated primitive pointer / index after linking the prompt.
 * @see decomp.me (100%)
 */
s32 addhero_draw_load_progress(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    u8 *base;
    u8 *resource;
    AddheroElement *p;
    AddheroElement *cursor;
    s32 result;
    s32 x;
    s32 i;
    u32 saved;

    x = -x_offset + 0x90;
    result = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_load_progress, 0x32), 4, x, -y_offset, 2);
    base = (u8 *)&g_addhero_glyph_load_progress - 0x32;
    result = func_800A88A0(result, ot, base + *(u16 *)(base + 0x1E), 4, x, 0xE - y_offset, 2);
    result = func_800A88A0(result, ot, base + *(u16 *)(base + 0xB2), 4, x, 0x1C - y_offset, 2);
    result = addhero_draw_progress_bar(result, ot);

    if (g_addhero_progress_active == 0)
    {
        resource = g_addhero_save_blob;
        p = (AddheroElement *)&g_addhero_element_pool[0];
        p->attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
        if (addhero_validate_save_blob(resource) == 0)
        {
            addhero_open_status_dialog(4);
            return result;
        }

        play_menu_sfx(0x7B, 0x80);
        saved = g_pad_ctx[0x858] >> 7;
        bcopy(resource + 0x770, g_pad_ctx + 0x840, 0x250);
        *(u32 *)(g_pad_ctx + 0x858) = (*(u32 *)(g_pad_ctx + 0x858) & ~0x80) | (saved << 7);
        *(u16 *)(g_pad_ctx + 0xD8) = *(u16 *)(resource + 0x254);
        *(u16 *)(g_pad_ctx + 0xDA) = *(u16 *)(resource + 0x256);
        *(u16 *)(g_pad_ctx + 0xDE) = 1;
        field_restore_fade_target();

        cursor = p;
        for (i = 0; i < ADDHERO_ELEMENT_COUNT; i++, cursor++)
        {
            if (cursor->attr.bits.state != ADDHERO_ELEMENT_STATE_INACTIVE)
            {
                cursor->attr.bits.state = ADDHERO_ELEMENT_STATE_CLOSING;
                cursor->attr.bits.transition_step = 8;
            }
        }
        field_restore_fade_target_with_duration(8);
        g_addhero_result = 1;
    }

    return result;
}

/**
 * @brief Draw the gradient progress bar whose width tracks elapsed ticks, when
 *        the bar is active.
 * @param prim Current primitive pointer/index the POLY_G4 is written to.
 * @param ot   Ordering table the primitive is linked into.
 * @return The advanced primitive pointer, unchanged when the bar is inactive.
 * @see decomp.me (100%)
 */
s32 addhero_draw_progress_bar(s32 prim, s32 *ot)
{
    POLY_G4 *g;
    s32 elapsed;
    s32 extent;
    s32 color;

    g = (POLY_G4 *)prim;
    if (g_addhero_progress_bar_active != 0)
    {
        elapsed = VSync(-1) - g_addhero_progress_start_tick;
        if (elapsed >= 0x101)
        {
            elapsed = 0x100;
        }
        color = 0xFFFF00;
        extent = elapsed * 0x120;
        SET_BGR0_PACKED(g, 0xFF);
        SET_POLY_G4_BGR1_PACKED(g, 0xFFFF);
        SET_POLY_G4_BGR3_PACKED(g, 0xFF0000);
        ((u8 *)g)[3] = 8;
        SET_POLY_G4_BGR2_PACKED(g, color);
        g->code = 0x38;
        g->x2 = 0;
        g->x0 = 0;
        if (extent < 0)
        {
            extent += 0xFF;
        }
        g->x3 = extent >> 8;
        g->x1 = extent >> 8;
        g->y1 = 0;
        g->y0 = 0;
        g->y3 = 0x2C;
        g->y2 = 0x2C;
        addPrim(ot, g);
        prim += 0x24;
    }
    return prim;
}

/**
 * @brief Reconfigure the primary element as a modal status dialog and reset the
 *        browser/IO state, storing the dialog message id.
 * @param message_id Dialog message id stored in g_addhero_dialog_state.
 * @see decomp.me (100%)
 */
void addhero_open_status_dialog(s32 message_id)
{
    play_menu_sfx(0x78, 0x80);
    g_addhero_element_pool[0].draw_handler = (void *)addhero_draw_status_dialog;
    g_addhero_element_pool[0].attr.bits.transition_step = 1;
    g_addhero_element_pool[0].attr.bits.state = ADDHERO_ELEMENT_STATE_OPENING;
    g_addhero_element_pool[0].attr.bits.x = 0x20;
    g_addhero_element_pool[0].attr.bits.y = 0x70;
    g_addhero_element_pool[0].size.bits.width_high = 1;
    g_addhero_element_pool[0].size.bits.height = 0x14;
    SET_ELEM_WIDTH_LOW(&g_addhero_element_pool[0], 0);
    func_800AA02C();
    g_addhero_write_in_progress = 0;
    g_addhero_progress_active = 0;
    g_addhero_selection_status = 0;
    g_addhero_io_busy = 0;
    g_addhero_entry_state = ADDHERO_ENTRY_STATE_IDLE;
    addhero_reset_entry_ranks();
    g_addhero_load_step = 0;
    g_addhero_dialog_state = message_id;
}

/**
 * @brief Reconfigure the primary element as a modal exit dialog and reset the
 *        browser/IO state, storing the dialog message id.
 * @param message_id Dialog message id stored in g_addhero_dialog_state.
 * @see decomp.me (100%)
 */
void addhero_open_exit_dialog(s32 message_id)
{
    play_menu_sfx(0x78, 0x80);
    g_addhero_element1.draw_handler = (void *)addhero_draw_exit_dialog;
    g_addhero_element1.attr.bits.transition_step = 1;
    g_addhero_element1.attr.bits.state = ADDHERO_ELEMENT_STATE_OPENING;
    g_addhero_element1.attr.bits.x = 0x20;
    g_addhero_element1.attr.bits.y = 0x70;
    g_addhero_element1.size.bits.width_high = 1;
    g_addhero_element1.size.bits.height = 0x14;
    SET_ELEM_WIDTH_LOW(&g_addhero_element1, 0);
    func_800AA02C();
    g_addhero_write_in_progress = 0;
    g_addhero_progress_active = 0;
    g_addhero_selection_status = 0;
    g_addhero_io_busy = 0;
    addhero_reset_entry_ranks();
    g_addhero_load_step = 0;
    g_addhero_dialog_state = message_id;
}

/**
 * @brief Draw the status dialog message for the current dialog state and close
 *        the element once the player acknowledges it.
 * @param ot   Ordering table the message glyph is linked into.
 * @param prim Current primitive pointer/index.
 * @param x_offset Horizontal offset; screen X is derived from it.
 * @param y_offset Vertical offset.
 * @return The updated primitive pointer.
 * @see decomp.me (100%)
 */
s32 addhero_draw_status_dialog(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;

    switch (g_addhero_dialog_state)
    {
    case 0:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_dialog_msg0, 0x3C), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 2:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_dialog_msg2, 0x40), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 3:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_dialog_msg3, 0x42), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 1:
    case 4:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_dialog_msg1, 0x3E), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    }
    if (g_pad_input & 0x220)
    {
        g_addhero_element_pool[0].attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
        func_800AA02C();
    }
    return prim;
}

/**
 * @brief Draw the exit dialog message and, on acknowledge, tear down all
 *        elements and request the overlay to exit with result 3.
 * @param ot   Ordering table the message glyph is linked into.
 * @param prim Current primitive pointer/index.
 * @param x_offset Horizontal offset; screen X is derived from it.
 * @param y_offset Vertical offset.
 * @return The updated primitive pointer.
 * @see decomp.me (100%)
 */
s32 addhero_draw_exit_dialog(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;
    AddheroElement *p;
    s32 i;

    switch (g_addhero_dialog_state)
    {
    case 0:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_dialog_msg0, 0x3C), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 2:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_dialog_msg2, 0x40), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 3:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_dialog_msg3, 0x42), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 1:
    case 4:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_dialog_msg1, 0x3E), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    }
    if (g_pad_input & 0x220)
    {
        g_addhero_result = 3;
        g_menu_element_counter = 0x20;
        p = &g_addhero_element_pool[0];
        for (i = 0; i < ADDHERO_ELEMENT_COUNT; i++)
        {
            p->size.word &= ~0x200;
            p->attr.word &= ~7;
            p++;
        }
        field_restore_fade_target_with_duration(8);
        func_800AA02C();
    }
    return prim;
}

/**
 * @brief Transfer-mode driver/renderer: draws the message for the current
 *        entry-state sentinel, runs the load/save confirm and progress steps,
 *        and handles cancel/back input.
 * @param ot   Ordering table the primitives are linked into.
 * @param prim Current primitive pointer/index.
 * @param x_offset Horizontal offset; screen X = 0x90 - x_offset.
 * @param y_offset Vertical offset applied to the message rows.
 * @return The updated primitive pointer.
 */
s32 addhero_draw_transfer_status(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    RECT pos;

    switch (g_addhero_entry_state)
    {
    case 0xF8:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_status_f8, 0x34), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case 0xF9:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_status_f8, 0x34), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case ADDHERO_ENTRY_STATE_IDLE:
        {
            s32 message_x;
            u8 *glyph_base;

            message_x = -x_offset + 0x90;
            glyph_base = (u8 *)&g_addhero_glyph_table;
            prim = func_800A88A0(prim, ot, glyph_base + g_addhero_glyph_table, 4, message_x, -y_offset, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(glyph_base, 0x1E), 4, message_x, 0xE - y_offset, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(glyph_base, 0xB2), 4, message_x, 0x1C - y_offset, 2);
        }
        break;
    case 0xFA:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_status_f8, 0x34), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case 0xFD:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_status_fd, 4), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case 0xFB:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_status_fb, 0x10), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case 0xFC:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_status_fc, 0x12), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case 0xF7:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_status_f7, 0x68), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case ADDHERO_ENTRY_STATE_LOAD_PROGRESS:
        {
            s32 message_x;
            u8 *glyph_base;
            POLY_G4 *bar;
            s32 next_prim;
            s32 elapsed_frames;
            s32 bar_extent;
            s32 bar_color;
            s32 dialog_state;

            message_x = -x_offset + 0x90;
            prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_load_progress, 0x32), 4, message_x, -y_offset, 2);
            glyph_base = (u8 *)&g_addhero_glyph_load_progress - 0x32;
            prim = func_800A88A0(prim, ot, GLYPH_OFF(glyph_base, 0x1E), 4, message_x, 0xE - y_offset, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(glyph_base, 0xB2), 4, message_x, 0x1C - y_offset, 2);
            next_prim = prim;
            bar = (POLY_G4 *)prim;
            if (g_addhero_progress_bar_active != 0)
            {
                elapsed_frames = VSync(-1) - g_addhero_progress_start_tick;
                if (elapsed_frames >= 0x101)
                {
                    elapsed_frames = 0x100;
                }
                bar_color = 0xFFFF00;
                bar_extent = elapsed_frames * 0x120;
                SET_BGR0_PACKED(bar, 0xFF);
                SET_POLY_G4_BGR1_PACKED(bar, 0xFFFF);
                SET_POLY_G4_BGR3_PACKED(bar, 0xFF0000);
                setlen(bar, 8);
                SET_POLY_G4_BGR2_PACKED(bar, bar_color);
                setcode(bar, 0x38);
                bar->x2 = 0;
                bar->x0 = 0;
                if (bar_extent < 0)
                {
                    bar_extent += 0xFF;
                }
                bar->x3 = bar_extent >> 8;
                bar->x1 = bar_extent >> 8;
                bar->y1 = 0;
                bar->y0 = 0;
                bar->y3 = 0x2C;
                bar->y2 = 0x2C;
                addPrim(ot, bar);
                next_prim = prim + 0x24;
            }
            prim = next_prim;
            if (g_addhero_progress_active == 0)
            {
                if (addhero_validate_save_blob(g_addhero_save_blob) == 0)
                {
                    play_menu_sfx(0x78, 0x80);
                    g_addhero_element_pool[0].draw_handler = (void *)addhero_draw_status_dialog;
                    g_addhero_element_pool[0].attr.bits.transition_step = 1;
                    g_addhero_element_pool[0].attr.bits.state = ADDHERO_ELEMENT_STATE_OPENING;
                    g_addhero_element_pool[0].attr.bits.x = 0x20;
                    g_addhero_element_pool[0].attr.bits.y = 0x70;
                    g_addhero_element_pool[0].size.bits.width_high = 1;
                    g_addhero_element_pool[0].size.bits.height = 0x14;
                    SET_ELEM_WIDTH_LOW(&g_addhero_element_pool[0], 0);
                    func_800AA02C();
                    g_addhero_write_in_progress = 0;
                    g_addhero_selection_status = 0;
                    g_addhero_io_busy = 0;
                    g_addhero_progress_active = 0;
                    g_addhero_entry_state = ADDHERO_ENTRY_STATE_IDLE;
                    addhero_reset_entry_ranks();
                    dialog_state = 4;
                    g_addhero_load_step = 0;
                    g_addhero_dialog_state = dialog_state;
                    return prim;
                }
                play_menu_sfx(0x7B, 0x80);
                g_addhero_entry_state = ADDHERO_ENTRY_STATE_SAVE_CONFIRM;
                addhero_enable_choice_toggle();
                func_800AA02C();
            }
        }
        break;
    case ADDHERO_ENTRY_STATE_CONFIRM_PROMPT:
        {
            s32 message_x;
            AddheroElement *element;
            s32 i;

            message_x = -x_offset + 0x90;
            prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_status_f3, 0x6E), 4, message_x, -y_offset, 2);
            prim = addhero_draw_choice_prompt(prim, ot, message_x, 0xE - y_offset);
            if (g_pad_input & PAD_BTN_CIRCLE)
            {
                play_menu_sfx(0x78, 0x80);
                addhero_enable_choice_toggle();
                g_addhero_entry_state = ADDHERO_ENTRY_STATE_SAVE_CONFIRM;
                func_800AA02C();
            }
            else if (g_pad_input & ADDHERO_CONFIRM_BUTTON_MASK)
            {
                if (g_addhero_choice_toggle != 0)
                {
                    play_menu_sfx(0x78, 0x80);
                    addhero_enable_choice_toggle();
                    g_addhero_entry_state = ADDHERO_ENTRY_STATE_SAVE_CONFIRM;
                    func_800AA02C();
                }
                else
                {
                    play_menu_sfx(0x7D, 0x80);
                    g_addhero_result = 3;
                    g_menu_element_counter = 0x20;
                    element = g_addhero_element_pool;
                    for (i = 0; i < ADDHERO_ELEMENT_COUNT; i++, element++)
                    {
                        element->size.bits.scrollable = 0;
                        element->attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
                    }
                    field_restore_fade_target_with_duration(8);
                    func_800AA02C();
                }
            }
        }
        break;
    case ADDHERO_ENTRY_STATE_SAVE_CONFIRM:
        {
            s32 message_x;
            u8 *base;
            s32 checksum;

            message_x = -x_offset + 0x90;
            prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_save_confirm_msg, 0x6A), 4, message_x, -y_offset, 2);
            base = (u8 *)&g_addhero_glyph_save_confirm_msg - 0x6A;
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x6C), 4, message_x, 0xE - y_offset, 2);
            prim = addhero_draw_choice_prompt(prim, ot, message_x, 0x1C - y_offset);
            if (g_pad_input & PAD_BTN_CIRCLE)
            {
                addhero_enable_choice_toggle();
                g_addhero_entry_state = ADDHERO_ENTRY_STATE_CONFIRM_PROMPT;
                play_menu_sfx(0x78, 0x80);
                func_800AA02C();
            }
            else if (g_pad_input & ADDHERO_CONFIRM_BUTTON_MASK)
            {
                if (g_addhero_choice_toggle != 0)
                {
                    addhero_enable_choice_toggle();
                    g_addhero_entry_state = ADDHERO_ENTRY_STATE_CONFIRM_PROMPT;
                    play_menu_sfx(0x78, 0x80);
                    func_800AA02C();
                }
                else
                {
                    play_menu_sfx(0x7E, 0x80);
                    base = g_addhero_save_blob;
                    bcopy(g_pad_ctx + 0x840, &((AddheroSaveBlob *)base)->context, sizeof(AddheroSaveContextBlock));
                    ((AddheroSaveBlob *)base)->context.inject_flags |= ADDHERO_INPUT_INJECTION_ENABLED;
                    checksum = addhero_compute_save_checksum(base);
                    ((AddheroSaveBlob *)base)->magic = ADDHERO_SAVE_MAGIC;
                    ((AddheroSaveBlob *)base)->checksum = checksum;
                    g_addhero_write_in_progress = 1;
                    g_addhero_load_step = &g_addhero_loadseq_save_begin;
                    g_addhero_entry_state = ADDHERO_ENTRY_STATE_SAVE_PROGRESS;
                }
            }
        }
        break;
    case ADDHERO_ENTRY_STATE_SAVE_PROGRESS:
        {
            s32 message_x;
            u8 *glyph_base;
            POLY_G4 *bar;
            s32 next_prim;
            s32 elapsed_frames;
            s32 bar_extent;
            s32 bar_color;
            AddheroElement *element;
            s32 i;

            message_x = -x_offset + 0x90;
            prim = func_800A88A0(prim, ot, GLYPH_SYM(g_addhero_glyph_save_progress, 0x1C), 4, message_x, -y_offset, 2);
            glyph_base = (u8 *)&g_addhero_glyph_save_progress - 0x1C;
            prim = func_800A88A0(prim, ot, GLYPH_OFF(glyph_base, 0x1E), 4, message_x, 0xE - y_offset, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(glyph_base, 0xB2), 4, message_x, 0x1C - y_offset, 2);
            next_prim = prim;
            bar = (POLY_G4 *)prim;
            if (g_addhero_progress_bar_active != 0)
            {
                elapsed_frames = VSync(-1) - g_addhero_progress_start_tick;
                if (elapsed_frames >= 0x101)
                {
                    elapsed_frames = 0x100;
                }
                bar_color = 0xFFFF00;
                bar_extent = elapsed_frames * 0x120;
                SET_BGR0_PACKED(bar, 0xFF);
                SET_POLY_G4_BGR1_PACKED(bar, 0xFFFF);
                SET_POLY_G4_BGR3_PACKED(bar, 0xFF0000);
                setlen(bar, 8);
                SET_POLY_G4_BGR2_PACKED(bar, bar_color);
                setcode(bar, 0x38);
                bar->x2 = 0;
                bar->x0 = 0;
                if (bar_extent < 0)
                {
                    bar_extent += 0xFF;
                }
                bar->x3 = bar_extent >> 8;
                bar->x1 = bar_extent >> 8;
                bar->y1 = 0;
                bar->y0 = 0;
                bar->y3 = 0x2C;
                bar->y2 = 0x2C;
                addPrim(ot, bar);
                next_prim = prim + 0x24;
            }
            prim = next_prim;
            if (g_addhero_write_in_progress == 0)
            {
                g_pad_ctx[0x840] = 0;
                play_menu_sfx(0x7A, 0x80);
                g_menu_element_counter = 0x20;
                element = g_addhero_element_pool;
                for (i = 0; i < ADDHERO_ELEMENT_COUNT; i++, element++)
                {
                    element->size.bits.scrollable = 0;
                    element->attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
                }
                field_restore_fade_target_with_duration(8);
                g_addhero_result = 2;
            }
        }
        break;
    default:
        {
            s32 message_x;
            s32 selected_y;
            s32 scroll_delta;
            u8 *glyph_base;

            message_x = -x_offset + 0x90;
            glyph_base = (u8 *)&g_addhero_glyph_table;
            prim = func_800A88A0(prim, ot, glyph_base + g_addhero_glyph_table, 4, message_x, -y_offset, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(glyph_base, 0x1E), 4, message_x, 0xE - y_offset, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(glyph_base, 0xB2), 4, message_x, 0x1C - y_offset, 2);
            if (g_addhero_entry_scan_active == 0)
            {
                if (g_addhero_io_busy != 0)
                {
                    return prim;
                }
                if ((u32)(*g_addhero_load_step - 6) < 2U)
                {
                    return prim;
                }
                if ((strncmp(g_lom_save_filename_prefix, &g_addhero_entries[g_addhero_card_slot][g_addhero_selected_row], 0xC) != 0) ||
                    (g_addhero_entry_identity != ((AddheroRecord *)g_pad_ctx)->identity))
                {
                    g_addhero_selected_row++;
                    if (g_addhero_selected_row >= g_addhero_entry_state)
                    {
                        if (g_addhero_entry_state != 0)
                        {
                            g_addhero_entry_state = 0xF7;
                        }
                        else
                        {
                            g_addhero_entry_state = 0xF8;
                        }
                    }
                    else
                    {
                        addhero_commit_selected_entry();
                        selected_y = g_addhero_selected_row * 0xE;
                        scroll_delta = selected_y - g_addhero_scroll_y;
                        if (scroll_delta >= 0x4B)
                        {
                            g_addhero_scroll_target_y = selected_y - 0x46;
                            g_addhero_scroll_frames = 4;
                        }
                        if (scroll_delta < 0)
                        {
                            g_addhero_scroll_target_y = selected_y;
                            g_addhero_scroll_frames = 4;
                        }
                    }
                }
                else
                {
                    g_addhero_progress_start_tick = VSync(-1);
                    g_addhero_progress_active = 1;
                    g_addhero_load_step = &g_addhero_loadseq_load_progress;
                    g_addhero_entry_state = ADDHERO_ENTRY_STATE_LOAD_PROGRESS;
                }
            }
        }
        break;
    case 0xFE:
        break;
    }

    if (g_addhero_io_busy != 0)
    {
        return prim;
    }
    if (g_addhero_entry_state == ADDHERO_ENTRY_STATE_LOAD_PROGRESS)
    {
        return prim;
    }
    if (g_addhero_entry_state == ADDHERO_ENTRY_STATE_SAVE_PROGRESS)
    {
        return prim;
    }
    if (g_addhero_entry_state == ADDHERO_ENTRY_STATE_SAVE_CONFIRM)
    {
        return prim;
    }
    if (g_addhero_entry_state == ADDHERO_ENTRY_STATE_CONFIRM_PROMPT)
    {
        return prim;
    }

    if (g_pad_input & PAD_BTN_CIRCLE)
    {
        AddheroElement *element;
        s32 i;
        s32 attr;

        D_80122718 = 3;
        play_menu_sfx(0x78, 0x80);
        field_restore_fade_target();
        element = g_addhero_element_pool;
        i = 0;
        do
        {
            attr = element->attr.word;
            if (attr & ADDHERO_ELEMENT_STATE_MASK)
            {
                element->attr.word = (((attr & ~ADDHERO_ELEMENT_STATE_MASK) | ADDHERO_ELEMENT_STATE_CLOSING) & ~ADDHERO_ELEMENT_PHASE_MASK) | 0x40;
            }
            i++;
            element++;
        } while (i < ADDHERO_ELEMENT_COUNT);
        return prim;
    }

    if ((g_pad_input & ADDHERO_CARD_SWITCH_BUTTON_MASK) && (g_addhero_entry_state != ADDHERO_ENTRY_STATE_IDLE))
    {
        play_menu_sfx(0x7D, 0x80);
        g_addhero_load_flow_active = 0;
        g_addhero_load_step = 0;
        g_addhero_scroll_frames = 0;
        g_addhero_scroll_target_y = 0;
        g_addhero_scroll_y = 0;
        g_addhero_selected_row = 0;
        g_addhero_entry_state = ADDHERO_ENTRY_STATE_IDLE;
        g_addhero_selection_status = 0;
        g_addhero_card_slot ^= 1;
        addhero_reset_entry_ranks();
        func_800AA02C();
        g_addhero_progress_bar_active = 0;
        g_pad_input = 0;
        g_addhero_load_step = &g_addhero_loadseq_start;
    }
    return prim;
}

/**
 * @brief Blit one character-slot icon into VRAM and emit the highlighted
 *        textured quad for it in the detail panel.
 * @param result Current primitive pointer/index the POLY_FT4 is written to.
 * @param ot     Ordering table the quad is linked into.
 * @param x      Left edge of the quad.
 * @param y      Top edge of the quad.
 * @param adjust Width added to the base quad for the highlight animation.
 * @param slot   Character/icon id for this position (0x7F means empty).
 * @param i      Index among the present (non-empty) slots.
 * @param j      Index among all three slots.
 * @return The advanced primitive pointer (result + 0x28), or @p result when the
 *         slot is empty.
 * @see decomp.me (100%)
 */
s32 addhero_draw_icon_highlight(s32 result, s32 *ot, s32 x, s32 y, s32 adjust, s32 slot, s32 i, s32 j)
{
    RECT rect;
    s32 temp;
    s8 shade;

    if (slot == 0x7F) return result;
    rect.x = i * 0x10;
    rect.y = 0x1F2;
    rect.w = 0x10;
    rect.h = 1;
    if ((j == 1) && (slot < 2)) {
        func_800A5638(g_addhero_icon_context, slot);
        func_80019A34(&rect, g_addhero_icon_context);
        func_80019788(0);
    } else if (slot >= 0x4F) {
        func_800A55E4(g_addhero_icon_context, g_addhero_icon_palette);
        func_80019A34(&rect, g_addhero_icon_context);
        func_80019788(0);
    } else {
        func_80019A34(&rect, (void *)((u8 *)&g_addhero_icon_image_table - 4 + g_addhero_icon_image_table[slot]));
    }
    temp = i * 3;
    rect.x = temp * 4 + 0x140;
    rect.y = 0xD0;
    rect.w = 0xC;
    rect.h = 0x30;
    func_80019A34(&rect, (void *)((u8 *)&g_addhero_icon_image_table + 0x1C + g_addhero_icon_image_table[slot]));
    SET_BGR0_PACKED((POLY_FT4 *)result, 0x808080);
    ((u8 *)result)[3] = 9;
    ((POLY_FT4 *)result)->code = 0x2C;
    ((POLY_FT4 *)result)->x2 = x;
    ((POLY_FT4 *)result)->x0 = x;
    ((POLY_FT4 *)result)->y1 = y;
    ((POLY_FT4 *)result)->y0 = y;
    ((POLY_FT4 *)result)->x3 = x + adjust;
    shade = temp * 0x10;
    ((POLY_FT4 *)result)->u2 = shade;
    ((POLY_FT4 *)result)->u0 = shade;
    shade += 0x2F;
    ((POLY_FT4 *)result)->u3 = shade;
    ((POLY_FT4 *)result)->u1 = shade;
    ((POLY_FT4 *)result)->v1 = 0xD0;
    ((POLY_FT4 *)result)->v0 = 0xD0;
    ((POLY_FT4 *)result)->x1 = x + adjust;
    ((POLY_FT4 *)result)->y3 = y + 0x2F;
    ((POLY_FT4 *)result)->y2 = y + 0x2F;
    ((POLY_FT4 *)result)->v3 = 0xFF;
    ((POLY_FT4 *)result)->v2 = 0xFF;
    ((POLY_FT4 *)result)->clut = (i & 0x3F) | 0x7C80;
    ((POLY_FT4 *)result)->tpage = 5;
    addPrim(ot, result);
    return result + 0x28;
}

/**
 * @brief Select the second (cancel) option as the default choice.
 * @see decomp.me (100%)
 */
void addhero_enable_choice_toggle(void)
{
    g_addhero_choice_toggle = 1;
}

/**
 * @brief Draw the two-option (yes/no) choice glyphs, highlighting the current
 *        selection, and flip the selection on left/right pad input.
 * @param prim Current primitive pointer/index.
 * @param ot   Ordering table the glyphs are linked into.
 * @param x    Center X the two options are placed around.
 * @param y    Baseline Y for both options.
 * @return The updated primitive pointer.
 * @see decomp.me (100%)
 */
s32 addhero_draw_choice_prompt(s32 prim, s32 *ot, s32 x, s32 y)
{
    u8 *p;
    u8 *base;
    s32 g1;
    s32 g2;
    s32 hi;
    s32 a3;

    p = (u8 *)&g_text_choice_glyph_offsets;
    hi = p[1] << 8;
    base = p - 0x36;
    a3 = 4;
    g1 = p[0] + (hi + (s32)base);
    if (g_addhero_choice_toggle != 0)
    {
        a3 = 5;
    }
    prim = func_800A88A0(prim, ot, (void *)g1, a3, x - 0x10, y, 1);
    a3 = 4;
    g2 = base[0x38] + ((base[0x39] << 8) + (s32)base);
    if (g_addhero_choice_toggle == 0)
    {
        a3 = 5;
    }
    prim = func_800A88A0(prim, ot, (void *)g2, a3, x + 8, y, 0);
    if (g_pad_input & 0xA000)
    {
        g_addhero_choice_toggle ^= 1;
        play_menu_sfx(0x7D, 0x80);
        g_pad_input = 0;
    }
    return prim;
}

/**
 * @brief Validate a loaded save blob by checking its stored checksum and the
 *        "ANA" magic tag.
 * @param base Base of the 0x4000-byte save blob.
 * @return 1 when the checksum and magic both match, 0 otherwise.
 */
s32 addhero_validate_save_blob(u8 *base)
{
    AddheroSaveBlob *save;

    save = (AddheroSaveBlob *)base;
    if (save->checksum == addhero_compute_save_checksum(base))
    {
        if (save->magic == ADDHERO_SAVE_MAGIC)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Compute the save-blob checksum over the first 0x33E0 bytes.
 * @param data Base of the save blob.
 * @return Twice the byte sum plus ADDHERO_SAVE_CHECKSUM_BIAS.
 */
s32 addhero_compute_save_checksum(u8 *data)
{
    s32 sum;
    u32 i;
    u8 *p;

    p = data;
    sum = 0;
    i = 0;
    do
    {
        i++;
        sum += *p;
        p++;
    } while (i < ADDHERO_SAVE_CHECKSUM_BYTES);
    return (sum * 2) + ADDHERO_SAVE_CHECKSUM_BIAS;
}

/**
 * @brief Format @p value as a big-endian double-byte decimal glyph string,
 *        suppressing leading zeros; emits a fixed overflow string past 999999.
 * @param out Destination glyph buffer.
 * @param value Value to format.
 * @return Pointer to the terminator written after the last glyph.
 */
s8 *addhero_format_decimal(s8 *out, s32 value)
{
    struct OverflowGlyphString { s8 data[7]; };
    extern s8 g_addhero_decimal_overflow_glyphs[];
    s32 digit;
    s32 divisor;
    s32 started;
    s8 *p;

    p = out;
    divisor = 100000;
    if (value >= divisor * 10)
    {
        *(struct OverflowGlyphString *)p = *(struct OverflowGlyphString *)g_addhero_decimal_overflow_glyphs;
        return p + 6;
    }

    started = 0;
    do
    {
        digit = value / divisor;
        if (digit != 0 || started != 0)
        {
            *p++ = (digit + 0x824F) >> 8;
            *p++ = digit + 0x4F;
            started = 1;
        }
        if (divisor == 1)
        {
            break;
        }
        if (divisor == 10)
        {
            started = 1;
        }
        value -= digit * divisor;
        divisor /= 10;
    } while (1);
    *p = 0;
    return p;
}

/**
 * @brief Format @p value as an ASCII hex string of up to @p max_chars digits,
 *        suppressing leading zeros, and null-terminate it.
 * @param out       Destination character buffer.
 * @param value     Value to format.
 * @param max_chars Maximum number of hex digits to emit.
 */
void addhero_format_hex(s8 *out, s32 value, s32 max_chars)
{
    s32 nibble;
    s32 shift_index;
    s32 started;

    shift_index = 7;
    started = 0;
    if (max_chars != 0)
    {
        do
        {
            nibble = (value >> (shift_index * 4)) & 0xF;
            if (nibble != 0 || started != 0)
            {
                addhero_hex_nibble_to_ascii(out, nibble);
                out++;
                max_chars--;
                started = 1;
                value -= nibble << (shift_index * 4);
            }
            shift_index--;
            if (shift_index == -1)
            {
                break;
            }
            if (shift_index == 0)
            {
                started = 1;
            }
        } while (max_chars);
    }
    *out = 0;
}

/**
 * @brief Write one nibble as its ASCII hex digit ('0'-'9', 'A'-'F'), or '_' for
 *        out-of-range values.
 * @param out   Destination byte.
 * @param value Nibble value to convert.
 * @see decomp.me (100%)
 */
void addhero_hex_nibble_to_ascii(s8 *out, s32 value)
{
    if (value < 10)
    {
        *out = value + 0x30;
    }
    else if (value < 16)
    {
        *out = value + 0x37;
    }
    else
    {
        *out = 0x5F;
    }
}

/**
 * @brief Parse up to @p len leading hex digits from @p s into an integer.
 * @param s   Text to parse.
 * @param len Maximum number of hex digits to consume.
 * @return The parsed value; 0 when no hex digits are present.
 * @see decomp.me (100%)
 */
u32 addhero_parse_hex(u8 *s, s32 len)
{
    u32 result;
    u32 tmp0;
    u32 tmp1;
    u32 tmp2;

    result = 0;
    while (((u8)(*s - '0') < 10) || ((u8)(*s - 'a') < 6) || ((u8)(*s - 'A') < 6))
    {
        if (len == 0)
        {
            break;
        }
        result <<= 4;
        if ((u8)(*s - '0') < 10)
        {
            tmp0 = result - 0x30;
            result = tmp0 + *s;
        }
        else if ((u8)(*s - 'A') < 6)
        {
            tmp1 = result - 0x37;
            result = tmp1 + *s;
        }
        else if ((u8)(*s - 'a') < 6)
        {
            tmp2 = result - 0x57;
            result = tmp2 + *s;
        }
        s++;
        len--;
    }
    return result;
}

/**
 * @brief Skip the leading hex-digit run of a field, then parse the next two hex
 *        digits (the suffix byte) that follow it.
 * @param text Field text to scan.
 * @return The parsed two-digit suffix byte value.
 * @see decomp.me (100%)
 */
s32 addhero_parse_hex_suffix_byte(u8 *text)
{
    u32 c;
    s32 count;
    u32 result;
    u32 tmp0;
    u32 tmp1;
    u32 tmp2;

    while (1)
    {
        c = *text;
        text++;
        if ((u32)(c - '0') < 10)
        {
            continue;
        }
        text--;
        if (text)
        {
            text++;
            text--;
        }

        text++;
        if ((u32)(c - 'a') < 6)
        {
            continue;
        }
        text--;
        if (text)
        {
            text++;
            text--;
        }

        text++;
        if ((u32)(c - 'A') < 6)
        {
            continue;
        }
        text--;
        if (text)
        {
            text++;
            text--;
        }
        break;
    }

    text++;
    count = 2;
    result = 0;
    while (((u8)(*text - '0') < 10) || ((u8)(*text - 'a') < 6) || ((u8)(*text - 'A') < 6))
    {
        if (count == 0)
        {
            break;
        }
        result <<= 4;
        if ((u8)(*text - '0') < 10)
        {
            tmp0 = result - 0x30;
            result = tmp0 + *text;
        }
        else if ((u8)(*text - 'A') < 6)
        {
            tmp1 = result - 0x37;
            result = tmp1 + *text;
        }
        else if ((u8)(*text - 'a') < 6)
        {
            tmp2 = result - 0x57;
            result = tmp2 + *text;
        }
        text++;
        count--;
    }
    return result;
}

/**
 * @brief Parse the hex value suffix of every "SD"-tagged directory entry on the
 *        active card, recording per-entry field values and their ranked bytes.
 * @return The maximum suffix byte value seen across all matching entries.
 * @see decomp.me (100%) https://decomp.me/scratch/7hY8R
 */
s32 addhero_parse_entry_fields(void)
{
    s32 i;
    s32 max;
    u8 *p;
    u8 *field;
    s32 count;
    s32 acc;
    u32 tmp0, tmp1, tmp2;
    s32 r;
    s32 *fields;

    max = 0;

    for (i = 0; i < g_addhero_entry_state; i++)
    {
        char *ref;
        u8 *tmp;
        ref = g_lom_save_filename_prefix;
        tmp = (u8 *)&g_addhero_entries[g_addhero_card_slot][i];

        if (strncmp(ref, tmp, 0xC) == 0)
        {
            p = (u8 *)(
                g_addhero_card_slot * ADDHERO_CARD_DIRECTORY_BYTES +
                ((i << 4) + (i << 4) + (i << 3)) +
                (s32)g_addhero_entries +
                0xC);

            for (count = 5, acc = 0;
                 (((u8)(*p - '0') < 10) ||
                  ((u8)(*p - 'a') < 6) ||
                  ((u8)(*p - 'A') < 6)) &&
                     count != 0;
                 p++, count--)
            {
                acc <<= 4;

                if ((u8)(*p - '0') < 10)
                {
                    tmp0 = acc - 0x30;
                    acc = tmp0 + *p;
                }
                else if ((u8)(*p - 'A') < 6)
                {
                    tmp1 = acc - 0x37;
                    acc = tmp1 + *p;
                }
                else if ((u8)(*p - 'a') < 6)
                {
                    tmp2 = acc - 0x57;
                    acc = tmp2 + *p;
                }
            }

            field = (u8 *)&g_addhero_entries[g_addhero_card_slot][i].name[0xC];
            fields = &g_addhero_entry_fields[g_addhero_card_slot * ADDHERO_DIRECTORY_ENTRY_COUNT];
            fields[i] = acc;

            r = addhero_parse_hex_suffix_byte(field);
            g_addhero_entry_suffix_values[i] = r;

            if (max < r)
            {
                max = r;
            }
        }
        else
        {
            s32 *fields = &g_addhero_entry_fields[g_addhero_card_slot * ADDHERO_DIRECTORY_ENTRY_COUNT];
            fields[i] = -1;
            g_addhero_entry_suffix_values[i] = 0;
        }
    }

    return max;
}

/**
 * @brief Rank the current card's entries by parsed field value, tag "full"
 *        entries, and pick the highest-valued entry to select.
 * @param unused0 Unused; kept for the original signature.
 * @param unused1 Unused; kept for the original signature.
 * @param unused2 Unused; kept for the original signature.
 * @return Index of the highest-valued entry.
 * @see decomp.me (100%)
 */
s32 addhero_rank_entries(s32 unused0, s32 unused1, s32 unused2)
{
    s32 *row;
    s32 *elem;
    s32 *rank_ptr;
    s32 *cmp_ptr;
    s32 *inc_ptr;
    s32 *base_rank;
    s32 *ecopy;
    s32 *max_ptr;
    s32 *field_base;
    s32 *field1;
    s32 slot;
    s32 *out_ptr;
    char *ent_ptr;
    s32 t0v;
    s32 i;
    s32 s3v;
    s32 count;
    s32 handle;
    s32 less_count;
    s32 j;

    addhero_parse_entry_fields();
    s3v = -1;
    addhero_sort_entries_by_type();
    i = 0;
    handle = addhero_parse_entry_fields();
    addhero_reset_entry_ranks();
    t0v = 1;
    if (g_addhero_entry_state > 0)
    {
        count = g_addhero_entry_state;
        base_rank = &g_addhero_entry_ranks[0];
        rank_ptr = base_rank;
        slot = g_addhero_card_slot;
        field1 = g_addhero_entry_fields;
        row = field1 + slot * ADDHERO_DIRECTORY_ENTRY_COUNT;
        elem = row;
        do
        {
            if (*elem >= 0)
            {
                j = 0;
                if (i > 0)
                {
                    j += 1; j -= 1;
                }
                if (*elem >= s3v)
                {
                    *rank_ptr = t0v;
                    s3v = *elem;
                    t0v += 1;
                }
                else
                {
                    less_count = j;
                    if (i > 0)
                    {
                        ecopy = elem;
                        inc_ptr = base_rank;
                        cmp_ptr = row;
                        do
                        {
                            if (*ecopy < *cmp_ptr)
                            {
                                less_count += 1;
                                *inc_ptr += 1;
                            }
                            inc_ptr += 1;
                            j += 1;
                            cmp_ptr += 1;
                        } while (j < i);
                    }
                    {
                        s32 rank_value;
                        do { do { do { rank_value = t0v - less_count; } while (0); } while (0); } while (0);
                        *rank_ptr = rank_value;
                    }
                    t0v += 1;
                }
            }
            rank_ptr += 1;
            i += 1;
            elem += 1;
        } while (i < count);
    }
    cmp_ptr = base_rank;
    inc_ptr = row;
    g_addhero_rank_count = t0v;
    t0v = -1;
    i = 0;
    s3v = 0;
    if (g_addhero_entry_state > 0)
    {
        s32 max_count;
        max_count = g_addhero_entry_state;
        slot = g_addhero_card_slot;
        field_base = g_addhero_entry_fields;
        max_ptr = (s32 *)((slot * 0x50) + (s32)field_base);
        do
        {
            if (t0v < *max_ptr)
            {
                t0v = *max_ptr;
                s3v = i;
            }
            i += 1;
            max_ptr += 1;
        } while (i < max_count);
        i = 0;
    }
    g_addhero_entry_value_limit = t0v + 1;
    if (g_addhero_entry_state > 0)
    {
        out_ptr = &g_addhero_entry_suffix_values[0];
        ent_ptr = (char *)&g_addhero_entries[0];
    loop_20:
        if (strncmp(&g_new_save_entry_prefix[0], (void *)((g_addhero_card_slot * ADDHERO_CARD_DIRECTORY_BYTES) + (s32)ent_ptr), 8) == 0)
        {
            *out_ptr = handle + 1;
        }
        else
        {
            out_ptr += 1;
            ent_ptr += 0x28;
            i += 1;
            if (i < g_addhero_entry_state)
            {
                goto loop_20;
            }
        }
    }
    return s3v;
}

/**
 * @brief Reset the 15 per-entry rank slots to -1 and the rank count to 0x28.
 * @see decomp.me (100%)
 */
void addhero_reset_entry_ranks(void)
{
    s32 i;
    s32 val;

    g_addhero_rank_count = 0x28;
    val = -1;
    for (i = 14; i >= 0; i--)
    {
        g_addhero_entry_ranks[i] = val;
    }
}

/**
 * @brief Test whether the active card holds at least one entry matching a known
 *        save-name prefix.
 * @return 1 if a known-type entry exists, 0 otherwise.
 * @see decomp.me (100%)
 */
s32 addhero_has_known_entry_type(void)
{
    s32 i;
    u8 *entry;

    i = 0;
    if (g_addhero_entry_state > 0)
    {
        do
        {
            entry = (u8 *)g_addhero_entries + i * ADDHERO_DIRECTORY_ENTRY_BYTES;
            if (strncmp(&g_lom_save_filename_prefix, (void *)(g_addhero_card_slot * ADDHERO_CARD_DIRECTORY_BYTES + (s32)entry), 0xC) == 0 ||
                strncmp(&g_lom_alt_save_filename_prefix, (void *)(g_addhero_card_slot * ADDHERO_CARD_DIRECTORY_BYTES + (s32)entry), 0xC) == 0)
            {
                return 1;
            }
            i++;
        } while (i < g_addhero_entry_state);
    }
    return 0;
}

/**
 * @brief Sum the block usage of the active card's entries and test whether it
 *        has reached the card's capacity.
 * @return 1 when total used blocks are >= 0xE, 0 otherwise.
 * @see decomp.me (100%)
 */
s32 addhero_entry_blocks_reach_limit(void)
{
    s32 i;
    s32 sum;
    s32 offset;

    i = 0;
    sum = 0;
    if (g_addhero_entry_state > 0)
    {
        offset = g_addhero_card_slot * ADDHERO_CARD_DIRECTORY_BYTES;
        do
        {
            do {
                sum += ((struct DIRENTRY *)((u8 *)g_addhero_entries + offset))->size / 8192;
            } while (0);
            i++;
            offset += ADDHERO_DIRECTORY_ENTRY_BYTES;
        } while (i < g_addhero_entry_state);
    }
    return sum >= 0xE;
}

/**
 * @brief Issue the two fixed card-directory probe requests for the active slot.
 * @see decomp.me (100%)
 */
void addhero_render_fixed_prompts(void)
{
    AddheroFileHeaderScratch buf;

    memcpy(&buf, &g_addhero_file_template, 6);
    ((u8 *)&buf)[2] += *(u8 *)&g_addhero_card_slot;
    strcat(&buf, &g_lom_save_dummy_filename);
    erase(&buf);

    memcpy(&buf, &g_addhero_file_template, 6);
    ((u8 *)&buf)[2] += *(u8 *)&g_addhero_card_slot;
    strcat(&buf, &g_lom_alt_save_dummy_filename);
    erase(&buf);
}

/**
 * @brief Inlined helper mirroring addhero_render_fixed_prompts: issue the two
 *        fixed card-directory probe requests for the active slot.
 * @see decomp.me (100%)
 */
static inline void addhero_probe_render_two(void)
{
    AddheroFileHeaderScratch p;

    memcpy(&p, &g_addhero_file_template, 6);
    ((u8 *)&p)[2] += *(u8 *)&g_addhero_card_slot;
    strcat(&p, &g_lom_save_dummy_filename);
    erase(&p);

    memcpy(&p, &g_addhero_file_template, 6);
    ((u8 *)&p)[2] += *(u8 *)&g_addhero_card_slot;
    strcat(&p, &g_lom_alt_save_dummy_filename);
    erase(&p);
}

/**
 * @brief Advance the active memory-card load/save sequence by one step.
 * @return One of the ADDHERO_LOAD_RESULT_* values describing how the caller
 *         should continue the sequence.
 */
s32 addhero_advance_load_sequence(void)
{
    AddheroLoadScratch card_path;
    s32 card_status0;
    s32 card_status1;
    s32 result;
    s32 attempts;
    s32 poll_status;
    s32 io_status;
    s32 entry_index;
    s32 empty_rank;
    s32 load_step;
    static void *const load_step_targets[] __attribute__((section(".discard"))) = {
        &&load_step_idle, &&load_step_card_info, &&load_step_poll_card_info, &&load_step_release_primary,
        &&load_step_poll_secondary, &&load_step_release_secondary, &&load_step_scan_entries, &&done,
        &&load_step_clear_card, &&load_step_load_card, &&load_step_erase_entry, &&done,
        &&done, &&done, &&done, &&load_step_poll_card_load,
        &&load_step_wait_secondary, &&load_step_read_entry, &&load_step_poll_entry_read, &&load_step_read_save,
        &&load_step_poll_save_read, &&done, &&done, &&done,
        &&load_step_check_card_type, &&load_step_write_save, &&load_step_poll_save_write, &&load_step_read_before_write,
        &&load_step_poll_prewrite_read, &&done, &&load_step_init_retries
    };

    memcpy(&card_path, &g_addhero_file_template, 6);
    result = ADDHERO_LOAD_RESULT_PENDING;
    ((u8 *)&card_path)[2] += *(u8 *)&g_addhero_card_slot;

    if (g_addhero_load_step == NULL)
    {
        goto done;
    }

    load_step = *g_addhero_load_step;
    if ((u32)load_step >= ADDHERO_LOAD_STEP_COUNT)
    {
        goto done;
    }
    goto *jtbl_80140098[load_step];

    load_step_card_info:
        result = ADDHERO_LOAD_RESULT_CONTINUE;
        _card_wait(g_addhero_card_slot);
        _card_info(g_addhero_card_slot * 0x10);
        g_addhero_load_step++;
        goto done;

    load_step_poll_card_info:
        poll_status = addhero_poll_primary_handle_group();
        if (poll_status >= 3)
        {
            goto poll_card_info_ge3;
        }
        if (poll_status > 0)
        {
            goto card_info_error;
        }
        if (poll_status == 0)
        {
            goto advance_after_card_info;
        }
        goto done;
    poll_card_info_ge3:
        if (poll_status == 3)
        {
            goto card_info_reset;
        }
        goto done;
    advance_after_card_info:
        g_addhero_load_step++;
        goto done;
    card_info_error:
        result = ADDHERO_LOAD_RESULT_COMPLETE;
        g_addhero_selection_status = 0;
        g_addhero_entry_state = 0xFD;
        g_addhero_load_step++;
        goto done;
    card_info_reset:
        g_addhero_rank_count = 0x28;
        empty_rank = -1;
        for (entry_index = 14; entry_index >= 0; entry_index--)
        {
            g_addhero_entry_ranks[entry_index] = empty_rank;
        }
        g_addhero_entry_state = ADDHERO_ENTRY_STATE_IDLE;
        g_addhero_load_step = &g_addhero_loadseq_start;
        goto done;

    load_step_release_primary:
        addhero_release_primary_handles();
        g_addhero_load_step++;
        goto done;

    load_step_poll_secondary:
        do
        {
            poll_status = addhero_poll_secondary_handle_group();
        } while (poll_status == -1);
        if (poll_status == 0)
        {
            g_addhero_load_step++;
            goto done;
        }
        if (poll_status < 0)
        {
            goto done;
        }
        if (poll_status >= 4)
        {
            goto done;
        }
        result = ADDHERO_LOAD_RESULT_COMPLETE;
        g_addhero_selection_status = 0;
        g_addhero_entry_state = 0xFD;
        goto done;

    load_step_release_secondary:
        addhero_release_secondary_handles();
        g_addhero_load_step++;
        goto done;

    load_step_scan_entries:
        addhero_probe_render_two();
        g_addhero_entry_scan_active = 1;
        if (addhero_begin_entry_scan(g_addhero_card_slot) == 0)
        {
            result = ADDHERO_LOAD_RESULT_ABORT;
            g_addhero_load_step = NULL;
            g_addhero_entry_state = 0xF8;
            g_addhero_entry_scan_active = 0;
            goto done;
        }
        attempts = 0;
        g_addhero_load_step++;
        do
        {
            if (addhero_scan_next_entry(g_addhero_card_slot) == 0)
            {
                if (g_addhero_mode != 0)
                {
                    g_addhero_selected_row = 0;
                }
                g_addhero_entry_scan_active = 0;
                if (g_addhero_entry_state == 0xF8)
                {
                    break;
                }
                if (g_addhero_entry_state == 0xFA)
                {
                    break;
                }
                addhero_commit_selected_entry();
                break;
            }
            attempts++;
        } while (attempts < 0x14);
        goto done;

    load_step_clear_card:
        result = ADDHERO_LOAD_RESULT_CONTINUE;
        _card_wait(g_addhero_card_slot);
        _card_clear(g_addhero_card_slot * 0x10);
        g_addhero_load_step++;
        goto done;

    load_step_load_card:
        result = ADDHERO_LOAD_RESULT_CONTINUE;
        _card_wait(g_addhero_card_slot);
        _card_load(g_addhero_card_slot * 0x10);
        g_addhero_primary_poll_countdown = 0x10;
        g_addhero_secondary_poll_countdown = 0x10;
        g_addhero_load_step++;
        goto done;

    load_step_idle:
        result = ADDHERO_LOAD_RESULT_ABORT;
        g_addhero_write_in_progress = 0;
        goto done;

    load_step_erase_entry:
        strcat(&card_path, (u8 *)g_addhero_entries + (g_addhero_card_slot * ADDHERO_CARD_DIRECTORY_BYTES) + (g_addhero_selected_row * ADDHERO_DIRECTORY_ENTRY_BYTES));
        attempts = 0;
        do
        {
            if (erase(&card_path) != 0)
            {
                break;
            }
            attempts++;
        } while (attempts < 0x14);
        g_addhero_load_step++;
        goto done;

    load_step_poll_card_load:
        poll_status = addhero_poll_primary_handle_group();
        if (poll_status >= 3)
        {
            goto poll_card_load_ge3;
        }
        if (poll_status > 0)
        {
            goto card_load_error;
        }
        if (poll_status == 0)
        {
            goto advance_after_card_load;
        }
        goto done;
    poll_card_load_ge3:
        if (poll_status == 3)
        {
            goto card_load_retry;
        }
        goto done;
    advance_after_card_load:
        g_addhero_load_step++;
        goto done;
    card_load_error:
        g_addhero_secondary_poll_countdown--;
        if (g_addhero_secondary_poll_countdown != 0)
        {
            goto reissue_card_load;
        }
        result = ADDHERO_LOAD_RESULT_COMPLETE;
        g_addhero_selection_status = 0;
        g_addhero_entry_state = 0xFD;
        goto done;
    card_load_retry:
        g_addhero_primary_poll_countdown--;
        if (g_addhero_primary_poll_countdown == 0)
        {
            goto card_load_timeout;
        }
    reissue_card_load:
        _card_wait(g_addhero_card_slot);
        _card_clear(g_addhero_card_slot * 0x10);
        _card_wait(g_addhero_card_slot);
        _card_load(g_addhero_card_slot * 0x10);
        goto done;
    card_load_timeout:
        result = ADDHERO_LOAD_RESULT_CARD_ERROR;
        g_addhero_entry_state = 0xFC;
        g_addhero_load_step = g_addhero_loadseq_card;
        goto done;

    load_step_wait_secondary:
        do
        {
            poll_status = addhero_poll_secondary_handle_group();
        } while (poll_status == -1);
        g_addhero_load_step++;
        goto done;

    load_step_read_entry:
        g_addhero_io_busy = 1;
        g_addhero_selection_status = 0;
        _card_wait(g_addhero_card_slot);
        g_addhero_file_handle = open(g_addhero_save_file_path, 0x8001);
        if (g_addhero_file_handle == -1)
        {
            goto done;
        }
        addhero_release_primary_handles();
        _card_wait(g_addhero_card_slot);
        if (read(g_addhero_file_handle, &g_addhero_entry_read_buffer,
                           g_addhero_selected_entry_extended != 0 ? 0x280 : 0x80) == -1)
        {
            close(g_addhero_file_handle);
            goto done;
        }
        g_addhero_load_step++;
        goto done;

    load_step_poll_entry_read:
        if (g_addhero_io_busy != 0)
        {
            poll_status = addhero_poll_primary_handle_group();
            if (poll_status == 0)
            {
                g_addhero_io_busy = 0;
                g_addhero_selection_status = 1;
                close(g_addhero_file_handle);
                goto done;
            }
            if (poll_status == -1)
            {
                goto done;
            }
            close(g_addhero_file_handle);
            g_addhero_entry_state = ADDHERO_ENTRY_STATE_IDLE;
            g_addhero_load_step = &g_addhero_loadseq_start;
        }
        else
        {
            g_addhero_load_step++;
        }
        goto done;

    load_step_read_save:
        g_addhero_progress_active = 1;
        g_addhero_progress_start_tick = VSync(-1);
        g_addhero_progress_bar_active = 1;
        _card_wait(g_addhero_card_slot);
        g_addhero_file_handle = open(g_addhero_save_file_path, 0x8001);
        addhero_release_primary_handles();
        _card_wait(g_addhero_card_slot);
        if (read(g_addhero_file_handle, g_addhero_save_blob, 0x4000) == -1)
        {
            close(g_addhero_file_handle);
            g_addhero_retry_count--;
            if (g_addhero_retry_count == 0)
            {
            show_read_error:
                addhero_open_status_dialog(1);
                goto done;
            }
            goto done;
        }
        g_addhero_load_step++;
        goto done;

    load_step_poll_save_read:
        io_status = addhero_poll_primary_handle_group();
        if (io_status == 0)
        {
            g_addhero_progress_active = 0;
            g_addhero_load_step++;
            close(g_addhero_file_handle);
            goto done;
        }
        if (io_status < 0)
        {
            goto done;
        }
        if (io_status >= 4)
        {
            goto done;
        }
        close(g_addhero_file_handle);
        g_addhero_retry_count--;
        if (g_addhero_retry_count == 0)
        {
            g_addhero_progress_bar_active = 0;
            goto show_read_error;
        }
        g_addhero_load_step--;
        goto done;

    load_step_check_card_type:
        attempts = 0;
        do
        {
            if (McxCardType(g_addhero_card_slot * 0x10) == 1)
            {
                break;
            }
            VSync(0);
            attempts++;
        } while (attempts < 0x14);
        if (attempts != 0x14)
        {
            func_80032174(0, &card_status0, &card_status1);
            if (card_status1 == 0)
            {
                g_addhero_load_step++;
                goto done;
            }
        }
        addhero_open_status_dialog(3);
        goto done;

    load_step_init_retries:
        g_addhero_retry_count = 5;
        g_addhero_load_step++;
        goto done;

    load_step_read_before_write:
        g_addhero_progress_active = 1;
        g_addhero_progress_start_tick = VSync(-1);
        g_addhero_progress_bar_active = 1;
        _card_wait(g_addhero_card_slot);
        g_addhero_file_handle = open(g_addhero_save_file_path, 0x8001);
        addhero_release_primary_handles();
        _card_wait(g_addhero_card_slot);
        if (read(g_addhero_file_handle, g_addhero_save_blob, 0x4000) == -1)
        {
            close(g_addhero_file_handle);
            g_addhero_retry_count--;
            if (g_addhero_retry_count == 0)
            {
            show_prewrite_read_error:
                addhero_open_exit_dialog(1);
                goto done;
            }
            goto done;
        }
        g_addhero_load_step++;
        goto done;

    load_step_poll_prewrite_read:
        io_status = addhero_poll_primary_handle_group();
        if (io_status == 0)
        {
            g_addhero_progress_active = 0;
            g_addhero_load_step++;
            close(g_addhero_file_handle);
            goto done;
        }
        if (io_status < 0)
        {
            goto done;
        }
        if (io_status >= 4)
        {
            goto done;
        }
        g_addhero_retry_count--;
        if (g_addhero_retry_count == 0)
        {
            g_addhero_progress_bar_active = 0;
            goto show_prewrite_read_error;
        }
        goto retry_previous_step;

    load_step_write_save:
        if (g_addhero_has_free_entry_space == 0)
        {
            _card_wait(g_addhero_card_slot);
            attempts = 0;
            do
            {
                if (erase(g_addhero_save_file_path) != 0)
                {
                    break;
                }
                attempts++;
            } while (attempts < 0x14);
        }
        strcat(&card_path, g_lom_save_dummy_filename);
        _card_wait(g_addhero_card_slot);
        g_addhero_file_handle = open(&card_path, 0x20200);
        if (g_addhero_file_handle != -1)
        {
            goto write_save_data;
        }
        close(-1);
        attempts = 0;
        do
        {
            if (erase(&card_path) != 0)
            {
                break;
            }
            attempts++;
        } while (attempts < 0x14);
    retry_save_write:
        g_addhero_retry_count--;
        if (g_addhero_retry_count == 0)
        {
        show_write_error:
            addhero_open_exit_dialog(0);
            goto done;
        }
        goto done;

    write_save_data:
        close(g_addhero_file_handle);
        strcpy(g_addhero_target_file_path, &card_path);
        _card_wait(g_addhero_card_slot);
        g_addhero_file_handle = open(g_addhero_target_file_path, 0x8002);
        addhero_release_primary_handles();
        g_addhero_progress_start_tick = VSync(-1);
        g_addhero_progress_bar_active = 1;
        _card_wait(g_addhero_card_slot);
        if (write(g_addhero_file_handle, g_addhero_save_blob, 0x4000) == -1)
        {
            close(g_addhero_file_handle);
            attempts = 0;
            do
            {
                if (erase(g_addhero_target_file_path) != 0)
                {
                    break;
                }
                attempts++;
            } while (attempts < 0x14);
            goto retry_save_write;
        }
        g_addhero_load_step++;
        goto done;

    load_step_poll_save_write:
        io_status = addhero_poll_primary_handle_group();
        if (io_status != 0)
        {
            if (io_status < 0)
            {
                goto done;
            }
            if (io_status >= 4)
            {
                goto done;
            }
            goto retry_save_finalize;
        }
        if (g_addhero_has_free_entry_space != 0)
        {
            _card_wait(g_addhero_card_slot);
            attempts = 0;
            do
            {
                if (erase(g_addhero_save_file_path) != 0)
                {
                    break;
                }
                attempts++;
            } while (attempts < 0x14);
        }
        _card_wait(g_addhero_card_slot);
        attempts = 0;
        do
        {
            if (rename(g_addhero_target_file_path, g_addhero_save_file_path) != 0)
            {
                break;
            }
            attempts++;
        } while (attempts < 0x14);
        g_addhero_write_in_progress = 0;
        g_addhero_load_step++;
        close(g_addhero_file_handle);
        goto done;

    retry_save_finalize:
        g_addhero_retry_count--;
        if (g_addhero_retry_count == 0)
        {
            g_addhero_progress_bar_active = 0;
            goto show_write_error;
        }
        goto retry_previous_step;

retry_previous_step:
    close(g_addhero_file_handle);
    g_addhero_load_step--;

done:
    return result;
}

/**
 * @brief Rewind the active card and restart the load sequence from its first
 *        step.
 * @see decomp.me (100.00%)
 */
void addhero_restart_load_sequence(void)
{
    _card_wait(g_addhero_card_slot);
    addhero_release_primary_handles();
    _card_info(g_addhero_card_slot * 0x10);
    g_addhero_load_step = g_addhero_loadseq_card;
}

/**
 * @brief Poll the primary handle group and, if any handle is still busy, rewind
 *        the active card so it can be retried.
 * @return The busy handle index, or -1 when all primary handles are idle.
 * @see decomp.me (100.00%)
 */
s32 addhero_poll_and_rewind_primary_handles(void)
{
    s32 busy_slot;

    busy_slot = addhero_poll_primary_handle_group();
    if (busy_slot != -1)
    {
        _card_wait(g_addhero_card_slot);
        _card_info(g_addhero_card_slot * 0x10);
    }
    return busy_slot;
}

/**
 * @brief Allocate and register the four primary and four secondary card stream
 *        handles and clear the progress/scan flags.
 * @see decomp.me (100.00%)
 */
void addhero_init_stream_handles(void)
{
    reset_controller_vsync_state();
    EnterCriticalSection();
    g_addhero_primary_handle0 = OpenEvent(0xF4000001, 4, 0x2000, 0);
    g_addhero_primary_handle1 = OpenEvent(0xF4000001, 0x8000, 0x2000, 0);
    g_addhero_primary_handle2 = OpenEvent(0xF4000001, 0x100, 0x2000, 0);
    g_addhero_primary_handle3 = OpenEvent(0xF4000001, 0x2000, 0x2000, 0);
    g_addhero_secondary_handle0 = OpenEvent(0xF0000011, 4, 0x2000, 0);
    g_addhero_secondary_handle1 = OpenEvent(0xF0000011, 0x8000, 0x2000, 0);
    g_addhero_secondary_handle2 = OpenEvent(0xF0000011, 0x100, 0x2000, 0);
    g_addhero_secondary_handle3 = OpenEvent(0xF0000011, 0x2000, 0x2000, 0);
    EnableEvent(g_addhero_primary_handle0);
    EnableEvent(g_addhero_primary_handle1);
    EnableEvent(g_addhero_primary_handle2);
    EnableEvent(g_addhero_primary_handle3);
    EnableEvent(g_addhero_secondary_handle0);
    EnableEvent(g_addhero_secondary_handle1);
    EnableEvent(g_addhero_secondary_handle2);
    EnableEvent(g_addhero_secondary_handle3);
    ExitCriticalSection();
    g_addhero_progress_bar_active = 0;
    g_addhero_entry_scan_active = 0;
}

/**
 * @brief Unregister and free the four primary and four secondary card stream
 *        handles.
 * @see decomp.me (100.00%)
 */
void addhero_shutdown_stream_handles(void)
{
    reset_controller_vsync_state();
    EnterCriticalSection();
    CloseEvent(g_addhero_primary_handle0);
    CloseEvent(g_addhero_primary_handle1);
    CloseEvent(g_addhero_primary_handle2);
    CloseEvent(g_addhero_primary_handle3);
    CloseEvent(g_addhero_secondary_handle0);
    CloseEvent(g_addhero_secondary_handle1);
    CloseEvent(g_addhero_secondary_handle2);
    CloseEvent(g_addhero_secondary_handle3);
    ExitCriticalSection();
}

/**
 * @brief Reset browser state and read the first directory entry of the given
 *        card page, priming the scan.
 * @param page Card page index to begin scanning.
 * @return 1 if a first entry was read, 0 if the page is empty.
 * @see decomp.me (100.00%)
 */
s32 addhero_begin_entry_scan(s32 page)
{
    AddheroEntryHeader buf;

    memcpy(&buf, &g_addhero_entry_header_template, 7);
    g_addhero_selected_row = 0;
    g_addhero_scroll_frames = 0;
    g_addhero_scroll_target_y = 0;
    g_addhero_scroll_y = 0;
    g_addhero_entry_state = 0;
    ((u8 *)&buf)[2] += page;
    if (firstfile(&buf, &g_addhero_entries[page][0]) != 0)
    {
        func_800B0170(&g_addhero_entries[page][g_addhero_entry_state]);
        g_addhero_entry_state += 1;
        return 1;
    }
    return 0;
}

/**
 * @brief Advance one step of the add-hero entry load scan for the given page.
 * @param page Page index whose entry block is being scanned.
 * @return 1 if an entry was consumed this step, 0 otherwise.
 * @see decomp.me (100.00%)
 */
s32 addhero_scan_next_entry(s32 page)
{
    s32 i;
    s32 sum;
    s32 offset;
    s32 selected;
    s32 count;
    s32 cond;

    if (nextfile(&g_addhero_entries[page][g_addhero_entry_state]) != 0)
    {
        func_800B0170(&g_addhero_entries[page][g_addhero_entry_state]);
        g_addhero_entry_state += 1;
        return 1;
    }

    func_800AA02C();
    if ((g_addhero_mode == 0) && (addhero_has_known_entry_type() == 0))
    {
        g_addhero_entry_state = 0xF8;
    }
    else
    {
        i = 0;
        sum = 0;
        g_addhero_has_free_entry_space = 0;
        count = g_addhero_entry_state;
        if (count > 0)
        {
            u8 *entries;
            do { entries = (u8 *)g_addhero_entries; } while (0);
            offset = g_addhero_card_slot * ADDHERO_CARD_DIRECTORY_BYTES;
            do
            {
                sum += ((struct DIRENTRY *)(offset + (s32)entries))->size / 8192;
                i++;
                offset += ADDHERO_DIRECTORY_ENTRY_BYTES;
            } while (i < count);
        }
        cond = sum >= 0xE;
        if (cond != 0)
        {
            selected = addhero_rank_entries(sum, i, count);
            if (addhero_has_known_entry_type() == 0)
            {
                g_addhero_entry_state = 0xFA;
                g_addhero_entry_value_limit = 0;
            }
            else
            {
                if (g_addhero_mode != 0)
                {
                    g_addhero_selected_row = 0;
                }
                g_addhero_selected_row = selected;
                addhero_scroll_to_selection();
            }
        }
        else
        {
            g_addhero_has_free_entry_space = 1;
            selected = addhero_rank_entries(sum, i, count);
            if (addhero_has_known_entry_type() == 0)
            {
                g_addhero_selected_row = 0;
                addhero_scroll_to_selection();
                g_addhero_entry_value_limit = 0;
            }
            else
            {
                if (g_addhero_mode != 0)
                {
                    g_addhero_selected_row = 0;
                }
                g_addhero_selected_row = selected;
                addhero_scroll_to_selection();
            }
        }
    }
    return 0;
}

/**
 * @brief Prepare the currently selected directory entry for loading: set the
 *        selection status, build its file spec, and arm the read step.
 * @see decomp.me (100.00%)
 */
void addhero_commit_selected_entry(void)
{
    AddheroFileHeader local;
    u8 *p;

    if (g_addhero_entry_state == 0)
    {
        g_addhero_selection_status = 3;
        return;
    }
    {
        s32 term1;
        s32 term2;
        term1 = g_addhero_card_slot * ADDHERO_CARD_DIRECTORY_BYTES;
        term2 = (g_addhero_selected_row * ADDHERO_DIRECTORY_ENTRY_BYTES) + (s32)g_addhero_entries;
        if (strncmp(&g_new_save_entry_prefix[0], (void *)(term1 + term2), 8) == 0)
        {
            g_addhero_selection_status = 2;
            return;
        }
    }
    memcpy(&local, &g_addhero_file_template, 6);
    p = (u8 *)&local;
    {
        s32 term1;
        s32 term2;
        term1 = g_addhero_card_slot * ADDHERO_CARD_DIRECTORY_BYTES;
        term2 = (g_addhero_selected_row * ADDHERO_DIRECTORY_ENTRY_BYTES) + (s32)g_addhero_entries;
        strcat(p, (void *)(term1 + term2));
    }
    {
        s32 slot;
        s32 value;
        value = *((u8 *)&local + 2);
        slot = (u8)g_addhero_card_slot;
        g_addhero_selection_status = 0;
        value += slot;
        *((u8 *)&local + 2) = value;
        strcpy(&g_addhero_save_file_path[0], p, slot);
    }
    g_addhero_load_step = &g_addhero_loadseq_file_ready[0];
    {
        s32 term1;
        s32 term2;
        term1 = g_addhero_card_slot * ADDHERO_CARD_DIRECTORY_BYTES;
        term2 = (g_addhero_selected_row * ADDHERO_DIRECTORY_ENTRY_BYTES) + (s32)g_addhero_entries;
        if (strncmp(&g_lom_save_filename_prefix[0], (void *)(term1 + term2), 0xC) == 0)
            g_addhero_selected_entry_extended = 1;
        else
            g_addhero_selected_entry_extended = 0;
    }
    g_addhero_io_busy = 1;
}

/**
 * @brief Release (poll to idle) all four primary card stream handles.
 * @see decomp.me (100.00%)
 */
void addhero_release_primary_handles(void)
{
    TestEvent(g_addhero_primary_handle0);
    TestEvent(g_addhero_primary_handle1);
    TestEvent(g_addhero_primary_handle2);
    TestEvent(g_addhero_primary_handle3);
}

/**
 * @brief Release (poll to idle) all four secondary card stream handles.
 * @see decomp.me (100.00%)
 */
void addhero_release_secondary_handles(void)
{
    TestEvent(g_addhero_secondary_handle0);
    TestEvent(g_addhero_secondary_handle1);
    TestEvent(g_addhero_secondary_handle2);
    TestEvent(g_addhero_secondary_handle3);
}

/**
 * @brief Poll the four primary card stream handles for one that is busy.
 * @return Index (0-3) of the first busy handle, or -1 when all are idle.
 * @see decomp.me (100.00%)
 */
s32 addhero_poll_primary_handle_group(void)
{
    if (TestEvent(g_addhero_primary_handle0) == 1)
    {
        return 0;
    }
    if (TestEvent(g_addhero_primary_handle1) == 1)
    {
        return 1;
    }
    if (TestEvent(g_addhero_primary_handle2) == 1)
    {
        return 2;
    }
    if (TestEvent(g_addhero_primary_handle3) == 1)
    {
        return 3;
    }
    return -1;
}

/**
 * @brief Poll the four secondary card stream handles for one that is busy.
 * @return Index (0-3) of the first busy handle, or -1 when all are idle.
 * @see decomp.me (100.00%)
 */
s32 addhero_poll_secondary_handle_group(void)
{
    if (TestEvent(g_addhero_secondary_handle0) == 1)
    {
        return 0;
    }
    if (TestEvent(g_addhero_secondary_handle1) == 1)
    {
        return 1;
    }
    if (TestEvent(g_addhero_secondary_handle2) == 1)
    {
        return 2;
    }
    if (TestEvent(g_addhero_secondary_handle3) == 1)
    {
        return 3;
    }
    return -1;
}

/**
 * @brief Reorder the active card's directory entries into a stable grouping:
 *        by suffix value within each known name prefix, then a third prefix,
 *        then any remaining entries, writing the result back in place.
 * @see decomp.me (100%)
 */
void addhero_sort_entries_by_type(void)
{
    struct DIRENTRY sorted[ADDHERO_DIRECTORY_ENTRY_COUNT];
    s32 out = 0;
    s32 group = 0;
    s32 i;
    do {
        i = 0;
        if (i < g_addhero_entry_state) {
            do {
                if (g_addhero_entry_suffix_values[i] == group &&
                    strncmp(g_lom_save_filename_prefix, &g_addhero_entries[g_addhero_card_slot][i], 0xC) == 0) {
                    bcopy(&g_addhero_entries[g_addhero_card_slot][i], &sorted[out], sizeof(struct DIRENTRY));
                    out++;
                }
                i++;
            } while (i < g_addhero_entry_state);
        }
        group++;
    } while (group < 8);

    group = 0;
    do {
        i = 0;
        if (i < g_addhero_entry_state) {
            do {
                if (g_addhero_entry_suffix_values[i] == group &&
                    strncmp(g_lom_alt_save_filename_prefix, &g_addhero_entries[g_addhero_card_slot][i], 0xC) == 0) {
                    bcopy(&g_addhero_entries[g_addhero_card_slot][i], &sorted[out], sizeof(struct DIRENTRY));
                    out++;
                }
                i++;
            } while (i < g_addhero_entry_state);
        }
        group++;
    } while (group < 8);

    i = 0;
    if (g_addhero_entry_state > 0) {
        do {
            if (strncmp(g_new_save_entry_prefix, &g_addhero_entries[g_addhero_card_slot][i], 8) == 0) {
                bcopy(&g_addhero_entries[g_addhero_card_slot][i], &sorted[out], sizeof(struct DIRENTRY));
                out++;
            }
            i++;
        } while (i < g_addhero_entry_state);
    }

    if (*(volatile s32 *)&g_addhero_entry_state > 0) {
        i = 0;
        do {
            if (strncmp(g_lom_save_filename_prefix, &g_addhero_entries[g_addhero_card_slot][i], 0xC) != 0 &&
                strncmp(g_lom_alt_save_filename_prefix, &g_addhero_entries[g_addhero_card_slot][i], 0xC) != 0 &&
                strncmp(g_new_save_entry_prefix, &g_addhero_entries[g_addhero_card_slot][i], 8) != 0) {
                bcopy(&g_addhero_entries[g_addhero_card_slot][i], &sorted[out], sizeof(struct DIRENTRY));
                out++;
            }
            i++;
        } while (i < g_addhero_entry_state);
    }

    i = 0;
    if (g_addhero_entry_state > 0) {
        do {
            bcopy(&sorted[i], &g_addhero_entries[g_addhero_card_slot][i], sizeof(struct DIRENTRY));
            i++;
        } while (i < g_addhero_entry_state);
    }
}

/**
 * @brief Render a signed decimal value as cached-glyph text, suppressing
 *        leading zeros and prefixing a minus glyph when negative.
 * @param prim      Current primitive pointer/index.
 * @param ot        Ordering table the glyphs are linked into.
 * @param value     Signed value to render.
 * @param x         X position (interpreted per @p alignment).
 * @param y         Y baseline.
 * @param palette   Glyph palette index.
 * @param alignment Text alignment mode passed to addhero_draw_cached_text.
 * @return The updated primitive pointer.
 * @see decomp.me (100%)
 */
s32 addhero_draw_signed_decimal(s32 prim, s32 *ot, s32 value, s32 x, s32 y, s32 palette, s32 alignment)
{
    u16 buf[7];
    s32 first_digit;
    s32 magnitude;
    s32 negative;

    magnitude = value;
    if (magnitude < 0)
    {
        magnitude = -magnitude;
        negative = 1;
    }
    else
    {
        negative = 0;
    }
    buf[1] = g_addhero_decimal_glyphs[magnitude / 10000];
    buf[2] = g_addhero_decimal_glyphs[(magnitude % 10000) / 1000];
    buf[3] = g_addhero_decimal_glyphs[(magnitude % 1000) / 100];
    buf[4] = g_addhero_decimal_glyphs[(magnitude % 100) / 10];
    buf[5] = g_addhero_decimal_glyphs[magnitude % 10];

    first_digit = 1;
    buf[6] = 0;

    while (first_digit < 5 && buf[first_digit] == 0x4F82)
    {
        first_digit++;
    }

    if (negative != 0)
    {
        first_digit--;
        buf[first_digit] = 0x5B81;
    }
    prim = addhero_draw_cached_text(prim, ot, (u8 *)&buf[first_digit], x, y, palette, alignment);
    return prim;
}

/**
 * @brief Render a byte as two hex-digit glyphs via the cached-text renderer.
 * @param prim       Current primitive pointer/index.
 * @param ot         Ordering table the glyphs are linked into.
 * @param byte_value Byte value to render.
 * @param x          X position.
 * @param y          Y baseline.
 * @param alignment  Text alignment mode.
 * @see decomp.me (100%)
 */
void addhero_draw_hex_byte(s32 prim, s32 ot, s32 byte_value, s32 x, s32 y, s32 alignment)
{
    u16 pair[3];
    s32 row;
    s32 adjusted;
    s32 off;
    u16 *base;

    adjusted = byte_value;
    if (byte_value < 0)
    {
        adjusted = byte_value + 15;
    }
    row = adjusted >> 4;
    off = row * 2;
    base = g_addhero_hex_glyphs;
    pair[0] = *(u16 *)((u8 *)base + off);
    off = (byte_value - row * 16) * 2;
    pair[1] = *(u16 *)((u8 *)base + off);
    pair[2] = 0;
    addhero_draw_cached_text(prim, ot, pair, x, y, 0, alignment);
}

/**
 * @brief Render a multibyte string through the glyph cache: measure it, apply
 *        left/center/right alignment, then emit one cached glyph per character
 *        and terminate the primitive list.
 * @param prim      Current primitive pointer/index.
 * @param ot        Ordering table the glyphs are linked into.
 * @param text      Null-terminated multibyte string to render.
 * @param x         X position (interpreted per @p alignment).
 * @param y         Y baseline.
 * @param palette   Glyph palette index.
 * @param alignment 0 left, 1 right (16px/char), 2 right (8px/char).
 * @return The updated primitive pointer past the terminator.
 * @see decomp.me (100%)
 */
s32 addhero_draw_cached_text(s32 prim, s32 *ot, u8 *text, s32 x, s32 y, s32 palette, s32 alignment)
{
    u8 *cursor;
    s32 count;
    u16 code;
    u8 *scan;

    cursor = text;
    count = 0;
    if (*cursor >= 0x20)
    {
        scan = cursor;
        do
        {
            code = *scan;
            if (code >= 0x80)
            {
                scan++;
            }
            scan++;
            count++;
        } while (*scan >= 0x20);
    }

    switch (alignment)
    {
    case 1:
        x -= count * 0x10;
        break;
    case 2:
        x -= count * 8;
        break;
    case 0:
    default:
        break;
    }
    g_addhero_text_line_start_x = x;
    g_addhero_glyph_cursor_x = x;
    g_addhero_glyph_cursor_y = y;

    while (1)
    {
        u32 lead = *cursor;

        if ((u8)lead == 0x20)
        {
            cursor++;
            g_addhero_glyph_cursor_x += 0x10;
            continue;
        }
        if ((u8)lead >= 0x80)
        {
            code = cursor[0];
            code = (code << 8) | cursor[1];
            cursor += 2;
        }
        else
        {
            if ((u8)lead < 0x20)
            {
                break;
            }
            if ((u32)(lead - 0x30) < 0x50)
            {
                code = *cursor - 0x7DE1;
                cursor++;
            }
            else
            {
                code = *cursor - 0x7AE1;
                cursor++;
            }
        }
        prim = addhero_render_cached_glyph(prim, ot, code, palette);
    }

    setDrawTPage(prim, 0, 0, 5);
    addPrim(ot, prim);
    return prim + 8;
}

/**
 * @brief Emit one glyph sprite, rasterizing and uploading the glyph to the VRAM
 *        cache first when it is not already cached.
 * @param prim           Current primitive pointer/index.
 * @param ot             Ordering table the sprite is linked into.
 * @param character_code Glyph code to render.
 * @param palette        Glyph palette index used when rasterizing.
 * @return The updated primitive pointer, unchanged when the glyph is missing or
 *         the cache is full.
 * @see decomp.me (100%)
 */
s32 addhero_render_cached_glyph(s32 prim, s32 *ot, s32 character_code, s32 palette)
{
    AddheroGlyphCacheEntry *entry;
    u8 *font_data;
    s32 font_address;
    u32 requested_code;
    s32 slot;
    s32 high_pixel_set;
    s32 code;
    RECT rect;

    u8 *raster;
    s32 color_index;
    s32 high_nibble_color;
    s32 row;
    s32 source_byte;

    u16 mask;
    volatile u8 *raster_byte;
    u8 packed_pixels;

    code = character_code;
    slot = 0;
    requested_code = code & 0xFFFF;
    entry = g_addhero_glyph_cache;

    while (slot < GLYPH_CACHE_SLOTS)
    {
        if (requested_code == entry->data.code)
        {
            return addhero_emit_glyph_sprite((AddheroGlyphSprite *)prim, ot, slot, palette);
        }
        slot++;
        entry++;
    }

    font_address = Krom2RawAdd(code & 0xFFFF);
    font_data = (u8 *)font_address;
    if (font_address == -1)
    {
        return prim;
    }

    raster = g_addhero_glyph_raster_cursor;
    row = 0;
    color_index = (palette + 1) * 2;
    high_nibble_color = color_index * 16;
    for (; row < 15; row++)
    {
        for (source_byte = 0; source_byte < 2; source_byte++)
        {
            mask = 0x80;

            for (slot = 0; slot < 4; slot++)
            {
                *raster = ((*font_data) & mask) ? color_index : 0;

                mask >>= 1;
                high_pixel_set = (*font_data) & mask;

                raster_byte = raster;
                packed_pixels = *raster_byte;
                if (high_pixel_set)
                {
                    packed_pixels += high_nibble_color;
                }

                *raster_byte = packed_pixels;

                mask >>= 1;
                raster++;
            }

            font_data++;
        }
    }

    slot = 0;
    while ((slot < GLYPH_CACHE_SLOTS) && (g_addhero_glyph_cache[slot].raw != 0))
    {
        slot++;
    }

    if (slot == GLYPH_CACHE_SLOTS)
    {
        return prim;
    }
    g_addhero_glyph_cache[slot].raw = code & 0xFFFF;
    prim = addhero_emit_glyph_sprite((AddheroGlyphSprite *)prim, ot, slot, palette);

    g_addhero_glyph_upload_x = (slot % GLYPH_CACHE_COLUMNS) * 4;
    g_addhero_glyph_upload_y = slot & GLYPH_CACHE_ROW_MASK;

    rect.w = 4;
    rect.h = 15;
    rect.x = g_addhero_glyph_upload_x + 0x140;
    rect.y = g_addhero_glyph_upload_y;

    func_80019A34(&rect, g_addhero_glyph_raster_cursor);
    func_80019788(0);

    g_addhero_glyph_raster_cursor += GLYPH_RASTER_BYTES;
    return prim;
}

/**
 * @brief Write a 16x16 sprite for a cached glyph at the current text cursor,
 *        mark the slot used, and advance the cursor (wrapping to the next line).
 * @param sprite     Destination sprite primitive.
 * @param ot         Ordering table the sprite is linked into.
 * @param cache_slot Glyph cache slot whose VRAM tile to sample.
 * @param palette    Unused here; the CLUT is fixed.
 * @return The primitive pointer advanced past the emitted sprite.
 * @see decomp.me (100%)
 */
s32 addhero_emit_glyph_sprite(AddheroGlyphSprite *sprite, s32 *ot, s32 cache_slot, s32 palette)
{
    u32 ot_tag_high_byte;
    s32 normalized_slot;
    u32 packet_address;
    s32 old_x;
    s32 new_x;
    s32 fits_line;

    g_addhero_glyph_cache[cache_slot].raw |= 0x10000;

    setSprt16(sprite);
    sprite->packet.g0 = 0x80;
    sprite->packet.b0 = 0x80;
    sprite->packet.r0 = 0x80;
    normalized_slot = cache_slot;
    setXY0(&sprite->packet, g_addhero_glyph_cursor_x, g_addhero_glyph_cursor_y);

    if (cache_slot < 0)
    {
        normalized_slot = cache_slot + 15;
    }

    setUV0(&sprite->packet, (cache_slot - ((normalized_slot >> 4) * 16)) * 16,
           cache_slot & GLYPH_CACHE_ROW_MASK);
    sprite->packet.clut = 0x7FD3;
    sprite->packet.tag = (sprite->packet.tag & GPU_TAG_HIGH_MASK) | (*ot & GPU_ADDR_MASK);

    packet_address = ((u32)sprite) & GPU_ADDR_MASK;
    ot_tag_high_byte = *ot & GPU_TAG_HIGH_MASK;

    sprite++;
    old_x = g_addhero_glyph_cursor_x;
    new_x = old_x + 16;
    fits_line = (old_x + 32) < 0x280;
    g_addhero_glyph_cursor_x = new_x;

    *ot = ot_tag_high_byte | packet_address;

    if (!fits_line)
    {
        g_addhero_glyph_cursor_x = g_addhero_text_line_start_x;
        g_addhero_glyph_cursor_y += 16;
    }

    return (s32)sprite;
}

/**
 * @brief Start a new glyph cache frame: rewind the raster cursor and clear each
 *        cache entry's per-frame "used" flag (the high half-word).
 * @see decomp.me (100.00%)
 */
void addhero_begin_glyph_cache_frame(void)
{
    s32 i;
    s32 *p;

    g_addhero_glyph_raster_cursor = g_addhero_glyph_raster_buffer;
    for (i = 0, p = (s32 *)g_addhero_glyph_cache; i < 0x100; i++, p++)
    {
        *p = (u16)*p;
    }
}

/**
 * @brief Evict cache entries not touched this frame by zeroing any slot whose
 *        "used" flag (bit 0x10000) is clear.
 * @see decomp.me (100.00%)
 */
void addhero_evict_unused_glyphs(void)
{
    s32 i;
    s32 *p;
    s32 flag;

    i = 0;
    flag = 0x10000;
    p = (s32 *)g_addhero_glyph_cache;
    for (; i < 0x100; i++, p++)
    {
        if (!(*p & flag))
        {
            *p = 0;
        }
    }
}

/**
 * @brief Fully reset the glyph cache: zero all 0x100 cache entries and clear the
 *        entire 0x8000-byte glyph raster buffer.
 * @see decomp.me (100.00%)
 */
void addhero_reset_glyph_cache(void)
{
    s32 i;
    s32 *p;
    u8 *q;

    i = 0xFF;
    p = (s32 *)g_addhero_glyph_cache;
    p += 0xFF;
    for (; i >= 0; i--, p--)
    {
        *p = 0;
    }

    i = 0;
    q = g_addhero_glyph_raster_buffer;
    for (; i <= 0x7FFF; i++)
    {
        *(u8 *)(i + (s32)q) = 0;
    }
}

/**
 * @brief Translate a source string into internal glyph codes via the single-
 *        and double-byte character tables, writing two output bytes per input
 *        character and null-terminating the result.
 * @param out Destination glyph-code buffer.
 * @param in  Null-terminated source string.
 * @see decomp.me (100%)
 */
void addhero_expand_text_glyph_codes(u8 *out, u8 *in)
{
    u32 c;
    s32 index;
    s16 lead;

    for (;;)
    {
        c = *in;
        if ((u8)c == 0)
        {
            goto done;
        }
        if ((u32)(c - 0x19) < 7)
        {
            u32 b1;
            s32 off;
            u8 *pa;
            u8 *pb;

            b1 = in[1];
            off = b1 >> 4;
            b1 &= 0xF;
            pa = g_addhero_double_byte_char_table + b1 * 2;
            pa += off * 33;
            lead = *in;
            pa += lead * 528;
            *out = *pa;
            out++;
            b1 = in[1];
            off = b1 >> 4;
            b1 &= 0xF;
            pb = g_addhero_double_byte_char_table + 1 + b1 * 2;
            pb += off * 33;
            lead = *in;
            pb += lead * 528;
            *out = *pb;
            out++;
            in += 2;
        }
        else if ((u8)c >= 0x21)
        {
            lead = *in;
            index = lead - 0x20;
            *out = g_addhero_single_byte_char_table[(index / 16) * 33 + (index & 0xF) * 2];
            out++;
            lead = *in;
            index = lead - 0x20;
            *out = g_addhero_single_byte_char_table[(index / 16) * 33 + (index & 0xF) * 2 + 1];
            out++;
            in += 1;
        }
        else
        {
            *out = g_addhero_single_byte_char_table[0];
            out++;
            *out = g_addhero_single_byte_char_table[1];
            out++;
            in += 1;
        }
    }
done:
    *out = 0;
}
