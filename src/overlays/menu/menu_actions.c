#include "menu_internal.h"

/*
 * GCC 2.8.0 inlined this helper into the action routines when MENU was one
 * translation unit. Keep an extern-inline view here while menu_screens.c emits
 * the standalone symbol at its original address.
 */
extern inline s32 menu_get_equipment_ability_mask(s32 excluded_slot)
{
    s32 equipment_index;
    s32 category_mask;
    MenuItemEntry* equipment;
    u32 item_attributes;

    category_mask = 0;
    equipment = (MenuItemEntry*)g_menu_equipment_base;
    equipment_index = 0;
    do
    {
        if ((equipment_index != excluded_slot) && (equipment->active != 0))
        {
            item_attributes = equipment->attributes.packed;
            if (item_attributes & MENU_ITEM_KIND_MASK)
            {
                category_mask |= D_800F0BEC[(item_attributes >> MENU_ITEM_CATEGORY_SHIFT) & MENU_ITEM_CATEGORY_MASK];
            }
            else
            {
                category_mask |= D_800F0BE0[(item_attributes >> MENU_ITEM_CATEGORY_SHIFT) & MENU_ITEM_CATEGORY_MASK];
            }
        }
        equipment_index += 1;
        equipment += 1;
    } while (equipment_index < MENU_EQUIPMENT_SLOT_COUNT);
    return category_mask;
}

/* ----- Equipment and Item Actions ----- */

/**
 * @brief Draw the character's equip/ability page and dispatch its confirm action.
 * @param ot Ordering-table pointer, forwarded to the glyph renderer.
 * @param state Scroll-list state for this page.
 * @param prim_buf Primitive buffer write cursor.
 * @param view_origin Viewport anchor; label origins are (0x30 - x, N - y).
 * @param active Non-zero to process input this frame; zero draws only.
 * @return Updated primitive buffer write cursor.
 */
s32 menu_equipment_action_callback(s32* ot, ScrollListState* state, s32 prim_buf, Vec2s* view_origin, int active)
{
    MenuSlotRect rect;
    ScrollListState* list;
    MenuContentItem* item;
    MenuContentItem* tbl;
    u8* pad_item;
    u8* cmp_tbl;
    u8* ctx;
    u8* equipment_ctx;
    s32 character_offset;
    u32 lhs_shift;
    u32 cmp_shift;
    s32 mask;
    s32 hit;
    s32 buf;
    s32 handle;
    s32 handle2;
    s32 off;
    s32 equipment_offset;
    s32 i9;
    list = state;
    buf = prim_buf;

    if ((g_pad_input & 0x40) && (active != 0))
    {
        menu_play_se(MENU_SE_CLOSE, MENU_SE_VOLUME);
        list->active = MENU_SLOT_STATE_CLOSING;
    }
    else if ((g_pad_input & 0x220) && (active != 0))
    {
        menu_play_se(MENU_SE_NAVIGATE, MENU_SE_VOLUME);
        switch (list->navigation.fields.selected_index)
        {
        case 0:
            switch (g_menu_active_subtype)
            {
            case 7:
            {
                s8* ability_mask_ptr;

                menu_clear_slots();
                g_menu_nodes[(g_menu_char_slot * 3) + 1].idx_nav.s.self_idx = 0x24;
                g_menu_nodes[(g_menu_char_slot * 3) + 1].label_id = D_8016869F[g_menu_nodes[(g_menu_char_slot * 3) + 1].idx_nav.s.self_idx];

                ability_mask_ptr = &g_menu_ability_mask;
                mask = menu_get_equipment_ability_mask(g_menu_active_subtype - MENU_EQUIPMENT_SUBTYPE_BASE);

                *ability_mask_ptr = mask;
                g_menu_content_ready = 1;
                menu_open_content_page(0);
                g_menu_draw_early_out = 1;
                return buf;
            }

            case 8:
            case 9:
            case 0xA:
            {
                s8* ability_mask_ptr;

                menu_clear_slots();
                g_menu_nodes[(g_menu_char_slot * 3) + 1].idx_nav.s.self_idx = 0x27;
                g_menu_nodes[(g_menu_char_slot * 3) + 1].label_id = D_8016869B[g_menu_nodes[(g_menu_char_slot * 3) + 1].idx_nav.s.self_idx];

                ability_mask_ptr = &g_menu_ability_mask;
                mask = menu_get_equipment_ability_mask(g_menu_active_subtype - MENU_EQUIPMENT_SUBTYPE_BASE);

                *ability_mask_ptr = mask;
                g_menu_content_ready = 1;
                menu_open_content_page(1);
                g_menu_draw_early_out = 1;
                return buf;
            }

            default:
                break;
            }
            break;

        case 1:
            if (g_menu_active_subtype == 7)
            {
                if (menu_stage_best_equipment_for_slot0() == 0)
                {
                    break;
                }
                menu_clear_slots();
                rect.x = 0xB0;
                rect.y = 0x40;
                rect.w = 0x70;
                rect.h = 0x30;
                list = (ScrollListState*)menu_slot_alloc(0, &rect);
                ((MenuSlot*)list)->content_cb = (s32 * (*)()) & menu_equipment_compare_callback;
                g_menu_compare_window_active = 1;
                ((MenuSlot*)list)->navigation.packed = (((MenuSlot*)list)->navigation.packed & 0xFE00FFFF) | 0x20000;

                menu_relink_pair();
                break;
            }
            g_item_slot_data[3] = 0;
            g_item_slot_data[2] = 0;
            g_item_slot_data[1] = 0;
            if (menu_stage_best_equipment_for_active_slot() == 0)
            {
                break;
            }
            menu_clear_slots();
            rect.x = 0xB0;
            rect.y = 0x40;
            rect.w = 0x70;
            rect.h = 0x30;
            list = (ScrollListState*)menu_slot_alloc(0, &rect);
            ((MenuSlot*)list)->content_cb = (s32 * (*)()) & menu_equipment_compare_callback;
            g_menu_compare_window_active = 1;
            ((MenuSlot*)list)->navigation.packed = (((MenuSlot*)list)->navigation.packed & 0xFE00FFFF) | 0x20000;

            menu_relink_pair();
            break;

        case 2:
            g_item_slot_data[3] = 0;
            g_item_slot_data[2] = 0;
            g_item_slot_data[1] = 0;
            menu_stage_best_equipment_for_slot0();
            g_menu_active_subtype = 8;
            menu_stage_best_equipment_for_active_slot();
            g_menu_active_subtype = 9;
            menu_stage_best_equipment_for_active_slot();
            g_menu_active_subtype = 0xA;
            menu_stage_best_equipment_for_active_slot();

            menu_clear_slots();
            rect.x = 0xB0;
            rect.y = 0x40;
            rect.w = 0x70;
            rect.h = 0x30;
            list = (ScrollListState*)menu_slot_alloc(0, &rect);
            ((MenuSlot*)list)->content_cb = (s32 * (*)()) & menu_equipment_compare_callback;
            g_menu_compare_window_active = 1;
            ((MenuSlot*)list)->navigation.packed = (((MenuSlot*)list)->navigation.packed & 0xFE00FFFF) | 0x20000;

            menu_relink_pair();
            break;

        case 3:
            if (*(u8*)g_menu_active_equipped_item == 0)
            {
                break;
            }
            menu_clear_slots();
            if (g_menu_active_subtype == 7)
            {
                g_menu_item_ptr = g_menu_active_equipped_item;
                g_menu_category0_item = g_menu_saved_category0_item;
                g_menu_nodes[(g_menu_char_slot * 3) + 1].idx_nav.s.self_idx = 0x14;
                g_menu_nodes[(g_menu_char_slot * 3) + 1].label_id = 0x12;
            }
            else if (g_menu_active_subtype >= 7)
            {
                if (g_menu_active_subtype < 0xB)
                {
                    g_menu_item_ptr = g_menu_active_equipped_item;
                    g_menu_category1_item = g_menu_saved_category1_item;
                    g_menu_nodes[(g_menu_char_slot * 3) + 1].idx_nav.s.self_idx = 0x17;
                    g_menu_nodes[(g_menu_char_slot * 3) + 1].label_id = 0x15;
                }
            }
            hit = menu_find_active_content_item();
            g_menu_hit_item_idx = hit;
            if (hit != -1)
            {
                tbl = g_menu_content_table[g_menu_nodes[g_menu_scene_type].idx_nav.s.self_idx];
                item = (MenuContentItem*)((hit * 8) + (s32)tbl);
                g_content_view_x = item->packed_x & 0x1FF;
                g_content_view_y = item->y - 8;
                g_menu_suppress_cursor = 5;
                g_menu_cursor_enable = 1;
            }
            g_menu_draw_early_out = 1;
            return buf;

        case 4:
            if (*(u8*)g_menu_active_equipped_item == 0)
            {
                break;
            }
            handle = func_800A9060();
            if (handle != 0)
            {
                if (menu_item_is_nondefault((const MenuItemEntry*)(((u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0)) + ((g_menu_active_subtype << 6) - 0x170))) != 0)
                {
                    func_800A8F8C(handle, (s32)(((u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0)) + ((g_menu_active_subtype << 6) - 0x170)));
                    switch (g_menu_active_subtype)
                    {
                    case 7:
                        g_menu_active_equipped_item = 0;
                        g_menu_saved_category0_item = 0;
                        character_offset = g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE;
                        ctx = (u8*)g_pad_ctx;
                        lhs_shift = ((MenuCharacterRecord*)(ctx + character_offset + MENU_CHARACTER_RECORD_OFFSET))->items[0].attributes.packed >> 10;
                        cmp_shift = ((MenuItemEntry*)(cmp_tbl = D_800F0BF8))->attributes.packed >> 10;
                        if ((lhs_shift & 0x3F) == (cmp_shift & 0x3F))
                        {
                            func_800A8F8C(&menu_character_record((PadContext*)ctx, g_menu_char_slot)->items[0], cmp_tbl);
                            break;
                        }
                        func_800A8F8C(&menu_character_record((PadContext*)ctx, g_menu_char_slot)->items[0], cmp_tbl);

                        i9 = 1;
                        do
                        {
                            off = i9 << 6;
                            pad_item = (u8*)g_pad_ctx + (off + (g_menu_char_slot * 0x250));
                            if (pad_item[0x640] != 0)
                            {
                                if ((((*(u32*)(pad_item + 0x654)) >> 10) & 0x3F) == 0)
                                {
                                    handle2 = func_800A9060();
                                    if (handle2 != 0)
                                    {
                                        func_800A8F8C(handle2, (s32)(((u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0)) + (off + 0x50)));
                                        menu_equipped_item(g_pad_ctx, i9)->active = 0;
                                        func_800A8FB4();
                                        break;
                                    }
                                    func_800A8F8C(((g_menu_char_slot * 0x250) + (s32)g_pad_ctx) + 0x640, handle);
                                    *(u8*)handle = 0;
                                    g_menu_message_line1 = (void*)menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), 86);
                                    menu_clear_slots();
                                    menu_open_content_page(6);
                                    return buf;
                                }
                            }
                            i9 += 1;
                        } while (i9 < 4);

                        if (menu_clear_pending_status() == 0)
                        {
                            break;
                        }
                        g_menu_message_line1 = (void*)menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), 29);
                        menu_clear_slots();
                        menu_open_content_page(6);
                        return buf;

                    case 8:
                    case 9:
                    case 0xA:
                        g_menu_active_equipped_item = 0;
                        g_menu_saved_category1_item = 0;
                        {
                            s32 equipment_subtype;
                            equipment_subtype = g_menu_active_subtype;
                            equipment_ctx = (u8*)g_pad_ctx;
                            equipment_offset = ((equipment_subtype - MENU_EQUIPMENT_SUBTYPE_BASE) << 6) + (g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE);
                            *(equipment_ctx + equipment_offset + 0x640) = 0;
                            func_800A8FB4();
                        }
                        break;
                    }
                }
                list->active = MENU_SLOT_STATE_CLOSING;
                break;
            }
            g_menu_message_line1 = (void*)menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), 86);
            menu_clear_slots();
            menu_open_content_page(6);
            return buf;
        }
    }

    buf = scroll_list_draw(buf, ot, list, g_menu_item_nav_entries, view_origin, active);

    buf = func_800A88A0(buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), 63), 1, 0x30 - view_origin->x, -view_origin->y, 2);
    buf = func_800A88A0(buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), 65), 1, 0x30 - view_origin->x, 0x10 - view_origin->y, 2);
    buf = func_800A88A0(buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), 66), 1, 0x30 - view_origin->x, 0x20 - view_origin->y, 2);
    buf = func_800A88A0(buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), 64), 1, 0x30 - view_origin->x, 0x30 - view_origin->y, 2);
    buf = func_800A88A0(buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), 67), 1, 0x30 - view_origin->x, 0x40 - view_origin->y, 2);
    return buf;
}

/**
 * @brief Content callback for the item-compare window opened by @ref menu_equipment_action_callback.
 * @param ot Ordering-table pointer.
 * @param state Scroll-list state for this window.
 * @param prim_buf Primitive buffer write cursor.
 * @param view_origin Viewport anchor in list-local coordinates.
 * @param active Non-zero when this window owns input.
 * @return Updated primitive write cursor; the unchanged @p prim_buf on the close path.
 */
s32 menu_equipment_compare_callback(s32* ot, ScrollListState* state, s32 prim_buf, Vec2s* view_origin, int active)
{
    u16 rect[4];
    ScrollListState* list;
    s32 i;
    s32 item;
    s32 slot_off;
    s32 buf;

    list = state;
    buf = prim_buf;
    if ((g_pad_input & 0x260) && (active != 0))
    {
        g_menu_compare_window_active = 0;

        if ((list->navigation.fields.selected_index != 0) || (g_pad_input & 0x40))
        {
            menu_play_se(MENU_SE_CLOSE, MENU_SE_VOLUME);
            list->active = MENU_SLOT_STATE_CLOSING;

            i = 0;
            do
            {
                if (g_item_slot_flags[i] != 0)
                {
                    item = (s32)g_item_slot_data[i];
                    if (item != 0)
                    {
                        menu_swap_item_records((MenuItemEntry*)item, (MenuItemEntry*)(((u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0)) + ((i << 6) + 0x50)));
                    }
                    else if (i == 0)
                    {
                        func_800A8F8C(func_800A9060(), (s32)((g_menu_char_slot * 0x250) + (s32)g_pad_ctx + 0x640));
                        func_800A8F8C((s32)((g_menu_char_slot * 0x250) + (s32)g_pad_ctx + 0x640), D_800F0BF8);
                    }
                    else
                    {
                        func_800A8F8C(func_800A9060(), (s32)(((u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0)) + ((i << 6) + 0x50)));
                        slot_off = g_menu_char_slot * 0x250;
                        *((u8*)g_pad_ctx + ((i << 6) + slot_off) + 0x640) = 0;
                    }
                }
                i += 1;
            } while (i < MENU_EQUIPMENT_SLOT_COUNT);
        }
        else
        {
            menu_play_se(MENU_SE_SELECT, MENU_SE_VOLUME);
            list->active = MENU_SLOT_STATE_CLOSING;

            if (g_item_slot_flags[0] != 0)
            {
                if (((g_item_slot_data[0] != 0) &&
                     ((((MenuItemEntry*)g_item_slot_data[0])->attributes.packed & 0xFC00) != (((MenuItemEntry*)g_menu_equipment_base)->attributes.packed & 0xFC00))) ||
                    ((g_item_slot_data[0] == 0) &&
                     ((D_800F0C0C & 0xFC00) != (((MenuItemEntry*)g_menu_equipment_base)->attributes.packed & 0xFC00))))
                {
                    if (menu_clear_pending_status() != 0)
                    {
                        s32 n = MENU_SLOT_COUNT - 1;

                        g_menu_message_line1 = (void*)menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), 29);
                        do
                        {
                            g_menu_slots[n].active = 0;
                            n -= 1;
                        } while (n >= 0);
                        menu_open_content_page(MENU_REDRAW_NAVIGATE);
                    }
                }
            }
        }

        func_800A8FB4();
        g_item_slot_flags[0] = 0;
        g_item_slot_flags[1] = 0;
        g_item_slot_flags[2] = 0;
        g_item_slot_flags[3] = 0;
        return buf;
    }

    buf = scroll_list_draw(buf, ot, list, g_menu_item_nav_entries, view_origin, active);
    buf = func_800A88A0(buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), 72), 1, 0x30 - view_origin->x, -view_origin->y, 2);
    buf = func_800A88A0(buf, ot, menu_text_entry(menu_text_table_base(MENU_TEXT_MESSAGES), 73), 1, 0x30 - view_origin->x, 0x10 - view_origin->y, 2);
    g_pad_input = 0;
    return buf;
}

/**
 * @brief Test whether an item record differs from the default/empty compare entry.
 * @param item_record Item record to compare with the default entry.
 * @return 1 as soon as a byte differs; 0 if all 0x40 bytes are equal.
 */
inline s32 menu_item_is_nondefault(const MenuItemEntry* item_record)
{
    const u8* item;
    const u8* default_item;
    u32 i;

    item = (const u8*)item_record;
    default_item = D_800F0BF8;

    for (i = 0; i < MENU_ITEM_RECORD_SIZE; i++, default_item++, item++)
    {
        if (*default_item != *item)
        {
            return 1;
        }
    }

    return 0;
}

/**
 * @brief Swap two 0x40-byte item records through a stack buffer.
 * @param first_item First item record.
 * @param second_item Second item record.
 */
inline void menu_swap_item_records(MenuItemEntry* first_item, MenuItemEntry* second_item)
{
    u8 swap_buffer[MENU_ITEM_RECORD_SIZE];

    func_800A8F8C(swap_buffer, first_item);
    func_800A8F8C(first_item, second_item);
    func_800A8F8C(second_item, swap_buffer);
}

/**
 * @brief Point @ref g_active_slot at the highest-numbered menu slot still in use.
 */
void menu_update_active_slot(void)
{
    s32 i;

    for (i = 0; i < MENU_SLOT_COUNT; i++)
    {
        if (g_menu_slots[i].active != 0)
        {
            g_active_slot = i;
        }
    }
}


/**
 * @brief Draw the learned Special Technique list and handle technique assignment.
 * @param ot Ordering-table pointer, forwarded to the glyph renderer.
 * @param state_arg Scroll-list state for this page.
 * @param prim_arg Primitive buffer write cursor.
 * @param view_origin Viewport anchor; glyph origins are (0x10 - x, rel - y).
 * @param active Non-zero to process input this frame; zero draws only.
 * @return Updated primitive buffer write cursor.
 */
s32 menu_special_technique_list_callback(s32* ot, ScrollListState* state_arg, s32 prim_arg, Vec2s* view_origin, s32 active)
{
    ScrollListState* state = state_arg;
    s32 prim = prim_arg;
    s32 selected_technique;
    s32 list_y;
    s32 group_index;
    s32 technique_index;
    s32 bit_mask;
    s32 group_flags;
    s32* group_flags_ptr;
    s32 relative_y;
    s32 scroll_y;
    u8 assignment_flag;
    s32 item_handle;

    if ((g_pad_input & 0x40) && (active != 0))
    {
        menu_play_se(MENU_SE_CLOSE, MENU_SE_VOLUME);
        state->active = MENU_SLOT_STATE_CLOSING;
        return prim;
    }

    selected_technique = -1;
    list_y = 0;
    group_index = 0;
    group_flags_ptr = (s32*)((u8*)g_pad_ctx + MENU_SPECIAL_TECHNIQUE_FLAGS_OFFSET);
    do
    {
        technique_index = 0;
        bit_mask = 1;
        group_flags = *group_flags_ptr;
        do
        {
            if (group_flags & bit_mask)
            {
                if (state->navigation.fields.selected_index == (list_y >> 4))
                {
                    selected_technique = technique_index + (group_index * MENU_SPECIAL_TECHNIQUES_PER_GROUP);
                }
                list_y += 0x10;
            }
            technique_index += 1;
            bit_mask <<= 1;
        } while (technique_index < MENU_SPECIAL_TECHNIQUES_PER_GROUP);
        group_index += 1;
        group_flags_ptr += 1;
    } while (group_index < MENU_SPECIAL_TECHNIQUE_GROUP_COUNT);

    if ((g_pad_input & 0x220) && (active != 0))
    {
        if ((selected_technique / MENU_SPECIAL_TECHNIQUES_PER_GROUP) ==
            (s32)(((u32)(((MenuItemEntry*)g_menu_equipment_base)->attributes.packed) >> MENU_ITEM_CATEGORY_SHIFT) & MENU_ITEM_CATEGORY_MASK))
        {
            u8* assignment_ptr = (u8*)g_pad_ctx + (g_menu_char_slot * 0x250);
            u8* character_ctx;

            assignment_ptr += g_menu_active_subtype;
            assignment_flag = *(assignment_ptr + 0x609);
            if (assignment_flag != MENU_NONE)
            {
                if (assignment_flag & 0x80)
                {
                    item_handle = func_800A9060();
                    if (item_handle != 0)
                    {
                        s32 slot_offset;

                        func_800A8F8C(item_handle, (u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0) + ((g_menu_active_subtype << 6) + 0x90));
                        slot_offset = ((g_menu_active_subtype + 1) << 6) + (g_menu_char_slot * 0x250);
                        *((u8*)g_pad_ctx + slot_offset + 0x640) = 0;
                        func_800A8FB4();
                    }
                    else
                    {
                        s32 slot_index = MENU_SLOT_COUNT - 1;
                        u8* message_table;

                        message_table = g_menu_state_ptr + ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_MESSAGES];
                        g_menu_message_line1 = message_table + *(u16*)(message_table + 0xAC);
                        do
                        {
                            g_menu_slots[slot_index].active = 0;
                            slot_index -= 1;
                        } while (slot_index >= 0);
                        menu_open_content_page(6);
                        return prim;
                    }
                }
            }
            character_ctx = (u8*)g_pad_ctx + (g_menu_char_slot * 0x250);
            character_ctx += g_menu_active_subtype;
            *(character_ctx + 0x609) = selected_technique % MENU_SPECIAL_TECHNIQUES_PER_GROUP;
            menu_play_se(MENU_SE_SELECT, MENU_SE_VOLUME);
            state->active = MENU_SLOT_STATE_CLOSING;
        }
        else
        {
            menu_play_se(MENU_SE_ERROR, MENU_SE_VOLUME);
        }
    }

    prim = scroll_list_draw(prim, ot, state, (u32*)&g_menu_scroll_nav_entries, view_origin, active);

    selected_technique = -1;
    list_y = 0;
    group_index = 0;
    group_flags_ptr = (s32*)((u8*)g_pad_ctx + MENU_SPECIAL_TECHNIQUE_FLAGS_OFFSET);
    scroll_y = state->scroll_y;
    do
    {
        technique_index = 0;
        bit_mask = 1;
        do
        {
            if (*group_flags_ptr & bit_mask)
            {
                relative_y = list_y - scroll_y;
                if (relative_y >= -0xF)
                {
                    if (relative_y < state->viewport_h - 0x10)
                    {
                        s32 glyph_color;
                        u8* name_table;
                        s32 glyph_offset;
                        u8* glyph;

                        glyph_color = 3;
                        name_table = g_menu_state_ptr;
                        name_table += ((MenuTextResources*)name_table)->table_offsets[MENU_TEXT_TECHNIQUE_NAMES];
                        glyph_offset = *(u16*)((u8*)name_table + (technique_index * 2) + (group_index * (MENU_SPECIAL_TECHNIQUES_PER_GROUP * 2)));
                        glyph = name_table + glyph_offset;
                        if (group_index ==
                            (s32)(((u32)(((MenuItemEntry*)g_menu_equipment_base)->attributes.packed) >> MENU_ITEM_CATEGORY_SHIFT) & MENU_ITEM_CATEGORY_MASK))
                        {
                            glyph_color = 1;
                        }
                        prim = func_800A88A0(prim, ot, glyph, glyph_color, 0x10 - view_origin->x, relative_y - view_origin->y, 0);
                    }
                }
                if (state->navigation.fields.selected_index == (list_y >> 4))
                {
                    selected_technique = technique_index + (group_index * MENU_SPECIAL_TECHNIQUES_PER_GROUP);
                }
                list_y += 0x10;
            }
            technique_index += 1;
            bit_mask <<= 1;
        } while (technique_index < MENU_SPECIAL_TECHNIQUES_PER_GROUP);
        group_index += 1;
        group_flags_ptr += 1;
    } while (group_index < MENU_SPECIAL_TECHNIQUE_GROUP_COUNT);

    if (selected_technique != -1)
    {
        u8* name_table = g_menu_state_ptr + ((MenuTextResources*)g_menu_state_ptr)->table_offsets[MENU_TEXT_TECHNIQUE_HELP];

        g_menu_help_text = (s32)(name_table + ((u16*)name_table)[selected_technique]);
    }
    return prim;
}

/* ----- Content Page Setup and Navigation Tables ----- */

/**
 * @brief Open the menu content window for the given content page id.
 * @param content_id Content page id (0-7); out of range is a no-op beyond the g_menu_pending_item_row reset.
 */
void menu_open_content_page(u32 content_id)
{
    MenuSlotRect rect;
    MenuSlot* slot;
    s32 v0;
    s32 j;
    s32 prev;
    s32 next;
    s32 more;
    s32 link;
    s32 word_self;
    s32 word_prev;

    g_menu_pending_item_row = 0xFF;
    switch (content_id)
    {
    case 0:
        rect.x = 0x40;
        rect.y = 0x60;
        rect.w = 0xF0;
        rect.h = 0x60;
        slot = menu_slot_alloc(3, &rect);
        slot->content_cb = (s32 * (*)()) & menu_inventory_list_callback;
        slot->has_title = 1;
        slot->anim_frame = 5;
        slot->active = 2;
        v0 = menu_build_inventory_nav_entries(0);
        g_menu_active_item_category = 0;
        slot->navigation.packed = (slot->navigation.packed & 0xFE00FFFF) | ((v0 & 0x1FF) << 16);
        break;

    case 1:
        rect.x = 0x40;
        rect.y = 0x60;
        rect.w = 0xF0;
        rect.h = 0x60;
        slot = menu_slot_alloc(3, &rect);
        slot->content_cb = (s32 * (*)()) & menu_inventory_list_callback;
        slot->has_title = 1;
        slot->anim_frame = 5;
        slot->active = 2;
        v0 = menu_build_inventory_nav_entries(1);
        g_menu_active_item_category = 1;
        slot->navigation.packed = (slot->navigation.packed & 0xFE00FFFF) | ((v0 & 0x1FF) << 16);
        break;

    case 2:
        rect.x = 0x40;
        rect.y = 0x60;
        rect.w = 0xF0;
        rect.h = 0x60;
        slot = menu_slot_alloc(3, &rect);
        slot->content_cb = (s32 * (*)()) & menu_inventory_list_callback;
        slot->has_title = 1;
        slot->anim_frame = 5;
        slot->active = 2;
        v0 = menu_build_inventory_nav_entries(2);
        g_menu_active_item_category = 2;
        slot->navigation.packed = (slot->navigation.packed & 0xFE00FFFF) | ((v0 & 0x1FF) << 16);
        break;

    case 3:
        rect.x = 0x40;
        rect.y = 0x2C;
        rect.w = 0xE8;
        rect.h = 0x90;
        slot = menu_slot_alloc(3, &rect);
        slot->content_cb = (s32 * (*)()) & menu_equipment_grid_callback;
        slot->has_title = 1;
        slot->anim_frame = 5;
        slot->active = 2;
        v0 = menu_build_equipment_nav_entries();
        g_menu_active_item_category = 3;
        slot->navigation.packed = (slot->navigation.packed & 0xFE00FFFF) | ((v0 & 0x1FF) << 16);
        break;

    case 4:
        rect.x = 0x40;
        rect.y = 0x2C;
        rect.w = 0xE8;
        rect.h = 0x80;
        slot = menu_slot_alloc(3, &rect);
        slot->content_cb = (s32 * (*)()) & menu_key_item_list_callback;
        slot->has_title = 1;
        slot->anim_frame = 5;
        slot->active = 2;
        v0 = menu_build_key_item_nav_entries();
        g_menu_active_item_category = 3;
        slot->navigation.packed = (slot->navigation.packed & 0xFE00FFFF) | ((v0 & 0x1FF) << 16);
        break;

    case 5:
        rect.x = 0x40;
        rect.y = 0x2C;
        rect.w = 0xE8;
        rect.h = 0x90;
        slot = menu_slot_alloc(3, &rect);
        slot->content_cb = (s32 * (*)()) & menu_ability_list_callback;
        slot->has_title = 1;
        slot->anim_frame = 5;
        slot->active = 2;
        v0 = menu_build_ability_nav_entries();
        g_menu_active_item_category = 3;
        slot->navigation.packed = (slot->navigation.packed & 0xFE00FFFF) | ((v0 & 0x1FF) << 16);
        break;

    case 6:
        rect.x = 0x10;
        rect.y = 0x60;
        rect.w = 0x120;
        rect.h = 0x20;
        slot = menu_slot_alloc(3, &rect);
        slot->content_cb = (s32 * (*)()) & menu_message_callback;
        slot->anim_frame = 5;
        slot->active = 2;
        slot->navigation.packed = (slot->navigation.packed & 0xFE00FFFF) | 0x10000;

        j = 0;
        do
        {
            s32 cur = g_menu_item_nav_entries[j];

            prev = 0;
            link = cur & ~0x3FFF;
            link = link | ((j * 0x10) & 0x3FFF);
            word_self = link;
            g_menu_item_nav_entries[j] = word_self;
            if ((j - 1) >= 0)
            {
                prev = j - 1;
            }
            word_prev = word_self & 0xFF803FFF;
            word_prev = word_prev | ((prev & 0x1FF) << 14);
            g_menu_item_nav_entries[j] = word_prev;
            next = j + 1;
            more = next < 1;
            link = 0;
            if (more != 0)
            {
                link = next;
            }
            g_menu_item_nav_entries[j] = (word_prev & 0x7FFFFF) | (link << 23);
            j = next;
        } while (more != 0);
        g_menu_draw_early_out = 1;
        break;

    case 7:
        rect.x = 0x10;
        rect.y = 0x60;
        rect.w = 0x120;
        rect.h = 0x30;
        slot = menu_slot_alloc(3, &rect);
        slot->content_cb = (s32 * (*)()) & menu_two_line_message_callback;
        slot->anim_frame = 5;
        slot->active = 2;
        slot->navigation.packed = (slot->navigation.packed & 0xFE00FFFF) | 0x10000;

        j = 0;
        do
        {
            s32 cur = g_menu_item_nav_entries[j];

            prev = 0;
            link = cur & ~0x3FFF;
            link = link | ((j * 0x10) & 0x3FFF);
            word_self = link;
            g_menu_item_nav_entries[j] = word_self;
            if ((j - 1) >= 0)
            {
                prev = j - 1;
            }
            word_prev = word_self & 0xFF803FFF;
            word_prev = word_prev | ((prev & 0x1FF) << 14);
            g_menu_item_nav_entries[j] = word_prev;
            next = j + 1;
            more = next < 1;
            link = 0;
            if (more != 0)
            {
                link = next;
            }
            g_menu_item_nav_entries[j] = (word_prev & 0x7FFFFF) | (link << 23);
            j = next;
        } while (more != 0);
        g_menu_draw_early_out = 1;
        break;
    }
}

/**
 * @brief Build circular navigation for usable equipment-grid entries.
 * @return Number of usable entries.
 */
s32 menu_build_equipment_nav_entries(void)
{
    s32 next_index;
    s32 entry_with_position;
    s32 previous_index;
    s32 wrapped_next_index;
    s32* entry;

    s32 packed_entry;
    s32 entry_with_previous;

    s32 item_count;
    s32 row;
    s32 column;
    u32 row_entries;
    s32 entry_mask;

    item_count = 0;
    row = 0;
    entry_mask = MENU_EQUIPMENT_GRID_ENTRY_MASK;
    do
    {
        row_entries = *(u32*)((u8*)g_pad_ctx + (row * 4) + MENU_EQUIPMENT_GRID_OFFSET);
        column = MENU_EQUIPMENT_GRID_COLUMN_COUNT - 1;
        do
        {
            if ((row_entries & entry_mask) >= MENU_EQUIPMENT_GRID_FIRST_VALID)
            {
                item_count += 1;
            }
            column -= 1;
            row_entries = row_entries >> 4;
        } while (column >= 0);
        row += 1;
    } while (row < MENU_EQUIPMENT_GRID_ROW_COUNT);

    g_menu_scroll_nav_entries[0] = 0;
    for (column = 0; column < item_count; column++)
    {
        entry = (s32*)g_menu_scroll_nav_entries + column;

        packed_entry = *entry;
        previous_index = column - 1;
        entry_with_position = packed_entry & ~MENU_ITEM_NAV_POSITION_MASK;
        entry_with_position = entry_with_position | ((column * MENU_ITEM_NAV_POSITION_STRIDE) & MENU_ITEM_NAV_POSITION_MASK);
        *entry = entry_with_position;

        if (previous_index < 0)
        {
            previous_index = item_count - 1;
        }
        entry_with_previous = entry_with_position & MENU_ITEM_NAV_PREVIOUS_CLEAR_MASK;
        entry_with_previous = entry_with_previous | ((previous_index & MENU_ITEM_NAV_INDEX_MASK) << MENU_ITEM_NAV_PREVIOUS_SHIFT);
        *entry = entry_with_previous;
        next_index = column + 1;
        wrapped_next_index = 0;
        if (next_index < item_count)
        {
            wrapped_next_index = next_index;
        }
        *entry = (entry_with_previous & MENU_ITEM_NAV_NEXT_CLEAR_MASK) | (wrapped_next_index << MENU_ITEM_NAV_NEXT_SHIFT);
    }
    return item_count;
}

/**
 * @brief Clear the key-item sentinel and build navigation for items with nonzero quantities.
 * @return Number of available key items.
 */
s32 menu_build_key_item_nav_entries(void)
{
    s32 next_index;
    s32 entry_with_position;
    s32 previous_index;
    s32 wrapped_next_index;
    s32* entry;

    s32 packed_entry;
    s32 entry_with_previous;

    s32 item_count;
    s32 remaining_entries;
    s32 entry_index;
    u8* quantity;

    item_count = 0;
    ((u8*)g_pad_ctx)[MENU_KEY_ITEM_TABLE_OFFSET + MENU_KEY_ITEM_SENTINEL_INDEX] = 0;
    quantity = (u8*)g_pad_ctx + MENU_KEY_ITEM_TABLE_OFFSET;
    remaining_entries = MENU_KEY_ITEM_TABLE_COUNT - 1;
    do
    {
        if (*quantity != 0)
        {
            item_count += 1;
        }
        remaining_entries -= 1;
        quantity += 1;
    } while (remaining_entries >= 0);

    g_menu_scroll_nav_entries[0] = 0;
    for (entry_index = 0; entry_index < item_count; entry_index++)
    {
        entry = (s32*)g_menu_scroll_nav_entries + entry_index;

        packed_entry = *entry;
        previous_index = entry_index - 1;
        entry_with_position = packed_entry & ~MENU_ITEM_NAV_POSITION_MASK;
        entry_with_position = entry_with_position | ((entry_index * MENU_ITEM_NAV_POSITION_STRIDE) & MENU_ITEM_NAV_POSITION_MASK);
        *entry = entry_with_position;

        if (previous_index < 0)
        {
            previous_index = item_count - 1;
        }
        entry_with_previous = entry_with_position & MENU_ITEM_NAV_PREVIOUS_CLEAR_MASK;
        entry_with_previous = entry_with_previous | ((previous_index & MENU_ITEM_NAV_INDEX_MASK) << MENU_ITEM_NAV_PREVIOUS_SHIFT);
        *entry = entry_with_previous;
        next_index = entry_index + 1;
        wrapped_next_index = 0;
        if (next_index < item_count)
        {
            wrapped_next_index = next_index;
        }
        *entry = (entry_with_previous & MENU_ITEM_NAV_NEXT_CLEAR_MASK) | (wrapped_next_index << MENU_ITEM_NAV_NEXT_SHIFT);
    }
    return item_count;
}

/**
 * @brief Build circular navigation for learned abilities.
 * @return Number of learned abilities.
 */
s32 menu_build_ability_nav_entries(void)
{
    s32 next_index;
    s32 entry_with_position;
    s32 previous_index;
    s32 wrapped_next_index;
    s32* entry;

    s32 packed_entry;
    s32 entry_with_previous;

    s32 item_count;
    s32 remaining;
    s32 entry_index;
    MenuAbilityEntry* ability;

    item_count = 0;
    ability = (MenuAbilityEntry*)((u8*)g_pad_ctx + MENU_ABILITY_TABLE_OFFSET);
    remaining = MENU_ABILITY_COUNT - 1;
    do
    {
        if (ability->flags & MENU_ABILITY_FLAG_LEARNED)
        {
            item_count += 1;
        }
        remaining -= 1;
        ability += 1;
    } while (remaining >= 0);

    g_menu_scroll_nav_entries[0] = 0;
    for (entry_index = 0; entry_index < item_count; entry_index++)
    {
        entry = (s32*)g_menu_scroll_nav_entries + entry_index;

        packed_entry = *entry;
        previous_index = entry_index - 1;
        entry_with_position = packed_entry & ~MENU_ITEM_NAV_POSITION_MASK;
        entry_with_position = entry_with_position | ((entry_index * MENU_ITEM_NAV_POSITION_STRIDE) & MENU_ITEM_NAV_POSITION_MASK);
        *entry = entry_with_position;

        if (previous_index < 0)
        {
            previous_index = item_count - 1;
        }
        entry_with_previous = entry_with_position & MENU_ITEM_NAV_PREVIOUS_CLEAR_MASK;
        entry_with_previous = entry_with_previous | ((previous_index & MENU_ITEM_NAV_INDEX_MASK) << MENU_ITEM_NAV_PREVIOUS_SHIFT);
        *entry = entry_with_previous;
        next_index = entry_index + 1;
        wrapped_next_index = 0;
        if (next_index < item_count)
        {
            wrapped_next_index = next_index;
        }
        *entry = (entry_with_previous & MENU_ITEM_NAV_NEXT_CLEAR_MASK) | (wrapped_next_index << MENU_ITEM_NAV_NEXT_SHIFT);
    }
    return item_count;
}

/* ----- Equipment Selection Helpers ----- */

/**
 * @brief Stage the best slot-0 equipment candidate for comparison.
 * @return 1 when a candidate was staged, otherwise 0.
 */
s32 menu_stage_best_equipment_for_slot0(void)
{
    MenuItemEntry* candidate;
    MenuItemEntry* slot_record;
    MenuItemEntry* slot_buffer;
    s32 records_differ;

    menu_stage_stack_shape(0, 0, 0, 0, 0, 0);

    candidate = menu_find_best_equipment_for_slot0();
    if (candidate != 0)
    {
        slot_record = (MenuItemEntry*)((g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE) + (s32)g_pad_ctx + 0x640);
        records_differ = menu_item_is_nondefault(slot_record);
        if (records_differ == 0)
        {
            func_800A8F8C(&menu_character_record(g_pad_ctx, g_menu_char_slot)->items[0], candidate);
            candidate->active = 0;
            g_item_slot_data[0] = 0;
        }
        else
        {
            slot_buffer = (MenuItemEntry*)((g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE) + (s32)g_pad_ctx + 0x640);
            menu_swap_item_records(slot_buffer, candidate);
            g_item_slot_data[0] = (u32)candidate;
        }
        g_item_slot_flags[0] = 1;
        return 1;
    }
    return 0;
}

/**
 * @brief Find a slot-0 upgrade compatible with the other equipped items.
 * @return First highest-valued inventory upgrade, or NULL if none improves the current item.
 */
MenuItemEntry* menu_find_best_equipment_for_slot0(void)
{
    s32 item_index;
    s32 category_mask;
    s32 item_value;
    s32 best_value;
    u32 item_attributes;
    MenuItemEntry* item;
    MenuItemEntry* best_item;

    best_value = 0;
    best_item = &menu_character_record(g_pad_ctx, g_menu_char_slot)->items[0];
    if (best_item->active != 0)
    {
        best_value = best_item->stat_values[0];
    }
    item = (MenuItemEntry*)((u8*)g_pad_ctx + MENU_ITEM_TABLE_OFFSET);
    best_item = 0;
    category_mask = menu_get_equipment_ability_mask(0);
    for (item_index = 0; item_index < MENU_ITEM_TABLE_COUNT; item_index++, item++)
    {
        if (item->active != 0)
        {
            item_attributes = item->attributes.packed;
            if (!(item_attributes & MENU_ITEM_KIND_MASK) &&
                !(category_mask & D_800F0BE0[(item_attributes >> MENU_ITEM_CATEGORY_SHIFT) & MENU_ITEM_CATEGORY_MASK]))
            {
                item_value = item->stat_values[0];
                if (best_value < item_value)
                {
                    best_item = item;
                    best_value = item_value;
                }
            }
        }
    }
    return best_item;
}

/**
 * @brief Stage the best active-slot upgrade, preserving any displaced item.
 * @return 1 if an upgrade was staged, otherwise 0.
 */
s32 menu_stage_best_equipment_for_active_slot(void)
{
    MenuItemEntry* candidate;
    u32* slot_data;
    PadContext* pad_context;
    s32 subtype_index;
    s32 slot_index;
    if (0)
    {
        func_800A8F8C(0, 0, 0, 0, 0, 0);
    }
    candidate = menu_find_best_equipment_for_active_slot();
    if (candidate != 0)
    {
        slot_index = 1;
        subtype_index = g_menu_active_subtype - MENU_EQUIPMENT_SUBTYPE_BASE;
        pad_context = g_pad_ctx;
        if (menu_equipped_item(pad_context, subtype_index)->active == 0)
        {
            slot_data = &g_item_slot_data[0];
            for (; slot_index < MENU_EQUIPMENT_SLOT_COUNT; slot_index += 1)
            {
                if ((u32)candidate == slot_data[slot_index])
                {
                    slot_data[slot_index] = (u32)((u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0) + ((g_menu_active_subtype << 6) - 0x170));
                    break;
                }
            }
            func_800A8F8C((u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0) + ((g_menu_active_subtype << 6) - 0x170), candidate);
            candidate->active = 0;
            g_item_slot_data[g_menu_active_subtype - MENU_EQUIPMENT_SUBTYPE_BASE] = 0;
        }
        else
        {
            slot_data = &g_item_slot_data[0];
            for (; slot_index < MENU_EQUIPMENT_SLOT_COUNT; slot_index += 1)
            {
                if ((u32)candidate == slot_data[slot_index])
                {
                    slot_data[slot_index] = (u32)((u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0) + ((g_menu_active_subtype << 6) - 0x170));
                    break;
                }
            }
            menu_swap_item_records((MenuItemEntry*)((g_menu_char_slot * MENU_CHARACTER_BLOCK_SIZE) + (s32)g_pad_ctx + (g_menu_active_subtype << 6) + 0x480), candidate);
            g_item_slot_data[g_menu_active_subtype - MENU_EQUIPMENT_SUBTYPE_BASE] = (u32)candidate;
        }
        g_item_slot_flags[g_menu_active_subtype - MENU_EQUIPMENT_SUBTYPE_BASE] = 1;
        return 1;
    }
    return 0;
}

/**
 * @brief Find an active-slot upgrade by the sum of its four equipment values.
 * @return First highest-valued compatible inventory upgrade, or NULL if none improves the current item.
 */
MenuItemEntry* menu_find_best_equipment_for_active_slot(void)
{
    s32 item_index;
    s32 excluded_categories;
    s32 item_value;
    s32 best_value;
    u32 item_attributes;
    MenuItemEntry* item;
    MenuItemEntry* best_item;

    best_item = &menu_character_record(g_pad_ctx, g_menu_char_slot)->items[g_menu_active_subtype - MENU_EQUIPMENT_SUBTYPE_BASE];
    if (best_item->active == 0)
    {
        best_value = 0;
    }
    else
    {
        best_value = best_item->stat_values[0] + best_item->stat_values[1] + best_item->stat_values[2] + best_item->stat_values[3];
    }
    best_item = 0;
    item = (MenuItemEntry*)((u8*)g_pad_ctx + MENU_ITEM_TABLE_OFFSET);
    excluded_categories = menu_get_equipment_ability_mask(g_menu_active_subtype - MENU_EQUIPMENT_SUBTYPE_BASE);
    for (item_index = 0; item_index < MENU_ITEM_TABLE_COUNT; item_index++, item++)
    {
        if (item->active != 0)
        {
            item_attributes = item->attributes.packed;
            if (((item_attributes & MENU_ITEM_KIND_MASK) == 0x100) &&
                !(excluded_categories & D_800F0BEC[(item_attributes >> MENU_ITEM_CATEGORY_SHIFT) & MENU_ITEM_CATEGORY_MASK]))
            {
                item_value = item->stat_values[0] + item->stat_values[1] + item->stat_values[2] + item->stat_values[3];
                if (best_value < item_value)
                {
                    best_item = item;
                    best_value = item_value;
                }
            }
        }
    }
    return best_item;
}

/* ----- Menu Utilities ----- */

/**
 * @brief Play a menu sound effect, unless a menu script is currently driving input.
 * @param sound_id Sound effect ID (see the MENU_SE_ constants in menu.c).
 * @param volume Playback volume (menu callers always pass 0x80).
 */
void menu_play_se(s32 sound_id, s32 volume)
{
    if (g_active_script == 0)
    {
        func_800A3938(sound_id, volume);
    }
}

/**
 * @brief Count occupied records at the start of the inventory item table.
 * @return Number of occupied records before the first empty entry.
 */
s32 menu_count_inventory_items(void)
{
    s32 count;
    MenuItemEntry* item;

    item = (MenuItemEntry*)((u8*)g_pad_ctx + MENU_ITEM_TABLE_OFFSET);
    for (count = 0; count < MENU_ITEM_TABLE_COUNT; count++)
    {
        if (item->active == 0)
        {
            break;
        }
        item += 1;
    }
    return count;
}

/**
 * @brief Draw a number with an upper limit of 99.
 * @param ot_entry Ordering-table entry for the emitted primitives.
 * @param packet_cursor Primitive write cursor.
 * @param value Number to draw; values above 99 are capped, negative values pass through.
 * @param format Renderer option, passed through; meaning unconfirmed.
 * @param origin Number position, passed through to the renderer.
 * @param style Renderer style, passed through unchanged.
 * @return Updated primitive write cursor.
 */
s32 menu_draw_clamped_number(s32* ot_entry, s32 packet_cursor, s32 value, s32 format, Vec2s* origin, s32 style)
{
    if (value >= 100)
    {
        value = 99;
    }
    return func_800A8A78(ot_entry, packet_cursor, value, format, origin, style);
}
