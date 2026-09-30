#ifndef CARDA_INTERNAL_H
#define CARDA_INTERNAL_H

#include "field_text.h"
#include "common.h"
#include "saved_game.h"
#include "pad.h"
#include "vector.h"
#include "gpu_packet.h"
#include "display.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"
#include "sdk/libmcx.h"
#include "field_menu_window.h"
#include "field_runtime.h"
#include "field_sound.h"
#include "field_ui_text.h"
#include "encoded_text.h"
#include "save_file.h"
#include "glyph_cache.h"
#include "card_events.h"
#include "card_callbacks.h"
#include "card_directory.h"
#include "cdrom.h"
#include "controller.h"
#include "sdk/strings.h"
#include "sdk/libetc.h"
#include "carda.h"
#include "carda_widgets.h"
#include "carda_save.h"
#include "carda_card.h"

/**
 * @brief Draw callback of a CARDA UI element: emits the element's content at
 *        the given transition offsets and returns the advanced primitive cursor.
 */
typedef void* (*CardaElementDrawFunc)(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
typedef PS1_STORED(CardaElementDrawFunc) CardaElementDrawPtr;

/**
 * @brief One animated CARDA UI element (a framed window plus its content).
 *
 * Eight of these form the element pool starting at g_carda_element_pool.  The nine-bit
 * window width straddles the two state words: its low eight bits are the top
 * byte of attr and its high bit is size.bits.width_high.  No bitfield can span
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
            u32 unk9 : 23;
        } bits;
    } size;
    CardaElementDrawPtr draw;
} CardaElement;

/** @brief CardaElement.attr.bits.state values. */
#define CARDA_ELEMENT_FREE 0
#define CARDA_ELEMENT_OPENING 1
#define CARDA_ELEMENT_OPEN 2
#define CARDA_ELEMENT_CLOSING 3
#define CARDA_ELEMENT_CLOSED 4

/** @brief Number of elements in the CARDA UI element pool. */
#define CARDA_ELEMENT_COUNT 8

/** @brief Frames an element takes to open or close; its window scales by transition_step / this. */
#define CARDA_ELEMENT_TRANSITION_STEPS 8

/** @brief Frames a closed element stays in CARDA_ELEMENT_CLOSED before it is freed. */
#define CARDA_ELEMENT_CLOSED_FRAMES 3

/** @brief Pool slot of the modal window (prompts, dialogs, progress), drawn with the bright frame; the builders hold it while allocating. */
#define CARDA_ELEMENT_MODAL 0

/** @brief Pool slot of the first allocated window: the entry list, the PocketStation transfer window or the item list. */
#define CARDA_ELEMENT_MAIN 1

/** @brief Bit position of the width's low byte inside CardaElement.attr.word. */
#define CARDA_ELEMENT_WIDTH_SHIFT 24

/** @brief Low eight bits of a CardaElement's window width. */
#define CARDA_ELEMENT_WIDTH_LOW(element) ((element)->attr.word >> CARDA_ELEMENT_WIDTH_SHIFT)

/**
 * @brief Full nine-bit window width of a CardaElement.
 * @param element Element whose width is read.
 * @param width_low The width's low byte, as read by CARDA_ELEMENT_WIDTH_LOW.
 */
#define CARDA_ELEMENT_WIDTH(element, width_low) ((s32)(((element)->size.bits.width_high << 8) | (width_low)))

/** @brief Store the low eight bits of a CardaElement's window width. */
#define CARDA_SET_ELEMENT_WIDTH_LOW(element, width)                                                                                                            \
    ((element)->attr.word = ((element)->attr.word & ((1 << CARDA_ELEMENT_WIDTH_SHIFT) - 1)) | ((u32)(width) << CARDA_ELEMENT_WIDTH_SHIFT))

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
 * Below CARDA_ENTRY_COUNT_LIMIT the value is the number of directory entries
 * read from the current card. From 0xE9 up it is a status whose message the
 * entry list or the PocketStation window shows instead of the entries; the
 * values below 0xF6 are only used by the PocketStation modes.
 */
#define CARDA_ENTRY_STATE_PET_ALREADY_ON_RANCH 0xE9       /**< The pet on the PocketStation is already on the ranch. */
#define CARDA_ENTRY_STATE_CONFIRM_RETURN 0xEA             /**< Asks whether to return the pet and erase Ring Ring Land. */
#define CARDA_ENTRY_STATE_FORMAT_FAILED 0xEB              /**< Status dialog CARDA_DIALOG_FORMAT_FAILED. */
#define CARDA_ENTRY_STATE_SAVE_CORRUPT 0xEC               /**< Status dialog CARDA_DIALOG_SAVE_CORRUPT. */
#define CARDA_ENTRY_STATE_NO_POCKETSTATION 0xED           /**< Status dialog CARDA_DIALOG_NOT_POCKETSTATION. */
#define CARDA_ENTRY_STATE_POCKETSTATION_NOT_INSERTED 0xEE /**< Status dialog CARDA_DIALOG_CARD_NOT_INSERTED. */
#define CARDA_ENTRY_STATE_UPLOAD_FAILED 0xEF              /**< Status dialog CARDA_DIALOG_LOAD_FAILED. */
#define CARDA_ENTRY_STATE_DOWNLOAD_FAILED 0xF0            /**< Status dialog CARDA_DIALOG_SAVE_FAILED. */
#define CARDA_ENTRY_STATE_SELECT_SLOT 0xF1                /**< Only the slot prompt is shown. */
#define CARDA_ENTRY_STATE_CONFIRM_DOWNLOAD 0xF2           /**< Asks whether to download Ring Ring Land. */
#define CARDA_ENTRY_STATE_DOWNLOADING 0xF3                /**< Writing Ring Ring Land; progress bar. */
#define CARDA_ENTRY_STATE_UPLOADING 0xF4                  /**< Reading Ring Ring Land back; progress bar. */
#define CARDA_ENTRY_STATE_NO_RING_RING_LAND 0xF5          /**< Ring Ring Land was not found; CARDA never sets it. */
#define CARDA_ENTRY_STATE_NOT_POCKETSTATION 0xF6          /**< The card is not a PocketStation. */
#define CARDA_ENTRY_STATE_NO_ROOM_FOR_DOWNLOAD 0xF7       /**< Not enough free blocks for Ring Ring Land. */
#define CARDA_ENTRY_STATE_NO_GAME_DATA 0xF8               /**< The card holds no Legend of Mana save data. */
#define CARDA_ENTRY_STATE_UNFORMATTED 0xF9                /**< The card is not formatted. */
#define CARDA_ENTRY_STATE_CARD_FULL 0xFA                  /**< Not enough free blocks for a new save. */
#define CARDA_ENTRY_STATE_ACCESS_FAILED 0xFB              /**< The card could not be accessed. */
#define CARDA_ENTRY_STATE_NO_SAVE_DATA 0xFC               /**< The card holds no save data. */
#define CARDA_ENTRY_STATE_NO_CARD 0xFD                    /**< No card answers. */
#define CARDA_ENTRY_STATE_BLANK 0xFE                      /**< Shows nothing; CARDA never sets it. */
#define CARDA_ENTRY_STATE_CHECKING_CARD 0xFF              /**< The card is being checked; no entries yet. */

/** @brief Entry-state values below this are entry counts the list draws. */
#define CARDA_ENTRY_COUNT_LIMIT 0x10

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

/** @brief carda_open_status_dialog messages (g_carda_dialog_state). */
#define CARDA_DIALOG_SAVE_FAILED 0
#define CARDA_DIALOG_LOAD_FAILED 1
#define CARDA_DIALOG_CARD_NOT_INSERTED 2
#define CARDA_DIALOG_NOT_POCKETSTATION 3
#define CARDA_DIALOG_SAVE_CORRUPT 4
#define CARDA_DIALOG_FORMAT_FAILED 5

/** @brief g_carda_selection_status values: what the details window shows. */
#define CARDA_SELECTION_NONE 0       /**< Nothing to show yet. */
#define CARDA_SELECTION_ENTRY_READ 1 /**< The selected entry's header has been read. */
#define CARDA_SELECTION_NEW_SAVE 2   /**< The new-save placeholder is selected. */
#define CARDA_SELECTION_EMPTY_CARD 3 /**< The card has no entries. */
#define CARDA_SELECTION_CARD_FULL 4  /**< The full-card placeholder is selected. */

/** @brief Buttons that confirm a choice. */
#define CARDA_CONFIRM_BUTTON_MASK (PAD_BTN_CROSS | PAD_BTN_L3)

/** @brief Buttons that switch to the other card slot. */
#define CARDA_CARD_SWITCH_BUTTON_MASK (PAD_BTN_SELECT | PAD_BTN_RIGHT | PAD_BTN_LEFT)

/** @brief Buttons that move a yes/no choice. */
#define CARDA_CHOICE_BUTTON_MASK (PAD_BTN_RIGHT | PAD_BTN_LEFT)

/** @brief g_carda_choice_toggle values: the selected choice of a yes/no prompt. */
#define CARDA_CHOICE_YES 0
#define CARDA_CHOICE_NO 1

/**
 * @brief Choice a yes/no prompt starts on.
 * @note JP starts on yes, US on no.
 */
#if defined(VERSION_JP)
#define CARDA_CHOICE_DEFAULT CARDA_CHOICE_YES
#else
#define CARDA_CHOICE_DEFAULT CARDA_CHOICE_NO
#endif

/** @brief Length of the new-save placeholder entry name ("AKIdummy"). */
#define CARDA_NEW_SAVE_ENTRY_NAME_LENGTH 8

/** @brief Length of the full-card placeholder entry name ("Fulldummy"). */
#define CARDA_CARD_FULL_ENTRY_NAME_LENGTH 9

/** @brief Height of one entry-list row and of one message line, in pixels. */
#define CARDA_TEXT_LINE_HEIGHT 14

/** @brief Entry-list window of the browser layout (save and load modes). */
#define CARDA_LIST_X 10
#define CARDA_LIST_Y 50
#define CARDA_LIST_WIDTH 300
#define CARDA_LIST_HEIGHT 88

/** @brief Rows that fit in the entry list, and the top of the last one. */
#define CARDA_LIST_VISIBLE_ROWS (CARDA_LIST_HEIGHT / CARDA_ENTRY_ROW_HEIGHT)
#define CARDA_LIST_LAST_ROW_Y ((CARDA_LIST_VISIBLE_ROWS - 1) * CARDA_ENTRY_ROW_HEIGHT)

/** @brief Frames a list scroll takes to reach its target. */
#define CARDA_SCROLL_FRAMES 4

/** @brief Scroll arrows, inset from the entry list's right edge, top and bottom. */
#define CARDA_SCROLL_ARROW_X (CARDA_LIST_X + CARDA_LIST_WIDTH - 8)
#define CARDA_SCROLL_ARROW_UP_Y (CARDA_LIST_Y + 8)
#define CARDA_SCROLL_ARROW_DOWN_Y (CARDA_LIST_Y + CARDA_LIST_HEIGHT - 8)

/** @brief Message window: prompts and progress messages. */
#define CARDA_MESSAGE_X 16
#define CARDA_MESSAGE_Y 90
#define CARDA_MESSAGE_WIDTH 288
#define CARDA_MESSAGE_HEIGHT 44 /**< Three lines. */

/** @brief PocketStation transfer window, also used for the format prompt. */
#define CARDA_TRANSFER_Y 76
#define CARDA_TRANSFER_HEIGHT 72 /**< Five lines. */

/** @brief Item list window (received items) and its arrows. */
#define CARDA_ITEM_LIST_X 32
#define CARDA_ITEM_LIST_Y 54
#define CARDA_ITEM_LIST_WIDTH 256
#define CARDA_ITEM_LIST_HEIGHT 144
#define CARDA_ITEM_LIST_VISIBLE_ROWS (CARDA_ITEM_LIST_HEIGHT / CARDA_TEXT_LINE_HEIGHT)
#define CARDA_ITEM_SCROLL_ARROW_X (CARDA_ITEM_LIST_X + CARDA_ITEM_LIST_WIDTH - 8)
#define CARDA_ITEM_SCROLL_ARROW_UP_Y (CARDA_ITEM_LIST_Y + 8)
#define CARDA_ITEM_SCROLL_ARROW_DOWN_Y (CARDA_ITEM_LIST_Y + CARDA_ITEM_LIST_HEIGHT - 8)

/**
 * @brief CARDA text table indexes.
 *
 * CARDA_TEXT_AT needs the index of the entry symbol it is given. The US
 * release leaves the PocketStation texts empty (29, 33, 47-49 and 58-87).
 * The order of the card-access message lines differs by version: US line 2 is
 * CARDA_TEXT_DO_NOT_REMOVE_CARD and line 3 CARDA_TEXT_CARD_OR_CONTROLLER,
 * JP the other way round.
 */
#define CARDA_TEXT_CHECKING_CARD 0
#define CARDA_TEXT_NOT_ENOUGH_BLOCKS 1
#define CARDA_TEXT_NO_CARD 2
#define CARDA_TEXT_MANA_LABEL 3
#define CARDA_TEXT_OTHER_GAME_LABEL 4
#define CARDA_TEXT_SAVE_TITLE 5
#define CARDA_TEXT_CARD_SLOT0_LABEL 6
#define CARDA_TEXT_CARD_SLOT1_LABEL 7
#define CARDA_TEXT_CARD_ACCESS_FAILED 8
#define CARDA_TEXT_NO_SAVE_DATA 9
#define CARDA_TEXT_NEW_SAVE_LABEL 10
#define CARDA_TEXT_SAVE_PROMPT 11
#define CARDA_TEXT_OVERWRITE_PROMPT 12
#define CARDA_TEXT_SAVING 14
#define CARDA_TEXT_DO_NOT_REMOVE_CARD 15
#define CARDA_TEXT_SAVED 16
#define CARDA_TEXT_MEMORY_CARD_IS 17 /**< JP only: subject line "The memory card is". */
#define CARDA_TEXT_NOT_FORMATTED 18 /**< Second line after a subject such as CARDA_TEXT_POCKETSTATION_IS. */
#define CARDA_TEXT_FORMAT_PROMPT 19
#define CARDA_TEXT_NEW_SAVE_TITLE 20
#define CARDA_TEXT_USES_TWO_BLOCKS 21
#define CARDA_TEXT_LOAD_TITLE 22
#define CARDA_TEXT_NUMBER_LABEL 23
#define CARDA_TEXT_LOAD_PROMPT 24
#define CARDA_TEXT_LOADING 25
#define CARDA_TEXT_NO_GAME_SAVE_DATA 26
#define CARDA_TEXT_NEWEST 27
#define CARDA_TEXT_OLDEST 28
#define CARDA_TEXT_RING_RING_LAND_LABEL 29
#define CARDA_TEXT_SAVE_FAILED 30
#define CARDA_TEXT_LOAD_FAILED 31
#define CARDA_TEXT_CARD_NOT_INSERTED 32
#define CARDA_TEXT_NOT_POCKETSTATION 33
#define CARDA_TEXT_WRONG_VERSION 42
#define CARDA_TEXT_CHECK_CARD_INSERTED 43
#define CARDA_TEXT_FORMATTING 44
#define CARDA_TEXT_NEEDS_TWO_BLOCKS 45
#define CARDA_TEXT_SAVE_CORRUPT 46
#define CARDA_TEXT_DOWNLOAD_RING_RING_LAND 48
#define CARDA_TEXT_NEEDS_SIX_BLOCKS 49
#define CARDA_TEXT_FORMAT_FAILED 50
#define CARDA_TEXT_CARD_FULL_LABEL 51
#define CARDA_TEXT_CHECKING_POCKETSTATION 58
#define CARDA_TEXT_NO_POCKETSTATION 59
#define CARDA_TEXT_POCKETSTATION_ACCESS_FAILED 60
#define CARDA_TEXT_POCKETSTATION_OR_CONTROLLER 61
#define CARDA_TEXT_POCKETSTATION_IS 62
#define CARDA_TEXT_POCKETSTATION_NOT_INSERTED 63
#define CARDA_TEXT_RING_RING_LAND_WAS 65
#define CARDA_TEXT_NOT_FOUND 66
#define CARDA_TEXT_RECEIVED_ITEMS 67
#define CARDA_TEXT_RETURN_PET 69
#define CARDA_TEXT_SWAP_PETS 71
#define CARDA_TEXT_GAME_FROM_POCKETSTATION 73
#define CARDA_TEXT_GAME_TO_POCKETSTATION 74
#define CARDA_TEXT_DOWNLOAD_OK 75
#define CARDA_TEXT_OVERWRITE_OK 76
#define CARDA_TEXT_SELECT_SLOT 77
#define CARDA_TEXT_RING_RING_LAND_TITLE 78
#define CARDA_TEXT_DOWNLOAD_FAILED 79
#define CARDA_TEXT_UPLOAD_FAILED 80
#define CARDA_TEXT_DOWNLOADING 81
#define CARDA_TEXT_UPLOADING 83
#define CARDA_TEXT_RING_RING_LAND_SIX_BLOCKS 85
#define CARDA_TEXT_WILL_BE_ERASED 86
#define CARDA_TEXT_PET_ALREADY_ON_RANCH 87
#define CARDA_TEXT_PLUS_MARKER 88
#define CARDA_TEXT_CARD_OR_CONTROLLER 89

/**
 * @brief Address of CARDA text @p index, reached through its own u16 offset-table entry @p entry.
 * @note The table start is derived back from the entry symbol, like FIELD_UI_TEXT_AT.
 */
#define CARDA_TEXT_AT(entry, index) ((u8*)&(entry) - (index) * 2 + (entry))

/**
 * @brief Text table index of the entry g_carda_text_card_unformatted names.
 * @note JP reorders the text offset table; this entry is index 18 there.
 */
#if defined(VERSION_JP)
#define CARDA_TEXT_CARD_UNFORMATTED 18
#else
#define CARDA_TEXT_CARD_UNFORMATTED 90
#endif

/** @brief Start of the CARDA text offset table, derived from entry @p entry at @p index. */
#define CARDA_TEXT_TABLE(entry, index) (&(entry) - (index))

/** @brief Address of CARDA text @p index in the u16 offset table starting at @p table. */
#define CARDA_TEXT(table, index) ((u8*)(table) + (table)[index])

/** @brief Height in pixels of one row of the save-file list. */
#define CARDA_ENTRY_ROW_HEIGHT 14

/** @brief Number of suffix groups the directory sort buckets saves into. */
#define CARDA_ENTRY_GROUP_COUNT 8

/** @brief Byte size of one memory-card block. */
#define CARDA_MEMORY_CARD_BLOCK_BYTES 8192

/** @brief Six-byte memory-card path buffer ("bu00:" plus terminator), byte aligned. */
typedef struct
{
    u8 raw[6];
} CardaFileHeaderScratch;

/** @brief Bytes read to show an entry: the card header and the first 0x100 bytes of the saved game. */
#define CARDA_ENTRY_READ_BYTES 0x280

/** @brief Bytes read to show an entry that is not a Legend of Mana save: its card header title and CLUT. */
#define CARDA_ENTRY_TITLE_READ_BYTES 0x80

/** @brief Highest count an item stack can reach. */
#define CARDA_ITEM_COUNT_MAX 99

/* FIELD / main-executable globals used by this overlay. */
extern s32 g_save_compatibility_tag;
extern s32 g_playtime_vsync_origin;
extern s32 g_field_card_pet_slot;
extern s32 g_pad_input;
extern s32 g_gosub_result_values;
extern s32 g_field_card_overlay_mode;
extern s32 g_menu_element_counter;

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
extern CardaElement g_carda_element_pool[8]; /**< UI element pool. */
extern CardaElement g_carda_element1_state;
extern s32 g_carda_exit_requested;
extern s32 g_carda_dialog_state;
extern s32 g_carda_received_item_count;
extern u8_ptr g_carda_save_blob;
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
extern FieldItemRecordPtr g_carda_items;
extern s32 g_carda_scroll_y;
extern s32 g_carda_save_in_progress;
/**
 * @brief Start of the selected entry's save file: only the card header and the
 *        first 0x100 bytes of the saved game are read (CARDA_ENTRY_READ_BYTES).
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
s32 field_set_fade_target();
void field_restore_fade_target(void);
void field_set_default_fade_target(void);
void field_restore_fade_target_with_duration();
void field_copy_golem_portrait_palette(void* buf, s32 arg1);
void field_copy_portrait_palette(void* buf, s32 arg1);
void* field_draw_text(void* prim, u_long* ot, u8* text, s32 color, s32 x, s32 y, s32 mode);
void* field_draw_number(u_long* ot, void* prim, s32 value, s32 color, DVECTOR* pos, s32 mode);
void field_flag_known_save();
void field_apply_region_level_ups(s32 slot);

#endif /* CARDA_INTERNAL_H */
