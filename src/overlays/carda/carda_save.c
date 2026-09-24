#include "carda_internal.h"

/**
 * @brief Size of one stored record in the g_pad_ctx block.
 * @note Record @c i lives at g_pad_ctx + i * CARDA_RECORD_SIZE; the CARDA_RECORD_*_OFFSET
 *       values are byte offsets from that address.
 */
#define CARDA_RECORD_SIZE 0x60

/** @brief Number of stored records. */
#define CARDA_RECORD_COUNT 5

/** @brief Offset of a record's data; its first byte is non-zero while the record is in use. */
#define CARDA_RECORD_ARRAY_OFFSET 0x2EF4

/** @brief Offset of a record's growth word (low byte kept, upper bits accumulate). */
#define CARDA_RECORD_GROWTH_OFFSET 0x2F0C

/** @brief Offset of a record's identifier word, compared against the save blob. */
#define CARDA_RECORD_ID_OFFSET 0x2F50

/** @brief Byte offset of the identifier word in the save blob at g_carda_save_blob. */
#define CARDA_SAVE_BLOB_ID_OFFSET 0x3B4

/** @brief Left edge of the save window text, before the transition offset. */
#define CARDA_SAVE_TEXT_X 0x90

/** @brief Pad bits that confirm a prompt. */
#define CARDA_PAD_CONFIRM 0x220

/** @brief Pad bit that cancels a prompt. */
#define CARDA_PAD_CANCEL 0x40

/** @brief Pad bits that move a yes/no choice (left and right). */
#define CARDA_PAD_LEFT_RIGHT 0xA000

/** @brief Card-sequence opcode that scans the card directory (see g_carda_save_step). */
#define CARDA_STEP_SCAN_ENTRIES 6

/**
 * @brief Save-flow state held in g_carda_entry_state while the mode 2/3 save window is up.
 * @note Values below CARDA_SAVE_STATE_DUPLICATE_RECORD are not states: they are the
 *       number of directory entries read from the card, searched one per frame for an
 *       existing save. The 0xF6-0xFF codes share their meaning with CLOAD's entry state.
 */
typedef enum CardaSaveState
{
    CARDA_SAVE_STATE_DUPLICATE_RECORD = 0xE9,         /**< A stored record already has this save's identifier. */
    CARDA_SAVE_STATE_CONFIRM_OVERWRITE = 0xEA,        /**< Mode 3: the save file exists; ask before replacing it. */
    CARDA_SAVE_STATE_STATUS_5 = 0xEB,                 /**< Status dialog 5 (carda_open_save_status_dialog). */
    CARDA_SAVE_STATE_STATUS_4 = 0xEC,                 /**< Status dialog 4 (carda_open_save_status_dialog). */
    CARDA_SAVE_STATE_STATUS_3 = 0xED,                 /**< Status dialog 3 (carda_open_save_status_dialog). */
    CARDA_SAVE_STATE_STATUS_CARD_INSERT_ERROR = 0xEE, /**< Status dialog 2: card not inserted properly. */
    CARDA_SAVE_STATE_STATUS_LOAD_FAILED = 0xEF,       /**< Status dialog 1: reading the card failed. */
    CARDA_SAVE_STATE_STATUS_SAVE_FAILED = 0xF0,       /**< Status dialog 0: writing the card failed. */
    CARDA_SAVE_STATE_SELECT_SLOT = 0xF1,              /**< Slot prompt only (carda_draw_slot_prompt). */
    CARDA_SAVE_STATE_CONFIRM_SAVE = 0xF2,             /**< Yes/no prompt before writing the save. */
    CARDA_SAVE_STATE_WRITING = 0xF3,                  /**< Card write sequence running (g_carda_save_in_progress). */
    CARDA_SAVE_STATE_CHECK_RECORDS = 0xF4,            /**< Wait for the timed step, then check the stored records. */
    CARDA_SAVE_STATE_UNUSED_NOTICE = 0xF5,            /**< Mode 3 notice without a slot prompt; never set in CARDA. */
    CARDA_SAVE_STATE_CARD_ERROR = 0xF6,               /**< The card type check kept failing. */
    CARDA_SAVE_STATE_NOT_ENOUGH_BLOCKS_ALT = 0xF7,    /**< Not enough free blocks, mode 2/3 wording. */
    CARDA_SAVE_STATE_NO_LOM_SAVE_DATA = 0xF8,         /**< No Legend of Mana save file on the card. */
    CARDA_SAVE_STATE_UNFORMATTED = 0xF9,              /**< The card is not formatted. */
    CARDA_SAVE_STATE_NOT_ENOUGH_BLOCKS = 0xFA,        /**< Not enough free blocks. */
    CARDA_SAVE_STATE_ACCESS_FAILED = 0xFB,            /**< Card access failed. */
    CARDA_SAVE_STATE_NO_SAVE_DATA = 0xFC,             /**< No save data on the card. */
    CARDA_SAVE_STATE_NO_CARD = 0xFD,                  /**< No memory card. */
    CARDA_SAVE_STATE_IDLE = 0xFE,                     /**< Nothing to draw. */
    CARDA_SAVE_STATE_CHECKING_CARD = 0xFF             /**< Card directory being read. */
} CardaSaveState;

/**
 * @brief Draw the two-line notice shared by the mode 3 error states (texts 0x41 and 0x42).
 * @param prim Primitive-buffer cursor.
 * @param ot Ordering-table head.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @param second_y Baseline of the second line, before the transition offset.
 * @return Advanced primitive-buffer cursor.
 */
static inline s32 carda_draw_mode3_notice(s32 prim, s32* ot, s32 x_offset, s32 y_offset, s32 second_y)
{
    s32 x;
    u16* text_table;

    x = -x_offset + CARDA_SAVE_TEXT_X;
    prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B0BA, 0x41), 4, x, -y_offset, 2);
    text_table = CARDA_TEXT_TABLE(D_8014B0BA, 0x41);
    return func_800A88A0(prim, ot, CARDA_TEXT(text_table, 0x42), 4, x, second_y - y_offset, 2);
}


/**
 * @brief Start closing every UI element.
 * @note Private copy of carda_clear_elements (carda.c), which this file inlines.
 */
static inline void carda_save_close_elements(void)
{
    CardaElement *element;
    s32 i;

    g_menu_element_counter = 0x20;
    element = g_carda_element_pool;
    for (i = 0; i < 8; i++)
    {
        element->attr.word &= ~7;
        element++;
    }
}

/**
 * @brief Claim the first free UI element.
 * @return The claimed element, or the pool head when every element is busy.
 * @note Private copy of carda_alloc_element (carda.c), which this file inlines.
 */
static inline CardaElement *carda_save_alloc_element(void)
{
    CardaElement *element;
    s32 i;

    element = g_carda_element_pool;
    for (i = 0; i < 8; i++, element++)
    {
        if ((element->attr.word & 7) == 0)
        {
            element->attr.word = (element->attr.word & ~7) | 1;
            return element;
        }
    }
    return g_carda_element_pool;
}

/**
 * @brief Draw the mode 2/3 save window and run its save flow.
 * @param ot Ordering-table head.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 * @note Dispatches on g_carda_entry_state (CardaSaveState): below CARDA_SAVE_STATE_DUPLICATE_RECORD
 *       it holds the directory entry count while the card is searched for an existing
 *       save; above it, the dialog currently shown. Dialog states also read the pad,
 *       move to the next state and start card sequences through g_carda_save_step.
 */
s32 carda_draw_save_flow(s32* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2]; /* never used, but the original stack frame reserves it */
    s32 counter;

    if (g_carda_element_pool[0].attr.f.state != 0)
    {
        /* The message states draw nothing while element 0 is active. */
        switch (g_carda_entry_state)
        {
        case CARDA_SAVE_STATE_DUPLICATE_RECORD:
        case CARDA_SAVE_STATE_STATUS_5:
        case CARDA_SAVE_STATE_STATUS_4:
        case CARDA_SAVE_STATE_STATUS_3:
        case CARDA_SAVE_STATE_STATUS_CARD_INSERT_ERROR:
        case CARDA_SAVE_STATE_STATUS_LOAD_FAILED:
        case CARDA_SAVE_STATE_STATUS_SAVE_FAILED:
        case CARDA_SAVE_STATE_NO_LOM_SAVE_DATA:
        case CARDA_SAVE_STATE_UNFORMATTED:
        case CARDA_SAVE_STATE_NOT_ENOUGH_BLOCKS:
        case CARDA_SAVE_STATE_ACCESS_FAILED:
        case CARDA_SAVE_STATE_NO_SAVE_DATA:
        case CARDA_SAVE_STATE_NO_CARD:
        case CARDA_SAVE_STATE_CHECKING_CARD:
            return prim;
        }
    }

    switch (g_carda_entry_state)
    {
    case CARDA_SAVE_STATE_NO_LOM_SAVE_DATA:
        prim = carda_draw_mode3_notice(prim, ot, x_offset, y_offset, 0xE);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, 0x1C - y_offset);
        break;

    case CARDA_SAVE_STATE_UNFORMATTED:
        if (g_carda_mode == 3)
        {
            prim = carda_draw_mode3_notice(prim, ot, x_offset, y_offset, 0xE);
            prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, 0x1C - y_offset);
        }
        else
        {
            prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(g_carda_text_card_unformatted, 0x5A), 4, -x_offset + CARDA_SAVE_TEXT_X, -y_offset, 2);
            prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, 0xE - y_offset);
        }
        break;

    case CARDA_SAVE_STATE_CARD_ERROR:
        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B07A, 0x21), 4, -x_offset + CARDA_SAVE_TEXT_X, -y_offset, 2);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, 0xE - y_offset);
        break;

    case CARDA_SAVE_STATE_CHECKING_CARD:
    {
        s32 x;
        u16* text_table;

        x = -x_offset + CARDA_SAVE_TEXT_X;
        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B0AC, 0x3A), 4, x, -y_offset, 2);
        text_table = CARDA_TEXT_TABLE(D_8014B0AC, 0x3A);
        prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 0x3D), 4, x, 0xE - y_offset, 2);
        prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 0x59), 4, x, 0x1C - y_offset, 2);
        break;
    }

    case CARDA_SAVE_STATE_NOT_ENOUGH_BLOCKS:
        if (g_carda_mode == 3)
        {
            prim = carda_draw_mode3_notice(prim, ot, x_offset, y_offset, 0xE);
            prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, 0x1C - y_offset);
        }
        else
        {
            s32 x;
            u16* text_table;

            x = -x_offset + CARDA_SAVE_TEXT_X;
            prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(g_carda_text_not_enough_blocks, 1), 4, x, -y_offset, 2);
            text_table = CARDA_TEXT_TABLE(g_carda_text_not_enough_blocks, 1);
            prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 0x2D), 4, x, 0xE - y_offset, 2);
            prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, 0x1C - y_offset);
        }
        break;

    case CARDA_SAVE_STATE_NOT_ENOUGH_BLOCKS_ALT:
        if (g_carda_mode == 3)
        {
            prim = carda_draw_mode3_notice(prim, ot, x_offset, y_offset, 0x10);
            g_carda_entry_state = CARDA_SAVE_STATE_NOT_ENOUGH_BLOCKS;
        }
        else
        {
            s32 x;
            u16* text_table;

            x = -x_offset + CARDA_SAVE_TEXT_X;
            prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(g_carda_text_not_enough_blocks, 1), 4, x, -y_offset, 2);
            text_table = CARDA_TEXT_TABLE(g_carda_text_not_enough_blocks, 1);
            prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 0x30), 4, x, 0x10 - y_offset, 2);
            prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 0x31), 4, x, 0x20 - y_offset, 2);
            if (g_pad_input & CARDA_PAD_CONFIRM)
            {
                play_menu_sfx(0x7D, 0x80);
                g_carda_entry_state = CARDA_SAVE_STATE_SELECT_SLOT;
                g_carda_choice_toggle = g_carda_card_slot;
                field_reset_input_repeat();
            }
        }
        break;

    case CARDA_SAVE_STATE_NO_CARD:
        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B0AE, 0x3B), 4, -x_offset + CARDA_SAVE_TEXT_X, -y_offset, 2);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, 0xE - y_offset);
        break;

    case CARDA_SAVE_STATE_ACCESS_FAILED:
        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B0B0, 0x3C), 4, -x_offset + CARDA_SAVE_TEXT_X, -y_offset, 2);
        break;

    case CARDA_SAVE_STATE_NO_SAVE_DATA:
        prim = carda_draw_mode3_notice(prim, ot, x_offset, y_offset, 0xE);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, 0x1C - y_offset);
        break;

    case CARDA_SAVE_STATE_UNUSED_NOTICE:
        prim = carda_draw_mode3_notice(prim, ot, x_offset, y_offset, 0x10);
        break;

    case CARDA_SAVE_STATE_STATUS_SAVE_FAILED:
        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B0D6, 0x4F), 4, -x_offset + CARDA_SAVE_TEXT_X, -y_offset, 2);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, 0xE - y_offset);
        break;

    case CARDA_SAVE_STATE_STATUS_LOAD_FAILED:
        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B0D8, 0x50), 4, -x_offset + CARDA_SAVE_TEXT_X, -y_offset, 2);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, 0xE - y_offset);
        break;

    case CARDA_SAVE_STATE_STATUS_CARD_INSERT_ERROR:
        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B0B6, 0x3F), 4, -x_offset + CARDA_SAVE_TEXT_X, -y_offset, 2);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, 0xE - y_offset);
        break;

    case CARDA_SAVE_STATE_STATUS_3:
        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B0AE, 0x3B), 4, -x_offset + CARDA_SAVE_TEXT_X, -y_offset, 2);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, 0xE - y_offset);
        break;

    case CARDA_SAVE_STATE_STATUS_4:
        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B094, 0x2E), 4, -x_offset + CARDA_SAVE_TEXT_X, -y_offset, 2);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, 0xE - y_offset);
        break;

    case CARDA_SAVE_STATE_STATUS_5:
        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B09C, 0x32), 4, -x_offset + CARDA_SAVE_TEXT_X, -y_offset, 2);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, 0xE - y_offset);
        break;

    case CARDA_SAVE_STATE_DUPLICATE_RECORD:
        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B0E6, 0x57), 4, -x_offset + CARDA_SAVE_TEXT_X, -y_offset, 2);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, 0xE - y_offset);
        break;

    case CARDA_SAVE_STATE_CONFIRM_OVERWRITE:
    {
        s32 x;
        s32 y;
        s32 choice_prim;
        u16* text_table;
        u8* choice_entry;
        u8* choice_table;
        s32 first_text;
        s32 second_text;
        s32 first_high;
        s32 color;

        x = -x_offset;
        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B0C2, 0x45), 4, x + CARDA_SAVE_TEXT_X, -y_offset, 2);
        text_table = CARDA_TEXT_TABLE(D_8014B0C2, 0x45);
        prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 0x55), 4, x + CARDA_SAVE_TEXT_X, 0xE - y_offset, 2);
        prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 0x56), 4, x + CARDA_SAVE_TEXT_X, 0x1C - y_offset, 2);
        y = 0x2A - y_offset;
        choice_entry = &g_text_choice_glyph_offsets;
        first_high = choice_entry[1] << 8;
        choice_table = choice_entry - 0x36;
        color = 4;
        first_text = choice_entry[0] + (first_high + (s32)choice_table);
        if (g_carda_choice_toggle != 0)
        {
            color = 5;
        }
        choice_prim = func_800A88A0(prim, ot, (void*)first_text, color, x + 0x80, y, 1);
        color = 4;
        second_text = choice_table[0x38] + ((choice_table[0x39] << 8) + (s32)choice_table);
        if (g_carda_choice_toggle == 0)
        {
            color = 5;
        }
        choice_prim = func_800A88A0(choice_prim, ot, (void*)second_text, color, x + 0x98, y, 0);
        if (g_pad_input & CARDA_PAD_LEFT_RIGHT)
        {
            g_carda_choice_toggle ^= 1;
            play_menu_sfx(0x7D, 0x80);
            g_pad_input = 0;
        }
        prim = choice_prim;
        if (g_pad_input & CARDA_PAD_CANCEL)
        {
            play_menu_sfx(0x7D, 0x80);
            g_carda_choice_toggle = g_carda_card_slot;
            g_carda_entry_state = CARDA_SAVE_STATE_SELECT_SLOT;
            field_reset_input_repeat();
            break;
        }
        if (g_pad_input & CARDA_PAD_CONFIRM)
        {
            if (g_carda_choice_toggle != 0)
            {
                play_menu_sfx(0x7D, 0x80);
                g_carda_choice_toggle = g_carda_card_slot;
                g_carda_entry_state = CARDA_SAVE_STATE_SELECT_SLOT;
                field_reset_input_repeat();
                break;
            }
            play_menu_sfx(0x7E, 0x80);
            g_carda_new_save_file = 0;
            g_carda_progress_start_tick = VSync(-1);
            g_carda_progress_active = 1;
            g_carda_save_step = g_carda_steps_read_save_prefix;
            g_carda_entry_state = CARDA_SAVE_STATE_CHECK_RECORDS;
        }
        break;
    }

    case CARDA_SAVE_STATE_CHECK_RECORDS:
    {
        s32 i;
        s32 x;
        s32 save_id;
        u16* text_table;
        CardaElement* element;

        x = -x_offset + CARDA_SAVE_TEXT_X;
        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B0CA, 0x49), 4, x, -y_offset, 2);
        text_table = CARDA_TEXT_TABLE(D_8014B0CA, 0x49);
        prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 0x53), 4, x, 0xE - y_offset, 2);
        prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 0x3D), 4, x, 0x1C - y_offset, 2);
        prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 0x59), 4, x, 0x2A - y_offset, 2);
        prim = carda_draw_progress_bar(prim, ot);
        if (g_carda_progress_active != 0)
        {
            break;
        }
        play_menu_sfx(0x7B, 0x80);
        g_carda_entry_state = CARDA_SAVE_STATE_CONFIRM_SAVE;
        D_80165F7C = 0;
        save_id = *(s32*)(g_carda_save_blob + CARDA_SAVE_BLOB_ID_OFFSET);
        for (i = 0; i < CARDA_RECORD_COUNT; i++)
        {
            u8* record;

            record = g_pad_ctx + i * CARDA_RECORD_SIZE;
            if (record[CARDA_RECORD_ARRAY_OFFSET] != 0 && *(s32*)(record + CARDA_RECORD_ID_OFFSET) == save_id)
            {
                D_80165F7C = 1;
                break;
            }
        }
        if (D_80165F7C != 0)
        {
            g_carda_entry_state = CARDA_SAVE_STATE_DUPLICATE_RECORD;
            g_carda_save_step = NULL;
            break;
        }
        if (g_carda_mode == 3)
        {
            for (D_801227C4 = 0; D_801227C4 < CARDA_RECORD_COUNT; D_801227C4++)
            {
                u8* record;

                record = g_pad_ctx + D_801227C4 * CARDA_RECORD_SIZE;
                if (record[CARDA_RECORD_ARRAY_OFFSET] == 0)
                {
                    break;
                }
            }
            g_field_card_overlay_mode = 5;
            if (D_801227C4 == CARDA_RECORD_COUNT)
            {
                g_field_card_overlay_mode = 7;
                g_menu_element_counter = 0x20;
                element = g_carda_element_pool;
                for (i = 0; i < 8; i++)
                {
                    element->attr.f.state = 0;
                    element++;
                }
                field_restore_fade_target_with_duration(8);
                break;
            }
            carda_apply_save_items();
            carda_restore_active_record();
            g_carda_selected_card_path = g_carda_save_card_path_prefix;
            g_gosub_result_values = D_801227C4;
            g_carda_selected_card_path.raw[2] += (u8)g_carda_card_slot;
            strcat(&g_carda_selected_card_path, g_lom_alt_save_filename_prefix);
            _card_wait(g_carda_card_slot);
            erase(&g_carda_selected_card_path);
            if (g_carda_received_item_count == 0)
            {
                CardaElement* element;

                g_menu_element_counter = 0x20;
                element = g_carda_element_pool;
                for (counter = 0; counter < 8; counter++)
                {
                    element->attr.f.state = 0;
                    element++;
                }
                field_restore_fade_target_with_duration(8);
                break;
            }
            carda_open_item_list();
            break;
        }
        g_carda_choice_toggle = 1;
        field_reset_input_repeat();
        break;
    }

    case CARDA_SAVE_STATE_SELECT_SLOT:
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, -y_offset);
        break;

    case CARDA_SAVE_STATE_CONFIRM_SAVE:
    {
        if (g_carda_new_save_file != 0)
        {
            s32 x;
            s32 y;
            s32 color;
            s32 first_text;
            s32 second_text;
            s32 first_high;
            s32 choice_prim;
            u8* choice_entry;
            u8* choice_table;
            u16* text_table;

            x = -x_offset;
            prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B0CC, 0x4A), 4, x + CARDA_SAVE_TEXT_X, -y_offset, 2);
            text_table = CARDA_TEXT_TABLE(D_8014B0CC, 0x4A);
            prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 0x4B), 4, x + CARDA_SAVE_TEXT_X, 0xE - y_offset, 2);
            color = 4;
            y = 0x1C - y_offset;
            choice_entry = &g_text_choice_glyph_offsets;
            first_high = choice_entry[1] << 8;
            choice_table = choice_entry - 0x36;
            first_text = choice_entry[0] + (first_high + (s32)choice_table);
            if (g_carda_choice_toggle != 0)
            {
                color = 5;
            }
            choice_prim = func_800A88A0(prim, ot, (void*)first_text, color, x + 0x80, y, 1);
            color = 4;
            second_text = choice_table[0x38] + ((choice_table[0x39] << 8) + (s32)choice_table);
            if (g_carda_choice_toggle == 0)
            {
                color = 5;
            }
            choice_prim = func_800A88A0(choice_prim, ot, (void*)second_text, color, x + 0x98, y, 0);
            if (g_pad_input & CARDA_PAD_LEFT_RIGHT)
            {
                g_carda_choice_toggle ^= 1;
                play_menu_sfx(0x7D, 0x80);
                g_pad_input = 0;
            }
            prim = choice_prim;
        }
        else
        {
            s32 x;
            s32 y;
            s32 color;
            s32 first_text;
            s32 second_text;
            s32 first_high;
            s32 choice_prim;
            u8* choice_entry;
            u8* choice_table;
            u16* text_table;

            x = -x_offset;
            prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B0C6, 0x47), 4, x + CARDA_SAVE_TEXT_X, -y_offset, 2);
            text_table = CARDA_TEXT_TABLE(D_8014B0C6, 0x47);
            prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 0x4A), 4, x + CARDA_SAVE_TEXT_X, 0xE - y_offset, 2);
            prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 0x4C), 4, x + CARDA_SAVE_TEXT_X, 0x1C - y_offset, 2);
            color = 4;
            y = 0x2A - y_offset;
            choice_entry = &g_text_choice_glyph_offsets;
            first_high = choice_entry[1] << 8;
            choice_table = choice_entry - 0x36;
            first_text = choice_entry[0] + (first_high + (s32)choice_table);
            if (g_carda_choice_toggle != 0)
            {
                color = 5;
            }
            choice_prim = func_800A88A0(prim, ot, (void*)first_text, color, x + 0x80, y, 1);
            color = 4;
            second_text = choice_table[0x38] + ((choice_table[0x39] << 8) + (s32)choice_table);
            if (g_carda_choice_toggle == 0)
            {
                color = 5;
            }
            choice_prim = func_800A88A0(choice_prim, ot, (void*)second_text, color, x + 0x98, y, 0);
            if (g_pad_input & CARDA_PAD_LEFT_RIGHT)
            {
                g_carda_choice_toggle ^= 1;
                play_menu_sfx(0x7D, 0x80);
                g_pad_input = 0;
            }
            prim = choice_prim;
        }
        if ((g_pad_input & CARDA_PAD_CANCEL) || ((g_pad_input & CARDA_PAD_CONFIRM) && g_carda_choice_toggle != 0))
        {
            play_menu_sfx(0x7D, 0x80);
            g_carda_entry_state = CARDA_SAVE_STATE_SELECT_SLOT;
            g_carda_choice_toggle = g_carda_card_slot;
            field_reset_input_repeat();
        }
        else if (g_pad_input & CARDA_PAD_CONFIRM)
        {
            play_menu_sfx(0x7E, 0x80);
            g_carda_received_item_count = 0;
            if (g_carda_new_save_file == 0)
            {
                carda_apply_save_items();
                g_gosub_result_values = D_801227C4;
            }
            else
            {
                g_gosub_result_values = CARDA_RECORD_COUNT;
            }
            carda_store_active_record();
            g_carda_save_in_progress = 1;
            g_carda_save_step = g_carda_steps_overwrite_alt_save;
            g_carda_entry_state = CARDA_SAVE_STATE_WRITING;
        }
        break;
    }

    case CARDA_SAVE_STATE_WRITING:
    {
        s32 x;
        s32 i;
        u16* text_table;
        CardaElement* element;

        x = -x_offset + CARDA_SAVE_TEXT_X;
        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B0DA, 0x51), 4, x, -y_offset, 2);
        text_table = CARDA_TEXT_TABLE(D_8014B0DA, 0x51);
        prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 0x3D), 4, x, 0xE - y_offset, 2);
        prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 0x59), 4, x, 0x1C - y_offset, 2);
        prim = carda_draw_progress_bar(prim, ot);
        if (g_carda_save_in_progress == 0)
        {
            g_field_card_overlay_mode = 4;
            play_menu_sfx(0x7A, 0x80);
            if (g_gosub_result_values == CARDA_RECORD_COUNT)
            {
                u8* record;

                record = g_pad_ctx + D_801227C4 * CARDA_RECORD_SIZE;
                record[CARDA_RECORD_ARRAY_OFFSET] = 0;
            }
            else
            {
                carda_restore_active_record();
            }
            if (g_carda_received_item_count != 0)
            {
                carda_open_item_list();
                break;
            }
            g_menu_element_counter = 0x20;
            element = g_carda_element_pool;
            for (i = 0; i < 8; i++)
            {
                element->attr.f.state = 0;
                element++;
            }
            field_restore_fade_target_with_duration(8);
        }
        break;
    }

    default:
    {
        s32 x;
        u16* text_table;

        x = -x_offset + CARDA_SAVE_TEXT_X;
        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B0AC, 0x3A), 4, x, -y_offset, 2);
        text_table = CARDA_TEXT_TABLE(D_8014B0AC, 0x3A);
        prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 0x3D), 4, x, 0xE - y_offset, 2);
        prim = func_800A88A0(prim, ot, CARDA_TEXT(text_table, 0x59), 4, x, 0x1C - y_offset, 2);
        /* Search one entry per frame, unless a directory scan (opcode 6 or 7) is running. */
        if (g_carda_entry_scan_active == 0 && g_carda_io_busy == 0 && (u32)(*g_carda_save_step - CARDA_STEP_SCAN_ENTRIES) >= 2U)
        {
            if (strncmp(g_lom_alt_save_filename_prefix, &g_carda_entries[g_carda_card_slot][g_carda_selected_row], 0xC) != 0)
            {
                s32 row_y;
                s32 delta;

                g_carda_selected_row++;
                if (g_carda_selected_row >= g_carda_entry_state)
                {
                    if (g_carda_mode == 3)
                    {
                        g_carda_entry_state = CARDA_SAVE_STATE_NO_LOM_SAVE_DATA;
                        break;
                    }
                    g_carda_selected_card_path = g_carda_save_card_path_prefix;
                    g_carda_new_save_file = 1;
                    g_carda_selected_card_path.raw[2] += (u8)g_carda_card_slot;
                    strcat(&g_carda_selected_card_path, g_lom_alt_save_filename_prefix);
                    carda_store_active_record();
                    g_carda_entry_state = CARDA_SAVE_STATE_CONFIRM_SAVE;
                    g_carda_choice_toggle = 1;
                    field_reset_input_repeat();
                    break;
                }
                carda_commit_selected_entry();
                row_y = g_carda_selected_row * CARDA_ENTRY_ROW_HEIGHT;
                delta = row_y - g_carda_scroll_y;
                if (delta >= 0x4B)
                {
                    g_carda_scroll_target_y = row_y - 0x46;
                    g_carda_scroll_frames = 4;
                }
                if (delta < 0)
                {
                    g_carda_scroll_target_y = row_y;
                    g_carda_scroll_frames = 4;
                }
                break;
            }
            if (g_carda_mode == 3)
            {
                g_carda_choice_toggle = 1;
                g_carda_new_save_file = 0;
                g_carda_progress_start_tick = VSync(-1);
                g_carda_progress_active = 1;
                g_carda_entry_state = CARDA_SAVE_STATE_CONFIRM_OVERWRITE;
            }
            else
            {
                g_carda_new_save_file = 0;
                g_carda_progress_start_tick = VSync(-1);
                g_carda_progress_active = 1;
                g_carda_save_step = g_carda_steps_read_save_prefix;
                g_carda_entry_state = CARDA_SAVE_STATE_CHECK_RECORDS;
            }
        }
        break;
    }

    case CARDA_SAVE_STATE_IDLE:
        break;
    }
    return prim;
}

/**
 * @brief Serialize the game state into the save buffer and copy the active record into it.
 */
void carda_store_active_record(void)
{
    cdrom_queue_read(0x5E2, g_carda_save_blob);
    cdrom_wait_queue_empty();
    card_resource_noop_hook(g_carda_save_blob, &CARDA_GAME_STATE->records[D_801227C4]);
}

/**
 * @brief Restore the active record from g_carda_saved_record_copy and apply the save's growth delta.
 */
void carda_restore_active_record(void)
{
    bcopy(g_carda_saved_record_copy, &CARDA_GAME_STATE->records[D_801227C4], sizeof(CardaSaveRecord));
    CARDA_GAME_STATE->records[D_801227C4].growth += g_carda_growth_delta;
    func_800C1230(D_801227C4);
}

/**
 * @brief Draw the card-slot prompt and handle switch, cancel and retry input.
 * @param prim GPU packet write cursor.
 * @param ot Ordering table receiving the text packets.
 * @param x Horizontal text anchor.
 * @param y Vertical text position.
 * @return GPU packet cursor after the prompt text.
 */
s32 carda_draw_slot_prompt(s32 prim, s32 *ot, s32 x, s32 y)
{
    CardaElement *element;
    s32 result;
    s32 i;

    result = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B0D2, 77), 4, x, y, 2);

    if (g_pad_input & 0xA000)
    {
        g_carda_entry_state = 0xF1;
        g_carda_card_slot ^= 1;
        play_menu_sfx(0x7D, 0x80);
        return result;
    }

    if (g_pad_input & 0x40)
    {
        switch (g_carda_mode)
        {
        case 2:
            g_field_card_overlay_mode = 6;
            break;
        case 3:
            g_field_card_overlay_mode = 7;
            break;
        default:
            g_field_card_overlay_mode = 3;
            break;
        }
        play_menu_sfx(0x78, 0x80);
        field_restore_fade_target();
        element = g_carda_element_pool;
        for (i = 0; i < 8; i++, element++)
        {
            if (element->attr.word & 7)
            {
                element->attr.word = (((element->attr.word & ~7) | 3) & ~0x78) | 0x40;
            }
        }
        return result;
    }

    if (g_pad_input & 0x220)
    {
        s32 slot;

        play_menu_sfx(0x7D, 0x80);
        slot = g_carda_card_slot;
        D_801660F8 = 0;
        g_carda_save_step = 0;
        g_carda_entry_state = 0xFF;
        g_carda_scroll_frames = 0;
        g_carda_scroll_target_y = 0;
        g_carda_scroll_y = 0;
        g_carda_selected_row = 0;
        g_carda_card_slot ^= 1;
        g_carda_selection_status = 0;
        /* Same reset as switching cards, but stay on the current slot. */
        g_carda_card_slot = slot;
        carda_reset_entry_ranks();
        carda_release_secondary_handles();
        carda_release_primary_handles();
        g_carda_progress_bar_active = 0;
        g_carda_entry_state = 0xFF;
        g_carda_save_step = g_carda_steps_initial_scan;
    }

    return result;
}

/**
 * @brief Reset the CARDA choice state and initialize its mode-specific element.
 * @param dialog_state Status-dialog state index.
 * @see matching: 100.00%
 */
void carda_open_save_status_dialog(s32 dialog_state)
{
    play_menu_sfx(0x78, 0x80);
    field_reset_input_repeat();
    g_carda_save_in_progress = 0;
    g_carda_progress_active = 0;
    g_carda_selection_status = 0;
    g_carda_io_busy = 0;
    carda_reset_entry_ranks();
    g_carda_save_step = 0;
    g_carda_dialog_state = dialog_state;

    if (g_carda_mode == 2 || g_carda_mode == 3)
    {
        switch (dialog_state)
        {
        case 0:
            g_carda_entry_state = 0xF0;
            break;
        case 1:
            g_carda_entry_state = 0xEF;
            break;
        case 2:
            g_carda_entry_state = 0xEE;
            break;
        case 3:
            g_carda_entry_state = 0xED;
            break;
        case 4:
            g_carda_entry_state = 0xEC;
            break;
        case 5:
            g_carda_entry_state = 0xEB;
            break;
        }
        g_carda_save_step = 0;
        return;
    }

    g_carda_element1_state.draw = (void *)carda_draw_save_status_dialog;
    g_carda_element1_state.attr.f.phase = 1;
    g_carda_element1_state.attr.f.state = 1;
    g_carda_element1_state.attr.f.x = 0x20;
    g_carda_element1_state.attr.f.y = 0x70;
    g_carda_element1_state.size.f.width_high = 1;
    g_carda_element1_state.size.f.height = 0x14;
    CARDA_SET_ELEMENT_WIDTH_LOW(&g_carda_element1_state, 0);
}

/**
 * @brief Draw the status message chosen by g_carda_dialog_state and close the menu on confirm.
 * @param ot Ordering table receiving the text packets.
 * @param prim GPU packet write cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return GPU packet cursor after the message.
 */
s32 carda_draw_save_status_dialog(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2]; /* never used, but the original stack frame reserves it */

    switch (g_carda_dialog_state)
    {
    case 0:
        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B074, 30), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 2:
        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B078, 32), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 3:
        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B07A, 33), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 1:
    case 4:
        prim = func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B076, 31), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    }
    if (g_pad_input & 0x220)
    {
        carda_save_close_elements();
        field_restore_fade_target_with_duration(8);
        field_reset_input_repeat();
    }
    return prim;
}

/**
 * @brief Close every element and open the item-list and its header windows.
 */
void carda_open_item_list(void)
{
    CardaElement *element;

    carda_save_close_elements();

    g_carda_element_pool[0].attr.f.state = 1;
    element = carda_save_alloc_element();
    element->draw = (void *)carda_draw_item_list;
    element->attr.f.phase = 2;
    element->attr.f.x = 0x20;
    element->attr.f.y = 0x36;
    element->size.f.width_high = 1;
    element->size.f.height = 0x90;
    CARDA_SET_ELEMENT_WIDTH_LOW(element, 0);

    element = carda_save_alloc_element();
    element->draw = (void *)carda_draw_item_list_header;
    element->attr.f.phase = 2;
    element->attr.f.x = 0x20;
    element->attr.f.y = 0x1A;
    element->size.f.width_high = 1;
    element->size.f.height = 0x10;
    g_carda_scroll_frames = 0;
    g_carda_scroll_target_y = 0;
    CARDA_SET_ELEMENT_WIDTH_LOW(element, 0);
    g_carda_scroll_y = 0;
    g_carda_element_pool[0].attr.f.state = 0;
}

/**
 * @brief Draw the item-list header text.
 * @param ot Ordering table receiving the text packets.
 * @param prim GPU packet write cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return GPU packet cursor after the text.
 */
s32 carda_draw_item_list_header(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2]; /* never used, but the original stack frame reserves it */
    return func_800A88A0(prim, ot, CARDA_TEXT_AT(D_8014B0BE, 67), 4, -x_offset + 0x80, -y_offset, 2);
}

/**
 * @brief Draw the visible rows of the received-item list and handle scrolling.
 * @param ot Ordering table receiving the text packets.
 * @param prim GPU packet write cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return GPU packet cursor after the list.
 */
s32 carda_draw_item_list(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    s32 row_y;
    s32 i;
    s32 result;
    s32 unused[2]; /* never used, but the original stack frame reserves it */

    result = prim;
    for (i = 0; i < g_carda_received_item_count; i++)
    {
        row_y = i * 14 - g_carda_scroll_y;
        if ((u32)(row_y + 13) < 157)
        {
            result = func_800A88A0(result, ot, (u8 *)D_8014B4D4 + D_8014B4D4[g_carda_received_item_ids[i]], 4, -x_offset + 0x80, row_y - y_offset, 2);
        }
    }

    if (g_carda_scroll_frames == 0)
    {
        if (g_pad_input & 0x1000)
        {
            if (g_carda_scroll_y != 0)
            {
                play_menu_sfx(0x7D, 0x80);
                g_carda_scroll_frames = 4;
                g_carda_scroll_target_y -= 14;
            }
        }
        else if (g_pad_input & 0x4000)
        {
            if ((g_carda_received_item_count * 14 - g_carda_scroll_y) >= 141)
            {
                play_menu_sfx(0x7D, 0x80);
                g_carda_scroll_frames = 4;
                g_carda_scroll_target_y += 14;
            }
        }
        else if (g_pad_input & 0x220)
        {
            play_menu_sfx(0x7E, 0x80);
            carda_save_close_elements();
            field_restore_fade_target_with_duration(8);
        }
    }
    return result;
}

/**
 * @brief Copy the save's record into g_carda_saved_record_copy and add the save's items to the game state.
 * @note Items already at CARDA_ITEM_COUNT_MAX are skipped; the ids that were added
 *       are listed in g_carda_received_item_ids and counted in g_carda_received_item_count.
 * @see (100%)
 */
void carda_apply_save_items(void)
{
    CardaSaveData *save;
    CardaSaveItemList *list;
    u32 i;
    u8 count;

    save = CARDA_SAVE_DATA;
    list = &save->items;
    g_carda_received_item_count = 0;
    bcopy(&save->record, g_carda_saved_record_copy, sizeof(CardaSaveRecord));
    g_carda_received_item_count = 0;
    g_carda_growth_delta = list->growth_delta;
    for (i = 0; i < list->count; i++)
    {
        count = CARDA_GAME_STATE->item_counts[list->ids[i]];
        if (count < CARDA_ITEM_COUNT_MAX)
        {
            CARDA_GAME_STATE->item_counts[list->ids[i]] = count + 1;
            g_carda_received_item_ids[g_carda_received_item_count] = list->ids[i];
            g_carda_received_item_count++;
        }
    }
}
