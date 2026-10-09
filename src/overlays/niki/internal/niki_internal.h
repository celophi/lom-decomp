#ifndef LOM_NIKI_INTERNAL_H
#define LOM_NIKI_INTERNAL_H

#include "common.h"
#include "common/saved_game.h"
#include "common/vector.h"
#include "main/display.h"
#include "overlays/field/field_menu_window.h"
#include "common/card_menu.h"
#include <kernel.h>
#include <libetc.h>
#include <libmcx.h>
#include "common/encoded_text.h"
#include "common/save_file.h"
#include "common/glyph_cache.h"
#include "common/card_events.h"
#include "common/card_directory.h"
#include <strings.h>
#include "main/controller.h"
#include <libapi.h>
#include "overlays/field/field_fade.h"
#include "overlays/field/field_input.h"
#include "overlays/field/field_sound.h"
#include "main/audio/akao.h"
#include <memory.h>
#include <libgpu.h>
#include "overlays/field/field_ui_text.h"

#define NIKI_SJIS_FULLWIDTH_ZERO 0x4F82
#define NIKI_SJIS_MINUS 0x5B81
#define NIKI_CANCEL_INPUT_MASK PAD_BTN_CIRCLE
#define NIKI_ELEMENT_WORD_STRIDE 3
#define NIKI_ELEMENT_TRANSITION_STEP_MASK 0x78
#define GLYPH_OFF(base, off) ((void*)((base) + *(u16*)((base) + (off))))
#define GPU_ADDR_MASK 0xFFFFFF
#define GPU_TAG_HIGH_MASK 0xFF000000
#define NIKI_SET_PACKET_LENGTH(prim, length) (((NikiPrimTag*)(prim))->len = (u8)(length))
#define NIKI_SET_PACKET_ADDRESS(prim, address) (((NikiPrimTag*)(prim))->addr = (uintptr_t)(address))
#define NIKI_SET_PACKET_CODE(prim, command) (((NikiPrimTag*)(prim))->code = (u8)(command))
#define NIKI_GET_PACKET_ADDRESS(prim) ((u32)(((NikiPrimTag*)(prim))->addr))
#define NIKI_ADD_PRIMITIVE(ordering_table, prim)                                                                                                               \
    (NIKI_SET_PACKET_ADDRESS((prim), NIKI_GET_PACKET_ADDRESS(ordering_table)), NIKI_SET_PACKET_ADDRESS((ordering_table), (prim)))
#define NIKI_TEXT_EXTENDED_LEAD_FIRST 0x19
#define NIKI_TEXT_EXTENDED_PAGE_COUNT 7
#define NIKI_TEXT_SINGLE_BYTE_BASE 0x20
#define NIKI_TEXT_PRINTABLE_FIRST 0x21
#define NIKI_SJIS_CODES_PER_ROW 16
#define NIKI_SJIS_ROWS_PER_PAGE 16

#define NIKI_SJIS_ROW_SHIFT 4

/** @brief Fields restored from a loaded save before returning to the game. */
typedef struct
{
    u8 unknown_0x000[0x197];
    u8 trailing_record_count;
    u8 unknown_0x198[0x254 - 0x198];
    u16 unknown_0x254;
    u16 unknown_0x256;
    u8 unknown_0x258[0x32E0 - 0x258];
    u8 trailing_data[0x100];
} NikiLoadedSavePayload;

/** @brief Memory-card transfer buffer with serialized and loaded-payload views. */
typedef union
{
    SaveFile save;
    NikiLoadedSavePayload loaded;
    u8 bytes[SAVE_FILE_BYTES];
} NikiSaveBuffer;

/** @brief Linked-list tag shared by GPU packets of different sizes. */
typedef struct
{
    s32 tag;
} NikiPacketHeader;

/** @brief GPU linked-list address and packet length, with a packed word view. */
typedef union
{
    s32 word;
    struct
    {
        u8 address[3];
        u8 length;
    } bytes;
} NikiGpuTag;

/** @brief GPU vertex color and command byte, with a packed word view. */
typedef union
{
    s32 word;
    struct
    {
        u8 r, g, b, code;
    } bytes;
} NikiGpuColor;

/** @brief Flat rectangle packet used for selection and inactive-card shading. */
typedef struct
{
    NikiGpuTag tag;
    NikiGpuColor color;
    s16 x0;
    s16 y0;
    s16 w;
    s16 h;
} NikiTile;

typedef struct
{
    unsigned addr : 24;
    unsigned len : 8;
    u8 r0, g0, b0, code;
} NikiPrimTag;

/** @brief Two bytes of a Shift-JIS character in the glyph lookup tables. */
typedef struct NikiSjisCode
{
    u8 lead;
    u8 trail;
} NikiSjisCode;

/** @brief Sixteen Shift-JIS characters followed by the table row delimiter. */
typedef struct NikiSjisRow
{
    NikiSjisCode codes[NIKI_SJIS_CODES_PER_ROW];
    u8 newline;
} NikiSjisRow;

/** @brief Lookup page selected by an extended character lead byte. */
typedef struct NikiSjisPage
{
    NikiSjisRow rows[NIKI_SJIS_ROWS_PER_PAGE];
} NikiSjisPage;

extern s32 g_niki_io_busy;
extern s32 g_niki_icon_phase;
extern s32 g_niki_confirm_latch;
extern s32 D_80164AE4;
extern s32 g_niki_mode;
extern s32 g_niki_exit_requested;
extern s32 g_niki_selection_status;
extern s32 g_niki_frame_parity;
extern s32 g_niki_progress_active;
/** @brief The saved game's item records (g_saved_game_ctx->items). */
extern FieldItemRecord* g_niki_items;
extern s32 g_niki_selected_row;
extern s32 g_pad_input;
extern s32 g_niki_scroll_frames;
extern s32 g_niki_scroll_y;
extern s32 g_niki_scroll_target_y;
extern s32 D_80164B80;
/** @brief Clear/load card state, then scan its directory. */
extern u8 g_niki_card_setup_sequence[];
/** @brief Release card events, refresh card information, and rescan entries. */
extern u8 g_niki_rescan_sequence[];
/** @brief Release primary events, then request and poll card information. */
extern u8 g_niki_card_info_sequence[];
extern CardMenuElement g_niki_element_pool[CARD_MENU_ELEMENT_COUNT];
extern s32 g_niki_entry_scan_active;
extern s32 g_field_niki_addhero_state;
extern s32 g_save_compatibility_tag;
extern s32 g_niki_icon_palette;
extern s32 g_niki_dialog_state;
/**
 * @brief Start of the selected entry's save file: only the card header and the
 *        first 0x100 bytes of the saved game are read (CARD_MENU_ENTRY_READ_BYTES).
 */
extern SaveFile g_niki_entry_file;
/**
 * @brief Card-screen message table: u16 offsets from its start, then the strings.
 * @note Each g_niki_text_* symbol is one slot of this table; the draw code adds
 *       the slot's value to the table address to find the string.
 */
extern u16 g_niki_text_table;
extern u16 g_niki_text_not_enough_blocks;
extern u16 g_niki_text_no_card;
extern u16 g_niki_text_mana_label;
extern u16 g_niki_text_other_game_label;
extern u16 g_niki_text_card_slot0_label;
extern u16 g_niki_text_card_slot1_label;
extern u16 g_niki_text_card_access_failed;
extern u16 g_niki_text_no_save_data;
extern u16 g_niki_text_new_save_label;
extern u16 g_niki_text_saving;
extern u16 g_niki_text_new_save_title;
extern u16 g_niki_text_number_label;
extern u16 g_niki_text_load_prompt;
extern u16 g_niki_text_loading;
extern u16 g_niki_text_no_game_save_data;
extern u16 g_niki_text_ring_ring_land_label;
extern u16 g_niki_text_save_failed;
extern u16 g_niki_text_load_failed;
extern u16 g_niki_text_card_not_inserted;
extern u16 g_niki_text_not_pocketstation;
extern u16 g_niki_text_select_save_data;
extern u16 g_niki_text_select_item;
extern u16 g_niki_text_no_items;
extern u16 g_niki_text_same_hero_data;
extern u16 g_niki_text_wrong_version;
extern u16 g_niki_text_no_load_file;
extern u16 g_niki_text_found_load_file;
extern u16 g_niki_text_trade_data_not_saved;
extern u16 g_niki_text_plus_marker;
/** @brief Location names, picked by the music track stored in a save. */
extern u16 g_niki_location_names[];
extern u8 g_field_ui_text_cant_hold_more[];
extern s32 g_niki_choice_toggle;

/** @brief Reset retries and read the selected save into the transfer buffer. */
extern u8 g_niki_load_save_sequence[];
extern NikiSaveBuffer g_niki_save_blob;
extern u8 D_8011F3D8[];
/** @brief Path selected for loading or replacing a save file. */
extern u8 g_niki_selected_save_path[];
/** @brief FIELD's NIKI outcome: 0 none or written back, 1 save loaded, 2 failed or declined. */
extern s32 g_field_niki_state;
extern s32 D_801227CC;
extern s32 D_801227F4;
extern s32 D_8011F418;
extern u8 g_field_shared_items[];
extern s32 g_niki_progress_bar_active;
extern s32 g_niki_progress_start_tick;
extern s32 g_niki_entry_ranks[];
extern s32 g_niki_rank_count;
/** @brief Party icon offsets, counted from the icon count word just before them. */
extern s32 g_niki_icon_offsets[];
extern u8 g_niki_icon_context[];
/** @brief Read the existing save before modifying and writing it back. */
extern u8 g_niki_read_saved_copy_sequence[];
/** @brief Reset retries and write the replacement save. */
extern u8 g_niki_write_save_sequence[];
extern s32 g_niki_entry_value_limit;
extern const CardPathTemplate g_niki_file_template;
extern s32 g_niki_file_handle;
extern s32 g_niki_retry_count;
extern s32 g_niki_selected_entry_extended;
extern s32 g_niki_primary_poll_countdown;
extern s32 g_niki_secondary_poll_countdown;
/** @brief Card has space to keep the old save until its replacement is written. */
extern s32 g_niki_preserve_old_save;
/** @brief Path written before renaming the replacement to the selected save path. */
extern u8 g_niki_temporary_save_path[];
/** @brief Directory search path matching every file on the card ("bu00:*"). */
extern const CardPathTemplate g_niki_entry_header_template;
/** @brief Read and poll the selected entry's preview header. */
extern u8 g_niki_preview_sequence[];

void niki_update_elements(FieldRenderHalf* frame);
void niki_update_and_draw_elements(FieldRenderHalf* frame);
s32 niki_update_load_sequence(void);
s32 niki_handle_input(void);
void* niki_draw_entry_list(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* niki_draw_header_label(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* niki_draw_card_slot0_label(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* niki_draw_card_slot1_label(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* niki_draw_selected_entry_details(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
u8* niki_draw_icon_highlight(u8* prim, u_long* ot, s32 x, s32 y, s32 width, s32 icon_index, s32 texture_slot, s32 palette_mode);
void* niki_draw_footer_label(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* niki_draw_state_page(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void niki_clear_elements();
s32 niki_advance_load_sequence(void);
void* niki_draw_status_dialog(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* niki_draw_secondary_status_dialog(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void niki_close_all_elements();
void niki_switch_card_slot();
void niki_commit_selected_entry(void);
void niki_scroll_to_selection();
CardMenuElement* niki_alloc_element();
void niki_enable_choice_toggle();
void* niki_draw_save_confirm_dialog(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* niki_draw_confirm_prompt(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void niki_sort_entries_by_type();
void niki_reset_entry_ranks(void);
s32 niki_rank_entries(void);
s32 niki_begin_entry_scan(s32);
s32 niki_scan_next_entry(s32);
void niki_open_status_dialog(s32);
void niki_open_secondary_status_dialog(s32);
void niki_build_ui_elements(void);
void niki_update_menu(FieldRenderHalf* frame);

#endif
