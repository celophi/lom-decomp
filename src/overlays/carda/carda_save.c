#include "carda_internal.h"

/** @brief Left edge of the save window text, before the transition offset. */
#define CARDA_SAVE_TEXT_X 0x90

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
    prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_ring_ring_land_was, 0x41), 4, x, -y_offset, 2);
    text_table = CARDA_TEXT_TABLE(g_carda_text_ring_ring_land_was, 0x41);
    return field_draw_text(prim, ot, CARDA_TEXT(text_table, 0x42), 4, x, second_y - y_offset, 2);
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
 * @note Dispatches on g_card_entry_state (g_card_entry_state): below CARDA_ENTRY_STATE_PET_ALREADY_ON_RANCH
 *       it holds the directory entry count while the card is searched for an existing
 *       save; above it, the dialog currently shown. Dialog states also read the pad,
 *       move to the next state and start card sequences through g_card_step.
 * @note JP changes this function; the JP build takes it from assembly.
 */
#if defined(VERSION_JP)
INCLUDE_ASM("overlays/carda/nonmatchings/carda_save", carda_draw_save_flow);
#else
s32 carda_draw_save_flow(s32* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2]; /* never used, but the original stack frame reserves it */
    s32 counter;

    if (g_carda_element_pool[0].attr.bits.state != 0)
    {
        /* The message states draw nothing while element 0 is active. */
        switch (g_card_entry_state)
        {
        case CARDA_ENTRY_STATE_PET_ALREADY_ON_RANCH:
        case CARDA_ENTRY_STATE_FORMAT_FAILED:
        case CARDA_ENTRY_STATE_SAVE_CORRUPT:
        case CARDA_ENTRY_STATE_NO_POCKETSTATION:
        case CARDA_ENTRY_STATE_POCKETSTATION_NOT_INSERTED:
        case CARDA_ENTRY_STATE_UPLOAD_FAILED:
        case CARDA_ENTRY_STATE_DOWNLOAD_FAILED:
        case CARDA_ENTRY_STATE_NO_GAME_DATA:
        case CARDA_ENTRY_STATE_UNFORMATTED:
        case CARDA_ENTRY_STATE_CARD_FULL:
        case CARDA_ENTRY_STATE_ACCESS_FAILED:
        case CARDA_ENTRY_STATE_NO_SAVE_DATA:
        case CARDA_ENTRY_STATE_NO_CARD:
        case CARDA_ENTRY_STATE_CHECKING_CARD:
            return prim;
        }
    }

    switch (g_card_entry_state)
    {
    case CARDA_ENTRY_STATE_NO_GAME_DATA:
        prim = carda_draw_mode3_notice(prim, ot, x_offset, y_offset, 0xE);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, 0x1C - y_offset);
        break;

    case CARDA_ENTRY_STATE_UNFORMATTED:
        if (g_carda_mode == 3)
        {
            prim = carda_draw_mode3_notice(prim, ot, x_offset, y_offset, 0xE);
            prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, 0x1C - y_offset);
        }
        else
        {
            prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_card_unformatted, CARDA_TEXT_CARD_UNFORMATTED), 4, -x_offset + CARDA_SAVE_TEXT_X,
                                   -y_offset, 2);
            prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, 0xE - y_offset);
        }
        break;

    case CARDA_ENTRY_STATE_NOT_POCKETSTATION:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_not_pocketstation, 0x21), 4, -x_offset + CARDA_SAVE_TEXT_X, -y_offset, 2);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, 0xE - y_offset);
        break;

    case CARDA_ENTRY_STATE_CHECKING_CARD:
    {
        s32 x;
        u16* text_table;

        x = -x_offset + CARDA_SAVE_TEXT_X;
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_checking_pocketstation, 0x3A), 4, x, -y_offset, 2);
        text_table = CARDA_TEXT_TABLE(g_carda_text_checking_pocketstation, 0x3A);
        prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, 0x3D), 4, x, 0xE - y_offset, 2);
        prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, 0x59), 4, x, 0x1C - y_offset, 2);
        break;
    }

    case CARDA_ENTRY_STATE_CARD_FULL:
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
            prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_not_enough_blocks, 1), 4, x, -y_offset, 2);
            text_table = CARDA_TEXT_TABLE(g_carda_text_not_enough_blocks, 1);
            prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, 0x2D), 4, x, 0xE - y_offset, 2);
            prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, 0x1C - y_offset);
        }
        break;

    case CARDA_ENTRY_STATE_NO_ROOM_FOR_DOWNLOAD:
        if (g_carda_mode == 3)
        {
            prim = carda_draw_mode3_notice(prim, ot, x_offset, y_offset, 0x10);
            g_card_entry_state = CARDA_ENTRY_STATE_CARD_FULL;
        }
        else
        {
            s32 x;
            u16* text_table;

            x = -x_offset + CARDA_SAVE_TEXT_X;
            prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_not_enough_blocks, 1), 4, x, -y_offset, 2);
            text_table = CARDA_TEXT_TABLE(g_carda_text_not_enough_blocks, 1);
            prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, 0x30), 4, x, 0x10 - y_offset, 2);
            prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, 0x31), 4, x, 0x20 - y_offset, 2);
            if (g_pad_input & CARDA_CONFIRM_BUTTON_MASK)
            {
                field_play_sound(0x7D, 0x80);
                g_card_entry_state = CARDA_ENTRY_STATE_SELECT_SLOT;
                g_carda_choice_toggle = g_card_slot;
                field_reset_input_repeat();
            }
        }
        break;

    case CARDA_ENTRY_STATE_NO_CARD:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_no_pocketstation, 0x3B), 4, -x_offset + CARDA_SAVE_TEXT_X, -y_offset, 2);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, 0xE - y_offset);
        break;

    case CARDA_ENTRY_STATE_ACCESS_FAILED:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_pocketstation_access_failed, 0x3C), 4, -x_offset + CARDA_SAVE_TEXT_X, -y_offset, 2);
        break;

    case CARDA_ENTRY_STATE_NO_SAVE_DATA:
        prim = carda_draw_mode3_notice(prim, ot, x_offset, y_offset, 0xE);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, 0x1C - y_offset);
        break;

    case CARDA_ENTRY_STATE_NO_RING_RING_LAND:
        prim = carda_draw_mode3_notice(prim, ot, x_offset, y_offset, 0x10);
        break;

    case CARDA_ENTRY_STATE_DOWNLOAD_FAILED:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_download_failed, 0x4F), 4, -x_offset + CARDA_SAVE_TEXT_X, -y_offset, 2);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, 0xE - y_offset);
        break;

    case CARDA_ENTRY_STATE_UPLOAD_FAILED:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_upload_failed, 0x50), 4, -x_offset + CARDA_SAVE_TEXT_X, -y_offset, 2);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, 0xE - y_offset);
        break;

    case CARDA_ENTRY_STATE_POCKETSTATION_NOT_INSERTED:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_pocketstation_not_inserted, 0x3F), 4, -x_offset + CARDA_SAVE_TEXT_X, -y_offset, 2);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, 0xE - y_offset);
        break;

    case CARDA_ENTRY_STATE_NO_POCKETSTATION:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_no_pocketstation, 0x3B), 4, -x_offset + CARDA_SAVE_TEXT_X, -y_offset, 2);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, 0xE - y_offset);
        break;

    case CARDA_ENTRY_STATE_SAVE_CORRUPT:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_save_corrupt, 0x2E), 4, -x_offset + CARDA_SAVE_TEXT_X, -y_offset, 2);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, 0xE - y_offset);
        break;

    case CARDA_ENTRY_STATE_FORMAT_FAILED:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_format_failed, 0x32), 4, -x_offset + CARDA_SAVE_TEXT_X, -y_offset, 2);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, 0xE - y_offset);
        break;

    case CARDA_ENTRY_STATE_PET_ALREADY_ON_RANCH:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_pet_already_on_ranch, 0x57), 4, -x_offset + CARDA_SAVE_TEXT_X, -y_offset, 2);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, 0xE - y_offset);
        break;

    case CARDA_ENTRY_STATE_CONFIRM_RETURN:
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
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_return_pet, 0x45), 4, x + CARDA_SAVE_TEXT_X, -y_offset, 2);
        text_table = CARDA_TEXT_TABLE(g_carda_text_return_pet, 0x45);
        prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, 0x55), 4, x + CARDA_SAVE_TEXT_X, 0xE - y_offset, 2);
        prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, 0x56), 4, x + CARDA_SAVE_TEXT_X, 0x1C - y_offset, 2);
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
        choice_prim = field_draw_text(prim, ot, (void*)first_text, color, x + 0x80, y, 1);
        color = 4;
        second_text = choice_table[0x38] + ((choice_table[0x39] << 8) + (s32)choice_table);
        if (g_carda_choice_toggle == 0)
        {
            color = 5;
        }
        choice_prim = field_draw_text(choice_prim, ot, (void*)second_text, color, x + 0x98, y, 0);
        if (g_pad_input & CARDA_CHOICE_BUTTON_MASK)
        {
            g_carda_choice_toggle ^= 1;
            field_play_sound(0x7D, 0x80);
            g_pad_input = 0;
        }
        prim = choice_prim;
        if (g_pad_input & PAD_BTN_CIRCLE)
        {
            field_play_sound(0x7D, 0x80);
            g_carda_choice_toggle = g_card_slot;
            g_card_entry_state = CARDA_ENTRY_STATE_SELECT_SLOT;
            field_reset_input_repeat();
            break;
        }
        if (g_pad_input & CARDA_CONFIRM_BUTTON_MASK)
        {
            if (g_carda_choice_toggle != 0)
            {
                field_play_sound(0x7D, 0x80);
                g_carda_choice_toggle = g_card_slot;
                g_card_entry_state = CARDA_ENTRY_STATE_SELECT_SLOT;
                field_reset_input_repeat();
                break;
            }
            field_play_sound(0x7E, 0x80);
            g_carda_new_save_file = 0;
            g_carda_progress_start_tick = VSync(-1);
            g_carda_progress_active = 1;
            g_card_step = g_carda_steps_read_save_prefix;
            g_card_entry_state = CARDA_ENTRY_STATE_UPLOADING;
        }
        break;
    }

    case CARDA_ENTRY_STATE_UPLOADING:
    {
        s32 i;
        s32 x;
        s32 save_id;
        u16* text_table;
        CardaElement* element;

        x = -x_offset + CARDA_SAVE_TEXT_X;
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_game_from_pocketstation, 0x49), 4, x, -y_offset, 2);
        text_table = CARDA_TEXT_TABLE(g_carda_text_game_from_pocketstation, 0x49);
        prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, 0x53), 4, x, 0xE - y_offset, 2);
        prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, 0x3D), 4, x, 0x1C - y_offset, 2);
        prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, 0x59), 4, x, 0x2A - y_offset, 2);
        prim = carda_draw_progress_bar(prim, ot);
        if (g_carda_progress_active != 0)
        {
            break;
        }
        field_play_sound(0x7B, 0x80);
        g_card_entry_state = CARDA_ENTRY_STATE_CONFIRM_DOWNLOAD;
        g_carda_pet_already_on_ranch = 0;
        save_id = CARDA_SAVE_DATA->record.unique_id;
        for (i = 0; i < PET_RECORD_COUNT; i++)
        {
            if (g_saved_game_ctx->pets[i].name[0] != 0 && g_saved_game_ctx->pets[i].unique_id == save_id)
            {
                g_carda_pet_already_on_ranch = 1;
                break;
            }
        }
        if (g_carda_pet_already_on_ranch != 0)
        {
            g_card_entry_state = CARDA_ENTRY_STATE_PET_ALREADY_ON_RANCH;
            g_card_step = NULL;
            break;
        }
        if (g_carda_mode == 3)
        {
            for (g_field_card_pet_slot = 0; g_field_card_pet_slot < PET_RECORD_COUNT; g_field_card_pet_slot++)
            {
                if (g_saved_game_ctx->pets[g_field_card_pet_slot].name[0] == 0)
                {
                    break;
                }
            }
            g_field_card_overlay_mode = 5;
            if (g_field_card_pet_slot == PET_RECORD_COUNT)
            {
                g_field_card_overlay_mode = 7;
                g_menu_element_counter = 0x20;
                element = g_carda_element_pool;
                for (i = 0; i < 8; i++)
                {
                    element->attr.bits.state = 0;
                    element++;
                }
                field_restore_fade_target_with_duration(8);
                break;
            }
            carda_apply_save_items();
            carda_restore_active_record();
            g_carda_selected_card_path = g_carda_save_card_path_prefix;
            g_gosub_result_values = g_field_card_pet_slot;
            g_carda_selected_card_path.raw[2] += (u8)g_card_slot;
            strcat(&g_carda_selected_card_path, g_lom_pocketstation_filename_prefix);
            _card_wait(g_card_slot);
            erase(&g_carda_selected_card_path);
            if (g_carda_received_item_count == 0)
            {
                CardaElement* element;

                g_menu_element_counter = 0x20;
                element = g_carda_element_pool;
                for (counter = 0; counter < 8; counter++)
                {
                    element->attr.bits.state = 0;
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

    case CARDA_ENTRY_STATE_SELECT_SLOT:
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, -y_offset);
        break;

    case CARDA_ENTRY_STATE_CONFIRM_DOWNLOAD:
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
            prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_game_to_pocketstation, 0x4A), 4, x + CARDA_SAVE_TEXT_X, -y_offset, 2);
            text_table = CARDA_TEXT_TABLE(g_carda_text_game_to_pocketstation, 0x4A);
            prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, 0x4B), 4, x + CARDA_SAVE_TEXT_X, 0xE - y_offset, 2);
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
            choice_prim = field_draw_text(prim, ot, (void*)first_text, color, x + 0x80, y, 1);
            color = 4;
            second_text = choice_table[0x38] + ((choice_table[0x39] << 8) + (s32)choice_table);
            if (g_carda_choice_toggle == 0)
            {
                color = 5;
            }
            choice_prim = field_draw_text(choice_prim, ot, (void*)second_text, color, x + 0x98, y, 0);
            if (g_pad_input & CARDA_CHOICE_BUTTON_MASK)
            {
                g_carda_choice_toggle ^= 1;
                field_play_sound(0x7D, 0x80);
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
            prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_swap_pets, 0x47), 4, x + CARDA_SAVE_TEXT_X, -y_offset, 2);
            text_table = CARDA_TEXT_TABLE(g_carda_text_swap_pets, 0x47);
            prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, 0x4A), 4, x + CARDA_SAVE_TEXT_X, 0xE - y_offset, 2);
            prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, 0x4C), 4, x + CARDA_SAVE_TEXT_X, 0x1C - y_offset, 2);
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
            choice_prim = field_draw_text(prim, ot, (void*)first_text, color, x + 0x80, y, 1);
            color = 4;
            second_text = choice_table[0x38] + ((choice_table[0x39] << 8) + (s32)choice_table);
            if (g_carda_choice_toggle == 0)
            {
                color = 5;
            }
            choice_prim = field_draw_text(choice_prim, ot, (void*)second_text, color, x + 0x98, y, 0);
            if (g_pad_input & CARDA_CHOICE_BUTTON_MASK)
            {
                g_carda_choice_toggle ^= 1;
                field_play_sound(0x7D, 0x80);
                g_pad_input = 0;
            }
            prim = choice_prim;
        }
        if ((g_pad_input & PAD_BTN_CIRCLE) || ((g_pad_input & CARDA_CONFIRM_BUTTON_MASK) && g_carda_choice_toggle != 0))
        {
            field_play_sound(0x7D, 0x80);
            g_card_entry_state = CARDA_ENTRY_STATE_SELECT_SLOT;
            g_carda_choice_toggle = g_card_slot;
            field_reset_input_repeat();
        }
        else if (g_pad_input & CARDA_CONFIRM_BUTTON_MASK)
        {
            field_play_sound(0x7E, 0x80);
            g_carda_received_item_count = 0;
            if (g_carda_new_save_file == 0)
            {
                carda_apply_save_items();
                g_gosub_result_values = g_field_card_pet_slot;
            }
            else
            {
                g_gosub_result_values = PET_RECORD_COUNT;
            }
            carda_store_active_record();
            g_carda_save_in_progress = 1;
            g_card_step = g_carda_steps_overwrite_alt_save;
            g_card_entry_state = CARDA_ENTRY_STATE_DOWNLOADING;
        }
        break;
    }

    case CARDA_ENTRY_STATE_DOWNLOADING:
    {
        s32 x;
        s32 i;
        u16* text_table;
        CardaElement* element;

        x = -x_offset + CARDA_SAVE_TEXT_X;
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_downloading, 0x51), 4, x, -y_offset, 2);
        text_table = CARDA_TEXT_TABLE(g_carda_text_downloading, 0x51);
        prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, 0x3D), 4, x, 0xE - y_offset, 2);
        prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, 0x59), 4, x, 0x1C - y_offset, 2);
        prim = carda_draw_progress_bar(prim, ot);
        if (g_carda_save_in_progress == 0)
        {
            g_field_card_overlay_mode = 4;
            field_play_sound(0x7A, 0x80);
            if (g_gosub_result_values == PET_RECORD_COUNT)
            {
                g_saved_game_ctx->pets[g_field_card_pet_slot].name[0] = 0;
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
                element->attr.bits.state = 0;
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
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_checking_pocketstation, 0x3A), 4, x, -y_offset, 2);
        text_table = CARDA_TEXT_TABLE(g_carda_text_checking_pocketstation, 0x3A);
        prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, 0x3D), 4, x, 0xE - y_offset, 2);
        prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, 0x59), 4, x, 0x1C - y_offset, 2);
        /* Search one entry per frame, unless a directory scan (opcode 6 or 7) is running. */
        if (g_carda_entry_scan_active == 0 && g_carda_io_busy == 0 && (u32)(*g_card_step - CARDA_STEP_SCAN_ENTRIES) >= 2U)
        {
            if (strncmp(g_lom_pocketstation_filename_prefix, &g_card_entries[g_card_slot][g_carda_selected_row], 0xC) != 0)
            {
                s32 row_y;
                s32 delta;

                g_carda_selected_row++;
                if (g_carda_selected_row >= g_card_entry_state)
                {
                    if (g_carda_mode == 3)
                    {
                        g_card_entry_state = CARDA_ENTRY_STATE_NO_GAME_DATA;
                        break;
                    }
                    g_carda_selected_card_path = g_carda_save_card_path_prefix;
                    g_carda_new_save_file = 1;
                    g_carda_selected_card_path.raw[2] += (u8)g_card_slot;
                    strcat(&g_carda_selected_card_path, g_lom_pocketstation_filename_prefix);
                    carda_store_active_record();
                    g_card_entry_state = CARDA_ENTRY_STATE_CONFIRM_DOWNLOAD;
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
                g_card_entry_state = CARDA_ENTRY_STATE_CONFIRM_RETURN;
            }
            else
            {
                g_carda_new_save_file = 0;
                g_carda_progress_start_tick = VSync(-1);
                g_carda_progress_active = 1;
                g_card_step = g_carda_steps_read_save_prefix;
                g_card_entry_state = CARDA_ENTRY_STATE_UPLOADING;
            }
        }
        break;
    }

    case CARDA_ENTRY_STATE_BLANK:
        break;
    }
    return prim;
}
#endif

/**
 * @brief Serialize the game state into the save buffer and copy the active record into it.
 */
void carda_store_active_record(void)
{
    cdrom_queue_read(0x5E2, g_carda_save_blob);
    cdrom_wait_queue_empty();
    card_resource_noop_hook(g_carda_save_blob, &g_saved_game_ctx->pets[g_field_card_pet_slot]);
}

/**
 * @brief Restore the active record from g_carda_saved_record_copy and apply the save's growth delta.
 */
void carda_restore_active_record(void)
{
    bcopy(g_carda_saved_record_copy, (u8*)&g_saved_game_ctx->pets[g_field_card_pet_slot], sizeof(PetRecord));
    g_saved_game_ctx->pets[g_field_card_pet_slot].progress.bits.experience += g_carda_growth_delta;
    field_apply_region_level_ups(g_field_card_pet_slot);
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

    result = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_select_slot, 77), 4, x, y, 2);

    if (g_pad_input & 0xA000)
    {
        g_card_entry_state = 0xF1;
        g_card_slot ^= 1;
        field_play_sound(0x7D, 0x80);
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
        field_play_sound(0x78, 0x80);
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

        field_play_sound(0x7D, 0x80);
        slot = g_card_slot;
        g_carda_format_declined = 0;
        g_card_step = 0;
        g_card_entry_state = 0xFF;
        g_carda_scroll_frames = 0;
        g_carda_scroll_target_y = 0;
        g_carda_scroll_y = 0;
        g_carda_selected_row = 0;
        g_card_slot ^= 1;
        g_carda_selection_status = 0;
        /* Same reset as switching cards, but stay on the current slot. */
        g_card_slot = slot;
        carda_reset_entry_ranks();
        clear_hardware_card_events();
        clear_software_card_events();
        g_carda_progress_bar_active = 0;
        g_card_entry_state = 0xFF;
        g_card_step = g_carda_steps_initial_scan;
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
    field_play_sound(0x78, 0x80);
    field_reset_input_repeat();
    g_carda_save_in_progress = 0;
    g_carda_progress_active = 0;
    g_carda_selection_status = 0;
    g_carda_io_busy = 0;
    carda_reset_entry_ranks();
    g_card_step = 0;
    g_carda_dialog_state = dialog_state;

    if (g_carda_mode == 2 || g_carda_mode == 3)
    {
        switch (dialog_state)
        {
        case 0:
            g_card_entry_state = 0xF0;
            break;
        case 1:
            g_card_entry_state = 0xEF;
            break;
        case 2:
            g_card_entry_state = 0xEE;
            break;
        case 3:
            g_card_entry_state = 0xED;
            break;
        case 4:
            g_card_entry_state = 0xEC;
            break;
        case 5:
            g_card_entry_state = 0xEB;
            break;
        }
        g_card_step = 0;
        return;
    }

    g_carda_element1_state.draw = (void *)carda_draw_save_status_dialog;
    g_carda_element1_state.attr.bits.transition_step = 1;
    g_carda_element1_state.attr.bits.state = 1;
    g_carda_element1_state.attr.bits.x = 0x20;
    g_carda_element1_state.attr.bits.y = 0x70;
    g_carda_element1_state.size.bits.width_high = 1;
    g_carda_element1_state.size.bits.height = 0x14;
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
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_save_failed, 30), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 2:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_card_not_inserted, 32), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 3:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_not_pocketstation, 33), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 1:
    case 4:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_load_failed, 31), 4, -x_offset + 0x80, -y_offset, 2);
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

    g_carda_element_pool[0].attr.bits.state = 1;
    element = carda_save_alloc_element();
    element->draw = (void *)carda_draw_item_list;
    element->attr.bits.transition_step = 2;
    element->attr.bits.x = 0x20;
    element->attr.bits.y = 0x36;
    element->size.bits.width_high = 1;
    element->size.bits.height = 0x90;
    CARDA_SET_ELEMENT_WIDTH_LOW(element, 0);

    element = carda_save_alloc_element();
    element->draw = (void *)carda_draw_item_list_header;
    element->attr.bits.transition_step = 2;
    element->attr.bits.x = 0x20;
    element->attr.bits.y = 0x1A;
    element->size.bits.width_high = 1;
    element->size.bits.height = 0x10;
    g_carda_scroll_frames = 0;
    g_carda_scroll_target_y = 0;
    CARDA_SET_ELEMENT_WIDTH_LOW(element, 0);
    g_carda_scroll_y = 0;
    g_carda_element_pool[0].attr.bits.state = 0;
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
    return field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_received_items, 67), 4, -x_offset + 0x80, -y_offset, 2);
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
            result = field_draw_text(result, ot, (u8*)g_carda_item_names + g_carda_item_names[g_carda_received_item_ids[i]], 4, -x_offset + 0x80,
                                     row_y - y_offset, 2);
        }
    }

    if (g_carda_scroll_frames == 0)
    {
        if (g_pad_input & 0x1000)
        {
            if (g_carda_scroll_y != 0)
            {
                field_play_sound(0x7D, 0x80);
                g_carda_scroll_frames = 4;
                g_carda_scroll_target_y -= 14;
            }
        }
        else if (g_pad_input & 0x4000)
        {
            if ((g_carda_received_item_count * 14 - g_carda_scroll_y) >= 141)
            {
                field_play_sound(0x7D, 0x80);
                g_carda_scroll_frames = 4;
                g_carda_scroll_target_y += 14;
            }
        }
        else if (g_pad_input & 0x220)
        {
            field_play_sound(0x7E, 0x80);
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
    bcopy((u8*)&save->record, g_carda_saved_record_copy, sizeof(PetRecord));
    g_carda_received_item_count = 0;
    g_carda_growth_delta = list->growth_delta;
    for (i = 0; i < list->count; i++)
    {
        count = g_saved_game_ctx->item_counts[list->ids[i]];
        if (count < CARDA_ITEM_COUNT_MAX)
        {
            g_saved_game_ctx->item_counts[list->ids[i]] = count + 1;
            g_carda_received_item_ids[g_carda_received_item_count] = list->ids[i];
            g_carda_received_item_count++;
        }
    }
}
