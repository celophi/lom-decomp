#include "field_text.h"
#include "addhero_internal.h"

/** @brief Side of a square party icon, in pixels (4-bit, so a quarter of that in VRAM halfwords). */
#define ADDHERO_ICON_SIZE 48

/** @brief VRAM area the details window's party icons are uploaded to, one after another. */
#define ADDHERO_ICON_VRAM_X SCREEN_WIDTH
#define ADDHERO_ICON_VRAM_Y 208

/** @brief Gaps between the choice prompt's centre and its right-aligned yes and left-aligned no. */
#define ADDHERO_CHOICE_YES_GAP 16
#define ADDHERO_CHOICE_NO_GAP 8

/** @brief One save icon in the icon-set resource: a 16-color CLUT followed by 48x48 4-bit pixels. */
typedef struct
{
    u16 clut[16];
    u8 pixels[ADDHERO_ICON_SIZE * ADDHERO_ICON_SIZE / 2];
} AddheroIconImage;

/**
 * @brief Icon @p icon of the loaded icon set.
 * @note g_addhero_icon_image_table holds per-icon byte offsets from the icon-set
 *       start, which is the word just before the table.
 */
#define ADDHERO_ICON_IMAGE(icon) ((AddheroIconImage*)((u8*)g_addhero_icon_image_table - 4 + g_addhero_icon_image_table[(icon)]))

/**
 * @brief Draw the load confirmation prompt and its yes/no choice, then act on
 *        input: cancel/back to the browser, or accept and swap this element to
 *        the load-progress bar.
 * @param ot   Ordering table the primitives are linked into.
 * @param prim Current primitive pointer/index.
 * @param x_offset Horizontal offset; the text is centred at ADDHERO_MESSAGE_WIDTH / 2 - x_offset.
 * @param y_offset Vertical offset applied to the prompt rows.
 * @return The updated primitive pointer after linking the prompt.
 */
void* addhero_draw_load_prompt(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT unused; /* never used, but the original stack frame reserves it */
    void* result;
    s32 x;
    s32 status;
    AddheroElement* p;

    x = -x_offset + ADDHERO_MESSAGE_WIDTH / 2;
    result = addhero_draw_choice_prompt(field_draw_text(prim, ot, ADDHERO_TEXT_AT(g_addhero_text_load_prompt, ADDHERO_TEXT_LOAD_PROMPT),
                                                        FIELD_TEXT_COLOR_NORMAL, x, -y_offset, FIELD_TEXT_ALIGN_CENTER),
                                        ot, x, ADDHERO_TEXT_LINE_HEIGHT - y_offset);

    status = addhero_poll_and_retry_card_info();
    if (status == ADDHERO_CARD_EVENT_ERROR || status == ADDHERO_CARD_EVENT_TIMEOUT)
    {
        g_addhero_element_pool[ADDHERO_ELEMENT_MODAL].attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
        field_reset_input_repeat();
        field_play_sound(FIELD_SOUND_ACTION_REFUSED, FIELD_SOUND_PAN_CENTRE);
        g_addhero_entry_state = ADDHERO_ENTRY_STATE_CHECKING_CARD;
        addhero_reset_entry_ranks();
        g_addhero_load_step = NULL;
    }
    else
    {
        if (g_pad_input & PAD_BTN_CIRCLE)
        {
            g_addhero_element_pool[ADDHERO_ELEMENT_MODAL].attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
            field_reset_input_repeat();
            field_play_sound(FIELD_SOUND_ACTION_REFUSED, FIELD_SOUND_PAN_CENTRE);
            g_addhero_load_step = g_addhero_loadseq_abort;
        }
        else if (g_pad_input & ADDHERO_CONFIRM_BUTTON_MASK)
        {
            if (g_addhero_choice_toggle != 0)
            {
                g_addhero_element_pool[ADDHERO_ELEMENT_MODAL].attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
                field_reset_input_repeat();
                field_play_sound(FIELD_SOUND_ACTION_REFUSED, FIELD_SOUND_PAN_CENTRE);
                g_addhero_load_step = g_addhero_loadseq_abort;
            }
            else
            {
                field_play_sound(FIELD_SOUND_SELECT, FIELD_SOUND_PAN_CENTRE);
                g_addhero_progress_active = 1;
                g_addhero_load_step = g_addhero_loadseq_load_begin;
                p = &g_addhero_element_pool[ADDHERO_ELEMENT_MODAL];
                p->draw_handler = addhero_draw_load_progress;
                p->attr.bits.transition_step = 1;
                p->attr.bits.state = ADDHERO_ELEMENT_STATE_OPENING;
                p->attr.bits.x = ADDHERO_MESSAGE_X;
                p->attr.bits.y = ADDHERO_MESSAGE_Y;
                p->size.bits.width_high = ADDHERO_MESSAGE_WIDTH >> 8;
                p->size.bits.height = ADDHERO_MESSAGE_HEIGHT;
                ADDHERO_SET_ELEMENT_WIDTH_LOW(p, ADDHERO_MESSAGE_WIDTH);
            }
        }
    }
    return result;
}

/**
 * @brief Draw the "loading new hero" prompt and, once the load has finished,
 *        validate and commit the freshly loaded save data.
 * @param ot   Ordering table the prompt primitives are linked into.
 * @param prim Current primitive pointer / index within the ordering table.
 * @param x_offset Horizontal offset; the text is centred at ADDHERO_MESSAGE_WIDTH / 2 - x_offset.
 * @param y_offset Vertical offset used to place the prompt rows.
 * @return The updated primitive pointer / index after linking the prompt.
 */
void* addhero_draw_load_progress(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT unused; /* never used, but the original stack frame reserves it */
    u16* text_table;
    SaveFile* file;
    AddheroElement* element;
    void* result;
    s32 x;
    s32 i;
    u32 pad_controlled;

    x = -x_offset + ADDHERO_MESSAGE_WIDTH / 2;
    result = field_draw_text(prim, ot, ADDHERO_TEXT_AT(g_addhero_text_loading, ADDHERO_TEXT_LOADING), FIELD_TEXT_COLOR_NORMAL, x, -y_offset,
                             FIELD_TEXT_ALIGN_CENTER);
    text_table = ADDHERO_TEXT_TABLE(g_addhero_text_loading, ADDHERO_TEXT_LOADING);
    result = field_draw_text(result, ot, ADDHERO_TEXT(text_table, ADDHERO_TEXT_DO_NOT_REMOVE_CARD), FIELD_TEXT_COLOR_NORMAL, x,
                             ADDHERO_TEXT_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
    result = field_draw_text(result, ot, ADDHERO_TEXT(text_table, ADDHERO_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x,
                             (ADDHERO_TEXT_LINE_HEIGHT * 2) - y_offset, FIELD_TEXT_ALIGN_CENTER);
    result = addhero_draw_progress_bar(result, ot);

    if (g_addhero_progress_active == 0)
    {
        file = &g_addhero_save_file;
        g_addhero_element_pool[ADDHERO_ELEMENT_MODAL].attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
        if (validate_save_file(file) == 0)
        {
            addhero_open_status_dialog(ADDHERO_DIALOG_INVALID_SAVE);
            return result;
        }

        field_play_sound(FIELD_SOUND_LOAD_DONE, FIELD_SOUND_PAN_CENTRE);
        pad_controlled = g_saved_game_ctx->characters[FIELD_PARTY_GUEST].info.bytes[0] >> FIELD_CHARACTER_PAD_CONTROLLED_SHIFT;
        bcopy((u8*)&file->saved_game.characters[FIELD_PARTY_HERO], (u8*)&g_saved_game_ctx->characters[FIELD_PARTY_GUEST], sizeof(FieldCharacterRecord));
        g_saved_game_ctx->characters[FIELD_PARTY_GUEST].info.word =
            (g_saved_game_ctx->characters[FIELD_PARTY_GUEST].info.word & ~FIELD_CHARACTER_PAD_CONTROLLED) |
            (pad_controlled << FIELD_CHARACTER_PAD_CONTROLLED_SHIFT);
        g_saved_game_ctx->guest_origin.ids.game_id = file->saved_game.identity.ids.game_id;
        g_saved_game_ctx->guest_origin.ids.save_id = file->saved_game.identity.ids.save_id;
        g_saved_game_ctx->guest_loaded = 1;
        field_restore_fade_target();

        element = g_addhero_element_pool;
        for (i = 0; i < ADDHERO_ELEMENT_COUNT; i++, element++)
        {
            if (element->attr.bits.state != ADDHERO_ELEMENT_STATE_INACTIVE)
            {
                element->attr.bits.state = ADDHERO_ELEMENT_STATE_CLOSING;
                element->attr.bits.transition_step = ADDHERO_ELEMENT_TRANSITION_STEPS;
            }
        }
        field_restore_fade_target_with_duration(ADDHERO_EXIT_FADE_FRAMES);
        g_addhero_result = ADDHERO_RESULT_LOADED;
    }

    return result;
}

/**
 * @brief Draw the gradient progress bar whose width tracks elapsed ticks, when
 *        the bar is active.
 * @param quad Primitive slot the POLY_G4 bar is written to.
 * @param ot Ordering table the primitive is linked into.
 * @return The advanced primitive pointer, unchanged when the bar is inactive.
 */
void* addhero_draw_progress_bar(POLY_G4* quad, u_long* ot)
{
    s32 elapsed;
    s32 width;

    if (g_addhero_progress_bar_active != 0)
    {
        elapsed = VSync(-1) - g_addhero_progress_start_tick;
        if (elapsed > ADDHERO_PROGRESS_FULL_TICKS)
        {
            elapsed = ADDHERO_PROGRESS_FULL_TICKS;
        }
        width = elapsed * ADDHERO_MESSAGE_WIDTH;
        SET_BGR0_PACKED(quad, GPU_COLOR_WORD(0xFF, 0, 0));
        SET_POLY_G4_BGR1_PACKED(quad, GPU_COLOR_WORD(0xFF, 0xFF, 0));
        SET_POLY_G4_BGR3_PACKED(quad, GPU_COLOR_WORD(0, 0, 0xFF));
        setlen(quad, 8);
        SET_POLY_G4_BGR2_PACKED(quad, GPU_COLOR_WORD(0, 0xFF, 0xFF));
        setcode(quad, GPU_CODE_POLY_G4);
        quad->x2 = 0;
        quad->x0 = 0;
        quad->x1 = quad->x3 = width / ADDHERO_PROGRESS_FULL_TICKS;
        quad->y1 = 0;
        quad->y0 = 0;
        quad->y3 = ADDHERO_MESSAGE_HEIGHT;
        quad->y2 = ADDHERO_MESSAGE_HEIGHT;
        addPrim(ot, quad);
        quad++;
    }
    return quad;
}

/**
 * @brief Reconfigure the primary element as a modal status dialog and reset the
 *        browser/IO state, storing the dialog message id.
 * @param message_id Dialog message id stored in g_addhero_dialog_state.
 */
void addhero_open_status_dialog(s32 message_id)
{
    field_play_sound(FIELD_SOUND_ACTION_REFUSED, FIELD_SOUND_PAN_CENTRE);
    g_addhero_element_pool[ADDHERO_ELEMENT_MODAL].draw_handler = addhero_draw_status_dialog;
    g_addhero_element_pool[ADDHERO_ELEMENT_MODAL].attr.bits.transition_step = 1;
    g_addhero_element_pool[ADDHERO_ELEMENT_MODAL].attr.bits.state = ADDHERO_ELEMENT_STATE_OPENING;
    g_addhero_element_pool[ADDHERO_ELEMENT_MODAL].attr.bits.x = ADDHERO_DIALOG_X;
    g_addhero_element_pool[ADDHERO_ELEMENT_MODAL].attr.bits.y = ADDHERO_DIALOG_Y;
    g_addhero_element_pool[ADDHERO_ELEMENT_MODAL].size.bits.width_high = ADDHERO_DIALOG_WIDTH >> 8;
    g_addhero_element_pool[ADDHERO_ELEMENT_MODAL].size.bits.height = ADDHERO_DIALOG_HEIGHT;
    ADDHERO_SET_ELEMENT_WIDTH_LOW(&g_addhero_element_pool[ADDHERO_ELEMENT_MODAL], ADDHERO_DIALOG_WIDTH);
    field_reset_input_repeat();
    g_addhero_write_in_progress = 0;
    g_addhero_progress_active = 0;
    g_addhero_selection_status = ADDHERO_SELECTION_NONE;
    g_addhero_io_busy = 0;
    g_addhero_entry_state = ADDHERO_ENTRY_STATE_CHECKING_CARD;
    addhero_reset_entry_ranks();
    g_addhero_load_step = NULL;
    g_addhero_dialog_state = message_id;
}

/**
 * @brief Reconfigure the primary element as a modal exit dialog and reset the
 *        browser/IO state, storing the dialog message id.
 * @param message_id Dialog message id stored in g_addhero_dialog_state.
 */
void addhero_open_exit_dialog(s32 message_id)
{
    field_play_sound(FIELD_SOUND_ACTION_REFUSED, FIELD_SOUND_PAN_CENTRE);
    g_addhero_element1.draw_handler = addhero_draw_exit_dialog;
    g_addhero_element1.attr.bits.transition_step = 1;
    g_addhero_element1.attr.bits.state = ADDHERO_ELEMENT_STATE_OPENING;
    g_addhero_element1.attr.bits.x = ADDHERO_DIALOG_X;
    g_addhero_element1.attr.bits.y = ADDHERO_DIALOG_Y;
    g_addhero_element1.size.bits.width_high = ADDHERO_DIALOG_WIDTH >> 8;
    g_addhero_element1.size.bits.height = ADDHERO_DIALOG_HEIGHT;
    ADDHERO_SET_ELEMENT_WIDTH_LOW(&g_addhero_element1, ADDHERO_DIALOG_WIDTH);
    field_reset_input_repeat();
    g_addhero_write_in_progress = 0;
    g_addhero_progress_active = 0;
    g_addhero_selection_status = ADDHERO_SELECTION_NONE;
    g_addhero_io_busy = 0;
    addhero_reset_entry_ranks();
    g_addhero_load_step = NULL;
    g_addhero_dialog_state = message_id;
}

/**
 * @brief Draw the status dialog message for the current dialog state and close
 *        the element once the player acknowledges it.
 * @param ot   Ordering table the message glyph is linked into.
 * @param prim Current primitive pointer/index.
 * @param x_offset Horizontal offset; screen X is derived from it.
 * @param y_offset Vertical offset.
 * @return The updated primitive pointer.
 */
void* addhero_draw_status_dialog(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT unused; /* never used, but the original stack frame reserves it */

    switch (g_addhero_dialog_state)
    {
    case ADDHERO_DIALOG_SAVE_FAILED:
        prim = field_draw_text(prim, ot, ADDHERO_TEXT_AT(g_addhero_text_save_failed, ADDHERO_TEXT_SAVE_FAILED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + ADDHERO_DIALOG_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case ADDHERO_DIALOG_CARD_NOT_INSERTED:
        prim = field_draw_text(prim, ot, ADDHERO_TEXT_AT(g_addhero_text_card_not_inserted, ADDHERO_TEXT_CARD_NOT_INSERTED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + ADDHERO_DIALOG_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case ADDHERO_DIALOG_NOT_POCKETSTATION:
        prim = field_draw_text(prim, ot, ADDHERO_TEXT_AT(g_addhero_text_not_pocketstation, ADDHERO_TEXT_NOT_POCKETSTATION), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + ADDHERO_DIALOG_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case ADDHERO_DIALOG_LOAD_FAILED:
    case ADDHERO_DIALOG_INVALID_SAVE:
        prim = field_draw_text(prim, ot, ADDHERO_TEXT_AT(g_addhero_text_load_failed, ADDHERO_TEXT_LOAD_FAILED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + ADDHERO_DIALOG_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    }
    if (g_pad_input & ADDHERO_CONFIRM_BUTTON_MASK)
    {
        g_addhero_element_pool[ADDHERO_ELEMENT_MODAL].attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
        field_reset_input_repeat();
    }
    return prim;
}

/**
 * @brief Draw the exit dialog message and, on acknowledge, tear down all
 *        elements and request the overlay to exit with ADDHERO_RESULT_CANCELLED.
 * @param ot   Ordering table the message glyph is linked into.
 * @param prim Current primitive pointer/index.
 * @param x_offset Horizontal offset; screen X is derived from it.
 * @param y_offset Vertical offset.
 * @return The updated primitive pointer.
 */
void* addhero_draw_exit_dialog(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT unused; /* never used, but the original stack frame reserves it */
    AddheroElement* p;
    s32 i;

    switch (g_addhero_dialog_state)
    {
    case ADDHERO_DIALOG_SAVE_FAILED:
        prim = field_draw_text(prim, ot, ADDHERO_TEXT_AT(g_addhero_text_save_failed, ADDHERO_TEXT_SAVE_FAILED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + ADDHERO_DIALOG_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case ADDHERO_DIALOG_CARD_NOT_INSERTED:
        prim = field_draw_text(prim, ot, ADDHERO_TEXT_AT(g_addhero_text_card_not_inserted, ADDHERO_TEXT_CARD_NOT_INSERTED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + ADDHERO_DIALOG_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case ADDHERO_DIALOG_NOT_POCKETSTATION:
        prim = field_draw_text(prim, ot, ADDHERO_TEXT_AT(g_addhero_text_not_pocketstation, ADDHERO_TEXT_NOT_POCKETSTATION), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + ADDHERO_DIALOG_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case ADDHERO_DIALOG_LOAD_FAILED:
    case ADDHERO_DIALOG_INVALID_SAVE:
        prim = field_draw_text(prim, ot, ADDHERO_TEXT_AT(g_addhero_text_load_failed, ADDHERO_TEXT_LOAD_FAILED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + ADDHERO_DIALOG_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    }
    if (g_pad_input & ADDHERO_CONFIRM_BUTTON_MASK)
    {
        g_addhero_result = ADDHERO_RESULT_CANCELLED;
        g_menu_element_counter = 0x20;
        p = &g_addhero_element_pool[ADDHERO_ELEMENT_MODAL];
        for (i = 0; i < ADDHERO_ELEMENT_COUNT; i++)
        {
            p->size.bits.scrollable = 0;
            p->attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
            p++;
        }
        field_restore_fade_target_with_duration(ADDHERO_EXIT_FADE_FRAMES);
        field_reset_input_repeat();
    }
    return prim;
}

/**
 * @brief Transfer-mode driver/renderer: draws the message for the current
 *        entry-state sentinel, runs the load/save confirm and progress steps,
 *        and handles cancel/back input.
 * @param ot   Ordering table the primitives are linked into.
 * @param prim Current primitive pointer/index.
 * @param x_offset Horizontal offset; the text is centred at ADDHERO_MESSAGE_WIDTH / 2 - x_offset.
 * @param y_offset Vertical offset applied to the message rows.
 * @return The updated primitive pointer.
 */
void* addhero_draw_transfer_status(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT unused; /* never used, but the original stack frame reserves it */

    switch (g_addhero_entry_state)
    {
    case ADDHERO_ENTRY_STATE_NO_GAME_DATA:
        prim = field_draw_text(prim, ot, ADDHERO_TEXT_AT(g_addhero_text_no_game_save_data, ADDHERO_TEXT_NO_GAME_SAVE_DATA), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + ADDHERO_MESSAGE_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case ADDHERO_ENTRY_STATE_BROWSER_READ_ERROR:
        prim = field_draw_text(prim, ot, ADDHERO_TEXT_AT(g_addhero_text_no_game_save_data, ADDHERO_TEXT_NO_GAME_SAVE_DATA), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + ADDHERO_MESSAGE_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case ADDHERO_ENTRY_STATE_CHECKING_CARD:
    {
        s32 message_x;
        u16* text_table;

        message_x = -x_offset + ADDHERO_MESSAGE_WIDTH / 2;
        text_table = &g_addhero_text_table;
        prim = field_draw_text(prim, ot, ADDHERO_TEXT(text_table, ADDHERO_TEXT_CHECKING_CARD), FIELD_TEXT_COLOR_NORMAL, message_x, -y_offset,
                               FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, ADDHERO_TEXT(text_table, ADDHERO_TEXT_DO_NOT_REMOVE_CARD), FIELD_TEXT_COLOR_NORMAL, message_x,
                               ADDHERO_TEXT_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, ADDHERO_TEXT(text_table, ADDHERO_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, message_x,
                               (ADDHERO_TEXT_LINE_HEIGHT * 2) - y_offset, FIELD_TEXT_ALIGN_CENTER);
    }
    break;
    case ADDHERO_ENTRY_STATE_CARD_FULL:
        prim = field_draw_text(prim, ot, ADDHERO_TEXT_AT(g_addhero_text_no_game_save_data, ADDHERO_TEXT_NO_GAME_SAVE_DATA), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + ADDHERO_MESSAGE_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case ADDHERO_ENTRY_STATE_NO_CARD:
        prim = field_draw_text(prim, ot, ADDHERO_TEXT_AT(g_addhero_text_no_card, ADDHERO_TEXT_NO_CARD), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + ADDHERO_MESSAGE_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case ADDHERO_ENTRY_STATE_ACCESS_FAILED:
        prim = field_draw_text(prim, ot, ADDHERO_TEXT_AT(g_addhero_text_card_access_failed, ADDHERO_TEXT_CARD_ACCESS_FAILED), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + ADDHERO_MESSAGE_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case ADDHERO_ENTRY_STATE_NO_SAVE_DATA:
        prim = field_draw_text(prim, ot, ADDHERO_TEXT_AT(g_addhero_text_no_save_data, ADDHERO_TEXT_NO_SAVE_DATA), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + ADDHERO_MESSAGE_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case ADDHERO_ENTRY_STATE_NO_LOAD_FILE:
        prim = field_draw_text(prim, ot, ADDHERO_TEXT_AT(g_addhero_text_no_load_file, ADDHERO_TEXT_NO_LOAD_FILE), FIELD_TEXT_COLOR_NORMAL,
                               -x_offset + ADDHERO_MESSAGE_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case ADDHERO_ENTRY_STATE_LOAD_PROGRESS:
    {
        s32 message_x;
        u16* text_table;
        POLY_G4* bar;
        s32 elapsed;
        s32 width;

        message_x = -x_offset + ADDHERO_MESSAGE_WIDTH / 2;
        prim = field_draw_text(prim, ot, ADDHERO_TEXT_AT(g_addhero_text_loading, ADDHERO_TEXT_LOADING), FIELD_TEXT_COLOR_NORMAL, message_x, -y_offset,
                               FIELD_TEXT_ALIGN_CENTER);
        text_table = ADDHERO_TEXT_TABLE(g_addhero_text_loading, ADDHERO_TEXT_LOADING);
        prim = field_draw_text(prim, ot, ADDHERO_TEXT(text_table, ADDHERO_TEXT_DO_NOT_REMOVE_CARD), FIELD_TEXT_COLOR_NORMAL, message_x,
                               ADDHERO_TEXT_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, ADDHERO_TEXT(text_table, ADDHERO_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, message_x,
                               (ADDHERO_TEXT_LINE_HEIGHT * 2) - y_offset, FIELD_TEXT_ALIGN_CENTER);
        bar = prim;
        if (g_addhero_progress_bar_active != 0)
        {
            elapsed = VSync(-1) - g_addhero_progress_start_tick;
            if (elapsed > ADDHERO_PROGRESS_FULL_TICKS)
            {
                elapsed = ADDHERO_PROGRESS_FULL_TICKS;
            }
            width = elapsed * ADDHERO_MESSAGE_WIDTH;
            SET_BGR0_PACKED(bar, GPU_COLOR_WORD(0xFF, 0, 0));
            SET_POLY_G4_BGR1_PACKED(bar, GPU_COLOR_WORD(0xFF, 0xFF, 0));
            SET_POLY_G4_BGR3_PACKED(bar, GPU_COLOR_WORD(0, 0, 0xFF));
            setlen(bar, 8);
            SET_POLY_G4_BGR2_PACKED(bar, GPU_COLOR_WORD(0, 0xFF, 0xFF));
            setcode(bar, GPU_CODE_POLY_G4);
            bar->x2 = 0;
            bar->x0 = 0;
            bar->x1 = bar->x3 = width / ADDHERO_PROGRESS_FULL_TICKS;
            bar->y1 = 0;
            bar->y0 = 0;
            bar->y3 = ADDHERO_MESSAGE_HEIGHT;
            bar->y2 = ADDHERO_MESSAGE_HEIGHT;
            addPrim(ot, bar);
            bar++;
        }
        prim = bar;
        if (g_addhero_progress_active == 0)
        {
            if (validate_save_file(&g_addhero_save_file) == 0)
            {
                s32 message_id = ADDHERO_DIALOG_INVALID_SAVE;

                field_play_sound(FIELD_SOUND_ACTION_REFUSED, FIELD_SOUND_PAN_CENTRE);
                g_addhero_element_pool[ADDHERO_ELEMENT_MODAL].draw_handler = addhero_draw_status_dialog;
                g_addhero_element_pool[ADDHERO_ELEMENT_MODAL].attr.bits.transition_step = 1;
                g_addhero_element_pool[ADDHERO_ELEMENT_MODAL].attr.bits.state = ADDHERO_ELEMENT_STATE_OPENING;
                g_addhero_element_pool[ADDHERO_ELEMENT_MODAL].attr.bits.x = ADDHERO_DIALOG_X;
                g_addhero_element_pool[ADDHERO_ELEMENT_MODAL].attr.bits.y = ADDHERO_DIALOG_Y;
                g_addhero_element_pool[ADDHERO_ELEMENT_MODAL].size.bits.width_high = ADDHERO_DIALOG_WIDTH >> 8;
                g_addhero_element_pool[ADDHERO_ELEMENT_MODAL].size.bits.height = ADDHERO_DIALOG_HEIGHT;
                ADDHERO_SET_ELEMENT_WIDTH_LOW(&g_addhero_element_pool[ADDHERO_ELEMENT_MODAL], ADDHERO_DIALOG_WIDTH);
                field_reset_input_repeat();
                g_addhero_write_in_progress = 0;
                g_addhero_selection_status = ADDHERO_SELECTION_NONE;
                g_addhero_io_busy = 0;
                g_addhero_progress_active = 0;
                g_addhero_entry_state = ADDHERO_ENTRY_STATE_CHECKING_CARD;
                addhero_reset_entry_ranks();
                g_addhero_load_step = NULL;
                g_addhero_dialog_state = message_id;
                return prim;
            }
            field_play_sound(FIELD_SOUND_LOAD_DONE, FIELD_SOUND_PAN_CENTRE);
            g_addhero_entry_state = ADDHERO_ENTRY_STATE_SAVE_CONFIRM;
            addhero_enable_choice_toggle();
            field_reset_input_repeat();
        }
    }
    break;
    case ADDHERO_ENTRY_STATE_CONFIRM_NO_SAVE:
    {
        s32 message_x;
        AddheroElement* element;
        s32 i;

        message_x = -x_offset + ADDHERO_MESSAGE_WIDTH / 2;
        prim = field_draw_text(prim, ot, ADDHERO_TEXT_AT(g_addhero_text_2p_data_not_saved, ADDHERO_TEXT_2P_DATA_NOT_SAVED), FIELD_TEXT_COLOR_NORMAL, message_x,
                               -y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = addhero_draw_choice_prompt(prim, ot, message_x, ADDHERO_TEXT_LINE_HEIGHT - y_offset);
        if (g_pad_input & PAD_BTN_CIRCLE)
        {
            field_play_sound(FIELD_SOUND_ACTION_REFUSED, FIELD_SOUND_PAN_CENTRE);
            addhero_enable_choice_toggle();
            g_addhero_entry_state = ADDHERO_ENTRY_STATE_SAVE_CONFIRM;
            field_reset_input_repeat();
        }
        else if (g_pad_input & ADDHERO_CONFIRM_BUTTON_MASK)
        {
            if (g_addhero_choice_toggle != 0)
            {
                field_play_sound(FIELD_SOUND_ACTION_REFUSED, FIELD_SOUND_PAN_CENTRE);
                addhero_enable_choice_toggle();
                g_addhero_entry_state = ADDHERO_ENTRY_STATE_SAVE_CONFIRM;
                field_reset_input_repeat();
            }
            else
            {
                field_play_sound(FIELD_SOUND_CURSOR, FIELD_SOUND_PAN_CENTRE);
                g_addhero_result = ADDHERO_RESULT_CANCELLED;
                g_menu_element_counter = 0x20;
                element = g_addhero_element_pool;
                for (i = 0; i < ADDHERO_ELEMENT_COUNT; i++, element++)
                {
                    element->size.bits.scrollable = 0;
                    element->attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
                }
                field_restore_fade_target_with_duration(ADDHERO_EXIT_FADE_FRAMES);
                field_reset_input_repeat();
            }
        }
    }
    break;
    case ADDHERO_ENTRY_STATE_SAVE_CONFIRM:
    {
        s32 message_x;
        u8* buffer; /* the text table, then the save file; one pointer serves both */
        s32 checksum;

        message_x = -x_offset + ADDHERO_MESSAGE_WIDTH / 2;
        prim = field_draw_text(prim, ot, ADDHERO_TEXT_AT(g_addhero_text_found_load_file, ADDHERO_TEXT_FOUND_LOAD_FILE), FIELD_TEXT_COLOR_NORMAL, message_x,
                               -y_offset, FIELD_TEXT_ALIGN_CENTER);
        buffer = (u8*)ADDHERO_TEXT_TABLE(g_addhero_text_found_load_file, ADDHERO_TEXT_FOUND_LOAD_FILE);
        prim = field_draw_text(prim, ot, ADDHERO_TEXT((u16*)buffer, ADDHERO_TEXT_OVERWRITE_2P_DATA), FIELD_TEXT_COLOR_NORMAL, message_x,
                               ADDHERO_TEXT_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = addhero_draw_choice_prompt(prim, ot, message_x, (ADDHERO_TEXT_LINE_HEIGHT * 2) - y_offset);
        if (g_pad_input & PAD_BTN_CIRCLE)
        {
            addhero_enable_choice_toggle();
            g_addhero_entry_state = ADDHERO_ENTRY_STATE_CONFIRM_NO_SAVE;
            field_play_sound(FIELD_SOUND_ACTION_REFUSED, FIELD_SOUND_PAN_CENTRE);
            field_reset_input_repeat();
        }
        else if (g_pad_input & ADDHERO_CONFIRM_BUTTON_MASK)
        {
            if (g_addhero_choice_toggle != 0)
            {
                addhero_enable_choice_toggle();
                g_addhero_entry_state = ADDHERO_ENTRY_STATE_CONFIRM_NO_SAVE;
                field_play_sound(FIELD_SOUND_ACTION_REFUSED, FIELD_SOUND_PAN_CENTRE);
                field_reset_input_repeat();
            }
            else
            {
                field_play_sound(FIELD_SOUND_SELECT, FIELD_SOUND_PAN_CENTRE);
                buffer = (u8*)&g_addhero_save_file;
                bcopy((u8*)&g_saved_game_ctx->characters[FIELD_PARTY_GUEST], (u8*)&((SaveFile*)buffer)->saved_game.characters[FIELD_PARTY_HERO],
                      sizeof(FieldCharacterRecord));
                ((SaveFile*)buffer)->saved_game.characters[FIELD_PARTY_HERO].info.word |= FIELD_CHARACTER_PAD_CONTROLLED;
                checksum = compute_save_checksum(buffer);
                ((SaveFile*)buffer)->magic = SAVE_FILE_MAGIC;
                ((SaveFile*)buffer)->checksum = checksum;
                g_addhero_write_in_progress = 1;
                g_addhero_load_step = g_addhero_loadseq_save_begin;
                g_addhero_entry_state = ADDHERO_ENTRY_STATE_SAVE_PROGRESS;
            }
        }
    }
    break;
    case ADDHERO_ENTRY_STATE_SAVE_PROGRESS:
    {
        s32 message_x;
        u16* text_table;
        POLY_G4* bar;
        s32 elapsed;
        s32 width;
        AddheroElement* element;
        s32 i;

        message_x = -x_offset + ADDHERO_MESSAGE_WIDTH / 2;
        prim = field_draw_text(prim, ot, ADDHERO_TEXT_AT(g_addhero_text_saving, ADDHERO_TEXT_SAVING), FIELD_TEXT_COLOR_NORMAL, message_x, -y_offset,
                               FIELD_TEXT_ALIGN_CENTER);
        text_table = ADDHERO_TEXT_TABLE(g_addhero_text_saving, ADDHERO_TEXT_SAVING);
        prim = field_draw_text(prim, ot, ADDHERO_TEXT(text_table, ADDHERO_TEXT_DO_NOT_REMOVE_CARD), FIELD_TEXT_COLOR_NORMAL, message_x,
                               ADDHERO_TEXT_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, ADDHERO_TEXT(text_table, ADDHERO_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, message_x,
                               (ADDHERO_TEXT_LINE_HEIGHT * 2) - y_offset, FIELD_TEXT_ALIGN_CENTER);
        bar = prim;
        if (g_addhero_progress_bar_active != 0)
        {
            elapsed = VSync(-1) - g_addhero_progress_start_tick;
            if (elapsed > ADDHERO_PROGRESS_FULL_TICKS)
            {
                elapsed = ADDHERO_PROGRESS_FULL_TICKS;
            }
            width = elapsed * ADDHERO_MESSAGE_WIDTH;
            SET_BGR0_PACKED(bar, GPU_COLOR_WORD(0xFF, 0, 0));
            SET_POLY_G4_BGR1_PACKED(bar, GPU_COLOR_WORD(0xFF, 0xFF, 0));
            SET_POLY_G4_BGR3_PACKED(bar, GPU_COLOR_WORD(0, 0, 0xFF));
            setlen(bar, 8);
            SET_POLY_G4_BGR2_PACKED(bar, GPU_COLOR_WORD(0, 0xFF, 0xFF));
            setcode(bar, GPU_CODE_POLY_G4);
            bar->x2 = 0;
            bar->x0 = 0;
            bar->x1 = bar->x3 = width / ADDHERO_PROGRESS_FULL_TICKS;
            bar->y1 = 0;
            bar->y0 = 0;
            bar->y3 = ADDHERO_MESSAGE_HEIGHT;
            bar->y2 = ADDHERO_MESSAGE_HEIGHT;
            addPrim(ot, bar);
            bar++;
        }
        prim = bar;
        if (g_addhero_write_in_progress == 0)
        {
            g_saved_game_ctx->characters[FIELD_PARTY_GUEST].name[0] = 0;
            field_play_sound(FIELD_SOUND_SAVE_DONE, FIELD_SOUND_PAN_CENTRE);
            g_menu_element_counter = 0x20;
            element = g_addhero_element_pool;
            for (i = 0; i < ADDHERO_ELEMENT_COUNT; i++, element++)
            {
                element->size.bits.scrollable = 0;
                element->attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
            }
            field_restore_fade_target_with_duration(ADDHERO_EXIT_FADE_FRAMES);
            g_addhero_result = ADDHERO_RESULT_SAVED;
        }
    }
    break;
    default:
    {
        s32 message_x;
        s32 selected_y;
        s32 scroll_delta;
        u16* text_table;

        message_x = -x_offset + ADDHERO_MESSAGE_WIDTH / 2;
        text_table = &g_addhero_text_table;
        prim = field_draw_text(prim, ot, ADDHERO_TEXT(text_table, ADDHERO_TEXT_CHECKING_CARD), FIELD_TEXT_COLOR_NORMAL, message_x, -y_offset,
                               FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, ADDHERO_TEXT(text_table, ADDHERO_TEXT_DO_NOT_REMOVE_CARD), FIELD_TEXT_COLOR_NORMAL, message_x,
                               ADDHERO_TEXT_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
        prim = field_draw_text(prim, ot, ADDHERO_TEXT(text_table, ADDHERO_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, message_x,
                               (ADDHERO_TEXT_LINE_HEIGHT * 2) - y_offset, FIELD_TEXT_ALIGN_CENTER);
        if (g_addhero_entry_scan_active == 0)
        {
            if (g_addhero_io_busy != 0)
            {
                return prim;
            }
            if (*g_addhero_load_step >= ADDHERO_STEP_SCAN_ENTRIES && *g_addhero_load_step <= ADDHERO_STEP_SCAN_DONE)
            {
                return prim;
            }
            if ((strncmp(g_lom_save_filename_prefix, g_addhero_entries[g_addhero_card_slot][g_addhero_selected_row].name,
                         ADDHERO_SAVE_FILENAME_PREFIX_LENGTH) != 0) ||
                (g_addhero_entry_file.saved_game.identity.word != g_saved_game_ctx->guest_origin.word))
            {
                g_addhero_selected_row++;
                if (g_addhero_selected_row >= g_addhero_entry_state)
                {
                    if (g_addhero_entry_state != 0)
                    {
                        g_addhero_entry_state = ADDHERO_ENTRY_STATE_NO_LOAD_FILE;
                    }
                    else
                    {
                        g_addhero_entry_state = ADDHERO_ENTRY_STATE_NO_GAME_DATA;
                    }
                }
                else
                {
                    addhero_commit_selected_entry();
                    selected_y = g_addhero_selected_row * ADDHERO_ENTRY_ROW_HEIGHT;
                    scroll_delta = selected_y - g_addhero_scroll_y;
                    if (scroll_delta > ADDHERO_LIST_HEIGHT - ADDHERO_ENTRY_ROW_HEIGHT)
                    {
                        g_addhero_scroll_target_y = selected_y - ADDHERO_LIST_LAST_ROW_Y;
                        g_addhero_scroll_frames = ADDHERO_SCROLL_FRAMES;
                    }
                    if (scroll_delta < 0)
                    {
                        g_addhero_scroll_target_y = selected_y;
                        g_addhero_scroll_frames = ADDHERO_SCROLL_FRAMES;
                    }
                }
            }
            else
            {
                g_addhero_progress_start_tick = VSync(-1);
                g_addhero_progress_active = 1;
                g_addhero_load_step = g_addhero_loadseq_load_progress;
                g_addhero_entry_state = ADDHERO_ENTRY_STATE_LOAD_PROGRESS;
            }
        }
    }
    break;
    case ADDHERO_ENTRY_STATE_BLANK:
        break;
    }

    if (g_addhero_io_busy != 0)
    {
        return prim;
    }
    if (g_addhero_entry_state == ADDHERO_ENTRY_STATE_LOAD_PROGRESS)
    {
        return prim;
    }
    if (g_addhero_entry_state == ADDHERO_ENTRY_STATE_SAVE_PROGRESS)
    {
        return prim;
    }
    if (g_addhero_entry_state == ADDHERO_ENTRY_STATE_SAVE_CONFIRM)
    {
        return prim;
    }
    if (g_addhero_entry_state == ADDHERO_ENTRY_STATE_CONFIRM_NO_SAVE)
    {
        return prim;
    }

    if (g_pad_input & PAD_BTN_CIRCLE)
    {
        AddheroElement* element;
        s32 i;

        D_80122718 = 3;
        field_play_sound(FIELD_SOUND_ACTION_REFUSED, FIELD_SOUND_PAN_CENTRE);
        field_restore_fade_target();
        element = g_addhero_element_pool;
        for (i = 0; i < ADDHERO_ELEMENT_COUNT; i++, element++)
        {
            if (element->attr.bits.state != ADDHERO_ELEMENT_STATE_INACTIVE)
            {
                element->attr.bits.state = ADDHERO_ELEMENT_STATE_CLOSING;
                element->attr.bits.transition_step = ADDHERO_ELEMENT_TRANSITION_STEPS;
            }
        }
        return prim;
    }

    if ((g_pad_input & ADDHERO_CARD_SWITCH_BUTTON_MASK) && (g_addhero_entry_state != ADDHERO_ENTRY_STATE_CHECKING_CARD))
    {
        field_play_sound(FIELD_SOUND_CURSOR, FIELD_SOUND_PAN_CENTRE);
        g_addhero_load_flow_active = 0;
        g_addhero_load_step = NULL;
        g_addhero_scroll_frames = 0;
        g_addhero_scroll_target_y = 0;
        g_addhero_scroll_y = 0;
        g_addhero_selected_row = 0;
        g_addhero_entry_state = ADDHERO_ENTRY_STATE_CHECKING_CARD;
        g_addhero_selection_status = ADDHERO_SELECTION_NONE;
        g_addhero_card_slot ^= 1;
        addhero_reset_entry_ranks();
        field_reset_input_repeat();
        g_addhero_progress_bar_active = 0;
        g_pad_input = 0;
        g_addhero_load_step = &g_addhero_loadseq_start;
    }
    return prim;
}

/**
 * @brief Blit one character-slot icon into VRAM and emit the highlighted
 *        textured quad for it in the detail panel.
 * @param quad  Quad packet to fill.
 * @param ot    Ordering table the quad is linked into.
 * @param x     Left edge of the quad.
 * @param y     Top edge of the quad.
 * @param width Quad width, which animates for the highlighted icon.
 * @param icon  Character icon id for this position (SAVE_NO_ICON means empty).
 * @param index VRAM icon slot: index among the present (non-empty) icons.
 * @param row   Index among all three party slots.
 * @return The primitive cursor after the quad, or @p quad when the slot is empty.
 */
void* addhero_draw_icon_highlight(POLY_FT4* quad, u_long* ot, s32 x, s32 y, s32 width, s32 icon, s32 index, s32 row)
{
    RECT rect;
    s32 column;
    s8 u;

    if (icon == SAVE_NO_ICON)
    {
        return quad;
    }

    setRECT(&rect, index * 16, VRAM_CLUT_Y, 16, 1);
    if ((row == FIELD_PARTY_GUEST) && (icon < SAVE_ICON_HERO_COUNT))
    {
        field_copy_portrait_palette(g_addhero_icon_context, icon);
        LoadImage(&rect, (u_long*)g_addhero_icon_context);
        DrawSync(0);
    }
    else if (icon >= SAVE_ICON_GOLEM_BASE)
    {
        field_copy_golem_portrait_palette(g_addhero_icon_context, g_addhero_icon_palette);
        LoadImage(&rect, (u_long*)g_addhero_icon_context);
        DrawSync(0);
    }
    else
    {
        LoadImage(&rect, (u_long*)ADDHERO_ICON_IMAGE(icon)->clut);
    }

    column = index * 3;
    setRECT(&rect, column * 4 + ADDHERO_ICON_VRAM_X, ADDHERO_ICON_VRAM_Y, ADDHERO_ICON_SIZE / 4, ADDHERO_ICON_SIZE);
    LoadImage(&rect, (u_long*)ADDHERO_ICON_IMAGE(icon)->pixels);

    SET_BGR0_PACKED(quad, GPU_TINT_NEUTRAL);
    setPolyFT4(quad);
    quad->x2 = x;
    quad->x0 = x;
    quad->y1 = y;
    quad->y0 = y;
    quad->x3 = x + width;
    u = column * 16;
    quad->u2 = u;
    quad->u0 = u;
    u += ADDHERO_ICON_SIZE - 1;
    quad->u3 = u;
    quad->u1 = u;
    quad->v1 = ADDHERO_ICON_VRAM_Y;
    quad->v0 = ADDHERO_ICON_VRAM_Y;
    quad->x1 = x + width;
    quad->y3 = y + ADDHERO_ICON_SIZE - 1;
    quad->y2 = y + ADDHERO_ICON_SIZE - 1;
    quad->v3 = ADDHERO_ICON_VRAM_Y + ADDHERO_ICON_SIZE - 1;
    quad->v2 = ADDHERO_ICON_VRAM_Y + ADDHERO_ICON_SIZE - 1;
    quad->clut = getClut(index * 16, VRAM_CLUT_Y);
    quad->tpage = getTPage(0, 0, ADDHERO_ICON_VRAM_X, 0);
    addPrim(ot, quad);

    return quad + 1;
}

/**
 * @brief Select the second (cancel) option as the default choice.
 * @note JP selects the first option instead.
 */
void addhero_enable_choice_toggle(void)
{
#if defined(VERSION_JP)
    g_addhero_choice_toggle = 0;
#else
    g_addhero_choice_toggle = 1;
#endif
}

/**
 * @brief Draw the two-option (yes/no) choice glyphs, highlighting the current
 *        selection, and flip the selection on left/right pad input.
 * @param prim Current primitive pointer/index.
 * @param ot   Ordering table the glyphs are linked into.
 * @param x    Center X the two options are placed around.
 * @param y    Baseline Y for both options.
 * @return The updated primitive pointer.
 */
void* addhero_draw_choice_prompt(void* prim, u_long* ot, s32 x, s32 y)
{
    u8* text_table;
    u8* text;
    s32 color;

    color = FIELD_TEXT_COLOR_NORMAL;
    text = FIELD_UI_TEXT_AT(g_text_choice_glyph_offsets, FIELD_UI_TEXT_YES);
    text_table = g_text_choice_glyph_offsets - FIELD_UI_TEXT_YES * 2;
    if (g_addhero_choice_toggle != 0)
    {
        color = FIELD_TEXT_COLOR_DIM;
    }
    prim = field_draw_text(prim, ot, text, color, x - ADDHERO_CHOICE_YES_GAP, y, FIELD_TEXT_ALIGN_RIGHT);

    color = FIELD_TEXT_COLOR_NORMAL;
    text = FIELD_UI_TEXT(text_table, FIELD_UI_TEXT_NO);
    if (g_addhero_choice_toggle == 0)
    {
        color = FIELD_TEXT_COLOR_DIM;
    }
    prim = field_draw_text(prim, ot, text, color, x + ADDHERO_CHOICE_NO_GAP, y, FIELD_TEXT_ALIGN_LEFT);
    if (g_pad_input & (PAD_BTN_LEFT | PAD_BTN_RIGHT))
    {
        g_addhero_choice_toggle ^= 1;
        field_play_sound(FIELD_SOUND_CURSOR, FIELD_SOUND_PAN_CENTRE);
        g_pad_input = 0;
    }
    return prim;
}

#include "../common/validate_save_file.inc.c"
#include "../common/compute_save_checksum.inc.c"
