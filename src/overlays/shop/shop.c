#include "shop.h"
#include "shop_internal.h"
#include "shop_render.h"
#include "shop_trade.h"
#include "field_text.h"

#define SHOP_MONEY_WINDOW_X 160
#define SHOP_MONEY_WINDOW_Y 32
#define SHOP_MONEY_WINDOW_WIDTH 122
#define SHOP_MONEY_WINDOW_HEIGHT 20
#define SHOP_LIST_WINDOW_X 8
#define SHOP_LIST_WINDOW_Y 56
#define SHOP_LIST_WINDOW_WIDTH 294
#define SHOP_LIST_WINDOW_HEIGHT 116
#define SHOP_DETAIL_WINDOW_X 10
#define SHOP_DETAIL_WINDOW_Y 176
#define SHOP_DETAIL_WINDOW_WIDTH 300
#define SHOP_DETAIL_WINDOW_HEIGHT 36
#define SHOP_TITLE_WINDOW_X 32
#define SHOP_TITLE_WINDOW_Y 32
#define SHOP_TITLE_WINDOW_WIDTH 112
#define SHOP_TITLE_WINDOW_HEIGHT 20

/** @brief Rows an R1/L1 page jump moves the cursor. */
#define SHOP_PAGE_ROWS 7
/** @brief Row offset past which the list scrolls to keep the cursor visible. */
#define SHOP_LIST_SCROLL_LIMIT 100
#define SHOP_LIST_SCROLL_LIMIT_ROWS 6
#define SHOP_SCROLL_FRAMES 4

static u8* shop_build_sell_list(u8* work_end);
static u8* shop_build_buy_list(u8* work_end, s32 entry_count, ShopEntry* entries, InventoryRecord* item_records);
static void shop_run_frame(ShopFrameContext* ctx, ShopPacketBuffer* buffer);
static s32 shop_handle_list_input(void);
static void shop_open_notice(s32 notice_id);
static void shop_draw(ShopFrameContext* ctx, ShopPacketBuffer* buffer);
static void shop_reset_windows(void);
static ShopWindow* shop_alloc_window(void);

/**
 * @brief Start a shop session and build its item list.
 * @param work Work memory; the two packet buffers are carved from its start.
 * @param is_buying Non-zero to buy from @p entries, zero to sell from the player's inventory.
 * @param entry_count Number of entries in @p entries when buying.
 * @param entries Stock offered when buying.
 * @param item_records Inventory records that record-type buy entries index.
 * @param title_text_id FIELD UI string shown in the title window.
 */
void shop_init(u8* work, s32 is_buying, s32 entry_count, ShopEntry* entries, InventoryRecord* item_records, s32 title_text_id)
{
    g_shop_work_buffer = (ShopPacketBuffer*)work;
    work += 2 * sizeof(ShopPacketBuffer);
    field_set_default_fade_target();
    g_shop_frame_index = 0;
    g_shop_finished = 0;
    g_shop_is_buying = is_buying;
    g_shop_title_text_id = title_text_id;
    field_reset_input_repeat();
    if (g_shop_is_buying != 0)
    {
        work = shop_build_buy_list(work, entry_count, entries, item_records);
    }
    else
    {
        work = shop_build_sell_list(work);
    }
    g_shop_work_end = work;
}

/**
 * @brief Run one shop frame into the current packet buffer and flip buffers.
 * @param ctx Host frame context whose ordering-table entry receives the packets.
 * @return Non-zero once the shop has closed.
 */
s32 shop_update(ShopFrameContext* ctx)
{
    ShopPacketBuffer* buffer;

    buffer = &g_shop_work_buffer[g_shop_frame_index];
    buffer->prim_cursor = buffer->packets;
    field_text_reset_scratch();
    shop_run_frame(ctx, buffer);
    field_text_upload_immediate_cache();
    g_shop_frame_index ^= 1;
    if (g_shop_finished != 0)
    {
        field_text_reset_windows();
    }
    return g_shop_finished;
}

/**
 * @brief List the player's inventory records and items for selling, then open the shop windows.
 * @param work_end First free byte after the packet buffers.
 * @return @p work_end; the list lives in overlay data.
 */
static u8* shop_build_sell_list(u8* work_end)
{
    ShopWindow* window;
    Vec2s unused_position; /* never used, but the compiled frame size depends on it */
    InventoryRecord* record;
    s32 count;
    s32 i;

    g_shop_prompt_active = 0;
    g_shop_confirm_choice = 0;
    record = g_pad_ctx->inventory;
    for (count = 0; count < INVENTORY_RECORD_COUNT; count++, record++)
    {
        if (record->active == 0)
        {
            break;
        }
        g_shop_entry_buffer[count].id = count + SHOP_ENTRY_RECORD_FLAG;
        g_shop_entry_buffer[count].count = 1;
        g_shop_entry_buffer[count].price = record->price;
    }

    for (i = 0; i < ITEM_TYPE_COUNT; i++)
    {
        if (g_pad_ctx->item_counts[i] != 0)
        {
            g_shop_entry_buffer[count].id = i;
            g_shop_entry_buffer[count].count = g_pad_ctx->item_counts[i];
            g_shop_entry_buffer[count].price = g_shop_item_sell_prices[i];
            count++;
        }
    }

    g_shop_entries = g_shop_entry_buffer;
    g_shop_entry_count = count;
    g_shop_scroll_frames = 0;
    g_shop_scroll_target = 0;
    g_shop_scroll_y = 0;
    g_shop_quantity = 1;
    g_shop_cursor = 0;
    g_shop_item_records = g_pad_ctx->inventory;
    shop_reset_windows();

    /* Keep the popup slot out of shop_alloc_window's reach while the fixed windows open. */
    g_shop_windows[SHOP_POPUP_WINDOW].frame.bits.state = SHOP_WINDOW_OPENING;

    window = shop_alloc_window();
    window->draw = shop_draw_money_window;
    window->frame.bits.anim_step = 1;
    window->frame.bits.x = SHOP_MONEY_WINDOW_X;
    window->extent.bits.width_high = SHOP_WINDOW_WIDTH_HIGH(SHOP_MONEY_WINDOW_WIDTH);
    window->extent.bits.height = SHOP_MONEY_WINDOW_HEIGHT;
    window->frame.bits.y = SHOP_MONEY_WINDOW_Y;
    window->frame.word = (window->frame.word & 0x00FFFFFF) | SHOP_WINDOW_WIDTH_LOW(SHOP_MONEY_WINDOW_WIDTH);

    window = shop_alloc_window();
    window->draw = shop_draw_list_window;
    window->frame.bits.anim_step = 1;
    window->frame.bits.x = SHOP_LIST_WINDOW_X;
    window->extent.bits.width_high = SHOP_WINDOW_WIDTH_HIGH(SHOP_LIST_WINDOW_WIDTH);
    window->extent.bits.height = SHOP_LIST_WINDOW_HEIGHT;
    window->frame.bits.y = SHOP_LIST_WINDOW_Y;
    window->frame.word = (window->frame.word & 0x00FFFFFF) | SHOP_WINDOW_WIDTH_LOW(SHOP_LIST_WINDOW_WIDTH);

    window = shop_alloc_window();
    window->draw = shop_draw_detail_window;
    window->frame.bits.anim_step = 1;
    window->frame.bits.x = SHOP_DETAIL_WINDOW_X;
    window->extent.bits.width_high = SHOP_WINDOW_WIDTH_HIGH(SHOP_DETAIL_WINDOW_WIDTH);
    window->extent.bits.height = SHOP_DETAIL_WINDOW_HEIGHT;
    window->frame.bits.y = SHOP_DETAIL_WINDOW_Y;
    window->frame.word = (window->frame.word & 0x00FFFFFF) | SHOP_WINDOW_WIDTH_LOW(SHOP_DETAIL_WINDOW_WIDTH);

    window = shop_alloc_window();
    window->draw = shop_draw_title_window;
    window->frame.bits.anim_step = 1;
    window->frame.bits.x = SHOP_TITLE_WINDOW_X;
    window->frame.bits.y = SHOP_TITLE_WINDOW_Y;
    window->extent.bits.width_high = SHOP_WINDOW_WIDTH_HIGH(SHOP_TITLE_WINDOW_WIDTH);
    window->extent.bits.height = SHOP_TITLE_WINDOW_HEIGHT;
    window->frame.word = (window->frame.word & 0x00FFFFFF) | SHOP_WINDOW_WIDTH_LOW(SHOP_TITLE_WINDOW_WIDTH);
    g_shop_notice_active = 0;
    g_shop_windows[SHOP_POPUP_WINDOW].frame.bits.state = SHOP_WINDOW_FREE;
    return work_end;
}

/**
 * @brief Copy the caller's stock into the shop list and open the shop windows.
 * @param work_end First free byte after the packet buffers.
 * @param entry_count Number of entries in @p entries.
 * @param entries Stock to offer.
 * @param item_records Inventory records that record-type entries index.
 * @return @p work_end; the list lives in overlay data.
 */
static u8* shop_build_buy_list(u8* work_end, s32 entry_count, ShopEntry* entries, InventoryRecord* item_records)
{
    ShopWindow* window;
    Vec2s unused_position; /* never used, but the compiled frame size depends on it */
    s32 i;

    g_shop_prompt_active = 0;
    g_shop_confirm_choice = 0;
    for (i = 0; i < entry_count; i++)
    {
        g_shop_entry_buffer[i].id = entries[i].id;
        g_shop_entry_buffer[i].count = entries[i].count;
        g_shop_entry_buffer[i].price = entries[i].price;
    }
    g_shop_entries = g_shop_entry_buffer;
    g_shop_entry_count = entry_count;
    g_shop_item_records = item_records;
    g_shop_scroll_frames = 0;
    g_shop_scroll_target = 0;
    g_shop_scroll_y = 0;
    g_shop_quantity = 1;
    g_shop_cursor = 0;
    shop_reset_windows();

    /* Takes the popup slot; it is released again below once the fixed windows are placed. */
    window = shop_alloc_window();
    window->draw = shop_draw_notice_window;
    window->frame.bits.anim_step = 1;
    window->frame.bits.x = SHOP_NOTICE_WINDOW_X;
    window->extent.bits.width_high = SHOP_WINDOW_WIDTH_HIGH(SHOP_NOTICE_WINDOW_WIDTH);
    window->extent.bits.height = SHOP_NOTICE_WINDOW_HEIGHT;
    window->frame.bits.y = SHOP_NOTICE_WINDOW_Y;
    window->frame.word = (window->frame.word & 0x00FFFFFF) | SHOP_WINDOW_WIDTH_LOW(SHOP_NOTICE_WINDOW_WIDTH);

    window = shop_alloc_window();
    window->draw = shop_draw_money_window;
    window->frame.bits.anim_step = 1;
    window->frame.bits.x = SHOP_MONEY_WINDOW_X;
    window->extent.bits.width_high = SHOP_WINDOW_WIDTH_HIGH(SHOP_MONEY_WINDOW_WIDTH);
    window->extent.bits.height = SHOP_MONEY_WINDOW_HEIGHT;
    window->frame.bits.y = SHOP_MONEY_WINDOW_Y;
    window->frame.word = (window->frame.word & 0x00FFFFFF) | SHOP_WINDOW_WIDTH_LOW(SHOP_MONEY_WINDOW_WIDTH);

    window = shop_alloc_window();
    window->draw = shop_draw_list_window;
    window->frame.bits.anim_step = 1;
    window->frame.bits.x = SHOP_LIST_WINDOW_X;
    window->extent.bits.width_high = SHOP_WINDOW_WIDTH_HIGH(SHOP_LIST_WINDOW_WIDTH);
    window->extent.bits.height = SHOP_LIST_WINDOW_HEIGHT;
    window->frame.bits.y = SHOP_LIST_WINDOW_Y;
    window->frame.word = (window->frame.word & 0x00FFFFFF) | SHOP_WINDOW_WIDTH_LOW(SHOP_LIST_WINDOW_WIDTH);

    window = shop_alloc_window();
    window->draw = shop_draw_detail_window;
    window->frame.bits.anim_step = 1;
    window->frame.bits.x = SHOP_DETAIL_WINDOW_X;
    window->extent.bits.width_high = SHOP_WINDOW_WIDTH_HIGH(SHOP_DETAIL_WINDOW_WIDTH);
    window->extent.bits.height = SHOP_DETAIL_WINDOW_HEIGHT;
    window->frame.bits.y = SHOP_DETAIL_WINDOW_Y;
    window->frame.word = (window->frame.word & 0x00FFFFFF) | SHOP_WINDOW_WIDTH_LOW(SHOP_DETAIL_WINDOW_WIDTH);

    window = shop_alloc_window();
    window->draw = shop_draw_title_window;
    window->frame.bits.anim_step = 1;
    window->frame.bits.x = SHOP_TITLE_WINDOW_X;
    window->frame.bits.y = SHOP_TITLE_WINDOW_Y;
    window->extent.bits.width_high = SHOP_WINDOW_WIDTH_HIGH(SHOP_TITLE_WINDOW_WIDTH);
    window->extent.bits.height = SHOP_TITLE_WINDOW_HEIGHT;
    g_shop_notice_active = 0;
    window->frame.word = (window->frame.word & 0x00FFFFFF) | SHOP_WINDOW_WIDTH_LOW(SHOP_TITLE_WINDOW_WIDTH);
    g_shop_windows[SHOP_POPUP_WINDOW].frame.bits.state = SHOP_WINDOW_FREE;
    return work_end;
}

/**
 * @brief Draw the windows, process list input, and advance the list scroll animation.
 * @param ctx Host frame context.
 * @param buffer Packet buffer for this frame.
 */
static void shop_run_frame(ShopFrameContext* ctx, ShopPacketBuffer* buffer)
{
    shop_draw(ctx, buffer);
    shop_handle_list_input();
    if (g_shop_scroll_frames != 0)
    {
        g_shop_scroll_y += (g_shop_scroll_target - g_shop_scroll_y) / g_shop_scroll_frames;
        g_shop_scroll_frames--;
    }
    else
    {
        g_shop_scroll_y = g_shop_scroll_target;
    }
}

/**
 * @brief Handle list navigation, quantity changes, trade confirmation, and closing the shop.
 * @return No value; the function has an int return type but never returns one, and callers ignore it.
 */
static s32 shop_handle_list_input(void)
{
    ShopWindow* window;
    ShopEntry* entry;
    ShopEntry* stock;
    s32 target_y;
    s32 current_scroll;
    s32 delta;
    s32 state;
    s32 i;

    state = g_shop_windows[SHOP_MONEY_WINDOW].frame.bits.state;
    if (state == SHOP_WINDOW_FREE)
    {
        g_shop_finished = 1;
    }
    if (state != SHOP_WINDOW_OPEN)
    {
        return;
    }

    if (g_shop_notice_active != 0)
    {
        if ((g_pad_input & (SHOP_CONFIRM_BUTTONS | SHOP_CANCEL_BUTTONS)) == 0)
        {
            return;
        }
        if (g_shop_windows[SHOP_POPUP_WINDOW].frame.bits.state == SHOP_WINDOW_FREE)
        {
            return;
        }
        g_shop_windows[SHOP_POPUP_WINDOW].frame.bits.state = SHOP_WINDOW_CLOSING;
        g_shop_windows[SHOP_POPUP_WINDOW].frame.bits.anim_step = SHOP_WINDOW_ANIM_STEPS;
        return;
    }

    if ((g_shop_prompt_active != 0) || (g_shop_scroll_frames != 0))
    {
        return;
    }

    /* R1 and L1 page through the list; i counts the rows still to move. */
    i = 1;
    if (g_pad_input & PAD_BTN_R1)
    {
        g_pad_input = PAD_BTN_DOWN;
        i = SHOP_PAGE_ROWS;
    }
    if (g_pad_input & PAD_BTN_L1)
    {
        g_pad_input = PAD_BTN_UP;
        i = SHOP_PAGE_ROWS;
    }
    while (i != 0)
    {
        if (g_pad_input & PAD_BTN_UP)
        {
            g_shop_quantity = 1;
            g_shop_cursor--;
            if (g_shop_cursor < 0)
            {
                g_shop_cursor = g_shop_entry_count - 1;
            }
        }
        if (g_pad_input & PAD_BTN_DOWN)
        {
            g_shop_quantity = 1;
            g_shop_cursor++;
            if (g_shop_cursor >= g_shop_entry_count)
            {
                g_shop_cursor = 0;
            }
        }
        /* A page jump stops at either end of the list. */
        if ((g_shop_cursor == g_shop_entry_count - 1) || (g_shop_cursor == 0))
        {
            i = 1;
        }
        i--;
    }

    if (g_pad_input & (PAD_BTN_UP | PAD_BTN_DOWN))
    {
        play_menu_sfx(SHOP_SFX_CURSOR, SHOP_SFX_VOLUME);
        target_y = g_shop_cursor << SHOP_ROW_HEIGHT_SHIFT;
        current_scroll = g_shop_scroll_y;
        delta = target_y - current_scroll;
        if (delta > SHOP_LIST_SCROLL_LIMIT)
        {
            g_shop_scroll_target = target_y - SHOP_LIST_SCROLL_LIMIT_ROWS * SHOP_ROW_HEIGHT;
            g_shop_scroll_frames = SHOP_SCROLL_FRAMES;
        }
        if (delta < 0)
        {
            g_shop_scroll_target = target_y;
            g_shop_scroll_frames = SHOP_SCROLL_FRAMES;
        }
    }

    if (g_pad_input & PAD_BTN_LEFT)
    {
        if (g_shop_quantity >= 2)
        {
            play_menu_sfx(SHOP_SFX_CURSOR, SHOP_SFX_VOLUME);
            g_shop_quantity--;
        }
    }

    if (g_pad_input & PAD_BTN_RIGHT)
    {
        stock = SHOP_SELECTED_ENTRY();
        if (stock->count != 0)
        {
            if (g_shop_quantity < stock->count)
            {
                play_menu_sfx(SHOP_SFX_CURSOR, SHOP_SFX_VOLUME);
                g_shop_quantity++;
            }
        }
        else
        {
            if (g_shop_quantity < SHOP_MAX_ITEM_COUNT)
            {
                play_menu_sfx(SHOP_SFX_CURSOR, SHOP_SFX_VOLUME);
                g_shop_quantity++;
            }
        }
    }

    if (g_pad_input & SHOP_CONFIRM_BUTTONS)
    {
        entry = SHOP_SELECTED_ENTRY();
        if (entry->id != SHOP_ENTRY_EMPTY)
        {
            if (g_shop_is_buying != 0)
            {
                if (g_pad_ctx->money >= (u32)(entry->price * g_shop_quantity))
                {
                    if (entry->id & SHOP_ENTRY_RECORD_FLAG)
                    {
                        if (field_find_free_inventory_record() == 0)
                        {
                            play_menu_sfx(SHOP_SFX_ERROR, SHOP_SFX_VOLUME);
                            shop_open_notice(SHOP_NOTICE_CANNOT_CARRY);
                        }
                        else
                        {
                            shop_open_buy_prompt();
                        }
                    }
                    else if (g_pad_ctx->item_counts[entry->id] >= SHOP_MAX_ITEM_COUNT)
                    {
                        play_menu_sfx(SHOP_SFX_ERROR, SHOP_SFX_VOLUME);
                        shop_open_notice(SHOP_NOTICE_CANNOT_CARRY);
                    }
                    else
                    {
                        shop_open_buy_prompt();
                    }
                }
                else
                {
                    play_menu_sfx(SHOP_SFX_ERROR, SHOP_SFX_VOLUME);
                }
            }
            else
            {
                shop_open_sell_prompt();
            }
        }
    }

    if (g_pad_input & SHOP_CANCEL_BUTTONS)
    {
        play_menu_sfx(SHOP_SFX_CANCEL, SHOP_SFX_VOLUME);
        if (g_shop_is_buying == 0)
        {
            field_compact_inventory();
        }
        field_restore_fade_target();
        window = g_shop_windows;
        for (i = 0; i < SHOP_WINDOW_COUNT; i++, window++)
        {
            if (window->frame.bits.state != SHOP_WINDOW_FREE)
            {
                window->frame.bits.state = SHOP_WINDOW_CLOSING;
                window->frame.bits.anim_step = SHOP_WINDOW_ANIM_STEPS;
            }
        }
    }
}

/**
 * @brief Show a notice in the popup window and block list input until it is dismissed.
 * @param notice_id Notice to show; see shop_draw_notice_window.
 */
static void shop_open_notice(s32 notice_id)
{
    g_shop_notice_id = notice_id;
    g_shop_windows[SHOP_POPUP_WINDOW].draw = shop_draw_notice_window;
    g_shop_notice_active = 1;
    g_shop_windows[SHOP_POPUP_WINDOW].frame.bits.state = SHOP_WINDOW_OPENING;
    g_shop_windows[SHOP_POPUP_WINDOW].frame.bits.anim_step = 1;
    g_shop_windows[SHOP_POPUP_WINDOW].frame.bits.x = SHOP_NOTICE_WINDOW_X;
    g_shop_windows[SHOP_POPUP_WINDOW].frame.bits.y = SHOP_NOTICE_WINDOW_Y;
    g_shop_windows[SHOP_POPUP_WINDOW].extent.bits.width_high = SHOP_WINDOW_WIDTH_HIGH(SHOP_NOTICE_WINDOW_WIDTH);
    g_shop_windows[SHOP_POPUP_WINDOW].extent.bits.height = SHOP_NOTICE_WINDOW_HEIGHT;
    g_shop_windows[SHOP_POPUP_WINDOW].frame.word =
        (g_shop_windows[SHOP_POPUP_WINDOW].frame.word & 0x00FFFFFF) | SHOP_WINDOW_WIDTH_LOW(SHOP_NOTICE_WINDOW_WIDTH);
    field_reset_input_repeat();
}

/**
 * @brief Draw every active shop window into this frame's packet buffer.
 * @param ctx Host frame context.
 * @param buffer Packet buffer for this frame.
 */
static void shop_draw(ShopFrameContext* ctx, ShopPacketBuffer* buffer)
{
    shop_draw_windows(ctx, buffer);
}

/**
 * @brief Free every window slot.
 */
static void shop_reset_windows(void)
{
    ShopWindow* window;
    s32 i;

    g_menu_element_counter = 0x20;
    window = g_shop_windows;
    for (i = 0; i < SHOP_WINDOW_COUNT; i++)
    {
        window->frame.bits.state = SHOP_WINDOW_FREE;
        window++;
    }
}

/**
 * @brief Claim the first free window slot and mark it opening.
 * @return The claimed window, or the popup window when every slot is in use.
 */
static ShopWindow* shop_alloc_window(void)
{
    ShopWindow* window;
    s32 i;

    window = g_shop_windows;
    for (i = 0; i < SHOP_WINDOW_COUNT; i++, window++)
    {
        if (window->frame.bits.state == SHOP_WINDOW_FREE)
        {
            window->frame.bits.state = SHOP_WINDOW_OPENING;
            return window;
        }
    }
    return &g_shop_windows[SHOP_POPUP_WINDOW];
}
