#ifndef ADDHERO_INTERNAL_H
#define ADDHERO_INTERNAL_H

#include "common.h"
#include "pad.h"
#include "vector.h"
#include "display.h"
#include "gpu_packet.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"
#include "sdk/kernel.h"

/* Declarations shared by ADDHERO implementation files. */

#define ADDHERO_DIRECTORY_ENTRY_COUNT 20
#define ADDHERO_DIRECTORY_ENTRY_BYTES sizeof(struct DIRENTRY)
#define ADDHERO_CARD_DIRECTORY_BYTES (ADDHERO_DIRECTORY_ENTRY_COUNT * ADDHERO_DIRECTORY_ENTRY_BYTES)
#define ADDHERO_CARD_BLOCK_BYTES 8192
#define ADDHERO_USED_BLOCK_LIMIT 14
#define ADDHERO_SAVE_FILENAME_PREFIX_LENGTH 12
#define ADDHERO_NEW_SAVE_FILENAME_PREFIX_LENGTH 8
#define ADDHERO_LOAD_RESULT_ABORT 2
#define ADDHERO_LOAD_RESULT_CONTINUE 3
#define ADDHERO_LOAD_RESULT_COMPLETE 4
#define ADDHERO_LOAD_RESULT_CARD_ERROR 5

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
 *   0xF7  drawn when the selected entry is not found while advancing the list
 *   0xF8/0xF9  mode-dependent card read/scan failure notice
 *   0xFB/0xFC  status/result notices
 *   0xFE  no-op (draws nothing)
 */
#define ADDHERO_ENTRY_STATE_CARD_FULL 0xFA     /* no known save entry and insufficient free blocks */
#define ADDHERO_ENTRY_STATE_CARD_IO_ERROR 0xFD /* card event error or exhausted timeout retries */
#define ADDHERO_ENTRY_STATE_IDLE 0xFF          /* browsing / reset; no operation active */

/** @brief Result returned when consuming a software or hardware card event. */
typedef enum
{
    ADDHERO_CARD_EVENT_NONE = -1,
    ADDHERO_CARD_EVENT_COMPLETE = 0,
    ADDHERO_CARD_EVENT_ERROR = 1,
    ADDHERO_CARD_EVENT_TIMEOUT = 2,
    ADDHERO_CARD_EVENT_NEW_CARD = 3,
    ADDHERO_CARD_EVENT_COUNT = 4
} AddheroCardEvent;

/** @brief Memory-card device prefix, such as "bu00", stored with word alignment. */
typedef union
{
    u32 word;
    struct
    {
        u8 name[2];
        u8 slot;
        u8 port;
    } characters;
} AddheroCardDevice;

/** @brief Eight-byte card-path template, including its suffix and terminator. */
typedef struct
{
    AddheroCardDevice device;
    u8 suffix[4];
} AddheroCardPathTemplate;

/** @brief Card path used to remove a placeholder save file. */
typedef struct
{
    AddheroCardDevice device;
    u8 suffix[28];
} AddheroProbeFilePath;

extern struct DIRENTRY g_addhero_entries[][ADDHERO_DIRECTORY_ENTRY_COUNT];
extern AddheroCardPathTemplate g_addhero_file_template;
extern u8* g_addhero_load_step;
extern s32 g_addhero_scroll_y;
extern s32 g_addhero_progress_active;
extern s32 g_addhero_scroll_target_y;
extern s32 g_addhero_mode;
extern s32 g_addhero_entry_state;
extern s32 g_addhero_card_slot;
extern s32 g_addhero_selected_row;
extern s32 g_addhero_selection_status;
extern s32 g_addhero_scroll_frames;
extern s32 g_addhero_io_busy;
extern s32 g_addhero_progress_bar_active;
extern s32 g_addhero_progress_start_tick;
extern s32 g_addhero_entry_scan_active;
extern s32 g_addhero_write_in_progress;
extern s32 g_addhero_rank_count;
extern s32 g_addhero_entry_suffix_values[];
extern s32 g_addhero_entry_ranks[];
extern s32 g_addhero_selected_entry_extended;
extern s32 g_addhero_entry_value_limit;
extern s32 g_addhero_has_free_entry_space;
extern u8 g_addhero_loadseq_start;
extern u8 g_addhero_loadseq_card[];
extern u8 g_addhero_save_blob[];
extern u8 g_addhero_entry_read_buffer;
extern u8 g_addhero_save_file_path[];
extern char g_lom_save_filename_prefix[];
extern char g_lom_alt_save_filename_prefix[];
extern char g_new_save_entry_prefix[];
extern char g_lom_save_dummy_filename[];
extern char g_lom_alt_save_dummy_filename[];

void addhero_scroll_to_selection(void);
void addhero_open_status_dialog(s32 message_id);
void addhero_open_exit_dialog(s32 message_id);
s32 addhero_rank_entries(void);
s32 addhero_has_known_entry_type(void);
s32 addhero_begin_entry_scan(s32 page);
s32 addhero_scan_next_entry(s32 page);
void addhero_clear_software_card_events(void);
void addhero_clear_hardware_card_events(void);
s32 addhero_poll_software_card_events(void);
s32 addhero_poll_hardware_card_events(void);
void addhero_sort_entries_by_type(void);
s32 strncmp(void* a, void* b, s32 n);
void field_reset_input_repeat(void);
s32 VSync(s32 arg0);
void bcopy(void* dst, void* src, s32 len);
void addhero_shutdown_card_events(void);
void addhero_begin_glyph_cache_frame(void);
void addhero_evict_unused_glyphs(void);
void addhero_reset_glyph_cache(void);
void addhero_init_card_events(void);
void addhero_restart_load_sequence(void);
s32 addhero_poll_and_retry_card_info(void);
void addhero_commit_selected_entry(void);
void* addhero_draw_cached_text(void* prim, u_long* ot, u8* text, s32 x, s32 y, s32 palette, s32 alignment);
s32 addhero_advance_load_sequence(void);
s32 strcat(void* a, void* b);
s32 read(s32 a, void* b, s32 c);
s32 write(s32 a, void* b, s32 c);
s32 close(s32 a);
s32 erase(void* a);
s32 strcpy(void* a, void* b);
s32 _card_info(s32 a);
s32 _card_wait(s32 a);


/* Moved from addhero.c when it was split into addhero.c and addhero_widgets.c. */
/* UI states and memory-card limits. */
#define ADDHERO_ELEMENT_COUNT 8
#define ADDHERO_ELEMENT_STATE_MASK 7
#define ADDHERO_ELEMENT_STATE_INACTIVE 0
#define ADDHERO_ELEMENT_STATE_OPENING 1
#define ADDHERO_ELEMENT_STATE_ACTIVE 2
#define ADDHERO_ELEMENT_STATE_CLOSING 3
#define ADDHERO_ELEMENT_STATE_FINISHING 4
#define ADDHERO_ENTRY_ROW_HEIGHT 14
#define ADDHERO_ENTRY_COUNT_LIMIT 0x10
#define ADDHERO_NO_ICON 0x7F

/* Return codes produced by addhero_advance_load_sequence. */
#define ADDHERO_LOAD_RESULT_NONE 0
#define ADDHERO_ENTRY_STATE_LOAD_PROGRESS 0xF6  /* importing a matched entry; progress bar */
#define ADDHERO_ENTRY_STATE_SAVE_PROGRESS 0xF5  /* write in progress; progress bar */
#define ADDHERO_ENTRY_STATE_SAVE_CONFIRM 0xF4   /* confirm dialog; accepting writes the save */
#define ADDHERO_ENTRY_STATE_CONFIRM_PROMPT 0xF3 /* companion choice prompt for save/exit flow */
#define ADDHERO_CONFIRM_BUTTON_MASK (PAD_BTN_CROSS | PAD_BTN_L3)
#define ADDHERO_CARD_SWITCH_BUTTON_MASK (PAD_BTN_SELECT | PAD_BTN_RIGHT | PAD_BTN_LEFT)
#define ADDHERO_INPUT_INJECTION_ENABLED 0x80
#define ADDHERO_SAVE_MAGIC 0x414E41
#define ADDHERO_SAVE_CHECKSUM_BYTES 0x33E0
#define ADDHERO_SAVE_CHECKSUM_BIAS 0x0414E410

/** @brief Bit position of the width's low byte inside AddheroElement.attr.word. */
#define ADDHERO_ELEMENT_WIDTH_SHIFT 24

/** @brief Low eight bits of an AddheroElement's window width. */
#define ADDHERO_ELEMENT_WIDTH_LOW(element) ((element)->attr.word >> ADDHERO_ELEMENT_WIDTH_SHIFT)

/**
 * @brief Full nine-bit window width of an AddheroElement.
 * @param element Element whose width is read.
 * @param width_low The width's low byte, as read by ADDHERO_ELEMENT_WIDTH_LOW.
 */
#define ADDHERO_ELEMENT_WIDTH(element, width_low) ((s32)(((element)->size.bits.width_high << 8) | (width_low)))

/** @brief Store the low eight bits of an AddheroElement's window width. */
#define ADDHERO_SET_ELEMENT_WIDTH_LOW(element, width) \
    ((element)->attr.word = ((element)->attr.word & ((1 << ADDHERO_ELEMENT_WIDTH_SHIFT) - 1)) | ((u32)(width) << ADDHERO_ELEMENT_WIDTH_SHIFT))

/**
 * @brief Address of ADDHERO text @p index, reached through its own u16 offset-table entry @p entry.
 * @note The table start is derived back from the entry symbol, like FIELD_UI_TEXT_AT.
 */
#define ADDHERO_TEXT_AT(entry, index) ((u8*)&(entry) - (index) * 2 + (entry))

/** @brief Start of the ADDHERO text offset table, derived from entry @p entry at @p index. */
#define ADDHERO_TEXT_TABLE(entry, index) (&(entry) - (index))

/** @brief Address of ADDHERO text @p index in the u16 offset table starting at @p table. */
#define ADDHERO_TEXT(table, index) ((u8*)(table) + (table)[index])

/**
 * @brief Address of FIELD UI string @p index, given its two-byte offset entry @p entry.
 * @note The table start is derived back from the entry symbol, as in FIELD's own lookups.
 */
#define FIELD_UI_TEXT_AT(entry, index) ((entry) - (index) * 2 + (entry)[0] + ((entry)[1] << 8))

/**
 * @brief Address of FIELD UI string @p index, given the start of the offset table in @p table.
 * @note Summed as integers, offset bytes first, like FIELD's own string lookups.
 */
#define FIELD_UI_TEXT(table, index) ((u8*)((table)[(index) * 2] + (((table)[(index) * 2 + 1] << 8) + (s32)(table))))

/** @brief Draw an element at its current animation offset and return the packet cursor. */
typedef void* (*AddheroElementDrawFunc)(u_long* ot, void* prim, s32 x_offset, s32 y_offset);

/**
 * @brief Animated panel or list element used by the ADDHERO interface.
 *
 * The nine-bit window width straddles the two state words: its low eight bits
 * are the top byte of attr and its high bit is size.bits.width_high. No
 * bitfield can span that boundary, so the low byte is read and written through
 * attr.word (see ADDHERO_ELEMENT_WIDTH and ADDHERO_SET_ELEMENT_WIDTH_LOW).
 */
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
    } attr;
    union
    {
        u32 word;
        struct
        {
            u32 width_high : 1;
            u32 height : 8;
            u32 scrollable : 1;
        } bits;
    } size;
    AddheroElementDrawFunc draw_handler;
} AddheroElement;

/** @brief Saved character metadata displayed in the card browser. */
typedef struct AddheroRecord
{
    u8 name[23];
    u8 marker_17;
    u32 unknown_18 : 25;
    u32 first_icon : 7;
    u8 _pad1c[3];
    u8 icon_palette;
    u32 entry_label : 18;
    u32 second_icon : 7;
    u32 third_icon : 7;
    u8 _pad24[12];
    s32 play_time_frames;
    u8 _pad34[0xCF - 0x34];
    u8 owner_id;
    u8 _padd0[4];
    u16 hero_id;
    u16 reserved_d6;
    s32 identity;
} AddheroRecord;

/** @brief Alternate title text in a loaded card-entry record. */
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
    u8 _pad1c[0x234];
} AddheroSaveContextBlock;

/** @brief Portion of the ADDHERO save image whose layout is used by this overlay. */
typedef struct
{
    u8 _pad0000[0x770];
    AddheroSaveContextBlock context;
    u8 _pad09c0[0x2A20];
    s32 checksum;
    s32 magic;
} AddheroSaveBlob;

/**
 * @brief Frame context the host passes to ADDHERO each frame: its first word is
 *        the ordering-table entry, followed later by the display-buffer index
 *        and the primitive cursor.
 */
typedef struct
{
    u_long ot;
    u8 _pad0004[0x40AE];
    s16 display_buffer_index;
    u8 _pad40b4[4];
    void* prim_cursor;
} AddheroDrawState;

/** @brief Three double-byte overflow glyphs and their string terminator. */
typedef struct
{
    s8 data[7];
} AddheroOverflowGlyphString;

extern AddheroOverflowGlyphString g_addhero_decimal_overflow_glyphs;
extern AddheroElement g_addhero_element_pool[ADDHERO_ELEMENT_COUNT];
extern AddheroElement g_addhero_element1;
extern AddheroRecord g_addhero_entry_metadata;

/* Shared controller/game context (main.h PadContext); addressed here as a byte
   buffer for the save-blob copies and metadata reads. */
extern u8* g_pad_ctx;
extern s32 g_save_slot_index;
extern s32 D_80122718;
extern s32 g_pad_input;
extern s32 g_menu_element_counter;
extern u8 g_addhero_loadseq_done[];
extern s32 g_addhero_icon_phase;
extern u8* g_addhero_pad_work_ptr;
extern s32 g_addhero_result;
extern s32 g_addhero_work_ram_base;
extern s32 g_addhero_exit_requested;
extern s32 g_addhero_choice_toggle;
extern s32 g_addhero_load_flow_active;
extern s32 g_addhero_icon_palette;
extern s32 g_addhero_frame_parity;
extern s32 g_addhero_dialog_state;
extern s32 g_addhero_entry_identity;
extern s32 g_addhero_icon_image_table[];
extern s32 g_addhero_entry_fields[][ADDHERO_DIRECTORY_ENTRY_COUNT];
extern u8 g_addhero_loadseq_abort[];
extern u8 g_addhero_loadseq_load_begin[];
extern u8 g_addhero_loadseq_load_progress[];
extern u8 g_addhero_loadseq_save_begin[];
extern u8 g_addhero_icon_context[];
extern u8 g_addhero_entry_record;
extern u8 g_addhero_entry_owner_id;
extern u8 g_text_time_separator_offset_bytes[2];
extern u8 g_text_choice_glyph_offsets[];
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

/* Overlay function declarations. */
void addhero_init(s32 work_base, s32 mode);
s32 addhero_state_step(AddheroDrawState* draw_state);
void addhero_build_ui_elements(void);
void addhero_update_state(AddheroDrawState* draw_state);
s32 addhero_update_load_sequence(void);
s32 addhero_handle_input(void);
void addhero_reset_state(void);
void addhero_close_all_elements(void);
void addhero_update_elements(AddheroDrawState* draw_state);
void* addhero_draw_entry_list(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* addhero_draw_mode_glyph(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* addhero_draw_card_slot0_label(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* addhero_draw_card_slot1_label(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* addhero_draw_selected_entry_details(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
u8* addhero_skip_hex_digits(u8* text);
void addhero_terminate_multibyte_text(void* buffer);
void addhero_clear_elements(void);
AddheroElement* addhero_alloc_element(void);
void addhero_update_and_draw_elements(AddheroDrawState* draw_state);
void addhero_deactivate_primary_element(void);
void addhero_text_append(u8* dst, u8* src);
s32 addhero_text_byte_length(u8* str);
void addhero_text_copy(u8* dst, u8* src);
void* addhero_draw_load_prompt(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* addhero_draw_load_progress(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* addhero_draw_progress_bar(POLY_G4* quad, u_long* ot);
void* addhero_draw_status_dialog(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* addhero_draw_exit_dialog(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* addhero_draw_transfer_status(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* addhero_draw_icon_highlight(POLY_FT4* quad, u_long* ot, s32 x, s32 y, s32 width, s32 icon, s32 index, s32 row);
void addhero_enable_choice_toggle(void);
void* addhero_draw_choice_prompt(void* prim, u_long* ot, s32 x, s32 y);
s32 addhero_validate_save_blob(u8* base);
s32 addhero_compute_save_checksum(u8* data);
s8* addhero_format_decimal(s8* out, s32 value);
void addhero_format_hex(s8* out, s32 value, s32 max_chars);
void addhero_hex_nibble_to_ascii(s8* out, s32 value);
u32 addhero_parse_hex(u8* s, s32 len);
s32 addhero_parse_hex_suffix_byte(u8* text);
s32 addhero_entry_blocks_reach_limit(void);
void addhero_erase_placeholder_files(void);

/* Game and SDK function declarations. */
void* func_800A88A0(void* prim, u_long* ot, u8* text, s32 color, s32 x, s32 y, s32 mode);
void* func_800A8A78(u_long* ot, void* prim, s32 value, s32 color, DVECTOR* pos, s32 mode);
void play_menu_sfx(s32 sfx_id, s32 volume);
void field_restore_fade_target(void);
void field_set_default_fade_target(void);
void field_restore_fade_target_with_duration(s32 arg0);

void func_800A55E4(void* buf, s32 arg1);
void func_800A5638(void* buf, s32 arg1);
void* func_800AD850(void* prim, u_long* ot, s32 x, s32 y, s32 width, s32 height, s32 display_buffer_index, s32 is_popup);
void* func_800AE76C(void* prim, u_long* ot, s32 x, s32 y, s32 direction);


void addhero_reset_entry_ranks(void);

/* Functions defined in addhero.c that other translation units use. */
s8* addhero_format_decimal();
void addhero_format_hex();
void addhero_hex_nibble_to_ascii();
u32 addhero_parse_hex();
s32 addhero_parse_hex_suffix_byte();
s32 addhero_parse_entry_fields(void);
void addhero_reset_entry_ranks();
s32 addhero_entry_blocks_reach_limit();
void addhero_erase_placeholder_files();

#endif
