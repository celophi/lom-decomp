#include "menu.h"

/** @brief Screen-space point used for the list viewport anchor. */
typedef struct
{
    s16 x; /* 0x00 */
    s16 y; /* 0x02 */
} Vec2s;

/**
 * @brief Scroll-list widget state.
 * @note Only the fields this file touches are named; see menu.c for the full block.
 */
typedef struct
{
    u8 unk0;        /* 0x00 - set to 3 to request a state change */
    u8 pad01[3];
    u16 unk4;       /* 0x04 - selected row (in units of 16 y-pixels) */
    u8 pad06[8];
    s16 viewport_h; /* 0x0E - visible list height */
    u16 scroll_x;   /* 0x10 */
    u16 scroll_y;   /* 0x12 - current applied y scroll offset */
} ScrollListState;

s32 scroll_list_draw(s32 prim_buf, s32* ot, ScrollListState* state, u32* entries, Vec2s* view_origin, int active);
s32 func_800A88A0(s32 prim, s32* ot, void* glyph, s32 a3, s32 x, s32 y, s32 mode);
void func_8014F210(s32 sound_id, s32 volume);

extern u32 D_80168C70;
extern s32 g_menu_active_subtype;
extern s32 g_menu_char_slot;
extern s32 g_menu_pending_overlay;

/**
 * @brief Draw the character's spell/ability grid and handle its selection input.
 *
 * Draws the scroll-list chrome, then walks the 12x8 grid of grid cells whose
 * presence bits live in the byte array at g_pad_ctx + 0x60 (one byte per row,
 * one bit per column). Each present cell advances the running y position by 16
 * and, when it falls inside the viewport, is drawn as a glyph via func_800A88A0.
 * The cell whose row matches the list's selection is remembered in @c sel.
 *
 * On confirm (pad bits 0x220) the selected cell index is written to the active
 * character's slot byte at g_pad_ctx + slot * 0x250 + g_menu_active_subtype +
 * 0x609. If a cell is selected, g_menu_pending_overlay is pointed at its
 * description entry.
 *
 * @param ot          Ordering-table pointer, forwarded to the glyph renderer.
 * @param state       Scroll-list state for this grid.
 * @param prim_buf    Primitive buffer write cursor.
 * @param view_origin Viewport anchor; the glyph origin is (0x10 - x, rel_y - y).
 * @param active      Non-zero to process input this frame; zero draws only.
 * @return Updated primitive buffer write cursor.
 * @note @c list aliases @c state, and @c row is reused to hold the 0x609 slot
 *       offset inside the store expression; both are required to match. The
 *       alias raises the parameter's allocation priority so it lands in s5, and
 *       the in-expression assignment fixes the operand order of the address add.
 * @see decomp.me (100%)
 */
s32 func_8014B7DC(s32* ot, ScrollListState* state, s32 prim_buf, Vec2s* view_origin, int active)
{
    s32 sel;
    s32 row;
    s32 col;
    s32 bit;
    s32 y;
    s32 rel_y;
    u8* mask;
    u32 scroll_y;
    void* base;
    void* sel_base;
    ScrollListState* list;

    list = state;

    if ((g_pad_input & 0x40) && (active != 0))
    {
        func_8014F210(0x7F, 0x80);
        list->unk0 = 3;
        g_pad_input = 0;
    }

    prim_buf = scroll_list_draw(prim_buf, ot, list, &D_80168C70, view_origin, active);

    y = 0;
    sel = -1;
    row = 0;
    mask = (u8*)g_pad_ctx + 0x60;
    scroll_y = list->scroll_y;

    do
    {
        col = 0;
        bit = 1;
        do
        {
            if (*mask & bit)
            {
                rel_y = y - scroll_y;
                if ((rel_y >= -0xF) && (rel_y < (list->viewport_h - 0x10)))
                {
                    base = (void*)(g_menu_state_ptr + *(s32*)(g_menu_state_ptr + 0x10));
                    prim_buf = func_800A88A0(prim_buf, ot,
                                             (void*)((u8*)base + *(u16*)((u8*)base + (col * 2) + (row * 0x10))),
                                             1, 0x10 - view_origin->x, rel_y - view_origin->y, 0);
                }
                if (list->unk4 == (y >> 4))
                {
                    sel = col + (row * 8);
                }
                y += 0x10;
            }
            col += 1;
            bit *= 2;
        } while (col < 8);
        row += 1;
        mask += 1;
    } while (row < 0xC);

    if ((g_pad_input & 0x220) && (active != 0))
    {
        *((u8*)g_pad_ctx + (g_menu_char_slot * 0x250) + g_menu_active_subtype + (row = 0x609)) = sel;
        func_8014F210(0x7E, 0x80);
        list->unk0 = 3;
    }

    if (sel != -1)
    {
        sel_base = (void*)(g_menu_state_ptr + *(s32*)(g_menu_state_ptr + 0x3C));
        g_menu_pending_overlay = (s32)((u8*)sel_base + *(u16*)((u8*)sel_base + (sel * 2)));
    }

    return prim_buf;
}
