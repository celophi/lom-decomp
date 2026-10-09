#ifndef CARD_MENU_H
#define CARD_MENU_H

#include "common.h"
#include "overlays/field/field_portrait.h"
#include "common/pad.h"
#include "main/display.h"
#include "common/gpu_packet.h"
#include "main/field_runtime.h"

/**
 * @file card_menu.h
 * @brief Memory-card menu screens compiled into ADDHERO, CARDA, CLOAD and NIKI.
 *
 * Each of these overlays links its own copy of the same menu code and data:
 * the pool of animated windows, the save browser (entry list, details window,
 * card-slot labels), the message and dialog windows, the yes/no prompt, the
 * transfer progress bar, the party icons and the text table. This header holds
 * what those copies share: layout, window and entry states, text indexes and
 * the card I/O limits. Overlay headers add only what is their own.
 */

/** @brief Pad buttons that confirm a prompt or a selection. */
#define CARD_MENU_CONFIRM_BUTTON_MASK (PAD_BTN_CROSS | PAD_BTN_L3)

/** @brief Pad buttons that switch the browser to the other card slot. */
#define CARD_MENU_CARD_SWITCH_BUTTON_MASK (PAD_BTN_SELECT | PAD_BTN_RIGHT | PAD_BTN_LEFT)

/**
 * @brief g_card_entry_state values shared by every card menu.
 *
 * Below CARD_MENU_ENTRY_COUNT_LIMIT the value is the number of entries read
 * from the current card. The states from 0xF8 up mean the same in every
 * overlay; each overlay defines its own states below them.
 */
#define CARD_MENU_ENTRY_COUNT_LIMIT 0x10
#define CARD_MENU_ENTRY_STATE_NO_GAME_DATA 0xF8 /**< The card holds no Legend of Mana save data. */
#define CARD_MENU_ENTRY_STATE_UNFORMATTED 0xF9  /**< The card is not formatted; the load-only menus show the no-game-data message. */
#define CARD_MENU_ENTRY_STATE_CARD_FULL 0xFA    /**< Not enough free blocks for a new save. */
#define CARD_MENU_ENTRY_STATE_ACCESS_FAILED 0xFB /**< The card could not be accessed. */
#define CARD_MENU_ENTRY_STATE_NO_SAVE_DATA 0xFC /**< The card holds no save data. */
#define CARD_MENU_ENTRY_STATE_NO_CARD 0xFD      /**< No card answers: an event error or the retries ran out. */
#define CARD_MENU_ENTRY_STATE_BLANK 0xFE        /**< Shows nothing; no menu sets it. */
#define CARD_MENU_ENTRY_STATE_CHECKING_CARD 0xFF /**< The card is being checked; no entries yet. */

/**
 * @brief Opcodes of the card load/save step tables every card menu runs one
 *        byte at a time (g_card_step). Opcodes without a handler make the
 *        sequence wait on them; each overlay adds its own opcodes in the gaps.
 */
typedef enum
{
    CARD_MENU_STEP_DONE = 0,                  /**< End of a step table; report CARD_MENU_SEQUENCE_FINISHED. */
    CARD_MENU_STEP_CARD_INFO = 1,             /**< Issue _card_info on the current slot. */
    CARD_MENU_STEP_POLL_CARD_INFO = 2,        /**< Wait for the _card_info result. */
    CARD_MENU_STEP_CLEAR_SOFTWARE_EVENTS = 3, /**< Clear the software card events. */
    CARD_MENU_STEP_POLL_HARDWARE_EVENTS = 4,  /**< Wait for and check the hardware card events. */
    CARD_MENU_STEP_CLEAR_HARDWARE_EVENTS = 5, /**< Clear the hardware card events. */
    CARD_MENU_STEP_SCAN_ENTRIES = 6,          /**< Erase the placeholder files and scan the card directory. */
    CARD_MENU_STEP_SCAN_DONE = 7,             /**< No handler: the sequence waits here after the scan. */
    CARD_MENU_STEP_CLEAR_CARD = 8,            /**< Issue _card_clear on the current slot. */
    CARD_MENU_STEP_LOAD_CARD = 9,             /**< Issue _card_load and arm the poll countdowns. */
    CARD_MENU_STEP_WAIT = 14,                 /**< No handler: the sequence waits here until other code replaces it. */
    CARD_MENU_STEP_POLL_CARD_LOAD = 15,       /**< Wait for the _card_clear/_card_load result, retrying. */
    CARD_MENU_STEP_WAIT_HARDWARE_EVENTS = 16, /**< Wait for any hardware card event. */
    CARD_MENU_STEP_READ_ENTRY = 17,           /**< Open the selected save and start reading its header. */
    CARD_MENU_STEP_POLL_ENTRY_READ = 18,      /**< Wait for the header read to finish. */
    CARD_MENU_STEP_READ_SAVE = 19,            /**< Open the selected save and start reading it. */
    CARD_MENU_STEP_POLL_SAVE_READ = 20,       /**< Wait for the save read to finish, retrying. */
    CARD_MENU_STEP_CHECK_POCKETSTATION = 24,  /**< Check that the card is a PocketStation (McxCardType). */
    CARD_MENU_STEP_INIT_RETRIES = 30          /**< Arm the read/write retry counter. */
} CardMenuStep;

/** @brief Step opcodes of the screens that write another player's save back (ADDHERO and NIKI). */
typedef enum
{
    CARD_MENU_EXCHANGE_STEP_ERASE_ENTRY = 10,        /**< Erase the selected directory entry. */
    CARD_MENU_EXCHANGE_STEP_WRITE_SAVE = 25,         /**< Create the placeholder file and start writing the save. */
    CARD_MENU_EXCHANGE_STEP_POLL_SAVE_WRITE = 26,    /**< Wait for the write and rename it over the selected save. */
    CARD_MENU_EXCHANGE_STEP_READ_BEFORE_WRITE = 27,  /**< Open the selected save and read it before writing. */
    CARD_MENU_EXCHANGE_STEP_POLL_PREWRITE_READ = 28  /**< Wait for that read to finish, retrying. */
} CardMenuExchangeStep;

/** @brief What running one step of the card sequence reports to its caller. */
typedef enum
{
    CARD_MENU_SEQUENCE_NONE = 0,       /**< Never returned. */
    CARD_MENU_SEQUENCE_WAIT = 1,       /**< Step handled; poll again next frame. */
    CARD_MENU_SEQUENCE_FINISHED = 2,   /**< The step table ended. */
    CARD_MENU_SEQUENCE_RUN_AGAIN = 3,  /**< A card command was issued; run the next step now. */
    CARD_MENU_SEQUENCE_NO_CARD = 4,    /**< The card stopped answering; the entry state says so. */
    CARD_MENU_SEQUENCE_UNFORMATTED = 5 /**< _card_load kept reporting a new card: the card is not formatted. */
} CardMenuSequenceResult;

/** @brief Bytes read to show an entry: the card header and the first 0x100 bytes of the saved game. */
#define CARD_MENU_ENTRY_READ_BYTES 0x280

/** @brief Bytes read to show an entry that is not a Legend of Mana save: its card header title and CLUT. */
#define CARD_MENU_ENTRY_TITLE_READ_BYTES 0x80

/** @brief Suffix groups (0 to 7) the directory sort buckets saves into. */
#define CARD_MENU_ENTRY_GROUP_COUNT 8

/** @brief Length of the new-save placeholder entry name. */
#define CARD_MENU_NEW_SAVE_NAME_LENGTH 8

/** @brief Attempts made at a synchronous card file operation before giving up. */
#define CARD_MENU_FILE_OP_ATTEMPTS 20

/** @brief Retries of a failed asynchronous save read or write. */
#define CARD_MENU_SAVE_RETRIES 5

/** @brief Polls of a card load that keeps reporting a new card before the card counts as unformatted. */
#define CARD_MENU_CARD_LOAD_RETRIES 16

/** @brief Choice selected in a yes/no prompt. */
#define CARD_MENU_CHOICE_YES 0
#define CARD_MENU_CHOICE_NO 1

/**
 * @brief Choice a yes/no prompt starts on.
 * @note JP starts on yes, US on no.
 */
#if defined(VERSION_JP)
#define CARD_MENU_CHOICE_DEFAULT CARD_MENU_CHOICE_YES
#else
#define CARD_MENU_CHOICE_DEFAULT CARD_MENU_CHOICE_NO
#endif

/** @brief Pad buttons that move a yes/no prompt to the other choice. */
#define CARD_MENU_CHOICE_BUTTON_MASK (PAD_BTN_RIGHT | PAD_BTN_LEFT)

/** @brief Gaps between a prompt's centre and its right-aligned "yes" and left-aligned "no". */
#define CARD_MENU_CHOICE_YES_GAP 16
#define CARD_MENU_CHOICE_NO_GAP 8

/** @brief VSync ticks a transfer progress bar takes to fill. */
#define CARD_MENU_PROGRESS_FULL_TICKS 256

/** @brief Corner colours of the progress bar's gradient (POLY_G4 vertices 0 to 3). */
#define CARD_MENU_PROGRESS_TOP_LEFT_COLOR GPU_COLOR_WORD(0xFF, 0, 0)        /**< Red. */
#define CARD_MENU_PROGRESS_TOP_RIGHT_COLOR GPU_COLOR_WORD(0xFF, 0xFF, 0)    /**< Yellow. */
#define CARD_MENU_PROGRESS_BOTTOM_LEFT_COLOR GPU_COLOR_WORD(0, 0xFF, 0xFF)  /**< Cyan. */
#define CARD_MENU_PROGRESS_BOTTOM_RIGHT_COLOR GPU_COLOR_WORD(0, 0, 0xFF)    /**< Blue. */

/** @brief VRAM area, right of the display buffers, the party icons (portraits) are uploaded to side by side. */
#define CARD_MENU_ICON_VRAM_X SCREEN_WIDTH
#define CARD_MENU_ICON_VRAM_Y 208

/**
 * @brief Icon @p icon of an icon set whose first per-icon offset is @p offsets.
 * @note The offsets count from the icon-set start, which is the word just before them.
 */
#define CARD_MENU_ICON_IMAGE(offsets, icon) ((FieldPortrait*)((u8*)(offsets) - 4 + (offsets)[icon]))

/** @brief Menu windows (elements) in a card menu's pool. */
#define CARD_MENU_ELEMENT_COUNT 8

/** @brief CardMenuElement.attr.bits.state values, and their mask in attr.word. */
#define CARD_MENU_ELEMENT_FREE 0    /**< Unused; the pool allocator may claim it. */
#define CARD_MENU_ELEMENT_OPENING 1 /**< Growing to full size over CARD_MENU_ELEMENT_TRANSITION_STEPS frames. */
#define CARD_MENU_ELEMENT_OPEN 2    /**< Drawn at full size. */
#define CARD_MENU_ELEMENT_CLOSING 3 /**< Shrinking away. */
#define CARD_MENU_ELEMENT_CLOSED 4  /**< Gone; freed after CARD_MENU_ELEMENT_CLOSED_FRAMES frames. */
#define CARD_MENU_ELEMENT_STATE_MASK 7

/** @brief Pool slots of the modal window (dialogs, prompts, progress) and of the first window the screen builds. */
#define CARD_MENU_ELEMENT_MODAL 0
#define CARD_MENU_ELEMENT_MAIN 1

/** @brief Frames a window takes to open or close; its size scales by transition_step / this. */
#define CARD_MENU_ELEMENT_TRANSITION_STEPS 8

/** @brief Frames a closed window stays in CARD_MENU_ELEMENT_CLOSED before it is freed. */
#define CARD_MENU_ELEMENT_CLOSED_FRAMES 3

/** @brief Draw a window's contents at its current animation offset and return the packet cursor. */
typedef void* (*CardMenuElementDrawFunc)(u_long* ot, void* prim, s32 x_offset, s32 y_offset);

/**
 * @brief One animated menu window (element) of a card menu's pool.
 *
 * The nine-bit window width straddles the two words: its low eight bits are
 * the top byte of attr and its high bit is size.bits.width_high. No bitfield
 * can span that boundary, so the low byte is read and written through
 * attr.word (see CARD_MENU_ELEMENT_WIDTH and CARD_MENU_SET_ELEMENT_WIDTH_LOW).
 */
typedef struct CardMenuElement
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
            /** @brief Free for the overlay: ADDHERO marks its scrolling entry list, CLOAD the windows it outlines. */
            u32 flag : 1;
        } bits;
    } size;
    CardMenuElementDrawFunc draw;
} CardMenuElement;

/** @brief Bit position of the width's low byte inside CardMenuElement.attr.word. */
#define CARD_MENU_ELEMENT_WIDTH_SHIFT 24

/** @brief Low eight bits of a window's width. */
#define CARD_MENU_ELEMENT_WIDTH_LOW(element) ((element)->attr.word >> CARD_MENU_ELEMENT_WIDTH_SHIFT)

/**
 * @brief Full nine-bit width of a window.
 * @param element Window whose width is read.
 * @param width_low The width's low byte, as read by CARD_MENU_ELEMENT_WIDTH_LOW.
 */
#define CARD_MENU_ELEMENT_WIDTH(element, width_low) ((s32)(((element)->size.bits.width_high << 8) | (width_low)))

/** @brief Store the low eight bits of a window's width. */
#define CARD_MENU_SET_ELEMENT_WIDTH_LOW(element, width) \
    ((element)->attr.word = ((element)->attr.word & ((1 << CARD_MENU_ELEMENT_WIDTH_SHIFT) - 1)) | ((u32)(width) << CARD_MENU_ELEMENT_WIDTH_SHIFT))

/** @brief Heights of one entry-list row and of one message-window line, in pixels. */
#define CARD_MENU_ENTRY_ROW_HEIGHT 14
#define CARD_MENU_LINE_HEIGHT 14

/** @brief Entry list: top and height (the left edge and width differ per screen). */
#define CARD_MENU_LIST_Y 50
#define CARD_MENU_LIST_HEIGHT 88

/** @brief Rows that fit in the entry list, and the top of the last one. */
#define CARD_MENU_LIST_VISIBLE_ROWS (CARD_MENU_LIST_HEIGHT / CARD_MENU_ENTRY_ROW_HEIGHT)
#define CARD_MENU_LIST_LAST_ROW_Y ((CARD_MENU_LIST_VISIBLE_ROWS - 1) * CARD_MENU_ENTRY_ROW_HEIGHT)

/** @brief Scroll arrows, inset from the entry list's top and bottom. */
#define CARD_MENU_SCROLL_ARROW_UP_Y (CARD_MENU_LIST_Y + 8)
#define CARD_MENU_SCROLL_ARROW_DOWN_Y (CARD_MENU_LIST_Y + CARD_MENU_LIST_HEIGHT - 8)

/** @brief Frames a list scroll takes to reach its target. */
#define CARD_MENU_SCROLL_FRAMES 4

/** @brief Entry list columns: the file label, the number label and the suffix value (JP moves the value right). */
#define CARD_MENU_ENTRY_LABEL_X 1
#define CARD_MENU_ENTRY_NUMBER_LABEL_X 112
#if defined(VERSION_JP)
#define CARD_MENU_ENTRY_VALUE_X 0x94
#else
#define CARD_MENU_ENTRY_VALUE_X 0x86
#endif

/** @brief Colour of the semi-transparent selected-row highlight. */
#define CARD_MENU_HIGHLIGHT_COLOR GPU_COLOR_WORD(0xF0, 0x80, 0xF0)

/** @brief Colour of the semi-transparent tile that dims the inactive card slot's label. */
#define CARD_MENU_INACTIVE_LABEL_COLOR GPU_COLOR_WORD(0x10, 0x10, 0x10)

/** @brief Card-slot label windows: width and text x (JP narrows both), and height. */
#if defined(VERSION_JP)
#define CARD_MENU_CARD_LABEL_WIDTH 0x70
#define CARD_MENU_CARD_LABEL_TEXT_X 0x38
#else
#define CARD_MENU_CARD_LABEL_WIDTH 0x80
#define CARD_MENU_CARD_LABEL_TEXT_X 0x40
#endif
#define CARD_MENU_CARD_LABEL_HEIGHT 16

/** @brief Details window of the browser layout, and its line spacing. */
#define CARD_MENU_DETAILS_X 30
#define CARD_MENU_DETAILS_Y 142
#define CARD_MENU_DETAILS_WIDTH 260
#define CARD_MENU_DETAILS_HEIGHT 52
#define CARD_MENU_DETAILS_LINE_HEIGHT 16

/** @brief Details window text: second column and the play-time digits (right edges). */
#if defined(VERSION_JP)
#define CARD_MENU_DETAILS_TEXT_X 80
#define CARD_MENU_DETAILS_HOURS_RIGHT_X 100
#define CARD_MENU_DETAILS_TIME_SEPARATOR_X 100
#define CARD_MENU_DETAILS_MINUTES_TENS_RIGHT_X 120
#define CARD_MENU_DETAILS_MINUTES_RIGHT_X 130
#else
#define CARD_MENU_DETAILS_TEXT_X 84
#define CARD_MENU_DETAILS_HOURS_RIGHT_X 112
#define CARD_MENU_DETAILS_TIME_SEPARATOR_X 111
#define CARD_MENU_DETAILS_MINUTES_TENS_RIGHT_X 125
#define CARD_MENU_DETAILS_MINUTES_RIGHT_X 133
#endif

/** @brief Message window (status, prompts and progress bar): left edge, width and three-line height. */
#define CARD_MENU_MESSAGE_X 16
#define CARD_MENU_MESSAGE_WIDTH 288
#define CARD_MENU_MESSAGE_HEIGHT 44

/** @brief Left edge and width of the dialog window (its top and height differ per screen). */
#define CARD_MENU_DIALOG_X 32
#define CARD_MENU_DIALOG_WIDTH 256

/** @brief Frames of the fade back to the host screen when a card menu exits. */
#define CARD_MENU_EXIT_FADE_FRAMES 8

/**
 * @brief Layout of the screens that read another player's save (ADDHERO and
 *        NIKI): the browser layout (mode 0) and the transfer layout (mode 1).
 */
#define CARD_MENU_EXCHANGE_LIST_X 28
#define CARD_MENU_EXCHANGE_LIST_WIDTH 264
#define CARD_MENU_EXCHANGE_SCROLL_ARROW_X (CARD_MENU_EXCHANGE_LIST_X + CARD_MENU_EXCHANGE_LIST_WIDTH - 16)
#define CARD_MENU_EXCHANGE_TITLE_X 36
#define CARD_MENU_EXCHANGE_TITLE_Y 10
#define CARD_MENU_EXCHANGE_TITLE_WIDTH 240
#define CARD_MENU_EXCHANGE_TITLE_HEIGHT 16
#if defined(VERSION_JP)
#define CARD_MENU_EXCHANGE_CARD_SLOT0_LABEL_X 0x28 /**< JP moves the narrower slot 0 label right. */
#else
#define CARD_MENU_EXCHANGE_CARD_SLOT0_LABEL_X 0x18
#endif
#define CARD_MENU_EXCHANGE_CARD_SLOT1_LABEL_X 160
#define CARD_MENU_EXCHANGE_CARD_LABEL_BROWSER_Y 30
#define CARD_MENU_EXCHANGE_CARD_LABEL_TRANSFER_Y 77
#define CARD_MENU_EXCHANGE_MESSAGE_Y 97
#define CARD_MENU_EXCHANGE_PROMPT_HEIGHT 30 /**< Two lines. */
#define CARD_MENU_EXCHANGE_DIALOG_Y 112
#define CARD_MENU_EXCHANGE_DIALOG_HEIGHT 20

/** @brief Entry list columns of the rank marker (JP moves it right) and of the "+" marker's right edge. */
#if defined(VERSION_JP)
#define CARD_MENU_EXCHANGE_ENTRY_MARKER_X 0xCC
#else
#define CARD_MENU_EXCHANGE_ENTRY_MARKER_X 0xC0
#endif
#define CARD_MENU_EXCHANGE_ENTRY_PLUS_RIGHT_X 242

/** @brief Dialog messages every card menu shows; each overlay adds its own from 5 up. */
#define CARD_MENU_DIALOG_SAVE_FAILED 0
#define CARD_MENU_DIALOG_LOAD_FAILED 1
#define CARD_MENU_DIALOG_CARD_NOT_INSERTED 2
#define CARD_MENU_DIALOG_NOT_POCKETSTATION 3 /**< The card is not a PocketStation; the US release has no text for it. */
#define CARD_MENU_DIALOG_INVALID_SAVE 4      /**< The save failed validation; shows the load-failed text. */

/** @brief What the details window shows for the selected entry. */
#define CARD_MENU_SELECTION_NONE 0       /**< Nothing to show yet. */
#define CARD_MENU_SELECTION_ENTRY_READ 1 /**< The selected entry's header has been read. */
#define CARD_MENU_SELECTION_NEW_SAVE 2   /**< The new-save placeholder is selected. */
#define CARD_MENU_SELECTION_EMPTY_CARD 3 /**< The card has no entries. */

/**
 * @brief Text table indexes.
 *
 * Every card menu overlay carries its own copy of the same text table: u16
 * offsets from the table start, then the strings. The US release leaves the
 * PocketStation texts empty (29, 33, 47-49 and 58-87). The order of the
 * card-access message lines differs by version: US line 2 is
 * CARD_MENU_TEXT_DO_NOT_REMOVE_CARD and line 3 CARD_MENU_TEXT_CARD_OR_CONTROLLER,
 * JP the other way round. CARD_MENU_TEXT_AT needs the index of the entry symbol
 * it is given.
 */
#define CARD_MENU_TEXT_CHECKING_CARD 0
#define CARD_MENU_TEXT_NOT_ENOUGH_BLOCKS 1
#define CARD_MENU_TEXT_NO_CARD 2
#define CARD_MENU_TEXT_MANA_LABEL 3
#define CARD_MENU_TEXT_OTHER_GAME_LABEL 4
#define CARD_MENU_TEXT_SAVE_TITLE 5
#define CARD_MENU_TEXT_CARD_SLOT0_LABEL 6
#define CARD_MENU_TEXT_CARD_SLOT1_LABEL 7
#define CARD_MENU_TEXT_CARD_ACCESS_FAILED 8
#define CARD_MENU_TEXT_NO_SAVE_DATA 9
#define CARD_MENU_TEXT_NEW_SAVE_LABEL 10
#define CARD_MENU_TEXT_SAVE_PROMPT 11
#define CARD_MENU_TEXT_OVERWRITE_PROMPT 12
#define CARD_MENU_TEXT_SAVING 14
#define CARD_MENU_TEXT_DO_NOT_REMOVE_CARD 15
#define CARD_MENU_TEXT_SAVED 16
#define CARD_MENU_TEXT_MEMORY_CARD_IS 17 /**< JP only: subject line "The memory card is". */
#define CARD_MENU_TEXT_NOT_FORMATTED 18 /**< Second line after a subject such as CARD_MENU_TEXT_POCKETSTATION_IS. */
#define CARD_MENU_TEXT_FORMAT_PROMPT 19
#define CARD_MENU_TEXT_NEW_SAVE_TITLE 20
#define CARD_MENU_TEXT_USES_TWO_BLOCKS 21
#define CARD_MENU_TEXT_LOAD_TITLE 22
#define CARD_MENU_TEXT_NUMBER_LABEL 23
#define CARD_MENU_TEXT_LOAD_PROMPT 24
#define CARD_MENU_TEXT_LOADING 25
#define CARD_MENU_TEXT_NO_GAME_SAVE_DATA 26
#define CARD_MENU_TEXT_NEWEST 27
#define CARD_MENU_TEXT_OLDEST 28
#define CARD_MENU_TEXT_RING_RING_LAND_LABEL 29
#define CARD_MENU_TEXT_SAVE_FAILED 30
#define CARD_MENU_TEXT_LOAD_FAILED 31
#define CARD_MENU_TEXT_CARD_NOT_INSERTED 32
#define CARD_MENU_TEXT_NOT_POCKETSTATION 33
#define CARD_MENU_TEXT_SELECT_SAVE_DATA 34
#define CARD_MENU_TEXT_SELECT_ITEM 35
#define CARD_MENU_TEXT_NO_ITEMS 39
#define CARD_MENU_TEXT_SAME_HERO_DATA 40
#define CARD_MENU_TEXT_WRONG_VERSION 42
#define CARD_MENU_TEXT_CHECK_CARD_INSERTED 43
#define CARD_MENU_TEXT_FORMATTING 44
#define CARD_MENU_TEXT_NEEDS_TWO_BLOCKS 45
#define CARD_MENU_TEXT_SAVE_CORRUPT 46
#define CARD_MENU_TEXT_DOWNLOAD_RING_RING_LAND 48
#define CARD_MENU_TEXT_NEEDS_SIX_BLOCKS 49
#define CARD_MENU_TEXT_FORMAT_FAILED 50
#define CARD_MENU_TEXT_CARD_FULL_LABEL 51
#define CARD_MENU_TEXT_NO_LOAD_FILE 52
#define CARD_MENU_TEXT_FOUND_LOAD_FILE 53
#define CARD_MENU_TEXT_OVERWRITE_2P_DATA 54
#define CARD_MENU_TEXT_2P_DATA_NOT_SAVED 55
#define CARD_MENU_TEXT_TRADE_DATA_NOT_SAVED 57
#define CARD_MENU_TEXT_CHECKING_POCKETSTATION 58
#define CARD_MENU_TEXT_NO_POCKETSTATION 59
#define CARD_MENU_TEXT_POCKETSTATION_ACCESS_FAILED 60
#define CARD_MENU_TEXT_POCKETSTATION_OR_CONTROLLER 61
#define CARD_MENU_TEXT_POCKETSTATION_IS 62
#define CARD_MENU_TEXT_POCKETSTATION_NOT_INSERTED 63
#define CARD_MENU_TEXT_RING_RING_LAND_WAS 65
#define CARD_MENU_TEXT_NOT_FOUND 66
#define CARD_MENU_TEXT_RECEIVED_ITEMS 67
#define CARD_MENU_TEXT_RETURN_PET 69
#define CARD_MENU_TEXT_SWAP_PETS 71
#define CARD_MENU_TEXT_GAME_FROM_POCKETSTATION 73
#define CARD_MENU_TEXT_GAME_TO_POCKETSTATION 74
#define CARD_MENU_TEXT_DOWNLOAD_OK 75
#define CARD_MENU_TEXT_OVERWRITE_OK 76
#define CARD_MENU_TEXT_SELECT_SLOT 77
#define CARD_MENU_TEXT_RING_RING_LAND_TITLE 78
#define CARD_MENU_TEXT_DOWNLOAD_FAILED 79
#define CARD_MENU_TEXT_UPLOAD_FAILED 80
#define CARD_MENU_TEXT_DOWNLOADING 81
#define CARD_MENU_TEXT_UPLOADING 83
#define CARD_MENU_TEXT_RING_RING_LAND_SIX_BLOCKS 85
#define CARD_MENU_TEXT_WILL_BE_ERASED 86
#define CARD_MENU_TEXT_PET_ALREADY_ON_RANCH 87
#define CARD_MENU_TEXT_PLUS_MARKER 88
#define CARD_MENU_TEXT_CARD_OR_CONTROLLER 89

/**
 * @brief Text table index of the "card is not formatted" message.
 * @note JP reorders the text offset table; this entry is index 18 there.
 */
#if defined(VERSION_JP)
#define CARD_MENU_TEXT_CARD_UNFORMATTED 18
#else
#define CARD_MENU_TEXT_CARD_UNFORMATTED 90
#endif

/**
 * @brief Address of text @p index, reached through its own u16 offset-table entry @p entry.
 * @note The table start is derived back from the entry symbol, like FIELD_UI_TEXT_AT.
 */
#define CARD_MENU_TEXT_AT(entry, index) ((u8*)&(entry) - (index) * 2 + (entry))

/** @brief Start of the text offset table, derived from entry @p entry at @p index. */
#define CARD_MENU_TEXT_TABLE(entry, index) (&(entry) - (index))

/** @brief Address of text @p index in the u16 offset table starting at @p table. */
#define CARD_MENU_TEXT(table, index) ((u8*)(table) + (table)[index])

/** @brief Address of the text whose table offset is @p offset. */
#define CARD_MENU_TEXT_BY_OFFSET(table, offset) ((u8*)((uintptr_t)(offset) + (uintptr_t)(table)))


/** @brief Pool of menu windows; CARD_MENU_ELEMENT_MODAL is the primary dialog. */
extern CardMenuElement g_card_menu_element_pool[CARD_MENU_ELEMENT_COUNT];

/** @brief Selected entry in the save list, and the list's scroll position, target and remaining scroll frames. */
extern s32 g_card_menu_selected_row;
extern s32 g_card_menu_scroll_y;
extern s32 g_card_menu_scroll_target_y;
extern s32 g_card_menu_scroll_frames;

/** @brief Yes/no prompt selection, CARD_MENU_CHOICE_YES or CARD_MENU_CHOICE_NO. */
extern s32 g_card_menu_choice_toggle;

/** @brief Nonzero once the menu has finished; the next frame update shuts the menu down. */
extern s32 g_card_menu_exit_requested;

/** @brief Flipped once per frame by the menu update. */
extern s32 g_card_menu_frame_parity;

/** @brief Portrait CLUT staged for LoadImage (one 4-bit palette), and the golem palette of the selected save. */
extern u8 g_card_menu_icon_context[GPU_CLUT_4BIT_COLORS * 2];
extern s32 g_card_menu_icon_palette;

/** @brief Text offset-table entries of the card slot 0 and slot 1 labels (CARD_MENU_TEXT_CARD_SLOT0_LABEL and the next text). */
extern u16 g_card_menu_text_card_slot0_label;
extern u16 g_card_menu_text_card_slot1_label;

/** @brief Party icon offsets, counted from the icon count word just before them (see CARD_MENU_ICON_IMAGE). */
extern s32 g_card_menu_icon_offsets[];

/** @brief Overlay mode passed in by FIELD; each overlay defines its own values. */
extern s32 g_card_menu_mode;

/** @brief Result of committing the selected entry (CARD_MENU_SELECTION_*). */
extern s32 g_card_menu_selection_status;

/** @brief Message the status dialog shows (CARD_MENU_DIALOG_* or an overlay's own). */
extern s32 g_card_menu_dialog_state;

/** @brief Set while a card operation is running. */
extern s32 g_card_menu_io_busy;

/** @brief Set while the directory scan is running. */
extern s32 g_card_menu_entry_scan_active;

/** @brief Set while a load or save transfer is running, and once the save has been built and is being written. */
extern s32 g_card_menu_progress_active;
extern s32 g_card_menu_write_in_progress;

/** @brief Whether the progress bar is shown, and the VSync count it started at. */
extern s32 g_card_menu_progress_bar_active;
extern s32 g_card_menu_progress_start_tick;

/** @brief Nonzero to read the selected entry up to CARD_MENU_ENTRY_READ_BYTES, zero to read only its title (CARD_MENU_ENTRY_TITLE_READ_BYTES). */
extern s32 g_card_menu_selected_entry_extended;

/** @brief Animation phase of the selected entry's party icons. */
extern s32 g_card_menu_icon_phase;

/**
 * @brief Text offset-table entries of the shared card-menu messages (indexes CARD_MENU_TEXT_*).
 * @note g_card_menu_text_table is the first entry; not every overlay has every message.
 */
extern u16 g_card_menu_text_table;
extern u16 g_card_menu_text_no_card;
extern u16 g_card_menu_text_mana_label;
extern u16 g_card_menu_text_other_game_label;
extern u16 g_card_menu_text_new_save_label;
extern u16 g_card_menu_text_number_label;
extern u16 g_card_menu_text_ring_ring_land_label;
extern u16 g_card_menu_text_plus_marker;
extern u16 g_card_menu_text_no_save_data;
extern u16 g_card_menu_text_no_game_save_data;
extern u16 g_card_menu_text_not_enough_blocks;
extern u16 g_card_menu_text_card_access_failed;
extern u16 g_card_menu_text_load_prompt;
extern u16 g_card_menu_text_select_item;
extern u16 g_card_menu_text_select_save_data;
extern u16 g_card_menu_text_save_failed;
extern u16 g_card_menu_text_load_failed;
extern u16 g_card_menu_text_card_not_inserted;
/** @brief "Not a PocketStation" dialog text; empty in the US release. */
extern u16 g_card_menu_text_not_pocketstation;

void card_menu_deactivate_primary_element(void);
void card_menu_close_all_elements(void);
void card_menu_scroll_to_selection(void);
CardMenuElement* card_menu_alloc_element(void);
void card_menu_enable_choice_toggle(void);
void card_menu_clear_elements(void);
void card_menu_update_elements(FieldRenderHalf* render);
s32 card_menu_update_frame(FieldRenderHalf* render);
void* card_menu_draw_choice_prompt(void* prim, u_long* ot, s32 x, s32 y);
void* card_menu_draw_icon_highlight(POLY_FT4* quad, u_long* ot, s32 x, s32 y, s32 width, s32 icon, s32 index, s32 row);
void* card_menu_draw_card_slot0_label(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* card_menu_draw_card_slot1_label(u_long* ot, void* prim, s32 x_offset, s32 y_offset);

void card_menu_open_status_dialog(s32 message_id);
void* card_menu_draw_status_dialog(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* card_menu_draw_save_status_dialog(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* card_menu_draw_progress_bar(POLY_G4* quad, u_long* ot);
void card_menu_init_card_events(void);
s32 card_menu_begin_entry_scan(s32 page);
void card_menu_commit_selected_entry(void);
void* card_menu_draw_load_prompt(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* card_menu_draw_entry_list(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* card_menu_draw_mode_title(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
void* card_menu_draw_cant_hold_more(u_long* ot, void* prim, s32 x_offset, s32 y_offset);

/** @brief Each overlay's own per-frame menu logic and window drawing, called by the shared functions above. */
void card_menu_update_state(FieldRenderHalf* render);
void card_menu_update_and_draw_elements(FieldRenderHalf* render);

#endif
