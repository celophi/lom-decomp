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

/* Declarations shared by ADDHERO implementation files. */

/** @brief Save files a memory card holds (its 15 data blocks). */
#define ADDHERO_CARD_SAVE_SLOTS 15
#define ADDHERO_USED_BLOCK_LIMIT 14
#define ADDHERO_LOAD_RESULT_NONE 0
#define ADDHERO_LOAD_RESULT_ABORT 2
#define ADDHERO_LOAD_RESULT_CONTINUE 3
#define ADDHERO_LOAD_RESULT_COMPLETE 4
#define ADDHERO_LOAD_RESULT_CARD_ERROR 5

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
extern CardPathTemplate g_addhero_file_template;
extern s32 g_addhero_scroll_y;
extern s32 g_addhero_progress_active;
extern s32 g_addhero_scroll_target_y;
extern s32 g_addhero_mode;
extern s32 g_addhero_selected_row;
extern s32 g_addhero_selection_status;
extern s32 g_addhero_scroll_frames;
extern s32 g_addhero_io_busy;
extern s32 g_addhero_progress_bar_active;
extern s32 g_addhero_progress_start_tick;
extern s32 g_addhero_entry_scan_active;
extern s32 g_addhero_write_in_progress;
extern s32 g_addhero_rank_count;
extern s32 g_addhero_entry_ranks[];
extern s32 g_addhero_selected_entry_extended;
extern s32 g_addhero_entry_value_limit;
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
/** @brief File name prefix of the PocketStation mini-game (Ring Ring Land) save. */
extern char g_lom_pocketstation_filename_prefix[];
extern char g_new_save_entry_prefix[];
extern char g_lom_save_dummy_filename[];
/** @brief Temporary file name used while a PocketStation save is written. */
extern char g_lom_pocketstation_dummy_filename[];

void addhero_scroll_to_selection(void);
void addhero_open_status_dialog(s32 message_id);
void addhero_open_exit_dialog(s32 message_id);
s32 addhero_rank_entries(void);
s32 addhero_has_known_entry_type(void);
s32 addhero_begin_entry_scan(s32 page);
s32 addhero_scan_next_entry(s32 page);
void addhero_sort_entries_by_type(void);
void addhero_init_card_events(void);
void addhero_commit_selected_entry(void);
s32 addhero_advance_load_sequence(void);

/** @brief Left edge and width of the entry-list window of the browser layout (mode 0). */
#define ADDHERO_LIST_X 28
#define ADDHERO_LIST_WIDTH 264

/** @brief X of the scroll arrows, inset from the entry list's right edge. */
#define ADDHERO_SCROLL_ARROW_X (ADDHERO_LIST_X + ADDHERO_LIST_WIDTH - 16)

/** @brief Title window of the browser layout. */
#define ADDHERO_TITLE_X 36
#define ADDHERO_TITLE_Y 10
#define ADDHERO_TITLE_WIDTH 240
#define ADDHERO_TITLE_HEIGHT 16

/** @brief Card-slot label windows; slot 0 sits at ADDHERO_CARD_SLOT0_LABEL_X. */
#define ADDHERO_CARD_SLOT1_LABEL_X 160
#define ADDHERO_CARD_LABEL_BROWSER_Y 30  /**< Browser layout. */
#define ADDHERO_CARD_LABEL_TRANSFER_Y 77 /**< Transfer layout. */

/** @brief Top of the message window (the transfer layout's status and the load prompt), and the two-line prompt height. */
#define ADDHERO_MESSAGE_Y 97
#define ADDHERO_PROMPT_HEIGHT 30  /**< Two lines. */

/** @brief Dialog window. */
#define ADDHERO_DIALOG_X 32
#define ADDHERO_DIALOG_Y 112
#define ADDHERO_DIALOG_WIDTH 256
#define ADDHERO_DIALOG_HEIGHT 20

/** @brief ADDHERO's own g_addhero_dialog_state message, after the shared CARD_MENU_DIALOG_* ones; it shows the load-failed text. */
#define ADDHERO_DIALOG_INVALID_SAVE 4

/** @brief g_addhero_result values, reported to the host when the overlay exits. */
#define ADDHERO_RESULT_LOADED 1
#define ADDHERO_RESULT_SAVED 2
#define ADDHERO_RESULT_CANCELLED 3

/** @brief Frames of the fade back to the host screen when the overlay exits. */
#define ADDHERO_EXIT_FADE_FRAMES 8

/**
 * @brief Commands in the card load/save sequence bytecode.
 * @note Opcodes without a case are no-ops.
 */
typedef enum
{
    ADDHERO_STEP_DONE = 0,                  /**< End of a step table; report ADDHERO_LOAD_RESULT_ABORT. */
    ADDHERO_STEP_CARD_INFO = 1,             /**< Issue _card_info on the current slot. */
    ADDHERO_STEP_POLL_CARD_INFO = 2,        /**< Wait for the _card_info result. */
    ADDHERO_STEP_CLEAR_SOFTWARE_EVENTS = 3, /**< Clear the software card events. */
    ADDHERO_STEP_POLL_HARDWARE_EVENTS = 4,  /**< Wait for and check the hardware card events. */
    ADDHERO_STEP_CLEAR_HARDWARE_EVENTS = 5, /**< Clear the hardware card events. */
    ADDHERO_STEP_SCAN_ENTRIES = 6,          /**< Erase the placeholder files and scan the card directory. */
    ADDHERO_STEP_SCAN_DONE = 7,             /**< No case: the sequence waits here after the scan; input stays blocked on this step and the scan. */
    ADDHERO_STEP_CLEAR_CARD = 8,            /**< Issue _card_clear on the current slot. */
    ADDHERO_STEP_LOAD_CARD = 9,             /**< Issue _card_load and arm the poll countdowns. */
    ADDHERO_STEP_ERASE_ENTRY = 10,          /**< Erase the selected directory entry. */
    ADDHERO_STEP_WAIT = 14,                 /**< No case: the sequence waits here until other code replaces it. */
    ADDHERO_STEP_POLL_CARD_LOAD = 15,       /**< Wait for the _card_clear/_card_load result, retrying. */
    ADDHERO_STEP_WAIT_HARDWARE_EVENTS = 16, /**< Wait for any hardware card event. */
    ADDHERO_STEP_READ_ENTRY = 17,           /**< Open the selected save and start reading its header. */
    ADDHERO_STEP_POLL_ENTRY_READ = 18,      /**< Wait for the header read to finish. */
    ADDHERO_STEP_READ_SAVE = 19,            /**< Open the selected save and start reading the blob. */
    ADDHERO_STEP_POLL_SAVE_READ = 20,       /**< Wait for the blob read to finish, retrying. */
    ADDHERO_STEP_CHECK_POCKETSTATION = 24,  /**< Check that the card is a PocketStation (McxCardType); no ADDHERO sequence uses it. */
    ADDHERO_STEP_WRITE_SAVE = 25,           /**< Create the placeholder file and start writing the save blob. */
    ADDHERO_STEP_POLL_SAVE_WRITE = 26,      /**< Wait for the write and rename it over the selected save. */
    ADDHERO_STEP_READ_BEFORE_WRITE = 27,    /**< Open the selected save and read the blob before writing. */
    ADDHERO_STEP_POLL_PREWRITE_READ = 28,   /**< Wait for that read to finish, retrying. */
    ADDHERO_STEP_INIT_RETRIES = 30          /**< Arm the read/write retry counter. */
} AddheroCardStep;
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

extern CardMenuElement g_addhero_element_pool[CARD_MENU_ELEMENT_COUNT];

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
extern s32 g_addhero_exit_requested;
extern s32 g_addhero_choice_toggle;
extern s32 g_addhero_load_flow_active;
extern s32 g_addhero_icon_palette;
extern s32 g_addhero_frame_parity;
extern s32 g_addhero_dialog_state;
extern s32 g_addhero_icon_image_table[];
extern u8 g_addhero_loadseq_abort[];
extern u8 g_addhero_loadseq_load_begin[];
extern u8 g_addhero_loadseq_load_progress[];
extern u8 g_addhero_loadseq_save_begin[];
extern u8 g_addhero_icon_context[];
extern u8 g_text_time_separator_offset_bytes[2];
extern u8 g_text_choice_glyph_offsets[];
extern u16 g_addhero_text_table;
extern u16 g_addhero_text_not_enough_blocks;
extern u16 g_addhero_text_no_card;
extern u16 g_addhero_text_mana_label;
extern u16 g_addhero_text_other_game_label;
extern u16 g_addhero_text_card_slot0_label;
extern u16 g_addhero_text_card_slot1_label;
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
void addhero_clear_elements(void);
CardMenuElement* addhero_alloc_element(void);
void addhero_update_and_draw_elements(AddheroDrawState* draw_state);
void addhero_deactivate_primary_element(void);
void* addhero_draw_load_prompt(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* addhero_draw_load_progress(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* addhero_draw_progress_bar(POLY_G4* quad, u_long* ot);
void* addhero_draw_status_dialog(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* addhero_draw_exit_dialog(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* addhero_draw_transfer_status(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* addhero_draw_icon_highlight(POLY_FT4* quad, u_long* ot, s32 x, s32 y, s32 width, s32 icon, s32 index, s32 row);
void addhero_enable_choice_toggle(void);
void* addhero_draw_choice_prompt(void* prim, u_long* ot, s32 x, s32 y);
s32 addhero_entry_blocks_reach_limit(void);
void addhero_erase_placeholder_files(void);

void addhero_reset_entry_ranks(void);

/* FIELD functions used by ADDHERO; FIELD stays resident while the overlay runs. */
void field_copy_portrait_palette(void* dest, s32 index);
void field_copy_golem_portrait_palette(u8* destination, s32 palette);
void field_flag_known_save(char* file_name);
void field_reset_input_repeat(void);
void field_restore_fade_target(void);
void field_set_default_fade_target(void);
void field_restore_fade_target_with_duration(s16 duration);

#endif
