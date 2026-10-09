#include "internal/cload_internal.h"
#include "overlays/field/field_text.h"
#include "main/display.h"
#include <memory.h>
#include <libetc.h>
#include "common/gpu_packet.h"
#include "overlays/field/field_input.h"


/** @brief CD resource index of the save-icon set. */
#define CLOAD_ICON_SET_RESOURCE 0x5E4

/** @brief Fixed scratch buffer the icon set is read into. */
#define CLOAD_ICON_SET_BUFFER ((CloadIconSetHeader *)LOAD_BUFFER_ADDRESS)

/** @brief Header of the save-icon set resource; all offsets are bytes from its start. */
typedef struct
{
    s32 unk0;
    s32 icon_table_offset;
    s32 image_offsets[3];
} CloadIconSetHeader;

/** @brief Icon @p icon in the loaded icon table, which holds per-icon byte offsets after a leading word. */
#define CLOAD_ICON_IMAGE(icon) ((FieldPortrait *)(g_cload_icon_resource + ((s32 *)g_cload_icon_resource)[(icon) + 1]))

/**
 * @brief Emit the GPU packets for a CLOAD window frame: a clipped draw
 *        environment and, optionally, three outlines plus a translucent fill.
 * @param prim Primitive-buffer cursor.
 * @param ot Ordering-table entry the packets are linked into.
 * @param x Window left edge.
 * @param y Window top edge.
 * @param w Window width.
 * @param h Window height.
 * @param flag Nonzero when drawing into the front buffer (draw area at y = SCREEN_HEIGHT).
 * @param draw_fill Nonzero to emit the outline and fill packets.
 * @return Advanced primitive-buffer cursor.
 */
CloadGpuPacket *cload_emit_window_frame(CloadGpuPacket *prim, u_long *ot, s32 x, s32 y, s32 w, s32 h, s32 flag, s32 draw_fill)
{
    DRAWENV draw_env;
    DR_ENV *env_packet;
    TILE *tile;
    DR_TPAGE *draw_mode;
    void *next_prim;

    env_packet = (DR_ENV *)prim;
    if (flag != 0)
    {
        SetDefDrawEnv(&draw_env, x + 2, y + SCREEN_HEIGHT + 2, w - 4, h - 3);
    }
    else
    {
        SetDefDrawEnv(&draw_env, x + 2, y + VRAM_BACK_DRAW_Y + 2, w - 4, h - 3);
    }
    SetDrawEnv(env_packet, &draw_env);
    addPrim(ot, env_packet);

    next_prim = env_packet + 1;
    if (draw_fill != 0)
    {
        next_prim = cload_emit_rect_outline(next_prim, ot, x, y, w, h, CLOAD_COLOR_WHITE);
        next_prim = cload_emit_rect_outline(next_prim, ot, x + 1, y + 1, w - 2, h - 2, 0);
        next_prim = cload_emit_rect_outline(next_prim, ot, x - 1, y - 1, w + 2, h + 2, 0);

        tile = next_prim;
        SET_BGR0_PACKED(tile, GPU_TINT_NEUTRAL);
        setTile(tile);
        setSemiTrans(tile, 1);
        setXY0(tile, x, y);
        setWH(tile, w, h);
        addPrim(ot, tile);

        draw_mode = (DR_TPAGE *)(tile + 1);
        setDrawTPage(draw_mode, 0, 0, getTPage(GPU_TEXTURE_4BIT, GPU_BLEND_SUBTRACT, 320, 0));
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
CloadGpuPacket *cload_emit_rect_outline(LINE_F2 *line, u_long *ot, s32 x, s32 y, s32 w, s32 h, s32 color)
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
    return (CloadGpuPacket *)(line + 1);
}

/**
 * @brief Emit a 16x16 scroll-arrow sprite from the third icon-set image and its texture-page packet.
 * @param sprite Sprite packet to fill; the DR_TPAGE follows it.
 * @param ot Ordering-table entry the packets are linked into.
 * @param x Sprite left edge.
 * @param y Sprite top edge.
 * @param flag Nonzero for the upper arrow (texture row 0), zero for the lower arrow (row 16).
 * @return Primitive-buffer cursor after the texture-page packet.
 */
CloadGpuPacket *cload_emit_scroll_arrow(SPRT *sprite, u_long *ot, s32 x, s32 y, s32 flag)
{
    DR_TPAGE *draw_mode;

    SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
    setSprt(sprite);
    setXY0(sprite, x, y);
    if (flag != 0)
    {
        sprite->v0 = 0;
    }
    else
    {
        sprite->v0 = 16;
    }
    sprite->u0 = 48;
    setWH(sprite, 16, 16);
    sprite->clut = getClut(0, 502);
    addPrim(ot, sprite);

    sprite++;
    draw_mode = (DR_TPAGE *)sprite;
    setDrawTPage(draw_mode, 0, 0, getTPage(GPU_TEXTURE_8BIT, GPU_BLEND_HALF, 640, 0));
    addPrim(ot, draw_mode);
    return (CloadGpuPacket *)(draw_mode + 1);
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
void *cload_draw_load_prompt(u_long *ot, void *prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2];
    void *result;
    s32 x;
    s32 status;
    CardMenuElement *prompt;

    x = -x_offset + 0x90;
    result = cload_draw_choice_prompt(field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_cload_text_load_prompt, CARD_MENU_TEXT_LOAD_PROMPT), 4, x, -y_offset, 2), ot, x, 0xE - y_offset);

    status = poll_and_retry_card_info();
    if (status == 1 || status == 2)
    {
        g_card_menu_element_pool[0].attr.bits.state = CARD_MENU_ELEMENT_FREE;
        field_reset_input_repeat();
        field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
        g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
        card_reset_entry_ranks();
        g_cload_load_step = 0;
    }
    else
    {
        if (g_pad_input & 0x40)
        {
            g_card_menu_element_pool[0].attr.bits.state = CARD_MENU_ELEMENT_FREE;
            field_reset_input_repeat();
            field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
            g_cload_load_step = g_cload_steps_card_reset;
        }
        else if (g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK)
        {
            if (g_card_menu_choice_toggle != 0)
            {
                g_card_menu_element_pool[0].attr.bits.state = CARD_MENU_ELEMENT_FREE;
                field_reset_input_repeat();
                field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
                g_cload_load_step = g_cload_steps_card_reset;
            }
            else
            {
                field_play_sound(FIELD_SOUND_SELECT, AKAO_PAN_CENTER);
                g_cload_progress_active = 1;
                g_cload_load_step = g_cload_steps_load_selected_save;
                prompt = &g_card_menu_element_pool[CARD_MENU_ELEMENT_MODAL];
                prompt->draw = cload_draw_load_progress;
                prompt->attr.bits.transition_step = 1;
                prompt->attr.bits.state = 1;
                prompt->attr.bits.x = 0x10;
                prompt->attr.bits.y = 0x5B;
                prompt->size.bits.width_high = 1;
                prompt->size.bits.height = 0x2B;
                prompt->attr.word = (prompt->attr.word & 0x00FFFFFF) | ((u32)0x20 << 24);
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
void *cload_draw_load_progress(u_long *ot, void *prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2];
    s32 x;
    void *result;
    u16 *text_table;

    x = -x_offset + 0x90;
    result = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_cload_text_loading, CARD_MENU_TEXT_LOADING), 4, x, -y_offset, 2);
    text_table = CARD_MENU_TEXT_TABLE(g_cload_text_loading, 25);
    result = field_draw_text(result, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_DO_NOT_REMOVE_CARD), 4, x, 0xE - y_offset, 2);
    result = field_draw_text(result, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CARD_OR_CONTROLLER), 4, x, 0x1C - y_offset, 2);
    result = cload_draw_progress_bar(result, ot);

    if (g_cload_progress_active == 0)
    {
        if (validate_save_file(&g_cload_save_file) == 0)
        {
            cload_open_status_dialog(4);
        }
        else
        {
            field_play_sound(FIELD_SOUND_LOAD_DONE, AKAO_PAN_CENTER);
            g_card_menu_element_pool[0].attr.bits.state = CARD_MENU_ELEMENT_FREE;
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
 *         g_cload_progress_bar_active is 0.
 * @note The bar is 288 pixels wide after 256 frames since g_cload_progress_start_tick.
 */
CloadGpuPacket *cload_draw_progress_bar(POLY_G4 *quad, u_long *ot)
{
    s32 elapsed;
    s32 width;

    if (g_cload_progress_bar_active != 0)
    {
        elapsed = VSync(-1) - g_cload_progress_start_tick;
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
        quad->y3 = 53;
        quad->y2 = 53;
        addPrim(ot, quad);
        quad++;
    }
    return (CloadGpuPacket *)quad;
}

/**
 * @brief Open the status dialog in the first element slot and abandon any
 *        load in progress.
 * @param dialog_state Message to show (0 save failed, 1 and 4 load failed, 2 insert card, 3 TODO: unknown).
 */
void cload_open_status_dialog(s32 dialog_state)
{
    CardMenuElement *dialog;
    s32 code;

    field_play_sound(FIELD_SOUND_ACTION_REFUSED, AKAO_PAN_CENTER);
    dialog = &g_card_menu_element_pool[CARD_MENU_ELEMENT_MODAL];
    dialog->attr.bits.transition_step = 1;
    dialog->attr.bits.state = 1;
    dialog->attr.bits.x = CARD_MENU_DIALOG_X;
    CARD_MENU_SET_ELEMENT_WIDTH_LOW(dialog, CARD_MENU_DIALOG_WIDTH);
    dialog->size.bits.width_high = CARD_MENU_DIALOG_WIDTH >> 8;
    if (dialog_state < 2 || dialog_state == 4)
    {
        dialog->size.bits.height = 0x1F;
        code = 0x60;
    }
    else
    {
        dialog->size.bits.height = 0x0F;
        code = 0x70;
    }
    dialog->attr.bits.y = code;
    dialog->draw = cload_draw_status_dialog;
    field_reset_input_repeat();
    D_80162370 = 0;
    g_cload_progress_active = 0;
    g_cload_selection_status = 0;
    g_cload_io_busy = 0;
    g_card_entry_state = CARD_MENU_ENTRY_STATE_CHECKING_CARD;
    card_reset_entry_ranks();
    g_cload_load_step = 0;
    g_cload_dialog_state = dialog_state;
}

/**
 * @brief Draw the message of the open status dialog and close it on confirm.
 * @param ot Ordering-table entry the text is linked into.
 * @param prim Primitive-buffer cursor.
 * @param x_offset Horizontal transition offset.
 * @param y_offset Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 */
void *cload_draw_status_dialog(u_long *ot, void *prim, s32 x_offset, s32 y_offset)
{
    s32 unused[2];
    u16 *text_table;

    switch (g_cload_dialog_state)
    {
    case 0:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_cload_text_save_failed, CARD_MENU_TEXT_SAVE_FAILED), 4, -x_offset + 0x80, -y_offset, 2);
        text_table = CARD_MENU_TEXT_TABLE(g_cload_text_save_failed, 30);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CHECK_CARD_INSERTED), 4, -x_offset + 0x80, -y_offset + 0x10, 2);
        break;
    case 1:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_cload_text_load_failed, CARD_MENU_TEXT_LOAD_FAILED), 4, -x_offset + 0x80, -y_offset, 2);
        text_table = CARD_MENU_TEXT_TABLE(g_cload_text_load_failed, 31);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_CHECK_CARD_INSERTED), 4, -x_offset + 0x80, -y_offset + 0x10, 2);
        break;
    case 2:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_cload_text_card_insert_error, CARD_MENU_TEXT_CARD_NOT_INSERTED), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 3:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(D_80145EDE, CARD_MENU_TEXT_NOT_POCKETSTATION), 4, -x_offset + 0x80, -y_offset, 2);
        break;
    case 4:
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT_AT(g_cload_text_load_failed, CARD_MENU_TEXT_LOAD_FAILED), 4, -x_offset + 0x80, -y_offset, 2);
        text_table = CARD_MENU_TEXT_TABLE(g_cload_text_load_failed, 31);
        prim = field_draw_text(prim, ot, CARD_MENU_TEXT(text_table, CARD_MENU_TEXT_SAVE_CORRUPT), 4, -x_offset + 0x80, -y_offset + 0x10, 2);
        break;
    }

    if (g_pad_input & CARD_MENU_CONFIRM_BUTTON_MASK)
    {
        g_card_menu_element_pool[0].attr.bits.state = CARD_MENU_ELEMENT_FREE;
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
 * @param icon Icon id; CLOAD_NO_ICON draws nothing, ids below 2 in row 1 and ids from
 *             0x4F up get a generated CLUT (field_copy_portrait_palette / field_copy_golem_portrait_palette).
 * @param index VRAM icon slot; selects the CLUT row entry and the texture column.
 * @param row Entry row; row 1 uses the generated CLUT for icons 0 and 1.
 * @return Primitive-buffer cursor after the quad, or @p quad unchanged for CLOAD_NO_ICON.
 */
void *cload_draw_icon_highlight(POLY_FT4 *quad, u_long *ot, s32 x, s32 y, s32 width, s32 icon, s32 index, s32 row)
{
    RECT rect;
    s32 column;
    u8 u;

    if (icon == SAVE_NO_ICON)
    {
        return quad;
    }

    setRECT(&rect, index * GPU_CLUT_4BIT_COLORS, VRAM_CLUT_Y, GPU_CLUT_4BIT_COLORS, 1);
    if ((row == 1) && (icon < 2))
    {
        field_copy_portrait_palette(g_card_menu_icon_context, icon);
        LoadImage(&rect, (u_long *)g_card_menu_icon_context);
        DrawSync(0);
    }
    else if (icon >= SAVE_ICON_GOLEM_BASE)
    {
        field_copy_golem_portrait_palette(g_card_menu_icon_context, g_card_menu_icon_palette);
        LoadImage(&rect, (u_long *)g_card_menu_icon_context);
        DrawSync(0);
    }
    else
    {
        LoadImage(&rect, (u_long *)CLOAD_ICON_IMAGE(icon)->clut);
    }

    column = index * 3;
    setRECT(&rect, column * 4 + CARD_MENU_ICON_VRAM_X, CARD_MENU_ICON_VRAM_Y, FIELD_PORTRAIT_SIZE / 4, FIELD_PORTRAIT_SIZE);
    LoadImage(&rect, (u_long *)CLOAD_ICON_IMAGE(icon)->pixels);

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
 *        images to VRAM x 0x200/0x240/0x280 (CLUT rows 0x1F4..0x1F6) and point
 *        g_cload_icon_resource at its icon table.
 * @note D_80180004/8/C/10 are marked `ignore:true` in cload_symbol_addrs.txt;
 *       the header fields are only reached through CLOAD_ICON_SET_BUFFER.
 */
void cload_load_icon_resources(void)
{
    CloadIconSetHeader *icon_set;
    RECT rect;

    cdrom_queue_read(CLOAD_ICON_SET_RESOURCE, CLOAD_ICON_SET_BUFFER);
    cdrom_wait_queue_empty();

    icon_set = CLOAD_ICON_SET_BUFFER;

    g_cload_icon_resource = (u8 *)icon_set + icon_set->icon_table_offset;

    setRECT(&rect, 0x200, 0, 0, 0x1F4);
    field_upload_image_resource(&rect, (u8 *)icon_set + icon_set->image_offsets[0], 1);

    setRECT(&rect, 0x240, 0, 0, 0x1F5);
    field_upload_image_resource(&rect, (u8 *)icon_set + icon_set->image_offsets[1], 1);

    setRECT(&rect, 0x280, 0, 0, 0x1F6);
    field_upload_image_resource(&rect, (u8 *)icon_set + icon_set->image_offsets[2], 1);
}

/**
 * @brief Draw the three icon-set images side by side (128 + 128 + 48 pixels
 *        wide), each as a sprite with its own texture-page packet.
 * @param sprite First sprite packet to fill.
 * @param ot Ordering-table entry the packets are linked into.
 * @return Primitive-buffer cursor after the last texture-page packet.
 */
CloadGpuPacket *cload_emit_icon_highlight_strip(SPRT *sprite, u_long *ot)
{
    s32 i;
    DR_TPAGE *draw_mode;

    for (i = 0; i < 3; i++)
    {
        SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
        setSprt(sprite);
        setXY0(sprite, i * 128 + 8, 0);
        setUV0(sprite, 0, 0);
        if (i == 2)
        {
            sprite->w = 48;
        }
        else
        {
            sprite->w = 128;
        }
        sprite->h = 224;
        sprite->clut = getClut(0, 500 + i);
        addPrim(ot, sprite);
        sprite++;

        draw_mode = (DR_TPAGE *)sprite;
        setDrawTPage(draw_mode, 0, 0, getTPage(GPU_TEXTURE_8BIT, GPU_BLEND_HALF, 512 + i * 64, 0));
        addPrim(ot, draw_mode);
        sprite = (SPRT *)(draw_mode + 1);
    }
    return (CloadGpuPacket *)sprite;
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
void *cload_draw_choice_prompt(void *prim, u_long *ot, s32 x, s32 y)
{
    u8 *text_table;
    u8 *text;
    s32 color;

    color = FIELD_TEXT_COLOR_NORMAL;
    text = FIELD_UI_TEXT_AT(g_text_choice_glyph_offsets, FIELD_UI_TEXT_YES);
    text_table = FIELD_UI_TEXT_TABLE(g_text_choice_glyph_offsets, FIELD_UI_TEXT_YES);
    if (g_card_menu_choice_toggle != 0)
    {
        color = FIELD_TEXT_COLOR_DIM;
    }
    prim = field_draw_text(prim, ot, text, color, x - CARD_MENU_CHOICE_YES_GAP, y, FIELD_TEXT_ALIGN_RIGHT);

    color = FIELD_TEXT_COLOR_NORMAL;
    text = FIELD_UI_TEXT(text_table, FIELD_UI_TEXT_NO);
    if (g_card_menu_choice_toggle == 0)
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
