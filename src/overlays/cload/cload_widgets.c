#include "internal/cload_internal.h"
#include "overlays/field/field_text.h"
#include "main/display.h"
#include <memory.h>
#include <libetc.h>
#include "common/gpu_packet.h"
#include "overlays/field/field_input.h"

/** @brief Fixed scratch buffer the icon set is read into. */
#define CLOAD_ICON_SET_BUFFER ((CloadIconSetHeader*)LOAD_BUFFER_ADDRESS)

/** @brief Images in the icon set: two 128-pixel backdrop slices and a third holding the last 48 pixels and the scroll arrows. */
#define CLOAD_ICON_SET_IMAGE_COUNT 3

/** @brief Header of the save-icon set resource; all offsets are bytes from its start. */
typedef struct
{
    s32 unknown_0; /**< TODO: unknown; CLOAD never reads it. */
    s32 icon_table_offset;
    s32 image_offsets[CLOAD_ICON_SET_IMAGE_COUNT];
} CloadIconSetHeader;

/** @brief VRAM home of the icon-set images: left edge of the first, the step between them, and the first one's CLUT row. */
#define CLOAD_ICON_SET_VRAM_X 512
#define CLOAD_ICON_SET_VRAM_STEP 64
#define CLOAD_ICON_SET_CLUT_Y 500

/** @brief Left edge of the backdrop on screen. */
#define CLOAD_BACKDROP_X 8

/** @brief Width of the two full backdrop slices and of the third image's backdrop part. */
#define CLOAD_BACKDROP_SLICE_WIDTH 128
#define CLOAD_BACKDROP_LAST_SLICE_WIDTH 48

/** @brief Width and height of a scroll arrow; the two arrows are stacked in the third icon-set image. */
#define CLOAD_SCROLL_ARROW_SIZE 16

/** @brief Inset of a window's clipped contents from its frame. */
#define CLOAD_FRAME_BORDER 2

/** @brief Icon @p icon in the loaded icon table (a counted offset table at the table start). */
#define CLOAD_ICON_IMAGE(icon) ((FieldPortrait*)CARD_OFFSET_TABLE_ENTRY((s32*)g_cload_icon_resource + 1, icon))

/**
 * @brief Emit the GPU packets for a CLOAD window frame: a clipped draw
 *        environment and, optionally, three outlines plus a translucent fill.
 * @param prim Primitive-buffer cursor.
 * @param ot Ordering-table entry the packets are linked into.
 * @param x Window left edge.
 * @param y Window top edge.
 * @param w Window width.
 * @param h Window height.
 * @param front_buffer Nonzero when drawing into the buffer whose draw area starts at y = SCREEN_HEIGHT.
 * @param draw_fill Nonzero to emit the outline and fill packets.
 * @return Advanced primitive-buffer cursor.
 * @note The contents are clipped CLOAD_FRAME_BORDER pixels inside the frame; the
 *       clip is one row taller than h - 2 * CLOAD_FRAME_BORDER (see CLOAD_WINDOW_HEIGHT).
 */
void* cload_emit_window_frame(void* prim, u_long* ot, s32 x, s32 y, s32 w, s32 h, s32 front_buffer, s32 draw_fill)
{
    DRAWENV draw_env;
    DR_ENV* env_packet;
    TILE* tile;
    DR_TPAGE* draw_mode;
    void* next_prim;

    env_packet = (DR_ENV*)prim;
    if (front_buffer != 0)
    {
        SetDefDrawEnv(&draw_env, x + CLOAD_FRAME_BORDER, y + SCREEN_HEIGHT + CLOAD_FRAME_BORDER, w - 2 * CLOAD_FRAME_BORDER,
                      h + 1 - 2 * CLOAD_FRAME_BORDER);
    }
    else
    {
        SetDefDrawEnv(&draw_env, x + CLOAD_FRAME_BORDER, y + VRAM_BACK_DRAW_Y + CLOAD_FRAME_BORDER, w - 2 * CLOAD_FRAME_BORDER,
                      h + 1 - 2 * CLOAD_FRAME_BORDER);
    }
    SetDrawEnv(env_packet, &draw_env);
    addPrim(ot, env_packet);

    next_prim = env_packet + 1;
    if (draw_fill != 0)
    {
        next_prim = cload_emit_rect_outline(next_prim, ot, x, y, w, h, GPU_COLOR_WHITE);
        next_prim = cload_emit_rect_outline(next_prim, ot, x + 1, y + 1, w - 2, h - 2, GPU_COLOR_BLACK);
        next_prim = cload_emit_rect_outline(next_prim, ot, x - 1, y - 1, w + 2, h + 2, GPU_COLOR_BLACK);

        tile = next_prim;
        SET_BGR0_PACKED(tile, GPU_TINT_NEUTRAL);
        setTile(tile);
        setSemiTrans(tile, 1);
        setXY0(tile, x, y);
        setWH(tile, w, h);
        addPrim(ot, tile);

        draw_mode = (DR_TPAGE*)(tile + 1);
        setDrawTPage(draw_mode, 0, 0, BLEND_SUBTRACT_TPAGE);
        addPrim(ot, draw_mode);
        next_prim = draw_mode + 1;
    }
    return next_prim;
}

/**
 * @brief Emit four line packets forming a rectangle outline.
 * @param line First of the four LINE_F2 packets to fill.
 * @param ot Ordering-table entry the lines are linked into.
 * @param x Rectangle left edge.
 * @param y Rectangle top edge.
 * @param w Rectangle width.
 * @param h Rectangle height.
 * @param color Packed 0x00BBGGRR line color.
 * @return Primitive-buffer cursor after the fourth line.
 */
void* cload_emit_rect_outline(LINE_F2* line, u_long* ot, s32 x, s32 y, s32 w, s32 h, s32 color)
{
    SET_BGR0_PACKED(line, color);
    setLineF2(line);
    setXY0(line, x, y);
    line->x1 = x + w;
    line->y1 = y;
    addPrim(ot, line);
    line++;

    SET_BGR0_PACKED(line, color);
    setLineF2(line);
    setXY0(line, x + w, y);
    line->x1 = x + w;
    line->y1 = y + h;
    addPrim(ot, line);
    line++;

    SET_BGR0_PACKED(line, color);
    setLineF2(line);
    setXY0(line, x + w, y + h);
    line->x1 = x;
    line->y1 = y + h;
    addPrim(ot, line);
    line++;

    SET_BGR0_PACKED(line, color);
    setLineF2(line);
    setXY0(line, x, y);
    line->x1 = x;
    line->y1 = y + h;
    addPrim(ot, line);
    return line + 1;
}

/**
 * @brief Emit a scroll-arrow sprite from the third icon-set image and its texture-page packet.
 * @param sprite Sprite packet to fill; the DR_TPAGE follows it.
 * @param ot Ordering-table entry the packets are linked into.
 * @param x Sprite left edge.
 * @param y Sprite top edge.
 * @param upper Nonzero for the upper arrow; the lower arrow is the texture row below it.
 * @return Primitive-buffer cursor after the texture-page packet.
 */
void* cload_emit_scroll_arrow(SPRT* sprite, u_long* ot, s32 x, s32 y, s32 upper)
{
    DR_TPAGE* draw_mode;

    SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
    setSprt(sprite);
    setXY0(sprite, x, y);
    if (upper != 0)
    {
        sprite->v0 = 0;
    }
    else
    {
        sprite->v0 = CLOAD_SCROLL_ARROW_SIZE;
    }
    sprite->u0 = CLOAD_BACKDROP_LAST_SLICE_WIDTH;
    setWH(sprite, CLOAD_SCROLL_ARROW_SIZE, CLOAD_SCROLL_ARROW_SIZE);
    sprite->clut = getClut(0, CLOAD_ICON_SET_CLUT_Y + 2);
    addPrim(ot, sprite);

    sprite++;
    draw_mode = (DR_TPAGE*)sprite;
    setDrawTPage(draw_mode, 0, 0, getTPage(GPU_TEXTURE_8BIT, GPU_BLEND_HALF, CLOAD_ICON_SET_VRAM_X + CLOAD_ICON_SET_VRAM_STEP * 2, 0));
    addPrim(ot, draw_mode);
    return draw_mode + 1;
}

/**
 * @brief Draw the load confirmation prompt and handle its input: a card
 *        change closes the prompt, cancel or "no" returns to the card reset
 *        steps, and "yes" starts loading the selected save.
 * @param ot Ordering-table entry the text is linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 */
void* cload_draw_load_prompt(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2];
    void* result;
    s32 x;
    s32 status;
    CardMenuElement* prompt;

    x = -x_offset + CARD_MENU_MESSAGE_CENTER_X;
    result = cload_draw_choice_prompt(
        field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_card_menu_text_load_prompt, CARD_MENU_TEXT_LOAD_PROMPT), FIELD_TEXT_COLOR_NORMAL, x, -y_offset, FIELD_TEXT_ALIGN_CENTER), ot, x, CARD_MENU_LINE_HEIGHT - y_offset);

    status = poll_and_retry_card_info();
    if (status == CARD_EVENT_ERROR || status == CARD_EVENT_TIMEOUT)
    {
        g_card_menu_element_pool[CARD_MENU_ELEMENT_MODAL].attr.bits.state = CARD_MENU_ELEMENT_FREE;
        field_reset_input_repeat();
        field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
        g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
        card_reset_entry_ranks();
        g_card_step = NULL;
    }
    else
    {
        if (g_pad_input & PAD_BTN_CIRCLE)
        {
            g_card_menu_element_pool[CARD_MENU_ELEMENT_MODAL].attr.bits.state = CARD_MENU_ELEMENT_FREE;
            field_reset_input_repeat();
            field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
            g_card_step = g_card_steps_card_info;
        }
        else if (g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK)
        {
            if (g_card_menu_choice_toggle != CARD_MENU_CHOICE_YES)
            {
                g_card_menu_element_pool[CARD_MENU_ELEMENT_MODAL].attr.bits.state = CARD_MENU_ELEMENT_FREE;
                field_reset_input_repeat();
                field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
                g_card_step = g_card_steps_card_info;
            }
            else
            {
                field_play_sound(FIELD_SOUND_SELECT, AKAO_PAN_CENTER);
                g_card_menu_progress_active = 1;
                g_card_step = g_card_steps_read_save;
                prompt = &g_card_menu_element_pool[CARD_MENU_ELEMENT_MODAL];
                prompt->draw = cload_draw_load_progress;
                prompt->attr.bits.transition_step = 1;
                prompt->attr.bits.state = CARD_MENU_ELEMENT_OPENING;
                prompt->attr.bits.x = CARD_MENU_MESSAGE_X;
                prompt->attr.bits.y = CLOAD_MESSAGE_Y;
                prompt->size.bits.width_high = CARD_MENU_MESSAGE_WIDTH >> 8;
                prompt->size.bits.height = CLOAD_WINDOW_HEIGHT(CARD_MENU_MESSAGE_HEIGHT);
                CARD_MENU_SET_ELEMENT_WIDTH_LOW(prompt, CARD_MENU_MESSAGE_WIDTH);
            }
        }
    }
    return result;
}

/**
 * @brief Draw the "loading" message and progress bar, then commit the save
 *        file to g_saved_game once the read has finished and validates.
 * @param ot Ordering-table entry the text is linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 */
void* cload_draw_load_progress(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2];
    s32 x;
    void* result;
    u16* text_table;

    x = -x_offset + CARD_MENU_MESSAGE_CENTER_X;
    result = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_cload_text_loading, CARD_MENU_TEXT_LOADING), FIELD_TEXT_COLOR_NORMAL, x, -y_offset, FIELD_TEXT_ALIGN_CENTER);
    text_table = CARD_MENU_TEXT_TABLE(g_cload_text_loading, CARD_MENU_TEXT_LOADING);
    result = field_draw_text(result, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_DO_NOT_REMOVE_CARD), FIELD_TEXT_COLOR_NORMAL, x, CARD_MENU_LINE_HEIGHT - y_offset, FIELD_TEXT_ALIGN_CENTER);
    result = field_draw_text(result, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CARD_OR_CONTROLLER), FIELD_TEXT_COLOR_NORMAL, x, CARD_MENU_LINE_HEIGHT * 2 - y_offset, FIELD_TEXT_ALIGN_CENTER);
    result = cload_draw_progress_bar(result, ot);

    if (g_card_menu_progress_active == 0)
    {
        if (validate_save_file(&g_cload_save_file) == 0)
        {
            cload_open_status_dialog(CARD_MENU_DIALOG_INVALID_SAVE);
        }
        else
        {
            field_play_sound(FIELD_SOUND_LOAD_DONE, AKAO_PAN_CENTER);
            g_card_menu_element_pool[CARD_MENU_ELEMENT_MODAL].attr.bits.state = CARD_MENU_ELEMENT_FREE;
            bcopy((u8*)&g_cload_save_file.saved_game, g_saved_game.bytes, SAVED_GAME_DATA_SIZE);
            g_save_compatibility_tag = g_saved_game.layout.compatibility_tag;
            g_playtime_vsync_origin = VSync(-1);
            g_card_menu_exit_requested = 1;
        }
    }
    return result;
}

/**
 * @brief Emit the time-based load progress bar as a gouraud-shaded quad.
 * @param quad Quad packet to fill.
 * @param ot Ordering-table entry the quad is linked into.
 * @return Primitive-buffer cursor after the quad, or @p quad unchanged while
 *         g_card_menu_progress_bar_active is 0.
 * @note The bar is 288 pixels wide after 256 frames since g_card_menu_progress_start_tick.
 */
void* cload_draw_progress_bar(POLY_G4* quad, u_long* ot)
{
    s32 elapsed;
    s32 width;

    if (g_card_menu_progress_bar_active != 0)
    {
        elapsed = VSync(-1) - g_card_menu_progress_start_tick;
        if (elapsed > CARD_MENU_PROGRESS_FULL_TICKS)
        {
            elapsed = CARD_MENU_PROGRESS_FULL_TICKS;
        }
        width = elapsed * CARD_MENU_MESSAGE_WIDTH;
        SET_BGR0_PACKED(quad, CARD_MENU_PROGRESS_TOP_LEFT_COLOR);
        SET_POLY_G4_BGR1_PACKED(quad, CARD_MENU_PROGRESS_TOP_RIGHT_COLOR);
        SET_POLY_G4_BGR3_PACKED(quad, CARD_MENU_PROGRESS_BOTTOM_RIGHT_COLOR);
        SET_POLY_G4_BGR2_PACKED(quad, CARD_MENU_PROGRESS_BOTTOM_LEFT_COLOR);
        setPolyG4(quad);
        quad->x2 = 0;
        quad->x0 = 0;
        quad->x1 = quad->x3 = width / CARD_MENU_PROGRESS_FULL_TICKS;
        quad->y1 = 0;
        quad->y0 = 0;
        quad->y3 = CLOAD_PROGRESS_BAR_HEIGHT;
        quad->y2 = CLOAD_PROGRESS_BAR_HEIGHT;
        addPrim(ot, quad);
        quad++;
    }
    return quad;
}

/**
 * @brief Open the status dialog in the first element slot and abandon any
 *        load in progress.
 * @param dialog_state CARD_MENU_DIALOG_* message to show.
 */
void cload_open_status_dialog(s32 dialog_state)
{
    CardMenuElement* dialog;
    s32 y;

    field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
    dialog = &g_card_menu_element_pool[CARD_MENU_ELEMENT_MODAL];
    dialog->attr.bits.transition_step = 1;
    dialog->attr.bits.state = CARD_MENU_ELEMENT_OPENING;
    dialog->attr.bits.x = CARD_MENU_DIALOG_X;
    CARD_MENU_SET_ELEMENT_WIDTH_LOW(dialog, CARD_MENU_DIALOG_WIDTH);
    dialog->size.bits.width_high = CARD_MENU_DIALOG_WIDTH >> 8;
    if (dialog_state <= CARD_MENU_DIALOG_LOAD_FAILED || dialog_state == CARD_MENU_DIALOG_INVALID_SAVE)
    {
        dialog->size.bits.height = CLOAD_DIALOG_HEIGHT(2);
        y = CARD_MENU_TWO_LINE_DIALOG_Y;
    }
    else
    {
        dialog->size.bits.height = CLOAD_DIALOG_HEIGHT(1);
        y = CARD_MENU_EXCHANGE_DIALOG_Y;
    }
    dialog->attr.bits.y = y;
    dialog->draw = cload_draw_status_dialog;
    field_reset_input_repeat();
    g_card_menu_write_in_progress = 0;
    g_card_menu_progress_active = 0;
    g_card_menu_selection_status = CARD_MENU_SELECTION_NONE;
    g_card_menu_io_busy = 0;
    g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
    card_reset_entry_ranks();
    g_card_step = 0;
    g_card_menu_dialog_state = dialog_state;
}

/**
 * @brief Draw the message of the open status dialog and close it on confirm.
 * @param ot Ordering-table entry the text is linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 */
void* cload_draw_status_dialog(u_long* ot, void* prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2];
    u16* text_table;

    switch (g_card_menu_dialog_state)
    {
    case CARD_MENU_DIALOG_SAVE_FAILED:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_card_menu_text_save_failed, CARD_MENU_TEXT_SAVE_FAILED), FIELD_TEXT_COLOR_NORMAL, -x_offset + CARD_MENU_DIALOG_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        text_table = CARD_MENU_TEXT_TABLE(g_card_menu_text_save_failed, CARD_MENU_TEXT_SAVE_FAILED);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CHECK_CARD_INSERTED), FIELD_TEXT_COLOR_NORMAL, -x_offset + CARD_MENU_DIALOG_WIDTH / 2, -y_offset + FIELD_TEXT_LINE_SPACING, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_DIALOG_LOAD_FAILED:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_card_menu_text_load_failed, CARD_MENU_TEXT_LOAD_FAILED), FIELD_TEXT_COLOR_NORMAL, -x_offset + CARD_MENU_DIALOG_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        text_table = CARD_MENU_TEXT_TABLE(g_card_menu_text_load_failed, CARD_MENU_TEXT_LOAD_FAILED);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CHECK_CARD_INSERTED), FIELD_TEXT_COLOR_NORMAL, -x_offset + CARD_MENU_DIALOG_WIDTH / 2, -y_offset + FIELD_TEXT_LINE_SPACING, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_DIALOG_CARD_NOT_INSERTED:
        prim =
            field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_card_menu_text_card_not_inserted, CARD_MENU_TEXT_CARD_NOT_INSERTED), FIELD_TEXT_COLOR_NORMAL, -x_offset + CARD_MENU_DIALOG_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_DIALOG_NOT_POCKETSTATION:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_card_menu_text_not_pocketstation, CARD_MENU_TEXT_NOT_POCKETSTATION), FIELD_TEXT_COLOR_NORMAL, -x_offset + CARD_MENU_DIALOG_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        break;
    case CARD_MENU_DIALOG_INVALID_SAVE:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_card_menu_text_load_failed, CARD_MENU_TEXT_LOAD_FAILED), FIELD_TEXT_COLOR_NORMAL, -x_offset + CARD_MENU_DIALOG_WIDTH / 2, -y_offset, FIELD_TEXT_ALIGN_CENTER);
        text_table = CARD_MENU_TEXT_TABLE(g_card_menu_text_load_failed, CARD_MENU_TEXT_LOAD_FAILED);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_SAVE_CORRUPT), FIELD_TEXT_COLOR_NORMAL, -x_offset + CARD_MENU_DIALOG_WIDTH / 2, -y_offset + FIELD_TEXT_LINE_SPACING, FIELD_TEXT_ALIGN_CENTER);
        break;
    }

    if (g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK)
    {
        g_card_menu_element_pool[CARD_MENU_ELEMENT_MODAL].attr.bits.state = CARD_MENU_ELEMENT_FREE;
        field_reset_input_repeat();
    }

    return prim;
}

/**
 * @brief Upload one save icon's CLUT and pixels to VRAM slot @p index and draw
 *        it as a 48x48 textured quad.
 * @param quad Quad packet to fill.
 * @param ot Ordering-table entry the quad is linked into.
 * @param x Quad left edge.
 * @param y Quad top edge.
 * @param width Quad width.
 * @param icon Save-browser party icon; SAVE_NO_ICON draws nothing. The guest's hero
 *             portraits (below SAVE_ICON_HERO_COUNT) and golems (from SAVE_ICON_GOLEM_BASE)
 *             get a generated CLUT (field_copy_portrait_palette / field_copy_golem_portrait_palette).
 * @param index VRAM icon slot; selects the CLUT row entry and the texture column.
 * @param row Party slot the icon belongs to (FIELD_PARTY_GUEST uses the hero portrait CLUTs).
 * @return Primitive-buffer cursor after the quad, or @p quad unchanged for SAVE_NO_ICON.
 */
void* cload_draw_icon_highlight(POLY_FT4* quad, u_long* ot, s32 x, s32 y, s32 width, s32 icon, s32 index, s32 row)
{
    RECT rect;
    u8 u;

    if (icon == SAVE_NO_ICON)
    {
        return quad;
    }

    setRECT(&rect, index * GPU_CLUT_4BIT_COLORS, VRAM_CLUT_Y, GPU_CLUT_4BIT_COLORS, 1);
    if ((row == FIELD_PARTY_GUEST) && (icon < SAVE_ICON_HERO_COUNT))
    {
        field_copy_portrait_palette(g_card_menu_icon_context, icon);
        LoadImage(&rect, (u_long*)g_card_menu_icon_context);
        DrawSync(0);
    }
    else if (icon >= SAVE_ICON_GOLEM_BASE)
    {
        field_copy_golem_portrait_palette(g_card_menu_icon_context, g_card_menu_icon_palette);
        LoadImage(&rect, (u_long*)g_card_menu_icon_context);
        DrawSync(0);
    }
    else
    {
        LoadImage(&rect, (u_long*)CLOAD_ICON_IMAGE(icon)->clut);
    }

    setRECT(&rect, index * (FIELD_PORTRAIT_SIZE / 4) + CARD_MENU_ICON_VRAM_X, CARD_MENU_ICON_VRAM_Y, FIELD_PORTRAIT_SIZE / 4, FIELD_PORTRAIT_SIZE);
    LoadImage(&rect, (u_long*)CLOAD_ICON_IMAGE(icon)->pixels);

    SET_BGR0_PACKED(quad, GPU_TINT_NEUTRAL);
    setPolyFT4(quad);
    quad->x2 = x;
    quad->x0 = x;
    quad->y1 = y;
    quad->y0 = y;
    quad->x3 = x + width;
    u = index * FIELD_PORTRAIT_SIZE;
    quad->u2 = u;
    quad->u0 = u;
    u += FIELD_PORTRAIT_SIZE - 1;
    quad->u3 = u;
    quad->u1 = u;
    quad->v1 = CARD_MENU_ICON_VRAM_Y;
    quad->v0 = CARD_MENU_ICON_VRAM_Y;
    quad->x1 = x + width;
    quad->y3 = y + FIELD_PORTRAIT_SIZE - 1;
    quad->y2 = y + FIELD_PORTRAIT_SIZE - 1;
    quad->v3 = CARD_MENU_ICON_VRAM_Y + FIELD_PORTRAIT_SIZE - 1;
    quad->v2 = CARD_MENU_ICON_VRAM_Y + FIELD_PORTRAIT_SIZE - 1;
    quad->clut = getClut(index * GPU_CLUT_4BIT_COLORS, VRAM_CLUT_Y);
    quad->tpage = getTPage(GPU_TEXTURE_4BIT, GPU_BLEND_HALF, CARD_MENU_ICON_VRAM_X, 0);
    addPrim(ot, quad);

    return quad + 1;
}

#include "../../common/card_menu/card_menu_deactivate_primary_element.inc.c"

/**
 * @brief Read the save-icon set into its fixed scratch buffer, upload its three
 *        images to VRAM from CLOAD_ICON_SET_VRAM_X (CLUT rows from
 *        CLOAD_ICON_SET_CLUT_Y) and point g_cload_icon_resource at its icon table.
 * @note D_80180004/8/C/10 are marked `ignore:true` in cload_symbol_addrs.txt;
 *       the header fields are only reached through CLOAD_ICON_SET_BUFFER.
 */
void cload_load_icon_resources(void)
{
    CloadIconSetHeader* icon_set;
    RECT rect;

    cdrom_queue_read(CD_RES_CLOAD_ICON_SET, CLOAD_ICON_SET_BUFFER);
    cdrom_wait_queue_empty();

    icon_set = CLOAD_ICON_SET_BUFFER;

    g_cload_icon_resource = (u8*)icon_set + icon_set->icon_table_offset;

    setRECT(&rect, CLOAD_ICON_SET_VRAM_X, 0, 0, CLOAD_ICON_SET_CLUT_Y);
    field_upload_image_resource(&rect, (u8*)icon_set + icon_set->image_offsets[0], 1);

    setRECT(&rect, CLOAD_ICON_SET_VRAM_X + CLOAD_ICON_SET_VRAM_STEP, 0, 0, CLOAD_ICON_SET_CLUT_Y + 1);
    field_upload_image_resource(&rect, (u8*)icon_set + icon_set->image_offsets[1], 1);

    setRECT(&rect, CLOAD_ICON_SET_VRAM_X + CLOAD_ICON_SET_VRAM_STEP * 2, 0, 0, CLOAD_ICON_SET_CLUT_Y + 2);
    field_upload_image_resource(&rect, (u8*)icon_set + icon_set->image_offsets[2], 1);
}

/**
 * @brief Draw CLOAD's backdrop picture: the three icon-set images side by side,
 *        CLOAD_BACKDROP_SLICE_WIDTH + CLOAD_BACKDROP_SLICE_WIDTH + CLOAD_BACKDROP_LAST_SLICE_WIDTH
 *        pixels wide and VRAM_DRAW_HEIGHT tall, each with its own texture-page packet.
 * @param sprite First sprite packet to fill.
 * @param ot Ordering-table entry the packets are linked into.
 * @return Primitive-buffer cursor after the last texture-page packet.
 */
void* cload_draw_backdrop(SPRT* sprite, u_long* ot)
{
    s32 i;
    DR_TPAGE* draw_mode;

    for (i = 0; i < CLOAD_ICON_SET_IMAGE_COUNT; i++)
    {
        SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
        setSprt(sprite);
        setXY0(sprite, CLOAD_BACKDROP_X + i * CLOAD_BACKDROP_SLICE_WIDTH, 0);
        setUV0(sprite, 0, 0);
        if (i == CLOAD_ICON_SET_IMAGE_COUNT - 1)
        {
            sprite->w = CLOAD_BACKDROP_LAST_SLICE_WIDTH;
        }
        else
        {
            sprite->w = CLOAD_BACKDROP_SLICE_WIDTH;
        }
        sprite->h = VRAM_DRAW_HEIGHT;
        sprite->clut = getClut(0, CLOAD_ICON_SET_CLUT_Y + i);
        addPrim(ot, sprite);
        sprite++;

        draw_mode = (DR_TPAGE*)sprite;
        setDrawTPage(draw_mode, 0, 0, getTPage(GPU_TEXTURE_8BIT, GPU_BLEND_HALF, CLOAD_ICON_SET_VRAM_X + i * CLOAD_ICON_SET_VRAM_STEP, 0));
        addPrim(ot, draw_mode);
        sprite = (SPRT*)(draw_mode + 1);
    }
    return sprite;
}

/**
 * @brief Initialize the two-choice prompt selection state.
 * @return Always 1 in US; JP returns nothing (no caller reads it).
 * @note JP selects the first choice instead of the second.
 */
s32 cload_enable_choice_toggle(void)
{
    g_card_menu_choice_toggle = CARD_MENU_CHOICE_DEFAULT;
#if !defined(VERSION_JP)
    return 1;
#endif
}

/**
 * @brief Draw the two choices of a yes/no prompt from the FIELD UI string table,
 *        highlighting the selected one, and toggle the selection on left/right.
 * @param prim Primitive-buffer cursor.
 * @param ot Ordering-table entry the text is linked into.
 * @param x Prompt center; the choices are drawn CARD_MENU_CHOICE_YES_GAP left and CARD_MENU_CHOICE_NO_GAP right of it.
 * @param y Prompt baseline.
 * @return Advanced primitive-buffer cursor.
 */
void* cload_draw_choice_prompt(void* prim, u_long* ot, s32 x, s32 y)
{
    u8* text_table;
    u8* text;
    s32 color;

    color = FIELD_TEXT_COLOR_NORMAL;
    text = FIELD_UI_TEXT_AT(g_text_choice_glyph_offsets, FIELD_UI_TEXT_YES);
    text_table = FIELD_UI_TEXT_TABLE(g_text_choice_glyph_offsets, FIELD_UI_TEXT_YES);
    if (g_card_menu_choice_toggle != CARD_MENU_CHOICE_YES)
    {
        color = FIELD_TEXT_COLOR_DIM;
    }
    prim = field_draw_text(prim, ot, text, color, x - CARD_MENU_CHOICE_YES_GAP, y, FIELD_TEXT_ALIGN_RIGHT);

    color = FIELD_TEXT_COLOR_NORMAL;
    text = FIELD_UI_TEXT(text_table, FIELD_UI_TEXT_NO);
    if (g_card_menu_choice_toggle == CARD_MENU_CHOICE_YES)
    {
        color = FIELD_TEXT_COLOR_DIM;
    }
    prim = field_draw_text(prim, ot, text, color, x + CARD_MENU_CHOICE_NO_GAP, y, FIELD_TEXT_ALIGN_LEFT);

    if (g_pad_input & CARD_MENU_CHOICE_BUTTON_MASK)
    {
        g_card_menu_choice_toggle ^= 1;
        field_play_sound(FIELD_SOUND_CURSOR, AKAO_PAN_CENTER);
        g_pad_input = 0;
    }
    return prim;
}
