#ifndef CARDA_INTERNAL_H
#define CARDA_INTERNAL_H

#include "field_text.h"
#include "common.h"
#include "vector.h"
#include "gpu_packet.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"

/**
 * @brief Draw callback of a CARDA UI element: emits the element's content at
 *        the given transition offsets and returns the advanced primitive cursor.
 */
typedef void *(*CardaElementDrawFunc)(u_long *ot, void *prim, s32 x_offset, s32 y_offset);

/**
 * @brief One animated CARDA UI element (a framed window plus its content).
 *
 * Eight of these form the element pool starting at g_carda_element_pool.  The nine-bit
 * window width straddles the two state words: its low eight bits are the top
 * byte of attr and its high bit is size.f.width_high.  No bitfield can span
 * that boundary, so the low byte is always read and written through attr.word
 * (see CARDA_ELEMENT_WIDTH and CARDA_SET_ELEMENT_WIDTH_LOW).
 */
typedef struct CardaElement
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
            u32 unk9 : 23;
        } f;
    } size;
    CardaElementDrawFunc draw;
} CardaElement;

/** @brief CardaElement.attr.f.state values. */
#define CARDA_ELEMENT_FREE 0
#define CARDA_ELEMENT_OPENING 1
#define CARDA_ELEMENT_OPEN 2
#define CARDA_ELEMENT_CLOSING 3
#define CARDA_ELEMENT_CLOSED 4

/** @brief Number of elements in the CARDA UI element pool. */
#define CARDA_ELEMENT_COUNT 8

/** @brief Number of phase steps an opening window takes to reach full size. */
#define CARDA_ELEMENT_PHASE_STEPS 8

/** @brief Bit position of the width's low byte inside CardaElement.attr.word. */
#define CARDA_ELEMENT_WIDTH_SHIFT 24

/** @brief Low eight bits of a CardaElement's window width. */
#define CARDA_ELEMENT_WIDTH_LOW(element) ((element)->attr.word >> CARDA_ELEMENT_WIDTH_SHIFT)

/**
 * @brief Full nine-bit window width of a CardaElement.
 * @param element Element whose width is read.
 * @param width_low The width's low byte, as read by CARDA_ELEMENT_WIDTH_LOW.
 */
#define CARDA_ELEMENT_WIDTH(element, width_low) ((s32)(((element)->size.f.width_high << 8) | (width_low)))

/** @brief Store the low eight bits of a CardaElement's window width. */
#define CARDA_SET_ELEMENT_WIDTH_LOW(element, width)                                                                        \
    ((element)->attr.word = ((element)->attr.word & ((1 << CARDA_ELEMENT_WIDTH_SHIFT) - 1)) | ((u32)(width) << CARDA_ELEMENT_WIDTH_SHIFT))

/**
 * @brief The FIELD render buffer CARDA draws into.
 *
 * Same layout as CLOAD's render buffer; CARDA links its windows into the
 * 16-entry ordering table at the start of the buffer.
 */
typedef struct
{
    u_long overlay_ot[0x10];
    u_long ordering_table[0x1000];
    DISPENV disp_env;
    DRAWENV draw_env;
    RECT clear_rect;
    void *prim_cursor;
} CardaRenderBuffer;

/**
 * @brief Address of CARDA text @p index, reached through its own u16 offset-table entry @p entry.
 * @note The table start is derived back from the entry symbol, like FIELD_UI_TEXT_AT.
 */
#define CARDA_TEXT_AT(entry, index) ((u8 *)&(entry) - (index) * 2 + (entry))

/** @brief Start of the CARDA text offset table, derived from entry @p entry at @p index. */
#define CARDA_TEXT_TABLE(entry, index) (&(entry) - (index))

/** @brief Address of CARDA text @p index in the u16 offset table starting at @p table. */
#define CARDA_TEXT(table, index) ((u8 *)(table) + (table)[index])

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


/** @brief Memory-card directory entry; layout matches Psy-Q struct DIRENTRY. */
typedef struct CardaDirEntry
{
    /* 0x00 */ char name[20];
    /* 0x14 */ s32 attr;
    /* 0x18 */ s32 size;
    /* 0x1C */ void *next;
    /* 0x20 */ s32 head;
    /* 0x24 */ char system[4];
} CardaDirEntry;

/** @brief Height in pixels of one row of the save-file list. */
#define CARDA_ENTRY_ROW_HEIGHT 14

/** @brief Number of directory entries per memory card. */
#define CARDA_ENTRIES_PER_CARD 20

/** @brief Number of suffix groups the directory sort buckets saves into. */
#define CARDA_ENTRY_GROUP_COUNT 8

/** @brief Byte size of one card's directory listing in g_carda_entries. */
#define CARDA_CARD_DIRECTORY_BYTES 0x320

/** @brief Byte size of one directory entry. */
#define CARDA_DIRECTORY_ENTRY_BYTES 0x28

/** @brief Byte size of one memory-card block. */
#define CARDA_MEMORY_CARD_BLOCK_BYTES 8192

/** @brief Six-byte memory-card path buffer ("bu00:" plus terminator), byte aligned. */
typedef struct
{
    u8 raw[6];
} CardaFileHeaderScratch;

/**
 * @brief 0x20-byte, word-aligned memory-card path scratch buffer.
 * The first six bytes are initialized from the "bu00:" device prefix before a
 * filename suffix is appended.
 */
typedef union
{
    u8 bytes[0x20];
    u32 align;
} CardaLoadScratch;

/** @brief 0x100-byte, word-aligned memory-card path buffer. */
typedef union
{
    u8 bytes[0x100];
    u32 align;
} CardaCardPathBuffer;

/** @brief 0x10-byte, word-aligned buffer initialized from the "bu00:*" search path. */
typedef union
{
    u8 bytes[0x10];
    u32 align;
} CardaCardSearchPath;

/** @brief Eight-byte, word-aligned memory-card path template ("bu00:"). */
typedef union
{
    char text[8];
    u32 align[2];
} CardaCardPathTemplate;

/** @brief Three two-byte overflow glyphs and their string terminator. */
typedef struct
{
    s8 data[7];
} CardaDecimalOverflow;

/** @brief One Shift-JIS character plus its terminator, copied whole into the save title. */
typedef struct
{
    s8 text[3];
} CardaSjisChar;

/** @brief Number of save records in the game state's record table. */
#define CARDA_SAVE_RECORD_COUNT 5

/** @brief Highest count an item stack can reach. */
#define CARDA_ITEM_COUNT_MAX 99

/** @brief One 0x60-byte record of the game state's record table. */
typedef struct CardaSaveRecord
{
    u8 active; /**< Nonzero when the slot holds a record. */
    u8 unk1[0x17];
    u32 unk18 : 8;
    u32 growth : 24; /**< Accumulates the save's growth delta. */
    u8 unk1C[0x40];
    s32 id; /**< Identifier matched when looking for a duplicate record. */
} CardaSaveRecord;

/** @brief The parts of the FIELD game state (g_pad_ctx) that CARDA reads and writes. */
typedef struct CardaGameState
{
    u8 unk0[0x25E0];
    u8 item_counts[0x914]; /**< Owned count of each item id. */
    CardaSaveRecord records[CARDA_SAVE_RECORD_COUNT];
} CardaGameState;

/** @brief Item list stored in a memory-card save. */
typedef struct CardaSaveItemList
{
    s32 growth_delta; /**< Added to the restored record's growth counter. */
    u32 count;        /**< Number of valid entries in ids. */
    u8 ids[0x50];
} CardaSaveItemList;

/** @brief The parts of the memory-card save buffer (g_carda_save_blob) that CARDA reads. */
typedef struct CardaSaveData
{
    u8 unk0[0x300];
    CardaSaveItemList items;
    CardaSaveRecord record; /**< Record restored into the game state. */
} CardaSaveData;

/**
 * @brief Typed views of the game state and save buffer.
 * @note g_pad_ctx and g_carda_save_blob are still declared as byte pointers because
 *       carda_draw_save_flow indexes them with raw offsets.
 */
#define CARDA_GAME_STATE ((CardaGameState *)g_pad_ctx)
#define CARDA_SAVE_DATA ((CardaSaveData *)g_carda_save_blob)

/**
 * @brief One 4-byte glyph-cache slot: the cached character code plus per-frame
 *        usage flags, also read as a single word when scanning for a free slot.
 */
typedef union
{
    u32 raw;
    struct
    {
        u16 code;
        u16 flags;
    } data;
} CardaGlyphCacheEntry;

/**
 * @brief 0x14-byte glyph packet: a Psy-Q SPRT_16 plus the trailing word that
 *        keeps consecutive cached-glyph packets 20 bytes apart.
 */
typedef struct
{
    SPRT_16 packet;
    u32 padding;
} CardaGlyphSprite;

/** @brief Glyph index marking an empty party-icon slot in CardaSaveMetadata. */
#define CARDA_NO_ICON 0x7F

/* Offset of the save-slot id inside the saved game (CardaSaveMetadata.save_slot_id). */
#define CARDA_SAVE_SLOT_ID_OFFSET 0xCF

/**
 * @brief The parts of the selected save file's saved game that the file-select
 *        screen shows.
 */
typedef struct
{
    u8 title[0x18];
    u32 unknown_0x18 : 25;
    u32 party_icon_0 : 7;
    u8 unknown_0x1c[3];
    u8 icon_palette;
    u32 location : 18; /**< Index into the location-name text table. */
    u32 party_icon_1 : 7;
    u32 party_icon_2 : 7;
    u8 unknown_0x24[0x30 - 0x24];
    s32 playtime; /**< Play time in 1/60 s ticks. */
    u8 unknown_0x34[CARDA_SAVE_SLOT_ID_OFFSET - 0x34];
    u8 save_slot_id;
} CardaSaveMetadata;

/**
 * @brief Text-bearing prefix of a PSX memory-card file header.
 *
 * The encoded title starts at +0x4 and spans two 0x20-byte lines.
 */
typedef struct
{
    u8 header[4];
    u8 title_line_1[0x20];
    u8 title_line_2[0x20];
} CardaCardHeaderText;

/**
 * @brief Address of FIELD UI string @p index, given its two-byte offset entry @p entry.
 * @note The table start is derived back from the entry symbol, as in FIELD's own lookups.
 */
#define FIELD_UI_TEXT_AT(entry, index) ((entry) - (index) * 2 + (entry)[0] + ((entry)[1] << 8))

/* FIELD / main-executable globals used by this overlay. */
extern s32 g_save_slot_index;
extern s32 g_playtime_vsync_origin;
extern u8 *g_pad_ctx;
extern s32 D_801227C4;
extern s32 g_pad_input;
extern s32 g_gosub_result_values;
extern s32 g_field_card_overlay_mode;
extern s32 g_menu_element_counter;

/* FIELD UI strings and memory-card file names. */
extern u8 D_800EC3D0[];
extern u8 g_text_time_separator_offset_bytes[2];
extern u8 g_text_choice_glyph_offsets;
extern char g_lom_save_filename_prefix[];
extern char g_lom_alt_save_filename_prefix[];
extern char g_lom_save_dummy_filename[];
extern char g_lom_alt_save_dummy_filename[];
extern char g_new_save_entry_prefix[];
extern char D_800ECFD0[];

/* CARDA read-only data. */
extern CardaSjisChar D_8014008C;
extern CardaSjisChar g_niki_file_template;
extern const CardaDecimalOverflow g_carda_decimal_overflow_text;
extern const CardaFileHeaderScratch g_carda_save_card_path_prefix;
extern const CardaCardPathTemplate g_carda_card_path_prefix;
extern const CardaCardPathTemplate g_carda_card_search_path;

/* CARDA text offset-table entries. */
extern u16 g_carda_text_check_memory_card;
extern u16 g_carda_text_not_enough_blocks;
extern u16 g_carda_text_no_memory_card;
extern u16 g_carda_text_mana;
extern u16 g_carda_text_other_game;
extern u16 D_8014B042;
extern u16 g_carda_text_card_slot_1;
extern u16 g_carda_text_card_slot_2;
extern u16 g_carda_text_card_access_failed;
extern u16 g_carda_text_no_save_data;
extern u16 g_carda_text_new_save;
extern u16 D_8014B04E;
extern u16 D_8014B050;
extern u16 D_8014B054;
extern u16 D_8014B058;
extern u16 D_8014B05E;
extern u16 g_carda_text_new_save_prompt;
extern u16 g_carda_text_save;
extern u16 g_carda_text_number_prefix;
extern u16 D_8014B068;
extern u16 D_8014B06A;
extern u16 g_carda_text_no_lom_save_data;
extern u16 g_carda_text_alt_save;
extern u16 D_8014B074;
extern u16 D_8014B076;
extern u16 D_8014B078;
extern u16 D_8014B07A;
extern u16 g_carda_text_version_error;
extern u16 D_8014B090;
extern u16 D_8014B092;
extern u16 D_8014B094;
extern u16 D_8014B09C;
extern u16 D_8014B09E;
extern u16 D_8014B0AC;
extern u16 D_8014B0AE;
extern u16 D_8014B0B0;
extern u16 D_8014B0B4;
extern u16 D_8014B0B6;
extern u16 D_8014B0BA;
extern u16 D_8014B0BE;
extern u16 D_8014B0C2;
extern u16 D_8014B0C6;
extern u16 D_8014B0CA;
extern u16 D_8014B0CC;
extern u16 D_8014B0D2;
extern u16 D_8014B0D4;
extern u16 D_8014B0D6;
extern u16 D_8014B0D8;
extern u16 D_8014B0DA;
extern u16 D_8014B0E6;
extern u16 g_carda_text_plus_marker;
extern u16 g_carda_text_card_unformatted;
extern u16 D_8014B4D4[];
extern u8 g_carda_save_title_template[];
extern u8 D_8014BEE4[];
extern s32 D_8014BF00[];
extern u16 g_carda_location_names[];
extern s32 D_8014CC54[];
extern u8 D_801629D0[];

/* Card-sequence step scripts (g_carda_save_step points into these). */
extern u8 g_carda_steps_initial_scan[];
extern u8 g_carda_steps_idle[];
extern u8 g_carda_steps_refresh_entries[];
extern u8 g_carda_steps_card_reset[];
extern u8 g_carda_steps_write_save[];
extern u8 g_carda_steps_write_save_keep_handles[];
extern u8 g_carda_steps_scan_and_write_alt_save[];
extern u8 g_carda_steps_write_alt_save[];
extern u8 g_carda_steps_read_selected_header[];
extern u8 g_carda_steps_load_selected_save[];
extern u8 g_carda_steps_card_check[];
extern u8 g_carda_steps_initial_scan_check_type[];
extern u8 g_carda_steps_read_save_prefix[];
extern u8 g_carda_steps_overwrite_alt_save[];
extern u8 g_carda_single_byte_char_table[];

/* CARDA state. */
extern u16 g_carda_decimal_glyphs[];
extern u16 g_carda_hex_glyphs[];
extern s32 g_carda_scroll_target_y;
extern s32 g_carda_new_save_file;
extern s32 g_carda_growth_delta;
extern u8 g_carda_received_item_ids[];
extern s32 D_80165F7C;
extern CardaElement g_carda_element_pool[8]; /**< UI element pool. */
extern CardaElement g_carda_element1_state;
extern s32 g_carda_exit_requested;
extern s32 g_carda_dialog_state;
extern s32 g_carda_received_item_count;
extern s32 g_carda_entry_state;
extern u8 *g_carda_save_blob;
extern s32 g_carda_selected_row;
extern s32 g_carda_choice_toggle;
extern s32 g_carda_scroll_frames;
extern s32 g_carda_io_busy;
extern s32 g_carda_frame_parity;
extern u8 g_carda_saved_record_copy[];
extern s32 g_carda_icon_phase;
extern s32 g_carda_icon_palette;
extern s32 g_carda_progress_active;
extern s32 g_carda_format_frames;
extern s32 g_carda_mode;
extern u8 g_carda_icon_context[];
extern s32 g_carda_card_slot;
extern s32 D_801660F8;
extern s32 g_carda_selection_status;
extern u8 *D_80166100;
extern s32 g_carda_scroll_y;
extern s32 D_80166108;
extern s32 D_8016610C;
extern s32 D_80166110;
extern s32 D_80166114;
extern s32 g_carda_save_in_progress;
extern CardaCardHeaderText g_carda_selected_file_header;
extern u8 D_80166124[];
extern CardaSaveMetadata g_carda_selected_save_metadata;
extern u8 g_carda_selected_save_slot_id;
extern u8 *g_carda_save_step;
extern s32 g_carda_file_handle;
extern s32 g_carda_entry_ranks[];
extern CardaFileHeaderScratch g_carda_selected_card_path;
extern s32 g_carda_rank_count;
extern s32 g_carda_retry_count;
extern CardaDirEntry g_carda_entries[2][CARDA_ENTRIES_PER_CARD]; /**< Directory listing of both cards. */
extern s32 g_carda_entry_suffix_values[];
extern s32 g_carda_selected_entry_extended;
extern s32 g_carda_primary_poll_countdown;
extern s32 D_80166AD8;
extern s32 g_carda_progress_bar_active;
extern s32 g_carda_entry_scan_active;
extern s32 g_carda_entry_fields[2][CARDA_ENTRIES_PER_CARD];
extern s32 g_carda_preserve_old_save;
extern s32 g_carda_progress_start_tick;
extern s32 g_carda_secondary_poll_countdown;
extern s32 D_80166B94;
extern s32 D_80166B98;
extern s32 D_80166B9C;
extern s32 D_80166BA0;
extern u8 g_carda_temp_card_path[];
extern CardaGlyphCacheEntry g_carda_glyph_cache[];
extern s32 g_carda_glyph_cursor_x;
extern s32 g_carda_glyph_cursor_y;
extern s32 g_carda_text_line_start_x;
extern s32 g_carda_glyph_upload_x;
extern s32 g_carda_glyph_upload_y;
extern u8 *g_carda_glyph_raster_cursor;
extern u8 g_carda_glyph_raster_buffer[];

/*
 * External callees.  The ones declared with an empty parameter list were
 * called without a prototype in the original sources and must stay that way.
 */
void field_reset_input_repeat(void);
s32 cdrom_wait_queue_empty();
s32 cdrom_queue_read();
s32 reset_controller_vsync_state();
s32 card_resource_noop_hook();
s32 OpenEvent(s32, s32, s32, s32);
void CloseEvent(s32);
s32 TestEvent(s32);
void EnableEvent(s32);
void EnterCriticalSection(void);
void ExitCriticalSection(void);
s32 open(void *, s32);
s32 read(s32, void *, s32);
s32 write(s32, void *, s32);
s32 close(s32);
s32 nextfile();
s32 rename(void *, void *);
s32 erase(void *);
u8 *Krom2RawAdd(u16 sjis_code);
s32 firstfile();
void bcopy();
s32 rand();
s32 strcat(void *, void *);
s32 strcpy(void *, void *, ...);
s32 strncmp();
s32 _card_info(s32);
s32 _card_load(s32);
s32 _card_write(s32, s32, void *);
s32 _card_read(s32, s32, void *);
s32 _card_wait(s32);
s32 _card_clear(s32);
s32 _card_format();
s32 VSync(s32);
s32 func_80032174(s32, void *, s32 *);
s32 func_80033E7C(s32);
s32 McxCardType(s32);
s32 func_80034648(s32, s32, s32);
s32 field_set_fade_target();
void field_restore_fade_target(void);
void field_restore_fade_target_with_duration();
void play_menu_sfx(s32, s32);
void func_800A55E4(void *buf, s32 arg1);
void func_800A5638(void *buf, s32 arg1);
void *func_800A88A0(void *prim, u_long *ot, u8 *text, s32 color, s32 x, s32 y, s32 mode);
void *func_800A8A78(u_long *ot, void *prim, s32 value, s32 color, DVECTOR *pos, s32 mode);
void *func_800AD850(void *prim, u_long *ot, s32 x, s32 y, s32 w, s32 h, s32 clear_y, s32 draw_fill);
void *func_800AE76C(void *prim, u_long *ot, s32 x, s32 y, s32 flag);
void func_800B0170();
void func_800C1230(s32 slot);

/*
 * CARDA functions.  The ones declared with an empty parameter list are called
 * before their definition with arguments that do not match it (or used to size
 * the outgoing-argument area), so they must not get a prototype here.
 */
s32 carda_update_frame(CardaRenderBuffer *frame);
void carda_build_ui_elements(void);
void carda_update_menu(CardaRenderBuffer *frame);
s32 carda_update_save_sequence(void);
s32 carda_handle_input(void);
void carda_switch_card(void);
void carda_close_all_elements(void);
void carda_scroll_to_selection(void);
void carda_update_elements(CardaRenderBuffer *frame);
void *carda_draw_entry_list(u_long *ot, void *prim, s32 x_offset, s32 y_offset);
void *carda_draw_header_label(u_long *ot, void *prim, s32 x_offset, s32 y_offset);
void *carda_draw_card_slot0_label(u_long *ot, void *prim, s32 x_offset, s32 y_offset);
void *carda_draw_card_slot1_label(u_long *ot, void *prim, s32 x_offset, s32 y_offset);
void *carda_draw_selected_entry_details(u_long *ot, void *prim, s32 x_offset, s32 y_offset);
void carda_terminate_multibyte_text(void *text);
void *carda_draw_field_notice(u_long *ot, void *prim, s32 x_offset, s32 y_offset);
void carda_clear_elements(void);
CardaElement *carda_alloc_element(void);
void carda_update_and_draw_elements(CardaRenderBuffer *frame);
void carda_deactivate_primary_element(void);
void carda_text_append(u8 *dest, u8 *src);
s32 carda_text_byte_length(u8 *text);
void carda_text_copy(u8 *dest, u8 *src);
void carda_build_save_file(void);
u8 *carda_skip_hex_digits(u8 *text);
s32 carda_test_option_flag_2(void);
s32 carda_compute_save_checksum(void *data);
void *carda_draw_load_prompt(u_long *ot, void *prim, s32 x_offset, s32 y_offset);
void *carda_draw_load_progress(u_long *ot, void *prim, s32 x_offset, s32 y_offset);
void *carda_draw_progress_bar(POLY_G4 *quad, u_long *ot);
void *carda_draw_save_prompt(u_long *ot, void *prim, s32 x_offset, s32 y_offset);
void *carda_draw_overwrite_prompt(u_long *ot, void *prim, s32 x_offset, s32 y_offset);
void *carda_draw_save_progress(u_long *ot, void *prim, s32 x_offset, s32 y_offset);
void *carda_draw_save_complete(u_long *ot, void *prim, s32 x_offset, s32 y_offset);
void *carda_draw_format_prompt(u_long *ot, void *prim, s32 x_offset, s32 y_offset);
void *carda_draw_format_progress(u_long *ot, void *prim, s32 x_offset, s32 y_offset);
void carda_open_status_dialog(s32 dialog_state);
void *carda_draw_status_dialog(u_long *ot, void *prim, s32 x_offset, s32 y_offset);
void *carda_draw_icon_highlight(POLY_FT4 *quad, u_long *ot, s32 x, s32 y, s32 width, s32 icon, s32 index, s32 row);
void carda_enable_choice_toggle(void);
void *carda_draw_choice_prompt(void *prim, u_long *ot, s32 x, s32 y);
s32 carda_draw_save_flow(s32 *ot, s32 prim, s32 x_offset, s32 y_offset);
void carda_store_active_record(void);
void carda_restore_active_record(void);
s32 carda_draw_slot_prompt(s32 prim, s32 *ot, s32 x, s32 y);
void carda_open_save_status_dialog(s32 dialog_state);
s32 carda_draw_save_status_dialog(s32 *ot, s32 prim, s32 x_offset, s32 y_offset);
void carda_open_item_list(void);
s32 carda_draw_item_list_header(s32 *ot, s32 prim, s32 x_offset, s32 y_offset);
s32 carda_draw_item_list(s32 *ot, s32 prim, s32 x_offset, s32 y_offset);
void carda_apply_save_items(void);
s8 *carda_format_decimal(s8 *out, s32 value);
void carda_format_hex(s8 *out, s32 value, s32 max_chars);
void carda_hex_nibble_to_ascii(s8 *out, s32 value);
u32 carda_parse_hex(u8 *s, s32 len);
s32 carda_parse_hex_suffix_byte(u8 *text);
s32 carda_parse_entry_fields(void);
s32 carda_rank_entries(void);
void carda_reset_entry_ranks(void);
s32 carda_has_known_entry_type(void);
s32 carda_card_lacks_free_blocks(void);
void carda_erase_fixed_card_files(void);
s32 carda_advance_save_sequence();
void carda_reset_to_new_save_entry(void);
void carda_restart_card_sequence();
s32 carda_poll_and_rewind_primary_handles(void);
void carda_init_stream_handles(void);
void carda_shutdown_stream_handles(void);
s32 carda_begin_entry_scan(s32 page);
s32 carda_scan_next_entry(s32 page);
void carda_commit_selected_entry(void);
void carda_release_primary_handles(void);
void carda_release_secondary_handles(void);
s32 carda_poll_primary_handle_group(void);
s32 carda_poll_secondary_handle_group(void);
void carda_sort_entries_by_type(void);
void *carda_draw_signed_decimal(void *prim, u_long *ot, s32 value, s32 x, s32 y, s32 palette, s32 alignment);
void carda_draw_hex_byte(void *prim, u_long *ot, s32 value, s32 x, s32 y, s32 alignment);
void *carda_draw_cached_text(void *prim, u_long *ot, u8 *text, s32 x, s32 y, s32 palette, s32 alignment);
void *carda_render_cached_glyph(void *prim, u_long *ot, u16 code, s32 palette);
void *carda_emit_glyph_sprite(CardaGlyphSprite *sprite, u_long *ot, s32 cache_slot, s32 palette);
void carda_begin_glyph_cache_frame(void);
void carda_evict_unused_glyphs(void);
void carda_reset_glyph_cache(void);
void carda_expand_text_glyph_codes(u8 *out, u8 *in);

#endif /* CARDA_INTERNAL_H */
