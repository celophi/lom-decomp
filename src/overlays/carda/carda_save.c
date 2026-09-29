#include "carda_save.h"
#include "carda_internal.h"

/** @brief Left edge of the save window text, before the transition offset. */
#define CARDA_SAVE_TEXT_X (CARDA_MESSAGE_WIDTH / 2)

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
    PetRecord record; /**< Pet restored into the saved game. */
} CardaSaveData;

/**
 * @brief The pet-transfer file in the save buffer.
 * @note g_carda_save_blob is a byte pointer because the buffer holds either a
 *       save file (SaveFile) or a pet-transfer file (CardaSaveData).
 */
#define CARDA_SAVE_DATA ((CardaSaveData*)g_carda_save_blob)

static void carda_restore_active_record(void);
static void* carda_draw_slot_prompt(void* prim, u_long* ot, s32 x, s32 y);
static void* carda_draw_save_status_dialog(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
static void carda_open_item_list(void);
static void* carda_draw_item_list_header(u_long* ot, void* prim, s32 x_offset, s32 y_offset);
static void carda_apply_save_items(void);

/**
 * @brief Draw the two-line notice shared by the mode 3 error states (texts 0x41 and 0x42).
 * @param prim Primitive-buffer cursor.
 * @param ot Ordering-table head.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @param second_y Baseline of the second line, before the transition offset.
 * @return Advanced primitive-buffer cursor.
 */
static inline void* carda_draw_mode3_notice(void* prim, u_long* ot, s32 x_offset, s32 y_offset, s32 second_y)
{
    s32 x;
    u16* text_table;

    x = -x_offset + CARDA_SAVE_TEXT_X;
    prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_ring_ring_land_was, CARDA_TEXT_RING_RING_LAND_WAS), FIELD_TEXT_COLOR_NORMAL, x, -y_offset,
                           FIELD_TEXT_ALIGN_CENTER);
    text_table = CARDA_TEXT_TABLE(g_carda_text_ring_ring_land_was, CARDA_TEXT_RING_RING_LAND_WAS);
    return field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_NOT_FOUND), FIELD_TEXT_COLOR_NORMAL, x, second_y - y_offset, FIELD_TEXT_ALIGN_CENTER);
}

/**
 * @brief Free every UI element and reset the window counter.
 * @note Releases windows immediately without a closing transition.
 */
static inline void carda_save_clear_elements(void)
{
    CardaElement* element;
    s32 i;

    g_menu_element_counter = 0x20;
    element = g_carda_element_pool;
    for (i = 0; i < CARDA_ELEMENT_COUNT; i++)
    {
        element->attr.bits.state = CARDA_ELEMENT_FREE;
        element++;
    }
}

/**
 * @brief Claim the first free UI element.
 * @return The claimed element, or the pool head when every element is busy.
 */
static inline CardaElement* carda_save_alloc_element(void)
{
    CardaElement* element;
    s32 i;

    element = g_carda_element_pool;
    for (i = 0; i < CARDA_ELEMENT_COUNT; i++, element++)
    {
        if (element->attr.bits.state == CARDA_ELEMENT_FREE)
        {
            element->attr.bits.state = CARDA_ELEMENT_OPENING;
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
 * @note Dispatches on g_card_entry_state: below CARDA_ENTRY_STATE_PET_ALREADY_ON_RANCH
 *       it holds the directory entry count while the card is searched for an existing
 *       save; above it, the dialog currently shown. Dialog states also read the pad,
 *       move to the next state and start card sequences through g_card_step.
 */
void* carda_draw_save_flow(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2];
    s32 counter;

    if (g_carda_element_pool[CARDA_ELEMENT_MODAL].attr.bits.state != CARDA_ELEMENT_FREE)
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
        prim = carda_draw_mode3_notice(prim, ot, x_offset, y_offset, CARDA_TEXT_LINE_HEIGHT);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, CARDA_TEXT_LINE_HEIGHT * 2 - y_offset);
        break;

    case CARDA_ENTRY_STATE_UNFORMATTED:
        if (g_carda_mode == CARDA_MODE_RETURN_PET)
        {
            prim = carda_draw_mode3_notice(prim, ot, x_offset, y_offset, CARDA_TEXT_LINE_HEIGHT);
            prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, CARDA_TEXT_LINE_HEIGHT * 2 - y_offset);
        }
        else
        {
            prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_card_unformatted, CARDA_TEXT_CARD_UNFORMATTED), FIELD_TEXT_COLOR_NORMAL,
                                   -x_offset + CARDA_SAVE_TEXT_X, -y_offset, FIELD_TEXT_ALIGN_CENTER);
            prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, CARDA_TEXT_LINE_HEIGHT - y_offset);
        }
        break;

    case CARDA_ENTRY_STATE_NOT_POCKETSTATION:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_not_pocketstation, CARDA_TEXT_NOT_POCKETSTATION), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_SAVE_TEXT_X, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, CARDA_TEXT_LINE_HEIGHT - y_offset);
        break;

    case CARDA_ENTRY_STATE_CHECKING_CARD:
    {
        s32 x;
        u16* text_table;

        x = -x_offset + CARDA_SAVE_TEXT_X;
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_checking_pocketstation, CARDA_TEXT_CHECKING_POCKETSTATION), FIELD_TEXT_COLOR_NORMAL, x,
                               -y_offset, FIELD_TEXT_ALIGN_CENTER);
        text_table = CARDA_TEXT_TABLE(g_carda_text_checking_pocketstation, CARDA_TEXT_CHECKING_POCKETSTATION);
        prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_POCKETSTATION_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x,
                               CARDA_TEXT_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x,
                               CARDA_TEXT_LINE_HEIGHT * 2 - y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    }

    case CARDA_ENTRY_STATE_CARD_FULL:
        if (g_carda_mode == CARDA_MODE_RETURN_PET)
        {
            prim = carda_draw_mode3_notice(prim, ot, x_offset, y_offset, CARDA_TEXT_LINE_HEIGHT);
            prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, CARDA_TEXT_LINE_HEIGHT * 2 - y_offset);
        }
        else
        {
            s32 x;
            u16* text_table;

            x = -x_offset + CARDA_SAVE_TEXT_X;
            prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_not_enough_blocks, CARDA_TEXT_NOT_ENOUGH_BLOCKS), FIELD_TEXT_COLOR_NORMAL, x, -y_offset,
                                   FIELD_TEXT_ALIGN_CENTER);
            text_table = CARDA_TEXT_TABLE(g_carda_text_not_enough_blocks, CARDA_TEXT_NOT_ENOUGH_BLOCKS);
            prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_NEEDS_TWO_BLOCKS), FIELD_TEXT_COLOR_NORMAL, x, CARDA_TEXT_LINE_HEIGHT - y_offset,
                                   FIELD_TEXT_ALIGN_CENTER);
            prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, CARDA_TEXT_LINE_HEIGHT * 2 - y_offset);
        }
        break;

    case CARDA_ENTRY_STATE_NO_ROOM_FOR_DOWNLOAD:
        if (g_carda_mode == CARDA_MODE_RETURN_PET)
        {
            prim = carda_draw_mode3_notice(prim, ot, x_offset, y_offset, 0x10);
            g_card_entry_state = CARDA_ENTRY_STATE_CARD_FULL;
        }
        else
        {
            s32 x;
            u16* text_table;

            x = -x_offset + CARDA_SAVE_TEXT_X;
            prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_not_enough_blocks, CARDA_TEXT_NOT_ENOUGH_BLOCKS), FIELD_TEXT_COLOR_NORMAL, x, -y_offset,
                                   FIELD_TEXT_ALIGN_CENTER);
            text_table = CARDA_TEXT_TABLE(g_carda_text_not_enough_blocks, CARDA_TEXT_NOT_ENOUGH_BLOCKS);
            prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_DOWNLOAD_RING_RING_LAND), FIELD_TEXT_COLOR_NORMAL, x, 0x10 - y_offset,
                                   FIELD_TEXT_ALIGN_CENTER);
            prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_NEEDS_SIX_BLOCKS), FIELD_TEXT_COLOR_NORMAL, x, 0x20 - y_offset,
                                   FIELD_TEXT_ALIGN_CENTER);
            if (g_pad_input & CARDA_CONFIRM_BUTTON_MASK)
            {
                field_play_sound(FIELD_SOUND_CURSOR, FIELD_SOUND_PAN_CENTRE);
                g_card_entry_state = CARDA_ENTRY_STATE_SELECT_SLOT;
                g_carda_choice_toggle = g_card_slot;
                field_reset_input_repeat();
            }
        }
        break;

    case CARDA_ENTRY_STATE_NO_CARD:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_no_pocketstation, CARDA_TEXT_NO_POCKETSTATION), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_SAVE_TEXT_X, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, CARDA_TEXT_LINE_HEIGHT - y_offset);
        break;

    case CARDA_ENTRY_STATE_ACCESS_FAILED:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_pocketstation_access_failed, CARDA_TEXT_POCKETSTATION_ACCESS_FAILED),
                               FIELD_TEXT_COLOR_NORMAL, -x_offset + CARDA_SAVE_TEXT_X, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;

    case CARDA_ENTRY_STATE_NO_SAVE_DATA:
        prim = carda_draw_mode3_notice(prim, ot, x_offset, y_offset, CARDA_TEXT_LINE_HEIGHT);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, CARDA_TEXT_LINE_HEIGHT * 2 - y_offset);
        break;

    case CARDA_ENTRY_STATE_NO_RING_RING_LAND:
        prim = carda_draw_mode3_notice(prim, ot, x_offset, y_offset, 0x10);
        break;

    case CARDA_ENTRY_STATE_DOWNLOAD_FAILED:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_download_failed, CARDA_TEXT_DOWNLOAD_FAILED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_SAVE_TEXT_X, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, CARDA_TEXT_LINE_HEIGHT - y_offset);
        break;

    case CARDA_ENTRY_STATE_UPLOAD_FAILED:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_upload_failed, CARDA_TEXT_UPLOAD_FAILED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_SAVE_TEXT_X, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, CARDA_TEXT_LINE_HEIGHT - y_offset);
        break;

    case CARDA_ENTRY_STATE_POCKETSTATION_NOT_INSERTED:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_pocketstation_not_inserted, CARDA_TEXT_POCKETSTATION_NOT_INSERTED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_SAVE_TEXT_X, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, CARDA_TEXT_LINE_HEIGHT - y_offset);
        break;

    case CARDA_ENTRY_STATE_NO_POCKETSTATION:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_no_pocketstation, CARDA_TEXT_NO_POCKETSTATION), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_SAVE_TEXT_X, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, CARDA_TEXT_LINE_HEIGHT - y_offset);
        break;

    case CARDA_ENTRY_STATE_SAVE_CORRUPT:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_save_corrupt, CARDA_TEXT_SAVE_CORRUPT), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_SAVE_TEXT_X, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, CARDA_TEXT_LINE_HEIGHT - y_offset);
        break;

    case CARDA_ENTRY_STATE_FORMAT_FAILED:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_format_failed, CARDA_TEXT_FORMAT_FAILED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_SAVE_TEXT_X, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, CARDA_TEXT_LINE_HEIGHT - y_offset);
        break;

    case CARDA_ENTRY_STATE_PET_ALREADY_ON_RANCH:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_pet_already_on_ranch, CARDA_TEXT_PET_ALREADY_ON_RANCH), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_SAVE_TEXT_X, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = carda_draw_slot_prompt(prim, ot, CARDA_SAVE_TEXT_X - x_offset, CARDA_TEXT_LINE_HEIGHT - y_offset);
        break;

    case CARDA_ENTRY_STATE_CONFIRM_RETURN:
    {
        s32 x;
        s32 y;
        void* choice_prim;
        u16* text_table;
        u8* choice_entry;
        u8* choice_table;
        u8* first_text;
        u8* second_text;
        s32 first_high;
        s32 color;

        x = -x_offset;
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_return_pet, CARDA_TEXT_RETURN_PET), FIELD_TEXT_COLOR_NORMAL, x + CARDA_SAVE_TEXT_X,
                               -y_offset, FIELD_TEXT_ALIGN_CENTER);
        text_table = CARDA_TEXT_TABLE(g_carda_text_return_pet, CARDA_TEXT_RETURN_PET);
        prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_RING_RING_LAND_SIX_BLOCKS), FIELD_TEXT_COLOR_NORMAL, x + CARDA_SAVE_TEXT_X,
                               CARDA_TEXT_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_WILL_BE_ERASED), FIELD_TEXT_COLOR_NORMAL, x + CARDA_SAVE_TEXT_X,
                               CARDA_TEXT_LINE_HEIGHT * 2 - y_offset, FIELD_TEXT_ALIGN_CENTER);
        y = CARDA_TEXT_LINE_HEIGHT * 3 - y_offset;
        choice_entry = &g_text_choice_glyph_offsets;
        first_high = choice_entry[1] << CARDA_ELEMENT_COUNT;
        choice_table = choice_entry - FIELD_UI_TEXT_YES * 2;
        color = FIELD_TEXT_COLOR_NORMAL;
        first_text = (u8*)(choice_entry[0] + (first_high + (s32)choice_table));
        if (g_carda_choice_toggle != 0)
        {
            color = FIELD_TEXT_COLOR_DIM;
        }
        choice_prim = field_draw_text(prim, ot, first_text, color, x + 0x80, y, FIELD_TEXT_ALIGN_RIGHT);
        color = FIELD_TEXT_COLOR_NORMAL;
        second_text = FIELD_UI_TEXT(choice_table, FIELD_UI_TEXT_NO);
        if (g_carda_choice_toggle == 0)
        {
            color = FIELD_TEXT_COLOR_DIM;
        }
        choice_prim = field_draw_text(choice_prim, ot, second_text, color, x + 0x98, y, FIELD_TEXT_ALIGN_LEFT);
        if (g_pad_input & CARDA_CHOICE_BUTTON_MASK)
        {
            g_carda_choice_toggle ^= 1;
            field_play_sound(FIELD_SOUND_CURSOR, FIELD_SOUND_PAN_CENTRE);
            g_pad_input = 0;
        }
        prim = choice_prim;
        if (g_pad_input & PAD_BTN_CIRCLE)
        {
            field_play_sound(FIELD_SOUND_CURSOR, FIELD_SOUND_PAN_CENTRE);
            g_carda_choice_toggle = g_card_slot;
            g_card_entry_state = CARDA_ENTRY_STATE_SELECT_SLOT;
            field_reset_input_repeat();
            break;
        }
        if (g_pad_input & CARDA_CONFIRM_BUTTON_MASK)
        {
            if (g_carda_choice_toggle != 0)
            {
                field_play_sound(FIELD_SOUND_CURSOR, FIELD_SOUND_PAN_CENTRE);
                g_carda_choice_toggle = g_card_slot;
                g_card_entry_state = CARDA_ENTRY_STATE_SELECT_SLOT;
                field_reset_input_repeat();
                break;
            }
            field_play_sound(FIELD_SOUND_SELECT, FIELD_SOUND_PAN_CENTRE);
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
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_game_from_pocketstation, CARDA_TEXT_GAME_FROM_POCKETSTATION), FIELD_TEXT_COLOR_NORMAL, x,
                               -y_offset, FIELD_TEXT_ALIGN_CENTER);
        text_table = CARDA_TEXT_TABLE(g_carda_text_game_from_pocketstation, CARDA_TEXT_GAME_FROM_POCKETSTATION);
        prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_UPLOADING), FIELD_TEXT_COLOR_NORMAL, x, CARDA_TEXT_LINE_HEIGHT - y_offset,
                               FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_POCKETSTATION_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x,
                               CARDA_TEXT_LINE_HEIGHT * 2 - y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x,
                               CARDA_TEXT_LINE_HEIGHT * 3 - y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = carda_draw_progress_bar(prim, ot);
        if (g_carda_progress_active != 0)
        {
            break;
        }
        field_play_sound(FIELD_SOUND_LOAD_DONE, FIELD_SOUND_PAN_CENTRE);
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
        if (g_carda_mode == CARDA_MODE_RETURN_PET)
        {
            for (g_field_card_pet_slot = 0; g_field_card_pet_slot < PET_RECORD_COUNT; g_field_card_pet_slot++)
            {
                if (g_saved_game_ctx->pets[g_field_card_pet_slot].name[0] == 0)
                {
                    break;
                }
            }
            g_field_card_overlay_mode = CARDA_RESULT_PET_RETURNED;
            if (g_field_card_pet_slot == PET_RECORD_COUNT)
            {
                g_field_card_overlay_mode = CARDA_RESULT_RETURN_PET_CANCELLED;
                g_menu_element_counter = 0x20;
                element = g_carda_element_pool;
                for (i = 0; i < CARDA_ELEMENT_COUNT; i++)
                {
                    element->attr.bits.state = CARDA_ELEMENT_FREE;
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
            strcat(g_carda_selected_card_path.raw, g_lom_pocketstation_filename_prefix);
            _card_wait(g_card_slot);
            erase(&g_carda_selected_card_path);
            if (g_carda_received_item_count == 0)
            {
                CardaElement* element;

                g_menu_element_counter = 0x20;
                element = g_carda_element_pool;
                for (counter = 0; counter < CARDA_ELEMENT_COUNT; counter++)
                {
                    element->attr.bits.state = CARDA_ELEMENT_FREE;
                    element++;
                }
                field_restore_fade_target_with_duration(8);
                break;
            }
            carda_open_item_list();
            break;
        }
        g_carda_choice_toggle = CARDA_CHOICE_DEFAULT;
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
            u8* first_text;
            u8* second_text;
            s32 first_high;
            void* choice_prim;
            u8* choice_entry;
            u8* choice_table;
            u16* text_table;

            x = -x_offset;
            prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_game_to_pocketstation, CARDA_TEXT_GAME_TO_POCKETSTATION), FIELD_TEXT_COLOR_NORMAL,
                                   x + CARDA_SAVE_TEXT_X, -y_offset, FIELD_TEXT_ALIGN_CENTER);
            text_table = CARDA_TEXT_TABLE(g_carda_text_game_to_pocketstation, CARDA_TEXT_GAME_TO_POCKETSTATION);
            prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_DOWNLOAD_OK), FIELD_TEXT_COLOR_NORMAL, x + CARDA_SAVE_TEXT_X,
                                   CARDA_TEXT_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
            color = FIELD_TEXT_COLOR_NORMAL;
            y = CARDA_TEXT_LINE_HEIGHT * 2 - y_offset;
            choice_entry = &g_text_choice_glyph_offsets;
            first_high = choice_entry[1] << CARDA_ELEMENT_COUNT;
            choice_table = choice_entry - FIELD_UI_TEXT_YES * 2;
            first_text = (u8*)(choice_entry[0] + (first_high + (s32)choice_table));
            if (g_carda_choice_toggle != 0)
            {
                color = FIELD_TEXT_COLOR_DIM;
            }
            choice_prim = field_draw_text(prim, ot, first_text, color, x + 0x80, y, FIELD_TEXT_ALIGN_RIGHT);
            color = FIELD_TEXT_COLOR_NORMAL;
            second_text = FIELD_UI_TEXT(choice_table, FIELD_UI_TEXT_NO);
            if (g_carda_choice_toggle == 0)
            {
                color = FIELD_TEXT_COLOR_DIM;
            }
            choice_prim = field_draw_text(choice_prim, ot, second_text, color, x + 0x98, y, FIELD_TEXT_ALIGN_LEFT);
            if (g_pad_input & CARDA_CHOICE_BUTTON_MASK)
            {
                g_carda_choice_toggle ^= 1;
                field_play_sound(FIELD_SOUND_CURSOR, FIELD_SOUND_PAN_CENTRE);
                g_pad_input = 0;
            }
            prim = choice_prim;
        }
        else
        {
            s32 x;
            s32 y;
            s32 color;
            u8* first_text;
            u8* second_text;
            s32 first_high;
            void* choice_prim;
            u8* choice_entry;
            u8* choice_table;
            u16* text_table;

            x = -x_offset;
            prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_swap_pets, CARDA_TEXT_SWAP_PETS), FIELD_TEXT_COLOR_NORMAL, x + CARDA_SAVE_TEXT_X,
                                   -y_offset, FIELD_TEXT_ALIGN_CENTER);
            text_table = CARDA_TEXT_TABLE(g_carda_text_swap_pets, CARDA_TEXT_SWAP_PETS);
            prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_GAME_TO_POCKETSTATION), FIELD_TEXT_COLOR_NORMAL, x + CARDA_SAVE_TEXT_X,
                                   CARDA_TEXT_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
            prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_OVERWRITE_OK), FIELD_TEXT_COLOR_NORMAL, x + CARDA_SAVE_TEXT_X,
                                   CARDA_TEXT_LINE_HEIGHT * 2 - y_offset, FIELD_TEXT_ALIGN_CENTER);
            color = FIELD_TEXT_COLOR_NORMAL;
            y = CARDA_TEXT_LINE_HEIGHT * 3 - y_offset;
            choice_entry = &g_text_choice_glyph_offsets;
            first_high = choice_entry[1] << CARDA_ELEMENT_COUNT;
            choice_table = choice_entry - FIELD_UI_TEXT_YES * 2;
            first_text = (u8*)(choice_entry[0] + (first_high + (s32)choice_table));
            if (g_carda_choice_toggle != 0)
            {
                color = FIELD_TEXT_COLOR_DIM;
            }
            choice_prim = field_draw_text(prim, ot, first_text, color, x + 0x80, y, FIELD_TEXT_ALIGN_RIGHT);
            color = FIELD_TEXT_COLOR_NORMAL;
            second_text = FIELD_UI_TEXT(choice_table, FIELD_UI_TEXT_NO);
            if (g_carda_choice_toggle == 0)
            {
                color = FIELD_TEXT_COLOR_DIM;
            }
            choice_prim = field_draw_text(choice_prim, ot, second_text, color, x + 0x98, y, FIELD_TEXT_ALIGN_LEFT);
            if (g_pad_input & CARDA_CHOICE_BUTTON_MASK)
            {
                g_carda_choice_toggle ^= 1;
                field_play_sound(FIELD_SOUND_CURSOR, FIELD_SOUND_PAN_CENTRE);
                g_pad_input = 0;
            }
            prim = choice_prim;
        }
        if ((g_pad_input & PAD_BTN_CIRCLE) || ((g_pad_input & CARDA_CONFIRM_BUTTON_MASK) && g_carda_choice_toggle != 0))
        {
            field_play_sound(FIELD_SOUND_CURSOR, FIELD_SOUND_PAN_CENTRE);
            g_card_entry_state = CARDA_ENTRY_STATE_SELECT_SLOT;
            g_carda_choice_toggle = g_card_slot;
            field_reset_input_repeat();
        }
        else if (g_pad_input & CARDA_CONFIRM_BUTTON_MASK)
        {
            field_play_sound(FIELD_SOUND_SELECT, FIELD_SOUND_PAN_CENTRE);
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
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_downloading, CARDA_TEXT_DOWNLOADING), FIELD_TEXT_COLOR_NORMAL, x, -y_offset,
                               FIELD_TEXT_ALIGN_CENTER);
        text_table = CARDA_TEXT_TABLE(g_carda_text_downloading, CARDA_TEXT_DOWNLOADING);
        prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_POCKETSTATION_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x,
                               CARDA_TEXT_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x,
                               CARDA_TEXT_LINE_HEIGHT * 2 - y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = carda_draw_progress_bar(prim, ot);
        if (g_carda_save_in_progress == 0)
        {
            g_field_card_overlay_mode = CARDA_RESULT_PET_SENT;
            field_play_sound(FIELD_SOUND_SAVE_DONE, FIELD_SOUND_PAN_CENTRE);
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
            for (i = 0; i < CARDA_ELEMENT_COUNT; i++)
            {
                element->attr.bits.state = CARDA_ELEMENT_FREE;
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
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_checking_pocketstation, CARDA_TEXT_CHECKING_POCKETSTATION), FIELD_TEXT_COLOR_NORMAL, x,
                               -y_offset, FIELD_TEXT_ALIGN_CENTER);
        text_table = CARDA_TEXT_TABLE(g_carda_text_checking_pocketstation, CARDA_TEXT_CHECKING_POCKETSTATION);
        prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_POCKETSTATION_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x,
                               CARDA_TEXT_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, CARDA_TEXT(text_table, CARDA_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x,
                               CARDA_TEXT_LINE_HEIGHT * 2 - y_offset, FIELD_TEXT_ALIGN_CENTER);
        /* Search one entry per frame, unless a directory scan (opcode 6 or 7) is running. */
        if (g_carda_entry_scan_active == 0 && g_carda_io_busy == 0 && (u32)(*g_card_step - CARDA_STEP_SCAN_ENTRIES) >= 2U)
        {
            if (strncmp(g_lom_pocketstation_filename_prefix, g_card_entries[g_card_slot][g_carda_selected_row].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) != 0)
            {
                s32 row_y;
                s32 delta;

                g_carda_selected_row++;
                if (g_carda_selected_row >= g_card_entry_state)
                {
                    if (g_carda_mode == CARDA_MODE_RETURN_PET)
                    {
                        g_card_entry_state = CARDA_ENTRY_STATE_NO_GAME_DATA;
                        break;
                    }
                    g_carda_selected_card_path = g_carda_save_card_path_prefix;
                    g_carda_new_save_file = 1;
                    g_carda_selected_card_path.raw[2] += (u8)g_card_slot;
                    strcat(g_carda_selected_card_path.raw, g_lom_pocketstation_filename_prefix);
                    carda_store_active_record();
                    g_card_entry_state = CARDA_ENTRY_STATE_CONFIRM_DOWNLOAD;
                    g_carda_choice_toggle = CARDA_CHOICE_DEFAULT;
                    field_reset_input_repeat();
                    break;
                }
                carda_commit_selected_entry();
                row_y = g_carda_selected_row * CARDA_ENTRY_ROW_HEIGHT;
                delta = row_y - g_carda_scroll_y;
                if (delta > CARDA_LIST_HEIGHT - CARDA_ENTRY_ROW_HEIGHT)
                {
                    g_carda_scroll_target_y = row_y - CARDA_LIST_LAST_ROW_Y;
                    g_carda_scroll_frames = CARDA_SCROLL_FRAMES;
                }
                if (delta < 0)
                {
                    g_carda_scroll_target_y = row_y;
                    g_carda_scroll_frames = CARDA_SCROLL_FRAMES;
                }
                break;
            }
            if (g_carda_mode == CARDA_MODE_RETURN_PET)
            {
                g_carda_choice_toggle = CARDA_CHOICE_DEFAULT;
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

/**
 * @brief Serialize the game state into the save buffer and copy the active record into it.
 */
void carda_store_active_record(void)
{
    cdrom_queue_read(0x5E2, g_carda_save_blob);
    cdrom_wait_queue_empty();
    card_prepare_pet_transfer(g_carda_save_blob, &g_saved_game_ctx->pets[g_field_card_pet_slot]);
}

/**
 * @brief Restore the active record from g_carda_saved_record_copy and apply the save's growth delta.
 */
static void carda_restore_active_record(void)
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
static void* carda_draw_slot_prompt(void* prim, u_long* ot, s32 x, s32 y)
{
    CardaElement* element;
    void* result;
    s32 i;

    result = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_select_slot, CARDA_TEXT_SELECT_SLOT), FIELD_TEXT_COLOR_NORMAL, x, y, FIELD_TEXT_ALIGN_CENTER);

    if (g_pad_input & CARDA_CHOICE_BUTTON_MASK)
    {
        g_card_entry_state = CARDA_ENTRY_STATE_SELECT_SLOT;
        g_card_slot ^= 1;
        field_play_sound(FIELD_SOUND_CURSOR, FIELD_SOUND_PAN_CENTRE);
        return result;
    }

    if (g_pad_input & PAD_BTN_CIRCLE)
    {
        switch (g_carda_mode)
        {
        case 2:
            g_field_card_overlay_mode = CARDA_RESULT_SEND_PET_CANCELLED;
            break;
        case 3:
            g_field_card_overlay_mode = CARDA_RESULT_RETURN_PET_CANCELLED;
            break;
        default:
            g_field_card_overlay_mode = CARDA_RESULT_CANCELLED;
            break;
        }
        field_play_sound(FIELD_SOUND_ACTION_REFUSED, FIELD_SOUND_PAN_CENTRE);
        field_restore_fade_target();
        element = g_carda_element_pool;
        for (i = 0; i < CARDA_ELEMENT_COUNT; i++, element++)
        {
            if (element->attr.bits.state != CARDA_ELEMENT_FREE)
            {
                element->attr.bits.state = CARDA_ELEMENT_CLOSING;
                element->attr.bits.transition_step = CARDA_ELEMENT_TRANSITION_STEPS;
            }
        }
        return result;
    }

    if (g_pad_input & CARDA_CONFIRM_BUTTON_MASK)
    {
        s32 slot;

        field_play_sound(FIELD_SOUND_CURSOR, FIELD_SOUND_PAN_CENTRE);
        slot = g_card_slot;
        g_carda_format_declined = 0;
        g_card_step = NULL;
        g_card_entry_state = CARDA_ENTRY_STATE_CHECKING_CARD;
        g_carda_scroll_frames = 0;
        g_carda_scroll_target_y = 0;
        g_carda_scroll_y = 0;
        g_carda_selected_row = 0;
        g_card_slot ^= 1;
        g_carda_selection_status = CARDA_SELECTION_NONE;
        /* Same reset as switching cards, but stay on the current slot. */
        g_card_slot = slot;
        carda_reset_entry_ranks();
        clear_hardware_card_events();
        clear_software_card_events();
        g_carda_progress_bar_active = 0;
        g_card_entry_state = CARDA_ENTRY_STATE_CHECKING_CARD;
        g_card_step = g_carda_steps_initial_scan;
    }

    return result;
}

/**
 * @brief Reset the CARDA choice state and initialize its mode-specific element.
 * @param dialog_state Status-dialog state index.
 */
void carda_open_save_status_dialog(s32 dialog_state)
{
    field_play_sound(FIELD_SOUND_ACTION_REFUSED, FIELD_SOUND_PAN_CENTRE);
    field_reset_input_repeat();
    g_carda_save_in_progress = 0;
    g_carda_progress_active = 0;
    g_carda_selection_status = CARDA_SELECTION_NONE;
    g_carda_io_busy = 0;
    carda_reset_entry_ranks();
    g_card_step = NULL;
    g_carda_dialog_state = dialog_state;

    if (CARDA_IS_POCKETSTATION_MODE(g_carda_mode))
    {
        switch (dialog_state)
        {
        case CARDA_DIALOG_SAVE_FAILED:
            g_card_entry_state = CARDA_ENTRY_STATE_DOWNLOAD_FAILED;
            break;
        case CARDA_DIALOG_LOAD_FAILED:
            g_card_entry_state = CARDA_ENTRY_STATE_UPLOAD_FAILED;
            break;
        case CARDA_DIALOG_CARD_NOT_INSERTED:
            g_card_entry_state = CARDA_ENTRY_STATE_POCKETSTATION_NOT_INSERTED;
            break;
        case CARDA_DIALOG_NOT_POCKETSTATION:
            g_card_entry_state = CARDA_ENTRY_STATE_NO_POCKETSTATION;
            break;
        case CARDA_DIALOG_SAVE_CORRUPT:
            g_card_entry_state = CARDA_ENTRY_STATE_SAVE_CORRUPT;
            break;
        case CARDA_DIALOG_FORMAT_FAILED:
            g_card_entry_state = CARDA_ENTRY_STATE_FORMAT_FAILED;
            break;
        }
        g_card_step = NULL;
        return;
    }

    g_carda_element1_state.draw = carda_draw_save_status_dialog;
    g_carda_element1_state.attr.bits.transition_step = 1;
    g_carda_element1_state.attr.bits.state = CARDA_ELEMENT_OPENING;
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
static void* carda_draw_save_status_dialog(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2];

    switch (g_carda_dialog_state)
    {
    case CARDA_DIALOG_SAVE_FAILED:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_save_failed, CARDA_TEXT_SAVE_FAILED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_ITEM_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARDA_DIALOG_CARD_NOT_INSERTED:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_card_not_inserted, CARDA_TEXT_CARD_NOT_INSERTED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_ITEM_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARDA_DIALOG_NOT_POCKETSTATION:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_not_pocketstation, CARDA_TEXT_NOT_POCKETSTATION), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_ITEM_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARDA_DIALOG_LOAD_FAILED:
    case CARDA_DIALOG_SAVE_CORRUPT:
        prim = field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_load_failed, CARDA_TEXT_LOAD_FAILED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + CARDA_ITEM_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    }
    if (g_pad_input & CARDA_CONFIRM_BUTTON_MASK)
    {
        carda_save_clear_elements();
        field_restore_fade_target_with_duration(8);
        field_reset_input_repeat();
    }
    return prim;
}

/**
 * @brief Clear the element pool and open the item list and its header.
 */
static void carda_open_item_list(void)
{
    CardaElement* element;

    carda_save_clear_elements();

    g_carda_element_pool[CARDA_ELEMENT_MODAL].attr.bits.state = CARDA_ELEMENT_OPENING;
    element = carda_save_alloc_element();
    element->draw = carda_draw_item_list;
    element->attr.bits.transition_step = 2;
    element->attr.bits.x = 0x20;
    element->attr.bits.y = CARDA_ITEM_LIST_Y;
    element->size.bits.width_high = 1;
    element->size.bits.height = CARDA_ITEM_LIST_HEIGHT;
    CARDA_SET_ELEMENT_WIDTH_LOW(element, 0);

    element = carda_save_alloc_element();
    element->draw = carda_draw_item_list_header;
    element->attr.bits.transition_step = 2;
    element->attr.bits.x = 0x20;
    element->attr.bits.y = 0x1A;
    element->size.bits.width_high = 1;
    element->size.bits.height = 0x10;
    g_carda_scroll_frames = 0;
    g_carda_scroll_target_y = 0;
    CARDA_SET_ELEMENT_WIDTH_LOW(element, 0);
    g_carda_scroll_y = 0;
    g_carda_element_pool[CARDA_ELEMENT_MODAL].attr.bits.state = CARDA_ELEMENT_FREE;
}

/**
 * @brief Draw the item-list header text.
 * @param ot Ordering table receiving the text packets.
 * @param prim GPU packet write cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return GPU packet cursor after the text.
 */
static void* carda_draw_item_list_header(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2];
    return field_draw_text(prim, ot, CARDA_TEXT_AT(g_carda_text_received_items, CARDA_TEXT_RECEIVED_ITEMS), FIELD_TEXT_COLOR_NORMAL,
                           -x_offset + CARDA_ITEM_LIST_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
}

/**
 * @brief Draw the visible rows of the received-item list and handle scrolling.
 * @param ot Ordering table receiving the text packets.
 * @param prim GPU packet write cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return GPU packet cursor after the list.
 */
void* carda_draw_item_list(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2];
    s32 row_y;
    s32 i;
    void* result;

    result = prim;
    for (i = 0; i < g_carda_received_item_count; i++)
    {
        row_y = i * CARDA_TEXT_LINE_HEIGHT - g_carda_scroll_y;
        if ((u32)(row_y + 13) < 157)
        {
            result = field_draw_text(result, ot, (u8*)g_carda_item_names + g_carda_item_names[g_carda_received_item_ids[i]], FIELD_TEXT_COLOR_NORMAL,
                                     -x_offset + CARDA_ITEM_LIST_WIDTH / 2, row_y - y_offset, FIELD_TEXT_ALIGN_CENTER);
        }
    }

    if (g_carda_scroll_frames == 0)
    {
        if (g_pad_input & PAD_BTN_UP)
        {
            if (g_carda_scroll_y != 0)
            {
                field_play_sound(FIELD_SOUND_CURSOR, FIELD_SOUND_PAN_CENTRE);
                g_carda_scroll_frames = CARDA_SCROLL_FRAMES;
                g_carda_scroll_target_y -= CARDA_TEXT_LINE_HEIGHT;
            }
        }
        else if (g_pad_input & PAD_BTN_DOWN)
        {
            if ((g_carda_received_item_count * CARDA_TEXT_LINE_HEIGHT - g_carda_scroll_y) >= 141)
            {
                field_play_sound(FIELD_SOUND_CURSOR, FIELD_SOUND_PAN_CENTRE);
                g_carda_scroll_frames = CARDA_SCROLL_FRAMES;
                g_carda_scroll_target_y += CARDA_TEXT_LINE_HEIGHT;
            }
        }
        else if (g_pad_input & CARDA_CONFIRM_BUTTON_MASK)
        {
            field_play_sound(FIELD_SOUND_SELECT, FIELD_SOUND_PAN_CENTRE);
            carda_save_clear_elements();
            field_restore_fade_target_with_duration(8);
        }
    }
    return result;
}

/**
 * @brief Copy the save's record into g_carda_saved_record_copy and add the save's items to the game state.
 * @note Items already at CARDA_ITEM_COUNT_MAX are skipped; the ids that were added
 *       are listed in g_carda_received_item_ids and counted in g_carda_received_item_count.
 */
static void carda_apply_save_items(void)
{
    CardaSaveData* save;
    CardaSaveItemList* list;
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
