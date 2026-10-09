#include "common/card_menu.h"

/**
 * @brief Upload one party icon to VRAM and draw it in the details window.
 * @param quad Primitive-buffer cursor the textured quad is written to.
 * @param ot Ordering-table entry the quad is linked into.
 * @param x Left edge of the icon.
 * @param y Top edge of the icon.
 * @param width Drawn width, which animates for the highlighted icon.
 * @param icon SAVE_ICON_* id of the party member, or SAVE_NO_ICON for an empty slot.
 * @param index VRAM slot: the position among the icons drawn so far.
 * @param row Party slot; the guest's hero icons take their palette from FIELD's portraits.
 * @return Primitive-buffer cursor after the quad, or @p quad for an empty slot.
 */
void* card_menu_draw_icon_highlight(POLY_FT4* quad, u_long* ot, s32 x, s32 y, s32 width, s32 icon, s32 index, s32 row)
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
        LoadImage(&rect, (u_long*)CARD_MENU_ICON_IMAGE(g_card_menu_icon_offsets, icon)->clut);
    }

    setRECT(&rect, index * (FIELD_PORTRAIT_SIZE / 4) + CARD_MENU_ICON_VRAM_X, CARD_MENU_ICON_VRAM_Y, FIELD_PORTRAIT_SIZE / 4, FIELD_PORTRAIT_SIZE);
    LoadImage(&rect, (u_long*)CARD_MENU_ICON_IMAGE(g_card_menu_icon_offsets, icon)->pixels);

    SET_BGR0_PACKED(quad, GPU_TINT_NEUTRAL);
    setPolyFT4(quad);
    quad->x0 = quad->x2 = x;
    quad->y0 = quad->y1 = y;
    quad->x1 = quad->x3 = x + width;
    quad->y2 = quad->y3 = y + FIELD_PORTRAIT_SIZE - 1;
    u = index * FIELD_PORTRAIT_SIZE;
    quad->u0 = quad->u2 = u;
    quad->u1 = quad->u3 = u + FIELD_PORTRAIT_SIZE - 1;
    quad->v0 = quad->v1 = CARD_MENU_ICON_VRAM_Y;
    quad->v2 = quad->v3 = CARD_MENU_ICON_VRAM_Y + FIELD_PORTRAIT_SIZE - 1;
    quad->clut = getClut(index * GPU_CLUT_4BIT_COLORS, VRAM_CLUT_Y);
    quad->tpage = getTPage(GPU_TEXTURE_4BIT, GPU_BLEND_HALF, CARD_MENU_ICON_VRAM_X, 0);
    addPrim(ot, quad);

    return quad + 1;
}
