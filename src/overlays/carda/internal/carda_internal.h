#ifndef CARDA_INTERNAL_H
#define CARDA_INTERNAL_H

#include "overlays/field/field_text.h"
#include "common.h"
#include "common/saved_game.h"
#include "common/pad.h"
#include "common/vector.h"
#include "common/gpu_packet.h"
#include "main/display.h"
#include <libgte.h>
#include <libgpu.h>
#include <libmcx.h>
#include "overlays/field/field_menu_window.h"
#include "main/field_runtime.h"
#include "overlays/field/field_sound.h"
#include "overlays/field/field_ui_text.h"
#include "common/encoded_text.h"
#include "common/save_file.h"
#include "common/glyph_cache.h"
#include "common/card_events.h"
#include "main/card_callbacks.h"
#include "common/card_directory.h"
#include "common/card_menu.h"
#include "main/cdrom.h"
#include "main/controller.h"
#include <strings.h>
#include <libetc.h>
#include "carda.h"
#include "carda_widgets.h"
#include "carda_save.h"
#include "carda_card.h"
/**
 * @brief g_carda_mode values: what FIELD opened the card screen for.
 * @note The US release keeps the PocketStation modes but leaves their texts empty.
 */
#define CARDA_MODE_SAVE 0       /**< Save the game to a memory card. */
#define CARDA_MODE_LOAD 1       /**< Load a saved game. */
#define CARDA_MODE_SEND_PET 2   /**< Download Ring Ring Land to a PocketStation, taking a ranch pet along. */
#define CARDA_MODE_RETURN_PET 3 /**< Upload the pet and the items it found from Ring Ring Land. */

/** @brief Nonzero when @p mode is one of the PocketStation transfers. */
#define CARDA_IS_POCKETSTATION_MODE(mode) ((mode) == CARDA_MODE_SEND_PET || (mode) == CARDA_MODE_RETURN_PET)

/**
 * @brief Results CARDA leaves in g_field_card_overlay_mode for FIELD.
 * @note FIELD sets it to the mode plus one before CARDA starts, so a finished
 *       save leaves 1 and a finished load leaves 2.
 */
#define CARDA_RESULT_CANCELLED 3
#define CARDA_RESULT_PET_SENT 4
#define CARDA_RESULT_PET_RETURNED 5
#define CARDA_RESULT_SEND_PET_CANCELLED 6
#define CARDA_RESULT_RETURN_PET_CANCELLED 7 /**< Also set when the ranch has no free pet slot. */

/**
 * @brief g_card_entry_state values.
 *
 * Below CARD_MENU_ENTRY_COUNT_LIMIT the value is the number of directory entries
 * read from the current card. From 0xE9 up it is a status whose message the
 * entry list or the PocketStation window shows instead of the entries; the
 * values below 0xF6 are only used by the PocketStation modes. The states from
 * 0xF8 up are the shared CARD_MENU_ENTRY_STATE_* values.
 */
#define CARDA_ENTRY_STATE_PET_ALREADY_ON_RANCH 0xE9       /**< The pet on the PocketStation is already on the ranch. */
#define CARDA_ENTRY_STATE_CONFIRM_RETURN 0xEA             /**< Asks whether to return the pet and erase Ring Ring Land. */
#define CARDA_ENTRY_STATE_FORMAT_FAILED 0xEB              /**< Status dialog CARDA_DIALOG_FORMAT_FAILED. */
#define CARDA_ENTRY_STATE_SAVE_CORRUPT 0xEC               /**< Status dialog CARDA_DIALOG_SAVE_CORRUPT. */
#define CARDA_ENTRY_STATE_NO_POCKETSTATION 0xED           /**< Status dialog CARD_MENU_DIALOG_NOT_POCKETSTATION. */
#define CARDA_ENTRY_STATE_POCKETSTATION_NOT_INSERTED 0xEE /**< Status dialog CARD_MENU_DIALOG_CARD_NOT_INSERTED. */
#define CARDA_ENTRY_STATE_UPLOAD_FAILED 0xEF              /**< Status dialog CARD_MENU_DIALOG_LOAD_FAILED. */
#define CARDA_ENTRY_STATE_DOWNLOAD_FAILED 0xF0            /**< Status dialog CARD_MENU_DIALOG_SAVE_FAILED. */
#define CARDA_ENTRY_STATE_SELECT_SLOT 0xF1                /**< Only the slot prompt is shown. */
#define CARDA_ENTRY_STATE_CONFIRM_DOWNLOAD 0xF2           /**< Asks whether to download Ring Ring Land. */
#define CARDA_ENTRY_STATE_DOWNLOADING 0xF3                /**< Writing Ring Ring Land; progress bar. */
#define CARDA_ENTRY_STATE_UPLOADING 0xF4                  /**< Reading Ring Ring Land back; progress bar. */
#define CARDA_ENTRY_STATE_NO_RING_RING_LAND 0xF5          /**< Ring Ring Land was not found; CARDA never sets it. */
#define CARDA_ENTRY_STATE_NOT_POCKETSTATION 0xF6          /**< The card is not a PocketStation. */
#define CARDA_ENTRY_STATE_NO_ROOM_FOR_DOWNLOAD 0xF7       /**< Not enough free blocks for Ring Ring Land. */

/** @brief Entry-state values below this count as entry counts for the card sequence and the input handler. */
#define CARDA_ENTRY_COUNT_INPUT_LIMIT 0x12

/**
 * @brief Commands in the card sequence bytecode that g_card_step walks.
 * @note Opcodes without a case (7, 14) are no-ops that hold the sequence in place.
 */
typedef enum CardaCardStep
{
    CARDA_STEP_DONE = 0,                                /**< End of a step table; report CARDA_SEQUENCE_FINISHED. */
    CARDA_STEP_CARD_INFO = 1,                           /**< Issue _card_info on the current slot. */
    CARDA_STEP_POLL_CARD_INFO = 2,                      /**< Wait for the _card_info result. */
    CARDA_STEP_CLEAR_SOFTWARE_EVENTS = 3,               /**< Clear the software card events. */
    CARDA_STEP_POLL_HARDWARE_EVENTS = 4,                /**< Wait for and check the hardware card events. */
    CARDA_STEP_CLEAR_HARDWARE_EVENTS = 5,               /**< Clear the hardware card events. */
    CARDA_STEP_SCAN_ENTRIES = 6,                        /**< Erase the placeholder files and scan the card directory. */
    CARDA_STEP_SCAN_DONE = 7,                           /**< No case: the sequence waits here after the scan. */
    CARDA_STEP_CARD_CLEAR = 8,                          /**< Issue _card_clear on the current slot. */
    CARDA_STEP_CARD_LOAD = 9,                           /**< Issue _card_load and arm the poll countdowns. */
    CARDA_STEP_SKIP = 10,                               /**< Advance without doing anything; entry point of the write tables. */
    CARDA_STEP_CREATE_TEMP_SAVE = 11,                   /**< Create the two-block save under the placeholder name. */
    CARDA_STEP_WRITE_TEMP_SAVE = 12,                    /**< Start writing the save file into the placeholder. */
    CARDA_STEP_POLL_TEMP_SAVE_WRITE = 13,               /**< Wait for the write, rename the file and patch its title. */
    CARDA_STEP_IDLE = 14,                               /**< No case: the sequence waits here until other code replaces it. */
    CARDA_STEP_POLL_CARD_LOAD = 15,                     /**< Wait for the _card_clear/_card_load result, retrying. */
    CARDA_STEP_CARD_WAIT = 16,                          /**< Wait for the pending card command to finish. */
    CARDA_STEP_READ_HEADER = 17,                        /**< Open the selected save and start reading its header. */
    CARDA_STEP_POLL_HEADER_READ = 18,                   /**< Wait for the header read to finish. */
    CARDA_STEP_READ_SAVE = 19,                          /**< Open the selected save and start reading it. */
    CARDA_STEP_POLL_SAVE_READ = 20,                     /**< Wait for the save read to finish. */
    CARDA_STEP_CREATE_POCKETSTATION_SAVE = 21,          /**< Create the six-block Ring Ring Land file. */
    CARDA_STEP_WRITE_POCKETSTATION_SAVE = 22,           /**< Start writing Ring Ring Land. */
    CARDA_STEP_POLL_POCKETSTATION_SAVE_WRITE = 23,      /**< Wait for the Ring Ring Land write and finish the file. */
    CARDA_STEP_CHECK_POCKETSTATION = 24,                /**< Check that the card is a PocketStation (McxCardType). */
    CARDA_STEP_POLL_CARD_PRESENT = 25,                  /**< Wait for the _card_info result; report a failure. */
    CARDA_STEP_WRITE_TEMP_POCKETSTATION_SAVE = 26,      /**< Create the Ring Ring Land placeholder file and start writing it. */
    CARDA_STEP_POLL_TEMP_POCKETSTATION_SAVE_WRITE = 27, /**< Wait for the write and rename it over the selected file. */
    CARDA_STEP_READ_SAVE_PREFIX = 28,                   /**< Open the selected file and start reading its first 1 KiB. */
    CARDA_STEP_POLL_SAVE_PREFIX_READ = 29,              /**< Wait for the 1 KiB read to finish. */
    CARDA_STEP_ARM_RETRIES = 30                         /**< Arm the file operation retry counter. */
} CardaCardStep;

/** @brief Results of carda_advance_card_sequence. */
typedef enum CardaSequenceResult
{
    CARDA_SEQUENCE_NONE = 0,       /**< Never returned. */
    CARDA_SEQUENCE_WAIT = 1,       /**< Step handled; poll again next frame. */
    CARDA_SEQUENCE_FINISHED = 2,   /**< The step table ended. */
    CARDA_SEQUENCE_RUN_AGAIN = 3,  /**< A card command was issued; run the next step now. */
    CARDA_SEQUENCE_NO_CARD = 4,    /**< The card stopped answering; the entry state says so. */
    CARDA_SEQUENCE_UNFORMATTED = 5 /**< _card_load kept reporting a new card: the card is not formatted. */
} CardaSequenceResult;

/** @brief CARDA's own carda_open_status_dialog messages, after the shared CARD_MENU_DIALOG_* ones. */
#define CARDA_DIALOG_SAVE_CORRUPT 4
#define CARDA_DIALOG_FORMAT_FAILED 5

/** @brief CARDA's own g_carda_selection_status value, after the shared CARD_MENU_SELECTION_* ones. */
#define CARDA_SELECTION_CARD_FULL 4  /**< The full-card placeholder is selected. */

/** @brief Length of the full-card placeholder entry name ("Fulldummy"). */
#define CARDA_CARD_FULL_ENTRY_NAME_LENGTH 9

/** @brief Left edge and width of the entry-list window of the browser layout (save and load modes). */
#define CARDA_LIST_X 10
#define CARDA_LIST_WIDTH 300

/** @brief X of the scroll arrows, inset from the entry list's right edge. */
#define CARDA_SCROLL_ARROW_X (CARDA_LIST_X + CARDA_LIST_WIDTH - 8)

/** @brief Top of the message window (prompts and progress messages). */
#define CARDA_MESSAGE_Y 90

/** @brief PocketStation transfer window, also used for the format prompt. */
#define CARDA_TRANSFER_Y 76
#define CARDA_TRANSFER_HEIGHT 72 /**< Five lines. */

/** @brief Item list window (received items) and its arrows. */
#define CARDA_ITEM_LIST_X 32
#define CARDA_ITEM_LIST_Y 54
#define CARDA_ITEM_LIST_WIDTH 256
#define CARDA_ITEM_LIST_HEIGHT 144
#define CARDA_ITEM_LIST_VISIBLE_ROWS (CARDA_ITEM_LIST_HEIGHT / CARD_MENU_LINE_HEIGHT)
#define CARDA_ITEM_SCROLL_ARROW_X (CARDA_ITEM_LIST_X + CARDA_ITEM_LIST_WIDTH - 8)
#define CARDA_ITEM_SCROLL_ARROW_UP_Y (CARDA_ITEM_LIST_Y + 8)
#define CARDA_ITEM_SCROLL_ARROW_DOWN_Y (CARDA_ITEM_LIST_Y + CARDA_ITEM_LIST_HEIGHT - 8)

/** @brief Six-byte memory-card path buffer ("bu00:" plus terminator), byte aligned. */
typedef struct
{
    u8 raw[6];
} CardaFileHeaderScratch;

/** @brief Highest count an item stack can reach. */
#define CARDA_ITEM_COUNT_MAX 99

/* FIELD / main-executable globals used by this overlay. */
extern s32 g_save_compatibility_tag;
extern s32 g_playtime_vsync_origin;
extern s32 g_field_card_pet_slot;
extern s32 g_pad_input;
extern s32 g_gosub_result_values;
extern s32 g_field_card_overlay_mode;

/* FIELD UI strings and memory-card file names. */
extern u8 g_field_ui_text_cant_hold_more[];
extern u8 g_text_time_separator_offset_bytes[2];
extern u8 g_text_choice_glyph_offsets;
extern char g_lom_pocketstation_filename_prefix[];
extern char g_lom_save_dummy_filename[];
extern char g_lom_pocketstation_dummy_filename[];
extern char g_new_save_entry_prefix[];
extern char g_card_full_entry_name[];

/* CARDA read-only data. */
extern const CardaFileHeaderScratch g_carda_save_card_path_prefix;

/* CARDA text offset-table entries. */
extern u16 g_carda_text_checking_card;
extern u16 g_carda_text_not_enough_blocks;
extern u16 g_carda_text_no_card;
extern u16 g_carda_text_mana_label;
extern u16 g_carda_text_other_game_label;
extern u16 g_carda_text_save_title;
extern u16 g_carda_text_card_slot0_label;
extern u16 g_carda_text_card_slot1_label;
extern u16 g_carda_text_card_access_failed;
extern u16 g_carda_text_no_save_data;
extern u16 g_carda_text_new_save_label;
extern u16 g_carda_text_save_prompt;
extern u16 g_carda_text_overwrite_prompt;
extern u16 g_carda_text_saving;
extern u16 g_carda_text_saved;
extern u16 g_carda_text_format_prompt;
extern u16 g_carda_text_new_save_title;
extern u16 g_carda_text_load_title;
extern u16 g_carda_text_number_label;
extern u16 g_carda_text_load_prompt;
extern u16 g_carda_text_loading;
extern u16 g_carda_text_no_lom_save_data;
extern u16 g_carda_text_ring_ring_land_label;
extern u16 g_carda_text_save_failed;
extern u16 g_carda_text_load_failed;
extern u16 g_carda_text_card_not_inserted;
extern u16 g_carda_text_not_pocketstation;
extern u16 g_carda_text_wrong_version;
extern u16 g_carda_text_formatting;
extern u16 g_carda_text_needs_two_blocks;
extern u16 g_carda_text_save_corrupt;
extern u16 g_carda_text_format_failed;
extern u16 g_carda_text_card_full_label;
extern u16 g_carda_text_checking_pocketstation;
extern u16 g_carda_text_no_pocketstation;
extern u16 g_carda_text_pocketstation_access_failed;
extern u16 g_carda_text_pocketstation_is;
extern u16 g_carda_text_pocketstation_not_inserted;
extern u16 g_carda_text_ring_ring_land_was;
extern u16 g_carda_text_received_items;
extern u16 g_carda_text_return_pet;
extern u16 g_carda_text_swap_pets;
extern u16 g_carda_text_game_from_pocketstation;
extern u16 g_carda_text_game_to_pocketstation;
extern u16 g_carda_text_select_slot;
extern u16 g_carda_text_ring_ring_land_title;
extern u16 g_carda_text_download_failed;
extern u16 g_carda_text_upload_failed;
extern u16 g_carda_text_downloading;
extern u16 g_carda_text_pet_already_on_ranch;
extern u16 g_carda_text_plus_marker;
extern u16 g_carda_text_card_unformatted;
#if defined(VERSION_JP)
extern u16 g_carda_text_memory_card_is;
#endif
extern u16 g_carda_item_names[];
extern u8 g_carda_save_title_template[];
extern u8 g_carda_bad_title_template[];
extern s32 g_carda_save_icon_offsets[];
extern u16 g_carda_location_names[];
extern s32 g_carda_icon_image_offsets[];

/* Card-sequence step scripts (g_card_step points into these). */
extern u8 g_carda_steps_initial_scan[];
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

/* CARDA state. */
extern s32 g_carda_scroll_target_y;
extern s32 g_carda_new_save_file;
extern s32 g_carda_growth_delta;
extern u8 g_carda_received_item_ids[];
extern s32 g_carda_pet_already_on_ranch;
extern CardMenuElement g_carda_element_pool[8]; /**< UI element pool. */
extern CardMenuElement g_carda_element1_state;
extern s32 g_carda_exit_requested;
extern s32 g_carda_dialog_state;
extern s32 g_carda_received_item_count;
extern u8* g_carda_save_blob;
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
extern u_long g_carda_icon_context[];
extern s32 g_carda_format_declined;
extern s32 g_carda_selection_status;
/** @brief The saved game's item records (g_saved_game_ctx->items). */
extern FieldItemRecord* g_carda_items;
extern s32 g_carda_scroll_y;
extern s32 g_carda_save_in_progress;
/**
 * @brief Start of the selected entry's save file: only the card header and the
 *        first 0x100 bytes of the saved game are read (CARD_MENU_ENTRY_READ_BYTES).
 */
extern SaveFile g_carda_selected_file;
extern s32 g_carda_file_handle;
extern s32 g_carda_entry_ranks[];
extern CardaFileHeaderScratch g_carda_selected_card_path;
extern s32 g_carda_rank_count;
extern s32 g_carda_retry_count;
extern s32 g_carda_selected_entry_extended;
extern s32 g_carda_primary_poll_countdown;
extern s32 g_carda_next_save_serial;
extern s32 g_carda_progress_bar_active;
extern s32 g_carda_entry_scan_active;
extern s32 g_carda_preserve_old_save;
extern s32 g_carda_progress_start_tick;
extern s32 g_carda_secondary_poll_countdown;
extern u8 g_carda_temp_card_path[];

/* FIELD entry points and library calls used by CARDA. */
void field_reset_input_repeat(void);
s32 OpenEvent(s32, s32, s32, s32);
void CloseEvent(s32);
s32 TestEvent(s32);
void EnableEvent(s32);
void EnterCriticalSection(void);
void ExitCriticalSection(void);
s32 open(void*, s32);
s32 read(s32, void*, s32);
s32 write(s32, void*, s32);
s32 close(s32);
s32 nextfile();
s32 rename(void*, void*);
s32 erase(void*);
u8* Krom2RawAdd(u16 sjis_code);
s32 firstfile();
s32 rand();
s32 _card_info(s32);
s32 _card_load(s32);
s32 _card_write(s32, s32, void*);
s32 _card_read(s32, s32, void*);
s32 _card_wait(s32);
s32 _card_clear(s32);
s32 _card_format();
s32 func_80033E7C(s32);
s32 func_80034648(s32, s32, s32);
void field_restore_fade_target(void);
void field_set_default_fade_target(void);
void field_restore_fade_target_with_duration();
void field_copy_golem_portrait_palette(void* buf, s32 arg1);
void field_copy_portrait_palette(void* buf, s32 arg1);
void field_flag_known_save();
void field_apply_region_level_ups(s32 slot);

#endif /* CARDA_INTERNAL_H */
