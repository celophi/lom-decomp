#include "shop_render.h"
#include "encoded_text.h"
#include "sdk/libgpu.h"

/**
 * @brief Low byte of a window's 9-bit width, read from the whole frame word.
 */
#define SHOP_WINDOW_WIDTH_LOW_BYTE(window) ((s32)((window)->frame.word >> 24))
/** @brief Join a window's width high bit with an already-read low byte @p low. */
#define SHOP_WINDOW_JOIN_WIDTH(window, low) (((window)->extent.bits.width_high << 8) | (low))
/** @brief Full 9-bit width of @p window. */
#define SHOP_WINDOW_WIDTH(window) SHOP_WINDOW_JOIN_WIDTH(window, SHOP_WINDOW_WIDTH_LOW_BYTE(window))

/** @brief Visible height of the list window's row area. */
#define SHOP_LIST_VIEW_HEIGHT 116
#define SHOP_SCROLL_ARROW_X 286
#define SHOP_SCROLL_ARROW_DOWN_Y 172
#define SHOP_SCROLL_ARROW_UP_Y 56
#define SHOP_SCROLLBAR_WIDTH 6
#define SHOP_SCROLLBAR_FRAME_WIDTH 10

#define SHOP_TITLE_TEXT_X 56
/**
 * @brief X of the money window label.
 * @note JP moves it to 96.
 */
#if defined(VERSION_JP)
#define SHOP_MONEY_LABEL_X 96
#else
#define SHOP_MONEY_LABEL_X 80
#endif
#define SHOP_MONEY_VALUE_X 48

/** @brief Numeric stat positions in the detail window. */
#if defined(VERSION_JP)
#define SHOP_WEAPON_VALUE_X 96
#define SHOP_ARMOR_VALUE_X 96
#define SHOP_OTHER_VALUE_X 48
#else
#define SHOP_WEAPON_VALUE_X 112
#define SHOP_ARMOR_VALUE_X 116
#define SHOP_OTHER_VALUE_X 66
#endif

/** @brief Inventory record named by record entry id @p id, summed as integers, index first. */
#define SHOP_ENTRY_RECORD(id) ((FieldItemRecord*)(((id) & SHOP_ENTRY_RECORD_INDEX_MASK) * sizeof(FieldItemRecord) + (u32)g_shop_item_records))
/** @brief Inventory record of the list entry under the cursor. */
#define SHOP_SELECTED_RECORD() SHOP_ENTRY_RECORD(SHOP_SELECTED_ENTRY()->id)

/**
 * @brief Address of the string whose offset sits @p index bytes into the archive section @p offset bytes into @p archive.
 * @note Summed as integers, index first.
 */
#define SHOP_ARCHIVE_TEXT(archive, offset, index) ((u8*)((offset) + (*(u16*)((index) + (offset) + (u32)(archive)) + (u32)(archive))))

/** @brief First category name of each kind in text archive section 2. */
#define WEAPON_CATEGORY_FIRST 0
#define ARMOR_CATEGORY_FIRST 11
#define OTHER_CATEGORY_FIRST 23
/** @brief Strings per row of the two-dimensional detail table in text archive section 3. */
#define DETAIL_TEXT_ROW_LENGTH 14

/**
 * @brief Draw every active window and advance its open or close animation.
 * @param ctx Host frame context; its ordering-table entry receives every packet.
 * @param buffer Packet buffer for this frame.
 */
void shop_draw_windows(ShopFrameContext* ctx, ShopPacketBuffer* buffer)
{
    u8* prim;
    u32* ot;
    ShopWindow* window;
    s32 inset_width;
    s32 inset_height;
    s32 i;
    s32 opening_x;
    s32 opening_width_low;
    s32 closing_x;
    s32 closing_width_low;
    s32 open_width_low;
    DRAWENV draw_env;

    ot = &ctx->ot;
    prim = buffer->prim_cursor;

    if (ctx->display_buffer_index != 0)
    {
        SetDefDrawEnv(&draw_env, 0, 240, 320, 224);
    }
    else
    {
        SetDefDrawEnv(&draw_env, 0, 8, 320, 224);
    }

    window = g_shop_windows;
    for (i = 0; i < SHOP_WINDOW_COUNT; i++, window++)
    {
        if (window->frame.bits.state != SHOP_WINDOW_FREE)
        {
            if (window->draw == shop_draw_list_window)
            {
                TILE* bar;
                s32 list_x;
                s32 list_width_low;

                if (g_shop_entry_count * SHOP_ROW_HEIGHT > g_shop_scroll_y + SHOP_LIST_VIEW_HEIGHT)
                {
                    prim = func_800AE76C(prim, ot, SHOP_SCROLL_ARROW_X, SHOP_SCROLL_ARROW_DOWN_Y, 0);
                }
                if (g_shop_scroll_y != 0)
                {
                    prim = func_800AE76C(prim, ot, SHOP_SCROLL_ARROW_X, SHOP_SCROLL_ARROW_UP_Y, 1);
                }

                SetDrawEnv((DR_ENV*)prim, &draw_env);
                addPrim(ot, prim);
                prim += sizeof(DR_ENV);

                bar = (TILE*)prim;
                setlen(bar, 3);
                *(u32*)&bar->r0 = 0xFFFF00; /* yellow; the code byte is set next */
                setcode(bar, 0x60);
                bar->w = SHOP_SCROLLBAR_WIDTH;
                bar->h = (window->extent.bits.height * (window->extent.bits.height / SHOP_ROW_HEIGHT)) / g_shop_entry_count;
                if (bar->h >= window->extent.bits.height - 2)
                {
                    bar->h = window->extent.bits.height;
                }
                bar->x0 = 1;
                bar->y0 = ((window->extent.bits.height - 3) * (g_shop_scroll_y / SHOP_ROW_HEIGHT)) / g_shop_entry_count;
                addPrim(ot, bar);
                prim += sizeof(TILE);

                /* The scrollbar frame sits 3 pixels right of the list window. */
                list_x = window->frame.bits.x;
                list_width_low = SHOP_WINDOW_WIDTH_LOW_BYTE(window);
                prim = func_800AD850(prim, ot, list_x + SHOP_WINDOW_JOIN_WIDTH(window, list_width_low) + 3, window->frame.bits.y, SHOP_SCROLLBAR_FRAME_WIDTH,
                                     window->extent.bits.height, ctx->display_buffer_index, i == 0);
            }

            SetDrawEnv((DR_ENV*)prim, &draw_env);
            addPrim(ot, prim);
            prim += sizeof(DR_ENV);

            switch (window->frame.bits.state)
            {
            case SHOP_WINDOW_OPENING:
                inset_width = (SHOP_WINDOW_WIDTH(window) * window->frame.bits.anim_step) / SHOP_WINDOW_ANIM_STEPS;
                inset_height = (window->extent.bits.height * window->frame.bits.anim_step) / SHOP_WINDOW_ANIM_STEPS;
                prim = window->draw(ot, prim, (SHOP_WINDOW_WIDTH(window) - inset_width) / 2, (window->extent.bits.height - inset_height) / 2);
                opening_x = window->frame.bits.x;
                opening_width_low = SHOP_WINDOW_WIDTH_LOW_BYTE(window);
                prim = func_800AD850(prim, ot, opening_x + (SHOP_WINDOW_JOIN_WIDTH(window, opening_width_low) - inset_width) / 2,
                                     window->frame.bits.y + (window->extent.bits.height - inset_height) / 2, inset_width, inset_height,
                                     ctx->display_buffer_index, i == 0);
                window->frame.bits.anim_step++;
                if (window->frame.bits.anim_step == SHOP_WINDOW_ANIM_STEPS)
                {
                    window->frame.bits.state = SHOP_WINDOW_OPEN;
                    field_reset_input_repeat();
                }
                break;

            case SHOP_WINDOW_OPEN:
                prim = window->draw(ot, prim, 0, 0);
                open_width_low = SHOP_WINDOW_WIDTH_LOW_BYTE(window);
                prim = func_800AD850(prim, ot, window->frame.bits.x, window->frame.bits.y, SHOP_WINDOW_JOIN_WIDTH(window, open_width_low),
                                     window->extent.bits.height, ctx->display_buffer_index, i == 0);
                break;

            case SHOP_WINDOW_CLOSING:
                inset_width = (SHOP_WINDOW_WIDTH(window) * window->frame.bits.anim_step) / SHOP_WINDOW_ANIM_STEPS;
                inset_height = (window->extent.bits.height * window->frame.bits.anim_step) / SHOP_WINDOW_ANIM_STEPS;
                prim = window->draw(ot, prim, (SHOP_WINDOW_WIDTH(window) - inset_width) / 2, (window->extent.bits.height - inset_height) / 2);
                closing_x = window->frame.bits.x;
                closing_width_low = SHOP_WINDOW_WIDTH_LOW_BYTE(window);
                prim = func_800AD850(prim, ot, closing_x + (SHOP_WINDOW_JOIN_WIDTH(window, closing_width_low) - inset_width) / 2,
                                     window->frame.bits.y + (window->extent.bits.height - inset_height) / 2, inset_width, inset_height,
                                     ctx->display_buffer_index, i == 0);
                window->frame.bits.anim_step--;
                if (window->frame.bits.anim_step == 0)
                {
                    window->frame.bits.state = SHOP_WINDOW_FREE;
                }
                g_shop_notice_active = 0;
                break;
            }
        }
    }
    buffer->prim_cursor = prim;
}

/**
 * @brief Draw the money window: the money label and the player's money.
 * @param ot Ordering-table entry for the emitted packets.
 * @param prim Packet cursor.
 * @param x_inset Horizontal inset of the opening or closing window.
 * @param y_inset Vertical inset of the opening or closing window.
 * @return Advanced packet cursor.
 */
u8* shop_draw_money_window(u32* ot, u8* prim, s32 x_inset, s32 y_inset)
{
    Vec2s position;
    s32 y;

    y = 2 - y_inset;
    prim = func_800A88A0(prim, ot, FIELD_UI_TEXT_AT(D_800EC3C4, 0), SHOP_TEXT_COLOR_NORMAL, SHOP_MONEY_LABEL_X - x_inset, y, 0);
    position.x = SHOP_MONEY_VALUE_X - x_inset;
    position.y = y;
    if (g_saved_game_ctx->money > SHOP_MAX_MONEY)
    {
        prim = func_800A8A78(ot, prim, SHOP_MAX_MONEY, SHOP_TEXT_COLOR_NORMAL, &position, 2);
    }
    else
    {
        prim = func_800A8A78(ot, prim, g_saved_game_ctx->money, SHOP_TEXT_COLOR_NORMAL, &position, 2);
    }
    return prim;
}

u8* shop_draw_list_window(u32* ot, u8* prim, s32 x_inset, s32 y_inset)
{
    Vec2s position;
    Vec2s* position_ptr;
    s32 i;
    s32 y;
    ShopEntry* entry;
    u16 id;
    s32 price;
    s32 row_y;
    s32 row_top;
    s32 x;
    u8* empty_text;
    TILE* tile;

    i = 0;
    x = x_inset;
    if (g_shop_entry_count > 0)
    {
        s32 name_x;
        u8* text_base;

        empty_text = D_800EC3E0;
        do
        {
            ShopEntry** entries = &g_shop_entries;

            name_x = 80 - x;
            text_base = (u8*)&g_shop_text_archive;
            position_ptr = &position;
            ot++;
            ot--;
            row_y = i * SHOP_ROW_HEIGHT;
            row_top = y_inset - 2;
            row_top += i;
            row_top -= i;
            row_y -= row_top;
            y = row_y - g_shop_scroll_y;
            if ((u32)(y + 15) < 131)
            {
                do
                {
                    entry = (ShopEntry*)(i * sizeof(ShopEntry) + (u32)*entries);
                } while (0);
                id = entry->id;
                if (id == SHOP_ENTRY_EMPTY)
                {
                    s32 low;
                    s32 high;
                    s32 base_addr;
                    s32 based;

                    low = D_800EC3E0[0];
                    high = empty_text[1];
                    base_addr = (s32)D_800EC3C4;
                    based = (high << 8) + base_addr;
                    prim = func_800A88A0(prim, ot, (u8*)(low + based), SHOP_TEXT_COLOR_NORMAL, name_x, y, 2);
                }
                else
                {
                    if (id & SHOP_ENTRY_RECORD_FLAG)
                    {
                        prim = func_800A88A0(prim, ot, (u8*)&g_shop_item_records[id & SHOP_ENTRY_RECORD_INDEX_MASK], SHOP_TEXT_COLOR_NORMAL, name_x, y, 2);
                    }
                    else
                    {
                        do
                        {
                            s32 section = ((u32*)&g_shop_text_archive)[2];
                            s32 name_index = *(u16*)entry;
                            u16 name_offset;
                            u8* text;

                            name_index *= 2;
                            name_offset = *(u16*)((name_index + section) + (s32)text_base);
                            text = (u8*)(section + (name_offset + (s32)text_base));
                            prim = func_800A88A0(prim, ot, text, SHOP_TEXT_COLOR_NORMAL, name_x, y, 2);
                        } while (0);
                    }
                    position.x = 208 - x;
                    position.y = y;
                    if (i == g_shop_cursor)
                    {
                        price = SHOP_SELECTED_ENTRY()->price * g_shop_quantity;
                        if (g_saved_game_ctx->money >= (u32)price || g_shop_is_buying == 0)
                        {
                            prim = func_800A8A78(ot, prim, price, SHOP_TEXT_COLOR_NORMAL, position_ptr, 0);
                        }
                        else
                        {
                            prim = func_800A8A78(ot, prim, price, SHOP_TEXT_COLOR_DISABLED, position_ptr, 0);
                        }
                        prim = func_800A88A0(prim, ot, D_800EC3C4 + D_800EC3DC[0] + (D_800EC3DC[1] << 8), SHOP_TEXT_COLOR_NORMAL, 172 - x, y, 0);
                        position.x = 184 - x;
                        prim = func_800A8A78(ot, prim, g_shop_quantity, SHOP_TEXT_COLOR_NORMAL, position_ptr, 0);
                    }
                    else
                    {
                        ShopEntry* price_entry = (ShopEntry*)(i * sizeof(ShopEntry) + (u32)g_shop_entries);

                        if ((u32)price_entry->price <= g_saved_game_ctx->money || g_shop_is_buying == 0)
                        {
                            prim = func_800A8A78(ot, prim, price_entry->price, SHOP_TEXT_COLOR_NORMAL, position_ptr, 0);
                        }
                        else
                        {
                            prim = func_800A8A78(ot, prim, price_entry->price, SHOP_TEXT_COLOR_DISABLED, position_ptr, 0);
                        }
                    }
                }
            }
        } while (++i < g_shop_entry_count);
    }

    {
        s32 cursor = g_shop_cursor;
        s32 scroll = g_shop_scroll_y;
        s32 cursor_y = cursor * SHOP_ROW_HEIGHT;
        s32 cursor_top = y_inset - 2;

        y = (cursor_y - cursor_top) - scroll;
    }
    tile = (TILE*)prim;
    *(u32*)&tile->r0 = 0xF080F0;
    setlen(tile, 3);
    setcode(tile, 0x62);
    setXY0(tile, 0, y - 1);
    setWH(tile, 294, 14);
    addPrim(ot, tile);
    return prim + sizeof(TILE);
}

/**
 * @brief Draw the detail window for the list entry under the cursor.
 *
 * Record entries show "<material> <category>" as the title plus a kind-specific
 * stat row; plain items show their name and the owned count.
 *
 * @param ot Ordering table that receives the primitives.
 * @param prim Next free primitive in the packet buffer.
 * @param x_inset Horizontal offset subtracted from every x coordinate.
 * @param y_inset Vertical offset subtracted from every y coordinate.
 * @return Next free primitive after the window's contents.
 */
u8* shop_draw_detail_window(u32* ot, u8* prim, s32 x_inset, s32 y_inset)
{
    Vec2s position;
    u8 name_text[48];
    u8 difference_text[48];
    u8 number_text[48];
    ShopEntry* entry;
    u16 id;

    entry = SHOP_SELECTED_ENTRY();
    id = entry->id;
    if (id != SHOP_ENTRY_EMPTY)
    {
        if (id & SHOP_ENTRY_RECORD_FLAG)
        {
            u8* dst = name_text;
            u32* material_offsets = &g_shop_text_archive.section_offsets[1];
            ShopTextArchive* archive;
            FieldItemRecord* record = SHOP_ENTRY_RECORD(id);
            u8* ui_entry;
            u8* ui_text_table;
            u32 attributes;
            s32 low;
            s32 high;
            u32 material;

            material = record->info.halves[1] & FIELD_ITEM_MATERIAL_MASK;
            archive = &g_shop_text_archive;
            encoded_text_copy(dst, SHOP_ARCHIVE_TEXT(archive, *material_offsets, material * 2));
            /* FIELD UI string 15, read as a little-endian offset relative to the table start */
            ui_entry = D_800EC3E2;
            ui_text_table = ui_entry - 15 * 2;
            low = *ui_entry++;
            high = *ui_entry;
            encoded_text_append(name_text, (u8*)(low + ((high << 8) + (s32)ui_text_table)));

            attributes = SHOP_SELECTED_RECORD()->info.word;
            switch (FIELD_ITEM_CATEGORY(attributes))
            {
            case FIELD_ITEM_CATEGORY_WEAPON:
            {
                u32 category_offset = archive->section_offsets[2];
                s32 x;
                s32 y;
                s32 difference;

                encoded_text_append(name_text, SHOP_ARCHIVE_TEXT(archive, category_offset, (FIELD_ITEM_TYPE(attributes) + WEAPON_CATEGORY_FIRST) * 2));
                y = 18 - y_inset;
                prim = func_800A88A0(prim, ot, FIELD_UI_TEXT(ui_text_table, 21), SHOP_TEXT_COLOR_NORMAL, 16 - x_inset, y, 0);
                x = SHOP_WEAPON_VALUE_X - x_inset;
                position.x = x;
                position.y = y;
                prim = func_800A8A78(ot, prim, SHOP_SELECTED_RECORD()->derived.values[0], SHOP_TEXT_COLOR_NORMAL, &position, 1);
                difference = SHOP_SELECTED_RECORD()->derived.values[0] - g_saved_game_ctx->characters[FIELD_PARTY_HERO].equipment[0].derived.values[0];
                encoded_text_copy(difference_text, FIELD_UI_TEXT(ui_text_table, 31));
                if (difference >= 0)
                {
                    encoded_text_append(difference_text, FIELD_UI_TEXT(ui_text_table, 11));
                }
                func_800A8B90(number_text, difference, 0);
                encoded_text_append(difference_text, number_text);
                encoded_text_append(difference_text, FIELD_UI_TEXT(ui_text_table, 32));
#if defined(VERSION_JP)
                position.x = 112 - x_inset;
#else
                position.x = x;
#endif
                position.y = y;
                prim = func_800A88A0(prim, ot, difference_text, SHOP_TEXT_COLOR_NORMAL, position.x, position.y, 0);
                break;
            }
            case FIELD_ITEM_CATEGORY_ARMOR:
            {
                u32 category_offset = archive->section_offsets[2];
                s32 y;
                FieldItemRecord* selected;
                u16 offset;

                offset = *(u16*)((u8*)&g_shop_text_archive + (FIELD_ITEM_TYPE(attributes) * 2 + category_offset) + ARMOR_CATEGORY_FIRST * 2);
                encoded_text_append(name_text, (u8*)(category_offset + (offset + (u32)archive)));
                y = 18 - y_inset;
                prim = func_800A88A0(prim, ot, FIELD_UI_TEXT(ui_text_table, 22), SHOP_TEXT_COLOR_NORMAL, 16 - x_inset, y, 0);
                position.x = SHOP_ARMOR_VALUE_X - x_inset;
                position.y = y;
                selected = SHOP_SELECTED_RECORD();
                prim = func_800A8A78(ot, prim,
                                     selected->derived.values[0] + selected->derived.values[1] + selected->derived.values[2] + selected->derived.values[3],
                                     SHOP_TEXT_COLOR_NORMAL, &position, 0);
                break;
            }
            default:
            {
                u32* category_offsets;
                ShopTextArchive* text_archive;
                FieldItemRecord* selected;
                u8* text;
                u32 detail_offset;
                u16 offset;

                category_offsets = &g_shop_text_archive.section_offsets[2];
                offset = *(u16*)((u8*)&g_shop_text_archive + (FIELD_ITEM_TYPE(SHOP_SELECTED_RECORD()->info.word) * 2 + *category_offsets) +
                                 OTHER_CATEGORY_FIRST * 2);
                text_archive = &g_shop_text_archive;
                text = (u8*)(*category_offsets + (offset + (u32)text_archive));
                encoded_text_append(name_text, text);
                prim = func_800A88A0(prim, ot, FIELD_UI_TEXT_AT(D_800EC3F2, 23), SHOP_TEXT_COLOR_NORMAL, 16 - x_inset, 18 - y_inset, 0);
                position.x = SHOP_OTHER_VALUE_X - x_inset;
                position.y = 18 - y_inset;
                prim = func_800A8A78(ot, prim, SHOP_SELECTED_RECORD()->derived.bytes[2], SHOP_TEXT_COLOR_NORMAL, &position, 0);
                detail_offset = text_archive->section_offsets[3];
                selected = SHOP_SELECTED_RECORD();
                offset =
                    *(u16*)((selected->derived.bytes[1] * 2) + ((selected->derived.bytes[0] * DETAIL_TEXT_ROW_LENGTH * 2) + detail_offset) + (u32)text_archive);
                text = (u8*)(detail_offset + (offset + (u32)text_archive));
                prim = func_800A88A0(prim, ot, text, SHOP_TEXT_COLOR_NORMAL, 284 - x_inset, 18 - y_inset, 1);
                break;
            }
            }
            prim = func_800A88A0(prim, ot, name_text, SHOP_TEXT_COLOR_NORMAL, 150 - x_inset, 2 - y_inset, 2);
        }
        else
        {
            u32* item_offsets = &g_shop_text_archive.section_offsets[0];
            ShopTextArchive* archive = &g_shop_text_archive;

            prim = func_800A88A0(prim, ot, SHOP_ARCHIVE_TEXT(archive, *item_offsets, entry->id * 2), SHOP_TEXT_COLOR_NORMAL, 150 - x_inset, 2 - y_inset, 2);
            prim = func_800A88A0(prim, ot, FIELD_UI_TEXT_AT(D_800EC3F4, 24), SHOP_TEXT_COLOR_NORMAL, 16 - x_inset, 18 - y_inset, 0);
            position.x = 112 - x_inset;
            position.y = 18 - y_inset;
            prim = func_800A8A78(ot, prim, g_saved_game_ctx->item_counts[SHOP_SELECTED_ENTRY()->id], SHOP_TEXT_COLOR_NORMAL, &position, 0);
        }
    }
    return prim;
}

/**
 * @brief Draw the title window with the FIELD UI string chosen by shop_init.
 * @param ot Ordering-table entry for the emitted packets.
 * @param prim Packet cursor.
 * @param x_inset Horizontal inset of the opening or closing window.
 * @param y_inset Vertical inset of the opening or closing window.
 * @return Advanced packet cursor.
 */
u8* shop_draw_title_window(u32* ot, u8* prim, s32 x_inset, s32 y_inset)
{
    Vec2s unused_position;
    s32 entry;

    entry = g_shop_title_text_id * 2;
    return func_800A88A0(prim, ot, D_800EC3C4 + D_800EC3C4[entry] + (D_800EC3C4[entry + 1] << 8), SHOP_TEXT_COLOR_NORMAL, SHOP_TITLE_TEXT_X - x_inset,
                         2 - y_inset, 2);
}
