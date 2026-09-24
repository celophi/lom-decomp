#include "field_text.h"
#include "addhero_internal.h"

/** @brief One save icon in the icon-set resource: a 16-color CLUT followed by 48x48 4-bit pixels. */
typedef struct
{
    u16 clut[16];
    u8 pixels[48 * 48 / 2];
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
 * @param x_offset Horizontal offset; screen X = 0x90 - x_offset.
 * @param y_offset Vertical offset applied to the prompt rows.
 * @return The updated primitive pointer after linking the prompt.
 * @see decomp.me (100%)
 */
void* addhero_draw_load_prompt(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT unused; /* never used, but the original stack frame reserves it */
    void* result;
    s32 x;
    s32 status;
    AddheroElement* p;

    x = -x_offset + 0x90;
    result = addhero_draw_choice_prompt(func_800A88A0(prim, ot, ADDHERO_TEXT_AT(g_addhero_glyph_load_prompt, 24), 4, x, -y_offset, 2), ot, x, 0xE - y_offset);

    status = addhero_poll_and_retry_card_info();
    if (status == ADDHERO_CARD_EVENT_ERROR || status == ADDHERO_CARD_EVENT_TIMEOUT)
    {
        g_addhero_element_pool[0].attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
        field_reset_input_repeat();
        play_menu_sfx(0x78, 0x80);
        g_addhero_entry_state = ADDHERO_ENTRY_STATE_IDLE;
        addhero_reset_entry_ranks();
        g_addhero_load_step = NULL;
    }
    else
    {
        if (g_pad_input & PAD_BTN_CIRCLE)
        {
            g_addhero_element_pool[0].attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
            field_reset_input_repeat();
            play_menu_sfx(0x78, 0x80);
            g_addhero_load_step = g_addhero_loadseq_abort;
        }
        else if (g_pad_input & ADDHERO_CONFIRM_BUTTON_MASK)
        {
            if (g_addhero_choice_toggle != 0)
            {
                g_addhero_element_pool[0].attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
                field_reset_input_repeat();
                play_menu_sfx(0x78, 0x80);
                g_addhero_load_step = g_addhero_loadseq_abort;
            }
            else
            {
                play_menu_sfx(0x7E, 0x80);
                g_addhero_progress_active = 1;
                g_addhero_load_step = g_addhero_loadseq_load_begin;
                p = &g_addhero_element_pool[0];
                p->draw_handler = addhero_draw_load_progress;
                p->attr.bits.transition_step = 1;
                p->attr.bits.state = ADDHERO_ELEMENT_STATE_OPENING;
                p->attr.bits.x = 0x10;
                p->attr.bits.y = 0x61;
                p->size.bits.width_high = 1;
                p->size.bits.height = 0x2C;
                ADDHERO_SET_ELEMENT_WIDTH_LOW(p, 0x20);
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
 * @param x_offset Horizontal offset used to place the prompt (screen X = 0x90 - x_offset).
 * @param y_offset Vertical offset used to place the prompt rows.
 * @return The updated primitive pointer / index after linking the prompt.
 * @see decomp.me (100%)
 */
void* addhero_draw_load_progress(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT unused; /* never used, but the original stack frame reserves it */
    u16* text_table;
    u8* resource;
    AddheroElement* element;
    void* result;
    s32 x;
    s32 i;
    u32 saved;

    x = -x_offset + 0x90;
    result = func_800A88A0(prim, ot, ADDHERO_TEXT_AT(g_addhero_glyph_load_progress, 25), 4, x, -y_offset, 2);
    text_table = ADDHERO_TEXT_TABLE(g_addhero_glyph_load_progress, 25);
    result = func_800A88A0(result, ot, ADDHERO_TEXT(text_table, 15), 4, x, 0xE - y_offset, 2);
    result = func_800A88A0(result, ot, ADDHERO_TEXT(text_table, 89), 4, x, 0x1C - y_offset, 2);
    result = addhero_draw_progress_bar(result, ot);

    if (g_addhero_progress_active == 0)
    {
        resource = g_addhero_save_blob;
        g_addhero_element_pool[0].attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
        if (addhero_validate_save_blob(resource) == 0)
        {
            addhero_open_status_dialog(4);
            return result;
        }

        play_menu_sfx(0x7B, 0x80);
        saved = g_pad_ctx[0x858] >> 7;
        bcopy(&((AddheroSaveBlob*)resource)->context, g_pad_ctx + 0x840, sizeof(AddheroSaveContextBlock));
        *(u32*)(g_pad_ctx + 0x858) = (*(u32*)(g_pad_ctx + 0x858) & ~0x80) | (saved << 7);
        *(u16*)(g_pad_ctx + 0xD8) = *(u16*)(resource + 0x254);
        *(u16*)(g_pad_ctx + 0xDA) = *(u16*)(resource + 0x256);
        *(u16*)(g_pad_ctx + 0xDE) = 1;
        field_restore_fade_target();

        element = g_addhero_element_pool;
        for (i = 0; i < ADDHERO_ELEMENT_COUNT; i++, element++)
        {
            if (element->attr.bits.state != ADDHERO_ELEMENT_STATE_INACTIVE)
            {
                element->attr.bits.state = ADDHERO_ELEMENT_STATE_CLOSING;
                element->attr.bits.transition_step = 8;
            }
        }
        field_restore_fade_target_with_duration(8);
        g_addhero_result = 1;
    }

    return result;
}

/**
 * @brief Draw the gradient progress bar whose width tracks elapsed ticks, when
 *        the bar is active.
 * @param prim Current primitive pointer/index the POLY_G4 is written to.
 * @param ot   Ordering table the primitive is linked into.
 * @return The advanced primitive pointer, unchanged when the bar is inactive.
 * @see decomp.me (100%)
 */
void* addhero_draw_progress_bar(POLY_G4* quad, u_long* ot)
{
    s32 elapsed;
    s32 width;

    if (g_addhero_progress_bar_active != 0)
    {
        elapsed = VSync(-1) - g_addhero_progress_start_tick;
        if (elapsed > 256)
        {
            elapsed = 256;
        }
        width = elapsed * 288;
        SET_BGR0_PACKED(quad, GPU_COLOR_WORD(0xFF, 0, 0));
        SET_POLY_G4_BGR1_PACKED(quad, GPU_COLOR_WORD(0xFF, 0xFF, 0));
        SET_POLY_G4_BGR3_PACKED(quad, GPU_COLOR_WORD(0, 0, 0xFF));
        setlen(quad, 8);
        SET_POLY_G4_BGR2_PACKED(quad, GPU_COLOR_WORD(0, 0xFF, 0xFF));
        setcode(quad, 0x38);
        quad->x2 = 0;
        quad->x0 = 0;
        quad->x1 = quad->x3 = width / 256;
        quad->y1 = 0;
        quad->y0 = 0;
        quad->y3 = 44;
        quad->y2 = 44;
        addPrim(ot, quad);
        quad++;
    }
    return quad;
}

/**
 * @brief Reconfigure the primary element as a modal status dialog and reset the
 *        browser/IO state, storing the dialog message id.
 * @param message_id Dialog message id stored in g_addhero_dialog_state.
 * @see decomp.me (100%)
 */
void addhero_open_status_dialog(s32 message_id)
{
    play_menu_sfx(0x78, 0x80);
    g_addhero_element_pool[0].draw_handler = addhero_draw_status_dialog;
    g_addhero_element_pool[0].attr.bits.transition_step = 1;
    g_addhero_element_pool[0].attr.bits.state = ADDHERO_ELEMENT_STATE_OPENING;
    g_addhero_element_pool[0].attr.bits.x = 0x20;
    g_addhero_element_pool[0].attr.bits.y = 0x70;
    g_addhero_element_pool[0].size.bits.width_high = 1;
    g_addhero_element_pool[0].size.bits.height = 0x14;
    ADDHERO_SET_ELEMENT_WIDTH_LOW(&g_addhero_element_pool[0], 0);
    field_reset_input_repeat();
    g_addhero_write_in_progress = 0;
    g_addhero_progress_active = 0;
    g_addhero_selection_status = 0;
    g_addhero_io_busy = 0;
    g_addhero_entry_state = ADDHERO_ENTRY_STATE_IDLE;
    addhero_reset_entry_ranks();
    g_addhero_load_step = NULL;
    g_addhero_dialog_state = message_id;
}

/**
 * @brief Reconfigure the primary element as a modal exit dialog and reset the
 *        browser/IO state, storing the dialog message id.
 * @param message_id Dialog message id stored in g_addhero_dialog_state.
 * @see decomp.me (100%)
 */
void addhero_open_exit_dialog(s32 message_id)
{
    play_menu_sfx(0x78, 0x80);
    g_addhero_element1.draw_handler = addhero_draw_exit_dialog;
    g_addhero_element1.attr.bits.transition_step = 1;
    g_addhero_element1.attr.bits.state = ADDHERO_ELEMENT_STATE_OPENING;
    g_addhero_element1.attr.bits.x = 0x20;
    g_addhero_element1.attr.bits.y = 0x70;
    g_addhero_element1.size.bits.width_high = 1;
    g_addhero_element1.size.bits.height = 0x14;
    ADDHERO_SET_ELEMENT_WIDTH_LOW(&g_addhero_element1, 0);
    field_reset_input_repeat();
    g_addhero_write_in_progress = 0;
    g_addhero_progress_active = 0;
    g_addhero_selection_status = 0;
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
 * @see decomp.me (100%)
 */
void* addhero_draw_status_dialog(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT unused; /* never used, but the original stack frame reserves it */

    switch (g_addhero_dialog_state)
    {
    case 0:
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT_AT(g_addhero_glyph_dialog_msg0, 30), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 2:
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT_AT(g_addhero_glyph_dialog_msg2, 32), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 3:
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT_AT(g_addhero_glyph_dialog_msg3, 33), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 1:
    case 4:
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT_AT(g_addhero_glyph_dialog_msg1, 31), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    }
    if (g_pad_input & ADDHERO_CONFIRM_BUTTON_MASK)
    {
        g_addhero_element_pool[0].attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
        field_reset_input_repeat();
    }
    return prim;
}

/**
 * @brief Draw the exit dialog message and, on acknowledge, tear down all
 *        elements and request the overlay to exit with result 3.
 * @param ot   Ordering table the message glyph is linked into.
 * @param prim Current primitive pointer/index.
 * @param x_offset Horizontal offset; screen X is derived from it.
 * @param y_offset Vertical offset.
 * @return The updated primitive pointer.
 * @see decomp.me (100%)
 */
void* addhero_draw_exit_dialog(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT unused; /* never used, but the original stack frame reserves it */
    AddheroElement* p;
    s32 i;

    switch (g_addhero_dialog_state)
    {
    case 0:
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT_AT(g_addhero_glyph_dialog_msg0, 30), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 2:
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT_AT(g_addhero_glyph_dialog_msg2, 32), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 3:
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT_AT(g_addhero_glyph_dialog_msg3, 33), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 1:
    case 4:
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT_AT(g_addhero_glyph_dialog_msg1, 31), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    }
    if (g_pad_input & ADDHERO_CONFIRM_BUTTON_MASK)
    {
        g_addhero_result = 3;
        g_menu_element_counter = 0x20;
        p = &g_addhero_element_pool[0];
        for (i = 0; i < ADDHERO_ELEMENT_COUNT; i++)
        {
            p->size.bits.scrollable = 0;
            p->attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
            p++;
        }
        field_restore_fade_target_with_duration(8);
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
 * @param x_offset Horizontal offset; screen X = 0x90 - x_offset.
 * @param y_offset Vertical offset applied to the message rows.
 * @return The updated primitive pointer.
 */
void* addhero_draw_transfer_status(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    RECT unused; /* never used, but the original stack frame reserves it */

    switch (g_addhero_entry_state)
    {
    case 0xF8:
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT_AT(g_addhero_glyph_status_f8, 26), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case 0xF9:
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT_AT(g_addhero_glyph_status_f8, 26), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case ADDHERO_ENTRY_STATE_IDLE:
    {
        s32 message_x;
        u16* text_table;

        message_x = -x_offset + 0x90;
        text_table = &g_addhero_glyph_table;
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT(text_table, 0), 4, message_x, -y_offset, 2);
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT(text_table, 15), 4, message_x, 0xE - y_offset, 2);
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT(text_table, 89), 4, message_x, 0x1C - y_offset, 2);
    }
    break;
    case ADDHERO_ENTRY_STATE_CARD_FULL:
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT_AT(g_addhero_glyph_status_f8, 26), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case ADDHERO_ENTRY_STATE_CARD_IO_ERROR:
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT_AT(g_addhero_glyph_status_fd, 2), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case 0xFB:
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT_AT(g_addhero_glyph_status_fb, 8), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case 0xFC:
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT_AT(g_addhero_glyph_status_fc, 9), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case 0xF7:
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT_AT(g_addhero_glyph_status_f7, 52), 4, -x_offset + 0x90, -y_offset, 2);
        break;
    case ADDHERO_ENTRY_STATE_LOAD_PROGRESS:
    {
        s32 message_x;
        u16* text_table;
        POLY_G4* bar;
        s32 elapsed;
        s32 width;

        message_x = -x_offset + 0x90;
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT_AT(g_addhero_glyph_load_progress, 25), 4, message_x, -y_offset, 2);
        text_table = ADDHERO_TEXT_TABLE(g_addhero_glyph_load_progress, 25);
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT(text_table, 15), 4, message_x, 0xE - y_offset, 2);
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT(text_table, 89), 4, message_x, 0x1C - y_offset, 2);
        bar = prim;
        if (g_addhero_progress_bar_active != 0)
        {
            elapsed = VSync(-1) - g_addhero_progress_start_tick;
            if (elapsed > 256)
            {
                elapsed = 256;
            }
            width = elapsed * 288;
            SET_BGR0_PACKED(bar, GPU_COLOR_WORD(0xFF, 0, 0));
            SET_POLY_G4_BGR1_PACKED(bar, GPU_COLOR_WORD(0xFF, 0xFF, 0));
            SET_POLY_G4_BGR3_PACKED(bar, GPU_COLOR_WORD(0, 0, 0xFF));
            setlen(bar, 8);
            SET_POLY_G4_BGR2_PACKED(bar, GPU_COLOR_WORD(0, 0xFF, 0xFF));
            setcode(bar, 0x38);
            bar->x2 = 0;
            bar->x0 = 0;
            bar->x1 = bar->x3 = width / 256;
            bar->y1 = 0;
            bar->y0 = 0;
            bar->y3 = 44;
            bar->y2 = 44;
            addPrim(ot, bar);
            bar++;
        }
        prim = bar;
        if (g_addhero_progress_active == 0)
        {
            if (addhero_validate_save_blob(g_addhero_save_blob) == 0)
            {
                s32 message_id = 4;

                play_menu_sfx(0x78, 0x80);
                g_addhero_element_pool[0].draw_handler = addhero_draw_status_dialog;
                g_addhero_element_pool[0].attr.bits.transition_step = 1;
                g_addhero_element_pool[0].attr.bits.state = ADDHERO_ELEMENT_STATE_OPENING;
                g_addhero_element_pool[0].attr.bits.x = 0x20;
                g_addhero_element_pool[0].attr.bits.y = 0x70;
                g_addhero_element_pool[0].size.bits.width_high = 1;
                g_addhero_element_pool[0].size.bits.height = 0x14;
                ADDHERO_SET_ELEMENT_WIDTH_LOW(&g_addhero_element_pool[0], 0);
                field_reset_input_repeat();
                g_addhero_write_in_progress = 0;
                g_addhero_selection_status = 0;
                g_addhero_io_busy = 0;
                g_addhero_progress_active = 0;
                g_addhero_entry_state = ADDHERO_ENTRY_STATE_IDLE;
                addhero_reset_entry_ranks();
                g_addhero_load_step = NULL;
                g_addhero_dialog_state = message_id;
                return prim;
            }
            play_menu_sfx(0x7B, 0x80);
            g_addhero_entry_state = ADDHERO_ENTRY_STATE_SAVE_CONFIRM;
            addhero_enable_choice_toggle();
            field_reset_input_repeat();
        }
    }
    break;
    case ADDHERO_ENTRY_STATE_CONFIRM_PROMPT:
    {
        s32 message_x;
        AddheroElement* element;
        s32 i;

        message_x = -x_offset + 0x90;
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT_AT(g_addhero_glyph_status_f3, 55), 4, message_x, -y_offset, 2);
        prim = addhero_draw_choice_prompt(prim, ot, message_x, 0xE - y_offset);
        if (g_pad_input & PAD_BTN_CIRCLE)
        {
            play_menu_sfx(0x78, 0x80);
            addhero_enable_choice_toggle();
            g_addhero_entry_state = ADDHERO_ENTRY_STATE_SAVE_CONFIRM;
            field_reset_input_repeat();
        }
        else if (g_pad_input & ADDHERO_CONFIRM_BUTTON_MASK)
        {
            if (g_addhero_choice_toggle != 0)
            {
                play_menu_sfx(0x78, 0x80);
                addhero_enable_choice_toggle();
                g_addhero_entry_state = ADDHERO_ENTRY_STATE_SAVE_CONFIRM;
                field_reset_input_repeat();
            }
            else
            {
                play_menu_sfx(0x7D, 0x80);
                g_addhero_result = 3;
                g_menu_element_counter = 0x20;
                element = g_addhero_element_pool;
                for (i = 0; i < ADDHERO_ELEMENT_COUNT; i++, element++)
                {
                    element->size.bits.scrollable = 0;
                    element->attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
                }
                field_restore_fade_target_with_duration(8);
                field_reset_input_repeat();
            }
        }
    }
    break;
    case ADDHERO_ENTRY_STATE_SAVE_CONFIRM:
    {
        s32 message_x;
        u8* buffer; /* the text table, then the save blob; one pointer serves both */
        s32 checksum;

        message_x = -x_offset + 0x90;
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT_AT(g_addhero_glyph_save_confirm_msg, 53), 4, message_x, -y_offset, 2);
        buffer = (u8*)ADDHERO_TEXT_TABLE(g_addhero_glyph_save_confirm_msg, 53);
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT((u16*)buffer, 54), 4, message_x, 0xE - y_offset, 2);
        prim = addhero_draw_choice_prompt(prim, ot, message_x, 0x1C - y_offset);
        if (g_pad_input & PAD_BTN_CIRCLE)
        {
            addhero_enable_choice_toggle();
            g_addhero_entry_state = ADDHERO_ENTRY_STATE_CONFIRM_PROMPT;
            play_menu_sfx(0x78, 0x80);
            field_reset_input_repeat();
        }
        else if (g_pad_input & ADDHERO_CONFIRM_BUTTON_MASK)
        {
            if (g_addhero_choice_toggle != 0)
            {
                addhero_enable_choice_toggle();
                g_addhero_entry_state = ADDHERO_ENTRY_STATE_CONFIRM_PROMPT;
                play_menu_sfx(0x78, 0x80);
                field_reset_input_repeat();
            }
            else
            {
                play_menu_sfx(0x7E, 0x80);
                buffer = g_addhero_save_blob;
                bcopy(g_pad_ctx + 0x840, &((AddheroSaveBlob*)buffer)->context, sizeof(AddheroSaveContextBlock));
                ((AddheroSaveBlob*)buffer)->context.inject_flags |= ADDHERO_INPUT_INJECTION_ENABLED;
                checksum = addhero_compute_save_checksum(buffer);
                ((AddheroSaveBlob*)buffer)->magic = ADDHERO_SAVE_MAGIC;
                ((AddheroSaveBlob*)buffer)->checksum = checksum;
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

        message_x = -x_offset + 0x90;
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT_AT(g_addhero_glyph_save_progress, 14), 4, message_x, -y_offset, 2);
        text_table = ADDHERO_TEXT_TABLE(g_addhero_glyph_save_progress, 14);
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT(text_table, 15), 4, message_x, 0xE - y_offset, 2);
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT(text_table, 89), 4, message_x, 0x1C - y_offset, 2);
        bar = prim;
        if (g_addhero_progress_bar_active != 0)
        {
            elapsed = VSync(-1) - g_addhero_progress_start_tick;
            if (elapsed > 256)
            {
                elapsed = 256;
            }
            width = elapsed * 288;
            SET_BGR0_PACKED(bar, GPU_COLOR_WORD(0xFF, 0, 0));
            SET_POLY_G4_BGR1_PACKED(bar, GPU_COLOR_WORD(0xFF, 0xFF, 0));
            SET_POLY_G4_BGR3_PACKED(bar, GPU_COLOR_WORD(0, 0, 0xFF));
            setlen(bar, 8);
            SET_POLY_G4_BGR2_PACKED(bar, GPU_COLOR_WORD(0, 0xFF, 0xFF));
            setcode(bar, 0x38);
            bar->x2 = 0;
            bar->x0 = 0;
            bar->x1 = bar->x3 = width / 256;
            bar->y1 = 0;
            bar->y0 = 0;
            bar->y3 = 44;
            bar->y2 = 44;
            addPrim(ot, bar);
            bar++;
        }
        prim = bar;
        if (g_addhero_write_in_progress == 0)
        {
            g_pad_ctx[0x840] = 0;
            play_menu_sfx(0x7A, 0x80);
            g_menu_element_counter = 0x20;
            element = g_addhero_element_pool;
            for (i = 0; i < ADDHERO_ELEMENT_COUNT; i++, element++)
            {
                element->size.bits.scrollable = 0;
                element->attr.bits.state = ADDHERO_ELEMENT_STATE_INACTIVE;
            }
            field_restore_fade_target_with_duration(8);
            g_addhero_result = 2;
        }
    }
    break;
    default:
    {
        s32 message_x;
        s32 selected_y;
        s32 scroll_delta;
        u16* text_table;

        message_x = -x_offset + 0x90;
        text_table = &g_addhero_glyph_table;
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT(text_table, 0), 4, message_x, -y_offset, 2);
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT(text_table, 15), 4, message_x, 0xE - y_offset, 2);
        prim = func_800A88A0(prim, ot, ADDHERO_TEXT(text_table, 89), 4, message_x, 0x1C - y_offset, 2);
        if (g_addhero_entry_scan_active == 0)
        {
            if (g_addhero_io_busy != 0)
            {
                return prim;
            }
            if (*g_addhero_load_step >= 6 && *g_addhero_load_step <= 7)
            {
                return prim;
            }
            if ((strncmp(g_lom_save_filename_prefix, g_addhero_entries[g_addhero_card_slot][g_addhero_selected_row].name, 0xC) != 0) ||
                (g_addhero_entry_identity != ((AddheroRecord*)g_pad_ctx)->identity))
            {
                g_addhero_selected_row++;
                if (g_addhero_selected_row >= g_addhero_entry_state)
                {
                    if (g_addhero_entry_state != 0)
                    {
                        g_addhero_entry_state = 0xF7;
                    }
                    else
                    {
                        g_addhero_entry_state = 0xF8;
                    }
                }
                else
                {
                    addhero_commit_selected_entry();
                    selected_y = g_addhero_selected_row * 0xE;
                    scroll_delta = selected_y - g_addhero_scroll_y;
                    if (scroll_delta >= 0x4B)
                    {
                        g_addhero_scroll_target_y = selected_y - 0x46;
                        g_addhero_scroll_frames = 4;
                    }
                    if (scroll_delta < 0)
                    {
                        g_addhero_scroll_target_y = selected_y;
                        g_addhero_scroll_frames = 4;
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
    case 0xFE:
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
    if (g_addhero_entry_state == ADDHERO_ENTRY_STATE_CONFIRM_PROMPT)
    {
        return prim;
    }

    if (g_pad_input & PAD_BTN_CIRCLE)
    {
        AddheroElement* element;
        s32 i;

        D_80122718 = 3;
        play_menu_sfx(0x78, 0x80);
        field_restore_fade_target();
        element = g_addhero_element_pool;
        for (i = 0; i < ADDHERO_ELEMENT_COUNT; i++, element++)
        {
            if (element->attr.bits.state != ADDHERO_ELEMENT_STATE_INACTIVE)
            {
                element->attr.bits.state = ADDHERO_ELEMENT_STATE_CLOSING;
                element->attr.bits.transition_step = 8;
            }
        }
        return prim;
    }

    if ((g_pad_input & ADDHERO_CARD_SWITCH_BUTTON_MASK) && (g_addhero_entry_state != ADDHERO_ENTRY_STATE_IDLE))
    {
        play_menu_sfx(0x7D, 0x80);
        g_addhero_load_flow_active = 0;
        g_addhero_load_step = NULL;
        g_addhero_scroll_frames = 0;
        g_addhero_scroll_target_y = 0;
        g_addhero_scroll_y = 0;
        g_addhero_selected_row = 0;
        g_addhero_entry_state = ADDHERO_ENTRY_STATE_IDLE;
        g_addhero_selection_status = 0;
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
 * @param icon  Character icon id for this position (ADDHERO_NO_ICON means empty).
 * @param index VRAM icon slot: index among the present (non-empty) icons.
 * @param row   Index among all three party slots.
 * @return The primitive cursor after the quad, or @p quad when the slot is empty.
 * @see decomp.me (100%)
 */
void* addhero_draw_icon_highlight(POLY_FT4* quad, u_long* ot, s32 x, s32 y, s32 width, s32 icon, s32 index, s32 row)
{
    RECT rect;
    s32 column;
    s8 u;

    if (icon == ADDHERO_NO_ICON)
    {
        return quad;
    }

    setRECT(&rect, index * 16, VRAM_CLUT_Y, 16, 1);
    if ((row == 1) && (icon < 2))
    {
        func_800A5638(g_addhero_icon_context, icon);
        LoadImage(&rect, (u_long*)g_addhero_icon_context);
        DrawSync(0);
    }
    else if (icon >= 0x4F)
    {
        func_800A55E4(g_addhero_icon_context, g_addhero_icon_palette);
        LoadImage(&rect, (u_long*)g_addhero_icon_context);
        DrawSync(0);
    }
    else
    {
        LoadImage(&rect, (u_long*)ADDHERO_ICON_IMAGE(icon)->clut);
    }

    column = index * 3;
    setRECT(&rect, column * 4 + SCREEN_WIDTH, 208, 12, 48);
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
    u += 47;
    quad->u3 = u;
    quad->u1 = u;
    quad->v1 = 208;
    quad->v0 = 208;
    quad->x1 = x + width;
    quad->y3 = y + 47;
    quad->y2 = y + 47;
    quad->v3 = 255;
    quad->v2 = 255;
    quad->clut = getClut(index * 16, VRAM_CLUT_Y);
    quad->tpage = getTPage(0, 0, 320, 0);
    addPrim(ot, quad);

    return quad + 1;
}

/**
 * @brief Select the second (cancel) option as the default choice.
 * @see decomp.me (100%)
 */
void addhero_enable_choice_toggle(void)
{
    g_addhero_choice_toggle = 1;
}

/**
 * @brief Draw the two-option (yes/no) choice glyphs, highlighting the current
 *        selection, and flip the selection on left/right pad input.
 * @param prim Current primitive pointer/index.
 * @param ot   Ordering table the glyphs are linked into.
 * @param x    Center X the two options are placed around.
 * @param y    Baseline Y for both options.
 * @return The updated primitive pointer.
 * @see decomp.me (100%)
 */
void* addhero_draw_choice_prompt(void* prim, u_long* ot, s32 x, s32 y)
{
    u8* text_table;
    u8* text;
    s32 color;

    color = 4;
    text = FIELD_UI_TEXT_AT(g_text_choice_glyph_offsets, 27);
    text_table = g_text_choice_glyph_offsets - 27 * 2;
    if (g_addhero_choice_toggle != 0)
    {
        color = 5;
    }
    prim = func_800A88A0(prim, ot, text, color, x - 0x10, y, 1);

    color = 4;
    text = FIELD_UI_TEXT(text_table, 28);
    if (g_addhero_choice_toggle == 0)
    {
        color = 5;
    }
    prim = func_800A88A0(prim, ot, text, color, x + 8, y, 0);
    if (g_pad_input & (PAD_BTN_LEFT | PAD_BTN_RIGHT))
    {
        g_addhero_choice_toggle ^= 1;
        play_menu_sfx(0x7D, 0x80);
        g_pad_input = 0;
    }
    return prim;
}

/**
 * @brief Validate a loaded save blob by checking its stored checksum and the
 *        "ANA" magic tag.
 * @param base Base of the 0x4000-byte save blob.
 * @return 1 when the checksum and magic both match, 0 otherwise.
 */
s32 addhero_validate_save_blob(u8* base)
{
    AddheroSaveBlob* save;

    save = (AddheroSaveBlob*)base;
    if (save->checksum == addhero_compute_save_checksum(base))
    {
        if (save->magic == ADDHERO_SAVE_MAGIC)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Compute the save-blob checksum over the first 0x33E0 bytes.
 * @param data Base of the save blob.
 * @return Twice the byte sum plus ADDHERO_SAVE_CHECKSUM_BIAS.
 */
s32 addhero_compute_save_checksum(u8* data)
{
    s32 sum;
    u32 bytes_read;

    sum = 0;
    bytes_read = 0;
    do
    {
        bytes_read++;
        sum += *data++;
    } while (bytes_read < ADDHERO_SAVE_CHECKSUM_BYTES);
    return (sum * 2) + ADDHERO_SAVE_CHECKSUM_BIAS;
}
