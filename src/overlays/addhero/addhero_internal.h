#ifndef ADDHERO_INTERNAL_H
#define ADDHERO_INTERNAL_H

#include "common.h"
#include "saved_game.h"
#include "pad.h"
#include "vector.h"
#include "display.h"
#include "gpu_packet.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"
#include "sdk/kernel.h"
#include "sdk/libapi.h"
#include "sdk/libetc.h"
#include "sdk/strings.h"
#include "sdk/libmcx.h"
#include "controller.h"
#include "field_menu_window.h"
#include "field_sound.h"
#include "field_ui_text.h"

/* Declarations shared by ADDHERO implementation files. */

#define ADDHERO_DIRECTORY_ENTRY_COUNT 20
#define ADDHERO_DIRECTORY_ENTRY_BYTES sizeof(struct DIRENTRY)
#define ADDHERO_CARD_DIRECTORY_BYTES (ADDHERO_DIRECTORY_ENTRY_COUNT * ADDHERO_DIRECTORY_ENTRY_BYTES)
#define ADDHERO_CARD_BLOCK_BYTES 8192

/** @brief Save files a memory card holds (its 15 data blocks). */
#define ADDHERO_CARD_SAVE_SLOTS 15
#define ADDHERO_USED_BLOCK_LIMIT 14
#define ADDHERO_SAVE_FILENAME_PREFIX_LENGTH 12
#define ADDHERO_NEW_SAVE_FILENAME_PREFIX_LENGTH 8
#define ADDHERO_LOAD_RESULT_NONE 0
#define ADDHERO_LOAD_RESULT_ABORT 2
#define ADDHERO_LOAD_RESULT_CONTINUE 3
#define ADDHERO_LOAD_RESULT_COMPLETE 4
#define ADDHERO_LOAD_RESULT_CARD_ERROR 5

/**
 * @brief g_addhero_entry_state values.
 *
 * Below ADDHERO_ENTRY_COUNT_LIMIT the value is the number of save entries
 * read from the current card (a card holds 15). From 0xF3 up it is a status
 * whose message the list or transfer window shows instead of the entries.
 */
#define ADDHERO_ENTRY_STATE_CONFIRM_NO_SAVE 0xF3    /**< Asks whether to leave without saving the 2P data. */
#define ADDHERO_ENTRY_STATE_SAVE_CONFIRM 0xF4       /**< A load file was found; asks whether to overwrite the 2P data. */
#define ADDHERO_ENTRY_STATE_SAVE_PROGRESS 0xF5      /**< Writing the save; progress bar. */
#define ADDHERO_ENTRY_STATE_LOAD_PROGRESS 0xF6      /**< Reading the selected save; progress bar. */
#define ADDHERO_ENTRY_STATE_NO_LOAD_FILE 0xF7       /**< The selected entry is not a load file. */
#define ADDHERO_ENTRY_STATE_NO_GAME_DATA 0xF8       /**< The card holds no Legend of Mana save data. */
#define ADDHERO_ENTRY_STATE_BROWSER_READ_ERROR 0xF9 /**< Card error while browsing; shows the no-game-data message. */
#define ADDHERO_ENTRY_STATE_CARD_FULL 0xFA          /**< Not enough free blocks for a new save. */
#define ADDHERO_ENTRY_STATE_ACCESS_FAILED 0xFB      /**< The card could not be accessed. */
#define ADDHERO_ENTRY_STATE_NO_SAVE_DATA 0xFC       /**< The card holds no save data. */
#define ADDHERO_ENTRY_STATE_NO_CARD 0xFD            /**< No card answers: an event error or the retries ran out. */
#define ADDHERO_ENTRY_STATE_BLANK 0xFE              /**< Shows nothing; ADDHERO never sets it. */
#define ADDHERO_ENTRY_STATE_CHECKING_CARD 0xFF      /**< The card is being checked; no entries yet. */

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
/** @brief Save file read or written by the load and save sequences. */
extern SaveFile g_addhero_save_file;
/**
 * @brief Start of the selected entry's save file: only the card header and the
 *        first 0x100 bytes of the saved game are read (ADDHERO_ENTRY_READ_BYTES).
 */
extern SaveFile g_addhero_entry_file;
extern char g_addhero_save_file_path[];
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

/** @brief Element pool size and AddheroElement.attr.bits.state values. */
#define ADDHERO_ELEMENT_COUNT 8
#define ADDHERO_ELEMENT_STATE_MASK 7
#define ADDHERO_ELEMENT_STATE_INACTIVE 0
#define ADDHERO_ELEMENT_STATE_OPENING 1
#define ADDHERO_ELEMENT_STATE_ACTIVE 2
#define ADDHERO_ELEMENT_STATE_CLOSING 3
#define ADDHERO_ELEMENT_STATE_FINISHING 4

/** @brief Element pool slot of the modal window (dialogs, load progress), drawn with the bright frame; the builders hold it while allocating. */
#define ADDHERO_ELEMENT_MODAL 0

/** @brief Element pool slot of the first allocated window: the entry list or the transfer status. */
#define ADDHERO_ELEMENT_MAIN 1

/** @brief Frames an element takes to open or close; its window scales by transition_step / this. */
#define ADDHERO_ELEMENT_TRANSITION_STEPS 8

/** @brief Frames a closed element stays in ADDHERO_ELEMENT_STATE_FINISHING before it is freed. */
#define ADDHERO_ELEMENT_FINISH_FRAMES 3

/** @brief Card header titles are Shift-JIS: bytes from this value up lead a two-byte character. */
#define ADDHERO_SJIS_LEAD_MIN 0x80

/** @brief Codes below this end the text drawn by addhero_draw_cached_text. */
#define ADDHERO_TEXT_FIRST_PRINTABLE 0x20

/** @brief Height of one entry-list row and of one message line, in pixels. */
#define ADDHERO_ENTRY_ROW_HEIGHT 14
#define ADDHERO_TEXT_LINE_HEIGHT 14

/** @brief Entry-state values below this are entry counts; see ADDHERO_ENTRY_STATE_CHECKING_CARD. */
#define ADDHERO_ENTRY_COUNT_LIMIT 0x10

/** @brief Entry-list window of the browser layout (mode 0). */
#define ADDHERO_LIST_X 28
#define ADDHERO_LIST_Y 50
#define ADDHERO_LIST_WIDTH 264
#define ADDHERO_LIST_HEIGHT 88

/** @brief Rows that fit in the entry list, and the top of the last one. */
#define ADDHERO_LIST_VISIBLE_ROWS (ADDHERO_LIST_HEIGHT / ADDHERO_ENTRY_ROW_HEIGHT)
#define ADDHERO_LIST_LAST_ROW_Y ((ADDHERO_LIST_VISIBLE_ROWS - 1) * ADDHERO_ENTRY_ROW_HEIGHT)

/** @brief Frames a list scroll takes to reach its target. */
#define ADDHERO_SCROLL_FRAMES 4

/** @brief Scroll arrows, inset from the entry list's right edge, top and bottom. */
#define ADDHERO_SCROLL_ARROW_X (ADDHERO_LIST_X + ADDHERO_LIST_WIDTH - 16)
#define ADDHERO_SCROLL_ARROW_UP_Y (ADDHERO_LIST_Y + 8)
#define ADDHERO_SCROLL_ARROW_DOWN_Y (ADDHERO_LIST_Y + ADDHERO_LIST_HEIGHT - 8)

/** @brief Title window of the browser layout. */
#define ADDHERO_TITLE_X 36
#define ADDHERO_TITLE_Y 10
#define ADDHERO_TITLE_WIDTH 240
#define ADDHERO_TITLE_HEIGHT 16

/** @brief Card-slot label windows; slot 0 sits at ADDHERO_CARD_SLOT0_LABEL_X. */
#define ADDHERO_CARD_SLOT1_LABEL_X 160
#define ADDHERO_CARD_LABEL_HEIGHT 16
#define ADDHERO_CARD_LABEL_BROWSER_Y 30  /**< Browser layout. */
#define ADDHERO_CARD_LABEL_TRANSFER_Y 77 /**< Transfer layout. */

/** @brief Details window of the browser layout. */
#define ADDHERO_DETAILS_X 30
#define ADDHERO_DETAILS_Y 142
#define ADDHERO_DETAILS_WIDTH 260
#define ADDHERO_DETAILS_HEIGHT 52

/** @brief Message window: the transfer layout's status and the load prompt. */
#define ADDHERO_MESSAGE_X 16
#define ADDHERO_MESSAGE_Y 97
#define ADDHERO_MESSAGE_WIDTH 288
#define ADDHERO_MESSAGE_HEIGHT 44 /**< Three lines. */
#define ADDHERO_PROMPT_HEIGHT 30  /**< Two lines. */

/** @brief Dialog window. */
#define ADDHERO_DIALOG_X 32
#define ADDHERO_DIALOG_Y 112
#define ADDHERO_DIALOG_WIDTH 256
#define ADDHERO_DIALOG_HEIGHT 20

/** @brief g_addhero_dialog_state messages. */
#define ADDHERO_DIALOG_SAVE_FAILED 0
#define ADDHERO_DIALOG_LOAD_FAILED 1
#define ADDHERO_DIALOG_CARD_NOT_INSERTED 2
#define ADDHERO_DIALOG_CARD_TYPE_ERROR 3 /**< Its US text is empty. */
#define ADDHERO_DIALOG_INVALID_SAVE 4    /**< Shows the load-failed message. */

/** @brief g_addhero_selection_status values. */
#define ADDHERO_SELECTION_NONE 0       /**< Nothing to show yet. */
#define ADDHERO_SELECTION_ENTRY_READ 1 /**< The selected entry's header has been read. */
#define ADDHERO_SELECTION_NEW_SAVE 2   /**< The new-save placeholder is selected. */
#define ADDHERO_SELECTION_EMPTY_CARD 3 /**< The card has no entries. */

/** @brief g_addhero_result values, reported to the host when the overlay exits. */
#define ADDHERO_RESULT_LOADED 1
#define ADDHERO_RESULT_SAVED 2
#define ADDHERO_RESULT_CANCELLED 3

/** @brief Ticks a progress bar takes to fill the message window. */
#define ADDHERO_PROGRESS_FULL_TICKS 256

/** @brief Frames of the fade back to the host screen when the overlay exits. */
#define ADDHERO_EXIT_FADE_FRAMES 8

/** @brief Memory-card channel of card slot @p slot (port in the high nibble). */
#define ADDHERO_CARD_CHANNEL(slot) ((slot) * 0x10)

/**
 * @brief ADDHERO text table indexes.
 * @note ADDHERO_TEXT_AT needs the index of the entry symbol it is given.
 */
#define ADDHERO_TEXT_CHECKING_CARD 0
#define ADDHERO_TEXT_NOT_ENOUGH_BLOCKS 1
#define ADDHERO_TEXT_NO_CARD 2
#define ADDHERO_TEXT_CARD_SLOT0_LABEL 6
#define ADDHERO_TEXT_CARD_SLOT1_LABEL 7
#define ADDHERO_TEXT_CARD_ACCESS_FAILED 8
#define ADDHERO_TEXT_NO_SAVE_DATA 9
#define ADDHERO_TEXT_SAVING 14
#define ADDHERO_TEXT_DO_NOT_REMOVE_CARD 15 /**< Second line of the card-access messages. */
#define ADDHERO_TEXT_NEW_SAVE_TITLE 20
#define ADDHERO_TEXT_USES_TWO_BLOCKS 21
#define ADDHERO_TEXT_LOAD_PROMPT 24
#define ADDHERO_TEXT_LOADING 25
#define ADDHERO_TEXT_NO_GAME_SAVE_DATA 26
#define ADDHERO_TEXT_NEWEST 27
#define ADDHERO_TEXT_OLDEST 28
#define ADDHERO_TEXT_SAVE_FAILED 30
#define ADDHERO_TEXT_LOAD_FAILED 31
#define ADDHERO_TEXT_CARD_NOT_INSERTED 32
#define ADDHERO_TEXT_CARD_TYPE_ERROR 33
#define ADDHERO_TEXT_SELECT_SAVE_DATA 34
#define ADDHERO_TEXT_SELECT_ITEM 35
#define ADDHERO_TEXT_SAME_HERO_DATA 40
#define ADDHERO_TEXT_WRONG_VERSION 42
#define ADDHERO_TEXT_NO_LOAD_FILE 52
#define ADDHERO_TEXT_FOUND_LOAD_FILE 53
#define ADDHERO_TEXT_OVERWRITE_2P_DATA 54
#define ADDHERO_TEXT_2P_DATA_NOT_SAVED 55
#define ADDHERO_TEXT_CARD_OR_CONTROLLER 89 /**< Third line of the card-access messages. */

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
    ADDHERO_STEP_SCAN_DONE = 7,             /**< No-op after the scan; input stays blocked on this step and the scan. */
    ADDHERO_STEP_CLEAR_CARD = 8,            /**< Issue _card_clear on the current slot. */
    ADDHERO_STEP_LOAD_CARD = 9,             /**< Issue _card_load and arm the poll countdowns. */
    ADDHERO_STEP_ERASE_ENTRY = 10,          /**< Erase the selected directory entry. */
    ADDHERO_STEP_POLL_CARD_LOAD = 15,       /**< Wait for the _card_clear/_card_load result, retrying. */
    ADDHERO_STEP_WAIT_HARDWARE_EVENTS = 16, /**< Wait for any hardware card event. */
    ADDHERO_STEP_READ_ENTRY = 17,           /**< Open the selected save and start reading its header. */
    ADDHERO_STEP_POLL_ENTRY_READ = 18,      /**< Wait for the header read to finish. */
    ADDHERO_STEP_READ_SAVE = 19,            /**< Open the selected save and start reading the blob. */
    ADDHERO_STEP_POLL_SAVE_READ = 20,       /**< Wait for the blob read to finish, retrying. */
    ADDHERO_STEP_CHECK_CARD_TYPE = 24,      /**< Wait for a card and check its status. */
    ADDHERO_STEP_WRITE_SAVE = 25,           /**< Create the placeholder file and start writing the save blob. */
    ADDHERO_STEP_POLL_SAVE_WRITE = 26,      /**< Wait for the write and rename it over the selected save. */
    ADDHERO_STEP_READ_BEFORE_WRITE = 27,    /**< Open the selected save and read the blob before writing. */
    ADDHERO_STEP_POLL_PREWRITE_READ = 28,   /**< Wait for that read to finish, retrying. */
    ADDHERO_STEP_INIT_RETRIES = 30          /**< Arm the read/write retry counter. */
} AddheroCardStep;

#define ADDHERO_CONFIRM_BUTTON_MASK (PAD_BTN_CROSS | PAD_BTN_L3)
#define ADDHERO_CARD_SWITCH_BUTTON_MASK (PAD_BTN_SELECT | PAD_BTN_RIGHT | PAD_BTN_LEFT)

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

extern s32 g_save_slot_index;

/**
 * @brief FIELD data word next to D_80122714; ADDHERO stores 3 into it when Circle cancels the browser.
 * @note TODO: purpose unknown; no code in the main executable or any overlay reads it.
 */
extern s32 D_80122718;
extern s32 g_pad_input;
extern s32 g_menu_element_counter;
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
extern s32 g_addhero_entry_fields[][ADDHERO_DIRECTORY_ENTRY_COUNT];
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
extern u16 g_addhero_text_alt_save_label;
extern u16 g_addhero_text_save_failed;
extern u16 g_addhero_text_load_failed;
extern u16 g_addhero_text_card_not_inserted;
extern u16 g_addhero_text_card_type_error;
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
u8* addhero_skip_hex_digits(u8* text);
void addhero_terminate_multibyte_text(void* buffer);
void addhero_clear_elements(void);
AddheroElement* addhero_alloc_element(void);
void addhero_update_and_draw_elements(AddheroDrawState* draw_state);
void addhero_deactivate_primary_element(void);
void addhero_text_append(u8* dst, u8* src);
s32 addhero_text_byte_length(u8* text);
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
s32 addhero_validate_save_file(SaveFile* file);
s32 addhero_compute_save_checksum(u8* data);
s8* addhero_format_decimal(s8* out, s32 value);
void addhero_format_hex(s8* out, s32 value, s32 max_chars);
void addhero_hex_nibble_to_ascii(s8* out, s32 value);
u32 addhero_parse_hex(u8* s, s32 len);
s32 addhero_parse_hex_suffix_byte(u8* text);
s32 addhero_entry_blocks_reach_limit(void);
void addhero_erase_placeholder_files(void);

void addhero_reset_entry_ranks(void);
s32 addhero_parse_entry_fields(void);

/* FIELD functions used by ADDHERO; FIELD stays resident while the overlay runs. */
void* field_draw_text(void* prim, u_long* ot, u8* text, s32 text_color, s32 x, s32 y, s32 flags);
void* field_draw_number(u_long* ot, void* prim, s32 value, s32 text_color, DVECTOR* position, s32 flags);
void field_copy_portrait_palette(void* dest, s32 index);
void field_copy_golem_portrait_palette(u8* destination, s32 palette);
void field_flag_known_save(char* file_name);
void field_reset_input_repeat(void);
void field_restore_fade_target(void);
void field_set_default_fade_target(void);
void field_restore_fade_target_with_duration(s16 duration);

#endif
