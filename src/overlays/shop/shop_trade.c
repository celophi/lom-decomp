#include "shop_trade.h"

#define SHOP_PROMPT_X 64
#define SHOP_PROMPT_Y 112
#define SHOP_PROMPT_WIDTH 192
#define SHOP_PROMPT_HEIGHT 32

static u8* shop_draw_buy_prompt(u32* ot, u8* prim, s32 x_inset, s32 y_inset);
static u8* shop_draw_sell_prompt(u32* ot, u8* prim, s32 x_inset, s32 y_inset);

/**
 * @brief Draw the notice selected by g_shop_notice_id.
 * @param ot Ordering-table entry for the emitted packets.
 * @param prim Packet cursor.
 * @param x_inset Horizontal inset of the opening or closing window.
 * @param y_inset Vertical inset of the opening or closing window.
 * @return Advanced packet cursor.
 */
u8* shop_draw_notice_window(u32* ot, u8* prim, s32 x_inset, s32 y_inset)
{
    Vec2s unused_position; /* never used, but the compiled frame size depends on it */

    switch (g_shop_notice_id)
    {
    case SHOP_NOTICE_CANNOT_CARRY:
        prim = func_800A88A0(prim, ot, FIELD_UI_TEXT_AT(D_800EC3D0, 6), SHOP_TEXT_COLOR_NOTICE, 128 - x_inset, -y_inset, 2);
        break;
    case SHOP_NOTICE_QUANTITY_REDUCED:
        prim = func_800A88A0(prim, ot, FIELD_UI_TEXT_AT(D_800EC3FE, 29), SHOP_TEXT_COLOR_NOTICE, 128 - x_inset, -y_inset, 2);
        break;
    }
    return prim;
}

/**
 * @brief Open the yes/no prompt confirming a purchase of the selected entry.
 */
void shop_open_buy_prompt(void)
{
    g_shop_windows[SHOP_POPUP_WINDOW].draw = shop_draw_buy_prompt;
    g_shop_prompt_active = 1;
    g_shop_confirm_choice = 0;
    g_shop_windows[SHOP_POPUP_WINDOW].frame.bits.state = SHOP_WINDOW_OPENING;
    g_shop_windows[SHOP_POPUP_WINDOW].frame.bits.anim_step = 1;
    g_shop_windows[SHOP_POPUP_WINDOW].frame.bits.x = SHOP_PROMPT_X;
    g_shop_windows[SHOP_POPUP_WINDOW].frame.bits.y = SHOP_PROMPT_Y;
    g_shop_windows[SHOP_POPUP_WINDOW].extent.bits.width_high = SHOP_WINDOW_WIDTH_HIGH(SHOP_PROMPT_WIDTH);
    g_shop_windows[SHOP_POPUP_WINDOW].extent.bits.height = SHOP_PROMPT_HEIGHT;
    g_shop_windows[SHOP_POPUP_WINDOW].frame.word = (g_shop_windows[SHOP_POPUP_WINDOW].frame.word & 0x00FFFFFF) | SHOP_WINDOW_WIDTH_LOW(SHOP_PROMPT_WIDTH);
    field_reset_input_repeat();
}

/**
 * @brief Run the purchase prompt and draw it.
 *
 * Confirming buys g_shop_quantity of the selected entry, reduced to what the
 * player can carry; a reduction reopens the popup as a notice.
 *
 * @param ot Ordering-table entry for the emitted packets.
 * @param prim Packet cursor.
 * @param x_inset Horizontal inset of the opening or closing window.
 * @param y_inset Vertical inset of the opening or closing window.
 * @return Advanced packet cursor.
 */
static u8* shop_draw_buy_prompt(u32* ot, u8* prim, s32 x_inset, s32 y_inset)
{
    s32 confirm_mask;
    Vec2s unused_position; /* never used, but the compiled frame size depends on it */
    ShopEntry* entry;
    ShopEntry* stock;
    InventoryRecord* record;
    s32 reduced;
    s32 i;
    s32 free_records;
    s32 room;
    u8* text_table;
    u8* text;
    s32 color;
    s32 value;
    s32 row_y;

    reduced = 0;
    confirm_mask = SHOP_CONFIRM_BUTTONS;
    if (g_shop_windows[SHOP_POPUP_WINDOW].frame.bits.state == SHOP_WINDOW_OPEN)
    {
        if (g_pad_input & SHOP_DPAD_BUTTONS)
        {
            play_menu_sfx(SHOP_SFX_CURSOR, SHOP_SFX_VOLUME);
            g_shop_confirm_choice ^= 1;
        }
        else if ((g_pad_input & confirm_mask) && (g_shop_confirm_choice == 0))
        {
            record = field_find_free_inventory_record();
            entry = SHOP_SELECTED_ENTRY();
            if (entry->id & SHOP_ENTRY_RECORD_FLAG)
            {
                free_records = &g_pad_ctx->inventory[INVENTORY_RECORD_COUNT] - record;
                if (free_records < g_shop_quantity)
                {
                    g_shop_quantity = free_records;
                    reduced = 1;
                }
                for (i = 0; i < g_shop_quantity; i++, record++)
                {
                    field_copy_inventory_record(record, &g_shop_item_records[SHOP_SELECTED_ENTRY()->id & SHOP_ENTRY_RECORD_INDEX_MASK]);
                }
                play_menu_sfx(SHOP_SFX_TRADE, SHOP_SFX_VOLUME);
                stock = SHOP_SELECTED_ENTRY();
                g_pad_ctx->money -= stock->price * g_shop_quantity;
                if (stock->count != 0)
                {
                    stock->count -= g_shop_quantity;
                    if (stock->count == 0)
                    {
                        stock->id = SHOP_ENTRY_EMPTY;
                    }
                }
            }
            else
            {
                room = SHOP_MAX_ITEM_COUNT - g_pad_ctx->item_counts[entry->id];
                if (room < g_shop_quantity)
                {
                    g_shop_quantity = room;
                    reduced = 1;
                }
                g_pad_ctx->item_counts[entry->id] += g_shop_quantity;
                play_menu_sfx(SHOP_SFX_TRADE, SHOP_SFX_VOLUME);
                stock = SHOP_SELECTED_ENTRY();
                g_pad_ctx->money -= stock->price * g_shop_quantity;
                if (stock->count != 0)
                {
                    stock->count -= g_shop_quantity;
                    if (stock->count == 0)
                    {
                        stock->id = SHOP_ENTRY_EMPTY;
                    }
                }
            }
            g_shop_quantity = 1;
            g_shop_prompt_active = 0;
            g_shop_windows[SHOP_POPUP_WINDOW].frame.bits.state = SHOP_WINDOW_FREE;
            if (reduced != 0)
            {
                u32 frame;

                /* Reopen the popup as the quantity-reduced notice; state, anim_step and x are packed together. */
                frame = g_shop_windows[SHOP_POPUP_WINDOW].frame.word & ~0x7;
                frame |= SHOP_WINDOW_OPENING;
                frame &= ~0x78;
                frame |= 1 << 3;
                frame &= ~0xFF80;
                frame |= SHOP_NOTICE_WINDOW_X << 7;
                value = 1;
                g_shop_notice_id = value;
                g_shop_windows[SHOP_POPUP_WINDOW].frame.word = frame;
                g_shop_windows[SHOP_POPUP_WINDOW].frame.bits.y = SHOP_NOTICE_WINDOW_Y;
                g_shop_windows[SHOP_POPUP_WINDOW].draw = shop_draw_notice_window;
                g_shop_notice_active = value;
                g_shop_windows[SHOP_POPUP_WINDOW].frame.word =
                    (g_shop_windows[SHOP_POPUP_WINDOW].frame.word & 0x00FFFFFF) | SHOP_WINDOW_WIDTH_LOW(SHOP_NOTICE_WINDOW_WIDTH);
                g_shop_windows[SHOP_POPUP_WINDOW].extent.bits.width_high = SHOP_WINDOW_WIDTH_HIGH(SHOP_NOTICE_WINDOW_WIDTH);
                g_shop_windows[SHOP_POPUP_WINDOW].extent.bits.height = SHOP_NOTICE_WINDOW_HEIGHT;
                field_reset_input_repeat();
            }
            field_reset_input_repeat();
        }
        else if ((g_pad_input & SHOP_CANCEL_BUTTONS) || ((g_pad_input & confirm_mask) && (g_shop_confirm_choice != 0)))
        {
            g_shop_windows[SHOP_POPUP_WINDOW].frame.bits.state = SHOP_WINDOW_FREE;
            play_menu_sfx(SHOP_SFX_CANCEL, SHOP_SFX_VOLUME);
            g_shop_prompt_active = 0;
            field_reset_input_repeat();
        }
    }

    text = FIELD_UI_TEXT_AT(D_800EC3F8, 26);
    /* D_800EC3F8 is table entry 26, so this is the start of FIELD's UI string table. */
    text_table = D_800EC3F8 - 26 * 2;
    prim = func_800A88A0(prim, ot, text, SHOP_TEXT_COLOR_NORMAL, 96 - x_inset, -y_inset, 2);

    value = SHOP_TEXT_COLOR_DISABLED;
    color = SHOP_TEXT_COLOR_NORMAL;
    text = FIELD_UI_TEXT(text_table, 27);
    if (g_shop_confirm_choice != 0)
    {
        color = value;
    }
    row_y = 16 - y_inset;
    prim = func_800A88A0(prim, ot, text, color, 72 - x_inset, row_y, 1);

    color = SHOP_TEXT_COLOR_DISABLED;
    text = FIELD_UI_TEXT(text_table, 28);
    if (g_shop_confirm_choice != 0)
    {
        color = SHOP_TEXT_COLOR_NORMAL;
    }
    prim = func_800A88A0(prim, ot, text, color, 104 - x_inset, row_y, 0);
    return prim;
}

/**
 * @brief Open the yes/no prompt confirming a sale of the selected entry.
 */
void shop_open_sell_prompt(void)
{
    g_shop_windows[SHOP_POPUP_WINDOW].draw = shop_draw_sell_prompt;
    g_shop_prompt_active = 1;
    g_shop_confirm_choice = 0;
    g_shop_windows[SHOP_POPUP_WINDOW].frame.bits.state = SHOP_WINDOW_OPENING;
    g_shop_windows[SHOP_POPUP_WINDOW].frame.bits.anim_step = 1;
    g_shop_windows[SHOP_POPUP_WINDOW].frame.bits.x = SHOP_PROMPT_X;
    g_shop_windows[SHOP_POPUP_WINDOW].frame.bits.y = SHOP_PROMPT_Y;
    g_shop_windows[SHOP_POPUP_WINDOW].extent.bits.width_high = SHOP_WINDOW_WIDTH_HIGH(SHOP_PROMPT_WIDTH);
    g_shop_windows[SHOP_POPUP_WINDOW].extent.bits.height = SHOP_PROMPT_HEIGHT;
    g_shop_windows[SHOP_POPUP_WINDOW].frame.word = (g_shop_windows[SHOP_POPUP_WINDOW].frame.word & 0x00FFFFFF) | SHOP_WINDOW_WIDTH_LOW(SHOP_PROMPT_WIDTH);
    field_reset_input_repeat();
}

/**
 * @brief Run the sale prompt and draw it.
 *
 * Confirming sells g_shop_quantity of the selected entry and credits the
 * player, capped at SHOP_MAX_MONEY.
 *
 * @param ot Ordering-table entry for the emitted packets.
 * @param prim Packet cursor.
 * @param x_inset Horizontal inset of the opening or closing window.
 * @param y_inset Vertical inset of the opening or closing window.
 * @return Advanced packet cursor.
 */
static u8* shop_draw_sell_prompt(u32* ot, u8* prim, s32 x_inset, s32 y_inset)
{
    Vec2s unused_position; /* never used, but the compiled frame size depends on it */
    ShopEntry* entry;
    ShopEntry* stock;
    u32 money;
    u8* text_table;
    u8* text;
    s32 color;
    s32 row_y;
    s32 state;
    s32 y;

    state = g_shop_windows[SHOP_POPUP_WINDOW].frame.bits.state;
    y = y_inset;
    if (state == SHOP_WINDOW_OPEN)
    {
        if (g_pad_input & SHOP_DPAD_BUTTONS)
        {
            play_menu_sfx(SHOP_SFX_CURSOR, SHOP_SFX_VOLUME);
            g_shop_confirm_choice ^= 1;
        }
        else if ((g_pad_input & SHOP_CONFIRM_BUTTONS) && (g_shop_confirm_choice == 0))
        {
            entry = SHOP_SELECTED_ENTRY();
            if (entry->id & SHOP_ENTRY_RECORD_FLAG)
            {
                g_shop_item_records[entry->id & SHOP_ENTRY_RECORD_INDEX_MASK].active = 0;
                SHOP_SELECTED_ENTRY()->id = SHOP_ENTRY_EMPTY;
            }
            else
            {
                g_pad_ctx->item_counts[entry->id] -= g_shop_quantity;
                stock = SHOP_SELECTED_ENTRY();
                stock->count -= g_shop_quantity;
                if (g_pad_ctx->item_counts[stock->id] == 0)
                {
                    stock->id = SHOP_ENTRY_EMPTY;
                }
            }
            play_menu_sfx(SHOP_SFX_TRADE, SHOP_SFX_VOLUME);
            money = g_pad_ctx->money + SHOP_SELECTED_ENTRY()->price * g_shop_quantity;
            g_pad_ctx->money = money;
            if (money > SHOP_MAX_MONEY)
            {
                g_pad_ctx->money = SHOP_MAX_MONEY;
            }
            g_shop_quantity = 1;
            g_shop_prompt_active = 0;
            g_shop_windows[SHOP_POPUP_WINDOW].frame.bits.state = SHOP_WINDOW_FREE;
            field_reset_input_repeat();
        }
        else if ((g_pad_input & SHOP_CANCEL_BUTTONS) || ((g_pad_input & SHOP_CONFIRM_BUTTONS) && (g_shop_confirm_choice != 0)))
        {
            g_shop_windows[SHOP_POPUP_WINDOW].frame.bits.state = SHOP_WINDOW_FREE;
            play_menu_sfx(SHOP_SFX_CANCEL, SHOP_SFX_VOLUME);
            g_shop_prompt_active = 0;
            field_reset_input_repeat();
        }
    }

    text = FIELD_UI_TEXT_AT(D_800EC3F8, 26);
    /* D_800EC3F8 is table entry 26, so this is the start of FIELD's UI string table. */
    text_table = D_800EC3F8 - 26 * 2;
    prim = func_800A88A0(prim, ot, text, SHOP_TEXT_COLOR_NORMAL, 96 - x_inset, -y, 2);

    color = SHOP_TEXT_COLOR_NORMAL;
    text = FIELD_UI_TEXT(text_table, 27);
    if (g_shop_confirm_choice != 0)
    {
        color = SHOP_TEXT_COLOR_DISABLED;
    }
    row_y = 16 - y;
    prim = func_800A88A0(prim, ot, text, color, 88 - x_inset, row_y, 1);

    color = SHOP_TEXT_COLOR_DISABLED;
    text = FIELD_UI_TEXT(text_table, 28);
    if (g_shop_confirm_choice != 0)
    {
        color = SHOP_TEXT_COLOR_NORMAL;
    }
    prim = func_800A88A0(prim, ot, text, color, 104 - x_inset, row_y, 0);
    return prim;
}
